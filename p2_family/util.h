/*
 * util.c - External declarations for the helper functions
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


#ifndef __UTIL_H__
#define __UTIL_H__

#include <sys/time.h>
#include <getopt.h>

#include "config.h"

#ifdef __cplusplus
extern "C" {
#endif


/** Returns the larger of two numerical values. */
#define MAX2(a,b) ((a)>(b)?(a):(b))

/** Returns the smaller of two numerical values. */
#define MIN2(a,b) ((a)<(b)?(a):(b))


/** Swaps the values of two variables. */
#define INT_SWAP(a,b) { int t=(a); (a)=(b); (b)=t; }

#define DOUBLE_SWAP(a,b) { double t=(a); (a)=(b); (b)=t; }

#define VOIDPTR_SWAP(a,b) { void *t=(a); (a)=(b); (b)=t; }

#define ABS(a) ((a) > 0 ? (a) : -(a))


/* External function declarations */

void init_bitcount(void);

int bitcount32(unsigned int n);

double timediff(struct timeval *time1, struct timeval *time0);


float randf(void);

double randd(void);


int round64(double x);

int round32(float x);


int cmp_int(const void *a, const void *b);


int kth_smallest(int a[], int n, int k);

double kth_smallest_double(double a[], int n, int k);

#define median(a, n) kth_smallest(a, n, (((n)&1) ? ((n)/2) : (((n)/2)-1)))


int strcicmp(const char *s1, const char *s2);

void *read_file(const char *file, int minsize, int nullpadding, int *size);


void write_pgm(const char *file, unsigned char *image, int width, int height);

unsigned char *read_pgm(const char *file, int *width, int *height);


char *make_short_options(const struct option *long_options);

#ifdef __cplusplus
}
#endif

#endif

