/*
 * filter_P2.h - External function declarations for P2-based vector index
 *               filters.
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


#ifndef __FILTER_P2_H__
#define __FILTER_P2_H__

#include "results.h"
#include "search.h"
#include "song.h"
#include "vindex.h"

#ifdef __cplusplus
extern "C" {
#endif


/**
 * A helper structure used by some index-accessing methods.
 */
typedef struct {
    /*int patternpos;
    int indexpos;
    int indexsize;
    indexrec *rec;*/
    indexvector *iv;
    int patternpos;
    int shift;
    int t;
} patternvector;


void filter_p2_greedy_pigeonhole(const songcollection *sc, const song *pattern,
        int alg, const searchparameters *parameters, matchset *ms);

#ifdef ENABLE_BLOSSOM4
void filter_p2_pigeonhole(const songcollection *sc, const song *pattern,
        int alg, const searchparameters *parameters, matchset *ms);
#endif

void filter_p2_window(const songcollection *sc, const song *pattern,
        int alg, const searchparameters *parameters, matchset *ms);

void filter_p2_select_local(const songcollection *sc, const song *pattern,
        int alg, const searchparameters *parameters, matchset *ms);

void filter_p2_select_global(const songcollection *sc, const song *pattern,
        int alg, const searchparameters *parameters, matchset *ms);

void filter_p2_points(const songcollection *sc, const song *pattern,
        int alg, const searchparameters *parameters, matchset *ms);

#ifdef __cplusplus
}
#endif


#endif

