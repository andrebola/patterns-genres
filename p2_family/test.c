/*
 * test.c - Test helper functions for the index filters and geometric
 *               scanners.
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


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include <limits.h>
#include <time.h>

#include "test.h"
#include "midifile.h"
#include "algorithms.h"
#include "song.h"
#include "util.h"


/** Command-line argument IDs (also used as short arguments) */

#define TEST_ARG_HELP               'h'
#define TEST_ARG_ALGORITHMS         'a'
#define TEST_ARG_ALG_NAME           'A'
#define TEST_ARG_SEED               's'
#define TEST_ARG_RUN_TWICE          512
#define TEST_ARG_NUM_REPEATS        'R'
#define TEST_ARG_TIME_ALLOCATION    513
#define TEST_ARG_C_WINDOW           'c'
#define TEST_ARG_D_WINDOW           'd'
#define TEST_ARG_P1_SAMPLES         514
#define TEST_ARG_P2_WINDOW          515
#define TEST_ARG_P2_THRESHOLD       516
#define TEST_ARG_P2_FIXED_POINTS    517
#define TEST_ARG_VECTOR_WIDTH       'W'
#define TEST_ARG_VECTOR_HEIGHT      'H'
#define TEST_ARG_QUANTIZE           'Q'
#define TEST_ARG_REMOVE_OCTAVES     'O'
#define TEST_ARG_SKIP_PERCUSSION    'P'
#define TEST_ARG_JOIN_SONGS         'j'
#define TEST_ARG_SHUFFLE_SONGS      518
#define TEST_ARG_SHUFFLE_PATTERNS   519
#define TEST_ARG_COLLECTION_NOTES   'n'
#define TEST_ARG_SIMILARITY_CUTOFF  520
#define TEST_ARG_PATTERN_PATH       'p'
#define TEST_ARG_GENERATE_PATTERNS  'g'
#define TEST_ARG_PATTERN_NOTES      'm'
#define TEST_ARG_PATTERN_MAX_SKIP   'S'
#define TEST_ARG_PATTERN_MAX_TP     'T'
#define TEST_ARG_PATTERN_ERRORS     'e'
#define TEST_ARG_INSERTED_PATTERNS  'i'
#define TEST_ARG_INSERTED_NOISE     'N'
#define TEST_ARG_OUTPUT             'o'
#define TEST_ARG_NUM_RESULTS        'r'
#define TEST_ARG_MULTIPLE_MATCHES   521
#define TEST_ARG_DISTANCE_MATRIX    'x'
#define TEST_ARG_MEASUREMENT_POINTS 522
#define TEST_ARG_RESULT_ROW_LABEL   523
#define TEST_ARG_VERBOSE            'v'
#define TEST_ARG_TIME_INDEXING      'I'
#define TEST_ARG_P3_REMOVE_GAPS     524

static const struct option LONG_OPTIONS[] = {
    {"help",                no_argument,        0, TEST_ARG_HELP},
    {"algorithms",          required_argument,  0, TEST_ARG_ALGORITHMS},
    {"alg-name",            required_argument,  0, TEST_ARG_ALG_NAME},
    {"seed",                required_argument,  0, TEST_ARG_SEED},
    {"run-twice",           no_argument,        0, TEST_ARG_RUN_TWICE},
    {"num-repeats",         required_argument,  0, TEST_ARG_NUM_REPEATS},
    {"time-allocation",     no_argument,        0, TEST_ARG_TIME_ALLOCATION},
    {"c-window",            required_argument,  0, TEST_ARG_C_WINDOW},
    {"d-window",            required_argument,  0, TEST_ARG_D_WINDOW},
    {"p1-samples",          required_argument,  0, TEST_ARG_P1_SAMPLES},
    {"p2-window",           required_argument,  0, TEST_ARG_P2_WINDOW},
    {"p2-threshold",        required_argument,  0, TEST_ARG_P2_THRESHOLD},
    {"p2-fixed-points",     required_argument,  0, TEST_ARG_P2_FIXED_POINTS},
    {"p3-remove-gaps",      required_argument,  0, TEST_ARG_P3_REMOVE_GAPS},
    {"vector-width",        required_argument,  0, TEST_ARG_VECTOR_WIDTH},
    {"vector-height",       required_argument,  0, TEST_ARG_VECTOR_HEIGHT},
    {"quantize",            required_argument,  0, TEST_ARG_QUANTIZE},
    {"remove-octaves",      no_argument,        0, TEST_ARG_REMOVE_OCTAVES},
    {"skip-percussion",     no_argument,        0, TEST_ARG_SKIP_PERCUSSION},
    {"join-songs",          required_argument,  0, TEST_ARG_JOIN_SONGS},
    {"shuffle-songs",       no_argument,        0, TEST_ARG_SHUFFLE_SONGS},
    {"collection-notes",    required_argument,  0, TEST_ARG_COLLECTION_NOTES},
    {"similarity-cutoff",   required_argument,  0, TEST_ARG_SIMILARITY_CUTOFF},
    {"pattern-dir",         required_argument,  0, TEST_ARG_PATTERN_PATH},
    {"shuffle-patterns",    no_argument,        0, TEST_ARG_SHUFFLE_PATTERNS},
    {"generate-patterns",   required_argument,  0, TEST_ARG_GENERATE_PATTERNS},
    {"pattern-notes",       required_argument,  0, TEST_ARG_PATTERN_NOTES},
    {"pattern-max-skip",    required_argument,  0, TEST_ARG_PATTERN_MAX_SKIP},
    {"max-transposition",   required_argument,  0, TEST_ARG_PATTERN_MAX_TP},
    {"pattern-errors",      required_argument,  0, TEST_ARG_PATTERN_ERRORS},
    {"inserted-patterns",   required_argument,  0, TEST_ARG_INSERTED_PATTERNS},
    {"inserted-noise",      required_argument,  0, TEST_ARG_INSERTED_NOISE},
    {"output",              required_argument,  0, TEST_ARG_OUTPUT},
    {"num-results",         required_argument,  0, TEST_ARG_NUM_RESULTS},
    {"multiple-matches",    no_argument,        0, TEST_ARG_MULTIPLE_MATCHES},
    {"distance-matrix",     required_argument,  0, TEST_ARG_DISTANCE_MATRIX},
    {"measurement-points",  required_argument,  0, TEST_ARG_MEASUREMENT_POINTS},
    {"row-label",           required_argument,  0, TEST_ARG_RESULT_ROW_LABEL},
    {"verbose",             no_argument,        0, TEST_ARG_VERBOSE},
    {"output-indexing-time",required_argument,  0, TEST_ARG_TIME_INDEXING},
    {0, 0, 0, 0}
};



/**
 * Prints program usage information.
 *
 * @param p default parameter values
 */
void test_print_usage(const test_parameters *p) {
    int i;
    puts("\nA test program for measuring speed and accuracy of music retrieval algorithms.\n");

    printf("Syntax: %s [options] <song collection>\n\n", p->program_name);

    puts(  "Where song collection is a directory of MIDI files\n");

    puts(  "Options:\n");

    fputs( "  -a, --algorithms [string list]  Algorithms to test [", stdout);
    i = 0;
    while (p->algorithm_list[i] >= 0) {
        printf("%s", get_algorithm_name(p->algorithm_list[i]));
        ++i;
        if (p->algorithm_list[i] >= 0) fputs(",", stdout);
    }
    fputs("]\n", stdout);
    puts(  "                                  Use \"-a help\" to print available algorithms\n");
    puts(  "  -s, --seed <uint>          Seed for the random number generator [time]\n");
    puts(  "  -R, --num-repeats <uint>   Number of times to repeat each run to smooth out");
    printf("                             timing inaccuracy [%d]\n\n",
            p->num_repeats);
    puts(  "      --run-twice            Run each algorithm twice, without counting time");
    puts(  "                             for the first run [no]\n");
    puts(  "      --time-allocation      Measure search time allocation to subtasks like");
    puts(  "                             indexing and verifying matches [no]");
    puts(  "  -c, --c-window <int>       Maximum number of notes to skip when selecting");
    printf("                             a vector for the index from a song [%d]\n",
            p->search_parameters.c_window);
    puts(  "                             The value to choose depends on the amount of");
    puts(  "                             polyphony and the type of queries made.");
    puts(  "                             Larger values result in a larger index.\n");

    printf("  -W, --vector-width <int>   Maximum width of an index vector in ms [%d]\n\n",
            p->data_parameters.avindex_vector_max_width);

    printf("  -H, --vector-height <int>  Maximum height of an index vector in halftones [%d]\n\n",
            p->data_parameters.avindex_vector_max_height);

    printf("  -Q, --quantize <int>       Quantization in milliseconds [%d]\n",
            p->search_parameters.quantization);
    puts(  "                             This is applied to both songs and patterns.");
    puts(  "                             NOT recommended at the moment; it is better to");
    puts(  "                             quantize the data properly with other tools\n");
    puts(  "  -O, --remove-octaves       Remove octave information [no]\n");
    puts(  "  -P, --skip-percussion      Skip MIDI percussion channel [no]\n");
    puts(  "      --p3-remove-gaps       Maximum gap length allowed between two consecutive\n");
    puts(  "                             notes [0]\n");
    puts(  "  -j, --join-songs <int>     Join all read songs into n songs [0]\n");
    puts(  "      --shuffle-songs        Shuffle songs in the collection [no]\n");
    puts(  "  -n, --collection-notes <int list>  Number of notes in the song");
    printf("                                     collection [%d]\n\n",
            p->collection_notes[0]);

    puts(  "Algorithm parameters:\n");

    puts(  "  -A, --alg-name <string>    Algorithm description to be used for output\n");
    puts(  "  -d, --p1-window <int>      Maximum number of notes to skip when selecting");
    printf("                             a vector from a pattern [%d]\n",
            p->search_parameters.d_window);
    puts(  "                             If this is set too high compared to c_window,");
    puts(  "                             the vector selected from the pattern may have");
    puts(  "                             been dropped from the index because of c_window.\n");
    printf("      --p1-samples <int>     Maximum number of samples to use in P1/F3 [%d]\n\n",
            p->search_parameters.p1_sample_size);
    puts(  "      --p2-window <int>      P2 indexing: Maximum number of vectors to use per");
    puts(  "                             pattern note [c_window/4]\n");
    puts(  "      --p2-threshold <float> P2 indexing (P2/F4, P2/F5): threshold for");
    printf("                             the amount of pattern vectors to use [%f]\n\n",
            p->search_parameters.p2_select_threshold);
    puts(  "      --p2-fixed-points <int>    P2' indexing: Number of fixed points that must");
    printf("                                 be in the matches [1/%d of pattern notes]\n\n",
            -p->search_parameters.p2_num_points);


    puts(  "Pattern input:\n");

    puts(  "  -p, --pattern-dir <path>       Read patterns from a MIDI directory.\n");

    puts(  "Generated patterns:\n");

    puts(  "  -g, --generate-patterns <int>  Extract the given number of random patterns");
    printf("                                 from the song collection [%d]\n\n",
            p->generated_pattern_count);

    printf("  -m, --pattern-notes <int list> Number of notes in a pattern [%d].\n\n",
            p->generated_pattern_notes[0]);
    puts(  "                                 Set to 0 to use song collection as patterns");
    puts(  "                                 (search for maximal repeated subpatterns).");

    puts(  "  -S, --pattern-max-skip <int>   Maximum number of notes to skip in the song");
    printf("                                 when generating a pattern [%d]\n\n",
            p->generated_pattern_max_skip);

    puts(  "  -T, --max-transposition <int>  Maximum transposition applied to generated");
    printf("                                 patterns [%d]\n\n",
            p->generated_pattern_max_transposition);

    puts(  "  -e, --pattern-errors <float>   Rate of errors introduced to an extracted");
    printf("                                 pattern [%f]. Start times and pitches are\n",
            p->generated_pattern_errors);
    puts(  "                                 changed for a random selection of notes in");
    puts(  "                                 the pattern. For error rate 0, there will be");
    puts(  "                                 no changed notes. For error rate 0.7, 70 % of");
    puts(  "                                 the notes will be changed.\n");

    printf("  -i, --inserted-patterns <int>  Number of inserted patterns [%d]\n", p->inserted_pattern_instances);

    printf("  -N, --inserted-noise <float>   Noise rate in inserted patterns [%f]\n", p->inserted_pattern_noise);

    puts(  "      --shuffle-patterns         Shuffle patterns in the collection [no]\n");

    puts(  "Output:\n");

    puts(  "  -o, --output <path>            Write result data to this file [stdout]\n");
    printf("  -r, --num-results <int>        Number of matches to retrieve per query [%d]\n\n",
            p->results);
    puts(  "      --multiple-matches         Allow multiple matches in the same song [no]\n");
    puts(  "  -x, --distance-matrix <path>   Write a distance matrix to a file [no]\n");
    puts(  "  -v, --verbose                  Print status information while running [no]");
    puts(  "                                 Use multiple times (-vv) for more verbosity.\n");
    puts(  "  -I, --output-indexing-time <path>  Write index size and building time");
    puts(  "                                     to this file [stdout]\n");

    puts(  "Output parameters for Precision-Recall and MAP curves:\n");

    printf("      --similarity-cutoff <float>  Recall cutoff similarity [%f]\n\n",
            p->similarity_cutoff);
    puts(  "      --measurement-points <int> Number of sample points to use in some\n");
    printf("                                 measurements (precision-recall) [%d]\n",
            p->measurement_points);
    puts(  "                                 Use 0 to output MAP values instead of P-R\n");
    printf("      --row-label <float>        Row label for MAP measurements [%f]\n",
            p->result_row_label);
    puts(  "                                 For example, generated pattern error rate.\n");

}


/**
 * Parses a list of algorithm names.
 *
 * @param string a list of algorithm names as a comma-separated string
 *
 * @return an array of algorithm IDs, terminated with a negative value
 */
int *test_parse_algorithm_list(const char *string) {
    int i = 0;
    int len = strlen(string);
    int *list;
    char *name;
    const char *ptr, *lastptr;

    name = (char *) malloc((len + 1) * sizeof(char));
    if (len > 1) len -= len >> 1;
    ++len;
    list = (int *) malloc(len * sizeof(int));

    if ((list == NULL) || (name == NULL)) {
        free(list);
        free(name);
        return NULL;
    }

    i = 0;
    lastptr = ptr = string;
    while (1) {
        if ((*ptr == ',') || (*ptr == ';') || (*ptr == '\0') || (*ptr == ' ')) {
            if (ptr != lastptr) {
                int id;
                name = strncpy(name, lastptr, ptr - lastptr);
                name[ptr - lastptr] = '\0';
                id = get_algorithm_id(name);
                if (id > 0) {
                    list[i] = id;
                    ++i;
                }
            }
            if (*ptr == '\0') break;
            lastptr = ptr + 1;
        }
        ++ptr;
    }
    list[i] = -1;
    if (i == 0) {
        print_algorithms();
        return list;
    }

    free(name);
    return list;
}


/**
 * Parses a list of integers.
 *
 * @param string a list of integers as a comma-separated string
 *
 * @return an array of integers, terminated with a negative value
 */
int *test_parse_integer_list(const char *string) {
    int i = 0;
    int len = strlen(string);
    int *list;
    const char *ptr;

    if (len > 1) len -= len >> 1;
    ++len;

    list = (int *) malloc(len * sizeof(int));
    if (list == NULL) return NULL;

    i = 0;
    ptr = string;
    while (*ptr != '\0') {
        char *end;
        int a = strtol(ptr, &end, 10);
        if (end != ptr) {
            list[i] = a;
            ++i;
            ptr = end;
        } else {
            ptr++;
        }
    }
    list[i] = -1;
    return list;
}


/**
 * Initializes the given parameter struct with default values.
 *
 * @param p parameter struct
 */
void test_init_parameters(test_parameters *p) {
    int i;
    p->program_name = NULL;
    p->song_path = NULL;
    p->pattern_path = NULL;

    p->algorithm_list = (int *) malloc((1 + INDEXTEST_DEFAULT_ALGORITHMS_NUM) *
        sizeof(int));
    for (i=0; i<INDEXTEST_DEFAULT_ALGORITHMS_NUM; ++i) {
        p->algorithm_list[i] = INDEXTEST_DEFAULT_ALGORITHMS[i];
    }
    p->algorithm_list[i] = -1;

    p->algorithm_name = NULL;
    p->seed = (unsigned int) time(NULL);
    p->run_twice = 0;
    p->num_repeats = 1;
    p->remove_octaves = 0;
    p->skip_percussion = 0;
    p->join_songs = 0;
    p->shuffle_songs = 0;
    p->similarity_cutoff = 0.1F;

    p->collection_notes = (int *) malloc(2 * sizeof(int));
    p->collection_notes[0] = 0;
    p->collection_notes[1] = -1;

    /*p->overall_generated_pattern_count = 1;*/
    p->generated_pattern_count = 1;
    p->generated_pattern_max_skip = 10;
    p->generated_pattern_max_transposition = 32;
    p->generated_pattern_errors = 0.0F;
    p->inserted_pattern_instances = 0;
    p->inserted_pattern_noise = 0.0F;
    p->shuffle_patterns = 0;

    p->generated_pattern_notes = (int *) malloc(2 * sizeof(int));
    p->generated_pattern_notes[0] = 100; p->generated_pattern_notes[1] = -1;

    p->output = NULL;
    p->results = 10;
    p->verbose = 0;
    p->multiple_matches_per_song = 0;
    p->distance_matrix_file = NULL;
    p->output_indexing_time = NULL;

    p->measurement_points = 0;
    p->result_row_label = 0.0F;

    p->data_parameters.avindex_vector_max_width = 10000;
    p->data_parameters.avindex_vector_max_height = 36;
    p->data_parameters.c_window = 30;

    p->search_parameters.c_window = p->data_parameters.c_window;
    p->search_parameters.d_window = 3;
    p->search_parameters.p1_sample_size = 50;
    p->search_parameters.p2_window = -1;
    p->search_parameters.p2_select_threshold = 0.5;
    p->search_parameters.p2_num_points = -10;
    p->search_parameters.p2_points = NULL;
    p->search_parameters.p3_calculate_difference = 1;
    p->search_parameters.p3_remove_gaps = 0;
    p->search_parameters.quantization = 0;
    p->search_parameters.msm_r = 32;
    p->search_parameters.measure_time_allocation = 0;
    p->search_parameters.sync_window_size = 25;
    p->search_parameters.sync_accuracy = 200;
    p->search_parameters.syncmap_accuracy = 200;

    p->next_parameter_group = NULL;
}


/**
 * Frees subparameter groups and algorithm lists from the given parameter
 * struct.
 *
 * @param p parameter struct
 */
void test_free_parameters(test_parameters *p) {
    free(p->algorithm_list);
    free(p->collection_notes);
    free(p->generated_pattern_notes);
    p->algorithm_list = NULL;
    p = p->next_parameter_group;
    while (p != NULL) {
        test_parameters *np = p->next_parameter_group;
        free(p->algorithm_list);
        p->algorithm_list = NULL;
        free(p);
        p = np;
    }
}


/**
 * Parses command-line arguments into the given struct, overriding the
 * previous values.
 *
 * @param argc argument count
 * @param argv argument list
 * @param p pointer to a parameter struct
 *
 * @return 1 if successful, 0 otherwise
 */
int test_parse_arguments(int argc, char **argv, test_parameters *p) {
    int i, option_index = 0;
    int default_algorithms = 1;
    char *short_options;
    test_parameters *global_parameters = p;
    short_options = make_short_options(LONG_OPTIONS);

    if (!short_options) return 0;

    p->program_name = argv[0];

    while ((i = getopt_long(argc, argv, short_options,
            LONG_OPTIONS, &option_index)) != -1) {
        switch (i) {
            case TEST_ARG_HELP:
                test_print_usage(p);
                return 0;
            case TEST_ARG_ALGORITHMS:
                if (optarg == NULL) {
                    print_algorithms();
                    return 0;
                }
                else if (default_algorithms) {
                    free(p->algorithm_list);
                    p->algorithm_list = test_parse_algorithm_list(optarg);
                    default_algorithms = 0;
                } else {
                    p->next_parameter_group = (test_parameters *) malloc(
                            sizeof(test_parameters));
                    if (p->next_parameter_group == NULL) return 0;
                    memcpy(p->next_parameter_group, global_parameters,
                            sizeof(test_parameters));
                    p = p->next_parameter_group;
                    p->next_parameter_group = NULL;
                    p->algorithm_list = test_parse_algorithm_list(optarg);
                }
                break;
            case TEST_ARG_ALG_NAME:
                p->algorithm_name = optarg;
                break;
            case TEST_ARG_SEED:
                if (p == global_parameters)
                    p->seed = (unsigned int) atoi(optarg);
                else fputs("Warning: parameter --seed cannot be used locally\n",
                        stderr);
                break;
            case TEST_ARG_RUN_TWICE:
                p->run_twice = 1;
                break;
            case TEST_ARG_NUM_REPEATS:
                p->num_repeats = MAX2(atoi(optarg), 1);
                break;
            case TEST_ARG_TIME_ALLOCATION:
                p->search_parameters.measure_time_allocation = 1;
                break;
            case TEST_ARG_C_WINDOW:
                if (p == global_parameters) {
                    p->search_parameters.c_window = MAX2(atoi(optarg), 1);
                    p->data_parameters.c_window = p->search_parameters.c_window;
                } else fputs("Warning: parameter --c-window cannot be used locally\n",
                        stderr);
                break;
            case TEST_ARG_D_WINDOW:
                p->search_parameters.d_window = MAX2(atoi(optarg), 1);
                break;
            case TEST_ARG_P1_SAMPLES:
                p->search_parameters.p1_sample_size = MAX2(atoi(optarg), 1);
                break;
            case TEST_ARG_P2_WINDOW:
                p->search_parameters.p2_window = MAX2(atoi(optarg), 1);
                break;
            case TEST_ARG_P2_THRESHOLD:
                p->search_parameters.p2_select_threshold =
                        MIN2(MAX2(atof(optarg), 0.0F), 1.0F);
                break;
            case TEST_ARG_P2_FIXED_POINTS:
                p->search_parameters.p2_num_points = atoi(optarg);
                break;
            case TEST_ARG_P3_REMOVE_GAPS:
                p->search_parameters.p3_remove_gaps = atoi(optarg);
                break;
            case TEST_ARG_VECTOR_WIDTH:
                if (p == global_parameters) {
                    p->data_parameters.avindex_vector_max_width =
                            MAX2(atoi(optarg), 1);
                } else fputs("Warning: parameter --vector-width cannot be used locally\n",
                        stderr);
                break;
            case TEST_ARG_VECTOR_HEIGHT:
                if (p == global_parameters) {
                    p->data_parameters.avindex_vector_max_height =
                            MAX2(atoi(optarg), 1);
                } else fputs("Warning: parameter --vector-height cannot be used locally\n",
                        stderr);
                break;
            case TEST_ARG_QUANTIZE:
                if (p == global_parameters) {
                    p->search_parameters.quantization = MAX2(atoi(optarg), 0);
                } else fputs("Warning: parameter --quantize cannot be used locally\n",
                        stderr);
                break;
            case TEST_ARG_REMOVE_OCTAVES:
                if (p == global_parameters)
                    p->remove_octaves = 1;
                else fputs("Warning: parameter --remove-octaves cannot be used locally\n",
                        stderr);
                break;
            case TEST_ARG_SKIP_PERCUSSION:
                if (p == global_parameters)
                    p->skip_percussion = 1;
                else fputs("Warning: parameter --skip-percussion cannot be used locally\n",
                        stderr);
                break;
            case TEST_ARG_JOIN_SONGS:
                if (p == global_parameters)
                    p->join_songs = atoi(optarg);
                else fputs("Warning: parameter --join-songs cannot be used locally\n",
                        stderr);
                break;
            case TEST_ARG_SHUFFLE_SONGS:
                if (p == global_parameters)
                    p->shuffle_songs = 1;
                else fputs("Warning: parameter --shuffle-songs cannot be used locally\n",
                        stderr);
                break;
            case TEST_ARG_COLLECTION_NOTES:
                if (p == global_parameters) {
                    free(p->collection_notes);
                    p->collection_notes = test_parse_integer_list(optarg);
                } else fputs("Warning: parameter --collection-notes cannot be used locally\n",
                        stderr);
                break;
            case TEST_ARG_SIMILARITY_CUTOFF:
                p->similarity_cutoff = MIN2(MAX2(atof(optarg), 0.0F), 1.0F);
                break;
            case TEST_ARG_PATTERN_PATH:
                p->pattern_path = optarg;
                break;
            case TEST_ARG_GENERATE_PATTERNS:
                p->generated_pattern_count = MAX2(atoi(optarg), 0);
/*                if (p->generated_pattern_count > global_parameters->overall_generated_pattern_count) 
                    global_parameters->overall_generated_pattern_count = p->generated_pattern_count;
*/
                break;
            case TEST_ARG_PATTERN_NOTES:
                if (p == global_parameters) {
                    free(p->generated_pattern_notes);
                    p->generated_pattern_notes = test_parse_integer_list(optarg);
                } else fputs("Warning: parameter --pattern-notes cannot be used locally\n",
                        stderr);
                break;
            case TEST_ARG_PATTERN_MAX_SKIP:
                p->generated_pattern_max_skip = MAX2(atoi(optarg), 0);
                break;
            case TEST_ARG_PATTERN_MAX_TP:
                p->generated_pattern_max_transposition = MAX2(atoi(optarg), 0);
                break;
            case TEST_ARG_PATTERN_ERRORS:
                p->generated_pattern_errors =
                        MIN2(MAX2(atof(optarg), 0.0F), 1.0F);
                break;
            case TEST_ARG_INSERTED_PATTERNS:
                p->inserted_pattern_instances = MAX2(atoi(optarg), 0);
                break;
            case TEST_ARG_INSERTED_NOISE:
                p->inserted_pattern_noise = MAX2(atof(optarg), 0.0F);
                break;
            case TEST_ARG_OUTPUT:
                p->output = optarg;
                break;
            case TEST_ARG_NUM_RESULTS:
                p->results = MAX2(atoi(optarg), 0);
                break;
            case TEST_ARG_VERBOSE:
                p->verbose += 1;
                break;
            case TEST_ARG_MULTIPLE_MATCHES:
                p->multiple_matches_per_song = 1;
                break;
            case TEST_ARG_DISTANCE_MATRIX:
                p->distance_matrix_file = optarg;
                break;
            case TEST_ARG_MEASUREMENT_POINTS:
                p->measurement_points = MAX2(atoi(optarg), 0);
                break;
            case TEST_ARG_RESULT_ROW_LABEL:
                p->result_row_label = atof(optarg);
                break;
            case TEST_ARG_TIME_INDEXING:
                p->output_indexing_time = optarg;
                break;

            default:
                return 0;
        }
    }

    free(short_options);

    if (p->algorithm_list[0] < 0) {
        fputs("\nNo search algorithms specified, exiting...\n", stderr);
        return 0;
    }

    p = global_parameters;
    while (p != NULL) {
        /* Parse remaining non-option arguments */
        if (optind < argc) {
            p->song_path = argv[optind];
        }

        /* Use the song file as a pattern file when writing a distance matrix */
        if ((p->distance_matrix_file != NULL) && (p->pattern_path == NULL)) {
            p->pattern_path = p->song_path;
        }

        if (p->remove_octaves) {
            p->data_parameters.avindex_vector_max_height = 12;
        }

        if (p->search_parameters.p2_window <= 0)
            p->search_parameters.p2_window = p->search_parameters.c_window / 4;

        p = p->next_parameter_group;
    }

    return 1;
}


/**
 * Reads user-specified songs into the given song collection structure.
 *
 * @param p parameter struct that contains a song collection path
 * @param scollection a song collection where the songs will be put
 * @param num_notes number of notes to crop from the data
 *
 * @return 1 if successful, 0 otherwise
 */
int test_read_songs(const test_parameters *p,
        songcollection *scollection, int num_notes) {
    struct timeval start, end;
    int songsread;

    if (p->song_path == NULL) {
        test_print_usage(p);
        fputs("\nError: song collection has not been specified\n\n", stderr);
        return 0;
    }

    /* Read songs */

    if (p->verbose >= LOG_IMPORTANT) fputs("Reading song files...\n", stderr);
    gettimeofday(&start, NULL);
    /*songsread = read_songfile(song_path, &scollection);*/
    songsread = read_midi_directory(p->song_path, scollection,
            p->skip_percussion);
    gettimeofday(&end, NULL);
    if (p->verbose >= LOG_IMPORTANT)
        fprintf(stderr, "Time: %f\n", timediff(&end, &start));

    if (songsread <= 0) {
        fprintf(stderr, "\nError: Unable to read songs from: %s\n\n",
                p->song_path);
        return 0;
    }

    /* Preprocess songs (sort, remove overlaps, quantize) */

    if (p->verbose >= LOG_IMPORTANT) fputs("\nPreprocessing...\n", stderr);
    gettimeofday(&start, NULL);

    preprocess_songs(scollection, p->search_parameters.quantization,
            p->shuffle_songs, -1);
    if (p->search_parameters.p3_remove_gaps)
        p3_optimize_song_collection(scollection,
                p->search_parameters.p3_remove_gaps,
                p->search_parameters.p3_remove_gaps >> 1);
    if (p->remove_octaves) sc_remove_octave_information(scollection);

    gettimeofday(&end, NULL);
    if (p->verbose >= LOG_IMPORTANT)
        fprintf(stderr,"Time: %f\n", timediff(&end, &start));

    if (p->join_songs) {
        songcollection *sc = join_songs(scollection, SONG_GAP, p->join_songs,
                num_notes, p->shuffle_songs);
        if (sc != NULL) {
            if (p->verbose >= LOG_IMPORTANT) {
                fprintf(stderr, "Joined %d songs into %d songs of %d notes\n",
                        scollection->size, p->join_songs, sc->songs[0].size);
            }
            free_song_collection(scollection);
            scollection->songs = sc->songs;
            scollection->size = sc->size;
            free(sc);
        }
    }

    return 1;
}


/**
 * Reads user-specified patterns into the given song collection structure.
 *
 * @param p parameter struct that contains a pattern collection path
 * @param scollection a song collection where the patterns will be put
 *
 * @return 1 if successful, 0 otherwise
 */
int test_read_patterns(const test_parameters *p,
        songcollection *pcollection, matchset *pmatches) {
    struct timeval start, end;
    int patternsread;

    if (p->verbose >= LOG_IMPORTANT)
        fputs("\nReading pattern files...\n", stderr);
    gettimeofday(&start, NULL);
    patternsread = read_midi_directory(p->pattern_path, pcollection,
            p->skip_percussion);
    /*patternsread = read_songfile(pattern_path, &pcollection);*/
    if (patternsread <= 0) {
        fprintf(stderr, "\nError: Unable to read patterns from: %s\n\n",
                p->pattern_path);
        return 0;
    }
    preprocess_songs(pcollection, p->search_parameters.quantization,
            p->shuffle_patterns, p->generated_pattern_notes[0]);
    if (p->remove_octaves) sc_remove_octave_information(pcollection);
    pmatches->matches = NULL;
    pmatches->size = 0;
    pmatches->num_matches = 0;

    gettimeofday(&end, NULL);
    if (p->verbose >= LOG_IMPORTANT)
        fprintf(stderr, "Time: %f\n", timediff(&end, &start));

    return 1;
}

/**
 * Generates patterns from a song collection according to the given parameters.
 * This function will also insert modified patterns to the collection if
 * p->inserted_pattern_instances is larger than zero.
 *
 * @param p parameter struct
 * @param scollection song collection from which the patterns will be generated
 * @param pcollection target pattern collection
 * @param pmatches data structure for storing the original positions of
 *        extracted patterns
 * @param min_notes minimum number of notes in the patterns
 * @param max_notes maximum number of notes in the patterns
 *
 * @return 1 if successful, 0 otherwise
 */
int test_generate_patterns(const test_parameters *p,
        songcollection *scollection, songcollection *pcollection,
        matchset *pmatches, int num_patterns, int min_notes, int max_notes) {
    struct timeval start, end;
    float errors;

    if (p->inserted_pattern_instances) errors = 0;
    else errors = p->generated_pattern_errors;

    if (p->verbose >= LOG_INFO)
        fputs("\nGenerating patterns...\n", stderr);
    gettimeofday(&start, NULL);

    generate_pattern_collection(num_patterns, pcollection,
            scollection, pmatches, min_notes, max_notes,
            p->generated_pattern_max_skip,
            p->generated_pattern_max_transposition,
            errors);

    if (p->inserted_pattern_instances) {
        insert_patterns(scollection, pcollection, p->inserted_pattern_instances,
                p->generated_pattern_errors, p->inserted_pattern_noise);
        update_song_collection_statistics(scollection);
        test_init_song_collection_data(p, scollection, p->algorithm_list);
    }

    gettimeofday(&end, NULL);
    if (p->verbose >= LOG_INFO)
        fprintf(stderr, "Time: %f\n", timediff(&end, &start));

    return 1;
}


/**
 * Initializes alternative data formats within the given song collection.
 *
 * @param p parameter struct
 * @param sc song collection to index
 */
void test_init_song_collection_data(const test_parameters *p,
        songcollection *sc, int *algorithms) {
    struct timeval start, end;

    if (p->verbose >= LOG_IMPORTANT) fputs("\nIndexing and converting to algorithm-specific formats...\n", stderr);
    gettimeofday(&start, NULL);

    update_song_collection_data(sc, algorithms, &p->data_parameters);

    gettimeofday(&end, NULL);
    if (p->verbose >= LOG_IMPORTANT)
        fprintf(stderr, "Time: %f\n", timediff(&end, &start));
    if (p->output_indexing_time != NULL) {
        FILE *f = fopen(p->output_indexing_time, "a");
        fprintf(f, "%d \t%d \t%d \t%f\n", sc->num_notes, sc->size,
                /* FIXME: memory usage */ 0, timediff(&end, &start) * 1000.0F);
        fclose(f);
    }
}


/**
 * Selects points from a pattern for P2' according to given parameters.
 *
 * @param sp search parameters that include number of points to generate.
 *        The selected points will be stored to this structure.
 * @param pattern pattern whose points are selected
 * @param verbose set to 1 to print status messages, 0 for silent operation
 */
void test_p2_select_points(searchparameters *sp, song *pattern, int verbose) {
    int i;
    int *points = sp->p2_points;
    int num_points = sp->p2_num_points;
    int d_window = sp->d_window;

    if (num_points >= pattern->size) {
        num_points = pattern->size - 1;
        sp->p2_num_points = num_points;
    }
    if (num_points > (pattern->size >> 1)) {
        for (i=0; i<num_points; ++i) {
            points[i] = i;
        }
        return;
    }
    for (i=0; i<num_points; ++i) {
        int j;
        int p1 = (int) (randf() * (float) pattern->size - 1);
        for (j=0; j<i; ++j) {
            if (points[j] == p1) {
                p1 = -1;
                break;
            }
        }
        if (p1 < 0) --i;
        else {
            points[i] = p1;
            /* select another point nearby to make the points effective */
            if ((i < (num_points - 1)) && (i < (pattern->size - 1))) {
                /* Is there already a point */
                int p2 = 0;
                for (j=0; j<i; ++j) {
                    int d = points[j] - p1;
                    if ((d > 0) && (d <= d_window)) {
                        p2 = -1;
                        break;
                    }
                }
                if (p2 >= 0) {
                    /* Add a point */
                    float d = (float) MIN2(pattern->size - 1 - i, d_window);
                    p2 = p1 + 1 + (int) (randf() * d);
                    ++i;
                    points[i] = p2;
                }
            }
        }
    }
    /* Sort points */
    qsort(points, num_points, sizeof(int), cmp_int);

    if (verbose >= LOG_INFO) {
        fprintf(stderr, "Selected %d points for P2':\n", num_points);
        for(i=0; i<num_points; ++i) {
            fprintf(stderr, " %d,", points[i]);
        }
        fputs("\n", stderr);
    }
}


/**
 * Loads a song collection from disk.
 *
 * @param p parameter group
 * @param sc target song collection data structure
 * @param num_notes number of notes to crop from the loaded data
 * @param force set to 1 to force reloading of songs
 *
 * @return 1 if successful, 0 otherwise.
 */
static int load_song_collection(const test_parameters *p, songcollection *sc,
        int num_notes, int force) {
    if ((num_notes == 0) || (num_notes > sc->num_notes) || force) {
        free_song_collection(sc);
        if (!test_read_songs(p, sc, num_notes)) {
            return 0;
        }
    }
    preprocess_songs(sc, 0, p->shuffle_songs, num_notes*MAX2(1, p->join_songs));
    update_song_collection_statistics(sc);

    return 1;
}


/**
 * Runs a set of tests according to the given parameters using an external
 * test function.
 *
 * @param p parameter group
 * @param testf test function
 */
void run_test(const test_parameters *p, test_function testf) {
    songcollection *scollection = NULL;
    songcollection *pcollection = NULL;
    matchset *pmatches = NULL;
    int algorithms[NUM_ALGORITHMS+1];
    const test_parameters *pp = NULL;
    int num_pcollections = 0;
    int i;

    /* Initialize the random number generator. This is used when selecting
     * patterns from the database. */
    srand(p->seed);

    /* Read the song collection */
    scollection = (songcollection *) calloc(1, sizeof(songcollection));

    /* Count pattern collections */
    if (p->pattern_path == NULL) {
        while(p->generated_pattern_notes[num_pcollections] >= 0)
            ++num_pcollections;
    } else num_pcollections = 1;
 
    pcollection = (songcollection *) calloc(1, sizeof(songcollection));
    pmatches = (matchset *) calloc(1, sizeof(matchset));

    /* Write headers and */
    pp = p;
    i = 0;
    while (pp != NULL) {
        int j = 0;
        int k = 0;
        while (pp->algorithm_list[k] >= 0) {
            if (pp->output != NULL) {
                FILE *f;
                f = fopen(pp->output, "w");
                if (pp->algorithm_name == NULL) {
                    fprintf(f, "# %s\n", get_algorithm_name(
                            pp->algorithm_list[k]));
                } else {
                    fprintf(f, "# %s\n", pp->algorithm_name);
                }
                fclose(f);
            }
            for (j=0; j<i; ++j) {
                if (algorithms[j] == pp->algorithm_list[k]) break;
            }
            if (j == i) {
                algorithms[j] = pp->algorithm_list[k];
                ++i;
            }
            ++k;
        }
        pp = pp->next_parameter_group;
    }
    algorithms[i] = -1;

    if (p->output_indexing_time != NULL) {
        FILE *f;
        f = fopen(p->output_indexing_time, "w");
        fprintf(f, "# Index construction\n#\n");
        fprintf(f, "# Columns:\n");
        fprintf(f, "# Notes in collection, songs, index size (bytes), construction time (ms)\n");
        fclose(f);
    }

    i = 0;
    while (1) {
        if (p->collection_notes[i] < 0) break;
        if (!load_song_collection(p, scollection, p->collection_notes[i], 1))
            goto run_test_exit;
        if (!p->inserted_pattern_instances) {
            test_init_song_collection_data(p, scollection, algorithms);
        }

        if (p->verbose >= LOG_IMPORTANT)
            fprintf(stderr, "\nRunning tests on song collection that contains %d notes\n", scollection->num_notes);

        pp = p;
        while (pp != NULL) {
            int j;
            if (p->verbose >= LOG_IMPORTANT) {
                fputs("\nAlgorithm: ", stderr);
                j = 0;
                while (pp->algorithm_list[j] >= 0) {
                    fprintf(stderr, "%s ",
                            get_algorithm_name(pp->algorithm_list[j]));
                    ++j;
                }
                fputs("\n", stderr);
            }
            for (j=0; j<num_pcollections; ++j) {
                int k = 0;
                /* Read or generate patterns */
                srand(pp->seed);
                if (pp->pattern_path == NULL) {
                    if (pp->generated_pattern_notes[j] == 0) {
                        /* Search for maximal repeated subpatterns. */
                        fputs("Warning: number of notes in generated patterns not specified\nUsing the whole song collection as a pattern set.\n", stderr);
                        memcpy(pcollection, scollection, sizeof(songcollection));
                    } else {
                        /* Reload song collection */ 
                        if ((pp->inserted_pattern_instances > 0) &&
                                (p->measurement_points == 0)) {
                            if (!load_song_collection(p, scollection,
                                    p->collection_notes[i], 1))
                                goto run_test_exit;
                        }
                        test_generate_patterns(pp, scollection, pcollection,
                                pmatches, pp->generated_pattern_count,
                                pp->generated_pattern_notes[j],
                                pp->generated_pattern_notes[j]);
                        update_song_collection_statistics(pcollection);
                    }
                } else {
                    test_read_patterns(pp, pcollection, pmatches);
                    update_song_collection_statistics(pcollection);
                }

                if (p->verbose >= LOG_IMPORTANT)
                    fprintf(stderr, "Requested pattern size %d\n",
                            pp->generated_pattern_notes[j]);

                while (pp->algorithm_list[k] >= 0) {
                    FILE *f;
                    char *res;
                    srand(pp->seed);
                    res = testf(pp, pp->algorithm_list[k],
                            scollection, pcollection, pmatches);
                    if (res != NULL) {
                        if (pp->output != NULL) {
                            f = fopen(pp->output, "a");
                            fputs(res, f);
                            fclose(f);
                        } else {
                            fputs(res, stdout);
                        }
                        free(res);
                    }
                    ++k;
                }
                free_match_set(pmatches);
                if (pcollection->songs != scollection->songs) {
                    free_song_collection(pcollection);
                } else {
                    memset(pcollection, 0, sizeof(songcollection));
                }
            }
            pp = pp->next_parameter_group;
        }

        if (p->collection_notes[0] < 0) break;
        ++i;
    }

    /* Clean up */

run_test_exit:

    free_song_collection(scollection);
    free_match_set(pmatches);

    if (pcollection != scollection) {
        free(pcollection);
    }
    free(scollection);
    free(pmatches);
}

