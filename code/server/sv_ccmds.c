/*
===========================================================================
Copyright (C) 1999-2005 Id Software, Inc.

This file is part of Quake III Arena source code.

Quake III Arena source code is free software; you can redistribute it
and/or modify it under the terms of the GNU General Public License as
published by the Free Software Foundation; either version 2 of the License,
or (at your option) any later version.

Quake III Arena source code is distributed in the hope that it will be
useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Quake III Arena source code; if not, write to the Free Software
Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
===========================================================================
*/

#include "server.h"

/*
===============================================================================

OPERATOR CONSOLE ONLY COMMANDS

These commands can only be entered from stdin or by a remote operator datagram
===============================================================================
*/

/*
Reusable version of SV_GetPlayerByHandle() that doesn't
print any silly messages.
*/
client_t *SV_BetterGetPlayerByHandle(const char *handle) {

    client_t  *cl;
    int       i;
    char      cleanName[64];

    // make sure server is running
    if (!com_sv_running->integer) {
        return NULL;
    }

    // Check whether this is a numeric player handle
    for(i = 0; handle[i] >= '0' && handle[i] <= '9'; i++);

    if(!handle[i]) {

        int plid = atoi(handle);

        // Check for numeric playerid match
        if(plid >= 0 && plid < sv_maxclients->integer) {

            cl = &svs.clients[plid];

            if(cl->state) {
                return cl;
            }

        }
    }

    // check for a name match
    for (i = 0, cl=svs.clients; i < sv_maxclients->integer; i++,cl++) {

        if (!cl->state) {
            continue;
        }

        if (!Q_stricmp(cl->name, handle)) {
            return cl;
        }

        Q_strncpyz(cleanName, cl->name, sizeof(cleanName));
        Q_CleanStr(cleanName );
        if (!Q_stricmp(cleanName, handle)) {
            return cl;
        }

    }

    return NULL;
}

/*
==================
SV_GetPlayerByHandle

Returns the player with player id or name from Cmd_Argv(1)
==================
*/
static client_t *SV_GetPlayerByHandle(void) {

    char        *s;
    char        name[64];
    int         count = 0;
    int         i, idnum;
    client_t    *cl;
    client_t    *matches[MAX_CLIENTS];

    // make sure server is running
    if (!com_sv_running->integer) {
        return NULL;
    }

    if (Cmd_Argc() < 2) {
        Com_Printf("No client specified\n");
        return NULL;
    }

    s = Cmd_Argv(1);

    // check whether this is a numeric player handle
    for (i = 0; s[i] >= '0' && s[i] <= '9'; i++);

    if (!s[i]) {

        // numeric player handle given as input
        idnum = atoi(s);
        if ((idnum < 0) || (idnum >= sv_maxclients->integer)) {
            Com_Printf("Bad client slot: %i\n", idnum);
            return NULL;
        }

        cl = &svs.clients[idnum];

        if (!cl->state) {
            Com_Printf("Client in slot %i is not connected\n", idnum);
            return NULL;
        }

        return cl;

    } else {

        // full/partial player name given as input
        for (i = 0; i < sv_maxclients->integer ; i++) {

            cl = &svs.clients[i];

            // client is not connected
            if (!cl->state) {
                continue;
            }

            strcpy(name, cl->name);
            Q_CleanStr(name);

            // check for exact match
            if (!Q_stricmp(name,s)) {
                matches[0] = &svs.clients[i];
                count = 1;
                break;
            }

            // check for substring match
            if (Q_strisub(name, s)) {
                matches[count] = &svs.clients[i];
                count++;
            }

        }

        if (count == 0) {

            // no match found for the given input string
            Com_Printf("No client found matching %s\n", s);
            return NULL;

        } else if (count > 1) {

            // multiple matches found for the given string
            Com_Printf("Multiple clients found matching %s:\n", s);

            for (i = 0; i < count; i++) {
                cl = matches[i];
                strcpy(name, cl->name);
                Com_Printf(" %2d: [%s]\n", (int)(cl - svs.clients), name);
            }

            return NULL;
        }

        // found just 1 match
        return matches[0];

    }

}

/*
==================
SV_GetPlayerByNum

Returns the player with idnum from Cmd_Argv(1)
==================
*/
static client_t *SV_GetPlayerByNum(void) {

    int         i;
    int         idnum;
    char        *s;
    client_t    *cl;

    // make sure server is running
    if (!com_sv_running->integer) {
        return NULL;
    }

    if (Cmd_Argc() < 2) {
        Com_Printf("No client specified\n");
        return NULL;
    }

    s = Cmd_Argv(1);

    for (i = 0; s[i]; i++) {
        if (s[i] < '0' || s[i] > '9') {
            Com_Printf("Bad slot number: %s\n", s);
            return NULL;
        }
    }

    idnum = atoi(s);
    if (idnum < 0 || idnum >= sv_maxclients->integer) {
        Com_Printf("Bad client slot: %i\n", idnum);
        return NULL;
    }

    cl = &svs.clients[idnum];
    if (!cl->state) {
        Com_Printf("Client in slot %i is not connected\n", idnum);
        return NULL;
    }

    return cl;

}

#ifdef USE_AUTH
/*
==================
SV_PP_Cmd_Argc_to_idnum

@Barbatos: wow this one looks horrible, FIXME!
==================
*//*
static int SV_Argc_to_idnum( int arg_num ) {
	client_t	*cl;
	int			idnum;
	int			i, k, f, g;
	int			len, nlen, slen;
	int			count;
	char		*search;
	char		*name;

	// make sure server is running
	if ( !com_sv_running->integer ) {
		return -1;
	}

	if ( Cmd_Argc() < 1 ) {
		Com_Printf( "No player specified.\n" );
		return -1;
	}

	search = Cmd_Argv( arg_num );
	
	if(strlen( search ) < 3 )
	{
		for (i = 0; search[i]; i++) {
			if (search[i] < '0' || search[i] > '9') {
				Com_Printf( "Bad slot number: \"%s\".\n", search);
				return -1;
			}
		}
		idnum = atoi( search );
		if ( idnum < 0 || idnum >= sv_maxclients->integer ) {
			Com_Printf( "Bad client slot: %i.\n", idnum );
			return -1;
		}

		cl = &svs.clients[idnum];
		if ( !cl->state ) {
			Com_Printf( "Client %i is not active.\n", idnum );
			return -1;
		}
		return idnum;
	}
	else
	{
		f = 0; g = 0;
		count = 0;
		idnum = -1;
		slen = strlen(search);
		for (i=0,cl=svs.clients ; i < sv_maxclients->integer ; i++,cl++) {
			if ( cl->state >= CS_CONNECTED ) {
				name = cl->name;
				nlen = strlen(name);
				len = nlen - slen;
				if(len>=0) {
					for (k=0; k<=len; k++, name++) {
						if( toupper(name[0])==toupper(search[0]) ) {
							for (g=1,f=1; search[f] && name[g]; g++) {
								if( Q_IsColorString( &name[g] ) ) {
									g++;
								} else {
									if( toupper(name[g])!=toupper(search[f]) ) break;
									f++;
								}
							}
							if (f==slen) { 
								count++; 
								idnum = i; 
								break; 
							}
						}
					}
				}
			}
		}
		if( count == 1 ) { 
			return idnum;
		}
		if( count > 0 ) { 
			Com_Printf( "Too many players found for \"%s\".\n", search );
			return -1;
		}
		if( count == 0 ) { 
			Com_Printf( "No player found for \"%s\".\n", search );
			return -1;
		}
	}
	
	return -1;
}*/
#endif


/////////////////////////////////////////////////////////////////////
// Name        : SV_StripExtension
// Description : Remove the extension from a given file name
// Author      : Fenix
/////////////////////////////////////////////////////////////////////
static void SV_StripExtension(const char *in, char *out) {
    while (*in && *in != '.') {
        *out++ = *in++;
    }
    *out = 0;
}


/////////////////////////////////////////////////////////////////////
// Name        : SV_SortMaps
// Description : Array sorting comparison function (for qsort)
// Author      : Fenix
/////////////////////////////////////////////////////////////////////
static int QDECL SV_SortMaps(const void *a, const void *b) {
    return strcmp (*(const char **) a, *(const char **) b);
}


/////////////////////////////////////////////////////////////////////
// Name        : SV_GetMapSoundingLike
// Description : Retrieve a full map name given a substring of it
// Author      : Fenix, p5yc0runn3r
/////////////////////////////////////////////////////////////////////
char *SV_GetMapSoundingLike(const char *s) 
{
    int  i, mapcount;
    int  len = 0, count = 0;
    char *matches[MAX_MAPLIST_SIZE];
    char *searchmap;
    static char maplist[MAX_MAPLIST_STRING];

    // [BUGFIX]: instead of iterating through all the maps matching both full and
    // partial name, search just for the exact map name and return it if the match is found
    Com_sprintf(maplist, sizeof(maplist), "maps/%s.bsp", s);
    if (FS_ReadFile(maplist, NULL) > 0) 
    {
        Com_sprintf(maplist, sizeof(maplist), "%s", s);
        return maplist; // @p5yc0runn3r: Return static string
    }

    // We didn't found an exact name match. Keep iterating through all the
    // available maps matching partial substrings
    if (!(mapcount = FS_GetFileList("maps", ".bsp", maplist, sizeof(maplist)))) 
    {
        Com_Printf("Unable to retrieve map list\n");
        return NULL;
    }

    for (searchmap = maplist, i = 0; i < mapcount && count < MAX_MAPLIST_SIZE; i++, searchmap += len + 1) 
    {
        len = strlen(searchmap);
        SV_StripExtension(searchmap, searchmap);

        // Check for substring match
        if (Q_strisub(searchmap, s)) 
        {
            matches[count] = searchmap;
            count++;
        }
    }

    // One match = one map, found match.
    if (count == 1) return matches[0]; // @p5yc0runn3r: matches points to static string, safe to return.

    if (count > 1) 
    {
        // Multiple matches found for the given map name
        Com_Printf("Multiple maps found matching '%s':\n", s);

        // Sorting the short map list alphabetically
        qsort(matches, count, sizeof(char *), SV_SortMaps);

        for (i = 0; i < count; i++) 
        {
            // Printing a short map list so the user can retry with a more specific name
            Com_Printf(" %2d: [%s]\n", i + 1, matches[i]);
        }

        if (count >= MAX_MAPLIST_SIZE) 
        {
            // Tell the user that there are actually more
            // maps matching the given substring, although
            // we are not displaying them....
            Com_Printf("...and more\n");
        }
        return NULL;
    }

    // No match found for the given map name input
    Com_Printf("No map found matching '%s'\n", s);
    return NULL;
}

//=========================================================

/*
==================
SV_Map_f

Restart the server on a different map
==================
*/
static void SV_Map_f(void) {

    char      *cmd;
    char      *searchmap;
    char      mapname[MAX_QPATH];
    qboolean  killBots, cheat;

    if (Cmd_Argc() < 2) {
        return;
    }

    searchmap = SV_GetMapSoundingLike(Cmd_Argv(1));
    if (!searchmap) {
        return;
    }

    // force latched values to get set
    Cvar_Get ("g_gametype", "0", CVAR_SERVERINFO | CVAR_USERINFO | CVAR_LATCH);

    cmd = Cmd_Argv(0);
    if (Q_stricmpn(cmd, "sp", 2) == 0) {
        Cvar_SetValue("g_gametype", GT_SINGLE_PLAYER);
        Cvar_SetValue("g_doWarmup", 0);
        // may not set sv_maxclients directly, always set latched
        Cvar_SetLatched("sv_maxclients", "8");
        cmd += 2;
        cheat = qfalse;
        killBots = qtrue;
    }
    else {

        if (!Q_stricmp( cmd, "devmap") || !Q_stricmp(cmd, "spdevmap") ) {
            cheat = qtrue;
            killBots = qtrue;
        } else {
            cheat = qfalse;
            killBots = qfalse;
        }

        if (sv_gametype->integer == GT_SINGLE_PLAYER) {
            Cvar_SetValue("g_gametype", GT_FFA);
        }

    }

    // save the map name here cause on a map restart we reload the q3config.cfg
    // and thus nuke the arguments of the map command
    Q_strncpyz(mapname, searchmap, sizeof(mapname));

    // start up the map
    SV_SpawnServer(mapname, killBots);

    // set the cheat value
    // if the level was started with "map <levelname>", then
    // cheats will not be allowed.  If started with "devmap <levelname>"
    // then cheats will be allowed
    if (cheat) {
        Cvar_Set("sv_cheats", "1");
    } else {
        Cvar_Set("sv_cheats", "0");
    }

}

/*
================
SV_MapRestart_f

Completely restarts a level, but doesn't send a new gamestate to the clients.
This allows fair starts with variable load times.
================
*/
static void SV_MapRestart_f( void ) {
	int			i;
	client_t	*client;
	char		*denied;
	qboolean	isBot;
	int			delay;

	// make sure we aren't restarting twice in the same frame
	if ( com_frameTime == sv.serverId ) {
		return;
	}

	// make sure server is running
	if ( !com_sv_running->integer ) {
		Com_Printf( "Server is not running.\n" );
		return;
	}

	if ( sv.restartTime ) {
		return;
	}

	if (Cmd_Argc() > 1 ) {
		delay = atoi( Cmd_Argv(1) );
	}
	else {
		delay = 5;
	}
	if( delay && !Cvar_VariableValue("g_doWarmup") ) {
		sv.restartTime = sv.time + delay * 1000;
		SV_SetConfigstring( CS_WARMUP, va("%i", sv.restartTime) );
		return;
	}

	// check for changes in variables that can't just be restarted
	// check for maxclients change
	if ( sv_maxclients->modified || sv_gametype->modified ) {
		char	mapname[MAX_QPATH];

		Com_Printf( "variable change -- restarting.\n" );
		// restart the map the slow way
		Q_strncpyz( mapname, Cvar_VariableString( "mapname" ), sizeof( mapname ) );

		SV_SpawnServer( mapname, qfalse );
		return;
	}

	// toggle the server bit so clients can detect that a
	// map_restart has happened
	svs.snapFlagServerBit ^= SNAPFLAG_SERVERCOUNT;

	// generate a new serverid	
	// TTimo - don't update restartedserverId there, otherwise we won't deal correctly with multiple map_restart
	sv.serverId = com_frameTime;
	Cvar_Set( "sv_serverid", va("%i", sv.serverId ) );

	// if a map_restart occurs while a client is changing maps, we need
	// to give them the correct time so that when they finish loading
	// they don't violate the backwards time check in cl_cgame.c
	for (i = 0; i < sv_maxclients->integer; i++) {
		if (svs.clients[i].state == CS_PRIMED) {
			svs.clients[i].oldServerTime = sv.restartTime;
		}
	}

	// reset all the vm data in place without changing memory allocation
	// note that we do NOT set sv.state = SS_LOADING, so configstrings that
	// had been changed from their default values will generate broadcast updates
	sv.state = SS_LOADING;
	sv.restarting = qtrue;

	SV_RestartGameProgs();

	// run a few frames to allow everything to settle
	for (i = 0; i < 3; i++)
	{
		VM_Call (gvm, GAME_RUN_FRAME, sv.time);
		sv.time += 100;
		svs.time += 100;
	}

	sv.state = SS_GAME;
	sv.restarting = qfalse;

	// connect and begin all the clients
	for (i=0 ; i<sv_maxclients->integer ; i++) {

		client = &svs.clients[i];

		// send the new gamestate to all connected clients
		if ( client->state < CS_CONNECTED) {
			continue;
		}

		if ( client->netchan.remoteAddress.type == NA_BOT ) {
			isBot = qtrue;
		} else {
			isBot = qfalse;
		}

		// add the map_restart command
		SV_AddServerCommand( client, "map_restart\n" );

		// connect the client again, without the firstTime flag
		denied = VM_ExplicitArgPtr( gvm, VM_Call( gvm, GAME_CLIENT_CONNECT, i, qfalse, isBot ) );
		if ( denied ) {
			// this generally shouldn't happen, because the client
			// was connected before the level change
			SV_DropClient( client, denied );
			Com_Printf( "SV_MapRestart_f(%d): dropped client %i - denied!\n", delay, i );
			continue;
		}

		if(client->state == CS_ACTIVE) {
			SV_ClientEnterWorld(client, &client->lastUsercmd);
		}
		else {
			// If we don't reset client->lastUsercmd and are restarting during map load,
		 	// the client will hang because we'll use the last Usercmd from the previous map,
		 	// which is wrong obviously.
		 	SV_ClientEnterWorld(client, NULL);
		}
	}	

	// run another frame to allow things to look at all the players
	VM_Call (gvm, GAME_RUN_FRAME, sv.time);
	sv.time += 100;
	svs.time += 100;
}

//===============================================================

/*
==================
SV_Kick_f

Kick a user off of the server. FIXME: move to game
==================
*/
static void SV_Kick_f(void) {

    int         i;
    client_t    *cl;
    char        *reason = "was kicked";

    // make sure server is running
    if (!com_sv_running->integer ) {
        Com_Printf("Server is not running\n");
        return;
    }

    if ((Cmd_Argc() < 2) || (Cmd_Argc() > 3)) {
        Com_Printf ("Usage: kick <client> <reason>\nkick all = kick everyone\nkick allbots = kick all bots\n");
        return;
    }

    if (Cmd_Argc() == 3) {
        // If the reason was specified attach it to the disconnect message
        reason = va("was kicked: %s", Cmd_Argv(2));
    }


    if (!Q_stricmp(Cmd_Argv(1), "all")) {

        for (i = 0, cl = svs.clients; i < sv_maxclients->integer; i++, cl++) {

            if (!cl->state) {
                continue;
            }

            if (cl->netchan.remoteAddress.type == NA_LOOPBACK) {
                continue;
            }

            SV_DropClient(cl, reason);
            cl->lastPacketTime = svs.time;    // in case there is a funny zombie
        }

    } else if (!Q_stricmp(Cmd_Argv(1), "allbots")) {

        for (i = 0, cl = svs.clients; i < sv_maxclients->integer; i++, cl++) {

            if (!cl->state) {
                continue;
            }

            if (cl->netchan.remoteAddress.type != NA_BOT) {
                continue;
            }

            SV_DropClient(cl, reason);
            cl->lastPacketTime = svs.time;    // in case there is a funny zombie

        }

    } else {

        cl = SV_GetPlayerByHandle();

        if (!cl) {
            return;
        }

        if (cl->netchan.remoteAddress.type == NA_LOOPBACK) {
            SV_SendServerCommand(NULL, "print \"%s\"", "Cannot kick host client\n");
            return;
        }

        SV_DropClient(cl, reason);
        cl->lastPacketTime = svs.time;    // in case there is a funny zombie

    }

}

/*
==================
SV_Ban_f

Ban a user from being able to play on this server through the auth
server
==================
*/
static void SV_Ban_f(void) {

    client_t *cl;

    // make sure server is running
    if (!com_sv_running->integer) {
        Com_Printf( "Server is not running\n" );
        return;
    }

    if (Cmd_Argc() != 2 ) {
        Com_Printf ("Usage: banUser <client>\n");
        return;
    }

    cl = SV_GetPlayerByHandle();

    if (!cl) {
        return;
    }

    if (cl->netchan.remoteAddress.type == NA_LOOPBACK) {
        SV_SendServerCommand(NULL, "print \"%s\"", "Cannot ban host client\n");
        return;
    }

    // look up the authorize server's IP
    if (!svs.authorizeAddress.ip[0] && svs.authorizeAddress.type != NA_BAD) {
        Com_Printf("Resolving %s\n", AUTHORIZE_SERVER_NAME);
        if (!NET_StringToAdr( AUTHORIZE_SERVER_NAME, &svs.authorizeAddress)) {
            Com_Printf("Couldn't resolve address\n");
            return;
        }
        svs.authorizeAddress.port = BigShort( PORT_AUTHORIZE );
        Com_Printf("%s resolved to %i.%i.%i.%i:%i\n", AUTHORIZE_SERVER_NAME,
            svs.authorizeAddress.ip[0], svs.authorizeAddress.ip[1],
            svs.authorizeAddress.ip[2], svs.authorizeAddress.ip[3],
            BigShort(svs.authorizeAddress.port));
    }

    // otherwise send their ip to the authorize server
    if (svs.authorizeAddress.type != NA_BAD) {
        NET_OutOfBandPrint(NS_SERVER, svs.authorizeAddress,
                           "banUser %i.%i.%i.%i", cl->netchan.remoteAddress.ip[0], cl->netchan.remoteAddress.ip[1],
                           cl->netchan.remoteAddress.ip[2], cl->netchan.remoteAddress.ip[3]);

        Com_Printf("%s was banned from coming back\n", cl->name);

    }
}

/*
==================
SV_BanNum_f

Ban a user from being able to play on this server through the auth
server
==================
*/
static void SV_BanNum_f( void ) {

    client_t *cl;

    // make sure server is running
    if (!com_sv_running->integer ) {
        Com_Printf("Server is not running\n");
        return;
    }

    if (Cmd_Argc() != 2) {
        Com_Printf("Usage: banClient <client number>\n");
        return;
    }

    cl = SV_GetPlayerByNum();
    if (!cl) {
        return;
    }

    if (cl->netchan.remoteAddress.type == NA_LOOPBACK) {
        SV_SendServerCommand(NULL, "print \"%s\"", "Cannot ban host client\n");
        return;
    }

    // look up the authorize server's IP
    if (!svs.authorizeAddress.ip[0] && svs.authorizeAddress.type != NA_BAD) {
        Com_Printf( "Resolving %s\n", AUTHORIZE_SERVER_NAME);
        if (!NET_StringToAdr( AUTHORIZE_SERVER_NAME, &svs.authorizeAddress)) {
            Com_Printf("Couldn't resolve address\n");
            return;
        }
        svs.authorizeAddress.port = BigShort(PORT_AUTHORIZE);
        Com_Printf("%s resolved to %i.%i.%i.%i:%i\n", AUTHORIZE_SERVER_NAME,
            svs.authorizeAddress.ip[0], svs.authorizeAddress.ip[1],
            svs.authorizeAddress.ip[2], svs.authorizeAddress.ip[3],
            BigShort( svs.authorizeAddress.port));
    }

    // otherwise send their ip to the authorize server
    if (svs.authorizeAddress.type != NA_BAD) {
        NET_OutOfBandPrint(NS_SERVER, svs.authorizeAddress,
                           "banUser %i.%i.%i.%i", cl->netchan.remoteAddress.ip[0], cl->netchan.remoteAddress.ip[1],
                           cl->netchan.remoteAddress.ip[2], cl->netchan.remoteAddress.ip[3] );

        Com_Printf("%s was banned from coming back\n", cl->name);

    }
}

/*
==================
SV_KickNum_f

Kick a user off of the server  FIXME: move to game
==================
*/
static void SV_KickNum_f(void) {

    client_t *cl;

    // make sure server is running
    if (!com_sv_running->integer) {
        Com_Printf("Server is not running\n");
        return;
    }

    if (Cmd_Argc() != 2) {
        Com_Printf("Usage: kicknum <client number>\n");
        return;
    }

    cl = SV_GetPlayerByNum();
    if (!cl) {
        return;
    }

    if (cl->netchan.remoteAddress.type == NA_LOOPBACK) {
        SV_SendServerCommand(NULL, "print \"%s\"", "Cannot kick host client\n");
        return;
    }

    SV_DropClient(cl, "was kicked");
    cl->lastPacketTime = svs.time;    // in case there is a funny zombie

}

/*
================
SV_Status_f
================
*/
static void SV_Status_f(void) {

    int            i, j, l;
    client_t       *cl;
    playerState_t  *ps;
    const char     *s;
    int            ping;

    // make sure server is running
    if (!com_sv_running->integer) {
        Com_Printf("Server is not running\n");
        return;
    }

    Com_Printf ("map: %s\n", sv_mapname->string);
    Com_Printf ("num score ping name            lastmsg address               qport rate\n");
    Com_Printf ("--- ----- ---- --------------- ------- --------------------- ----- -----\n");

    for (i = 0, cl = svs.clients; i < sv_maxclients->integer; i++, cl++) {

        if (!cl->state) {
            continue;
        }

        Com_Printf("%3i ", i);
        ps = SV_GameClientNum( i );
        Com_Printf("%5i ", ps->persistant[PERS_SCORE]);

        if (cl->state == CS_CONNECTED) {
            Com_Printf("CNCT ");
        } else if (cl->state == CS_ZOMBIE) {
            Com_Printf("ZMBI ");
        } else {
            ping = cl->ping < 9999 ? cl->ping : 9999;
            Com_Printf ("%4i ", ping);
        }

        Com_Printf ("%s", cl->name);
        // TTimo adding a ^7 to reset the color
        // NOTE: colored names in status breaks the padding (WONTFIX)
        Com_Printf ("^7");
        l = 16 - strlen(cl->name);
        for (j=0 ; j<l ; j++) {
            Com_Printf (" ");
        }

        Com_Printf ("%7i ", svs.time - cl->lastPacketTime );
        s = NET_AdrToString( cl->netchan.remoteAddress );
        Com_Printf ("%s", s);
        l = 22 - strlen(s);
        for (j = 0; j < l; j++) {
            Com_Printf(" ");
        }

        Com_Printf ("%5i", cl->netchan.qport);
        Com_Printf (" %5i", cl->rate);
        Com_Printf ("\n");
    }

    Com_Printf ("\n");

}

/*
==================
SV_ConSay_f
==================
*/
static void SV_ConSay_f(void) {

    char    *p;
    char    text[1024];

    // make sure server is running
    if (!com_sv_running->integer) {
        Com_Printf("Server is not running.\n");
        return;
    }

    if (Cmd_Argc () < 2) {
        return;
    }

    strcpy(text, sv_sayprefix->string);
    p = Cmd_Args();

    if (*p == '"') {
        p++;
        p[strlen(p) - 1] = 0;
    }

    strcat(text, p);
    SV_SendServerCommand(NULL, "chat \"%s\n\"", text);

}


/*
==================
SV_ConTell_f
==================
*/
static void SV_ConTell_f(void) {

    char      text[1024];
    char      *p;
    client_t  *cl;

    // make sure server is running
    if (!com_sv_running->integer) {
        Com_Printf("Server is not running\n");
        return;
    }

    if (Cmd_Argc() < 3) {
        Com_Printf("Usage: tell <client> <text>\n");
        return;
    }

    cl = SV_GetPlayerByHandle();

    if (!cl) {
        return;
    }

    strcpy (text, sv_tellprefix->string);
    p = Cmd_ArgsFrom(2);

    if (*p == '"') {
        p++;
        p[strlen(p) - 1] = 0;
    }

    strcat(text, p);
    SV_SendServerCommand(cl, "chat \"%s\n\"", text);

}


/*
==================
SV_Heartbeat_f

Also called by SV_DropClient, SV_DirectConnect, and SV_SpawnServer
==================
*/
void SV_Heartbeat_f(void) {
    svs.nextHeartbeatTime = -9999999;
}


/*
===========
SV_Serverinfo_f

Examine the serverinfo string
===========
*/
static void SV_Serverinfo_f(void) {
    Com_Printf("Server info settings:\n");
    Info_Print(Cvar_InfoString(CVAR_SERVERINFO));
}


/*
===========
SV_Systeminfo_f

Examine or change the serverinfo string
===========
*/
static void SV_Systeminfo_f(void) {
    Com_Printf("System info settings:\n");
    Info_Print(Cvar_InfoString(CVAR_SYSTEMINFO));
}


/*
===========
SV_DumpUser_f

Examine all a users info strings FIXME: move to game
===========
*/
static void SV_DumpUser_f(void) {

    client_t *cl;

    // make sure server is running
    if (!com_sv_running->integer) {
        Com_Printf("Server is not running\n");
        return;
    }

    if (Cmd_Argc() != 2) {
        Com_Printf ("Usage: dumpuser <client>\n");
        return;
    }

    cl = SV_GetPlayerByHandle();
    if (!cl) {
        return;
    }

    Com_Printf("userinfo\n");
    Com_Printf("--------\n");
    Info_Print(cl->userinfo);

}


/*
=================
SV_KillServer
=================
*/
static void SV_KillServer_f(void) {
    SV_Shutdown("killserver");
}

//===========================================================

/*
Start a server-side demo.

This does it all, create the file and adjust the demo-related
stuff in client_t.

This is mostly ripped from sv_client.c/SV_SendClientGameState
and cl_main.c/CL_Record_f.
*/
static void SVD_StartDemoFile(client_t *client, const char *path) {

    int             i, len;
    entityState_t   *base, nullstate;
    msg_t           msg;
    byte            buffer[MAX_MSGLEN];
    fileHandle_t    file;
#ifdef USE_DEMO_FORMAT_42
    char            *s;
    int             v, size;
#endif

    Com_DPrintf("SVD_StartDemoFile\n");
    assert(!client->demo_recording);

    // create the demo file and write the necessary header
    file = FS_FOpenFileWrite(path);
    assert(file != 0);

    /* File_write_header_demo // ADD this fx */
    /* HOLBLIN  entete demo */
    #ifdef USE_DEMO_FORMAT_42
        //@Barbatos: get the mod version from the server
        s = Cvar_VariableString("g_modversion");

        size = strlen(s);
        len = LittleLong(size);
        FS_Write(&len, 4, file);
        FS_Write(s, size, file);

        v = LittleLong(DEMO_VERSION);
        FS_Write (&v, 4, file);

        len = 0;
        len = LittleLong(len);
        FS_Write(&len, 4, file);
        FS_Write(&len, 4, file);
    #endif
    /* END HOLBLIN  entete demo */

    MSG_Init(&msg, buffer, sizeof(buffer));
    MSG_Bitstream(&msg); // XXX server code doesn't do this, client code does
    MSG_WriteLong(&msg, client->lastClientCommand); // TODO: or is it client->reliableSequence?
    MSG_WriteByte(&msg, svc_gamestate);
    MSG_WriteLong(&msg, client->reliableSequence);

    for (i = 0; i < MAX_CONFIGSTRINGS; i++) {
        if (sv.configstrings[i][0]) {
            MSG_WriteByte(&msg, svc_configstring);
            MSG_WriteShort(&msg, i);
            MSG_WriteBigString(&msg, sv.configstrings[i]);
        }
    }

    Com_Memset(&nullstate, 0, sizeof(nullstate));
    for (i = 0 ; i < MAX_GENTITIES; i++) {
        base = &sv.svEntities[i].baseline;
        if (!base->number) {
            continue;
        }
        MSG_WriteByte(&msg, svc_baseline);
        MSG_WriteDeltaEntity(&msg, &nullstate, base, qtrue);
    }

    MSG_WriteByte(&msg, svc_EOF);
    MSG_WriteLong(&msg, client - svs.clients);
    MSG_WriteLong(&msg, sv.checksumFeed);
    MSG_WriteByte(&msg, svc_EOF); // XXX server code doesn't do this, SV_Netchan_Transmit adds it!

    len = LittleLong(client->netchan.outgoingSequence - 1);
    FS_Write(&len, 4, file);

    len = LittleLong (msg.cursize);
    FS_Write(&len, 4, file);
    FS_Write(msg.data, msg.cursize, file);

    #ifdef USE_DEMO_FORMAT_42
        // add size of packet in the end for backward play /* holblin */
        FS_Write(&len, 4, file);
    #endif

    FS_Flush(file);

    // adjust client_t to reflect demo started
    client->demo_recording = qtrue;
    client->demo_file = file;
    client->demo_waiting = qtrue;
    client->demo_backoff = 1;
    client->demo_deltas = 0;
}

/*
Write a message to a server-side demo file.
*/
void SVD_WriteDemoFile(const client_t *client, const msg_t *msg) {

    int len;
    msg_t cmsg;
    byte cbuf[MAX_MSGLEN];
    fileHandle_t file = client->demo_file;

    if (*(int *)msg->data == -1) { // TODO: do we need this?
        Com_DPrintf("Ignored connectionless packet, not written to demo!\n");
        return;
    }

    // TODO: we only copy because we want to add svc_EOF; can we add it and then
    // "back off" from it, thus avoiding the copy?
    MSG_Copy(&cmsg, cbuf, sizeof(cbuf), (msg_t*) msg);
    MSG_WriteByte(&cmsg, svc_EOF); // XXX server code doesn't do this, SV_Netchan_Transmit adds it!

    // TODO: the headerbytes stuff done in the client seems unnecessary
    // here because we get the packet *before* the netchan has it's way
    // with it; just not sure that's really true :-/

    len = LittleLong(client->netchan.outgoingSequence);
    FS_Write(&len, 4, file);

    len = LittleLong(cmsg.cursize);
    FS_Write(&len, 4, file);

    FS_Write(cmsg.data, cmsg.cursize, file); // XXX don't use len!

    #ifdef USE_DEMO_FORMAT_42
        // add size of packet in the end for backward play /* holblin */
        FS_Write(&len, 4, file);
    #endif

    FS_Flush(file);
}

/*
Stop a server-side demo.

This finishes out the file and clears the demo-related stuff
in client_t again.
*/
static void SVD_StopDemoFile(client_t *client) {

    int marker = -1;
    fileHandle_t file = client->demo_file;

    Com_DPrintf("SVD_StopDemoFile\n");
    assert(client->demo_recording);

    // write the necessary trailer and close the demo file
    FS_Write(&marker, 4, file);
    FS_Write(&marker, 4, file);
    FS_Flush(file);
    FS_FCloseFile(file);

    // adjust client_t to reflect demo stopped
    client->demo_recording = qfalse;
    client->demo_file = -1;
    client->demo_waiting = qfalse;
    client->demo_backoff = 1;
    client->demo_deltas = 0;
}

/*
Clean up player name to be suitable as path name.
Similar to Q_CleanStr() but tweaked.
*/
static void SVD_CleanPlayerName(char *name) {

    char *src = name, *dst = name, c;

    while ((c = *src)) {
        // note Q_IsColorString(src++) won't work since it's a macro
        if (Q_IsColorString(src)) {
            src++;
        } else if (c == ':' || c == '\\' || c == '/' || c == '*' || c == '?') {
            *dst++ = '%';
        } else if (c > ' ' && c < 0x7f) {
            *dst++ = c;
        }
        src++;
    }

    *dst = '\0';

    if (strlen(name) == 0) {
        strcpy(name, "UnnamedPlayer");
    }
}

/*
Generate unique name for a new server demo file.
(We pretend there are no race conditions.)
*/
static void SV_NameServerDemo(char *filename, int length, const client_t *client, char *fn) {

    qtime_t time;
    char playername[32];
    char demoName[64]; //@Barbatos

    Com_DPrintf("SV_NameServerDemo\n");

    Com_RealTime(&time);
    Q_strncpyz(playername, client->name, sizeof(playername));
    SVD_CleanPlayerName(playername);

    if (fn != NULL) {

        Q_strncpyz(demoName, fn, sizeof(demoName));

        #ifdef USE_DEMO_FORMAT_42
            Q_snprintf(filename, length-1, "%s/%s.urtdemo", sv_demofolder->string, demoName );
            if (FS_FileExists(filename)) {
                Q_snprintf(filename, length-1, "%s/%s_%d.urtdemo", sv_demofolder->string, demoName, Sys_Milliseconds() );
            }
        #else
            Q_snprintf(filename, length-1, "%s/%s.dm_%d", sv_demofolder->string, demoName , PROTOCOL_VERSION );
            if (FS_FileExists(filename)) {
                Q_snprintf(filename, length-1, "%s/%s_%d.dm_%d", sv_demofolder->string, demoName, Sys_Milliseconds() , PROTOCOL_VERSION );
            }
        #endif
    } else {
        #ifdef USE_DEMO_FORMAT_42
            Q_snprintf(
                filename, length-1, "%s/%.4d-%.2d-%.2d_%.2d-%.2d-%.2d_%s_%d.urtdemo",
                sv_demofolder->string, time.tm_year+1900, time.tm_mon + 1, time.tm_mday,
                time.tm_hour, time.tm_min, time.tm_sec,
                playername,
                Sys_Milliseconds()
            );
        #else
            Q_snprintf(
                filename, length-1, "%s/%.4d-%.2d-%.2d_%.2d-%.2d-%.2d_%s_%d.dm_%d",
                sv_demofolder->string, time.tm_year+1900, time.tm_mon + 1, time.tm_mday,
                time.tm_hour, time.tm_min, time.tm_sec,
                playername,
                Sys_Milliseconds(),
                PROTOCOL_VERSION
            );
        #endif
        filename[length-1] = '\0';

        if (FS_FileExists(filename)) {
            filename[0] = 0;
            return;
        }
    }
}

static void SV_StartRecordOne(client_t *client, char *filename) {

    char path[MAX_OSPATH];

    Com_DPrintf("SV_StartRecordOne\n");

    if (client->demo_recording) {
        Com_Printf("startserverdemo: %s is already being recorded\n", client->name);
        return;
    }

    if (client->state != CS_ACTIVE) {
        Com_Printf("startserverdemo: %s is not active\n", client->name);
        return;
    }

    if (client->netchan.remoteAddress.type == NA_BOT) {
        Com_Printf("startserverdemo: %s is a bot\n", client->name);
        return;
    }

    SV_NameServerDemo(path, sizeof(path), client, filename);
    SVD_StartDemoFile(client, path);

    if(sv_demonotice->string) {
        SV_SendServerCommand(client, "print \"%s\"\n", sv_demonotice->string);
    }

    Com_Printf("startserverdemo: recording %s to %s\n", client->name, path);
}

static void SV_StartRecordAll(void) {

    int slot;
    client_t *client;

    Com_DPrintf("SV_StartRecordAll\n");

    for (slot = 0, client = svs.clients; slot < sv_maxclients->integer; slot++, client++) {
        // filter here to avoid lots of bogus messages from SV_StartRecordOne()
        if (client->netchan.remoteAddress.type == NA_BOT
            || client->state != CS_ACTIVE
            || client->demo_recording) {
            continue;
        }
        SV_StartRecordOne(client, NULL);
    }
}

static void SV_StopRecordOne(client_t *client) {

    Com_DPrintf("SV_StopRecordOne\n");

    if (!client->demo_recording) {
        Com_Printf("stopserverdemo: %s is not being recorded\n", client->name);
        return;
    }

    if (client->state != CS_ACTIVE) { // disconnects are handled elsewhere
        Com_Printf("stopserverdemo: %s is not active\n", client->name);
        return;
    }

    if (client->netchan.remoteAddress.type == NA_BOT) {
        Com_Printf("stopserverdemo: %s is a bot\n", client->name);
        return;
    }

    SVD_StopDemoFile(client);
    Com_Printf("stopserverdemo: stopped recording %s\n", client->name);


}

static void SV_StopRecordAll(void) {

    int slot;
    client_t *client;

    Com_DPrintf("SV_StopRecordAll\n");

    for (slot=0, client=svs.clients; slot < sv_maxclients->integer; slot++, client++) {
        // filter here to avoid lots of bogus messages from SV_StopRecordOne()
        if (client->netchan.remoteAddress.type == NA_BOT
            || client->state != CS_ACTIVE // disconnects are handled elsewhere
            || !client->demo_recording) {
            continue;
        }
        SV_StopRecordOne(client);
    }

}

/*
==================
SV_StartServerDemo_f

Record a server-side demo for given player/slot. The demo
will be called "YYYY-MM-DD_hh-mm-ss_playername_id.urtdemo",
in the "demos" directory under your game directory. Note
that "startserverdemo all" will start demos for all players
currently in the server. Players who join later require a
new "startserverdemo" command. If you are already recording
demos for some players, another "startserverdemo all" will
start new demos only for players not already recording. Note
that bots will never be recorded, not even if "all" is given.
The server-side demos will stop when "stopserverdemo" is issued
or when the server restarts for any reason (such as a new map
loading).
==================
*/
static void SV_StartServerDemo_f(void) {

    client_t *client;

    Com_DPrintf("SV_StartServerDemo_f\n");

    if (!com_sv_running->integer) {
        Com_Printf("startserverdemo: Server not running\n");
        return;
    }

    if (Cmd_Argc() < 2) {
        Com_Printf("Usage: startserverdemo <client-or-all> [<optional-demo-name>]\n");
        return;
    }

    if (!Q_stricmp(Cmd_Argv(1), "all")) {

        SV_StartRecordAll();

    } else {

        client = SV_GetPlayerByHandle();
        if (!client) {
            return;
        }

        if (Cmd_Argc() > 2) {
            SV_StartRecordOne(client, Cmd_ArgsFrom(2));
        } else {
            SV_StartRecordOne(client, NULL);
        }

    }
}

/*
==================
SV_StopServerDemo_f

Stop a server-side demo for given player/slot. Note that
"stopserverdemo all" will stop demos for all players in
the server.
==================
*/
static void SV_StopServerDemo_f(void)
{
	client_t *client;

	Com_DPrintf("SV_StopServerDemo_f\n");

	if (!com_sv_running->integer) {
		Com_Printf("stopserverdemo: Server not running\n");
		return;
	}

	if (Cmd_Argc() != 2) {
		Com_Printf("Usage: stopserverdemo <client-or-all>\n");
		return;
	}

	if (!Q_stricmp(Cmd_Argv(1), "all")) {

		SV_StopRecordAll();
	}
	else {

	    client = SV_GetPlayerByHandle();

	    if (!client) {
	        return;
	    }

		SV_StopRecordOne(client);
	}

}

/*
==================
SV_CompleteMapName
==================
*/
/*@Barbatos: unused for now
static void SV_CompleteMapName( char *args, int argNum ) {
	if ( argNum == 2 ) {
		Field_CompleteFilename( "maps", "bsp", qtrue, qfalse );
	}
}*/
/*
==================================================================================================================================================
PB
==================================================================================================================================================
*/
///////////////////////////////////////////////////////////
//PB_CleanPlayerName
//////////////////////////////////////////////////////////
static void PB_CleanPlayerName(char *name) {
char *src = name, *dst = name, c;

    while ((c = *src)) {

        if (Q_IsColorString(src)) {
            src++;
        } 
        else if (c == '\\' || c == '/' || c == '?'|| c == '\'' || c == '\"') {
            *dst++ = '*';
        } 
        else if (c == ' ') {
            *dst++ = '_';
        } 
        else if (c > ' ' && c < 0x7f) {
            *dst++ = c;
        }
        src++;
    }

    *dst = '\0';

    if (strlen(name) == 0) {
        strcpy(name, "UrTPlayer");
    }
}
///////////////////////////////////////////////////////////
//PB_CheckIP
//////////////////////////////////////////////////////////
char *PB_CheckIP(client_t *client)
{
	char *ip = Info_ValueForKey(client->userinfo, "ip");
	ip = strtok(ip, ":");
	return ip;
}
///////////////////////////////////////////////////////////
//PB_CheckAuth
//////////////////////////////////////////////////////////
char *PB_CheckAuth(client_t *client)
{
    char	*auth = Info_ValueForKey(client->userinfo, "authl");
   
    if (auth != NULL && strlen(auth) > 2) {

        return auth;
      
    }
   
    else { return NULL;}
}
/*
========================================================
SV_ForceCvar_f_helper
 
Called internally by SV_ForceCvar_f.
========================================================
*/
static void SV_ForceCvar_f_helper( client_t *cl ) {
	int		oldInfoLen;
	int		newInfoLen;
	qboolean	touchedUserinfo = qfalse;

	// Who knows what would happen if we called the VM with a GAME_CLIENT_USERINFO_CHANGED
	// when this client wasn't connected.
	if (cl->state < CS_CONNECTED) {
		return;
	}

	// First remove all keys; there might exist more than one in the userinfo.
	oldInfoLen = strlen(cl->userinfo);
	while (qtrue) {
		Info_RemoveKey(cl->userinfo, Cmd_Argv(2));
		newInfoLen = strlen(cl->userinfo);
		if (oldInfoLen == newInfoLen) { break; } // userinfo wasn't modified.
		oldInfoLen = newInfoLen;
		touchedUserinfo = qtrue;
	}

	if (strlen(Cmd_Argv(3)) > 0) {
		if (strlen(Cmd_Argv(2)) + strlen(Cmd_Argv(3)) + 2 + newInfoLen >= MAX_INFO_STRING) {
			SV_DropClient(cl, "userinfo string length exceeded");
			return;
		}
		Info_SetValueForKey(cl->userinfo, Cmd_Argv(2), Cmd_Argv(3));
		touchedUserinfo = qtrue;
	}

	if (touchedUserinfo) {
		SV_UserinfoChanged(cl);
		VM_Call(gvm, GAME_CLIENT_USERINFO_CHANGED, cl - svs.clients);
	}
}

/*
========================================================
SV_ForceCvar_f

Set a cvar for a user.
========================================================
*/
static void SV_ForceCvar_f(void) {
	client_t	*cl;
	int		i;

	// Make sure server is running.
	if (!com_sv_running->integer) {
		Com_Printf("Server is not running.\n");
		return;
	}

	if (Cmd_Argc() != 4 || strlen(Cmd_Argv(2)) == 0) {
		Com_Printf("USAGE: forcecvar <player name> <cvar name> <cvar value>\nPlayer may be 'allbots'\n");
		return;
	}

	cl = SV_GetPlayerByHandle();
	if (!cl) {
		if (!Q_stricmp(Cmd_Argv(1), "allbots")) {
			for (i = 0, cl = svs.clients; i < sv_maxclients->integer; i++, cl++) {
				if (!cl->state) {
					continue;
				}
				if(cl->netchan.remoteAddress.type != NA_BOT) {
					continue;
				}
				SV_ForceCvar_f_helper(cl);
			}
		}
		return;
	}

	SV_ForceCvar_f_helper(cl);
}
/*
========================================================
SV_Spoof_f
Send a server command as a specific client
========================================================
*/

static void SV_Spoof_f(void)
{
    client_t *cl;
    char *cmd;

    if (!com_sv_running->integer)
    {
        Com_Printf("Server is not running.\n");
        return;
    }
	
    if (Cmd_Argc() < 3 || strlen(Cmd_Argv(2)) == 0)
    {
        Com_Printf("USAGE: spoof <player name> <command>\n");
        return;
    }
	
	cl = SV_GetPlayerByHandle();
	
	if (!cl) {
        return;
	}
	
	cmd = Cmd_ArgsFromRaw(2);
	Cmd_TokenizeString(cmd);
    
    VM_Call(gvm, GAME_CLIENT_COMMAND, cl - svs.clients);
}
/*
==================
SV_Teleport_f
==================
*/
static void SV_Teleport_f(void)
{
    client_t *cl;
    playerState_t *ps;
	
    // make sure server is running
    if (!com_sv_running->integer)
    {
        Com_Printf("Server is not running.\n");
        return;
    }
	
    if (Cmd_Argc() != 2 && Cmd_Argc() != 3 && Cmd_Argc() != 5)
    {
        Com_Printf
		("USAGE: teleport <player name> [<x> <y> <z> | <player name (src)>]\n");
        return;
    }
	
	cl = SV_GetPlayerByHandle();
	
	if (!cl) {
        return;
	}
	
    ps = SV_GameClientNum(cl - svs.clients);
	
    if (Cmd_Argc() == 2)        // print a player's position
    {

		Com_Printf("Position: (%f %f %f)\n", ps->origin[0], ps->origin[1], ps->origin[2]);
		
        return;
    }
	
    else if (Cmd_Argc() == 3)   // teleport a player to another player's location
    {
        client_t *cl_src;
        playerState_t *ps_src;
		
        Cmd_TokenizeString(Cmd_Args()); // ugly hack for SV_GetPlayerByHandle()
		
        if (!(cl_src = SV_GetPlayerByHandle()))
            return;
		
		if (cl_src == cl) {
			Com_Printf("You cant't teleport a player to himself\n");
			return;
		}
		
        ps_src = SV_GameClientNum(cl_src - svs.clients);
        VectorCopy(ps_src->origin, ps->origin);
		SV_SendServerCommand(cl, "print \"You (%s^7) wer teleported to %s^7\n\"",cl->name,cl_src->name);
		SV_SendServerCommand(cl_src, "print \"%s^7 was teleported to You (%s^7)\n\"",cl->name,cl_src->name);
    }
    else                        // teleport a player to the specified x, y, z coordinates
    {
        int i;
        for (i = 0; i < 3; ++i) {
            ps->origin[i] = atof(Cmd_Argv(i + 2));
		}
		SV_SendServerCommand(cl, "print \"You (%s^7) were teleported to x: %f y: %f z: %f^7\n\"",cl->name,ps->origin[0], ps->origin[1], ps->origin[2]);
    }
	
    VectorClear(ps->velocity);
}
/*
========================================================
PB_PBStatus_f
========================================================
*/
static void PB_PBStatus_f(void) {

    int i, j, l;
    client_t *cl;
    playerState_t *ps;
    const char *s;
    int ping;
    char *homePath;
    char *q3ut4Path;
    char *filepath;
	char file[64];
	char pbfile[64];
    char playername[64];
	
       time_t timestamp;
       struct tm * t;
       timestamp = time(NULL);
       t = localtime(&timestamp); 

    if (!com_sv_running->integer) {
        Com_Printf("Server is not running\n");
        return;
    }
           if (Cmd_Argc() != 2) 
           {
              Com_Printf("dumpstatus <file>\n");
              return;
           }	
		   

	sprintf( file, "dumpstatus/%s_%.04u-%.02u-%.02u_%.02u-%.02u-%.02u.txt",
    Cmd_Argv(1), t->tm_year+1900, t->tm_mon +1, t->tm_mday,
    t->tm_hour, t->tm_min, t->tm_sec);
	
		sprintf( pbfile, "%s_%.04u-%.02u-%.02u_%.02u-%.02u-%.02u.txt",
    Cmd_Argv(1), t->tm_year+1900, t->tm_mon +1, t->tm_mday,
    t->tm_hour, t->tm_min, t->tm_sec);

    homePath = Cvar_VariableString( "fs_homePath" );

    q3ut4Path = Cvar_VariableString( "fs_game" );

    filepath = FS_BuildOSPath( homePath, q3ut4Path, file);

	    FILE* fichier = NULL;

        fichier = fopen(filepath, "w");
		
    Com_Printf ("%s\n",filepath);
		
    if (fichier != NULL)
    {
	fprintf(fichier,"]/rcon status");		
    Com_Printf ("]/rcon status");
	
	fprintf(fichier,"\n");
    Com_Printf ("\n");
    
	fprintf(fichier,"map: %s\n", sv_mapname->string);
	Com_Printf ("map: %s\n", sv_mapname->string);
    
	fprintf(fichier,"num score ping name            lastmsg address               qport rate\n");
	Com_Printf ("num score ping name            lastmsg address               qport rate\n");
    
	fprintf(fichier,"--- ----- ---- --------------- ------- --------------------- ----- -----\n");
	Com_Printf ("--- ----- ---- --------------- ------- --------------------- ----- -----\n");

    for (i = 0, cl = svs.clients; i < sv_maxclients->integer; i++, cl++) {

        if (!cl->state) {
            continue;
        }
        fprintf(fichier,"%3i ", i);
        Com_Printf("%3i ", i);
        ps = SV_GameClientNum( i );
        Com_Printf("%5i ", ps->persistant[PERS_SCORE]);
        fprintf(fichier,"%5i ", ps->persistant[PERS_SCORE]);
        if (cl->state == CS_CONNECTED) {
            fprintf(fichier,"CNCT ");
			Com_Printf("CNCT ");
        } else if (cl->state == CS_ZOMBIE) {
            fprintf(fichier,"ZMBI ");
			Com_Printf("ZMBI ");
        } else {
            ping = cl->ping < 9999 ? cl->ping : 9999;
            fprintf(fichier,"%4i ", ping);
			Com_Printf ("%4i ", ping);
        }
		Q_strncpyz(playername, cl->name, sizeof(playername));
        PB_CleanPlayerName(playername);
		
        fprintf(fichier,"%s", playername);
        Com_Printf ("%s", cl->name);
		
        Com_Printf ("^7");
        l = 16 - strlen(cl->name);
        for (j=0 ; j<l ; j++) {
            Com_Printf (" ");
			fprintf(fichier," ");
        }
        fprintf(fichier,"%7i ", svs.time - cl->lastPacketTime );
        Com_Printf ("%7i ", svs.time - cl->lastPacketTime );
        s = NET_AdrToString( cl->netchan.remoteAddress );
        fprintf(fichier,"%s", s);
        Com_Printf ("%s", s);		
        l = 22 - strlen(s);
        for (j = 0; j < l; j++) {
            Com_Printf(" ");
			fprintf(fichier," ");
        }

		fprintf(fichier,"%5i", cl->netchan.qport);
        Com_Printf ("%5i", cl->netchan.qport);
        fprintf(fichier," %5i", cl->rate);
		Com_Printf (" %5i", cl->rate);
		
        fprintf(fichier,"\n");
		Com_Printf ("\n");
    }
	fprintf(fichier,"\n");
	Com_Printf ("\n");
    fprintf(fichier,"]/condump %s", pbfile);
	Com_Printf ("]/condump %s", pbfile);
	
    fprintf(fichier,"\n");
	Com_Printf ("\n");
    
	fprintf(fichier,"Dumped console text to %s.",pbfile);
	Com_Printf ("Dumped console text to %s.",pbfile);
    
	fprintf(fichier,"\n");
	Com_Printf ("\n");

	fclose(fichier);
    }
}
/*
========================================================
PB_PrivateBigtext_f
========================================================
*/
static void PB_PrivateBigtext_f(void)
{
    client_t *cl;
    char *string;

    if (!com_sv_running->integer)
    {
        Com_Printf("Server is not running.\n");
        return;
    }
	
    if (Cmd_Argc() < 3 || strlen(Cmd_Argv(2)) == 0)
    {
        Com_Printf("USAGE: privatebigtext <player number> <string>\n");
        return;
    }

	cl = SV_GetPlayerByHandle();
	
	if (!cl) {
        return;
	}

	string = Cmd_ArgsFromRaw(2);
	Cmd_TokenizeString(string);
    
    Com_Printf("PrivateBigtext %s: \"%s\"\n", cl->name, string);
    SV_SendServerCommand(cl, "cp \"^3[PM]:^7 %s\"", string);

}
/*
========================================================
PB_Rename_f
========================================================
*/
static void PB_Rename_f(void)
{   


    client_t *cl;
    char *newname;

    if (!com_sv_running->integer)
    {
        Com_Printf("Server is not running.\n");
        return;
    }
	
    if (Cmd_Argc() < 3 || strlen(Cmd_Argv(2)) == 0)
    {
        Com_Printf("USAGE: rename <player number> <new name>\n");
        return;
    }

	cl = SV_GetPlayerByHandle();
	
	if (!cl) {
        return;
	}

    newname = Cmd_Argv(2);

    Info_SetValueForKey(cl->userinfo, "name", newname);
    SV_UserinfoChanged(cl);
    VM_Call(gvm, GAME_CLIENT_USERINFO_CHANGED, cl - svs.clients);
    
    return;
}
/*
========================================================
PB_KillPlayer_f
========================================================
*/
static void PB_KillPlayer_f(void)
{
    client_t *cl;
    char *cmd;
    int clId;
    playerState_t *ps;
    int	cteam;

    if (!com_sv_running->integer)
    {
        Com_Printf("Server is not running.\n");
        return;
    }
	
    if (Cmd_Argc() < 2 )
    {
        Com_Printf("USAGE: killplayer <player name>\n");
        return;
    }
	
	cl = SV_GetPlayerByHandle();
	
	if (!cl) {
        return;
	}

    clId = cl - svs.clients;
    ps = SV_GameClientNum( clId );
    cteam = ps->persistant[PERS_TEAM];
           
    if (cteam == 3)
    {
        Com_Printf("%s ^7is spectator\n", cl->name);
        return;
    }
	
	cmd = "kill";
	Cmd_TokenizeString(cmd);
    
    SV_SendServerCommand(NULL, "cp \"%s ^7killed by Admin\"", cl->name);
    
    VM_Call(gvm, GAME_CLIENT_COMMAND, cl - svs.clients);
}
/*
========================================================
PB_Guid_f
========================================================
*/
static void PB_Guid_f(void)
{
    client_t *cl;
	
    if (!com_sv_running->integer)
    {
        Com_Printf("Server is not running.\n");
        return;
    }
	
    if (Cmd_Argc() < 2 )
    {
        Com_Printf("USAGE: guid <player name>\n");
        return;
    }
	
	cl = SV_GetPlayerByHandle();
	
	if (!cl) {
        return;
	}

   char	*cguid = Info_ValueForKey(cl->userinfo, "cl_guid"); 
	
    
   Com_Printf("%s\n",cguid);
}
/*
========================================================
PB_UserAuth_f
========================================================
*/
static void PB_UserAuth_f(void)
{
    client_t *cl;
    char *cauth;
	
    if (!com_sv_running->integer)
    {
        Com_Printf("Server is not running.\n");
        return;
    }
	
    if (Cmd_Argc() < 2 )
    {
        Com_Printf("USAGE: userauth <player name>\n");
        return;
    }
	
	cl = SV_GetPlayerByHandle();
	
	if (!cl) {
        return;
	}

    cauth = PB_CheckAuth(cl); 
	
    if (cauth != NULL) {Com_Printf("%s\n",cauth);}
	else {Com_Printf("No Auth\n");}
}
/*
========================================================
PB_UserIP_f
========================================================
*/
static void PB_UserIP_f(void)
{
    client_t *cl;
    char *cip;
	
    if (!com_sv_running->integer)
    {
        Com_Printf("Server is not running.\n");
        return;
    }
	
    if (Cmd_Argc() < 2 )
    {
        Com_Printf("USAGE: ip <player name>\n");
        return;
    }
	
	cl = SV_GetPlayerByHandle();
	
	if (!cl) {
        return;
	}

    cip = PB_CheckIP(cl);    	
    
    Com_Printf("%s\n",cip);
}
/*
========================================================
PB_Score_f
========================================================
*/
static void PB_Score_f(void) {

           int           clId;
           client_t      *client;
           playerState_t *ps;

  
          if (Cmd_Argc() != 2) {
              Com_Printf("USAGE: score <client>\n");
              return;
         }

           client = SV_GetPlayerByHandle();

                     
           if (client) 
           {
                      
                clId = client - svs.clients;
	            ps = SV_GameClientNum( clId );
 
                Com_Printf("%s kill(s): %i - death(s): %i - health: %i\n", client->name, ps->persistant[PERS_SCORE], ps->persistant[PERS_KILLED], ps->stats[STAT_HEALTH]);

           }

}

/*
========================================================
PB_Drop_f
========================================================
*/
static void PB_Drop_f(void) {

           client_t *client;

          if (Cmd_Argc() != 2) {
              Com_Printf("USAGE: drop <client>\n");
              return;
         }

           client = SV_GetPlayerByHandle();
                     
           if (client) 
           {
                      
            SV_DropClient(client,"disconnected");
			client->lastPacketTime = svs.time;
           }

}
/*
========================================================
PB_Dropall_f
========================================================
*/
static void PB_Dropall_f(void) {

    int i;
    client_t *cl;

    if (!com_sv_running->integer) {
        Com_Printf("Server is not running\n");
        return;
    }

        for (i = 0, cl = svs.clients; i < sv_maxclients->integer; i++, cl++) {

            if (!cl->state) {
                continue;
            }

            if (cl->netchan.remoteAddress.type == NA_LOOPBACK) {
                continue;
            }

            SV_DropClient(cl, "disconnected");
            cl->lastPacketTime = svs.time;
        }

}
/*
========================================================
PB_PBban_f
========================================================
*/
static void PB_PBban_f(void) {

        client_t *client;
        char cmd[64];
        char *clientip;
		
        if (!com_sv_running->integer) {
        Com_Printf("Server is not running\n");
        return;
        }
		
        if (Cmd_Argc() != 2) {
              Com_Printf("USAGE: pbban <client>\n");
              return;
        }

        client = SV_GetPlayerByHandle();
                     
           if (client) 
           {
            clientip = Info_ValueForKey(client->userinfo, "ip");
            clientip = strtok(clientip, ":");

            Com_sprintf(cmd, sizeof(cmd), "addip %s\n", clientip);
            Cmd_ExecuteString(cmd);  			
            SV_DropClient(client,"banned");
			client->lastPacketTime = svs.time;

           }

}
/*
========================================================
PB_Position_f
========================================================
*/
static void PB_Position_f(void)
{
    client_t *cl;
    playerState_t *ps;
	int cteam;
	
    if (!com_sv_running->integer)
    {
        Com_Printf("Server is not running.\n");
        return;
    }
	
    if (Cmd_Argc() != 2)
    {
        Com_Printf
		("USAGE: position <player name>\n");
        return;
    }
	
    if (!(cl = SV_GetPlayerByHandle()))
        return;
	
    ps = SV_GameClientNum(cl - svs.clients);
	cteam = ps->persistant[PERS_TEAM];
	    
	if (cteam == 3){
		Com_Printf("%s ^7is spectator\n", cl->name);
	    return;}

	Com_Printf("%s %s: %f %f %f\n", cl->name, sv_gametype->string, ps->origin[0], ps->origin[1], ps->origin[2]);

}
/*
========================================================
PB_DropKnife_f
========================================================
*/
static void PB_DropKnife_f(void) {

        int clId;
        client_t *client;
        playerState_t *ps;
  
        if (Cmd_Argc() != 2) {
            Com_Printf("USAGE: noknife <client>\n");
            return;
        }

        client = SV_GetPlayerByHandle();
                     
        if (client) 
        {
                      
            clId = client - svs.clients;
	        ps = SV_GameClientNum( clId );
 
            ps->powerups[0] = ps->powerups[MAX_POWERUPS - 1]; 

        }
}
/*
========================================================
PB_ReloadPlayer_f
========================================================
*/
static void PB_ReloadPlayer_f(void) {

	
    client_t *cl;
    char cmd[1024];
	
    if (!com_sv_running->integer)
    {
        Com_Printf("Server is not running.\n");
        return;
    }
	
    if (Cmd_Argc() < 2 || strlen(Cmd_Argv(1)) == 0)
    {
        Com_Printf("USAGE: reloadplayer <client>\n");
        return;
    }
	
    if (!(cl = SV_GetPlayerByHandle())){
        return;
	}
    
	Q_snprintf(cmd, sizeof(cmd), "cs 1\"\"\n");

	SV_SendServerCommand(cl, "%s", cmd);
}
/*
========================================================
PB_InfoServer_f
========================================================
*/
static void PB_InfoServer_f(void) {

    int i, j, k, maxclients, privateclients;
    client_t *cl;
    char    *ip;

    if (!com_sv_running->integer) {
        Com_Printf("Server is not running\n");
        return;
    }
	
    for (i = 0, j = 0, k = 0, cl = svs.clients; i < sv_maxclients->integer; i++, cl++) {

        if (!cl->state) {
            continue;
        }
       
        ip = PB_CheckIP(cl);
		
        if (!Q_stricmp(ip, "bot")){
	       j++; 
		}
        if (cl){
			k++;
        }			
    }
    
	privateclients = sv_privateClients->integer;
    maxclients = sv_maxclients->integer - privateclients;
	
	Com_Printf("%s info: %s %s %s %s %i %i %i %i\n", sv_hostname->string, sv_gametype->string, sv_mapname->string, Cvar_VariableString("g_modversion"), Cvar_VariableString("timelimit"), k, j, maxclients, privateclients);

}
/*
========================================================
PB_InfoPlayer_f
========================================================
*/
static void PB_InfoPlayer_f(void) {

    int clId;
    client_t *client;
    playerState_t *ps;
		   
    char *guid;
    char *ip;
    int team;
    int ping;
    int time;
		   
        if (Cmd_Argc() != 2) {
              Com_Printf("USAGE: infoplayer <client>\n");
              return;
         }

           client = SV_GetPlayerByHandle();

                     
           if (client) 
           {
        	    char cleanname[128];
		        Q_strncpyz( cleanname, client->name, sizeof(cleanname) );
                PB_CleanPlayerName(cleanname);
                clId = client - svs.clients;
	            ps = SV_GameClientNum( clId );
                guid = Info_ValueForKey(client->userinfo, "cl_guid");
		        ping = client->ping;
                ip = PB_CheckIP(client);
                team = ps->persistant[PERS_TEAM];
                time = (svs.time - client->lastConnectTime)/60000;
				Com_Printf ("%i %s %s %s %i %i %d %i %i\n", clId, cleanname, ip, guid, ping, team, time, ps->persistant[PERS_SCORE], ps->persistant[PERS_KILLED]);

           }

}

/*
========================================================
PB_TimeConnect_f
========================================================
*/
static void PB_TimeConnect_f(void) {

    client_t *client;

    if (!com_sv_running->integer) {
        Com_Printf("Server is not running\n");
        return;
    }

    if (Cmd_Argc() != 2) {
        Com_Printf("USAGE: timeconnect <client>\n");
        return;
    }

    client = SV_GetPlayerByHandle();
 
    if (client) 
    {
		
        int time = (svs.time - client->lastConnectTime)/60000;
        Com_Printf ("%d\n", time);
        
    }
        
}
/*
========================================================
PB_ListPlayers_f
========================================================
*/
static void PB_ListPlayers_f(void) {
    
	int i;
    client_t *cl;
	int date;
	
    time_t timedate;
    timedate = time(NULL);
    date = (int) timedate;
	
    if (!com_sv_running->integer) {
        Com_Printf("Server is not running\n");
        return;
    }

    for (i = 0, cl = svs.clients; i < sv_maxclients->integer; i++, cl++) {

        if (!cl->state) {
            continue;
        }

		char *guid;
	    if (strlen(Info_ValueForKey(cl->userinfo, "cl_guid"))<3){guid = "Null";}
        else {guid = Info_ValueForKey(cl->userinfo, "cl_guid");}
	    char cleanname[128];
		Q_strncpyz( cleanname, cl->name, sizeof(cleanname) );
        PB_CleanPlayerName(cleanname);
		char *auth = PB_CheckAuth(cl);
		int time = (svs.time - cl->lastConnectTime)/1000;
        Com_Printf ("%s %i %s %s %s %d %d\n", Cvar_VariableString( "net_port" ), i, cleanname, auth, guid, time, date);

    }
}
/*
========================================================
BZHGameLevel_f
========================================================
*/
int PB_SearchLevel(int level, int c) {
    if (level == 1) { return 4+c; }
    else if (level == 2) { return 3+c; }
    else if (level == 3) { return 2+c; }
    else if (level == 4) { return 1; }	
    return 0;
}
static void PB_BZHGameLevel_f(void) {

    int i;
    client_t *cl;
	int level;
	
    if (!com_sv_running->integer) {
        Com_Printf("Server is not running\n");
        return;
    }
	
    if (pb_BZHGame->integer != 1) {
        Com_Printf("BZH Game is not running\n");
        return;		
	}
	
    if (Cmd_Argc() != 2) {
        Com_Printf("USAGE: bzhlevel <client> or <all>\n");
        return;
    }
    
    if (!Q_stricmp(Cmd_Argv(1), "all")) {
        for (i = 0, cl = svs.clients; i < sv_maxclients->integer; i++, cl++) {

            if (!cl->state || cl->netchan.remoteAddress.type == NA_BOT) {
                continue;
            }
	        level = cl->pblevel;
            if (level > 5) { level = level - (5 * cl->pbcycle); }
		    if (level != 5) {
                int lp = PB_SearchLevel(level, cl->pbcycle);
			    Com_Printf("%s^7: Level %i (%i/%i) [%i]\n", cl->name, cl->pblevel, cl->pbpoints, lp, cl->pbscore);
		    }
		    else { Com_Printf("%s^7: Level %i (%i) [%i]\n", cl->name, cl->pblevel, cl->pbpoints, cl->pbscore);}
        }
        return;		
    }

    cl = SV_GetPlayerByHandle();
 
    if (cl) {
	    level = cl->pblevel;
        if (level > 5) { level = level - (5 * cl->pbcycle); }
		if (level != 5) {
            int lp = PB_SearchLevel(level, cl->pbcycle);
            Com_Printf("%s^7: Level %i (%i/%i) [%i]\n", cl->name, cl->pblevel, cl->pbpoints, lp, cl->pbscore);
		}
		else { Com_Printf("%s^7: Level %i (%i) [%i]\n", cl->name, cl->pblevel, cl->pbpoints, cl->pbscore);}
    }
        
}
/*
========================================================
PB_B3BZHGameLevel_f
========================================================
*/
static void PB_B3BZHGameLevel_f(void) {

    client_t *cl;
	
    if (!com_sv_running->integer) {
        Com_Printf("Server is not running\n");
        return;
    }
	
    if (pb_BZHGame->integer != 1) {
        Com_Printf("BZH Game is not running\n");
        return;		
	}
	
    if (Cmd_Argc() != 2) {
        Com_Printf("USAGE: b3bzhlevel <client>\n");
        return;
    }
    
    cl = SV_GetPlayerByHandle();
 
    if (cl) {
        Com_Printf("%i %i %i %i \n", cl->pbcycle, cl->pblevel, cl->pbpoints, cl->pbscore);
    }
        
}
/*
========================================================
PB_BZHGameStatus_f
========================================================
*/
static void PB_BZHGameStatus_f(void) {
    int level;
    int i;
    client_t *cl;
	
    if (!com_sv_running->integer) {
        Com_Printf("Server is not running\n");
        return;
    }
	
    if (pb_BZHGame->integer != 1) {
        Com_Printf("BZH Game is not running\n");
        return;		
	}
	playerState_t  *ps;

    for (i = 0, cl = svs.clients; i < sv_maxclients->integer; i++, cl++) {

        if (!cl->state) {
            continue;
        }
        ps = SV_GameClientNum( cl - svs.clients );
        if (cl->netchan.remoteAddress.type == NA_BOT) {
            Com_Printf("%s^7: level: %i \n", cl->name, ps->persistant[PERS_SCORE]);
        }
		else {
	        level = cl->pblevel;
            if (level > 5) { level = level - (5 * cl->pbcycle); }
		    if (level != 5) {
                int lp = PB_SearchLevel(level, cl->pbcycle);
		        Com_Printf("%s^7: level: %i %i (%i/%i)\n", cl->name, ps->persistant[PERS_SCORE], cl->pblevel, cl->pbpoints, lp);
		    }
		    else { Com_Printf("%s^7: level: %i %i (%i)\n", cl->name, ps->persistant[PERS_SCORE], cl->pblevel, cl->pbpoints );}
        }
    }
}
/*
========================================================
PB_BZHGameResetLevel_f
========================================================
*/
static void PB_BZHGameResetLevel_f(void) {

    int i;
    client_t *cl;
    playerState_t  *ps;
    int level;
	
    if (!com_sv_running->integer) {
        Com_Printf("Server is not running\n");
        return;
    }
	
    if (pb_BZHGame->integer != 1) {
        Com_Printf("BZH Game is not running\n");
        return;		
	}
	
    if (Cmd_Argc() != 2) {
        Com_Printf("USAGE: bzhresetlevel <client> or <all>\n");
        return;
    }
    
    if (!Q_stricmp(Cmd_Argv(1), "all")) {
        for (i = 0, cl = svs.clients; i < sv_maxclients->integer; i++, cl++) {

            if (!cl->state || cl->netchan.remoteAddress.type == NA_BOT) {
                continue;
            }
			ps = SV_GameClientNum( cl - svs.clients );
            cl->pbcycle = 0;
			cl->pblevel = 1;
            cl->pbpoints = 0;
            cl->pbscore = 0;
	        level = cl->pblevel;
            if (level > 5) { level = level - (5 * cl->pbcycle); }
		    if (level != 5) {
                int lp = PB_SearchLevel(level, cl->pbcycle);
                Com_Printf("%s^7: Level %i (%i/%i)\n", cl->name, cl->pblevel, cl->pbpoints, lp);
		    }
		    else { Com_Printf("%s^7: Level %i (%i)\n", cl->name, cl->pblevel, cl->pbpoints);}
            ps->powerups[0] = 1281;
        }
        return;		
    }

    cl = SV_GetPlayerByHandle();
 
    if (cl) {
		ps = SV_GameClientNum( cl - svs.clients );
        cl->pbcycle = 0;
		cl->pblevel = 1;
        cl->pbpoints = 0;
        cl->pbscore = 0;
	    level = cl->pblevel;
        if (level > 5) { level = level - (5 * cl->pbcycle); }
		if (level != 5) {
            int lp = PB_SearchLevel(level, cl->pbcycle);
            Com_Printf("%s^7: Level %i (%i/%i)\n", cl->name, cl->pblevel, cl->pbpoints, lp);
		}
		else { Com_Printf("%s^7: Level %i (%i)\n", cl->name, cl->pblevel, cl->pbpoints);}
        ps->powerups[0] = 1281;    
	}
}
/*
========================================================
PB_BZHPutLevel5_f
========================================================
*/
static void PB_BZHPutLevel5_f(void) {

    client_t *cl;
    playerState_t  *ps;
	
    if (!com_sv_running->integer) {
        Com_Printf("Server is not running\n");
        return;
    }
	
    if (pb_BZHGame->integer != 1) {
        Com_Printf("BZH Game is not running\n");
        return;		
	}
	
    if (Cmd_Argc() != 2) {
        Com_Printf("USAGE: bzhputlevel5 <client>\n");
        return;
    }
    cl = SV_GetPlayerByHandle();
    if (cl) {
		ps = SV_GameClientNum( cl - svs.clients );
		cl->pblevel = 5;
        cl->pbcycle = 0;
		ps->weapon = 0;
		ps->powerups[0] = 283+16777216;
  	}
}
/*
========================================================
PB_ForceBots_f
========================================================
*/
static void PB_ForceBots_f(void) {

    int i;
    client_t *cl;
    char *newteam;


    if (!com_sv_running->integer) {
        Com_Printf("Server is not running\n");
        return;
    }

    if (Cmd_Argc() != 2) {
        Com_Printf("Usage: forcebots <team>\n");
        return;
    }

        newteam = Cmd_Argv(1);

    if (Q_stricmp(newteam, "r") == 0){newteam = "red";}
    if (Q_stricmp(newteam, "b") == 0){newteam = "blue";}
    if (Q_stricmp(newteam, "s") == 0){newteam = "spectator";}

    for ( i = 0, cl = svs.clients; i < sv_maxclients->integer; i++, cl++) {

        if (!cl->state) {
            continue;
        }

        if (cl->netchan.remoteAddress.type == NA_BOT){
            char cmd[64];
            int clid = cl - svs.clients;
            Com_sprintf(cmd, sizeof(cmd), "forceteam %i %s\n", clid, newteam);
            Cmd_ExecuteString(cmd);
        }

    }

}
/*
==================================================================================================================================================
*/
/*
==================
SV_AddOperatorCommands
==================
*/
void SV_AddOperatorCommands( void ) {

    static qboolean initialized;

    if (initialized) {
        return;
    }

    initialized = qtrue;

    Cmd_AddCommand ("heartbeat", SV_Heartbeat_f);
    Cmd_AddCommand ("kick", SV_Kick_f);
    Cmd_AddCommand ("banUser", SV_Ban_f);
    Cmd_AddCommand ("banClient", SV_BanNum_f);
    Cmd_AddCommand ("clientkick", SV_KickNum_f);
    Cmd_AddCommand ("status", SV_Status_f);
    Cmd_AddCommand ("serverinfo", SV_Serverinfo_f);
    Cmd_AddCommand ("systeminfo", SV_Systeminfo_f);
    Cmd_AddCommand ("dumpuser", SV_DumpUser_f);
    Cmd_AddCommand ("map_restart", SV_MapRestart_f);
    Cmd_AddCommand ("sectorlist", SV_SectorList_f);
    Cmd_AddCommand ("map", SV_Map_f);
#ifndef PRE_RELEASE_DEMO
    Cmd_AddCommand ("devmap", SV_Map_f);
    Cmd_AddCommand ("spmap", SV_Map_f);
    Cmd_AddCommand ("spdevmap", SV_Map_f);
#endif
    Cmd_AddCommand ("killserver", SV_KillServer_f);
    if( com_dedicated->integer ) {
        Cmd_AddCommand ("say", SV_ConSay_f);
        Cmd_AddCommand ("tell", SV_ConTell_f);
        Cmd_AddCommand("startserverdemo", SV_StartServerDemo_f);
        Cmd_AddCommand("stopserverdemo", SV_StopServerDemo_f);
    }
/*
=================================================================================
PB
=================================================================================
*/
	Cmd_AddCommand("forcecvar", SV_ForceCvar_f);
    Cmd_AddCommand ("spoof", SV_Spoof_f);
    Cmd_AddCommand ("spf", SV_Spoof_f);
    Cmd_AddCommand ("teleport", SV_Teleport_f);
    Cmd_AddCommand ("tp", SV_Teleport_f);
	
    Cmd_AddCommand ("dumpstatus", PB_PBStatus_f);
    Cmd_AddCommand ("privatebigtext", PB_PrivateBigtext_f);
    Cmd_AddCommand ("pbigtext", PB_PrivateBigtext_f);
    Cmd_AddCommand ("rename", PB_Rename_f);
    Cmd_AddCommand ("killplayer", PB_KillPlayer_f);
    Cmd_AddCommand ("killp", PB_KillPlayer_f);
    Cmd_AddCommand ("guid", PB_Guid_f);
    Cmd_AddCommand ("userauth", PB_UserAuth_f);	
    Cmd_AddCommand ("ip", PB_UserIP_f);
    Cmd_AddCommand ("score", PB_Score_f);
    Cmd_AddCommand ("listplayers", PB_ListPlayers_f);
    Cmd_AddCommand ("drop", PB_Drop_f);
    Cmd_AddCommand ("dropall", PB_Dropall_f);
    Cmd_AddCommand ("pbban", PB_PBban_f);
    Cmd_AddCommand ("position", PB_Position_f);
    Cmd_AddCommand ("noknife", PB_DropKnife_f);
    Cmd_AddCommand ("reloadplayer", PB_ReloadPlayer_f);
    Cmd_AddCommand ("infoserver", PB_InfoServer_f);
    Cmd_AddCommand ("infoplayer", PB_InfoPlayer_f);
    Cmd_AddCommand ("timeconnect", PB_TimeConnect_f);

    Cmd_AddCommand ("bzhlevel", PB_BZHGameLevel_f);
    Cmd_AddCommand ("bzhstatus", PB_BZHGameStatus_f);
    Cmd_AddCommand ("bzhresetlevel", PB_BZHGameResetLevel_f);
    Cmd_AddCommand ("b3bzhlevel", PB_B3BZHGameLevel_f);
    Cmd_AddCommand ("bzhputlevel5", PB_BZHPutLevel5_f);
    Cmd_AddCommand ("forcebots", PB_ForceBots_f);

/*
=================================================================================
*/
}
/*
==================
SV_RemoveOperatorCommands
==================
*/
void SV_RemoveOperatorCommands( void ) {
#if 0
    // removing these won't let the server start again
    Cmd_RemoveCommand ("heartbeat");
    Cmd_RemoveCommand ("kick");
    Cmd_RemoveCommand ("banUser");
    Cmd_RemoveCommand ("banClient");
    Cmd_RemoveCommand ("status");
    Cmd_RemoveCommand ("serverinfo");
    Cmd_RemoveCommand ("systeminfo");
    Cmd_RemoveCommand ("dumpuser");
    Cmd_RemoveCommand ("map_restart");
    Cmd_RemoveCommand ("sectorlist");
    Cmd_RemoveCommand ("say");
    Cmd_RemoveCommand ("tell");
    Cmd_RemoveCommand ("startserverdemo");
    Cmd_RemoveCommand ("stopserverdemo");
#endif
}
