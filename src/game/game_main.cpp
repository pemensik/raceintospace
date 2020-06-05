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

//****************************************************************
//*Interplay's BUZZ ALDRIN's RACE into SPACE                     *
//*                                                              *
//*Formerly -=> LiftOff : Race to the Moon :: IBM version MCGA   *
//*Copyright 1991 by Strategic Visions, Inc.                     *
//*Designed by Fritz Bronner                                     *
//*Programmed by Michael K McCarty                               *
//*                                                              *
//****************************************************************

#include <errno.h>
#include <ctype.h>
#include <assert.h>
#include <sstream>

#include "display/graphics.h"
#include "display/surface.h"
#include "display/image.h"

#include "filesystem.h"
#include "Buzz_inc.h"
#include "game_main.h"
#include "options.h"
#include "utils.h"
#include "admin.h"
#include "aimast.h"
#include "ast4.h"
#include "endgame.h"
#include "intel.h"
#include "intro.h"
#include "mc.h"
#include "mis_c.h"
#include "museum.h"
#include "newmis.h"
#include "news.h"
#include "place.h"
#include "port.h"
#include "prefs.h"
#include "records.h"
#include "review.h"
#include "start.h"
#include "state_utils.h"
#include "pace.h"
#include "sdlhelper.h"
#include "gr.h"
#include "crash.h"
#include "endianness.h"
#include "crew.h"
#include "music_vorbis.h"

#ifdef CONFIG_MACOSX
// SDL.h needs to be included here to replace the original main() with
// what it needs for the Mac
#include <SDL.h>
#endif

static char df;
LOG_DEFAULT_CATEGORY(LOG_ROOT_CAT);

void InitData(void);
void MMainLoop(void);
void Progress(char mode);
void MainLoop(void);
void DockingKludge(void);
void OpenEmUp(void);
void CloseEmUp(unsigned char error, unsigned int value);

int music_init(const char *impl)
{
    if (music)
	    delete music;
    if (impl && !strcmp(impl,"none"))
	    music = new MusicNone();
    // TODO: Is vorbis support mandatory?
    else if (impl && !strcmp(impl, "vorbis"))
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


int game_main_impl(int argc, char *argv[])
{
    FILE *fin;
    const char *see_readme = "look for further instructions in the README file";

    char ex;

    // initialize the filesystem
    Filesystem::init(argv[0]);

    if (!display::image::libpng_versions_match()) {
        std::stringstream message;
        message
                << "This build was compiled against libpng " << display::image::libpng_headers_version()
                << ", but is running with libpng " << display::image::libpng_runtime_version() << ".";
        crash("libpng mismatch", message.str());
    }

    setup_options(argc, argv);
    Filesystem::addPath(options.dir_gamedata);
    Filesystem::addPath(options.dir_savegame);
    /* hacking... */
    log_setThreshold(&_LOGV(LOG_ROOT_CAT), MAX(0, LP_NOTICE - (int)options.want_debug));

    fin = open_gamedat("USA_PORT.DAT");

    if (fin == NULL) {
        CRITICAL1("can't find game data files");
        NOTICE1("set environment variable BARIS_DATA or edit config file");
        NOTICE2("%s", see_readme);

        crash("Data missing", "Unable to locate game data files.");
    }

    fclose(fin);

    if (create_save_dir() != 0) {
        CRITICAL3("can't create save directory `%s': %s",
                  options.dir_savegame, strerror(errno));
        NOTICE1("set environment variable BARIS_SAVE to a writable directory");
        NOTICE2("%s", see_readme);

        crash("Save directory", "Couldn't create save directory");
    }

    av_setup();
    music_init(getenv("BARIS_MUSIC"));

    helpText = "i000";
    keyHelpText = "k000";

    LOAD = QUIT = 0;

    xMODE = 0;

    xMODE |= xMODE_NOCOPRO;

    Data = (Players *)xmalloc(sizeof(struct Players) + 1);
    buffer = (char *)xmalloc(BUFFER_SIZE);

    DEBUG3("main buffer %p (%d)", buffer, BUFFER_SIZE);

    memset(buffer, 0x00, BUFFER_SIZE);

    OpenEmUp();                   // OPEN SCREEN AND SETUP GOODIES

    if (options.want_intro) {
        Introd();
    }

    ex = 0;

    while (ex == 0) {

        MakeRecords();

#define UNCOMPRESSED_RAST 1

#ifndef UNCOMPRESSED_RAST
        fin = sOpen("RAST.DAT", "rb", 0);
        i = fread(buffer, 1, BUFFER_SIZE, fin);
        fclose(fin);

        DEBUG2("reading Players: size = %d", (int)sizeof(struct Players));
        RLED(buffer, (char *)Data, i);
#else
        fin = sOpen("URAST.DAT", "rb", 0);
        fread(Data, 1, (sizeof(struct Players)), fin);
        fclose(fin);
#endif
        SwapGameDat();  // Take care of endian read

        if (Data->Checksum != (sizeof(struct Players))) {
            /* XXX: too drastic */
            CRITICAL1("wrong version of data file");
            exit(EXIT_FAILURE);
        }

        display::graphics.screen()->clear();
        PortPal(0);
        key = 0;
        helpText = "i000";
        keyHelpText = "i000";
        df = 1;

        music->Start(M_LIFTOFF);

        switch (MainMenuChoice()) {
        case 0:  // New Game
            LOAD = QUIT = 0, BUTLOAD = 0;
            HARD1 = UNIT1 = 0;
            MAIL = -1;
            Option = -1;
            helpText = "i013";
            Prefs(0);                     // GET INITIAL PREFS FROM PLAYER
            plr[0] = Data->Def.Plr1;       // SET GLOBAL PLAYER VALUES
            plr[1] = Data->Def.Plr2;
            Data->plr[0] = Data->Def.Plr1;  // SET STRUCTURE PLAYER VALUES
            Data->plr[1] = Data->Def.Plr2;

            if (plr[0] == 2 || plr[0] == 3) {
                AI[0] = 1;
            } else {
                AI[0] = 0;
            }

            if (plr[1] == 2 || plr[1] == 3) {
                AI[1] = 1;
            } else {
                AI[1] = 0;
            }

            InitData();                   // PICK EVENT CARDS N STUFF
            MainLoop();                   // PLAY GAME
            display::graphics.screen()->clear();
            break;

        case 1: // Play Old Game
            LOAD = QUIT = BUTLOAD = 0;
            HARD1 = UNIT1 = 0;
            MAIL = -1;
            Option = -1;
            FileAccess(1);

            if (LOAD == 1) {
                if (Option == -1 && MAIL == -1) {
                    MainLoop();    //Regular game
                } else { //Modem game
                    WARNING1("can't do modem games");
                    break;
                }
            } else if (!QUIT) {
                FadeOut(2, 10, 0, 0);
            }

            QUIT = 0;
            display::graphics.screen()->clear();
            break;

        case 2:
            df = 0;
            Credits();
            df = 1;
            break;

        case 3:
            //KillMusic();
            ex = 1;
            FadeOut(2, 10, 0, 0);
            break;
        }
    }

    display::graphics.destroy();
    CloseEmUp(0, 0); // Normal Exit
    exit(EXIT_SUCCESS);
}

int game_main(int argc, char *argv[])
{
    // Do all the work in game_main_impl(), but trap exceptions here, since we're called from C
    try {
        return game_main_impl(argc, argv);
    } catch (const std::exception &e) {
        fprintf(stderr, "unhandled exception: %s\n", e.what());
        abort();
    } catch (const std::string &e) {
        fprintf(stderr, "unhandled exception: %s\n", e.c_str());
        abort();
    } catch (...) {
        fprintf(stderr, "unhandled exception of unknown type, terminating\n");
        abort();
    }
}

