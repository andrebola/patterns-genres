/*
 * align.h - Symbolic score alignment structures and definitions.
 *
 * Version 2008-02-29
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

#ifndef __ALIGN_H__
#define __ALIGN_H__

#include "config.h"
#include "midifile.h"

#ifdef __cplusplus
extern "C" {
#endif


/**
 * Alignment parameters.
 */
typedef struct {
    int verbose;
    char *program_name;

    int algorithm;
    int mapping_algorithm;
    int search_algorithm;

    char *pattern_path;
    char *song_path;
    char *output_midi;
    char *output_map;
    int max_pattern_size;

    float max_slope;
    int skip_cost;
    int map_accuracy;
    int p3_pattern_window_size;
    int p3_num_scales;
    float *p3_scales;

    float song_tempo;
    int delay;
} alignparameters;


/**
 * Alignment line within an alignment map.
 */
typedef struct {
    int pattern_position;
    int pattern_time;
    int target_time;
    float initial_slope;
    unsigned char *values;
    char *transpositions;
} alignmentline;


/**
 * Alignment map: a similarity matrix for all possible unscaled
 * alignments of small song segments.
 */
typedef struct {
    int width;
    int height;
    int accuracy;
    alignmentline *lines;
    unsigned char *vbuffer;
    char *tbuffer;
    int target_duration;
    int pattern_duration;
    const song *target;
    const song *pattern;
} alignmentmap;


typedef struct {
    int size;
    int *pattern_times;
    int *target_times;
    unsigned char *quality;
    const song *pattern;
    const song *target;
} alignment;


void align(const songcollection *sc, const song *s, const song *p, int alg,
        const alignparameters *parameters, alignment *initial_align,
	alignment *result_align);

void map_alignments(const songcollection *sc, const song *s, const song *p,
        int alg, const alignparameters *parameters, alignmentmap *map);

alignmentmap *init_alignmentmap(void);

void free_alignmentmap(alignmentmap *map);

void write_alignmentmap_pgm(const char *filename, const alignmentmap *map,
        const alignment *a, int invert, int fill_gaps);

void calculate_initial_alignment(const alignparameters *parameters,
        const alignmentmap *map, alignment *a);

void normalize_alignmentmap(alignmentmap *map, int avoid_clipping);

void align_midi_song(midisong *midi_s, alignment *a);

void align_init_parameters(alignparameters *p);

void align_free_parameters(alignparameters *p);

alignment *init_alignment(void);

void clear_alignment(alignment *a);

void free_alignment(alignment *a);

int align_parse_arguments(int argc, char **argv, alignparameters *p);

void delay_alignment(alignment *a, int delay);

void print_alignment(alignment *a);

#ifdef __cplusplus
}
#endif

#endif

