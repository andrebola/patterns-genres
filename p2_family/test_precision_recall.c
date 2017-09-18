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
 * Picks a ground truth algorithm that corresponds to the given algorithm.
 *
 * @param alg algorithm ID
 *
 * @return algorithm ID or 0 if no suitable algorithm is available
 */
static int select_ground_truth_algorithm(int alg) {
    int i;
    int type = get_algorithm_type(alg);
    int subtype = get_algorithm_subtype(alg);
    for (i=0; i<=MIN2(alg, 4); ++i) {
        if ((get_algorithm_type(i) == type) && 
                (get_algorithm_subtype(i) == subtype)) {
            return i;
        }
    }
    return 0;
}


/**
 * Constructs a precision-recall curve for an algorithm by comparing it to
 * a ground-truth method (for example indexed P2 vs P2).
 *
 * @param p operation parameters
 * @param alg the search algorithm or index filter to use
 * @param sc a song collection
 * @param pc searched patterns as a song collection
 * @param pmatches pointer to a data structure where results will be stored
 *
 * @return result string with precision-recall values for each measurement point
 *         (p->measurement_points) or a mean average precision value if there
 *         are no measurement points
 */
static char *test_precision_recall(const test_parameters *p, int alg,
        const songcollection *sc, const songcollection *pc,
        const matchset *pmatches) {
    int i;
    int gt_alg;
    matchset gt_matches, matches;
    searchparameters *sp;
    float *psamples;
    float map = 0.0F;
    char *results;

    if ((sc->size == 0) || (pc->size == 0)) return NULL;

    results = (char *) calloc(32 * MAX2(1, p->measurement_points),
            sizeof(char));
    if (results == NULL) return NULL;

    gt_alg = select_ground_truth_algorithm(alg);
    if (gt_alg <= 0) {
        fprintf(stderr, "Error: unable to select a ground truth algorithm for algorithm %s\n", get_algorithm_name(alg));
        return NULL;
    }

    if (p->measurement_points > 0) {
        psamples = (float *) malloc(p->measurement_points * sizeof(float));
    } else {
        psamples = NULL;
    }

    init_match_set(&gt_matches, sc->size, 0, p->multiple_matches_per_song);
    init_match_set(&matches, sc->size, 0, p->multiple_matches_per_song);

    if (p->verbose >= LOG_INFO) {
        fputs("\n\n\n=======================================================\n",
                stderr);
        fprintf(stderr, "\nSearching with %s\n", get_algorithm_name(alg));
    }

    sp = (searchparameters *) malloc(sizeof(searchparameters));
    if (sp == NULL) goto exit;

    memcpy(sp, &p->search_parameters, sizeof(searchparameters));

    if ((alg == FILTER_P2_POINTS) || (alg == ALG_P2_POINTS)) {
        if (sp->p2_num_points < 0)
            sp->p2_num_points = pc->max_song_size / (-sp->p2_num_points);
        sp->p2_points = malloc(sp->p2_num_points * sizeof(int));
    }

    for (i=0; i<p->measurement_points; ++i) {
        psamples[i] = 0.0F;
    }

    /* Calculate precision and recall for each query */
    for (i=0; i<pc->size; ++i) {
        int j;
        float average_precision = 0.0F;
        int next_sample_index = 0;
        float next_sample_recall = 0.0F;
        float last_recall = 0.0F;
        float last_precision = 0.0F;
        int num_relevant = 0;

        fprintf(stderr, "Pattern %d\n", i);

        if (p->verbose >= LOG_INFO) {
            fputs("\n----------------------------", stderr);
            if ((pmatches != NULL) &&
                    (pmatches->num_matches > i)) {
                fprintf(stderr, "\nPattern %d: %s\n    (strt: %d, end: %d, transp: %d)\n\n",
                        i+1, pc->songs[i].title,
                        pmatches->matches[i].start,
                        pmatches->matches[i].end,
                        pmatches->matches[i].transposition);
            } else {
                fprintf(stderr, "\nPattern %d: %s\n\n", i+1,
                        pc->songs[i].title);
            }
        }

        if ((alg == FILTER_P2_POINTS) || (alg == ALG_P2_POINTS))
            test_p2_select_points(sp, &pc->songs[i], p->verbose);

        search(sc, &pc->songs[i], gt_alg, sp, &gt_matches);
        search(sc, &pc->songs[i], alg, sp, &matches);

        for (j=0; j<gt_matches.num_matches; ++j) {
            match *m = &gt_matches.matches[j];

            if (m->similarity < p->similarity_cutoff) {
                gt_matches.num_matches = j;
                break;
            }
        }
        if (gt_matches.num_matches == 0) {
            fputs("Error in test_precision_recall(): empty ground-truth\n",
                stderr);
            continue;
        }

        for (j=0; j<matches.num_matches; ++j) {
            match *m = &matches.matches[j];
            int k;
            for (k=0; k<gt_matches.num_matches; ++k) {
                match *m2 = &gt_matches.matches[k];
                if (m->song == m2->song) {
                    float precision, recall;
                    ++num_relevant;
                    precision = (float) num_relevant / (float) (j+1);
                    average_precision += precision;
                    if (p->measurement_points > 0) {
                        recall = (float) num_relevant /
                                (float) gt_matches.num_matches;
                        /*fprintf(stderr, "%f %f\n", recall, precision);*/
                        if (last_recall == 0.0F) last_precision = precision;
                        while (recall >= next_sample_recall * 0.9999F) {
                            float rslope = (recall - next_sample_recall) /
                                    (recall - last_recall);
                            float sample = precision -
                                    (precision - last_precision) * rslope;
                            if (sample > 1.0F) sample = 1.0F;
                            else if (sample < 0.0F) sample = 0.0F;
                            /*fprintf(stderr, " s %d %f\n", next_sample_index, sample);*/
                            psamples[next_sample_index] += sample;
                            ++next_sample_index;
                            next_sample_recall = ((float) next_sample_index) /
                                    ((float) (p->measurement_points - 1));
                            if (next_sample_index >= p->measurement_points)
                                break;
                        }
                        last_precision = precision;
                        last_recall = recall;
                    }
                    /* break */
                    k = gt_matches.num_matches;
                }
            }
            if ((p->measurement_points > 0) &&
                    (next_sample_index >= p->measurement_points)) break;
        }

        /* printf("%f %d\n", average_precision, gt_matches.num_matches); */
        average_precision = average_precision /
                ((float) gt_matches.num_matches);
        map += average_precision;

        if (p->verbose >= LOG_INFO) print_results(&matches, sc);

        clear_match_set(&matches);
        clear_match_set(&gt_matches);
    }

    if (p->measurement_points > 0) {
        char *r = results;
        for (i=0; i<p->measurement_points; ++i) {
            float sample = 0.0F;
            int n;
            if (pc->size > 0)
                sample = psamples[i] / ((float) pc->size);
            n = snprintf(r, 32, "%f %f\n",
                    ((float) i) / ((float) (p->measurement_points - 1)), sample);
            r = &r[MIN2(n, 32)];
/*
            if (i == p->measurement_points-1) {
                snprintf(r, 32, "# MAP at %f: %f\n\n", p->result_row_label,
                    map / ((float) pc->size));
            }
*/
        }
    } else {
        snprintf(results, 32, "%f %f\n", p->result_row_label,
                map / ((float) pc->size));
    }

exit:
    if ((alg == FILTER_P2_POINTS) || (alg == ALG_P2_POINTS)) {
        free(sp->p2_points);
    }

    free(sp);
    free(psamples);
    free_match_set(&matches);
    free_match_set(&gt_matches);

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

    p.measurement_points = 11;

    if (!test_parse_arguments(argc, argv, &p)) return 1;

    run_test(&p, &test_precision_recall);

    test_free_parameters(&p);

    return 0;

}


