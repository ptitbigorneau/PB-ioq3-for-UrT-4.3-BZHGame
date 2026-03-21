/*
===============================================================================================================================================================
PB BZHGame
Author      : PtitBigorneau
===============================================================================================================================================================
*/

#include "server.h"
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>

static int Lweapons[28][5] = {
{ 0, 0, 0, 0, 0 },
{ 12, 1281, 0, 5, 0 }, // 1 ut_weapon_knife
{ 14, 3842, 2, 15, 3 }, // 2 ut_weapon_beretta
{ 15, 1795, 2, 7, 3 }, // 3 ut_weapon_deagle
{ 16, 2052, 24, 8, 2 }, // 4 ut_weapon_spas12
{ 18, 73221, 2, 30, 2 }, // 5 ut_weapon_mp5k
{ 17, 73222, 2, 30, 2 }, // 6 ut_weapon_ump45
{ 22, 263, 3, 1, 1 }, // 7 ut_weapon_hk69
{ 19, 138760, 2, 30, 1 }, // 8 ut_weapon_lr
{ 20, 138761, 2, 30, 1 }, // 9 ut_weapon_g36
{ 21, 2058, 3, 8, 1 }, // 10 ut_weapon_psg1
{ 25, 523, 0, 2, 0 }, // 11 ut_weapon_grenade_he
{ 26, 524, 0, 2, 0 }, // 12 ut_weapon_flash
{ 27, 525, 0, 2, 0 }, // 13 ut_weapon_smoke
{ 28, 1294, 3, 5, 1 }, // 14 ut_weapon_sr8
{ 30, 138767, 2, 30, 1 }, // 15 ut_weapon_ak103
{ 34, 272, 0, 1, 0 }, // 16 ut_weapon_bomb
{ 36, 23057, 1, 90, 1 }, // 17 ut_weapon_negev
{ 0, 0, 0, 0, 0 }, //  null
{ 38, 138771, 2, 30, 1 }, // 19 ut_weapon_m4
{ 39, 68628, 2, 12, 3 }, // 20 ut_weapon_glock
{ 40, 2581, 2, 10, 3 }, // 21 ut_weapon_magnum
{ 41, 8214, 2, 30, 2 }, // 22 ut_weapon_mac11
{ 42, 1559, 3, 6, 1 }, // 23 ut_weapon_frf1
{ 43, 1304, 15, 5, 2 }, // 24 ut_weapon_benelli
{ 44, 12825, 2, 50, 2 }, // 25 ut_weapon_p90
{ 45, 1562, 3, 6, 3 }, // 26 ut_weapon_colt1911
{ 46, 283, 1, 1, 0 }  // 27 ut_weapon_tod50
};
extern int Lweapons[28][5];

/*
===============================================================================
*/
///////////////////////////////////////////////////////////
//PB_Replace_str
//////////////////////////////////////////////////////////
static char *PB_Replace_str(const char *s, const char *str1, const char *str2) {

    char *result;
    int i, cnt = 0;
    int nstr2 = strlen(str2);
    int nstr1 = strlen(str1);

    for (i = 0; s[i] != '\0'; i++) {
        if (strstr(&s[i], str1) == &s[i]) {
            cnt++;
             i += nstr1 - 1;
        }
    }

    result = (char *)malloc(i + cnt * (nstr2 - nstr1) + 1);
    i = 0;

    while (*s){
        if (strstr(s, str1) == s) {
            strcpy(&result[i], str2);
            i += nstr2;
            s += nstr1;
        }
        else {
            result[i++] = *s++;
        }
    }

    result[i] = '\0';
    return result;
}
//////////////////////////
//PB_Random
//////////////////////////
int PB_Random(int i, int id) {

    int nombre = 0;
    srand(time(NULL)+id);
    nombre = rand() % (i);
    return nombre;
}
//////////////////////////
//PB_SearchUser
//////////////////////////
client_t *PB_SearchUser(int cid) {

client_t *cl;
int j;

for (j = 0, cl = svs.clients; j < sv_maxclients->integer; j++, cl++) {
        if (!cl->state) { continue; }
        if (cid == j) { return cl; break;}
    }
    return NULL;
}
///////////////////////////////////////////////////////////
//PB_SearchIDWeapon
//////////////////////////////////////////////////////////
int PB_SearchIDWeapon(int powerups, int option) {

    int chargeurs = 0;
    int ammo = 0;
    int idweap = 0;

    if (powerups > 16777216) {
        chargeurs = powerups/16777216;
        ammo = (powerups -(chargeurs*16777216))/256;
        if (ammo >= 256) {
            powerups = powerups - 65536;
            chargeurs = powerups/16777216;
            ammo = (powerups -(chargeurs*16777216))/256;
               if (ammo >= 256) {
                powerups = powerups - 65536;
                chargeurs = powerups/16777216;
                ammo = (powerups -(chargeurs*16777216))/256;
            }
        }

        idweap = powerups - ((chargeurs*16777216)+(ammo*256));
    }
    else {
        ammo = powerups/256;
        if (ammo >= 256) {
            powerups = powerups - 65536;
            ammo = powerups/256;
        }
        idweap = powerups - (ammo*256);
        chargeurs = 0;
    }

    if (option == 1) { return idweap; }
    if (option == 2) { return chargeurs; }
    if (option == 3) { return ammo; }

    return 0;

}
//////////////////////////
// PB_TestMapcycle
//////////////////////////
static int PB_TestMapcycle(const char *mapcycle){
    FILE* fichiern = NULL;
    fichiern = fopen(mapcycle, "r");

    if (fichiern != NULL) {
        return 1;
        fclose(fichiern);

    }
    else {return 0;}

}
//////////////////////////
// PB_Searchnextmap
//////////////////////////
static char *PB_Searchnextmap(void){

        char *gnextmapname;
        int testmc;
        gnextmapname = Cvar_VariableString( "g_nextmap" );

        if ((!Q_stricmp(gnextmapname, "") == 0) && (SV_GetMapSoundingLike(gnextmapname) !=NULL)){
            return SV_GetMapSoundingLike(gnextmapname);
        }
        else{

            FILE* mapsfile = NULL;
            char lmaps[128];

            char *mhomePath;
            char *mbasePath;
            char *mq3ut4Path;
            char *mapcyclefile;
            char *mapcyclefile2;
            char *filemapcycle = Cvar_VariableString( "g_mapcycle" );

            char *testb = NULL;
            int n = 0;
            char static firstmap[128];
            char *test = NULL;
            char *test2 = NULL;

            mhomePath = Cvar_VariableString( "fs_homePath" );

            mbasePath = Cvar_VariableString( "fs_basepath" );

            mq3ut4Path = Cvar_VariableString( "fs_game" );

            mapcyclefile = FS_BuildOSPath( mhomePath, mq3ut4Path , filemapcycle);
            mapcyclefile2 = FS_BuildOSPath( mbasePath, "q3ut4" , filemapcycle);

            testmc = PB_TestMapcycle(mapcyclefile);

            if (testmc == 1){mapsfile = fopen(mapcyclefile, "r");}
            else {mapsfile = fopen(mapcyclefile2, "r");}

            if (mapsfile != NULL) {
                while (fgets(lmaps, sizeof(lmaps), mapsfile) != NULL) {
                    Cmd_TokenizeString( lmaps );

                    if ((n == 0)&&(strcmp(Cmd_Argv(0),"") != 0)){
                        n++;
                        strcpy(firstmap,Cmd_Argv(0));

                    }

                    if (Q_stricmp(Cmd_Argv(0), sv_mapname->string)==0){

                        test = "ok";
                        continue;
                    }
                    if (test != NULL){

                        if (testb == NULL){

                            if (strpbrk(Cmd_Argv(0), "{") != NULL){
                                testb = "ok";
                                continue;}

                            if (strcmp(Cmd_Argv(0), "") == 0){continue;}
                            else{

                                return Cmd_Argv(0);
                                test2 = "ok";
                                break;
                            }

                        }

                        if (testb != NULL){
                            if (strpbrk(Cmd_Argv(0), "}") != NULL){testb = NULL;
                                continue;}
                        }
                    }
                }

                fclose(mapsfile);

            }

            if ((test == NULL)||(test2 == NULL)||(strcmp(Cmd_Argv(0), "") == 0)||strlen(Cmd_Argv(0))< 3){

                if (strlen(firstmap) < 3){
                    return Cvar_VariableString( "g_nextCycleMap" );
                }
                else {
                     return firstmap;
                }

            }

        }
    return NULL;
}
///////////////////////////////////////////////////////////
//PB_RechargeWeapons
//////////////////////////////////////////////////////////
static void PB_RechargeWeapons( client_t *cl, int weap ) {

    int i;
    int idweap;
    int nammo;
    int nchargeur;

    playerState_t  *ps = SV_GameClientNum( cl - svs.clients );
    char *gear =  Info_ValueForKey(cl->userinfo, "gear");
    int extra = 2;

    if (strstr(gear, "X") != NULL) {extra = 3;}

    for (i = 0; i < MAX_POWERUPS; i++) {

        idweap = PB_SearchIDWeapon(ps->powerups[i], 1);
        nchargeur = PB_SearchIDWeapon(ps->powerups[i], 2);
        nammo = PB_SearchIDWeapon(ps->powerups[i], 3);

        if (weap != idweap && weap != -1) { continue; }

        int DeltaAmmo = Lweapons[idweap][3] - nammo;
        int DeltaChargeurs = (extra*Lweapons[idweap][2]) - nchargeur;

        if (idweap == 1 ) {
            ps->powerups[i] = ps->powerups[i] + (DeltaAmmo*256);
        }

        else if (idweap == 12 || idweap == 13 || idweap == 11 ) {
            ps->powerups[i] = idweap + 512;
        }
        else if (idweap == 16) {
            ps->powerups[i] = ps->powerups[i];
        }
        else if (idweap == 27) {
            ps->powerups[i] = 283+16777216;
        }
        else {
            ps->powerups[i] = ps->powerups[i] + (DeltaChargeurs*16777216) + (DeltaAmmo*256);
        }
    }
}
//////////////////////////
// PB_SearchWeapon
//////////////////////////
char *PB_SearchWeapon(int id) {

    if ( id == 12 ) {
        return "Knife";
    }
    else if ( id == 13 ) {
        return "Knife Thrown";
    }
    else if ( id == 14 ) {
        return "Beretta 92G";
    }
    else if ( id == 15 ) {
        return ".50 Desert Eagle";
    }
    else if ( id == 16 ) {
        return "Franchi Spas-12";
    }
    else if ( id == 17 ) {
        return "H&K Ump45";
    }
    else if ( id == 18 ) {
        return "H&K Mp5k";
    }
    else if ( id == 19 ) {
        return "ZM LR300 ML";
    }
    else if ( id == 20 ) {
        return "H&K G36";
    }
    else if ( id == 21 ) {
        return "H&K PSG-1";
    }
    else if ( id == 22 ) {
        return "H&K 69";
    }
    else if ( id == 23 ) {
        return "Bled";
    }
    else if ( id == 24 ) {
        return "shot of Boots";
    }
    else if ( id == 25 ) {
        return "HE Grenade";
    }
    else if ( id == 28 ) {
        return "Remington SR8";
    }
    else if ( id == 30 ) {
        return "Kalashnikov AK103";
    }
    else if ( id == 31 ) {
        return "Sploded";
    }
    else if ( id == 32 ) {
        return "Slapped";
    }
    else if ( id == 33 ) {
        return "Smited";
    }
    else if ( id == 34 ) {
        return "Bombed";
    }
    else if ( id == 35 ) {
        return "Nuked";
    }
    else if ( id == 36 ) {
        return "IMI Negev";
    }
    else if ( id == 37 ) {
        return "H&K 69";
    }
    else if ( id == 38 ) {
        return "M4A1";
    }
    else if ( id == 39 ) {
        return "Glock 18";
    }
    else if ( id == 40 ) {
        return "Colt 1911";
    }
    else if ( id == 41 ) {
        return "Ingram MAC-11";
    }
    else if ( id == 42 ) {
        return "FR-F1";
    }
    else if ( id == 43 ) {
        return "Benelli M4 Super 90";
    }
    else if ( id == 44 ) {
        return "FN Hesrstal P90";
    }
    else if ( id == 45 ) {
        return ".44 Magnum Revolver";
    }
    else if ( id == 46 ) {
        return "TOD-50";
    }
    else if ( id == 47 ) {
        return "Flag";
    }
    else if ( id == 48 ) {
        return "Goomba";
    }
    else {return "Unknown Weapon";}

}
//////////////////////////
// PB_SearchHitWeapon
//////////////////////////
char *PB_SearchHitWeapon(int id) {

    if ( id == 1 ) {
        return "Knife";
    }
    else if ( id == 2 ) {
        return "Beretta 92G";
    }
    else if ( id == 3 ) {
        return ".50 Desert Eagle";
    }
    else if ( id == 4 ) {
        return "Franchi Spas-12";
    }
    else if ( id == 5 ) {
        return "H&K Ump45";
    }
    else if ( id == 6 ) {
        return "H&K Mp5k";
    }
    else if ( id == 8 ) {
        return "ZM LR300 ML";
    }
    else if ( id == 9 ) {
        return "H&K G36";
    }
    else if ( id == 10 ) {
        return "H&K PSG-1";
    }
    else if ( id == 11 ) {
        return "HE Grenade";
    }
    else if ( id == 14 ) {
        return "Remington SR8";
    }
    else if ( id == 15 ) {
        return "Kalashnikov AK103";
    }
    else if ( id == 17 ) {
        return "IMI Negev";
    }
    else if ( id == 19 ) {
        return "M4A1";
    }
    else if ( id == 20 ) {
        return "Glock 18";
    }
    else if ( id == 21 ) {
        return "Colt 1911";
    }
    else if ( id == 22 ) {
        return "Ingram MAC-11";
    }
    else if ( id == 23 ) {
        return "FR-F1";
    }
    else if ( id == 24 ) {
        return "Benelli M4 Super 90";
    }
    else if ( id == 25 ) {
        return "FN Hesrstal P90";
    }
    else if ( id == 26 ) {
        return ".44 Magnum Revolver";
    }
    else if ( id == 27 ) {
        return "TOD-50";
    }
    else if ( id == 29 ) {
        return "shot of Boots";
    }
    else if ( id == 30 ) {
        return "Knife Thrown";
    }
    else if ( id == 7 ) {
        return "H&K 69";
    }
    else {return NULL;}

}
///////////////////////////////////////////////////////////
//PB_SearchIDUTWeapons
//////////////////////////////////////////////////////////
int PB_SearchIDUTWeapons(char *utweapon) {

    if ( Q_stricmp(utweapon, "ut_weapon_knife") == 0 ) { return 1; }
    else if ( Q_stricmp(utweapon, "ut_weapon_beretta") == 0 ) { return 2; }
    else if ( Q_stricmp(utweapon, "ut_weapon_deagle") == 0 ) { return 3; }
    else if ( Q_stricmp(utweapon, "ut_weapon_spas12") == 0 ) { return 4; }
    else if ( Q_stricmp(utweapon, "ut_weapon_mp5k") == 0 ) { return 5; }
    else if ( Q_stricmp(utweapon, "ut_weapon_ump45") == 0 ) { return 6; }
    else if ( Q_stricmp(utweapon, "ut_weapon_hk69") == 0 ) { return 7; }
    else if ( Q_stricmp(utweapon, "ut_weapon_lr") == 0 ) { return 8; }
    else if ( Q_stricmp(utweapon, "ut_weapon_g36") == 0 ) { return 9; }
    else if ( Q_stricmp(utweapon, "ut_weapon_psg1") == 0 ) { return 10; }
    else if ( Q_stricmp(utweapon, "ut_weapon_grenade_he") == 0 ) { return 11; }
    else if ( Q_stricmp(utweapon, "ut_weapon_flash") == 0 ) { return 12; }
    else if ( Q_stricmp(utweapon, "ut_weapon_smoke") == 0 ) { return 13; }
    else if ( Q_stricmp(utweapon, "ut_weapon_sr8") == 0 ) { return 14; }
    else if ( Q_stricmp(utweapon, "ut_weapon_ak103") == 0 ) { return 15; }
    else if ( Q_stricmp(utweapon, "ut_weapon_bomb") == 0 ) { return 16; }
    else if ( Q_stricmp(utweapon, "ut_weapon_negev") == 0 ) { return 17; }
    else if ( Q_stricmp(utweapon, "ut_weapon_m4") == 0 ) { return 19; }
    else if ( Q_stricmp(utweapon, "ut_weapon_glock") == 0 ) { return 20; }
    else if ( Q_stricmp(utweapon, "ut_weapon_magnum") == 0 ) { return 21; }
    else if ( Q_stricmp(utweapon, "ut_weapon_mac11") == 0 ) { return 22; }
    else if ( Q_stricmp(utweapon, "ut_weapon_frf1") == 0 ) { return 23; }
    else if ( Q_stricmp(utweapon, "ut_weapon_benelli") == 0 ) { return 24; }
    else if ( Q_stricmp(utweapon, "ut_weapon_p90") == 0 ) { return 25; }
    else if ( Q_stricmp(utweapon, "ut_weapon_colt1911") == 0 ) { return 26; }
    else if ( Q_stricmp(utweapon, "ut_weapon_tod50") == 0 ) { return 27; }
    else { return 0; }
}
//////////////////////////
// PB_SearchHitLocation
//////////////////////////
char *PB_SearchHitLocation(int id) {

    if ( id == 1 ) {
        return "Head";
    }
    else if ( id == 2 ) {
        return "Helmet";
    }
    else if ( id == 3 ) {
        return "Torso";
    }
    else if ( id == 4 ) {
        return "Vest";
    }
    else if ( id == 5 ) {
        return "Left Arm";
    }
    else if ( id == 6 ) {
        return "Right Arm";
    }
    else if ( id == 7 ) {
        return "Groin";
    }
    else if ( id == 8 ) {
        return "Butt";
    }
    else if ( id == 9 ) {
        return "Left Upper Leg";
    }
    else if ( id == 10 ) {
        return "Right Upper Leg";
    }
    else if ( id == 11 ) {
        return "Left Lower Leg";
    }
    else if ( id == 12 ) {
        return "Right Lower Leg";
    }
    else if ( id == 13 ) {
        return "Left Foot";
    }
    else if ( id == 14 ) {
        return "Right Foot";
    }
    else {return "Unknown";}

}
//////////////////////////
// PB_BotFemaleName
//////////////////////////
char *PB_BotFemaleName(int id) {

    if ( id == 0 ) {
        return "^6HitGirl";
    }
    else if ( id == 2 ) {
        return "^6Emily";
    }
    else if ( id == 4 ) {
        return "^6Zoe";
    }
    else if ( id == 6 ) {
        return "^6Jennifer";
    }
    else if ( id == 8 ) {
        return "^6Angel";
    }
    else if ( id == 10 ) {
        return "^6Melody";
    }
    else if ( id == 12 ) {
        return "^6Penny";
    }
    else if ( id == 14 ) {
        return "^6Sherlyn";
    }
    else if ( id == 16 ) {
        return "^6Lize";
    }
    else {return "^6Kath";}

}
//////////////////////////
// PB_BotArmbandColor
//////////////////////////
char *PB_BotArmbandColor(int id) {

    if ( id == 0 ) {
        return "128,128,0";
    }
    else if ( id == 1 ) {
        return "255,102,0";
    }
    else if ( id == 2 ) {
        return "255,255,0";
    }
    else if ( id == 3 ) {
        return "255,0,0";
    }
    else if ( id == 4 ) {
        return "255,0,255";
    }
    else if ( id == 5 ) {
        return "0,255,0";
    }
    else if ( id == 6 ) {
        return "0,255,255";
    }
    else if ( id == 7 ) {
        return "255,255,255";
    }
    else if ( id == 8 ) {
        return "128,0,128";
    }
    else if ( id == 9 ) {
        return "255,0,128";
    }
    else if ( id == 10 ) {
        return "128,0,255";
    }
    else {return "128,128,128";}

}
///////////////////////////////////////////////////////////
//PB_SearchNameTeam
//////////////////////////////////////////////////////////
char *PB_SearchNameTeam(int n) {

    if (n == 0) {
        return "^3Free";
    }
    else if (n == 1) {
        return "^1Red";
    }
    else if (n == 2) {
        return "^4Blue";
    }
    else if (n == 3) {
        return "^3Spectator";
    }
    else {return NULL;}
}
///////////////////////////////////////////////////////////
//PB_SearchColorTeam
//////////////////////////////////////////////////////////
char *PB_SearchColorTeam(int n) {

    if (n == 0) {
        return "^2";
    }
    else if (n == 1) {
        return "^1";
    }
    else if (n == 2) {
        return "^4";
    }
    else if (n == 3) {
        return "^3";
    }
    else {return NULL;}
}
/*
=======================
PB_SaveWeapons
=======================
*/
static void PB_SaveWeapons(client_t *client) {

    playerState_t  *ps = SV_GameClientNum( client - svs.clients );
    int i;

    for (i = 0; i < MAX_POWERUPS; i++) {
        client->powerups[i] = ps->powerups[i];
    }
}
/*
=======================
PB_SwitchSlotWeapon
=======================
*/
static void PB_SwitchSlotWeapon(client_t *client) {

    playerState_t  *ps = SV_GameClientNum( client - svs.clients );
    int i;
    int idweap;
    int level = client->pblevel;
    int j = -1;
    int k = -1;
    //prim secon side knife
    if (level > 5) { level = level - (5 * client->pbcycle); }
    if (level == 3) {
        for (i = 0; i < MAX_POWERUPS; i++) {
            idweap = PB_SearchIDWeapon(ps->powerups[i], 1);
            if (Lweapons[idweap][4] == 3) { j=i; }
        }
        ps->weapon = j;
    }
    if (level == 2) {
        for (i = 0; i < MAX_POWERUPS; i++) {
            idweap = PB_SearchIDWeapon(ps->powerups[i], 1);
            if (Lweapons[idweap][4] == 2) { j=i; }
            if (Lweapons[idweap][4] == 3) { k=i; }
        }
        if (j != -1) {ps->weapon = j;}
        else {
            if (k != -1) {ps->weapon = k;}
            else {ps->weapon = 0;}
        }
    }

}
/*
=======================
PB_GiveFlag
=======================
*/
/*static void PB_GiveFlag(client_t *cl, int team) {

    char *cmd;

    if (team == 1) { cmd = "give team_CTF_redflag";}    
    else { cmd = "give team_CTF_blueflag";}    
    Cmd_TokenizeString(cmd);
    VM_Call(gvm, GAME_CLIENT_COMMAND, cl - svs.clients);
}*/
/*
=======================
PB_GiveHealth
=======================
*/
static void PB_GiveHealth( client_t *cl ) {
    char *cmd = "give health";
    Cmd_TokenizeString(cmd);
    VM_Call(gvm, GAME_CLIENT_COMMAND, cl - svs.clients);
}
/*
=======================
PB_GiveKevlar
=======================
*/
static void PB_GiveKevlar(client_t *cl) {

    char *cmd;

    cmd = "give ut_item_vest";    
    Cmd_TokenizeString(cmd);
    VM_Call(gvm, GAME_CLIENT_COMMAND, cl - svs.clients);
}
/*
=======================
PB_GiveMedkit
=======================
*/
static void PB_GiveMedkit(client_t *cl) {

    char *cmd;

    cmd = "give ut_item_medkit";    
    Cmd_TokenizeString(cmd);
    VM_Call(gvm, GAME_CLIENT_COMMAND, cl - svs.clients);
}
/*
=======================
PB_GiveGrenade
=======================
*/
static void PB_GiveGrenade(client_t *cl) {

    char *cmd;

    cmd = "give ut_weapon_grenade_he";    
    Cmd_TokenizeString(cmd);
    VM_Call(gvm, GAME_CLIENT_COMMAND, cl - svs.clients);
}
/*
=======================
PB_GiveLaser
=======================
*/
/*static void PB_GiveLaser(client_t *cl) {

    char *cmd;

    cmd = "give ut_item_laser";    
    Cmd_TokenizeString(cmd);
    VM_Call(gvm, GAME_CLIENT_COMMAND, cl - svs.clients);
}*/
/*
=======================
PB_NukeClient
=======================
*/
static void PB_NukeClient(client_t *cl) {
    char cmd[64];
    int clid = cl - svs.clients;
    Com_sprintf(cmd, sizeof(cmd), "nuke %i\n", clid);
    Cmd_ExecuteString(cmd);
}
/*
===============================================================================================================================================================
PB_ControlWeapons
===============================================================================================================================================================
*/
/*
=======================
PB_BZHControlWeapons
=======================
*/
void PB_BZHControlWeapons( client_t *cl ) {

    int i;
    int idweap;
    //int nammo;
    int level = cl->pblevel;
    if (level > 5) { level = level - (5 * cl->pbcycle); }
    playerState_t  *ps = SV_GameClientNum( cl - svs.clients );
    //knife
    /*for (i = 0; i < MAX_POWERUPS; i++) {
        idweap = PB_SearchIDWeapon(ps->powerups[i], 1);
        nammo = PB_SearchIDWeapon(ps->powerups[i], 3);
        if (i ==0 && idweap == 1 ) {
           if (nammo < 5) { ps->powerups[i] = ps->powerups[i] + 256; }

        }
    }*/
    if (level == 1) {
        for (i = 0; i < MAX_POWERUPS; i++) {
            idweap = PB_SearchIDWeapon(ps->powerups[i], 1);
            if (idweap == 27 ) {
                ps->powerups[i] = 0;
            }
        }
    }
    int t = 0;
    if (level == 2) {
        for (i = 0; i < MAX_POWERUPS; i++) {
            idweap = PB_SearchIDWeapon(ps->powerups[i], 1);
            if (Lweapons[idweap][4] == 1) { ps->powerups[i] = 0; }
            if (t == 0 && Lweapons[idweap][4] == 2 && i == cl->weapsecond) {t = 1; continue;}
            if (idweap == 27 ) {
                ps->powerups[i] = 0;
            }
        }
        for (i = 0; i < MAX_POWERUPS; i++) {
            idweap = PB_SearchIDWeapon(ps->powerups[i], 1);
            if (Lweapons[idweap][4] == 1) { ps->powerups[i] = 0; }
            if (t == 1 && Lweapons[idweap][4] == 2 && i != cl->weapsecond) { ps->powerups[i] = 0; }
            if (idweap == 27 ) {
                ps->powerups[i] = 0;
            }
        }
    }
    if (level == 3) {
        for (i = 0; i < MAX_POWERUPS; i++) {
            idweap = PB_SearchIDWeapon(ps->powerups[i], 1);
            if (Lweapons[idweap][4] == 1 || Lweapons[idweap][4] == 2) { ps->powerups[i] = 0; }
            if (idweap == 27 ) {
                ps->powerups[i] = 0;
            }
        }
    }
    if (level == 4) {
        for (i = 0; i < MAX_POWERUPS; i++) {
            idweap = PB_SearchIDWeapon(ps->powerups[i], 1);
            if (idweap == 1) { continue; }
            if (idweap == 27) { continue; }
            ps->powerups[i] = 0;
        }
    }
    if (level == 5) {
        for (i = 0; i < MAX_POWERUPS; i++) {
            idweap = PB_SearchIDWeapon(ps->powerups[i], 1);
            if (idweap == 1) { ps->powerups[i] = 0; }
            if (Lweapons[idweap][4] == 1 || Lweapons[idweap][4] == 2 || Lweapons[idweap][4] == 3) { ps->powerups[i] = 0; }
        }
    }
 }
/*
=======================
PB_ControlStamina
=======================
*/
void PB_ControlStamina( client_t *cl ) {

    playerState_t  *ps = SV_GameClientNum( cl - svs.clients );
    int weap = PB_SearchIDWeapon(ps->powerups[ps->weapon], 1);

    if (weap == 11 || weap == 12 || weap == 13 || weap == 16 || weap < 5 || weap == 20 || weap == 21 || weap == 26 ) {ps->stats[STAT_STAMINA] = ps->stats[STAT_HEALTH] * 300;}
}

/*
=======================
PB_ControlWeapons
=======================
*/
void PB_ControlWeapons( client_t *cl ) {

    if (pb_BZHGame->integer > 0 && pb_BZHGame->integer < 3) {
        PB_ControlStamina( cl );
    }
    if (pb_BZHGame->integer==1) {
        PB_BZHControlWeapons( cl );
    }
 }
/*
===============================================================================================================================================================
PB GameControl
===============================================================================================================================================================
*/
/*
=======================
PB_Pub
=======================
*/
void PB_Pub( void ) {

    if (!Q_stricmp(pb_filepub->string, "")) {
        return;
    }

    if (pb_timepub->integer !=0 || pb_timepub->integer > 5 || pb_timepub->integer < 3601) {

        static int lastTime = 0;
        static int lastLigne = 0;

        char *positionpub;

        if (!Q_stricmp(pb_positionpub->string, "chat")) {positionpub = "chat";}
        else if (!Q_stricmp(pb_positionpub->string, "cp")) {positionpub = "cp";}
        else {positionpub = "print";}

        char *pubhomePath;
        char *pubq3ut4Path;

        pubhomePath = Cvar_VariableString( "fs_homePath" );

        pubq3ut4Path = Cvar_VariableString( "fs_game" );

        if (lastTime + pb_timepub->integer*1000 <= svs.time){

            FILE* filepub = NULL;
            static char ligne[1024]="";
            filepub = fopen(FS_BuildOSPath( pubhomePath, pubq3ut4Path, pb_filepub->string), "r");

            if (!filepub) { return; }

            int NombreLignes = 0;
            int i;
            if (filepub != NULL) {

                i = 0;

                while (fgets(ligne, sizeof(ligne), filepub) != NULL) {

                    char a[64] = "";
                    char b[64] = "";
                    char *nligne = NULL;

                    if (strcmp(ligne, "\n") != 0){

                        if(strstr(ligne, "@map") != NULL) {

                            Com_sprintf(a, sizeof(a), "@map");
                            Com_sprintf(b, sizeof(b), "%s", sv_mapname->string);
                        }

                        if(strstr(ligne, "@time") != NULL) {

                            Com_sprintf(a, sizeof(a), "@time");

                            time_t timestamp;
                            struct tm * t;
                            timestamp = time(NULL);
                            t = localtime(&timestamp);

                            Com_sprintf(b, sizeof(b), "%02u:%02u", t->tm_hour, t->tm_min);
                        }

                        if(strstr(ligne, "@nextmap") != NULL) {
                            char *nextmap = PB_Searchnextmap();
                            Com_sprintf(a, sizeof(a), "@nextmap");
                            Com_sprintf(b, sizeof(b), "%s", nextmap);
                        }

                        if (Q_stricmp(a, "")) {

                            nligne = ligne;
                            nligne = PB_Replace_str(ligne, a, b);
                            Com_sprintf(ligne, sizeof(ligne), nligne);
                        }

                        if (lastLigne == i) {
                            SV_SendServerCommand(NULL, "%s \"%s\"", positionpub, ligne);

                        }
                        i++;
                    }
                    NombreLignes++;
                }

            }

            lastLigne++;

            if ( lastLigne >= NombreLignes) {lastLigne = 0;}
            lastTime = svs.time;
            fclose(filepub);
        }

    }
}

/*
=======================
PB_SpawnControl
=======================
*/
void PB_SpawnControl( void ) {

    client_t *cl;
    int i;
    int j;
    int k;
    for (i = 0, cl = svs.clients; i < sv_maxclients->integer; i++, cl++) {
        if (!cl->state || cl->netchan.remoteAddress.type == NA_BOT ) { continue; }
        playerState_t  *ps = SV_GameClientNum( cl - svs.clients );
        if ( ps->persistant[PERS_TEAM] != 3) {
            if (cl->spawnposition[0] == ps->origin[0] || cl->spawnposition[1] == ps->origin[1]) {
                for (j = 0, k= 0; j < MAX_POWERUPS; j++, k++) {
                    if (cl->pblevel != 4 || cl->pblevel != 5) {
                        if (ps->powerups[j] == 272) {
                            k--; continue;
                        }
                        if (ps->powerups[j] != 272) {
                            ps->powerups[j] = cl->powerups[k];
                        }
                    }
                    else {
                        continue;
                    }
                    if (PB_SearchIDWeapon(ps->powerups[j], 1) != 1 && PB_SearchIDWeapon(ps->powerups[j], 1) != 16 && PB_SearchIDWeapon(ps->powerups[j], 1) != 11 && PB_SearchIDWeapon(ps->powerups[j], 1) != 12 && PB_SearchIDWeapon(ps->powerups[j], 1) != 13) {
                        ps->powerups[j] = ps->powerups[j] + (2*16777216);
                    }

                }
            }
        }
    }

    for (i = 0, cl = svs.clients; i < sv_maxclients->integer; i++, cl++) {
        if (!cl->state || cl->netchan.remoteAddress.type == NA_BOT ) { continue; }

        if (cl->spawntime+500 == svs.time) {

            playerState_t  *ps = SV_GameClientNum( cl - svs.clients );
            
            int level = cl->pblevel;
            if (level > 5) { level = level - (5 * cl->pbcycle); }
            
            if (level == 1) {
                SV_SendServerCommand(cl, "cp \"^4Level %i ^7(^4%i/%i^7): All weapons allowed\"\n", cl->pblevel, cl->pbpoints, 4+cl->pbcycle);
            }
            if (level == 2) {
                SV_SendServerCommand(cl, "cp \"^6Level %i ^7(^6%i/%i^7): No primary weapons\"\n", cl->pblevel, cl->pbpoints, 3+cl->pbcycle);
                PB_SwitchSlotWeapon(cl);
            }
            else if (level == 3) {
                SV_SendServerCommand(cl, "cp \"^5Level %i ^7(^5%i/%i^7): No primary and secondary weapons\"\n", cl->pblevel, cl->pbpoints, 2+cl->pbcycle);
                PB_SwitchSlotWeapon(cl);
            }
            else if (level == 4) {
                SV_SendServerCommand(cl, "cp \"^2Level %i ^7(^2%i/%i^7): Knife-Only\"\n", cl->pblevel, cl->pbpoints, 1);
                ps->weapon = 0;
                PB_GiveKevlar( cl );
                PB_GiveMedkit( cl );
            }
            else if (level == 5) {
                SV_SendServerCommand(cl, "cp \"^1Level %i ^7: TOD50 only\"\n", cl->pblevel);
                ps->powerups[0] = 283+16777216;
                ps->weapon = 0;
            }
        }
    }
}

/*
=======================
PB_GameControl
=======================
*/
void PB_GameControl( void ) {

    if (Q_stricmp(pb_filepub->string, "") != 0 && pb_timepub->integer !=0) {
        if (sv.time > 5000){
            PB_Pub();
        }
    }
    if (sv_gametype->integer > 8) { Cvar_Set("pb_bzhgame", "");}
    if (pb_BZHGame->integer == 1) {
        PB_SpawnControl();
    }
    if (sv.time >= 10000 && sv.time < 11000) {
        Cvar_Set("sv_cheats", "1");
    }
}
/*
===============================================================================================================================================================
PB CheckDeadorAlive KillDistance BZHGameScores
===============================================================================================================================================================
*/
/*
=======================
PB_CheckDeadorAlive
=======================
*/
static void PB_CheckDeadorAlive( client_t *aclient, client_t *vclient, char *arg ) {

    playerState_t  *ps;

    int            t, min, tens, sec;

    sec = ( sv.time - sv.restartTime ) / 1000;
    t = ( sv.time - sv.restartTime );
    min = sec / 60;
    sec -= min * 60;
    tens = sec / 10;
    sec -= tens * 10;

    char *homePath;
    char *q3ut4Path;
    char *logpath;

    int aclid = aclient - svs.clients;
    int vclid = vclient - svs.clients;

    homePath = Cvar_VariableString( "fs_homePath" );

    q3ut4Path = Cvar_VariableString( "fs_game" );

    logpath = FS_BuildOSPath( homePath, q3ut4Path, Cvar_VariableString("g_log"));

    ps = SV_GameClientNum( vclient - svs.clients );

    FILE* fichier = NULL;

    fichier = fopen(logpath, "a");

    if (fichier != NULL)
    {
        if (strcmp(arg,"spawn") == 0){
            fprintf(fichier, "%3i:%i%i ClientAlive: %i %f %f %i\n", min, tens, sec, vclid, ps->origin[0], ps->origin[1], t);

        }
        if (strcmp(arg,"dead") == 0){
            fprintf(fichier, "%3i:%i%i ClientDead: %i %i %f %f %i\n", min, tens, sec, vclid, aclid, ps->origin[0], ps->origin[1], t);

        }
        fclose(fichier);
    }

}
/*
=======================
PB_KillDistance
=======================
*/
static void PB_KillDistance( client_t *aclient, client_t *vclient, int idweapon ) {

    if (aclient - svs.clients == vclient - svs.clients || aclient->netchan.remoteAddress.type == NA_BOT)
    {
        return;
    }

    playerState_t  *aps = SV_GameClientNum( aclient - svs.clients );
    playerState_t  *vps = SV_GameClientNum( vclient - svs.clients );

    float dx = aps->origin[0] - vps->origin[0];
    float dy = aps->origin[1] - vps->origin[1];
    float adx = (float)fabs(dx);
    float ady = (float)fabs(dy);
    float dx2dy2 = (adx * adx) + (ady * ady);

    float dist = (float)sqrtf(dx2dy2);

    char *weapon = PB_SearchWeapon(idweapon);

    int ateam = aps->persistant[PERS_TEAM];
    int vteam = vps->persistant[PERS_TEAM];

    char acname[64];
    char vcname[64];

    int hitid = vclient->lasthitlocation;

    char *hitlocation = PB_SearchHitLocation(hitid);

    Q_strncpyz(acname, aclient->name, sizeof(acname));
    Q_CleanStr(acname);

    Q_strncpyz(vcname, vclient->name, sizeof(vcname));
    Q_CleanStr(vcname);

    if (idweapon != 22 && idweapon != 23 && idweapon != 25 && idweapon != 37) {

        if (( sv_gametype->integer > 2 && sv_gametype->integer < 9 ) || sv_gametype->integer == 10 ) {

            if (ateam == vteam ) {
                SV_SendServerCommand(NULL, "print\"%s%s ^7killed %s%s^7 from ^3%.2f m ^7[%s][%s]\"\n", PB_SearchColorTeam(ateam), acname, PB_SearchColorTeam(vteam), vcname, (dist/50), hitlocation, weapon );
                return;
            }
        }

        if (hitid == 1 || hitid == 2)
        {
            aclient->headshotskills = aclient->headshotskills + 1;

            char *texths = "kills";

            if (aclient->headshotskills < 2) {texths = "kill";}
            SV_SendServerCommand(NULL, "chat\"%s%s ^7got ^3%i ^7HeadShot %s!\"\n", PB_SearchColorTeam(ateam), acname, aclient->headshotskills, texths);
            SV_SendServerCommand(NULL, "print\"%s%s ^7killed %s%s ^7 with a ^3HeadShot^7 from ^3%.2f ^7[%s]\"\n", PB_SearchColorTeam(ateam), acname, PB_SearchColorTeam(vteam), vcname, (dist/50), weapon );
        }
        else {
            SV_SendServerCommand(NULL, "print\"%s%s ^7killed %s%s^7 from ^3%.2f ^7[%s][%s]\"\n", PB_SearchColorTeam(ateam), acname, PB_SearchColorTeam(vteam), vcname, (dist/50), hitlocation, weapon );
        }
    }
    else {

        SV_SendServerCommand(NULL, "print\"%s%s ^7killed %s%s^7 from ^3%.2f ^7[%s]\"\n", PB_SearchColorTeam(ateam), acname, PB_SearchColorTeam(vteam), vcname, (dist/50), weapon );
    }

}
/*
=======================
PB_BZHGameScores
=======================
*/
static void PB_BZHGameScores( client_t *aclient,  client_t *vclient, int idweap ) {

    playerState_t  *aps = SV_GameClientNum( aclient - svs.clients );
    playerState_t  *vps = SV_GameClientNum( vclient - svs.clients );

    int ateam = aps->persistant[PERS_TEAM];
    int vteam = vps->persistant[PERS_TEAM];
    
    char acname[64];
    char vcname[64];
    
    Q_strncpyz(acname, aclient->name, sizeof(acname));
    Q_CleanStr(acname);
    char *cateam = PB_SearchColorTeam(ateam);

    Q_strncpyz(vcname, vclient->name, sizeof(vcname));
    Q_CleanStr(vcname);
    char *cvteam = PB_SearchColorTeam(vteam);
    
    int alevel = aclient->pblevel;
    int vlevel = vclient->pblevel;

    if (alevel > 5) { alevel = alevel - (5 * aclient->pbcycle); }
    if (vlevel > 5) { vlevel = vlevel - (5 * vclient->pbcycle); }
    
    if (aclient - svs.clients != vclient - svs.clients || idweap == 7 || idweap == 35) {

        if (sv_gametype->integer == 0 || sv_gametype->integer == 1 || sv_gametype->integer == 7 || idweap == 7 || idweap == 35) {
            ateam = 1; vteam = 2;
        }

        if (ateam != vteam) {
            if (aclient->netchan.remoteAddress.type != NA_BOT) {
                if (alevel < 4) {
                    if (idweap == 12 || idweap == 24) {
                        PB_GiveGrenade(aclient);
                    }                    
                }
                if (alevel == 1 && idweap != 23 && idweap != 7) {

                    aclient->pbpoints = aclient->pbpoints + 1;

                    if (aclient->pbpoints >= 4+aclient->pbcycle) {
                        SV_SendServerCommand(aclient, "chat\"^3[PM]: ^2[+1 point] ^7Level %i (%i/%i)\"\n", aclient->pblevel, aclient->pbpoints, 4+aclient->pbcycle);
                        aclient->pbpoints = 0;
                        aclient->pblevel = aclient->pblevel + 1;
                        SV_SendServerCommand(aclient, "chat\"^3[PM]: ^7New Level: Level %i\"\n", aclient->pblevel);
                        SV_SendServerCommand(NULL, "print\"%s%s ^7is now a level %i\"\n", cateam, acname, aclient->pblevel);
                        PB_SwitchSlotWeapon(aclient);
                        PB_RechargeWeapons(aclient, -1);
                        PB_GiveHealth( aclient );
                    }
                    else {
                        SV_SendServerCommand(aclient, "chat\"^3[PM]: ^2[+1 point] ^7Level %i (%i/%i)\"\n", aclient->pblevel, aclient->pbpoints, 4+aclient->pbcycle);
                    }
                }
                else if (alevel == 2 && idweap != 23 && idweap != 7) {

                    aclient->pbpoints = aclient->pbpoints + 1;

                    if (aclient->pbpoints >= 3+aclient->pbcycle) {
                        SV_SendServerCommand(aclient, "chat\"^3[PM]: ^2[+1 point] ^7Level %i (%i/%i)\"\n", aclient->pblevel, aclient->pbpoints, 3+aclient->pbcycle);
                        aclient->pbpoints = 0;
                        aclient->pblevel = aclient->pblevel + 1;
                        SV_SendServerCommand(aclient, "chat\"^3[PM]: ^7New Level: Level %i\"\n", aclient->pblevel);
                        SV_SendServerCommand(NULL, "print\"%s%s ^7is now a level %i\"\n", cateam, acname, aclient->pblevel);
                        PB_SwitchSlotWeapon(aclient);
                        PB_RechargeWeapons(aclient, -1);
                        PB_GiveHealth( aclient );
                    }
                    else {
                        SV_SendServerCommand(aclient, "chat\"^3[PM]: ^2[+1 point] ^7Level %i (%i/%i)\"\n", aclient->pblevel, aclient->pbpoints, 3+aclient->pbcycle);
                    }
                }
                else if (alevel == 3 && idweap != 23 && idweap != 7) {

                    aclient->pbpoints = aclient->pbpoints + 1;

                    if (aclient->pbpoints >= 2+aclient->pbcycle) {
                        SV_SendServerCommand(aclient, "chat\"^3[PM]: ^2[+1 point] ^7Level %i (%i/%i)\"\n", aclient->pblevel, aclient->pbpoints, 2+aclient->pbcycle);
                        aclient->pbpoints = 0;
                        aclient->pblevel = aclient->pblevel + 1;
                        SV_SendServerCommand(aclient, "chat\"^3[PM]: ^7New Level: Level %i\"\n", aclient->pblevel);
                        SV_SendServerCommand(NULL, "print\"%s%s ^7is now a level %i\"\n", cateam, acname, aclient->pblevel);
                        aps->weapon = 0;
                        PB_GiveHealth( aclient );
                        PB_GiveKevlar( aclient );
                        PB_GiveMedkit( aclient );
                    }
                    else {
                        SV_SendServerCommand(aclient, "chat\"^3[PM]: ^2[+1 point] ^7Level %i (%i/%i)\"\n", aclient->pblevel, aclient->pbpoints, 2+aclient->pbcycle);
                    }
                }
                else if (alevel == 4 && idweap != 23 && idweap != 7) {
                    if (idweap == 12 || idweap == 13 || idweap == 24) {

                        aclient->pbpoints = aclient->pbpoints + 1;

                        if (aclient->pbpoints >= 1) {
                           SV_SendServerCommand(aclient, "chat\"^3[PM]: ^2[+1 point] ^7Level %i (%i/%i)\"\n", aclient->pblevel, aclient->pbpoints, 1);
                           aclient->pbpoints = 0;
                           aclient->pblevel = aclient->pblevel + 1;
                           SV_SendServerCommand(aclient, "chat\"^3[PM]: ^7New Level: Level %i\"\n", aclient->pblevel);
                           SV_SendServerCommand(NULL, "print\"%s%s ^7is now a level %i\"\n", cateam, acname, aclient->pblevel);
                           aps->powerups[0] = 283+16777216;
                           PB_GiveHealth( aclient );

                        }
                        else {
                            SV_SendServerCommand(aclient, "chat\"^3[PM]: ^2[+1 point] ^7Level %i (%i/%i)\"\n", aclient->pblevel, aclient->pbpoints, 1);
                       }
                    }
                }
                else if (alevel == 5 && idweap != 7) {
                    if (idweap == 46) {
                        aclient->pbpoints = aclient->pbpoints + 1;
                        aclient->pbscore = aclient->pbscore + 1;
                        if (aclient->pbpoints > 19 ) { PB_NukeClient(aclient); }
                    }
                }

            }

            if (vclient->netchan.remoteAddress.type != NA_BOT) {

                vclient->pbpoints = vclient->pbpoints - 1;

                if (vlevel == 1) {

                    if (vclient->pbpoints < 0) { vclient->pbpoints = 0; }
                    else {SV_SendServerCommand(vclient, "chat\"^3[PM]: ^1[-1 point] ^7Level %i (%i/%i)\"\n", vclient->pblevel, vclient->pbpoints, 4+vclient->pbcycle);}
                }
                else if (vlevel == 2) {

                    if (vclient->pbpoints < 0) {
                        vclient->pbpoints = 3+vclient->pbcycle;
                        vclient->pblevel = vclient->pblevel - 1;
                        SV_SendServerCommand(vclient, "chat\"^3[PM]: ^1[-1 point] ^7Return to Level %i (%i/%i)\"\n", vclient->pblevel, vclient->pbpoints, 4+vclient->pbcycle);
                        SV_SendServerCommand(NULL, "print\"%s%s ^7returns to level %i\"\n", cvteam, vcname, vclient->pblevel);
                    }
                    else {
                        SV_SendServerCommand(vclient, "chat\"^3[PM]: ^1[-1 point] ^7Level %i (%i/%i)\"\n", vclient->pblevel, vclient->pbpoints, 3+vclient->pbcycle);
                    }
                }
                else if (vlevel == 3) {

                     if (vclient->pbpoints < 0) {
                        vclient->pbpoints = 2+vclient->pbcycle;
                        vclient->pblevel = vclient->pblevel - 1;
                        SV_SendServerCommand(vclient, "chat\"^3[PM]: ^1[-1 point] ^7Return to Level %i (%i/%i)\"\n", vclient->pblevel, vclient->pbpoints, 3+vclient->pbcycle);
                        SV_SendServerCommand(NULL, "print\"%s%s ^7returns to level %i\"\n", cvteam, vcname, vclient->pblevel);
                    }
                    else {
                        SV_SendServerCommand(vclient, "chat\"^3[PM]: ^1[-1 point] ^7Level %i (%i/%i)\"\n", vclient->pblevel, vclient->pbpoints, 2+vclient->pbcycle);
                    }
                }
                else if (vlevel == 4) {

                    if (vclient->pbpoints < 0) {
                        vclient->pbpoints = 0;
                        SV_SendServerCommand(vclient, "chat\"^3[PM]: ^2Too Hard For You! ^5Command: ^7!toohardforme ^5or ^7!thfm \"\n");
                    }
                    /*else {
                        SV_SendServerCommand(vclient, "chat\"^3[PM]: ^7Level %i (%i/%i)\"\n", vclient->pblevel, vclient->pbpoints, 1);
                    }*/
                }
                else if (vlevel == 5) {
                    if (vclient->pbpoints+1 < 0) { vclient->pbpoints=0;}
                    char *textpoints = "point";
                    char *colorpoints = "^1";
                    if (vclient->pbpoints+1 > 0) {textpoints = "points"; colorpoints = "^2";}
                    SV_SendServerCommand(vclient, "chat\"^3[PM]: ^7Level %i finished with %s%i %s \"\n", vclient->pblevel, colorpoints, vclient->pbpoints+1, textpoints);
                    SV_SendServerCommand(NULL, "cp\"%s%s^7: ^1Level %i ^7finished with %s%i %s \"\n", cvteam, vclient->name, vclient->pblevel, colorpoints, vclient->pbpoints+1, textpoints);
                    vclient->pbpoints = 0;
                    vclient->pblevel = vclient->pblevel + 1;
                    vclient->pbcycle = vclient->pbcycle + 1;
                    SV_SendServerCommand(vclient, "chat\"^3[PM]: ^7New Level %i (%i/%i)\"\n", vclient->pblevel, vclient->pbpoints, 4+vclient->pbcycle);
                    SV_SendServerCommand(NULL, "print\"%s%s ^7is now a level %i\"\n", cvteam, vcname, vclient->pblevel);


                }

            }
        }
    }
}
/*
===============================================================================================================================
PB Commandes
===============================================================================================================================
*/
/*
==============================
PB Commande Too_Hard_For_Me
==============================
*/
void PB_TooHardForMe(client_t *cl) 
{
    playerState_t  *ps = SV_GameClientNum( cl - svs.clients );

    int team = ps->persistant[PERS_TEAM];

    char cname[64];
    Q_strncpyz(cname, cl->name, sizeof(cname));
    Q_CleanStr(cname);

    if (team == 3)
    {
	    SV_SendServerCommand(cl, "chat\"^3[PM]: ^7Nice try, but spectators can't use this command.\"\n");
        return;
    }

    char *cteam = PB_SearchColorTeam(team);
    
    int kill = ps->persistant[PERS_SCORE];
    int death = ps->persistant[PERS_KILLED];
    
    int level = cl->pblevel;
	int colorlevel;
	
    if (level > 5) { level = level - (5 * cl->pbcycle); }

	if (level == 1) {colorlevel = 4;}
	else if (level == 2) {colorlevel = 6;}
	else if (level == 3) {colorlevel = 5;}
	else if (level == 4) {colorlevel = 2;}
	else if (level == 5) {colorlevel = 1;}

	if (level == 1 || level == 2 || level == 3) {
	    SV_SendServerCommand(cl, "chat\"^3[PM]: ^%iLevel %i ^7- Have courage! You can do it!\"\n", colorlevel , level);
	    SV_SendServerCommand(cl, "chat\"^3[PM]: ^7Return to ^4level 1^7. This is only possible in the Knife-Only Level.\"\n");
	}
    else if (level == 4) {
        if ((kill - death) < 0 ) {
            cl->pblevel = 1;
	        char *cmd = "kill";
	        Cmd_TokenizeString(cmd);
            SV_SendServerCommand(cl, "chat\"^3[PM]: ^1Too Hard For You! ^7Oups! Back to ^4level 1^7\"\n");
            SV_SendServerCommand(NULL, "print\"^6Too Hard For Him! %s%s ^7is back on level 1\"\n", cteam, cname);
            VM_Call(gvm, GAME_CLIENT_COMMAND, cl - svs.clients);
        }
        else {
            SV_SendServerCommand(cl, "cp\"^3[PM]: ^%iLevel %i^7 - Have courage! You can do it!\"\n", colorlevel , level);
        }

    }
    else {
        SV_SendServerCommand(NULL, "cp\"%s%s^7: ^%iLevel %i^7- Too Hard For Him, apparently!^7\"\n", cteam, cl->name, colorlevel , level);
    }
}
/*
===============================================================================================================================
*/
/*
=====================================================================
PB_Events
=====================================================================
*/
/*
=======================
PB_EventKill
=======================
*/
void PB_EventKill(char event[1024])
{
    if (sv_gametype->integer == 9) {
         return; 
    }

    Cmd_TokenizeString( event );

    if (atoi(Cmd_Argv(1)) == 1022 && atoi(Cmd_Argv(2)) < 100) {

        client_t *vcl = PB_SearchUser(atoi(Cmd_Argv(2)));
        client_t *acl = vcl;
        PB_CheckDeadorAlive( acl, vcl, "dead" );
        if (pb_BZHGame->integer == 1) {
            PB_BZHGameScores(acl, vcl, atoi(Cmd_Argv(3)));
        }
    }
    if (atoi(Cmd_Argv(1)) < 100 && atoi(Cmd_Argv(2)) < 100) {

        client_t *acl = PB_SearchUser(atoi(Cmd_Argv(1)));
        client_t *vcl = PB_SearchUser(atoi(Cmd_Argv(2)));

        PB_CheckDeadorAlive( acl, vcl, "dead" );

        if (Cvar_VariableValue("g_loghits") != 0) {
            PB_KillDistance( acl, vcl, atoi(Cmd_Argv(3)));
        }
            
        if (pb_BZHGame->integer == 1) {
            PB_BZHGameScores(acl, vcl, atoi(Cmd_Argv(3)));
        }

    }
}
/*
=======================
PB_EventHit
=======================
*/
void PB_EventHit(char event[1024])
{
    if (sv_gametype->integer == 9 || Cvar_VariableValue("g_loghits") == 0) {
         return; 
    }

    Cmd_TokenizeString( event );
    
    if (atoi(Cmd_Argv(1)) < 100 && atoi(Cmd_Argv(2)) < 100) {
        client_t *vcl = PB_SearchUser(atoi(Cmd_Argv(1)));
        vcl->lasthitlocation = atoi(Cmd_Argv(3));
        vcl->lasthitweapon = atoi(Cmd_Argv(4));
    }
}
/*
=======================
PB_EventClientSpawn
=======================
*/
void PB_EventClientSpawn(char event[1024])
{
    if (sv_gametype->integer == 9) {
         return; 
    }

    Cmd_TokenizeString( event );

    if (atoi(Cmd_Argv(1)) < 100) {

        client_t *cl = PB_SearchUser(atoi(Cmd_Argv(1)));
        playerState_t  *ps = SV_GameClientNum( cl - svs.clients );

        PB_CheckDeadorAlive( cl, cl, "spawn" );

        if ( cl->netchan.remoteAddress.type == NA_BOT ) {
            return;
        }
        else {
            cl->spawnposition[0] = ps->origin[0];
            cl->spawnposition[1] = ps->origin[1];

            if (ps->persistant[PERS_SPAWN_COUNT] < 2 || ps->persistant[PERS_HITS] == 0) {
                cl->lasthitlocation = -1;
                cl->lasthitweapon = -1;
                cl->headshothits = 0;
                cl->headshotskills = 0;
                cl->pbpoints = 0;
                cl->pblevel = 1;
                cl->pbcycle = 0;
                cl->pbscore = 0;
                cl->weapsecond = -1;
            }
            cl->spawntime = svs.time;
            int level = cl->pblevel;
            if (level > 5) { level = level - (5 * cl->pbcycle); }
                
            PB_BZHControlWeapons(cl);
                
            if (level == 5) {
                ps->powerups[0] = 283+16777216;
            }
            PB_SaveWeapons(cl);
        }
    }
}
/*
=======================
PB_EventClientBegin
=======================
*/
void PB_EventClientBegin(char event[1024])
{
    if (sv_gametype->integer == 9) {
         return; 
    }

    Cmd_TokenizeString( event );
    
    if (atoi(Cmd_Argv(1)) < 100) {

        client_t *cl = PB_SearchUser(atoi(Cmd_Argv(1)));

        if ( cl->netchan.remoteAddress.type == NA_BOT ) {
            char *armband = PB_BotArmbandColor(atoi(Cmd_Argv(1)));
            if (atoi(Cmd_Argv(1))%2) {
                
                char *color ="^5";
                char name[64];
                strcpy(name, color);
                strcat(name, cl->name);
                Info_SetValueForKey(cl->userinfo, "name", name);
                Info_SetValueForKey(cl->userinfo, "racered", "3");
                Info_SetValueForKey(cl->userinfo, "raceblue", "3");
                Info_SetValueForKey(cl->userinfo, "racefree", "3");
                Info_SetValueForKey(cl->userinfo, "funred", "ninja");
                Info_SetValueForKey(cl->userinfo, "funblue", "ninja");
                Info_SetValueForKey(cl->userinfo, "funfree", "ninja");
                Info_SetValueForKey(cl->userinfo, "cg_RGB", armband);
                SV_UserinfoChanged(cl);
                VM_Call(gvm, GAME_CLIENT_USERINFO_CHANGED, cl - svs.clients);

            }
            else {
                char *name = PB_BotFemaleName(atoi(Cmd_Argv(1)));
                Info_SetValueForKey(cl->userinfo, "name", name);
                Info_SetValueForKey(cl->userinfo, "racered", "1");
                Info_SetValueForKey(cl->userinfo, "raceblue", "1");
                Info_SetValueForKey(cl->userinfo, "racefree", "1");
                Info_SetValueForKey(cl->userinfo, "funred", "ninja");
                Info_SetValueForKey(cl->userinfo, "funblue", "ninja");
                Info_SetValueForKey(cl->userinfo, "funfree", "ninja");
                Info_SetValueForKey(cl->userinfo, "cg_RGB", armband);
                SV_UserinfoChanged(cl);
                VM_Call(gvm, GAME_CLIENT_USERINFO_CHANGED, cl - svs.clients);
            }
        }
    }
}
/*
=======================
PB_EventItem
=======================
*/
void PB_EventItem(char event[1024])
{
    if (sv_gametype->integer == 9){ return; }

    Cmd_TokenizeString( event );
    
    if (pb_BZHGame->integer == 1) {
        if (atoi(Cmd_Argv(1)) < 100) {
            client_t *cl = PB_SearchUser(atoi(Cmd_Argv(1)));
            int idweapp = PB_SearchIDUTWeapons(Cmd_Argv(2));
            int idweap;
            if (Lweapons[idweapp][4] == 2) {
                playerState_t  *ps = SV_GameClientNum( cl - svs.clients );
                int i;
                for (i = 0; i < MAX_POWERUPS; i++) {
                    idweap = PB_SearchIDWeapon(ps->powerups[i], 1);
                    if (Lweapons[idweap][4] == 2) {
                        if (idweapp != idweap) {
                            cl->weapsecond = i;    
                        }
                    }                        
                }
            }
            PB_RechargeWeapons( cl, idweapp );
        }
    }
}
