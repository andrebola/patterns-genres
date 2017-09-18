/*
 * song_window_P3.h - External function declarations for P3 song window.
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


#ifndef __SONG_WINDOW_P3_H__
#define __SONG_WINDOW_P3_H__

#include "config.h"
#include "song.h"
#include "geometric_P3.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * P3 song window data structure.
 */
typedef struct {
    int x1, x2;
    int s1, s2;
    int e1, e2;
    unsigned int notes_on_1[4];
    unsigned int notes_on_2[4];
    const p3song *p3s;
    int songend;
    p3song window;
    TurningPoint *startpoints;
    TurningPoint *endpoints;
} p3s_window;


void init_p3s_window(p3s_window *w, const p3song *p3s);

void free_p3s_window(p3s_window *w);

void move_p3s_window(p3s_window *w, int x1, int x2);


#ifdef __cplusplus
}
#endif

#endif

