This submission is a Linux submission.

First, use GNU Make to compile the needed source files. Type in

make

and all sources should successfully compile. Then, to run any test, write

chmod a+x submission.sh
./submission.sh /path/to/folder/withMIDIfile/ /path/to/query.mid


- This submission uses only one thread by default.

- Memory usage stays very low as long as there is no one huge file.

- Expected runtime when using each midi-file as a query from the essen-database 
and giving the essen database as folder should be around 10 minutes, probably 
slightly less - therefore one run, for example 

./submission.sh /path/to/essen/midi/ /path/to/essen/midi/some_midi_file.mid

would take around 1 second.

- The cache files used by the script (readfiles.txt) just store all the paths to
the midi-files. Thus the space usage should be very conservative.

- C++ compiler and a shell are required (tested on a bash shell)

Should any problems arise, please contact laitinen.mika@gmail.com
