/*
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

#pragma once

enum music_track {
    M_ASSEMBLY,
    M_ASTTRNG,
    M_BADNEWS,
    M_DRUMSM,
    M_ELEPHANT,
    M_FILLER,
    M_FUTURE,
    M_GOOD,
    M_HARDWARE,
    M_HISTORY,
    M_INTEL,
    M_INTELLEG,
    M_INTERLUD,
    M_LIFTOFF,
    M_MISSPLAN,
    M_NEW1950,
    M_NEW1970,
    M_PRES,
    M_PRGMTRG,
    M_RD,
    M_SOVTYP,
    M_SUCCESS,
    M_SVFUN,
    M_SVLOGO,
    M_SVPORT,
    M_THEME,
    M_UNSUCC,
    M_USFUN,
    M_USMIL,
    M_USPORT,
    M_USSRMIL,
    M_VICTORY,
    M_MAX_MUSIC
};

class Music
{
public:
	virtual ~Music() {}
	virtual const char *Name() const = 0;

	virtual void Start(enum music_track track, int loop=1) = 0;
	virtual void Stop(enum music_track track) = 0;
	virtual void Stop() = 0;
	virtual bool IsPlaying() = 0;
	virtual bool IsTrackPlaying(enum music_track track) = 0;
	virtual void Pump() = 0;
	virtual void SetMute(bool muted) = 0;

	static const char *TrackName(enum music_track track);
};

class MusicNone : public Music
{
public:
	virtual ~MusicNone() {}
	virtual const char *Name() const { return "none"; }
	virtual void Start(enum music_track track, int loop=1);
	virtual void Stop(enum music_track track);
	virtual void Stop();
	virtual bool IsPlaying();
	virtual bool IsTrackPlaying(enum music_track track);
	virtual void Pump() {}
	virtual void SetMute(bool muted) {}
};

extern Music *music;
