/*
 * geometric_SP2.c - Geometric algorithm SP2
 *
 * Version 2008-02-24
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
#include <limits.h>
#include <math.h>
#include <string.h>

#include "config.h"
#include "search.h"
#include "song.h"
#include "util.h"
#include "geometric_SP2.h"

#define TEMPO_FACTOR 2


/**
 * Search a song collection with c_geometric_sp3_scan().
 *
 * @param sc a song collection to scan
 * @param pattern pattern to search for
 * @param alg search algorithm to use. See algorithms.h for algorithm IDs.
 * @param parameters search parameters
 * @param ms match set for returning search results
 */
void alg_sp2(const songcollection *sc, const song *pattern, int alg,
        const searchparameters *parameters, matchset *ms) {
    int i;
    for (i=0; i<sc->size; ++i) {
        scan_song_sp2(&sc->songs[i], pattern, ms);
    }
}


/**
 * Geometric matching algorithm SP3 that uses an adaptive time scale and
 * can handle errors in note timing and pitch intervals.
 *
 * @param s song to scan
 * @param p pattern that is searched for
 * @param results structure for results
 *
 * @return 1 when successful, 0 otherwise
 */
void scan_song_sp2(const song *s, const song *p, matchset *ms) {
    char output_file[256];
    vector *pnotes = p->notes;
    vector *snotes = s->notes;
    unsigned char *buffer;
    int buf_scale = 100;
    int buf_origin = pnotes[p->size-1].strt / buf_scale;
    int buf_width = buf_origin + (snotes[s->size-1].strt +
            snotes[s->size-1].dur) / buf_scale;
    int buf_height = 256;
    buffer = (unsigned char *) malloc(buf_width * buf_height *
            sizeof(unsigned char));
    memset(buffer, 255, buf_width * buf_height);


    snprintf(output_file, 256, "sp2_%d_%d.pgm", p->id, s->id);
    write_pgm(output_file, buffer, buf_width, buf_height);

    free(buffer);
    return;
}

