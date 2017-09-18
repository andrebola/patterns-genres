/*
 * geometric_P2.c - Geometric algorithm P2
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


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>

#include "config.h"
#include "search.h"
#include "song.h"
#include "util.h"
#include "priority_queue.h"
#include "geometric_P2.h"


#define COMPENSATION_FACTOR 2


/**
 * Search a song collection with scan_song_p2().
 *
 * @param sc a song collection to scan
 * @param pattern pattern to search for
 * @param alg search algorithm to use. See algorithms.h for algorithm IDs.
 * @param parameters search parameters
 * @param ms match set for returning search results
 */
void alg_p2(const songcollection *sc, const song *pattern, int alg,
        const searchparameters *parameters, matchset *ms) {
    int i;
    song *q_pattern = NULL;
    const song *pat;

    if (!init_match_set(ms, 500000, 0, 0)) {
    
        fprintf(stderr, "Error to allocate memory \n");
    }

    if (parameters->quantization > 0) {
        q_pattern = p2_compensate_quantization(pattern,
                parameters->quantization);
        pat = q_pattern;
    } else pat = pattern;

    for (i=0; i<sc->size; ++i) {
#ifdef DEBUG
        fprintf(stderr, "Scanning song %s\n", sc->songs[i].title);
        fprintf(stderr, "Pattern size: %d\n", pat->size);
#endif
        //scan_song_p2(&sc->songs[i], pat, pat->size, ms); // TODO: pass parameter->quantization
        scan_song_p2(&sc->songs[i], pattern, pattern->size, ms);
    }

    if (q_pattern != NULL) {
        free_song(q_pattern);
        free(q_pattern);
    }
}


/**
 * Search a song collection with scan_song_p2_points().
 *
 * @param sc a song collection to scan
 * @param pattern pattern to search for
 * @param alg search algorithm to use. See algorithms.h for algorithm IDs.
 * @param parameters search parameters
 * @param ms match set for returning search results
 */
void alg_p2_points(const songcollection *sc, const song *pattern, int alg,
        const searchparameters *parameters, matchset *ms) {
    int i;
    song *q_pattern = NULL;
    const song *pat;

    if (parameters->quantization > 0) {
        q_pattern = p2_compensate_quantization(pattern,
                parameters->quantization);
        pat = q_pattern;
    } else pat = pattern;

    for (i=0; i<sc->size; ++i) {
#ifdef DEBUG
        fprintf(stderr, "Scanning song %s\n", sc->songs[i].title);
#endif
        scan_song_p2_points(&sc->songs[i], pat, parameters->p2_num_points,
                parameters->p2_points, ms);
    }

    if (q_pattern != NULL) {
        free_song(q_pattern);
        free(q_pattern);
    }
}


/**
 * Quantizes the pattern with two or more alternative notes instead of
 * just one. This may be useful with the P2 scanning algorithms (but not with
 * index filters).
 *
 * @param p the pattern to quantize
 * @param q amount of quantization in milliseconds
 *
 * @return a quantized pattern
 */
song *p2_compensate_quantization(const song *p, const int q) {
    int i;
    vector *qnotes, *pnotes;
    song *newp = (song *) malloc(sizeof(song));

    if (newp == NULL) return NULL;
    memcpy(newp, p, sizeof(song));
    newp->size = p->size * COMPENSATION_FACTOR;
    newp->notes = (vector *) malloc(newp->size *
            sizeof(vector));
    if (newp->notes == NULL) {
        free(newp);
        return NULL;
    }   

    pnotes = p->notes;
    qnotes = newp->notes;
    for (i=0; i<p->size; ++i) {
        vector *pn = &pnotes[i];
        vector *qn = &qnotes[i*COMPENSATION_FACTOR];
        qn->strt = pn->strt;
        qn->ptch = pn->ptch;
        qn->dur = pn->dur;
        qn->velocity = pn->velocity;
        qn->instrument = pn->instrument;

        qn = &qnotes[i*COMPENSATION_FACTOR+1];
        qn->strt = pn->strt + q;
        qn->ptch = pn->ptch;
        qn->dur = pn->dur;
        qn->velocity = pn->velocity;
        qn->instrument = pn->instrument;

        if (COMPENSATION_FACTOR == 3) {
            qn = &qnotes[i*COMPENSATION_FACTOR+2];
            qn->strt = pn->strt + q + q;
            qn->ptch = pn->ptch;
            qn->dur = pn->dur;
            qn->velocity = pn->velocity;
            qn->instrument = pn->instrument;
        }
    }
    lexicographic_sort(newp);
    return newp;
}


/**
 * Scanning phase of geometric algorithm P2. This algorithm is described in
 * Esko Ukkonen, Kjell Lemstrom and Veli Makinen: Sweepline the Music! In
 * Computer Science in Perspective (LNCS 2598), R. Klein, H.-W. Six, L. Wegner
 * (Eds.), pp. 330-342, 2003.
 *
 * Unlike in the article, end of source is not detected by putting
 * (infinity,infinity) to the end of source, but with indexes.
 * Also because the source format consists of variable length chords
 * the implementation is fairly complex and differs somewhat from
 * the pseudocode. Consult the article for details.
 *
 * @param s the song to scan
 * @param p pattern to search for
 * @param errors allowed number of errors (missing notes) in a match
 * @param ms pointer to a structure where the results will be stored
 *
 * @return 1 when successful, 0 otherwise
 */
int scan_song_p2(const song *s, const song *p, const int errors,
        matchset *ms) {

    int num_loops, i;
    pqroot *tree = NULL;
    unsigned int *q;
    int pattern_end;
    int c, maxcount, min_pattern_size;
    int previous_key;
    unsigned int matchpos = 0;
    vector *pattern = p->notes;
    vector *text = s->notes;

#if 0
    /* Keep all matched notes for the best match here. This is not
     * actually used, but serves as an example of what information
     * can be extracted. */
    unsigned int matchednotes[pattern_size];
#endif
#ifdef P2_CALCULATE_COMMON_DURATION
    float common_duration = 0;
    unsigned int pattern_duration = 0;
#endif

    if ((p->size == 0) || (s->size == 0)) return 0;
    if (errors >= p->size) min_pattern_size = 0;
    else min_pattern_size = p->size - errors;

    q = (unsigned int *) malloc(p->size * sizeof(unsigned int));

    /* Initialize the priority queue */
    tree = pq_create(p->size);

    pattern_end = p->notes[p->size-1].strt;

    /* Initialize q array: all point to the first note of the source */
    for (i = 0; i < p->size; i++) {
        pqnode *node;
        q[i] = 0;

        /* Add translation vectors to the priority queue */
        node = pq_getnode(tree, i);
        node->key1 = (((int) text[0].strt - (int) pattern[i].strt +
                pattern_end) << 8) + (int) text[0].ptch -
                (int) pattern[i].ptch + NOTE_PITCHES;
        /* printf("key: %d\n", node->key1); */
        pq_update_key1_p2(tree, node);
#ifdef P2_CALCULATE_COMMON_DURATION
        pattern_duration += pattern[i].dur;
#endif
    }

    c = 0;
    maxcount = 1;
    previous_key = INT_MIN;
    
    /* Loop as long as we can take items away from the priority queue.
     * p->size items are added before,
     * p->size * s->size - p->size items are added in the loop. */
    num_loops = s->size * p->size;

    /* Get the smallest translation vector.
     * Equal difference vectors come out of priority queue in min_key order. */
    for (i = 0; i < num_loops; i++) {
        pqnode *min = pq_getmin(tree);
        unsigned int patternpos = min->index;
        int textpos = q[patternpos];
        vector *textnote = &text[textpos];
        vector *patternnote = &pattern[patternpos];

        if (previous_key == min->key1) {
            /* Another matching note */

#if 0
            /* Store matched note index */
            matchednotes[c] = textpos;
#endif
            ++c;
            if (c > maxcount) maxcount = c;
        } else {
            /* end of a matching section */
            if ((c == maxcount) && (c >= min_pattern_size)) {
                int start = (previous_key >> 8) + pattern[0].strt - pattern_end;
                int end = (previous_key >> 8) + pattern[p->size - 1].strt +
                        pattern[p->size - 1].dur - pattern_end;
                char transposition = (char) ((previous_key & 0xFF) -
                        NOTE_PITCHES);
#ifdef P2_CALCULATE_COMMON_DURATION                   
                float similarity = common_duration / pattern_duration;
#elif P2_NORMALIZE_SIMILARITY
                float similarity = ((float) c + 1.0F) /
                        ((float) MIN2(p->size, s->size));
#else
                float similarity = ((float) c + 1.0F) /
                        ((float) p->size);
#endif
                insert_match(ms, s->id, start, end,
                        transposition, similarity);
            }
            previous_key = min->key1;
            matchpos = textpos;
            c = 0;
#ifdef P2_CALCULATE_COMMON_DURATION                   
            common_duration = 0.0F;
#endif
#if 0
            matchednotes[0] = textpos;
#endif
        }

#ifdef P2_CALCULATE_COMMON_DURATION
        /* Count common duration of matching notes in pattern and text. */
        if (patternnote->dur < textnote->dur)
            common_duration += patternnote->dur;
        else common_duration += textnote->dur;
#endif

        /* Update q pointer: move to next position in the text
         * (from the current position pointed by q). */
        if (textpos < s->size - 1) {
            /* The current pointer is not at the end of source:
             * move to the next note */
            q[patternpos]++;
            textnote = &text[q[patternpos]];

            /* Update the vector in the priority queue. */
            min->key1 = (((int) textnote->strt - (int) patternnote->strt +
                    pattern_end) << 8) + (int) textnote->ptch -
                    (int) patternnote->ptch + NOTE_PITCHES;
            pq_update_key1_p2(tree, min);
        } else {
            /* Current pointer is at the end of the text;
             * remove the difference vector from the priority queue. */
            min->key1 = INT_MAX;
            pq_update_key1_p2(tree, min);
        }
    }

    free(q);
    pq_free(tree);
    return 1;
}


/**
 * Counts the number of matching notes for a given pattern and data
 * position.
 *
 * @param s a song
 * @param songpos position in the song
 * @param p pattern to match
 * @param patternpos position in the pattern that aligns with songpos
 * @param ms structure where the match information is stored to
 *
 * @return match information item
 */
match *alignment_check_p2(const song *s, unsigned short songpos,
        const song *p, unsigned short patternpos, matchset *ms) {

    int i, j, end, count;
    int p0, v0;
    int mstart, mend;
    char mtransposition;
    float msimilarity;
    vector *pnotes = p->notes;
    vector *snotes = s->notes;
    match *m;

    if ((songpos >= s->size) || (patternpos >= p->size)) return NULL;

    /* Scan the end */
    count = 1;
    i = patternpos + 1;
    j = songpos;

    p0 = ((int) pnotes[patternpos].strt << 8) +
            (int) pnotes[patternpos].ptch;
    v0 = ((int) snotes[songpos].strt << 8) +
            (int) snotes[songpos].ptch;

    for (; i < p->size; ++i) {
        int pi = ((int) pnotes[i].strt << 8) + (int) pnotes[i].ptch - p0;
        int vi = INT_MIN;

        /* Skip over notes that are not in the pattern. */
        do {
            ++j;
            if (j >= s->size) break;
            vi = ((int) snotes[j].strt << 8) + (int) snotes[j].ptch - v0;
        } while (vi < pi);

        /* Increase counter if there is a matching note */
        if (vi == pi) ++count;
        else --j;
    }
    end = j;

    /* Scan the beginning */
    i = patternpos - 1;
    j = songpos;
    for (; i >= 0; --i) {
        int pi = p0 - ((int) pnotes[i].strt << 8) - (int) pnotes[i].ptch;
        int vi = INT_MIN;
        /* Skip over notes that are not in the pattern. */
        do {
            --j;
            if (j < 0) break;
            vi = v0 - ((int) snotes[j].strt << 8) - (int) snotes[j].ptch;
        } while (vi < pi);

        /* Increase counter if there is a matching note */
        if (vi == pi) ++count;
        else ++j;
    }

    mstart = snotes[songpos].strt - pnotes[patternpos].strt;
    mend = mstart + pnotes[p->size-1].strt + pnotes[p->size-1].dur;
    mtransposition = snotes[songpos].ptch - pnotes[patternpos].ptch;
    msimilarity = ((float) count) / ((float) p->size);
    m = insert_match(ms, s->id, mstart, mend, mtransposition, msimilarity);

    /* Find out matching note positions if they are requested */
    if ((m != NULL) && (m->num_notes > 0) && (m->notes != NULL)) {
        m->notes[0] = j;
        p0 = ((int) pnotes[0].strt << 8) +
                (int) pnotes[0].ptch;
        v0 = ((int) snotes[j].strt << 8) +
                (int) snotes[j].ptch;

        /*if (m->num_notes > p->size) m->num_notes = p->size;*/
        for (i = 1; i < m->num_notes; ++i) {
            int pi = ((int) pnotes[i].strt << 8) + (int) pnotes[i].ptch - p0;
            int vi;

            /* Skip over notes that are not in the pattern. */
            do {
                ++j;
                if (j >= s->size) return m;
                vi = ((int) snotes[j].strt << 8) + (int) snotes[j].ptch - v0;
            } while (vi < pi);

            /* Is there a matching note? If not, exit. */
            if (vi != pi) m->notes[i] = -1;
            else m->notes[i] = j;
        }
    }
    return m;
}


/**
 * Scanning phase of geometric algorithm P2' when only a single point is
 * specified and P1 cannot be used as a first step.
 *
 * @param s the song to scan
 * @param p pattern to search for
 * @param errors allowed number of errors (missing notes) in a match
 * @param ms pointer to a structure where the results will be stored
 * @param num_points number of fixed pattern points
 * @param points an array of pattern note positions, specifying the group
 *        of fixed points
 *
 * @return 1 when successful, 0 otherwise
 */
static int scan_song_p2_1_point(const song *s, const song *p, const int errors,
        matchset *ms, int num_points, const int *points) {

    int num_loops, i;
    pqroot *tree = NULL;
    unsigned int *q;
    int pattern_end;
    int c, maxcount, min_pattern_size;
    int previous_key;
    unsigned int matchpos = 0;
    vector *pattern = p->notes;
    vector *text = s->notes;
    char *point_found;

#if 0
    /* Keep all matched notes for the best match here. This is not
     * actually used, but serves as an example of what information
     * can be extracted. */
    unsigned int matchednotes[pattern_size];
#endif
#ifdef P2_CALCULATE_COMMON_DURATION
    float common_duration = 0;
    unsigned int pattern_duration = 0;
#endif

    if ((p->size == 0) || (s->size == 0)) return 0;
    if (errors >= p->size) min_pattern_size = 0;
    else min_pattern_size = p->size - errors;

    point_found = (char *) calloc(p->size, sizeof(char));
    q = (unsigned int *) malloc(p->size * sizeof(unsigned int));

    /* Initialize the priority queue */
    tree = pq_create(p->size);

    pattern_end = p->notes[p->size-1].strt;

    /* Initialize q array: all point to the first note of the source */
    for (i = 0; i < p->size; i++) {
        pqnode *node;
        q[i] = 0;

        /* Add translation vectors to the priority queue */
        node = pq_getnode(tree, i);
        node->key1 = (((int) text[0].strt - (int) pattern[i].strt +
                pattern_end) << 8) + (int) text[0].ptch -
                (int) pattern[i].ptch + NOTE_PITCHES;
        /* printf("key: %d\n", node->key1); */
        pq_update_key1_p2(tree, node);
#ifdef P2_CALCULATE_COMMON_DURATION
        pattern_duration += pattern[i].dur;
#endif
    }

    c = 0;
    maxcount = 1;
    previous_key = INT_MIN;
    
    /* Loop as long as we can take items away from the priority queue.
     * p->size items are added before,
     * p->size * s->size - p->size items are added in the loop. */
    num_loops = s->size * p->size;

    /* Get the smallest translation vector.
     * Equal difference vectors come out of priority queue in min_key order. */
    for (i = 0; i < num_loops; i++) {
        pqnode *min = pq_getmin(tree);
        unsigned int patternpos = min->index;
        int textpos = q[patternpos];
        vector *textnote = &text[textpos];
        vector *patternnote = &pattern[patternpos];

        if (previous_key == min->key1) {
            /* Another matching note */

#if 0
            /* Store matched note index */
            matchednotes[c] = textpos;
#endif
            ++c;
            if (c > maxcount) maxcount = c;
            point_found[patternpos] = 1;
        } else {
            int j;
            /* end of a matching section */
            if ((c == maxcount) && (c >= min_pattern_size)) {
                j = 0;
                for (; j<num_points; ++j) {
                    int pos = points[j];
                    if (!point_found[pos]) {
                        j = -1;
                        break;
                    }
                }
                if (j >= 0) { 
                    int start = (previous_key >> 8) + pattern[0].strt -
                            pattern_end;
                    int end = (previous_key >> 8) + pattern[p->size - 1].strt +
                            pattern[p->size - 1].dur - pattern_end;
                    char transposition = (char) ((previous_key & 0xFF) -
                            NOTE_PITCHES);
#ifdef P2_CALCULATE_COMMON_DURATION                   
                    float similarity = common_duration / pattern_duration;
#elif P2_NORMALIZE_SIMILARITY
                    float similarity = ((float) c + 1.0F) /
                            ((float) MIN2(p->size, s->size));
#else
                    float similarity = ((float) c + 1.0F) /
                            ((float) p->size);
#endif
                    insert_match(ms, s->id, start, end,
                            transposition, similarity);
                }
            }
            previous_key = min->key1;
            matchpos = textpos;
            c = 0;
#ifdef P2_CALCULATE_COMMON_DURATION                   
            common_duration = 0.0F;
#endif
            for (j=0; j<num_points; ++j) {
                point_found[points[j]] = 0;
            }
#if 0
            matchednotes[0] = textpos;
#endif
        }

#ifdef P2_CALCULATE_COMMON_DURATION
        /* Count common duration of matching notes in pattern and text. */
        if (patternnote->dur < textnote->dur)
            common_duration += patternnote->dur;
        else common_duration += textnote->dur;
#endif

        /* Update q pointer: move to next position in the text
         * (from the current position pointed by q). */
        if (textpos < s->size - 1) {
            /* The current pointer is not at the end of source:
             * move to the next note */
            q[patternpos]++;
            textnote = &text[q[patternpos]];

            /* Update the vector in the priority queue. */
            min->key1 = (((int) textnote->strt - (int) patternnote->strt +
                    pattern_end) << 8) + (int) textnote->ptch -
                    (int) patternnote->ptch + NOTE_PITCHES;
            pq_update_key1_p2(tree, min);
        } else {
            /* Current pointer is at the end of the text;
             * remove the difference vector from the priority queue. */
            min->key1 = INT_MAX;
            pq_update_key1_p2(tree, min);
        }
    }

    free(point_found);
    free(q);
    pq_free(tree);
    return 1;
}



/**
 * Geometric P2': P2 with fixed points that must be found in the matches.
 * The algorithm calls normal P2 if less than two points are given.
 * Otherwise it first runs P1 and evaluates matches with a P2-based checking
 * function.
 *
 * @param s the song to scan
 * @param p pattern to search for
 * @param ms pointer to a structure where the results will be stored
 * @param num_points number of fixed pattern points
 * @param points an array of pattern note positions, specifying the group
 *        of fixed points
 *
 * @return 1 when successful, 0 otherwise
 */
int scan_song_p2_points(const song *s, const song *p,
        int num_points, const int *points, matchset *ms) {
    unsigned int i, j, pattern_size, song_size;
    int v0, p0;
    unsigned int *q;
    vector *pattern, *text;

    if ((p->size < 2) || (s->size < 2)) return 0;
    if (num_points < 2) return scan_song_p2_1_point(s, p, p->size, ms,
            num_points, points);

    pattern_size = num_points;
    song_size = s->size;
    pattern = p->notes;
    text = s->notes;

    /* q is an array that points to the last checked note for each
     * pattern position */
    q = (unsigned int *) calloc(pattern_size, sizeof(unsigned int));
    if (q == NULL) return 0;

    p0 = ((int) pattern[points[0]].strt << 8) + (int) pattern[points[0]].ptch
            - NOTE_PITCHES;

    /* Scan all notes */
    for (i = 0; i < song_size - pattern_size + 1; ++i) {
        v0 = ((int) text[i].strt << 8) + (int) text[i].ptch - p0;
        q[0] = i + 1;

        /* Start finding points of the transposed pattern starting from
         * this source position and do it until a match cannot be found. */
        for (j=1; j < pattern_size; ++j) {
            int v;
            int pj = ((int) pattern[points[j]].strt << 8) +
                    (int) pattern[points[j]].ptch
                    - NOTE_PITCHES;
            if (q[j] < q[j-1]) q[j] = q[j-1];
            if (q[j] == song_size) {
                free(q);
                return 0;
            }
            /* Move q over the notes that can't be part of a match for any
             * remaining pattern position. */
            do {
                v = ((int) text[q[j]].strt << 8) + (int) text[q[j]].ptch - pj;
                q[j]++;
            } while ((v < v0) && (q[j] < song_size));
            /* After the loop we are at either a matching note or bigger,
             * i.e. there is no matching note. */
            if (v != v0) {
                q[j]--;
                break;
            }
        }

        /* Check matching positions with P2 */
        if (j == pattern_size) {
            alignment_check_p2(s, i, p, points[0], ms);
        }
    }
    free(q);
    return 1;
}

