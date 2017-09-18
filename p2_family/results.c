/*
 * results.c - Functions for manipulating search result data structures
 *
 * Version 2007-08-21
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
/*#include <limits.h>*/

#include "config.h"
#include "results.h"
/*#include "util.h"*/


/**
 * Frees memory used by a match result.
 *
 * @param ms the match to free
 */
void free_match(match *m) {
    if((m != NULL) && (m->notes != NULL)) free(m->notes);
}

/**
 * Frees memory used by a set of match results.
 *
 * @param ms the set of matches to free
 */
void free_match_set(matchset *ms) {
    if(ms->matches != NULL) {
        int i;
        for (i=0; i<ms->size; ++i) {
            free_match(&ms->matches[i]);
        }
        free(ms->matches);
        ms->matches = NULL;
        ms->size = 0;
        ms->time.indexing = 0.0;
        ms->time.verifying = 0.0;
        ms->time.other = 0.0;
        ms->time.measure = 0;
    }
}


/**
 * Initializes a set of match results.
 *
 * @param ms the set of matches to initialize
 * @param size capacity of the set
 * @param pattern_size space required for storing note positions of the matches.
 *        If zero, no space is reserved for storing the positions.
 * @param multiple_matches_per_song sets whether multiple matches per song
 *        are stored (1) or if only the best match is kept (0)
 *
 * @return 1 if successful, 0 otherwise;
 */
int init_match_set(matchset *ms, int size, int pattern_size,
        int multiple_matches_per_song) {
    ms->multiple_matches_per_song = multiple_matches_per_song;
    ms->time.indexing = 0.0;
    ms->time.verifying = 0.0;
    ms->time.other = 0.0;
    ms->time.measure = 0;
    ms->size = size;
    ms->num_matches = 0;
    ms->matches = (match *) calloc(size, sizeof(match));
    if (ms->matches == NULL) {
        printf("ERROR in init_match_set(): failed to allocate memory");
        return 0;
    }
    if (pattern_size > 0) {
        int i;
        for (i=0; i<ms->size; ++i) {
            match *m = &ms->matches[i];
            m->num_notes = pattern_size;
            m->notes = (int *) calloc(pattern_size, sizeof(int));
        }
    }
    return 1;
}


/**
 * Clears a set of match results.
 *
 * @param ms the set of matches to free
 */
void clear_match_set(matchset *ms) {
    ms->time.indexing = 0.0;
    ms->time.verifying = 0.0;
    ms->time.other = 0.0;
    ms->time.measure = 0;
    ms->num_matches = 0;
    if(ms->matches != NULL) {
        int i, j;
        match *m;
        for (i=0; i<ms->size; ++i) {
            m = &ms->matches[i];
            m->song = -1;
            m->start = 0;
            m->end = 0;
            m->transposition = 0;
            m->similarity = 0.0F;
            if (m->notes != NULL) {
                for (j=0; j<m->num_notes; ++j) m->notes[j] = -1;
            }
        }
    }
}


/**
 * Checks if the given match overlaps with a specific interval.
 *
 * @param m a match item
 * @param start interval start
 * @param end interval end
 *
 * @return 1 if the match overlaps with the interval, 0 otherwise
 */
static INLINE int match_overlap(match *m, int start, int end) {
    if ((m->end <= m->start) && ((m->start >= end) || (m->start < start)))
        return 0;
    else if ((m->start >= end) || (m->end <= start)) return 0;
    else return 1;
}


/**
 * Adds a match to a set of matches if it is good enough to fit.
 *
 * Note: caller should take care of filling the note position array within
 * the match item.
 *
 * @param ms a set of matches where the new match is added
 * @param songid song number
 * @param start match start time in the song
 * @param end match end time in the song
 * @param transposition match transposition in the song compared to the
 *        original pattern
 * @param similarity match score given by the caller
 *
 * @return the match item if it was inserted, NULL otherwise.
 */
match *insert_match(matchset *ms, int songid, int start, int end,
        char transposition, float similarity) {
    int i;
    int same_song = 0;
    match *m = NULL;
    int *mnotes = NULL;

    /* Check if there is already a match for this song */
    for (i=0; i<ms->num_matches; ++i) {
        match *mi = &ms->matches[i];
        if (songid == mi->song) {
            same_song++;
	    int j;
            /* If multiple matches per song are allowed and the previous
               match doesn't overlap with this one, continue */
            if (ms->multiple_matches_per_song &&
                    (!match_overlap(mi, start, end))) continue;

            if (similarity <= mi->similarity) return NULL;

            /* Move up in the array */
            for (j=i-1; j>=0; --j) {
                if (ms->matches[j].similarity < similarity)
                    memcpy(&ms->matches[j+1], &ms->matches[j],
                            sizeof(match));
                else break;
            }
            m = &ms->matches[j+1];
            
	    break;
        }
    }
    if (m == NULL) {
        /* Multiple matches per song are allowed or there was not a previous
           result for this song */
        for (i=0; i<ms->size; ++i) {
            match *mi = &ms->matches[i];
            if (similarity > mi->similarity) {
                int j;
                mnotes = ms->matches[ms->size-1].notes;

                /* Move other matches down in the array */
                if (i >= ms->num_matches) ms->num_matches = i + 1;
                for (j=ms->size - 1; j>i; --j) {
                    memcpy(&ms->matches[j], &ms->matches[j-1], sizeof(match));
                }
                m = mi;
                break;
            }
        }
        /*if (same_song == 0){
fprintf(stderr, "MATCH added %d\n", ms->size);
m = &ms->matches[ms->size];
if (m == NULL){

fprintf(stderr, "MATCH added is NULL\n");
}
fprintf(stderr, "MATCH added 1\n");
//m->notes = mnotes;
        m->song = songid;
        m->start = start;
        m->end = end;
        m->transposition = transposition;
        m->similarity = similarity;
fprintf(stderr, "MATCH added %d\n", ms->size);
    
//m = calloc(1, sizeof(match));
//fprintf(stderr, "MATCH added 2\n");
//ms->size += 1;
//fprintf(stderr, "MATCH added 3\n");
//ms->matches[0] = *m;
	}*/
    }
    if (m != NULL) {
        m->notes = mnotes;
        m->song = songid;
        m->start = start;
        m->end = end;
        m->transposition = transposition;
        m->similarity = similarity;
    }
    return m;
}


