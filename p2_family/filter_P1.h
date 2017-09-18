/*
 * filter_P1.h - External function declarations for P1-based vector index
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


#ifndef __FILTER_P1_H__
#define __FILTER_P1_H__

#include "results.h"
#include "search.h"
#include "song.h"

#ifdef __cplusplus
extern "C" {
#endif


void filter_p1_random(const songcollection *sc, const song *pattern,
        int alg, const searchparameters *parameters, matchset *ms);

void filter_p1_select_1(const songcollection *sc, const song *pattern,
        int alg, const searchparameters *parameters, matchset *ms);

void filter_p1_select_2(const songcollection *sc, const song *pattern,
        int alg, const searchparameters *parameters, matchset *ms);

void filter_p1_sample(const songcollection *sc, const song *pattern,
        int alg, const searchparameters *parameters, matchset *ms);

#ifdef __cplusplus
}
#endif


#endif

