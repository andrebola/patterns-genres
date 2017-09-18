/*
 * indextest_main.c - A test program for the index filters
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
#include <time.h>
#include <getopt.h>

#include "test.h"
#include "algorithms.h"
#include "song.h"
#include "util.h"


/**
 * Initializes a distance matrix.
 *
 * @param a matrix height
 * @param b matrix width
 *
 * @return the allocated, zero-filled matrix
 */
static float **init_matrix(int a, int b) {
    float **m;
    int i;
    m = (float **) malloc(a * sizeof(float *));
    for (i=0; i<a; ++i) {
        int j;
        m[i] = (float *) malloc(b * sizeof(float));
        for(j=0; j<b; ++j) m[i][j] = 0.0F;
    }
    return m;
}

/**
 * Frees a distance matrix.
 *
 * @param m the matrix to free
 * @param a matrix height
 */
static void free_matrix(float **m, int a) {
    int i;
    for (i=0; i<a; ++i) {
        if (m[i] != NULL) free(m[i]);
    }
    free(m);
}

/**
 * Writes a distance matrix to a file.
 *
 * @param p operation parameters
 * @param algorithm the search algorithm or index filter to use
 * @param sc a song collection
 * @param patterns searched patterns as a song collection
 */
static void write_distance_matrix(indextest_parameters *p, int algorithm,
        songcollection *sc, songcollection *patterns) {
    float **distances;
    FILE *f;
    int i;
    struct timeval start, end;
    double diff;
    matchset ms;

    init_match_set(&ms, sc->size, 0, 0);
    distances = init_matrix(patterns->size, sc->size);

    /* Append to the file */
    f = fopen(p->distance_matrix_file, "a");
    if (!f) {
        fprintf(stderr, "\nError: Unable to write to file: %s",
                p->distance_matrix_file);
        free_matrix(distances, patterns->size);
        free_match_set(&ms);
        return;
    }

    gettimeofday(&start, NULL);

    if (p->verbose)
        fprintf(stderr, "\nCalculating distances with algorithm %s\n",
                get_algorithm_name(algorithm));
    for (i=0; i<patterns->size; ++i) {
        int j;
        clear_match_set(&ms);
        for (j=0; j<sc->size; ++j)
            distances[i][j] = 0.0F;

        if (p->verbose)
            fprintf(stderr, "%d: %s\n", i, patterns->songs[i].title);

        search(sc, &patterns->songs[i], algorithm,
                &p->search_parameters, &ms);

        for (j=0; j<ms.num_matches; ++j)
            distances[i][ms.matches[j].song] = -ms.matches[j].similarity;
    }
    free_match_set(&ms);

    gettimeofday(&end, NULL);

    /* Write data to the file */

    fputs(get_algorithm_name(algorithm), f);
    diff = timediff(&end, &start);
    fprintf(f, "\n%f\t%f\n", diff, diff / (double) patterns->size);
 
    for (i=0; i<sc->size; i++) {
        if (sc->songs[i].title != NULL) fputs(sc->songs[i].title, f);
        else fputs("NO TITLE", f);
        fputs("\n", f);
    }

    fputs("<DISTANCES>\t<DISTANCES>\n", f);
    for (i=0; i<patterns->size; ++i) {
        int j;
        for (j=0; j<sc->size; ++j)
            fprintf(f, "%d\t%d\t%f\n", i, j, distances[i][j]);
    }

    fclose(f);
    free_matrix(distances, patterns->size);
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
    songcollection scollection;
    songcollection pcollection;
    matchset pmatches;
    indextest_parameters p;
    int i;

    indextest_init_parameters(&p);
    if (!indextest_parse_arguments(argc, argv, &p)) return 1;

    if (p->distance_matrix_file == NULL) {
        indextest_run_test(&p, indextest_measure_searchtime);
    } else {
        indextest_run_test(&p, write_distance_matrix);
    }

    indextest_free_parameters(&p);

    if (p.verbose) fputs(" Done.\n", stderr);

    return 0;
}

