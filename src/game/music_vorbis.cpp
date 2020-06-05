/*
 Copyright (C) 2007 Will Glynn

 Derived from music.c:
 Copyright (C) 2005 Michael K. McCarty & Fritz Bronner

 This program is free software; you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation; either version 2 of the License, or
 (at your option) any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program; if not, write to the Free Software
 Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

// This file handles music (as you might guess by the name)

#include "Buzz_inc.h"
#include "pace.h"
#include "utils.h"
#include "sdlhelper.h"

#include "music.h"
#include "music_vorbis.h"

// This structure defines each track
struct music_file {
    // Pointer to the audio buffer, once loaded from disk
    char *buf;
    size_t buf_size;

    // Can this track be played? i.e., does it exist?
    int unplayable;

    // Is this track playing?
    int playing;

    // The SDL audio chunk describing this track
    struct audio_chunk chunk;
};
struct music_file music_files[M_MAX_MUSIC];

// Ensure that the specified music track is in memory, loading it if required
static void music_load(enum music_track track)
{
    char fname[20] = "";
    const char *trackname;
    int i;
    ssize_t bytes;

    // Check to see if the track is already loaded or known broken; if so, we're already done
    if (music_files[track].buf != NULL || music_files[track].unplayable) {
        return;
    }

    // Find the name for this track
    trackname = Music::TrackName(track);
    snprintf(fname, sizeof(fname), "%s.OGG", trackname);

    // Bail out if this track isn't known
    if (strlen(fname) == 0) {
        music_files[track].unplayable = 1;
        return;
    }

    // Load the file
    bytes = load_audio_file(fname, &music_files[track].buf, &music_files[track].buf_size);

    // Handle failure gracefully
    if (bytes < 0) {
        free(music_files[track].buf);
        music_files[track].unplayable = 1;
        return;
    }

    // XXX: Trim the last two seconds from the audio file, since it's broken
    // This should really be done on the Vorbis files, rather than in the player
    if (bytes > 2 * 11025) {
        music_files[track].buf_size -= 2 * 11025;
        music_files[track].buf = (char *)xrealloc(music_files[track].buf, music_files[track].buf_size);
    }
}

void music_pump()
{
    // TODO: Check to see that all the tracks we think are playing actually are
    // This doesn't apply to looped tracks, since those keep playing forever
}


MusicVorbis::~MusicVorbis()
{
    Stop();
}

void MusicVorbis::Start(enum music_track track, int loop)
{
    // Load the track as necessary
    music_load(track);

    // Ensure that this track is playable
    if (music_files[track].unplayable) {
        return;
    }

    // Ignore requests to play a track that's already playing
    if (music_files[track].playing) {
        return;
    }

    // Initialize the audio chunk
    music_files[track].chunk.data = music_files[track].buf;
    music_files[track].chunk.size = music_files[track].buf_size;
    music_files[track].chunk.loop = loop;

    // XXX: Stop the existing music, since we need the music channel
    // This should be changed to dynamic channel allocation, to allow layering music tracks
    Stop();

    // Play the track, and indicate that it's playing
    play(&music_files[track].chunk, AV_MUSIC_CHANNEL);
    music_files[track].playing = 1;
}

void MusicVorbis::Stop(enum music_track track)
{
    if (music_files[track].playing) {
        // XXX: stop the global music channel
        // This should be assigned on a per-track basis
        av_silence(AV_MUSIC_CHANNEL);

        music_files[track].playing = 0;
    }
}

void MusicVorbis::Stop()
{
    int i;

    // Iterate through the list and stop any playing tracks by calling music_stop_track()
    for (i = 0; i < M_MAX_MUSIC; i ++) {
        if (music_files[i].playing) {
            Stop((music_track)i);
        }
    }
}

bool MusicVorbis::IsPlaying()
{
    int i;

    for (i = 0; i < M_MAX_MUSIC; i ++) {
        if (music_files[i].playing) {
            return true;
        }
    }

    return false;
}

bool MusicVorbis::IsTrackPlaying(enum music_track track)
{
    return music_files[track].playing;
}

void MusicVorbis::SetMute(bool muted)
{
    MuteChannel(AV_MUSIC_CHANNEL, muted);
}
