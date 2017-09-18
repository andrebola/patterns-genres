/*
 * align_P3.h - External function declarations for P3 alignment algorithm
 *
 * Version 2008-03-05
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


#ifndef __ALIGN_P3_H__
#define __ALIGN_P3_H__

#include "config.h"
#include "search.h"
#include "song.h"
#include "align.h"

#ifdef __cplusplus
extern "C" {
#endif

/* External function declarations */


void align_p3(const songcollection *sc, const song *s, const song *p,
        int alg, const alignparameters *parameters, alignmentmap *map);

/*void align_p3(const songcollection *sc, const song *s, const song *p, int alg,
        const alignparameters *parameters, const alignment *initial_align,
        alignment *result_align);

void map_alignments_p3(const songcollection *sc, const song *s, const song *p,
        int alg, const alignparameters *parameters, alignmentmap *map);
*/

#ifdef __cplusplus
}
#endif

#endif

