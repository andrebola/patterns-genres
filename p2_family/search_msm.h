/*
 * search_msm.h - Constants and external declarations for the MSM search
 *                functions.
 *
 * Version 2007-09-07
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


#ifndef __SEARCH_MSM_H__
#define __SEARCH_MSM_H__

#include "config.h"
#include "results.h"
#include "search.h"
#include "song.h"

#ifdef __cplusplus
extern "C" {
#endif


/* External function declarations */

void alg_msm(const songcollection *sc, const song *pattern, int alg,
        const searchparameters *parameters, matchset *ms);

void *init_msm_song_collection(void);
int build_msm_song_collection(void *msm_sc, const songcollection *sc,
	const dataparameters *dp);
void clear_msm_song_collection(void *msm_sc);
void free_msm_song_collection(void *msm_sc);

#ifdef __cplusplus
}
#endif

#endif

