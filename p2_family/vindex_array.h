/*
 * vindex_array.h - Structures and external declarations for the index
 *                  functions of vindex_array.
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


#ifndef __VINDEX_ARRAY_H__
#define __VINDEX_ARRAY_H__

#include "config.h"
#include "data.h"
#include "song.h"

#ifdef __cplusplus
extern "C" {
#endif


/**
 * Index record (a single position of a vector).
 */
typedef struct {
    unsigned short song;
    unsigned short note;
} indexrec;

/**
 * Indexed vector (contains multiple position records).
 */
typedef struct {
    int size;
    /* Variable-length buffer for records. This is called "flexible array
       member" in C99. */
    indexrec records[];
} indexvector;


/**
 * Vector index structure.
 */
typedef struct {
    /* Number of index vectors. In this implementation this is
     * the same as width * height. */
    int size;
    /* Maximum vector width */
    int width;
    /* Maximum vector height */
    int height;
    /* Maximum number of consecutive notes the algorithm is allowed to skip when
     * building an index for a song collection */
    int c_window;
    /* A continuous buffer of index records for all the vectors */
    char *buffer;
    /* Pointers to indexed vectors within the buffer */
    indexvector **vectors;
    /* The indexed song collection */
    const songcollection *scollection;
    /* Index memory usage in bytes */
    int memory_usage;
} vectorindex;



/* Global inline functions */


/**
 * Returns the position of the specified vector in the index structure.
 *
 * @param vindex the index structure
 * @param x vector's x component
 * @param y vector's y component
 *
 * @return position of the vector
 */
static INLINE indexvector *get_index_vector(const vectorindex *vindex,
        int x, int y) {
    y += vindex->height >> 1;
    if ((y < 0) || (x < 0) || (y >= vindex->height) || (x >= vindex->width))
        return NULL;
    return vindex->vectors[y * vindex->width + x];
}

/* External function declarations. */


void *init_vectorindex(void);

int build_vectorindex(void *index, const songcollection *sc,
	const dataparameters *dp);

void clear_vectorindex(void *index);

void free_vectorindex(void *index);


#ifdef __cplusplus
}
#endif

#endif

