#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

<<<<<<< HEAD
#include <lv2/lv2plug.in/ns/ext/atom/util.h>
#include <lv2/lv2plug.in/ns/ext/midi/midi.h>
#include <lv2/lv2plug.in/ns/ext/urid/urid.h>
#include <lv2/lv2plug.in/ns/lv2core/lv2.h>
#define TRIGGER_URI "http://www.steffen-thurian.de/lv2/mididrumtrigger"
=======
#include "/usr/include/lv2/lv2plug.in/ns/ext/atom/util.h"
#include "/usr/include/lv2/lv2plug.in/ns/ext/midi/midi.h"
#include "/usr/include/lv2/lv2plug.in/ns/ext/urid/urid.h"
#include "/usr/include/lv2/lv2plug.in/ns/lv2core/lv2.h"
#define TRIGGER_URI "http://www.steffen-thurian.de/lv2/trigger"
>>>>>>> 553a10b0382771dcb42c81b99b89433e3071f2f5

typedef enum {
	TRIGGER_THRESHOLD   = 0,
	TRIGGER_INPUT  = 1,
	TRIGGER_OUTPUT = 2,
	TRIGGER_SCAN_TIME = 3,
	TRIGGER_MASK_TIME = 4,
	TRIGGER_MIDI_NOTE = 5,
	TRIGGER_GAIN = 6,
	TRIGGER_DETECT = 7,
} PortIndex;

typedef enum {
	STATE_MASK = 0,
	STATE_SCAN = 1,
	STATE_MEASURE = 2
} State;

typedef enum {
	AMPLITUDE = 0,
	RMS = 1,
	CONSTANT = 2
} Velocity_Detection;

typedef struct {
	float* threshold;
	float* input;
	LV2_Atom_Sequence* output;
	float* mask_time;
	float* scan_time;
	float max;
	double samplerate;
	float* midi_note;
	float* velocity_detection;
	long elapsed_time;
	float* gain;
	// Features
	LV2_URID_Map* map;
	State state;
	struct {
			LV2_URID midi_MidiEvent;
			LV2_URID atomSequence;
	} uris;
} Trigger;

static LV2_Handle
instantiate(const LV2_Descriptor*     descriptor,
            double                    rate,
            const char*               bundle_path,
            const LV2_Feature* const* features)
{
	Trigger* trigger = (Trigger*)malloc(sizeof(Trigger));
	trigger->state = STATE_SCAN;
	trigger->samplerate = rate;
	trigger->elapsed_time = 0;
	trigger->max = 0;
	LV2_URID_Map* map = NULL;
	for (int i = 0; features[i]; ++i) {
			if (!strcmp(features[i]->URI, LV2_URID__map)) {
					map = (LV2_URID_Map*)features[i]->data;
					break;
			}
	}
	if (!map) {
		return NULL;
	}
	trigger->map = map;
	trigger->uris.atomSequence = map->map(map->handle, LV2_ATOM__Sequence);
	trigger->uris.midi_MidiEvent = map->map(map->handle, LV2_MIDI__MidiEvent);
	return (LV2_Handle)trigger;
}

static void
connect_port(LV2_Handle instance,
             uint32_t   port,
             void*      data)
{
	Trigger* trigger = (Trigger*)instance;

	switch ((PortIndex)port) {
		case TRIGGER_THRESHOLD:
			trigger->threshold = (float*)data;
			break;
		case TRIGGER_SCAN_TIME:
			trigger->scan_time = (float*)data;
			break;
		case TRIGGER_MASK_TIME:
			trigger->mask_time = (float*)data;
			break;
		case TRIGGER_INPUT:
			trigger->input = (float*)data;
			break;
		case TRIGGER_OUTPUT:
			trigger->output = (LV2_Atom_Sequence*)data;
			break;
		case TRIGGER_MIDI_NOTE:
			trigger->midi_note = (float*)data;
			break;
		case TRIGGER_GAIN:
			trigger->gain = (float*)data;
			break;
		case TRIGGER_DETECT:
			trigger->velocity_detection = (float*) data;
			break;
	}
}

static void
activate(LV2_Handle instance)
{
	Trigger* trigger = (Trigger*)instance;
	trigger->elapsed_time = 0;
}

static int maxvelocity(int val)
{
	return val > 127? 127:val;
}

static void sendMidiEvent(Trigger* trigger, uint8_t midiEvent, uint8_t note, uint8_t velocity)
{
	LV2_Atom_Sequence* const output = trigger->output;
	typedef struct {
		LV2_Atom_Event event;
		uint8_t        msg[3];
	} MIDINoteEvent;
	MIDINoteEvent midiNote;
	memset(&midiNote, 0, sizeof(MIDINoteEvent));
	trigger->output->atom.type = trigger->uris.atomSequence;
	const uint32_t out_capacity = trigger->output->atom.size;
	lv2_atom_sequence_clear(trigger->output);
	midiNote.event.body.type   = trigger->uris.midi_MidiEvent;	
	midiNote.event.body.size   = 3;  
	midiNote.msg[0] = midiEvent;  
	midiNote.msg[1] = note; 	
	midiNote.msg[2] = maxvelocity(velocity);
	lv2_atom_sequence_append_event(trigger->output, out_capacity, (LV2_Atom_Event*)&midiNote);
}

static void
run(LV2_Handle instance, uint32_t n_samples)
{	
	Trigger* trigger = (Trigger*)instance;
	for (uint32_t pos = 0; pos < n_samples; pos++)
	{
		trigger->elapsed_time++;		
		switch(trigger->state)
		{
			case STATE_SCAN:
			{
				const float        threshold   = *(trigger->threshold);
				const float* const input  = trigger->input;
				if(fabs(input[pos])>=threshold)
				{
					trigger->elapsed_time = 0;
					trigger->state = STATE_MEASURE;
					trigger->max = fabs(input[pos]);
				}
				break;
			}
			case STATE_MEASURE:
			{
				int method = *trigger->velocity_detection;
				const float* const input  = trigger->input;				
				if(trigger->elapsed_time < ((*trigger->scan_time)/1000)*trigger->samplerate)
				{
					switch(method)
					{
						case AMPLITUDE:
							if(trigger->max < fabs(input[pos]))
								trigger->max = fabs(input[pos]);
							break;
						case RMS:
							trigger->max+=fabs(input[pos])*fabs(input[pos]);
							break;
						case CONSTANT:
							trigger->max=1.0f;
							break;
					}
					
				} else
				{
					if(method == RMS)
					{
						trigger->max = sqrt(trigger->max/trigger->elapsed_time); 
					}
					sendMidiEvent(trigger, LV2_MIDI_MSG_NOTE_ON, *trigger->midi_note,trigger->max*127*(*trigger->gain));
					trigger->state = STATE_MASK;
					trigger->elapsed_time = 0;
				}
				break;
			}
			case STATE_MASK:
			{
				if(trigger->elapsed_time*1.0f >= ((*trigger->mask_time)/1000.0f)*trigger->samplerate)
				{
					sendMidiEvent(trigger, LV2_MIDI_MSG_NOTE_OFF, *trigger->midi_note,0);
					trigger->state = STATE_SCAN;
					trigger->elapsed_time = 0;
				}
				break;
			}
		}
	
	}
}

static void
deactivate(LV2_Handle instance)
{
}

static void
cleanup(LV2_Handle instance)
{
	free(instance);
}

const void*
extension_data(const char* uri)
{
	return NULL;
}

static const LV2_Descriptor descriptor = {
	TRIGGER_URI,
	instantiate,
	connect_port,
	activate,
	run,
	deactivate,
	cleanup,
	extension_data
};

LV2_SYMBOL_EXPORT
const LV2_Descriptor*
lv2_descriptor(uint32_t index)
{
	switch (index) {
	case 0:
		return &descriptor;
	default:
		return NULL;
	}
}
