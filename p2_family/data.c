/*
 * data.c - Functions for managing data formats and indices
 *
 * Version 2008-01-20
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
#include <string.h>

#include "config.h"
#include "data.h"
#include "song.h"

#ifdef VINDEX_ARRAY
#include "vindex_array.h"
#else
#include "vindex_tree.h"
#endif

#ifdef ENABLE_MSM
#include "search_msm.h"
#endif

#include "geometric_P3.h"


static void *(* const INIT_DATA_FORMAT[])(void) = {
    init_vectorindex,
#ifdef ENABLE_MSM
    init_msm_song_collection,
#else
    NULL,
#endif
    init_p3_song_collection,
};

static int (* const CONVERT_SONG_COLLECTION[])(
        void *data, const songcollection *sc, const dataparameters *dp) = {
    build_vectorindex,
#ifdef ENABLE_MSM
    build_msm_song_collection,
#else
    NULL,
#endif
    build_p3_song_collection,
};

static void (* const CLEAR_DATA_FORMAT[])(void *data) = {
    clear_vectorindex,
#ifdef ENABLE_MSM
    clear_msm_song_collection,
#else
    NULL,
#endif
    clear_p3_song_collection,
};


static void (* const FREE_DATA_FORMAT[])(void *data) = {
    free_vectorindex,
#ifdef ENABLE_MSM
    free_msm_song_collection,
#else
    NULL,
#endif
    free_p3_song_collection,
};


void *init_data_format(int format) {
    if ((format > 0) && (format <= NUM_DATA_FORMATS)) {
        return INIT_DATA_FORMAT[format-1]();
    } else return NULL;
}

int convert_song_collection(int format, void *data, const songcollection *sc,
        const dataparameters *dp) {
    if ((format > 0) && (format <= NUM_DATA_FORMATS)) {
        return CONVERT_SONG_COLLECTION[format-1](data, sc, dp);
    } else return 0;
}

void clear_data_format(int format, void *data) {
    if ((format > 0) && (format <= NUM_DATA_FORMATS)) {
        CLEAR_DATA_FORMAT[format-1](data);
    }
}

void free_data_format(int format, void *data) {
    if ((format > 0) && (format <= NUM_DATA_FORMATS)) {
        FREE_DATA_FORMAT[format-1](data);
    }
}

