/*
 * align_P3.c - A modified version of the P3 implementation for symbolic score
 *              alignment
 *
 * Version 2008-02-18
 *
 *
 * Copyright (C) 2004 Mika Turkia
 * Copyright (C) 2008 Niko Mikkila
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

#include "search.h"
#include "song.h"
#include "geometric_P3.h"
#include "priority_queue.h"
#include "util.h"
#include "align.h"
#include "align_P3.h"
#include "song_window.h"
#include "song_window_P3.h"


/*#define MAP_VALUE_AVERAGE 1*/


/** 
 * A modified version of the geometric P3 symbolic music retrieval algorithm
 * for score alignment.
 *
 * @param p3s song to scan
 * @param pattern pattern song
 * @param aline_size length of the alignment score array
 * @param aline_shift accuracy of the score array is calculated as
 *        1 << aline_shift. For example, aline_shift=8 results in each
 *        array slot representing 256 milliseconds of time.
 * @param aline the resulting alignment score for each position of the
 *        pattern within the target song will be written to this array.
 *        The array should contain negative values by default so that
 *        gaps can be filled afterwards.
 * @param transpositions pattern transposition for each aline value.
 */
static void align_turningpoints_p3(const p3song *p3s, const song *pattern,
        int aline_size, int aline_shift, int *aline, char *transpositions) {

    int i, j, num_loops;
    int best = 0;
    float pattern_duration = 0.0;
    VerticalTranslationTableItem *verticaltranslationtable;
    VerticalTranslationTableItem *item = NULL;
    TurningPoint *startpoints = p3s->startpoints;
    TurningPoint *endpoints = p3s->endpoints;
    int num_tpoints = p3s->size;
    int pattern_size = pattern->size;
    vector *pnotes = pattern->notes;

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

    /*printf("P3 Notes: %d\n", p3s->size);*/

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
        start = x + pnotes[0].strt;
        if (start >= 0) {
            start >>= aline_shift;
            if ((start < aline_size) && (aline[start] < item->value)) {
                aline[start] = item->value;
                transpositions[start] = y;
            }
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

    /* Free the reserved memory. */
    pq_free(pq);
    free(translation_vectors);
    free(verticaltranslationtable);
}


/**
 * Aligns a pattern with a song with the geometric P3 algorithm, by using
 * an initial alignment as a bootstrap. Alignment scores will be calculated
 * for each position and returned as a list of start times for each pattern
 * note.
 *
 * @param sc song collection that possibly contains the song in the p3song
 *        format that this function uses. If sc is NULL, necessary conversions
 *        will be made before further processing.
 * @param s song to align with the pattern
 * @param p pattern
 * @param parameters alignment parameters
 * @param initial_align initial alignment
 * @param result_align resulting refined alignment
 */
/*void align_p3(const songcollection *sc, const song *s, const song *p, int alg,
        const alignparameters *parameters, const alignment *initial_align,
        alignment *result_align) {

}*/

/**
 * Maps local alignments of two songs with the geometric P3 algorithm.
 *
 * @param sc song collection that possibly contains the song in the p3song
 *        format that this function uses. If sc is NULL, necessary conversions
 *        will be made before further processing.
 * @param s song to align the pattern with. If either of the aligned songs
 *        is the "correct" score, it should be given here to improve alignment
 *        results of the P3 algorithm
 * @param p a pattern song that is aligned with the other song. This should
 *        be the piece that may contain more errors.
 * @param parameters alignment parameters
 * @param map the resulting alignment map will be written to this structure 
 */
void align_p3(const songcollection *sc, const song *s, const song *p,
        int alg, const alignparameters *parameters, alignmentmap *map) {


    
}

#if 0
    p3songcollection *p3sc = sc->data[DATA_P3];
    p3song *p3s;
    p3song *scaled_p3s;
    int *aline;
    char *transpositions;
    int map_height, map_width;
    int accuracy = parameters->map_accuracy;
    int i;
    int w_start, w_end;
    int w_size = parameters->p3_pattern_window_size;
    int buffer_pos = 0;
    int pduration = 0, sduration = 0;
    int num_scales = parameters->p3_num_scales;
    float max_scale = 1.0F;
    TurningPoint *orig_sp, *orig_ep;

    /* Buffer that is used when calculating map lines */
    unsigned char **tmplines;
    unsigned int **curline;
    int tmplines_num = MIN2(4095, (1 + w_size / accuracy));
    int tmplines_pos, tmplines_patternpos;
    int tmplines_used = 0;
    float *tmpmultipliers;

    /* Pattern window */
    song pw;
    vector *pw_notes = (vector *) malloc(p->size * sizeof(vector));
    pw.notes = pw_notes;

    if ((p3sc == NULL) || (s->id >= p3sc->size)) {
        fputs("Error in map_alignments_p3: song collection does not contain P3 data.\nUse update_song_collection_data() to generate it.\n", stderr);
        return;
    }
    p3s = &p3sc->p3_songs[s->id];
    if (p3s == NULL) {
        fputs("Error in map_alignments_p3: empty P3 song data", stderr);
        return;
    }

    tmplines = (unsigned char **) malloc(num_scales * sizeof(unsigned char *));
    curline = (unsigned int **) malloc(num_scales * sizeof(unsigned int *));
    tmpmultipliers = (float *) malloc((tmplines_num+2) * sizeof(float *));
    tmplines_pos = 0;
    tmplines_patternpos = 0;
    tmpmultipliers[0] = 0.0F;
    for (i=1; i<tmplines_num+2; ++i) {
        tmpmultipliers[i] = 1.0F / ((float) i);
    }

    /*strt = 0;
    ppos = (int *) malloc(p->size * sizeof(int));*/
    for (i=0; i<p->size; ++i) {
        int e = p->notes[i].strt + p->notes[i].dur;
        if (e > pduration) pduration = e;
        /*if (p->notes[i].strt >= strt) {
            strt += accuracy;
            ppos[map_height] = i;
            map_height++;
        }*/
    }
    map_height = 1 + pduration / accuracy;

    sduration = p3s->endpoints[p3s->size-1].x;
    map_width = 1 + sduration / accuracy;

    /* Make scaled versions of the song */
    scaled_p3s = (p3song *) malloc(num_scales * sizeof(p3song));
    orig_sp = p3s->startpoints;
    orig_ep = p3s->endpoints;
    for (i=0; i<num_scales; ++i) {
        int j;
        float scale = parameters->p3_scales[i];
        int scaled_sduration = (int) ((float) sduration * scale) + 1;
        TurningPoint *sp, *ep;

        if (scale > max_scale) max_scale = scale;
        sp = (TurningPoint *) malloc(p3s->size *
                sizeof(TurningPoint));
        ep = (TurningPoint *) malloc(p3s->size *
                sizeof(TurningPoint));
        scaled_p3s[i].song = NULL;
        scaled_p3s[i].size = p3s->size;
        scaled_p3s[i].startpoints = sp;
        scaled_p3s[i].endpoints = ep;
        for (j=0; j<p3s->size; ++j) {
            sp[j].x = (int) ((float) orig_sp[j].x * scale);
            sp[j].y = orig_sp[j].y;
            ep[j].x = (int) ((float) orig_ep[j].x * scale);
            ep[j].y = orig_ep[j].y;
        }
        tmplines[i] = (unsigned char *) calloc(tmplines_num * scaled_sduration,
                sizeof(unsigned char));
        curline[i] = (unsigned int *) calloc(scaled_sduration,
                sizeof(unsigned int));
    }

    aline = (int *) calloc((int) (1.0F + max_scale *
            (float) sduration), sizeof(int));
    transpositions = (char *) malloc((int) (1.0F + max_scale *
            (float) sduration) * sizeof(char));

    map->width = map_width;
    map->height = map_height;
    map->accuracy = accuracy;
    map->lines = malloc(map_height * sizeof(alignmentline));
    map->vbuffer = (unsigned char *) calloc(map_height * map_width,
            sizeof(unsigned char));
    map->tbuffer = (char *) calloc(map_height * map_width, sizeof(char));
    map->target = s;
    map->pattern = p;
    map->target_duration = sduration;
    map->pattern_duration = pduration;

    w_start = 0;
    w_end = w_size;
    for (i=0; i<map_height; ++i, w_start+=accuracy, w_end+=accuracy) {
        int j, k, ns;
        float durationmul;
        alignmentline *mapline = &map->lines[i];

        if (parameters->verbose) {
            fprintf(stderr, "  %d / %d\r", i, map_height);
        }

        mapline->pattern_position = 0;
        mapline->pattern_time = w_start;
        mapline->values = &map->vbuffer[buffer_pos];
        mapline->transpositions = &map->tbuffer[buffer_pos];
        buffer_pos += map_width;

        /* Pick notes that fall within the window */
        k = 0;
        ns = 0;
        pw.duration = 0;
        for (j=0; j<p->size; ++j) {
            vector *note = &p->notes[j];
            if ((note->strt < w_end) &&
                    ((note->strt + note->dur) > w_start)) {
                vector *pwnote = &pw_notes[k];
                pwnote->strt = MAX2(w_start, note->strt);
                pwnote->dur = (MIN2(w_end, (note->strt + note->dur))) -
                        pwnote->strt;
                pwnote->ptch = note->ptch;
                if (k == 0) {
                    mapline->pattern_position = j;
                    ns = pwnote->strt;
                }
                pwnote->strt -= ns;
                pw.duration += pwnote->dur;
                ++k;
            } else if (note->strt > w_end) break;
        }
        if (k == 0) {
            continue;
        }
        pw.size = k;
        lexicographic_sort(&pw);
        durationmul = 255.0F / ((float) pw.duration);
        
        for (ns=0; ns<num_scales; ++ns) {
            float scale = parameters->p3_scales[ns];
            int scaled_sduration = (int) ((float) sduration * scale) + 1;
            unsigned char *tmpline =
                    &tmplines[ns][tmplines_pos*scaled_sduration];
            unsigned int *cline = curline[ns];

            memset(aline, 0, scaled_sduration * sizeof(int));
            /*memset(transpositions, 0, scaled_sduration * sizeof(char));*/

            align_turningpoints_p3(&scaled_p3s[ns], &pw, scaled_sduration,
                    aline, transpositions);

            for (j=0; j<scaled_sduration; ++j) {
                aline[j] = (int) (durationmul * ((float) aline[j]));
                cline[j] += (unsigned int) (1<<20) + ((unsigned int) aline[j]);
            }

            /* Read the current finished map line */
            j = 0;
            k = 0;
            while ((j < scaled_sduration) && (k < map_width)) {
                int end = MIN2(j + (int) (scale * (float) accuracy),
                        scaled_sduration);
                int val = 0;
               /* char transp = 0;*/

#ifdef MAP_VALUE_AVERAGE
                int num = 0;
                for (; j<end; ++j) {
                    int cval = ((float) (cline[j] & 0xFFFFF)) *
                            tmpmultipliers[cline[j] >> 20];
                    if (cval > 0) {
                        val += cval;
                        ++num;
                    }
                }
                val = ((float) val) / ((float) num);
#else
                for (; j<end; ++j) {
                    unsigned char tmp;
                    tmp = (unsigned char) (((float)
                            (cline[j] & 0xFFFFF)) * tmpmultipliers[cline[j] >> 20]);
                    val = MAX2(val, tmp);
                }
#endif

                mapline->values[k] = (unsigned char) MAX2(mapline->values[k],
                        val);
                ++k;
            }
            if (tmplines_used >= tmplines_num) {
                int l;
                int start = accuracy * (tmplines_num + 1);
                j = scaled_sduration - accuracy - 1;
                k = scaled_sduration - 1;
                l = scaled_sduration - start - 1;
                for (; l >= 0; --j,--k,--l) {
                    cline[k] = cline[j] - (unsigned int) (1<<20) -
                            ((unsigned int) tmpline[l]);
                }
                for (; j >= 0; --j,--k) {
                    cline[k] = cline[j];
                }
                for (; k >= 0; --k) {
                    cline[k] = 0;
                }
            } else {
                /* Shift the line */
                j = scaled_sduration - accuracy - 1;
                k = scaled_sduration - 1;
                for(; j >= 0; --j,--k) {
                    cline[k] = cline[j];
                }
                for(; k >= 0; --k) {
                    cline[k] = 0;
                }
            }
            for (j=0; j<scaled_sduration; ++j) {
                tmpline[j] = (unsigned char) aline[j];
            }
        }
        ++tmplines_used;
        ++tmplines_pos;
        if (tmplines_pos >= tmplines_num) tmplines_pos = 0;
    }
    for (i=0; i<num_scales; ++i) {
        free(scaled_p3s[i].startpoints);
        free(scaled_p3s[i].endpoints);
        free(tmplines[i]);
        free(curline[i]);
    }
    free(curline);
    free(tmplines);
    free(scaled_p3s);
    free(pw_notes);
    free(aline);
    free(transpositions);
    free(tmpmultipliers);
}
#endif

