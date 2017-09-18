/*
 * filter_P1.c - Vector index search filters based on the P1 algorithm.
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
#include <limits.h>

#include "config.h"
#include "filter_P1.h"
#include "geometric_P1.h"
#include "results.h"
#include "vindex.h"
#include "util.h"


#define MAX_TRIES 100

/**
 * Index filter P1(v1). Picks the first valid vector from the pattern and
 * retrieves positions where it occurs in the song collection.
 *
 * @param sc a song collection
 * @param pattern the input pattern to search for
 * @param alg algorithm ID as defined in search.h; not used in this function
 * @param parameters search parameters
 * @param ms information about the found matches will be stored here
 */
void filter_p1_random(const songcollection *sc, const song *pattern,
        int alg, const searchparameters *parameters, matchset *ms) {
    int tries = 0;
    int d = parameters->d_window;
    /*int lastsong = -1;*/
    int pattern_index = 0;
    int ir;
    int numrecords;
    indexvector *iv;
    indexrec *records;
    match *m;
    song *songs = sc->songs;
    vectorindex *vindex = sc->data[DATA_VINDEX];

#ifdef MEASURE_TIME_ALLOCATION
    struct timeval t1, t2;
#endif

    if (vindex == NULL) {
        fputs("Error in filter_p1_random(): song collection does not contain vectorindex data.\nUse update_song_collection_data() before calling this function.\n", stderr);
        return;
    }

    /* Pick a vector randomly. */
    while (1) {
        int p1 = (int) (randf() * (float) (pattern->size-d-1));
        int p2 = MIN2(pattern->size-1, p1 + 1 + (int) (randf() * (float) d));
        /*int p2 = p1 + 1;*/
        int x, y;
        ++tries;
        if (tries > MAX_TRIES) {
            fputs("Error in filter_p1_random(): failed to pick an index vector\n", stderr);
            return;
        }
        if (p2 - p1 > d) continue;
        x = pattern->notes[p2].strt - pattern->notes[p1].strt;
        y = pattern->notes[p2].ptch - pattern->notes[p1].ptch;

#ifdef MEASURE_TIME_ALLOCATION
        if (ms->time.measure) gettimeofday(&t1, NULL);
#endif
        iv = get_index_vector(vindex, x, y);
#ifdef MEASURE_TIME_ALLOCATION
        if (ms->time.measure) {
            gettimeofday(&t2, NULL);
            ms->time.indexing += timediff(&t2, &t1);
        }
#endif
        if (iv != NULL) {
            pattern_index = p1;
            break;
        }
    }

    records = iv->records;
    numrecords = iv->size;
    for (ir=0; ir<numrecords; ++ir) {
        indexrec *r = &records[ir];
        /*if (r->song == lastsong) continue;*/

#ifdef MEASURE_TIME_ALLOCATION
        if (ms->time.measure) {
            gettimeofday(&t1, NULL);
            ms->time.indexing += timediff(&t1, &t2);
        }
#endif
        m = alignment_check_p1(&songs[r->song],
                r->note, pattern, pattern_index, ms);
        if (m != NULL) {
            /* Match found, skip to the next song. */
            /*lastsong = r->song;*/
            /*m->song = r->song;*/
            if (ms->num_matches == ms->size) return;
        }
#ifdef MEASURE_TIME_ALLOCATION
        if (ms->time.measure) {
            gettimeofday(&t2, NULL);
            ms->time.verifying += timediff(&t2, &t1);
        }
#endif
    }
}


/**
 * Index filter P1(v2). Picks the least common valid vector from the
 * pattern to limit the number of locations to check.
 *
 * @param sc a song collection
 * @param pattern the input pattern to search for
 * @param alg algorithm ID as defined in search.h; not used in this function
 * @param parameters search parameters
 * @param ms information about the found matches will be stored here
 */
void filter_p1_select_1(const songcollection *sc, const song *pattern,
        int alg, const searchparameters *parameters, matchset *ms) {
    int i, j, ir;
    int patternpos = -1;
    int smallestcount = INT_MAX;
    indexvector *smallestiv = NULL;
    indexrec *records;
    song *songs = sc->songs;
    match *m;
    vectorindex *vindex = sc->data[DATA_VINDEX];

    if (vindex == NULL) {
        fputs("Error in filter_p1_select_1: song collection does not contain vectorindex data.\nUse update_song_collection_data() before calling this function.\n", stderr);
        return;
    }

    for (i=0; i<pattern->size-1; ++i) {
        for (j=i+1; j<pattern->size; ++j) {
            int x, y;
            indexvector *iv;
            if (j >= i + parameters->d_window) break;
            x = pattern->notes[j].strt - pattern->notes[i].strt;
            y = pattern->notes[j].ptch - pattern->notes[i].ptch;
            iv = get_index_vector(vindex, x, y);
            if ((iv != NULL) && (iv->size < smallestcount)) {
                smallestcount = iv->size;
                smallestiv = iv;
                patternpos = i;
            }
        }
    }
    if (smallestiv == NULL) {
        fputs("Error in filter_p1_select_1(): failed to pick an index vector\n", stderr);
        return;
    }

    records = smallestiv->records;

    j = -1;
    for (ir=0; ir<smallestcount; ++ir) {
        indexrec *r = &records[ir];
        /*if (r->song == j) continue;*/
        m = alignment_check_p1(&songs[r->song], r->note, pattern, patternpos,
                ms);
        if (m != NULL) {
            /* Match found, skip to the next song. */
            /*j = r->song;*/
            /*m->song = r->song;*/
            if (ms->num_matches == ms->size) return;
        }
    }   
}


/**
 * Index filter P1(v3). Picks the two least common valid vector from the pattern
 * and only checks locations where both vectors appear at the same relative
 * positions as in the pattern.
 *
 * @param sc a song collection
 * @param pattern the input pattern to search for
 * @param alg algorithm ID as defined in search.h; not used in this function
 * @param parameters search parameters
 * @param ms information about the found matches will be stored here
 */
void filter_p1_select_2(const songcollection *sc, const song *pattern,
        int alg, const searchparameters *parameters, matchset *ms) {
    int i, j, k, limit = INT_MAX;
    indexvector *iv1 = NULL;
    indexvector *iv2 = NULL;
    int iv1_patternpos = -1, iv2_patternpos = -1;
    int patternstrt, patternptch;
    /*int lastsong = -1;*/

    indexrec *records;
    vector *note;
    song *songs = sc->songs;
    match *m;
    vectorindex *vindex = sc->data[DATA_VINDEX];

#ifdef MEASURE_TIME_ALLOCATION
    struct timeval t1, t2;
    if (ms->time.measure) gettimeofday(&t1, NULL);
#endif

    if (vindex == NULL) {
        fputs("Error in filter_p1_select_1: song collection does not contain vectorindex data.\nUse update_song_collection_data() before calling this function.\n", stderr);
        return;
    }

    /* Pick the two least common vectors from the pattern. */
    for (i=0; i<pattern->size-1; ++i) {
        for (j=i+1; j<pattern->size; ++j) {
            int x, y;
            indexvector *iv;
            if (j >= i + parameters->d_window) break;
            x = pattern->notes[j].strt - pattern->notes[i].strt;
            y = pattern->notes[j].ptch - pattern->notes[i].ptch;
            iv = get_index_vector(vindex, x, y);
            if (iv != NULL) {
                if (iv1 == NULL) {
                    iv1 = iv;
                    iv1_patternpos = i;
                    limit = iv->size;
                } else if (iv2 == NULL) {
                    iv2 = iv;
                    iv2_patternpos = i;
                    if (iv->size < limit) limit = iv->size;
                } else if (iv->size < limit) {
                    if (iv2->size < iv1->size) {
                        iv1 = iv;
                        iv1_patternpos = i;
                    } else {
                        iv2 = iv;
                        iv2_patternpos = i;
                    }
                    limit = iv->size;
                }
            }
        }
    }
    if ((iv1 == NULL) || (iv2 == NULL)) {
        fputs("Error in filter_p1_select_2(): failed to pick index vectors\n", stderr);
        return;
    }

    /* Switch the vectors so that v1 is the less frequent one */
    if (iv1->size > iv2->size) {
        indexvector *tmp = iv1;
        iv1 = iv2;
        iv2 = tmp;

        i = iv2_patternpos;
        iv2_patternpos = iv1_patternpos;
        iv1_patternpos = i;
    }

    records = iv1->records;
    note = &pattern->notes[iv1_patternpos];
    patternstrt = note->strt;
    patternptch = note->ptch;
    k = 0;

#ifdef MEASURE_TIME_ALLOCATION
    if (ms->time.measure) {
        gettimeofday(&t2, NULL);
        ms->time.indexing += timediff(&t2, &t1);
    }
#endif

    /* Merge the location lists */
    for (i=0; i<iv1->size; ++i) {
        int songnumber = records[i].song;
        int songpos = records[i].note;
        song *s = &songs[songnumber];
        int songstrt, songptch;
        int failed = 0;
        int patterndelta;
        indexrec *orecords;

        note = &s->notes[songpos];
        songstrt = note->strt;
        songptch = note->ptch;

        /* Check other vectors in the ascending order of their occurrence
           counts. */
        note = &pattern->notes[iv2_patternpos];
        patterndelta = ((patternstrt - note->strt) << 8) +
                (patternptch - note->ptch);
        orecords = iv2->records;
        for (j=k; j<iv2->size; ++j) {
            int osongnumber = orecords[j].song;
            if (osongnumber < songnumber) k = j + 1;
            else if (osongnumber == songnumber) {
                int osongpos = orecords[j].note;
                song *os = &songs[osongnumber];
                int songdelta;

                note = &os->notes[osongpos];
                songdelta = ((songstrt - note->strt) << 8) +
                        (songptch - note->ptch);
                /* Break if found a pair. */
                if (patterndelta < songdelta) {
                    k = j + 1;
                } else if (patterndelta == songdelta) {
                    k = j + 1;
                    break;
                } else {
                    failed = 1;
                    break;
                }
            } else {
                failed = 1;
                break;
            }
        }

#ifdef MEASURE_TIME_ALLOCATION
        if (ms->time.measure) {
            gettimeofday(&t1, NULL);
            ms->time.other += timediff(&t1, &t2);
        }
#endif
        if (!failed) {
            indexrec *r = &records[i];
            /*if (r->song == lastsong) continue;*/
            m = alignment_check_p1(&songs[r->song], r->note, pattern,
                    iv1_patternpos, ms);
            if (m != NULL) {
                /* Match found, skip to the next song. */
                /*lastsong = r->song;*/
                /*m->song = r->song;*/
                if (ms->num_matches == ms->size) return;
            }
        }
#ifdef MEASURE_TIME_ALLOCATION
        if (ms->time.measure) {
            gettimeofday(&t2, NULL);
            ms->time.verifying += timediff(&t2, &t1);
        }
#endif
    }
}


/**
 * Index filter P1(v4). Selects the least common valid vector from a group of
 * random vectors picked from the pattern. This makes the search time complexity
 * linear in regard to pattern size.
 *
 * @param sc a song collection
 * @param pattern the input pattern to search for
 * @param alg algorithm ID as defined in search.h; not used in this function
 * @param parameters search parameters
 * @param ms information about the found matches will be stored here
 */
void filter_p1_sample(const songcollection *sc, const song *pattern,
        int alg, const searchparameters *parameters, matchset *ms) {
    int i, lastsong;
    indexrec *records;
    song *songs = sc->songs;
    match *m;
    indexvector _best_iv;
    indexvector *best_iv = &_best_iv;
    int best_pattern_pos = 0;
    vectorindex *vindex = sc->data[DATA_VINDEX];

    _best_iv.size = INT_MAX;

    if (vindex == NULL) {
        fputs("Error in filter_p1_sample: song collection does not contain vectorindex data.\nUse update_song_collection_data() before calling this function.\n", stderr);
        return;
    }

    if (pattern->size < 2) return;

    /* Pick vcount random vectors from the pattern */
    for (i=0; i<parameters->p1_sample_size; ++i) {
        int j, k, x, y;
        indexvector *iv;
        j = (int) (randd() * ((double) pattern->size - 2));
        if (j < 0) j = 0;
        k = j + (int) (randd() * ((double) parameters->d_window));
        if (k <= j) k = j + 1;
        else if (k >= pattern->size) k = pattern->size - 1;

        x = pattern->notes[k].strt - pattern->notes[j].strt;
        y = pattern->notes[k].ptch - pattern->notes[j].ptch;
        iv = get_index_vector(vindex, x, y);
        if ((iv != NULL) && (iv->size < best_iv->size)) {
            best_iv = iv;
            best_pattern_pos = j;
        }
    }

    /* If there were no valid vectors in the random group, try P1/F2 */
    if (best_iv->size == INT_MAX) {
        fputs("Warning in filter_p1_sample(): no valid vectors found. Trying again with P1/F2...", stderr);
        filter_p1_select_1(sc, pattern, alg, parameters, ms);
        return;
    }

    records = best_iv->records;

    lastsong = -1;
    for (i=0; i<best_iv->size; ++i) {
        indexrec *r = &records[i];
        /*if (r->song == lastsong) continue;*/
        m = alignment_check_p1(&songs[r->song], r->note, pattern,
                best_pattern_pos, ms);
        if (m != NULL) {
            /* Match found, skip to the next song. */
            /*lastsong = r->song;*/
            /*m->song = r->song;*/
            if (ms->num_matches == ms->size) return;
        }
    }   
}

