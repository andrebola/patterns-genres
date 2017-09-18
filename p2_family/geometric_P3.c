/*
 * geometric_P3.c - Geometric algorithm P3
 *
 * Version 2007-05-28
 *
 *
 * Copyright (C) 2007 Niko Mikkila and Mika Turkia
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


#include <stdlib.h>
#include <limits.h>

#include "search.h"
#include "song.h"
#include "geometric_P3.h"
#include "priority_queue.h"
#include "util.h"



/**
 * Allocates and initializes a P3 song.
 *
 * @return an empty P3 song
 */
void init_p3_song(p3song *p3s) {
    p3s->song = NULL;
    p3s->size = 0;
    p3s->startpoints = NULL;
    p3s->endpoints = NULL;
}


/**
 * Frees a P3 song.
 *
 * @param p3s pointer to the song to be freed
 */
void free_p3_song(p3song *p3s) {
    free(p3s->startpoints);
    free(p3s->endpoints);
    p3s->startpoints = NULL;
    p3s->endpoints = NULL;
    p3s->size = 0;
    p3s->song = NULL;
}

/**
 * Converts a song to the P3 format.
 *
 * @param s the song to convert
 * @param p3s pointer to target data structure
 */
void song_to_p3(const song *s, p3song *p3s) {
    int i;
    vector *notes = s->notes;
    TurningPoint *startpoints, *endpoints;

    p3s->song = s;
    p3s->size = s->size;
    p3s->startpoints = (TurningPoint *) malloc(s->size * sizeof(TurningPoint));
    p3s->endpoints = (TurningPoint *) malloc(s->size * sizeof(TurningPoint));
    startpoints = p3s->startpoints;
    endpoints = p3s->endpoints;

    /* Copy notes */
    for (i=0; i<s->size; ++i) {
        vector *n = &notes[i];
        if (n->strt >= P3_TIME_LIMIT) {
            p3s->size = i;
            break;
        }

        startpoints[i].x = n->strt;
        startpoints[i].y = n->ptch;

        endpoints[i].x = n->strt + n->dur;
        if (endpoints[i].x >= P3_TIME_LIMIT) endpoints[i].x = P3_TIME_LIMIT - 1;
        endpoints[i].y = n->ptch;
    }

    /* Sort the endpoints */
    qsort(endpoints, i, sizeof(TurningPoint), compare_turningpoints);
}


/**
 * Initializes a P3 song collection struct.
 *
 * @return pointer to the data structure.
 */
void *init_p3_song_collection(void) {
    p3songcollection *p3sc = (p3songcollection *) calloc(1,
            sizeof(p3songcollection));
    return p3sc;
}


/**
 * Converts a song collection to the P3 format.
 *
 * @param sc song collection to convert
 * @param p3sc target P3 song collection
 */
int build_p3_song_collection(void *p3sc, const songcollection *sc,
        const dataparameters *dp) {
    int i;
    p3songcollection *_p3sc = (p3songcollection *) p3sc;

    _p3sc->song_collection = sc;
    _p3sc->size = sc->size;
    _p3sc->p3_songs = calloc(sc->size, sizeof(p3song));
    for (i=0; i<sc->size; ++i) {
        song_to_p3(&sc->songs[i], &_p3sc->p3_songs[i]);
    }
    return 1;
}


/**
 * Clears and re-initializes the given P3 song collection.
 *
 * @param p3sc P3 song data
 */
void clear_p3_song_collection(void *p3sc) {
    int i;
    p3songcollection *_p3sc = (p3songcollection *) p3sc;

    for (i=0; i<_p3sc->size; ++i) {
        free_p3_song(&_p3sc->p3_songs[i]);
    }
    free(_p3sc->p3_songs);
    _p3sc->p3_songs = NULL;
    _p3sc->song_collection = NULL;
    _p3sc->size = 0;
}


/**
 * Frees the given P3 song collection.
 *
 * @param p3sc P3 song data
 */
void free_p3_song_collection(void *p3sc) {
    clear_p3_song_collection(p3sc);
    free(p3sc);
}


/**
 * Search a song collection with c_geometric_p3_scan().
 *
 * @param sc a song collection to scan
 * @param pattern pattern to search for
 * @param alg search algorithm to use. See algorithms.h for algorithm IDs.
 * @param parameters search parameters
 * @param ms match set for returning search results
 */
void alg_p3(const songcollection *sc, const song *pattern, int alg,
        const searchparameters *parameters, matchset *ms) {
    int i;
    p3songcollection *p3sc = (p3songcollection *) sc->data[DATA_P3];
    if (p3sc == NULL) {
        fputs("Error in alg_p3: song collection does not contain P3 data.\nUse update_song_collection_data() to generate it.\n", stderr);
        return;
    }

    for (i=0; i<p3sc->size; ++i) {
        scan_p3(&p3sc->p3_songs[i], pattern, parameters, ms);
    }
}

/** 
 * Scanning phase of geometric algorithm P3. This algorithm is described in
 * Esko Ukkonen, Kjell Lemstrom and Veli Makinen: Sweepline the Music! In
 * Computer Science in Perspective (LNCS 2598), R. Klein, H.-W. Six, L. Wegner
 * (Eds.), pp. 330-342, 2003.
 *
 * The algorithm described in this article had a flaw that allowed the total
 * duration of a match to exceed the total duration of the pattern. This
 * problem has been solved by merging overlapping segments in the note data
 * during the calculation of turning points. However overlapping notes in the
 * pattern are currently not checked for. The problem can also be solved using
 * counters.
 *
 * Rough description of the algorithm: First a priority queue is created, which
 * will give translations in sorted order. Translation vectors for first source
 * note and all pattern notes are inserted to the queue. The priority queue is
 * then processed with the help of an array of vertical translations, which
 * stores value, slope and previous x translation for each vertical translation
 * y. For each vector extracted from the priority queue, we check the type of
 * the turning point associated with this vector and adjust the slope and value
 * accordingly. Then we move on in the source and add to the queue a new
 * translation vector for each new source note and the pattern note associated
 * with the previous translation vector. This loop is repeated until the end of
 * source. After the processing we have acquired the longest common time for
 * this pattern and source. This method returns only the best match for each
 * song. Consult the article for details.
 *
 * @param p3s song to scan
 * @param pattern pattern song
 * @param searchparameters search parameters
 * @param ms pointer to a structure where the results will be stored
 *
 * @return 1 when successful, 0 otherwise
 */
int scan_p3(const p3song *p3s, const song *pattern,
        const searchparameters *parameters, matchset *ms) {

    int i, j, num_loops;
    VerticalTranslationTableItem *verticaltranslationtable;
    int best = 0;
    int beststart = 0;
    int bestend = 0;
    int besty = 0;
    float bestsimilarity = 0.0F;
    VerticalTranslationTableItem *item = NULL;
    float pattern_duration = 0.0F;
    float difference = 0.0F;
    TurningPoint *startpoints = p3s->startpoints;
    TurningPoint *endpoints = p3s->endpoints;
    int num_tpoints = p3s->size;
    int pattern_size = pattern->size;
    vector *pnotes = pattern->notes;
    const song *s = p3s->song;

    /* Create a priority queue */
    pqroot *pq;
    pqnode *min;
    TranslationVector *translation_vectors;
    
    if ((pattern_size == 0) || (num_tpoints == 0)) return 0;

    pq = pq_create(pattern_size * 4);
    translation_vectors = (TranslationVector *)
            malloc(pattern_size * 4 * sizeof(TranslationVector));

    /* Initialize a vertical translation array */
    verticaltranslationtable = (VerticalTranslationTableItem *) malloc(
            NOTE_PITCHES * 2 * sizeof(VerticalTranslationTableItem));
    for (i = 0; i < (NOTE_PITCHES * 2); i++) {
        verticaltranslationtable[i].value = 0;
        verticaltranslationtable[i].slope = 0;
        verticaltranslationtable[i].prev_x = 0;
    }

    /* Create an array whose items have two pointers each: one for startpoints
     * and one for endpoints. Each item points to turning point array item Also
     * populate the priority queue with initial items. */
    for (i = 0, j = 0; i < pattern_size; i++) {
        TranslationVector *v;
        pattern_duration += (float) pnotes[i].dur;


        /* Add translation vectors calculated from the note start points */

        min = pq_getnode(pq, j);
        v = &translation_vectors[j];
        v->tpindex = 0;
        v->patternindex = i;
        v->y = (int) startpoints[0].y - (int) pnotes[i].ptch;
        v->x = (int) startpoints[0].x -
                (int) (pnotes[i].strt + pnotes[i].dur);
        v->pattern_is_start = 0;
        v->text_is_start = 1;
        min->key1 = (v->x << 8) + v->y + NOTE_PITCHES;
        min->pointer = v;
        pq_update_key1_p3(pq, min);
        ++j;

        min = pq_getnode(pq, j);
        v = &translation_vectors[j];
        v->tpindex = 0;
        v->patternindex = i;
        v->y = (int) startpoints[0].y - (int) pnotes[i].ptch;
        v->x = (int) startpoints[0].x - (int) pnotes[i].strt;
        v->pattern_is_start = 1;
        v->text_is_start = 1;
        min->key1 = (v->x << 8) + v->y + NOTE_PITCHES;
        min->pointer = v;
        pq_update_key1_p3(pq, min);
        ++j;


        /* Add translation vectors calculated from the note end points */

        min = pq_getnode(pq, j);
        v = &translation_vectors[j];
        v->tpindex = 0;
        v->patternindex = i;
        v->y = (int) endpoints[0].y - (int) pnotes[i].ptch;
        v->x = (int) endpoints[0].x - (int) (pnotes[i].strt +
                pnotes[i].dur);
        v->pattern_is_start = 0;
        v->text_is_start = 0;
        min->key1 = (v->x << 8) + v->y + NOTE_PITCHES;
        min->pointer = v;
        pq_update_key1_p3(pq, min);
        ++j;

        min = pq_getnode(pq, j);
        v = &translation_vectors[j];
        v->tpindex = 0;
        v->patternindex = i;
        v->y = (int) endpoints[0].y - (int) pnotes[i].ptch;
        v->x = (int) endpoints[0].x - (int) pnotes[i].strt;
        v->pattern_is_start = 1;
        v->text_is_start = 0;
        min->key1 = (v->x << 8) + v->y + NOTE_PITCHES;
        min->pointer = v;
        pq_update_key1_p3(pq, min);
        ++j;
    }

    best = 0;
    num_loops = (pattern_size * num_tpoints) << 2;

    for (i = 0; i < num_loops; i++) {
        int x, y;
        /* Get the minimum element */
        TranslationVector *v;
        min = pq_getmin(pq);
        v = (TranslationVector *) min->pointer;
        x = v->x;
        y = v->y;

        /* Update value */
        item = &verticaltranslationtable[NOTE_PITCHES + y];
        item->value += item->slope * (x - item->prev_x);
        item->prev_x = x;

        /* Adjust slope */
        if (v->text_is_start != v->pattern_is_start) {
            item->slope++;
        } else {
            item->slope--;
        }

        /* Check for a match */
        if (item->value >= best) {
            beststart = x + pnotes[0].strt;
            bestend = x + pnotes[pattern_size - 1].strt +
                    pnotes[pattern_size - 1].dur;
            bestsimilarity = ((float) item->value) / pattern_duration;
            besty = y;
            best = item->value;
            /*printf("%d\n",  item->value);*/
        }

        /* Move the pointer and insert a new translation vector according to
         * the turning point type. */
        if (v->tpindex < num_tpoints - 1) {
            const vector *patp = &pnotes[v->patternindex];
            v->tpindex++;

            if (v->text_is_start) {
                const TurningPoint *startp = &startpoints[v->tpindex];
                v->x = (int) startp->x - (int) patp->strt;
                v->y = (int) startp->y - (int) patp->ptch;
            } else {
                const TurningPoint *endp = &endpoints[v->tpindex];
                v->x = (int) endp->x - (int) patp->strt;
                v->y = (int) endp->y - (int) patp->ptch;
            }
            if (!v->pattern_is_start) {
                v->x -= (int) patp->dur;
            }
            min->key1 = (v->x << 8) + v->y + NOTE_PITCHES;
            pq_update_key1_p3(pq, min);
        } else {
            /* 'Remove' a translation vector by making it very large.
             * It won't be extracted since there will be only as many loops as 
             * there are real vectors. */
            min->key1 = INT_MAX;
            pq_update_key1_p3(pq, min);
        }
    }

    if (s != NULL) {
        if (parameters->p3_calculate_difference) {
            for (i=0; i<s->size; ++i) {
                int start = s->notes[i].strt;
                int end = start + s->notes[i].dur;
                if (start > bestend) break;
                if (end >= beststart) {
                    difference += MIN2(bestend, end) -
                        MAX2(beststart, start);
                }
            }
            difference = (float) best / difference;
        }
        insert_match(ms, s->id, beststart, bestend, besty,
                bestsimilarity * difference);
    } else {
        fputs("Warning in scan_p3: original song data is not available. Skipping difference calculations.\n", stderr);
        insert_match(ms, -1, beststart, bestend, besty,
                bestsimilarity * difference);
    }

    /* Free the reserved memory. */
    pq_free(pq);
    free(translation_vectors);
    free(verticaltranslationtable);
    return 1;
}


/**
 * Compares two turning points lexicographically. This is used to preprocess
 * data for the P3 algorithm. See the article for algorithm's input
 * requirements.
 * 
 * @param aa a turning point
 * @param bb another turning point
 * @return -1 if aa comes before bb, 0 if they are the same and 1 if aa comes
 *         after bb
 */
int compare_turningpoints(const void *aa, const void *bb) {
    /* sort by x coordinate of the translation vector */
    const TurningPoint *a, *b;
    a = (const TurningPoint *) aa;
    b = (const TurningPoint *) bb;
    if (a->x < b->x)
        return -1;
    else if (a->x > b->x)
        return 1;
    else {
        if (a->y < b->y)
            return -1;
        else if (a->y > b->y)
            return 1;
        else
            return 0;
    }
}


