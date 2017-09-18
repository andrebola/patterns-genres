/*
 * song.h - Structures and external declarations for the song collection
 *          functions
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

#ifndef __SONG_H__
#define __SONG_H__

#include "config.h"
#include "data_formats.h"
#include "results.h"

#ifdef __cplusplus
extern "C" {
#endif


/** Number of discrete note pitches. Since the data mostly comes from MIDI
 * files, 128 is a natural choise. */
#define NOTE_PITCHES 128

/** Maximum song duration in milliseconds. See vector.strt below */
#define MAX_SONG_DURATION 1 << 23


/**
 * A two-dimensional note vector for the geometric algorithms.
 */
typedef struct {
    /* Note start time in 1/1024 seconds. If this is not accurate enough or
     * does not give enough room, the data needs to be scaled appropriately.
     *
     * NOTE: Some of the algorithm implementations only support 23-bit time
     * values, which means 8192 seconds for the whole song (the last 8 bits
     * are used to store the pitch to the same 32-bit vector and the direction
     * sign needs one bit). */
    int strt;

    /* Duration of the note in 1/1024 seconds */
    int dur;

    /* MIDI pitch value */
    char ptch;

    /* Note velocity or volume */
    char velocity;

    /* Instrument/track id in the song structure */
    unsigned short instrument;
} vector;


/**
 * Instrument and track metadata.
 */
typedef struct {
    /* MIDI channel and port number.
     * The actual channel number (0-15) is (channel % 16) and
     * the MIDI port is (channel / 16). */
    unsigned char channel;

    /* MIDI track number */
    short track;

    /* MIDI program (voice) number */
    char program;

    /* MIDI bank number */
    short bank;

    /* Track name */
    char *track_name;

    /* Module name */ 
    char *module_name;

    /* Program name */
    char *program_name;
} instrument;


/**
 * A set of instruments.
 */
typedef struct {
    int size;
    instrument *i;
} instrumentdata;


/**
 * A generic MIDI event. This is used to describe events that are not
 * important to this system, but that are still handy to have around.
 */
typedef struct {
    /* Event time */
    int time;

    /* Note or instrument that is affected by this event. Negative
       values refer to instrument IDs, positive values refer to notes. */
    int target;

    /* Event type */
    int type;

    /* Event value */
    int value;

    /* Pointer to more event data */
    unsigned char *data;
} midievent;


/**
 * A set of MIDI events
 */
typedef struct {
    int size;
    midievent *e;
} mididata;


/**
 * A song record.
 */
typedef struct {
    /* ID number or position in a song collection array */
    int id;

    /* Song title */
    char *title;

    /* Song duration in milliseconds */
    int duration;

    /* Number of notes in the song */
    int size;

    /* Notes */
    vector *notes;

    /* Instrument descriptions */
    instrumentdata instruments;

    /* MIDI control message track. */
    mididata control;

    /* Tempo and time signature track. */
    mididata tempo;
} song;

#if 0
/**
 * A container for song data in algorithm-specific formats.
 */
typedef struct {
    /* Data format, as defined in algorithms.h */
    int format;

    /* Song collection data */
    void *data;
} songdata;
#endif

/**
 * A song collection.
 */
typedef struct {
    /* Number of songs in the collection */
    int size;

    /* Songs */
    song *songs;

    /* Overall number of notes in the collection. */
    int num_notes;

    /* Number of notes in the largest song. */
    int max_song_size;

    /* Algorithm-specific data formats and indices.
     * This is an array of pointers indexed by data format IDs
     * (DATA_* in algorithms.h). Also see update_song_collection_data() */
    void *data[NUM_DATA_FORMATS+1];
} songcollection;



/* External function declarations */


void init_song(song *s, int id, const char *title, int maxnotes);

void free_song(song *s);

int init_song_collection(songcollection *sc, int maxsongs);

void free_song_collection(songcollection *sc);

int read_songfile(const char *songfile, songcollection *sc);

int generate_patterns_song(songcollection *pc, const song *s, matchset *ms, int length, int window);
int generate_pattern(song *pattern, const songcollection *sc, match *m,
        int length, int maxskip, char maxtranspose, float errors);

int generate_pattern_collection(int patterncount, songcollection *pc,
        const songcollection *sc, matchset *ms, int minlength, int maxlength,
        int maxskip, char maxtranspose, float errors);

void preprocess_songs(songcollection *sc, int q, int shuffle, int notes);

void lexicographic_sort(song *s);

int compare_notes(const void *aa, const void *bb);

void remove_overlap(song *s);

void quantize(song *s, int q);

void remove_octave_information(song *s);

void p3_optimize_song_collection(songcollection *sc, int mingap, int mindur);

void p3_optimize_song(song *song, int mingap, int mindur);

void sc_remove_octave_information(const songcollection *sc);

songcollection *join_songs(songcollection *sc, int gap,
        int num_songs, int song_size, int shuffle);

void update_song_collection_statistics(songcollection *sc);

void free_song_collection_data(songcollection *sc);

void update_song_collection_data(songcollection *sc, const int *algorithms,
        const dataparameters *dp);

void insert_pattern_to_song(song *s, int songpos, const song *p, float errors,
        float noise);

void insert_patterns(songcollection *sc, const songcollection *pc,
        int pattern_instances, float errors, float noise);

#ifdef __cplusplus
}
#endif

#endif

