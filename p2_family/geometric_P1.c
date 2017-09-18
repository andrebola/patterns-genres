/*
 * geometric_P1.c - Geometric algorithm P1
 *
 * Version 2007-05-28
 *
 *
 * Copyright (C) 2007 Niko Mikkila and Mika Turkia
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


/*
   Bugs: dummy 'infinity' value should be added to the end of source, so that
   the implementation could be simplified to follow pseudocode more closely.
   The decision to not add it made the algorithm overly complex. 
*/

#include <stdio.h>
#include <stdlib.h>
#include <limits.h>

#include "search.h"
#include "song.h"
#include "geometric_P1.h"


/**
 * Search a song collection with scan_song_p1().
 *
 * @param sc a song collection to scan
 * @param pattern pattern to search for
 * @param alg search algorithm to use. See algorithms.h for algorithm IDs.
 * @param parameters search parameters
 * @param ms match set for returning search results
 */
void alg_p1(const songcollection *sc, const song *pattern, int alg,
        const searchparameters *parameters, matchset *ms) {
    int i;
    for (i=0; i<sc->size; ++i) {
        scan_song_p1(&sc->songs[i], pattern, ms);
    }
}


/**
 * Scanning phase of geometric algorithm P1. This algorithm is described in
 * Esko Ukkonen, Kjell Lemstrom and Veli Makinen: Sweepline the Music! In
 * Computer Science in Perspective (LNCS 2598), R. Klein, H.-W. Six, L. Wegner
 * (Eds.), pp. 330-342, 2003.
 *
 * Unlike in the article, end of source is not detected by putting
 * (infinity,infinity) to the end of source, but with indexes.
 *
 * @param s the song to scan
 * @param p pattern to search for
 * @param ms pointer to a structure where the results will be stored
 *
 * @return 1 when successful, 0 otherwise
 */
int scan_song_p1(const song *s, const song *p, matchset *ms) {
    unsigned int i, j, pattern_size, song_size;
    int v0, p0;
    int sid = s->id;
    unsigned int *q;
    vector *pattern, *text;

    if ((p->size < 2) || (s->size < 2)) return 0;
    if (s->size < p->size) {
        /* Swap song and pattern if pattern is larger */
        pattern_size = s->size;
        song_size = p->size;
        pattern = s->notes;
        text = p->notes;
    } else {
        pattern_size = p->size;
        song_size = s->size;
        pattern = p->notes;
        text = s->notes;
    }

    /* q is an array that points to the last checked note for each
     * pattern position */
    q = (unsigned int *) calloc(pattern_size, sizeof(unsigned int));
    if (q == NULL) return 0;

    p0 = ((int) pattern[0].strt << 8) + (int) pattern[0].ptch
            - NOTE_PITCHES;

    /* Scan all notes */
    for (i = 0; i < song_size - pattern_size + 1; ++i) {
        v0 = ((int) text[i].strt << 8) + (int) text[i].ptch - p0;
        q[0] = i + 1;

        /* Start finding points of the transposed pattern starting from
         * this source position and do it until a match cannot be found. */
        for (j=1; j < pattern_size; ++j) {
            int v;
            int pj = ((int) pattern[j].strt << 8) + (int) pattern[j].ptch
                    - NOTE_PITCHES;
            if (q[j] < q[j-1]) q[j] = q[j-1];
            if (q[j] == song_size) {
                free(q);
                return 0;
            }
            /* Move q over the notes that can't be part of a match for any
             * remaining pattern position. */
            do {
                v = ((int) text[q[j]].strt << 8) + (int) text[q[j]].ptch - pj;
                q[j]++;
            } while ((v < v0) && (q[j] < song_size));
            /* After the loop we are at either a matching note or bigger,
             * i.e. there is no matching note. */
            if (v != v0) {
                q[j]--;
                break;
            }
        }

        /* Check if a match was found */
        if (j == pattern_size) {
            /* Return the match */
            int start = text[i].strt;
            int end = start + pattern[pattern_size-1].strt +
                    pattern[pattern_size-1].dur - pattern[0].strt;
            char transposition = text[i].ptch - pattern[0].ptch;
            insert_match(ms, sid, start, end, transposition, 1.0F);
        }
    }
    free(q);
    return 1;
}


/**
 * Checks if there is an exact match to a pattern in the given position.
 *
 * @param s a song
 * @param songpos position in the song
 * @param p pattern to match
 * @param patternpos position in the pattern that aligns with songpos
 * @param ms structure where the match information is stored to
 *
 * @return match information item
 */
match *alignment_check_p1(const song *s, unsigned short songpos,
        const song *p, unsigned short patternpos, matchset *ms) {

    int i, j, p0, v0, end;
    vector *pnotes = p->notes;
    vector *snotes = s->notes;
    match *m;

    if ((songpos >= s->size) || (patternpos >= p->size)) return NULL;

    /* Scan the end */
    i = patternpos + 1;
    j = songpos;
    p0 = ((int) pnotes[patternpos].strt << 8) +
            (int) pnotes[patternpos].ptch;
    v0 = ((int) snotes[songpos].strt << 8) +
            (int) snotes[songpos].ptch;
    for (; i < p->size; ++i) {
        int pi = ((int) pnotes[i].strt << 8) + (int) pnotes[i].ptch - p0;
        int vi;

        /* Skip over notes that are not in the pattern. */
        do {
            ++j;
            if (j >= s->size) return NULL;
            vi = ((int) snotes[j].strt << 8) + (int) snotes[j].ptch - v0;
        } while (vi < pi);

        /* Is there a matching note? If not, exit. */
        if (vi != pi) return NULL;
    }
    end = j;

    /* Scan the beginning */
    i = patternpos - 1;
    j = songpos;
    for (; i >= 0; --i) {
        int pi = p0 - ((int) pnotes[i].strt << 8) - (int) pnotes[i].ptch;
        int vi;

        /* Skip over notes that are not in the pattern. */
        do {
            --j;
            if (j < 0) return NULL;
            vi = v0 - ((int) snotes[j].strt << 8) - (int) snotes[j].ptch;
        } while (vi < pi);

        /* Is there a matching note? If not, exit. */
        if (vi != pi) return NULL;
    }

    if (ms == NULL) return (match *) 1;

    m = insert_match(ms, s->id, snotes[j].strt,
            snotes[end].strt + snotes[end].dur,
            snotes[j].ptch - pnotes[0].ptch, 1.0F);
    if (m == NULL) return NULL;

    /* Match found */
/*    if (ms->num_matches >= ms->size) i = ms->size - 1;
    else {
        i = ms->num_matches;
        ++ms->num_matches;
    }

    m = &ms->matches[i];
    m->song = s->id;
    m->start = snotes[j].strt;
    m->end = snotes[end].strt + snotes[end].dur;
    m->transposition = snotes[j].ptch - pnotes[0].ptch;
    m->similarity = 1.0F;
*/
    /* Find out matching note positions if they are requested */
    if ((m->num_notes > 0) && (m->notes != NULL)) {
        m->notes[0] = j;
        p0 = ((int) pnotes[0].strt << 8) +
                (int) pnotes[0].ptch;
        v0 = ((int) snotes[j].strt << 8) +
                (int) snotes[j].ptch;

        /*if (m->num_notes > p->size) m->num_notes = p->size;*/
        for (i = 1; i < m->num_notes; ++i) {
            int pi = ((int) pnotes[i].strt << 8) + (int) pnotes[i].ptch - p0;
            int vi;

            /* Skip over notes that are not in the pattern. */
            do {
                ++j;
                if (j >= s->size) return m;
                vi = ((int) snotes[j].strt << 8) + (int) snotes[j].ptch - v0;
            } while (vi < pi);

            /* Is there a matching note? If not, exit. */
            if (vi != pi) return m;
            m->notes[i] = j;
        }
    }
    return m;
}


