all: objects
	#g++ -Wall notifymidi.cpp song.o midifile.o util.o results.o data.o geometric_P3.o algorithms.o vindex_array.o partial.o -o notifymidi -O2
	#g++ -Wall create_note_database.cpp song.o midifile.o util.o results.o data.o geometric_P3.o algorithms.o vindex_array.o partial.o -o create_note_database -O2
	g++ -Wall  partial.cpp song.o midifile.o util.o results.o data.o geometric_P2.o geometric_P3.o algorithms.o vindex_array.o -std=c++11 -o partial -O2

objects:
	gcc song.c -g -c -std=gnu99 -o song.o
	gcc align.c -g -c -o align.o
	gcc midifile.c -g -c -o midifile.o
	gcc util.c -g -c -o util.o
	gcc results.c -g -c -o results.o
	gcc data.c -g -c -D VINDEX_ARRAY -o data.o
	gcc geometric_P2.c -g -c -o geometric_P2.o
	gcc geometric_P3.c -g -c -o geometric_P3.o
	gcc algorithms.c -g -c -o algorithms.o
	gcc vindex_array.c -g -c -o vindex_array.o
	g++ -Wall partial.cpp -c -std=c++11 -o partial.o 

clean:
	rm *.o
	rm test
