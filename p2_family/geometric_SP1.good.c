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

#define COMPARE_TO_ORIGIN 1

#define ADJUST_SCALE 1

#define CALCULATE_WEIGHTS 1

#define WEIGHT_DURATION 1

#define ENFORCE_MAX_SKIPS 1

#define MAX_PATTERN_SIZE 10000

#define STEP_SCALE 1

#define TEMPO_FACTOR 2
#define PATTERN_ANCHOR_POS 3
/*#define MAX_SKIP 9*/
#define MAX_INITIAL_SKIP 2

#define MAX_SKIP 5

#define HASH_SHIFT 7


#define MAX_SONG_SILENCE 60000


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


#define HASH_TIME_FACTOR 5

#define TRIGGER_WEIGHT 60

#define EFFECT_HEIGHT 3
#define EFFECT_WIDTH 19
#define EFFECT_CENTER_X 9
#define EFFECT_CENTER_Y 1

static char EFFECT[] = {
  0,  5, 10, 13, 20, 25, 40, 50, 65,  75, 65, 50, 40, 25, 20, 13, 10,  5,  0,
 11, 13, 20, 25, 40, 50, 65, 75, 90, 100, 90, 75, 65, 50, 40, 25, 20, 13, 11,
  0,  5, 10, 13, 20, 25, 40, 50, 65,  75, 65, 50, 40, 25, 20, 13, 10,  5,  0
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
    int i;
    int max_similarity = 0;
    float similarity_scale = 0.01F /
            (float) (MIN2(MAX_PATTERN_SIZE, p->size) - 2);
    int song_end = s->size-1, song_duration, hash_size, hash_width;
    char *weight_hash;
    unsigned short *position_hash;
    vector *snotes = s->notes;
    vector *pnotes = p->notes;
#ifdef WRITE_PGM
    char path[256];
#endif

    if ((s->size < 2) || (p->size < 2)) return 0;

    for (i=0; i<s->size; ++i) {
        if ((i+1 < s->size) && (snotes[i+1].strt - snotes[i].strt >
                MAX_SONG_SILENCE)) {
            song_end = i;
            break;
        }
    }

    song_duration = 1000 + snotes[song_end].strt;
    hash_width = song_duration >> HASH_TIME_FACTOR;
    hash_size = NOTE_PITCHES * hash_width;

    weight_hash = (char *) malloc(hash_size * sizeof(char));
    position_hash = (unsigned short *) malloc(hash_size *
            sizeof(unsigned short));
    if ((weight_hash == NULL) || (position_hash == NULL)) {
        free(weight_hash);
        free(position_hash);
        return 0;
    }

    /** Build the hash (or actually a lookup table) */
    memset(weight_hash, 0, hash_size);
    memset(position_hash, 255, hash_size << 1);

    /*for (i=0; i<hash_size; ++i) {
        weight_hash[i] = 0;
        position_hash[i] = USHRT_MAX;
    }*/

    for (i=0; i<=song_end; ++i) {
        int y = snotes[i].ptch;
        int x = snotes[i].strt >> HASH_TIME_FACTOR;
        /* int pos = y + (x << HASH_SHIFT); */
        int effect_pos = -1;
        int j;
        for (j=0; j<EFFECT_HEIGHT; ++j) {
            int k;
            int y2 = y + j - EFFECT_CENTER_Y;
            if ((y2 < 0) || (y2 >= NOTE_PITCHES)) {
                effect_pos += EFFECT_WIDTH;
                continue;
            }
            for (k=0; k<EFFECT_WIDTH; ++k) {
                int x2 = x + k - EFFECT_CENTER_X;
                int pos2;
                effect_pos++;
                if ((x2 < 0) || (x2 >= hash_width)) continue;
                pos2 = y2 + (x2 << HASH_SHIFT);
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

    for (i=0; i<=song_end; ++i) {
        int j;
        for (j=0; j<=MIN2(MAX_INITIAL_SKIP, p->size - 2); ++j) {
            int ypos = pnotes[MIN2(j + PATTERN_ANCHOR_POS, p->size - 1)].ptch -
                    pnotes[j].ptch + snotes[i].ptch;
            int px = (pnotes[MIN2(j + PATTERN_ANCHOR_POS, p->size - 1)].strt -
                    pnotes[j].strt);
            int xstart = (snotes[i].strt + (px >> TEMPO_FACTOR)) >>
                    HASH_TIME_FACTOR;
            int xend = (snotes[i].strt + (px << TEMPO_FACTOR)) >>
                    HASH_TIME_FACTOR;
            int xpos;
            int lastpos = USHRT_MAX;
            float pslope;
#ifdef STEP_SCALE
            float lastscale = -1.0F;
#endif

            if (px == 0) continue;
            pslope = 1.0F / (float) px;

            for (xpos = xstart; xpos <= xend; ++xpos) {
                int k;
                int similarity = 0;
                float scale;
                int pos = ypos + (xpos << HASH_SHIFT);
#ifdef ENFORCE_MAX_SKIPS
                int skips = 0;
#endif
                int last_i = i;
                int last_k = j;

                if ((pos < 0) || (pos > hash_size)) continue;
                if (weight_hash[pos] < TRIGGER_WEIGHT) continue;

                pos = position_hash[pos];
                if (pos == lastpos) continue;
                lastpos = pos;

#ifdef STEP_SCALE
                scale = ((float) (snotes[pos].strt - snotes[i].strt)) * pslope;
                if ((scale > lastscale * 0.8F) && (scale < lastscale * 1.2F))
                    continue;
                lastscale = scale;
#endif

                for (k=j+1; k < MIN2(p->size, MAX_PATTERN_SIZE); ++k) {
                    int px2, py2, pos2, songpos, hash_px2;
                    /*if (k == j + PATTERN_ANCHOR_POS) continue;*/
                    py2 = pnotes[k].ptch - pnotes[last_k].ptch;
                    px2 = (int) (((float) (pnotes[k].strt -
                            pnotes[last_k].strt)) * scale);
                    hash_px2 = (px2 + snotes[last_i].strt) >> HASH_TIME_FACTOR;
                    pos2 = py2 + snotes[last_i].ptch + (hash_px2 << HASH_SHIFT);
                    if ((pos2 < 0) || (pos2 > hash_size)) continue;

                    songpos = position_hash[pos2];

                    if (weight_hash[pos2] > 0) {
                        float ss = 0.0F;
#ifdef WEIGHT_DURATION
                        float ddiff, sdur;
#endif 
#ifdef CALCULATE_WEIGHTS
                        int dy = ABS(snotes[songpos].ptch -
                                (py2 + snotes[last_i].ptch));
                        int dx = ABS(snotes[songpos].strt -
                                (px2 + snotes[last_i].strt));
                        float slope;
                        if (px2 < 100) px2 = 100;
                        if (dy < 3) {
                            slope = 1.0F - MIN2(1.0F, (float) dx / (float) px2);
                            slope = slope * slope * slope * slope;
                            ss = (1.0F / MAX2(1.0F, ((float) (dy << 1)))) *
                                    slope;
                            ss = 50.0F * ss;
/*                        if (ss > 50.0F) fprintf(stderr,"%f %f\n", ss, ABS((float) (snotes[songpos].strt - (px2 + snotes[last_i].strt)) / (float) px2));*/
/*                        fprintf(stderr,"%d\t%d\t%d\t%f\t%f\n", dy, dx, weight_hash[pos2], ss, (float)dx/(float)px2);*/
                        }
#else
                        ss = 0.5F * ((float) weight_hash[pos2]);
#endif
#ifdef WEIGHT_DURATION
                        sdur = scale * ((float) pnotes[k].dur);
                        ddiff = ((float) snotes[songpos].dur) - sdur;
                        if (ddiff < 0) {
                            ss += 50.0F * (1.0F + ddiff / sdur);
                        } else {
                            ss += 50.0F * sdur / (sdur + ddiff);
                        }
#endif
                        similarity += ss;

#ifndef COMPARE_TO_ORIGIN
                        last_i = songpos;
                        last_k = k;
#endif

#ifdef ADJUST_SCALE
                        /*fprintf(stderr, "Old scale: %f  ", scale);*/
                        scale = 0.50F * (scale +
                            ((float) (snotes[songpos].strt -
                            snotes[i].strt)) /
                            (float) (pnotes[k].strt -
                            pnotes[j].strt));
                        /*fprintf(stderr, "New scale: %f\n", scale);*/
#endif

                    }

#ifdef ENFORCE_MAX_SKIPS
                    if (weight_hash[pos2] < TRIGGER_WEIGHT) {
                        if (skips >= MAX_SKIP) break;
                        ++skips;
                    } else {
                        skips = 0;
                    }
#endif
                }

                if (similarity > max_similarity) {
                    int start = snotes[i].strt - ((float) (pnotes[j].strt -
                            pnotes[0].strt) * scale);
                    int end = start + ((float) (pnotes[p->size-1].strt -
                            pnotes[0].strt) * scale);
                    int transposition = snotes[i].ptch - pnotes[j].ptch;
                    max_similarity = similarity;
                    insert_match(ms, s->id, start, end, transposition,
                            ((float) similarity) * similarity_scale);
                }
            }
        }
    }

    free(weight_hash);
    free(position_hash);

    return 1;
}

