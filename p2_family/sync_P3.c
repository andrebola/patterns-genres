/*
 * sync_P3.c - A modified version of the P3 implementation for symbolic music
 *             synchronization
 *
 * Version 2008-02-18
 *
 *
 * Copyright (C) 2008 Niko Mikkila and Mika Turkia
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
#include <string.h>
#include <math.h>

#include "search.h"
#include "song.h"
#include "geometric_P3.h"
#include "sync_P3.h"
#include "priority_queue.h"
#include "util.h"

/* #define CORRELATION_CUTOFF 1 */

#define ALLOW_SKIPS 1


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
 * @param pattern_size number of notes in the pattern
 * @param pattern pattern as a note vector array
 * @param num_tpoints number of turning points (notes in the text)
 * @param startpoints array of note starting points in lexicographical order
 * @param endpoints array of note ending points in lexicographical order
 * @param ms pointer to a structure where the results will be stored
 *
 * @return 1 when successful, 0 otherwise
 */
static void sync_turningpoints_p3(int pattern_size, const vector *pattern,
        int num_tpoints, const TurningPoint *startpoints,
        const TurningPoint *endpoints, int corr_size, int *corr) {

    int i, j, num_loops;
    VerticalTranslationTableItem *verticaltranslationtable;
    int best = 0;
    VerticalTranslationTableItem *item = NULL;
    float pattern_duration = 0.0;

    /* Create a priority queue */
    pqroot *pq;
    pqnode *min;
    TranslationVector *translation_vectors;
    
    if ((pattern_size <= 0) || (num_tpoints <= 0)) return;

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
        pattern_duration += (float) pattern[i].dur;


        /* Add translation vectors calculated from the note start points */

        min = pq_getnode(pq, j);
        v = &translation_vectors[j];
        v->tpindex = 0;
        v->patternindex = i;
        v->y = (int) startpoints[0].y - (int) pattern[i].ptch;
        v->x = (int) startpoints[0].x -
                (int) (pattern[i].strt + pattern[i].dur);
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
        v->y = (int) startpoints[0].y - (int) pattern[i].ptch;
        v->x = (int) startpoints[0].x - (int) pattern[i].strt;
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
        v->y = (int) endpoints[0].y - (int) pattern[i].ptch;
        v->x = (int) endpoints[0].x - (int) (pattern[i].strt +
                pattern[i].dur);
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
        v->y = (int) endpoints[0].y - (int) pattern[i].ptch;
        v->x = (int) endpoints[0].x - (int) pattern[i].strt;
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
        int start;
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
        if (v->text_is_start != v->pattern_is_start)
            item->slope++;
        else
            item->slope--;

        /* Check for a match */
        start = x + pattern[0].strt;
        if ((start >= 0) && (start < corr_size))
            corr[start] = MAX2(corr[start], item->value);

        /* Move the pointer and insert a new translation vector according to
         * the turning point type. */
        if (v->tpindex < num_tpoints - 1) {
            const vector *patp = &pattern[v->patternindex];
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

    /* Free the reserved memory. */
    pq_free(pq);
    free(translation_vectors);
    free(verticaltranslationtable);
}


/**
 * Preprocesses a query for the P3 algorithm. The searched pattern is not
 * modified, but the target text is converted to two separate sets of turning
 * points: one for the note start points and one for the end points, both
 * lexicographically ordered.
 *
 * @param s the song to scan
 * @param p pattern to search for
 * @param ms pointer to a structure where the results will be stored
 *
 * @return 1 when successful, 0 otherwise
 *
 * @see c_geometric_p3_scan_turningpoints()
 */
static void sync_song_p3(const song *s, const song *p,
        const searchparameters *parameters, matchset *ms) {

    int i;
    int strt;
    TurningPoint *startpoints, *endpoints;
    vector *text = s->notes;
    vector *pattern = p->notes;
    unsigned char *syncmap;
    int *linemap;
    int syncmap_w, syncmap_h;
    int num_windows;
    int *unscaled_corr, *corr, *cpath;
#ifdef CORRELATION_CUTOFF
    int *mediancorr;
#endif
    float *slope, *newslope;
    int unscaled_corr_size, corr_size;
    char output_file[256];
    int lastrow = -1;
    int lastwindowpos = INT_MAX;
    pqroot *maxlinepq = pq_create(1);
    int *ppos;
    int map_accuracy = parameters->syncmap_accuracy / parameters->sync_accuracy;
    int match_start = 0;
    int match_end = 0;
    float similarity = 0.0F;
    int wcount = 0;
#ifdef ALLOW_SKIPS
    int lastbestcorr = 0;
    int lastbestpos = 0;
    float lastbestslope = 1.0F;
#endif

    if ((p->size == 0) || (s->size == 0)) return;

    startpoints = (TurningPoint *) malloc(s->size * sizeof(TurningPoint));
    endpoints = (TurningPoint *) malloc(s->size * sizeof(TurningPoint));

    for (i = 0; i < s->size; i++) {
        startpoints[i].x = (int)((float)text[i].strt*1.0F);
        startpoints[i].y = text[i].ptch;

        endpoints[i].x = (int)((float)text[i].strt*1.0F) + text[i].dur;
        endpoints[i].y = text[i].ptch;
    }
    /* Sort the endpoints */
    qsort(endpoints, s->size, sizeof(TurningPoint), compare_turningpoints);

    for (i=0; i<1; ++i) {
        pqnode *node = pq_getnode(maxlinepq, i);
        node->key1 = INT_MIN;
        pq_update_key1_p3(maxlinepq, node);
    }

    /* Initialize a correlation array */
    unscaled_corr_size = startpoints[s->size-1].x;
    unscaled_corr = (int *) malloc(unscaled_corr_size * sizeof(int));
    corr_size = startpoints[s->size-1].x / parameters->sync_accuracy;
    corr = (int *) malloc(corr_size * sizeof(int));
#ifdef CORRELATION_CUTOFF
    mediancorr = (int *) malloc(corr_size * sizeof(int));
#endif
    cpath = (int *) calloc(corr_size, sizeof(int));
    slope = (float *) malloc(corr_size * sizeof(float));
    newslope = (float *) malloc(corr_size * sizeof(float));
    for (i=0; i<corr_size; ++i) {
        slope[i] = 1.0F;
        newslope[i] = 1.0F;
    }

    syncmap_w = MAX2(1, unscaled_corr_size / parameters->syncmap_accuracy);
    syncmap_h = MAX2(1, pattern[p->size-1].strt / parameters->syncmap_accuracy);
    syncmap = (unsigned char *) malloc(syncmap_w * syncmap_h *
            sizeof(unsigned char));
    memset(syncmap, 255, syncmap_w * syncmap_h * sizeof(unsigned char));

    strt = -1;
    num_windows = 0;
    ppos = (int *) malloc(p->size * sizeof(int));
    for (i=0; i<p->size; ++i) {
        if (pattern[i].strt > strt) {
            strt = pattern[i].strt;
            ++wcount;
            if (wcount >= 5) {
                ppos[num_windows] = i;
                num_windows++;
                wcount = 0;
            }
        }
    }
    /*num_windows = 4 * p->size / parameters->sync_window_size - 1;
    if (num_windows < 1) num_windows = 1;*/

    linemap = (int *) calloc(num_windows * corr_size, sizeof(int));
    if (linemap == NULL) {
        fprintf(stderr, "Error in sync_song_p3: failed to allocate memory for a synchronization line map\n");
        goto end;
    }

    for (i=0; i<num_windows; ++i) {
        int j;
#ifdef CORRELATION_CUTOFF
        int average_corr = 0;
#endif
        int pos = ppos[i];
        float pattern_duration = 0.0F;
        int row = MAX2(0, syncmap_h - 1 - (pattern[pos].strt /
                parameters->syncmap_accuracy));
        unsigned char *syncmap_row = &syncmap[syncmap_w * row];
        int *linemap_row = &linemap[(i-1) * corr_size];
        int window_notes = MIN2(p->size - pos, parameters->sync_window_size);
        int windowpos = pattern[pos].strt;
        /*fprintf(stderr, "Pos: %d : %d : %d\n", pos, window_notes, (int) syncmap_row);*/
        fprintf(stderr, "Pos: %d/%d : %d : %d\n", i, num_windows, pos, syncmap_w*row);
        if (window_notes <= 1) break;
        memset(unscaled_corr, 0, unscaled_corr_size * sizeof(int));
        memset(corr, 0, corr_size * sizeof(int));
        sync_turningpoints_p3(window_notes,
                &pattern[pos], s->size, startpoints, endpoints,
                unscaled_corr_size, unscaled_corr);
        for (j=pos; j<(pos+window_notes); ++j)
            pattern_duration += (float) pattern[j].dur;
        for (j=0; j<unscaled_corr_size; ++j) {
            int cpos = MIN2(j / parameters->sync_accuracy, corr_size-1);
            corr[cpos] = MAX2(corr[cpos], unscaled_corr[j]);
        }
/*        for (j=0; j<corr_size; ++j) {
            average_corr += corr[j];
        }
        average_corr = average_corr / corr_size;*/
#ifdef CORRELATION_CUTOFF
        memcpy(mediancorr, corr, corr_size * sizeof(int));
        average_corr = kth_smallest(mediancorr, corr_size, 3 * corr_size / 4);
        for (j=0; j<corr_size; ++j) {
            if (corr[j] < average_corr) {
                corr[j] = (int) (1000.0F * ((float) (corr[j] - average_corr)) / pattern_duration);
            } else {
                corr[j] = (int) (1000.0F * (float) corr[j] / pattern_duration);
            }

        }
#else 
        for (j=0; j<corr_size; ++j) {
            corr[j] = (int) (1000.0F * (float) corr[j] / pattern_duration);
        }
#endif
        for (j=0; j<corr_size; ++j) {
            /* TODO: optimize the integer division out */
            int syncmap_column = MIN2(syncmap_w - 1, j / map_accuracy);
            unsigned char color = 255 - (unsigned char)
                    (255.0 * MIN2((float)MAX2(corr[j],0) / 1000.0F, 1.0F));
            syncmap_row[syncmap_column] = MIN2(syncmap_row[syncmap_column],
                    color);
        }
        if ((lastrow > 0) && (lastrow > row + 1)) {
            for (j=lastrow-1; j>row; --j) {
                memcpy(&syncmap[syncmap_w * j], &syncmap[syncmap_w * lastrow],
                        syncmap_w * sizeof(unsigned char));
            }
        }
        if (lastwindowpos < windowpos) {
            float h = ((float) (windowpos - lastwindowpos)) /
                    (float) parameters->sync_accuracy;
            float w1 = h / 1.5F;
            float w2 = h * 1.5F;
            float wdelta = MAX2(1.0F, w2 - w1);
            int n = 0;
#ifdef ALLOW_SKIPS
            int bestcorr = 0;
            int bestpos = 0;
            float bestslope = 1.0F;
#endif
            pqroot *pq;
            pq = pq_create((int) wdelta);
            for (j=0; j<MIN2((int) w1, corr_size); ++j) {
                linemap_row[j] = INT_MAX;
                newslope[j] = 1.0F;
            }
            for (j=0; j<corr_size; ++j) {
                int jw = j + w1;
                pqnode *node = pq_getnode(pq, n);
                node->key1 = INT_MAX - MAX2(1, cpath[j]);
                node->key2 = j;
                pq_update_key1_p3(pq, node);
                ++n;
                if (n >= (int) wdelta) n = 0;
                if (jw < corr_size) {
                    float slp;
                    float slopemul;
                    node = pq_getmin(pq);
#ifdef ALLOW_SKIPS
                    if (cpath[node->key2] >= lastbestcorr) {
#endif
                        slp = ((float)(jw - node->key2)) / h;
                        slopemul = slp / slope[node->key2];
                        if (slopemul > 1.0F) slopemul = 1.0F / slopemul;
#ifdef CORRELATION_CUTOFF
                        if (corr[jw] < 0) {
                            corr[jw] = MAX2(0, (int) (((float) corr[jw]) +
                                ((float) cpath[node->key2])));
                        }
                        else
#endif
                        {
                            corr[jw] = (int) (((float) corr[jw])
                                * slopemul * slopemul
                                + ((float) cpath[node->key2]));
                        }
                        newslope[jw] = slope[node->key2] + 0.1 *
                                (slp - slope[node->key2]);
                        linemap_row[jw] = node->key2;
#ifdef ALLOW_SKIPS
                    } else {
                        corr[jw] += lastbestcorr;
                        newslope[jw] = lastbestslope;
                        linemap_row[jw] = lastbestpos;
                    }
                }
                {
                int balancedcorr = (int) ((1.0F -
                        ABS(((float)j / (float)corr_size) -
                        ((float)windowpos / (float)pattern[p->size-1].strt))) *
                        (float) corr[j]);
                if (balancedcorr > bestcorr) {
                    bestcorr = balancedcorr;
                    bestpos = j;
                    bestslope = newslope[j];
                }
                }
            }
            lastbestcorr = bestcorr;
            lastbestpos = bestpos;
            lastbestslope = bestslope;
#else
                }
            }
#endif
/*            for (j=w1; j<corr_size; ++j) {
                int k;
                float max = -1.0F;
                int maxpos = 0;
                int slopemul = 1.0F;
                for (k=MAX2(0, j-wdelta); k<=j; ++k) {
                    float val;
                    int slopediff = ((float) (j-k) / (float) h) / slope[k];
                    if (slopediff > 1.0F) slopediff = 1.0F / slopediff;
                    val = (float) cpath[k];
                    if (val > max) {
                        max = val;
                        maxpos = k;
                        slopemul = slopediff;
                    }
                }
                corr[j] = (int) ((float) corr[j] * slopemul) + max;
                slope[j] = slope[maxpos] + 0.25 * (((float) (j-k) / (float) h) - slope[maxpos]);
                linemap_row[j] = (unsigned short) (j-maxpos);
            }*/
            pq_free(pq);
            {
                for (j=0; j<corr_size; ++j) {
                    pqnode *node = pq_getmin(maxlinepq);
                    int c = corr[j];
                    /*int c = (int) (1000.0F * (float) corr[j] /
                            sqrtf((float) MAX2(j * parameters->sync_accuracy,
                            pattern[pos].strt)));*/
                    if (node->key1 < c) {
                        node->key1 = c;
                        node->key2 = j;
                        node->pointer = (void *) i;
                        pq_update_key1_p3(maxlinepq, node);
                    }
                }
            }
/*            if (i == num_windows - 1) {
                int r = i-1;
                w1 = corr_size;
                for (j=0; j<corr_size;) {
                    int end = MIN2(corr_size, j + 5000);
                    int max = j;
                    pqnode *node = pq_getmin(maxlinepq);
                    for (; j<end; ++j) {
                        if (corr[max] < corr[j]) max = j;
                    }
                    if (node->key1 < corr[max]) {
                        node->key1 = corr[max];
                        node->key2 = max;
                        node->pointer = (void *) r;
                        pq_update_key1_p3(maxlinepq, node);
                    }
                }
            } else {
                int r = i-2;
                int max = MAX2(0, corr_size - w1 - 2);
                pqnode *node = pq_getmin(maxlinepq);
                for (j=max+1; j<corr_size; ++j) {
                    if (cpath[j] > cpath[max]) max = j;
                }
                if (node->key1 < cpath[max]) {
                    node->key1 = cpath[max];
                    node->key2 = max;
                    node->pointer = (void *) r;
                    pq_update_key1_p3(maxlinepq, node);
                }
            }*/
            memcpy(cpath, corr, corr_size * sizeof(int));
            VOIDPTR_SWAP(slope, newslope);
        }
        lastrow = row;
        lastwindowpos = windowpos;
    }
    while(1) {
        pqnode *node = pq_getmin(maxlinepq);
        if (node->key1 == INT_MAX) break;
        if (node->key1 > 0) {
            int row = (int) node->pointer;
            int col = node->key2;
            while(row >= 0) {
                int pos = ppos[row];
                int map_row = MAX2(0, syncmap_h - 1 - (pattern[pos].strt /
                    parameters->syncmap_accuracy));
                int c;
                /*syncmap[syncmap_w * map_row + col/map_accuracy] = 0;*/
                if (row > 0) {
                    c = linemap[(row-1) * corr_size + col];
                    if (c == INT_MAX) break;
                    else col = c;
                }
                --row;
            }
            similarity = (float) ((double) node->key1 / (1000.0 * (double) ((int) node->pointer - MAX2(0, row))));
            match_start = col * parameters->sync_accuracy; 
            match_end = node->key2 * parameters->sync_accuracy;
        }
        node->key1 = INT_MAX;
        pq_update_key1_p3(maxlinepq, node);
    }
    snprintf(output_file, 256, "sync_%d_%d.pgm", p->id, s->id);
    write_pgm(output_file, syncmap, syncmap_w, syncmap_h);
    insert_match(ms, s->id, match_start, match_end, 0, similarity);

end:
    pq_free(maxlinepq);
    free(ppos);
    free(linemap);
    free(syncmap);
    free(newslope);
    free(slope);
#ifdef CORRELATION_CUTOFF
    free(mediancorr);
#endif
    free(cpath);
    free(corr);
    free(unscaled_corr);
    free(startpoints);
    free(endpoints);
}



/**
 * Scan a song collection with sync_song_p3().
 *
 * @param sc a song collection to scan
 * @param pattern pattern to search for
 * @param alg search algorithm to use. See algorithms.h for algorithm IDs.
 * @param parameters search parameters
 * @param ms match set for returning search results
 */
void alg_sync_p3(const songcollection *sc, const song *pattern, int alg,
        const searchparameters *parameters, matchset *ms) {
    int i;
    for (i=0; i<sc->size; ++i) {
        sync_song_p3(&sc->songs[i], pattern, parameters, ms);
    }
}

