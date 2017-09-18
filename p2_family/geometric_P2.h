/*
 * geometric_P2.h - External function declarations for Geometric algorithm P2
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


#ifndef __GEOMETRIC_P2_H__
#define __GEOMETRIC_P2_H__

#include "search.h"
#include "song.h"


#ifdef __cplusplus
extern "C" {
#endif


void alg_p2(const songcollection *sc, const song *pattern, int alg,
        const searchparameters *parameters, matchset *ms);

void alg_p2_points(const songcollection *sc, const song *pattern, int alg,
        const searchparameters *parameters, matchset *ms);

int scan_song_p2(const song *s, const song *p, const int errors,
        matchset *ms);

song *p2_compensate_quantization(const song *p, const int q);

match *alignment_check_p2(const song *s, unsigned short songpos,
        const song *p, unsigned short patternpos, matchset *ms);

int scan_song_p2_points(const song *s, const song *p,
        int num_points, const int *points, matchset *ms);

#ifdef __cplusplus
}
#endif


#endif

