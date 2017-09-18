/*
 * midifile.c - Functions for reading and writing Standard MIDI Files.
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


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <dirent.h>
#include <sys/stat.h>

#include "config.h"
#include "midifile.h"
#include "priority_queue.h"
#include "song.h"
#include "util.h"



/**
 * Recursive function that scans midi files from a tree of directories.
 *
 * @param path the directory to scan
 * @param sc song collection where the read songs will be stored. Use NULL to
 *        only count the number of files.
 * @param count number of potential MIDI files found will be stored here
 * @param skip_percussion set to 1 to skip percussion channel in the MIDI files
 *        that will be read
 */
static void scan_midi_files(const char *path, songcollection *sc, int *count,
        int skip_percussion) {
    int n;
    struct stat statbuf;

    if (stat(path, &statbuf) == 0) {
        if (S_ISDIR(statbuf.st_mode)) {
            struct dirent **files = NULL;
            char *p;
            int pathlen = strlen(path);

            p = (char *) malloc((pathlen + 2) * sizeof(char));
            if (p == NULL) return;
            strcpy(p, path);
            if ((pathlen > 0) && (path[pathlen-1] != '/')) {
                p[pathlen] = '/';
                p[pathlen+1] = 0;
                ++pathlen;
            }

            n = scandir(p, &files, 0, alphasort);
            if (n >= 0) {
                int i;
                for (i = 0; i < n; ++i) {
                    char *fn;
                    if ((strcmp(files[i]->d_name, "..") == 0) ||
                            (strcmp(files[i]->d_name, ".") == 0)) {
                        free(files[i]);
                        continue;
                    }
                    fn = (char *) malloc((pathlen +
                            strlen(files[i]->d_name) + 1) * sizeof(char));
                    if (fn == NULL) {
                        for (; i<n; ++i) free(files[i]);
                        break;
                    }
                    strcpy(fn, p);
                    strcpy(fn + pathlen, files[i]->d_name);
                    scan_midi_files(fn, sc, count, skip_percussion);
                    free(fn);
                    free(files[i]);
                }
            } else fprintf(stderr, "Error: couldn't open directory: %s\n", p);

            free(p);
            free(files);
        } else if (S_ISREG(statbuf.st_mode)) {
            if ((sc != NULL) && (*count < sc->size)) {
                if (read_midi_file(path, &sc->songs[*count], NULL,
                        skip_percussion)) {
                    sc->songs[*count].id = *count;
                    *count += 1;
                }
            } else *count += 1;
        }
    }
}


/**
 * Reads a directory of MIDI files recursively.
 *
 * @param path the directory to scan
 * @param sc a song collection for the read files
 * @param skip_percussion set to 1 to skip percussion channel in the MIDI files
 *        that will be read
 *
 * @return number of MIDI files that were successfully read
 */
int read_midi_directory(const char *path, songcollection *sc,
        int skip_percussion) {
    int count = 0;
    scan_midi_files(path, NULL, &count, skip_percussion);
    sc->size = count;
    if (count > 0) {
        sc->songs = (song *) calloc(count, sizeof(song));
        count = 0;
        scan_midi_files(path, sc, &count, skip_percussion);
        sc->size = count;
    }
    fprintf(stderr, "%d MIDI files read\n", count);
    return count;
}


/**
 * Reads an unsigned short value from MIDI data.
 *
 * @param buffer MIDI data buffer
 * @param i position in the buffer. The pointer will be moved forward.
 *
 * @returns the read value
 */
static INLINE unsigned short midi_parse_short(const unsigned char *buffer,
        int *i) {
    int value = buffer[(*i)++];
    value <<= 8;
    value += buffer[(*i)++];
    return value;
}

/**
 * Writes an unsigned short value to a MIDI data buffer.
 *
 * @param value the value to write
 * @param buffer MIDI data buffer
 * @param i position in the buffer. The pointer will be moved forward.
 */
static INLINE void midi_write_short(unsigned short value, unsigned char *buffer,
        int *i) {
    buffer[(*i)++] = value >> 8;
    buffer[(*i)++] = value & 0xFF;
}


/**
 * Reads an unsigned int value from MIDI data.
 *
 * @param buffer MIDI data buffer
 * @param i position in the buffer. The pointer will be moved forward.
 *
 * @returns the read value
 */
static INLINE unsigned int midi_parse_int(const unsigned char *buffer, int *i) {
    int value = buffer[(*i)++];
    value <<= 8;
    value += buffer[(*i)++];
    value <<= 8;
    value += buffer[(*i)++];
    value <<= 8;
    value += buffer[(*i)++];
    return value;
}

/**
 * Writes an unsigned int value to a MIDI data buffer.
 *
 * @param value the value to write
 * @param buffer MIDI data buffer
 * @param i position in the buffer. The pointer will be moved forward.
 */
static INLINE void midi_write_int(unsigned int value, unsigned char *buffer,
        int *i) {
    buffer[(*i)++] = (value >> 24);
    buffer[(*i)++] = (value >> 16) & 0xFF;
    buffer[(*i)++] = (value >> 8) & 0xFF;
    buffer[(*i)++] = value & 0xFF;
}


/**
 * Reads a variable-length value from MIDI data.
 *
 * @param buffer MIDI data buffer
 * @param i position in the buffer. The pointer will be moved forward.
 *
 * @returns the read value
 */
static INLINE int midi_parse_varlen(const unsigned char *buffer, int *i) {
    int max_i = *i + 4;
    unsigned int value;
    unsigned char c;

    if ((value = buffer[(*i)++]) & 0x80) {
        value &= 0x7F;
        do {
            value = (value << 7) + ((c = buffer[(*i)++]) & 0x7F);
        } while ((c & 0x80) && ((*i) < max_i));
        if (c & 0x80) {
            fprintf(stderr, "Error in midi_parse_varlen(): Invalid variable-length quantity\n");
            return -value;
        }
    }
    return(value);
}


/**
 * Writes a variable-length value to a MIDI data buffer.
 *
 * @param value the value to write
 * @param buffer MIDI data buffer
 * @param i position in the buffer. The pointer will be moved forward.
 */
static INLINE void midi_write_varlen(unsigned int value, unsigned char *buffer,
        int *i) {
    unsigned int tmp;
    tmp = value & 0x7F;
    while ((value >>= 7)) {
        tmp <<= 8;
        tmp |= ((value & 0x7F) | 0x80);
    }
    while (1) {
        buffer[(*i)++] = tmp;
        if (tmp & 0x80) tmp >>= 8;
        else break;
    }
}

/**
 * Parses an event from MIDI data.
 *
 * @param buffer MIDI data buffer
 * @param buffersize size of the buffer
 * @param i current position in the buffer
 * @param event struct where the event data will be stored
 *
 * @return -2 or -1 if the buffer ends unexpectedly, 1 if successful,
 *         0 for end of track
 */
static INLINE int parse_event(const unsigned char *buffer, int buffersize,
        int *i, track_event *event) {
    int b;
    int length;
    int delta = -1;

    # if 0
    while ((*i < buffersize) && (delta < 0)) {
        delta = midi_parse_varlen(buffer, i);
        if (delta < 0) {
            /* Skip odd data */
            char c;
            while ((*i < buffersize) && ((c = buffer[*i]) & 0x80)) {
                *i += 1;
            }
            while ((*i < buffersize) && (!((c = buffer[*i]) & 0x80))) {
                *i += 1;
            }
        }
    }
    #endif
    if (*i >= buffersize) return -2;
    delta = midi_parse_varlen(buffer, i);
    if (delta < 0) return -2;

    event->tick += (double) delta;
    if (*i >= buffersize) return -1;

    b = buffer[(*i)++];
    /* Is there a status byte? Otherwise use running status (previous status) */
    if (b & STATUS_BIT) event->status = b;
    else {
        (*i)--;
        b = event->status;
    }

    switch (b & STATUS_MASK) {
        case EVENT_NOTE_OFF:
        case EVENT_NOTE_ON:
        case EVENT_AFTERTOUCH:
        case EVENT_CONTROLLER:
        case EVENT_PITCH_WHEEL:
            length = 2;
            break;
        case EVENT_PROGRAM_CHANGE:
        case EVENT_CHANNEL_PRESSURE:
            length = 1;
            break;
        case EVENT_SYSEX:
            /* Meta event */
            if (b == EVENT_META) {
                if (*i >= buffersize) return -1;
                b = buffer[(*i)++];
                event->metatype = b;
                if (*i >= buffersize) return -1;
                length = (int) midi_parse_varlen(buffer, i);
            /* SYSEX event or escaped data */
            } else if ((b == EVENT_SYSEX) || (b == EVENT_ESCAPE)) {
                if (*i >= buffersize) return -1;
                length = (int) midi_parse_varlen(buffer, i);
                if ((*i + length) > buffersize) length = buffersize - *i;
            /* Unescaped System Common or Realtime message */
            } else {
                fputs("Warning in parse_event(): ignoring unescaped MIDI system message\n", stderr);
                length = 0;
            }
            break;
        default:
            length = 1;
    }
    event->data = buffer + *i;
    event->length = length;
    /* Return -1 if the message was not complete */
    if ((*i + length) > buffersize) return -1;
    *i += length;
    if (event->metatype == EVENT_END_OF_TRACK) return 0;
    else return 1;
}


/**
 * Writes a MIDI event to a MIDI data buffer.
 *
 * @param event a MIDI track event
 * @param tick time value
 * @param buffer MIDI data buffer
 * @param i position in the buffer. The pointer will be moved forward.
 */
static INLINE void midi_write_event(track_event *event, int tick,
        unsigned char *buffer, int *i) {
    int j;
    midi_write_varlen(tick, buffer, i);
    buffer[(*i)++] = event->status;
    switch (event->status & STATUS_MASK) {
        case EVENT_SYSEX:
            /* Meta event */
            if (event->status == EVENT_META) buffer[(*i)++] = event->metatype;

            midi_write_varlen(event->length, buffer, i);
            for (j=0; j<event->length; ++j) {
                buffer[(*i)++] = event->data[j];
            }
            break;
        default:
            if (event->length >= 1) buffer[(*i)++] = event->data[0];
            if (event->length >= 2) buffer[(*i)++] = event->data[1];
            break;
    }
}


/**
 * Parses a set of MIDI tracks.
 *
 * @param tracks track data buffers
 * @param track_lengths lengths of track buffers
 * @param num_tracks number of tracks to parse
 * @param s song where the read music data will be stored
 * @param division_type PPQN or SMPTE
 * @param division ppqn division or SMPTE framerate
 * @param skip_percussion set to 1 to skip reading percussion channel events
 */
static void parse_tracks(const unsigned char **tracks,
        const int *track_lengths, const int num_tracks, song * const s,
        midisong * const midi_s, const int division_type, const int division,
        const int skip_percussion) {
    parse_tracks2(tracks, track_lengths, num_tracks, s, midi_s, division_type, division, skip_percussion, 0);
} 
void parse_tracks2(const unsigned char **tracks,
        const int *track_lengths, const int num_tracks, song * const s,
        midisong * const midi_s, const int division_type, const int division,
        const int skip_percussion, const int only_rhythm) {
    int i;
    vector *playing_notes[MIDI_CHANNELS][NOTE_PITCHES];
    int patch[MIDI_CHANNELS];
    track_event *track_events;
    int *track_pointers;
    /*double *track_last_tick;*/
    double ms_per_tick;
    double tempo_change_time = 0.0;
    double tempo_change_tick = 0.0;
    double curtime = 0.0;

    pqroot *pq = pq_create(num_tracks);
    if (pq == NULL) {
        fputs("Error in parse_tracks: failed to allocate a priority queue\n", stderr);
        return;
    }

    track_events = (track_event *) calloc(num_tracks, sizeof(track_event));
    track_pointers = (int *) calloc(num_tracks, sizeof(int));
    /*track_last_tick = (double *) calloc(num_tracks, sizeof(double));*/

    if ((track_events == NULL) || (track_pointers == NULL)) {
        fputs("Error in parse_tracks: failed to allocate temporary buffers\n", stderr);
        pq_free(pq);
        free(track_events);
        free(track_pointers);
        /*free(track_last_tick);*/
        return;
    }

    memset(playing_notes, 0, MIDI_CHANNELS * NOTE_PITCHES * sizeof(vector *));
    memset(patch, 0, MIDI_CHANNELS * sizeof(int));

    /* Initialize time base */
    if (division_type == SMPTE) ms_per_tick = 1000.0 / ((double) division);
    else ms_per_tick = MIDI_DEFAULT_TEMPO / ((double) division);

    /* Insert first events from each track to the queue */
    for (i=0; i<num_tracks; ++i) {
        pqnode *node = pq_getnode(pq, i);
        track_event *e = &track_events[i];
        int r = parse_event(tracks[i], track_lengths[i], &track_pointers[i], e);
        /* FIXME: Check for a time overflow */
        if (r > 0) {
            node->key1 = (int) e->tick;
            node->key2 = i;
            node->pointer = e;
        } else {
            node->key1 = INT_MAX;
        }
        pq_update_key1_p3(pq, node);
    }

    if ((s != NULL) && (s->size > 0)) {
        tempo_change_time = MIDI_TYPE3_GAP;
    }

    while (1) {
        int r, t;
        track_event *e;

        pqnode *node = pq_getmin(pq);
        if ((node->key1 == INT_MAX) || (node->pointer == NULL)) break;

        e = node->pointer;
        t = node->key2; 

        curtime = tempo_change_time + (e->tick - tempo_change_tick) *
                ms_per_tick;

        if (s != NULL) {
            switch (e->status & STATUS_MASK) {
                case EVENT_NOTE_OFF: {
                    int channel = e->status & CHANNEL_MASK;
                    int pitch;
                    vector *n;
                    if (skip_percussion && (channel == MIDI_PERCUSSION_CHANNEL))
                        break;
                    pitch = e->data[0];
                    /* velocity = e->data[1]; */
                    n = playing_notes[channel][pitch];
                    if (n != NULL) {
                        n->dur = ((int) curtime) - n->strt;
                        playing_notes[channel][pitch] = NULL;
                    }
                    }
                    break;
                case EVENT_NOTE_ON: {
                    int channel = e->status & CHANNEL_MASK;
                    int pitch;
                    char velocity;
                    vector *n;
                    if (skip_percussion && (channel == MIDI_PERCUSSION_CHANNEL))
                        break;
                    pitch = e->data[0] & 0x7F;
                    velocity = e->data[1] & 0x7F;
                    n = playing_notes[channel][pitch];
                    if (n != NULL) {
                        n->dur = ((int) curtime) - n->strt;
                        if (velocity == 0) {
                            playing_notes[channel][pitch] = NULL;
                            break;
                        }
                    }
                    n = &s->notes[s->size++];
                    n->strt = (int) curtime;
                    n->dur = 0;
		    if (only_rhythm == 1){
		        n->ptch = (char) 1;
		    } else {
		        n->ptch = (char) pitch;
		    }
                    n->velocity = velocity;
                    n->instrument = (channel << 8) + patch[channel];
                    /*n->instrument = current_instrument[t][channel];*/
                    playing_notes[channel][pitch] = n;

                    /*printf("Note (start:%d, dur:%d, pitch:%d, velocity:%d, channel:%d, track:%d, instrument: %d)\n",
                            n->strt, n->dur, n->ptch, n->velocity, channel, t,
                            n->instrument);*/
                    }
                    break;
                case EVENT_CONTROLLER:
                    if ((e->status == CONTROLLER_ALL_NOTES_OFF) ||
                            (e->status == CONTROLLER_ALL_SOUND_OFF)) {
                        int c;
                        for (c=0; c<MIDI_CHANNELS; ++c) {
                            int p;
                            for (p=0; p<NOTE_PITCHES; ++p) {
                                vector *n = playing_notes[c][p];
                                if (n != NULL) {
                                    n->dur = ((int) curtime) - n->strt;
                                }
                                playing_notes[c][p] = NULL;
                            }
                        }
                    }
                    break;
                case EVENT_PROGRAM_CHANGE: {
                    int channel = e->status & CHANNEL_MASK;
                    patch[channel] = e->data[0];
                    }
                    break;
#if 0
                case EVENT_AFTERTOUCH:
                    break;
                case EVENT_PITCH_WHEEL:
                    break;
                case EVENT_CHANNEL_PRESSURE:
                    break;
                case EVENT_SYSEX:
                    break;
#endif
                default:
                    break;
            }
        }
        if (e->status == EVENT_META) {
            /* Meta event */
            if (e->metatype == EVENT_TEMPO && (division_type != SMPTE)) {
                int tempo = (((int) e->data[0]) << 16) +
                        (((int) e->data[1]) << 8) + ((int) e->data[2]);
                tempo_change_time = curtime;
                tempo_change_tick = e->tick;
                ms_per_tick = ((double) tempo) / (1000.0 * ((double) division));
            }
        }

        if (midi_s != NULL) {
            track_event *te = &midi_s->track_data[t][midi_s->track_size[t]++];
            memcpy(te, e, sizeof(track_event));
            te->tick = curtime;
        }

        /*track_last_tick = e->tick;*/

        /* Parse next event */
        r = parse_event(tracks[t], track_lengths[t], &track_pointers[t], e);
        /* FIXME: Check for a time overflow */
        if (r > 0) node->key1 = (int) e->tick;
        else {
            node->key1 = INT_MAX;
            if ((r == 0) && (midi_s != NULL)) {
                track_event *te =
                        &midi_s->track_data[t][midi_s->track_size[t]++];
                memcpy(te, e, sizeof(track_event));
                te->tick = curtime;    
            }
        }

        pq_update_key1_p3(pq, node);
    }

    if (s != NULL) {
        /* Stop all remaining notes */
        for (i=0; i<MIDI_CHANNELS; ++i) {
            int j;
            for (j=0; j<NOTE_PITCHES; ++j) {
                vector *n = playing_notes[i][j];
                if (n != NULL) {
                    n->dur = ((int) curtime) - n->strt;
                }
            }
        }
        s->duration = (int) curtime;
    }

    pq_free(pq);
    free(track_events);
    free(track_pointers);
    /*free(track_last_tick);*/
}


/**
 * Reads a song from a standard MIDI file.
 *
 * @param file path to the MIDI file
 * @param s pointer to a song data structure for storing the song in
 *        a geometric format. Use NULL to not store song data.
 * @param midi_s pointer to a midi data structure for storing all the events.
 *        Use NULL to not store MIDI data.
 * @param skip_percussion 1 to skip percussion notes (MIDI channel 10),
 *        0 to parse all notes.
 *
 * @return 1 if successful, 0 otherwise
 */
int read_midi_file(const char *path, song *s, midisong *midi_s,
        int skip_percussion) {
    return read_midi_file2(path, s, midi_s, skip_percussion, 0);
}
int read_midi_file2(const char *path, song *s, midisong *midi_s,
        int skip_percussion, const int only_rhythm) {
    int i, last_pos;
    int filesize;
    int format = -1;
    const unsigned char **tracks = NULL;
    int *track_lengths = NULL;
    int header_num_tracks = 0;
    int num_tracks = 0;
    int division_type = PPQN; 
    int division = MIDI_DEFAULT_PPQN_DIVISION;
    unsigned char *buffer;

    buffer = read_file(path, SMF_MIN_SIZE, 10, &filesize);
    if (buffer == NULL) return 0;


    for (i=0; i<filesize - SMF_MIN_SIZE; ++i) {
        if ((buffer[i] == 'M') && (buffer[i+1] == 'T') &&
                (buffer[i+2] == 'h') && (buffer[i+3] == 'd')) {
            i += 8;
            format = midi_parse_short(buffer, &i);
            header_num_tracks = midi_parse_short(buffer, &i) + 1;

            division = midi_parse_short(buffer, &i);
            if (division & 0x8000) {
                /* SMPTE division */
                int rate = ((division & 0xFF00) >> 8);
                division_type = SMPTE;
                division = rate * (division & 0xFF);
            }

            /*printf("MIDI header detected (format:%d, smpte:%d, division:%d, tracks:%d)\n",
                    format + 1, division_type, division, num_tracks);*/
            break;
        }
    }
    if (i > filesize - 8) {
        free(buffer);
        return 0;
    }
    if (format > 3) {
        fprintf(stderr, "Warning in read_midi_file(): unknown MIDI file type %d, parsing anyway...\n", format + 1);
        fprintf(stderr, "    File: %s\n", path);
    } else if (format < 0) {
        /* No valid MIDI header found. Search for track data. */
        format = 1;
        i = 0;
    }

    if (header_num_tracks > 0) {
        tracks = (const unsigned char **) malloc(header_num_tracks *
                sizeof(const unsigned char *));
        track_lengths = (int *) malloc(header_num_tracks * sizeof(int));
    }

    last_pos = i;
    for (; i<filesize - 8; ++i) {
        if ((buffer[i] == 'M') && (buffer[i+1] == 'T') &&
                (buffer[i+2] == 'r') && (buffer[i+3] == 'k')) {
            int len;
            i += 4;
            len = midi_parse_int(buffer, &i);

            if (i + len > i) {
                if (i + len > filesize) len = filesize - i;
                /* If there are more tracks than the header reported,
                 * allocate more space. */
                if (num_tracks >= header_num_tracks) {
                    int j;
                    const unsigned char **new_tracks =
                            (const unsigned char **) malloc(2 * num_tracks *
                            sizeof(const unsigned char *));
                    int *new_track_lengths = (int *) malloc (2 * num_tracks *
                            sizeof(int));
                    for (j=0; j<header_num_tracks; ++j) {
                        new_tracks[j] = tracks[j];
                        new_track_lengths[j] = track_lengths[j];
                    }
                    free(tracks);
                    free(track_lengths);
                    tracks = new_tracks;
                    track_lengths = new_track_lengths;
                    header_num_tracks = 2 * num_tracks;
                }
                tracks[num_tracks] = buffer + i;
                track_lengths[num_tracks] = len;
                ++num_tracks;
            }

            i += len - 1;
            last_pos = i + 1;
        } else {
            /* Unknown chunk, try to skip */
            int len;
            i += 4;
            len = midi_parse_int(buffer, &i);
            if ((i + len > i) && (i + len < filesize - 8)) {
                i += len - 1;
            } else {
                /* Skipping failed, search for a track chunk ID */
                for (i = last_pos; i<filesize - 8; ++i) {
                    if ((buffer[i] == 'M') && (buffer[i+1] == 'T') &&
                        (buffer[i+2] == 'r') && (buffer[i+3] == 'k')) {
                        i--;
                        break;
                    }
                }
            }
        }
    }
    if (num_tracks > 0) {
        int num_events = filesize >> 1;

        if (s != NULL) {
            s->notes = (vector *) malloc(num_events * sizeof(vector));
            if (s->notes == NULL) goto EXIT;
            s->size = 0;
            s->duration = 0;
        }

        if (format == 2) {
            /* Concatenate tracks of Type 3 MIDI files */
            if (midi_s != NULL) {
                midi_s->track_size = (int *) malloc(sizeof(int));
                midi_s->track_data = (track_event **) malloc(sizeof(
                        track_event *));
                if ((midi_s->track_size == NULL) ||
                        (midi_s->track_data == NULL)) {
                    free(midi_s->track_size);
                    free(midi_s->track_data);
                    goto EXIT;
                }
                midi_s->track_size[0] = 0;
                midi_s->track_data[0] = (track_event *) malloc(num_events *
                        sizeof(track_event));
                if (midi_s->track_data[0] == NULL) {
                    free(midi_s->track_size);
                    free(midi_s->track_data);
                    goto EXIT;
                }
                midi_s->num_tracks = 1;
                midi_s->buffer = buffer;
                midi_s->buffer_size = filesize;
            }

            for (i=0; i<num_tracks; ++i) {
                parse_tracks2(&tracks[i], &track_lengths[i], 1, s, midi_s,
                        division_type, division, skip_percussion, only_rhythm);
            }
        } else {
            /* Normal Type 1 or Type 2 file is parsed directly */

            if (midi_s != NULL) {
                midi_s->track_size = (int *) malloc(num_tracks * sizeof(int));
                midi_s->track_data = (track_event **) malloc(num_tracks *
                        sizeof(track_event *));
                if ((midi_s->track_size == NULL) ||
                        (midi_s->track_data == NULL)) {
                    free(midi_s->track_size);
                    free(midi_s->track_data);
                    goto EXIT;
                }
                for (i=0; i<num_tracks; ++i) {
                    midi_s->track_size[i] = 0;
                    midi_s->track_data[i] = (track_event *) malloc(
                            (track_lengths[i] >> 1) * sizeof(track_event));
                    if (midi_s->track_data[i] == NULL) {
                        for (--i; i>=0; --i) {
                            free(midi_s->track_data[i]);
                        }
                        free(midi_s->track_size);
                        free(midi_s->track_data);
                        goto EXIT;
                    }
                }
                midi_s->num_tracks = num_tracks;
                midi_s->buffer = buffer;
                midi_s->buffer_size = filesize;
            }

            parse_tracks2(tracks, track_lengths, num_tracks, s, midi_s,
                    division_type, division, skip_percussion, only_rhythm);
        }

        if (MIDI_READ_TRIM_BUFFERS) {
            if (s != NULL) {
                vector *notes = (vector *) malloc(s->size * sizeof(vector));
                if (notes != NULL) {
                    memcpy(notes, s->notes, s->size * sizeof(vector));
                    free(s->notes);
                    s->notes = notes;
                }
            }
            if (midi_s != NULL) {
                for (i=0; i<num_tracks; ++i) {
                    int tsize = midi_s->track_size[i];
                    track_event *events = (track_event *)
                            malloc(tsize * sizeof(track_event));
                    if (events != NULL) {
                        memcpy(events, midi_s->track_data[i], tsize *
                                sizeof(track_event));
                        free(midi_s->track_data[i]);
                        midi_s->track_data[i] = events;
                    }
                }
            }
        }

        /* Name the song */
        i = strlen(path);
        if (s != NULL) {
            s->title = (char *) malloc((i+1) * sizeof(char));
            if (s->title != NULL) {
                memcpy(s->title, path, (i+1) * sizeof(char));
            }
        }
        if (midi_s != NULL) {
            midi_s->name = (char *) malloc((i+1) * sizeof(char));
            if (midi_s->name != NULL) {
                memcpy(midi_s->name, path, (i+1) * sizeof(char));
            }
        }

EXIT:
        if ((midi_s == NULL) || (midi_s->buffer != buffer)) {
            free(buffer);
        }
        free(tracks);
        free(track_lengths);
        return 1;
    } else {
        free(buffer);
        return 0;
    }
}

/**
 * Writes a MIDI song to a standard MIDI file.
 *
 * @param file output file path
 * @param midi_s pointer to a midi data structure
 *
 * @return 1 if successful, 0 otherwise
 */
int write_midi_file(const char *path, const midisong *midi_s,
        int force_leading_silence) {
    FILE *f;
    int i, pos;
    int bufferlen = MIDI_HEADER_SIZE;
    int ret = 1;
    int num_tracks = midi_s->num_tracks;
    int division = MIDI_DEFAULT_PPQN_DIVISION;
    int tempo = MIDI_DEFAULT_TEMPO;
    double ticks_per_ms;
    unsigned char *buffer;

    /* Estimate track lengths in bytes to reserve a memory buffer */
    for (i=0; i<num_tracks; ++i) {
        int j;
        for (j=0; j<midi_s->track_size[i]; ++j) {
            bufferlen += 10 + midi_s->track_data[i][j].length;
        }
        bufferlen += MIDI_TRACK_HEADER_SIZE;
    }

    buffer = (unsigned char *) malloc(bufferlen * sizeof(unsigned char));

    /* MIDI header */
    memcpy(buffer, "MThd", 4);
    pos = 4;
    midi_write_int(6, buffer, &pos);
    midi_write_short(MIDI_DEFAULT_FORMAT, buffer, &pos);
    midi_write_short(num_tracks, buffer, &pos);
    midi_write_short(division, buffer, &pos);

    ticks_per_ms = (double) division / (double) tempo;

    /* Tracks */
    for (i=0; i<num_tracks; ++i) {
        int j = 0, track_length_pos;
        int eot = 0;
        double lasttick = 0.0;
        memcpy(&buffer[pos], "MTrk", 4);
        pos += 4;
        track_length_pos = pos;
        pos += 4;
        if ((force_leading_silence) && (i == num_tracks - 1)) {
            track_event note;
            unsigned char notedata[2];
            for (; j<midi_s->track_size[i]; ++j) {
                track_event *te = &midi_s->track_data[i][j];
                double tick;
                unsigned int tdelta;
                if (te->tick > 0.0) break;
                /* Skip tempo and end-of-track events */
                if (te->status == EVENT_META) {
                    if (te->metatype == EVENT_TEMPO) continue;
                    else if (te->metatype == EVENT_END_OF_TRACK) {
                        if (j < midi_s->track_size[i] - 1) continue;
                        else eot = 1;
                    }
                }
                tick = te->tick * ticks_per_ms;
                tdelta = (unsigned int) (tick - lasttick);
                midi_write_event(te, tdelta, buffer, &pos);
                lasttick += tdelta;
            }
            notedata[0] = 0;
            notedata[1] = 1;
            /* Note on */
            note.tick = 0.0;
            note.status = EVENT_NOTE_ON;
            note.metatype = 0;
            note.length = 2;
            note.data = notedata;
            midi_write_event(&note, 0, buffer, &pos);
            /* Note off */
            note.status = EVENT_NOTE_OFF;
            midi_write_event(&note, 0, buffer, &pos);
        }
        for (; j<midi_s->track_size[i]; ++j) {
            track_event *te = &midi_s->track_data[i][j];
            double tick;
            unsigned int tdelta;
            /* Skip tempo and end-of-track events */
            if (te->status == EVENT_META) {
                if (te->metatype == EVENT_TEMPO) continue;
                else if (te->metatype == EVENT_END_OF_TRACK) {
                    if (j < midi_s->track_size[i] - 1) continue;
                    else eot = 1;
                }
            }
            tick = te->tick * ticks_per_ms;
            tdelta = (unsigned int) (tick - lasttick);
            midi_write_event(te, tdelta, buffer, &pos);
            lasttick += tdelta;
        }
        /* Add an end-of-track event if there wasn't one */
        if (!eot) {
            track_event te;
            te.tick = 0.0;
            te.status = EVENT_META;
            te.metatype = EVENT_END_OF_TRACK;
            te.length = 0;
            te.data = NULL;
            midi_write_event(&te, 0, buffer, &pos);
        }
        midi_write_int(pos - track_length_pos - 4, buffer, &track_length_pos);
    }


    f = fopen(path, "w");
    if (f == NULL) {
        fprintf(stderr, "ERROR in write_midi_file: failed to open file '%s'\n",
                path);
        ret = 0;
        goto EXIT;
    }
    fwrite(buffer, sizeof(unsigned char), pos, f);
    fclose(f);

EXIT:
    free(buffer);
    return ret;
}

/**
 * Frees MIDI song data.
 *
 * @param midi_s pointer to a MIDI song
 */
void free_midisong(midisong *midi_s) {
    int i;
    for (i=0; i<midi_s->num_tracks; ++i) free(midi_s->track_data[i]);
    free(midi_s->track_data);
    free(midi_s->track_size);
    free(midi_s->buffer);
    free(midi_s->name);
}

