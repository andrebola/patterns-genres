import tempfile
from subprocess import call
import json
import os
from os import listdir
from os.path import isfile, join, basename
from random import shuffle

midis_location = "/PATH/TO/lmd_full/"
p2_location = "/PATH/TO/p2_family/partial"
dest_location = "/PATH/TO/patterns_p2/chroma/"

def main():

    midis_genre = json.load(open("/PATH/TO/midi_w_genre.json"))
    folders = os.listdir(midis_location)
    shuffle(folders)
    for folder in folders:
        onlyfiles = [f for f in listdir(join(midis_location, folder)) if isfile(join(midis_location, folder, f))]
        for f in onlyfiles:
            try:
                midifile = join(midis_location, folder, f)
                lakh_filename = basename(f)
                destfile = join(dest_location, lakh_filename.replace(".mid", ".json"))
                if (not os.path.isfile(destfile)):
                    if lakh_filename.replace(".mid", "") in midis_genre:
                        patterns_keys = {"3": "P2tonic3", "4": "P2tonic4", "5": "P2tonic5", "8": "P2tonic8", "10": "P2tonic10", "15": "P2tonic15b", "20": "P2tonic20", "25": "P2tonic25"}
                        output = {}
                        for length, window, similarity  in [("3","2","0.9"), ("4","2","0.9"), ("5","3","0.5"), ("8","3","0.5"), ("10","3","0.5"), ("15","3","0.5"), ("20","3","0.5"), ("25","3","0.5")]:
                            new_file, filename = tempfile.mkstemp()
                            return_code = call([p2_location + " ".join(["", midifile, filename, length, window, similarity])], shell=True)
                            patterns = json.load(open(filename))
                            os.close(new_file)
                            os.unlink(filename)
                            output[patterns_keys[length]] = patterns

                        json.dump(output, open(destfile, "w"))
            except ValueError:
                os.close(new_file)
                os.unlink(filename)
                print "error on file %s" % midifile

if __name__ == "__main__":
    main()
