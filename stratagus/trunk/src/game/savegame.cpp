//   ___________		     _________		      _____  __
//   \_	  _____/______   ____   ____ \_   ___ \____________ _/ ____\/  |_
//    |    __) \_  __ \_/ __ \_/ __ \/    \  \/\_  __ \__  \\   __\\   __\ 
//    |     \   |  | \/\  ___/\  ___/\     \____|  | \// __ \|  |   |  |
//    \___  /   |__|    \___  >\___  >\______  /|__|  (____  /__|   |__|
//	  \/		    \/	   \/	     \/		   \/
//  ______________________                           ______________________
//			  T H E   W A R   B E G I N S
//	   FreeCraft - A free fantasy real time strategy game engine
//
/**@name savegame.c	-	Save game. */
//
//	(c) Copyright 2001,2002 by Lutz Sammer, Andreas Arens
//
//	FreeCraft is free software; you can redistribute it and/or modify
//	it under the terms of the GNU General Public License as published
//	by the Free Software Foundation; only version 2 of the License.
//
//	FreeCraft is distributed in the hope that it will be useful,
//	but WITHOUT ANY WARRANTY; without even the implied warranty of
//	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//	GNU General Public License for more details.
//
//	$Id$

//@{

/*----------------------------------------------------------------------------
--	Includes
----------------------------------------------------------------------------*/

#include <stdio.h>
#if !defined(_MSC_VER) || !defined(_WIN32_WCE)
#include <time.h>
#endif

#include "freecraft.h"

#include "icons.h"
#include "ui.h"
#include "construct.h"
#include "unittype.h"
#include "unit.h"
#include "upgrade.h"
#include "depend.h"
#include "interface.h"
#include "missile.h"
#include "tileset.h"
#include "map.h"
#include "player.h"
#include "ai.h"
#include "campaign.h"
#include "trigger.h"

#include "ccl.h"

/*----------------------------------------------------------------------------
--	Variables
----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------
--	Functions
----------------------------------------------------------------------------*/

/**
**	Save a game to file.
**
**	@param filename	File name to be stored.
**
**	@note	Later we want to store in a more compact binary format.
*/
global void SaveGame(const char* filename)
{
    time_t now;
    FILE* file;
    char* s;
    char* s1;

    file=fopen(filename,"wb");
    if( !file ) {
	fprintf(stderr,"Can't save to `%s'\n",filename);
	return;
    }

    time(&now);
    s=ctime(&now);
    if( (s1=strchr(s,'\n')) ) {
	*s1='\0';
    }

    //
    //	Parseable header
    //
    fprintf(file,";;;(save-game\n");
    fprintf(file,";;;  'comment\t\"Generated by FreeCraft Version " VERSION "\"\n");
    fprintf(file,";;;  'comment\t\"Visit http://FreeCraft.Org for more informations\"\n");
    fprintf(file,";;;  'comment\t\"$Id$\"\n");
    fprintf(file,";;;  'type\t\"%s\"\n","single-player");
    fprintf(file,";;;  'date\t\"%s\"\n",s);
    fprintf(file,";;;  'map\t\"%s\"\n",TheMap.Description);
    fprintf(file,";;;  'engine\t'(%d %d %d)\n",
	FreeCraftMajorVersion,FreeCraftMinorVersion,FreeCraftPatchLevel);
    fprintf(file,";;;  'savefile\t'(%d %d %d)\n",
	FreeCraftMajorVersion,FreeCraftMinorVersion,FreeCraftPatchLevel);
    fprintf(file,";;;  'preview\t\"%s.pam\"\n",filename);
    fprintf(file,";;;  )\n");

    // FIXME: probably not the right place for this
    fprintf(file,"(set-game-cycle! %lu)\n",GameCycle);

    SaveCcl(file);
    SaveIcons(file);
    SaveCursors(file);
    SaveUserInterface(file);
    SaveTilesets(file);
    SaveConstructions(file);
    SaveDecorations(file);
    SaveUnitTypes(file);
    SaveUpgrades(file);
    SaveDependencies(file);
    SaveButtons(file);
    SaveMissileTypes(file);
    SavePlayers(file);
    SaveMap(file);
    SaveUnits(file);
    SaveAi(file);
    SaveSelections(file);
    SaveGroups(file);
    SaveMissiles(file);
    SaveTriggers(file);
    SaveCampaign(file);

    // FIXME: find all state information which must be saved.

    fclose(file);
}

//@}
