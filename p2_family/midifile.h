/*
 * midifile.h - Function declarations for the MIDI file I/O
 *
 * Version 2007-08-15
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

#ifndef __MIDIFILE_H__
#define __MIDIFILE_H__

#include "config.h"
#include "song.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Minimum size of a MIDI file */
#define SMF_MIN_SIZE 14
/** Gap length to use between concatenated tracks of Type 3 files. */
#define MIDI_TYPE3_GAP 1000

#define MIDI_DEFAULT_FORMAT 1
#define MIDI_DEFAULT_PPQN_DIVISION 96
#define MIDI_DEFAULT_TEMPO 500.0

/** Should the note buffers be reallocated after the exact number of notes is
  * known. Otherwise the amount of memory allocated for them will usually be
  * 2-3 times higher than necessary. */
#define MIDI_READ_TRIM_BUFFERS 1


/* Constants */

#define MIDI_CHANNELS 16
#define MIDI_PERCUSSION_CHANNEL 9

#define PPQN 0
#define SMPTE 1

#define STATUS_BIT 0x80
#define STATUS_MASK 0xF0
#define CHANNEL_MASK 0x0F

/* Voice events */
#define EVENT_NOTE_OFF 0x80
#define EVENT_NOTE_ON 0x90
#define EVENT_AFTERTOUCH 0xA0
#define EVENT_CONTROLLER 0xB0
#define EVENT_PROGRAM_CHANGE 0xC0
#define EVENT_CHANNEL_PRESSURE 0xD0
#define EVENT_PITCH_WHEEL 0xE0

/* System events */
#define EVENT_SYSEX 0xF0
#define EVENT_ESCAPE 0xF7
#define EVENT_META 0xFF

/* Meta events */
#define EVENT_END_OF_TRACK 0x2F
#define EVENT_TEMPO 0x51

/* Controllers */
#define CONTROLLER_ALL_SOUND_OFF 120
#define CONTROLLER_ALL_NOTES_OFF 123

#define MIDI_HEADER_SIZE 14
#define MIDI_TRACK_HEADER_SIZE 8


/**
 * A struct for MIDI events.
 */
typedef struct {
    double tick;
    unsigned char status;
    unsigned char metatype;
    const unsigned char *data;
    int length;
} track_event;


/**
 * MIDI song container type.
 */
typedef struct {
    char *name;
    int num_tracks;
    int *track_size;
    track_event **track_data;
    int buffer_size;
    unsigned char *buffer;
} midisong;



/* External function declarations */

int read_midi_directory(const char *path, songcollection *sc,
        int skip_percussion);

int read_midi_file(const char *file, song *s, midisong *midi_s,
        int skip_percussion);

int read_midi_file2(const char *file, song *s, midisong *midi_s,
        int skip_percussion, const int only_rhythm);


int write_midi_file(const char *path, const midisong *midi_s,
        int force_leading_silence);

void free_midisong(midisong *midi_s);

#ifdef __cplusplus
}
#endif

#endif

