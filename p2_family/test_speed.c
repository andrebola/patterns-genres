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
#include "song.h"
#include "util.h"


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
        const matchset *pattern_matches, time_measures *m) {
    int i;
    double total_time = 0.0;
    double *t;
    matchset ms;
    searchparameters *sp;
#ifdef MEASURE_TIME_ALLOCATION
    double *t_indexing;
    double *t_other;
    double *t_verifying;

    t_indexing = (double *) malloc(patterns->size * sizeof(double));
    t_other = (double *) malloc(patterns->size * sizeof(double));
    t_verifying = (double *) malloc(patterns->size * sizeof(double));
#endif
    t = (double *) malloc(patterns->size * sizeof(double));

    init_match_set(&ms, p->results, 0, p->multiple_matches_per_song);

    m->lowest = (double) INT_MAX;
    m->highest = -1;

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
        struct timeval start, end;
        double delta;
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

        gettimeofday(&start, NULL);

        for (j=0; j<p->num_repeats; ++j) {
            search(sc, &patterns->songs[i], algorithm,
                    sp, &ms);
        }
        gettimeofday(&end, NULL);
        delta = timediff(&end, &start) / ((double) p->num_repeats);
        total_time += delta;
        t[i] = delta;
#ifdef MEASURE_TIME_ALLOCATION
        if (ms.time.measure) {
            t_indexing[i] = ms.time.indexing;
            t_other[i] = ms.time.other;
            t_verifying[i] = ms.time.verifying;
        }
#endif
        if (delta < m->lowest) m->lowest = delta;
        if (delta > m->highest) m->highest = delta;

        if (p->verbose >= LOG_INFO) print_results(&ms, sc);

    }

    m->q1 = kth_smallest_double(t, patterns->size, patterns->size / 4);
    m->q2 = kth_smallest_double(t, patterns->size, patterns->size / 2);
    m->q3 = kth_smallest_double(t, patterns->size,
            patterns->size - patterns->size / 4 - 1);
    m->mean = total_time / ((double) patterns->size);

    if (p->verbose >= LOG_INFO)
        fprintf(stderr, "\nTime mean:%f lowest:%f q1:%f q2:%f q3:%f highest:%f\n",
                m->mean, m->lowest, m->q1, m->q2, m->q3, m->highest);


#ifdef MEASURE_TIME_ALLOCATION
    if (p->search_parameters.measure_time_allocation) {
        m->indexing_q2 = kth_smallest_double(t_indexing, patterns->size,
                patterns->size / 2);
        m->other_q2 = kth_smallest_double(t_other, patterns->size,
                patterns->size / 2);
        m->verifying_q2 = kth_smallest_double(t_verifying, patterns->size,
                patterns->size / 2);
    }
    free(t_indexing);
    free(t_other);
    free(t_verifying);
#endif

    free(t);
    free_match_set(&ms);
    if ((algorithm == FILTER_P2_POINTS) || (algorithm == ALG_P2_POINTS)) {
        free(sp->p2_points);
    }
    free(sp);
}


/**
 * Test program for measuring algorithm execution speed.
 *
 * @param p parameter group
 * @param alg algorithm to test
 * @param sc song collection
 * @param pc patterns as a song collection
 * @param pmatches pointer to a data structure where results will be stored
 *
 * @return result string with quartile and mean query times
 */
static char *test_execution_speed(const test_parameters *p, int alg,
        const songcollection *sc, const songcollection *pc,
        const matchset *pmatches) {
    time_measures m;
    char *results;

    if ((sc->size == 0) || (pc->size == 0)) return NULL;

    results = (char *) malloc(256);

#ifdef MEASURE_TIME_ALLOCATION
    m.indexing_q2 = 0;
    m.other_q2 = 0;
    m.verifying_q2 = 0;
#endif

    search_patterns(p, alg, sc, pc, pmatches, &m);

#ifdef MEASURE_TIME_ALLOCATION
    if (p->search_parameters.measure_time_allocation) {
        snprintf(results, 256, "%d \t%d \t%f \t%f \t%f \t%f \t%f \t%f \t%f \t%f \t%f\n",
                sc->num_notes, pc->max_song_size,
                1000.0 * m.mean, 1000.0 * m.lowest, 1000.0 * m.q1, 1000.0 * m.q2,
                1000.0 * m.q3, 1000.0 * m.highest, 1000.0 * m.indexing_q2,
                1000.0 * m.other_q2, 1000.0 * m.verifying_q2);
    } else {
        snprintf(results, 256, "%d \t%d \t%f \t%f \t%f \t%f \t%f \t%f\n",
                sc->num_notes, pc->max_song_size,
                1000.0 * m.mean, 1000.0 * m.lowest, 1000.0 * m.q1, 1000.0 * m.q2,
                1000.0 * m.q3, 1000.0 * m.highest);
    }
#else
    snprintf(results, 256, "%d \t%d \t%f \t%f \t%f \t%f \t%f \t%f\n",
            sc->num_notes, pc->max_song_size,
            1000.0 * m.mean, 1000.0 * m.lowest, 1000.0 * m.q1, 1000.0 * m.q2,
            1000.0 * m.q3, 1000.0 * m.highest);
#endif

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

    run_test(&p, &test_execution_speed);

    test_free_parameters(&p);

    return 0;

}


