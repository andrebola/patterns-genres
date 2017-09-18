/*
 * test.h - Declarations and definitions for test helper functions.
 *
 * Version 2007-09-20
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

#ifndef __INDEXTEST_H__
#define __INDEXTEST_H__

#include "algorithms.h"
#include "data.h"
#include "results.h"
#include "search.h"
#include "song.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct _test_parameters {
    char *program_name;
    char *song_path;
    char *pattern_path;
    int *algorithm_list;
    char *algorithm_name;
    unsigned int seed;
    int run_twice;
    int num_repeats;
    int remove_octaves;
    int skip_percussion;
    int join_songs;
    int shuffle_songs;
    int *collection_notes;
    float similarity_cutoff;
    /*int overall_generated_pattern_count;*/
    int generated_pattern_count;
    int *generated_pattern_notes;
    int generated_pattern_max_skip;
    int generated_pattern_max_transposition;
    float generated_pattern_errors;
    int inserted_pattern_instances;
    float inserted_pattern_noise;
    int shuffle_patterns;
    char *output;
    int results;
    int verbose;
    int measurement_points;
    float result_row_label;
    int multiple_matches_per_song;
    char *distance_matrix_file;
    char *output_indexing_time;
    dataparameters data_parameters;
    searchparameters search_parameters;
    struct _test_parameters *next_parameter_group;
} test_parameters;

typedef struct {
    double mean;
    double lowest;
    double q1;
    double q2;
    double q3;
    double highest;
#ifdef MEASURE_TIME_ALLOCATION
    double indexing_q2;
    double other_q2;
    double verifying_q2;
#endif
} time_measures;


#define INDEXTEST_DEFAULT_ALGORITHMS_NUM 1
static const int INDEXTEST_DEFAULT_ALGORITHMS[] = {ALG_P2};


void test_print_usage(const test_parameters *p);

/*void test_search(const test_parameters *p, int algorithm,
        const songcollection *sc, const vectorindex *vindex,
        const songcollection *patterns, const matchset *pattern_matches,
	time_measures *m);

void test_precision_recall(const test_parameters *p, int algorithm,
        int gt_algorithm, const songcollection *sc, const vectorindex *vindex,
        const songcollection *patterns, const matchset *pattern_matches,
        int num_points, float *precision_recall, float *map);
*/

int *test_parse_algorithm_list(const char *string);

int *test_parse_integer_list(const char *string);

void test_init_parameters(test_parameters *p);

void test_free_parameters(test_parameters *p);

int test_parse_arguments(int argc, char **argv, test_parameters *p);

void test_p2_select_points(searchparameters *sp, song *pattern, int verbose);

int test_read_songs(const test_parameters *p,
        songcollection *scollection, int num_notes);

int test_read_patterns(const test_parameters *p,
        songcollection *pcollection, matchset *pmatches);

int test_generate_patterns(const test_parameters *p,
        songcollection *scollection, songcollection *pcollection,
        matchset *pmatches, int num_patterns, int min_notes, int max_notes);

void test_init_song_collection_data(const test_parameters *p,
        songcollection *sc, int *algorithms);

/**
 * Test function declaration.
 *
 * See test_run_test()
 *
 * @param p parameter group
 * @param alg algorithm to test
 * @param vindex pointer to a vector index, or NULL if the algorithm does
 *        not use an index
 * @param sc song collection
 *
 * @return test results as a string
 */
typedef char *(*test_function)(
        const test_parameters *p, int alg,
        const songcollection *sc, const songcollection *pc,
	const matchset *pmatches);

void run_test(const test_parameters *p, test_function testf);


#ifdef __cplusplus
}
#endif

#endif

