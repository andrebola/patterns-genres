/*
 * align.c - Symbolic song synchronization functions.
 *
 * Version 2008-02-29
 *
 *
 * Copyright (C) 2008 Niko Mikkila
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
#include <assert.h>

#include "config.h"
#include "midifile.h"
#include "priority_queue.h"
#include "test.h"
#include "util.h"

#include "align.h"
#include "align_P3.h"
#include "song_window.h"

/** Skip percussion tracks when converting MIDI files to the internal formats */
#define SKIP_PERCUSSION 1

/*#define SKIP_LINES 10*/
#define SKIP_LINES 10


/*#define KEEP_SLOPE 1

#define SLOPE_UPDATE 0.3F*/

/* #define KEEP_TRANSPOSITION 1 */

static float P3_SCALES[][7] = {
    {1.0F,  0.0F,  0.0F,  0.0F,  0.0F,  0.0F,  0.0F},
    {1.0F,  1.5F,  0.0F,  0.0F,  0.0F,  0.0F,  0.0F},
    {0.66F, 1.0F,  1.5F,  0.0F,  0.0F,  0.0F,  0.0F},
    {0.66F, 1.0F,  1.5F,  2.25F, 0.0F,  0.0F,  0.0F},
    {0.44F, 0.66F, 1.0F,  1.5F,  2.25F, 0.0F,  0.0F},
    {0.44F, 0.66F, 1.0F,  1.2F,  1.5F,  2.25F, 0.0F},
    {0.44F, 0.66F, 0.83F, 1.0F,  1.2F,  1.5F,  2.25F}
};


static void (* const ALIGNMENT_FUNCTIONS[])(const songcollection *sc,
        const song *s, const song *p, int alg,
        const alignparameters *parameters, const alignment *initial_align,
        alignment *result_align) = {
          NULL,
/*  1 */  NULL, 
/*  2 */  NULL, 
/*  3 */  NULL, 
/*  4 */  NULL, 
/*  5 */  NULL, 
/*  6 */  NULL, 
/*  7 */  NULL, 
/*  8 */  NULL, 
/*  9 */  NULL, 
/* 10 */  NULL, 
/* 11 */  NULL, 
/* 12 */  NULL, 
/* 13 */  NULL, 
/* 14 */  NULL, 
/* 15 */  NULL, 
/* 16 */  NULL, 
/* 17 */  NULL, 
/* 18 */  NULL, 
/* 19 */  NULL, 
/* 20 */  NULL, 
/* 21 */  NULL, 
/* 22 */  NULL, 
/* 23 */  NULL, 
/* 24 */  NULL, 
/* 25 */  NULL, 
/* 26 */  NULL, 
/* 27 */  NULL,
};

static void (* const ALIGNMENT_MAP_FUNCTIONS[])(const songcollection *sc,
        const song *s, const song *p, int alg,
        const alignparameters *parameters, alignmentmap *map) = {
          NULL,
/*  1 */  NULL, 
/*  2 */  NULL, 
/*  3 */  NULL, 
/*  4 */  NULL, 
/*  5 */  NULL, 
/*  6 */  NULL, 
/*  7 */  NULL, 
/*  8 */  NULL, 
/*  9 */  NULL, 
/* 10 */  NULL, 
/* 11 */  NULL, 
/* 12 */  NULL, 
/* 13 */  NULL, 
/* 14 */  NULL, 
/* 15 */  NULL, 
/* 16 */  NULL, 
/* 17 */  NULL, 
/* 18 */  NULL, 
/* 19 */  NULL, 
/* 20 */  NULL, 
/* 21 */  NULL, 
/* 22 */  NULL, 
/* 23 */  NULL, 
/* 24 */  NULL, 
/* 25 */  NULL, 
/* 26 */  NULL, 
/* 27 */  align_p3, 
};



/** Command-line argument IDs (also used as short arguments) */

#define ALIGN_ARG_HELP               'h'
#define ALIGN_ARG_ALGORITHM          'a'
#define ALIGN_ARG_MAPPING_ALGORITHM  'A'
#define ALIGN_ARG_SEARCH_ALGORITHM   's'
#define ALIGN_ARG_OUTPUT_MIDI        'o'
#define ALIGN_ARG_OUTPUT_PGM_MAP     'm'
#define ALIGN_ARG_MAX_SLOPE          'S'
#define ALIGN_ARG_SKIP_COST          'c'
#define ALIGN_ARG_MAP_ACCURACY       'M'
#define ALIGN_ARG_PATTERN            'p'
#define ALIGN_ARG_MAX_PATTERN_SIZE   'n'
#define ALIGN_ARG_P3_WINDOW_SIZE     'w'
#define ALIGN_ARG_P3_NUM_SCALES      'x'
#define ALIGN_ARG_SONG_TEMPO         't'
#define ALIGN_ARG_ALIGNMENT_DELAY    'd'


static const struct option LONG_OPTIONS[] = {
    {"help",                no_argument,        0, ALIGN_ARG_HELP},
    {"algorithm",           required_argument,  0, ALIGN_ARG_ALGORITHM},
    {"mapping-algorithm",   required_argument,  0, ALIGN_ARG_MAPPING_ALGORITHM},
    {"search-algorithm",    required_argument,  0, ALIGN_ARG_SEARCH_ALGORITHM},
    {"output-midi",         required_argument,  0, ALIGN_ARG_OUTPUT_MIDI},
    {"output-pgm-map",      required_argument,  0, ALIGN_ARG_OUTPUT_PGM_MAP},
    {"max-slope",           required_argument,  0, ALIGN_ARG_MAX_SLOPE},
    {"skip-cost",           required_argument,  0, ALIGN_ARG_SKIP_COST},
    {"map-accuracy",        required_argument,  0, ALIGN_ARG_MAP_ACCURACY},
    {"pattern",             required_argument,  0, ALIGN_ARG_PATTERN},
    {"max-pattern-size",    required_argument,  0, ALIGN_ARG_MAX_PATTERN_SIZE},
    {"p3-window-size",      required_argument,  0, ALIGN_ARG_P3_WINDOW_SIZE},
    {"p3-num-scales",       required_argument,  0, ALIGN_ARG_P3_NUM_SCALES},
    {"song-tempo",          required_argument,  0, ALIGN_ARG_SONG_TEMPO},
    {"delay",               required_argument,  0, ALIGN_ARG_ALIGNMENT_DELAY},
    {0, 0, 0, 0}
};


/**
 * Prints program usage information.
 *
 * @param p default parameter values
 */
static void align_print_usage(const alignparameters *p) {
    puts("\nTest program for symbolic score alignment algorithms. Finds a matching song");
    puts("from the given song collection and aligns it with the pattern\n");

    printf("Syntax: %s [options] -p <song> <song collection> \n\n", p->program_name);

    puts(  "Where song collection is a single MIDI file or a directory of MIDI files\n");

    puts(  "Options:\n");

    fputs( "  -p, --pattern [filename]          Pattern song\n", stdout);

    fputs( "  -n, --max-pattern-size [int]      Maximum pattern size\n", stdout);

    fputs( "  -o, --output-midi [path]          Write aligned MIDI song to this file [no]\n", stdout);

    fputs( "  -m, --output-pgm-map [path]       Output alignment map as a PGM image [no]\n", stdout);

    fputs( "  -a, --algorithm [string]          Alignment algorithm [AP3]\n", stdout);
    fputs( "  -A, --mapping-algorithm [string]  Algorithm used for alignment map\n", stdout);
    fputs( "                                    generation [AP3]\n", stdout);

    fputs( "  -s, --search-algorithm [string]   Search algorithm [P3]\n", stdout);

    fputs( "  -S, --max-slope [float]           Max alignment time slope [3.0]\n", stdout);

    fputs( "  -c, --skip-cost [int]             Alignment skip cost [400]\n", stdout);

    fputs( "  -M, --map-accuracy [int]          Map accuracy in milliseconds [200]\n", stdout);

    fputs( "  -w, --p3-window-size [int]        AP3 window size [2000]\n", stdout);
    fputs( "  -x, --p3-num-scales [int]         Number of scale variations scanned [1]\n", stdout);

    fputs( "  -t, --song-tempo [float]          Adjust song tempo [1.0]\n", stdout);

    fputs( "  -d, --delay [int]                 Alignment delay in milliseconds [0]\n\n", stdout);
}


/**
 * Initializes the given parameter struct with default values.
 *
 * @param p parameter struct
 */
void align_init_parameters(alignparameters *p) {
    p->verbose = 1;
    p->program_name = NULL;
    p->algorithm = ALG_ALIGN_P3;
    p->mapping_algorithm = ALG_ALIGN_P3;
    p->search_algorithm = ALG_P3;
    p->pattern_path = NULL;
    p->max_pattern_size = -1;
    p->output_midi = NULL;
    p->output_map = NULL;
    p->song_path = NULL;
    p->max_slope = 3.0F;
    p->skip_cost = 400;
    p->map_accuracy = 100;
    p->p3_pattern_window_size = 2000;

    p->p3_num_scales = 1;
    p->p3_scales = P3_SCALES[0];

    p->song_tempo = 1.0F;
    p->delay = 0;
}

void align_free_parameters(alignparameters *p) {
    /*free(p->p3_scales);*/
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
int align_parse_arguments(int argc, char **argv, alignparameters *p) {
    int i, j, option_index = 0, ret = 1;
    char *short_options = make_short_options(LONG_OPTIONS);

    if (!short_options) return 0;

    p->program_name = argv[0];

    while ((i = getopt_long(argc, argv, short_options,
            LONG_OPTIONS, &option_index)) != -1) {
        switch (i) {
            case ALIGN_ARG_HELP:
                align_print_usage(p);
                ret = 0;
                goto EXIT;
            case ALIGN_ARG_ALGORITHM:
                j = get_algorithm_id(optarg);
                if (j == 0) {
                    print_algorithms();
                    ret = 0;
                    goto EXIT;
                } else p->algorithm = j;
                break;
            case ALIGN_ARG_MAPPING_ALGORITHM:
                j = get_algorithm_id(optarg);
                if (j == 0) {
                    print_algorithms();
                    ret = 0;
                    goto EXIT;
                } else p->mapping_algorithm = j;
                break;
            case ALIGN_ARG_SEARCH_ALGORITHM:
                j = get_algorithm_id(optarg);
                if (j == 0) {
                    print_algorithms();
                    ret = 0;
                    goto EXIT;
                } else p->search_algorithm = j;
                break;
            case ALIGN_ARG_PATTERN:
                p->pattern_path = optarg;
                break;
            case ALIGN_ARG_OUTPUT_MIDI:
                p->output_midi = optarg;
                break;
            case ALIGN_ARG_OUTPUT_PGM_MAP:
                p->output_map = optarg;
                break;
            case ALIGN_ARG_MAX_SLOPE:
                p->max_slope = MAX2(1.0F, atof(optarg));
                break;
            case ALIGN_ARG_SKIP_COST:
                p->skip_cost = MAX2(0, atoi(optarg));
                break;
            case ALIGN_ARG_MAP_ACCURACY:
                p->map_accuracy = MAX2(1, atoi(optarg));
                break;
            case ALIGN_ARG_P3_WINDOW_SIZE:
                p->p3_pattern_window_size = MAX2(2, atoi(optarg));
                break;
            case ALIGN_ARG_P3_NUM_SCALES:
                p->p3_num_scales = MIN2(7, MAX2(1, atoi(optarg)));
                p->p3_scales = P3_SCALES[p->p3_num_scales-1];
                break;

            case ALIGN_ARG_SONG_TEMPO:
                p->song_tempo = atof(optarg);
                break;
            case ALIGN_ARG_ALIGNMENT_DELAY:
                p->delay = atoi(optarg);
                break;
            case ALIGN_ARG_MAX_PATTERN_SIZE:
                p->max_pattern_size = atoi(optarg);
                break;

            default:
                ret = 0;
                goto EXIT;
        }
    }

    /* Parse remaining non-option arguments */
    if (optind < argc) {
        p->song_path = argv[optind];
    }

EXIT:
    free(short_options);
    return ret;
}


/**
 * Aligns a pattern with a song.
 *
 * @param sc the song collection
 * @param s song to align the pattern with. If either of the aligned songs
 *        is the "correct" score, it should be given here to improve alignment
 *        results of the P3 algorithm
 * @param p a pattern song that is aligned with the other song. This should
 *        be the piece that may contain more errors.
 * @param alg the algorithm ID as defined in algorithms.h
 * @param parameters alignment parameters for different algorithms
 * @param initial_align initial alignment. See calculate_initial_alignment().
 * @param result_align resulting alignment
 */
void align(const songcollection *sc, const song *s, const song *p, int alg,
        const alignparameters *parameters, alignment *initial_align,
        alignment *result_align) {

    if ((alg <= 0) || (alg > NUM_ALGORITHMS)) {
        fprintf(stderr, "Error in align: Undefined algorithm %d\n", alg);
        return;
    }
    if (ALIGNMENT_FUNCTIONS[alg] != NULL) {
        ALIGNMENT_FUNCTIONS[alg](sc, s, p, alg, parameters, initial_align,
                result_align);
    } else {
        fprintf(stderr, "Error in align: No alignment function defined for algorithm %d\n", alg);
    }
}


/**
 * Maps all local alignments of two songs.
 *
 * @param sc the song collection
 * @param s song to align the pattern with. If either of the aligned songs
 *        is the "correct" score, it should be given here to improve alignment
 *        results of the P3 algorithm
 * @param p a pattern song that is aligned with the other song. This should
 *        be the piece that may contain more errors.
 * @param alg the algorithm ID as defined in algorithms.h
 * @param parameters alignment parameters for different algorithms
 * @param map resulting alignment map will be stored here
 */
void map_alignments(const songcollection *sc, const song *s, const song *p,
        int alg, const alignparameters *parameters, alignmentmap *map) {

    if ((alg <= 0) || (alg > NUM_ALGORITHMS)) {
        fprintf(stderr, "Error in map_alignments: Undefined algorithm %d\n",
                alg);
        return;
    }
    if (ALIGNMENT_MAP_FUNCTIONS[alg] != NULL) {
        ALIGNMENT_MAP_FUNCTIONS[alg](sc, s, p, alg, parameters, map);
    } else {
        fprintf(stderr, "Error in map_alignments: No alignment function defined for algorithm %d\n", alg);
    }
}



alignmentmap *init_alignmentmap(void) {
    alignmentmap *map = (alignmentmap *) calloc(1, sizeof(alignmentmap));
    return map;
}

void free_alignmentmap(alignmentmap *map) {
    free(map->lines);
    free(map->vbuffer);
    free(map->tbuffer);
    free(map);
}

alignment *init_alignment(void) {
    alignment *a = (alignment *) calloc(1, sizeof(alignment));
    return a;
}

void clear_alignment(alignment *a) {
    free(a->pattern_times);
    free(a->target_times);
    free(a->quality);
    memset(a, 0, sizeof(alignment));
}

void free_alignment(alignment *a) {
    free(a->pattern_times);
    free(a->target_times);
    free(a->quality);
    free(a);
}



/**
 * Writes the given alignment map to disk as a PGM image file.
 *
 * @param filename output file path
 * @param map alignment map to write
 * @param a alignment to overlay on the image. Use NULL to only output the map.
 * @param invert set to 1 to invert the image colors
 * @param fill_gaps set to 1 to fill empty image lines with previous rows.
 */
void write_alignmentmap_pgm(const char *filename, const alignmentmap *map,
        const alignment *a, int invert, int fill_gaps) {
    unsigned char *buffer;
    int width, height;
    int i, pos, starttime, endtime, line, last_i;
    alignmentline *mapline;

    if (map->height <= 0) return;

    width = map->width;
    height = map->pattern_duration / map->accuracy;
    buffer = (unsigned char *) calloc(width * height, sizeof(unsigned char));
    if (buffer == NULL) {
        fputs("Error in write_alignmentmap_pgm: failed to allocate memory for image buffer\n", stderr);
        return;
    }
    pos = width * (height - 1);
    starttime = 0;
    endtime = map->accuracy;
    mapline = &map->lines[0];
    line = 0;
    last_i = -1;
    for (i=height-1; i>=0; --i) {
        if (fill_gaps) {
            int j;
            for (j=last_i-1; j>i; --j) {
                int k, end;
                end = pos + width + width;
                for (k=pos+width+1; k<end; ++k) {
                    buffer[k] = buffer[k+width-1];
                }
            }
            last_i = i + 1;
        }
        while (mapline->pattern_time < endtime) {
            int j, k;
            unsigned char *mapvalues = mapline->values;
            /*char *maptransp = (char *) mapline->transpositions;*/
            for (j=pos, k=0; j<pos+width; ++j, ++k) {
                buffer[j] = MAX2(buffer[j], mapvalues[k]);
                /*buffer[j] = (unsigned char) (maptransp[k] + 128);*/
            }
            ++line;
            last_i = i;
            if (line >= map->height) break;
            else mapline = &map->lines[line];
        }
        if (line >= map->height) break;
        pos -= width;
        starttime += map->accuracy;
        endtime += map->accuracy;
    }

    if (a != NULL) {
        float rres = 1.0F / (float) map->accuracy;
        for (i=0; i<a->size; ++i) {
            int x = (int) (rres * (float) a->target_times[i]);
            int y = height - 1 - (int) (rres * (float) a->pattern_times[i]);
            if ((x < 0) || (y < 0) || (x >= width) || (y >= height)) continue;
            buffer[y*width + x] = 0;
        }
    }

    if (invert) {
        int size = width * height;
        for (i=0; i<size; ++i) {
            buffer[i] = 255 - buffer[i];
        }
    }
    write_pgm(filename, buffer, width, height);
    free(buffer);
}


/**
 * Calculates an initial alignment from an alignment map.
 *
 * @param parameters alignment parameters
 * @param map alignment map. See map_alignments().
 * @param a the calculated initial alignment
 */
void calculate_initial_alignment(const alignparameters *parameters,
        const alignmentmap *map, alignment *a) {
    int i, p, mp, skiplines;
    int width = map->width;
    int height = map->height;
    int *lines, *sum, *lastsum, *lastline, *stationarytime;
    unsigned char *mvalues = map->lines[0].values;
    int max = 0, maxpos = 0;
    float rres = 1.0F / (float) parameters->map_accuracy;
    float slope = rres * parameters->max_slope;
    float rslope = rres / parameters->max_slope;
    int skip_cost = (int) (rres * 1000.0F * (float) parameters->skip_cost);
    int max_stationarytime = parameters->max_slope * parameters->map_accuracy;
#ifdef KEEP_SLOPE
    float *lineslope, *lastlineslope;
    int slope_depth = parameters->max_slope + 2;
#endif

    if (height == 0) return;

    lines = (int *) malloc(width * height * sizeof(int));
    sum = (int *) malloc(width * sizeof(int));
    lastsum = (int *) malloc(width * sizeof(int));
    stationarytime = (int *) malloc(width * sizeof(int));
#ifdef KEEP_SLOPE
    lineslope = (float *) malloc(width * sizeof(float));
    lastlineslope = (float *) malloc(width * sizeof(float));
#endif
    lastline = &lines[0];

    if ((lines == NULL) || (sum == NULL) || (lastsum == NULL) ||
            (stationarytime == NULL)
#ifdef KEEP_SLOPE
            || (lineslope == NULL) || (lastlineslope == NULL)
#endif
            ) {
        fputs("Error in calculate_initial_alignment: unable to allocate memory for buffers", stderr);
        free(lines);
        free(sum);
        free(lastsum);
        free(stationarytime);
#ifdef KEEP_SLOPE
        free(lineslope);
        free(lastlineslope);
#endif
        return;
    }

    for (i=0; i<width; ++i) {
        lastsum[i] = mvalues[i];
        lastline[i] = -1;
        stationarytime[i] = 0;
#ifdef KEEP_SLOPE
        lastlineslope[i] = 1.0F;
#endif
        if (lastsum[i] > max) {
            max = lastsum[i];
            maxpos = i;
        }
    }
    max = max - skip_cost;
    for (i=1; i<height; ++i) {
        int j, lstart;
        int newmax = 0;
        int newmaxpos = -1;
        int *line = &lines[i * width];
        float dt = map->lines[i].pattern_time - map->lines[i-1].pattern_time;
        int w1 = (int) (dt * rslope);
        int w2 = (int) (dt * slope);
        int pqsize = MAX2(1, w2 - w1 + 1);
        int n = 0;
#ifdef KEEP_TRANSPOSITION
        char *transp = map->lines[i].transpositions;
        char *lasttransp = map->lines[i-1].transpositions;
#endif
        pqroot *pq = pq_create(pqsize);

        mvalues = map->lines[i].values;

        /* The first entries of each map line may not have predecessors */
        if ((w1 < width) && ((stationarytime[w1] + dt) >= max_stationarytime))
            lstart = w1+1;
        else lstart = w1;
        for (j=0; j<MIN2(lstart, width); ++j) {
            sum[j] = mvalues[j];
            line[j] = -1;
            stationarytime[j] = 0;
#ifdef KEEP_SLOPE
            lineslope[j] = 1.0F;
#endif
        }
        for (j=lstart; j<width; ++j) {
            int newsum;
            int newpos;
            if ((stationarytime[j] + dt) < max_stationarytime) {
                pqnode *node;

                /* Add a new value to the window queue */
                node = pq_getnode(pq, n);
                node->key1 = INT_MAX - 1 - lastsum[j-w1];
                node->key2 = j-w1;
                pq_update_key1_p3(pq, node);

                /* Get maximum sum within the window */
                node = pq_getmin(pq);
                newsum = INT_MAX - 1 - node->key1;
                newpos = node->key2;
            } else {
                pqnode *node;

                /* Get maximum sum within the window */
                node = pq_getmin(pq);
                newsum = INT_MAX - 1 - node->key1;
                newpos = node->key2;

                /* Add a new value to the window queue */
                node = pq_getnode(pq, n);
                node->key1 = INT_MAX - 1 - lastsum[j-w1];
                node->key2 = j-w1;
                pq_update_key1_p3(pq, node);
            }
            if (newsum < max) {
                sum[j] = max + mvalues[j];
                line[j] = -2 - maxpos;
#ifdef KEEP_SLOPE
                lineslope[j] = lastlineslope[maxpos];
#endif
            } else {
                sum[j] = mvalues[j];
#ifdef KEEP_TRANSPOSITION
                if (ABS((int) transp[j] - (int) lasttransp[newpos]) >
                        KEEP_TRANSPOSITION) {
                    sum[j] = 0;
                }
#endif
#ifdef KEEP_SLOPE
                {
                float slp, slpmul;
                int depth;
                int *ll = lastline;
                int lpos = newpos;
                for (depth=0; (depth<slope_depth) && (i-depth-1>=0) && (ll >= lines); ++depth) {
                    if (ll[lpos] < 0) break;
                    lpos = ll[lpos];
                    ll -= width * sizeof(int);
                }
                slpmul = (float) (map->lines[i].pattern_time -
                        map->lines[i-depth].pattern_time);
                slp = (float) (j - lpos);
                if ((slpmul > 0.0F) && (slp > 0.0F)) slp = rres * slpmul / slp;
                else slp = 1.0;
                slpmul = slp / lastlineslope[newpos];
                if (slpmul > 1.0F) slpmul = 1.0F / slpmul;
                /*printf("pos: %d  slp: %f, slpmul: %f\n", j, slp, slpmul);*/
                sum[j] = slpmul * (float) sum[j];
                lineslope[j] = lastlineslope[newpos] +
                        SLOPE_UPDATE * (slp - lastlineslope[newpos]);
                }
#endif
                sum[j] += newsum;
                line[j] = newpos;
            }

            if (newpos == j) stationarytime[j] += (int) dt;
            else stationarytime[j] = 0;

            if (sum[j] > newmax) {
                newmax = sum[j];
                newmaxpos = j;
            }
            ++n;
            if (n == pqsize) n = 0;
        }
        max = newmax - skip_cost;
        maxpos = newmaxpos;
        VOIDPTR_SWAP(sum, lastsum);
#ifdef KEEP_SLOPE
        VOIDPTR_SWAP(lineslope, lastlineslope);
#endif
        lastline = line;
        pq_free(pq);
    }

    free(sum);
    free(lastsum);
    free(stationarytime);
#ifdef KEEP_SLOPE
    free(lineslope);
    free(lastlineslope);
#endif

    /* Calculate the number of alignment points */
    p = (height - 1) * width;
    skiplines = 0;
    a->size = 0;
    i = maxpos;
    for (mp=height-1; (mp>=0); --mp,p-=width) {
        int tt;
        if (lines[p+maxpos] < -1) {
            tt = -1 - maxpos * map->accuracy;
            skiplines = 0;
        } else tt = maxpos * map->accuracy;
        if (skiplines <= 0) {
            skiplines = SKIP_LINES;
            ++a->size;
        }
        maxpos = lines[p + maxpos];
        if (maxpos == -1) {
            break;
        } else if (maxpos < -1) maxpos = -2 - maxpos;
        --skiplines;
    }
    maxpos = i;

    a->pattern = map->pattern;
    a->target = map->target;
    a->pattern_times = (int *) calloc(a->size, sizeof(int));
    a->target_times = (int *) calloc(a->size, sizeof(int));
    a->quality = (unsigned char *) calloc(a->size,
            sizeof(unsigned char));

    p = (height - 1) * width;
    skiplines = 0;
    for (mp=height-1,i=a->size-1; (mp>=0) && (i>=0); --mp,p-=width) {
        int tt;
        if (lines[p+maxpos] < -1) {
            tt = -1 - maxpos * map->accuracy;
            skiplines = 0;
        } else tt = maxpos * map->accuracy;
        if (skiplines <= 0) {
            a->pattern_times[i] = map->lines[mp].pattern_time;
            a->target_times[i] = tt;
            a->quality[i] = map->lines[mp].values[maxpos];
            skiplines = SKIP_LINES;
            --i;
        }
        maxpos = lines[p + maxpos];
        if (maxpos == -1) {
            fputs("Warning in calculate_initial_alignment: unexpected end of alignment line\n", stderr);
            break;
        } else if (maxpos < -1) maxpos = -2 - maxpos;
        --skiplines;
    }
    free(lines);
}


/**
 * Normalizes first each column and then each row of an alignment map to have
 * a mean value of 128 (there will be variation if values are clipped).
 *
 * @param map the alignment map to normalize
 * @param avoid_clipping set to 1 to avoid clipping the highest and lowest
 *        values
 */
void normalize_alignmentmap(alignmentmap *map, int avoid_clipping) {
    int i;
    int width = map->width;
    int height = map->height;
    int mapsize = width * height;
    unsigned char *buffer = map->vbuffer;

    /* Column-wise normalization: shift the values in each column so that
     * the mean is 128 */
    for (i=0; i<width; ++i) {
        int pos = i;
        int min = 255;
        int max = 0;
        int adjust;
        int colmean = 0;
        while (pos < mapsize) {
            colmean += buffer[pos];
            min = MIN2(buffer[pos], min);
            max = MAX2(buffer[pos], max);
            pos += width;
        }
        colmean = colmean / height;
        adjust = (int) (128 - colmean);

        pos = i; 
        if (avoid_clipping && ((min + adjust < 0) || (max + adjust > 255))) {
            float scale = 127.0F / (127.0F + (float) ABS(adjust));
            while (pos < mapsize) {
                buffer[pos] = MIN2(255, MAX2(0, (int) (((float) buffer[pos] -
                        (float) colmean) * scale + 127.0F)));
                pos += width;
            }
        } else {
            while (pos < mapsize) {
                buffer[pos] = MIN2(255, MAX2(0, ((int) buffer[pos]) + adjust));
                pos += width;
            }
        }
    }

    /* Row-wise normalization: shift the values in each row so that
     * the mean is 128 */
    i = 0;
    while (i < mapsize) {
        int lastpos = i + width;
        int min = 255;
        int max = 0;
        int adjust;
        int rowmean = 0;
        while (i < lastpos) {
            rowmean += buffer[i];
            min = MIN2(buffer[i], min);
            max = MAX2(buffer[i], max);
            ++i;
        }
        rowmean = rowmean / width;
        adjust = (int) (128 - rowmean);

        i -= width;
        if (avoid_clipping && ((min + adjust < 0) || (max + adjust > 255))) {
            float scale = 127.0F / (127.0F + (float) ABS(adjust));
            while (i < lastpos) {
                buffer[i] = MIN2(255, MAX2(0, (int) (((float) buffer[i] -
                        (float) rowmean) * scale + 127.0F)));
                ++i;
            }
        } else {
            while (i < lastpos) {
                buffer[i] = MIN2(255, MAX2(0, ((int) buffer[i]) + adjust));
                ++i;
            }
        }
    }
}

/**
 * Adjusts song tempo by a given scale ratio.
 *
 * @param scale tempo ratio. For example 3.0 triples the tempo, 0.5 halves it.
 */
static void adjust_song_tempo(song *s, float scale) {
    vector *notes = s->notes;
    int i;
    for (i=0; i<s->size; ++i) {
        notes[i].strt = (int) ((float) notes[i].strt * scale);
        notes[i].dur = (int) ((float) notes[i].dur * scale);
    }
}


static INLINE void midi_song_init_event_queue(midisong *midi_s,
        pqroot *eventqueue, int *track_position) {
    int i;
    int num_tracks = midi_s->num_tracks;
    for (i=0; i<num_tracks; ++i) {
        pqnode *node = pq_getnode(eventqueue, i);
        if (midi_s->track_size[i] > 0) {
            track_event *te = &midi_s->track_data[i][0];
            node->key1 = (int) te->tick;
            node->pointer = te;
        } else {
            node->key1 = INT_MAX;
            node->pointer = NULL;
        }
        node->key2 = i;
        pq_update_key1_p3(eventqueue, node);
        track_position[i] = 0;
    }
}

static INLINE void midi_song_get_next_event(midisong *midi_s,
        pqroot *eventqueue, int *track_position, track_event **te, int *track) {
    pqnode *node = pq_getmin(eventqueue);
    int i = node->key2;
    *track = i;
    *te = (track_event *) node->pointer;
    if ((i<0) || (i >= midi_s->num_tracks)) return;
    ++track_position[i];
    if (track_position[i] < midi_s->track_size[i]) {
        track_event *e = &midi_s->track_data[i][track_position[i]];
        node->key1 = (int) e->tick;
        node->pointer = e;
    } else {
        node->key1 = INT_MAX;
        node->pointer = NULL;
    }
    pq_update_key1_p3(eventqueue, node);
}


/**
 * Aligns a MIDI song by the given alignment data.
 *
 * @param midi_s MIDI song to align
 * @param a the alignment to use
 */
void align_midi_song(midisong *midi_s, alignment *a) {
    int i, apos;
    int skips = 0;
    int num_tracks = midi_s->num_tracks;
    int *track_position;
    int *ebuf_position;
    int *pattern_times = a->pattern_times;
    int *target_times = a->target_times;
    track_event **ebuf;
    pqroot *eventqueue;
    double slope = 1.0;

    /* Count skips */
    for (i=0; i<a->size; ++i) {
        if (a->target_times[i] < 0) {
            ++skips;
        }
    }
    ebuf = (track_event **) malloc(num_tracks * sizeof(track_event *));
    ebuf_position = (int *) calloc(num_tracks, sizeof(int));
    track_position = (int *) malloc(num_tracks * sizeof(int));
    if ((ebuf == NULL) || (ebuf_position == NULL) || (track_position == NULL)) {
        fputs("Error in align_midi_song(): Failed to allocate memory for track array\n", stderr);
        free(ebuf);
        free(ebuf_position);
        free(track_position);
        return;
    }
    for (i=0; i<num_tracks; ++i) {
        ebuf[i] = (track_event *) malloc((midi_s->track_size[i]+2) *
                sizeof(track_event) * (skips + 1));
        if (ebuf[i] == NULL) {
            for (--i; i>=0; --i) free(ebuf[i]);
            free(ebuf);
            free(ebuf_position);
            free(track_position);
            fputs("Error in align_midi_song(): Failed to allocate memory for track event buffer\n", stderr);
            return;
        }
    }

    /* Create an event queue, sorted by event time */
    eventqueue = pq_create(num_tracks);

    
    /* Loop through all continuous alignment sequences that are separated
     * by skips in the target song. Skips are marked with negative time
     * stamps (-1-target_time). */
    apos = 0;
    while(apos < a->size) {
        int curtime = -1;
        int track;
        track_event *te = NULL;
        int target_time = target_times[apos];
        target_time = (target_time < 0) ? (-1 - target_time) : target_time;

        midi_song_init_event_queue(midi_s, eventqueue, track_position);

        /* Find the first event that is potentially within the window */
        while(curtime < target_time) {
            unsigned char status;
            midi_song_get_next_event(midi_s, eventqueue, track_position,
                    &te, &track);
            if (te == NULL) break;
            else curtime = te->tick;

            /* Keep record of state variations, such as program changes */
            status = te->status & STATUS_MASK;
            if ((status == EVENT_PROGRAM_CHANGE) ||
                    (status == EVENT_CONTROLLER) ||
                    ((status == EVENT_SYSEX) &&
                        ((te->status == EVENT_SYSEX) ||
                        (te->status == EVENT_ESCAPE)))) {
                track_event *e = &ebuf[track][ebuf_position[track]++];
                memcpy(e, te, sizeof(track_event));
                if (apos == 0) e->tick = 0;
                else e->tick = pattern_times[apos];
                
            /* Copy META events that are at the beginning of the track */
            } else if ((apos == 0) && (te->status == EVENT_META)) {
                track_event *e = &ebuf[track][ebuf_position[track]++];
                memcpy(e, te, sizeof(track_event));
                e->tick = 0.0;
            }
        }
/*printf("APOS: %d time: %d, mtime: %d\n", apos, target_time, curtime);*/
        /* End of the target song was encountered. Find the next skip,
         * if there is one */
        if (te == NULL) {
            while ((++apos < a->size) && (target_times[apos]) >= 0);
            continue;
        }
        /* Align target to the current alignment window sequence */
        do {
            int pattern_time = pattern_times[apos];
            int next_target_time = INT_MAX;
            /* Calculate time slope for the alignment window */
            while(++apos < a->size) {
                if (target_time < target_times[apos]) {
                    next_target_time = target_times[apos];
                    slope = (double) (pattern_times[apos] - pattern_time) /
                            (double) (target_times[apos] - target_time);
                    break;
                } else if (target_times[apos] < 0) {
                    next_target_time = target_time + (int) ((double)
                            (pattern_times[apos] - pattern_time) / slope);
                    break;
                }
            }
            if (apos >= a->size) {
                /* End. Insert end-of-track events. */
                for (track=0; track<num_tracks; ++track) {
                    track_event *e = &ebuf[track][ebuf_position[track]++];
                    e->tick = pattern_times[a->size-1];
                    e->status = EVENT_META;
                    e->metatype = EVENT_END_OF_TRACK;
                    e->data = NULL;
                    e->length = 0;
                }
                break;
            }
            /* Align events within the window and put them to the new buffer */
            while(curtime < next_target_time) {
                track_event *e = &ebuf[track][ebuf_position[track]++];
                memcpy(e, te, sizeof(track_event));
                e->tick = pattern_time + (int) (slope *
                        (double) (e->tick - target_time));
/*printf("  tick: %f (%f) curtime: %d, slope: %f, t1: %d, p1: %d\n", e->tick, te->tick, curtime, slope, target_time, pattern_time);*/
                midi_song_get_next_event(midi_s, eventqueue, track_position,
                        &te, &track);
                if (te == NULL) break;
                else curtime = te->tick;
            }
            if (te == NULL) break;
            target_time = next_target_time;
        } while((apos < a->size) && (target_times[apos] >= 0));
    }

    /* Replace old track event arrays with new ones */
    for (i=0; i<num_tracks; ++i) {
        free(midi_s->track_data[i]);
        midi_s->track_data[i] = (track_event *) malloc(ebuf_position[i] *
                sizeof(track_event));
        memcpy(midi_s->track_data[i], ebuf[i], ebuf_position[i] *
                sizeof(track_event));
        midi_s->track_size[i] = ebuf_position[i];
        free(ebuf[i]);
    }
    free(ebuf);
    free(ebuf_position);
    free(track_position);
    pq_free(eventqueue);
}

void delay_alignment(alignment *a, int delay) {
    int i;
    for (i=0; i<a->size; ++i) {
        if (a->target_times[i] > 0)
            a->target_times[i] = MAX2(0, (a->target_times[i] + delay));
        else
            a->target_times[i] = MIN2(-1, (a->target_times[i] + delay));
    }
}

void print_alignment(alignment *a) {
    int i;
    for (i=0; i<a->size; ++i) {
        printf("%d\t%d\t%d\n", a->pattern_times[i], a->target_times[i], a->quality[i]);
    }
}


int main(int argc, char **argv) {
    alignparameters p;
    songcollection sc, pc;
    int i;
    int algorithms[4];
    dataparameters data_parameters;
    struct timeval start, end;

    align_init_parameters(&p);
    if (!align_parse_arguments(argc, argv, &p)) return 1;
    if ((p.song_path == NULL) || (p.pattern_path == NULL)) {
        fputs("\nError: no pattern or songs specified\n\n", stderr);
        return 1;
    }

    algorithms[0] = p.algorithm;
    algorithms[1] = p.mapping_algorithm;
    algorithms[2] = p.search_algorithm;
    algorithms[3] = -1;

    data_parameters.c_window = 30;
    data_parameters.avindex_vector_max_width = 10000;
    data_parameters.avindex_vector_max_height = 128;

    init_song_collection(&sc, 0);
    init_song_collection(&pc, 0);

    /* Read songs */

    if (p.verbose >= LOG_IMPORTANT) fputs("Reading song files...\n", stderr);
    gettimeofday(&start, NULL);
    i = read_midi_directory(p.song_path, &sc, SKIP_PERCUSSION);
    if (i <= 0) {
        fprintf(stderr, "\nError: Unable to read songs from: %s\n\n",
                p.song_path);
        goto EXIT;
    }
    i = read_midi_directory(p.pattern_path, &pc, SKIP_PERCUSSION);
    if (i <= 0) {
        fprintf(stderr, "\nError: Unable to read songs from: %s\n\n",
                p.pattern_path);
        goto EXIT;
    }
    gettimeofday(&end, NULL);
    if (p.verbose >= LOG_IMPORTANT)
        fprintf(stderr, "Time: %f\n", timediff(&end, &start));


    /* Preprocess songs (convert, remove overlaps) */

    if (p.verbose >= LOG_IMPORTANT) fputs("\nPreprocessing...\n", stderr);
    gettimeofday(&start, NULL);

    preprocess_songs(&sc, 0, 0, -1);
    p3_optimize_song_collection(&sc, 100, 50);
    if (p.song_tempo != 1.0F) {
        for (i=0; i<sc.size; ++i) {
            adjust_song_tempo(&sc.songs[i], p.song_tempo);
        }
    }
    update_song_collection_statistics(&sc);
    update_song_collection_data(&sc, algorithms, &data_parameters);
    preprocess_songs(&pc, 0, 0, -1);
    p3_optimize_song_collection(&pc, 100, 50);
    update_song_collection_statistics(&pc);
    update_song_collection_data(&pc, algorithms, &data_parameters);

    gettimeofday(&end, NULL);

    if (p.verbose >= LOG_IMPORTANT)
        fprintf(stderr,"Time: %f\n", timediff(&end, &start));

    srand(1234);
    for (i=0; i<sc.size; ++i) {
        test_song_window(&sc.songs[i], 30, 30, 1, 30000);
    }

#if 0
    for (i=0; i<pc.size; ++i) {
        song *pattern = &pc.songs[i];
        song *target = &sc.songs[0];

        alignmentmap *map = init_alignmentmap();
        alignment *a = init_alignment();
        fprintf(stderr, "Aligning pattern #%d: %s\n", pattern->id,
                pattern->title);
        fprintf(stderr, "    With target: %s\n", target->title);
    
        if (p.max_pattern_size > 0) pattern->size = p.max_pattern_size;

        fputs("Generating alignment map...\n", stderr);
        gettimeofday(&start, NULL);
        map_alignments(&sc, target, pattern, p.mapping_algorithm, &p, map);
        normalize_alignmentmap(map, 0);
        gettimeofday(&end, NULL);
        fprintf(stderr, "Time: %f\n", timediff(&end, &start));

        fputs("Calculating alignment...\n", stderr);
        gettimeofday(&start, NULL);
        calculate_initial_alignment(&p, map, a);
        gettimeofday(&end, NULL);
        fprintf(stderr, "Time: %f\n", timediff(&end, &start));

        fputs("\nAlignment:\n", stderr);
        print_alignment(a);

        if (p.output_map != NULL) {
            fprintf(stderr, "\nWriting alignment map to file: %s\n\n", p.output_map);
            write_alignmentmap_pgm(p.output_map, map, a, 1, 1);
        }

        delay_alignment(a, p.delay);

        if (p.output_midi != NULL) {
            midisong midi_s;
            read_midi_file(target->title, NULL, &midi_s, 0);
            align_midi_song(&midi_s, a);
            write_midi_file(p.output_midi, &midi_s, 1);
            free_midisong(&midi_s);
        }
        free_alignmentmap(map);
        free_alignment(a);
    }
#endif
EXIT:
    align_free_parameters(&p);
    free_song_collection(&sc);
    free_song_collection(&pc);

    return 0;
}

