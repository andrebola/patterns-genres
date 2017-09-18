/*
 * gh.h - Structures and external declarations for geometric hashing.
 *
 * Version 2007-10-25
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


#ifndef __GH_H__
#define __GH_H__

#include "config.h"
#include "song.h"

#ifdef __cplusplus
extern "C" {
#endif


/**
 * Song record within a geometric hash.
 */
typedef struct {
    unsigned short **data;
    unsigned short song;
    int positions;
    int words;
} hashrecord;


/**
 * Geometric hash.
 */
typedef struct {
    songcollection *sc;
    int window;
    int w, h;
    int size;
    hashrecord *records;
} geometrichash;



/* External function declarations. */


void build_geometric_hash(geometrichash *gh, const songcollection *sc,
        int window, int w, int h);

void free_geometric_hash(geometrichash *gh);

void search(const vectorindex *vindex, const song *pattern, int window,
        matchset *ms);


#ifdef __cplusplus
}
#endif

#endif

