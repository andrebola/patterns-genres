/*
 * search.h - Constants and external declarations for the search functions.
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


#ifndef __SEARCH_H__
#define __SEARCH_H__

#include "config.h"
#include "song.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Structure for search parameters
 */
typedef struct {
    int quantization;
    int c_window;
    int d_window;
    int p1_sample_size;
    int p2_window;
    float p2_select_threshold;
    int p2_num_points;
    int *p2_points;

    /* Calculate the overall length of song notes that do _not_ overlap with
     * the pattern at the position of maximal overlapping. This is used as an
     * additional similarity factor and can be useful when comparing whole
     * songs or polyphonic transcriptions. */
    int p3_calculate_difference;

    int p3_remove_gaps;
 
    int msm_r;
    int measure_time_allocation;
    int sync_accuracy;
    int syncmap_accuracy;
    int sync_window_size;
} searchparameters;


void search(const songcollection *sc, const song *pattern, int alg,
        const searchparameters *parameters, matchset *ms);

void print_results(const matchset *ms, const songcollection *sc);


#ifdef __cplusplus
}
#endif

#endif

