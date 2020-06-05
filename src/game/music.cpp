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

/* This file handles common music interface.
 *
 * Provides common API to other code parts. */

#include "Buzz_inc.h"
#include "pace.h"
#include "utils.h"
#include "music.h"
#include "assert.h"

Music *music = new MusicNone();

// A map of music_tracks to filenames
static struct music_key {
    enum music_track track;
    const char *name;
} music_key[] = {
    { M_ASSEMBLY, "assembly" },
    { M_ASTTRNG, "asttrng" },
    { M_BADNEWS, "badnews" },
    { M_DRUMSM, "drumsm" },
    { M_ELEPHANT, "elephant" },
    { M_FILLER, "filler" },
    { M_FUTURE, "future" },
    { M_GOOD, "good" },
    { M_HARDWARE, "hardware" },
    { M_HISTORY, "history" },
    { M_INTEL, "intel" },
    { M_INTELLEG, "intelleg" },
    { M_INTERLUD, "interlud" },
    { M_LIFTOFF, "liftoff" },
    { M_MISSPLAN, "missplan" },
    { M_NEW1950, "new1950" },
    { M_NEW1970, "new1970" },
    { M_PRES, "pres" },
    { M_PRGMTRG, "prgmtrg" },
    { M_RD, "r&d" },
    { M_SOVTYP, "sovtyp" },
    { M_SUCCESS, "success" },
    { M_SVFUN, "svfun" },
    { M_SVLOGO, "svlogo" },
    { M_SVPORT, "svport2" },
    { M_THEME, "theme" },
    { M_UNSUCC, "unsucc" },
    { M_USFUN, "usfun" },
    { M_USMIL, "usmil" },
    { M_USPORT, "usport2" },
    { M_USSRMIL, "ussrmil" },
    { M_VICTORY, "victory" },
    { M_MAX_MUSIC, NULL },
};

const char * Music::TrackName(enum music_track track)
{
    assert(track < M_MAX_MUSIC);
    return music_key[track].name;
}

// Start playing the given track
void music_start_loop(enum music_track track, int loop)
{
    music->Start(track, loop);
}

// Stop a specific track
void music_stop_track(enum music_track track)
{
    music->Stop(track);
}

// Stop all tracks
void music_stop()
{
    music->Stop();
}

int music_is_playing()
{
    return music->IsPlaying();
}

int music_is_track_playing(enum music_track track)
{
    return music->IsTrackPlaying(track);
}

#if 0
int music_init(const char *impl)
{
    if (music)
	    delete music;
    if (!strcmp(impl,"none"))
	    music = new MusicNone();
    // TODO: Is vorbis support mandatory?
    else if (!strcmp(impl, "vorbis"))
	    music = new MusicVorbis();
    else {
#ifdef MUSIC_DEFAULT
	    music = new MUSIC_DEFAULT();
#else
	    music = new MusicVorbis();
#endif
    }
    return true;
}
#endif
