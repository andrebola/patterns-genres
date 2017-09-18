/*
 * vindex_array.c - Vector-based index structure for polyphonic music
 *                  retrieval. This variant is stored and accessed as
 *                  a two-dimensional array.
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


#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <math.h>
#include <string.h>

#include "config.h"
#include "data.h"
#include "util.h"
#include "vindex_array.h"


/**
 * Returns the position of the specified vector in the index structure.
 *
 * @param vindex the index structure
 * @param x vector's x component
 * @param y vector's y component
 *
 * @return position of the vector
 */
static INLINE int index_vector_position(const vectorindex *vindex, int x, int y) {
    y += vindex->height >> 1;
    if ((y < 0) || (x < 0) || (y >= vindex->height) || (x >= vindex->width))
        return -1;
    return (y * vindex->width + x);
}


/**
 * Adds a new record to the index structure. The memory buffer that
 * contains all the index records for the vector is set if it hasn't been
 * done before.
 *
 * @param vindex the index structure
 * @param x the x component of the indexed vector
 * @param y the y component of the indexed vector
 * @param songid song id in the song collection
 * @param note vector start position in the song
 * @param records pointer to a buffer where the index records should be stored
 *        to. This is actually only needed when the buffer pointer has not yet
 *        been set for the vector in question.
 *
 * @return the number of index records reserved from the records buffer. This
 * is usually 0.
 *
 * @see populate_vectorindex()
 */
#if 0
static INLINE int index_add(vectorindex *vindex, int x, int y,
        unsigned short songid, unsigned short note, indexrec *records) {
    indexvector *v;
    int pos = index_vector_position(vindex, x, y);
    if (pos < 0) return 0;

    v = &vindex->vectors[pos];
    if (v->allocate < 0) return 0;

    if (v->allocate > 0) {
        /* The record buffer has not yet been set; use the given pointer and
         * return the number of records needed (that information is stored
         * in v->allocate). */
        v->records = records;
        pos = v->allocate;
        v->allocate = 0;
    } else {
        pos = 0;
    }
    v->records[v->size].song = songid;
    v->records[v->size].note = note;
    v->size++;
    return pos;
}
#endif

/**
 * Initializes a vector index structure.
 *
 * @return pointer to the index structure.
 */
void *init_vectorindex(void) {
    vectorindex *vindex = calloc(1, sizeof(vectorindex));
    return vindex;
}

/**
 * Initializes and builds a vector index for a song collection.
 *
 * @param data pointer to a vector index structure
 * @param sc the song collection for which the index is built
 * @param max_x maximum width of an index vector
 * @param max_y maximum height of an index vector
 * @param c_window maximum number of consecutive notes the algorithm is
 *        allowed to skip when selecting indexed vectors
 * @param indexing_time if not NULL, time spent on populating the index
 *        is written here in seconds
 */
int build_vectorindex(void *data, const songcollection *sc,
        const dataparameters *dp) {
    int i, bp;
    int *vector_counts;
    int total_vcount;
    int indexed_vectors;
    vectorindex *vindex = (vectorindex *) data;

    vindex->width = dp->avindex_vector_max_width;
    vindex->height = dp->avindex_vector_max_height << 1;
    vindex->size = vindex->width * vindex->height;
    vector_counts = (int *) calloc(vindex->size,  sizeof(int));
    if (vector_counts == NULL) return 0;

/*    for (y=0; y<vindex->height; ++y) {
        for (x=0; x<vindex->width; ++x) {
            indexvector *v = vindex->vectors[y * vindex->width + x];
            v->size = 0;
            v->records = NULL;
        }
    }
*/
    vindex->c_window = dp->c_window;
    vindex->scollection = sc;

    /* Count all vectors so that we can allocate the right amount of memory */
    total_vcount = 0;
    for (i=0; i<sc->size; ++i) {
        int j;
        song *s = &sc->songs[i];
        for (j=0; j<s->size-1; ++j) {
            int end, k;
            if (vindex->c_window == INT_MAX) end = s->size - 1;
            else {
                end = j + vindex->c_window;
                if (end >= s->size) end = s->size - 1;
            }
            for (k=j+1; k<=end; ++k) {
                int x, pos;
                char y;
                x = s->notes[k].strt - s->notes[j].strt;
                y = s->notes[k].ptch - s->notes[j].ptch;
                pos = index_vector_position(vindex, x, y);
                if (pos >= 0) {
                    ++vector_counts[pos];
                    ++total_vcount;
                } else if (x >= vindex->width) break;
            }
        }
    }
    indexed_vectors = 0;
    for (i=0; i<vindex->size; ++i) {
        if (vector_counts[i] > MAX_INDEX_BUCKET_SIZE) vector_counts[i] = 0;
        else if (vector_counts[i] > 0) ++indexed_vectors;
    }

    /* Calculate total memory usage */
    vindex->memory_usage = total_vcount * sizeof(indexrec) +
            indexed_vectors * sizeof(indexvector);


    /* Allocate a continuous memory buffer for index vectors and records */
    vindex->buffer = (char *) malloc(vindex->memory_usage);

    /* Allocate space for pointers to index vectors */
    vindex->vectors = (indexvector **) malloc(vindex->size
            * sizeof(indexvector *));

    vindex->memory_usage += vindex->size * sizeof(indexvector *)
            + sizeof(vectorindex);

    /* Divide the buffer among the vectors */
    bp = 0;
    for (i=0; i<vindex->size; ++i) {
        if (vector_counts[i] > 0) {
            vindex->vectors[i] = (indexvector *) (&vindex->buffer[bp]);
            vindex->vectors[i]->size = 0;
            bp += vector_counts[i] * sizeof(indexrec)
                    + sizeof(indexvector);
        } else vindex->vectors[i] = NULL;
    }

    /* Scan and store individual records */
    for (i=0; i<sc->size; ++i) {
        int j;
        song *s = &sc->songs[i];
        for (j=0; j<s->size-1; ++j) {
            int end, k;
            if (vindex->c_window == INT_MAX) end = s->size - 1;
            else {
                end = j + vindex->c_window;
                if (end >= s->size) end = s->size - 1;
            }
            for (k=j+1; k<=end; ++k) {
                int x = s->notes[k].strt - s->notes[j].strt;
                char y = s->notes[k].ptch - s->notes[j].ptch;
                indexvector *iv = get_index_vector(vindex, x, y);
                if (iv != NULL) {
                    iv->records[iv->size].song = i;
                    iv->records[iv->size].note = j;
                    iv->size++;
                } else if (x >= vindex->width) break;
            }
        }
    }

    free(vector_counts);
    return 1;
}


/**
 * Frees memory buffers of a vector index and re-initializes it.
 *
 * @param data the index structure
 */
void clear_vectorindex(void *data) {
    vectorindex *vindex = (vectorindex *) data;
    if (vindex != NULL) {
        if (vindex->buffer != NULL) free(vindex->buffer);
        if (vindex->vectors != NULL) free(vindex->vectors);
        vindex->memory_usage = 0;
        vindex->size = 0;
    }
}


/**
 * Frees the given vector index data structure.
 *
 * @param data the index structure
 */
void free_vectorindex(void *data) {
    vectorindex *vindex = (vectorindex *) data;
    if (vindex != NULL) {
        clear_vectorindex(vindex);
        free(vindex);
    }
}


