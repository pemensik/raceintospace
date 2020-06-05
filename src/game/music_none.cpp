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

/* This file handles music ignorance.
 *
 * Its purpose is to compile, when no music would be provided at all. */

#include "Buzz_inc.h"
#include "pace.h"
#include "utils.h"
#include "music.h"

// This structure defines each track
struct music_file {
    // Is this track playing?
    int playing;
};
static struct music_file music_files[M_MAX_MUSIC];

/* Follows implementation of fake music player */

void MusicNone::Start(enum music_track track, int loop)
{
    music_files[track].playing = 1;
}

void MusicNone::Stop(enum music_track track)
{
    if (music_files[track].playing) {
        music_files[track].playing = 0;
    }
}

void MusicNone::Stop()
{
    int i;

    // Iterate through the list and stop any playing tracks by calling music_stop_track()
    for (i = 0; i < M_MAX_MUSIC; i ++) {
        if (music_files[i].playing) {
            Stop((music_track)i);
        }
    }
}

bool MusicNone::IsPlaying()
{
    int i;

    for (i = 0; i < M_MAX_MUSIC; i ++) {
        if (music_files[i].playing)
		return true;
    }
    return false;
}

bool MusicNone::IsTrackPlaying(enum music_track track)
{
    return music_files[track].playing;
}

