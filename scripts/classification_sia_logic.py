import json
from os import listdir, remove
from os.path import isfile, join, basename
from collections import Counter
from sklearn.svm import SVC, LinearSVR
from sklearn.linear_model import SGDClassifier, LogisticRegression
import numpy as np
from sklearn.feature_extraction import DictVectorizer
from sklearn import metrics
from sklearn.preprocessing import MultiLabelBinarizer, LabelEncoder 
from sklearn.model_selection import StratifiedKFold, cross_validate
from fractions import gcd
from sklearn.pipeline import Pipeline
from sklearn.decomposition import TruncatedSVD
from sklearn.multiclass import OneVsRestClassifier

def encode_pattern_both(patterns_list, tpqn, normalize=False):
    strd_tpqn = 6
    ticks = [point[0] for point in patterns_list]
    min_tick = min(ticks)
    ticks = map(lambda x:(x - min_tick), ticks)
    ticks = map(lambda x:x*strd_tpqn/tpqn, ticks)
    encoded = ""
    for i in range(len(ticks)):
        encoded += "("
        encoded += str(ticks[i])
        encoded += "|"
        encoded += str(patterns_list[i][1])
        encoded += ")"
    return encoded


def encode_pattern_only_rhythm(patterns_list, tpqn, normalize=False):
    strd_tpqn_first = 96
    strd_tpqn = 6
    ticks = [point[0] for point in patterns_list]
    min_tick = min(ticks)
    ticks = map(lambda x:(x - min_tick), ticks)
    ticks = map(lambda x:x*strd_tpqn_first/tpqn, ticks)
    last = -100
    new_ticks =[]
    for i in ticks:
        if i - last > 3:
            new_ticks.append(i)
            last = i
    last = -100
    ticks = map(lambda x:x*strd_tpqn/strd_tpqn_first, new_ticks)
    new_ticks =[]
    for i in ticks:
        if i - last !=0 and i <22:
            new_ticks.append(i)
            last = i
    ticks = new_ticks
    if len(ticks) < 3:
        return ""
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

def load_data (tonic_path, only_rhythm_path, genres_map, tpqn_map):
    genre_dataset = "topmagd"
    onlyfiles = [f for f in listdir(tonic_path) if isfile(join(tonic_path, f))]
    genres_patterns = {}
    genres_patterns_labels = {}
    for f in onlyfiles:
        filename = basename(f)
        try:
            if only_rhythm_path == None or isfile(join(only_rhythm_path, f)):
                tonic_pattern = json.load(open(tonic_path + f))
                pattern_rhythm = {}
                if only_rhythm_path and isfile(join(only_rhythm_path, f)):
                    pattern_rhythm = json.load(open(only_rhythm_path + f))
                lakh_id = filename.replace(".json", "")

                if lakh_id in genres_map[genre_dataset]:
                        for key in tonic_pattern.keys():
                            if key not in ["jsDsRhythm", "dsChroma"]:
                                normalized_patterns = {}
                                for patterns_list in tonic_pattern[key]:
                                    if len(patterns_list) >2 and len(patterns_list) < 20:

                                        encoded_pattern = encode_pattern_both(patterns_list, tpqn_map[lakh_id])
                                        if encoded_pattern != "":
                                            normalized_patterns.setdefault(encoded_pattern, 0)
                                            normalized_patterns[encoded_pattern] += 1
                                key_rhythm = "PatternsRythm2"
                                if key_rhythm in pattern_rhythm:
                                    for patterns_list in pattern_rhythm[key_rhythm]:
                                        if len(patterns_list) >2 and len(patterns_list) < 20:

                                            encoded_pattern = encode_pattern_only_rhythm(patterns_list, tpqn_map[lakh_id])
                                            if encoded_pattern != "":
                                                normalized_patterns.setdefault(encoded_pattern, 0)
                                                normalized_patterns[encoded_pattern] += 1
                                if len(normalized_patterns):
                                    genres_patterns.setdefault(key, [])
                                    genres_patterns_labels.setdefault(key, [])
                                    genres_patterns[key].append(normalized_patterns)
                                    genres_patterns_labels[key].append(set(genres_map[genre_dataset][lakh_id]))
        except ValueError:
            pass
    return genres_patterns, genres_patterns_labels

def classify(genres_patterns, genres_patterns_labels):
    results = {}
    for key in genres_patterns.keys():
        tr = DictVectorizer()
        conv_X_train = tr.fit_transform(genres_patterns[key])
        clb = MultiLabelBinarizer()
        y = clb.fit_transform(genres_patterns_labels[key])
        skf = StratifiedKFold(n_splits=3)

        clf = OneVsRestClassifier(LogisticRegression(C=1,class_weight='balanced'))
        scoring = {'precision': 'precision_weighted',
                   'recall': 'recall_weighted',
                   'accuracy': 'accuracy',
                   'prec_sampl': 'precision_samples',
                   'rec_sampl': 'recall_samples',
                   'f1_samples': 'f1_samples',
                   'f1_weighted': 'f1_weighted',
                   'roc_auc': 'roc_auc'
         }

        scores = cross_validate(clf, conv_X_train, y, cv=5, scoring=scoring, return_train_score=True)
        for k in scores.keys():
            scores[k] = scores[k].tolist()
        results[key] = scores
        json.dump(results, open("/tmp/classification_tonic.json", "w"))

    return results

def count_patterns(genres_patterns, genres_patterns_labels):
    results = {}
    for key in genres_patterns.keys():
        l = genres_patterns[key]
        results[key] = len(set([item for sublist in l for item in sublist]))
    return results


def main():
    # Load json with annotations of genre for each midi
    genres_map = json.load(open("midi_genre_map.json"))
    # Load json with information of Beat Per Quarter Note of each midi file
    tpqn_map = json.load(open("tpqn.json"))
    # Location of the extracter patterns
    siapath = "patterns/chroma/"
    siapath2 = "patterns/rhythm/"

    genres_patterns, genres_patterns_labels = load_data(siapath, None, genres_map, tpqn_map)
    results = classify(genres_patterns, genres_patterns_labels)
    json.dump(results, open("classification_res_tonic_logi.json", "w"))
    genres_patterns, genres_patterns_labels = load_data(siapath, siapath2, genres_map, tpqn_map)
    results = classify(genres_patterns, genres_patterns_labels)
    json.dump(results, open("classification_res_rhythm_and_tonic_logi.json", "w"))

    # The following lines are used only to count the patterns
    """
    results = count_patterns(genres_patterns, genres_patterns_labels)
    json.dump(results, open("count_patterns_sia_only_tonic.json", "w"))
    genres_patterns, genres_patterns_labels = load_data(siapath, siapath2, genres_map, tpqn_map)
    results = count_patterns(genres_patterns, genres_patterns_labels)
    json.dump(results, open("count_patterns_sia_rhythm_and_tonic.json", "w"))
    """

if __name__ == "__main__":
    main()
