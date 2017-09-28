"""
Script to count the number of ocurrences of each pattern for each genre.
"""

import json
from fractions import gcd
from os import listdir
from os.path import isfile, join, basename
from collections import Counter

strd_tpqn = 96
def encode_pattern(ticks, tpqn, normalize=False):
    min_tick = min(ticks)
    ticks = map(lambda x:(x - min_tick), ticks)
    ticks = map(lambda x:x*strd_tpqn/tpqn, ticks)
    last = -100
    new_ticks =[]
    for i in ticks:
#        if i - last < 3:
#        if i - last < 3 or i - last > 1000:
        if i - last > 3:
            new_ticks.append(i)
            last = i
    last = -100
    ticks = map(lambda x:x*6/strd_tpqn, new_ticks)
    new_ticks =[]
    for i in ticks:
        if i - last !=0 and i <22:
            new_ticks.append(i)
            last = i
    ticks = new_ticks
    if len(ticks) < 3:
        return ""
    # Normalize?
    if normalize == True:
        cd_ticks = reduce(gcd, ticks)
        if cd_ticks != 0:
            ticks = map(lambda x:x/cd_ticks, ticks)
    last = 100
    for i in ticks:
        if i - last > 24:
            return ""
        last = i
    return "|".join(map(str, ticks))


def load_patterns (mypath, genres_map, tpqn_map):

    onlyfiles = [f for f in listdir(mypath) if isfile(join(mypath, f))]
    genres_patterns = {"masd":{}, "magd":{}, "topmagd":{}}
    genres_patterns_norm = {"masd":{}, "magd":{}, "topmagd":{}}
    for f in onlyfiles:
        filename = basename(f)
        print mypath + f
        try:
            pattern = json.load(open(mypath + f))
            lakh_id = filename.replace(".json", "")
            for genre_dataset in ["masd", "magd", "topmagd"]:
                if lakh_id in genres_map[genre_dataset]:
                    for genre in genres_map[genre_dataset][lakh_id]:
                        for key in pattern.keys():
                            if key not in [ "jsDsRhythm", "status"]:
                                normalized_patterns = set()
                                normalized_patterns_norm = set()
                                for patterns_list in pattern[key]:
                                    if len(patterns_list) >2 and len(patterns_list) < 20:
                                        ticks = [point[0] for point in patterns_list]
                                        encoded_pattern_norm = encode_pattern(ticks, tpqn_map[lakh_id], True)
                                        ticks = [point[0] for point in patterns_list]
                                        encoded_pattern = encode_pattern(ticks, tpqn_map[lakh_id], False)
                                        if encoded_pattern != "":
                                            normalized_patterns.add(encoded_pattern)   
                                        if encoded_pattern_norm != "":
                                            normalized_patterns_norm.add(encoded_pattern_norm)   
                                genres_patterns[genre_dataset].setdefault(key, {}).setdefault(genre, []).extend(list(normalized_patterns))
                                genres_patterns_norm[genre_dataset].setdefault(key, {}).setdefault(genre, []).extend(list(normalized_patterns_norm))
        except ValueError:
            print "Error"
    return genres_patterns, genres_patterns_norm

def group_patterns_by_genre(genres_patterns):
    output = {"masd":{}, "magd":{}, "topmagd":{}}
    for genre_dataset in genres_patterns.keys():
        for key in genres_patterns[genre_dataset].keys():
            output[genre_dataset][key] = {}
            for genre in genres_patterns[genre_dataset][key].keys():
                c = Counter(genres_patterns[genre_dataset][key][genre])
                output[genre_dataset][key][genre] = {}
                for p, v in list(c.items()):
                    output[genre_dataset][key][genre][p] = v
    return output

def count_patterns():
    siapath = "patterns/rhythm/"
    p2path = "patterns_p2/rhythm/"
    p2path_ext = "patterns_p2_ext/rhythm/"
    
    genres_map = json.load(open("midi_genre_map.json"))
    tpqn_map = json.load(open("tpqn.json"))    
   

    genres_patterns_p2_ext, genres_patterns_p2_ext_norm = load_patterns(p2path_ext, genres_map, tpqn_map)
    output_p2_ext = group_patterns_by_genre(genres_patterns_p2_ext)
    json.dump(genres_patterns_p2_ext, open("patterns_p2_ext.json", "w"))
    json.dump(output_p2_ext, open("genres_patterns_p2_ext.json", "w"))
    """
    genres_patterns_sia, genres_patterns_sia_norm = load_patterns(siapath, genres_map, tpqn_map)
    genres_patterns_p2, genres_patterns_p2_norm = load_patterns(p2path, genres_map, tpqn_map)
    output_sia = group_patterns_by_genre(genres_patterns_sia)
    output_sia_norm = group_patterns_by_genre(genres_patterns_sia_norm)
    output_p2 = group_patterns_by_genre(genres_patterns_p2)
    output_p2_norm = group_patterns_by_genre(genres_patterns_p2_norm)

    json.dump(genres_patterns_sia, open("patterns_sia.json", "w"))
    json.dump(genres_patterns_sia_norm, open("patterns_sia_norm.json", "w"))
    json.dump(genres_patterns_p2, open("patterns_p2.json", "w"))
    json.dump(genres_patterns_p2_norm, open("patterns_p2_norm.json", "w"))
    json.dump(output_p2, open("genres_patterns_p2.json", "w"))
    json.dump(output_p2_norm, open("genres_patterns_p2_norm.json", "w"))
    json.dump(output_sia, open("genres_patterns_sia.json", "w"))
    json.dump(output_sia_norm, open("genres_patterns_sia_norm.json", "w"))
    """

if __name__ == "__main__":
    count_patterns()
