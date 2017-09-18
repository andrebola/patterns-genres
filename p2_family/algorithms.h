/*
 * algorithms.h - Constants and external declarations for managing search
 *                algorithms
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


#ifndef __ALGORITHMS_H__
#define __ALGORITHMS_H__

#include "config.h"

#ifdef __cplusplus
extern "C" {
#endif


/** Number of algorithms in geometric-cbmr. Remember to edit
  * the SEARCH_FUNCTIONS array in search.c when changing this constant. */
#define NUM_ALGORITHMS 27

/* Algorithms and index filters that are available in geometric-cbmr. */


/** Geometric P1 algorithm for finding complete matches of two pointsets.
  * See geometric_P1.c for details. */
#define ALG_P1 1

/** Geometric P2 algorithm for finding partial matches of two pointsets.
  * See geometric_P2.c for details. */
#define ALG_P2 2

/** Geometric P2' algorithm for finding partial matches of two pointsets.
  * This variant takes a set of points that must be present in the matches as
  * input. */
#define ALG_P2_POINTS 3

/** Geometric P3 algorithm for finding the maximal overlap between two
  * sets of horizontal line segments, See geometric_P3.c for details. */
#define ALG_P3 4

/** Geometric SP1 algorithm. */
#define ALG_SP1 5

/** Geometric SP2 algorithm. */
#define ALG_SP2 6


/** P1 index filter that extracts a random valid vector from the pattern
  * to retrieve potential match locations from the index. See vectorindex.c. */
#define FILTER_P1_RANDOM 7

/** P1 index filter that extracts the least frequend valid vector from the
  * pattern to limit the number of locations to check. */
#define FILTER_P1_SELECT_1 8

/** P1 index filter that extracts two least frequent vectors from the
  * pattern and checks only locations where both vectors appear at the
  * same relative positions as in the pattern. */
#define FILTER_P1_SELECT_2 9

/** P1 index filter that selects a group of sample vectors and uses the least
  * frequent vector from that group to retrieve potential match locations. */
#define FILTER_P1_SAMPLE 10

/** P2 index filter that extracts all vectors within the given window
  * from the pattern and scans them in the same way as the actual P2 algorithm
  * scans points. */
#define FILTER_P2_WINDOW 11

/** P2 index filter. Same as FILTER_P2_WINDOW except that only half of
  * the valid vectors within each window position are used. Vectors are sorted
  * by their occurrence counts in the database and those vectors that are
  * least frequently found are used in the search. This usually results in a
  * tenfold speed increase, but accuracy will suffer. */
#define FILTER_P2_SELECT_LOCAL 12

/** P2 index filter. Same as FILTER_P2_SELECT_LOCAL except that the half
  * of the vectors to use is selected globally from all valid vectors. This
  * gives less frequent vectors and therefore increases search speed
  * considerably. Accuracy will suffer. */
#define FILTER_P2_SELECT_GLOBAL 13

/** P2 pigeonhole */
#define FILTER_P2_PH 14

/** P2 greedy pigeonhole */
#define FILTER_P2_GREEDY_PH 15


/** P2' index filter that selects vectors around points specified by
  * the user. These points must be found from the matches. */
#define FILTER_P2_POINTS 16


/* Algorithms that are available in MSM */

#define ALG_MSM_MFD_2D 17

#define ALG_MSM_MFD_2D_QUICK 18

#define ALG_MSM_MFD_1D 19

#define ALG_MSM_MFD_1D_QUICK 20

#define ALG_MSM_MFD_1D_QUICK_SPACEEFF 21

#define ALG_MSM_FFT 22

#define ALG_MSM_FFT_LOOKUP_1 23

#define ALG_MSM_FFT_LOOKUP_2 24

#define ALG_FG_6 25

#define ALG_FG_7 26


/* Alignment algorithms */

/** P3-based score alignment method */
#define ALG_ALIGN_P3 27


/* Problem types */

#define PROBLEM_1 1
#define PROBLEM_2 2
#define PROBLEM_3 3
#define PROBLEM_4 4
#define PROBLEM_ALIGN 5


/* External function declarations */

void print_algorithms(void);

int get_algorithm_id(const char *name);

int get_algorithm_type(int alg);

int get_algorithm_subtype(int alg);

int get_algorithm_data_format(int alg);

const char *get_algorithm_name(int alg);

const char *get_algorithm_full_name(int alg);

const char *get_algorithm_description(int alg);

#ifdef __cplusplus
}
#endif

#endif

