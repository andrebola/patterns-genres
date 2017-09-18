/*
 * results.h - Search result structures and external declarations.
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

#ifndef __RESULTS_H__
#define __RESULTS_H__

#include "config.h"

#ifdef __cplusplus
extern "C" {
#endif


/**
 * Information about a single matching position in a song.
 */
typedef struct {
    /* Song id in the song collection */
    int song;

    /* Match start time */
    int start;

    /* Match end time. This can be the end time of the last matching
       note, so that the algorithm does not need to check the end times
       of all the matching notes */
    int end;

    /* Transposition of the match compared to the pattern */
    char transposition;

    /* Number of notes in the pattern */
    int num_notes;

    /* Note positions that match the pattern notes. -1 means no match. */
    int *notes;

    /* Similarity score */
    float similarity;
} match;


/**
 * Search time allocation.
 */
typedef struct {
   int measure;
   double indexing;
   double verifying;
   double other;
} searchtime;


/**
 * A set of matches.
 */
typedef struct {
    /* Flag for setting how multiple matches in a song should be stored:
       1: the set can contain many matches from the same song, interleaved
          with other matches
       0: only the best match of each song is stored. */
    int multiple_matches_per_song;

    /* Capacity of the match set */
    int size;

    /* Number of items the set actually holds */
    int num_matches;

    /* Items */
    match *matches;

    /* Search time */
    searchtime time;
} matchset;


/* External function declarations */


void free_match(match *m);

void free_match_set(matchset *ms);

int init_match_set(matchset *ms, int size, int pattern_size,
        int multiple_matches_per_song);

void clear_match_set(matchset *ms);

match *insert_match(matchset *ms, int song, int start, int end,
        char transposition, float similarity);


#ifdef __cplusplus
}
#endif

#endif

