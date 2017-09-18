/*
 * song_window.c - Implements a sliding window within a song.
 *
 * Version 2008-04-20
 *
 *
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
#include <assert.h>

#include "song.h"
#include "geometric_P3.h"
#include "song_window_P3.h"
#include "util.h"


/**
 * Initializes a P3 song window.
 */
void init_p3s_window(p3s_window *w, const p3song *p3s) {
    w->x1 = 0;
    w->x2 = 0;
    w->s1 = 0;
    w->s2 = 0;
    w->e1 = 0;
    w->e2 = 0;
    memset(w->notes_on_1, 0, 4 * sizeof(unsigned int));
    memset(w->notes_on_2, 0, 4 * sizeof(unsigned int));
    w->p3s = p3s;
    w->songend = p3s->endpoints[p3s->size-1].x;
    w->startpoints = (TurningPoint *) malloc(p3s->size * sizeof(TurningPoint));
    w->endpoints = (TurningPoint *) malloc(p3s->size * sizeof(TurningPoint));
    w->window.startpoints = w->startpoints;
    w->window.endpoints = w->endpoints;
    w->window.song = p3s->song;
    w->window.size = 0;
    init_bitcount();
}


void free_p3s_window(p3s_window *w) {
    free(w->startpoints);
    free(w->endpoints);
    free_p3_song(&w->window);
    w->startpoints = NULL;
    w->endpoints = NULL;
    w->window.startpoints = NULL;
    w->window.endpoints = NULL;
    w->window.song = NULL;
    w->window.size = 0;
    w->p3s = NULL;
}


/**
 * Selects a known position that is closest to the new position.
 */
static void move_p3s_window_edge_init(const p3s_window *w,
        int new_x, int *x, int *s, int *e, unsigned int *notes_on) {

    int delta = ABS(new_x - *x);

    if (new_x < ((w->x1 + w->x2) >> 1)) {
        if (new_x < (w->x1 >> 1)) {
            /* Return if the edge is already at the closest position */
            if (delta <= new_x) return;
            *x = 0;
            memset(notes_on, 0, 4 * sizeof(unsigned int));
            *s = 0;
            *e = 0;
        } else {
            if (delta <= ABS(new_x - w->x1)) return;
            *x = w->x1;
            memcpy(notes_on, w->notes_on_1, 4 * sizeof(unsigned int));
            *s = w->s1;
            *e = w->e1;
        }
    } else {
        if (new_x > ((w->x2 + w->songend) >> 1)) {
            if (delta <= ABS(new_x - w->songend)) return;
            *x = w->songend;
            memset(notes_on, 0, 4 * sizeof(unsigned int));
            *s = w->p3s->size;
            *e = w->p3s->size;
        } else {
            if (delta <= ABS(new_x - w->x1)) return;
            *x = w->x2;
            memcpy(notes_on, w->notes_on_2, 4 * sizeof(unsigned int));
            *s = w->s2;
            *e = w->e2;
        }
    }
}


/**
 * Keeps track of playing notes while moving a window edge.
 */
static void move_p3s_window_edge(int new_x, int old_x,
        int *spos, int *epos, unsigned int *notes_on, int num_turningpoints,
        TurningPoint *startpoints, TurningPoint *endpoints) {

    int next_spos = *spos;
    int next_epos = *epos;

    if (new_x > old_x) {
        /* Scan forward */
        while(1) {
            int start = 1;
            int cur_x;
            char cur_y;
            int slot, bit;

            if (next_spos < num_turningpoints) {
                cur_x = startpoints[next_spos].x;
                if ((next_epos < num_turningpoints) &&
                        (cur_x >= endpoints[next_epos].x)) {
                    start = 0;
                    cur_x = endpoints[next_epos].x;
                    cur_y = endpoints[next_epos].y;
                } else {
                    cur_y = startpoints[next_spos].y;
                }
            } else if (next_epos < num_turningpoints) {
                start = 0;
                cur_x = endpoints[next_epos].x;
                cur_y = endpoints[next_epos].y;
            } else break;
            if (cur_x > new_x) break;

            /* Calculate pitch vector bit position for the note */
            slot = cur_y >> 5;
            bit = cur_y - (slot << 5);

            if (start) {
                /* Set the corresponding bit in the vector */
                notes_on[slot] |= 1 << bit;
                next_spos++;
            } else {
                /* Clear the corresponding bit in the vector */
                notes_on[slot] &= 0xFFFFFFFF - (1 << bit);
                next_epos++;
            }
        }
    } else if (new_x < old_x) {
        /* Scan backward */
        next_epos--;
        next_spos--;
        while(1) {
            int end = 1;
            int cur_x;
            char cur_y;
            int slot, bit;

            if (next_epos >= 0) {
                cur_x = endpoints[next_epos].x;
                if ((next_spos >= 0) && (cur_x < startpoints[next_spos].x)) {
                    end = 0;
                    cur_x = startpoints[next_spos].x;
                    cur_y = startpoints[next_spos].y;
                } else {
                    cur_y = endpoints[next_epos].y;
                }
            } else if (next_spos >= 0) {
                end = 0;
                cur_x = startpoints[next_spos].x;
                cur_y = startpoints[next_spos].y;
            } else break;
            if (cur_x < new_x) break;

            /* Calculate pitch vector bit position for the note */
            slot = cur_y >> 5;
            bit = cur_y - (slot << 5);

            if (end) {
                /* Set the corresponding bit in the vector */
                notes_on[slot] |= 1 << bit;
                next_spos--;
            } else {
                /* Clear the corresponding bit in the vector */
                notes_on[slot] &= 0xFFFFFFFF - (1 << bit);
                next_epos--;
            }
        }
        next_spos++;
        next_epos++;
    }
    *spos = next_spos;
    *epos = next_epos;
}


/**
 * Adds turningpoints for notes that are on at the window edge.
 */
static void move_p3s_window_mark_playing_notes(unsigned int *notes_on,
        int time, int num_turningpoints, TurningPoint *tp,
        int *pos, int start) {

    int i;
    int p = *pos;
    int num_points = bitcount32(notes_on[0]) + bitcount32(notes_on[1]) +
            bitcount32(notes_on[2]) + bitcount32(notes_on[3]);
    if (!num_points) return;
    if (start) {
        p -= num_points;
        assert(p >= 0);
        *pos = p;
    } else {
        assert(p + num_points <= num_turningpoints);
        *pos = p + num_points;
    }
    for (i=0; i<4; ++i) {
        int j;
        unsigned int on_32 = notes_on[i];
        if (!on_32) continue;
        for (j=0; j<4; ++j) {
            int k;
            unsigned char on_8 = on_32 & 0xFF;
            on_32 >>= 8;
            if (!on_8) continue;
            for (k=0; k<8; ++k) {
                unsigned char on_1 = on_8 & 0x1;
                on_8 >>= 1;
                if (on_1) {
                    tp[p].x = time;
                    tp[p].y = (i << 5) + (j << 3) + k;
                    ++p;
                }
            }
        }
    }
}

/**
 * Moves a window within P3 song.
 */
void move_p3s_window(p3s_window *w, int x1, int x2) {
    const p3song *s = w->p3s;
    TurningPoint *sp = s->startpoints;
    TurningPoint *ep = s->endpoints;
    int ssize = s->size;
    int i, j;

    unsigned int notes_on_1[4], notes_on_2[4];
    int old_x1 = 0, s1 = 0, e1 = 0;
    int old_x2, s2, e2;

    if (x1 > x2) INT_SWAP(x1, x2);

    /* Move the left edge */
    memset(notes_on_1, 0, 4 * sizeof(unsigned int));
    move_p3s_window_edge_init(w, x1, &old_x1, &s1, &e1, notes_on_1);
    move_p3s_window_edge(x1, old_x1, &s1, &e1, notes_on_1, ssize, sp, ep);

    /* Move the right edge */
    old_x2 = old_x1;
    s2 = s1;
    e2 = e1;
    memcpy(notes_on_2, w->notes_on_1, 4 * sizeof(unsigned int));
    move_p3s_window_edge_init(w, x2, &old_x2, &s2, &e2, notes_on_2);
    move_p3s_window_edge(x2, old_x2, &s2, &e2, notes_on_2, ssize, sp, ep);

    memcpy(w->notes_on_1, notes_on_1, 4 * sizeof(unsigned int));
    memcpy(w->notes_on_2, notes_on_2, 4 * sizeof(unsigned int));

    /* Copy turning points that are not yet in place */

    /* Startpoints */
    if (s1 < w->s1) {
        i = s1;
        j = MIN2(w->s1, s2);
        memcpy(&w->startpoints[i], &sp[i], (j - i) * sizeof(TurningPoint));
    }
    if (s2 > w->s2) {
        i = MAX2(w->s2, s1);
        j = s2;
        memcpy(&w->startpoints[i], &sp[i], (j - i) * sizeof(TurningPoint));
    }

    /* Endpoints */
    if (e1 < w->e1) {
        i = e1;
        j = MIN2(w->e1, e2);
        memcpy(&w->endpoints[i], &ep[i], (j - i) * sizeof(TurningPoint));
    }
    if (e2 > w->e2) {
        i = MAX2(w->e2, e1);
        j = e2;
        memcpy(&w->endpoints[i], &ep[i], (j - i) * sizeof(TurningPoint));
    }

    i = s1;
    j = e2;
    move_p3s_window_mark_playing_notes(notes_on_1, x1, ssize, w->startpoints,
            &i, 1);
    move_p3s_window_mark_playing_notes(notes_on_2, x2, ssize, w->endpoints,
            &j, 0);

    w->s1 = s1;
    w->e1 = e1;
    w->s2 = s2;
    w->e2 = e2;
    w->x1 = x1;
    w->x2 = x2;
    w->window.startpoints = &w->startpoints[i];
    w->window.endpoints = &w->endpoints[e1];
    i = s2 - i;
    assert(i == j - e1);
    w->window.size = i;
}


#ifdef ENABLE_UNIT_TESTS

void test_p3s_window(const song *s, int moves, int resets, int relative_time,
        int window_length) {

    int r;
    p3song p3s;
    p3s_window sw;
    song testw;
    p3song testw_p3s;
    int x1, x2;
    int song_duration = s->endpoints[s->size-1].x;

    song_to_p3(s, &p3s);

    init_song(&testw, 0, NULL, s->size);

    init_p3s_window(&sw, s, relative_time);

    for (r=0; r<resets; ++r) {
        int m;
        fputs("------- Window reset -------\n", stderr);
        x1 = (int) (randf() * 0.5F * (float) song_duration);
        x2 = x1 + (int) (randf() * (float) window_length);
        for (m=0; m<moves; ++m) {
            /* Scan in a slower way to produce a ground truth */
            int i;
            testw.size = 0;
            if (x1 > x2) INT_SWAP(x1, x2);
            for (i=0; i<s->size; ++i) {
                vector *n = &s->notes[i];
                int start = n->strt;
                int end = start + n->dur;
                if ((start < x2) && (end > x1)) {
                    vector *tn = &testw.notes[testw.size];
                    memcpy(tn, n, sizeof(vector));
                    tn->strt = MAX2(start, x1);
                    if (relative_time) {
                        tn->strt -= x1;
                    }
                    tn->dur = MIN2(end, x2) - MAX2(start, x1);
                    ++testw.size;
                }
            }
            lexicographic_sort(&testw);
            song_to_p3(&testw, &testw_p3s);

            /* Move the window */
            move_song_window(&sw, x1, x2);

            /* Compare to ground truth */
            for (i=0; i<testw_p3s.size; ++i) {
                TurningPoint *sp = &testw_p3s.startpoints[i];
                if (i >= sw.window.size) {
                    fprintf(stderr, "Error in test_p3s_window: too few notes within the window (%d instead of %d)\n", sw.window.size, testw_p3s.size);
                    break;
                }
                if (sp.x != sw.window.startpoints[i].x) {
                    fprintf(stderr, "Error in test_song_window: wrong note start time(%d instead of %d)\n", sw.window.startpoints[i].x, sp.x);
                    break;
                }
                if (sp.y != sw.window.startpoints[i].y) {
                    fprintf(stderr, "Error in test_song_window: wrong note pitch (%d instead of %d)\n", sw.window.startpoints[i].y, sp.y);
                    break;
                }
            }
            if (i < testw_p3s.size) {
                fprintf(stderr, "Window x1:%d, x2:%d\n", x1, x2);
                fputs("GT:     ", stderr);
                for (i=0; i<testw_p3s.size; ++i) {
                    TurningPoint *sp = &testw_p3s.startpoints[i];
                    fprintf(stderr, "(%d, %d) ", sp.x, sp.y);
                }
                fputs("\nWindow: ", stderr);
                for (i=0; i<sw.window.size; ++i) {
                    TurningPoint *sp = &sw.window.startpoints[i];
                    fprintf(stderr, "(%d, %d) ", sp.x, sp.y);
                }
                fputs("\n\n", stderr);

            }
            x1 += (int) (randf() * 2.0F * (float) window_length);
            x2 += (int) (randf() * 2.0F * (float) window_length);
        }
    }

    free_p3_song(&testw);
    free_p3s_window(&sw);
}

#endif /* ENABLE_UNIT_TESTS */

