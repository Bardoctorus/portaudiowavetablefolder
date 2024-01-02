default:
	gcc -o prog main.c -I/usr/local/include -framework CoreServices -framework CoreFoundation -framework AudioUnit -framework AudioToolbox -framework CoreAudio -lportaudio
clean:
	rm -vrf prog 
