/*
 * data_formats.h - Constants for data formats
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


#ifndef __DATA_FORMATS_H__
#define __DATA_FORMATS_H__

#ifdef __cplusplus
extern "C" {
#endif


/* Index and data format types */

#define NUM_DATA_FORMATS 3

#define DATA_NONE 0
#define DATA_VINDEX 1
#define DATA_MSM 2
#define DATA_P3 3


/**
 * Structure for data format parameters
 */
typedef struct {
    int c_window;
    int avindex_vector_max_width;
    int avindex_vector_max_height;
} dataparameters;


#ifdef __cplusplus
}
#endif

#endif

