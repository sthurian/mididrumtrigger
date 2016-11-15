BUNDLE = mididrumtrigger.lv2
INSTALL_DIR = /usr/local/lib/lv2


$(BUNDLE): manifest.ttl mididrumtrigger.ttl mididrumtrigger.so
	rm -rf $(BUNDLE)
	mkdir $(BUNDLE)
	cp manifest.ttl mididrumtrigger.ttl mididrumtrigger.so $(BUNDLE)

mididrumtrigger.so: mididrumtrigger.c
<<<<<<< HEAD
	gcc -shared -fPIC -DPIC mididrumtrigger.c -std=c99 -o mididrumtrigger.so
=======
	g++ -shared -fPIC -DPIC mididrumtrigger.c `pkg-config --cflags --libs lv2-plugin` -o mididrumtrigger.so
>>>>>>> 553a10b0382771dcb42c81b99b89433e3071f2f5

install: $(BUNDLE)
	mkdir -p $(INSTALL_DIR)
	rm -rf $(INSTALL_DIR)/$(BUNDLE)
	cp -R $(BUNDLE) $(INSTALL_DIR)

clean:
	rm -rf $(BUNDLE) mididrumtrigger.so
