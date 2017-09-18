/*
 * algorithms.c - Functions for managing search algorithms
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
#include <string.h>

#include "config.h"
#include "algorithms.h"
#include "data_formats.h"
#include "util.h"


typedef struct {
    int id;
    int type;
    int subtype;
    int index;
    const char *name;
    const char *print_name;
    const char *description;
} alg_info;


static const alg_info ALGORITHM_INFO[] = {
    {ALG_P1,                        PROBLEM_1, 1, DATA_NONE,
    "P1",       "P1",
    "Finds complete matches of a pointset within the database."},

    {ALG_P2,                        PROBLEM_2, 1, DATA_NONE,
    "P2",       "P2",
    "Finds partial matches of a pointset within the database."},

    {ALG_P2_POINTS,                 PROBLEM_2, 2, DATA_NONE,
    "P2p",      "P2'",
    "Same as P2 except that this variant takes a set of points that must be present in the matches as input, to speed up searching."},

    {ALG_P3,                        PROBLEM_3, 1, DATA_P3,
    "P3",       "P3",
    "Finds the maximal overlapping of two sets of horizontal line segments."},

    {ALG_SP1,                       PROBLEM_4, 1, DATA_NONE,
    "SP1",      "SP1",
    "Experimental timescale-invariant algorithm for approximate matching."},

    {ALG_SP2,                       PROBLEM_4, 2, DATA_NONE,
    "SP2",      "SP2",
    "Another experimental timescale-invariant algorithm for approximate matching."},

    {FILTER_P1_RANDOM,              PROBLEM_1, 1, DATA_VINDEX,
    "P1F0",     "P1/F0",
    "P1 index filter that picks a random vector from the pattern to retrieve potential match locations from the index."},

    {FILTER_P1_SELECT_1,            PROBLEM_1, 1, DATA_VINDEX,
    "P1F1",     "P1/F1",
    "P1 index filter that picks the least frequent valid vector from the pattern to limit the number of potential locations."},

    {FILTER_P1_SELECT_2,            PROBLEM_1, 1, DATA_VINDEX,
    "P1F2",     "P1/F2",
    "P1 index filter that picks two least frequent valid vector from the pattern and checks only locatons where both vectors appear at the same relative positions as in the pattern."},

    {FILTER_P1_SAMPLE,              PROBLEM_1, 1, DATA_VINDEX,
    "P1F3",     "P1/F3",
    "P1 index filter that selects a group of sample vectors from the pattern and uses the least frequent vectors from that group to retrieve potential match locations."},

    {FILTER_P2_WINDOW,              PROBLEM_2, 1, DATA_VINDEX,
    "P2F4",     "P2/F4",
    "P2 index filter that extracts all vectors within the given window from the pattern and compares them to the database in the same way as P2 compares points."},

    {FILTER_P2_SELECT_LOCAL,        PROBLEM_2, 1, DATA_VINDEX,
    "P2F5",     "P2/F5",
    "P2 index filter that works like P2/F4 except that only the least frequent half of the valid vectors within each window position are used."},

    {FILTER_P2_SELECT_GLOBAL,       PROBLEM_2, 1, DATA_VINDEX,
    "P2F5g",    "P2/F5 (global select)",
    "Same as P2/F5 except that the least frequent vectors used in the search are selected globally--not within each window position."},

#ifdef ENABLE_BLOSSOM4
    {FILTER_P2_PH,                  PROBLEM_2, 1, DATA_VINDEX,
    "P2F6",     "P2/F6",
    "P2 index filter based on the pigeonhole principle and use of P1 index filters. This version uses the blossom4 graph matching algorithm to retrieve an optimal set of vectors from the pattern."},
#else
    {0, 0, 0, 0, "", "", ""},
#endif

    {FILTER_P2_GREEDY_PH,           PROBLEM_2, 1, DATA_VINDEX,
    "P2F6g",    "P2/F6 (greedy)",
    "P2 index filter based on the pigeonhole principle and use of P1 index filters. This version uses a greedy search to retrieve the set of search vectors."},

    {FILTER_P2_POINTS,              PROBLEM_2, 2, DATA_VINDEX,
    "P2pF7",    "P2'/F7",
    "P2' index filter that selects vectors around the user-specified points."},

#ifdef ENABLE_MSM

    {ALG_MSM_MFD_2D,                PROBLEM_2, 1, DATA_MSM,
    "MSM2D",    "MSM (MFD 2D)",
    "MSM algorithm that sorts and counts all the distances of the 2D points."},

    {ALG_MSM_MFD_2D_QUICK,          PROBLEM_2, 1, DATA_MSM,
    "MSM2Dq",   "MSM (MFD 2D quick)",
    "MSM algorithm that sorts and counts all the distances of the 2D points using only one sorting operation."},

    {ALG_MSM_MFD_1D,                PROBLEM_2, 1, DATA_MSM,
    "MSM1D",    "MSM (MFD 1D)",
    "MSM algorithm that sorts and counts all the distances of the 1D points."},

    {ALG_MSM_MFD_1D_QUICK,          PROBLEM_2, 1, DATA_MSM,
    "MSM1Dq",   "MSM (MFD 1D quick)",
    "MSM algorithm that sorts and counts all the distances of the 1D points using only one sorting operation."},

    {ALG_MSM_MFD_1D_QUICK_SPACEEFF, PROBLEM_2, 1, DATA_MSM,
    "MSM1Dqs",  "MSM (MFD 1D quick space-efficient)",
    "As MSM (MFD 1D quick), but more space-efficient."},

    {ALG_MSM_FFT,                   PROBLEM_2, 1, DATA_MSM,
    "MSMFFT",   "MSM (FFT)",
    "The full MSM algorithm with FFT."},

    {ALG_MSM_FFT_LOOKUP_1,          PROBLEM_2, 1, DATA_MSM,
    "MSMFFTl1", "MSM (FFT lookup 1)",
    "A variation of the MSM FFT algorithm."},

    {ALG_MSM_FFT_LOOKUP_2,          PROBLEM_2, 1, DATA_MSM,
    "MSMFFTl2", "MSM (FFT lookup 2)",
    "A variation of the MSM FFT algorithm."},

#else
    {0, 0, 0, 0, "", "", ""},
    {0, 0, 0, 0, "", "", ""},
    {0, 0, 0, 0, "", "", ""},
    {0, 0, 0, 0, "", "", ""},
    {0, 0, 0, 0, "", "", ""},
    {0, 0, 0, 0, "", "", ""},
    {0, 0, 0, 0, "", "", ""},
    {0, 0, 0, 0, "", "", ""},
#endif

#ifdef ENABLE_FG
    
    {ALG_FG_6,                      PROBLEM_1, 1, DATA_VINDEX,
    "FG6", "FG6",
    "Kimmo Fredrikson and Szymon Grabowski: alg 6, sparse_delta_gaps_tr_1Lf"},

    {ALG_FG_7,                      PROBLEM_1, 1, DATA_VINDEX,
    "FG7", "FG7",
    "Kimmo Fredrikson and Szymon Grabowski: alg 7, delta_gaps_dp_tr_co"},
#else
    {0, 0, 0, 0, "", "", ""},
    {0, 0, 0, 0, "", "", ""},
#endif

    {ALG_ALIGN_P3,                  PROBLEM_ALIGN, 1, DATA_P3,
    "AP3", "Align P3",
    "Finds the maximal overlapping of two sets of horizontal line segments."},

    {-1, 0, 0, 0, NULL, NULL, NULL}
};


/**
 * Prints algorithm names and information to stderr.
 */
void print_algorithms() {
    int i = 0;
    fputs("\nAvailable algorithms:\n\n", stderr);
    fputs("Problem Type    Index   Name            Full Name\n", stderr);
    fputs("-----------------------------------------------------------\n", stderr);

    while (ALGORITHM_INFO[i].id >= 0) {
        if (ALGORITHM_INFO[i].id == 0) {
            ++i;
            continue;
        }
        const char *space;
        if (strlen(ALGORITHM_INFO[i].name) >= 8) space = "\t";
        else space = "\t\t";
        fprintf(stderr, "  %d.%d\t\t%d\t%s%s%s\n", ALGORITHM_INFO[i].type,
            ALGORITHM_INFO[i].subtype, ALGORITHM_INFO[i].index,
            ALGORITHM_INFO[i].name, space, ALGORITHM_INFO[i].print_name);
        ++i;
    }
    fputs("\n", stderr);
}

/**
 * Returns algorithm id when given a name string.
 *
 * @param name algorithm name as a string
 *
 * @return the algorithm ID as defined in algorithms.h or 0 if no matching ID
 *         was found
 */
int get_algorithm_id(const char *name) {
    int i = 0;
    while (ALGORITHM_INFO[i].id >= 0) {
        if (strcicmp(name, ALGORITHM_INFO[i].name) == 0) {
            return ALGORITHM_INFO[i].id;
        }
        ++i;
    }
    return 0;
}

/**
 * Returns algorithm type
 *
 * @param alg the algorithm ID as defined in algorithms.h
 *
 * @return algorithm type
 */
int get_algorithm_type(int alg) {
    if ((alg > 0) && (alg <= NUM_ALGORITHMS)) {
        return ALGORITHM_INFO[alg-1].type;
    } else return 0;
}

/**
 * Returns algorithm subtype
 *
 * @param alg the algorithm ID as defined in algorithms.h
 *
 * @return algorithm subtype
 */
int get_algorithm_subtype(int alg) {
    if ((alg > 0) && (alg <= NUM_ALGORITHMS)) {
        return ALGORITHM_INFO[alg-1].subtype;
    } else return 0;
}

/**
 * Returns the index type required by the algorithm
 *
 * @param alg the algorithm ID as defined in algorithms.h
 *
 * @return algorithm index type
 */
int get_algorithm_data_format(int alg) {
    if ((alg > 0) && (alg <= NUM_ALGORITHMS)) {
        return ALGORITHM_INFO[alg-1].index;
    } else return 0;
}

/**
 * Returns the name of an algorithm.
 *
 * @param alg the algorithm ID as defined in algorithms.h
 *
 * @return algorithm name as a string
 */
const char *get_algorithm_name(int alg) {
    if ((alg > 0) && (alg <= NUM_ALGORITHMS)) {
        return ALGORITHM_INFO[alg-1].name;
    } else return NULL;
}

/**
 * Returns the full name of an algorithm for printing.
 *
 * @param alg the algorithm ID as defined in algorithms.h
 *
 * @return printable algorithm name as a string
 */
const char *get_algorithm_full_name(int alg) {
    if ((alg > 0) && (alg <= NUM_ALGORITHMS)) {
        return ALGORITHM_INFO[alg-1].print_name;
    } else return NULL;
}

/**
 * Returns algorithm description.
 *
 * @param alg the algorithm ID as defined in algorithms.h
 *
 * @return algorithm description as a string
 */
const char *get_algorithm_description(int alg) {
    if ((alg > 0) && (alg <= NUM_ALGORITHMS)) {
        return ALGORITHM_INFO[alg-1].description;
    } else return NULL;
}


