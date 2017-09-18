/*
 * search.c - Functions for managing search algorithms
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

#include "algorithms.h"
#include "config.h"
#include "filter_P1.h"
#include "filter_P2.h"
#include "geometric_P1.h"
#include "geometric_P2.h"
#include "geometric_P3.h"
#include "geometric_SP1.h"
#include "geometric_SP2.h"
#include "sync_P3.h"
#include "results.h"
#include "search.h"
#include "song.h"

#ifdef ENABLE_FG
#include "fg.h"
#endif

#ifdef ENABLE_MSM
#include "search_msm.h"
#endif


static void (* const SEARCH_FUNCTIONS[])(const songcollection *sc,
        const song *pattern, int alg, const searchparameters *parameters,
        matchset *ms) = {
          NULL,
/*  1 */  alg_p1, 
/*  2 */  alg_p2,
/*  3 */  alg_p2_points,
/*  4 */  alg_p3,
/*  5 */  alg_sp1,
/*  6 */  alg_sp2,
/*  7 */  filter_p1_random,
/*  8 */  filter_p1_select_1,
/*  9 */  filter_p1_select_2,
/* 10 */  filter_p1_sample,
/* 11 */  filter_p2_window,
/* 12 */  filter_p2_select_local,
/* 13 */  filter_p2_select_global,
#ifdef ENABLE_BLOSSOM4
/* 14 */  filter_p2_pigeonhole,
#else
/* 14 */  NULL,
#endif
/* 15 */  filter_p2_greedy_pigeonhole,
/* 16 */  filter_p2_points,
#ifdef ENABLE_MSM
/* 17 */  alg_msm,
/* 18 */  alg_msm,
/* 19 */  alg_msm,
/* 20 */  alg_msm,
/* 21 */  alg_msm,
/* 22 */  alg_msm,
/* 23 */  alg_msm,
/* 24 */  alg_msm,
#else
/* 17 */  NULL,
/* 18 */  NULL,
/* 19 */  NULL,
/* 20 */  NULL,
/* 21 */  NULL,
/* 22 */  NULL,
/* 23 */  NULL,
/* 24 */  NULL,
#endif
#ifdef ENABLE_FG
/* 25 */  alg_fg,
/* 26 */  alg_fg,
#else
/* 25 */  NULL,
/* 26 */  NULL,
#endif
/* 27 */  NULL,
};


/**
 * Scans the given song collection exhaustively to find matches
 * to a pattern.
 *
 * @param sc the song collection
 * @param pattern a pattern to search in the collection
 * @param alg the algorithm ID as defined in algorithms.h
 * @param parameters search parameters for different algorithms
 * @param ms structure where the matches will be stored
 */
void search(const songcollection *sc, const song *pattern, int alg,
        const searchparameters *parameters, matchset *ms) {

    if ((alg <= 0) || (alg > NUM_ALGORITHMS)) {
        fprintf(stderr, "Error in search(): Undefined algorithm %d\n", alg);
        return;
    }
    if (SEARCH_FUNCTIONS[alg] != NULL) {
        SEARCH_FUNCTIONS[alg](sc, pattern, alg, parameters, ms);
    } else {
        fprintf(stderr, "Error in search(): No search function defined for algorithm %d\n", alg);
    }
}


/**
 * Prints search results.
 *
 * @param ms the set of matches to print
 * @param sc song collection where the search was run. This can also be NULL.
 */
void print_results(const matchset *ms, const songcollection *sc) {
    if (ms->num_matches > 0) {
        int i;
        fputs("Matches:\n\n", stderr);
        for (i=0; i<ms->num_matches; ++i) {
            match *m = &ms->matches[i];
            if ((sc != NULL) && (sc->songs[m->song].title != NULL)) {
                fprintf(stderr, "  song %d, %s:\n    (strt: %d, end: %d, transp: %d, sim: %f)\n\n",
                    m->song+1, sc->songs[m->song].title, m->start, m->end,
                    m->transposition, m->similarity);
            } else {
                fprintf(stderr, "  song %d:\n    (strt: %d, end: %d, transp: %d, sim: %f)\n\n",
                    m->song+1, m->start, m->end, m->transposition,
                    m->similarity);

            }
        }
        return;
    } else {
        fputs("No matches.\n", stderr);
    }
}

