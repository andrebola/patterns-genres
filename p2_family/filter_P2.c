/*
 * filter_P2.c - Vector index search filters based on the P2 algorithm.
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
#include "priority_queue.h"

#include "config.h"
#include "filter_P2.h"
#include "geometric_P1.h"
#include "geometric_P2.h"
#include "util.h"
#include "results.h"
#include "vindex.h"

#ifdef ENABLE_BLOSSOM4
#include "match.h"
#endif


#define MAX_EDGE_WEIGHT 1000000

/**
 * P2/F6-greedy, index filter that is based on the pigeonhole principle and
 * use of P1 index filters. Error tolerance of this filter is controlled
 * by p2_select_threshold in the search parameters struct. 0.0 means no
 * expected corruption, 0.5 means that half of the notes may be wrong in the
 * potential matches. 0.5 is the highest error rate that this filter can handle
 * without loss in accuracy. Lower values make the algorithm run faster, and
 * it should still find most of the matches whose error rate is higher
 * than the specified tolerance level.
 *
 * @param sc a song collection
 * @param pattern the input pattern to search for
 * @param alg algorithm ID as defined in search.h; not used in this function
 * @param parameters search parameters
 * @param ms information about the found matches will be stored here
 */
void filter_p2_greedy_pigeonhole(const songcollection *sc, const song *pattern,
        int alg, const searchparameters *parameters, matchset *ms) {
    int i;
    int vcount = 0;
    int num_pattern_vectors;
    char *selected = (char *) calloc(pattern->size, sizeof(char));
    vector *pnotes = pattern->notes;
    pqroot *pq;
    patternvector *chosenvectors = (patternvector *) malloc(pattern->size *
            sizeof(patternvector));
    vectorindex *vindex = sc->data[DATA_VINDEX];
    song *songs = sc->songs;

    if ((vindex == NULL) || (selected == NULL) || (chosenvectors == NULL)) {
        free(selected);
        free(chosenvectors);
        if (vindex == NULL) {
            fputs("Error in filter_p2_greedy_pigeonhole(): song collection does not contain\nvectorindex data. Use update_song_collection_data() before calling this function\n", stderr);
        }
        return;
    }

    num_pattern_vectors = parameters->p2_select_threshold * pattern->size;
    if (num_pattern_vectors <= 0) num_pattern_vectors = 1;

    for (i=0; i<pattern->size-1; ++i) {
        int j;
        int end = i + parameters->d_window;
        chosenvectors[i].patternpos = i;
        chosenvectors[i].iv = NULL;
        if (selected[i]) continue;
        if (end >= pattern->size) end = pattern->size - 1;
        for (j=i+1; j<=end; ++j) {
            int x, y;
            indexvector *iv;
            if (selected[j]) continue;
            x = pnotes[j].strt - pnotes[i].strt;
            y = pnotes[j].ptch - pnotes[i].ptch;
            iv = get_index_vector(vindex, x, y);
            if (iv != NULL) {
                if ((chosenvectors[i].iv == NULL) ||
                        (iv->size < chosenvectors[i].iv->size)) {
                    chosenvectors[i].t = j;
                    chosenvectors[i].iv = iv;
                }
            }
        }
        if (chosenvectors[i].iv != NULL) {
            selected[chosenvectors[i].t] = 1;
            ++vcount;
        }
    }
    free(selected);

    if (vcount == 0) {
        free(chosenvectors);
        return;
    }

    /* Sort the vectors by their frequencies */
    pq = pq_create(vcount);
    vcount = 0;
    for (i=0; i<pattern->size-1; ++i) {
        if (chosenvectors[i].iv != NULL) {
            pqnode *node = pq_getnode(pq, vcount);
            node->key1 = chosenvectors[i].iv->size;
            node->key2 = i;
            pq_update_key1_p3(pq, node);
            ++vcount;
        }
    }

    vcount = 0;
    while (vcount < num_pattern_vectors) {
        int k;
        indexrec *records;

        pqnode *min = pq_getmin(pq);

        if (min->key1 == INT_MAX) break;
        i = min->key2;

        records = chosenvectors[i].iv->records;
        for (k=0; k<chosenvectors[i].iv->size; ++k) {
            song *s = &songs[records[k].song];
            alignment_check_p2(s, records[k].note, pattern, i, ms); 
        }

        min->key1 = INT_MAX;
        min->key2 = INT_MAX;
        pq_update_key1_p3(pq, min);
        ++vcount;
    }
    /*fprintf(stderr, "Vectors: %d\n", vcount);*/
    pq_free(pq);
    free(chosenvectors);
}


#ifdef ENABLE_BLOSSOM4
/**
 * P2/F6, index filter that is based on the pigeonhole principle and
 * use of P1 index filters. This version uses a graph matching algorithm to
 * retrieve an optimal set of vectors.
 *
 * Error tolerance of this filter is controlled by p2_select_threshold in the
 * search parameters struct. 0.0 means no expected corruption, 0.5 means that
 * half of the notes may be wrong in the potential matches. 0.5 is the highest
 * error rate that this filter can handle without loss in accuracy. Lower
 * values make the algorithm run faster, and it should still find most of the
 * matches whose error rate is higher than the specified tolerance level.
 *
 * @param sc a song collection
 * @param pattern the input pattern to search for
 * @param alg algorithm ID as defined in search.h; not used in this function
 * @param parameters search parameters
 * @param ms information about the found matches will be stored here
 */
void filter_p2_pigeonhole(const songcollection *sc, const song *pattern,
        int alg, const searchparameters *parameters, matchset *ms) {
    int i, vcount = 0;
    int num_pattern_vectors;
    int *edges, *edgelengths, *graph_match;
    vector *pnotes = pattern->notes;
    pqroot *pq;
    vectorindex *vindex = sc->data[DATA_VINDEX];
    song *songs = sc->songs;
#ifdef MEASURE_TIME_ALLOCATION
    struct timeval t1, t2;
#endif

    if (vindex == NULL) {
        fputs("Error in filter_p2_pigeonhole(): song collection does not contain vectorindex data.\nUse update_song_collection_data() before calling this function.\n", stderr);
        return;
    }

    edges = (int *) malloc((1 + 2 * pattern->size * parameters->d_window) *
            sizeof(int));
    edgelengths = (int *) malloc((1 + pattern->size * parameters->d_window) *
            sizeof(int));
    graph_match = (int *) calloc(pattern->size * 2, sizeof(int));

    if ((edges == NULL) || (edgelengths == NULL) || (graph_match == NULL)) {
        fputs("Error in filter_p2_pigeonhole: failed to allocate memory", stderr);
        goto EXIT;
    }

    num_pattern_vectors = parameters->p2_select_threshold * pattern->size;
    if (num_pattern_vectors <= 0) num_pattern_vectors = 1;

#ifdef MEASURE_TIME_ALLOCATION
    if (ms->time.measure) gettimeofday(&t1, NULL);
#endif

    for(i=0; i<pattern->size; ++i) {
        int j;
        /* Keep track of vertices so that those with no connecting
           edges can be located. */
        for (j=i+1; j<=i+parameters->d_window; ++j) {
            int x, y;
            indexvector *iv;
            if (j >= pattern->size) break;
            x = pnotes[j].strt - pnotes[i].strt;
            y = pnotes[j].ptch - pnotes[i].ptch;
            iv = get_index_vector(vindex, x, y);
            if (iv != NULL) {
                /* Mark these vertices connected */
                graph_match[i] = 1;
                graph_match[j] = 1;

                edgelengths[vcount >> 1] = iv->size;
                edges[vcount] = i;
                ++vcount;
                edges[vcount] = j;
                ++vcount;
                /*printf("edge: %d->%d, %d\n", i, j, iv->size);*/
            }
        }
        if (graph_match[i] == 0) {
            fputs("Warning: Unconnected vertex -- adding a dummy edge\n",
                    stderr);
/*
            edgelengths[vcount >> 1] = MAX_EDGE_WEIGHT;
            edges[vcount] = i;
            ++vcount;
            edges[vcount] = pattern->size;
            ++vcount;
*/
        } else graph_match[i] = 0;
    }

#ifdef MEASURE_TIME_ALLOCATION
    if (ms->time.measure) {
        gettimeofday(&t2, NULL);
        ms->time.indexing += timediff(&t2, &t1);
    }
#endif

    vcount >>= 1;
    if (blossom4_match(pattern->size, vcount, edges, edgelengths, 0, 0, 0, 0,
            graph_match)) {
#ifdef MEASURE_TIME_ALLOCATION
        if (ms->time.measure) {
            gettimeofday(&t1, NULL);
            ms->time.other += timediff(&t1, &t2);
        }
#endif
        fputs("Error in filter_p2_pigeonhole: graph matching failed.\nRunning a greedy search instead.\n", stderr);
        filter_p2_greedy_pigeonhole(sc, pattern, alg, parameters, ms);
#ifdef MEASURE_TIME_ALLOCATION
        if (ms->time.measure) {
            gettimeofday(&t2, NULL);
            ms->time.verifying += timediff(&t2, &t1);
        }
#endif
        goto EXIT;
    }
    free(edges);
    free(edgelengths);
    edges = NULL;
    edgelengths = NULL;

#ifdef MEASURE_TIME_ALLOCATION
    if (ms->time.measure) {
        gettimeofday(&t1, NULL);
        ms->time.other += timediff(&t1, &t2);
    }
#endif

    /* Sort the vectors by their frequencies */
    pq = pq_create((pattern->size >> 1) + 1);
    vcount = 0;
    for (i=0; i<pattern->size; i+=2) {
        int x, y;
        indexvector *iv;
        pqnode *node;

        if ((graph_match[i] >= pattern->size) ||
                (graph_match[i+1] >= pattern->size)) {
            /* Dummy edge */
            continue;
        }

        if (graph_match[i] == graph_match[i+1]) break;

        x = pnotes[graph_match[i+1]].strt - pnotes[graph_match[i]].strt;
        y = pnotes[graph_match[i+1]].ptch - pnotes[graph_match[i]].ptch;
        iv = get_index_vector(vindex, x, y);

        node = pq_getnode(pq, vcount);
        node->key1 = iv->size;
        node->key2 = graph_match[i];
        node->pointer = iv->records;
        pq_update_key1_p3(pq, node);
        ++vcount;
    }

#ifdef MEASURE_TIME_ALLOCATION
    if (ms->time.measure) {
        gettimeofday(&t2, NULL);
        ms->time.indexing += timediff(&t2, &t1);
    }
#endif

    if (vcount > num_pattern_vectors) vcount = num_pattern_vectors;
    while (vcount >= 0) {
        int k;
        indexrec *records;

        pqnode *min = pq_getmin(pq);

        if (min->key1 == INT_MAX) break;
        i = min->key2;

        records = (indexrec *) min->pointer;
        for (k=0; k<min->key1; ++k) {
            song *s = &songs[records[k].song];
            alignment_check_p2(s, records[k].note, pattern, i, ms); 
        }

        min->key1 = INT_MAX;
        min->key2 = INT_MAX;
        pq_update_key1_p3(pq, min);
        --vcount;
    }

#ifdef MEASURE_TIME_ALLOCATION
    if (ms->time.measure) {
        gettimeofday(&t1, NULL);
        ms->time.verifying += timediff(&t1, &t2);
    }
#endif

    /*fprintf(stderr, "Vectors: %d\n", vcount);*/
    pq_free(pq);
EXIT:
    free(edges);
    free(edgelengths);
    free(graph_match);
}

#endif


/**
 * Index filter P2/F4. Picks all valid vectors from the pattern and scans them
 * (or their starting points) in the same way that the actual P2 algorithm scans
 * notes.
 *
 * @param sc a song collection
 * @param pattern the input pattern to search for
 * @param alg algorithm ID as defined in search.h; not used in this function
 * @param parameters search parameters
 * @param ms information about the found matches will be stored here
 */
void filter_p2_window(const songcollection *sc, const song *pattern,
        int alg, const searchparameters *parameters, matchset *ms) {
    int i, j;
    int vcount = pattern->size * parameters->p2_window;

    int songid = INT_MIN;
    int previous_key = INT_MIN;
    int previous_spos = -1;
    int previous_ppos = -1;
    int count = 0;
    int maxcount = 0;
    /* int matchstart = 0; */
    vector *pnotes = pattern->notes;
    vectorindex *vindex = sc->data[DATA_VINDEX];
    pqroot *pq;
    song *songs = sc->songs;
    patternvector *chosenvectors = (patternvector *) malloc(vcount *
            sizeof(patternvector));
    vcount = 0;

    if (vindex == NULL) {
        fputs("Error in filter_p2_window(): song collection does not contain vectorindex data.\nUse update_song_collection_data() before calling this function.\n", stderr);
        free(chosenvectors);
        return;
    }

    /* Pick all the valid vectors from the pattern. */
    for (i=0; i<pattern->size - 1; ++i) {
        int end = i + parameters->p2_window;
        if (end >= pattern->size) end = pattern->size - 1;
        for (j=i+1; j<=end; ++j) {
            int x = pnotes[j].strt - pnotes[i].strt;
            int y = pnotes[j].ptch - pnotes[i].ptch;
            indexvector *iv = get_index_vector(vindex, x, y);
            if (iv != NULL) {
                chosenvectors[vcount].iv = iv;
                chosenvectors[vcount].patternpos = i;
                chosenvectors[vcount].shift = (((int) pnotes[i].strt -
                    (int) pnotes[0].strt) << 8) +
                    (int) pnotes[i].ptch -
                    (int) pnotes[0].ptch + NOTE_PITCHES;
                chosenvectors[vcount].t = 0;
                ++vcount;
            }
        }
    }
    if (vcount == 0) {
        free(chosenvectors);
        return;
    }

    /* Initialize a priority queue. */
    pq = pq_create(vcount);

    /* Merge the location lists by inserting them to the priority queue. */
    for (i=0; i<vcount; ++i) {
        pqnode *node = pq_getnode(pq, i);
        indexrec *ir = chosenvectors[i].iv->records;
        song *s = &songs[ir->song];
        vector *textnote = &s->notes[ir->note];
        vector *patternnote = &pnotes[chosenvectors[i].patternpos];
        node->key1 = ir->song;
        node->key2 = ((int) (textnote->strt - patternnote->strt) << 8) +
                (int) (textnote->ptch - patternnote->ptch) + NOTE_PITCHES;
        node->pointer = ir;
        pq_update(pq, node);
        chosenvectors[i].t = 1;
    }

    /* Scan through the data and mark the location where the largest number
     * of vectors are in the same relative positions as in the pattern. */
    --vcount;
    while (1) {
        pqnode *min = pq_getmin(pq);
        i = min->index;
        if ((min->key1 == songid) && (min->key2 == previous_key)) {
            ++count;
            if (count > maxcount) maxcount = count;
        } else {
            /* Check if the previous match was a good one */
            if ((count == maxcount) && (count > 1)) {
#ifdef ORDER_F4_F5_RESULTS_WITH_P2
                alignment_check_p2(&songs[songid], previous_spos,
                        pattern, previous_ppos, ms);
#else
                int match_start = (previous_key >> 8) + pnotes[0].strt;
                int match_end = (previous_key >> 8) +
                        pnotes[pattern->size - 1].strt +
                        pnotes[pattern->size - 1].dur;
                char transposition = (char) ((previous_key & 0xFF) -
                        NOTE_PITCHES);
                float similarity = ((float) maxcount) / ((float) vcount);
                insert_match(ms, songid, match_start, match_end, transposition,
                        similarity);
#endif
            }
            if (min->key1 == INT_MAX) break;
            count = 0;
            if (songid != min->key1) {
                /* Next song, reset counter */
                songid = min->key1;
                maxcount = 0;
            }
            previous_key = min->key2;
            previous_ppos = chosenvectors[i].patternpos;
            previous_spos = ((indexrec *) min->pointer)->note;
        }
        if (chosenvectors[i].t < chosenvectors[i].iv->size) {    
            indexrec *ir = &chosenvectors[i].iv->records[chosenvectors[i].t];
            song *s = &songs[ir->song];
            vector *textnote = &s->notes[ir->note];
            vector *patternnote = &pnotes[chosenvectors[i].patternpos];
            min->key1 = ir->song;
            min->key2 = ((int) (textnote->strt - patternnote->strt) << 8) +
                    (int) (textnote->ptch - patternnote->ptch) + NOTE_PITCHES;
            min->pointer = ir;
            pq_update(pq, min);
            ++chosenvectors[i].t;
        } else {
            min->key1 = INT_MAX;
            min->key2 = INT_MAX;
            pq_update(pq, min);
        }
    }
    free(chosenvectors);
    pq_free(pq);
}


/**
 * Index filter P2/F5. Picks locally a group of least frequent vectors in the
 * pattern to approximate P2. Seach parameter p2_select_threshold controls the
 * ratio of vectors to discard: 1.0 keeps all vectors, which is the same as
 * P2/V5, 0.0 discards all. Value 0.5 is generally a good choise. Longest
 * vectors are discarded and the shortest will be kept.
 *
 * @param sc a song collection
 * @param pattern the input pattern to search for
 * @param alg algorithm ID as defined in search.h; not used in this function
 * @param parameters search parameters
 * @param ms information about the found matches will be stored here
 */
void filter_p2_select_local(const songcollection *sc, const song *pattern,
        int alg, const searchparameters *parameters, matchset *ms) {
    int i, j;
    int vcount = pattern->size * parameters->p2_window;

    int songid = INT_MIN;
    int previous_key = INT_MIN;
    int previous_spos = -1;
    int previous_ppos = -1;
    int count = 0;
    int maxcount = 0;
    /* int matchstart = 0; */
    vector *pnotes = pattern->notes;
    vectorindex *vindex = sc->data[DATA_VINDEX];
    pqroot *pq;
    song *songs = sc->songs;
    patternvector *chosenvectors = (patternvector *) malloc(vcount *
            sizeof(patternvector));
    int *bucket_size = (int *) malloc(parameters->p2_window * sizeof(int));
    vcount = 0;

    if (vindex == NULL) {
        fputs("Error in filter_p2_select_local(): song collection does not contain\nvectorindex data. Use update_song_collection_data() before calling this function\n", stderr);
        free(chosenvectors);
        free(bucket_size);
        return;
    }

    /* Pick all the valid vectors from the pattern. */
    for (i=0; i<pattern->size - 1; ++i) {
        int c = 0;
        int end = i + parameters->p2_window;
        if (end >= pattern->size) end = pattern->size - 1;

        /* Calculate vector occurrence median */
        memset(bucket_size, 0, parameters->p2_window * sizeof(int));
        for (j=i+1; j<=end; ++j) {
            int x = pnotes[j].strt - pnotes[i].strt;
            int y = pnotes[j].ptch - pnotes[i].ptch;
            indexvector *iv = get_index_vector(vindex, x, y);
            if (iv != NULL) {
                bucket_size[c] = iv->size;
                ++c;
            }
        }
        if (c == 0) continue;
        maxcount = kth_smallest(bucket_size, c, MIN2(c-1,
                (int) (((float) c) * parameters->p2_select_threshold)));

        /* Select vectors */
        for (j=i+1; j<=end; ++j) {
            int x = pnotes[j].strt - pnotes[i].strt;
            int y = pnotes[j].ptch - pnotes[i].ptch;
            indexvector *iv = get_index_vector(vindex, x, y);
            if ((iv != NULL) && (iv->size <= maxcount)) {
                chosenvectors[vcount].iv = iv;
                chosenvectors[vcount].patternpos = i;
                chosenvectors[vcount].shift = (((int) pnotes[i].strt -
                    (int) pnotes[0].strt) << 8) +
                    (int) pnotes[i].ptch -
                    (int) pnotes[0].ptch + NOTE_PITCHES;
                chosenvectors[vcount].t = 0;
                ++vcount;
            }
        }
    }
    free(bucket_size);
    if (vcount == 0) {
        free(chosenvectors);
        return;
    }

    /* Initialize a priority queue. */
    pq = pq_create(vcount);

    /* Merge the location lists by inserting them to the priority queue. */
    for (i=0; i<vcount; ++i) {
        pqnode *node = pq_getnode(pq, i);
        indexrec *ir = chosenvectors[i].iv->records;
        song *s = &songs[ir->song];
        vector *textnote = &s->notes[ir->note];
        vector *patternnote = &pnotes[chosenvectors[i].patternpos];
        node->key1 = ir->song;
        node->key2 = ((int) (textnote->strt - patternnote->strt) << 8) +
                (int) (textnote->ptch - patternnote->ptch) + NOTE_PITCHES;
        node->pointer = ir;
        pq_update(pq, node);
        chosenvectors[i].t = 1;
    }

    /* Scan through the data and mark the location where the largest number
     * of vectors are in the same relative positions as in the pattern. */
    --vcount;
    maxcount = 0;
    while (1) {
        pqnode *min = pq_getmin(pq);
        i = min->index;
        if ((min->key1 == songid) && (min->key2 == previous_key)) {
            ++count;
            if (count > maxcount) maxcount = count;
        } else {
            /* Check if the previous match was a good one */
            if ((count == maxcount) && (count > 1)) {
#ifdef ORDER_F4_F5_RESULTS_WITH_P2
                alignment_check_p2(&songs[songid], previous_spos,
                        pattern, previous_ppos, ms);
#else
                int match_start = (previous_key >> 8) + pnotes[0].strt;
                int match_end = (previous_key >> 8) +
                        pnotes[pattern->size - 1].strt +
                        pnotes[pattern->size - 1].dur;
                char transposition = (char) ((previous_key & 0xFF) -
                        NOTE_PITCHES);
                float similarity = ((float) maxcount) / ((float) vcount);

                insert_match(ms, songid, match_start, match_end, transposition,
                        similarity);
#endif
    /*printf("%d, %d\n", songid + 1, maxcount);*/
            }
            if (min->key1 == INT_MAX) break;
            count = 0;
            if (songid != min->key1) {
                /* Next song, reset counter */
                songid = min->key1;
                maxcount = 0;
            }
            previous_key = min->key2;
            previous_ppos = chosenvectors[i].patternpos;
            previous_spos = ((indexrec *) min->pointer)->note;
        }
        if (chosenvectors[i].t < chosenvectors[i].iv->size) {    
            indexrec *ir = &chosenvectors[i].iv->records[chosenvectors[i].t];
            song *s = &songs[ir->song];
            vector *textnote = &s->notes[ir->note];
            vector *patternnote = &pnotes[chosenvectors[i].patternpos];
            min->key1 = ir->song;
            min->key2 = ((int) (textnote->strt - patternnote->strt) << 8) +
                    (int) (textnote->ptch - patternnote->ptch) + NOTE_PITCHES;
            min->pointer = ir;
            pq_update(pq, min);
            ++chosenvectors[i].t;
        } else {
            min->key1 = INT_MAX;
            min->key2 = INT_MAX;
            pq_update(pq, min);
        }
    }
    free(chosenvectors);
    pq_free(pq);
}


/**
 * Index filter P2/F6-global. Picks globally a group of least frequent vectors
 * in the pattern to approximate P2. Seach parameter p2_select_threshold
 * controls the ratio of vectors to discard: 1.0 keeps all vectors, which is
 * the same as P2/V5, 0.0 discards all. Value 0.5 is generally a good choise.
 * Longest vectors are discarded and the shortest will be kept.
 *
 * @param sc a song collection
 * @param pattern the input pattern to search for
 * @param alg algorithm ID as defined in search.h; not used in this function
 * @param parameters search parameters
 * @param ms information about the found matches will be stored here
 */
void filter_p2_select_global(const songcollection *sc, const song *pattern,
        int alg, const searchparameters *parameters, matchset *ms) {
    int i, j;
    int vcount = pattern->size * parameters->p2_window;

    int songid = INT_MIN;
    int previous_key = INT_MIN;
    int previous_spos = -1;
    int previous_ppos = -1;
    int count = 0;
    int maxcount;
    vector *pnotes = pattern->notes;
    vectorindex *vindex = sc->data[DATA_VINDEX];
    pqroot *pq;
    song *songs = sc->songs;
    patternvector *chosenvectors;
    int *bucket_size = (int *) malloc(vcount * sizeof(int));
    if (bucket_size == NULL) return;


    if (vindex == NULL) {
        fputs("Error in filter_p2_select_global(): song collection does not contain\nvectorindex data. Use update_song_collection_data() before calling this function\n", stderr);
        free(bucket_size);
        return;
    }

    /* Calculate the index bucket size median of the possible index vectors. */
    vcount = 0;
    for (i=0; i<pattern->size - 1; ++i) {
        int end = i + parameters->p2_window;
        if (end >= pattern->size) end = pattern->size - 1;
        for (j=i+1; j<=end; ++j) {
            int x = pattern->notes[j].strt - pattern->notes[i].strt;
            int y = pattern->notes[j].ptch - pattern->notes[i].ptch;
            indexvector *iv = get_index_vector(vindex, x, y);
            if (iv != NULL) {
                bucket_size[vcount] = iv->size;
                ++vcount;
            }
        }
    }

    if (vcount * parameters->p2_select_threshold < vcount) {
        maxcount = kth_smallest(bucket_size, vcount, MIN2(vcount-1,
                (int) (((float) vcount) * parameters->p2_select_threshold)));
    } else maxcount = INT_MAX;

    free(bucket_size);
    vcount = vcount * parameters->p2_select_threshold;
    if (vcount == 0) {
        return;
    }

    chosenvectors = (patternvector *) malloc(vcount * sizeof(patternvector));

    /* Pick vectors from the pattern. */
    for (i=0; i<pattern->size - 1; ++i) {
        int end = i + parameters->p2_window;
        if (end >= pattern->size) end = pattern->size - 1;
        for (j=i+1; j<=end; ++j) {
            int x = pnotes[j].strt - pnotes[i].strt;
            int y = pnotes[j].ptch - pnotes[i].ptch;
            indexvector *iv = get_index_vector(vindex, x, y);
            if ((iv != NULL) && (iv->size <= maxcount)) {
                chosenvectors[count].iv = iv;
                chosenvectors[count].patternpos = i;
                chosenvectors[count].shift = (((int) pnotes[i].strt -
                    (int) pnotes[0].strt) << 8) +
                    (int) pnotes[i].ptch -
                    (int) pnotes[0].ptch + NOTE_PITCHES;
                chosenvectors[count].t = 0;
                ++count;
                if (count >= vcount) {
                    j = end;
                    i = pattern->size;
                }
            }
        }
    }
    /* Initialize a priority queue. */
    pq = pq_create(vcount);

    /* Merge the location lists by inserting them to the priority queue. */
    for (i=0; i<vcount; ++i) {
        pqnode *node = pq_getnode(pq, i);
        indexrec *ir = chosenvectors[i].iv->records;
        song *s = &songs[ir->song];
        vector *textnote = &s->notes[ir->note];
        vector *patternnote = &pnotes[chosenvectors[i].patternpos];
        node->key1 = ir->song;
        node->key2 = ((int) (textnote->strt - patternnote->strt) << 8) +
                (int) (textnote->ptch - patternnote->ptch) + NOTE_PITCHES;
        node->pointer = ir;
        pq_update(pq, node);
        chosenvectors[i].t = 1;
    }

    /* Scan through the data and mark the location where the largest number
     * of vectors are in the same relative positions as in the pattern. */
    count = 0;
    maxcount = 0;
    --vcount;
    while (1) {
        pqnode *min = pq_getmin(pq);
        i = min->index;
        if ((min->key1 == songid) && (min->key2 == previous_key)) {
            ++count;
            if (count > maxcount) maxcount = count;
        } else {
            /* Check if the previous match was a good one */
            if ((count == maxcount) && (count > 1)) {
#ifdef ORDER_F4_F5_RESULTS_WITH_P2
                alignment_check_p2(&songs[songid], previous_spos,
                        pattern, previous_ppos, ms);
#else
                int match_start = (previous_key >> 8) + pnotes[0].strt;
                int match_end = (previous_key >> 8) +
                        pnotes[pattern->size - 1].strt +
                        pnotes[pattern->size - 1].dur;
                char transposition = (char) ((previous_key & 0xFF) -
                        NOTE_PITCHES);
                float similarity = ((float) maxcount) / ((float) vcount);
                insert_match(ms, songid, match_start, match_end, transposition,
                        similarity);
#endif
            }
            if (min->key1 == INT_MAX) break;

            count = 0;
            if (songid != min->key1) {
                /* Next song, reset counter */
                songid = min->key1;
                maxcount = 0;
            }
            previous_key = min->key2;
            previous_ppos = chosenvectors[i].patternpos;
            previous_spos = ((indexrec *) min->pointer)->note;
        }
        if (chosenvectors[i].t < chosenvectors[i].iv->size) {    
            indexrec *ir = &chosenvectors[i].iv->records[chosenvectors[i].t];
            song *s = &songs[ir->song];
            vector *textnote = &s->notes[ir->note];
            vector *patternnote = &pnotes[chosenvectors[i].patternpos];
            min->key1 = ir->song;
            min->key2 = ((int) (textnote->strt - patternnote->strt) << 8) +
                    (int) (textnote->ptch - patternnote->ptch) + NOTE_PITCHES;
            min->pointer = ir;
            pq_update(pq, min);
            ++chosenvectors[i].t;
        } else {
            min->key1 = INT_MAX;
            min->key2 = INT_MAX;
            pq_update(pq, min);
        }
    }
    free(chosenvectors);
    pq_free(pq);
}


/**
 * Index filter P2'/F7. Picks all valid pattern vectors around user-specified
 * notes that must be found in the matches. Otherwise works like the generic P2
 * filter.
 *
 * @param sc a song collection
 * @param pattern the input pattern to search for
 * @param alg algorithm ID as defined in search.h; not used in this function
 * @param parameters search parameters
 * @param ms information about the found matches will be stored here
 */
void filter_p2_points(const songcollection *sc, const song *pattern,
        int alg, const searchparameters *parameters, matchset *ms) {
    int i;
    int vcount = 0;

    vector *pnotes = pattern->notes;
    song *songs = sc->songs;
    patternvector *chosenvectors = (patternvector *) malloc(
            2 * parameters->d_window * sizeof(patternvector));
    vectorindex *vindex = sc->data[DATA_VINDEX];
    int num_points = parameters->p2_num_points;
    int *points = parameters->p2_points;

    if (chosenvectors == NULL) return;
    if (vindex == NULL) {
        fputs("Error in filter_p2_points(): song collection does not contain\nvectorindex data. Use update_song_collection_data() before calling this function\n", stderr);
        free(chosenvectors);
        return;
    }

    if (num_points > 1) {
        int smallestcount = INT_MAX;
        int smallestpos = -1;
        int ppos = -1;
        indexvector *smallestiv;
        for (i=0; i<num_points; ++i) {
            int j;
            for (j=i+1; j<num_points; ++j) {
                int x, y;
                int pj = points[j];
                int pi = points[i];
                indexvector *iv;
                if (pj >= pi + parameters->d_window) break;
                x = pnotes[pj].strt - pnotes[pi].strt;
                y = pnotes[pj].ptch - pnotes[pi].ptch;
                iv = get_index_vector(vindex, x, y);
                if ((iv != NULL) && (iv->size < smallestcount)) {
                    smallestcount = iv->size;
                    smallestiv = iv;
                    ppos = pi;
                }
            }
        }
        if (smallestpos >= 0) {
            chosenvectors[vcount].iv = smallestiv;
            chosenvectors[vcount].patternpos = ppos;
            chosenvectors[vcount].shift = (((int) pnotes[ppos].strt -
                    (int) pnotes[0].strt) << 8) +
                    (int) pnotes[ppos].ptch -
                    (int) pnotes[0].ptch + NOTE_PITCHES;
            chosenvectors[vcount].t = 0;
            ++vcount;       
        } else num_points = 1;
    }

    if (num_points <= 1) {
        int p, start, end;
        if (num_points == 0) p = 0;
        else p = points[0];
        start = p - parameters->d_window;
        if (start < 0) start = 0;
        end = p + parameters->d_window;
        if (end >= pattern->size) end = pattern->size - 1;
        for (i=start; i<=end; ++i) {
            int x, y, pstart;
            indexvector *iv;
            /*printf("%d %d %d %d\n", i, p, start, end);*/
            if (i < p) {
                pstart = i;
                x = pnotes[p].strt - pnotes[i].strt;
                y = pnotes[p].ptch - pnotes[i].ptch;
            } else if (i > p) {
                pstart = p;
                x = pnotes[i].strt - pnotes[p].strt;
                y = pnotes[i].ptch - pnotes[p].ptch;
            } else continue;
            iv = get_index_vector(vindex, x, y);
            if (iv != NULL) {
                chosenvectors[vcount].iv = iv;
                chosenvectors[vcount].patternpos = pstart;
                chosenvectors[vcount].shift = (((int) pnotes[pstart].strt -
                    (int) pnotes[0].strt) << 8) +
                    (int) pnotes[pstart].ptch -
                    (int) pnotes[0].ptch + NOTE_PITCHES;
                ++vcount;
            }
        }
        for (i=0; i<vcount; ++i) {
            int j;
            indexvector *iv = chosenvectors[i].iv;
            /*printf("%d: %d\n", i, chosenvectors[i].patternpos);*/
            for (j=0; j<iv->size; ++j) {
                alignment_check_p2(&songs[iv->records[j].song],
                        iv->records[j].note, pattern,
                        chosenvectors[i].patternpos, ms);
            }
        }
    } else {
        song subpat;

        subpat.notes = (vector *) malloc(num_points * sizeof(vector));
        if (subpat.notes == NULL) {
            free(chosenvectors);
            return;
        }
        subpat.size = num_points;
        for (i=0; i<num_points; ++i) {
            subpat.notes[i] = pnotes[points[i]];
        }
 
        for (i=0; i<vcount; ++i) {
            int j;
            indexvector *iv = chosenvectors[i].iv;
            /*printf("%d: %d\n", i, chosenvectors[i].patternpos);*/
            for (j=0; j<iv->size; ++j) {
                song *s = &songs[iv->records[j].song];
                int spos = iv->records[j].note;
                int ppos = chosenvectors[i].patternpos;
                if (alignment_check_p1(s, spos, pattern, ppos, NULL))
                    alignment_check_p2(s, spos, pattern, ppos, ms);
            }
        }
        free(subpat.notes);
    }

    free(chosenvectors);
}

