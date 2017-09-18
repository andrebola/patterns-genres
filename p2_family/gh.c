/*
 * gh.c - Geometric hash implementation for polyphonic music retrieval.
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


#include <stdio.h>
#include <stdlib.h>
/*#include <limits.h>
#include <math.h>
#include <string.h>*/

/*#include "geometric_P1.h"
#include "geometric_P2.h"
#include "geometric_P3.h"
#include "priority_queue.h"*/

#include "config.h"
#include "algorithms.h"
#include "util.h"
#include "gh.h"

static int BITCOUNT_TABLE_INITIALIZED = 0;
static char BITCOUNT_TABLE[65536];


/**
 * Initializes a lookup table for counting the number of bits in 16-bit values.
 */
static void init_bitcount_table() {
    int i;
    BITCOUNT_TABLE[0] = 0;
    for (i=1; i<65536; ++i) {
        BITCOUNT_TABLE[i] = (i & 1) + BITCOUNT_TABLE[i >> 1];
    }
}


/**
 * Frees memory buffers of a geometric hash.
 *
 * @param gh the hash to free
 */
void free_geometric_hash(geometrichash *gh);
    if (gh != NULL) {
        int i;
        if (gh->records != NULL) {
            for (i=0; i<gh->size; ++i) {
                free(gh->records[i].data);
            }
            free(gh->records);
        }
    }
}

/**
 * Builds a geometric hash for the given song collection.
 *
 * @param gh pointer to an uninitialized geometric hash struct
 * @param sc song collection
 * @param window_start hash feature extraction window leading edge note position
 * @param window_end hash feature extraction window trailing edge note position
 * @param window_height feature extraction window height in pitches
 * @param w projection space width
 * @param h projection space height
void build_geometric_hash(geometrichash *gh, const songcollection *sc,
        int window, int w, int h) {
    if (! BITCOUNT_TABLE_INITIALIZED) {
        
    }
}


void search(const vectorindex *vindex, const song *pattern, int window,
        matchset *ms);


/**
 * Initializes and builds a vector index for a song collection.
 *
 * @param vindex the vector index to build
 * @param sc the song collection for which the index is built
 * @param max_x maximum width of an index vector
 * @param max_y maximum height of an index vector
 * @param c_window maximum number of consecutive notes the algorithm is
 *        allowed to skip when selecting indexed vectors
 */
void build_vectorindex(vectorindex *vindex, const songcollection *sc,
        int max_x, int max_y, int c_window) {
    int x, y;

    vindex->width = max_x;
    vindex->height = max_y << 1;
    vindex->size = vindex->width * vindex->height;
    vindex->vectors = (indexvector *) malloc(vindex->size * sizeof(indexvector));
    if (vindex->vectors == NULL) return;

    for (y=0; y<vindex->height; ++y) {
        for (x=0; x<vindex->width; ++x) {
            indexvector *v = &vindex->vectors[y * vindex->width + x];
            v->size = 0;
            v->allocate = 0;
            v->records = NULL;
        }
    }
    vindex->c_window = c_window;
    vindex->scollection = sc;
    populate_vectorindex(vindex);
}

