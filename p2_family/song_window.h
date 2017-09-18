/*
 * song_window.h - External function declarations for song window.
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


#ifndef __SONG_WINDOW_H__
#define __SONG_WINDOW_H__

#include "config.h"
#include "song.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Song window data structure.
 */
typedef struct {
    int x1, x2;
    int n1, n2;
    unsigned int notes_on[4];
    vector *notes_on_p[128];
    int relative;
    const song *s;
    song window;
} song_window;


void init_song_window(song_window *w, const song *s, int relative_time);

void free_song_window(song_window *w);

void move_song_window(song_window *w, int x1, int x2);


#ifdef ENABLE_UNIT_TESTS

void test_song_window(const song *s, int moves, int resets, int relative_time,
        int window_length);

#endif


#ifdef __cplusplus
}
#endif

#endif

