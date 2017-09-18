/*
 * geometric_P3.h - Structures and external function declarations for
 *                  Geometric algorithm P3
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


#ifndef __GEOMETRIC_P3_H__
#define __GEOMETRIC_P3_H__

#include "config.h"
#include "search.h"
#include "song.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Song duration limit caused by the priority queue key coding used in this
 * implementation */
#define P3_TIME_LIMIT (1 << 23)


/**
 * A structure for items in the vertical translation table. It stores value,
 * slope and previous x for each vertical translation (y).
 */
typedef struct {
    int slope;
    int value;
    int prev_x;
} VerticalTranslationTableItem;


/**
 * Startpoints (strt,ptch) and endpoints (strt+dur,ptch) are called
 * turning points. They are precalculated and stored in separate arrays of 
 * this struct. This adds to space requirements but makes the algorithm
 * simpler. (startpoints could be traversed from source data but endpoint
 * order must be stored somehow anyway.)
 */
typedef struct {
    int x;
    int y;
} TurningPoint;


/**
 * A translation vector structure. Four of these are allocated for each
 * note in the pattern and the vectors are updated when scanning through
 * the song.
 */
typedef struct {
    /* Index to turning point array; which turning point is associated
       with this translation vector. */
    int tpindex;
    int patternindex;
    int x;
    int y;
    char text_is_start;
    char pattern_is_start;
} TranslationVector;


/**
 * Structure for storing a song in P3 format.
 */
typedef struct {
    const song *song;
    int size;
    TurningPoint *startpoints;
    TurningPoint *endpoints;
} p3song;


/**
 * Structure for storing a song collection in P3 format.
 */
typedef struct {
    int size;
    p3song *p3_songs;
    const songcollection *song_collection;
} p3songcollection;


/* External function declarations */


void alg_p3(const songcollection *sc, const song *pattern, int alg,
        const searchparameters *parameters, matchset *ms);

int scan_song_p3(const song *s, const song *p, matchset *ms);

int scan_p3(const p3song *p3s, const song *pattern,
        const searchparameters *parameters, matchset *ms);

int compare_turningpoints(const void *aa, const void *bb);


void *init_p3_song_collection(void);
int build_p3_song_collection(void *p3_songcollection, const songcollection *sc,
	const dataparameters *dp);
void clear_p3_song_collection(void *p3_songcollection);
void free_p3_song_collection(void *p3_songcollection);

void init_p3_song(p3song *p3s);
void free_p3_song(p3song *p3s);
void song_to_p3(const song *s, p3song *p3s);


#ifdef __cplusplus
}
#endif

#endif

