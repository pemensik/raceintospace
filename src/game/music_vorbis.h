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

#include "music.h"

class MusicVorbis : public Music
{
public:
	virtual ~MusicVorbis();
	virtual const char *Name() const { return "vorbis"; }
	virtual void Start(enum music_track track, int loop=1);
	virtual void Stop(enum music_track track);
	virtual void Stop();
	virtual bool IsPlaying();
	virtual bool IsTrackPlaying(enum music_track track);
	virtual void Pump() {}
	virtual void SetMute(bool muted);
};

