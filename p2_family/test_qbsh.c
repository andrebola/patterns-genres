/*
 * test_collection_size.c - A test program for the index filters
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
#include <getopt.h>
#include <limits.h>
#include <time.h>

#include "test.h"
#include "algorithms.h"
#include "search.h"
#include "song.h"
#include "util.h"

#define PATH_SEPARATOR '/'
#define SUFFIX_SEPARATOR '.'


static int print_filename(char *output, int output_size, int pos,
        const char *str, int truncate_path) {
    int start = 0;
    int end = strlen(str);
    int i;
    if (truncate_path) {
        int l = strlen(str);
        for (i=l-1; i>=0; --i) {
            if (str[i] == PATH_SEPARATOR) break;
        }
        start = i + 1;
        for (i=l-1; i>=start; --i) {
            if (str[i] == SUFFIX_SEPARATOR) break;
        }
        if (i <= start) end = l;
        else end = i;
    }
    if ((output_size - pos) <= (end - start)) {
        end = start + (output_size - pos) - 1;
        fputs("Error in print_song_title: output string is too short",
                stderr);
    }
    for (i=start; i<end; ++i) {
        output[pos] = str[i];
        ++pos;
    }
    if (pos < output_size) output[pos] = '\0';
    return pos;
}


/**
 * Searches matches for a set of patterns using the specified algorithm.
 *
 * @param p operation parameters
 * @param algorithm the search algorithm or index filter to use
 * @param sc a song collection
 * @param patterns searched patterns as a song collection
 * @param pattern_matches original position information for patterns extracted
 *        from the song collection
 * @param m pointer to a structure where calculated time measurement values
 *        will be stored
 */
static void search_patterns(const test_parameters *p, int algorithm,
        const songcollection *sc, const songcollection *patterns,
        const matchset *pattern_matches,
        char *results, unsigned int results_size) {
    int i;
    matchset ms;
    searchparameters *sp;
    unsigned int rpos = 0;

    init_match_set(&ms, p->results, 0, p->multiple_matches_per_song);

    if (p->verbose >= LOG_INFO) {
        fputs("\n\n\n=======================================================\n",
                stderr);
        fprintf(stderr, "\nSearching with %s\n", get_algorithm_name(algorithm));
    }

    sp = (searchparameters *) malloc(sizeof(searchparameters));
    memcpy(sp, &p->search_parameters, sizeof(searchparameters));

    if ((algorithm == FILTER_P2_POINTS) || (algorithm == ALG_P2_POINTS)) {
        if (p->search_parameters.p2_num_points < 0)
                sp->p2_num_points = patterns[0].size /
                        (-p->search_parameters.p2_num_points);
        sp->p2_points = malloc(sp->p2_num_points * sizeof(int));
    }

    for (i=0; i<patterns->size; ++i) {
        int j;

        if (p->verbose >= LOG_INFO) {
            fputs("\n----------------------------", stderr);
            if ((pattern_matches != NULL) &&
                    (pattern_matches->num_matches > i)) {
                fprintf(stderr, "\nPattern %d: %s\n    (strt: %d, end: %d, transp: %d)\n\n",
                        i+1, patterns->songs[i].title,
                        pattern_matches->matches[i].start,
                        pattern_matches->matches[i].end,
                        pattern_matches->matches[i].transposition);
            } else {
                fprintf(stderr, "\nPattern %d: %s\n\n", i+1,
                        patterns->songs[i].title);
            }
        }

        if ((algorithm == FILTER_P2_POINTS) || (algorithm == ALG_P2_POINTS))
            test_p2_select_points(sp, &patterns->songs[i], p->verbose);

        for (j=0; j<p->num_repeats; ++j) {
            search(sc, &patterns->songs[i], algorithm, sp, &ms);
        }

        if (p->verbose >= LOG_INFO) print_results(&ms, sc);

        rpos = print_filename(results, results_size, rpos,
                patterns->songs[i].title, 0);
        rpos = print_filename(results, results_size, rpos, ":", 0);

        for (j=0; j<ms.num_matches; ++j) {
            match *m = &ms.matches[j];
            rpos = print_filename(results, results_size, rpos, " ", 0);
            rpos = print_filename(results, results_size, rpos,
                    sc->songs[m->song].title, 1);
        }
        rpos = print_filename(results, results_size, rpos, "\n", 0);
    }

    free_match_set(&ms);
    if ((algorithm == FILTER_P2_POINTS) || (algorithm == ALG_P2_POINTS)) {
        free(sp->p2_points);
    }
    free(sp);
}


/**
 * Test program for searching within the MIREX QBSH data set.
 *
 * @param p parameter group
 * @param alg algorithm to test
 * @param sc song collection
 * @param pc patterns as a song collection
 * @param pmatches pointer to a data structure where results will be stored
 *
 * @return test results as a string
 */
static char *test_qbsh(const test_parameters *p, int alg,
        const songcollection *sc, const songcollection *pc,
        const matchset *pmatches) {
    char *results;
    unsigned int len = 0;
    unsigned int results_length = 0;
    int i;

    /* Estimate output size */
    for (i=0; i<sc->size; ++i) {
        if (len < strlen(sc->songs[i].title)) len = strlen(sc->songs[i].title);
    }
    len = (len + 2) * p->results;
    for (i=0; i<pc->size; ++i) {
        results_length += strlen(pc->songs[i].title) + 4 + len;
    }
    results = (char *) calloc(results_length + 1, sizeof(char));

    /* Search */
    search_patterns(p, alg, sc, pc, pmatches, results, results_length);

    return results;
}


/**
 * Vector index test program.
 *
 * @param argc number of arguments
 * @param argv argument array
 *
 * @return 0
 */
int main(int argc, char **argv) {
    test_parameters p;

    test_init_parameters(&p);
    if (!test_parse_arguments(argc, argv, &p)) return 1;

    run_test(&p, &test_qbsh);

    test_free_parameters(&p);

    return 0;

}


