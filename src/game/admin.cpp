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
// Interplay's BUZZ ALDRIN's RACE into SPACE
//
// Formerly -=> LiftOff : Race to the Moon :: IBM version MCGA
// Copyright 1991 by Strategic Visions, Inc.
// Designed by Fritz Bronner
// Programmed by Michael K McCarty
//
// All Administration Main Files
/** \file admin.c Responsible for the Administration office.
 *
 */

// This file handles the Administration building and some of its subsections: Future Missions, Time Capsule (save/load game).
// It also includes some code for Modem and PBEM.

#include <assert.h>

#include <algorithm>
#include <vector>

#include "display/graphics.h"
#include "display/surface.h"
#include "display/palettized_surface.h"

#include "admin.h"
#include "Buzz_inc.h"
#include "utils.h"
#include "ast1.h"
#include "draw.h"
#include "budget.h"
#include "future.h"
#include "game_main.h"
#include "place.h"
#include "port.h"
#include "prefs.h"
#include "rdplex.h"
#include "mc.h"
#include "mission_util.h"
#include "sdlhelper.h"
#include "gr.h"
#include "pace.h"
#include "endianness.h"
#include "filesystem.h"

#include <ctype.h>

#define MODEM_ERROR 4
#define NOTSAME 2
#define SAME_ABORT 0
#define YES 1

LOG_DEFAULT_CATEGORY(LOG_ROOT_CAT)

void DrawFiles(char now, char loc, const std::vector<SFInfo> &savegames);
void DrawTimeCapsule(int display);
std::vector<SFInfo> GenerateTables(SaveGameType saveType);
char GetBlockName(char *Nam);
SaveGameType GetSaveType(const SaveFileHdr &header);
void BadFileType();
void FileText(char *name);
int FutureCheck(char plr, char type);
void LoadGame(const char *filename);
bool OrderSaves(const SFInfo &a, const SFInfo &b);
char RequestX(char *s, char md);

namespace
{
enum FileButtons {
    ENABLE_LOAD = 0x01,
    ENABLE_SAVE = 0x02,
    ENABLE_PBEM = 0x04,
    ENABLE_DELETE = 0x08,
    ENABLE_PLAY = 0x10,
    ENABLE_QUIT = 0x20
};
};


/* Control loop for the Administration Office menu.
 *
 * This provides access to the Budget Office, Hardware Purchase,
 * Future Missions, Astronaut Recruiting, Preferences, and Time Capsule.
 *
 * \param plr  The current player.
 */
void Admin(char plr)
{
    int i, beg;
    char AName[7][22] = {"BUDGET OFFICE", "HARDWARE PURCHASE", "FUTURE MISSIONS",
                         "ASTRONAUT RECRUITING", "PREFERENCES", "TIME CAPSULE", "EXIT ADMINISTRATION"
                        };
    char AImg[7] = {1, 2, 3, 4, 6, 7, 0};

    if (plr == 1) {
        strncpy(&AName[3][0], "COSMO", 5);
    }

    if (Data->P[plr].AstroDelay > 0) {
        AImg[3] = 5;
    }

    beg = 1;

    do {
        if (beg) {
            beg = 0;
        } else {

            FadeOut(2, 10, 0, 0);

            DrawSpaceport(plr);
            PortPal(plr);
            fill_rectangle(166, 191, 318, 198, 3);
            display::graphics.setForegroundColor(0);
            draw_string(257, 197, "CASH:");
            draw_megabucks(285, 197, Data->P[plr].Cash);
            display::graphics.setForegroundColor(11);
            draw_string(256, 196, "CASH:");
            draw_megabucks(284, 196, Data->P[plr].Cash);
            display::graphics.setForegroundColor(0);

            if (Data->Season == 0) {
                draw_string(166, 197, "SPRING 19");
            } else {
                draw_string(166, 197, "FALL 19");
            }

            draw_number(0, 0, Data->Year);
            display::graphics.setForegroundColor(11);

            if (Data->Season == 0) {
                draw_string(165, 196, "SPRING 19");
            } else {
                draw_string(165, 196, "FALL 19");
            }

            draw_number(0, 0, Data->Year);

            FadeIn(2, 10, 0, 0);
        }

        music_start(M_GOOD);

        helpText = (plr == 0) ? "i702" : "i703";
        keyHelpText = (plr == 0) ? "k601" : "k602";

        i = BChoice(plr, 7, (char *)AName, AImg);

        switch (i) {
        case 1:
            music_stop();
            music_start(M_DRUMSM);
            helpText = "i007";
            keyHelpText = "k007";
            Budget(plr);
            key = 0;
            music_stop();
            break;

        case 2:
            music_stop();
            helpText = "i008";
            HPurc(plr);
            key = 0;
            break;

        case 3:
            helpText = "i010";
            keyHelpText = "k010";
            music_stop();
            music_start(M_MISSPLAN);
            Future(plr);
            music_stop();
            key = 0;
            break;

        case 4:
            music_stop();
            helpText = "i012";
            keyHelpText = "k012";
            AstSel(plr);
            key = 0;
            break;

        case 5:
            music_stop();
            helpText = "i013";
            keyHelpText = "k013";
            Prefs(1);
            key = 0;
            break;

        case 6:
            helpText = "i128";
            keyHelpText = "k128";
            FileAccess(0);
            key = 0;
            break;

        case 7:
            break;

        default:
            key = 0;
            break;

        }

        if (Data->P[plr].AstroDelay > 0) {
            AImg[3] = 5;
        }
    } while (!(i == 7 || (i == 6 && (QUIT || LOAD))));

    music_stop();
    helpText = "i000";
    keyHelpText = "k000";
    WaitForMouseUp();
}


/**
 * Copies the astronaut/cosmonaut pools into the cache.
 *
 * Since the game supports both historical and custom-defined rosters,
 * depending on the game setting the appropriate roster is copied into
 * a cache - MEN.DAT - so the game will be assured of having roster
 * data available.
 *
 * TODO: Eliminate use of global buffer.
 */
void CacheCrewFile()
{
    if (Data->Def.Input % 2 == 0) {
        // Historical Crews
        FILE *fin = sOpen("CREW.DAT", "rb", FT_DATA);
        size_t size = fread(buffer, 1, BUFFER_SIZE, fin);
        fclose(fin);
        fin = sOpen("MEN.DAT", "wb", FT_SAVE);
        fwrite(buffer, size, 1, fin);
        fclose(fin);
    } else {
        // User Crews
        FILE *fin = sOpen("USER.DAT", "rb", FT_SAVE);

        if (!fin) {
            fin = sOpen("USER.DAT", "rb", FT_DATA);
        }

        size_t size = fread(buffer, 1, BUFFER_SIZE, fin);
        fclose(fin);
        fin = sOpen("MEN.DAT", "wb", FT_SAVE);
        fwrite(buffer, size, 1, fin);
        fclose(fin);
    }
}


/* Creates a list of all the save files of the selected type, up to 100.
 *
 * Entries are ordered by save title.
 *
 * Currently, only the first 100 valid savegames are detected.
 *
 * \param saveType  Include Normal, Modem, or Play by Email.
 * \return  Entries ordered by save title.
 */
std::vector<SFInfo> GenerateTables(SaveGameType saveType)
{
    struct ffblk ffblk;
    const int maxSaves = 100;
    SaveFileHdr header;
    SFInfo saveInfo;
    std::vector<SFInfo> results;

    for (bool done = first_saved_game(&ffblk);
         done != true && results.size() < maxSaves;
         done = next_saved_game(&ffblk)) {

        if (strlen(ffblk.ff_name) > sizeof(saveInfo.Name) - 1) {
            NOTICE2("Save name too long: %s, skipping", ffblk.ff_name);
            continue;
        }

        FILE *fin = sOpen(ffblk.ff_name, "rb", 1);

        if (fin == NULL) {
            NOTICE2("Unable to open save file %s, skipping",
                    ffblk.ff_name);
            continue;
        }

        size_t bytes = fread(&header, 1, sizeof(header), fin);
        fclose(fin);

        if (bytes != sizeof(header)) {
            NOTICE2("Unable to read save file %s, skipping",
                    ffblk.ff_name);
            continue;
        }

        if (saveType == SAVEGAME_Normal ||
            saveType == GetSaveType(header)) {
            memset(&saveInfo, 0, sizeof(saveInfo));
            strcpy(saveInfo.Title, header.Name);
            strcpy(saveInfo.Name, ffblk.ff_name);
            saveInfo.time = ffblk.ff_ftime;
            saveInfo.date = ffblk.ff_fdate;
            results.push_back(saveInfo);
        }
    }

    std::sort(results.begin(), results.end(), OrderSaves);
    return results;
}


/* Draws the File save/load interface and manages the GUI.
 *
 * To determine which menu options are accessible, this uses the
 * global MAIL and Option variables. For normal games, MAIL & Option
 * are set to -1. These are used to identify the current player in a
 * Play-by-mail or Modem game, being set to the side of the player:
 * 0 for Player 1 (USA), 1 for Player 2 (USSR). Practically, both
 * Play-by-mail and Modem play are currently disabled.
 *
 * \param mode  0 if saving is allowed, 1 if not, 2 if only email saves.
 */
void FileAccess(char mode)
{
    char sc = 0;
    int i, now, done, BarB, temp;
    FILE *fin, *fout;
    char Name[12];
    SaveGameType saveType = SAVEGAME_Normal;
    const int SCRATCH_SIZE = 64000;
    char scratch[SCRATCH_SIZE];  // scratch buffer, will be tossed automatically at the end of the routine

    //sp. case -> no regular save off mail/modem game
    if ((mode == 0 || mode == 1) && (MAIL != -1 || Option != -1)) {
        sc = 2;
    }

    // TODO: Since mode == 2 is checked later, when it cannot occur,
    // I'm guessing this is some kind of hack added at a later time.
    // This needs to be deciphered and better documented. -- rnyoakum
    if (mode == 2) {
        mode = 0;
        sc = 1;  //only allow mail save
    }

    helpText = "i128";
    keyHelpText = "k128";
    FadeOut(2, 10, 0, 0);
    display::graphics.screen()->clear();

    saveType = SAVEGAME_Normal;

    if (Option != -1) {
        saveType = SAVEGAME_Modem;
    } else if (mode == 2) {
        saveType = SAVEGAME_PlayByMail;
    }

    std::vector<SFInfo> savegames = GenerateTables(saveType);

    int enable = ENABLE_PLAY | ENABLE_QUIT;

    if (mode == 0) {
        if (sc == 0 || sc == 2) {
            enable |= ENABLE_SAVE;
        } else if (sc == 1) {
            enable |= ENABLE_PBEM;
        }
    }

    if (savegames.size() > 0) {
        enable |= ENABLE_DELETE;

        if (sc == 0 || sc == 2) {
            enable |= ENABLE_LOAD;
        }
    }

    DrawTimeCapsule(enable);

    done = BarB = now = 0;
    DrawFiles(0, 0, savegames);

    if (savegames.size()) {
        FileText(&savegames[now].Name[0]);
    }

    FadeIn(2, 10, 0, 0);


    while (!done) {
        GetMouse();

        for (i = 0; i < 9; i++) {
            // Right Select Box
            if (x >= 40 && y >= (53 + i * 8) && x <= 188 && y <= (59 + i * 8) && mousebuttons > 0 && (now - BarB + i) <= (savegames.size() - 1)) {

                now -= BarB;
                now += i;
                BarB = i;
                DrawFiles(now, BarB, savegames);
                FileText(&savegames[now].Name[0]);
                WaitForMouseUp();
            }
        }

        if ((sc == 0 || sc == 2) && savegames.size() > 0 && ((x >= 209 && y >= 50 && x <= 278 && y <= 58 && mousebuttons > 0)
                || (key == 'L'))) {
            InBox(209, 50, 278, 58);
            delay(250);

            if (mode == 1) {
                temp = 1;
            } else {
                temp = Help("i101");
            }

            WaitForMouseUp();

            if (temp >= 0) {
                // Read in Saved game data
                LoadGame(savegames[now].Name);
                done = LOAD;

                if (LOAD) {
                    fOFF = now;  // save file offset
                }
            }  // temp

            OutBox(209, 50, 278, 58);  // Button Out
            key = 0;

        }  // LOAD
        else if ((sc == 0 || sc == 2) && mode == 0 && ((x >= 209 && y >= 64 && x <= 278 && y <= 72 && mousebuttons > 0)
                 || (key == 'S'))) {
            InBox(209, 64, 278, 72);
            delay(250);

            WaitForMouseUp();

            SaveFileHdr header;
            memset(&header, 0x00, sizeof(header));
            done = GetBlockName(header.Name);  // Checks Free Space
            header.ID = RaceIntoSpace_Signature;
            header.Name[sizeof(header.Name) - 1] = 0x1A;
            temp = NOTSAME;

            for (i = 0; (i < savegames.size() && temp == 2); i++) {
                if (strcmp(header.Name, savegames[i].Title) == 0) {
                    temp = RequestX("REPLACE FILE", 1);

                    if (temp == SAME_ABORT) {
                        done = 0;
                    }
                }
            }

            if (done == YES) {
                i--;  // decrement to correct for the FOR loop
                strcpy(header.PName[0], Data->P[plr[0] % 2].Name);
                strcpy(header.PName[1], Data->P[plr[1] % 2].Name);

                // Modem save game hack
                if (Option != -1) {
                    if (Option == 0) {
                        plr[0] = 6;
                        plr[1] = 1;
                    } else if (Option == 1) {
                        plr[0] = 0;
                        plr[1] = 7;
                    }

                    Data->Def.Plr1 = plr[0];
                    Data->Def.Plr2 = plr[1];
                    Data->plr[0] = Data->Def.Plr1;
                    Data->plr[1] = Data->Def.Plr2;
                    AI[0] = 0;
                    AI[1] = 0;
                }

                header.Country[0] = Data->plr[0];
                header.Country[1] = Data->plr[1];
                header.Season = Data->Season;
                header.Year = Data->Year;
                header.dataSize = sizeof(struct Players);

                // Copy in the end-of-turn save data
                if (interimData.endTurnSaveSize) {
                    header.compSize = interimData.endTurnSaveSize;
                    memcpy(scratch, interimData.endTurnBuffer, interimData.endTurnSaveSize);
                } else {
                    header.compSize = 0;
                    memset(scratch, 0, SCRATCH_SIZE);
                }

                if (temp == NOTSAME) {
                    i = 0;
                    fin = NULL;

                    do {
                        if (fin) {
                            fclose(fin);
                        }

                        i++;
                        snprintf(Name, sizeof(Name), "BUZZ%d.SAV", i);
                        fin = sOpen(Name, "rb", FT_SAVE_CHECK);
                    } while (fin != NULL);  // Find unique name

                    fin = sOpen(Name, "wb", 1);
                } else {
                    fin = sOpen(savegames[i].Name, "wb", 1);
                }

                fwrite(&header, sizeof(header), 1, fin);

                //-----------------------------------
                // Specs: Special Modem Save Kludge |
                //-----------------------------------
                if (Option != -1) {
                    if (Option == 0) {
                        plr[0] = 0;
                        plr[1] = 1;
                    } else if (Option == 1) {
                        plr[0] = 0;
                        plr[1] = 1;
                    }

                    Data->Def.Plr1 = plr[0];
                    Data->Def.Plr2 = plr[1];
                    Data->plr[0] = Data->Def.Plr1;
                    Data->plr[1] = Data->Def.Plr2;
                    AI[0] = 0;
                    AI[1] = 0;
                }

                // Save End Of Turn State
                fwrite(interimData.endTurnBuffer, interimData.endTurnSaveSize, 1, fin);

                // Save Replay information
                fwrite(interimData.tempReplay, interimData.replaySize, 1, fin);

                // Save Event information
                fwrite(interimData.eventBuffer, interimData.eventSize, 1, fin);

                fclose(fin);
            }  // end done if

            OutBox(209, 64, 278, 72);
            key = 0;
        } else if (sc == 1 && mode == 0 && ((x >= 209 && y >= 78 && x <= 278 && y <= 86 && mousebuttons > 0)
                                            || (key == 'M'))) { // PLAY-BY-MAIL SAVE GAME
            MailSave();

            OutBox(209, 78, 278, 86);
            key = 0;
            QUIT = 1;
            return;
        } else if (savegames.size() > 0 && ((x >= 209 && y >= 92 && x <= 278 && y <= 100 && mousebuttons > 0)
                                            || (key == 'D'))) {
            InBox(209, 92, 278, 100);
            delay(250);
            WaitForMouseUp();
            OutBox(209, 92, 278, 100);
            // perform delete
            i = RequestX("DELETE FILE", 1);

            if (i == 1) {

                remove_savedat(savegames[now].Name);
                memset(Name, 0x00, sizeof Name);
                saveType = SAVEGAME_Normal;

                if (Option != -1) {
                    saveType = SAVEGAME_Modem;
                } else if (mode == 2) {
                    saveType = SAVEGAME_PlayByMail;
                }

                savegames = GenerateTables(saveType);
                now = 0;
                BarB = 0;
                DrawFiles(now, BarB, savegames);
                FileText(&savegames[now].Name[0]);

                if (savegames.size() == 0) {
                    InBox(207, 48, 280, 60);
                    fill_rectangle(208, 49, 279, 59, 3);
                    display::graphics.setForegroundColor(1);
                    draw_string_highlighted(233, 56, "LOAD", 0);
                    InBox(207, 90, 280, 102);
                    fill_rectangle(208, 91, 279, 101, 3);
                    display::graphics.setForegroundColor(1);
                    draw_string_highlighted(226, 98, "DELETE", 0);
                }

            }

            key = 0;
        } else if ((x >= 209 && y >= 106 && x <= 278 && y <= 114 && mousebuttons > 0) || (key == 'P')) {
            InBox(209, 106, 278, 114);
            delay(250);
            WaitForMouseUp();
            OutBox(209, 106, 278, 114);
            key = 0;
            done = 1;
        } else if ((x >= 209 && y >= 120 && x <= 278 && y <= 128 && mousebuttons > 0) || (key == 'Q')) {
            InBox(209, 120, 278, 128);
            delay(250);
            WaitForMouseUp();
            OutBox(209, 120, 278, 128);
            // perform quit
            i = RequestX("QUIT", 1);

            // Modem Play => reset the modem
            if (Option != -1 && i == 1) {
                DoModem(2);
            }

            if (i == 1) {
                MAIL = Option = -1;
                QUIT = done = 1;
            }

            key = 0;
        } else if ((x >= 191 && y >= 50 && x <= 202 && y <= 87 && mousebuttons > 0) || key == UP_ARROW) {
            InBox(191, 50, 202, 87);

            if (BarB == 0) {
                if (now > 0) {
                    now--;
                    DrawFiles(now, BarB, savegames);
                    FileText(&savegames[now].Name[0]);
                }
            }

            if (BarB > 0) {
                BarB--;
                now--;
                DrawFiles(now, BarB, savegames);
                FileText(&savegames[now].Name[0]);
            }

            // WaitForMouseUp();
            OutBox(191, 50, 202, 87);

            // perform Up Button
            key = 0;
        } else if (key == K_PGUP) { // Page Up

            if (now > 0) {
                now -= 9;

                if (now < 0) {
                    now = 0;
                    BarB = 0;
                }

                DrawFiles(now, BarB, savegames);
                FileText(&savegames[now].Name[0]);
            }

            // perform Up Button
            key = 0;

        } else if (key == K_PGDN) {  // Page Down

            if (now < (savegames.size() - 9)) {
                now += 9;

                if (now > (savegames.size() - 1)) {
                    now = savegames.size() - 1;
                    BarB = 8;
                }

                DrawFiles(now, BarB, savegames);
                FileText(&savegames[now].Name[0]);
            }

            key = 0;
        } else if ((x >= 191 && y >= 89 && x <= 202 && y <= 126 && mousebuttons > 0) || key == DN_ARROW) {
            InBox(191, 89, 202, 126);

            if (BarB == 8) {
                if (now < (savegames.size() - 1)) {
                    now++;
                    DrawFiles(now, BarB, savegames);
                    FileText(&savegames[now].Name[0]);
                }
            }

            if (BarB < 8 && now < (savegames.size() - 1)) {
                BarB++;
                now++;
                DrawFiles(now, BarB, savegames);
                FileText(&savegames[now].Name[0]);
            }


            // WaitForMouseUp();
            OutBox(191, 89, 202, 126);

            // perform Down Button
            key = 0;
        }
    }  //while

    if (LOAD == 1) {
        OutBox(209, 50, 278, 60);  // Button Out
    }

    if (mode == 1 && QUIT == 1) {
        FadeOut(2, 10, 0, 0);
    }
}


/* Displays (a section of) the savegame files.
 *
 * Up to 9 files are shown at a given time.
 *
 * \param now     The savegame index of the current save file.
 * \param loc     The display index of the current save file.
 * \param savegames  TODO
 */
void DrawFiles(char now, char loc, const std::vector<SFInfo> &savegames)
{
    int j = 0;
    int start = now - loc;

    fill_rectangle(38, 49, 190, 127, 0);

    if (savegames.size() > 0) {
        ShBox(39, 52 + loc * 8, 189, 60 + loc * 8);
    }

    display::graphics.setForegroundColor(1);

    for (int i = start; i < start + 9 && i < savegames.size(); i++, j++) {
        draw_string(40, 58 + j * 8, savegames[i].Title);
    }
}


/**
 * Display the savegame file interface GUI.
 *
 * \param display  flags indicating which buttons to enable.
 */
void DrawTimeCapsule(int display)
{
    ShBox(34, 32, 283, 159);
    InBox(37, 35, 204, 45);
    fill_rectangle(38, 36, 203, 44, 7);
    InBox(207, 35, 280, 45);
    fill_rectangle(208, 36, 279, 44, 10);
    InBox(37, 48, 204, 128);
    fill_rectangle(38, 49, 203, 127, 0);
    InBox(37, 132, 280, 156);
    ShBox(191, 50, 202, 87);
    draw_up_arrow(194, 55);
    ShBox(191, 89, 202, 126);
    draw_down_arrow(194, 94);

    if (display & ENABLE_LOAD) {
        IOBox(207, 48, 280, 60);  // Load
    } else {
        InBox(207, 48, 280, 60);
    }

    if (display & ENABLE_SAVE) {
        IOBox(207, 62, 280, 74);  // Regular Save game
    } else {
        InBox(207, 62, 280, 74);
    }

    if (display & ENABLE_PBEM) {
        IOBox(207, 76, 280, 88);  // Mail Save game
    } else {
        InBox(207, 76, 280, 88);
    }

    if (display & ENABLE_DELETE) {
        IOBox(207, 90, 280, 102);  // Delete
    } else {
        InBox(207, 90, 280, 102);
    }

    if (display & ENABLE_PLAY) {
        IOBox(207, 104, 280, 116);  // Play
    } else {
        InBox(207, 104, 280, 116);
    }

    if (display & ENABLE_QUIT) {
        IOBox(207, 118, 280, 130);  // Quit
    } else {
        InBox(207, 118, 280, 130);
    }

    display::graphics.setForegroundColor(11);
    draw_string(59, 42, "TIME CAPSULE REQUEST");
    draw_string(219, 42, "FUNCTIONS");
    display::graphics.setForegroundColor(1);
    draw_string_highlighted(233, 56, "LOAD", 0);
    display::graphics.setForegroundColor(1);
    draw_string_highlighted(233, 70, "SAVE", 0);
    display::graphics.setForegroundColor(1);
    draw_string_highlighted(221, 84, "MAIL SAVE", 0);
    display::graphics.setForegroundColor(1);
    draw_string_highlighted(227, 98, "DELETE", 0);
    display::graphics.setForegroundColor(1);
    draw_string_highlighted(233, 112, "PLAY", 0);
    display::graphics.setForegroundColor(1);
    draw_string_highlighted(234, 126, "QUIT", 0);
}


/**
 * Extract the Save file type from its header.
 *
 * This function doesn't guarantee correctness for malformed headers.
 *
 * \param header  the header of a save game file.
 * \return  if standard, PBEM, or network game.
 */
SaveGameType GetSaveType(const SaveFileHdr &header)
{
    if (header.Country[0] == 8 || header.Country[1] == 9) {
        return SAVEGAME_PlayByMail;
    } else if (header.Country[0] == 6 || header.Country[1] == 7) {
        return SAVEGAME_Modem;
    } else {
        return SAVEGAME_Normal;
    }
}


/* Creates an Autosave with the supplied save name.
 *
 * The save is composed of:
 *
 *   - the save file header (a SaveFileHdr struct)
 *   - the (compressed) struct Players variable Data holding most of
 *     the state data
 *   - the Replay information detailing the events of launches
 *   - the event data, consisting of each turn's news text
 *
 * \param name  The filename to write the save under.
 */
void save_game(char *name)
{
    FILE *outf;
    SaveFileHdr hdr;

    EndOfTurnSave((char *) Data, sizeof(struct Players));

    if (interimData.endTurnBuffer == NULL) {
        WARNING1("Don't have End of Turn Save Data!");
        return;
    }

    memset(&hdr, 0, sizeof hdr);

    hdr.ID = RaceIntoSpace_Signature;
    strcpy(hdr.Name, "AUTOSAVE");
    hdr.Name[sizeof hdr.Name - 1] = 0x1a;

    strcpy(hdr.PName[0], Data->P[plr[0] % 2].Name);
    strcpy(hdr.PName[1], Data->P[plr[1] % 2].Name);
    hdr.Country[0] = Data->plr[0];
    hdr.Country[1] = Data->plr[1];
    hdr.Season = Data->Season;
    hdr.Year = Data->Year;
    hdr.dataSize = sizeof(struct Players);
    hdr.compSize = interimData.endTurnSaveSize;

    if ((outf = sOpen(name, "wb", 1)) == NULL) {
        WARNING2("can't save to file `%s'", name);
        return;
    }

    // Write Save Game Header
    fwrite(&hdr, sizeof hdr, 1, outf);

    // Write End of Turn Save Data
    fwrite(interimData.endTurnBuffer, interimData.endTurnSaveSize, 1, outf);

    // Copy Replay data into Save file
    interimData.replaySize = sizeof(REPLAY) * MAX_REPLAY_ITEMS;
    fwrite(interimData.tempReplay, interimData.replaySize, 1, outf);

    // Copy Event data into Save file
    fwrite(interimData.eventBuffer, interimData.eventSize, 1, outf);

    fclose(outf);
}


/* Creates and displays an entry form for submitting a savegame name
 * and accepts the game description as input.
 *
 * \param name  The string location where the name is stored.
 * \return  1 if the name is submitted, 0 if aborted.
 */
char GetBlockName(char *Nam)
{
    int i, key;

    display::LegacySurface local(164, 77);
    local.copyFrom(display::graphics.legacyScreen(), 39, 50, 202, 126);
    ShBox(39, 50, 202, 126);
    i = 1;

    if (i == 1) {
        InBox(43, 67, 197, 77);
        fill_rectangle(44, 68, 196, 76, 13);
        display::graphics.setForegroundColor(11);
        draw_string(47, 74, "ENTER FILE DESCRIPTION");
        InBox(51, 95, 190, 105);
        fill_rectangle(52, 96, 189, 104, 0);
    } else {
        InBox(43, 67, 197, 77);
        fill_rectangle(44, 68, 196, 76, 13);
        display::graphics.setForegroundColor(11);
        draw_string(47, 74, "NOT ENOUGH DISK SPACE");
        delay(2000);
        local.copyTo(display::graphics.legacyScreen(), 39, 50);
        return 0;
    }

    gr_sync();
    key = 0;
    i = 0;

    while (!(key == K_ENTER || key == K_ESCAPE)) {
        av_block();
        gr_sync();
        key = bioskey(0);

        if (key >= 'a' && key <= 'z') {
            key = toupper(key);
        }

        if (key & 0x00ff) {
            if ((i < 21) && ((key == ' ') || ((key >= 'A' && key <= 'Z')) ||
                             (key >= '0' && key <= '9'))) { // valid key
                Nam[i++] = key;
                display::graphics.setForegroundColor(1);
                draw_string(53, 102, &Nam[0]);
                key = 0;
            }

            if (i > 0 && key == 0x08) {
                Nam[--i] = 0x00;

                fill_rectangle(52, 96, 189, 104, 0);
                display::graphics.setForegroundColor(1);
                draw_string(53, 102, &Nam[0]);

                key = 0;
            }
        }
    }

    local.copyTo(display::graphics.legacyScreen(), 39, 50);

    if (key == K_ENTER && i >= 1) {
        return 1;
    } else {
        return 0;
    }
}


/* Displays an alert popup warning that a save file is corrupted.
 */
void BadFileType()
{
    display::LegacySurface local(164, 77);
    local.copyFrom(display::graphics.legacyScreen(), 39, 50, 202, 126);
    ShBox(39, 50, 202, 126);
    InBox(43, 67, 197, 77);
    fill_rectangle(44, 68, 196, 76, 13);
    display::graphics.setForegroundColor(11);
    draw_string(47, 74, "CORRUPT SAVE FILE");
    delay(2000);
    local.copyTo(display::graphics.legacyScreen(), 39, 50);
    PauseMouse();
    local.copyTo(display::graphics.legacyScreen(), 39, 50);
}


/* Displays a brief text summary of the savegame contents.
 *
 * The summary includes the names of the two space program directors,
 * the type of savegame, and the turn (as year & season).
 *
 * \param name  A savegame filename.
 */
void FileText(char *name)
{
    FILE *fin;
    SaveFileHdr header;

    fill_rectangle(38, 133, 279, 155, 3);
    display::graphics.setForegroundColor(1);
    fin = sOpen(name, "rb", 1);

    if (fin == NULL) {
        display::graphics.setForegroundColor(11);
        draw_string(70, 147, "NO HISTORY RECORDED");
        return;
    }

    fread(&header, sizeof(header), 1, fin);

    fclose(fin);

    grMoveTo(40, 139);

    //if (((char)SaveHdr->Country[0])&0x02) display::graphics.setForegroundColor(7+(SaveHdr->Country[1]-2)*3);

    display::graphics.setForegroundColor(5);

    //display::graphics.setForegroundColor(6+(SaveHdr->Country[0]%2)*3);
    if (header.Country[0] == 6 || header.Country[1] == 7) {
        draw_string(0, 0, "MODEM DIRECTOR ");
    } else if (header.Country[0] == 8 || header.Country[1] == 9) {
        draw_string(0, 0, "PBEM DIRECTOR ");
    } else if (header.Country[0] == 2) {
        draw_string(0, 0, "COMPUTER DIRECTOR ");
    } else {
        draw_string(0, 0, "HUMAN DIRECTOR ");
    }

    draw_string(0, 0, &header.PName[0][0]);
    draw_string(0, 0, " OF THE U.S.A.");

    grMoveTo(40, 147);
    //if (((char)SaveHdr->Country[1])&0x02) display::graphics.setForegroundColor(7+(SaveHdr->Country[1]-2)*3);

    display::graphics.setForegroundColor(9);

    if (header.Country[0] == 6 || header.Country[1] == 7) {
        draw_string(0, 0, "VS. MODEM DIRECTOR ");
    } else if (header.Country[0] == 8 || header.Country[1] == 9) {
        draw_string(0, 0, "VS. PBEM DIRECTOR ");
    } else if (header.Country[1] == 3) {
        draw_string(0, 0, "VS. COMPUTER DIRECTOR ");
    } else {
        draw_string(0, 0, "VS. HUMAN DIRECTOR ");
    }

    draw_string(0, 0, &header.PName[1][0]);
    draw_string(0, 0, " OF THE U.S.S.R.");

    grMoveTo(40, 154);
    display::graphics.setForegroundColor(11);

    if (header.Season == 0) {
        if (header.Country[0] == 8) {
            draw_string(0, 0, "U.S.A. TURN IN THE SPRING OF ");
        } else if (header.Country[1] == 9) {
            draw_string(0, 0, "SOVIET TURN IN THE SPRING OF ");
        } else {
            draw_string(0, 0, "THE SPRING OF ");
        }
    } else {
        if (header.Country[0] == 8) {
            draw_string(0, 0, "U.S.A. TURN IN THE FALL OF ");
        } else if (header.Country[1] == 9) {
            draw_string(0, 0, "SOVIET TURN IN THE FALL OF ");
        } else {
            draw_string(0, 0, "THE FALL OF ");
        }
    }

    draw_number(0, 0, 19);
    draw_number(0, 0, header.Year);
    draw_string(0, 0, ".");
}


/* Displays the Launch Pad menu.
 *
 * The menu is used for accessing planned missions, either future
 * missions set for next season or missions scheduled for the current
 * season. The former are referenced as "Future" missions, while the
 * latter is used as an entrance point for the Vehicle Assembly
 * building.
 *
 * \param plr   The index of the active player (Player 0 or Player 1).
 * \param type  0 for Future Missions, 1 for Vehicle Assembly.
 * \return      0, 1, or 2 for launch pads A, B, and C; 5 to exit.
 */
int FutureCheck(char plr, char type)
{
    int i;
    int pad;
    int p[3];
    int m[3];
    int t = 0;
    int tx[3] = {0, 0, 0};

    for (i = 0; i < 3; i++) {
        p[i] = Data->P[plr].LaunchFacility[i];

        if (type == 0) {
            m[i] = Data->P[plr].Future[i].MissionCode;
        } else if (type == 1) {
            m[i] = (Data->P[plr].Mission[i].Hard[4] > 0) ? 1 : 0;
        } else {
            // only types 0 and 1 are valid
            assert(false);
        }
    }

    FadeOut(2, 10, 0, 0);


    PortPal(plr);

    boost::shared_ptr<display::PalettizedSurface> launchPads(Filesystem::readImage("images/lpads.but.1.png"));

    if (type == 0) {
        helpText = "i010";
        keyHelpText = "k010";
    } else {
        helpText = "i015";
        keyHelpText = "k015";
    }

    display::graphics.screen()->clear();
    ShBox(59, 12, 269, 186);
    InBox(64, 17, 213, 29);
    fill_rectangle(65, 18, 212, 28, 7);
    ShBox(217, 17, 264, 29);
    IOBox(217, 17, 264, 29);
    display::graphics.setForegroundColor(11);
    draw_string(231, 25, "E");
    display::graphics.setForegroundColor(9);
    draw_string(235, 25, "XIT");
    display::graphics.setForegroundColor(11);

    if (type == 0) {
        draw_string(74, 25, "FUTURE ");
    } else if (plr == 0) {
        draw_string(83, 25, "VAB ");
    } else {
        draw_string(83, 25, "VIB ");
    }

    draw_string(0, 0, "LAUNCH SELECTION");

    for (i = 0; i < 3; i++) {
        InBox(64, 35 + 51 * i, 104, 66 + 51 * i);
        InBox(64, 69 + 51 * i, 104, 79 + 51 * i);
        InBox(108, 35 + 51 * i, 264, 64 + 51 * i);
        IOBox(108, 67 + 51 * i, 264, 79 + 51 * i);

        if (p[i] > 1) {
            display::graphics.setForegroundColor(5);
            draw_string(111, 50 + i * 51, "THIS FACILITY IS ");

            if (p[i] == 20) {
                draw_string(0, 0, "DESTROYED.");
            } else {
                draw_string(0, 0, "DAMAGED.");
            }

            draw_string(111, 57 + i * 51, "IT WILL COST ");
            draw_number(0, 0, abs(p[i]));
            draw_string(0, 0, "MB TO REPAIR.");

            if (type == 0) {
                draw_string(113, 75 + i * 51, "REPAIR LAUNCH FACILITY");
            } else {
                InBox(110, 69 + i * 51, 262, 77 + i * 51);
            }

            t = 2;
        }

        if (p[i] == 1) {
            display::graphics.setForegroundColor(1);

            if (type == 1) {
                GetMisType(Data->P[plr].Mission[i].MissionCode);
                draw_string(111, 41 + i * 51, Mis.Abbr);
                int MisCod = Data->P[plr].Mission[i].MissionCode;

                // Show duration level only on missions with a
                // Duration step - Leon
                if (IsDuration(MisCod)) {
                    int duration = Data->P[plr].Mission[i].Duration;
                    draw_string(0, 0, GetDurationParens(duration));
                }

                if (i < 2) {
                    if (Data->P[plr].Mission[i + 1].part == 1) {
                        draw_string(111, 61 + i * 51, "PRIMARY MISSION PART");
                        draw_string(111, 61 + (i + 1) * 51, "SECONDARY MISSION PART");
                    }
                }
            } else {
                if (!Data->P[plr].Future[i].MissionCode) {
                    display::graphics.setForegroundColor(1);
                }

                GetMisType(Data->P[plr].Future[i].MissionCode);

                draw_string(111, 41 + i * 51, Mis.Abbr);
                int MisCod = Data->P[plr].Future[i].MissionCode;

                // Show duration level only on missions with a Duration step - Leon
                if (IsDuration(MisCod)) {
                    int duration = Data->P[plr].Future[i].Duration;
                    draw_string(0, 0, GetDurationParens(duration));
                }

                if (i < 2) {
                    if (Data->P[plr].Future[i + 1].part == 1) {
                        draw_string(111, 61 + i * 51, "PRIMARY MISSION PART");
                        draw_string(111, 61 + (i + 1) * 51, "SECONDARY MISSION PART");
                    }
                }
            }

            if (type == 0) {
                if (Data->P[plr].Future[i].part == 0) {
                    if (m[i] == 0) {
                        draw_string(113, 75 + i * 51, "ASSIGN FUTURE MISSION");
                        t = 1;
                    } else {
                        draw_string(113, 75 + i * 51, "REASSIGN FUTURE MISSION");
                        t = 0;
                    }
                } else {
                    draw_string(113, 75 + i * 51, "SECOND PART OF JOINT MISSION");
                    t = 0;
                    tx[i] = 1;
                    InBox(110, 69 + i * 51, 262, 77 + i * 51);
                }
            } else {
                if (m[i] == 0) {
                    draw_string(113, 75 + i * 51, "ASSEMBLE HARDWARE");
                    t = 1;
                } else {
                    draw_string(113, 75 + i * 51, "REASSEMBLE HARDWARE");
                    t = 0;
                }
            }
        }

        if (p[i] == -1) {
            display::graphics.setForegroundColor(9);  // Changed from 5
            draw_string(111, 41 + i * 51, "NO FACILITY EXISTS");

            if (type == 0) {
                draw_string(111, 49 + i * 51, "PURCHASE LAUNCH FACILITY");
                draw_string(111, 57 + i * 51, "FOR: 20 MB'S");
                draw_string(111, 75 + i * 51, "PURCHASE FACILITY");
            } else {
                InBox(110, 69 + i * 51, 262, 77 + i * 51);
            }

            t = 3;
        }

        display::graphics.setForegroundColor(11);

        draw_string(72, 76 + i * 51, "PAD ");

        draw_character(0x41 + i);

        display::graphics.screen()->draw(launchPads, 156 * plr + t * 39, i * 30, 39, 30, 65, 36 + i * 51);
    }

    FadeIn(2, 10, 0, 0);

    WaitForMouseUp();
    pad = -1;
    x = y = mousebuttons = key = 0;

    while (pad == -1) {
        key = 0;
        GetMouse();

        if (x >= 219 && y >= 19 && x <= 262 && y <= 27 && mousebuttons > 0) {
            InBox(219, 19, 262, 27);
            WaitForMouseUp();
            pad = 5;
            key = 0;
        }

        if (((!(x >= 59 && y >= 12 && x <= 269 && y <= 186)) && mousebuttons > 0) || key == K_ESCAPE || key == K_ENTER || key == 'E') {
            InBox(219, 19, 262, 27);
            pad = 5;
            key = 0;
        }

        for (i = 0; i < 3; i++) {
            if ((x >= 110 && y >= 69 + i * 51 && x <= 262 && y <= 77 + i * 51 && tx[i] != 1 && mousebuttons > 0) || (tx[i] != 1 && key == 'A' + i)) {
                InBox(110, 69 + i * 51, 262, 77 + i * 51);
                WaitForMouseUp();
                key = 0;
                delay(300);

                if (p[i] == 1) {
                    pad = i;
                }

                if (p[i] == -1 && Data->P[plr].Cash >= 20 && type == 0) {

                    display::graphics.screen()->draw(launchPads, 156 * plr + 39, i * 30, 39, 30, 65, 36 + i * 51);
                    Data->P[plr].Cash -= 20;
                    Data->P[plr].Spend[0][3] += 20;
                    Data->P[plr].LaunchFacility[i] = 1;
                    p[i] = 1;
                    ShBox(110, 69 + i * 51, 262, 77 + i * 51);
                    fill_rectangle(109, 36 + 51 * i, 263, 63 + 51 * i, 3);
                    display::graphics.setForegroundColor(5);
                    // Missions(plr, 111, 41 + i * 51, m[i], 0);
                    MissionName(m[i], 111, 41 + i * 51, 24);
                    draw_string(113, 75 + i * 51, "ASSIGN FUTURE MISSION");

                } else if (p[i] == -1 && Data->P[plr].Cash < 20 && type == 0) {
                    Help("i129");
                }


                if (p[i] > 4 && Data->P[plr].Cash >= abs(Data->P[plr].LaunchFacility[i])
                    && type == 0) {

                    display::graphics.screen()->draw(launchPads, 156 * plr + 39, i * 30, 39, 30, 65, 36 + i * 51);
                    Data->P[plr].Cash -= Data->P[plr].LaunchFacility[i];
                    Data->P[plr].Spend[0][3] += Data->P[plr].LaunchFacility[i];
                    Data->P[plr].LaunchFacility[i] = 1;
                    p[i] = 1;
                    fill_rectangle(109, 36 + 51 * i, 263, 63 + 51 * i, 3);
                    ShBox(110, 69 + i * 51, 262, 77 + i * 51);
                    display::graphics.setForegroundColor(5);
                    // Missions(plr, 111, 41 + i * 51, m[i], 0);
                    MissionName(m[i], 111, 41 + i * 51, 24);
                    draw_string(113, 75 + i * 51, "ASSIGN FUTURE MISSION");

                } else if (p[i] > 4 && Data->P[plr].Cash < abs(Data->P[plr].LaunchFacility[i])
                           && type == 0) {
                    Help("i129");
                }
            }
        }
    }

    return pad;
}


/**
 * Reads a save file and populates the active game data.
 *
 * Fills the Data and interimData global variables with data
 * extracted from the save file. Automatically corrects for endian
 * differences between the system and saved information.
 *
 * The Play-by-Modem mode is disabled in the game, and the code is a
 * bit of a mess. It's preserved here faithfully, but that doesn't
 * guarantee it will _work_.
 * (Note: Modem never worked in Windows, even in DOSBox running on
 *        Windows 98SE. It worked only in a native DOS environment,
 *        and then of course wouldn't have worked with a winmodem.
 *         -Leon)
 *
 *   header.compSize   is the size (in bytes) of the Data global
 *                     when compressed.
 *   header.dataSize   is the size of the Data global when expanded
 *
 * TODO: The new values for the global variables are assigned as they
 * are read, which reduces memory requirements but means the
 * current game state may be overwritten before an error in the file
 * is noticed. This would prevent loading, but make it impossible to
 * return to the active game.
 *
 * TODO: Add error handling on read/write commands.
 *
 * \param filename  the name, including extension, relative to the
 *     save directory.
 */
void LoadGame(const char *filename)
{
    REPLAY *load_buffer = NULL;
    SaveFileHdr header;

    FILE *fin = sOpen(filename, "rb", 1);

    fseek(fin, 0, SEEK_END);
    size_t fileLength = ftell(fin);
    rewind(fin);
    fread(&header, 1, sizeof(header), fin);

    // Determine Endian Swap, 31663 is for old save games
    bool endianSwap = (header.dataSize != sizeof(struct Players) &&
                       header.dataSize != 31663);

    if (endianSwap) {
        header.compSize = _Swap16bit(header.compSize);
        header.dataSize = _Swap16bit(header.dataSize);

        if (header.dataSize != sizeof(struct Players) &&
            header.dataSize != 31663) {
            // TODO: Feels like BadFileType() should be launched by
            // FileAccess, which runs the interface. Throw an
            // exception or return an error code?
            fclose(fin);
            BadFileType();
            return;
        }
    }

    size_t readLen = header.compSize;
    load_buffer = (REPLAY *)malloc(readLen);
    fread(load_buffer, 1, readLen, fin);
    RLED((char *) load_buffer, (char *)Data, header.compSize);
    free(load_buffer);

    // Swap Players' Data
    if (endianSwap) {
        _SwapGameDat();
    }

    //MSF now holds MaxRDBase (from 1.0.0)
    if (Data->P[0].Probe[PROBE_HW_ORBITAL].MSF == 0) {
        for (int j = 0; j < NUM_PLAYERS; j++) {
            for (int k = 0; k < 7; k++) {
                Data->P[j].Probe[k].MSF = Data->P[j].Probe[k].MaxRD;
                Data->P[j].Rocket[k].MSF = Data->P[j].Rocket[k].MaxRD;
                Data->P[j].Manned[k].MSF = Data->P[j].Manned[k].MaxRD;
                Data->P[j].Misc[k].MSF = Data->P[j].Misc[k].MaxRD;
            }
        }
    }

    // Read the Replay Data
    load_buffer = (REPLAY *)malloc((sizeof(REPLAY)) * MAX_REPLAY_ITEMS);
    fread(load_buffer, 1, sizeof(REPLAY) * MAX_REPLAY_ITEMS, fin);

    if (endianSwap) {
        REPLAY *r = NULL;
        r = load_buffer;

        for (int j = 0; j < MAX_REPLAY_ITEMS; j++) {
            for (int k = 0; k < r->Qty; k++) {
                r[j].Off[k] = _Swap16bit(r[j].Off[k]);
            }
        }
    }

    interimData.replaySize = sizeof(REPLAY) * MAX_REPLAY_ITEMS;
    memcpy(interimData.tempReplay, load_buffer, interimData.replaySize);
    free(load_buffer);

    size_t eventSize = fileLength - ftell(fin);

    // Read the Event Data
    load_buffer = (REPLAY *)malloc(eventSize);
    fread(load_buffer, 1, eventSize, fin);
    fclose(fin);

    if (endianSwap) {
        // File Structure is 84 longs 42 per side
        for (int j = 0; j < 84; j++) {
            OLDNEWS *on = (OLDNEWS *) load_buffer + (j * sizeof(OLDNEWS));

            if (on->offset) {
                on->offset = _Swap32bit(on->offset);
                on->size = _Swap16bit(on->size);
            }
        }
    }

    // Save Event information
    if (interimData.eventBuffer) {
        free(interimData.eventBuffer);
    }

    interimData.eventBuffer = (char *) malloc(eventSize);
    interimData.eventSize = eventSize;
    memcpy(interimData.eventBuffer, load_buffer, eventSize);
    interimData.tempEvents = (OLDNEWS *) interimData.eventBuffer;

    free(load_buffer);

    if (GetSaveType(header) == SAVEGAME_Normal) {
        Option = MAIL = -1;

        Data->plr[0] = plr[0] = Data->Def.Plr1;
        Data->plr[1] = plr[1] = Data->Def.Plr2;

        AI[0] = (plr[0] == 2 || plr[0] == 3);
        AI[1] = (plr[1] == 2 || plr[1] == 3);

        CacheCrewFile();
        LOAD = 1;
    } else if (GetSaveType(header) == SAVEGAME_PlayByMail) {
        Option = -1;
        MAIL = !(header.Country[0] == 8);

        Data->plr[0] = Data->Def.Plr1 = plr[0] = 0;
        Data->plr[1] = Data->Def.Plr2 = plr[1] = 1;

        AI[0] = AI[1] = 0;
        Data->Season = header.Season;
        Data->Year = header.Year;

        CacheCrewFile();
        LOAD = 1;
    } else if (GetSaveType(header) == SAVEGAME_Modem) {
        // Modem connect up
        if (header.Country[0] == 6) {
            plr[0] = header.Country[0];
            plr[1] = 1;
        } else {
            plr[1] = header.Country[1];
            plr[0] = 0;
        }

        // Modem Play => reset the modem
        if (Option != -1) {
            DoModem(2);
        }

        Option = MPrefs(1);
        LOAD = (Option != MODEM_ERROR);

        // kludge
        if (Option == 0 || Option == 2) {
            Option = 0;
        } else if (Option == 1 || Option == 3) {
            Option = 1;
        } else if (Option == MODEM_ERROR) {
            Option = -1;
        }

        // TODO: Should Modem games call CacheCrewFile()?
    }
}


/**
 * Sort SFInfo objects by Title then Name.
 */
bool OrderSaves(const SFInfo &a, const SFInfo &b)
{
    int titleOrder = xstrcasecmp(a.Title, b.Title);
    return (titleOrder < 0) ||
           (titleOrder == 0 && xstrcasecmp(a.Name, b.Name) < 0);
}


/* Creates a popup confirmation box, blocking input until resolved.
 *
 * \param s   The heading text.
 * \param md  1 if the background underneath should be redrawn on close.
 * \return  1 for yes, 0 for no.
 */
char RequestX(char *s, char md)
{
    char i;
    display::LegacySurface local(196, 84);


    if (md == 1) { // Save Buffer
        local.copyFrom(display::graphics.legacyScreen(), 85, 52, 280, 135);
    }

    i = strlen(s) >> 1;
    display::graphics.setForegroundColor(0);
    ShBox(85, 52, 249, 135);
    IOBox(170, 103, 243, 130);
    IOBox(91, 103, 164, 130);
    InBox(92, 58, 243, 97);
    display::graphics.setForegroundColor(1);
    draw_heading(111, 110, "YES", 0, 0);
    draw_heading(193, 110, "NO", 0, 0);
    display::graphics.setForegroundColor(11);
    draw_heading(166 - i * 10, 65, &s[0], 0, -1);
    draw_string(136, 94, "ARE YOU SURE?");

    WaitForMouseUp();
    i = 2;

    while (i == 2) {
        GetMouse();

        if ((x >= 172 && y >= 105 && x <= 241 && y <= 128 && mousebuttons != 0) || (key == 'N')) {
            InBox(172, 105, 241, 128);
            i = 0;
            delay(50);
            key = 0;
        }

        if ((x > 93 && y >= 105 && x <= 162 && y <= 128 && mousebuttons != 0) || (key == 'Y')) {
            InBox(93, 105, 162, 128);
            i = 1;
            delay(50);
            key = 0;
        }
    }

    if (md == 1) {

        WaitForMouseUp();
        local.copyTo(display::graphics.legacyScreen(), 85, 52);
    }

    WaitForMouseUp();
    return i;
}


/* Updates the game state stored in the save buffer.
 *
 * The game tracks all the data that will be written to a save file
 * in a buffer, interimData, modifying it when the save should update
 * to reflect changes. Saving the game writes this buffer to a new
 * savegame file. This compresses byte-formatted data and stores it
 * in the interimData end-of-turn buffer which contains the game
 * state information (planned missions, research levels, etc.).
 *
 * When a save file is created, it starts with a SaveFileHdr header
 * and is populated with the data stored in the interimData structure.
 * The interimData variable holds:
 *   - the (compressed) struct Players variable Data holding most of
 *     the state data
 *   - the Replay information detailing the events of launches
 *   - the event data, consisting of each turn's event cards and the
 *     news text generated
 *
 * In normal usage, the variable Data (which contains the game state)
 * is copied into the interimData end-of-turn buffer at the beginning of
 * each player's turn. In this manner, loading the save returns the
 * player to the beginning of their turn (on which the save was
 * created). The interimData buffer is also updated when the player
 * quits the game, updating the Autosave to their current state so the
 * player may resume their game where they left off.
 *
 * \param inData   Data formatted as a raw byte block.
 * \param dataLen  the size of the data block in bytes.
 * \return  the size of the compressed data.
 */
int32_t EndOfTurnSave(char *inData, int dataLen)
{
    if (!inData || dataLen == 0) {
        return 0;
    }

    char *buffer = (char *)xmalloc(dataLen * 2);
    int compressedLen = RLEC(inData, buffer, dataLen);

    if (interimData.endTurnBuffer) {
        free(interimData.endTurnBuffer);
    }

    interimData.endTurnSaveSize = compressedLen;
    interimData.endTurnBuffer = (char *)malloc(interimData.endTurnSaveSize);
    memcpy(interimData.endTurnBuffer, buffer, interimData.endTurnSaveSize);
    free(buffer);

    return interimData.endTurnSaveSize;
}

void MailSave()
{
    int done, temp, i;
    FILE *fin;
    SaveFileHdr header;

    std::vector<SFInfo> savegames = GenerateTables(SAVEGAME_PlayByMail);

    display::graphics.screen()->clear();

    FadeIn(2, 10, 0, 0);

    WaitForMouseUp();
    memset(header.Name, 0x00, sizeof(header.Name));

    do {
        done = GetBlockName(header.Name); // Checks Free Space
        header.ID = RaceIntoSpace_Signature;
        header.Name[sizeof(header.Name) - 1] = 0x1A;
        temp = NOTSAME;

        for (i = 0; (i < savegames.size() && temp == 2); i++) {
            if (strcmp(header.Name, savegames[i].Title) == 0) {
                temp = RequestX("REPLACE FILE", 1);

                if (temp == SAME_ABORT) {
                    done = 0;
                }
            }
        }
    } while (done == 0);

    if (done == YES) {
        i--;  // decrement to correct for the FOR loop
        strcpy(header.PName[0], Data->P[plr[0] % 2].Name);
        strcpy(header.PName[1], Data->P[plr[1] % 2].Name);

        // Play-By-Mail save game hack
        //
        // If MAIL == 0, we are playing as the U.S. We need to
        // save the game such that the U.S. starts again
        if (MAIL == 0) {
            plr[0] = 8;
            plr[1] = 0;
        }
        // Playing as the Soviets
        else if ((MAIL == 1)) {
            plr[0] = 0;
            plr[1] = 9;
        }

        Data->Def.Plr1 = plr[0];
        Data->Def.Plr2 = plr[1];
        Data->plr[0] = Data->Def.Plr1;
        Data->plr[1] = Data->Def.Plr2;
        AI[0] = 0;
        AI[1] = 0;

        header.Country[0] = Data->plr[0];
        header.Country[1] = Data->plr[1];
        header.Season = Data->Season;
        header.Year = Data->Year;
        header.dataSize = sizeof(struct Players);

        EndOfTurnSave((char *) Data, sizeof(struct Players));
        header.compSize = interimData.endTurnSaveSize;

        if (temp == NOTSAME) {
            i = 0;
            fin = NULL;

            do {
                i++;

                if (fin) {
                    fclose(fin);
                }

                snprintf(Name, sizeof(Name), "BUZZ%d.SAV", i);
                fin = sOpen(Name, "rb", 1);
            } while (fin != NULL); // Find unique name

            fin = sOpen(Name, "wb", 1);
        } else {
            fin = sOpen(savegames[i].Name, "wb", 1);
        }

        // Write the Save Game Header
        fwrite(&header, sizeof(header), 1, fin);

        // Save End of Turn Data
        fwrite(interimData.endTurnBuffer, interimData.endTurnSaveSize, 1, fin);

        // Save Replay Data
        interimData.replaySize = sizeof(REPLAY) * MAX_REPLAY_ITEMS;
        fwrite(interimData.tempReplay, interimData.replaySize, 1, fin);

        // Save Event Data
        fwrite(interimData.eventBuffer, interimData.eventSize, 1, fin);

        fclose(fin);
    }
}


// EOF
