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


#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <string.h>

#include "song.h"
#include "geometric_P3.h"
#include "song_window.h"
#include "util.h"


/**
 * Initializes a song window.
 *
 * @param w pointer to the initialized window structure.
 * @param s windowed song
 * @param relative_time If 0, the original notes are clipped by the window
 *        constraints. Otherwise note timings are adjusted to be relative to
 *        the window.
 */
void init_song_window(song_window *w, const song *s, int relative_time) {
    w->x1 = 0;
    w->x2 = 0;
    w->n1 = 0;
    w->n2 = 0;
    memset(w->notes_on, 0, 4 * sizeof(unsigned int));
    memset(w->notes_on_p, 0, 128 * sizeof(vector *));
    init_song(&w->window, 0, NULL, s->size);
    w->s = s;
    w->relative = relative_time;
}

/**
 * Frees a song window.
 */
void free_song_window(song_window *w) {
    free_song(&w->window);
    w->s = NULL;
}

/**
 * Keeps track of playing notes while moving a window edge.
 */
static void move_edge(int new_x, int old_x,
        int *pos, unsigned int *notes_on, vector **notes_on_p, const song *s) {

    int i;
    int next_pos;
    vector *notes = s->notes;

    if (new_x == old_x) return;
    else if (new_x < old_x) {
        old_x = 0;
        *pos = 0;
        memset(notes_on, 0, 4 * sizeof(unsigned int));
        memset(notes_on_p, 0, 128 * sizeof(vector *));
    }

    /* Check if any previously onset notes are still playing at the new
     * window position */
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
                    int n = (i << 5) + (j << 3) + k;
                    /* Clear finished notes */
                    if (notes_on_p[n]->strt + notes_on_p[n]->dur <= new_x) {
                        notes_on[i] &= 0xFFFFFFFF - ((1 << k) << (j << 3));
                    }
                }
            }
        }
    }

    /* Scan forward and mark notes that play at the new window edge */
    for (next_pos = *pos; next_pos < s->size; ++next_pos) {
        vector *n = &notes[next_pos];
        if (n->strt > new_x) break;
        else {
            int end = n->strt + n->dur;
            if (end > new_x) {
                /* Calculate pitch vector bit position for the note */
                int slot = n->ptch >> 5;
                int bit = n->ptch - (slot << 5);

                /* Set the corresponding bit in the vector */
                notes_on[slot] |= 1 << bit;

                notes_on_p[(int) n->ptch] = n;
            }
        }
    }
    *pos = next_pos;
}


/**
 * Adds notes that are on at the window edge.
 */
static void add_playing_notes(song *window, unsigned int *notes_on,
        vector **notes_on_p, int x1, int x2) {

    int i, p = 0;
    vector *wnotes = window->notes;
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
                    int pitch = (i << 5) + (j << 3) + k;
                    memcpy(&wnotes[p], notes_on_p[pitch], sizeof(vector));
                    ++p;
                }
            }
        }
    }
    window->size = p;
}

/**
 * Moves a window within a song. This implementation works best if the window
 * is only moved forward; otherwise the beginning of the song needs to be
 * scanned through, which slows the operation down considerably. Size of the
 * window may vary, however.
 */
void move_song_window(song_window *w, int x1, int x2) {
    int ssize = w->s->size;
    vector *notes = w->s->notes;
    vector *wnotes = w->window.notes;
    int i, j;

    if (x1 > x2) INT_SWAP(x1, x2);

    /* Move the left edge */
    move_edge(x1, w->x1, &w->n1, w->notes_on, w->notes_on_p, w->s);

    add_playing_notes(&w->window, w->notes_on, w->notes_on_p, x1, x2);

    /* Copy rest of the notes */
    j = w->window.size;
    for (i=w->n1; i<ssize; ++i, ++j) {
        vector *n = &notes[i];
        if (n->strt >= x2) break;
        memcpy(&wnotes[j], n, sizeof(vector));
    }
    w->n2 = i;
    w->window.size = j;
    w->x1 = x1; 
    w->x2 = x2; 

    /* Adjust start times and durations */
    if (w->relative) {
        for (i=0; i<j; ++i) {
            vector *n = &wnotes[i];
            int start = n->strt;
            int end = start + n->dur;
            n->strt = MAX2(start - x1, 0);
            n->dur = MIN2(end, x2) - MAX2(start, x1);
        }
    } else {
        for (i=0; i<j; ++i) {
            vector *n = &wnotes[i];
            int start = n->strt;
            int end = start + n->dur;
            n->strt = MAX2(start, x1);
            n->dur = MIN2(end, x2) - n->strt;
        }
    }
}


#ifdef ENABLE_UNIT_TESTS

void test_song_window(const song *s, int moves, int resets, int relative_time,
        int window_length) {

    int r;
    song_window sw;
    song testw;
    int x1, x2;
    int song_duration = s->notes[s->size-1].strt;

    init_song(&testw, 0, NULL, s->size);

    init_song_window(&sw, s, relative_time);

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

            /* Move the window */
            move_song_window(&sw, x1, x2);

            /* Compare to ground truth */
            for (i=0; i<testw.size; ++i) {
                vector *n = &testw.notes[i];
                if (i >= sw.window.size) {
                    fprintf(stderr, "Error in test_song_window: too few notes within the window (%d instead of %d)\n", sw.window.size, testw.size);
                    break;
                }
                if (n->strt != sw.window.notes[i].strt) {
                    fprintf(stderr, "Error in test_song_window: wrong note start time(%d instead of %d)\n", sw.window.notes[i].strt, n->strt);
                    break;
                }
                if (n->dur != sw.window.notes[i].dur) {
                    fprintf(stderr, "Error in test_song_window: wrong note duration (%d instead of %d)\n", sw.window.notes[i].dur, n->dur);
                    break;
                }
            }
            if (i < testw.size) {
                fprintf(stderr, "Window x1:%d, x2:%d\n", x1, x2);
                fputs("GT:     ", stderr);
                for (i=0; i<testw.size; ++i) {
                    vector *n = &testw.notes[i];
                    fprintf(stderr, "(%d, %d, %d) ", n->strt, n->dur, n->ptch);
                }
                fputs("\nWindow: ", stderr);
                for (i=0; i<sw.window.size; ++i) {
                    vector *n = &sw.window.notes[i];
                    fprintf(stderr, "(%d, %d, %d) ", n->strt, n->dur, n->ptch);
                }
                fputs("\n\n", stderr);

            }
            x1 += (int) (randf() * 2.0F * (float) window_length);
            x2 += (int) (randf() * 2.0F * (float) window_length);
        }
    }

    free_song(&testw);
    free_song_window(&sw);
}

#endif /* ENABLE_UNIT_TESTS */

