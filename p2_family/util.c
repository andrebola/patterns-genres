/*
 * util.c - Helper functions
 *
 * Version 2007-05-28
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
#include <time.h>
#include <math.h>
#include <ctype.h>
#include <getopt.h>

#include "config.h"
#include "util.h"


static int bitcount_initialized = 0;
static unsigned char bitcount_16[1<<16];


/**
 * Generates the bitcount lookup table.
 */
void init_bitcount(void) {
    int i, mask, step;

    if (bitcount_initialized) return;

    bitcount_16[0] = 0;
    bitcount_16[1] = 1;
    bitcount_16[2] = 1;
    bitcount_16[3] = 2;
    mask = 0x3;
    step = 0x8;
    for (i=4; i<(1<<16); ++i) {
        if (i >= step) {
            mask = step - 1;
            step <<= 1;
        }
        bitcount_16[i] = 1 + bitcount_16[i & mask];
    }

    bitcount_initialized = 1;
}


/**
 * Counts the number of bits in an unsigned integer.
 */
int bitcount32(unsigned int n) {
    return bitcount_16[n >> 16] + bitcount_16[n & 0xFFFF];
}


/**
 * Returns the difference of two timevals in seconds (with a microsecond
 * precision).
 *
 * @param time1 the later measurement, as returned by time()
 * @param time0 the earlier measurement
 *
 * @return difference in seconds
 */
double timediff(struct timeval *time1, struct timeval *time0) {
    double diffseconds = difftime(time1->tv_sec, time0->tv_sec);
    diffseconds += (double) ((long) time1->tv_usec - (long) time0->tv_usec) /
        1000000.0;
    return diffseconds;
}


/**
 * Returns a random number in range [0.0, 1.0].
 *
 * @return random number as a float
 */
float randf(void) {
    int r = rand();
    return (float) r / (float) RAND_MAX;
}


/**
 * Returns a random number in range [0.0, 1.0].
 *
 * @return random number as a double
 */
double randd(void) {
    int r = rand();
    return (double) r / (double) RAND_MAX;
}


/**
 * Rounds the given float number to the nearest integer.
 *
 * @param x a float number
 *
 * @return the rounded number
 */
int round32(float x) {
    if (x < 0.0) {
        double r = ceil((double) x - 0.5);
        return (int) r;
    } else {
        double r = floor((double) x + 0.5);
        return (int) r;
    }
}


/**
 * Rounds the given double number to the nearest integer.
 *
 * @param x a double number
 *
 * @return the rounded number
 */
int round64(double x) {
    if (x < 0.0) {
        double r = ceil(x - 0.5);
        return (int) r;
    } else {
        double r = floor(x + 0.5);
        return (int) r;
    }
}


/**
 * Compares two integers for qsort.
 *
 * @param a first integer
 * @param b second integer
 *
 * @return a negative number if a < b, positive if a > b and
 *         zero if they are equal
 */
int cmp_int(const void *a, const void *b) {
    const int *ia = (const int *) a;
    const int *ib = (const int *) b;
    return *ia  - *ib; 
}


/**
 * Returns the k:th smallest item from an array of integers.
 * Implementation by N. Devillard. This code is in public domain.
 *
 * Reference: N. Wirth, Algorithms + data structures = programs,
 *            Prentice-Hall, 1976 
 *
 * @param a an array of integers
 * @param n number of items
 * @param k the rank of the item that will be returned
 *
 * @return the k:th smallest item.
 */
int kth_smallest(int a[], int n, int k) {
    int i, j, l, m;
    int x;

    l = 0;
    m = n - 1;
    while (l < m) {
        x = a[k];
        i = l;
        j = m;
        do {
            while (a[i] < x) i++;
            while (x < a[j]) j--;
            if (i <= j) {
                INT_SWAP(a[i], a[j]);
                i++;
                j--;
            }
        } while (i <= j);
        if (j < k) l = i;
        if (k < i) m = j;
    }
    return a[k];
}


/**
 * Returns the k:th smallest item from an array of doubles.
 * See kth_smallest() above.
 *
 * @param a an array of doubles
 * @param n number of items
 * @param k the rank of the item that will be returned
 *
 * @return the k:th smallest item.
 */
double kth_smallest_double(double a[], int n, int k) {
    int i, j, l, m;
    double x;

    l = 0;
    m = n - 1;
    while (l < m) {
        x = a[k];
        i = l;
        j = m;
        do {
            while (a[i] < x) i++;
            while (x < a[j]) j--;
            if (i <= j) {
                DOUBLE_SWAP(a[i], a[j]);
                i++;
                j--;
            }
        } while (i <= j);
        if (j < k) l = i;
        if (k < i) m = j;
    }
    return a[k];
}


/**
 * Case-insensitive string comparison.
 *
 * @param s1 a string
 * @param s2 another string
 *
 * @return -1 if s1 comes before s2 in lexicographic order, 1 if s1 comes
 *         after s2 and 0 if the two strings are equal.
 */
int strcicmp(const char *s1, const char *s2) {
    for(;;) {
        int c1, c2;
        c1 = tolower((unsigned char) *s1++);
        c2 = tolower((unsigned char) *s2++);
        if ((c1 == 0) || (c1 != c2))
            return c1 - c2;
    }
}


/**
 * Reads a file into a memory buffer.
 *
 * @param file path to a file.
 * @param minsize minimum file size expected. If the file is smaller, this
 *        function will return NULL.
 * @param nullpadding number of null bytes that the end of the buffer should
 *        be padded with
 * @param size pointer to an integer where the buffer size will be stored
 *
 * @return pointer to the memory buffer or NULL if there was an error.
 */
void *read_file(const char *file, int minsize, int nullpadding, int *size) {
    FILE *f;
    int i, filesize;
    unsigned char *buffer;

    f = fopen(file, "r");
    if (f == NULL) {
        printf("ERROR in read_file(): failed to open file '%s'\n", file);
        *size = 0;
        return NULL;
    }

    fseek(f, 0, SEEK_END);
    filesize = ftell(f);
    if (filesize < minsize) {
        printf("ERROR in read_file(): file '%s' is too short\n", file);
        fclose(f);
        *size = 0;
        return NULL;
    }
    fseek(f, 0, SEEK_SET);
    buffer = (unsigned char *) malloc((filesize + nullpadding) *
            sizeof(unsigned char)); 
    if (buffer == NULL) {
        printf("ERROR in read_file(): failed to allocate a memory buffer for file data\n");
        fclose(f);
        *size = 0;
        return NULL;
    }
    i = fread(buffer, sizeof(char), filesize, f);
    if (i != filesize) {
        printf("ERROR in read_file(): failed to read file '%s'\n", file);
        free(buffer);
        fclose(f);
        *size = 0;
        return NULL;
    }

    /* Pad with null characters */
    for (i=0; i<nullpadding; ++i) {
        buffer[filesize + i] = '\0';
    }

    fclose(f);
    *size = filesize + nullpadding;
    return buffer;
}

/**
 * Writes an 8-bit grayscale image to disk as a PGM (portable graymap) file.
 *
 * @param file output file name
 * @param image grayscale image data, 1 byte per pixel
 * @param width image width in pixels
 * @param height image height in pixels
 */
void write_pgm(const char *file, unsigned char *image, int width, int height) {
    FILE *f;

    f = fopen(file, "w");
    if (f == NULL) {
        fprintf(stderr, "Error in write_pgm: failed to open file %s for writing\n", file);
        return;
    }

    /* Write PAM header */
    /*
    fputs("P7\n", f);
    fprintf(f, "WIDTH %d\n", width);
    fprintf(f, "HEIGHT %d\n", height);
    fputs("DEPTH 1\n", f);
    fputs("MAXVAL 255\n", f);
    fputs("TUPLTYPE GRAYSCALE\n", f);
    fputs("ENDHDR\n", f);
    */

    /* Write PGM header */
    fprintf(f, "P5 %d %d %d\n", width, height, 255);
    fwrite(image, sizeof(unsigned char), width * height, f);
    fclose(f);
}

/**
 * Reads PGM (portable graymap) files.
 *
 * @param file input file name
 * @param width memory location where image width will be written as an int
 * @param height memory location where image height will be written as an int
 * @return image buffer
 */
unsigned char *read_pgm(const char *file, int *width, int *height) {
    FILE *f;
    int colors;
    unsigned int size;
    unsigned char *buffer;

    f = fopen(file, "r");
    if (f == NULL) {
        fprintf(stderr, "Error in read_pgm: failed to read file %s\n", file);
        return NULL;
    }

    /* Write PGM header */
    if (fscanf(f, "P5 %d %d %d\n", width, height, &colors) == EOF) {
        fprintf(stderr, "Error in read_pgm: unknown file format or corrupted header\n");
        return NULL;
    }

    size = (unsigned int) (*width) * (*height);
    buffer = (unsigned char*) calloc(size, sizeof(unsigned char));
    if (fread(buffer, sizeof(unsigned char), size, f) < size) {
        fprintf(stderr, "Error in read_pgm: unexpected end of file\n");
    }
    fclose(f);
    return buffer;
}


/**
 * Builds a getopt short option string from an option struct.
 *
 * @param long_options struct that describes long options
 *
 * @return short option string. This must be freed at some point.
 */
char *make_short_options(const struct option *long_options) {
    int i, j, str_length = 0, num_options = 0;
    char *options;
    while (long_options[num_options].name != NULL) {
        if (long_options[num_options].val < 256) {
            ++str_length;
            if (long_options[num_options].has_arg == required_argument)
                ++str_length;
            else if (long_options[num_options].has_arg == optional_argument)
                str_length += 2;
        }
        ++num_options;
    }

    if (!str_length) return (char *) calloc(1, sizeof(char));

    options = (char *) malloc (str_length + 1);
    if (options == NULL) return NULL;

    j = 0;
    for (i=0; i<num_options; ++i) {
        if (long_options[i].val < 256) {
            options[j] = long_options[i].val;
            ++j;
            if (long_options[i].has_arg == required_argument) {
                options[j] = ':';
                ++j;
            } else if (long_options[i].has_arg == optional_argument) {
                options[j] = ':';
                ++j;
                options[j] = ':';
                ++j;
            }
        }
    }
    options[j] = '\0';

    return options;
}


