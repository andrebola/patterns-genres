/*
 * config.h - Configuration values
 *
 * Version 2007-05-28
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


#ifndef __CONFIG_H__
#define __CONFIG_H__


/*
 * General settings
 */


/** Debugging messages. */
/* #define DEBUG 1 */

/* Log levels */
#define LOG_IMPORTANT 1
#define LOG_INFO 2
#define LOG_DEBUG 3

#define ENABLE_UNIT_TESTS 1

/*
 * Song collection and index settings
 */

/**
 * Maximum number of vectors allowed to be stored into a single index bucket.
 */
#define MAX_INDEX_BUCKET_SIZE INT_MAX
/* #define MAX_INDEX_BUCKET_SIZE 1000 */


/** Gap between joined songs in milliseconds */
#define SONG_GAP 1000


/*
 * Algorithm settings
 */

/** Make P2 calculate the common duration for the best match. */
/* #define P2_CALCULATE_COMMON_DURATION 1 */

/** Normalize match similarity by min(song size, pattern size) instead of just
  * pattern size. */
/* #define P2_NORMALIZE_SIMILARITY 1 */


/** Order P2/F4 and P2/F5 matches with check_p2, to make the list more
  * accurate. This does not have much effect on search speed. */
#define ORDER_F4_F5_RESULTS_WITH_P2 1


/** Measure time allocation to algorithm subtasks
  * (index lookup, verification, ...) separately. */
#define MEASURE_TIME_ALLOCATION 1

/*
 * Compiler settings
 */


/* Don't use the inline directive when GCC runs in strict ANSI mode (-ansi) */

#if defined(__GNUC__)
    #if defined(__STRICT_ANSI__)
        #define INLINE
    #else
        #define INLINE inline
    #endif
#else
    #define INLINE inline
#endif


#endif

