# patterns-genres


P2 algorithm
------------


The code in the folder p2_family was originally written by Mika Laitinen, some modification were made by Andres Ferraro in order to split the input midi file and query using the note sequences. 

The scripts generate_patterns_tonic.py and generate_patterns.py where used to run the P2 algorithm over the Lakh dataset of midis, only to the songs annotated by genre. 
To run this scripts first the code needs to be compiled by running `make all`

The information of which midi files should be processed is in the file midi_w_genre.json. The output will be one json file per Midi file and will be placed in the folder specified in the script.

Sia algorithm
-------------

The code in the folder siatec was originally written by David Meredith, and has been modified by Andrés Ferraro on Aug-2017.

The main is located in the file SiaMain.java, where initially the folder containing the midis of the Lakh dataset is loaded and only the files with genre annotations are processed. The information of which files contains genre annotations is read from the file midi_w_genre.json.

To compile the code execute:

`ant -f . -Dnb.internal.action.name=rebuild clean jar`

Finally, the script siatec.sh could be used to run the code.

Scripts
-------

Inside the folder "scripts" is located the code used to make the classification by genre using the different patters, and a script used to count the different patters found for each algorithm.

Data
-----

The data folder contains 3 json files with the genre annotations and bit per quarter note of each midi file.

