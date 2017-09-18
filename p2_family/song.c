/*
 * song.c - Functions for handling patterns and song collections
 *
 * Version 2007-08-21
 *
 *
 * Copyright (C) 2007 Niko Mikkila
 *
 * University of Helsinki, Department of Computer Science, C-BRAHMS project
 *
 * Contact: mikkila@cs.helsinki.fi
 *
 *
 * This file is part of geometric-cbmr,
 * C-BRAHMS Geometric algorithms for Content-Based Music Retrieval.
 *
 * Geometric-cbmr is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the Free
 * Software Foundation; either version 2 of the License, or (at your option)
 * any later version.
 *
 * Geometric-cbmr is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 * or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * geometric-cbmr; if not, write to the Free Software Foundation, Inc., 59
 * Temple Place, Suite 330, Boston, MA 02111-1307 USA
 */


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>

#include "config.h"
#include "algorithms.h"
#include "data.h"
#include "song.h"
#include "util.h"


/**
 * Frees memory used by a song.
 *
 * @param s the song to free
 */
void free_song(song *s) {
    if (s->notes != NULL) {
        free(s->notes);
        s->notes = NULL;
    }
    if (s->title != NULL) {
        free(s->title);
        s->title = NULL;
    }
}

/**
 * Initializes a song.
 *
 * @param s the song to initialize
 * @param id song id in the song collection
 * @param title song title string
 * @param maxnotes maximum number of notes in the song
 */
void init_song(song *s, int id, const char *title, int maxnotes) {
    s->id = id;
    if (title != NULL) {
        s->title = (char *) malloc((strlen(title) + 1) * sizeof(char));
        strcpy(s->title, title);
    } else s->title = NULL;
    s->notes = (vector *) malloc(maxnotes * sizeof(vector));
    s->size = 0;
    s->duration = 0;
    memset(&s->instruments, 0, sizeof(instrumentdata));
    memset(&s->control, 0, sizeof(mididata));
    memset(&s->tempo, 0, sizeof(mididata));
}


/**
 * Frees memory used by a song collection.
 *
 * @param sc the song collection to free
 */
void free_song_collection(songcollection *sc) {
    int i;
    free_song_collection_data(sc);
    if (sc->songs != NULL) {
        for (i=0; i<sc->size; ++i) {
            free_song(&sc->songs[i]);
        }
        free(sc->songs);
    }
}



/**
 * Allocates memory for a song collection.
 *
 * @param sc the song collection to initialize
 * @param maxsongs maximum number of songs
 *
 * @return 1 if successful, 0 otherwise
 */
int init_song_collection(songcollection *sc, int maxsongs) {
    int i;
    if (maxsongs > 0) {
        sc->songs = (song *) calloc(maxsongs, sizeof(song));
    } else sc->songs = NULL;
    sc->size = 0;
    sc->num_notes = 0;
    sc->max_song_size = 0;
    for (i=0; i<=NUM_DATA_FORMATS; ++i) sc->data[i] = NULL;
    if (sc->songs == NULL) return 0;
    else return 1;
}


/**
 * Reads songs from a text file. After this, consider using function
 * preprocess_songs on the song collection to sort and quantize the data.
 *
 * Songfile format:
 *   line 1: <number of songs in the file><\n>
 *   n (>1): <name of a song><\n>
 *      n+1: <number of notes in the song><\n>
 *      n+2: note in format: <MIDI pitch (0..127)> <start time> <end time><\n>
 *      n+3: note
 *      ...
 *
 * Example: The beginning of a file with 10 songs and with the first song
 *          having 150 notes
 * 10
 * testsong-1
 * 150
 * 53 0 625
 * 65 625 1874
 * 69 625 1562
 * ...
 *
 *
 * @param songfile name of the file to read
 * @param songcollection the collection to store data to
 *
 * @return the number of songs read
 */
int read_songfile(const char *songfile, songcollection *sc) {
    int i, filesize, size, numofsongs;
    char *buffer, *line, *endptr;

    buffer = read_file(songfile, 2, 1, &filesize);
    if (buffer == NULL) return 0;
    --filesize;

    line = buffer;
    numofsongs = (int) strtol(line, &endptr, 10);
    line = endptr + 1;

    if (!init_song_collection(sc, numofsongs)) {
        fputs("ERROR in read_songfile(): failed to allocate memory\n", stderr);
        free(buffer);
        return 0;
    }

    for (i=0; i<numofsongs; ++i) {
        song *s = &sc->songs[sc->size];

        /* Read the song title */
        if (line >= buffer + filesize) break;
        endptr = strpbrk(line, "\n");
        size = (int) (endptr - line);
        s->title = (char *) malloc((size + 1) * sizeof(char));
        if (s->title == NULL) {
            fputs("ERROR in read_songfile(): failed to allocate memory\n",
                stderr);
            free(buffer);
            return 0;
        }
        strncpy(s->title, line, size);
        s->title[size] = '\0';
        s->id = i;
        line = endptr + 1;

        /* Read the number of notes */
        if (line >= buffer + filesize) break;
        s->size = (int) strtol(line, &endptr, 10);
        line = endptr + 1;

        s->duration = 0;
        if (s->size <= 0) {
            s->notes = NULL;
        } else {
            int j;
            s->notes = (vector *) malloc(sizeof(vector) * s->size);
            if (s->notes == NULL) {
                fputs("ERROR in read_songfile(): failed to allocate memory",
                        stderr);
                free(buffer);
                return 0;
            }
            /* Read the notes */
            for (j=0; j<s->size; ++j) {
                char pitch;
                int start, end;

                if (line >= buffer + filesize) break;
                pitch = (char) strtol(line, &endptr, 10);
                line = endptr;
                if (line >= buffer + filesize) break;
                start = (int) strtol(line, &endptr, 10);
                line = endptr;
                if (line >= buffer + filesize) break;
                end = (int) strtol(line, &endptr, 10);
                line = endptr + 1;
                s->notes[j].ptch = pitch;
                s->notes[j].strt = start;
                s->notes[j].dur = end;
                if (end > s->duration) s->duration = end;
            }
            s->size = j;
        }
        sc->size++;
    }
    free(buffer);
    return sc->size;
}


/**
 * Checks the given song collection for problematic expressions, such as many
 * notes starting at the same time (or overlapping) at the same pitch. This
 * function also sorts notes if they are not in lexicographic order and
 * optionally quantizes note start times and durations.
 *
 * @param sc the song collection to process
 * @param q amount of quantization to apply in milliseconds. 0 or 1 means
 *        no quantization, 125 means that everything will be quantized to
 *        the precision of 1/8 seconds. Quantization is useful for raw
 *        MIDI input when using algorithms P1 and P2.
 * @param shuffle if nonzero, the song collection will be shuffled (songs
 *        reordered randomly)
 * @param notes song collection size in notes. If this is larger than zero,
 *        the collection will be cropped to the size given.
 */
void preprocess_songs(songcollection *sc, int q, int shuffle, int notes) {
    int i, n;
    if (notes > 0) n = notes;
    else n = 0;
    if (shuffle) {
        for (i=0; i<sc->size; ++i) {
            int j = randf() * ((float) (sc->size - 1));
            song s = sc->songs[i];
            sc->songs[i] = sc->songs[j];
            sc->songs[j] = s;
        }
    }
    for (i=0; i<sc->size; ++i) {
        if (sc->songs[i].size > 0) {
            lexicographic_sort(&sc->songs[i]);
            quantize(&sc->songs[i], q);
            remove_overlap(&sc->songs[i]);
        }
        if (notes > 0) {
            n -= sc->songs[i].size;
            if (n <= 0) {
                int s;
                sc->songs[i].size += n;
                ++i;
                s = i;
                for (; i<sc->size; ++i) {
                    free_song(&sc->songs[i]);
                }
                sc->size = s;
                return;
            }
        }
    }
    if ((notes > 0) && (notes != INT_MAX) && (n > 0))
        fputs("Warning in preprocess_songs(): song collection has less notes than requested\n", stderr);
}


/**
 * Sorts the notes of a song lexicographically if they are not already in
 * ascending order.
 *
 * @param s a song whose notes may need sorting
 */
void lexicographic_sort(song *s) {
    vector *n = s->notes;
    int i;
    int laststart;
    char lastpitch;

    if (s->size < 2) return;

    laststart = n[0].strt;
    lastpitch = n[0].ptch;

    /* Check if the notes are already ordered */
    for (i=1; i<s->size; ++i) {
        if ((n[i].strt < laststart) || ((n[i].strt == laststart)
                && (n[i].ptch < lastpitch))) {
            /* Notes are not in order, sort them all */
            qsort(n, s->size, sizeof(vector), compare_notes);
            return;
        }
        laststart = n[i].strt;
        lastpitch = n[i].ptch;
    }
}


/**
 * Compares two notes lexicographically. This is used to preprocess data for
 * the geometric algorithms.
 * 
 * @param aa a note
 * @param bb another note
 * @return -1 if aa comes before bb, 0 if they are the same and 1 if aa comes
 *         after bb
 */
int compare_notes(const void *aa, const void *bb) {
    const vector *a, *b;
    a = (const vector *) aa;
    b = (const vector *) bb;
    if (a->strt < b->strt)
        return -1;
    else if (a->strt > b->strt)
        return 1;
    else {
        if (a->ptch < b->ptch)
            return -1;
        else if (a->ptch > b->ptch)
            return 1;
        else
            return 0;
    }
}


/**
 * Removes notes that start at the same time as another note and shortens
 * overlapping notes: if two notes play at the same pitch level, the first
 * one is shortened to end before the second starts.
 *
 * @param s a song. The note array of this song will be reallocated.
 */
void remove_overlap(song *s) {
    vector *n = s->notes;
    int i;
    int laststart, lastpitch;
    int lastend[128];
    int lastnote[128];
    int notes_to_remove = 0;
    vector *newnotes;

    if (s->size < 2) return;

    /* Initialize */
    laststart = n[0].strt;
    lastpitch = n[0].ptch;
    for (i=0; i<128; ++i) {
        lastend[i] = -1;
        lastnote[i] = 0;
    }
    lastend[lastpitch] = laststart + n[0].dur;

    /* Shorten notes that overlap with later notes and mark completely
       overlapping notes (that are played at the same time and pitch as a note
       before them) for removal. */
    for (i=1; i<s->size; ++i) {
        int curstart = n[i].strt;
        int curend = curstart + n[i].dur;
        int curpitch = n[i].ptch;

        if ((laststart == curstart) && (lastpitch == curpitch)) {
            ++notes_to_remove;

            /* Grow the previous note's duration to cover the removed note */
            if (lastend[curpitch] < n[i].strt + n[i].dur) {
                lastend[curpitch] = n[i].strt + n[i].dur;
                n[lastnote[curpitch]].dur = n[i].dur;
            }
            /* Mark the note for removal */
            n[i].strt = INT_MAX;
        } else {
            /* Shorten overlapping notes */
            if (lastend[curpitch] > curstart) {
                s->notes[lastnote[curpitch]].dur -= lastend[curpitch] -
                    curstart;
            }
            lastnote[curpitch] = i;
            lastend[curpitch] = curend;
            laststart = curstart;
            lastpitch = curpitch;
            if (n[i].strt == INT_MAX) n[i].strt = INT_MAX - 1;
        }
    }
    if (notes_to_remove > 0) {
        int counter = 0;
        newnotes = (vector *) malloc(sizeof(vector) *
                (s->size - notes_to_remove));
        for (i=0; i<s->size; ++i) {
            if (n[i].strt != INT_MAX) {
                memcpy(&newnotes[counter], &n[i], sizeof(vector));
                ++counter;
            }
        }
        free(n);
        s->notes = newnotes;
        s->size = counter;
    }
}


void p3_optimize_song_collection(songcollection *sc, int mingap, int mindur) {
    int i;
    for (i=0; i<sc->size; ++i) {
        p3_optimize_song(&sc->songs[i], mingap, mindur);
    } 
}

/**
 * Combines notes that play at the same pitch and are separated by a gap
 * of less than mingap milliseconds. Also removes separate notes that are
 * shorter than mindur.
 *
 * @param s a song. The note array of this song will be reallocated.
 */
void p3_optimize_song(song *s, int mingap, int mindur) {
    vector *n = s->notes;
    int i;
    int lastend[128];
    vector *lastnote[128];
    int notes_to_remove = 0;
    vector *newnotes;

    if (s->size < 2) return;

    /* Initialize */
    for (i=0; i<128; ++i) {
        lastend[i] = -mingap;
        lastnote[i] = NULL;
    }

    for (i=0; i<s->size; ++i) {
        int curstart = n[i].strt;
        int curend = curstart + n[i].dur;
        int curpitch = n[i].ptch;

        if (lastnote[curpitch] != NULL) {
            if (lastend[curpitch] + mingap > curstart) {
                lastnote[curpitch]->dur = curend - lastnote[curpitch]->strt;
                n[i].strt = INT_MAX;
                ++notes_to_remove;
                lastend[curpitch] = curend;
            } else {
                if (lastnote[curpitch]->dur < mindur) {
                    lastnote[curpitch]->strt = INT_MAX;
                    ++notes_to_remove;
                }
                lastnote[curpitch] = &n[i];
                lastend[curpitch] = curend;
            }
        } else {
            lastnote[curpitch] = &n[i];
            lastend[curpitch] = curend;
        }
    }
    printf("Song %d: notes before: %d, notes after: %d\n", s->id, s->size, s->size - notes_to_remove);
    if (notes_to_remove > 0) {
        int counter = 0;
        newnotes = (vector *) malloc(sizeof(vector) *
                (s->size - notes_to_remove));
        for (i=0; i<s->size; ++i) {
            if (n[i].strt != INT_MAX) {
                memcpy(&newnotes[counter], &n[i], sizeof(vector));
                ++counter;
            }
        }
        free(n);
        s->notes = newnotes;
        s->size = counter;
    }
    printf("notes after: %d\n", s->size);
}


/**
 * Quantizes the note start times and durations to the closest multiple
 * of the given time interval.
 *
 * @param s a song to quantize
 * @param q time interval in milliseconds
 */
void quantize(song *s, int q) {
    int i, half;
    if (q <= 1) return;
    half = q >> 1;

    for (i=0; i<s->size; ++i) {
        /* Quantize note start times */
        int r = s->notes[i].strt % q;
        if (r > half) s->notes[i].strt += q - r;
        else s->notes[i].strt -= r;

        /* Quantize durations */
        r = s->notes[i].dur % q;
        if (r > half) s->notes[i].dur += q - r;
        else s->notes[i].dur -= r;

        /* Don't allow zero durations */
        if (s->notes[i].dur == 0) s->notes[i].dur = q;
    }
}

void remove_octave_information(song *s) {
    int i;
    vector *notes = s->notes;
    for (i=0; i<s->size; ++i) {
        notes[i].ptch = notes[i].ptch % 12;
    }
}

void sc_remove_octave_information(const songcollection *sc) {
    int i;
    song *songs = sc->songs;
    for (i=0; i<sc->size; ++i) {
        remove_octave_information(&songs[i]);
    }
}


/**
 * Joins all songs in a song collection into one long song. Songs are
 * only joined until the overall duration reaches MAX_SONG_DURATION.
 *
 * @param sc a songcollection whose songs will be joined
 * @param gap length of the gap between consecutive songs, in milliseconds
 * @param num_songs number of new songs to create
 * @param shuffle set to 1 to shuffle the song collection before joining
 *
 * @return a new song collection with num_songs new songs
 */
songcollection *join_songs(songcollection *sc, int gap, int num_songs,
        int song_size, int shuffle) {
    int i, snum, end, numnotes;
    songcollection *nsc = (songcollection *) malloc(sizeof(songcollection));
    song *ns;

    if (nsc == NULL) return NULL;

    init_song_collection(nsc, num_songs);

    /* Count notes */
    numnotes = 0;
    for (i=0; i<sc->size; ++i) {
        numnotes += sc->songs[i].size;
    }
    if (numnotes < song_size) song_size = numnotes;

    for (snum=0; snum<num_songs; ++snum) {
        if (shuffle) preprocess_songs(sc, 0, shuffle, -1);
        ns = &nsc->songs[snum];
        init_song(ns, snum, "", numnotes);
        nsc->size = snum+1;

        numnotes = 0;
        end = 0;
        for (i=0; i<sc->size; ++i) {
            int j = numnotes;
            song *s = &sc->songs[i];
            memcpy(&ns->notes[numnotes], s->notes,
                    MIN2(s->size, song_size-numnotes) * sizeof(vector));
            numnotes += MIN2(s->size, song_size-numnotes);
            for (; j<numnotes; ++j) {
                ns->notes[j].strt += end;
                if (ns->notes[j].strt + ns->notes[j].dur >= MAX_SONG_DURATION) {
                    fputs("Warning: Maximum song duration reached\n", stderr);
                    ns->size = j;
                    ns->duration = ns->notes[j-1].strt + ns->notes[j-1].dur;
                    goto next_song;
                }
            }
            end = ns->notes[numnotes-1].strt + ns->notes[numnotes-1].dur + gap;
            if (numnotes >= song_size) break;
        }
        ns->size = numnotes;
        ns->duration = end;
next_song:
        fprintf(stderr,"Song duration: %d\n", ns->duration);
    }
    return nsc;
}


void update_song_collection_statistics(songcollection *sc) {
    int count, i;
    count = 0;
    sc->max_song_size = 0;
    for (i=0; i<sc->size; ++i) {
        int s = sc->songs[i].size;
        count += s;
        if (s > sc->max_song_size) sc->max_song_size = s;
    }
    sc->num_notes = count;
}

/**
 * Generates a pattern from a song position.
 *
 * @param pattern a pointer to a song structure that will hold the generated
 *        data
 * @param sc a song collection
 * @param m a structure for storing the pattern position in the collection
 * @param length length of the generated pattern
 *
 * @return 1 if successful, 0 otherwise
 */
int get_pattern(song *pattern, const song *s, match *m,
        int length, int position) {
    int sindex = -1;
    int i, lastpos, p, start, errorcount;
    char minpitch, maxpitch;
    char *error_positions;


    if (s == NULL) {
        fputs("Error in generate_pattern(): unable to find a song that is long enough\n", stderr);
        fprintf(stderr, "  Requested pattern size was %d\n", length);
        pattern->notes = NULL;
        pattern->size = 0;
        return 0;
    }

    if (length < 0) {
        fputs("Error in get_pattern(): wrong argument length\n",
                stderr);
        return 0;
    }
    if (position+length >= s->size ) {
        fputs("Error in get_pattern(): wrong argument position+length\n",
                stderr);
        return 0;
    }
    if (position < 0) {
        fputs("Error in get_pattern(): wrong argument position\n",
                stderr);
        return 0;
    }

    /* Allocate memory for the pattern */
    pattern->notes = (vector *) calloc(length, sizeof(vector));
    if (pattern->notes == NULL) {
        fputs("Error in generate_pattern(): failed to allocate memory\n",
                stderr);
        return 0;
    }
    pattern->title = (char *) malloc(strlen(s->title) + 1);
    if (pattern->title == NULL) {
        fputs("Error in generate_pattern(): failed to allocate memory\n",
                stderr);
        free(pattern->notes);
        return 0;
    }
    strcpy(pattern->title, s->title);

    pattern->duration = 0;
    start = s->notes[position].strt;
    minpitch = NOTE_PITCHES - 1;
    maxpitch = 0;

    /* Fill in match information */
    m->song = sindex;
    m->start = start;
    m->num_notes = length;
    m->notes = (int *) malloc(length * sizeof(int));
    if (m->notes == NULL) {
        fputs("Error in generate_pattern(): failed to allocate memory\n",
                stderr);
        free(pattern->notes);
        free(pattern->title);
        return 0;
    }

    m->similarity = 1.0F;


    char last_note_pitch = -1;
    int last_note_tick = -1;
    int pattern_pos = 0;
    for (i=0; i<length; ++i) {
        char pitch = s->notes[position].ptch;
        if (pitch < minpitch) minpitch = pitch;
        if (pitch > maxpitch) maxpitch = pitch;
        
	if ((s->notes[position].strt - start != last_note_tick) || (pitch != last_note_pitch)){
	    last_note_tick = s->notes[position].strt - start;
	    last_note_pitch = pitch;
            pattern->notes[pattern_pos].ptch = pitch;
	    pattern->notes[pattern_pos].strt = s->notes[position].strt - start;
            pattern->notes[pattern_pos].dur = s->notes[position].dur;
	
	    m->notes[pattern_pos] = position;

            ++position;
	    ++pattern_pos;
	    pattern->size = pattern_pos;
            if (position >= s->size) {
                fputs("Error in generate_pattern(): unable to retrieve complete pattern: song eneded.\n", stderr);
                free(pattern->notes);
                pattern->notes = NULL;
                pattern->size = 0;
                return 0;
            }
	}
    }
    m->end = start + pattern->notes[length-1].strt +
            pattern->notes[length-1].dur;
    pattern->duration = m->end - m->start;
}

/**
 * Generates a pattern from a random song in the given song collection.
 *
 * @param pattern a pointer to a song structure that will hold the generated
 *        data
 * @param sc a song collection
 * @param m a structure for storing the pattern position in the collection
 * @param length length of the generated pattern
 * @param maxskip maximum number of notes that the function is allowed to skip
 *        when selecting each consecutive note from a random song in the
 *        collection
 * @param maxtranspose maximum allowed transposition for the whole pattern
 * @param errors percentage of generated errors in the pattern (notes that do
 *        not appear in the original song)
 *
 * @return 1 if successful, 0 otherwise
 */
int generate_pattern(song *pattern, const songcollection *sc, match *m,
        int length, int maxskip, char maxtranspose, float errors) {
    int sindex = -1;
    song *s = NULL;
    int i, lastpos, p, start, errorcount;
    char minpitch, maxpitch;
    char *error_positions;

    if (errors > 1.0F) errors = 1.0F;
    else if (errors < 0.0F) errors = 0.0F;

    /* Select a random song */
    for (i=0; i<100; ++i) {
        int r = rand();
        sindex = (int) ((float) sc->size * ((float) r / (float) RAND_MAX));
        s = &sc->songs[sindex];
        if (s->size > length * 2) {
            break;
        } else s = NULL;
    }
    if (s == NULL) {
        fputs("Error in generate_pattern(): unable to find a song that is long enough\n", stderr);
        fprintf(stderr, "  Requested pattern size was %d\n", length);
        pattern->notes = NULL;
        pattern->size = 0;
        return 0;
    }

    /* Select a random start position */
    i = s->size / maxskip;
    if (i < length) maxskip = s->size / length;
    lastpos = s->size - length * (maxskip + 1) - 1;
    p = 0;
    if (lastpos > 0) {
        int r = rand();
        p = (int) ((float) lastpos * ((float) r / (float) RAND_MAX));
    } else {
        maxskip = 0;
        if (length > s->size - 2) {
            length = s->size;
        } else {
            int r = rand();
            lastpos = s->size - length - 1;
            p = (int) ((float) lastpos * ((float) r / (float) RAND_MAX));
        }
    }

    /* Allocate memory for the pattern */
    pattern->notes = (vector *) calloc(length, sizeof(vector));
    if (pattern->notes == NULL) {
        fputs("Error in generate_pattern(): failed to allocate memory\n",
                stderr);
        return 0;
    }
    pattern->title = (char *) malloc(strlen(s->title) + 1);
    if (pattern->title == NULL) {
        fputs("Error in generate_pattern(): failed to allocate memory\n",
                stderr);
        free(pattern->notes);
        return 0;
    }
    strcpy(pattern->title, s->title);

    pattern->duration = 0;
    pattern->size = length;
    start = s->notes[p].strt;
    minpitch = NOTE_PITCHES - 1;
    maxpitch = 0;

    /* Fill in match information */
    m->song = sindex;
    m->start = start;
    m->num_notes = length;
    m->notes = (int *) malloc(length * sizeof(int));
    if (m->notes == NULL) {
        fputs("Error in generate_pattern(): failed to allocate memory\n",
                stderr);
        free(pattern->notes);
        free(pattern->title);
        return 0;
    }

    m->similarity = 1.0F - errors;


    for (i=0; i<length; ++i) {
        char pitch = s->notes[p].ptch;
        if (pitch < minpitch) minpitch = pitch;
        if (pitch > maxpitch) maxpitch = pitch;
        pattern->notes[i].ptch = pitch;
        pattern->notes[i].strt = s->notes[p].strt - start;
        pattern->notes[i].dur = s->notes[p].dur;

        m->notes[i] = p;

        if (maxskip > 0) {
            int r = rand();
            p += 1 + (int) (((float) maxskip) * (((float) r) / ((float) RAND_MAX)));
        } else ++p;
        if (p >= s->size) {
            fputs("Error in generate_pattern(): unable to retrieve complete pattern: song eneded.\n", stderr);
            free(pattern->notes);
            pattern->notes = NULL;
            pattern->size = 0;
            return 0;
        }
    }
    m->end = start + pattern->notes[length-1].strt +
            pattern->notes[length-1].dur;
    pattern->duration = m->end - m->start;

    /* Transpose the notes if needed and possible */
    if (maxtranspose != 0) {
        int r = rand();
        if (minpitch - maxtranspose < 0)
            maxtranspose = minpitch;
        if ((int) maxpitch + (int) maxtranspose >= NOTE_PITCHES)
            maxtranspose = NOTE_PITCHES - maxpitch - 1;
        maxtranspose = maxtranspose - (char) ((float) maxtranspose * 2.0 *
                (r / (float) RAND_MAX));
        for (i=0; i<length; ++i) {
            pattern->notes[i].ptch = pattern->notes[i].ptch + maxtranspose;
        }
        m->transposition = -maxtranspose;
    }

    /* Change some notes to simulate errors */
    errorcount = (float) length * errors;
    error_positions = malloc(length * sizeof(char));

    for (i=0; i<length; ++i) error_positions[i] = 0;
    for (i=0; i<errorcount; ++i) {
        int j, r;
        r = rand();
        p = (int) ((float) (length - i) * ((float) r / (float) RAND_MAX));
        for (j=0; j<length; ++j) {
            if (error_positions[j] == 0) {
                if (p <= 0) {
                    unsigned int next, prev;
                    /* Modify note at position j */
                    error_positions[j] = 1;

                    /* Change start time */
                    if (j > 0) prev = pattern->notes[j-1].strt;
                    else prev = 0;
                    if (j < length - 1) next = pattern->notes[j+1].strt;
                    else next = pattern->notes[j].strt + 4000;
                    r = rand();
                    next = prev + (unsigned int) ((float)
                        (next - prev) * ((float) r / (float) RAND_MAX));
                    pattern->notes[j].strt = next;

                    /* Change duration */
                    r = rand();
                    pattern->notes[j].dur = (int) ((float)
                            pattern->notes[j].dur * 2.0F *
                            ((float) r / (float) RAND_MAX));

                    /* Change pitch slightly if there is room */
                    if (next != prev) {
                        int pitch = pattern->notes[j].ptch;
                        if ((pitch > 1) && (pitch < 126)) {
                            r = rand();
                            pattern->notes[j].ptch += 2 - (char) (5.0F *
                                   ((float) r / (float) RAND_MAX));
                        }
                    }

                    /* Mark the changed note to the match information */
                    m->notes[j] = -1;

                    /* Break the loop */
                    j = length;
                }
                --p;
            }
        }
    }
    if (errorcount > 0) {
        /*fprintf(stderr, "sorting...");*/
        /* Make sure the pattern is ordered. It should be. */
        lexicographic_sort(pattern);
    }
    free(error_positions);
    return 1;
}


/**
 * Generates a collection of patterns from the given song.
 *
 * @param pc pointer to a structure that will hold the generated data (patterns)
 * @param sc a song collection
 * @param ms a structure for storing pattern positions in the song collection
 * @param length of a pattern
 * @param window number of notes of overlap between patterns
 *
 * @return number of patterns generated, 0 if there was an error
 */
int generate_patterns_song(songcollection *pc, const song *s, matchset *ms, int length, int window) {
    int i;
    int patterncount = (length/window) * (s->size / length);
    if (!init_song_collection(pc, patterncount)) {
        fputs("ERROR in generate_patterncollection(): failed to allocate memory",
                stderr);
        return 0;
    }

    if (!init_match_set(ms, patterncount, 0, 1)) {
        free_song_collection(pc);
        return 0;
    }
    int k = 0;
    for (int j=0; j < (length / window); ++j) {
	    for (i=0; i < (s->size / length); ++i) {

		int position = (i*length) + (window * j);
		if (position + length < s->size){
			if (!get_pattern(&pc->songs[k], s, &ms->matches[k], length, position)) {
			    for (k=k-1; k>=0; --k) {
				free_song(&pc->songs[k]);
			    }
			    free_match_set(ms);
			    return 0;
			}
			pc->songs[k].id = k;
			ms->num_matches++;
			k++;
	        }
	    }
    }
    pc->size = k;
    return k;
}


/**
 * Generates a collection of random patterns from the given song collection.
 *
 * @param patterncount number of patterns to generate
 * @param pc pointer to a structure that will hold the generated data
 * @param sc a song collection
 * @param ms a structure for storing pattern positions in the song collection
 * @param minlength minimum length of a pattern
 * @param maxlength maximum length of a pattern
 * @param maxskip maximum number of notes that the function is allowed to skip
 *        when selecting each consecutive note from a random song in the
 *        collection
 * @param maxtranspose maximum allowed transposition for a pattern
 * @param errors percentage of generated errors in each pattern (notes that do
 *        not appear in the original song)
 *
 * @return number of patterns generated, 0 if there was an error
 */
int generate_pattern_collection(int patterncount, songcollection *pc,
        const songcollection *sc, matchset *ms, int minlength, int maxlength,
        int maxskip, char maxtranspose, float errors) {
    int i;
    if (!init_song_collection(pc, patterncount)) {
        fputs("ERROR in generate_patterncollection(): failed to allocate memory",
                stderr);
        return 0;
    }

    if (!init_match_set(ms, patterncount, 0, 0)) {
        free_song_collection(pc);
        return 0;
    }

    for (i=0; i<patterncount; ++i) {
        int r = rand();
        int l;
        if (maxlength - minlength > 0) {
            l = minlength + (int) ((float) (maxlength - minlength) *
                ((float) r / (float) RAND_MAX));
        } else {
            l = minlength;
        }
        if (!generate_pattern(&pc->songs[i], sc, &ms->matches[i], l, maxskip,
                maxtranspose, errors)) {
            for (i=i-1; i>=0; --i) {
                free_song(&pc->songs[i]);
            }
            free_match_set(ms);
            return 0;
        }
        pc->songs[i].id = i;
        ms->num_matches++;
    }
    pc->size = patterncount;
    return patterncount;
}


/**
 * Inserts pattern to the given song.
 *
 * @param s a song
 * @param songpos position within the song
 * @param p pattern to insert
 * @param errors ratio of notes that are changed in the inserted pattern
 *        [0.0-1.0]
 * @param noise ratio of added notes in the inserted pattern [>= 0.0]
 */
void insert_pattern_to_song(song *s, int songpos, const song *p, float errors,
        float noise) {
    int i, start, spos;
    int nsize = (int) (noise * ((float) p->size));
    int num_errors = errors * ((float) p->size);
    float error_level = errors;
    float noise_level = noise;
    int ssize = s->size + p->size + nsize;
    vector *snotes = (vector *) malloc(ssize * sizeof(vector));
    if (snotes == NULL) {
        fputs("Error in insert_pattern_to_song: unable to allocate note buffer\n", stderr);
        return;
    }
    lexicographic_sort(s);
    songpos = MIN2(songpos, s->size);
    spos = songpos;

    /* Copy the first notes */
    memcpy(snotes, s->notes, spos * sizeof(vector));

    if (spos > 0) start = s->notes[spos-1].strt + s->notes[spos-1].dur;
    else start = 0;
    for (i=0; i<p->size; ++i, ++spos) {
        int j;
        int noisestart, noiseend;

        if (p->size - i <= num_errors) error_level = 1.1F;

        memcpy(&snotes[spos], &p->notes[i], sizeof(vector));
        snotes[spos].strt += start;
        noisestart = snotes[spos].strt;

        /* Change some of the notes according to the error rate */
        if ((num_errors > 0) && (randf() < error_level)) {
            /*int pitch;*/
            snotes[spos].strt = MAX2(0, snotes[spos].strt + 10000 -
                    (int) (randf() * 20000.0F));
            /* Change the note pitch by 32 or less */
            /*pitch = snotes[spos].ptch;
            pitch = MIN2(pitch, 127 - pitch);
            pitch = MIN2(32, pitch);
            snotes[spos].ptch = snotes[spos].ptch + pitch -
                    (int) (randf() * (float) (pitch * 2));*/
            snotes[spos].ptch = (char) (randf() * 127.0F);
            num_errors--;
        }

        /* Add extra notes (noise) */
        if (i < (p->size - 1)) {
            noiseend = noisestart + p->notes[i+1].strt - p->notes[i].strt;
        } else {
            noiseend = noisestart + 2000;
            noise_level = (float) nsize + 0.1F;
        }
        if ((nsize > 0) && (noise_level >= 1.0F)) {
            for (j=0; (((float) j)<noise_level) && (nsize > 0); ++j) {
                vector *note = &snotes[++spos];
                note->strt = noisestart + (int) (randf() * (float)
                        (noiseend - noisestart));
                note->dur = (int) (randf() * 2000.0F);
                note->ptch = (char) (randf() * 127.0F);
                note->velocity = (char) (randf() * 127.0F);
                note->instrument = 1;
                --nsize;
            }
            noise_level = 0.0F;
        }
        noise_level += noise;
    }
    if (songpos < s->size) {
        start += p->notes[p->size - 1].strt + p->notes[p->size - 1].dur;
        memcpy(&snotes[spos], &s->notes[songpos], (s->size - songpos) *
            sizeof(vector));
        for (i=spos; i<ssize; ++i) {
            snotes[i].strt += start;
        }
    }
    free(s->notes);
    s->notes = snotes;
    s->size = ssize;
    s->duration = s->notes[ssize-1].strt + s->notes[ssize-1].dur;
    lexicographic_sort(s);
    remove_overlap(s);
}


/**
 * Inserts patterns to the given song collection.
 *
 * @param sc song collection
 * @param pc patterns to insert
 * @param pattern_instances number of times that each pattern is inserted
 * @param errors ratio of notes that are changed in the inserted patterns
 *        [0.0-1.0]
 * @param noise ratio of added notes in inserted patterns [>= 0.0]
 */
void insert_patterns(songcollection *sc, const songcollection *pc,
        int pattern_instances, float errors, float noise) {
    int i;
    for (i=0; i<pc->size; ++i) {
        int j;
        for (j=0; j<pattern_instances; ++j) {
            song *s = &sc->songs[(int) (randf() * (float) (sc->size - 1))];
            int songpos = (int) (randf() * (float) s->size);
            /* Spread error rate so that inserted patterns can be ordered by
               their similarity to the original pattern */
            float e = errors - randf() * (MIN2(errors, 0.25F));
            insert_pattern_to_song(s, songpos, &pc->songs[i], e, noise);
        }
    }
}

/**
 * Frees memory reserved for algorithm-specific song data.
 *
 * @param sc a song collection
 */
void free_song_collection_data(songcollection *sc) {
    int i;
    for (i=1; i<=NUM_DATA_FORMATS; ++i) {
        if (sc->data[i] != NULL) {
            free_data_format(i, sc->data[i]);
            sc->data[i] = NULL;
        }
    }
}


/**
 * Updates alternative data formats and indices within a song collection.
 *
 * @param sc a song collection
 * @param algorithm_list a -1-terminated list of algorithm IDs to define the
 *        required data formats. If NULL, data in all formats is updated.
 */
void update_song_collection_data(songcollection *sc, const int *algorithm_list,
        const dataparameters *dp) {
    const int *alg = algorithm_list;
    free_song_collection_data(sc);
    if (alg == NULL) {
        int i;
        for (i=1; i<=NUM_DATA_FORMATS; ++i) {
            sc->data[i] = init_data_format(i); 
            if (sc->data[i] == NULL) {
                fprintf(stderr, "Error in update_song_collection_data: initializing data format %d failed\n", i);
                return;
            }
            if (!convert_song_collection(i, sc->data[i], sc, dp)) {
                fprintf(stderr, "Error in update_song_collection_data: converting to format %d failed\n", i);
            }
        }
    } else {
        while (*alg > 0) {
            int format = get_algorithm_data_format(*alg);
            if ((format > 0) && (format <= NUM_DATA_FORMATS) &&
                    (sc->data[format] == NULL)) {
                sc->data[format] = init_data_format(format); 
                if (sc->data[format] == NULL) {
                    fprintf(stderr, "Error in update_song_collection_data: initializing data format %d failed\n", format);
                    return;
                }
                if (!convert_song_collection(format, sc->data[format], sc, dp)) {
                    fprintf(stderr, "Error in update_song_collection_data: converting to format %d failed\n", format);
                }
            }
            alg++;
        }
    }
}

