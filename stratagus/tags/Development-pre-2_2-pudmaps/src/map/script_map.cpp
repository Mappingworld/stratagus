//       _________ __                 __
//      /   _____//  |_____________ _/  |______     ____  __ __  ______
//      \_____  \\   __\_  __ \__  \\   __\__  \   / ___\|  |  \/  ___/
//      /        \|  |  |  | \// __ \|  |  / __ \_/ /_/  >  |  /\___ |
//     /_______  /|__|  |__|  (____  /__| (____  /\___  /|____//____  >
//             \/                  \/          \//_____/            \/
//  ______________________                           ______________________
//                        T H E   W A R   B E G I N S
//         Stratagus - A free fantasy real time strategy game engine
//
/**@name script_map.c - The map ccl functions. */
//
//      (c) Copyright 1999-2004 by Lutz Sammer and Jimmy Salmon
//
//      This program is free software; you can redistribute it and/or modify
//      it under the terms of the GNU General Public License as published by
//      the Free Software Foundation; only version 2 of the License.
//
//      This program is distributed in the hope that it will be useful,
//      but WITHOUT ANY WARRANTY; without even the implied warranty of
//      MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//      GNU General Public License for more details.
//
//      You should have received a copy of the GNU General Public License
//      along with this program; if not, write to the Free Software
//      Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
//      02111-1307, USA.
//
//      $Id$

//@{

/*----------------------------------------------------------------------------
--  Includes
----------------------------------------------------------------------------*/

#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include "stratagus.h"
#include "unit.h"
#include "unittype.h"
#include "script.h"
#include "map.h"
#include "tileset.h"
#include "minimap.h"
#include "actions.h"
#include "campaign.h"
#include "ui.h"
#include "player.h"

/*----------------------------------------------------------------------------
--  Functions
----------------------------------------------------------------------------*/

/**
**  Parse a stratagus map.
**
**  @param l  Lua state.
*/
static int CclStratagusMap(lua_State* l)
{
	const char* value;
	int args;
	int j;
	int subargs;
	int k;

	//
	//  Parse the list: (still everything could be changed!)
	//

	args = lua_gettop(l);
	for (j = 0; j < args; ++j) {
		value = LuaToString(l, j + 1);
		++j;

		if (!strcmp(value, "version")) {
			char buf[32];

			value = LuaToString(l, j + 1);
			sprintf(buf, StratagusFormatString, StratagusFormatArgs(StratagusVersion));
			if (strcmp(buf, value)) {
				fprintf(stderr, "Warning not saved with this version.\n");
			}
		} else if (!strcmp(value, "uid")) {
			TheMap.Info.MapUID = LuaToNumber(l, j + 1);
		} else if (!strcmp(value, "description")) {
			value = LuaToString(l, j + 1);
			TheMap.Info.Description = strdup(value);
		} else if (!strcmp(value, "the-map")) {
			if (!lua_istable(l, j + 1)) {
				LuaError(l, "incorrect argument");
			}
			subargs = luaL_getn(l, j + 1);
			for (k = 0; k < subargs; ++k) {
				lua_rawgeti(l, j + 1, k + 1);
				value = LuaToString(l, -1);
				lua_pop(l, 1);
				++k;

				if (!strcmp(value, "terrain")) {
					int i;

					lua_rawgeti(l, j + 1, k + 1);
					if (!lua_istable(l, -1)) {
						LuaError(l, "incorrect argument");
					}
					lua_rawgeti(l, -1, 1);
					value = LuaToString(l, -1);
					lua_pop(l, 1);
					// ignore (l, -1, 2)
					lua_pop(l, 1);

					free(TheMap.TerrainName);
					TheMap.TerrainName = strdup(value);

					// Lookup the index of this tileset.
					for (i = 0; TilesetWcNames[i] &&
						strcmp(value, TilesetWcNames[i]); ++i) {
					}
					TheMap.Terrain = i;
					LoadTileset();
				} else if (!strcmp(value, "size")) {
					lua_rawgeti(l, j + 1, k + 1);
					if (!lua_istable(l, -1)) {
						LuaError(l, "incorrect argument");
					}
					lua_rawgeti(l, -1, 1);
					TheMap.Info.MapWidth = LuaToNumber(l, -1);
					lua_pop(l, 1);
					lua_rawgeti(l, -1, 2);
					TheMap.Info.MapHeight = LuaToNumber(l, -1);
					lua_pop(l, 1);
					lua_pop(l, 1);

					free(TheMap.Fields);
					TheMap.Fields = calloc(TheMap.Info.MapWidth * TheMap.Info.MapHeight,
						sizeof(*TheMap.Fields));
					TheMap.Visible[0] = calloc(TheMap.Info.MapWidth * TheMap.Info.MapHeight / 8, 1);
					InitUnitCache();
					// FIXME: this should be CreateMap or InitMap?
				} else if (!strcmp(value, "fog-of-war")) {
					TheMap.NoFogOfWar = 0;
					--k;
				} else if (!strcmp(value, "no-fog-of-war")) {
					TheMap.NoFogOfWar = 1;
					--k;
				} else if (!strcmp(value, "filename")) {
					 lua_rawgeti(l, j + 1, k + 1);
					TheMap.Info.Filename = strdup(LuaToString(l, -1));
					lua_pop(l, 1);
				} else if (!strcmp(value, "map-fields")) {
					int i;
					int subsubargs;
					int subk;

					lua_rawgeti(l, j + 1, k + 1);
					if (!lua_istable(l, -1)) {
						LuaError(l, "incorrect argument");
					}

					subsubargs = luaL_getn(l, -1);
					if (subsubargs != TheMap.Info.MapWidth * TheMap.Info.MapHeight) {
						fprintf(stderr, "Wrong tile table length: %d\n", subsubargs);
					}
					i = 0;
					for (subk = 0; subk < subsubargs; ++subk) {
						int args2;
						int j2;

						lua_rawgeti(l, -1, subk + 1);
						if (!lua_istable(l, -1)) {
							LuaError(l, "incorrect argument");
						}
						args2 = luaL_getn(l, -1);
						j2 = 0;

						lua_rawgeti(l, -1, j2 + 1);
						TheMap.Fields[i].Tile = LuaToNumber(l, -1);
						lua_pop(l, 1);
						++j2;
						lua_rawgeti(l, -1, j2 + 1);
						TheMap.Fields[i].SeenTile = LuaToNumber(l, -1);
						lua_pop(l, 1);
						++j2;
						for (; j2 < args2; ++j2) {
							lua_rawgeti(l, -1, j2 + 1);
							if (lua_isnumber(l, -1)) {
								TheMap.Fields[i].Value = LuaToNumber(l, -1);
								lua_pop(l, 1);
								continue;
							}
							value = LuaToString(l, -1);
							lua_pop(l, 1);
							if (!strcmp(value, "explored")) {
								++j2;
								lua_rawgeti(l, -1, j2 + 1);
								TheMap.Fields[i].Visible[(int)LuaToNumber(l, -1)] = 1;
								lua_pop(l, 1);
							} else if (!strcmp(value, "human")) {
								TheMap.Fields[i].Flags |= MapFieldHuman;

							} else if (!strcmp(value, "land")) {
								TheMap.Fields[i].Flags |= MapFieldLandAllowed;
							} else if (!strcmp(value, "coast")) {
								TheMap.Fields[i].Flags |= MapFieldCoastAllowed;
							} else if (!strcmp(value, "water")) {
								TheMap.Fields[i].Flags |= MapFieldWaterAllowed;

							} else if (!strcmp(value, "mud")) {
								TheMap.Fields[i].Flags |= MapFieldNoBuilding;
							} else if (!strcmp(value, "block")) {
								TheMap.Fields[i].Flags |= MapFieldUnpassable;

							} else if (!strcmp(value, "wall")) {
								TheMap.Fields[i].Flags |= MapFieldWall;
							} else if (!strcmp(value, "rock")) {
								TheMap.Fields[i].Flags |= MapFieldRocks;
							} else if (!strcmp(value, "wood")) {
								TheMap.Fields[i].Flags |= MapFieldForest;

							} else if (!strcmp(value, "ground")) {
								TheMap.Fields[i].Flags |= MapFieldLandUnit;
							} else if (!strcmp(value, "air")) {
								TheMap.Fields[i].Flags |= MapFieldAirUnit;
							} else if (!strcmp(value, "sea")) {
								TheMap.Fields[i].Flags |= MapFieldSeaUnit;
							} else if (!strcmp(value, "building")) {
								TheMap.Fields[i].Flags |= MapFieldBuilding;

							} else {
							   LuaError(l, "Unsupported tag: %s" _C_ value);
							}
						}
						lua_pop(l, 1);
						++i;
					}
					lua_pop(l, 1);
				} else {
				   LuaError(l, "Unsupported tag: %s" _C_ value);
				}
			}

		} else {
		   LuaError(l, "Unsupported tag: %s" _C_ value);
		}
	}

	return 0;
}

/**
**  Reveal the complete map.
**
**  @param l  Lua state.
*/
static int CclRevealMap(lua_State* l)
{
	if (lua_gettop(l) != 0) {
		LuaError(l, "incorrect argument");
	}
	if (CclInConfigFile) {
		FlagRevealMap = 1;
	} else {
		RevealMap();
	}

	return 0;
}

/**
**  Center the map.
**
**  @param l  Lua state.
*/
static int CclCenterMap(lua_State* l)
{
	if (lua_gettop(l) != 2) {
		LuaError(l, "incorrect argument");
	}
	ViewportCenterViewpoint(TheUI.SelectedViewport,
		LuaToNumber(l, 1), LuaToNumber(l, 2), TileSizeX / 2, TileSizeY / 2);

	return 0;
}

/**
**  Define the starting viewpoint for a given player.
**
**  @param l  Lua state.
*/
static int CclSetStartView(lua_State* l)
{
	int p;

	if (lua_gettop(l) != 3) {
		LuaError(l, "incorrect arguments");
	}
	p = LuaToNumber(l, 1);
	Players[p].StartX = LuaToNumber(l, 2);
	Players[p].StartY = LuaToNumber(l, 3);

	return 0;
}

/**
**  Show Map Location
**
**  @param l  Lua state.
*/
static int CclShowMapLocation(lua_State* l)
{
	Unit* target;
	const char* unitname;

	// Put a unit on map, use its properties, except for
	// what is listed below

	if (lua_gettop(l) != 4) {
		LuaError(l, "incorrect argument");
	}
	unitname = LuaToString(l, 5);
	target = MakeUnit(UnitTypeByIdent(unitname), ThisPlayer);
	target->Orders[0].Action = UnitActionStill;
	target->HP = 0;
	target->X = LuaToNumber(l, 1);
	target->Y = LuaToNumber(l, 2);
	target->TTL = GameCycle + LuaToNumber(l, 4);
	target->CurrentSightRange = LuaToNumber(l, 3);
	MapMarkUnitSight(target);
	return 0;
}

/**
**  Set the default map.
**
**  @param l  Lua state.
*/
static int CclSetDefaultMap(lua_State* l)
{
	if (lua_gettop(l) != 1) {
		LuaError(l, "incorrect argument");
	}
	strncpy(DefaultMap, LuaToString(l, 1), sizeof(DefaultMap) - 1);
	return 0;
}

/**
**  Set fog of war on/off.
**
**  @param l  Lua state.
*/
static int CclSetFogOfWar(lua_State* l)
{
	if (lua_gettop(l) != 1) {
		LuaError(l, "incorrect argument");
	}
	TheMap.NoFogOfWar = !LuaToBoolean(l, 1);
	if (!CclInConfigFile) {
		UpdateFogOfWarChange();
	}
	return 0;
}

/**
**  Enable display of terrain in minimap.
**
**  @param l  Lua state.
*/
static int CclSetMinimapTerrain(lua_State* l)
{
	if (lua_gettop(l) != 1) {
		LuaError(l, "incorrect argument");
	}
	MinimapWithTerrain = LuaToBoolean(l, 1);
	return 0;
}

/**
**  Fog of war opacity.
**
**  @param l  Lua state.
*/
static int CclSetFogOfWarOpacity(lua_State* l)
{
	int i;

	if (lua_gettop(l) != 1) {
		LuaError(l, "incorrect argument");
	}
	i = LuaToNumber(l, 1);
	if (i < 0 || i > 255) {
		PrintFunction();
		fprintf(stdout, "Opacity should be 0 - 256\n");
		i = 100;
	}
	FogOfWarOpacity = i;

	if (!CclInConfigFile) {
		InitMapFogOfWar();
	}

	return 0;
}

/**
**  Set forest regeneration speed.
**
**  @param l  Lua state.
**
**  @return   Old speed
*/
static int CclSetForestRegeneration(lua_State* l)
{
	int i;
	int old;

	if (lua_gettop(l) != 1) {
		LuaError(l, "incorrect argument");
	}
	i = LuaToNumber(l, 1);
	if (i < 0 || i > 255) {
		PrintFunction();
		fprintf(stdout, "Regeneration speed should be 0 - 255\n");
		i = 100;
	}
	old = ForestRegeneration;
	ForestRegeneration = i;

	if (!CclInConfigFile) {
		InitMapFogOfWar();
	}

	lua_pushnumber(l, old);
	return 1;
}

/**
**  Define Fog graphics
**
**  @param l  Lua state.
*/
static int CclSetFogOfWarGraphics(lua_State* l)
{
	const char* FogGraphicFile;

	if (lua_gettop(l) != 1) {
		LuaError(l, "incorrect argument");
	}

	FogGraphicFile = LuaToString(l, 1);
	if (TheMap.FogGraphic) {
		FreeGraphic(TheMap.FogGraphic);
	}
	TheMap.FogGraphic = NewGraphic(FogGraphicFile, TileSizeX, TileSizeY);

	return 0;
}

/**
**  Select the tileset when loading a map
**
**  @param l  Lua state.
*/
static int CclSelectTileset(lua_State* l)
{
	const char* tileset;
	int i;

	if (lua_gettop(l) != 1) {
		LuaError(l, "incorrect argument");
	}

	tileset = LuaToString(l, 1);

	free(TheMap.TerrainName);
	TheMap.TerrainName = strdup(tileset);

	printf("%s\n", TilesetWcNames[0]);
	// Lookup the index of this tileset.
	for (i = 0; TilesetWcNames[i] &&
		strcmp(tileset, TilesetWcNames[i]); ++i) {
	}
	TheMap.Terrain = i;
	LoadTileset();

	return 0;
}
	
/**
**  Define Fog graphics
**
**  @param l  Lua state.
*/
static int CclSetTile(lua_State* l)
{
	int tile;
	int w;
	int h;
	Tileset *tileset;

	if (lua_gettop(l) != 3) {
		LuaError(l, "incorrect argument");
	}
	
	tile = LuaToNumber(l, 1);
	w = LuaToNumber(l, 2);
	h = LuaToNumber(l, 3);
	tileset = Tilesets[TheMap.Terrain];

	TheMap.Fields[w + h * TheMap.Info.MapWidth].Tile = tileset->Table[tile];
	TheMap.Fields[w + h * TheMap.Info.MapWidth].Value = 0;
	TheMap.Fields[w + h * TheMap.Info.MapWidth].Flags = tileset->FlagsTable[tile];

	return 0;
}

/**
** Define the type of each player available for the map
**
**  @param l  Lua state.
*/
static int CclDefinePlayerTypes(lua_State* l)
{
	const char* type;
	int numplayers;
	int i;

	numplayers = lua_gettop(l); /* Number of players == number of arguments */
	if (numplayers < 2) {
		LuaError(l, "Not enough players");
	}

	for (i = 0; i < numplayers && i < PlayerMax; i++) {
		type = LuaToString(l, i + 1);
		if (!strcmp(type, "neutral")) {
			TheMap.Info.PlayerType[i] = PlayerNeutral;
		} else if (!strcmp(type, "nobody")) {
			TheMap.Info.PlayerType[i] = PlayerNobody;
		} else if (!strcmp(type, "computer")) {
			TheMap.Info.PlayerType[i] = PlayerComputer;
		} else if (!strcmp(type, "person")) {
			TheMap.Info.PlayerType[i] = PlayerPerson;
		} else if (!strcmp(type, "rescue-passive")) {
			TheMap.Info.PlayerType[i] = PlayerRescuePassive;
		} else if (!strcmp(type, "rescue-active")) {
			TheMap.Info.PlayerType[i] = PlayerRescueActive;
		} else {
			LuaError(l, "Unsupported tag: %s" _C_ type);
		}
	}
	for (i = numplayers; i < PlayerMax; i++) {
		TheMap.Info.PlayerType[i] = PlayerNobody;
	}
	return 0;
}

/**
**  Register CCL features for map.
*/
void MapCclRegister(void)
{
	lua_register(Lua, "StratagusMap", CclStratagusMap);
	lua_register(Lua, "RevealMap", CclRevealMap);
	lua_register(Lua, "CenterMap", CclCenterMap);
	lua_register(Lua, "SetStartView", CclSetStartView);
	lua_register(Lua, "ShowMapLocation", CclShowMapLocation);

	lua_register(Lua, "SetDefaultMap", CclSetDefaultMap);
	lua_register(Lua, "SetFogOfWar", CclSetFogOfWar);
	lua_register(Lua, "SetMinimapTerrain", CclSetMinimapTerrain);

	lua_register(Lua, "SetFogOfWarGraphics", CclSetFogOfWarGraphics);
	lua_register(Lua, "SetFogOfWarOpacity", CclSetFogOfWarOpacity);

	lua_register(Lua, "SetForestRegeneration",CclSetForestRegeneration);

	lua_register(Lua, "SelectTileset", CclSelectTileset);
	lua_register(Lua, "SetTile", CclSetTile);
	lua_register(Lua, "DefinePlayerTypes", CclDefinePlayerTypes);
}

//@}