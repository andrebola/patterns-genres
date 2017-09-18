/*
 * geometrictest.c - A test program for the geometric algorithms
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
#include <time.h>

#include "geometric_P1.h"
#include "geometric_P2.h"
#include "geometric_P3.h"
#include "util.h"

/* Uncomment to use only monophonic patterns. */
/* #define MONOPHONIC_PATTERN */

/* Note length in time units. */
#define NOTE_LENGTH 10

/* Portion of the notes that should be distorted when generating the third
 * pattern. */
#define DISTORTION_RATE 0.33

/* Uncomment to test the accuracy of P2 and P3 algorithms.
 * Geometrictest should be called repeatedly with different seeds from a shell
 * script (testaccuracy.sh) */
/* #define TESTACCURACY */



/**
 * Generates a random music score.
 *
 * @param s a song item where the generated score will be stored
 * @param duration length of the generated piece of music in time units
 * @param notes number of generated notes
 * @param startpoints pointer to a location where the function should store a
 *        pointer to an array of note start points. The array is reserved in
 *        this function and should be later freed by the caller.
 * @param endpoints pointer to a location where the function should store a
 *        pointer to an array of note end points. The array is reserved in
 *        this function and should be later freed by the caller.
 */
static void generate_score(song *s, unsigned int duration, unsigned int notes,
        TurningPoint **startpoints, TurningPoint **endpoints) {
    unsigned int i, j;
    int *playing_notes = (int *) malloc(128 * sizeof(int));
    vector *text = (vector *) malloc(notes * sizeof(vector));
    (*startpoints) = (TurningPoint *) malloc(notes * sizeof(TurningPoint));
    (*endpoints) = (TurningPoint *) malloc(notes * sizeof(TurningPoint));

    for (i = 0; i < notes; i++) {
        text[i].strt = (unsigned int) ((float) duration * randf());
        text[i].ptch = (unsigned int) (127.0F * randf());
        text[i].dur = (unsigned short) (10.0F * randf() + 1.0F);
    }

    /* Sort by starting times */
    qsort(text, notes, sizeof(vector), compare_notes);

    /* Don't allow overlapping notes (same pitch and overlap in onset times)
       because the P3 algorithm implementation may have a problem
       with them. Polyphony is allowed. */
    for (i = 0; i < 128; i++) {
        playing_notes[i] = 0;
    }
    for (i = 0; i < notes; i++) {
        j = text[i].ptch;
        if (playing_notes[j] > text[i].strt) {
            text[i].strt = playing_notes[j];
        }
        playing_notes[j] = text[i].strt + text[i].dur;
    }
    /* Need to sort again because some notes may have been moved.*/
    qsort(text, notes, sizeof(vector), compare_notes);

    j = 0;
    for (i = 0; i < notes; i++) {
        (*startpoints)[i].x = text[i].strt;
        (*startpoints)[i].y = text[i].ptch;

        (*endpoints)[i].x = text[i].strt + text[i].dur;
        (*endpoints)[i].y = text[i].ptch;
    }
    /* Sort endpoints */
    qsort((*endpoints), notes, sizeof(TurningPoint), compare_turningpoints);

    free(playing_notes);
    s->size = notes;
    s->duration = duration;
    s->notes = text;
    s->title = NULL;
}


/**
 * Generates a random pattern.
 *
 * @param p a song item where the generated pattern will be stored
 * @param pattern_notes number of notes to generate
 * @param duration duration of the score in time units
 * @param note_duration maximum length of a note in time units (for example 10).
 */
static void generate_random_pattern(song *p, int pattern_notes, int duration,
        int note_duration) {

    int i, j;
    vector *pattern = (vector *) malloc(pattern_notes * sizeof(vector));
    unsigned int pattern_start = (unsigned int) ((float) duration * randf());
    unsigned int pattern_duration = 0;

    j = pattern_start;
    for (i = 0; i < pattern_notes; i++) {
        pattern[i].strt = j + (unsigned int) ((float) note_duration * randf());
        pattern[i].ptch = (unsigned int) (127.0F * randf());
        pattern[i].dur = 1 + (unsigned int) ((float) note_duration * randf());
        pattern_duration += pattern[i].dur;
        j += pattern_duration;
    }
    p->size = pattern_notes;
    p->duration = duration;
    p->notes = pattern;
    p->title = NULL;
}


/**
 * Extracts a random pattern of notes from the given music data.
 *
 * @param p a song item where the generated pattern will be stored
 * @param s music data where the pattern is extracted
 * @param pattern_notes number of notes to generate
 * @param monophonic set to 1 to only generate monophonic patterns, 0 to
 *        allow polyphony
 * @param pattern_start a place to store the start position of the generated
 *        pattern within the song
 * @param transposition a place to store the transposition applied to the
 *        pattern
 */
static void make_pattern(song *p, song *s, int pattern_notes,
        int monophonic, unsigned int *pattern_start, int *transposition) {

    int i, j, start;
    int mintranspose, maxtranspose;
    vector *pattern = (vector *) malloc(pattern_notes * sizeof(vector));
    vector *text = s->notes;

    /* Copy a (monophonic) sequence of notes. This will be repeated until a
     * clean sequence is received. */
    j = s->size;
    while (j >= s->size) {
        start = (int) ((float) (s->size - 2 * pattern_notes) * randf());
        if (start < 0) start = 0;
        j = start;
        mintranspose = -127;
        maxtranspose = 127;

        for (i = 0; i < pattern_notes; i++) {
            pattern[i].strt = text[j].strt - text[start].strt;
            pattern[i].ptch = text[j].ptch;
            pattern[i].dur = text[j].dur;
            if (0 - (int) pattern[i].ptch > mintranspose)
                mintranspose = 0 - (int) pattern[i].ptch;
            if (127 - (int) pattern[i].ptch < maxtranspose)
                maxtranspose = 127 - (int) pattern[i].ptch;

            if (monophonic) {
                /* Skip to a note that does not overlap with this note */
                while ((j < s->size) && (text[j].strt < text[start].strt +
                        pattern[i].strt + pattern[i].dur)) {
                    ++j;
                }
            } else ++j;

            if (j >= s->size) {
                /* Failed to extract a complete pattern. Next try... */
                i = pattern_notes;
            }
        }
    }
    (* pattern_start) = text[start].strt;

    /* Transpose */
    (* transposition) = mintranspose + (int) ((float)
            (maxtranspose - mintranspose) * randf());
    for (i = 0; i < pattern_notes; i++) {
        pattern[i].ptch = pattern[i].ptch + (* transposition);
    }
    p->size = pattern_notes;
    p->notes = pattern;
    p->title = NULL;
}


/**
 * Copies and distorts a pattern.
 * The distortions are changes to the pitch of some notes.
 *
 * @param out the output pattern
 * @param in the input pattern
 * @param errors the portion of notes to distort
 */
static void distort_pattern(song *out, song *in, float errors) {
    int i;
    vector *newpattern = (vector *) malloc(in->size * sizeof(vector));

    /* Copy and distort the pattern */
    for (i = 0; i < in->size; i++) {
        vector *n = &in->notes[i];
        newpattern[i].strt = n->strt;
        newpattern[i].ptch = n->ptch;
        newpattern[i].dur = n->dur;
        if (randf() < errors) {
            if (n->ptch < 64)
                newpattern[i].ptch += (char) (60.0F * randf());
            else
                newpattern[i].ptch -= (char) (60.0F * randf());
        }
    }
    out->size = in->size;
    out->notes = newpattern;
    out->title = NULL;
}


/**
 * Prints search results.
 *
 * @param ms the set of matches to print
 * @param sc song collection where the search was run. This can also be NULL.
 */
static void print_results(matchset *ms, songcollection *sc) {
    if (ms->num_matches > 0) {
        int i;
        printf("Matches:\n\n");
        for (i=0; i<ms->num_matches; ++i) {
            match *m = &ms->matches[i];
            if ((sc != NULL) && (sc->songs[m->song].title != NULL)) {
                printf("  song %d, %s:\n    (strt: %d, end: %d, transp: %d, sim: %f)\n\n",
                    m->song+1, sc->songs[m->song].title, m->start, m->end, m->transposition,
                    m->similarity);
            } else {
                printf("  song %d:\n    (strt: %d, end: %d, transp: %d, sim: %f)\n\n",
                    m->song+1, m->start, m->end, m->transposition,
                    m->similarity);

            }
        }
        return;
    } else {
        printf("No matches.\n");
    }
}


/** 
 * A simple test program for the geometric algorithms P1, P2 and P3.
 *
 * This program generates a "musical" score randomly and scans it with
 * a pattern of:
 * (1) n random notes
 * (2) n-note transposed sequential part of the score
 * (3) former with some distortions
 *
 * You can test the algorithms with monophonic patterns (simultaneously playing
 * notes discarded) by uncommenting the corresponding line at the beginning of
 * this file.
 *
 * Also note that P3 will by design find more matches than P2, so that with some
 * parameters P2 finds the correct match almost always, but P3 only
 * sometimes. This can be seen especially with massively polyphonic data, try:
 *     geometrictest 2 1000 5000 3
 *     geometrictest 3 1000 5000 3
 * Or uncomment TESTACCURACY at the top of this file, recompile and run
 * testaccuracy.sh. (first column is 1 if P2 found the correct match, second if
 * P3 found the correct match, otherwise 0, repeated 100 times)
 *
 * P3 behaves worse than P2 here because P2 requires the differences in
 * note starting times to match exactly. This effectively reduces the number
 * of possible matches. P3 only finds the maximum common duration without
 * further restrictions.
 *
 * @param argc number of arguments
 * @param argv argument array
 *
 * @return 0
 */
int main(int argc, char **argv) {
    unsigned int pattern_size = 20;

    unsigned int alg = 0;
    unsigned int text_size = 1000;
    unsigned int length = text_size;

    unsigned int seed = 0;
    unsigned int pattern2_time = 0;
    int transposition = 0;

    song s, p1, p2, p3;
    TurningPoint *startpoints;
    TurningPoint *endpoints;
    matchset ms;
    int result;

    if (argc >= 5) {
        sscanf(argv[1], "%u", &alg);
        sscanf(argv[2], "%u", &length);

        sscanf(argv[3], "%u", &text_size);

        sscanf(argv[4], "%u", &pattern_size);

        if (argc >= 6) {
            sscanf(argv[5], "%u", &seed);
#ifdef TESTACCURACY
            seed += time(NULL);
#endif
        } else {
            seed = time(NULL);
        }
    } else {
        printf("\n");
        printf("A simple test program for the geometric algorithms P1, P2 and P3\n\n");
        printf("This program generates a set of random notes and scans it with a pattern of:\n");
        printf("(i) n random notes\n");
        printf("(ii) n-note transposed sequential part of the score\n");
        printf("(iii) former with some distortions\n\n");
        printf("Usage:   geometrictest <alg> <song duration> <song size> <pattern size> [seed]\n\n");
        printf("Where:   alg            = 1, 2 or 3 (for P1, P2 and P3 correspondingly)\n");
        printf("         score duration = length of randomly generated score in time units.\n");
        printf("                          Maximum value is 8388607.\n");
        printf("                          Note length is 1 to %d units\n", NOTE_LENGTH);
        printf("         score size     = number of notes in the generated score\n");
        printf("         pattern size   = number of notes in a pattern\n");
        printf("         (seed          = random number generator seed as an unsigned integer)\n\n");
        printf("Example: geometrictest 3 10000 1000 20\n\n");
        return 0;
    }

    if ((alg < 1) || (alg > 3)) {
        printf("\nInvalid algorithm. Allowed values are 1, 2 and 3.\n\n");
        return 0;
    }
    if ((length < 1) || (pattern_size < 2) || (text_size < 2 * pattern_size)) {
        printf("\nToo small pattern or score. The score must be at least\ntwo times as long as the pattern. Exiting.\n\n");
        return 0;
    }
    if (length > 8388607) {
        printf("\nThe score duration (%d) is too long. The maximum is 8388607 (2^23 - 1).\n\n", length);
        return 0;
    }

    /* Clear song items */
    memset(&s, 0, sizeof(song));
    memset(&p1, 0, sizeof(song));
    memset(&p2, 0, sizeof(song));
    memset(&p3, 0, sizeof(song));

#ifndef TESTACCURACY
    printf("\n\nGeometric P%d algorithm on %d random notes and pattern size of %d.\nGenerating patterns...\n\n", alg, text_size, pattern_size);
#endif
    
    init_match_set(&ms, 1, pattern_size, 1);

    srand(seed);

    /* Generate the music data and three patterns. */
    generate_score(&s, length, text_size, &startpoints, &endpoints);

    generate_random_pattern(&p1, pattern_size, length, NOTE_LENGTH);

#ifdef MONOPHONIC_PATTERN
    printf("\nExtracting a monophonic pattern from the generated score.\n");
    printf("This may be impossible with some parameters, in which case the program\n");
    printf("will run in an endless loop. Terminate with CTRL-C.\n");
    make_pattern(&p2, &s, pattern_size, 1, &pattern2_time, &transposition);
#else
    make_pattern(&p2, &s, pattern_size, 0, &pattern2_time, &transposition);
#endif
    distort_pattern(&p3, &p2, DISTORTION_RATE);



#if 0
    /* Print the score and the patterns. */

    printf("\nText:\n");
    for (i = pattern2_time; i < pattern2_time + pattern_size; i++) {
        printf("Note %d (start:%d, dur:%d, pitch:%d)\n",
                i, s->notes[i].strt, s->notes[i].dur, s->notes[i].ptch);
        printf("Note (start:%d, pitch:%d)\n",
                startpoints[i].x, startpoints[i].y);
        printf("Note (end:%d, pitch:%d)\n", endpoints[i].x,
                endpoints[i].y);
    }
    printf("\nPattern 2:\n");
    for (i = 0; i < pattern_size; i++) {
        printf("Note (start:%d, dur:%d, pitch:%d)\n", p2->notes[i].strt,
                p2->notes[i].dur, p2->notes[i].ptch);
    }
    printf("\nPattern 3:\n");
    for (i = 0; i < pattern_size; i++) {
        printf("Note (start:%d, dur:%d, pitch:%d)\n", p3->notes[i].strt,
                p3->notes[i].dur, p3->notes[i].ptch);
    }
#endif


    /*********************************************************************
     **  Run the algorithms
     *********************************************************************/

#ifdef TESTACCURACY

    result = c_geometric_p2_scan(s, p2, pattern_size >> 1, &ms);
    if (ms.matches[0]->transposition != transposition) printf("0");
    else printf("1");
    clear_match_set(&ms);
    result = c_geometric_p3_scan(pattern_size, p2->notes,
            text_size, startpoints, endpoints, &ms, 0);
    if (ms.matches[0]->transposition != transposition) printf(", 0\n");
    else printf(", 1\n");

#else

    printf("\n(i) Scanning a random pattern (this is not supposed to match well)\n");
    if (alg == 1) {
        result = c_geometric_p1_scan(&s, &p1, &ms);
    } else if (alg == 2) {
        result = c_geometric_p2_scan(&s, &p1, pattern_size, &ms);
    } else {
        p3song
        result = c_geometric_p3_scan_turningpoints(pattern_size, p1.notes,
                text_size, startpoints, endpoints, &ms, 0);
    }
    print_results(&ms, NULL);
    clear_match_set(&ms);


    printf("\n(ii) Scanning pattern copied from\n      (pos: %d, transposition: %d)\n", pattern2_time, transposition);
    if (alg == 1) {
        result = c_geometric_p1_scan(&s, &p2, &ms);
    } else if (alg == 2) {
        result = c_geometric_p2_scan(&s, &p2, pattern_size, &ms);
    } else {
        result = c_geometric_p3_scan_turningpoints(pattern_size, p2.notes,
                text_size, startpoints, endpoints, &ms, 0);
    }
    print_results(&ms, NULL);
    clear_match_set(&ms);


    printf("\n(iii) Scanning distorted pattern with about %d percent of the notes changed.\n", (int) (DISTORTION_RATE * 100.0));
    if (alg == 1) {
        result = c_geometric_p1_scan(&s, &p3, &ms);
    } else if (alg == 2) {
        result = c_geometric_p2_scan(&s, &p3, pattern_size, &ms);
    } else {
        result = c_geometric_p3_scan_turningpoints(pattern_size, p3.notes,
                text_size, startpoints, endpoints, &ms, 0);
    }
    print_results(&ms, NULL);


    printf("\n");

#endif

    free_match_set(&ms);
    free_song(&s);
    free_song(&p1);
    free_song(&p2);
    free_song(&p3);
    free(startpoints);
    free(endpoints);

    return 0;
}

