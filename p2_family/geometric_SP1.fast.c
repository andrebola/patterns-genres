/*
 * geometric_SP1.c - Geometric algorithm SP1
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
#include <math.h>
#include <string.h>

#include "config.h"
#include "song.h"
#include "util.h"
#include "geometric_SP1.h"

/*#define WRITE_PGM 1*/

#define MAX_PATTERN_SIZE 10000

#define COMPARE_TO_ORIGIN 1

#define ADJUST_SCALE 1

/*#define CALCULATE_WEIGHTS 1*/

#define WEIGHT_DURATION 1

#define ENFORCE_MAX_SKIPS 1

#define STEP_SCALE 1


#define TEMPO_FACTOR 2

/*#define ANCHOR_POS 3*/

#define ANCHOR_DELTA 2000

#define MAX_INITIAL_SKIP 2

#define MAX_SKIP 3

#define HASH_SHIFT 7


#define MAX_SONG_SILENCE 60000
#define END_PADDING 1000

#define PATTERN_MIN_DELTA 2.0F


/*
#define HASH_TIME_FACTOR 6

#define TRIGGER_WEIGHT 20

#define EFFECT_HEIGHT 5
#define EFFECT_WIDTH 19
#define EFFECT_CENTER_X 9
#define EFFECT_CENTER_Y 2

static char EFFECT[] = {
 0, 0, 0, 1, 2,  3,  4,  7, 13,  25, 13,  7,  4,  3, 2, 1, 0, 0, 0,
 0, 1, 2, 3, 4,  7, 13, 25, 50,  75, 50, 25, 13,  7, 4, 3, 2, 1, 0,
 1, 2, 3, 4, 7, 13, 25, 50, 75, 100, 75, 50, 25, 13, 7, 4, 3, 2, 1,
 0, 1, 2, 3, 4,  7, 13, 25, 50,  75, 50, 25, 13,  7, 4, 3, 2, 1, 0,
 0, 0, 0, 1, 2,  3,  4,  7, 13,  25, 13,  7,  4,  3, 2, 1, 0, 0, 0,
};
*/

/*
#define HASH_TIME_FACTOR 5

#define TRIGGER_WEIGHT 70

#define EFFECT_HEIGHT 5
#define EFFECT_WIDTH 33
#define EFFECT_CENTER_X 16
#define EFFECT_CENTER_Y 2

static char EFFECT[] = {
 0, 0, 0, 0, 1, 2, 3,  4,  5,  6,  8, 11, 13, 20, 25, 30,  35, 30, 25, 20, 13, 11,  8,  6,  5,  4, 3, 2, 1, 0, 0, 0, 0,
 0, 0, 1, 2, 3, 4, 5,  6,  8, 11, 13, 20, 25, 40, 50, 65,  75, 65, 50, 40, 25, 20, 13, 11,  8,  6, 5, 4, 3, 2, 1, 0, 0,
 1, 2, 3, 4, 5, 6, 8, 11, 13, 20, 25, 40, 50, 65, 75, 90, 100, 90, 75, 65, 50, 40, 25, 20, 13, 11, 8, 6, 5, 4, 3, 2, 1,
 0, 0, 1, 2, 3, 4, 5,  6,  8, 11, 13, 20, 25, 40, 50, 65,  75, 65, 50, 40, 25, 20, 13, 11,  8,  6, 5, 4, 3, 2, 1, 0, 0,
 0, 0, 0, 0, 1, 2, 3,  4,  5,  6,  8, 11, 13, 20, 25, 30,  35, 30, 25, 20, 13, 11,  8,  6,  5,  4, 3, 2, 1, 0, 0, 0, 0,
};
*/


#define HASH_TIME_FACTOR 48
#define HASH_TIME_MULTIPLIER 1.0F/48.0F

#define TRIGGER_WEIGHT 60

#define EFFECT_HEIGHT 3
#define EFFECT_WIDTH 19
#define EFFECT_CENTER_X 9
#define EFFECT_CENTER_Y 1

static char EFFECT[] = {
  1,  5, 10, 13, 20, 25, 40, 50, 65,  75, 65, 50, 40, 25, 20, 13, 10,  5,  1,
 11, 13, 20, 25, 40, 50, 65, 75, 90, 100, 90, 75, 65, 50, 40, 25, 20, 13, 11,
  1,  5, 10, 13, 20, 25, 40, 50, 65,  75, 65, 50, 40, 25, 20, 13, 10,  5,  1
};

/*0,  0,  0,  0,  0,  0,  2,  5,  8,  10,  8,  5,  2,  0,  0,  0,  0,  0,  0,*/



#ifdef WRITE_PGM
static void write_pgm(const char *path, const unsigned char *data,
        int w, int h) {
    int i;
    FILE *f;
    unsigned char buf[1];

    f = fopen(path, "w");
    if (f == NULL) {
        fprintf(stderr, "Error in write_pgm(): failed to write to file %s\n",
                path);
        return;
    }

    /* Write PAM header */
    /*
    fputs("P7\n", f);
    fprintf(f, "WIDTH %d\n", s->size);
    fprintf(f, "HEIGHT %d\n", p->size);
    fputs("DEPTH 1\n", f);
    fputs("MAXVAL 255\n", f);
    fputs("TUPLTYPE GRAYSCALE\n", f);
    fputs("ENDHDR\n", f);
    */

    /* Write PGM header */
    fprintf(f, "P5 %d %d %d\n", w, h, 255);


    /* Write image data */
    for (i=0; i<w*h; ++i) {
        buf[0] = 255 - data[i];
        fwrite(&buf, 1, 1, f);
    }
    fclose(f);
}
#endif


/**
 * Geometric matching algorithm SP1 that uses an adaptive time scale and
 * can handle errors in note timing and pitch intervals.
 *
 * @param s song to scan
 * @param p pattern that is searched for
 * @param results structure for results
 *
 * @return 1 when successful, 0 otherwise
 */
int c_geometric_sp1_scan(const song *s, const song *p, matchset *ms) {
    int   i;
    int psize = MIN2(MAX_PATTERN_SIZE, p->size);
    float max_similarity = 0.0F;
    float similarity_scale = 1.0F / ((float) psize);
    int   song_end = s->size;
    int   hash_size, hash_width;
    int   song_duration;

    char  *weight_hash;
    unsigned short *position_hash;
    float *s_x;
    int   *s_y;
    float *s_dur;
    float *p_x;
    int   *p_y;
    float *p_dur;

    vector *snotes = s->notes;
    vector *pnotes = p->notes;

#ifdef WRITE_PGM
    char path[256];
#endif

    if ((s->size < 2) || (p->size < 2)) return 0;

    for (i=1; i<s->size; ++i) {
        if (snotes[i].strt - snotes[i-1].strt > MAX_SONG_SILENCE) {
            song_end = i;
            break;
        }
    }

    song_duration = END_PADDING + snotes[song_end-1].strt;
    hash_width = song_duration / HASH_TIME_FACTOR;
    hash_size = NOTE_PITCHES * hash_width;

    weight_hash = (char *) malloc(hash_size * sizeof(char));
    position_hash = (unsigned short *) malloc(hash_size *
            sizeof(unsigned short));
    s_x = (float *) malloc(song_end * sizeof(float));
    s_y = (int *) malloc(song_end * sizeof(int));
    s_dur = (float *) malloc(song_end * sizeof(float));
    p_x = (float *) malloc(p->size * sizeof(float));
    p_y = (int *) malloc(p->size * sizeof(int));
    p_dur = (float *) malloc(p->size * sizeof(float));
    if ((weight_hash == NULL) || (position_hash == NULL) ||
            (s_x == NULL) || (s_y == NULL) || (s_dur == NULL) ||
            (p_x == NULL) || (p_y == NULL) || (p_dur == NULL)) {
        free(weight_hash);
        free(position_hash);
        free(s_x);
        free(s_y);
        free(s_dur);
        free(p_x);
        free(p_y);
        free(p_dur);
        return 0;
    }

    /** Build the hash (or actually a lookup table) */
    memset(weight_hash, 0, hash_size);
    memset(position_hash, 255, hash_size << 1);

    for (i=0; i<song_end; ++i) {
        s_x[i] = ((float) snotes[i].strt) * HASH_TIME_MULTIPLIER;
        s_y[i] = snotes[i].ptch;
        s_dur[i] = ((float) snotes[i].dur) * HASH_TIME_MULTIPLIER;
    }
    for (i=0; i<p->size; ++i) {
        p_x[i] = ((float) pnotes[i].strt) * HASH_TIME_MULTIPLIER;
        p_y[i] = pnotes[i].ptch;
        p_dur[i] = ((float) pnotes[i].dur) * HASH_TIME_MULTIPLIER;
    }

    for (i=0; i<song_end; ++i) {
        int y = snotes[i].ptch - EFFECT_CENTER_Y;
        int x = (snotes[i].strt / HASH_TIME_FACTOR) - EFFECT_CENTER_X;
        int effect_pos = 0;
        int pos = (x << HASH_SHIFT) + y;
        int j = 0;
        while ((y < 0) && (j < EFFECT_HEIGHT)) {
            ++j;
            ++y;
            effect_pos += EFFECT_WIDTH;
        }
        for (; j<EFFECT_HEIGHT; ++j, ++pos) {
            int k;
            int pos2;
            if (pos >= hash_size) break;
            pos2 = pos;
            for (k=0; k<EFFECT_WIDTH; ++k, pos2 += NOTE_PITCHES, ++effect_pos) {
                if ((pos2 < 0) || (pos2 >= hash_size)) continue;
                if (weight_hash[pos2] < EFFECT[effect_pos]) {
                    weight_hash[pos2] = EFFECT[effect_pos];
                    position_hash[pos2] = i;
                }
            }
        }
    }

#ifdef WRITE_PGM
    sprintf(path, "image-%d.pgm", s->id);
    write_pgm(path, (unsigned char *) weight_hash, 128, hash_width);
#endif

    /*fprintf(stderr, "Scanning %s\n", s->title);*/

    for (i=0; i<song_end; ++i) {
        int j;
        for (j=0; j<=MIN2(MAX_INITIAL_SKIP, p->size - 2); ++j) {
            int anchor;
            int hash_y, hash_x, hash_x_end;
            int pos;
            float pslope;
            int lastpos = USHRT_MAX;
#ifdef STEP_SCALE
            float lastscale = -1.0F;
#endif

#ifdef ANCHOR_POS
            anchor = MIN2(j + ANCHOR_POS, p->size-1);
#else
            for (anchor=j+1; anchor < p->size; ++anchor) {
                if (pnotes[anchor].strt - pnotes[j].strt > ANCHOR_DELTA) break;
            }
            if (anchor >= p->size) anchor = p->size - 1;
#endif

            pslope = p_x[anchor] - p_x[j];
            if (pslope < 1.0F) continue;
            pslope = 1.0F / pslope;

            hash_y = p_y[anchor] - p_y[j] + s_y[i];
            hash_x = (snotes[i].strt + ((pnotes[anchor].strt -
                    pnotes[j].strt) / TEMPO_FACTOR)) / HASH_TIME_FACTOR;
            hash_x_end = (snotes[i].strt + ((pnotes[anchor].strt -
                    pnotes[j].strt) * TEMPO_FACTOR)) / HASH_TIME_FACTOR;
            pos = hash_y + (hash_x << HASH_SHIFT);

            for (; hash_x <= hash_x_end; ++hash_x, pos += NOTE_PITCHES) {
                int direction;
                float similarity = 0.0F;
                float initial_scale;
                int songpos;
#ifdef ENFORCE_MAX_SKIPS
                int skips = 0;
#endif

                if (pos < 0) continue;
                else if (pos >= hash_size) break;

                if (weight_hash[pos] < TRIGGER_WEIGHT) continue;

                songpos = position_hash[pos];
                if (songpos == lastpos) continue;
                lastpos = songpos;

                initial_scale = (s_x[songpos] - s_x[i]) * pslope;
#ifdef STEP_SCALE
                if ((initial_scale > lastscale * 0.8F) &&
                        (initial_scale < lastscale * 1.2F))
                    continue;
                lastscale = initial_scale;
#endif

                for (direction=-1; direction < 2; direction += 2) {
                    int k;
                    float scale = initial_scale;
                    float last_x = s_x[i];
                    int last_y = s_y[i];
                    int last_k = j;

                    for (k=j+direction; ((k>=0) && (k<psize)); k+=direction) {
                        float dpx = (int) ((p_x[k] - p_x[last_k]) * scale);
                        int dpy = p_y[k] - p_y[last_k];
                        float x = last_x + dpx;
                        int y = last_y + dpy;
                        int hp = y + (((int) x) << HASH_SHIFT);
                        int sp;
                        float sim;

                        if ((hp < 0) || (hp >= hash_size)) continue;
#ifdef ENFORCE_MAX_SKIPS
                        if (weight_hash[hp] < TRIGGER_WEIGHT) {
                            if (skips > MAX_SKIP) break;
                            ++skips;
                        } else {
                            skips = 0;
                        }
#endif

                        sp = position_hash[hp];
                        if (sp == USHRT_MAX) continue;

                        sim = 0.0F;
#ifdef CALCULATE_WEIGHTS
                        {
                            int dy = ABS(s_y[sp] - y);
                            float dx = ABS(s_x[sp] - x);
                            float slope;
                            if (dpx < PATTERN_MIN_DELTA)
                                dpx = PATTERN_MIN_DELTA;
                            slope = 1.0F - MIN2(1.0F, ABS(dx / dpx));
                            slope = slope * slope * slope * slope;
                            sim = (0.5F / MAX2(1.0F, (float)(dy << 1))) * slope;
                        }
#else
                        sim = 0.005F * ((float) weight_hash[hp]);
#endif
#ifdef WEIGHT_DURATION
                        {
                            float pdur = scale * p_dur[k];
                            float d = s_dur[sp] - pdur;
                            if (d < 0) sim += 0.5F * (1.0F + d / pdur);
                            else sim += 0.5F * pdur / (pdur + d);
                        }
#endif
                        similarity += sim;

#ifndef COMPARE_TO_ORIGIN
                        last_k = k;
                        last_x = x;
                        last_y = y;
#endif
#ifdef ADJUST_SCALE
                        scale = 0.5F * (scale + (s_x[sp] - s_x[i]) /
                                (p_x[k] - p_x[j]));
#endif
                    }
                }
                if (similarity > max_similarity) {
                    int start = snotes[i].strt - ((float) (pnotes[j].strt -
                            pnotes[0].strt) * initial_scale);
                    int end = start + ((float) (pnotes[p->size-1].strt -
                            pnotes[0].strt) * initial_scale);
                    int transposition = snotes[i].ptch - pnotes[j].ptch;
                    max_similarity = similarity;
                    insert_match(ms, s->id, start, end, transposition,
                            similarity * similarity_scale);
                }
            }
        }
    }

    free(weight_hash);
    free(position_hash);
    free(s_x);
    free(s_y);
    free(s_dur);
    free(p_x);
    free(p_y);
    free(p_dur);

    return 1;
}

