#include "cServer.h"
#include "Basedef.h"
#include "SendFunc.h"
#include "GetFunc.h"
#include <array>
#include <io.h>
#include <fcntl.h>
#include <algorithm>
#include <string>
#include <sstream>
#include <filesystem>
#include <map>
#include "pugixml.hpp"
#include "CEventManager.h"
#include "ItemsAction.h"
#include <tuple>
#include <ctime>

using namespace std::string_literals;
// Declaraaao das variaveis externs
char g_pLanguageString[MAX_STRING][MAX_MESSAGE];
WORD g_pMobGrid[4096][4096];
char g_pHeightGrid[4096][4096];
eMapAttribute g_pAttributeMap[1024][1024];
STRUCT_ITEMLIST g_pItemList[MAX_ITEMLIST];
STRUCT_SKILLDATA SkillData[256];
char EffectName[256][32];
STRUCT_TELEPORT mTeleport[MAX_TELEPORT];
STRUCT_MOB NPCBase[50];
WORD g_pItemGrid[4096][4096];
STRUCT_INITITEM g_pInitItem[4096];
BASE_InitItem g_pInitItemFile[4096];
STRUCT_GUILDZONE g_pCityZone[5];
int ChargedGuildList[10][5];
DWORD g_pHitRate[1024];
STRUCT_PISTA_DE_RUNAS pPista[MAX_ROOM];
char g_pGuildNotice[MAX_GUILD][128];
STRUCT_GUILDINFO g_pGuild[MAX_GUILD];
INT32 g_pGuildPoint [MAX_GUILD];
INT32 g_pGuildAlly[MAX_GUILD];
INT32 g_pGuildWar[MAX_GUILD];
STRUCT_NPC_EVENT npcEvent[MAX_NPCEVENTO];
STRUCT_NPCQUEST_CFILE questNPC[MAX_NPCQUEST];
stPositionCP g_pPositionCP[MAX_MESSAGE];
STRUCT_STOREDONATE g_pStore[MAX_STORE][MAX_DONATEITEM];
INT16 g_pBlockedItem[MAX_BLOCKITEM];
STRUCT_ITEMPAC g_pPacItem[MAX_PACITEM];

int g_pGenerateIndexes[MAX_ROOM] = {LICH_ID, TORRE_ID, VALKY_ID, SULRANG, HELL_BOSS};
int g_pGenerateLoops[MAX_ROOM] = {1, 3, 1, 3, 1, 0};

/* Event variables */
constexpr std::array<std::array<int, 9>, 7> Runes = 
{ {
	{ 5110, 5112, 5115, 5113, 5111, 0, 0, 0, 0 },
	{ 5114, 5113, 5117, 5111, 5115, 5112, 0, 0, 0 },
	{ 5118, 5121, 5122, 5116, 5130, 5119, 0, 0, 0 },
	{ 5122, 5126, 5121, 5116, 5119, 0, 0, 0, 0 },
	{ 5125, 5126, 5124, 5127, 0, 0, 0, 0, 0 },
	{ 5120, 5131, 5118, 5119, 5123, 5132, 5121, 0, 0},
	{ 5130, 5131, 5119, 5133, 5120, 5123, 5132, 5129, 5128 }
	}
};

int g_pTeleBarlog[5][4][8] =
{ // maxX, maxY, minX, minY, centerX, centerY
	{
		{3381, 1168, 3379, 1166, 3431, 1182, 2},
		{3381, 1180, 3379, 1178, 3431, 1182, 2},
	},
	{
		{3342, 1225, 3340, 1223, 3359, 1174, 3},
		{3353, 1226, 3351, 1223, 3359, 1174, 3},
		{3364, 1225, 3362, 1223, 3359, 1174, 3}
	},
	{
		{3380, 1239, 3378, 1237, 3348, 1263, 4},
		{3388, 1239, 3386, 1237, 3348, 1263, 4},
		{3396, 1239, 3394, 1237, 3348, 1263, 4},
		{3404, 1239, 3402, 1237, 3348, 1263, 4}
	},
	{
		{3431, 1199, 3429, 1197, 3422, 1232, 1}
	},
	{
		{0, 0, 0, 0, 3384, 1199, 0}
	}
};

unsigned int g_pPistaCoords[7][6] = {
	{3328,1599,3452,1660,3350,1655},
	{3328,1539,3452,1595,3393,1585},
	{3328,1407,3452,1466,3418,1451},
	{3328,1025,3452,1148,3380,1085},
	{3328,1280,3454,1406,3444,1396},
	{3328,1153,3452,1276,3430,1180},
	{3328,1467,3452,1535,3434,1502}
};

INT32 g_pGroundMask[6][4][6][6] = {
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 18, 18, 18, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 18, 0, 0, 0, 0, 0, 18, 0, 0, 0, 
	0, 0, 18, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 18, 18, 18, 0, 
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 18, 0, 0, 0, 0, 0, 18, 0, 0, 
	0, 0, 0, 18, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 18, 18, 18, 18, 
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 18, 0, 0, 0, 0, 0, 18, 0, 
	0, 0, 0, 0, 18, 0, 0, 0, 0, 0, 18, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 18, 18, 18, 
	18, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 18, 0, 0, 0, 0, 0, 18, 0, 0, 
	0, 0, 0, 18, 0, 0, 0, 0, 0, 18, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 18, 18, 0, 0, 
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 18, 0, 0, 0, 0, 0, 
	18, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 18, 18, 0, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 18, 0, 0, 0, 0, 0, 18, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 18, 18, 18, 18, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 18, 0, 0, 0, 0, 0, 18, 0, 0, 0, 0, 0, 18, 0, 0, 0, 0, 0, 18, 0, 0, 
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 18, 18, 18, 18, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 18, 0, 0, 0, 0, 0, 18, 0, 0, 0, 0, 0, 18, 0, 0, 0, 0, 0, 18, 0, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 0, 0, 0, 0, 18, 18, 18, 18, 18, 18, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 18, 18, 18, 18, 
	18, 18, 0, 18, 0, 0, 18, 0, 0, 18, 0, 0, 18, 0, 0, 18, 0, 0, 18, 0, 0, 18, 0, 0, 18, 0, 0, 18, 0, 0, 18, 0, 0, 18, 0, 0, 
	18, 0, 18, 18, 18, 18, 18, 18, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 18, 18, 18, 18, 18, 18, 0, 0, 0, 0,
	0, 0, 0, 18, 0, 0, 18, 0, 0, 18, 0, 0, 18, 0, 0, 18, 0, 0, 18, 0, 0, 18, 0, 0, 18, 0, 0, 18, 0, 0, 18, 0, 0, 18, 0, 0, 18, 0
};

STRUCT_MOUNTDATA mMont[30] =
{
	{0, 1, 0, 0, 4},// 0 - Porco
	{0, 1, 0, 0, 4}, // 1 - Javali
	{50, 9, 0, 0, 5}, // 2 - Lobo
	{80, 14, 0, 0, 5},// 3 -  Dragao Menor
	{100, 19, 0, 0, 4}, // 4 - Urso
	{150, 23, 0, 0, 5}, // 5 - Dente de Sabre
	{250, 47, 40, 0, 6}, // 6 - S/ Sela N
	{300, 57, 50, 0, 6}, //7 - Fantasma N
	{350, 61, 60, 0, 6}, // 8 - Leve N
	{400, 66, 70, 0, 6}, // 9 - Equip N
	{500, 80, 80, 0, 6}, // 10 - Anda N
 	{250, 47, 0, 16, 6}, // 11 - s/ sela B
	{300, 57, 0, 20, 6}, // 12 - Fantasma B
	{350, 61, 0, 24, 6}, // 13 - Leve B
	{400, 66, 0, 28, 6}, // 14 - Equip B
	{500, 80, 0, 32, 6}, // 15 - Anda B
	{550, 85, 0, 0, 6}, // 16 - Fenrir
	{600, 85, 0, 0, 6},  // 17 - Dragao
	{550, 85, 0, 20, 6}, // 18 - Fenrir das Sombras
	{650, 95, 60, 28, 6}, // 19 - Tigre de Fogo
	{700, 104, 80, 32, 6}, // 20 - DV
	{570, 85, 20, 16, 6}, // 21 - Unicarnio
	{570, 85, 30, 8, 6}, // 22 - Pegasus
	{570, 85, 40, 12, 6}, // 23 - unisus
	{590, 90, 30, 20, 6}, // 24 - Grifo
	{600, 90, 40, 16, 6}, // 25 - HipoGrifo 
	{600, 90, 50, 16, 6}, // 26 - Grifo Sangrento
	{600, 38, 60, 28, 6}, // 27 - Svaldfire
	{300, 90, 60, 28, 6}, // 28 - sleipnir
	{150, 23, 0, 20, 6} // pantera
};

TransBonus pSummonBonus[50] = {
	 { 80, 350, 70, 75, 100, 400, 0, 0, 0 },
	 { 80, 300, 70, 150, 125, 400, 0, 0, 0 },
	 { 80, 450, 70, 125, 125, 400, 0, 0, 0 },
	 { 80, 400, 70, 200, 150, 400, 0, 0, 0},
	 { 80, 550, 70, 175, 150, 400, 0, 0, 0 },
	 { 80, 450, 70, 250, 175, 400, 0, 0, 0 },
	 { 100, 550, 60, 250, 174, 400, 0, 0, 0 },
	 { 130, 300, 80, 200, 180, 250, 0, 0, 0 }
 };

int Taxes[64] = {
	900, 900, 900, 900, 900, 900, 900, 900, 900, 900, 900, 900, 900, 900,
	900, 900, 900, 900, 900, 900, 900, 900, 900, 900, 900, 900, 900, 900, 
	900, 900, 900, 900, 900, 900, 900, 900, 900, 900, 900, 900, 900, 900, 
	900, 900, 900, 900, 900, 900, 900, 900, 900, 900, 900, 900, 900, 900, 
	900, 900, 900
};

unsigned char ItemGrid[8][8] = {
    { 01, 00, 00, 00, 00, 00, 00, 00 }, { 01, 00, 01, 00, 00, 00, 00, 00 },
    { 01, 00, 01, 00, 01, 00, 00, 00 }, { 01, 00, 01, 00, 01, 00, 01, 00 },
    { 01, 01, 00, 00, 00, 00, 00, 00 }, { 01, 01, 01, 01, 00, 00, 00, 00 },
    { 01, 01, 01, 01, 01, 01, 00, 00 }, { 01, 01, 01, 01, 01, 01, 01, 01 }
};

//--------------------------------
// Read messages strings
//--------------------------------
bool ReadLanguageFile()
{
	FILE *pFile = NULL;

	fopen_s(&pFile, "Language.txt", "rt");
	if (!pFile)
		return false;

	char szTMP[1024];
	while (fgets(szTMP, 1024, pFile))
	{
		if (*szTMP == '#')
			continue;

		int len = strlen(szTMP);
		for (int i = 0; i < len; i++)
		{
			if (szTMP[i] == '\t')
				szTMP[i] = ' ';
		}

		int index = -1;
		char message[128];


		int ret = sscanf_s(szTMP, "%d %*s %[^\n]", &index, message, 128);
		if (ret != 2)
			continue;

		strncpy_s(g_pLanguageString[index], message, 128);
	}

	fclose(pFile);
	return true;
}


int func_4012DA(int arg)
{
	int local1 = 0;
	if ((arg / 10) > 5)
		return 1;

	return 0;
}
// ----
// Retorna um namero aleatario
// ----
int Rand()
{
	static bool started = false;
	if (!started)
	{
		std::srand(std::time(nullptr));

		started = true;
	}

	if (sServer.NewRandomMode)
		return std::rand();

	static long long i = 115;
	i = ((i * 214013) + 253101111);

	return ((i >> 16) & 0x7FFF);
}

// ----
// La o arquivo de configuraaao com o IP/Porta de conexao
// ----
bool LoadConfig()
{
	FILE *pFile = NULL;

	fopen_s(&pFile, "serverip.txt", "rt");
	if (!pFile)
		return false;

	char szTMP[256];
	while (fgets(szTMP, 256, pFile))
	{
		if (*szTMP == '#' || *szTMP == '\n')
			continue;

		char cmd[32], val[32];
		int ret = sscanf_s(szTMP, "%[^=]=%[^\n]", cmd, 32, val, 32);
		if (ret != 2)
			continue;

		if (!strcmp(cmd, "Porta"))
			sServer.Config.Porta = atoi(val);
		else if (!strcmp(cmd, "IP"))
			strncpy_s(sServer.Config.IP, 32, val, 32);
	}

	fclose(pFile);
	return true;
}

bool LoadDataServer()
{
	char line[1024];
	FILE *hFile = NULL;

	fopen_s(&hFile, "dataserver.txt", "r");
	
	if (!hFile)
		return false;

	while (fgets(line, sizeof(line), hFile))
	{
		if (*line == '#' || *line == '\n')
			continue;

		char cmd[32], val[32];
		int ret = sscanf_s(line, "%[^=]=%[^\n]", cmd, 32, val, 32);

		if (ret != 2)
			continue;

		if (!strcmp(cmd, "Porta"))
			sServer.Data.Porta = atoi(val);
		else if (!strcmp(cmd, "IP"))
			strncpy_s(sServer.Data.IP, 16, val, 16);
	}

	fclose(hFile);
	return true;
}

bool ReadInitItem()
{
	FILE *pFile = NULL;
	char line[1024];

	fopen_s(&pFile, "InitItem.csv", "r");
	if(pFile)
	{
		while (1)
		{
			char *ret = fgets(line, 1024, pFile);

			if (ret == 0)
				break;

			for (int i = 0; i < 1024; i++)
				if (line[i] == ',')
					line[i] = ' ';

			int index = -1;
			int posX = 0;
			int posY = 0;
			int rotate = 0;

			sscanf_s(line, "%d %d %d %d", &index, &posX, &posY, &rotate);

			if (index == -1)
				continue;

			if(index == 773)
				index = index;

			g_pInitItemFile[sServer.InitCount].sIndex = index;
			g_pInitItemFile[sServer.InitCount].PosX = posX;
			g_pInitItemFile[sServer.InitCount].PosY = posY;
			g_pInitItemFile[sServer.InitCount].Rotate = rotate;
			sServer.InitCount++;
		}

		fclose(pFile);
		return true;
	}
	
	return false;
}

bool ReadGuild()
{
	FILE *fp; 
	fopen_s(&fp, "guild_zone.txt", "rt");

    if(fp == NULL)
	{
		memset(g_pCityZone, 0, sizeof(g_pCityZone));
        return false;
    }

    int zone_index, ret;
    STRUCT_GUILDZONE zone;
	char tmp[1024];

    while(fgets(tmp, sizeof(tmp), fp))
    {
        if(tmp[0] == '\n' || tmp[0] == '#')
            continue;

        zone_index = -1;
        memset(&zone, 0, sizeof(zone));

		ret = sscanf_s(tmp, "%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%I64d,%d,%d",
				&zone_index, &zone.owner_index, &zone.chall_index, &zone.chall_index_2, &zone.tower_x, &zone.tower_y, &zone.area_guild_x,
				&zone.area_guild_y, &zone.city_x, &zone.city_y, &zone.city_min_x,
				&zone.city_min_y, &zone.city_max_x, &zone.city_max_y, &zone.area_guild_min_x,
				&zone.area_guild_min_y, &zone.area_guild_max_x, &zone.area_guild_max_y, &zone.war_min_x,
				&zone.war_min_y, &zone.war_max_x, &zone.war_max_y, &zone.guilda_war_x,
				&zone.guilda_war_y, &zone.guildb_war_x, &zone.guildb_war_y, &zone.impost, &zone.perc_impost, &zone.win_count);

		if (zone_index >= 0 && zone_index < 5)
		{
			memcpy(&g_pCityZone[zone_index], &zone, sizeof(STRUCT_GUILDZONE));

			if (sServer.Channel >= 1 && sServer.Channel <= 10)
				ChargedGuildList[sServer.Channel - 1][zone_index] = zone.owner_index;
		}
    }

	fclose(fp);
	return true;
}

bool LoadGuild()
{
	FILE *pFile = NULL;
	fopen_s(&pFile, "..\\DBsrv\\guilds.txt", "rt");

	if (!pFile)
	{
		MessageBox(NULL, "Can't open guilds.txt", "-system", MB_OK | MB_APPLMODAL);
		return false;
	}
	//memset(g_pGuild, 0, sizeof g_pGuild);
	char str[256];
	while(true)
	{
		char *ret = fgets(str, 255, pFile);
		if (ret == NULL)
			break;

		if(str[0] == '\n' || str[0] == '#')
			continue;

		INT32 fameGuild = 0,
			  guildId   = 0,
			  kingdom   = 0,
			  citizen   = 0,
			  wins		= 0;

		char guildName[32] = { 0, },
			 subName  [ 3][32];

		for(INT32 i = 0; i < 3; i++)
			memset(subName, 0, 32 * 3);

		memset(guildName, 0, 32);

		INT32 rtn = sscanf_s(str, "%d, %d, %d, %d, %d, %[^,], %[^,], %[^,], %[^,]", &guildId, &fameGuild, &kingdom, &citizen, &wins,
			guildName, 16, subName[0], 16, subName[1], 16,subName[2], 16);

		if(rtn < 5)
		{
			MessageBoxA(NULL, "Can't parse strings on Guilds.txt", NULL, MB_OK);
			continue;
		}

		if(guildId < 0 || guildId >= MAX_GUILD)
			continue;

		if(fameGuild < 0)
			fameGuild = 0;

		g_pGuild[guildId].Citizen = citizen;
		g_pGuild[guildId].Fame    = fameGuild;
		g_pGuild[guildId].Kingdom = kingdom;
		g_pGuild[guildId].Wins	  = wins;
		
		g_pGuild[guildId].MobName = std::string(guildName);

		for(INT32 i = 0; i < 3; i++)
		{
			if(!subName[i][0])
				continue;

			g_pGuild[guildId].SubGuild[i] = std::string(subName[i]);
		}
	}

	fclose(pFile);
	return true;
}

void ApplyAttribute(char *pHeight, int size)
{
	int endx = g_HeightPosX + size;
	int endy = g_HeightPosY + size;

	int xx = 0;
	int yy = 0;

	for(int y = g_HeightPosY; y < endy; y++)
	{
		for(int x = g_HeightPosX; x < endx; x++)
		{
			xx = (x >> 2) & 0x3FF;
			yy = (y >> 2) & 0x3FF;

			eMapAttribute att = g_pAttributeMap[yy][xx];

			if (att.Value & 2)
				pHeight[x + g_HeightWidth * (y - g_HeightPosY) - g_HeightPosX] = 127;
		}
	}
}

bool ReadHeightMap()
{
	int Handle;
	_sopen_s(&Handle, "./heightmap.dat", _O_RDONLY | _O_BINARY, _SH_DENYNO, _S_IREAD | _S_IWRITE);

	if (Handle == -1)
		return false;

	_read(Handle, (void*)g_pHeightGrid, sizeof(g_pHeightGrid));
	_close(Handle);
	return true;
}

bool LoadQuiz()
{
	FILE *pFile = NULL;
	fopen_s(&pFile, "Data\\Quiz.txt", "rt");

	if (!pFile)
		return false;

	STRUCT_QUIZQUESTIONS* last = nullptr;

	char szTMP[256] = { 0 };
	while (fgets(szTMP, 256, pFile))
	{
		if (*szTMP == '\n')
			continue;

		if (*szTMP == '#')
		{
			sServer.QuizQuestions.push_back(STRUCT_QUIZQUESTIONS{});
			last = &sServer.QuizQuestions.back();
		}

		char cmd[128] = { 0 }, val[128] = { 0 };
		int ret = sscanf_s(szTMP, "%[^=]=%[^\n]", cmd, 127, val, 127);
		if (ret != 2)
			continue;

		if (last == nullptr)
			continue;

		std::string command{ cmd };

		if (command == "Question")
			last->Question = val;
		else
		{
			int index;
			if (sscanf_s(cmd, "Answer%d", &index) != 1)
				continue;

			if (index >= 0 && index < 4)
				last->Answers[index] = val;
		}
	}

	fclose(pFile);
	return true;
}


bool ReadAttributeMap()
{
    FILE *pFile = NULL; 
	
	fopen_s(&pFile, "AttributeMap.dat", "rb");

    if(!pFile)
		return false;

    fread(g_pAttributeMap, 1024, 1024, pFile);
    fclose(pFile);

    return true;
}

bool ReadGameConfigv2()
{
	const std::string filename = "gameconfig_v2.xml";
	if (!std::filesystem::exists(filename))
		return false;

	pugi::xml_document doc;
	pugi::xml_parse_result result = doc.load_file(filename.c_str(), pugi::parse_full);
	if (!result)
	{
		std::cout << "parsed with errors, attr value: [" << doc.child("node").attribute("attr").value() << "]\n";
		std::cout << "Error description: " << result.description() << "\n";
		std::cout << "Error offset: " << result.offset << " (error at [..." << (doc.text().as_string() + result.offset) << "]\n\n";

		return false;
	}

	auto configNode = doc.child("configs");

	{
		auto generalNode = configNode.child("general");

		if (generalNode != nullptr)
		{
			sServer.RvR.Hour = generalNode.attribute("rvrHour").as_int();
			sServer.TowerWar.Hour = generalNode.attribute("towerHour").as_int();
			sServer.WeekHour = generalNode.attribute("cityHour").as_int();
			sServer.CastleHour = generalNode.attribute("noatunHour").as_int();
			sServer.MaxWaterEntrance = generalNode.attribute("maxWater").as_int();

			if (generalNode.child_value("cliver") != nullptr)
			{
				if(!std::string(generalNode.child_value("cliver")).empty())
					sServer.CliVer = std::stoi(generalNode.child_value("cliver"));
			}
		}
	}

	{
		auto kefra = configNode.child("kefra");

		sServer.FirstKefra = kefra.attribute("firstKefra").as_int();
		sServer.KefraKiller = kefra.attribute("kefraKiller").as_int();
		sServer.KefraDead = kefra.attribute("kefraDead").as_int();
	}

	{
		auto kingdomBattle = configNode.child("kingdomBattle");
		if (kingdomBattle != nullptr)
			sServer.KingdomBattle.Winner = kingdomBattle.attribute("winner").as_int();
	}

	{
		auto tower = configNode.child("towerWar");
		if (tower != nullptr)
			sServer.TowerWar.Guild = tower.attribute("guild").as_int();
	}

	{
		auto lanhouse = configNode.child("lanhouseN");
		if (lanhouse != nullptr)
			sServer.LanHouseN.TotalToKill = lanhouse.attribute("totalToKill").as_int();
	}

	{
		auto lanhouse = configNode.child("lanhouseM");
		if (lanhouse != nullptr)
			sServer.LanHouseM.TotalToKill = lanhouse.attribute("totalToKill").as_int();
	}

	{
		auto pesaLevel = configNode.child("nightmare");
		if (pesaLevel != nullptr)
			sServer.MaximumPesaLevel = pesaLevel.attribute("maximumPesaLevel").as_int();
	}

	return true;
}

bool WriteGameConfigv2()
{
	pugi::xml_document doc;
	auto configNode = doc.append_child("configs");

	{
		auto generalNode = configNode.append_child("general");

		generalNode.append_attribute("rvrHour").set_value(sServer.RvR.Hour);
		generalNode.append_attribute("towerHour").set_value(sServer.TowerWar.Hour);
		generalNode.append_attribute("noatunHour").set_value(sServer.CastleHour);
		generalNode.append_attribute("cityHour").set_value(sServer.WeekHour);
		generalNode.append_attribute("maxWater").set_value(sServer.MaxWaterEntrance);

		generalNode.append_child("cliver").append_child(pugi::node_pcdata).set_value(std::to_string(sServer.CliVer).c_str());

	}

	{
		auto kefra = configNode.append_child("kefra");

		kefra.append_attribute("firstKefra").set_value(sServer.FirstKefra);
		kefra.append_attribute("kefraKiller").set_value(sServer.KefraKiller);
		kefra.append_attribute("kefraDead").set_value(sServer.KefraDead);
	}

	{
		auto kingdomBattle = configNode.append_child("kingdomBattle");
		kingdomBattle.append_attribute("winner").set_value(sServer.KingdomBattle.Winner);
	}

	{
		auto warTower = configNode.append_child("towerWar");
		warTower.append_attribute("guild").set_value(sServer.TowerWar.Guild);
	}

	{
		auto lanhouse = configNode.append_child("lanhouseN");
		lanhouse.append_attribute("totalToKill").set_value(sServer.LanHouseN.TotalToKill);
	}

	{
		auto lanhouse = configNode.append_child("lanhouseM");
		lanhouse.append_attribute("totalToKill").set_value(sServer.LanHouseM.TotalToKill);
	}

	{
		auto pesaLevel = configNode.append_child("nightmare");
		pesaLevel.append_attribute("maximumPesaLevel").set_value(sServer.MaximumPesaLevel);
	}

	doc.save_file("gameconfig_v2.xml");
	return true;
}

bool ReadGameConfig()
{
	FILE *pFile = NULL;

	fopen_s(&pFile, "gameconfig.txt", "r");
	if(pFile)
	{
		bool modeDrop = false;
		bool modeBonusExp = false;
		bool modeRef = false;
		INT32 mode = -1;
		int totalDrop = 0;

		char line[1024];
		while(fgets(line, 1024, pFile))
		{
			if(*line == '#' || *line == '\n')
				continue;

			char cmd[1024];

			int ret = sscanf_s(line, "%[^\n]", cmd, 1024);
			if(!modeDrop && !modeBonusExp && !modeRef)
			{
				if(!strcmp(cmd, "Game server drop"))
					modeDrop = true;
				else if(!strcmp(cmd, "Banus experiancia"))
					modeBonusExp = true;
				else if(!strcmp(cmd, "Rates Refinaaao Abenaoada"))
					modeRef = true;
				else if(!strncmp(cmd, "NewbieZone", 10))
				{
					int level = 0;
					sscanf_s(line, "%*[^=]=%d", &level);
					sServer.NewbieZone = level;
				}
				else if(!strncmp(cmd, "Channel", 7))
				{
					int level = 0;
					sscanf_s(line, "%*[^=]=%d", &level);
					sServer.Channel = level;
				}
				else if(!strncmp(cmd, "TotalChannel", 12))
				{
					int level = 0;
					sscanf_s(line, "%*[^=]=%d", &level);
					sServer.TotalServer = level;
				}
				else if(!strncmp(cmd, "GuildHour", 9))
				{
					int aux = 0;
					sscanf_s(line, "%*[^=]=%d", &aux);
					sServer.GuildHour = aux;
				}
				else if(!strncmp(cmd, "GuildDay", 10))
				{
					int aux = 0;
					sscanf_s(line, "%*[^=]=%d", &aux);
					sServer.GuildDay = aux;
				}
				
				else if(!strncmp(cmd, "NewbieHour", 10))
				{
					int aux = 0;
					sscanf_s(line, "%*[^=]=%d", &aux);
					sServer.NewbieHour = aux;
				}
				else if(!strncmp(cmd, "Coloseum", 8))
				{
					int aux = 0 , aux2 = 0;
					sscanf_s(line, "%*[^=]=%d %d", &aux, &aux2);
					sServer.BRItem = aux;
					sServer.BRHour = aux2;
				}
				else if(!strncmp(cmd, "StatSapphire", 10))
				{
					int aux = 0;
					sscanf_s(line, "%*[^=]=%d", &aux);
					sServer.StatSapphire = aux;
				}
			}
			else
			{
				if(modeDrop)
				{
					int value[10];
					int ret = sscanf_s(line, "%d %d %d %d %d %d %d %d %d %d", &value[0], &value[1], &value[2], &value[3], &value[4], &value[5], &value[6], &value[7], &value[8], &value[9]);

					for(int i = totalDrop, x = 0; i < (totalDrop + ret) || x < 10; i++, x++)
					{
						if(i > 63)
							break;

						Taxes[i] = value[x];
					}
					totalDrop += ret;

					if(totalDrop >= 63)
					{
						int total = 0;
						for(int i = 0 ; i < 64; i++)
							total += Taxes[i];

						total /= 64;

						modeDrop = false;
					}
				}
				else if(modeBonusExp)
				{
					int val = 0;
					int ret =  sscanf_s(line, "%d", &val);

					if(ret != 1)
					{
						modeBonusExp = false;
						continue;
					}

					sServer.BonusEXP = val;

					modeBonusExp = false;
				}
				else if(modeRef)
				{
					int index = 0, ref = 0;
					int ret = sscanf_s(line, "%d=%d", &index, &ref);

					sServer.RateRef[index] = ref;

					if(index == 4)
						modeRef = false;
				}
			}
		}

		if(sServer.TotalServer == 0)
			sServer.TotalServer = 1;

		fclose(pFile);
		return true;
	}
	
	if(sServer.TotalServer == 0)
		sServer.TotalServer = 1;

	return false;
}

bool ReadNPCBase()
{
    static const char* npc_name[39] = {
        "Condor", "Javali", "Lobo", "Urso", "Tigre",
        "Gorila", "Dragao_Negro", "Succubus", "", "", "Porco",
        "Javali_", "Lobo_", "Dragao_Menor", "Urso_",
        "Dente_de_Sabre", "Sem_Sela", "Fantasma", "Leve",
        "Equipado", "Andaluz", "Sem_Sela_", "Fantasma_",
        "Leve_", "Equipado_", "Andaluz_", "Fenrir", "Dragao",
        "Grande_Fenrir", "Tigre_de_Fogo", "Dragao_Vermelho",
        "Unicornio", "Pegasus", "Unisus", "Grifo", "Hippo_Grifo",
        "Grifo_Sangrento", "Sleipnir", "Svadilfari"
    };
	
	for(int i = 0 ; i < 39; i++) 
	{
		if(!*npc_name[i])
			continue;

		FILE *pFile = NULL;
		
		char szTMP[1024];
		sprintf_s(szTMP, "npc_base/%s", npc_name[i]);

		fopen_s(&pFile, szTMP, "rb");

		if(pFile) 
		{
			fread(&NPCBase[i], 1, sizeof STRUCT_MOB, pFile);

			fclose(pFile);
		}
		else 
		{
			sprintf_s(szTMP, "Falha ao ler arquivo: %s", npc_name[i]);

			MessageBoxA(NULL, szTMP, szTMP, MB_OK);
		}
	}

	return true;
}

bool SaveGuildZone()
{
	FILE *pFile = NULL;
	fopen_s(&pFile, "guild_zone.txt", "w+");

	if (pFile)
	{
		for(INT32 i = 0; i < 5; i++)
		{
			STRUCT_GUILDZONE *zone= &g_pCityZone[i]; 

			fprintf(pFile, "%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%I64d,%d,%d\n",
				i, ChargedGuildList[sServer.Channel - 1][i], zone->chall_index, zone->chall_index_2, zone->tower_x, zone->tower_y, zone->area_guild_x,
				zone->area_guild_y, zone->city_x, zone->city_y, zone->city_min_x,
				zone->city_min_y, zone->city_max_x, zone->city_max_y, zone->area_guild_min_x,
				zone->area_guild_min_y, zone->area_guild_max_x, zone->area_guild_max_y, zone->war_min_x,
				zone->war_min_y, zone->war_max_x, zone->war_max_y, zone->guilda_war_x,
				zone->guilda_war_y, zone->guildb_war_x, zone->guildb_war_y, zone->impost, zone->perc_impost, zone->win_count);
		}

		fclose(pFile);
		return true;
	}

	return false;
}

int GetEmptyUser()
{
	for (int i = 1; i < MAX_PLAYER; i++)
	{
		if (pUser[i].Status == USER_EMPTY)
			return i;
	}

	return NULL;
}

CUser *GetUserBySocket(DWORD socket)
{
	for (int i = 1; i < MAX_PLAYER; i++)
	{
		if (pUser[i].Socket.Socket == socket)
			return &pUser[i];
	}

	return NULL;
}

bool SendClientMessage(int clientId, const char *msg, ...)
{
	if(clientId <= 0 || clientId >= MAX_PLAYER)
		return false;

	/* Arglist */
	char buffer[256];
	va_list arglist;
	va_start(arglist, msg);
	vsprintf_s(buffer, msg, arglist);
	va_end(arglist);
	/* Fim arlist */

	p101 packet;
	memset(&packet, 0, sizeof packet);

	packet.Header.ClientId = 0;
	packet.Header.PacketId = 0x101;
	buffer[127] = '\0';

	strncpy_s(packet.Msg, buffer, 128);

	if(buffer[0] != '.')
		Log(clientId, LOG_INGAME, "[Mensagem do servidor]> %s", buffer);

	return pUser[clientId].AddMessage((BYTE*)&packet, sizeof packet);
}

void GridMulticast_2(short posX, short posY, BYTE *sendPak, int sIndex)
{
	int VisX = VIEWGRIDX, VisY = VIEWGRIDY,
		minPosX = (posX - HALFGRIDX),
		minPosY = (posY - HALFGRIDY);

	if((minPosX + VisX) >= 4096)
		VisX = (VisX - (VisX + minPosX - 4096));

	if((minPosY + VisY) >= 4096)
		VisY = (VisY - (VisY + minPosY - 4096));

    if(minPosX < 0)
	{
		minPosX = 0;
		VisX = (VisX + minPosX);
	}

	if(minPosY < 0)
	{
		minPosY = 0;
		VisY = (VisY + minPosY);
	}

    int maxPosX = (minPosX + VisX),
        maxPosY = (minPosY + VisY);

    for(int nY = minPosY; nY < maxPosY; nY++)
    {
        for(int nX = minPosX; nX < maxPosX; nX++)
        {
            short mobID = g_pMobGrid[nY][nX];
            if(mobID <= 0 || sIndex == mobID)
                continue;

            if(sendPak == NULL || mobID >= MAX_PLAYER)
				continue;

			if (pMob[mobID].Mode == 0)
				continue;

			if(*(WORD*)&sendPak[4] == 0x338)
			{
				*(INT64*)&sendPak[24] = pMob[mobID].Mobs.Player.Exp;
				*(UINT32*)&sendPak[12] = static_cast<int>(pMob[mobID].Mobs.Hold);
			}

			pUser[mobID].AddMessage(sendPak, *(short*)&sendPak[0]);
        }
    }
}


void GridMulticast(int sIndex, unsigned int posX, unsigned int posY, BYTE *buf)
{
    if(sIndex <= 0 || sIndex >= MAX_MOB)
        return ;

	if(sIndex == 0 || pMob[sIndex].Target.X == 0)
		return;
	
	CMob *mob = (CMob*)&pMob[sIndex];

	INT32 mobId = g_pMobGrid[mob->Target.Y][mob->Target.X];
	if(mobId == sIndex && mobId != 0)
		g_pMobGrid[mob->Target.Y][mob->Target.X] = 0;

	if (g_pMobGrid[posY][posX] != sIndex && g_pMobGrid[posY][posX] != 0)
		GetEmptyMobGrid(sIndex, &posX, &posY);

	if (g_pMobGrid[posY][posX] != sIndex && g_pMobGrid[posY][posX] != 0)
	{
		if (sIndex < MAX_PLAYER)
			Log(sIndex, LOG_INGAME, "PC step in other mob's grid", "-system", 0);
	}

	g_pMobGrid[posY][posX] = sIndex;
		
	int VisX = VIEWGRIDX, VisY = VIEWGRIDY,
        minPosX = (mob->Target.X - HALFGRIDX),
		minPosY = (mob->Target.Y - HALFGRIDY);


	if((minPosX + VisX) >= 4096)
		VisX = (VisX - (VisX + minPosX - 4096));

	if((minPosY + VisY) >= 4096)
		VisY = (VisY - (VisY + minPosY - 4096));

    if(minPosX < 0)
	{
		minPosX = 0;
		VisX = (VisX + minPosX);
	}

	if(minPosY < 0)
	{
		minPosY = 0;
		VisY = (VisY + minPosY);
	}

    int maxPosX = (minPosX + VisX),
        maxPosY = (minPosY + VisY);

	int dVisX = VIEWGRIDX, dVisY = VIEWGRIDY,
		dminPosX = (posX - HALFGRIDX),
		dminPosY = (posY - HALFGRIDY);

	if((dminPosX + dVisX) >= 4096)
		dVisX = (dVisX - (dVisX + dminPosX - 4096));

	if((dminPosY + dVisY) >= 4096)
		dVisY = (dVisY - (dVisY + dminPosY - 4096));

    if(dminPosX < 0)
	{
		dminPosX = 0;
		dVisX = (dVisX + dminPosX);
	}

	if(dminPosY < 0)
	{
		dminPosY = 0;
		dVisY = (dVisY + dminPosY);
	}

    int dmaxPosX = (dminPosX + dVisX),
        dmaxPosY = (dminPosY + dVisY);

    for(int nY = minPosY; nY < maxPosY; nY++)
    {
        for(int nX = minPosX; nX < maxPosX; nX++)
        {
            short mobID = g_pMobGrid[nY][nX];
			if (mobID > 0 && sIndex != mobID)
			{
				if (buf != NULL && mobID < MAX_PLAYER)
					pUser[mobID].AddMessage(buf, *(short*)&buf[0]);

				if (nX < dminPosX || nX >= dmaxPosX ||
					nY < dminPosY || nY >= dmaxPosY)
				{
					if (mobID < MAX_PLAYER)
						SendRemoveMob(mobID, sIndex, 0, 0); //SendSignalParm(mobID, sIndex, 0x165, 0);

					if (sIndex < MAX_PLAYER)
						SendRemoveMob(sIndex, mobID, 0, 0);
				}
			}

			WORD item = g_pItemGrid[nY][nX];
			if (item != 0)
			{
				if (nX < dminPosX || nX >= dmaxPosX ||
					nY < dminPosY || nY >= dmaxPosY)
				{
					if (item >= 0 && item < 4096 && sIndex < MAX_PLAYER)
						SendRemoveItem(sIndex, item, 0);
				}
			}
        }
    }

    for(int nY = dminPosY; nY < dmaxPosY; nY++)
    {
        for(int nX = dminPosX; nX < dmaxPosX; nX++)
        {
            short mobID = g_pMobGrid[nY][nX];
            short initID = g_pItemGrid[nY][nX];

            if(nX < minPosX || nX >= maxPosX ||
			   nY < minPosY || nY >= maxPosY)
            {
                if(mobID > 0 && sIndex != mobID)
                {
					if(pMob[mobID].Mode == 0)
					{
						g_pMobGrid[nY][nX] = 0;

						Log(SERVER_SIDE, LOG_ERROR, "MOB GRID HAS EMPTY MOB - %s %dx %dy", pMob[mobID].Mobs.Player.MobName, pMob[mobID].Target.X, pMob[mobID].Target.Y);
						continue;
					}

                    if(sIndex < MAX_PLAYER)
						SendCreateMob(sIndex, mobID);

                    if(mobID < MAX_PLAYER)
						SendCreateMob(mobID, sIndex);
			
                    if(buf != NULL && mobID < MAX_PLAYER)
						pUser[mobID].AddMessage(buf, *(short*)&buf[0]);
                }
				
				if(initID > 0 && sIndex > 0 && sIndex < MAX_PLAYER)
					SendCreateItem(sIndex, initID, 0);
				
				if(initID > 0 && mobID > 0 && mobID < MAX_PLAYER)
					SendCreateItem(mobID, initID, 0);
            }
        }
	}
	
	p36C *LOCAL_85 = (p36C*)buf;
//	memset(LOCAL_85, 0, sizeof p36C);
	
	mob->Last.Time = LOCAL_85->Header.TimeStamp;
	mob->Last.Speed = LOCAL_85->Speed;
	mob->Last.X = LOCAL_85->PosX;
	mob->Last.Y = LOCAL_85->PosY;

    mob->Target.X = posX;
    mob->Target.Y = posY;
}

void Teleportar(int clientId, unsigned int posX, unsigned int posY)
{
	INT32 LOCAL_1 = GetEmptyMobGrid(clientId, &posX, &posY);
	if (!LOCAL_1)
	{
		if (clientId < MAX_PLAYER)
			Log(clientId, LOG_INGAME, "Falha ao encontrar espaao vago no mapa para o usuario. Posiaao: %ux %uy", posX, posY);

		return;
	}

	p36C packet{};
	GetAction(clientId, posX, posY, &packet);

	packet.stEffect = 1;

	if (clientId < MAX_PLAYER)
	{
		pUser[clientId].AddMessage((BYTE*)&packet, sizeof packet);
		Log(clientId, LOG_INGAME, "Teleportado para a posiaao %u %u", posX, posY);
	}

	GridMulticast(clientId, posX, posY, (BYTE*)&packet);
}

short get_effect_index(const char *s)
{
    int i;
    for(i = 0; i < 256; i++)
        if(!strcmp(EffectName[i], s))
            return i;

    return atoi(s);
}

bool ReadItemEffect()
{
	memset(EffectName, 0, sizeof(EffectName));
	FILE *fp;
	fopen_s(&fp, "ItemEffect.h", "rt");
    if(fp == NULL)
        return false;

    int ret, index;
    const char *cmm = "#define";

    char line[1024];
    char val[64];

    while(fgets(line, sizeof(line), fp))
    {
        if(strncmp(line, cmm, strlen(cmm)) == 0)
        {
            index = -1;
            *val = '\0';
            ret = sscanf_s(line, "#define %s %d", val, 31, &index);
            if(ret != 2)
                continue;

            if(index >= 0 && index < 256)
				strcpy_s(EffectName[index], val);
        }
    }

    fclose(fp);
    return true;
}

bool ReadItemList()
{
    FILE *fp = NULL;
	fopen_s(&fp, "ItemList.csv", "rt");

    if(fp == NULL)
    {
        memset(g_pItemList, 0, sizeof(g_pItemList));
        return false;
    }

	if(!ReadItemEffect())
	{
		if(fp)
			fclose(fp);

		return false;
	}

    int itemID, ret;
    STRUCT_ITEMLIST item;

    char line[1024];

    memset(g_pItemList, 0, sizeof(g_pItemList));
    while(fgets(line, sizeof(line), fp))
    {
        char meshBuf[MAX_MESH_BUFFER];
		char scoreBuf[MAX_SCORE_BUFFER] = { 0, };
        char effBuf[MAX_EFFECT][MAX_EFFECT_NAME];

        if(*line == '\n' || *line == '#')
            continue;

        memset(effBuf, 0, sizeof(effBuf));
        memset(meshBuf, 0, sizeof(meshBuf));
        memset(scoreBuf, 0, sizeof(scoreBuf));
        memset(&item, 0, sizeof(STRUCT_ITEMLIST));

        char *p = line;
        while(*p != '\0')
        {
            if(*p == ',')
                *p = ' ';
            p++;
        }

        ret = sscanf_s(line, "%d %s %s %s %hd %d %hd %hd %hd %s %hd %s %hd %s %hd %s %hd %s %hd %s %hd %s %hd %s %hd %s %hd %s %hd %s %hd %s %hd",
                     &itemID, item.ItemName, 63, meshBuf, MAX_MESH_BUFFER, scoreBuf, MAX_SCORE_BUFFER, &item.Unique, &item.Price, &item.Pos, &item.Extreme, &item.nGrade,
                     effBuf[ 0], MAX_EFFECT_NAME, &item.stEffect[ 0].sValue, effBuf[ 1], MAX_EFFECT_NAME, &item.stEffect[ 1].sValue, effBuf[ 2], MAX_EFFECT_NAME, &item.stEffect[ 2].sValue,
                     effBuf[ 3], MAX_EFFECT_NAME, &item.stEffect[ 3].sValue, effBuf[ 4], MAX_EFFECT_NAME, &item.stEffect[ 4].sValue, effBuf[ 5], MAX_EFFECT_NAME, &item.stEffect[ 5].sValue,
                     effBuf[ 6], MAX_EFFECT_NAME, &item.stEffect[ 6].sValue, effBuf[ 7], MAX_EFFECT_NAME, &item.stEffect[ 7].sValue, effBuf[ 8], MAX_EFFECT_NAME, &item.stEffect[ 8].sValue,
                     effBuf[ 9], MAX_EFFECT_NAME, &item.stEffect[ 9].sValue, effBuf[10], MAX_EFFECT_NAME, &item.stEffect[10].sValue, effBuf[11], MAX_EFFECT_NAME, &item.stEffect[11].sValue);

        if(ret < 9 || itemID <= 0 || itemID >= MAX_ITEMLIST)
            continue;

        sscanf_s(meshBuf, "%hd.%d", &item.IndexMesh, &item.nIndexTexture);
        sscanf_s(scoreBuf, "%hd.%hd.%hd.%hd.%hd", &item.Level, &item.Str, &item.Int, &item.Dex, &item.Con);

        int i;
        for(i = 0; i < MAX_EFFECT; i++)
            item.stEffect[i].sEffect = get_effect_index(effBuf[i]);

        memcpy(&g_pItemList[itemID], &item, sizeof(STRUCT_ITEMLIST));
    }

    fclose(fp);
    return true;
}


bool ReadSkillData()
{
	FILE *fp;
	fopen_s(&fp, "SkillData.csv", "rt");

	if(fp == NULL)
	{
		memset(SkillData, 0, sizeof(SkillData));
		return false;
	}

	char line[1024];
	while(fgets(line, sizeof(line), fp))
	{
		int index;
		STRUCT_SKILLDATA spell;

		if(*line == '\n' || *line == '#')
			continue;

		index = -1;
		memset(&spell, 0, sizeof(spell));

		char *p = line;
		while(*p != '\0')
		{
			if(*p == ',')
				*p = ' ';
			p++;
		}

		int ret = sscanf_s(line, "%d %d %d %d %d %d %d %d %d %d %d %d %d %*s %*s %d %d %d %d %d %d %d %d",
				&index, &spell.Points, &spell.Target, &spell.Mana, &spell.Delay, &spell.Range,
				&spell.InstanceType, &spell.InstanceValue, &spell.TickType, &spell.TickValue,
				&spell.AffectType, &spell.AffectValue, &spell.Time, &spell.InstanceAttribute,
				&spell.TickAttribute, &spell.Aggressive, &spell.Maxtarget, &spell.PartyCheck,
				&spell.AffectResist, &spell.Passive_Check, &spell.ForceDamage);

		if(ret < 19 || index < 0 || index >= 256)
			continue;

		//spell.Time >>= 2;

		memcpy(&SkillData[index], &spell, sizeof(STRUCT_SKILLDATA));
	}

	fclose(fp);
	return true;
}

bool ReadTeleport()
{
	FILE *pFile = NULL;

	fopen_s(&pFile, "Teleport.txt", "r");
	if(pFile)
	{
		int index = 0;
		char line[1024];
		while(fgets(line, 1024, pFile))
		{
			if(*line == '#' || *line == '\n')
				continue;

			int value[5] = {0, 0, 0, 0, 0};
			int ret = sscanf_s(line, "%d, %d, %d, %d, %d", &value[0], &value[1], &value[2], &value[3], &value[4]);

			if(ret != 5)
				continue;

			STRUCT_TELEPORT *m = &mTeleport[index];
			m->Price = value[4];

			m->SrcPos.X = value[0];
			m->SrcPos.Y = value[1];

			m->DestPos.X = value[2];
			m->DestPos.Y = value[3];

			index++;

			if(index == MAX_TELEPORT)
				break;
		}

		fclose(pFile);
		return true;
	}

	return false;
}

bool CanEquip(STRUCT_ITEM *pItem, STRUCT_MOB *mob, int pSlot, int pClass, p376 *p, int classMaster)
{
    if(pItem->sIndex <= 0 || pItem->sIndex >= 6500)
    { // Verifica se o id do item eh valido
        return false;
    }

    if(pSlot == 15)
    { // Verifica se o client esta tentando trocar a capa
        return false;
    }

    short ItemUnique = g_pItemList[pItem->sIndex].Unique;
    if(pSlot != -1)
    { // Verifica o slot do item
        short ItemPos = GetItemAbility(pItem, EF_POS);
        if(((ItemPos >> pSlot) & 1) == 0)
            return false; // Verifica se pode mover o item para este slot

        if(pSlot == 6 || pSlot == 7)
        { // Slot das armas/escudos
            int SrcSlot;
            if(pSlot == 6)
                SrcSlot = 7;
            else // pSlot = 7
                SrcSlot = 6;

			short SrcItemID = mob->Equip[SrcSlot].sIndex;
            if(SrcItemID > 0 && SrcItemID < 6500)
            { // Verifica o id da outra arma
                short SrcUnique = g_pItemList[SrcItemID].Unique;
				short SrcPos = GetItemAbility(&mob->Equip[SrcSlot], EF_POS);

                if(ItemPos == 64 || SrcPos == 64)
                { // Verifica se a arma usada eh de 2 maos

                    if(ItemUnique == 46)
                    { // Armas de arremeco
                        if(SrcPos != 128)
                        { // A segunda arma esta a mao do escudo
                            return false;
                        }
                    }

                    else if(SrcUnique == 46)
                    { // Armas de arremeco
                        if(ItemPos != 128)
                        { // A segunda arma esta a mao do escudo
                            return false;
                        }
                    }
                    else // Outros tipos de armas
                        return false;
                }
            }
        }
    }

	pClass = GetInfoClass(mob->Equip[0].EF2);
	if((mob->Equip[0].sIndex >= 22 && mob->Equip[0].sIndex <= 25) || mob->Equip[0].sIndex == 32)
	{
		pClass = 2;

		if(mob->Equip[0].EFV2 >= ARCH)
			pClass = GetInfoClass(mob->Equip[0].EF2);
	}

    short ItemClass = GetItemAbility(pItem, EF_CLASS);
	short pos = GetItemAbility(pItem, EF_POS);

	if(pos == 0)
	{
		Log(SERVER_SIDE, LOG_ERROR, "Item com POS 0 - ItemId: %d", pItem->sIndex);
		return false;
	}

	if(ItemClass == 0)
		return false;

	if(mob->Equip[0].EFV2 == 1)
		if(((ItemClass >> pClass) & 1) == 0)
			return false;
		else NULL;
	else 
		if(((ItemClass >> pClass) & 1) == 0 && pos <= 32)
			return false;

    // Dados do requerimento
    short ItemRLevel = GetItemAbility(pItem, EF_LEVEL);
    short ItemRSTR = GetItemAbility(pItem, EF_REQ_STR);
    short ItemRINT = GetItemAbility(pItem, EF_REQ_INT);
    short ItemRDEX = GetItemAbility(pItem, EF_REQ_DEX);
    short ItemRCON = GetItemAbility(pItem, EF_REQ_CON);
    short ItemWType = GetItemAbility(pItem, EF_WTYPE);

    ItemWType %= 10;
    int divItemWType = (ItemWType / 10);

    if(pSlot == 7 && ItemWType != 0)
    {
        int porcDim = 100;
        if(divItemWType == 0 && ItemWType > 1)
            porcDim = 130;
        else if(divItemWType == 6 && ItemWType > 1)
            porcDim = 150;

        ItemRLevel = ((ItemRLevel * porcDim) / 100);
        ItemRSTR = ((ItemRSTR * porcDim) / 100);
        ItemRINT = ((ItemRINT * porcDim) / 100);
        ItemRDEX = ((ItemRDEX * porcDim) / 100);
        ItemRCON = ((ItemRCON * porcDim) / 100);
    }

	int ItemMobType = GetEffectValueByIndex(pItem->sIndex, EF_MOBTYPE);
	if(ItemMobType != 0)
	{
		if(ItemMobType == 2)
		{
			if(classMaster == MORTAL)
			{
				if(ItemRLevel > static_cast<short>(mob->BaseScore.Level))
					return false;
			}
			else 
				return false;
		}
		else if(ItemMobType == CELESTIAL)
		{
			if(classMaster >= CELESTIAL)
				return true;

			return false;
		}
		else if(classMaster != MORTAL)
		{
			if(pSlot == 1)
			{
				if(pItem->sIndex >= 3500 && pItem->sIndex <= 3508)
					return true;

				return false;
			}
		} 
		else
			return false;
	}

	if(classMaster >= ARCH)
	{
		if(pSlot == 1)
			if((pItem->sIndex >= 3500 && pItem->sIndex <= 3508) || pItem->sIndex == 747)
				return true;
			else
				return false;
	}

	if(classMaster >= ARCH)
		return true;

    // Verificacao dos atributos do personagem
	if(ItemRSTR <= mob->CurrentScore.Str &&
       ItemRINT <= mob->CurrentScore.Int &&
       ItemRDEX <= mob->CurrentScore.Dex &&
       ItemRCON <= mob->CurrentScore.Con &&
       ItemRLevel <= static_cast<int>(mob->CurrentScore.Level))
        return true;

    return false;
}

short GetItemAbility(STRUCT_ITEM *itemPtr, int eff)
{
    int result = 0;

    int itemID = itemPtr->sIndex;

    int unique = g_pItemList[itemID].Unique;
    int pos = g_pItemList[itemID].Pos;

    if(eff == EF_DAMAGEADD || eff == EF_MAGICADD)
        if(unique < 41 || unique > 50)
            return 0;

    if(eff == EF_CRITICAL)
        if(itemPtr->stEffect[1].cEffect == EF_CRITICAL2 || itemPtr->stEffect[2].cEffect == EF_CRITICAL2)
            eff = EF_CRITICAL2;

    if(eff == EF_DAMAGE && pos == 32)
        if(itemPtr->stEffect[1].cEffect == EF_DAMAGE2 || itemPtr->stEffect[2].cEffect == EF_DAMAGE2)
            eff = EF_DAMAGE2;

    if(eff == EF_ACADD)
        if(itemPtr->stEffect[1].cEffect == EF_ACADD2 || itemPtr->stEffect[2].cEffect == EF_ACADD2)
            eff = EF_ACADD2;

    if(eff == EF_LEVEL && itemID >= 2330 && itemID < 2360)
        result = (itemPtr->stEffect[1].cEffect - 1);
    else if(eff == EF_LEVEL)
        result += g_pItemList[itemID].Level;

    if(eff == EF_REQ_STR)
        result += g_pItemList[itemID].Str;
    if(eff == EF_REQ_INT)
        result += g_pItemList[itemID].Int;
    if(eff == EF_REQ_DEX)
        result += g_pItemList[itemID].Dex;
    if(eff == EF_REQ_CON)
        result += g_pItemList[itemID].Con;

    if(eff == EF_POS)
        result += g_pItemList[itemID].Pos;

    if(eff != EF_INCUBATE)
    {
        for(int i = 0; i < 12; i++)
        {
            if(g_pItemList[itemID].stEffect[i].sEffect != eff)
                continue;

            int val = g_pItemList[itemID].stEffect[i].sValue;
            if(eff == EF_ATTSPEED && val == 1)
                val = 10;

            result += val;
            break;
        }
    }

	if(itemPtr->sIndex >= 2330 && itemPtr->sIndex < 2390)
    {
        if(eff == EF_MOUNTHP)
            return *(WORD*)&itemPtr->stEffect[0].cEffect;

        if(eff == EF_MOUNTSANC)
            return itemPtr->stEffect[1].cEffect;

        if(eff == EF_MOUNTLIFE)
            return itemPtr->stEffect[1].cValue;

        if(eff == EF_MOUNTFEED)
            return itemPtr->stEffect[2].cEffect;

        if(eff == EF_MOUNTKILL)
            return itemPtr->stEffect[2].cValue;
		
		if(itemPtr->sIndex >= 2360 && itemPtr->sIndex < 2390 && *(short*)&itemPtr->stEffect[0].cEffect > 0)
        {
			int mountIndex = itemPtr->sIndex - 2360;
			if(mountIndex < 0 || mountIndex > 29)
				return 0;

			STRUCT_MOUNTDATA mont = mMont[mountIndex];

            int ef2 = itemPtr->stEffect[1].cEffect;
            if(eff == EF_DAMAGE)
				return static_cast<short>((mont.atkFisico * (ef2 + 20) / 100));

            if(eff == EF_MAGIC)
				return static_cast<short>((mont.atkMagico * (ef2 + 15) / 100));

            if(eff == EF_PARRY)
				return static_cast<short>(mont.Evasion);

            if(eff == EF_RUNSPEED)
				return static_cast<short>(mont.speedMove);

            if(eff == EF_RESIST1 || eff == EF_RESIST2 ||
				eff == EF_RESIST3 || eff == EF_RESIST4 || eff == EF_RESISTALL)
			   return static_cast<short>(mont.Resist);
        }
		
        return result;
    }
	
	if(itemPtr->sIndex >= 3980 && itemPtr->sIndex <= 3999 && eff == EF_RUNSPEED)
		return 6;
	
    for(int i = 0; i < 3; i++)
    {
        if(itemPtr->stEffect[i].cEffect != eff)
            continue;

        int val = itemPtr->stEffect[i].cValue;
        if(eff == EF_ATTSPEED && val == 1)
            val = 10;

        result += val;
    }

    if(eff == EF_RESIST1 || eff == EF_RESIST2 ||
       eff == EF_RESIST3 || eff == EF_RESIST4)
    {
        for(int i = 0; i < 12; i++)
        {
            if(g_pItemList[itemID].stEffect[i].sEffect != EF_RESISTALL)
                continue;

            result += g_pItemList[itemID].stEffect[i].sValue;
            break;
        }

        for(int i = 0; i < 3; i++)
        {
            if(itemPtr->stEffect[i].cEffect != EF_RESISTALL)
                continue;

            result += itemPtr->stEffect[i].cValue;
            break;
        }
    }

	int sanc = GetItemSanc(itemPtr);
	if (itemPtr->sIndex <= 40)
		sanc = 0;

	if (sanc >= 9 && (pos & 0xF00) != 0)
		sanc += 1;

	if (sanc > 15)
		sanc = 15;

	if (sanc != 0 && eff != EF_GRID && eff != EF_CLASS &&
		eff != EF_POS && eff != EF_WTYPE && eff != EF_RANGE &&
		eff != EF_LEVEL && eff != EF_REQ_STR && eff != EF_REQ_INT &&
		eff != EF_REQ_DEX && eff != EF_REQ_CON && eff != EF_VOLATILE &&
		eff != EF_INCUBATE && eff != EF_INCUDELAY && eff != EF_UNKNOW1 &&
		eff != EF_MOBTYPE)
	{
		INT32 value = sanc;

		if (value == 10)
			value = 10;
		else if (value == 11)
			value = 12;
		else if (value == 12)
			value = 15;
		else if (value == 13)
			value = 18;
		else if (value == 14)
			value = 23;
		else if (value == 15)
			value = 28;

		result = result * (value + 10) / 10;
	}

	if (eff == EF_RUNSPEED)
	{
		if (result >= 3)
			result = 2;

		if (result > 0 && sanc >= 9)
			result++;
	}

	if (eff == EF_HWORDGUILD || eff == EF_LWORDGUILD)
	{
		int x = result;
		result = x;
	}

	if (eff == EF_GRID)
		if (result < 0 || result > 7)
			result = 0;

    return result;
}

short GetItemAbilityNoSanc(STRUCT_ITEM *itemPtr, int eff)
{
	int result = 0;

	int itemID = itemPtr->sIndex;

	int unique = g_pItemList[itemID].Unique;
	int pos = g_pItemList[itemID].Pos;

	if (eff == EF_DAMAGEADD || eff == EF_MAGICADD)
		if (unique < 41 || unique > 50)
			return 0;

	if (eff == EF_CRITICAL)
		if (itemPtr->stEffect[1].cEffect == EF_CRITICAL2 || itemPtr->stEffect[2].cEffect == EF_CRITICAL2)
			eff = EF_CRITICAL2;

	if (eff == EF_DAMAGE && pos == 32)
		if (itemPtr->stEffect[1].cEffect == EF_DAMAGE2 || itemPtr->stEffect[2].cEffect == EF_DAMAGE2)
			eff = EF_DAMAGE2;

	if (eff == EF_ACADD)
		if (itemPtr->stEffect[1].cEffect == EF_ACADD2 || itemPtr->stEffect[2].cEffect == EF_ACADD2)
			eff = EF_ACADD2;

	if (eff == EF_LEVEL && itemID >= 2330 && itemID < 2360)
		result = (itemPtr->stEffect[1].cEffect - 1);
	else if (eff == EF_LEVEL)
		result += g_pItemList[itemID].Level;

	if (eff == EF_REQ_STR)
		result += g_pItemList[itemID].Str;
	if (eff == EF_REQ_INT)
		result += g_pItemList[itemID].Int;
	if (eff == EF_REQ_DEX)
		result += g_pItemList[itemID].Dex;
	if (eff == EF_REQ_CON)
		result += g_pItemList[itemID].Con;

	if (eff == EF_POS)
		result += g_pItemList[itemID].Pos;

	if (eff != EF_INCUBATE)
	{
		for (int i = 0; i < 12; i++)
		{
			if (g_pItemList[itemID].stEffect[i].sEffect != eff)
				continue;

			int val = g_pItemList[itemID].stEffect[i].sValue;
			if (eff == EF_ATTSPEED && val == 1)
				val = 10;

			result += val;
			break;
		}
	}

	if (itemPtr->sIndex >= 2330 && itemPtr->sIndex < 2390)
	{
		if (eff == EF_MOUNTHP)
			return *(WORD*)&itemPtr->stEffect[0].cEffect;

		if (eff == EF_MOUNTSANC)
			return itemPtr->stEffect[1].cEffect;

		if (eff == EF_MOUNTLIFE)
			return itemPtr->stEffect[1].cValue;

		if (eff == EF_MOUNTFEED)
			return itemPtr->stEffect[2].cEffect;

		if (eff == EF_MOUNTKILL)
			return itemPtr->stEffect[2].cValue;

		if (itemPtr->sIndex >= 2360 && itemPtr->sIndex < 2390 && *(short*)&itemPtr->stEffect[0].cEffect > 0)
		{
			int mountIndex = itemPtr->sIndex - 2360;
			if (mountIndex < 0 || mountIndex > 29)
				return 0;

			STRUCT_MOUNTDATA mont = mMont[mountIndex];

			int ef2 = itemPtr->stEffect[1].cEffect;
			if (eff == EF_DAMAGE)
				return static_cast<short>((mont.atkFisico * (ef2 + 20) / 100));

			if (eff == EF_MAGIC)
				return static_cast<short>((mont.atkMagico * (ef2 + 15) / 100));

			if (eff == EF_PARRY)
				return static_cast<short>(mont.Evasion);

			if (eff == EF_RUNSPEED)
				return static_cast<short>(mont.speedMove);

			if (eff == EF_RESIST1 || eff == EF_RESIST2 ||
				eff == EF_RESIST3 || eff == EF_RESIST4 || eff == EF_RESISTALL)
				return static_cast<short>(mont.Resist);
		}

		return result;
	}

	if (itemPtr->sIndex >= 3980 && itemPtr->sIndex <= 3999 && eff == EF_RUNSPEED)
		return 6;

	for (int i = 0; i < 3; i++)
	{
		if (itemPtr->stEffect[i].cEffect != eff)
			continue;

		int val = itemPtr->stEffect[i].cValue;
		if (eff == EF_ATTSPEED && val == 1)
			val = 10;

		result += val;
	}

	if (eff == EF_RESIST1 || eff == EF_RESIST2 ||
		eff == EF_RESIST3 || eff == EF_RESIST4)
	{
		for (int i = 0; i < 12; i++)
		{
			if (g_pItemList[itemID].stEffect[i].sEffect != EF_RESISTALL)
				continue;

			result += g_pItemList[itemID].stEffect[i].sValue;
			break;
		}

		for (int i = 0; i < 3; i++)
		{
			if (itemPtr->stEffect[i].cEffect != EF_RESISTALL)
				continue;

			result += itemPtr->stEffect[i].cValue;
			break;
		}
	}

	return result;
}

int GetMaxAbility(STRUCT_MOB *usr, int eff)
{
    int MaxAbility = 0;

    for(int i = 0; i < 16; i++)
    {
		if(usr->Equip[i].sIndex == 0)
            continue;

        short ItemAbility = GetItemAbility(&usr->Equip[i], eff);
        if(MaxAbility < ItemAbility)
            MaxAbility = ItemAbility;
    }

    return MaxAbility;
}

int GetMobAbility(STRUCT_CHARINFO* usr, int eff)
{
	int value = GetMobAbility(&usr->Player, eff);
	if (eff == EF_RANGE)
		return value;
  

	return value;
}

int GetMobAbility(STRUCT_MOB *usr, int eff)
{
    int LOCAL_1 = 0;

    if(eff == EF_RANGE)
    {
        LOCAL_1 = GetMaxAbility(usr, eff);

		int LOCAL_2 = (usr->Equip[0].sIndex / 10);
        if(LOCAL_1 < 2 && LOCAL_2 == 3)
            if((usr->LearnedSkill[0] & 0x40ULL) != 0)
                LOCAL_1 = 2;

        return LOCAL_1;
    }

    int LOCAL_18[16];
    for(int LOCAL_19 = 0; LOCAL_19 < 16; LOCAL_19++)
    {
        LOCAL_18[LOCAL_19] = 0;

		int LOCAL_20 = usr->Equip[LOCAL_19].sIndex;
        if(LOCAL_20 == 0 && LOCAL_19 != 7)
            continue;

        if(LOCAL_19 >= 1 && LOCAL_19 <= 5)
            LOCAL_18[LOCAL_19] = g_pItemList[LOCAL_20].Unique;

        if(eff == EF_DAMAGE && LOCAL_19 == 6)
            continue;

        if(eff == EF_MAGIC && LOCAL_19 == 7)
            continue;

		if (LOCAL_19 == 12 && eff != EF_SAVEMANA && eff != EF_AC && eff != EF_BONUSEXPIND)
			continue;

        if(LOCAL_19 == 7 && eff == EF_DAMAGE)
        {
            int dam1 = (GetItemAbility(&usr->Equip[6], EF_DAMAGE) +
                        GetItemAbility(&usr->Equip[6], EF_DAMAGE2));
            int dam2 = (GetItemAbility(&usr->Equip[7], EF_DAMAGE) +
                        GetItemAbility(&usr->Equip[7], EF_DAMAGE2));

			int arm1 = usr->Equip[6].sIndex;
            int arm2 = usr->Equip[7].sIndex;

            int unique1 = 0;
            if(arm1 > 0 && arm1 < MAX_ITEMLIST)
                unique1 = g_pItemList[arm1].Unique;

            int unique2 = 0;
            if(arm2 > 0 && arm2 < MAX_ITEMLIST)
                unique2 = g_pItemList[arm2].Unique;

            if(unique1 != 0 && unique2 != 0)
            {
                int porc = 0;
                if(unique1 == unique2)
                    porc = 70;
                else
                    porc = 50;

                if(dam1 > dam2)
                    LOCAL_1 = ((LOCAL_1 + dam1) + ((dam2 * porc) / 100));
                else
                    LOCAL_1 = ((LOCAL_1 + dam2) + ((dam1 * porc) / 100));

                continue;
            }

            if(dam1 > dam2)
                LOCAL_1 += dam1;
            else
                LOCAL_1 += dam2;

            continue;
        }

        int LOCAL_28 = GetItemAbility(&usr->Equip[LOCAL_19], eff);
        if(eff == EF_ATTSPEED && LOCAL_28 == 1)
            LOCAL_28 = 10;

        LOCAL_1 += LOCAL_28;
    }

    if(eff == EF_AC && LOCAL_18[1] != 0)
        if(LOCAL_18[1] == LOCAL_18[2] && LOCAL_18[2] == LOCAL_18[3] &&
           LOCAL_18[3] == LOCAL_18[4] && LOCAL_18[4] == LOCAL_18[5])
            LOCAL_1 = ((LOCAL_1 * 105) / 100);

    return LOCAL_1;
}

bool CanCarry(STRUCT_ITEM* Dest, STRUCT_ITEM* Inven, int DestX, int DestY, int* error)
{
	int pItemGrid = GetItemAbility(Dest, EF_GRID);
	
	unsigned char pGridDest[8];
	memcpy(pGridDest, ItemGrid[pItemGrid], 8);

	unsigned char invSlots[MAX_INVEN];
	memset(invSlots, 0, MAX_INVEN);

	for(int i = 0; i < MAX_INVEN; i++)
    {
		if(Inven[i].sIndex == 0)
            continue;

		pItemGrid = GetItemAbility(Dest, EF_GRID);

		unsigned char pGridInv[8];
	    memcpy(pGridInv, g_pItemGrid[pItemGrid], 8);

		int pInvX = (i % 9);
        int pInvY = (i / 9);
		for(int y = 0; y < 4; y++)
        {
            for(int x = 0; x < 2; x++)
            {
                if(pGridInv[(y * 2) + x] == 0)
                    continue;

                if((y + pInvY) < 0 || (y + pInvY) >= 7)
                    continue;

                if((x + pInvX) < 0 || (x + pInvX) >= 9)
                    continue;

                invSlots[(y + pInvY) * 9 + x + pInvX] = (i + 1);
            }
        }
	}

	for(int y = 0; y < 4; y++)
    {
        for(int x = 0; x < 2; x++)
        {
            if(pGridDest[(y * 2) + x] == 0)
                continue;

            if((y + DestY) <  0 || (x + DestX) <  0 || (y + DestY) >= 7 || (x + DestX) >= 9)
            {
                *error = -1;
                return FALSE;
            }

            if(invSlots[(y + DestY) * 9 + x + DestX] == 0)
                continue;

            *error = invSlots[(y + DestY) * 9 + x + DestX];
            return FALSE;
        }
    }

	return TRUE;
}

bool CanCargo(STRUCT_ITEM *destItem, STRUCT_ITEM *Inventory, int pDestX, int pDestY)
{
    int pGrid = GetItemAbility(destItem, EF_GRID);

	unsigned char pGridDest[8];
	memcpy(pGridDest, g_pItemGrid[pGrid], 8);

	unsigned char invSlots[128];
	memset(invSlots, 0, 128);

	for(int slot = 0; slot < 128; slot++)
    {
        if(Inventory[slot].sIndex == 0)
            continue;

        pGrid =  GetItemAbility(&Inventory[slot], EF_GRID);

        unsigned char pGridInv[8];
	    memcpy(pGridInv, g_pItemGrid[pGrid], 8);

        int pInvX = (slot % 9);
        int pInvY = (slot / 9);
        for(int y = 0; y < 4; y++)
        {
            for(int x = 0; x < 2; x++)
            {
                if(pGridInv[(y * 2) + x] == 0)
                    continue;

                if((y + pInvY) < 0 || (y + pInvY) >= 14)
                    continue;

                if((x + pInvX) < 0 || (x + pInvX) >= 9)
                    continue;

                invSlots[(y + pInvY) * 9 + x + pInvX] = (slot + 1);
            }
        }
    }

    for(int y = 0; y < 4; y++)
    {
        for(int x = 0; x < 2; x++)
        {
            if(pGridDest[(y * 2) + x] == 0)
                continue;

            if((y + pDestY) <   0 || (x + pDestX) <  0 ||
               (y + pDestY) >= 14 || (x + pDestX) >= 9)
                return false;

            if(invSlots[(y + pDestY) * 9 + x + pDestX] == 0)
                continue;

            return false;
        }
    }

    return true;
}

void CharLogOut(int clientId)
{
	if(pUser[clientId].Status != USER_PLAY)
	{
		SendSignal(clientId, clientId, 0x116);

		return;
	}
	
	if(pUser[clientId].Trade.ClientId != 0 || pUser[clientId].IsAutoTrading)
		RemoveTrade(clientId);

	LogGold(clientId);

	pUser[clientId].User.Position.X = 0;
	pUser[clientId].User.Position.Y = 0;
	pUser[clientId].User.CharSlot = -1;

	// Salva
	SaveUser(clientId, 1);
	DeleteMob(clientId, 2);

	RemoveParty(clientId);

	// Seta como o usuario esta na charList
	pUser[clientId].Status = USER_SELCHAR;

	// Remove o registro da quest atual
	pUser[clientId].QuestAccess = 0;

	if(pMob[clientId].Target.Y > 0 && pMob[clientId].Target.Y < 4096 && pMob[clientId].Target.X > 0 && pMob[clientId].Target.X < 4096)
 		g_pMobGrid[pMob[clientId].Target.Y][pMob[clientId].Target.X] = MOB_EMPTY;

	pMob[clientId].Mode = 0;
	SendSignal(clientId, clientId, 0x116);

	pUser[clientId].PremierStore.CurrentScore = 0;
	pUser[clientId].PremierStore.Time   = 0;
	pUser[clientId].PremierStore.Wait   = 0;
	pUser[clientId].PremierStore.Count  = 0;

	pUser[clientId].aHack.Question = -1;
	pUser[clientId].aHack.Response = 0;
	pUser[clientId].aHack.Error    = 0;
	pUser[clientId].aHack.Next     = 60;
	pUser[clientId].aHack.Last     = sServer.SecCounter;

	pUser[clientId].MacIntegrity.WasWarned = false;
	pUser[clientId].MacIntegrity.IsChecked = false;

	sServer.Zakum.Unregister(clientId);
	std::fill(std::begin(pUser[clientId].TimeStamp.Skills), std::end(pUser[clientId].TimeStamp.Skills), std::chrono::steady_clock::time_point());
}	

void CheckIdle(int clientId)
{
	INT32 LOCAL_1 = sServer.SecCounter, 
		  LOCAL_2 = pUser[clientId].TimeStamp.LastReceiveTime;
		
	if(LOCAL_2 > LOCAL_1)
		pUser[clientId].TimeStamp.LastReceiveTime = sServer.SecCounter;

	if(LOCAL_2 < (LOCAL_1 - 1440))
		pUser[clientId].TimeStamp.LastReceiveTime = sServer.SecCounter;

	if(LOCAL_2 < (LOCAL_1 - 720))
	{
		Log(clientId, LOG_INGAME, "Desconectado por inatividade... Last: %d. Counter: %d. Status: %d", pUser[clientId].TimeStamp.LastReceiveTime, sServer.SecCounter, pUser[clientId].Status);
		LogPlayer(clientId, "Desconectado por inatividade");

		CloseUser(clientId);
	}
}

void SetBattle(int arg1, int arg2)
{
	//0x00401014 Lib: Server.obj Class : (null)

    if(arg1 <= 0     || arg2 <= 0 ||
	   arg1 >= 30000 || arg2 >= 30000)
	   return;

	if(!pMob[arg1].Mode)
		return;

	if(!pMob[arg2].Mode)
		return;

	if(arg1 < MAX_PLAYER)
	{
		if(pUser[arg1].Status != USER_PLAY)
			return;
	}
	
	if (pMob[arg1].Target.X < (pMob[arg2].Target.X - VIEWGRIDX))
	    return;
		
	if (pMob[arg1].Target.X >(pMob[arg2].Target.X + VIEWGRIDX))
		return;

	if (pMob[arg1].Target.Y < (pMob[arg2].Target.Y - VIEWGRIDY))
		return;

	if (pMob[arg1].Target.Y >(pMob[arg2].Target.Y + VIEWGRIDY))
		return;
	
	INT32 mode = pMob[arg1].Mode;

	pMob[arg1].Mode = 5;
	pMob[arg1].AddEnemyList(arg2);

	int LOCAL_1 = pMob[arg1].GenerateID,
		LOCAL_2 = Rand() & 0x80000003;

	if(mode != 5)
	{
		if (pMob[arg1].GenerateID == GUARDIAN_TOWER_BLUE)
		{
			SendNotice("A Torre Guardiao do reino Blue esta sendo atacado");
			SendKingdomBattleInfo(SERVER_SIDE, CAPE_BLUE, true);

			sServer.KingdomBattle.Info[0].CurrentScore = true;
		}
		else if (pMob[arg1].GenerateID == GUARDIAN_TOWER_RED)
		{
			SendNotice("A Torre Guardiao do reino Red esta sendo atacado");
			SendKingdomBattleInfo(SERVER_SIDE, CAPE_RED, true);

			sServer.KingdomBattle.Info[1].CurrentScore = true;
		}
		else if (pMob[arg1].GenerateID == 8) // rei blue
		{
			SendNotice("O Rei Harabard esta sendo atacado");

			SendKingdomBattleInfo(SERVER_SIDE, CAPE_BLUE, true);
			sServer.KingdomBattle.Info[0].CurrentScore = true;
		}
		else if (pMob[arg1].GenerateID == 9) // rei blue
		{
			SendNotice("O Rei Glantuar esta sendo atacado");
			SendKingdomBattleInfo(SERVER_SIDE, CAPE_RED, true);

			sServer.KingdomBattle.Info[1].CurrentScore = true;
		}

		if(LOCAL_1 >= 0 && LOCAL_1 < 8192)
		{
			if((mGener.pList[LOCAL_1].FightAction[LOCAL_2][0]))
				if(!pMob[arg1].Leader)
					SendSay(arg1, mGener.pList[LOCAL_1].FightAction[LOCAL_2]);
		}
	}
}

void SetAffect(int clientId, int skillId, int time)
{
	int tickType = SkillData[skillId].AffectType;
	int affectId = GetEmptyAffect(clientId, tickType);

	if (tickType <= 0 || (affectId < 0 && affectId >= 32))
		return;

	pMob[clientId].Mobs.Affects[affectId].Index = tickType;
	pMob[clientId].Mobs.Affects[affectId].Master = SkillData[skillId].AffectValue;
	pMob[clientId].Mobs.Affects[affectId].Time = time;
	pMob[clientId].Mobs.Affects[affectId].Value = 1;

	pMob[clientId].GetCurrentScore(clientId);
}

int SetAffect(int clientId, int skillId, int delay, int level)
{
	CMob *spw = &pMob[clientId];

	int tickType = SkillData[skillId].AffectType;
	int skillType = SkillData[skillId].Aggressive; // LOCAL2

	if(spw->Mobs.Player.AffectInfo.Resist && skillType != 0)
		return 0;
	
	int affectId = GetEmptyAffect(clientId, tickType);//LOCAL_3
	if(tickType <= 0 || (affectId < 0 && affectId >= 32))
		return 0;

	int local4 = spw->Mobs.Affects[affectId].Index;
	int aux = 1;

	spw->Mobs.Affects[affectId].Index = tickType;
	spw->Mobs.Affects[affectId].Master = SkillData[skillId].AffectValue;
	
	unsigned int time = SkillData[skillId].Time * delay / 100;

	if(local4 == tickType)
	{
		if(time > spw->Mobs.Affects[affectId].Time)
			spw->Mobs.Affects[affectId].Time = time;
	}
	else
	{
		spw->Mobs.Affects[affectId].Time = time;
		aux = 2;
	}

	if(local4 == tickType)
	{
		if(level > spw->Mobs.Affects[affectId].Value)
			spw->Mobs.Affects[affectId].Value = level;
	}
	else
		spw->Mobs.Affects[affectId].Value = level;

	pMob[clientId].GetCurrentScore(clientId);

	return aux;
}

int SetBuff(int clientId, int buffId, int master, int value, int time)
{
	return SetBuff(clientId, buffId, master, value, time, true);
}

int SetBuff(int clientId, int buffId, int master, int value, int time, bool overrideBuffTime)
{
	INT32 LOCAL_3 = GetEmptyAffect(clientId, buffId);

	if(buffId > 0 && LOCAL_3 >= 0 && LOCAL_3 < 32)
	{
		STRUCT_AFFECT *affect = pMob[clientId].Mobs.Affects;

		affect[LOCAL_3].Index = buffId;
		affect[LOCAL_3].Master = master;

		if(affect[LOCAL_3].Time > 0 && overrideBuffTime)
			affect[LOCAL_3].Time += time;
		else
			affect[LOCAL_3].Time = time;

		affect[LOCAL_3].Value = value;

		pMob[clientId].GetCurrentScore(clientId);
		return true;
	}

	return false;
}

int SetTick(int clientId, int skillId, int delay, int level)
{
	CMob *spw = &pMob[clientId];
	if(spw->Mobs.Player.Info.Merchant == 1 && clientId > MAX_PLAYER)
		return false;

	INT32 LOCAL_1 = SkillData[skillId].TickType;
	INT32 LOCAL_2 = SkillData[skillId].Aggressive;

	if((spw->Mobs.AffectInfo & 2) && LOCAL_2 != 0)
		return false;

	INT32 LOCAL_3 = GetEmptyAffect(clientId, LOCAL_1);

	if(LOCAL_1 > 0 && LOCAL_3 >= 0 && LOCAL_3 < 32)
	{
		STRUCT_AFFECT *affect = spw->Mobs.Affects;

		affect[LOCAL_3].Index = LOCAL_1;
		affect[LOCAL_3].Master = SkillData[skillId].TickValue;

		affect[LOCAL_3].Time = (delay * SkillData[skillId].Time) / 100;

		if(delay >= 10000)
			affect[LOCAL_3].Time = 10000;

		affect[LOCAL_3].Value = level;

		pMob[clientId].GetCurrentScore(clientId);
		return true;
	}

	return false;
}

void MobKilled(int arg1, int arg2, int arg3, int arg4)
{
	char temp[128];
	if(arg1 <= 0 || arg1 >= 30000)
		return;

	if(arg2 <= 0 || arg2 >= 30000)
		return;

	if(pMob[arg1].Mode == 0)
		return;

#if defined(_DEBUG)
	if (arg2 < MAX_PLAYER && arg1 > MAX_PLAYER)
		Log(arg2, LOG_INGAME, "Matou o mob %s. GenerID: %d", pMob[arg1].Mobs.Player.MobName, pMob[arg1].GenerateID);
#endif

	if (pMob[arg1].GenerateID == TORRE_RVR_BLUE || pMob[arg1].GenerateID == TORRE_RVR_RED)
	{
		if (arg2 == 29999)
		{
			DeleteMob(arg1, 1);

			return;
		}

		int connId = arg2;
		if (connId >= MAX_PLAYER)
		{
			int summonerId = pMob[connId].Summoner;

			if (summonerId > 0 && summonerId < MAX_PLAYER)
				connId = summonerId;
			else
				connId = 0;
		}

		if (connId <= MAX_PLAYER && pUser[connId].Status != USER_PLAY)
			connId = 0;

		// Ressuscita a torre
		pMob[arg1].Mobs.Player.CurrentScore.Hp = pMob[arg1].Mobs.Player.CurrentScore.MaxHp;

		p364 packet;
		GetCreateMob(arg1, (BYTE*)&packet);

		GridMulticast_2(pMob[arg1].Target.X, pMob[arg1].Target.Y, (BYTE*)&packet, 0);

		int towerId = 0;

		if (pMob[arg1].GenerateID == TORRE_RVR_BLUE)
			towerId = 1;
		else
			towerId = 0;

		sServer.RvR.Points[towerId] += 25;
		pMob[connId].Mobs.RvRPoints += 15;

		for(INT32 i = 1; i < MAX_PLAYER; i++)
		{
			if(pUser[i].Status != USER_PLAY)
				continue;

			if(pMob[i].Target.X >= 1041 && pMob[i].Target.X <= 1248 && pMob[i].Target.Y >= 1950 && pMob[i].Target.Y <= 2158)
				SendClientMessage(i, "A torre do reino %s foi destruada por %s. +25 pontos para o reino %s.", (pMob[arg1].GenerateID == TORRE_RVR_BLUE) ? "Blue" : "Red", pMob[connId].Mobs.Player.MobName, (towerId == 0) ? "Blue" : "Red");

			// RVR
			if (pMob[i].Target.X >= 1041 && pMob[i].Target.X <= 1248 &&
				pMob[i].Target.Y >= 1950 && pMob[i].Target.Y <= 2158 && sServer.RvR.CurrentScore == 1)
			{
				int cape = pMob[i].Mobs.Player.CapeInfo;

				INT32 posX  = 0;
				INT32 posY = 0;

				if (cape == CAPE_BLUE)
				{
					if (!(Rand() % 2))
					{
						posX = 1061 - Rand() % 5;
						posY = 2113 + Rand() % 5;
					}
					else
					{
						posX = 1091 - Rand() % 5;
						posY = 2140 + Rand() % 5;
					}
				}
				else
				{
					if (!(Rand() % 2))
					{
						posX = 1238 - Rand() % 5;
						posY = 1983 + Rand() % 5;
					}
					else
					{
						posX = 1211 - Rand() % 5;
						posY = 1955 + Rand() % 5;
					}
				}
				if(posX != 0 && posY != 0)
					Teleportar(i, posX, posY);
			}
		}
		
		for (int i = 0; i < MAX_PLAYER; i++)
		{
			if (pMob[i].Target.X >= 1041 && pMob[i].Target.X <= 1248 &&
				pMob[i].Target.Y >= 1950 && pMob[i].Target.Y <= 2158)
			{
				if (pUser[i].Status != USER_PLAY)
					continue;

				// Atualiza o placar de kill
				SendCounterMob(i, sServer.RvR.Points[1], sServer.RvR.Points[0]);
			}
		}

		return;
	}

	if (pMob[arg1].GenerateID == TORRE_ERION)
	{
		if (arg2 == 29999)
		{
			DeleteMob(arg1, 1);

			return;
		}

		int cId = arg2;
		if (cId >= MAX_PLAYER)
		{
			int summonerId = pMob[cId].Summoner;

			if (summonerId > 0 && summonerId < MAX_PLAYER)
				cId = summonerId;
			else
				cId = 0;
		}

		if (cId < MAX_PLAYER && pUser[cId].Status != USER_PLAY)
			cId = 0;

		INT32 guildId = pMob[cId].Mobs.Player.Guild;
		sServer.TowerWar.Guild = guildId;
			
		SendNotice("O jogador %s da guild %s derrubou a Torre de Erion.", pMob[cId].Mobs.Player.MobName, g_pGuild[guildId].MobName.c_str());

		// Ressuscita a torre
		pMob[arg1].Mobs.Player.CurrentScore.Hp = pMob[arg1].Mobs.Player.CurrentScore.MaxHp;
		pMob[arg1].Mobs.Player.Guild   = sServer.TowerWar.Guild;

		p364 packet;
		GetCreateMob(arg1, (BYTE*)&packet);

		GridMulticast_2(pMob[arg1].Target.X, pMob[arg1].Target.Y, (BYTE*)&packet, 0);
		
		ClearTowerArea(0);

		Log(SERVER_SIDE, LOG_INGAME, "A Torre foi derrubada pela guild %s (%d)", g_pGuild[guildId].MobName.c_str(), guildId);
		return;
	}
	/*
	if(arg1 < MAX_PLAYER)
	{
		Users[arg1].Potion.CountHp = pMob[arg1].Mobs.Player.CurrentScore.MaxHp;
		SetReqHp(arg1);
		pMob[arg1].Mobs.Player.CurrentScore.Hp = pMob[arg1].Mobs.Player.CurrentScore.MaxHp;
		SendScore(arg1);
		return;
	}*/

	// 0045AB5D
	STRUCT_ITEM *LOCAL_1 = &pMob[arg1].Mobs.Player.Equip[13];
	if(LOCAL_1->sIndex == 769)
	{
		INT32 LOCAL_2 = GetItemSanc(LOCAL_1);
		if(LOCAL_2 <= 0)
			memset(LOCAL_1, 0, sizeof LOCAL_1);
		else
		{
			LOCAL_2 = LOCAL_2 - 1;

			SetItemSanc(LOCAL_1, LOCAL_2, 0);
		}

		if(arg1 > 0 && arg1 < MAX_PLAYER)
		{
			SendItem(arg1, SlotType::Equip, 13, LOCAL_1);

			SendEmotion(arg1, 14, 2);

			p364 packet;
			GetCreateMob(arg1, (BYTE*)&packet);

			GridMulticast_2(pMob[arg1].Target.X, pMob[arg1].Target.Y, (BYTE*)&packet, 0);
		}
		
		SendEquip(arg1);

		pMob[arg1].Mobs.Player.CurrentScore.Hp = pMob[arg1].Mobs.Player.CurrentScore.MaxHp;
		if(arg1 < MAX_PLAYER)
		{
			SetReqHp(arg1);
			SendScore(arg1);
		}
			
		return;
	}

	INT8 pergaRessuSlot = GetFirstSlot(arg1, 3463);
	if (pergaRessuSlot != -1 && arg1 < MAX_PLAYER)
	{
		// Pergaminho da ressureiaao
		// Enviar carta avisando que mataram o jovem, e depois ressucita-lo
		AmountMinus(&pMob[arg1].Mobs.Player.Inventory[pergaRessuSlot]);
		SendItem(arg1, SlotType::Inv, pergaRessuSlot, &pMob[arg1].Mobs.Player.Inventory[pergaRessuSlot]);

		if (arg2 < MAX_PLAYER)
		{
			char szMsg[120];
			memset(szMsg, 0, 120);

			sprintf_s(szMsg, "![%s] te matou em %dx : %dy.", pMob[arg2].Mobs.Player.MobName, pMob[arg1].Target.X, pMob[arg1].Target.Y);

			SendClientMessage(arg1, szMsg);

			LogPlayer(arg1, "[%s] te matou em %dx : %dy - Pergaminho da Ressurreiaao", pMob[arg2].Mobs.Player.MobName, pMob[arg1].Target.X, pMob[arg1].Target.Y);
		}
		
		pMob[arg1].Mobs.Player.CurrentScore.Hp = pMob[arg1].Mobs.Player.CurrentScore.MaxHp;

		p364 packet;
		GetCreateMob(arg1, (BYTE*)&packet);

		GridMulticast_2(pMob[arg1].Target.X, pMob[arg1].Target.Y, (BYTE*)&packet, 0);

		SetReqHp(arg1);
		SendScore(arg1);
		return;
	}
	
	if(arg1 < MAX_PLAYER && (pMob[arg1].Mobs.Player.BaseScore.Level > 500 || (pUser[arg1].AccessLevel != 0 && pUser[arg1].IsAdmin)))
	{
		pUser[arg1].Potion.CountHp = pMob[arg1].Mobs.Player.CurrentScore.MaxHp;
		pUser[arg1].Potion.CountMp = pMob[arg1].Mobs.Player.CurrentScore.MaxMp;
		SetReqHp(arg1);

		pMob[arg1].Mobs.Player.CurrentScore.Hp = pMob[arg1].Mobs.Player.CurrentScore.MaxHp;
		pMob[arg1].Mobs.Player.CurrentScore.Mp = pMob[arg1].Mobs.Player.CurrentScore.MaxMp;

		SendScore(arg1);
		return;
	}

	if ((sServer.WeekMode == 3 || sServer.ForceWeekDay == 3) && arg1 < MAX_PLAYER) // guerra ativa
	{
		int posX = pMob[arg1].Target.X;
		int posY = pMob[arg1].Target.Y;
		for (int i = 0; i < 5; i++)
		{
			int minX = g_pCityZone[i].war_min_x;
			int minY = g_pCityZone[i].war_min_y;
			int maxX = g_pCityZone[i].war_max_x;
			int maxY = g_pCityZone[i].war_max_y;

			if (posX < minX || posY < minY || posX > maxX || posY > maxY)
				continue;

			// identifica se a guild a ou b
			int guildId = pMob[arg1].Mobs.Player.Guild;

			int zonePosX = -1;
			int zonePosY = -1;

			// verifica
			if (guildId != g_pCityZone[i].chall_index && guildId != g_pCityZone[i].chall_index_2 && guildId != ChargedGuildList[sServer.Channel - 1][i])
			{
				Log(SERVER_SIDE, LOG_INGAME, "O usuario %s (%s) foi identificado na area de guild mas nao a desafiante nem dono da cidade. GuildId: %d. %dx %dy", pMob[arg1].Mobs.Player.MobName,
					pUser[arg1].User.Username, guildId, posX, posY);

				break;
			}

			// Remove uma vida do usuario
			--pMob[arg1].Lifes;

			// esta dentro da guerra e possui vidas
			if (pMob[arg1].Lifes >= 0)
			{
				pMob[arg1].Mobs.Player.CurrentScore.Hp = pMob[arg1].Mobs.Player.CurrentScore.MaxHp;
				pMob[arg1].Mobs.Player.CurrentScore.Mp = pMob[arg1].Mobs.Player.CurrentScore.MaxMp;

				pUser[arg1].Potion.CountHp = pMob[arg1].Mobs.Player.CurrentScore.MaxHp;
				pUser[arg1].Potion.CountMp = pMob[arg1].Mobs.Player.CurrentScore.MaxMp;

				SetReqHp(arg1);

				for (int iAffect = 0; iAffect < 32; iAffect++)
				{
					if (pMob[arg1].Mobs.Affects[iAffect].Index == 32)
					{
						memset(&pMob[arg1].Mobs.Affects[iAffect], 0, sizeof STRUCT_AFFECT);

						SendAffect(arg1);
						break;
					}
				}

				SendClientMessage(arg1, "Voca possui %d vidas restantes", pMob[arg1].Lifes);
				SendScore(arg1);

				Log(arg1, LOG_INGAME, "Revivido dentro da area da Guerra. Vidas restantes: %d", pMob[arg1].Lifes);
				return;
			}
		}
	}


	if(sServer.CastleState > 1 && pMob[arg1].Target.X == 1046 && pMob[arg1].Target.Y == 1690 && arg1 > 0 && arg1 < MAX_PLAYER)
		Teleportar(arg1, 1057, 1742);
	
	INT32 LOCAL_3 = pMob[arg2].Mobs.Player.Equip[0].sIndex;

	if(arg2 >= MAX_PLAYER && pMob[arg2].Mobs.Player.CapeInfo == 4 && LOCAL_3 >= 315 && LOCAL_3 <= 345 && arg1 > MAX_PLAYER && pMob[arg1].Mobs.Player.CapeInfo != 4)
	{
		INT32 LOCAL_4 = pMob[arg2].Summoner;

		if(LOCAL_4 > 0 && LOCAL_4 < MAX_PLAYER && pUser[LOCAL_4].Status != 0 && pMob[LOCAL_4].Mode != 0)
		{
			STRUCT_ITEM *LOCAL_5 = &pMob[LOCAL_4].Mobs.Player.Equip[14];

			if(LOCAL_5->sIndex >= 2330 && LOCAL_5->sIndex < 2360)
			{
				BYTE LOCAL_6 = LOCAL_5->stEffect[1].cEffect; // EBP - 18h
				BYTE LOCAL_7 = LOCAL_5->stEffect[2].cValue; // EBP - 1Ch
				BYTE LOCAL_8 = (char)((LOCAL_6 & 255)+ 100); // EBP - 20h
				
				if (LOCAL_5->sIndex == 2330)
					LOCAL_8 = LOCAL_6 + 25;

				else if (LOCAL_5->sIndex == 2331)
					LOCAL_8 = LOCAL_6 + 35;

				else if (LOCAL_5->sIndex == 2332)
					LOCAL_8 = LOCAL_6 + 45;

				else if (LOCAL_5->sIndex == 2333)
					LOCAL_8 = LOCAL_6 + 55;

				else if (LOCAL_5->sIndex == 2334)
					LOCAL_8 = LOCAL_6 + 65;

				else if (LOCAL_5->sIndex == 2335)
					LOCAL_8 = LOCAL_6 + 75;

				if(LOCAL_6 < pMob[arg1].Mobs.Player.CurrentScore.Level && LOCAL_6 < 100)
				{
					LOCAL_7 = LOCAL_7 + 1;

					if(LOCAL_7 >= LOCAL_8)
					{
						LOCAL_7 = 1;
						LOCAL_6 = LOCAL_6 + 1;

						LOCAL_5->stEffect[2].cValue = LOCAL_7;
						LOCAL_5->stEffect[1].cEffect = LOCAL_6;

						SendClientMessage(LOCAL_4, "Sua montaria subiu de level.");

						SendItem(LOCAL_4, SlotType::Equip, 14, &pMob[LOCAL_4].Mobs.Player.Equip[14]);
						MountProcess(LOCAL_4, 0);
					}
					else
					{
						LOCAL_5->stEffect[2].cValue = LOCAL_7;
						SendItem(LOCAL_4, SlotType::Equip, 14, &pMob[LOCAL_4].Mobs.Player.Equip[14]);
					}
				}
			}
		}
	}

	// 0045AF04
	p338 LOCAL_13;
	memset(&LOCAL_13, 0, sizeof(p338));

	LOCAL_13.Header.PacketId = 0x338;
	LOCAL_13.Header.Size = sizeof p338;
	LOCAL_13.Header.ClientId = 30000;

	LOCAL_13.killed = arg1;
	LOCAL_13.killer = arg2;
	LOCAL_13.Hold = static_cast<int>(pMob[arg2].Mobs.Hold);
	LOCAL_13.Exp = pMob[arg2].Mobs.Player.Exp;

	pMob[arg1].Mobs.Player.CurrentScore.Hp = 0;


	INT32 LOCAL_14 = pMob[arg2].Leader;
	if(LOCAL_14 == 0)
		LOCAL_14 = arg2;

	// 45AF56

	if(arg2 >= MAX_PLAYER && pMob[arg2].Mobs.Player.CapeInfo == 4)
	{
		INT32 LOCAL_15 = pMob[arg2].Summoner;

		if(LOCAL_15 <= 0 || LOCAL_15 >= MAX_PLAYER || pUser[LOCAL_15].Status != 22)
		{
			GridMulticast_2(pMob[arg1].Target.X, pMob[arg1].Target.Y, (BYTE*)&LOCAL_13, 0);

			if(arg1 >= MAX_PLAYER)
				DeleteMob(arg1, 1);

			return;
		}
		else
			arg2 = LOCAL_15;
	}
 
	if (arg1 < MAX_PLAYER && arg2 < MAX_PLAYER)
	{
		if (pMob[arg1].Target.X >= 1041 && pMob[arg1].Target.X <= 1248 &&
			pMob[arg1].Target.Y >= 1950 && pMob[arg1].Target.Y <= 2158 && sServer.RvR.CurrentScore == 1)
		{
			int capeKiller = pMob[arg2].Mobs.Player.CapeInfo;
			int capeDead = pMob[arg1].Mobs.Player.CapeInfo;

			if ((capeKiller == CAPE_RED || capeKiller	== CAPE_BLUE) &&
			      (capeDead == CAPE_RED || capeDead		== CAPE_BLUE))
			{
				if (capeDead != capeKiller)
				{
					int capeId = capeKiller - 7;
					sServer.RvR.Points[capeId]++;		

					pMob[arg2].Mobs.RvRPoints++;

					for(INT32 i = 1; i < MAX_PLAYER; i++)
					{
						if(pUser[i].Status != USER_PLAY)
							continue;

						if (pMob[i].Target.X >= 1041 && pMob[i].Target.X <= 1248 &&
							pMob[i].Target.Y >= 1950 && pMob[i].Target.Y <= 2158)
						{
							SendClientMessage(i, "%s matou %s e pontuou para o reino %s", pMob[arg2].Mobs.Player.MobName, pMob[arg1].Mobs.Player.MobName, (capeId == 0) ? "Blue" : "Red");
							SendCounterMob(i, sServer.RvR.Points[1], sServer.RvR.Points[0]);
						}
					}
				}
			}
		}
	}

	if(arg1 < MAX_PLAYER && pMob[arg1].Mobs.Player.BaseScore.Level <= 399)
	{
		INT32 LOCAL_16 = pMob[arg1].Mobs.Player.BaseScore.Level;
		if(LOCAL_16 < 0 || LOCAL_16 > 399)
			return;

		INT32 evId = pMob[arg1].Mobs.Player.Equip[0].EFV2;
		INT64 LOCAL_17 = g_pNextLevel[evId][LOCAL_16];
		INT64 LOCAL_18 = g_pNextLevel[evId][LOCAL_16 + 1];

		INT64 LOCAL_19 = LOCAL_18 - LOCAL_17;
		INT64 LOCAL_20 = LOCAL_19 / 20;

		if(LOCAL_16 >= 30)
			LOCAL_20 = LOCAL_19 / 22;
		
		if(LOCAL_16 >= 40)
			LOCAL_20 = LOCAL_19 / 25;

		if(LOCAL_16 >= 50)
			LOCAL_20 = LOCAL_19 / 30;

		if(LOCAL_16 >= 60)
			LOCAL_20 = LOCAL_19 / 35;

		if(LOCAL_16 >= 70)
			LOCAL_20 = LOCAL_19 / 40;

		if(LOCAL_16 >= 80)
			LOCAL_20 = LOCAL_19 / 45;

		if(LOCAL_16 >= 90)
			LOCAL_20 = LOCAL_19 / 50;

		if(LOCAL_16 >= 100)
			LOCAL_20 = LOCAL_19 / 55;

		if(LOCAL_16 >= 150)
			LOCAL_20 = LOCAL_19 / 70;

		if(LOCAL_16 >= 200)
			LOCAL_20 = LOCAL_19 / 85;

		if(LOCAL_16 >= 250)
			LOCAL_20 = LOCAL_19 / 100;

		// 0045B154
		if(LOCAL_20 < 0)
			LOCAL_20 = 0;

		if(LOCAL_20 > 30000)
			LOCAL_20 = 30000;

		INT32 LOCAL_21 = GetCurKill(arg2);
		INT32 LOCAL_22 = GetTotKill(arg2);
		INT32 LOCAL_23 = GetPKPoint(arg2);
		INT32 LOCAL_24 = GetPKPoint(arg1);
		INT32 LOCAL_25 = GetGuilty(arg1);
		INT32 LOCAL_26 = GetCurKill(arg1);
		
		INT32 LOCAL_27 = pMob[arg1].Mobs.Player.CapeInfo;
		INT32 LOCAL_28 = pMob[arg2].Mobs.Player.CapeInfo; 

		INT32 LOCAL_29 = 0;
		INT32 LOCAL_30 = 0;

		if(LOCAL_24 <= 0)
			LOCAL_20 = LOCAL_20 * 5;
		else if(LOCAL_24 <= 25)
			LOCAL_20 = LOCAL_20 * 3;

		INT32 LOCAL_31 = pMob[arg1].Target.X >> 7;
		INT32 LOCAL_32 = pMob[arg1].Target.Y >> 7;

		if(arg2 < MAX_PLAYER)
		{
			LOCAL_20 = LOCAL_20 / 6;

			if((LOCAL_27 == 7 && LOCAL_28 == 8) || (LOCAL_27 == 8 && LOCAL_28 == 7))
				LOCAL_29 = 1;

			INT32 LOCAL_33 = pMob[arg1].Mobs.Player.Guild;
			INT32 LOCAL_34 = pMob[arg2].Mobs.Player.Guild;

			INT32 LOCAL_35 = MAX_GUILD;

			//if(LOCAL_33 > 0 && LOCAL_33 < LOCAL_35 && LOCAL_34 > 0 && LOCAL_34 < LOCAL_35 && g_pGuildWar[LOCAL_33] == LOCAL_34 && g_pGuildWar[LOCAL_34] == LOCAL_33)
			//	LOCAL_30 = 1;

			if(sServer.CastleState != 0 && LOCAL_31 == 8 && LOCAL_32 == 13)
				LOCAL_30 = 1;
		}

		Log(arg1, LOG_INGAME, "Morreu para %s %dx %dy", pMob[arg2].Mobs.Player.MobName, pMob[arg2].Target.X, pMob[arg2].Target.Y);
		Log(arg2, LOG_INGAME, "Matou %s em %dx %dy", pMob[arg1].Mobs.Player.MobName, pMob[arg1].Target.X, pMob[arg1].Target.Y);

		INT32 LOCAL_36 = GetArena(pMob[arg1].Target.X, pMob[arg1].Target.Y);
		INT32 LOCAL_37 = GetVillage(pMob[arg1].Target.X, pMob[arg1].Target.Y);
		INT32 warArea = -1;

		for(INT32 i = 0 ; i < 5; i++)
		{
			if(pMob[arg1].Target.X >= g_pCityZone[i].war_min_x && pMob[arg1].Target.X <= g_pCityZone[i].war_max_x && pMob[arg1].Target.Y >= g_pCityZone[i].war_min_y && g_pCityZone[i].war_max_y)
			{
				warArea = i;
				break;
			}
		}

		INT32 LOCAL_38 = 0;

		if(LOCAL_31 == 0 && LOCAL_32 == 31)
			LOCAL_38 = 1;

		if(LOCAL_36 == 5 && LOCAL_37 == 5 && LOCAL_38 == 0)
		{
			if(LOCAL_29 != 0)
				LOCAL_20 = LOCAL_20 + ((LOCAL_20 * LOCAL_26) >> 2);

			if(LOCAL_20 > 150000)
				LOCAL_20 = 150000;
			
			// 0045B47B
			if(LOCAL_20 < 0)
				LOCAL_20 = 0;
			
			if(pMob[arg1].Target.X >= 2447 && pMob[arg1].Target.X <= 2545 && pMob[arg1].Target.Y >= 1851 && pMob[arg1].Target.Y <= 1924)
				LOCAL_20 = 0;

			if (!sServer.KefraDead && pMob[arg1].Target.X >= 2176 && pMob[arg1].Target.X <= 2559 && pMob[arg1].Target.Y >= 3840 && pMob[arg1].Target.Y <= 4096)
				LOCAL_20 = 0;

			if (pMob[arg1].Target.X >= 1041 && pMob[arg1].Target.X <= 1248 && pMob[arg1].Target.Y >= 1950 && pMob[arg1].Target.Y <= 2158)
				LOCAL_20 = 0;

			if(LOCAL_30 != 0)
				LOCAL_20 = 0;

            if (pMob[arg1].Mobs.Info.LvBlocked)
                LOCAL_20 = 0;

			if(LOCAL_16	>= 20 || pMob[arg1].Mobs.Player.Equip[0].EFV2 >= ARCH)
			{
				if(arg2 < MAX_PLAYER)
					pMob[arg1].Mobs.Player.Exp = pMob[arg1].Mobs.Player.Exp - LOCAL_20;
				else if(sServer.DeadPoint != 0)
					pMob[arg1].Mobs.Player.Exp = pMob[arg1].Mobs.Player.Exp - LOCAL_20;

				if(pMob[arg1].Mobs.Player.Exp < 0)
					pMob[arg1].Mobs.Player.Exp = 0;

				if(sServer.DeadPoint != 0 || arg2 < MAX_PLAYER)
				{
					sprintf_s(temp, "Voca perdeu %I64d de experiancia", LOCAL_20);
					SendClientMessage(arg1, temp);

					SendEtc(arg1);

					Log(arg1, LOG_INGAME, "Perdeu %I64d de experiancia.", LOCAL_20);
				}

				while (pMob[arg1].Mobs.Player.BaseScore.Level > 0 && pMob[arg1].Mobs.Player.Exp < g_pNextLevel[pMob[arg1].Mobs.Player.Equip[0].EFV2][pMob[arg1].Mobs.Player.BaseScore.Level - 1])
					pMob[arg1].Mobs.Player.BaseScore.Level--;

				SendEtc(arg1);
			}
			else
				SendClientMessage(arg1, "Level abaixo de 20 nao perde experiancia."); // No caso, MeessageString, ou seila o nome

			if(LOCAL_16 >= 99 && arg2 < MAX_PLAYER && LOCAL_30 == 0)
			{ // Navel acima de 100
				INT64 holdValue = 0;
				INT32 classMaster = pMob[arg1].Mobs.Player.Equip[0].EFV2;
				if(classMaster == 1 || classMaster == 2)
					holdValue = 15000;
				else 
					holdValue = 50000;
				
				if(pMob[arg1].Target.X >= 2447 && pMob[arg1].Target.X <= 2545 && pMob[arg1].Target.Y >= 1850 && pMob[arg1].Target.Y <= 1921)
					holdValue = 0;

				if (pMob[arg1].Target.X >= 1041 && pMob[arg1].Target.X <= 1248 && pMob[arg1].Target.Y >= 1950 && pMob[arg1].Target.Y <= 2158)
					holdValue = 0;

				for (INT32 g = 0; g < MAX_MESSAGE; g++)
				{
					if (pMob[arg1].Target.X >= g_pPositionCP[g].Min.X && pMob[arg1].Target.X <= g_pPositionCP[g].Max.X && pMob[arg1].Target.Y >= g_pPositionCP[g].Min.Y && pMob[arg1].Target.Y <= g_pPositionCP[g].Max.Y)
					{
						holdValue = 0;

						break;
					}
				}

				if (pMob[arg1].Target.X >= 2180 && pMob[arg1].Target.X <= 2558 && pMob[arg1].Target.Y >= 3837 && pMob[arg1].Target.Y <= 4096)
					holdValue = 0;

				if (pMob[arg1].Target.X >= 2448 && pMob[arg1].Target.X <= 2545 && pMob[arg1].Target.Y >= 1850 && pMob[arg1].Target.Y <= 1921 && sServer.TowerWar.CurrentScore)
					holdValue = 0;

				if (LOCAL_30 == 0)
					holdValue = 0;

				if(holdValue + pMob[arg1].Mobs.Hold > (g_pNextLevel[classMaster][LOCAL_16]))
					pMob[arg1].Mobs.Hold = g_pNextLevel[classMaster][LOCAL_16];
				else
					pMob[arg1].Mobs.Hold += holdValue;

				LOCAL_13.Hold = static_cast<int>(pMob[arg1].Mobs.Hold);
			}

			if(LOCAL_29 != 0 && warArea == -1)
			{
				SetCurKill(arg2, LOCAL_21 + 1);
				SetTotKill(arg2, LOCAL_22 + 1);

				SendItem(arg2, SlotType::Inv, 63, &pMob[arg2].Mobs.Player.Inventory[63]);

				if(LOCAL_30 == 0 && LOCAL_25 == 0)
				{
					INT32 LOCAL_42 = (-(LOCAL_24 / 20)) * 3;

					if(LOCAL_42 < -3)
						LOCAL_42 = -3;

					if(LOCAL_42 > 0)
						LOCAL_42 = 0;

					if(LOCAL_25 > 0)
						LOCAL_42 = 0;
					
					for(INT32 g = 0; g < MAX_MESSAGE; g++)
					{
						if(pMob[arg1].Target.X >= g_pPositionCP[g].Min.X && pMob[arg1].Target.X <= g_pPositionCP[g].Max.X && pMob[arg1].Target.Y >= g_pPositionCP[g].Min.Y && pMob[arg1].Target.Y <= g_pPositionCP[g].Max.Y)
						{
							LOCAL_42 = 0;

							break;
						}
					}
 
					if(pMob[arg1].Target.X >= 2180 && pMob[arg1].Target.X <= 2558 && pMob[arg1].Target.Y >= 3837 && pMob[arg1].Target.Y <= 4096)
						LOCAL_42 = 0;

					if (pMob[arg1].Target.X >= 2448 && pMob[arg1].Target.X <= 2545 && pMob[arg1].Target.Y >= 1850 && pMob[arg1].Target.Y <= 1924 && sServer.TowerWar.CurrentScore)
						LOCAL_42 = 0;
						
					Log(arg2, LOG_INGAME, "CP %d diminuiu em %d", LOCAL_23 - 75, LOCAL_42);
					LogPlayer(arg2, "CP %d foi diminuado em %d", LOCAL_23 - 75, LOCAL_42);

					LOCAL_23 = LOCAL_23 + LOCAL_42;

					SetPKPoint(arg2, LOCAL_23);

					sprintf_s(temp, "CP %d diminuiu em %d", LOCAL_23 - 75, LOCAL_42);
					SendClientMessage(arg2, temp);
				}
			}
			else if(LOCAL_30 == 0 && arg2 < MAX_PLAYER && warArea == -1) // 0045B74F
			{
				INT32 LOCAL_43 = (-(LOCAL_24 / 25) * 3);

				if(LOCAL_43 < -3)
					LOCAL_43 = -3;

				if(LOCAL_43 > 0)
					LOCAL_43 = 0;

				if(LOCAL_25 > 0)
					LOCAL_43 = 0;

				if(pMob[arg1].Mobs.Player.Equip[15].sIndex == 548 || pMob[arg1].Mobs.Player.Equip[15].sIndex == 549 || pMob[arg2].Mobs.Player.Equip[15].sIndex == 548 || pMob[arg2].Mobs.Player.Equip[15].sIndex == 549)
					LOCAL_43 = LOCAL_43 * 3;
	
				if(pMob[arg1].Target.X >= 2180 && pMob[arg1].Target.X <= 2541 && pMob[arg1].Target.Y >= 3858 && pMob[arg1].Target.Y <= 4051)
					LOCAL_43 = 0;
				
				for(INT32 g = 0; g < MAX_MESSAGE; g++)
				{
					if(pMob[arg1].Target.X >= g_pPositionCP[g].Min.X && pMob[arg1].Target.X <= g_pPositionCP[g].Max.X && pMob[arg1].Target.Y >= g_pPositionCP[g].Min.Y && pMob[arg1].Target.Y <= g_pPositionCP[g].Max.Y)
					{
						LOCAL_43 = 0;

						break;
					}
				}

				if (pMob[arg1].Target.X >= 2448 && pMob[arg1].Target.X <= 2545 && pMob[arg1].Target.Y >= 1850 && pMob[arg1].Target.Y <= 1921 && sServer.TowerWar.CurrentScore)
					LOCAL_43 = 0;

				LOCAL_23 = LOCAL_23 + LOCAL_43;

				SetPKPoint(arg2, LOCAL_23);

				if(LOCAL_43 != 0)
				{
					sprintf_s(temp, "CP %d diminuiu em %d", LOCAL_23 - 75, -LOCAL_43);

					SendClientMessage(arg2, temp);
					Log(arg2, LOG_INGAME, "CP %d diminui em %d", LOCAL_23 - 75, -LOCAL_43);
				}
				
				/*
				if(LOCAL_24 <= 60)
				{
					INT32 LOCAL_44 = (75 - LOCAL_24) / 10;
					INT32 LOCAL_45 = 0;
					for(INT32 LOCAL_46 = 0; LOCAL_46 < 63; LOCAL_46 ++)
					{
						if(Rand() % 5)
							continue;
						INT32 LOCAL_47 = pMob[arg1].Target.X;
						INT32 LOCAL_48 = pMob[arg1].Target.Y;
							
						STRUCT_ITEM *LOCAL_49 = &pMob[arg1].Mobs.Player.Inventory[LOCAL_46];
						if(LOCAL_49->sIndex <= 0 || LOCAL_49->sIndex > MAX_ITEMLIST)
							continue;
						if(LOCAL_49->sIndex == 508 || LOCAL_49 ->sIndex == 509 || LOCAL_49->sIndex == 522 || LOCAL_49->sIndex == 531 || LOCAL_49->sIndex == 446)
							continue;
						INT32 LOCAL_50 = Rand() & 0x80000003;
						INT32 LOCAL_51 = CreateItem(LOCAL_47, LOCAL_48, LOCAL_49, 1, 1);
						if(LOCAL_51 >= 5000 || LOCAL_51 <= 0)
							continue;
							
						memset(LOCAL_49, 0, 8);
						// INICIO DO BUFFER -> EBP - 108h
						char buffer[28];
						*(WORD*)&buffer[4] = 0x175;
						*(WORD*)&buffer[0] = 28;
						*(DWORD*)&buffer[12] = 1;
						*(DWORD*)&buffer[16] = LOCAL_46;
						*(DWORD*)&buffer[20] = LOCAL_50;
						*(WORD*)&buffer[24] = LOCAL_47;
						*(WORD*)&buffer[26] = LOCAL_48;
						GridMulticast_2(pMob[arg2].Target.X, pMob[arg2].Target.Y, (BYTE*)&buffer, 28);
						LOCAL_45 = LOCAL_45 + 1;
						// Revisar esse RAND \/
						if(!Rand() & 0x80000001)
							break;
						if(LOCAL_45 >= LOCAL_44)
							break;
					}
				}
					
				if(LOCAL_24 <= 35)
				{
					INT32 LOCAL_67 = (LOCAL_24 + 10) / 10;
					if(LOCAL_67 <= 0)
						LOCAL_67 = 1;
					INT32 LOCAL_68 = Rand() % LOCAL_67;
					//0045BB71
					INT32 LOCAL_69;
					// Nao entendi direito essa parte, ta muito confuso
					while(!LOCAL_68)
					{ 
						LOCAL_69 = Rand() % 14 + 1;
						if(LOCAL_69 != 12)
						{
							INT32 LOCAL_70 = pMob[arg1].Target.X;
							INT32 LOCAL_71 = pMob[arg1].Target.Y;
							STRUCT_ITEM *LOCAL_72 = &pMob[arg1].Mobs.Player.Equip[LOCAL_69];
							if(LOCAL_72->sIndex <= 0  || LOCAL_72->sIndex > MAX_ITEMLIST)
								break;
								
							if(LOCAL_72->sIndex == 508 || LOCAL_72->sIndex == 509 || LOCAL_72->sIndex == 522 || LOCAL_72->sIndex == 531 || LOCAL_72->sIndex == 446)
								break;
							INT32 LOCAL_73 = Rand() & 0x80000003;
							INT32 LOCAL_74 = CreateItem(LOCAL_70, LOCAL_71, LOCAL_72, 1, 1);
							if(LOCAL_74 >= 5000 || LOCAL_74 <= 0)
								break;
							char buffer[28];
							*(WORD*)&buffer[4] = 0x175;
							*(WORD*)&buffer[0] = 28;
							*(DWORD*)&buffer[12] = 0;
							*(DWORD*)&buffer[16] = LOCAL_69;
							*(DWORD*)&buffer[20] = LOCAL_73;
							*(WORD*)&buffer[24] = LOCAL_70;
							*(WORD*)&buffer[26] = LOCAL_71;
								
							GridMulticast_2(pMob[arg2].Target.X, pMob[arg2].Target.Y, (BYTE*)&buffer, 28);
						}
					}
				}
					*/
				p364 LOCAL_116;
				GetCreateMob(arg2, (BYTE*)&LOCAL_116);

				GridMulticast_2(pMob[arg2].Target.X, pMob[arg2].Target.Y, (BYTE*)&LOCAL_116, 0);

				if(LOCAL_29 != 0)
					SetCurKill(arg1, 0);
			}
		}
		else
		{
			//0045BF43 - Voca nao pode perder experiancia nesta area
			SendClientMessage(arg1, "Voca nao pode perder experiancia nesta area");
		}

		if(pMob[arg1].Mobs.Player.Equip[13].sIndex == 753 || pMob[arg1].Mobs.Player.Equip[13].sIndex == 1726)
		{
			const auto& familiarItem = pMob[arg1].Mobs.Player.Equip[13];
			INT32 LOCAL_123 = GetItemSanc(&pMob[arg1].Mobs.Player.Equip[13]);

			if (LOCAL_123 <= 0)
			{
				Log(arg1, LOG_INGAME, "%s %s foi destruado", g_pItemList[familiarItem.sIndex].ItemName, familiarItem.toString().c_str());
				memset(&pMob[arg1].Mobs.Player.Equip[13], 0, 8);
			}
			else
			{
				LOCAL_123 --;
				if(pMob[arg1].Mobs.Player.Equip[13].stEffect[0].cEffect == EF_SANC)
					pMob[arg1].Mobs.Player.Equip[13].stEffect[0].cValue = LOCAL_123;
				else if(pMob[arg1].Mobs.Player.Equip[13].stEffect[1].cEffect == EF_SANC)
					pMob[arg1].Mobs.Player.Equip[13].stEffect[1].cValue = LOCAL_123;
				else if(pMob[arg1].Mobs.Player.Equip[13].stEffect[2].cEffect == EF_SANC)
					pMob[arg1].Mobs.Player.Equip[13].stEffect[2].cValue = LOCAL_123;

				Log(arg1, LOG_INGAME, "%s %s perdeu uma refinaaao. Refinaaaes restantes: %d", g_pItemList[familiarItem.sIndex].ItemName, familiarItem.toString().c_str(), LOCAL_123);
			}

			SendItem(arg1, SlotType::Equip, 13, &pMob[arg1].Mobs.Player.Equip[13]);
		}
		
		GridMulticast_2(pMob[arg1].Target.X, pMob[arg1].Target.Y, (BYTE*)&LOCAL_13, 0);
		return;
	}

	//0045C208
	if(arg2 >= MAX_PLAYER || pMob[arg1].Mobs.Player.CapeInfo == 4)
	{
		GridMulticast_2(pMob[arg1].Target.X, pMob[arg1].Target.Y, (BYTE*)&LOCAL_13, 0);

		DeleteMob(arg1, 1);
		return;
	}

	//0045C267
	auto LOCAL_124 = GetExpApply(pMob[arg1].Mobs.Player.Exp, arg2, arg1);
	INT32 LOCAL_125 = 0;
	/*
	if(pMob[arg2].ExpBonus > 0 && pMob[arg2].ExpBonus < 20)
		LOCAL_125 = LOCAL_124 * pMob[arg2].ExpBonus / 100;
		*/
	UINT32 LOCAL_126 = 30;
	UINT32 LOCAL_127 = 0;
	UINT32 LOCAL_128 = 0;
	UINT32 LOCAL_129 = pMob[arg2].Target.X;
	UINT32 LOCAL_130 = pMob[arg2].Target.Y;
	UINT32 LOCAL_131;
	UINT32 LOCAL_132 = 0;

	for(; LOCAL_132 < 13; LOCAL_132++)
	{
		if(LOCAL_132 == 12)
			LOCAL_131 = LOCAL_14;
		else
			LOCAL_131 = pMob[LOCAL_14].PartyList[LOCAL_132];

		if(LOCAL_131 <= 0 || LOCAL_131 >= MAX_PLAYER)
			continue;

		if (LOCAL_129 < (pMob[LOCAL_131].Target.X - HALFGRIDX))
			continue;
		if (LOCAL_129 >(pMob[LOCAL_131].Target.X + HALFGRIDX))
			continue;
		if (LOCAL_130 < (pMob[LOCAL_131].Target.Y - HALFGRIDY))
			continue;
		if (LOCAL_130 >(pMob[LOCAL_131].Target.Y + HALFGRIDY))
			continue;

		LOCAL_127 = pMob[LOCAL_131].Mobs.Player.CurrentScore.Level +  LOCAL_126 + LOCAL_127;
		LOCAL_128 ++;
	}

	if(LOCAL_128 <= 0 || LOCAL_128 > 13 || LOCAL_127 <= 0)
	{
		GridMulticast_2(pMob[arg1].Target.X, pMob[arg1].Target.Y, (BYTE*)&LOCAL_13, 0);

		DeleteMob(arg1, 1);
		return;
	}
	
	static INT32 PARTYBONUS [13] = {100, 100, 200, 300, 400, 500, 500, 500, 500, 500, 500, 500, 500};
	static INT32 g_EmptyMob = MAX_PLAYER;
	static INT32 DOUBLEMODE = 0; // 758FAC8

 
	long long LOCAL_133 = PARTYBONUS[LOCAL_128];
	if(LOCAL_128 > 1)
		LOCAL_133 = LOCAL_133 + g_EmptyMob - 100;

	long long LOCAL_134 = LOCAL_124;
	long long LOCAL_135 = LOCAL_124 * LOCAL_133 / 25;

	for(LOCAL_132 = 0; LOCAL_132 < 13; LOCAL_132 ++)
	{
		if(LOCAL_132 == 0)
			LOCAL_131 = LOCAL_14;
		else
			LOCAL_131 = pMob[LOCAL_14].PartyList[LOCAL_132 - 1];

		if(LOCAL_131 <= 0 || LOCAL_131 >= MAX_PLAYER)
			continue;

		if(!pMob[LOCAL_131].Mobs.Player.CurrentScore.Hp)
			continue;
		
		if((pMob[LOCAL_131].Target.X < 1152 || pMob[LOCAL_131].Target.X > 1282 || pMob[LOCAL_131].Target.Y < 130 || pMob[LOCAL_131].Target.Y > 217) &&
			(pMob[LOCAL_131].Target.X < 1049 || pMob[LOCAL_131].Target.X > 1130 || pMob[LOCAL_131].Target.Y < 272 || pMob[LOCAL_131].Target.Y > 334))
		{
			if(LOCAL_129 < (pMob[LOCAL_131].Target.X - HALFGRIDX))
				continue;
			if (LOCAL_129 > (pMob[LOCAL_131].Target.X + HALFGRIDX))
				continue;
			if (LOCAL_130 < (pMob[LOCAL_131].Target.Y - HALFGRIDY))
				continue;
			if (LOCAL_130 > (pMob[LOCAL_131].Target.Y + HALFGRIDY))
				continue;
		}

		long long LOCAL_136 = ((pMob[LOCAL_131].Mobs.Player.CurrentScore.Level + LOCAL_126) * LOCAL_135) / LOCAL_127;
		if(LOCAL_136 < 0 || LOCAL_136 > 500000000)
			LOCAL_136 = 500000000;

		LOCAL_136 = LOCAL_136 * 6 / 10;
		if(LOCAL_136 > LOCAL_134)
			LOCAL_136 = LOCAL_134;

		LOCAL_136 += LOCAL_126;
		LOCAL_136 = GetExpApply_2(LOCAL_136, LOCAL_131, arg2, true);

		INT64 hold = pMob[LOCAL_131].Mobs.Hold;
		if(hold > 0)
		{
			if(hold > LOCAL_136)
			{
				hold -= LOCAL_136;
				LOCAL_136 = 0;
			}
			else
			{
				LOCAL_136 -= static_cast<int>(hold);
				hold = 0;
			}

			pMob[LOCAL_131].Mobs.Hold = hold;
		}

		pMob[LOCAL_131].CheckQuarter(LOCAL_136);
		pMob[LOCAL_131].Mobs.Player.Exp += LOCAL_136;
		pMob[LOCAL_131].CheckGetLevel();
	}

	INT32 LOCAL_137 = pMob[arg1].GenerateID;
	INT32 LOCAL_138 = Rand() & 0x80000003;

	if(LOCAL_137 >= 0 && mGener.pList[LOCAL_137].DieAction[LOCAL_138][0] && pMob[arg1].Leader == 0)
		SendSay(arg1, mGener.pList[LOCAL_137].DieAction[LOCAL_138]);

	GridMulticast_2(pMob[arg1].Target.X, pMob[arg1].Target.Y, (BYTE*)&LOCAL_13, 0);

	// 0045C809
	INT32 LOCAL_139 = pMob[arg2].Mobs.Player.CapeInfo;
	if(LOCAL_139 != 0 && LOCAL_139 != 4 && LOCAL_139 != CAPE_BLUE && LOCAL_139 != CAPE_RED)
	{
		DeleteMob(arg1, 1);
		return;
	}

	INT32 LOCAL_140 = arg3;
	if(LOCAL_140 == 0)
		LOCAL_140 = pMob[arg1].Target.X;

	INT32 LOCAL_141 = arg4;
	if(LOCAL_141 == 0)
		LOCAL_141 = pMob[arg1].Target.Y;

	INT32 LOCAL_142 = pMob[arg1].GenerateID;
	if(pMob[arg1].Mobs.Player.Equip[0].sIndex == 219 && LOCAL_142 != GUARDIAN_TOWER_BLUE && LOCAL_142 != GUARDIAN_TOWER_RED)
	{
		if(pMob[arg1].Target.X < 0 || pMob[arg1].Target.X >= 4096 || pMob[arg1].Target.Y < 0 || pMob[arg1].Target.Y >= 4096)
		{
			Log(SERVER_SIDE, LOG_ERROR, "Nenhum portao do castelo para abrir na posiaao.");
			// 0045C958

			return;
		}
		
		INT32 LOCAL_143 = g_pItemGrid[pMob[arg1].Target.Y][pMob[arg1].Target.X];
		if(LOCAL_143 < 0 || LOCAL_143 >= 5000 || g_pInitItem[LOCAL_143].Open == 0)
		{
			Log(SERVER_SIDE, LOG_ERROR, "Nenhum portao do castelo para abrir.");
			return;
		}

		int LOCAL_144;
		INT32 LOCAL_145 = UpdateItem(LOCAL_143, 1, &LOCAL_144);

		//
		if(LOCAL_145 != 0)
		{
			p374 LOCAL_150;
			LOCAL_150.Header.PacketId = 0x374;
			LOCAL_150.Header.ClientId = 0x7530;
			LOCAL_150.gateId = LOCAL_143 + 10000;
			LOCAL_150.Header.Size = 20;
			LOCAL_150.status = g_pInitItem[LOCAL_143].CurrentScore;
			LOCAL_150.unknow = 0;

			GridMulticast_2(g_pInitItem[LOCAL_143].PosX, g_pInitItem[LOCAL_143].PosY, (BYTE*)&LOCAL_150, 0);

			g_pInitItem[LOCAL_143].IsOpen = 0;
		}
	}
	// 0045CE87
	if(LOCAL_142 == 3 && pMob[arg1].Mobs.Player.Equip[0].sIndex == 258)
	{
		Log(arg2, LOG_INGAME, "Derrotou Lorde Zakum");

		char szTMP[256];

		sprintf_s(szTMP, g_pLanguageString[_SN_Zakum_Killed], pMob[arg2].Mobs.Player.MobName);

		//0045CEAF
		SendNoticeArea(szTMP, 2176, 1160, 2300, 1276);

		sServer.Zakum.Clear = 1;
		INT32 LOCAL_170 = pMob[arg1].Target.X,
			  LOCAL_171 = pMob[arg1].Target.Y;

		STRUCT_ITEM LOCAL_173;
		memset(&LOCAL_173, 0, 8);

		LOCAL_173.sIndex = 753;
		
		INT32 LOCAL_174 = (Rand() % 5) - 1;
		INT32 LOCAL_175 = (Rand() % 5) - 1;
		INT32 LOCAL_176 = 0;
		INT32 LOCAL_177 = 174 + LOCAL_175 + LOCAL_176;
		INT32 LOCAL_178 = 0;

		if(LOCAL_177 < 0)
			LOCAL_177 = 0;

		if(LOCAL_177 < 9)
			LOCAL_178 = Rand() % 10;
		
		LOCAL_173.stEffect[0].cEffect = EF_SANC;
		SetItemSanc(&LOCAL_173, LOCAL_177, LOCAL_178);

		for (INT32 LOCAL_780 = 1; LOCAL_780 <= 2; LOCAL_780++)
		{
			INT32 LOCAL_781 = Rand() & 0x80000007;
			if (LOCAL_781 == 0)
			{
				LOCAL_173.stEffect[LOCAL_780].cEffect = EF_HP;
				LOCAL_173.stEffect[LOCAL_780].cValue = (Rand() % 41) + 20;
			}
			else if (LOCAL_781 == 1)
			{
				LOCAL_173.stEffect[LOCAL_780].cEffect = EF_DAMAGE;
				LOCAL_173.stEffect[LOCAL_780].cValue = (Rand() % 21) + 5;
			}
			else if (LOCAL_781 == 2)
			{
				LOCAL_173.stEffect[LOCAL_780].cEffect = EF_ATTSPEED;
				LOCAL_173.stEffect[LOCAL_780].cValue = (Rand() % 11) + 5;
			}
			else if (LOCAL_781 == 3)
			{
				LOCAL_173.stEffect[LOCAL_780].cEffect = EF_MP;
				LOCAL_173.stEffect[LOCAL_780].cValue = (Rand() % 51) + 20;
			}
			else if (LOCAL_781 == 4)
			{
				LOCAL_173.stEffect[LOCAL_780].cEffect = EF_MAGIC;
				LOCAL_173.stEffect[LOCAL_780].cValue = (Rand() % 7) + 2;
			}
			else if (LOCAL_781 == 5)
			{
				LOCAL_173.stEffect[LOCAL_780].cEffect = EF_STR;
				LOCAL_173.stEffect[LOCAL_780].cValue = (Rand() & 0x8000000F) + 5;
			}
			else if (LOCAL_781 == 6)
			{
				LOCAL_173.stEffect[LOCAL_780].cEffect = EF_INT;
				LOCAL_173.stEffect[LOCAL_780].cValue = (Rand() & 0x8000000F) + 5;
			}
			else if (LOCAL_781 == 7)
			{
				LOCAL_173.stEffect[LOCAL_780].cEffect = EF_DEX;
				LOCAL_173.stEffect[LOCAL_780].cValue = (Rand() & 0x8000000F) + 5;
			}
		}

		INT32 slotId = GetFirstSlot(arg2, 0);
		if(slotId != -1)
		{
			memset(&pMob[arg2].Mobs.Player.Inventory[slotId], 0, sizeof STRUCT_ITEM);
			memcpy(&pMob[arg2].Mobs.Player.Inventory[slotId], &LOCAL_173, sizeof STRUCT_ITEM);

			SendItem(arg2, SlotType::Inv, slotId, &LOCAL_173);

			Log(arg2, LOG_INGAME, "Item IMP adicionado corretamente ao inventario [%d] [%d %d %d %d %d %d]", LOCAL_173.sIndex, LOCAL_173.EF1, LOCAL_173.EFV1, LOCAL_173.EF2,
				LOCAL_173.EFV2, LOCAL_173.EF3, LOCAL_173.EFV3);

			LogPlayer(arg2, "Recebido IMP na quest Lorde Zakum");
		}
		//CreateItem(LOCAL_170, LOCAL_171, &LOCAL_173, Rand() & 0x080000003, 1);
	}
	else if(LOCAL_142 >= TORRE_NOATUN && LOCAL_142 < TORRE_NOATUN + 3)
	{// 0045D057
		INT32 LOCAL_179 = LOCAL_142 - TORRE_NOATUN;
		sServer.LiveTower[LOCAL_179] = 0;

		for(INT32 LOCAL_180 = 0 ; LOCAL_180 < 3; LOCAL_180++)
		{
			if(sServer.LiveTower[LOCAL_180] != 0)
				break;
		}
	}
	// 0045D0C6
	else if(LOCAL_142 == 8 || LOCAL_142 == 9)
	{
		int index = LOCAL_142 - 8;

		if (sServer.KingdomBattle.Info[index].isTowerAlive)
		{
			SendClientMessage(arg2, "Para derrotar o Rei, a necessario passar pela Torre Guardia!");

			pMob[arg1].Mobs.Player.CurrentScore.Hp = pMob[arg1].Mobs.Player.CurrentScore.MaxHp;

			p364 packet{};
			GetCreateMob(arg1, reinterpret_cast<BYTE*>(&packet));
			
			GridMulticast_2(pMob[arg1].Target.X, pMob[arg1].Target.Y, reinterpret_cast<BYTE*>(&packet), 0);
			return;
		}
	
		if (LOCAL_142 == 8)
			sServer.KingdomBattle.Winner = CAPE_RED;
		else
			sServer.KingdomBattle.Winner = CAPE_BLUE;

		SendNotice("O Reino %s conquistou o reino %s", LOCAL_142 == 8 ? "Red" : "Blue", LOCAL_142 == 8 ? "Blue" : "Red");

		Log(SERVER_SIDE, LOG_INGAME, "O Reino %s conquistou o reino %s", LOCAL_142 == 8 ? "Red" : "Blue", LOCAL_142 == 8 ? "Blue" : "Red");

		for (int i = 1; i < MAX_PLAYER; i++)
		{
			if (pUser[i].Status != USER_PLAY)
				continue;

			pMob[i].GetCurrentScore(i);
			SendScore(i);
		}
		
		sServer.KingdomBattle.Info[index].CurrentScore = false;

		SendKingdomBattleInfo(SERVER_SIDE, CAPE_BLUE, sServer.KingdomBattle.Info[0].CurrentScore);
		SendKingdomBattleInfo(SERVER_SIDE, CAPE_RED, sServer.KingdomBattle.Info[1].CurrentScore);
	}
	else if (LOCAL_142 == GUARDIAN_TOWER_BLUE || LOCAL_142 == GUARDIAN_TOWER_RED)
	{
		SendNotice("A Torre Guardia do Reino %s foi derrotada", LOCAL_142 == GUARDIAN_TOWER_BLUE ? "Blue" : "Red");

		sServer.KingdomBattle.Info[LOCAL_142 - GUARDIAN_TOWER_BLUE].isTowerAlive = false;
		sServer.KingdomBattle.Info[LOCAL_142 - GUARDIAN_TOWER_BLUE].TowerId = 0;
	}
	else if(LOCAL_142 == 0 || LOCAL_142 == 1 || LOCAL_142 == 2)
	{
		INT32 LOCAL_181 = Rand() % 14;
		INT32 LOCAL_182 = 0;
	
		if(LOCAL_181 == 0)
			LOCAL_182 = 420;
		
		if(LOCAL_181 == 1)
			LOCAL_182 = 421;

		if(LOCAL_182 != 0)
		{
			STRUCT_ITEM LOCAL_184;
			memset(&LOCAL_184, 0, 8);

			LOCAL_184.sIndex = LOCAL_182;

			SetItemBonus(&LOCAL_184, 0, 0, 0);

			PutItem(arg2, &LOCAL_184);
		}
	}
	else if(LOCAL_142 == 5 || LOCAL_142 == 6 || LOCAL_142 == 7)
	{
		INT32 LOCAL_185 = Rand() % 14,
			  LOCAL_186 = 0;

		if(LOCAL_185 == 0)
			LOCAL_186 = Rand() % 7 + 421;

		if(LOCAL_186 != 0)
		{
			STRUCT_ITEM LOCAL_188;
			memset(&LOCAL_188, 0, 8);

			LOCAL_188.sIndex = LOCAL_186;
			
			SetItemBonus(&LOCAL_188, 0, 0, 0);
			
			PutItem(arg2, &LOCAL_188);
				//CreateItem(LOCAL_140, LOCAL_141, &LOCAL_188, Rand() & 0x80000003, 1);
		}
	}
	else if(LOCAL_142 == 3)
	{
		INT32 LOCAL_189 = Rand() % 7;
		INT32 LOCAL_190 = 0;

		if(LOCAL_189 == 0)
			LOCAL_190 = 1106;
		if(LOCAL_189 == 1)
			LOCAL_190 = 1256;
		if(LOCAL_189 == 2)
			LOCAL_190 = 1418;
		if(LOCAL_189 == 3)
			LOCAL_190 = 1568;

		if(LOCAL_190 != 0)
		{
			STRUCT_ITEM LOCAL_192;
			memset(&LOCAL_192, 0, sizeof STRUCT_ITEM);

			LOCAL_192.sIndex = LOCAL_190;

			SetItemBonus(&LOCAL_192, 75, 1, 0);
			
			PutItem(arg2, &LOCAL_192);
		}
	}

	else if(LOCAL_142 == VALKY_ID  || LOCAL_142 == HELL_BOSS || LOCAL_142 == COELHO_ID || LOCAL_142 == BARLOG)
	{
		INT32 pistaId = 2;
		if(LOCAL_142 == HELL_BOSS)
			pistaId = 4;
		else if(LOCAL_142 == COELHO_ID)
			pistaId = 6;
		else if(LOCAL_142 == BARLOG)
			pistaId = 5;

		// Teleporta para o boss
		STRUCT_PISTA_DE_RUNAS *pista = &pPista[pistaId];

		int party = -1;
		for(int i = 0; i < MAX_PARTYPISTA;i ++)
		{
			for(int x = 0; x < 13;x++)
			{
				if(pista->Clients[i][x] == arg2)
				{
					party = i;
					break;
				}
			}
		}

        static std::map<int, std::string> bossName
        {
            { VALKY_ID, "Valkyria" },
            { HELL_BOSS, "Hell" },
            { COELHO_ID, "Coelho" },
            { BARLOG, "Barlog" },
        };

		if (party != -1)
		{
			GiveRuna(pistaId, party);
		
			for (int i = 0; i < MAX_PARTYPISTA; ++i)
			{
				if (i == party)
					continue;

                LogRune(pistaId, i, "O boss "s + bossName[LOCAL_142] + "foi morto pelo grupo "s + std::to_string(party) + ". Este grupo nao recebera a premiaaao. Morto por "s + pMob[arg2].Mobs.Player.MobName);
                MessageRune(pistaId, i, "O boss foi derrotado por outro grupo.");
			}

			LogRune(pistaId, party, "O Boss "s + bossName[LOCAL_142] + " foi morto pelo jogador "s + pMob[arg2].Mobs.Player.MobName + " do grupo.");
		}
		else
		{
            for (int i = 0; i < MAX_PARTYPISTA; ++i)
                LogRune(pistaId, i, "O Boss "s + bossName[LOCAL_142] + " foi morto pelo jogador "s.append(pMob[arg2].Mobs.Player.MobName).append(" que nao estava registrado na pista."));
		}
	}
	else if(LOCAL_142 >= KALINTZ_MAGE && LOCAL_142 <= KALINTZ_MAGE +7 )
	{
		STRUCT_PISTA_DE_RUNAS *pista = &pPista[3];

		int party = -1;
		for(int i = 0; i < MAX_PARTYPISTA;i ++)
		{
			for(int x = 0; x < 13;x++)
			{
				if(pista->Clients[i][x] == arg2)
				{
					party = i;
					break;
				}
			}
		}

		if(party != -1)
		{
			pista->inSec.Points[party] ++;

			int generateMob = SULRANG + Rand() % 3;
			if(generateMob > 189)
				generateMob --;

			GenerateMob(generateMob, 0, 0);
			LogRune(3, party, "Derrotou o KALINTZ e adquiriu 1 ponto para a equipe. Total de pontos: "s + (std::to_string(pista->inSec.Points[party])));
		}
	}
	else if(LOCAL_142 >= SULRANG && LOCAL_142 < SULRANG + 2)
	{
		int _rand = Rand () % 8;
		int generateIdd = KALINTZ_MAGE + _rand;

		GenerateMob(generateIdd, 0, 0);

		const STRUCT_PISTA_DE_RUNAS *pista = &pPista[3];
		int party = -1;
		for (int i = 0; i < MAX_PARTYPISTA; i++)
		{
			for (int x = 0; x < 13; x++)
			{
				if (pista->Clients[i][x] == arg2)
				{
					party = i;
					break;
				}
			}
		}

		if (party != -1)
			Log(3, party, "Derrotou o SULRANG. Renascendo spot de Kalintz");
	}
	else if(LOCAL_142 == LICH_ID)
	{
		STRUCT_PISTA_DE_RUNAS *pista = &pPista[0];

		int party = -1;
		for(int i = 0; i < MAX_PARTYPISTA;i ++)
		{
			for(int x = 0; x < 13;x++)
			{
				if(pista->Clients[i][x] == arg2)
				{
					party = i;
					break;
				}
			}
		}

		bool reborn = true;
		if(party != -1)
		{
			int _rand = rand() % 100;
			if(_rand <= 30)
			{
				GiveRuna(0, party);
				reborn = false;
			}
		}

		for(int i = 1000; i < 30000; i++)
		{
			if(pMob[i].GenerateID == LICH_ID)
				DeleteMob(i, 1);
		}

		if(reborn)
			GenerateMob(LICH_ID, 0, 0);
	}
	else if (LOCAL_142 >= TORRE_ID && LOCAL_142 < TORRE_ID + 3)
	{
		int groupIndex = LOCAL_142 - TORRE_ID;

		const auto pista = &pPista[1];
		for (int i = 0; i < 13; ++i)
		{
			auto memberId = pista->Clients[groupIndex][i];
			if (memberId <= 0 || memberId >= MAX_PLAYER)
				continue;

			Log(memberId, LOG_INGAME, "A Torre do grupo da Pista +1 morreu.");
			SendClientMessage(memberId, "Voca nao defendeu sua Torre");
		}
	}
	else if(LOCAL_142 >= HELL && LOCAL_142 < (HELL + 47))
	{
		if(LOCAL_142 != HELL_BOSS)
		{
			int indexRoom = LOCAL_142 - HELL;
			STRUCT_PISTA_DE_RUNAS *pista = &pPista[4];

			int party = -1;
			for (int i = 0; i < MAX_PARTYPISTA; i++)
			{
				for (int x = 0; x < 13; x++)
				{
					if (pista->Clients[i][x] == arg2)
					{
						party = i;

						break;
					}
				}
			}

			if(pista->inSec.RoomSorted[indexRoom] && mGener.pList[LOCAL_142].MobCount <= 1)
			{
				for(int i = 0; i < 13; i++)
				{
					int memberId = pista->Clients[party][i];
					if(memberId <= 0 || memberId >= MAX_PLAYER)
						continue;

					if(pMob[memberId].Target.X < g_pPistaCoords[4][0] || pMob[memberId].Target.X > g_pPistaCoords[4][2] || pMob[memberId].Target.Y < g_pPistaCoords[4][1] || pMob[memberId].Target.Y > g_pPistaCoords[4][3])
						continue;
					
					Teleportar(memberId, 3358, 1340);
					SendClientMessage(memberId, "Teleportado para a sala do Boss");

					Log(memberId, LOG_INGAME, "Teleportado para a area do boss +4.");
				} 

				for (int i = 0; i < MAX_PARTYPISTA; i++)
				{
					if (i == party)
						continue;

					for (int t = 0; t < 13; ++t)
					{
						int memberId = pista->Clients[i][t];
						if (memberId <= 0 || memberId >= MAX_PLAYER)
							continue;

						SendClientMessage(memberId, "A sala sorteada foi derrotada por outro grupo. Voca falhou na refinaaao!");
					}

					LogRune(4, i, "A sala sorteada foi derrotada por outro grupo.");
				}

				if(!pista->inSec.Born)
				{
					// Gera o BOSS no local
					GenerateMob(HELL_BOSS, 0, 0);

					pista->inSec.Born = true;
				
					for(int i = 1000; i < 30000;i++)
					{
						if(pMob[i].GenerateID == HELL_BOSS)
						{
							pista->inSec.BossID = i;
							break;
						}
					}
				}
			}
			else
			{
				if (party != -1)
				{
					std::stringstream str;
					str << "Derrotou a sala " << indexRoom << ". Sala nao sorteada. Mobs restantes: " << mGener.pList[LOCAL_142].MobCount - 1;

					LogRune(4, party, str.str());
				}
			}
		}
	}
	else if (LOCAL_142 == KEFRA)
	{
		if (arg2 == 29999)
		{
			DeleteMob(arg1, 1);

			return;
		}

		int killerId = arg2;
		if (arg2 >= MAX_PLAYER)
		{
			killerId = pMob[arg2].Summoner;

			if (killerId > 0 && killerId < MAX_PLAYER && pUser[killerId].Status != USER_PLAY)
				killerId = 0;
		}
		
		Log(SERVER_SIDE, LOG_INGAME, "KEFRA - Kefra derrotado por %s - GuildId: %d.", pMob[killerId].Mobs.Player.MobName, pMob[killerId].Mobs.Player.Guild);

		if (killerId)
		{
			if (pMob[killerId].Mobs.Player.Guild)
				SetGuildFame(pMob[killerId].Mobs.Player.Guild, g_pGuild[pMob[killerId].Mobs.Player.Guild].Fame + 100);

			_MSG_NOTIFY_KEFRA_DEATH packet;
			memset(&packet, 0, sizeof _MSG_NOTIFY_KEFRA_DEATH);

			packet.Header.ClientId = 0;
			packet.Header.PacketId = MSG_NOTIFY_KEFRA_DEATH;
			packet.Header.Size	   = sizeof _MSG_NOTIFY_KEFRA_DEATH;

			strncpy_s(packet.MobName, pMob[killerId].Mobs.Player.MobName, 16);

			AddMessageDB((BYTE*)&packet, sizeof _MSG_NOTIFY_KEFRA_DEATH);

			if(pMob[killerId].Mobs.Player.Guild != 0)
				sServer.KefraKiller = pMob[killerId].Mobs.Player.Guild;
			else
				sServer.KefraKiller = 0;
		}
		
		for(INT32 i = GUARDAS_KEFRA; i < GUARDAS_KEFRA + 18; i++)
		{
			// Faz o mob nao ficar nascendo apas morrer 
			mGener.pList[i].MinuteGenerate = -1;
		}

		for(INT32 i = 1000; i < MAX_MOB; i++)
		{
			if(pMob[i].GenerateID < GUARDAS_KEFRA || pMob[i].GenerateID > GUARDAS_KEFRA + 18)
				continue;

			MobKilled(i, i, 0, 0);
		}

		sServer.KefraDead = true;

		if (sServer.KefraKiller != 0)
		{
			SendNotice("O jogador [%s] da guild [%s] derrotou Kefra", pMob[killerId].Mobs.Player.MobName, g_pGuild[sServer.KefraKiller].MobName.c_str());

		}
		else
			SendNotice(g_pLanguageString[_NN_Kefra_PlayerKill]);
	}
	else if(LOCAL_142 >= COLOSSEUM_ID && LOCAL_142 < (COLOSSEUM_ID + 4))
	{
		int totalPL = 0;
		int totalPO = 0;
		for(INT32 i = 1;i < MAX_PLAYER; i++)
		{
			if(pUser[i].Status != USER_PLAY || pMob[i].Mobs.Player.CurrentScore.Hp <= 0)
				continue;
				
			if(pMob[i].Target.X >= 2604 && pMob[i].Target.Y >= 1708 && pMob[i].Target.X <= 2648 && pMob[i].Target.Y <= 1744)
			{
				for(INT32 t = 0; t < 64;t++)
				{
					STRUCT_ITEM *item = &pMob[arg1].Mobs.Player.Inventory[t];
					if(item->sIndex <= 0 || item->sIndex >= MAX_ITEMLIST)
						continue;

					INT32 _rand = Rand () % 100;
					INT32 rate = 40;

					if (sServer.Colosseum.Type == eColosseumType::Mystic)
						rate = 75;

					if(_rand < rate)
					{
						Log(i, LOG_INGAME, "Recebeu item %s %s no Coliseu", g_pItemList[item->sIndex].ItemName, item->toString());

						PutItem(i, item);
						if (item->sIndex == 413)
							totalPL++;
						else if (item->sIndex == 412)
							totalPO++;
					}
				}
			}
		}

		Log(SERVER_SIDE, LOG_INGAME, "Coliseu: Total de PLs %d. Total de POs %d", totalPL, totalPO);
	}

	if (pMob[arg1].BossInfoId != -1 && pMob[arg1].BossInfoId < sServer.Boss.size())
	{
		int connId = arg2;
		if (connId > MAX_PLAYER)
		{
			int summonerId = pMob[connId].Summoner;

			if (summonerId > 0 && summonerId < MAX_PLAYER)
				connId = summonerId;
			else
				connId = 0;
		}

		auto& boss = sServer.Boss[pMob[arg1].BossInfoId];
		INT32 guildId = pMob[connId].Mobs.Player.Guild;
		if (guildId != 0)
		{
			SendNotice("O jogador %s da guild %s derrotou %s", pMob[connId].Mobs.Player.MobName, g_pGuild[guildId].MobName.c_str(), pMob[arg1].Mobs.Player.MobName);

			if (boss.Fame != 0)
				SetGuildFame(guildId, g_pGuild[guildId].Fame + boss.Fame);

			Log(SERVER_SIDE, LOG_INGAME, "%s foi derrotado por %s (%s-%d). Fame: %d. Fame ganha: %d", pMob[arg1].Mobs.Player.MobName, pMob[connId].Mobs.Player.MobName, g_pGuild[guildId].MobName.c_str(), guildId, g_pGuild[guildId].Fame, boss.Fame);
		}
		else
		{
			SendNotice("O jogador %s derrotou o %s", pMob[arg2].Mobs.Player.MobName, pMob[arg1].Mobs.Player.MobName);
			Log(SERVER_SIDE, LOG_INGAME, "%s foi derrotado por %s, SEM GUILD", pMob[arg1].Mobs.Player.MobName, pMob[connId].Mobs.Player.MobName);
		}

		boss.GenerGenerated = 0;
		boss.LastUpdate = std::chrono::steady_clock::now();
	}

	INT32 LOCAL_193 = pMob[arg1].Mobs.Player.Coin;
	INT32 rate = 6;
	INT32 level = pMob[arg2].Mobs.Player.BaseScore.Level;
	switch (pMob[arg2].Mobs.Player.Equip[0].EFV2)
	{
	case 1:
		if (level < 100)
			rate = 2;
		else if (level < 200)
			rate = 3;
		else if (level < 300)
			rate = 5;

		break;
	case 2:
		if (level < 100)
			rate = 2;
		else if (level < 200)
			rate = 3;
		else if (level < 300)
			rate = 5;
		break;
	}

	int mobLevel = pMob[arg1].Mobs.Player.CurrentScore.Level;
	if (mobLevel >= 250 && LOCAL_193 == 0)
	{
		LOCAL_193 = 500 + ((mobLevel - 250) * 5);
		if (LOCAL_193 > 1500)
			LOCAL_193 = 1500;
	}

	if ((pMob[arg2].Target.X >= 3600 && pMob[arg2].Target.X <= 3700 && pMob[arg2].Target.Y >= 3600 && pMob[arg2].Target.Y <= 3700) ||
		(pMob[arg2].Target.X >= 3732 && pMob[arg2].Target.X <= 3816 && pMob[arg2].Target.Y >= 3476 && pMob[arg2].Target.Y <= 3562))
		rate = 3;

	if(LOCAL_193 != 0 && (!(Rand() % rate)))
	{
		INT32 gold = LOCAL_193;
		
		INT32 aux = gold >> 1;
		if(aux <= 0)
			aux = 1;

		gold = gold + aux + (Rand() % aux);
		if(sServer.GoldBonus != 0)
			gold += ((gold * sServer.GoldBonus) / 100);

		unsigned int xgold = pMob[arg2].Mobs.Player.Coin + gold;
		if(xgold > 2000000000)
		{
			int zgold = 2000000000 - pMob[arg2].Mobs.Player.Coin;
			if(zgold > 0)
			{
				if(gold > zgold)
					gold = zgold;
			}
		}

		if(gold != 0)
		{
			// Adiciona o gold ao usuario
			pMob[arg2].Mobs.Player.Coin += gold;

			if(arg2 > 0 && arg2 < MAX_PLAYER)
			{
				pUser[arg2].Coin += gold;
				pUser[arg2].GoldCount ++;

				if(pUser[arg2].GoldCount >= 10)
					LogGold(arg2);
			}

			if(pMob[arg2].Mobs.Player.Coin == 2000000000)
				SendClientMessage(arg2, "Nao possui espaao para guardar gold.");

			// Envia a atualizaaao de gold
			SendSignalParm(arg2, arg2, 0x3AF, pMob[arg2].Mobs.Player.Coin);
		}
	}

	if (sServer.LanHouseN.MobId == arg1)
	{
		sServer.LanHouseN.MobId = 0;
		sServer.LanHouseN.Count = 0;
		for (int i = 1; i < MAX_PLAYER; i++)
		{
			if (pUser[i].Status != USER_PLAY)
				continue;

			if (pMob[i].Target.X >= 3600 && pMob[i].Target.X <= 3700 && pMob[i].Target.Y >= 3600 && pMob[i].Target.Y <= 3700)
				SendClientMessage(i, "O boss da LanHouse foi morto por %s", pMob[arg2].Mobs.Player.MobName);
		}
	}

	if (pMob[arg2].Target.X >= 3600 && pMob[arg2].Target.X <= 3700 && pMob[arg2].Target.Y >= 3600 && pMob[arg2].Target.Y <= 3700 && pMob[arg2].Mobs.Player.CapeInfo != 4)
	{
		if (++sServer.LanHouseN.Count >= sServer.LanHouseN.TotalToKill && sServer.LanHouseN.MobId == 0)
		{
			sServer.LanHouseN.Count = 0;

			std::array<STRUCT_POSITION, 4> positions =
			{
				{
					{3610,3685},
					{3610, 3610},
					{3685, 3610},
					{3685, 3683}
				}
			};

			auto randomPos = select_randomly(std::begin(positions), std::end(positions));
			sServer.LanHouseN.MobId = CreateMob("DarkCaveira", randomPos->X, randomPos->Y, "npc");

			Log(SERVER_SIDE, LOG_INGAME, "O boss da area da LanHouse nasceu em %hux %huy", randomPos->X, randomPos->Y);
			SendNoticeArea("O boss da LanHouse (N) nasceu", 3600, 3600, 3700, 3700);
		}
	}

	if (sServer.LanHouseM.MobId == arg1)
	{
		sServer.LanHouseM.MobId = 0;
		sServer.LanHouseM.Count = 0;
		for (int i = 1; i < MAX_PLAYER; i++)
		{
			if (pUser[i].Status != USER_PLAY)
				continue;

			if (pMob[i].Target.X >= 3732 && pMob[i].Target.X <= 3816 && pMob[i].Target.Y >= 3476 && pMob[i].Target.Y <= 3562)
				SendClientMessage(i, "O boss da LanHouse foi morto por %s", pMob[arg2].Mobs.Player.MobName);
		}
	}

	if (pMob[arg2].Target.X >= 3732 && pMob[arg2].Target.X <= 3816 && pMob[arg2].Target.Y >= 3476 && pMob[arg2].Target.Y <= 3562 && pMob[arg2].Mobs.Player.CapeInfo != 4)
	{
		if (++sServer.LanHouseM.Count >= sServer.LanHouseM.TotalToKill && sServer.LanHouseM.MobId == 0)
		{
			sServer.LanHouseM.Count = 0;
			std::array<STRUCT_POSITION, 4> positions =
			{
				{
					{3813, 3557},
					{3813, 3482},
					{3738, 3482},
					{3738, 3557}
				}
			};

			auto randomPos = select_randomly(std::begin(positions), std::end(positions));
			sServer.LanHouseM.MobId = CreateMob("PerGorila", randomPos->X, randomPos->Y, "npc");

			Log(SERVER_SIDE, LOG_INGAME, "O boss da area da LanHouse (M) nasceu em %hux %huy", randomPos->X, randomPos->Y);
			SendNoticeArea("O boss da LanHouse (M) nasceu", 3732, 3476, 3816, 3562);
		}
	}

	// 0045D646
	INT32 LOCAL_198 = pMob[arg1].Mobs.Player.CurrentScore.Level;
	static const int FixDropTax[64] = 
	{
		400,  400,  400,  400, 400, 400, 400,  400,  4,  4,  4,  4, 900,  900,  900,  900,  20000, 20000, 20000, 20000, 20000, 20000, 
		35, 35, 1500, 1500, 1500, 1500, 1500, 1500, 1500, 1500, 1500, 1500, 1500, 1500, 1500, 1500, 1500, 1500, 1500, 1500, 1500, 1500, 1500, 1500, 
		1500, 1500, 3000, 3000, 3000, 3000, 3000, 3000, 3000, 3000, 1, 35, 500, 2500, 5000, 5000, 10000, 20000
	};

	// 0045D857 testando
	for(LOCAL_132 = 0; LOCAL_132 < 64; LOCAL_132++)
	{
		STRUCT_ITEM *LOCAL_207 = (STRUCT_ITEM*)&pMob[arg1].Mobs.Player.Inventory[LOCAL_132];
		if(!pMob[arg1].Mobs.Player.Inventory[LOCAL_132].sIndex)
			continue;
		
		INT32 t = 0;
		for(; t < MAX_BLOCKITEM; t++)
		{
			if(pMob[arg1].Mobs.Player.Inventory[LOCAL_132].sIndex == g_pBlockedItem[t])
				break;
		}

		if(t != MAX_BLOCKITEM)
			continue;

		// Liberado drop dos itens abaixo somente com o Kefra morto
		if((LOCAL_207->sIndex >= 674 && LOCAL_207->sIndex <= 677) || LOCAL_207->sIndex == 770)
		{
			if(!sServer.KefraDead)
				continue;
		}

		INT32 LOCAL_203 = FixDropTax[LOCAL_132]; // 4C90B8
		INT32 LOCAL_204 = Taxes[LOCAL_132]; //Taxes[LOCAL_132] + mobKiller->Game.Bonus.DropBonus;

		if(LOCAL_204 != 100 && LOCAL_204 != 0)
		{
			LOCAL_204 = 10000 / LOCAL_204;
			LOCAL_203 = LOCAL_203 *		 LOCAL_204 / 100;
		}
		
		int bonusDrop = 0;
		for (int i = 0; i < 3; i++)
		{
			if (LOCAL_207->stEffect[i].cEffect == EF_DROPBONUS)
			{
				bonusDrop = LOCAL_207->stEffect[i].cValue;
				break;
			}
		}

		if (bonusDrop > 0 && bonusDrop < 100)
			LOCAL_203 -= (LOCAL_203 * bonusDrop / 100);

		INT32 fada = pMob[arg2].Mobs.Player.Equip[13].sIndex;
		if(fada == 3901 || fada == 3904 || fada == 3905 || fada == 3907 || fada == 3908 || fada == 3914)
			LOCAL_203 -= (LOCAL_203 *  32 / 100);
	
		if (fada == 3915)
			LOCAL_203 -= (LOCAL_203 * 40 / 100);

		if(fada == 3902)
			LOCAL_203 -= (LOCAL_203 *  16 / 100);

		if(pMob[arg2].DropBonus > 0 && pMob[arg2].DropBonus < 100)
			LOCAL_203 -= (LOCAL_203 * pMob[arg2].DropBonus / 100);

		if(LOCAL_203 <= 0)
			LOCAL_203 = 1;

		INT32 LOCAL_205 = LOCAL_132 >> 3;
		if(LOCAL_132 >= 60) 
		{// 0045D929  |. 81BD E8FCFFFF >|CMP [LOCAL.198],0AA
			if(LOCAL_198 < 170)
				LOCAL_203 = LOCAL_203 * 90 / 100;
			else if(LOCAL_198 < 200)	
				LOCAL_203 = LOCAL_203 * 60 / 100;
			else if(LOCAL_198 < 230)
				LOCAL_203 = LOCAL_203 * 50 / 100;
			else if(LOCAL_198 < 255)
				LOCAL_203 = LOCAL_203 * 43 / 100;
			else
				LOCAL_203 = LOCAL_203 * 38 / 100;
		}
		else
		{
			if(LOCAL_205 == 0 || LOCAL_205 == 1 || LOCAL_205 == 2)
			{
				if(LOCAL_198 < 10)
					LOCAL_203 = LOCAL_203 * 3 / 10; 
				else if(LOCAL_198 < 20)
					LOCAL_203 = (LOCAL_203 << 2) / 10;
				else if(LOCAL_198 < 30)
					LOCAL_203 = LOCAL_203 * 5 / 10;
				else if(LOCAL_198 < 40)
					LOCAL_203 = LOCAL_203 * 6 / 10;
				else if(LOCAL_198 < 50)
					LOCAL_203 = LOCAL_203 * 7 / 10;
				else if(LOCAL_198 < 60)
					LOCAL_203 = (LOCAL_203 << 3) / 10;
			}
		}
			
		if(LOCAL_132 == 8 || LOCAL_132 == 9 || LOCAL_132 == 10)
			LOCAL_203 = 4;

		if(LOCAL_203 > 32000)
			LOCAL_203 = 32000;

		int realRate = LOCAL_203;
		if(LOCAL_203 > 0)
			LOCAL_203 = Rand () % LOCAL_203;
		else
			LOCAL_203 = 0;

		if(LOCAL_203 != 0 && LOCAL_132 != 11)
			continue;

		if(LOCAL_207->sIndex <= 0 || LOCAL_207->sIndex > 6500)
			continue;

		if(g_pItemList[LOCAL_207->sIndex].Level > 140)
			if((LOCAL_203 & 0x80000001) == 1)
				continue;

		int LOCAL_209 = 0;
		if(arg2 > 0 && arg2 < MAX_PLAYER)
			LOCAL_209 = pMob[arg2].DropBonus;

		// Fada dourada / Fada Prateada
		bool isValley = pMob[arg2].IsInsideValley();
		if (fada == 3914 || fada == 3915 || (isValley && fada == 3917))
		{
			constexpr std::array agruppedItems
			{ 
				412, 413, 419, 420,
				2390, 2391, 2392, 2393, 2394, 2395, 2396, 2397, 2398, 2399, 2400, 2401, 2402,
				2403, 2404, 2405, 2406, 2407, 2408, 2409, 2410, 2411, 2412, 2413, 2414, 2415,
				2416, 2417, 2418, 2419, 4016, 4017, 4018, 4019, 4020, 4850
			};

			std::vector<uint16_t> excludedItems
			{
				3140, 
				1774
			};

			if (isValley)
			{
				excludedItems.push_back(540);
				excludedItems.push_back(541);
			}

			auto itemId = LOCAL_207->sIndex;
			bool agroup = std::find(agruppedItems.begin(), agruppedItems.end(), LOCAL_207->sIndex) != agruppedItems.end();
			if (agroup)
			{
				bool bag1 = TimeRemaining(pMob[arg2].Mobs.Player.Inventory[60].EFV1, pMob[arg2].Mobs.Player.Inventory[60].EFV2, pMob[arg2].Mobs.Player.Inventory[60].EFV3 + 1900) > 0.0f;
				bool bag2 = TimeRemaining(pMob[arg2].Mobs.Player.Inventory[61].EFV1, pMob[arg2].Mobs.Player.Inventory[61].EFV2, pMob[arg2].Mobs.Player.Inventory[61].EFV3 + 1900) > 0.0f;

				int max = GetMaxAmountItem(LOCAL_207);
				int slotId = -1;
				for (int i = 0; i < 60; i++)
				{
					if (i >= 30 && !bag1 && !bag2)
						break;

					if (i >= 30 && i <= 44 && !bag1)
						continue;

					if (i >= 45 && i <= 59 && !bag2)
						continue;

					if (pMob[arg2].Mobs.Player.Inventory[i].sIndex == LOCAL_207->sIndex)
					{
						if (GetItemAmount(&pMob[arg2].Mobs.Player.Inventory[i]) < max)
						{
							slotId = i;

							break;
						}
					}
				}

				if (slotId != -1)
				{
					// fazemos a capia para 
					STRUCT_ITEM temporaryItem = *LOCAL_207;
					STRUCT_ITEM* dstItem = &pMob[arg2].Mobs.Player.Inventory[slotId];
					if (AgroupItem(arg2, &temporaryItem, dstItem))
					{
						Log(arg2, LOG_INGAME, "Item %s foi agrupado pela %s", g_pItemList[LOCAL_207->sIndex].ItemName, g_pItemList[fada].ItemName);

						SendItem(arg2, SlotType::Inv, slotId, dstItem);

						// Parte nao feita - 0045DCF2
						if (LOCAL_132 == 8 || LOCAL_132 == 9 || LOCAL_132 == 10)
							LOCAL_132 = 11;
						continue;
					}
				}
			}
			else if (fada == 3915 && (itemId == 4010 || itemId == 4011 || (itemId >= 4026 && itemId <= 4029)))
			{
				// fazemos a capia para nao apagarmos o item do inventario do mob
				STRUCT_ITEM temporaryItem = *LOCAL_207;

				if (GoldBar(arg2, SlotType::Inv, 0, &temporaryItem))
					Log(arg2, LOG_INGAME, "Barra de gold utilizado pela fada dourada");

				// Parte nao feita - 0045DCF2
				if (LOCAL_132 == 8 || LOCAL_132 == 9 || LOCAL_132 == 10)
					LOCAL_132 = 11;

				continue;
			}
			else if(std::find(excludedItems.begin(), excludedItems.end(), LOCAL_207->sIndex) == excludedItems.end())
			{
				// fazemos a capia para nao apagarmos o item do inventario do mob
				STRUCT_ITEM temporaryItem = *LOCAL_207;

				auto sellResult = SellItem(arg2, &temporaryItem);
				if (sellResult == eSellItemResult::Success)
				{
					Log(arg2, LOG_INGAME, "Item %s vendido pela %s - [%d] [%d %d %d %d %d %d] em %dx %dy - %s", g_pItemList[LOCAL_207->sIndex].ItemName, g_pItemList[fada].ItemName, LOCAL_207->sIndex, LOCAL_207->EF1, LOCAL_207->EFV1, LOCAL_207->EF2,
						LOCAL_207->EFV2, LOCAL_207->EF3, LOCAL_207->EFV3, pMob[arg2].Target.X, pMob[arg2].Target.Y, pMob[arg1].Mobs.Player.MobName);

					// Parte nao feita - 0045DCF2
					if (LOCAL_132 == 8 || LOCAL_132 == 9 || LOCAL_132 == 10)
						LOCAL_132 = 11;
					continue;
				}
				else
					Log(arg2, LOG_INGAME, "Item nao vendido pela fada pelo motivo de %d", (int)sellResult);
			}
		}

		SetItemBonus(LOCAL_207, pMob[arg1].Mobs.Player.CurrentScore.Level, 0, LOCAL_209);

		INT32 slotId = GetFirstSlot(arg2, 0);
		if(slotId == -1)
		{
			Log(arg2, LOG_INGAME, "Nao dropou o item %s por falta de espaao [%d] [%d %d %d %d %d %d] em %dx %dy - %s (1/%d)", g_pItemList[LOCAL_207->sIndex].ItemName, LOCAL_207->sIndex, LOCAL_207->EF1, LOCAL_207->EFV1, LOCAL_207->EF2,
				LOCAL_207->EFV2, LOCAL_207->EF3, LOCAL_207->EFV3, pMob[arg2].Target.X, pMob[arg2].Target.Y, pMob[arg1].Mobs.Player.MobName, realRate);

			LogPlayer(arg2, "Nao dropou o item %s no mob %s por falta de espaao no inventario", g_pItemList[LOCAL_207->sIndex].ItemName, pMob[arg1].Mobs.Player.MobName);
			SendClientMessage(arg2, "Seu inventario esta cheio");
		}
		else
		{
			memcpy(&pMob[arg2].Mobs.Player.Inventory[slotId], LOCAL_207, 8);
			SendItem(arg2, SlotType::Inv, slotId, LOCAL_207);

			Log(arg2, LOG_INGAME, "Dropado item %s [%d] [%d %d %d %d %d %d] em %dx %dy - %s (1/%d)", g_pItemList[LOCAL_207->sIndex].ItemName, LOCAL_207->sIndex, LOCAL_207->EF1, LOCAL_207->EFV1, LOCAL_207->EF2,
				LOCAL_207->EFV2, LOCAL_207->EF3, LOCAL_207->EFV3, pMob[arg2].Target.X, pMob[arg2].Target.Y, pMob[arg1].Mobs.Player.MobName, realRate);

			LogPlayer(arg2, "Dropado o item %s no mob %s", g_pItemList[LOCAL_207->sIndex].ItemName, pMob[arg1].Mobs.Player.MobName);

			auto& user = pUser[arg2];
			auto found = std::find_if(user.Dropped.Items.begin(), user.Dropped.Items.end(), [](const DroppedItem& dropped) {
				return dropped.Item.sIndex <= 0 || dropped.Item.sIndex >= MAX_ITEMLIST;
			});

			if (found == user.Dropped.Items.end())
			{
				if (++user.Dropped.LastIndex >= user.Dropped.Items.size())
					user.Dropped.LastIndex = 0;

				found = user.Dropped.Items.begin() + user.Dropped.LastIndex;
			}

			// Fazemos uma capia para ca
			found->Item = *LOCAL_207;
			found->SlotId = slotId;
			found->Time = std::chrono::steady_clock::now();
		}

		// Parte nao feita - 0045DCF2
		if(LOCAL_132 == 8 || LOCAL_132 == 9 || LOCAL_132 == 10)
			LOCAL_132 = 11;
	}

	// 0045DE03
	if(arg2 < MAX_PLAYER)
	{
		if(pMob[arg1].Mobs.Player.Equip[0].sIndex == 239 || pMob[arg1].Mobs.Player.Equip[0].sIndex == 241)
		{
			if(!(Rand() % 20))
			{
				INT32 LOCAL_278 = 1;
				INT32 LOCAL_279 = 1;
				INT32 LOCAL_280 = 2;

				LOCAL_278 = LOCAL_278 << (char)LOCAL_280;
				LOCAL_279 = LOCAL_279 << (char)((LOCAL_280 + 1));

				if((pMob[arg2].Mobs.Player.QuestInfo.Mystical_GetQuest) && !(pMob[arg2].Mobs.Player.QuestInfo.Mystical_GetAmuleto))
				{
					SendClientMessage(arg2, g_pLanguageString[_NN_Watching_Town_Success]);

					pMob[arg2].Mobs.Player.QuestInfo.Mystical_CanGetAmuleto = 1;
				}
			}
		}
	}

	if(pMob[arg2].Mobs.Player.AffectInfo.DrainHP)
	{
		INT32 LOCAL_281 = pMob[arg1].Mobs.Player.CurrentScore.MaxHp;
		INT32 LOCAL_282 = pMob[arg1].Mobs.Player.CurrentScore.Special[3];

		LOCAL_281 = LOCAL_281 / 30 + LOCAL_282;

		INT32 LOCAL_283 = pMob[arg2].Mobs.Player.CurrentScore.Hp;

		pMob[arg2].Mobs.Player.CurrentScore.Hp += LOCAL_283;
		if(pMob[arg2].Mobs.Player.CurrentScore.Hp > pMob[arg2].Mobs.Player.CurrentScore.MaxHp)
			pMob[arg2].Mobs.Player.CurrentScore.Hp = pMob[arg2].Mobs.Player.CurrentScore.MaxHp;

		if(LOCAL_283 != pMob[arg2].Mobs.Player.CurrentScore.Hp)
		{
			if(arg2 > 0 && arg2 < MAX_PLAYER)
			{
				SetReqHp(arg2);
				SetReqMp(arg2);
			}

			p18A LOCAL_287{};
			LOCAL_287.Header.PacketId = 0x18A;
			LOCAL_287.Header.Size = sizeof p18A;
			LOCAL_287.Header.ClientId = arg2;

			LOCAL_287.CurHP = pMob[arg2].Mobs.Player.CurrentScore.Hp;
			LOCAL_287.Incress = LOCAL_287.CurHP - LOCAL_283;

			INT32 LOCAL_288 = pMob[arg2].Target.X;
			INT32 LOCAL_289 = pMob[arg2].Target.Y;

			GridMulticast_2(LOCAL_288, LOCAL_289, (BYTE*)&LOCAL_287, 0);
		}
	}

	if (sServer.DropArea.CurrentScore && pUser[arg2].DropEvent.IsValid)
	{
		bool hasPotion = false;
		for (int i = 0; i < 32; i++)
		{
			if (pMob[arg2].Mobs.Affects[i].Index == 59)
			{
				hasPotion = true;

				break;
			}
		}

		bool isPesa = false;
		for (int i = 1; i < 3; i++)
		{
			if (pMob[arg2].Target.X >= g_pPesaArea[i][0] && pMob[arg2].Target.X <= g_pPesaArea[i][2] && pMob[arg2].Target.Y >= g_pPesaArea[i][1] && pMob[arg2].Target.Y <= g_pPesaArea[i][3])
				isPesa = true;
		}

		bool isWater = false;
		for (int i = 0; i < 3; ++i)
		{
			for (int x = 0; x < 9; ++x)
			{
				if (pMob[arg2].Target.X >= waterMaxMin[i][x][0] && pMob[arg2].Target.X <= waterMaxMin[i][x][2] && pMob[arg2].Target.Y >= waterMaxMin[i][x][1] && pMob[arg2].Target.Y <= waterMaxMin[i][x][3])
					isWater = true;
			}

			if (isWater)
				break;
		}

		if (!isPesa && !isWater)
		{
			for (const auto& area : sServer.DropArea.areas)
			{
				if (area.Limit != 0 && area.Dropped >= area.Limit)
					continue;

				if (pMob[arg2].Target.X < area.Min.X || pMob[arg2].Target.X > area.Max.X || pMob[arg2].Target.Y < area.Min.Y || pMob[arg2].Target.Y > area.Max.Y)
					continue;

				int rate = area.Rate;

				int fada = pMob[arg2].Mobs.Player.Equip[13].sIndex;
				if (fada == 3901 || fada == 3904 || fada == 3905 || fada == 3907 || fada == 3908 || fada == 3914)
					rate -= (rate * 15 / 100);

				if (fada == 3915)
					rate -= (rate * 18 / 100);

				if (fada == 3902)
					rate -= (rate * 10 / 100);

				if (hasPotion)
					rate -= (rate * 30 / 100);

				int rand = Rand() % rate;
				if (rand != 0)
					break;

				const STRUCT_ITEM& item = area.Item;
				int slotId = GetFirstSlot(arg2, 0);
				if (slotId == -1)
				{
					SendClientMessage(arg2, "Falta espaao no inventario");

					Log(arg2, LOG_INGAME, "Nao dropou o item do evento %s por falta de espaao no inventario", item.toString().c_str());
					break;
				}

				area.Dropped++;

				pUser[arg2].DropEvent.Dropped++;

				if (sServer.DropArea.Message)
				{
					if (area.Limit == 0)
						SendNotice("%s dropou o item %s. Total dropado: %d", pMob[arg2].Mobs.Player.MobName, g_pItemList[item.sIndex].ItemName, area.Dropped);
					else
						SendNotice("%s dropou o item %s. Total dropado: %d de %d", pMob[arg2].Mobs.Player.MobName, g_pItemList[item.sIndex].ItemName, area.Dropped, area.Limit);
				}

				Log(arg2, LOG_INGAME, "Foi dropado o item do evento %s. Rate: %d. Gerado: %d. Total de dropados: %d", item.toString().c_str(), area.Rate, rand, pUser[arg2].DropEvent.Dropped);

				pMob[arg2].Mobs.Player.Inventory[slotId] = item;
				SendItem(arg2, SlotType::Inv, slotId, &pMob[arg2].Mobs.Player.Inventory[slotId]);
				break;
			}
		}
	}

	DeleteMob(arg1, 1);
}

void ClearAreaTeleport(unsigned int x1, unsigned int y1, unsigned int x2, unsigned int y2, unsigned int DestX, unsigned int DestY, const char* areaName = "")
{	//0x00401596
	int LOCAL_1 = 1;

	for(; LOCAL_1 < MAX_PLAYER; LOCAL_1++)
	{
		if(pUser[LOCAL_1].Status != USER_PLAY)
			continue;

		if(!pMob[LOCAL_1].Mode)
			continue;

		if(pMob[LOCAL_1].Target.X >= x1 && pMob[LOCAL_1].Target.X <= x2 &&
			pMob[LOCAL_1].Target.Y >= y1 && pMob[LOCAL_1].Target.Y <= y2)
		{
			if(pMob[LOCAL_1].Mobs.Player.CurrentScore.Hp <= 0)
			{
				pMob[LOCAL_1].Mobs.Player.CurrentScore.Hp = 1;
				SendScore(LOCAL_1);
			}
		
			Log(LOCAL_1, LOG_INGAME, "Enviado para a cidade atravas do ClearAreaTeleport. Posiaao: %ux %uy. Teleportado para \"%s\"", pMob[LOCAL_1].Target.X, pMob[LOCAL_1].Target.Y, areaName);
			Teleportar(LOCAL_1, DestX, DestY);
		}
	}
}

void ClearArea(unsigned int x1, unsigned int y1, unsigned int x2, unsigned int y2)
{	//0x00401596
	int LOCAL_1 = 0;

	for (; LOCAL_1 < MAX_PLAYER; LOCAL_1++)
	{
		if (pUser[LOCAL_1].Status != USER_PLAY)
			continue;

		if (!pMob[LOCAL_1].Mode)
			continue;

		if (pMob[LOCAL_1].Target.X < x1 || pMob[LOCAL_1].Target.X > x2 ||
			pMob[LOCAL_1].Target.Y < y1 || pMob[LOCAL_1].Target.Y > y2)
			continue;

		if (pMob[LOCAL_1].Mobs.Player.CurrentScore.Hp <= 0)
		{
			pMob[LOCAL_1].Mobs.Player.CurrentScore.Hp = 1;
			SendScore(LOCAL_1);
		}

		Log(LOCAL_1, LOG_INGAME, "Enviado para a cidade atravas do ClearArea. Posiaao: %dx %dy", pMob[LOCAL_1].Target.X, pMob[LOCAL_1].Target.Y);
		DoRecall(LOCAL_1);
	}
}

void ClearArea(unsigned int x1, unsigned int y1, unsigned int x2, unsigned int y2, const char* questName)
{	//0x00401596
	int LOCAL_1 = 0;

	for (; LOCAL_1 < MAX_PLAYER; LOCAL_1++)
	{
		if (pUser[LOCAL_1].Status != USER_PLAY)
			continue;

		if (!pMob[LOCAL_1].Mode)
			continue;

		if (pMob[LOCAL_1].Target.X < x1 || pMob[LOCAL_1].Target.X > x2 ||
			pMob[LOCAL_1].Target.Y < y1 || pMob[LOCAL_1].Target.Y > y2)
			continue;

		Log(LOCAL_1, LOG_INGAME, "Enviado para a cidade atravas do ClearArea. Posiaao: %dx %dy - Quest %s", pMob[LOCAL_1].Target.X, pMob[LOCAL_1].Target.Y, questName);
		DoRecall(LOCAL_1);
	}
}

void DoRemoveHide(INT32 clientId)
{
	if(clientId <= 0 || clientId >= MAX_PLAYER)
		return;

	for (INT32 i = 0; i < 32; i++)
	{
		if (pMob[clientId].Mobs.Affects[i].Index == 28)
		{
			memset(&pMob[clientId].Mobs.Affects[i], 0, sizeof STRUCT_AFFECT);

			pMob[clientId].GetCurrentScore(clientId);

			SendScore(clientId);
		}
	}
}

void DoRemovePossuido(INT32 clientId)
{
	if (clientId <= 0 || clientId >= MAX_PLAYER)
		return;

	for (INT32 i = 0; i < 32; i++)
	{
		if (pMob[clientId].Mobs.Affects[i].Index == 24)
		{
			memset(&pMob[clientId].Mobs.Affects[i], 0, sizeof STRUCT_AFFECT);

			pMob[clientId].GetCurrentScore(clientId);

			SendScore(clientId);
		}
	}
}

int CreateItem(int posX, int posY, STRUCT_ITEM *item, int unknow, int arg4, int status)
{
	if(item->sIndex <= 0 || item->sIndex >= MAX_ITEMLIST)
		return 0;

	GetEmptyItemGrid(&posX, &posY);

	// Checagem de terreno nao feita
	if(g_pItemGrid[posY][posX])
		return 0;

	INT32 LOCAL_1 = GetEmptyItem();
	if(LOCAL_1 == 0)
		return 0;

	g_pInitItem[LOCAL_1].Open = 1;
	g_pInitItem[LOCAL_1].PosX = posX;
	g_pInitItem[LOCAL_1].PosY = posY;

	memcpy(&g_pInitItem[LOCAL_1].Item, item, sizeof STRUCT_ITEM);

	g_pInitItem[LOCAL_1].Rotation		= unknow;
	g_pInitItem[LOCAL_1].CurrentScore		= status;
	g_pInitItem[LOCAL_1].IsOpen		= 90;
	g_pInitItem[LOCAL_1].Unknow_36	= 0;
	g_pInitItem[LOCAL_1].CanRun		= GetItemAbility(item, EF_GROUND);

	g_pItemGrid[posY][posX]			= LOCAL_1;
	g_pInitItem[LOCAL_1].HeightGrid	= g_pHeightGrid[posY][posX];
	
	p26E packet;
	GetCreateItem(LOCAL_1, &packet);

	GridMulticast_2(posX, posY, (BYTE*)&packet, 0);
	return LOCAL_1;
}

void SetCurKill(int sIndex, int cFrag)
{
	if(cFrag < 0)
		cFrag = 0;
	else if(cFrag > 200) 
		cFrag = 200;

	pMob[sIndex].Mobs.Player.Inventory[63].EFV1 = cFrag;
}

void SetGuilty(int Cid, int arg_2)
{//0x00401488
	if(Cid <= 0 || Cid >= MAX_PLAYER)
	    return;

	if(arg_2 < 0)
		arg_2 = 0;
	else if(arg_2 > 50)
		arg_2 = 50;

	int LOCAL_1 = arg_2;
	pMob[Cid].Mobs.Player.Inventory[63].EF2 = LOCAL_1;
}

void SetTotKill(int sIndex, int tFrag)
{
	if (tFrag < 0)
		tFrag = 0;

	if (tFrag > 32767)
		tFrag = 32767;

	pMob[sIndex].Mobs.Player.Inventory[63].EFV2 = tFrag % 256;
	pMob[sIndex].Mobs.Player.Inventory[63].EFV3 = tFrag >> 8;
}

void SetItemBonus(STRUCT_ITEM* item)
{
	int LOCAL_4 = item->sIndex;
	int LOCAL_5 = g_pItemList[item->sIndex].Unique;
	int LOCAL_7 = g_pItemList[item->sIndex].Pos;

	int LOCAL_9 = 0;

	INT32 ability = GetItemAbility(item, EF_MOBTYPE);
	if ((LOCAL_7 & 254) && LOCAL_7 != 128)
	{
		int LOCAL_10 = 59;
		int LOCAL_11 = 0;
		int LOCAL_12 = 0;

		int LOCAL_13 = Rand() % 101;

		int LOCAL_14 = 100;

		LOCAL_14 = LOCAL_13 % 3;

		if (LOCAL_7 == 2)
		{
			LOCAL_10 = EF_ATTSPEED;
			LOCAL_12 = 3;

			if (LOCAL_14 == 1)
			{
				LOCAL_10 = EF_MAGIC;
				LOCAL_12 = 2;
			}
		}
		else if (LOCAL_7 == 4 || LOCAL_7 == 8)
		{
			LOCAL_10 = EF_CRITICAL2;
			LOCAL_12 = 10;
			LOCAL_11 = 1;

			if (LOCAL_14 == 2)
			{
				LOCAL_10 = EF_AC;
				LOCAL_12 = 5;
				LOCAL_11 = 0;
			}
		}
		else if (LOCAL_7 == 16)
		{
			LOCAL_10 = EF_ACADD2;
			LOCAL_12 = 5;
		}
		else if (LOCAL_7 == 32)
		{
			LOCAL_10 = EF_DAMAGE2;
			LOCAL_12 = 6;
			LOCAL_11 = -1;
		}
		else if (LOCAL_7 == 64 || LOCAL_7 == 192)
		{
			if (LOCAL_5 == 44 || LOCAL_5 == 47)
			{
				switch (LOCAL_14)
				{
				case 0:
					LOCAL_10 = EF_MAGIC;
					LOCAL_12 = 4;
					LOCAL_11 = -1;
					break;
				case 1:
					LOCAL_10 = EF_SPECIALALL;
					LOCAL_12 = 3;
					LOCAL_11 = -1;
					break;
				}
			}
			else if (LOCAL_14 == 0)
			{
				LOCAL_10 = EF_ATTSPEED;
				LOCAL_12 = 3;
				LOCAL_11 = 1;
			}
			else if (LOCAL_14 == 1)
			{
				LOCAL_10 = EF_DAMAGE;
				LOCAL_12 = 9;
				LOCAL_11 = -1;
			}
			else if (LOCAL_14 == 2)
			{
				LOCAL_10 = EF_SPECIALALL;
				LOCAL_12 = 3;
			}
		}

		int LOCAL_15 = Rand() % 100;
		int LOCAL_16 = LOCAL_15 % 3;

		int LOCAL_17 = 59;
		int LOCAL_18 = 0;
		int LOCAL_19 = 0;

		switch (LOCAL_7)
		{
		case 2:
			LOCAL_17 = EF_HP;
			LOCAL_18 = 10;

			if (LOCAL_16 == 1)
			{
				LOCAL_17 = EF_AC;
				LOCAL_18 = 5;
				LOCAL_19 = -1;
			}
			break;
		case 4:
		case 8:
			if (LOCAL_16 == 2)
			{
				LOCAL_17 = EF_AC;
				LOCAL_18 = 5;
				LOCAL_19 = -1;
			}
			else if (LOCAL_16 == 0)
			{
				LOCAL_17 = EF_MAGIC;
				LOCAL_18 = 2;
				LOCAL_19 = -1;
			}
			else if (LOCAL_16 == 1)
			{
				LOCAL_17 = EF_DAMAGE;
				LOCAL_18 = 6;
				LOCAL_19 = -1;
			}
			break;
		case 16:
			if (LOCAL_16 == 0)
			{
				LOCAL_17 = EF_MAGIC;
				LOCAL_18 = 2;
				LOCAL_19 = -1;
			}
			else if (LOCAL_16 == 1)
			{
				LOCAL_17 = EF_DAMAGE;
				LOCAL_18 = 6;
				LOCAL_19 = -1;
			}
			else if (LOCAL_16 == 2)
			{
				LOCAL_17 = EF_SPECIALALL;
				LOCAL_18 = 3;
			}
			break;
		case 32:
			if (LOCAL_16 == 0)
			{
				LOCAL_17 = EF_MAGIC;
				LOCAL_18 = 2;
				LOCAL_19 = -1;
			}
			else if (LOCAL_16 == 1)
			{
				LOCAL_17 = EF_SPECIALALL;
				LOCAL_18 = 3;
			}
			break;
		case 64:
		case 128:
		case 192:
			if (LOCAL_5 == 44 || LOCAL_5 == 47)
			{
				if (LOCAL_16 == 0)
				{
					LOCAL_17 = EF_MAGIC;
					LOCAL_18 = 4;
					LOCAL_19 = -1;
				}
				else if (LOCAL_16 == 1)
				{
					LOCAL_17 = EF_SPECIALALL;
					LOCAL_18 = 3;
					LOCAL_19 = 1;
				}
			}
			else
			{
				if (LOCAL_16 == 0)
				{
					LOCAL_17 = EF_ATTSPEED;
					LOCAL_18 = 3;
					LOCAL_19 = 1;
				}
				else if (LOCAL_16 == 1)
				{
					LOCAL_17 = EF_DAMAGE;
					LOCAL_18 = 9;
					LOCAL_19 = -1;
				}
				else if (LOCAL_16 == 2)
				{
					LOCAL_17 = EF_SPECIALALL;
					LOCAL_18 = 3;
					LOCAL_19 = 0x01;
				}
			}
			break;
		}

		int LOCAL_20 = 0;
		LOCAL_13 = Rand() % 100;

		if (LOCAL_13 <= 20)
			LOCAL_20 = 6;
		else if (LOCAL_13 < 60)
			LOCAL_20 = 5;
		else if (LOCAL_13 < 80)
			LOCAL_20 = 4;
		else
			LOCAL_20 = 3;

		if (LOCAL_9 != 0 && LOCAL_20 < 4)
			LOCAL_20 = 4;

		LOCAL_20 += LOCAL_11;

		if (ability == 1 && LOCAL_20 >= 3)
			LOCAL_20 = 2;

		if (!item->stEffect[1].cEffect && LOCAL_20 > 0)
		{
			// EBP - 28 = LOCAL_10
			item->stEffect[1].cEffect = LOCAL_10;
			item->stEffect[1].cValue = LOCAL_20 * LOCAL_12;
		}
		else if (!item->stEffect[1].cEffect && LOCAL_20 <= 0 && LOCAL_7 == 32)
		{
			item->stEffect[1].cEffect = LOCAL_10;
			item->stEffect[1].cValue = 0;
		}
		else if (!item->stEffect[1].cEffect)
		{
			item->stEffect[1].cEffect = EF_UNIQUE;
			item->stEffect[1].cValue = Rand() % 127;
		}

		int LOCAL_21 = 0;
		LOCAL_15 = Rand() % 0x64;

		if (LOCAL_15 <= 20) // 2
			LOCAL_21 = 6;
		else if (LOCAL_15 < 60) // 9
			LOCAL_21 = 5;
		else if (LOCAL_13 < 80)
			LOCAL_21 = 4;
		else
			LOCAL_21 = 3;

		if (LOCAL_9 != 0 && LOCAL_21 < 3)
			LOCAL_21 = 0x03;

		LOCAL_21 += LOCAL_19;

		if (ability == 1 && LOCAL_21 >= 3)
			LOCAL_21 = 2;

		// Limita o adicional dano para 45 + 27
		if (LOCAL_7 >= 64 && LOCAL_17 == EF_DAMAGE && ((LOCAL_21 >= 4 && LOCAL_20 >= 5) || (LOCAL_21 >= 5 && LOCAL_20 >= 4)))
			LOCAL_21 = 3;

		// Limita o adicional dano para 45 + 27
		if (LOCAL_7 >= 64 && LOCAL_17 == EF_MAGIC && ((LOCAL_21 >= 4 && LOCAL_20 >= 5) || (LOCAL_21 >= 5 && LOCAL_20 >= 4)))
			LOCAL_21 = 3;

		if (LOCAL_21 > 0x00 && !item->stEffect[2].cEffect)
		{
			item->stEffect[2].cEffect = LOCAL_17;
			item->stEffect[2].cValue = LOCAL_21 * LOCAL_18;
		}

		if (!(Rand() % 80))
		{
			int LOCAL_22 = Rand() % 0x064;

			int LOCAL_23 = 6;
			int LOCAL_24 = 35;
			int LOCAL_25 = 85;
			int LOCAL_26 = 100;

			LOCAL_23 = 6;
			LOCAL_24 = 35;
			LOCAL_25 = 70;
			LOCAL_25 = 85;
			LOCAL_26 = 100;

			if (LOCAL_22 < LOCAL_26)
			{ // Aqui ele gera adicionais tipo: Inteligancia, foraa e tal ^^ 
				for (int i = 0; i < 3; i++)
				{
					int LOCAL_28 = Rand() % 0x0A;
					int LOCAL_29 = dwValue[LOCAL_28];
					int LOCAL_30 = 2;
					int LOCAL_31 = dwValue_02[LOCAL_28][LOCAL_30][0];
					int LOCAL_32 = dwValue_02[LOCAL_28][LOCAL_30][1];

					int LOCAL_33 = LOCAL_32 + 1 - LOCAL_31;

					if (LOCAL_33 == 0)
						LOCAL_33 = 1;

					int LOCAL_34 = Rand() % LOCAL_33 + LOCAL_31;

					item->stEffect[i].cEffect = LOCAL_29;
					item->stEffect[i].cValue = LOCAL_34;
				}
			}
		}
	}
}
void SetItemBonus(STRUCT_ITEM *item, int level, int sanc, int bonus)
{
	INT32 LOCAL_1 = bonus >> 3;
	
	if(LOCAL_1 < 0)
		LOCAL_1 = 0;

	if(LOCAL_1 > 2)
		LOCAL_1 = 2;

	INT32 LOCAL_2 = -1;
	INT32 LOCAL_3 = -1;

	if(item->stEffect[0].cEffect >= 100 && item->stEffect[0].cEffect <= 105)
	{
		LOCAL_2 = item->stEffect[0].cEffect - 100;
		LOCAL_3 = item->stEffect[0].cValue;

		item->stEffect[0].cEffect = 0;
		item->stEffect[0].cValue = 0;
	}
	
	if(sanc == 0 && level >= 210)
		level -= 47;

	int LOCAL_4 = item->sIndex;
	int LOCAL_5 = g_pItemList[item->sIndex].Unique;
	int LOCAL_6 = g_pItemList[item->sIndex].Level; // LEVEL
	int LOCAL_7 = g_pItemList[item->sIndex].Pos;

	int LOCAL_8 = (level - LOCAL_6) / 25;
	if(LOCAL_2 != -1)
		LOCAL_8 = LOCAL_2;

	//0044443D  |. A3 B4695C01    MOV DWORD PTR DS:[15C69B4],EAX -> Reparei que nao a utilizado ^^ 

	int LOCAL_9 = 0;

	if(LOCAL_8 >= 4)
		LOCAL_9 = 1;

	if(LOCAL_8 < 0)
		LOCAL_8 = 0;

	if(LOCAL_8 > 3)
		LOCAL_8 = 3;

	if(sanc != 0)
		if(LOCAL_8 >= 3)
			LOCAL_8 = 3;
	
	INT32 ability = GetItemAbility(item, EF_MOBTYPE);
	if((LOCAL_7 & 254) && !item->stEffect[0].cEffect && LOCAL_7 != 128)
	{
		int LOCAL_10 = 59;
		int LOCAL_11 = 0;
		int LOCAL_12 = 0;
			
		int LOCAL_13 = Rand() % 101;

		int LOCAL_14 = 100;

		// Nao original, nao sei o que fazem aqui ^^ 
		if(LOCAL_1 == 0)
			LOCAL_1 = 1;

		if(LOCAL_13 == 0)
			LOCAL_13 = 1;

		if(sanc != 0)
			LOCAL_14 = LOCAL_13 % 3;
		else if(LOCAL_8 == 0)
			LOCAL_14 = LOCAL_13 % (7 - LOCAL_1);
		else if(LOCAL_8 == 1)
			LOCAL_14 = LOCAL_13 % (5 - LOCAL_1);
		else if(LOCAL_8 == 2)
			LOCAL_14 = LOCAL_13 % (5 - LOCAL_1);
		else if(LOCAL_8 >= 3)
			LOCAL_14 = LOCAL_13 & 0x80000003;

		if(LOCAL_7 == 2)
		{
			if(LOCAL_14 == 0)
			{
				LOCAL_10 = EF_ATTSPEED; 
				LOCAL_12 = 3;
			}
			else if(LOCAL_14 == 1)
			{
				LOCAL_10 = EF_MAGIC;
				LOCAL_12 = 2;
			}
		}
		else if(LOCAL_7 == 4 || LOCAL_7 == 8)
		{
			LOCAL_10 = EF_CRITICAL2;
			LOCAL_12 = 10;
			LOCAL_11 = 1;

			if(LOCAL_14 == 2)
			{
				LOCAL_10 = EF_AC;
				LOCAL_12 = 5;
				LOCAL_11 = 0;
			}
		}
		else if(LOCAL_7 == 16)
		{
			LOCAL_10 = EF_ACADD2;
			LOCAL_12 = 5;
		}
		else if(LOCAL_7 == 32)
		{
			LOCAL_10 = EF_DAMAGE2;
			LOCAL_12 = 6;
			LOCAL_11 = -1;
		}
		else if(LOCAL_7 == 64 || LOCAL_7 == 192)
		{
			if(LOCAL_5 == 44 || LOCAL_5 == 47)
			{
				switch(LOCAL_14)
				{
					case 0:
						LOCAL_10 = EF_MAGIC;
						LOCAL_12 = 4;
						LOCAL_11 = -1;
						break;
					case 1:
						LOCAL_10 = EF_SPECIALALL;
						LOCAL_12 = 3;
						LOCAL_11 = -1;
						break;
				}
			}
			else if(LOCAL_14 == 0)
			{
				LOCAL_10 = EF_ATTSPEED;
				LOCAL_12 = 3;
				LOCAL_11 = 1;
			}
			else if(LOCAL_14 == 1) 
			{
				LOCAL_10 = EF_DAMAGE;
				LOCAL_12 = 9;
				LOCAL_11 = -1;
			}
			else if(LOCAL_14 == 2)
			{
				LOCAL_10 = EF_SPECIALALL;
				LOCAL_12 = 3;
			}
		}

		int LOCAL_15 = Rand() % 100;
		if(sanc != 0)
			LOCAL_15 = (LOCAL_15 << 1) / 3;
	
		int LOCAL_16 = 100;
		if(sanc != 0)
			LOCAL_16 = LOCAL_15 & 0x80000003;
		else if(LOCAL_8 == 0)
			LOCAL_16 = LOCAL_15 & 0x80000007;
		else if(LOCAL_8 == 1 || LOCAL_8 == 2)
			LOCAL_16 = LOCAL_15 % 6;
		else if(LOCAL_8 >= 3)
			LOCAL_16 = LOCAL_15 & 0x80000003;

		int LOCAL_17 = 59;
		int LOCAL_18 = 0;
		int LOCAL_19 = 0;

		switch(LOCAL_7)
		{
			case 2:
				if(LOCAL_16 == 0)
				{
					LOCAL_17 = EF_HP;
					LOCAL_18 = 10;
				}
				else if(LOCAL_16 == 1)
				{
					LOCAL_17 = EF_AC;
					LOCAL_18 = 5;
					LOCAL_19 = -1;
				}
				break;
			case 4:
			case 8:
				if(LOCAL_16 == 2)
				{
					LOCAL_17 = EF_AC;
					LOCAL_18 = 5;
					LOCAL_19 = -1;
				}
				else if(LOCAL_16 == 0)
				{
					LOCAL_17 = EF_MAGIC;
					LOCAL_18 = 2;
					LOCAL_19 = -1;
				}
				else if(LOCAL_16 == 1)
				{
					LOCAL_17 = EF_DAMAGE;
					LOCAL_18 = 6;
					LOCAL_19 = -1;
				}
				break;
			case 16:
				if(LOCAL_16 == 0)
				{
					LOCAL_17 = EF_MAGIC;
					LOCAL_18 = 2;
					LOCAL_19 = -1;
				}
				else if(LOCAL_16 == 1)
				{
					LOCAL_17 = EF_DAMAGE;
					LOCAL_18 = 6;
					LOCAL_19 = -1;
				}
				else if(LOCAL_16 == 2)
				{
					LOCAL_17 = EF_SPECIALALL;
					LOCAL_18 = 3;
				}
				else if(LOCAL_16 == 3)
				{
					LOCAL_17 = EF_RESISTALL;
					LOCAL_18 = 3;
				}
				break;
			case 32:
				if(LOCAL_16 == 0)
				{
					LOCAL_17 = EF_MAGIC;
					LOCAL_18 = 2;
					LOCAL_19 = -1;
				}
				else if(LOCAL_16 == 1)
				{
					LOCAL_17 = EF_SPECIALALL;
					LOCAL_18 = 3;
				}
				break;
			case 64:
			case 128:
			case 192:
				if(LOCAL_5 == 44 || LOCAL_5 == 47)
				{
					if(LOCAL_16 == 0)
					{
						LOCAL_17 = EF_MAGIC;
						LOCAL_18 = 4;
						LOCAL_19 = -1;
					}
					else if(LOCAL_16 == 1)
					{
						LOCAL_17 = EF_SPECIALALL;
						LOCAL_18 = 3;
						LOCAL_19 = 1;
					}
				}
				else
				{
					if(LOCAL_16 == 0)
					{
						LOCAL_17 = EF_ATTSPEED;
						LOCAL_18 = 3;
						LOCAL_19 = 1;
					}
					else if(LOCAL_16 == 1)
					{
						LOCAL_17 = EF_DAMAGE;
						LOCAL_18 = 9;
						LOCAL_19 = -1;
					}
					else if(LOCAL_16 == 2)
					{
						LOCAL_17 = EF_SPECIALALL;
						LOCAL_18 = 3;
						LOCAL_19 = 0x01;
					}
				}
				break;
		}

		int LOCAL_20 = 0;
		LOCAL_13 = Rand () % 100;

		if(LOCAL_8 == 0)
		{
			if(LOCAL_13 < 2)
				LOCAL_20 = 4;
			else if(LOCAL_13 < 6)
				LOCAL_20 = 3;
			else if(LOCAL_13 < 24)
				LOCAL_20 = 2;
			else if(LOCAL_13 < 55)
				LOCAL_20 = 1;
			else 
				LOCAL_20 = 0;
		}
		else if(LOCAL_8 == 1)
		{
			if(LOCAL_13 < 3)
				LOCAL_20 = 5;
			else if(LOCAL_13 < 12)
				LOCAL_20 = 4;
			else if(LOCAL_13 < 24)
				LOCAL_20 = 3;
			else if(LOCAL_13 < 65)
				LOCAL_20 = 2;
			else 
				LOCAL_20 = 1;
		}
		else if(LOCAL_8 == 2)
		{
			if(LOCAL_13 < 5)
				LOCAL_20 = 5;
			else if(LOCAL_13 < 20)
				LOCAL_20 = 4;
			else if(LOCAL_13 < 60)
				LOCAL_20 = 3;
			else 
				LOCAL_20 = 2;
		}
		else if(LOCAL_8 == 3)
		{
			if(LOCAL_13 <= 15)
				LOCAL_20 = 6;
			else if(LOCAL_13 < 40)
				LOCAL_20 = 5;
			else if(LOCAL_13 < 80)
				LOCAL_20 = 4;
			else
				LOCAL_20 = 3;
		}
		else if(LOCAL_8 >= 4)
		{
			if(LOCAL_13 < 2)
				LOCAL_20 = 7;
			else if(LOCAL_13 < 6)
				LOCAL_20 = 6;
			else if(LOCAL_13 < 17)
				LOCAL_20 = 5;
			else
				LOCAL_20 = 4;
		}

		if(LOCAL_9 != 0 && LOCAL_20 < 4)
			LOCAL_20 = 4;

		LOCAL_20 += LOCAL_11;

		if(sanc != 0 && LOCAL_20 == 0)
			LOCAL_20 = 1;

		if(ability == 1 && LOCAL_20 >= 3)
			LOCAL_20 = 2;

		if(!item->stEffect[1].cEffect && LOCAL_20 > 0)
		{
			// EBP - 28 = LOCAL_10
			item->stEffect[1].cEffect = LOCAL_10;
			item->stEffect[1].cValue = LOCAL_20 * LOCAL_12;
		}
		else if(!item->stEffect[1].cEffect && LOCAL_20 <= 0 && LOCAL_7 == 32)
		{
			item->stEffect[1].cEffect = LOCAL_10;
			item->stEffect[1].cValue = 0;
		}
		else if(!item->stEffect[1].cEffect)
		{
			item->stEffect[1].cEffect = EF_UNIQUE;
			item->stEffect[1].cValue = Rand() % 127;
		}

		int LOCAL_21 = 0;

		LOCAL_15 = Rand() % 0x64;

		if(LOCAL_8 == 0)
		{
			if(LOCAL_15 < 0x02)
				LOCAL_21 = 4;
			else if(LOCAL_15 < 0x06)
				LOCAL_21 = 3;
			else if(LOCAL_15 < 0x18)
				LOCAL_21 = 2;
			else if(LOCAL_15 < 0x037)
				LOCAL_21 = 1;
			else 
				LOCAL_21 = 0;
		}
		else if(LOCAL_8 == 1)
		{
			if(LOCAL_15 < 3)
				LOCAL_21 = 5;
			else if(LOCAL_15 < 12)
				LOCAL_21 = 4;
			else if(LOCAL_15 < 24)
				LOCAL_21 = 3;
			else if(LOCAL_15 < 65)
				LOCAL_21 = 2;
			else 
				LOCAL_21 = 1;
		}
		else if(LOCAL_8 == 2)
		{
			if(LOCAL_15 < 7)
				LOCAL_21 = 5;
			else if(LOCAL_15 < 20) // 16
				LOCAL_21 = 4;
			else if(LOCAL_15 < 60)
				LOCAL_21 = 3;
			else 
				LOCAL_21 = 2;
		}
		else if(LOCAL_8 == 3)
		{
			if(LOCAL_15 <= 15) // 2
				LOCAL_21 = 6;
			else if(LOCAL_15 < 40) // 9
				LOCAL_21 = 5;
			else if(LOCAL_13 < 80)
				LOCAL_21 = 4;
			else
				LOCAL_21 = 3;
		}
		else if(LOCAL_8 >= 4)
		{
			if(LOCAL_15 < 2)
				LOCAL_21 = 7;
			else if(LOCAL_15 < 7)
				LOCAL_21 = 6;
			else if(LOCAL_15 < 17)
				LOCAL_21 = 5;
			else if(LOCAL_15 < 75)
				LOCAL_21 = 4;
			else
				LOCAL_21 = 3;
		}

		if(LOCAL_9 != 0 && LOCAL_21 < 3)
			LOCAL_21 = 0x03;

		if(sanc != 0 && LOCAL_21 >= 0x05)
			LOCAL_21 = 0x04;

		LOCAL_21 += LOCAL_19;

		if(LOCAL_1 != 0 && LOCAL_21 == 0)
			LOCAL_21 = LOCAL_1;

		if(sanc != 0 && LOCAL_21 == 0)
			LOCAL_21 = 0x01;
		
		if(ability == 1 && LOCAL_21 >= 3)
			LOCAL_21 = 2;

		// Limita o adicional dano para 45 + 27
		if(LOCAL_7 >= 64 && LOCAL_17 == EF_DAMAGE && ((LOCAL_21 >= 4 && LOCAL_20 >= 5) || (LOCAL_21 >= 5 && LOCAL_20 >= 4)))
			LOCAL_21 = 3;

		// Limita o adicional dano para 45 + 27
		if(LOCAL_7 >= 64 && LOCAL_17 == EF_MAGIC && ((LOCAL_21 >= 4 && LOCAL_20 >= 5) || (LOCAL_21 >= 5 && LOCAL_20 >= 4)))
			LOCAL_21 = 3;

		if(LOCAL_21 > 0x00 && !item->stEffect[2].cEffect)
		{
			item->stEffect[2].cEffect = LOCAL_17;
			item->stEffect[2].cValue = LOCAL_21 * LOCAL_18;
		}
		else if(!item->stEffect[2].cEffect)
		{
			item->stEffect[2].cEffect = EF_UNIQUE;
			item->stEffect[2].cValue = Rand() % 0x7F;
		}
			
		if(!item->stEffect[0].cEffect)
		{
			int LOCAL_22 = Rand() % 0x064;

			if(sanc != 0)
				LOCAL_22 >>= 1;

			int LOCAL_23 = 1;
			int LOCAL_24 = 12;
			int LOCAL_25 = 45;
			int LOCAL_26 = 70;

			if(LOCAL_8 >= 0x03 || LOCAL_8 == 0x02)
			{
				LOCAL_23 = 6;
				LOCAL_24 = 35;
				LOCAL_25 = 70;
				LOCAL_25 = 85;
				LOCAL_26 = 100;
			}

			if(LOCAL_8 == 0x01 || LOCAL_8 == 0x0)
			{
				LOCAL_23 = 6;
				LOCAL_24 = 2;
				LOCAL_25 = 55;
				LOCAL_25 = 75;
				LOCAL_26 = 90;
			}

			if(LOCAL_22 < LOCAL_23)
			{
				item->stEffect[0].cEffect = 43;
				item->stEffect[0].cValue = 2;

				if(LOCAL_3 > 0x02)
				{
					int LOCAL_27 = Rand() % 100;

					if(LOCAL_3 == 3)
					{
						if(LOCAL_27 < 30)
							item->stEffect[0].cValue = 3;
					}
					else if(LOCAL_3 == 4)
					{
						if(LOCAL_27 < 10)
							item->stEffect[0].cValue = 4;
						else if(LOCAL_27 < 40)
							item->stEffect[0].cValue = 3;
					}
					else if(LOCAL_3 == 5)
					{
						if(LOCAL_27 < 10)
							item->stEffect[0].cValue = 5;
						else if(LOCAL_27 < 30)
							item->stEffect[0].cValue = 4;
						else if(LOCAL_27 < 60)
							item->stEffect[0].cValue = 3;
					}
					else if(LOCAL_3 == 6)
					{
						if(LOCAL_27 < 10)
							item->stEffect[0].cValue  = 6;
						else if(LOCAL_27 < 20)
							item->stEffect[0].cValue = 5;
						else if(LOCAL_27 < 30)
							item->stEffect[0].cValue = 4;
						else if(LOCAL_27 < 60)
							item->stEffect[0].cValue = 3;
					}
					else if(LOCAL_3 == 7)
					{
						if(LOCAL_27 < 4)
							item->stEffect[0].cValue = 7;
						else if(LOCAL_27 < 10)
							item->stEffect[0].cValue = 6;
						else if(LOCAL_27 < 20)
							item->stEffect[0].cValue = 5;
						else if(LOCAL_27 < 35)
							item->stEffect[0].cValue = 4;
						else if(LOCAL_27 < 60)
							item->stEffect[0].cValue = 3;
					}
				}
			}
			else if(LOCAL_22 < LOCAL_24)
			{
				item->stEffect[0].cEffect = EF_SANC;
				item->stEffect[0].cValue = 1;
			}
			else if(LOCAL_22 < LOCAL_25)
			{
				item->stEffect[0].cEffect = EF_SANC;
				item->stEffect[0].cValue = 0;
			}
			else if(LOCAL_22 < LOCAL_26)
			{ // Aqui ele gera adicionais tipo: Inteligancia, foraa e tal ^^ 
				int LOCAL_28 = Rand() % 0x0A;
				int LOCAL_29 = dwValue[LOCAL_28];
				int LOCAL_30 = LOCAL_8;
				if(LOCAL_30 >= 2)
					LOCAL_30 = 2;
				int LOCAL_31 = dwValue_02[LOCAL_28][LOCAL_30][0];
				int LOCAL_32 = dwValue_02[LOCAL_28][LOCAL_30][1];

				int LOCAL_33 = LOCAL_32 + 1 - LOCAL_31;

				if(LOCAL_33 == 0)
					LOCAL_33 = 1;

				int LOCAL_34 = Rand() % LOCAL_33 + LOCAL_31;

				item->stEffect[0].cEffect = LOCAL_29;
				item->stEffect[0].cValue = LOCAL_34;
			}
			else
			{
				item->stEffect[0].cEffect = 0x3B;
				item->stEffect[0].cValue = Rand() % 0x7F;
			}
		}
	}

	int LOCAL_35 = 0;
	for(LOCAL_35 = 0; LOCAL_35 < 12; LOCAL_35++)
	{
		int LOCAL_36 = g_pItemList[LOCAL_4].Level;

		switch(g_pItemList[LOCAL_4].stEffect[LOCAL_35].sEffect)
		{
			case EF_SANC:
				item->stEffect[0].cEffect = EF_SANC;
				item->stEffect[0].cValue = static_cast<BYTE>(g_pItemList[LOCAL_4].stEffect[LOCAL_35].sValue);
				break;
			case EF_AMOUNT:
				item->stEffect[0].cEffect = EF_AMOUNT;
				item->stEffect[0].cValue = static_cast<BYTE>(g_pItemList[LOCAL_4].stEffect[LOCAL_35].sValue);
				break;
			case EF_INCUBATE:
				item->stEffect[0].cEffect = EF_INCUBATE;

				BYTE LOCAL_37 = Rand() % 4 + g_pItemList[LOCAL_4].stEffect[LOCAL_35].sValue;
				if(LOCAL_37 > 9)
					LOCAL_37 = 9;

				item->stEffect[0].cValue = static_cast<BYTE>(LOCAL_37);
				break;
		}
	}

	// item->sIndex == 412 || item->sIndex == 413 || 
	if(item->sIndex == 419 || item->sIndex == 420 || item->sIndex == 753)
	{
		if(!item->stEffect[0].cEffect)
		{
			item->stEffect[0].cEffect = EF_UNIQUE;
			item->stEffect[0].cValue = Rand() % 0x7F;
		}
		if(!item->stEffect[1].cEffect)
		{
			item->stEffect[1].cEffect = EF_UNIQUE;
			item->stEffect[1].cValue = Rand() % 0x7F;
		}
		if(!item->stEffect[2].cEffect)
		{
			item->stEffect[2].cEffect = EF_UNIQUE;
			item->stEffect[2].cValue = Rand() % 0x7F;
		}
	}

	if((item->sIndex >= 447 && item->sIndex <= 450) || (item->sIndex >= 692 && item->sIndex <= 695))
	{
		if(!item->stEffect[0].cEffect)
		{
			item->stEffect[0].cEffect = EF_UNIQUE;
			item->stEffect[0].cValue = Rand() % 256;
		}
		if(!item->stEffect[1].cEffect)
		{
			item->stEffect[1].cEffect = EF_UNIQUE;
			item->stEffect[1].cValue = Rand() % 256;
		}
		if(!item->stEffect[2].cEffect)
		{
			item->stEffect[2].cEffect = EF_UNIQUE;
			item->stEffect[2].cValue = Rand() % 256;
		}
	}
}

bool SetItemSanc(STRUCT_ITEM *item, int sanc, int arg3)
{
	if(sanc > 15 || sanc < 0)
		return false;

	if(arg3 > 20)
		arg3 = 20;
	if(arg3 < 0)
		arg3 = 0;

	int i = 0;
	int actualSanc = 0;
	for (; i < 3; i++)
	{
		if (item->stEffect[i].cEffect == 43 || (item->stEffect[i].cEffect >= 116 && item->stEffect[i].cEffect <= 125))
		{
			actualSanc = item->stEffect[i].cValue;
			break;
		}
	}

	if(sanc > 9)
	{
		// se o item ja possuir uam refinaaao maior que +11, respeitamos o adicional  de gema que
		// ele ja possui
		int value = 0;
		if (actualSanc > 230)
			value = (actualSanc - 230) % 4;

		sanc = 230 + ((sanc - 10) * 4) + value;
		if(sanc > 250)
			sanc = 250;
	}
    
	int	sc = sanc + (10 * arg3);

	if(i == 3)
	{
		for(i = 0;i<3;i++)
		{
			if(item->stEffect[i].cEffect == 0)
			{
				item->stEffect[i].cEffect = EF_SANC;
				item->stEffect[i].cValue = sc;
				return true;
			}
		}
		// Impossavel de refinar
		if( i == 3 )
			return false;
	}
	else
	{
		item->stEffect[i].cValue = sc;
		return true;
	}

	return false;
}

void SetPKPoint(int clientId,int points)
{
	if(clientId <= 0 || clientId >= MAX_PLAYER) 
		return;

	if(points < 1) 
		points = 1;

	if(points > 150) 
		points = 150;

	unsigned char cv = points;
	pMob[clientId].Mobs.Player.Inventory[63].EF1 = cv;
}

void SetReqHp(int clientId)
{//0x00401492
    if(clientId <= 0 || clientId >= MAX_PLAYER)
		return;
 
    if(pMob[clientId].Mobs.Player.CurrentScore.Hp > pMob[clientId].Mobs.Player.CurrentScore.MaxHp)
		pMob[clientId].Mobs.Player.CurrentScore.Hp = pMob[clientId].Mobs.Player.CurrentScore.MaxHp;
 
    if(pUser[clientId].Potion.CountHp < pMob[clientId].Mobs.Player.CurrentScore.Hp)
		pUser[clientId].Potion.CountHp = pMob[clientId].Mobs.Player.CurrentScore.Hp;
}

void SetReqMp(int clientId)
{
    if(clientId <= 0 || clientId >= MAX_PLAYER)
		return;
	 
    if(pMob[clientId].Mobs.Player.CurrentScore.Mp > pMob[clientId].Mobs.Player.CurrentScore.MaxMp)
		pMob[clientId].Mobs.Player.CurrentScore.Mp = pMob[clientId].Mobs.Player.CurrentScore.MaxMp;
 
    if(pUser[clientId].Potion.CountMp < pMob[clientId].Mobs.Player.CurrentScore.Mp)
		pUser[clientId].Potion.CountMp = pMob[clientId].Mobs.Player.CurrentScore.Mp;
}

 int MountCon[30] = 
 { 
	 0,	 800,	 900,	 -800,	 0,	 0,	 0,	 0,	 0,	 0,	 0,	 0,
	 0,	 0,	 0,	 0,	 0,	 600,	 0,	 0,	 0,	 0,	 0,	 0,
	 0,	 0,	 0,	 0,	 0
 };
 
int GenerateSummon(int User,int SummonId, STRUCT_ITEM *Item)
{
	int Leader = pMob[User].Leader;
	if(Leader <= 0)
		Leader = User;

	int mobId = mGener.GetEmptyNPCMob();
	if(mobId == 0)
	{
		SendClientMessage(User,"No EmpytMobNPC");
		return 0;
	}

	if(SummonId < 0 || SummonId >= 50)
		return 0;
	
	int summonFaceId = NPCBase[SummonId].Equip[0].sIndex;
	int LOCAL4 = 0;
	int ptCont = 0;

	int idxPt;
	for(idxPt = 0;idxPt < 12; idxPt++)
	{
		int idx = pMob[Leader].PartyList[idxPt];

		if(idx == 0)
			break;
	}

	if(idxPt >= 12)
	{
		SendClientMessage(User,"Nao a possivel envocar com grupo cheio");
		return 0;
	}
	
	memset(&pMob[mobId].PartyList, 0, 24);
	memcpy(&pMob[mobId].Mobs.Player, &NPCBase[SummonId], sizeof STRUCT_MOB);
	pMob[mobId].Mobs.Player.BaseScore.Level  = pMob[User].Mobs.Player.BaseScore.Level;
	pMob[mobId].Mobs.Player.CurrentScore.Level  = pMob[User].Mobs.Player.BaseScore.Level;
	strcat_s(pMob[mobId].Mobs.Player.MobName,"^");

	for(int i=0; i<16; i++)
		if(pMob[mobId].Mobs.Player.MobName[i] == 95)
			pMob[mobId].Mobs.Player.MobName[i] = 32;

	memset(&pMob[mobId].Mobs.Affects, 0, sizeof(STRUCT_AFFECT) * 32);
	
	int sINT = pMob[User].Mobs.Player.CurrentScore.Int + (pMob[User].Mobs.Player.CurrentScore.Int * 15 / 100);
	int sMast = pMob[User].Mobs.Player.CurrentScore.Special[2] + 50;

	if (Item == nullptr && pMob[User].Mobs.Player.Class == 2)
	{
		if (pMob[User].Mobs.Player.LearnedSkill[0] & (1 << 15)) // se tiver oitava skill succubus
		{
			sINT += ceil((static_cast<double>(sINT) * 0.105));
			sMast += 50;
		}
	}

	int calcInt = pSummonBonus[SummonId].MinAttack * sINT / 100;
	int calcMast = pSummonBonus[SummonId].MaxAttack * sMast / 100;

	pMob[mobId].Mobs.Player.BaseScore.Damage = pMob[mobId].Mobs.Player.BaseScore.Damage + calcInt + calcMast;

	calcInt = pSummonBonus[SummonId].MinDefense * sINT / 100;
	calcMast = pSummonBonus[SummonId].MaxDefense * sMast / 100;

	pMob[mobId].Mobs.Player.BaseScore.Ac = pMob[mobId].Mobs.Player.BaseScore.Ac + calcInt + calcMast;

	calcInt = pSummonBonus[SummonId].MinHp * sINT / 100;
	calcMast = pSummonBonus[SummonId].MaxHp * sMast / 100;

	pMob[mobId].Mobs.Player.CurrentScore.MaxHp = pMob[mobId].Mobs.Player.BaseScore.MaxHp + calcInt + calcMast;
	
	pMob[mobId].Mobs.Player.BaseScore.MaxHp = pMob[mobId].Mobs.Player.CurrentScore.MaxHp;
	pMob[mobId].GenerateID = -1;
	pMob[mobId].Formation = 5;
	pMob[mobId].RouteType = 5;
	pMob[mobId].Mode = 4;
	pMob[mobId].Segment.Progress = 0;
	pMob[mobId].Segment.Direction = 0;
	pMob[mobId].Summoner = User;
	pMob[mobId].clientId = mobId;

	memset(pMob[mobId].Segment.ListX, 0 , 5 * sizeof(int));
	memset(pMob[mobId].Segment.ListY, 0 , 5 * sizeof(int));

	pMob[mobId].Leader = Leader;
	pMob[mobId].Last.Time = CurrentTime;

	if(Item != 0)
	{
		int ret = GetItemAbility(Item, EF_MOUNTSANC); //LOCAL_30

		if(ret >= 100)
			ret = 100; 

		int faceId = pMob[mobId].Mobs.Player.Equip[0].sIndex;		// LOCAL_31
		INT16 Con = 0; // LOCAL_32

		if(faceId >= 315 && faceId < 345)
		{
			int result = faceId - 315;
			int valCon = MountCon[result]; // LOCAL_33
			int calc1 = (valCon / 2) - 1000; // LOCAL_34
			int calc2 = valCon - calc1; // LOCAL_35
	
			Con = (calc1 + ((calc2 * ret) / 100)); // LOCAL_32

			pMob[mobId].Mobs.Player.BaseScore.Con = Con;
			pMob[mobId].Mobs.Player.CurrentScore.Con = Con;

			int retn = GetItemAbility(Item,EF_MOUNTSANC);

			pMob[mobId].Mobs.Player.BaseScore.Damage = pMob[mobId].Mobs.Player.BaseScore.Damage + ((retn & 255) * 6);
			pMob[mobId].Mobs.Player.CurrentScore.Damage = pMob[mobId].Mobs.Player.BaseScore.Damage;
		}
	}

	if(pMob[User].GuildDisable == 0)
	{
		pMob[mobId].Mobs.Player.Guild		= pMob[User].Mobs.Player.Guild;
		pMob[mobId].Mobs.Player.GuildLevel  = 0;
	}

	pMob[mobId].Mobs.Player.CurrentScore.Hp = pMob[mobId].Mobs.Player.CurrentScore.MaxHp;

	if(true) //?
	{
		if(pMob[User].Mobs.Player.Equip[15].sIndex == 543 || pMob[User].Mobs.Player.Equip[15].sIndex == 545)
		{
			memset(&pMob[mobId].Mobs.Player.Equip[15], 0, sizeof(STRUCT_ITEM));
			pMob[mobId].Mobs.Player.Equip[15].sIndex = 734;
		}

		if(pMob[User].Mobs.Player.Equip[15].sIndex == 544 || pMob[User].Mobs.Player.Equip[15].sIndex == 546)
		{
			memset(&pMob[mobId].Mobs.Player.Equip[15], 0, sizeof(STRUCT_ITEM));
			pMob[mobId].Mobs.Player.Equip[15].sIndex = 735;
		}

		if(pMob[User].Mobs.Player.Equip[15].sIndex == 548 || pMob[User].Mobs.Player.Equip[15].sIndex == 549)
		{
			memset(&pMob[mobId].Mobs.Player.Equip[15], 0, sizeof(STRUCT_ITEM));
			pMob[mobId].Mobs.Player.Equip[15].sIndex = 550;
		}
	}
	
	pMob[mobId].Mobs.Player.CapeInfo = 4;
	
	pMob[mobId].GetCurrentScore(MAX_PLAYER);
	
	memset(&pMob[mobId].EnemyList, 0, sizeof(short) * 4);

	unsigned int X = pMob[User].Target.X;
	unsigned int Y = pMob[User].Target.Y;

	int retMobGrid = GetEmptyMobGrid(mobId,&X,&Y);
	if(retMobGrid == 0)
	{
		pMob[mobId].Mode = 0;
		return 0;
	}

	pMob[mobId].Target.X = X;
	pMob[mobId].Last.X = X;

	pMob[mobId].Target.Y = Y;
	pMob[mobId].Last.Y = Y;

	if(Item == NULL)
	{
		pMob[mobId].Mobs.Affects[0].Index = 24;
		pMob[mobId].Mobs.Affects[0].Master = 0;
		pMob[mobId].Mobs.Affects[0].Value = 0;
		pMob[mobId].Mobs.Affects[0].Time = 30;

		if(SummonId >= 28 && SummonId <= 37)
			pMob[mobId].Mobs.Affects[0].Time = 128;
	}
	
	if(Item != NULL)
	{
		if(*(short*)&Item->EF1 > pMob[mobId].Mobs.Player.CurrentScore.MaxHp)
			*(short*)&Item->EF1 = pMob[mobId].Mobs.Player.CurrentScore.MaxHp;
	
		pMob[mobId].Mobs.Player.BaseScore.Hp = *(short*)&Item->EF1;
		pMob[mobId].Mobs.Player.CurrentScore.Hp = pMob[mobId].Mobs.Player.BaseScore.Hp;
	}

	g_pMobGrid[Y][X] = mobId;

	pMob[mobId].SpawnType = 2;
	SendGridMob(mobId);
	pMob[mobId].SpawnType = 1;

	int MobId = mobId;

	pMob[Leader].PartyList[idxPt] = MobId;
	pMob[MobId].Leader = Leader;

	if(ptCont == 0)
		SendAddParty(Leader,Leader, 1);

	SendAddParty(MobId,Leader, 1);

	if(ptCont == 0)
		SendAddParty(MobId,MobId, 0);

	SendAddParty(Leader,MobId, 0);

	for(int i=0; i<12; i++)
	{
		int partyID = pMob[Leader].PartyList[i];

		if(partyID == 0)
			continue;

		if(partyID != MobId)
			SendAddParty(partyID,MobId,0);

		SendAddParty(MobId,partyID,0);
	}

	return 1;
}

// Boa parte feita, mas tem que revisar bem, principalmente a parte das fadas e etc.

int ProcessAffect(int clientId)
{
	INT32 LOCAL_1 = 0;
	INT32 LOCAL_2 = 0;
	INT32 LOCAL_3 = 0;
	INT32 LOCAL_4 = 0;
	INT32 LOCAL_6 = 0;
	INT32 LOCAL_5 = pMob[clientId].Mobs.Player.Equip[13].sIndex;
	if(clientId < MAX_PLAYER && (LOCAL_5 == 754 || LOCAL_5 == 769 || LOCAL_5 == 1726))
	{
		INT32 LOCAL_6 = pMob[clientId].CurrentTarget;

		if(LOCAL_6)
		{
			if(LOCAL_6 != clientId)
			{
				if(LOCAL_6 <= 0 || LOCAL_6 > 30000)
					pMob[clientId].CurrentTarget = 0;
				else
				{
					if(pMob[clientId].Mode == 0)
						pMob[clientId].CurrentTarget = 0;
					else
					{
						eMapAttribute LOCAL_7 = GetAttribute(pMob[clientId].Target.X, pMob[clientId].Target.Y); //GET ATTRIBUTE
						eMapAttribute targetAttribute = GetAttribute(pMob[LOCAL_6].Target.X, pMob[LOCAL_6].Target.Y); //GET ATTRIBUTE
						if(LOCAL_7.PvP)
						{
							if((LOCAL_6 < MAX_PLAYER || pMob[LOCAL_6].Mobs.Player.CapeInfo == 4) && !pUser[clientId].AllStatus.PK)
								pMob[clientId].CurrentTarget = 0;
						}

						if (LOCAL_6 < MAX_PLAYER && !targetAttribute.PvP)
							pMob[clientId].CurrentTarget = 0;
					}
				}

				if (pMob[clientId].CurrentTarget != 0)
				{
					INT32 LOCAL_8 = pMob[clientId].Leader;
					if (LOCAL_8 == 0)
						LOCAL_8 = clientId;

					INT32 LOCAL_9 = pMob[clientId].Leader;
					if (LOCAL_9 == 0)
						LOCAL_9 = LOCAL_6;

					INT32 LOCAL_10 = pMob[clientId].Mobs.Player.Guild;
					if (pMob[clientId].GuildDisable != 0)
						LOCAL_10 = 0;

					INT32 LOCAL_11 = pMob[LOCAL_6].Mobs.Player.Guild;
					if (pMob[LOCAL_6].GuildDisable != 0)
						LOCAL_11 = 0;

					if (LOCAL_10 == 0 && LOCAL_11 == 0)
						LOCAL_10 = -1;

					if (LOCAL_8 == LOCAL_9 || LOCAL_10 == LOCAL_11)
						pMob[clientId].CurrentTarget = 0;
					//4503AC
					INT32 LOCAL_12 = 0;
					INT32 LOCAL_13 = pMob[clientId].Mobs.Player.CapeInfo;
					INT32 LOCAL_14 = pMob[LOCAL_6].Mobs.Player.CapeInfo;

					if ((LOCAL_13 == 7 && LOCAL_14 == 7) || (LOCAL_13 == 8 && LOCAL_14 == 8))
						LOCAL_12 = 1;


					if ((LOCAL_12 != 1 || pUser[clientId].AllStatus.PK != 0) || (LOCAL_12 != 1 || LOCAL_6 < MAX_PLAYER))
					{
						UINT32 LOCAL_15 = pMob[clientId].Target.X;
						UINT32 LOCAL_16 = pMob[clientId].Target.Y;

						if (pMob[LOCAL_6].Target.X >= (LOCAL_15 - VIEWGRIDX) && pMob[LOCAL_6].Target.X <= (LOCAL_15 + VIEWGRIDX) &&
							pMob[LOCAL_6].Target.Y >= (LOCAL_16 - VIEWGRIDY) && pMob[LOCAL_6].Target.Y <= (LOCAL_16 + VIEWGRIDY))
						{
							MSG_AttackOne packet;
							memset(&packet, 0, sizeof packet);

							packet.Header.PacketId = 0x39D;
							packet.Header.ClientId = clientId;
							packet.Header.Size = sizeof packet;
							packet.Header.TimeStamp = 0x0E0A1ACA;

							packet.PosX = pMob[clientId].Target.X;
							packet.PosY = pMob[clientId].Target.Y;

							packet.AttackerID = clientId;
							packet.Motion = 0xFE;

							packet.SkillIndex = 32;
							if (LOCAL_5 == 769)
								packet.SkillIndex = 34;
							else if (LOCAL_5 == 1726)
								packet.SkillIndex = 36;

							packet.Target[0].TargetID = LOCAL_6;
							packet.Target[0].Damage = -1;

							// 004505C9

							pUser[clientId].TimeStamp.TimeStamp = 0x0E0A1ACA;
							pUser[clientId].PacketControl((BYTE*)& packet, sizeof MSG_AttackOne);
							pUser[clientId].TimeStamp.TimeStamp = 0;
						}
						else
							pMob[clientId].CurrentTarget = 0;
					}
				}
			}
			else
				pMob[clientId].CurrentTarget = 0;
		}
	}

	for(INT32 LOCAL_28 = 0; LOCAL_28 < 32; LOCAL_28 ++)
	{
		INT32 LOCAL_29 = pMob[clientId].Mobs.Affects[LOCAL_28].Index;
		if(LOCAL_29 <= 0)
			continue;

		INT32 LOCAL_30 = pMob[clientId].Mobs.Player.CurrentScore.MaxHp;
		INT32 LOCAL_31 = pMob[clientId].Mobs.Player.CurrentScore.Hp;
		INT32 LOCAL_32 = pMob[clientId].Mobs.Affects[LOCAL_28].Master;
		INT32 LOCAL_33 = pMob[clientId].Mobs.Affects[LOCAL_28].Value;
		INT32 LOCAL_34 = pMob[clientId].Mobs.Player.CurrentScore.Level;

		if(pMob[clientId].Mobs.Player.Equip[0].EFV2 >= CELESTIAL)
			LOCAL_34 += 400;

		if(LOCAL_29 == 17) // AURA DA VIDA
		{
			INT32 LOCAL_35 = (LOCAL_33 << 1) + LOCAL_32 + 50;
			LOCAL_31 = LOCAL_31 + LOCAL_35;

			if(LOCAL_31 < 1)
				LOCAL_31 = 1;

			if(LOCAL_31 > LOCAL_30)
				LOCAL_31 = LOCAL_30;

			if(pMob[clientId].Mobs.Player.CurrentScore.Hp != LOCAL_31)
				LOCAL_1 = 1;

			LOCAL_4 = LOCAL_31 - pMob[clientId].Mobs.Player.CurrentScore.Hp;

			pMob[clientId].Mobs.Player.CurrentScore.Hp = LOCAL_31;

			LOCAL_3 = 1;
		}

		else if(LOCAL_29 == 20) // VENENO 
		{
			if (clientId < MAX_PLAYER)
			{
				LOCAL_32 = 100;
				if (clientId >= MAX_PLAYER)
					LOCAL_32 = LOCAL_32 - pMob[clientId].Mobs.Player.LearnedSkill[0];

				LOCAL_32 = LOCAL_32 / 10;

				float LOCAL_164 = (100.0f - LOCAL_32) / 100.0f;
				INT32 LOCAL_37 = 1000;

				if (clientId >= MAX_PLAYER)
					LOCAL_37 = (100 - pMob[clientId].Mobs.Player.LearnedSkill[0]) * 10;

				INT32 LOCAL_38 = (LOCAL_31 - LOCAL_37);
				LOCAL_31 = static_cast<float>(LOCAL_31) * LOCAL_164;

				if (LOCAL_31 < LOCAL_38)
					LOCAL_31 = LOCAL_38;

				if (LOCAL_31 < 1)
					LOCAL_31 = 1;

				if (LOCAL_31 > LOCAL_30)
					LOCAL_31 = LOCAL_30;

				if (pMob[clientId].Mobs.Player.CurrentScore.Hp != LOCAL_31)
					LOCAL_1 = 1;

				LOCAL_4 = LOCAL_31 - pMob[clientId].Mobs.Player.CurrentScore.Hp;
				if (clientId >= MAX_PLAYER)
				{
					INT32 itemId = pMob[clientId].Mobs.Player.Equip[13].sIndex;
					if (itemId == 786 || itemId == 1936 || itemId == 1937)
					{
						INT32 _sanc = GetItemSanc(&pMob[clientId].Mobs.Player.Equip[13]); // local209
						if (_sanc < 2)
							_sanc = 2;

						INT32 multHP = 1;
						switch (itemId)
						{
						case 1936:
							multHP = 10;
							break;

						case 1937:
							multHP = 1000;
							break;
						}

						multHP *= _sanc;
						INT32 damage = -LOCAL_4 / multHP;

						LOCAL_31 = pMob[clientId].Mobs.Player.CurrentScore.Hp - damage;
					}
				}

				pMob[clientId].Mobs.Player.CurrentScore.Hp = LOCAL_31;

				// Aqui ele adiciona a parada na struct
				if (clientId > 0 && clientId < MAX_PLAYER)
					pUser[clientId].Potion.CountHp += LOCAL_4;

				LOCAL_3 = 1;

				if (clientId >= MAX_PLAYER && pMob[clientId].Mobs.Player.CapeInfo == 4)
					LinkMountHp(clientId);
			}
		}
		/*
		else if(LOCAL_29 == 21)
		{
			INT32 LOCAL_39 = LOCAL_30;
				
			LOCAL_39 = LOCAL_39 * LOCAL_32 / 100;

			LOCAL_31 = LOCAL_31 + LOCAL_39;

			if(LOCAL_31 < 1)
				LOCAL_31 = 1;

			if(LOCAL_31 > LOCAL_30)
				LOCAL_31 = LOCAL_30;
				
			if(pMob[clientId].Mobs.Player.CurrentScore.Hp != LOCAL_31)
				LOCAL_1 = 1;

			LOCAL_4 = pMob[clientId].Mobs.Player.CurrentScore.Hp - LOCAL_31;
				
			pMob[clientId].Mobs.Player.CurrentScore.Hp = LOCAL_31;
		}*/
		else if(LOCAL_29 == 23) // AURA BESTIAL
		{ // 00451A99
			if(clientId >= MAX_PLAYER)
				goto check;

			eMapAttribute LOCAL_108 = GetAttribute(pMob[clientId].Target.X, pMob[clientId].Target.Y);
			INT32 LOCAL_109 = pMob[clientId].Mobs.Player.CapeInfo;
			if(LOCAL_108.Village)
				goto check;

			MSG_AttackOne LOCAL_132;
			memset(&LOCAL_132, 0, sizeof MSG_AttackOne);

			INT32 LOCAL_133 = pMob[clientId].CurrentTarget;
			if(LOCAL_133 != 0)
			{
				INT32 LOCAL_134 = 0;
				INT32 LOCAL_135 = pMob[clientId].Target.X;
				INT32 LOCAL_136 = pMob[LOCAL_133].Target.X;

				if(LOCAL_133 == clientId)
					pMob[clientId].CurrentTarget = 0;
				else
				{
					if(LOCAL_133 <= 0 || LOCAL_133 >= 30000)
						pMob[clientId].CurrentTarget = 0;
					else
					{
						if(pMob[LOCAL_133].Mode == 0)
							pMob[clientId].CurrentTarget = 0;
						else
						{
							eMapAttribute LOCAL_137 = GetAttribute(pMob[clientId].Target.X, pMob[clientId].Target.Y);
							INT32 LOCAL_138 = pMob[clientId].Leader;
							if(LOCAL_138 == 0)
								LOCAL_138 = clientId;

							INT32 LOCAL_139 = pMob[LOCAL_133].Leader;
							if(LOCAL_139 == 0)
								LOCAL_139 = LOCAL_133;

							INT32 LOCAL_140 = pMob[clientId].Mobs.Player.Guild;
							if(pMob[clientId].GuildDisable != 0)
								LOCAL_140 = 0;

							INT32 LOCAL_141 = pMob[LOCAL_133].Mobs.Player.Guild;
							if(pMob[LOCAL_133].GuildDisable != 0)
								LOCAL_141 = 0;

							INT32 LOCAL_142 = g_pGuildAlly[LOCAL_140];
							if(LOCAL_142 == 0)
								LOCAL_142 = -2;

							if(LOCAL_140 == 0 && LOCAL_141 == 0)
								LOCAL_140 = -1;

							// 00451D27
							if(LOCAL_138 == LOCAL_139)
								goto label;

							if(LOCAL_140 == LOCAL_141)
								goto label;

							if(LOCAL_142 == LOCAL_141)
								goto label;

							// a um mob.
							if (LOCAL_133 >= MAX_PLAYER)
							{
								// PK desativado e a uma evocaaao.
								if (!pUser[clientId].AllStatus.PK && pMob[LOCAL_133].Summoner)
									goto label;
							}
							if(LOCAL_133 < MAX_PLAYER)
							{
								if(!pUser[clientId].AllStatus.PK)
								{
									// essa verificaaao sempre vai entrar lol
									if(LOCAL_133 < MAX_PLAYER || LOCAL_136 == 4 || LOCAL_134 == 1)
										goto label;
								}

								if(!LOCAL_137.PvP)
								{
									if(LOCAL_133 < MAX_PLAYER || LOCAL_136 ==4 || LOCAL_134 == 1)
										goto label;
								}

									
								if(pMob[LOCAL_133].Mobs.Player.AffectInfo.Snoop || pMob[LOCAL_133].Mobs.Player.Info.Merchant & 1)
								{
									pMob[clientId].CurrentTarget = 0;
									goto label;
								}

								if(pMob[LOCAL_133].Mobs.Player.AffectInfo.SlowMov)
									continue;
							}

							if((LOCAL_135 == 7 && LOCAL_136 == 7) || (LOCAL_135 == 8 && LOCAL_136 == 8))
								LOCAL_134 = 1;

							if(clientId < MAX_PLAYER)
							{
								if(LOCAL_134 == 1 && !pUser[clientId].AllStatus.PK) 
									goto label;

								if(!pUser[clientId].AllStatus.PK && LOCAL_133 < MAX_PLAYER)
								{
									pMob[clientId].CurrentTarget = 0;

									goto label;
								}
							}

							if(LOCAL_134 == 1 && LOCAL_133 >= MAX_PLAYER)
								goto label;

							UINT32 LOCAL_143 = pMob[clientId].Target.X;
							UINT32 LOCAL_144 = pMob[clientId].Target.Y;

							if (pMob[LOCAL_133].Target.X < LOCAL_143 - VIEWGRIDX || pMob[LOCAL_133].Target.X > LOCAL_143 + VIEWGRIDX || pMob[LOCAL_133].Target.Y < LOCAL_144 - VIEWGRIDY || pMob[LOCAL_133].Target.Y > LOCAL_144 + VIEWGRIDY)
							{
								pMob[clientId].CurrentTarget = 0;

								goto label;
							}

							LOCAL_132.Target[0].TargetID = LOCAL_133;
							LOCAL_132.Target[0].Damage = -1;
						}
					}
				}
			}
	label:
			INT32 LOCAL_145 = 0;
			INT32 LOCAL_146 = pMob[clientId].Target.Y - 1;
			INT32 LOCAL_147 = pMob[clientId].Target.X - 1;

			for(INT32 LOCAL_148 = LOCAL_146; LOCAL_148 <= LOCAL_146 + 2; LOCAL_148++)
			{
				if(LOCAL_148 < 0 || LOCAL_148 >= 4096)
					continue;

				for(INT32 LOCAL_149 = LOCAL_147; LOCAL_149 <= LOCAL_147 + 2; LOCAL_149++)
				{
					INT32 LOCAL_150 = g_pMobGrid[LOCAL_148][LOCAL_149];
					if(LOCAL_150 <= 0 || LOCAL_150 > 30000)
						continue;

					if(pMob[LOCAL_150].Mode == 0)
						continue;

					INT32 LOCAL_151 = pMob[LOCAL_150].Mobs.Player.CapeInfo;
					INT32 LOCAL_152 = 0;

					// 004520BE

					if((LOCAL_109 == 7 && LOCAL_151 == 7) || (LOCAL_109 == 8 && LOCAL_151 == 8))
						LOCAL_152 = 1;

					if(LOCAL_152 == 1 && LOCAL_150 >= MAX_PLAYER)
						continue;

					if(LOCAL_151 == 6)
						continue;

					if(LOCAL_150 == clientId)
						continue;

					INT32 LOCAL_153 = pMob[clientId].Leader;
					if(LOCAL_153 == 0)
						LOCAL_153 = clientId;

					INT32 LOCAL_154 = pMob[LOCAL_150].Leader;
					if(LOCAL_154 == 0)
						LOCAL_154 = LOCAL_150;

					INT32 LOCAL_155 = pMob[clientId].Mobs.Player.Guild;
					if(pMob[clientId].GuildDisable != 0)
						LOCAL_155 = 0;

					INT32 LOCAL_156 = pMob[LOCAL_150].Mobs.Player.Guild;
					if(pMob[LOCAL_150].GuildDisable != 0)
						LOCAL_156 = 0;

					INT32 LOCAL_157 =  g_pGuildAlly[LOCAL_155];
					if(LOCAL_157 == 0)
						LOCAL_157 = -2;

					if(LOCAL_155 == 0 && LOCAL_156 == 0)
						LOCAL_155 = -1;

					if(LOCAL_153 == LOCAL_154)
						continue;
						
					if(LOCAL_155 == LOCAL_156)
						continue;

					if(LOCAL_157 == LOCAL_156)
						continue;

					//0045226B
					if(clientId < MAX_PLAYER)
					{
						if(!pUser[clientId].AllStatus.PK)
						{
							// Verifica se o alvo a um player ou se a uma evocaaao.
							// Se tiver com PK ativo, nao vai atacar.
							if(LOCAL_150 < MAX_PLAYER || pMob[LOCAL_150].Summoner)// || LOCAL_152 == 1)
								continue;
						}

						if(!LOCAL_108.PvP)
						{
							if(LOCAL_150 < MAX_PLAYER)// || LOCAL_152 == 1)
								continue;
						}

						if(pMob[LOCAL_150].Mobs.Player.AffectInfo.Snoop || pMob[LOCAL_150].Mobs.Player.Info.Merchant & 1)
							continue;
					}
					else
					{
						if(LOCAL_150 >= MAX_PLAYER && LOCAL_151 != 4)
							continue;
					}

					LOCAL_132.Target[LOCAL_145].TargetID = LOCAL_150;
					LOCAL_132.Target[LOCAL_145].Damage = -1;

					LOCAL_145++;
				}
			}

			if(!LOCAL_132.Target[0].TargetID)
				continue;

			LOCAL_132.Header.PacketId = 0x367;
			LOCAL_132.Header.ClientId = clientId;
			LOCAL_132.Header.Size = sizeof MSG_AttackOne;
			LOCAL_132.Header.TimeStamp = 0xE0A1ACA;
				
			LOCAL_132.PosX = pMob[clientId].Target.X;
			LOCAL_132.PosY = pMob[clientId].Target.Y;

			LOCAL_132.Motion = -2;
			LOCAL_132.AttackerID = clientId;

			LOCAL_132.SkillIndex = 52;

			pUser[clientId].TimeStamp.TimeStamp = 0xE0A1ACA;
			pUser[clientId].PacketControl((BYTE*)&LOCAL_132, sizeof MSG_AttackOne);
			pUser[clientId].TimeStamp.TimeStamp = 0;

		}
		else if(LOCAL_29 == 22)
		{ // 00450943
			INT32 LOCAL_40 = 0;
			INT32 LOCAL_41 = 0;
			INT32 LOCAL_42 = 0;
			INT32 LOCAL_43 = 0;
			INT32 LOCAL_44 = 0;
			INT32 LOCAL_45 = 0;
			eMapAttribute LOCAL_46 = GetAttribute(pMob[clientId].Target.X, pMob[clientId].Target.Y);
			INT32 LOCAL_47 = pMob[clientId].Mobs.Player.CapeInfo;

			if(LOCAL_46.Village)
				goto check;

			//004509E4
			INT32 LOCAL_48 = pMob[clientId].Target.Y - 1;
			INT32 LOCAL_49 = pMob[clientId].Target.X - 1;
			INT32 LOCAL_50 = pMob[clientId].Leader;

			if(LOCAL_50 <= 0)
				LOCAL_50 = clientId;

			for(INT32 LOCAL_51 = LOCAL_48; LOCAL_51 <= (LOCAL_48 + 2); LOCAL_51++)
			{
				if(LOCAL_51 < 0 || LOCAL_51 >= 4096)
					continue;

				for(INT32 LOCAL_52 = LOCAL_49; LOCAL_52 <= (LOCAL_49 + 2); LOCAL_52++)
				{
					if(LOCAL_52 < 0 || LOCAL_52 >= 4096)
						continue;

					INT32 LOCAL_53 = g_pMobGrid[LOCAL_51][LOCAL_52];
					if(LOCAL_53 <= 0 || LOCAL_53 >= 30000)
						continue;

					if(pMob[LOCAL_53].Mode == 0)
						continue;

					if(pMob[LOCAL_53].Mobs.Player.CurrentScore.Hp <= 0)
						continue;

					if(LOCAL_50 == pMob[LOCAL_53].Leader)
						continue;

					INT32 LOCAL_54 = pMob[LOCAL_53].Mobs.Player.CapeInfo;
					INT32 LOCAL_55 = 0;

					if((LOCAL_47 == 7 && LOCAL_54 == 7) || (LOCAL_47 == 8 && LOCAL_54 == 8))
						LOCAL_55 = 1;

					//if(pMob[LOCAL_53].Mobs.Player.AffectInfo.SlowMov)
					//	continue;
					//00450C12  |. 85C0           ||TEST EAX,EAX
					if(clientId < MAX_PLAYER)
					{
						if(!pUser[clientId].AllStatus.PK)
						{
							if(LOCAL_53 < MAX_PLAYER || LOCAL_54 == 4) //||// LOCAL_55 == 1)
								continue;
						}

						if(!LOCAL_46.PvP && (LOCAL_53 < MAX_PLAYER || LOCAL_54 == 4))// || LOCAL_55 == 1))
							continue;

						if(pMob[LOCAL_53].Mobs.Player.AffectInfo.Snoop || pMob[LOCAL_53].Mobs.Player.Info.Merchant & 1)
							continue;
					}
					else
					{
						if(LOCAL_53 >= MAX_PLAYER && LOCAL_54 == 4)
							continue;
					}

					if(LOCAL_55 == 1 && LOCAL_53 >= MAX_PLAYER)
						continue;

					if(LOCAL_54 == 6)
						continue;

					if(LOCAL_53 == clientId)
						continue;

					INT32 LOCAL_56 = pMob[clientId].Leader;
					if(LOCAL_56 == 0)
						LOCAL_56 = clientId;

					// 00450CCB
					INT32 LOCAL_57 = pMob[LOCAL_53].Leader;
					if(LOCAL_57 == 0)
						LOCAL_57 = LOCAL_53;

					INT32 LOCAL_58 = pMob[clientId].Mobs.Player.Guild;
					if(pMob[clientId].GuildDisable != 0)
						LOCAL_58 = 0;

					INT32 LOCAL_59 = pMob[LOCAL_53].Mobs.Player.Guild;
					if(pMob[LOCAL_53].GuildDisable != 0)
						LOCAL_59 = 0;

					INT32 LOCAL_60 = g_pGuildAlly[LOCAL_58];
					if(LOCAL_60 == 0)
						LOCAL_60 = -2;

					if(LOCAL_58 == 0 && LOCAL_59 == 0)
						LOCAL_58 = -1;

					// 00450D6E - GuildAlly nao feito
					if(LOCAL_56 == LOCAL_57)
						continue;

					if(LOCAL_58 == LOCAL_59)
						continue;

					//
					if(LOCAL_60 == LOCAL_59)
						continue;

					if(LOCAL_40 == 0)
					{
						LOCAL_40 = LOCAL_53;
						continue;
					}

					if(LOCAL_40 == LOCAL_53)
						continue;

					if(LOCAL_41 == 0)
					{
						LOCAL_41 = LOCAL_53;
						continue;
					}

					if(LOCAL_41 == LOCAL_53)
						continue;

					// 00450E3E
					if(LOCAL_42 == 0)
					{
						LOCAL_42 = LOCAL_53 ;
						continue;
					}

					if(LOCAL_42 == LOCAL_53)
						continue;

					if(LOCAL_43 == 0)
					{
						LOCAL_43 = LOCAL_53;
						continue;
					}

					if(LOCAL_43 == LOCAL_53)
						continue;

					if(LOCAL_44 == 0)
					{
						LOCAL_44 = LOCAL_53;
						continue;
					}

					if(LOCAL_44 == LOCAL_53)
						continue;

					if(LOCAL_45 == 0)
					{
						LOCAL_45 = LOCAL_53;
						break;
					}
				}

				if(LOCAL_45 != 0)
					break;
			}

			if(LOCAL_42 == 0 || LOCAL_43 == 0 || LOCAL_44 == 0 || LOCAL_45 == 0)
			{
				INT32 LOCAL_61 = pMob[clientId].Target.Y - 2;
				INT32 LOCAL_62 = pMob[clientId].Target.X - 2;

				for(INT32 LOCAL_63 = LOCAL_61; LOCAL_63 <= LOCAL_61 + 4; LOCAL_63++)
				{
					if(LOCAL_63 < 0 || LOCAL_63 >= 4096)
						continue;

					for(INT32 LOCAL_64 = LOCAL_62; LOCAL_64 <= LOCAL_62 + 4; LOCAL_64++)
					{
						if(LOCAL_64 < 0 || LOCAL_64 >= 4096)
							continue;

						INT32 LOCAL_65 = g_pMobGrid[LOCAL_63][LOCAL_64];
						if(LOCAL_65 <= 0 || LOCAL_65 >= MAX_MOB)
							continue;

						if(pMob[LOCAL_65].Mode == 0)
							continue;

						if(pMob[LOCAL_65].Mobs.Player.CurrentScore.Hp <= 0)
							continue;

						// 00451041
						if(LOCAL_50 == pMob[LOCAL_65].Leader)
							continue;

						INT32 LOCAL_66 = pMob[LOCAL_65].Mobs.Player.CapeInfo;
						INT32 LOCAL_67 = 0;
						if((LOCAL_47 == 7 && LOCAL_66 == 7) || (LOCAL_47 == 8 && LOCAL_66 == 8))
							LOCAL_67 = 1;

						if(LOCAL_67 == 1 && LOCAL_65 >= MAX_PLAYER)
							continue;

						//if(pMob[LOCAL_65].Mobs.Player.AffectInfo.SlowMov)
						//	continue;

						if(clientId < MAX_PLAYER)
						{
							if(!pUser[clientId].AllStatus.PK)
							{
								if(LOCAL_65 < MAX_PLAYER || LOCAL_66 == 4 || LOCAL_67 == 1)
									continue;
							}

							if(!LOCAL_46.PvP)
							{
								if(LOCAL_65 < MAX_PLAYER || LOCAL_66 == 4  || LOCAL_67 == 1)
									continue;
							}

							if((pMob[LOCAL_65].Mobs.AffectInfo & 0x40) || pMob[LOCAL_65].Mobs.Player.Info.Merchant & 1)
								continue;
						}
						else
						{
							if(LOCAL_65 >= MAX_PLAYER && LOCAL_66 == 4)
								continue;
						}

						if(LOCAL_65 == clientId)
							continue;

						if(LOCAL_66 == 6)
							continue;

						INT32 LOCAL_68 = pMob[clientId].Leader;
						if(LOCAL_68 == 0)
							LOCAL_68 = clientId;

						// 004511D4
						INT32 LOCAL_69 = pMob[LOCAL_65].Leader;
						if(LOCAL_69 == 0)
							LOCAL_69 = LOCAL_65;

						INT32 LOCAL_70 = pMob[clientId].Mobs.Player.Guild;
						if(pMob[clientId].GuildDisable != 0)
							continue;

						INT32 LOCAL_71 = pMob[LOCAL_65].Mobs.Player.Guild;
						if(pMob[LOCAL_65].GuildDisable != 0)
							LOCAL_71 = 0;

						INT32 LOCAL_72 = g_pGuildAlly[LOCAL_70]; // 00451277 - GuildAlly
						if(LOCAL_72 == 0)
							LOCAL_72 = -2;

						if(LOCAL_70 == 0 && LOCAL_71 == 0)
							LOCAL_70 = -1;

						if(LOCAL_68 == LOCAL_69)
							continue;

						if(LOCAL_70 == LOCAL_71)
							continue;

						if(LOCAL_72 == LOCAL_71)
							continue;

						if(LOCAL_40 == 0)
						{
							LOCAL_40 = LOCAL_65;

							continue;
						}

						if(LOCAL_40 == LOCAL_65)
							continue;

						if(LOCAL_41 == 0)
						{
							LOCAL_41 = LOCAL_65;
							continue;
						}

						if(LOCAL_41 == LOCAL_65)
							continue;

						if(LOCAL_42 == 0)
						{
							LOCAL_42 = LOCAL_65;

							continue;
						}

						if(LOCAL_42 == LOCAL_65)
							continue;

						if(LOCAL_43 == 0)
						{
							LOCAL_43 = LOCAL_65;
							continue;
						}

						if(LOCAL_43 == LOCAL_65)
							continue;

						if(LOCAL_44 == 0)
						{
							LOCAL_44 = LOCAL_65;

							continue;
						}

						if(LOCAL_44 == LOCAL_65)
							continue;

						if(LOCAL_45 == 0)
						{
							LOCAL_45 = LOCAL_65;
							break;
						}
					}

					if(LOCAL_45 != 0)
						break;
				}
			}

			if(LOCAL_40 != 0)
			{
				MSG_AttackOne LOCAL_95;
				memset(&LOCAL_95, 0, sizeof MSG_AttackOne);

				LOCAL_95.Header.PacketId = 0x367;
				LOCAL_95.Header.ClientId = clientId;
				LOCAL_95.Header.Size = sizeof MSG_AttackOne;

				LOCAL_95.Header.TimeStamp = 0x0E0A1ACA;
				LOCAL_95.PosX = pMob[clientId].Target.X;
				LOCAL_95.PosY = pMob[clientId].Target.Y;

				LOCAL_95.Motion = -2;

				if(pMob[clientId].Mobs.Player.Equip[0].sIndex == 219)
					LOCAL_95.Motion = -4;

				LOCAL_95.AttackerID = clientId;
				LOCAL_95.SkillIndex = 33;

				INT32 LOCAL_96 = Rand() % 100;

				INT32 LOCAL_97 = LOCAL_33 + LOCAL_34  + LOCAL_96;

				LOCAL_95.Target[0].TargetID = LOCAL_40;
				LOCAL_95.Target[0].Damage = -1;

				if(LOCAL_97 > 300)
				{
					LOCAL_95.Target[1].TargetID = LOCAL_41;
					LOCAL_95.Target[1].Damage = -1;
				}
				if(LOCAL_97 > 350)
				{
					LOCAL_95.Target[2].TargetID = LOCAL_42;
					LOCAL_95.Target[2].Damage = -1;
				}
				if(LOCAL_97 > 400)
				{
					LOCAL_95.Target[3].TargetID = LOCAL_43;
					LOCAL_95.Target[3].Damage = -1;
				}
				if(LOCAL_97 > 450)
				{
					LOCAL_95.Target[4].TargetID = LOCAL_44;
					LOCAL_95.Target[4].Damage = -1;
				}
				if(LOCAL_97 > 500)
				{
					LOCAL_95.Target[5].TargetID = LOCAL_45;
					LOCAL_95.Target[5].Damage = -1;
				}

				if(clientId < MAX_PLAYER)
				{
					pUser[clientId].TimeStamp.TimeStamp = 0x00E0A1ACA;
					pUser[clientId].PacketControl((BYTE*)&LOCAL_95, sizeof MSG_AttackOne);
				}
				else
				{
					//004515B2
					INT32 LOCAL_98 = 0;

					for(;  LOCAL_98 < 13; LOCAL_98 ++)
					{
						INT32 LOCAL_99 = LOCAL_95.Target[LOCAL_98].TargetID;
						INT32 LOCAL_100 = pMob[clientId].Mobs.Player.BaseScore.Damage;
						LOCAL_100 = (LOCAL_100 * ((Rand() & 0x80000007) + 5)) / 10;
						LOCAL_100 = LOCAL_100 - (pMob[clientId].Mobs.Player.CurrentScore.Ac >> 1);

						if(LOCAL_100 < 0)
							LOCAL_100 = Rand() % 100;

						LOCAL_95.Target[LOCAL_98].Damage = LOCAL_100;
						if(LOCAL_99 <= 0 || LOCAL_99 >= 30000)
							continue;

						INT32 LOCAL_101 = pMob[LOCAL_99].Leader;
						if(LOCAL_95.Target[LOCAL_98].Damage > 0)
						{
							if(LOCAL_101 <= 0)
								LOCAL_101 = LOCAL_99;

							SetBattle(LOCAL_101, LOCAL_28);
							SetBattle(LOCAL_28, LOCAL_101);

							for(INT32 LOCAL_102 = 0; LOCAL_102 < 12;  LOCAL_102++)
							{
								INT32 LOCAL_103 = pMob[LOCAL_101].PartyList[LOCAL_102];

								if(LOCAL_103 < MAX_PLAYER)
									continue;

								if(pMob[LOCAL_103].Mode == 0 || !pMob[LOCAL_103].Mobs.Player.CurrentScore.Hp)
								{
									if(pMob[LOCAL_103].Mode != 0)
										DeleteMob(LOCAL_103, 1);

									pMob[LOCAL_103].PartyList[LOCAL_102] = 0;
									continue;
								}

								SetBattle(LOCAL_103, LOCAL_28);
								SetBattle(LOCAL_28, LOCAL_103);
							}
						}

						if(LOCAL_95.Target[0].Damage > 0 || LOCAL_95.Target[0].Damage <= -5)
						{
							INT32 LOCAL_104 = LOCAL_95.Target[0].Damage;
							INT32 petDamage = 0;
							INT32 petId = pMob[LOCAL_99].Mobs.Player.Equip[14].sIndex;

							if(pMob[LOCAL_99].isPetAlive())
							{
								if (pMob[LOCAL_99].isNormalPet())
								{
									LOCAL_104 = AbsorveDamageByPet(&pMob[LOCAL_99], LOCAL_95.Target[0].Damage);
									petDamage = LOCAL_95.Target[0].Damage - LOCAL_104;
								}
								LOCAL_95.Target[0].Damage = LOCAL_104;
							}
							
							INT32 itemId = pMob[LOCAL_99].Mobs.Player.Equip[13].sIndex;
							if(itemId == 786 || itemId == 1936 || itemId == 1937)
							{
								INT32 LOCAL_107 = GetItemSanc(&pMob[LOCAL_99].Mobs.Player.Equip[13]);
								if(LOCAL_107 < 2)
									LOCAL_107 = 2;
								
								INT32 multHP = 1;
								switch(itemId)
								{
									case 1936:
										multHP = 10;
										break;

									case 1937:
										multHP = 1000;
										break;
								}

								multHP *= LOCAL_107;
								pMob[LOCAL_99].Mobs.Player.CurrentScore.Hp -= (LOCAL_95.Target[0].Damage / multHP);
							}
							else
								pMob[LOCAL_99].Mobs.Player.CurrentScore.Hp -= LOCAL_95.Target[0].Damage;

							if(LOCAL_99 >= MAX_PLAYER && pMob[LOCAL_99].Mode == 4)
								LinkMountHp(LOCAL_99);

							if(petDamage > 0)
								ProcessAdultMount(LOCAL_99 , petDamage);
						}

						if(LOCAL_99 > 0 && LOCAL_99 < MAX_PLAYER)
						{
							pUser[LOCAL_99].Potion.CountHp -= LOCAL_95.Target[0].Damage;

							SetReqHp(LOCAL_99);
						}

						if(pMob[LOCAL_99].Mobs.Player.CurrentScore.Hp <= 0)
							MobKilled(LOCAL_99, clientId, 0, 0);
					}

					GridMulticast_2(pMob[clientId].Target.X, pMob[clientId].Target.Y, (BYTE*)&LOCAL_95, 0);
				}
			}
		}

check:
		pMob[clientId].Mobs.Affects[LOCAL_28].Time -= 1;

		if((INT32)pMob[clientId].Mobs.Affects[LOCAL_28].Time <= 0)
		{
			if(pMob[clientId].Mobs.Affects[LOCAL_28].Index == 16)
			{
				LOCAL_2 = 1;

				pMob[clientId].Mobs.Player.Equip[0].sIndex = pMob[clientId].Mobs.Player.Equip[0].EF2;
			}

			if(pMob[clientId].Mobs.Affects[LOCAL_28].Index == 8)
				pMob[clientId].Jewel = 0;

			pMob[clientId].Mobs.Affects[LOCAL_28].Index = 0;
			pMob[clientId].Mobs.Affects[LOCAL_28].Time = 0;
			pMob[clientId].Mobs.Affects[LOCAL_28].Master = 0;
			pMob[clientId].Mobs.Affects[LOCAL_28].Value = 0;

			LOCAL_1 = 1;
		}

	}

	if(LOCAL_3 != 0)
	{
		if(clientId > 0 && clientId < MAX_PLAYER)
		{	
			SetReqHp(clientId);
		
			SetReqMp(clientId); 
		}

		p18A packet;
		packet.Header.PacketId = 0x18A;
		packet.Header.Size = 18;
		packet.Header.ClientId = clientId;

		packet.CurHP = pMob[clientId].Mobs.Player.CurrentScore.Hp;
		packet.Incress = LOCAL_4;

		INT32 LOCAL_162 = pMob[clientId].Target.X;
		INT32 LOCAL_163 = pMob[clientId].Target.Y;

		GridMulticast_2(LOCAL_162, LOCAL_163, (BYTE*)&packet, 0);
	}

	if(LOCAL_6 != 0)
	{
	//	p364 packet;

	//	GetCreateMob(clientId, (BYTE*)&packet);
	//	GridMulticast_2(pMob[clientId].Target.X, pMob[clientId].Target.Y,  (BYTE*)&packet, 0);
	}
	
	if(LOCAL_1 != 0)
	{
		pMob[clientId].GetCurrentScore(clientId);

		SendScore(clientId);

		if(LOCAL_2 != 0)
			SendEquip(clientId);

		//p364 packet;
		//GetCreateMob(clientId, (BYTE*)&packet);

		//GridMulticast_2(pMob[clientId].Target.X, pMob[clientId].Target.Y, (BYTE*)&packet, 0);

		return 1;
	}

	return 0;
}

int ApplyHp(int clientId)
{
	//0x00401253
	if(pUser[clientId].Potion.CountHp > pMob[clientId].Mobs.Player.CurrentScore.MaxHp)
		pUser[clientId].Potion.CountHp = pMob[clientId].Mobs.Player.CurrentScore.MaxHp;
	
	int LOCAL_1 = pMob[clientId].Mobs.Player.CurrentScore.Hp,
		LOCAL_2 = pUser[clientId].Potion.CountHp;

	if(LOCAL_2 <= LOCAL_1)
		return 0;

	INT32 level =  pMob[clientId].Mobs.Player.CurrentScore.Level;
	if(pMob[clientId].Mobs.Player.Equip[0].EFV2 >= CELESTIAL)
		level += 400;

	int LOCAL_3 = level + 100;
	pMob[clientId].Mobs.Player.CurrentScore.Hp += LOCAL_3;
		
	if(pMob[clientId].Mobs.Player.CurrentScore.Hp > LOCAL_2)
		pMob[clientId].Mobs.Player.CurrentScore.Hp = LOCAL_2;
		
	return 1;
}

int ApplyMp(int clientId)
{
	//0x00401253
	if(pUser[clientId].Potion.CountMp > pMob[clientId].Mobs.Player.CurrentScore.MaxMp)
		pUser[clientId].Potion.CountMp = pMob[clientId].Mobs.Player.CurrentScore.MaxMp;
	
	int LOCAL_1 = pMob[clientId].Mobs.Player.CurrentScore.Mp,
		LOCAL_2 = pUser[clientId].Potion.CountMp;

	if(LOCAL_2 <= LOCAL_1)
		return 0;
	
	INT32 level =  pMob[clientId].Mobs.Player.CurrentScore.Level;
	if(pMob[clientId].Mobs.Player.Equip[0].EFV2>= CELESTIAL)
		level += 400;

	int LOCAL_3 = level + 100;
		
	pMob[clientId].Mobs.Player.CurrentScore.Mp += LOCAL_3;
		
	if(pMob[clientId].Mobs.Player.CurrentScore.Mp > LOCAL_2)
		pMob[clientId].Mobs.Player.CurrentScore.Mp = LOCAL_2;
		
	return 1;
}

void AmountMinus(STRUCT_ITEM *item)
{
	int index = 0;
	int amount = 0;
	for(int i = 0;i < 3;i++)
	{
		if(item->stEffect[i].cEffect == EF_AMOUNT)
		{
			index = i;
			amount = item->stEffect[i].cValue;

			break;
		}
	}

	if(amount <= 1)
		memset(item, 0, sizeof STRUCT_ITEM);
	else
		item->stEffect[index].cValue --;
}

void MountProcess(int clientId, STRUCT_ITEM *item)
{
	STRUCT_ITEM *LOCAL_1 = &pMob[clientId].Mobs.Player.Equip[14];

	INT32 LOCAL_2 = 1;
	if(item != nullptr)
		memcpy(item, LOCAL_1, 8);

	if(LOCAL_2 == 0)
		return;

	INT32 LOCAL_3 = pMob[clientId].Leader;
	if(LOCAL_3 == 0)
		LOCAL_3 = clientId;

	for(INT32 LOCAL_4 = 0; LOCAL_4 < 12; LOCAL_4 ++)
	{
		INT32 LOCAL_5 = pMob[LOCAL_3].PartyList[LOCAL_4];

		if(LOCAL_5 <= 0 || LOCAL_5 > 30000)
			continue;

		INT32 LOCAL_6 = pMob[LOCAL_5].Mobs.Player.Equip[0].sIndex;

		if(pMob[LOCAL_5].Summoner == clientId && LOCAL_6 >= 315 && LOCAL_6 < 345)
			DeleteMob(LOCAL_5, 3);
	}

	INT32 LOCAL_7 = LOCAL_1->sIndex - 2320;
	if(LOCAL_7 >= 10 && LOCAL_7 < 40)
	{
		INT32 LOCAL_8 = GetItemAbility(LOCAL_1, 80);

		if(LOCAL_8 >= 0)
			GenerateSummon(clientId, LOCAL_7, LOCAL_1);
	}
}

void RemoveParty(INT32 clientId)
{
	CMob *player = &pMob[clientId];

	int leader = player->Leader;
	if (leader < 0 || leader >= MAX_MOB)
		return;

	if (leader && leader < MAX_PLAYER)
	{
		if (pUser[leader].Status != USER_PLAY)
		{
			player->Leader = 0;
			return;
		}
	}

	if (clientId > 0 && clientId < MAX_PLAYER)
		SendRemoveParty(clientId, 0);

	if (leader)
	{
		SendRemoveParty(leader, clientId);

		CMob *Leader = &pMob[leader];
		player->Leader = 0;

		for (int i = 0; i < 12; i++)
		{
			int partyMob = Leader->PartyList[i];

			if (!partyMob)
				continue;

			// Remove o clientid que saiu do grupo da array do lider
			if (partyMob == clientId)
				Leader->PartyList[i] = 0;

			// Remove os nomes da janela do client que saiu do grupo
			if (partyMob > 0 && partyMob < MAX_PLAYER)
				if (pUser[partyMob].Status == USER_PLAY)
					SendRemoveParty(partyMob, clientId);
		}

	}
	else // Lider saindo do grupo
	{
		int groupCount = 0;
		// Encontra o namero total de players no grupo
		for (INT8 i = 0; i < 12; i++)
			if (player->PartyList[i] < MAX_PLAYER && pUser[player->PartyList[i]].Status == USER_PLAY)
				groupCount++;

		bool isClueLeader{ false };
		int clueSanc{ -1 };
		int clueParty{ -1 };

		// itera sobre todas as pistas
		for (int iPista = 0; iPista < 10; ++iPista)
		{
			// itera sobre todos os grupos
			for (int iParty = 0; iParty < MAX_PARTYPISTA; ++iParty)
			{
				auto& pista = pPista[iPista].Clients[iParty];

				int memberId = pista[12];
				if (memberId > 0 && memberId < MAX_PLAYER && clientId == memberId)
				{
					clueParty = iParty;
					clueSanc = iPista;

					isClueLeader = true;
					break;
				}
			}

			if (isClueLeader)
				break;
		}

		int newLeader = 0;
		for (int i = 0; i < 12; ++i)
		{
			if (player->PartyList[i] <= 0 || player->PartyList[i] >= MAX_PLAYER)
				continue;

			newLeader = player->PartyList[i];
			break;
		}

		// Caso o primeiro indice seja menor que 750 e o grupo tenha 2 jogadores ou mais.
		if (newLeader != 0 && groupCount >= 2)
		{
			// O primeiro indice vai ser o novo lider.
			CMob *newLider = &pMob[newLeader];

			for (int i = 0, count = 0; i < 12; ++i)	
			{
				if (player->PartyList[i] != 0 && player->PartyList[i] != newLeader)
					newLider->PartyList[count++] = player->PartyList[i];
			}

			// Informa o novo lider de sua condiaao
			newLider->Leader = 0;

			// Remove o cara 
			SendRemoveParty(newLeader, clientId);

			// Adiciona ele de novo como lider, provavelmente.
			SendAddParty(newLeader, newLeader, 1);

			if (isClueLeader)
			{
				auto& pista = pPista[clueSanc].Clients[clueParty];
				pista[12] = newLeader;

				for (int i = 0; i < 12; ++i)
				{
					if (pPista[clueSanc].Clients[clueParty][i] == newLeader)
					{
						pPista[clueSanc].Clients[clueParty][i] = 0;

						break;
					}
				}

				for (int i = 0; i < 13; ++i)
				{
					int memberId = pista[i];
					if (memberId <= 0 || memberId >= MAX_PLAYER)
						continue;

					SendClientMessage(memberId, "O lader do grupo da pista foi passado para %s", newLider->Mobs.Player.MobName);
					Log(memberId, LOG_INGAME, "Lader passado para o usuario %s (%s)", newLider->Mobs.Player.MobName, pUser[newLeader].User.Username);
				}
			}		
		}
		else // Remove todo mundo do grupo
		{
			for (int i = 0; i < 12; i++)
			{
				int partyMob = player->PartyList[i];

				if (partyMob <= 0 || partyMob > MAX_MOB)
					continue;

				player->PartyList[i] = 0;

				CMob *Member = &pMob[partyMob];
				Member->Leader = 0;

				if (partyMob <= 0 || partyMob >= MAX_PLAYER)
					continue;

				if (pUser[partyMob].Status == 22)
					SendRemoveParty(partyMob, 0);
			}

			return;
		}

		for (int iterator = 0; iterator < 12; iterator++)
		{
			int partyMob = player->PartyList[iterator];
			if (partyMob <= 0 || partyMob > MAX_MOB || partyMob == newLeader)
				continue;

			CMob *mob = &pMob[partyMob];
			mob->Leader = newLeader;

			SendRemoveParty(partyMob, clientId);
			SendAddParty(partyMob, player->PartyList[0], 1);
		}

		memset(player->PartyList, 0, sizeof INT16 * 12);
	}
}


//void RemoveParty(int clientId, int targetId)
//{
//	// Se targetId != 0 quer dizer que o lader tentou usar o sistema de kikar usuario
//	if(targetId != 0)
//	{
//		CMob *spw = &pMob[clientId];
//		// Checa se o usuario a lader
//		// Se for = 0, a possavel retirar o usuario
//		if(spw->Leader != 0)
//		{
//			SendClientMessage(clientId, g_pLanguageString[_NN_Party_Leader_Only]);
//
//			return;
//		}
//	}
//
//	if(targetId == 0)
//		targetId = clientId;
//	
//	int leader = pMob[targetId].Leader;
//	if(leader == 0)
//		leader = targetId;
//
//	
//
//	// Retira quem foi retirado do lader do grupo
//	SendRemoveParty(leader, targetId);
//	SendRemoveParty(targetId, leader);
//
//	if(targetId > 0 && targetId < MAX_PLAYER)
//		SendRemoveParty(targetId, 0);
//
//	if(clientId > 0 && clientId < MAX_PLAYER)
//		SendRemoveParty(clientId, 0);
//
//	// Quem foi retirado fica sem liderenaa
//	pMob[targetId].Leader = 0;
//
//	int count = 0;
//	for (int i = 0; i < 12; i++)
//		if (pMob[leader].PartyList[i] != 0)
//			count++;
//
//	if(count >= 2)
//	{ // Grupo ainda ha outras pessoas
//		if(leader == targetId)
//		{
//			int secondUser = 0;
//			int tmpSecondUser = 0;
//
//			for(int i = 0 ;i < 12; i ++)
//			{
//				int mobId = pMob[leader].PartyList[i];
//				if(mobId <= 0 || mobId >= MAX_MOB)
//					continue;
//
//				tmpSecondUser = mobId;
//				break;
//			}
//
//			for(int i = 0 ;i < 12; i++)
//			{ // Procura o segundo usuario possavel do grupo
//				int memberId = pMob[leader].PartyList[i];
//				if(memberId <= 0 || memberId > MAX_PLAYER)
//					continue;
//
//				if(tmpSecondUser == memberId)
//					continue;
//
//				secondUser = tmpSecondUser;
//				break;
//			}
//			
//			// Caso nao exista um segundo usuario (caso de evocaaaes apenas no grupo), simplesmente o desfaz assim como a TMsrv faz
//			if(secondUser == 0)
//			{
//				pMob[leader].Leader = 0;
//
//				SendRemoveParty(leader, 0);
//				for(int i = 0; i < 12; i++)
//				{
//					int mobId = pMob[leader].PartyList[i];
//					if(mobId <= 0 || mobId >= MAX_MOB)
//						continue;
//
//					pMob[leader].PartyList[i] = 0;
//					pMob[mobId].Leader = 0;
//
//					if(mobId < MAX_PLAYER)
//						SendRemoveParty(mobId, 0);
//				}
//			}
//			else
//			{
//				// Seto o nvoo lader como 'lader de algum grupo'
//				pMob[secondUser].Leader = 0;
//
//				SendRemoveParty(secondUser, targetId);
//				SendAddParty(secondUser, secondUser, 1);
//
//				for(int x = 0; x < 12; x++)
//				{
//					int mobId = pMob[leader].PartyList[x];
//					if(mobId <= 0 || mobId >= MAX_MOB)
//						continue;
//
//					if(mobId == secondUser)
//					{
//						pMob[leader].PartyList[x] = 0;
//						continue;
//					}
//
//					pMob[mobId].Leader = secondUser;
//
//					SendRemoveParty(mobId, targetId);
//					SendAddParty(mobId, secondUser, 1);
//				}
//
//				memcpy(pMob[secondUser].PartyList, pMob[leader].PartyList, sizeof pMob[secondUser].PartyList);
//
//				memset(pMob[leader].PartyList, 0, sizeof pMob[leader].PartyList);
//			}
//		} 
//		else
//		{
//			SendRemoveParty(leader, targetId);
//
//			for(int i = 0 ; i < 12; i ++)
//			{
//				int mobId = pMob[leader].PartyList[i];
//				if(mobId <= 0 || mobId >= MAX_MOB)
//					continue;
//
//				if(mobId == targetId)
//					pMob[leader].PartyList[i] = 0;
//
//				if(mobId < MAX_PLAYER)
//					SendRemoveParty(mobId, targetId);
//			}
//		}
//	}
//	else
//	{ // Grupo sera totalmente desfeito
//		pMob[leader].Leader = 0;
//
//		SendRemoveParty(leader, 0);
//
//		for(int i = 0; i < 12; i++)
//		{
//			int mobId = pMob[leader].PartyList[i];
//			if(mobId <= 0 || mobId >= MAX_MOB)
//				continue;
//
//			pMob[leader].PartyList[i] = 0;
//			pMob[mobId].Leader = 0;
//
//			if(mobId < MAX_PLAYER)
//				SendRemoveParty(mobId, 0);
//		}
//	}
//}

void RegenMob(int User)
{
	if (User <= 0 || User >= MAX_PLAYER)
		return;

	int GuildID = pMob[User].Mobs.Player.Guild;

	pUser[User].CharLoginTime++;

	if (!(pUser[User].CharLoginTime % 450))
	{/*
		int Frag = GetPKPoint(User);
		if (Frag < 75)
		{
			Frag++;

			SetPKPoint(User, Frag);
			SendClientMessage(User, g_pLanguageString[_DD_PKPointPlus], Frag - 75, 1);
		}*/

		STRUCT_ITEM *MountBufferItem = &pMob[User].Mobs.Player.Equip[14];
		if (MountBufferItem->sIndex >= 2300 && MountBufferItem->sIndex < 2330)
		{
			int Delay = GetItemAbility(MountBufferItem, EF_INCUDELAY);
			if (Delay > 0)
			{
				Delay--;
				MountBufferItem->EFV3 = Delay;

				SendItem(User, SlotType::Equip, 14, &pMob[User].Mobs.Player.Equip[14]);

				SendClientMessage(User, g_pLanguageString[_NN_Incu_Proceed]);

				LogPlayer(User,  "Perdeu 1 incubaaao do ovo %s. Incubaaao atual: %d", g_pItemList[MountBufferItem->sIndex].ItemName, Delay);
				Log(User, LOG_INGAME, "Perdeu 1 incubaaao do ovo %s [%d]. Incubaaao atual: %d", g_pItemList[MountBufferItem->sIndex].ItemName, MountBufferItem->sIndex, Delay);
			}
		}

		if (MountBufferItem->sIndex >= 2330 && MountBufferItem->sIndex < 2390 && *(short*)&MountBufferItem->EF1 > 0)
		{
			int v44 = MountBufferItem->EF3;

			int v48 = (MountBufferItem->sIndex - 2330) % 30;

			if (v48 <= 15)
				v44 -= 2;
			else
				v44 -= 4;

			bool revi = false;
			for(INT32 i = 0; i < 32; i++)
			{
				if(pMob[User].Mobs.Affects[i].Index == 51)
				{
					revi = true;
					break;
				}
			}

			if(!revi)
			{
				if (v44 <= 1)
				{
					*(short*)&MountBufferItem->EF1 = 0;
					MountBufferItem->EF3 = 4;

					SendClientMessage(User, g_pLanguageString[_NN_Mount_died]);

					Log(User, LOG_INGAME, "Montaria %s %s morreu por falta de raaao", g_pItemList[MountBufferItem->sIndex].ItemName, MountBufferItem->toString().c_str());
					LogPlayer(User, "Montaria %s morreu por falta de raaao. Alimite a montaria constantemente", g_pItemList[MountBufferItem->sIndex].ItemName);

					if (MountBufferItem->sIndex >= 2360 && MountBufferItem->sIndex < 2390)
						ProcessAdultMount(User, 0);
					if (MountBufferItem->sIndex < 2360)
						MountProcess(User, nullptr);
					else
						pMob[User].GetCurrentScore(User);
				}
				else
					MountBufferItem->EF3 = v44;
			}

			SendItem(User, SlotType::Equip, 14, &pMob[User].Mobs.Player.Equip[14]);
		}
	}

	int Guilty = GetGuilty(User);
	if (Guilty > 0)
	{
		Guilty--;
		SetGuilty(User, Guilty);

		if (Guilty == 0)
		{
			p364 m;
			GetCreateMob(User, (BYTE*) &m);

			GridMulticast_2(pMob[User].Target.X, pMob[User].Target.Y, (BYTE*) &m, 0);

			for (int s = MAX_PLAYER; s < 30000; s++)
			{
				if (pMob[s].Mode != 5)
					continue;

				if (pMob[s].Mobs.Player.CapeInfo != 4)
					continue;

				if (pMob[s].CurrentTarget == User)
				{
					pMob[s].CurrentTarget = 0;
					pMob[s].Mode = 4;
				}

				for (int l = 0; l < 4; l++)
				{
					if (pMob[s].EnemyList[l] != User)
						continue;

					pMob[s].EnemyList[l] = 0;
					pMob[s].Mode = 4;
				}
			}
		}
	}

	int regenBase = 100;
	eMapAttribute vD4 = GetAttribute(pMob[User].Target.X, pMob[User].Target.Y);

	if (vD4.Village)
		regenBase = 500;

	if (vD4.Guild)
		regenBase = 1000;

#pragma region 7556
	if (pMob[User].Mode && pMob[User].Mobs.Player.CurrentScore.Hp && User < MAX_PLAYER && pUser[User].Status == USER_PLAY)
	{
		int currentHp = pMob[User].Mobs.Player.CurrentScore.Hp;
		int currentMp = pMob[User].Mobs.Player.CurrentScore.Mp;
		int maxHp = pMob[User].Mobs.Player.CurrentScore.MaxHp;
		int maxMp = pMob[User].Mobs.Player.CurrentScore.MaxMp;
		int LOCAL74 = currentHp;
		int regen = pMob[User].RegenHP;
		// LOCAL68 = vCC

		int newHp = (regenBase + regen) * 5;
		currentHp = currentHp + newHp;

		if (currentHp > maxHp)
			currentHp = maxHp;

		pMob[User].Mobs.Player.CurrentScore.Hp = currentHp;

		regen = pMob[User].RegenMP;
		
		int newMp = (regenBase + regen) * 5;
		currentMp = currentMp + newMp;

		if (currentMp > maxMp)
			currentMp = maxMp;

		pMob[User].Mobs.Player.CurrentScore.Mp = currentMp;

		p181 packet;
		packet.Header.PacketId = 0x181;
		packet.Header.ClientId = User;
		packet.Header.Size = sizeof p181;

		packet.Hp = currentHp;
		packet.Mp = currentMp;

		SetReqHp(User);
		SetReqMp(User);

		if (User > 0 && User < MAX_PLAYER)
		{
			packet.MaxHp = pUser[User].Potion.CountHp;
			packet.MaxMp = pUser[User].Potion.CountMp;
		}

		int TargetX = pMob[User].Target.X;
		int TargetY = pMob[User].Target.Y;

		GridMulticast_2(TargetX, TargetY, (BYTE*)&packet, 0); // maybe

		SendSetHpMp(User);
	}
#pragma endregion

	/*if (pMob[User].Mode && pMob[User].Mobs.Player.CurrentScore.Hp && User >= MAX_PLAYER || Users[User].CurrentScore == USER_PLAY)
	{
		int MaxHp = pMob[User].Mobs.Player.CurrentScore.MaxHp;
		int MaxMp = pMob[User].Mobs.Player.CurrentScore.MaxMp;
		int Hp = pMob[User].Mobs.Player.CurrentScore.Hp;
		int Mp = pMob[User].Mobs.Player.CurrentScore.Mp;

		int calcRegenHPMP = pMob[User].Mobs.Player.RegenHP;
		calcRegenHPMP = calcRegenHPMP * vCC / 100;

		int calcRegenHP = MaxHp * calcRegenHPMP / 120 + vD0;
		Hp = Hp + calcRegenHP;

		if (Hp > MaxHp)
			Hp = MaxHp;

		pMob[User].Mobs.Player.CurrentScore.Hp = Hp;

		calcRegenHPMP = pMob[User].Mobs.Player.RegenMP;

		calcRegenHPMP = calcRegenHPMP * vCC / 100;

		int calcRegenMP = MaxMp * calcRegenHPMP / 120 + vD0;

		Mp = Mp + calcRegenMP;

		if (Mp > MaxMp)
			Mp = MaxMp;

		pMob[User].Mobs.Player.CurrentScore.Mp = Mp;

		p181 packet;
		packet.Header.PacketId = 0x181;
		packet.Header.ClientId = User;
		packet.Header.Size = sizeof p181;

		packet.Hp = Hp;
		packet.Mp = Mp;

		SetReqHp(User);
		SetReqMp(User);

		packet.MaxHp = Users[User].Potion.CountHp;
		packet.MaxMp = Users[User].Potion.CountMp;

		int TargetX = pMob[User].Target.X;
		int TargetY = pMob[User].Target.Y;

		GridMulticast_2(TargetX, TargetY, (BYTE*) &packet, 0); // maybe

	}*/
}

int UpdateItem(int arg1, int arg2, int* arg3)
{ // ADDRBASE = 0x8B9E778
	INT32 LOCAL_1 = g_pInitItem[arg1].Rotation;

	INT32 LOCAL_2 = UpdateItem2(g_pInitItem[arg1].CanRun, g_pInitItem[arg1].CurrentScore, arg2, g_pInitItem[arg1].PosX, g_pInitItem[arg1].PosY, LOCAL_1, arg3);
	if(LOCAL_2 == 0)
		return 0;

	g_pInitItem[arg1].HeightGrid = *arg3;

	INT32 LOCAL_3 = g_pInitItem[arg1].CanRun;
	INT32 LOCAL_4 = g_pInitItem[arg1].CurrentScore;

	g_pInitItem[arg1].IsOpen = 0;

	if(LOCAL_3 >= 6 || LOCAL_3 < 0)
		return 1;

	if(LOCAL_4 != arg2)
	{
		INT32 LOCAL_5 = g_pInitItem[arg1].PosX;
		INT32 LOCAL_6 = g_pInitItem[arg1].PosY;

		for(INT32 LOCAL_7 = 0; LOCAL_7 <= 4; LOCAL_7 ++)
		{
			for(INT32 LOCAL_8 = 0; LOCAL_8 <= 4; LOCAL_8 ++)
			{// 0045F785
				INT32 LOCAL_9 = g_pGroundMask[LOCAL_3][LOCAL_1][LOCAL_7][LOCAL_8];
				UINT32 LOCAL_10 = LOCAL_5 + LOCAL_8 - 2;
				UINT32 LOCAL_11 = LOCAL_6 + LOCAL_7 - 2;

				if(LOCAL_10 < 1 || LOCAL_11 < 1 || LOCAL_10 > 4094 || LOCAL_11 > 4094)
					continue;

				if(LOCAL_9 == 0)
					continue;

				INT32 LOCAL_12 = g_pMobGrid[LOCAL_11][LOCAL_10];
				if(LOCAL_12 == 0)
					continue;

				if(pMob[LOCAL_12].Mobs.Player.Equip[0].sIndex == 220)
				{
					DeleteMob(LOCAL_12, 3);

					continue;
				}

				INT32 LOCAL_13 = GetEmptyMobGrid(LOCAL_12, &LOCAL_10, &LOCAL_11);
				if(LOCAL_13 != 0)
				{
					pMob[LOCAL_12].Route[0] = 0;

					p36C LOCAL_26;
					GetAction(LOCAL_12, LOCAL_10, LOCAL_11, &LOCAL_26);

					LOCAL_26.Speed = 20;
					LOCAL_26.stEffect = 0;

					GridMulticast_2(LOCAL_10, LOCAL_11, (BYTE*)&LOCAL_26, 0);

					if(LOCAL_12 < MAX_PLAYER)
						pUser[LOCAL_12].AddMessage((BYTE*)&LOCAL_26, sizeof p36C);
				}
			}
		}
	}

	INT32 LOCAL_27 = GetItemAbility(&g_pInitItem[arg1].Item, EF_KEYID);

	//if(LOCAL_27 == 15 && LOCAL_4 == 1 && arg2 == 3)
	//	CreateMob("GATE", g_pInitItem[arg1].PosX, g_pInitItem[arg1].PosY, "npc");

	g_pInitItem[arg1].CurrentScore = arg2;
	return 1;
}


int UpdateItem2(int arg1,int arg2,int arg3,int arg4,int arg5,int arg7,int *arg8)
{
	*arg8 = 0;

	INT32 LOCAL_1 = 0;
	if(arg1 >= 6 || arg1 < 0)
		return 0;

	if(arg2 == 1 && (arg3 == 3 || arg3 == 2))
		LOCAL_1 = 1;

	if(arg3 == 1 && (arg2 == 3 || arg2 == 2))
		LOCAL_1 = -1;

	if(LOCAL_1 == 0)
		return 0;
	
	INT32 LOCAL_2 = 0;
	for( ; LOCAL_2 <= 5; LOCAL_2 ++)
	{
		INT32 LOCAL_3 = 0;
		for( ; LOCAL_3 <= 5; LOCAL_3++)
		{
			INT32 LOCAL_4 = g_pGroundMask[arg1][arg7][LOCAL_2][LOCAL_3];
			INT32 LOCAL_5 = LOCAL_3 + arg4 - 2;
			INT32 LOCAL_6 = LOCAL_2 + arg5 - 2;

			if(LOCAL_4 == 0)
				continue;

			LOCAL_4 = LOCAL_4 * LOCAL_1;

			// 0040BCCB
			if(LOCAL_5 - g_HeightPosX < 1)
				break;

			if(LOCAL_6 - g_HeightPosY < 1)
				break;

			if(LOCAL_5 - g_HeightPosX > g_HeightWidth - 2)
				break;

			if(LOCAL_5 - g_HeightPosY > g_HeightHeight - 2)
				break;

			INT32 LOCAL_7 = g_pHeightGrid[LOCAL_6 - g_HeightPosY][LOCAL_5 - g_HeightPosX] + LOCAL_4;
			if(LOCAL_7 > 255)
				LOCAL_7 = 255;

			if(LOCAL_7 < 0)
				LOCAL_7 = 0;

			if(g_pGroundMask[arg1][arg7][LOCAL_2][LOCAL_3] != 0)
				*arg8 = LOCAL_7;

			g_pHeightGrid[LOCAL_6 - g_HeightPosY][LOCAL_5 - g_HeightPosX] = LOCAL_7;
		}
	}

	return 1;
}

bool ReadMob(STRUCT_MOB *mob, const char *folder)
{
	FILE *pFile = NULL;
		
	char szTMP[1024];
	sprintf_s(szTMP, "%s/%s", folder, mob->MobName);

	fopen_s(&pFile, szTMP, "rb");
	if(pFile) 
	{
		fread(mob, 1, sizeof STRUCT_MOB, pFile);		
		fclose(pFile);
		return true;
	}

	return false;
}

INT32 CreateMob(const char *mob, int posX, int posY, const char *folder)
{
	INT32 LOCAL_1 = mGener.GetEmptyNPCMob();
	if(LOCAL_1 == 0)
		return -1;

	pMob[LOCAL_1] = CMob{};
	pMob[LOCAL_1].BossInfoId = MAXUINT32;
	pMob[LOCAL_1].clientId = LOCAL_1;
	strncpy_s(pMob[LOCAL_1].Mobs.Player.MobName, mob, 16);

	memset(&pMob[LOCAL_1].PartyList, 0, sizeof INT16 * 12);

	INT32 LOCAL_2 = ReadMob(&pMob[LOCAL_1].Mobs.Player, folder);
	if(LOCAL_2 == 0)
		return false;

	pMob[LOCAL_1].Mobs.Player.MobName[15] = 0;

	INT32 LOCAL_3;
	for(LOCAL_3 = 0; LOCAL_3 < 16;LOCAL_3 ++)
	{
		if(pMob[LOCAL_1].Mobs.Player.MobName[LOCAL_3] == '_')
			pMob[LOCAL_1].Mobs.Player.MobName[LOCAL_3] = ' ';
		if (pMob[LOCAL_1].Mobs.Player.MobName[LOCAL_3] == '@')
			pMob[LOCAL_1].Mobs.Player.MobName[LOCAL_3] = ' ';
	}

	memset(&pMob[LOCAL_1].Mobs.Affects, 0, sizeof STRUCT_AFFECT * 32);

	for(LOCAL_3 = 0; LOCAL_3 < 5; LOCAL_3 ++)
	{
		if(pMob[LOCAL_1].Mobs.Player.Equip[0].sIndex == 220 || pMob[LOCAL_1].Mobs.Player.Equip[0].sIndex == 219 || pMob[LOCAL_1].Mobs.Player.Equip[0].sIndex == 358)
		{
			pMob[LOCAL_1].Segment.ListX[LOCAL_3] = posX;
			pMob[LOCAL_1].Segment.ListY[LOCAL_3] = posY;
		}
		else
		{
			pMob[LOCAL_1].Segment.ListX[LOCAL_3] = posX + (Rand() % 5) - 2;
			pMob[LOCAL_1].Segment.ListY[LOCAL_3] = posY + (Rand() % 5) - 2;
		}
	}

	pMob[LOCAL_1].GenerateID = -1;
	pMob[LOCAL_1].Formation = 0;
	pMob[LOCAL_1].RouteType = 0;
	pMob[LOCAL_1].Mode = 4;
	pMob[LOCAL_1].Segment.Progress = 0;
	pMob[LOCAL_1].Leader = 0;
	pMob[LOCAL_1].WaitSec = 10;

	pMob[LOCAL_1].clientId = LOCAL_1;

	pMob[LOCAL_1].GetCurrentScore(MAX_PLAYER);

	pMob[LOCAL_1].Mobs.Player.CurrentScore.Hp = pMob[LOCAL_1].Mobs.Player.CurrentScore.MaxHp;
	pMob[LOCAL_1].Segment.Direction = 0;

	memset(&pMob[LOCAL_1].EnemyList, 0, 8);
	
	UINT32 LOCAL_5 = pMob[LOCAL_1].Segment.ListX[0];
	UINT32 LOCAL_6 = pMob[LOCAL_1].Segment.ListY[0];

	INT32 LOCAL_7 = GetEmptyMobGrid(LOCAL_1, &LOCAL_5, &LOCAL_6);
	if(LOCAL_7 == 0)
	{
		pMob[LOCAL_1].Mode = 0;
		pMob[LOCAL_1].Mobs.Player.MobName[0] = 0;
		pMob[LOCAL_1].GenerateID = -1;

		return -2;
	}

	pMob[LOCAL_1].Last.Time = clock();

	pMob[LOCAL_1].Segment.X = LOCAL_5;
	pMob[LOCAL_1].Target.X = LOCAL_5;
	pMob[LOCAL_1].Last.X = LOCAL_5;

	pMob[LOCAL_1].Segment.Y = LOCAL_6;
	pMob[LOCAL_1].Target.Y = LOCAL_6;
	pMob[LOCAL_1].Last.Y = LOCAL_6;

	INT32 LOCAL_17 = pMob[LOCAL_1].Mobs.Player.BaseScore.MaxMp;
	if(LOCAL_17 != 0)
	{
		SetAffect(LOCAL_1, LOCAL_17, 30000, 200);
		SetTick(LOCAL_1, LOCAL_17, 30000, 200);
	}

	g_pMobGrid[LOCAL_6][LOCAL_5] = LOCAL_1;
	
	pMob[LOCAL_1].SpawnType = 2;
	SendGridMob(LOCAL_1);
	pMob[LOCAL_1].SpawnType = 0;

	return LOCAL_1;
} 


void DoRecall(int clientId)
{
	UINT32 LOCAL_1 = 0;
	UINT32 LOCAL_2 = 0;
	INT32 LOCAL_3 = pMob[clientId].Mobs.Player.Info.CityID;
	
	LOCAL_1 = g_pCityZone[LOCAL_3].city_x + (Rand() % 15);
	LOCAL_2 = g_pCityZone[LOCAL_3].city_y + (Rand() % 15);

	INT32 LOCAL_4 = pMob[clientId].Mobs.Player.Guild;
	if(LOCAL_4 > 0)
	{
		for(INT32 LOCAL_5 = 0; LOCAL_5 < 5; LOCAL_5++)
		{
			if(ChargedGuildList[sServer.Channel - 1][LOCAL_5] == LOCAL_4)
			{
				LOCAL_1 = g_pCityZone[LOCAL_5].area_guild_x;
				LOCAL_2 = g_pCityZone[LOCAL_5].area_guild_y;

				break;
			}
		}
	}

	// RVR
	if (pMob[clientId].Target.X >= 1041 && pMob[clientId].Target.X <= 1248 &&
		pMob[clientId].Target.Y >= 1950 && pMob[clientId].Target.Y <= 2158 && sServer.RvR.CurrentScore == 1)
	{
		int cape = pMob[clientId].Mobs.Player.CapeInfo;

		if (cape == CAPE_BLUE)
		{
			if (!(Rand() % 2))
			{
				LOCAL_1 = 1061 - Rand() % 5;
				LOCAL_2 = 2113 + Rand() % 5;
			}
			else
			{
				LOCAL_1 = 1091 - Rand() % 5;
				LOCAL_2 = 2140 + Rand() % 5;
			}
		}
		else
		{
			if (!(Rand() % 2))
			{
				LOCAL_1 = 1238 + Rand() % 5;
				LOCAL_2 = 1983 + Rand() % 5;
			}
			else
			{
				LOCAL_1 = 1211 + Rand() % 5;
				LOCAL_2 = 1955 + Rand() % 5;
			}
		}
	}

	bool LOCAL_6 = GetEmptyMobGrid(clientId, &LOCAL_1, &LOCAL_2);
	if (LOCAL_6 == false)
	{
		if (clientId < MAX_PLAYER)
			Log(clientId, LOG_INGAME, "DoRecall - Falha ao encontrar espaao vago no mapa para o usuario. Posiaao: %ux %uy", LOCAL_1, LOCAL_2);

		return;
	}

	p36C LOCAL_19;
	memset(&LOCAL_19, 0, sizeof p36C);

	GetAction(clientId, LOCAL_1, LOCAL_2, &LOCAL_19);

	LOCAL_19.stEffect = 1;
	if (clientId < MAX_PLAYER)
		pUser[clientId].AddMessage((BYTE*)&LOCAL_19, sizeof p36C);

	GridMulticast(clientId, LOCAL_1, LOCAL_2, (BYTE*)&LOCAL_19);

	Log(clientId, LOG_INGAME, "DoRecall - Enviado para cidade: %ux %uy", LOCAL_1, LOCAL_2);
}

void BASE_InitializeHitRate()
{
	memset(g_pHitRate, 0, 4096);

	INT32 LOCAL_1 = 512;
	INT32 LOCAL_2 = 0;
	INT32 LOCAL_3 = 0;

	while(true)
	{
		INT32 LOCAL_4 = 0;

		for(;LOCAL_4 < 1024; LOCAL_4 ++)
		{
			if(g_pHitRate[LOCAL_4] != 0)
				continue;

			if(LOCAL_3 == 0)
				g_pHitRate[LOCAL_4] = LOCAL_2;
			else if(LOCAL_3 == 1)
				g_pHitRate[LOCAL_4 ] = 512 - LOCAL_2;
			else if(LOCAL_3 == 2)
				g_pHitRate[LOCAL_4] = LOCAL_2 + 512;
			else
				g_pHitRate[LOCAL_4] = 1024 - LOCAL_2;

			if(g_pHitRate[LOCAL_4] > 999)
				g_pHitRate[LOCAL_4] = 999;

			LOCAL_3 ++;
			if(LOCAL_3 >= 4)
				LOCAL_3 = 0;
			if(LOCAL_3 == 0)
				LOCAL_2++;
		}

		LOCAL_1 /= 2;

		if(LOCAL_1 == 0)
			break;
	}
	
	g_pHitRate[0] = 512;
}

void ProcessAdultMount(int clientId, int damage)
{
	STRUCT_ITEM *LOCAL_1 = &pMob[clientId].Mobs.Player.Equip[14];

	if(LOCAL_1->sIndex < 2360 || LOCAL_1->sIndex >= 2390)
		return;

	// Checa se possui a porra do Poaao Revigorante xD
	for(INT32 i = 0 ; i < 32 ;i ++ )
	{
		if(pMob[clientId].Mobs.Affects[i].Index == 51)
			return;
	}

	INT32 LOCAL_2 = LOCAL_1->sIndex - 2360;
	INT32 LOCAL_3 = NPCBase[LOCAL_2 + 10].CurrentScore.MaxHp;
	INT32 LOCAL_4 = LOCAL_1->stEffect[2].cEffect;
	
	if(LOCAL_4 <= 0)
	{
		if(LOCAL_1->stEffect[1].cEffect)
		{
			*(short*)&LOCAL_1->stEffect[0].cEffect = 0;
			LOCAL_4 = 0;
		}
	}

	INT32 LOCAL_5 = *(short*)&LOCAL_1->stEffect[0].cEffect;
	INT32 LOCAL_6 = *(short*)&LOCAL_1->stEffect[0].cEffect - damage; // EBP - 18h

	if(LOCAL_6 >= LOCAL_3)
		LOCAL_6 = LOCAL_3;

	INT32 LOCAL_7 = LOCAL_6;

	*(short*)&LOCAL_1->stEffect[0].cEffect = LOCAL_7;

	if(LOCAL_6 <= 0)
		LOCAL_1->stEffect[2].cEffect = 0;

	// Pet morreu
	if(LOCAL_5 > 0 && LOCAL_6 <= 0)
	{
		SendEquip(clientId); // 401069

		Log(clientId, LOG_INGAME, "Pet %s [%d] [%d %d %d %d %d %d] morreu. Hit: %d", g_pItemList[LOCAL_1->sIndex].ItemName,
			LOCAL_1->sIndex, LOCAL_1->stEffect[0].cEffect, LOCAL_1->stEffect[0].cValue, LOCAL_1->stEffect[1].cEffect, LOCAL_1->stEffect[1].cValue, LOCAL_1->stEffect[2].cEffect, LOCAL_1->stEffect[2].cValue,
			damage);

		LogPlayer(clientId, "Pet %s morreu levando um hit de %d. HP antes do hit: %d", g_pItemList[LOCAL_1->sIndex].ItemName, damage, LOCAL_5);
	}

	if(LOCAL_5 != LOCAL_6 && clientId < MAX_PLAYER)
		SendItem(clientId, SlotType::Equip, 14, LOCAL_1); 
}

void LinkMountHp(int arg1)
{
	if(arg1 < MAX_PLAYER || arg1 >= 30000)
		return;

	if(pMob[arg1].Mobs.Player.CapeInfo != 4)
		return;

	INT32 LOCAL_1 = pMob[arg1].Mobs.Player.Equip[0].sIndex;
	if(LOCAL_1 < 315 || LOCAL_1 >= 345)
		return;

	INT32 LOCAL_2 = pMob[arg1].Summoner;
	if(pMob[LOCAL_2].Mode == 0 || pUser[LOCAL_2].Status != USER_PLAY)
		return;

	INT16 LOCAL_3 = pMob[LOCAL_2].Mobs.Player.Equip[14].sIndex - 2330;
	INT16 LOCAL_4 = LOCAL_1 - 315;

	if(LOCAL_3 != LOCAL_4)
		return;

	INT16 LOCAL_5 = *(short*)&pMob[LOCAL_2].Mobs.Player.Equip[14].stEffect[0].cEffect;
	INT16 LOCAL_6 = pMob[arg1].Mobs.Player.CurrentScore.Hp;
	if(LOCAL_5 != LOCAL_6)
	{
		*(short*)&pMob[LOCAL_2].Mobs.Player.Equip[14].stEffect[0].cEffect = LOCAL_6;

		SendItem(LOCAL_2, SlotType::Equip, 14, &pMob[LOCAL_2].Mobs.Player.Equip[14]);
	}
}

float TimeRemaining(int dia, int mes, int ano)
{
    time_t rawnow = time(NULL);
    struct tm now; localtime_s(&now, &rawnow);

	int month  = now.tm_mon; //0 Janeiro, 1 Fev
	int day    = now.tm_mday;
	int year   = now.tm_year;

	struct std::tm a = {0,0,0, day, month, year};
	struct std::tm b = {0,0,0, dia, mes - 1, ano-1900};

	std::time_t x = std::mktime(&a);
	std::time_t y = std::mktime(&b);

	if ( x != (std::time_t)(-1) && y != (std::time_t)(-1) )
	{
		double difference = (std::difftime(y, x) / (60 * 60 * 24));
		return static_cast<float>(difference);
	}

	return 0;
}

void BASE_GetFirstKey(const char * source, char * dest)
{
	if ((source[0] >= 'A' && source[0] <= 'Z') || (source[0] >= 'a' && source[0] <= 'z'))
	{
		dest[0] = source[0];
		dest[1] = 0;

		return;
	}

	strcpy_s(dest, 4, "etc");
}

INT32 GetFirstSlot(int clientId, int itemId)
{
	STRUCT_ITEM *item = pMob[clientId].Mobs.Player.Inventory;
	for(int i = 0; i < 30; i++)
	{
		if(item[i].sIndex == itemId)
			return i;
	}

	if(item[60].sIndex == 3467)
	{
		float remainig = TimeRemaining(item[60].EFV1, item[60].EFV2, item[60].EFV3 + 1900);
		if(remainig > 0.0f)
		{
			for(int i = 30; i < 45; i++)
			{
				if(item[i].sIndex == itemId)
					return i;
			}
		}
		else
		{
			Log(clientId, LOG_INGAME, "Bolsa do Andarilho acabou. Slot 60. %02d/%02d/%04d", item[60].EFV1, item[60].EFV2, item[60].EFV3 + 1900);
			LogPlayer(clientId, "Bolsa do andarilho acabou. Data de finalizaaao: %02d/%02d/%04d", item[60].EFV1, item[60].EFV2, item[60].EFV3 + 1900);

			memset(&item[60], 0, sizeof STRUCT_ITEM);
			
			// Atualiza o inventario
			SendItem(clientId, SlotType::Inv, 60, &item[60]);
		}
	}
	
	if(item[61].sIndex == 3467)
	{
		float remainig = TimeRemaining(item[61].EFV1, item[61].EFV2, item[61].EFV3 + 1900);
		if(remainig > 0.0f)
		{
			for(int i = 45; i < 60; i++)
			{
				if(item[i].sIndex == itemId)
					return i;
			}
		}
		else
		{
			Log(clientId, LOG_INGAME, "Bolsa do Andarilho acabou. Slot 61. %02d/%02d/%04d", item[61].EFV1, item[61].EFV2, item[61].EFV3 + 1900);
			LogPlayer(clientId, "Bolsa do andarilho acabou. Data de finalizaaao: %02d/%02d/%04d", item[61].EFV1, item[61].EFV2, item[61].EFV3 + 1900);
			memset(&item[61], 0, sizeof STRUCT_ITEM);
			
			// Atualiza o inventario
			SendItem(clientId, SlotType::Inv, 61, &item[61]);
		}
	}

	return -1;
}

void RemoveTrade(int clientId)
{
	if(clientId <= 0 || clientId >= MAX_PLAYER)
		return;

	memset(&pUser[clientId].Trade, 0, sizeof (p383));

	for(int i = 0; i < 15; i++)
		pUser[clientId].Trade.Slot[i] = -1;

	memset(&pUser[clientId].AutoTrade, 0, sizeof (pUser[0].AutoTrade));

	for(int i = 0; i < 12; i++)
		pUser[clientId].AutoTrade.Slots[i] = -1;

	if(pUser[clientId].Status != USER_PLAY)
		return;

	SendSignal(clientId, clientId, 0x384);

	if(pUser[clientId].IsAutoTrading)
	{
		int TargetX = pMob[clientId].Target.X;
		int TargetY = pMob[clientId].Target.Y;

		p364 sm;
		GetCreateMob(clientId,(BYTE*)&sm);

		GridMulticast_2(TargetX, TargetY, (BYTE*)&sm,0);

		pUser[clientId].IsAutoTrading = 0;
	}
}

void ProcessDecayItem()
{
	for(INT32 LOCAL_1 = 0; LOCAL_1 < 8; LOCAL_1 ++)
	{
		sServer.ItemCount ++;

		if(sServer.ItemCount >= 4096)
			sServer.ItemCount = (sServer.InitCount + 1);

		if(g_pInitItem[sServer.ItemCount].Item.sIndex == 1727 || (g_pInitItem[sServer.ItemCount].Item.sIndex >= 3145 && g_pInitItem[sServer.ItemCount].Item.sIndex <= 3149) || (g_pInitItem[sServer.ItemCount].Item.sIndex >= 794 && g_pInitItem[sServer.ItemCount].Item.sIndex <= 798))
			continue;

		if(g_pInitItem[sServer.ItemCount].Item.sIndex == 4922 || g_pInitItem[sServer.ItemCount].Item.sIndex == 4923 || g_pInitItem[sServer.ItemCount].Item.sIndex == 4143 || g_pInitItem[sServer.ItemCount].Item.sIndex == 471)
			continue;

		if(g_pInitItem[sServer.ItemCount].Open == 0)
			continue;

		if(g_pInitItem[sServer.ItemCount].Unknow_36 == 0)
		{
			g_pInitItem[sServer.ItemCount].Unknow_36 = 1;

			continue;
		}
			
		INT32 LOCAL_2 = g_pInitItem[sServer.ItemCount].PosX;
		INT32 LOCAL_3 = g_pInitItem[sServer.ItemCount].PosY;

		memset(&g_pInitItem[sServer.ItemCount].Item, 0, sizeof STRUCT_ITEM);

		g_pItemGrid[LOCAL_3][LOCAL_2] = 0;

		g_pInitItem[sServer.ItemCount].Open = 0;

		p16F packet;
		packet.Header.PacketId = 0x16F;
		packet.Header.Size = sizeof p16F;
		packet.Header.ClientId = 0x7530;

		packet.initID = sServer.ItemCount + 10000;

		GridMulticast_2(LOCAL_2, LOCAL_3, (BYTE*)&packet, 0);
	}
}

void SummonGuild(int arg1, int arg2, int arg3, int arg4, int arg5)
{
	INT32 LOCAL_1 = 0;
	if(arg1 <= 0)
		return;

	for(INT32 LOCAL_2 = 1; LOCAL_2 < MAX_PLAYER; LOCAL_2++)
	{
		if(pUser[LOCAL_2].Status != USER_PLAY)
			continue;

		if(pMob[LOCAL_2].Mode == 0)
			continue;

		if(pMob[LOCAL_2].Mobs.Player.Guild != arg1)
			continue;
		
		INT32 LOCAL_3 = GetVillage(pMob[LOCAL_2].Target.X, pMob[LOCAL_2].Target.Y);
		if(LOCAL_3 != arg5)
			continue;

		if(arg5 == 2 && pMob[LOCAL_2].Mobs.Player.BaseScore.Level > 100000)
		{
			SendClientMessage(LOCAL_2, g_pLanguageString[_NN_3rd_village_limit]);

			continue;
		}

		Teleportar(LOCAL_2, arg2, arg3);

		LOCAL_1 ++;
		if(LOCAL_1 == 30)
		{
			arg2 ++;
			arg3 ++;
		}

		if(LOCAL_1 == 45)
		{
			arg2 -= 2;
			arg3 -= 2;
		}

		if(LOCAL_1 >= arg4)
			break;
	}
}

void SummonGuild(int arg1, int arg2, int arg3, int arg4)
{
	INT32 LOCAL_1 = 0;
	if(arg1 <= 0)
		return;

	for(INT32 LOCAL_2 = 1; LOCAL_2 < MAX_PLAYER; LOCAL_2++)
	{
		if(pUser[LOCAL_2].Status != USER_PLAY)
			continue;

		if(pMob[LOCAL_2].Mode == 0)
			continue;

		if(pMob[LOCAL_2].Mobs.Player.Guild != arg1)
			continue;
		
		Teleportar(LOCAL_2, arg2, arg3);

		LOCAL_1 ++;
		if(LOCAL_1 == 30)
		{
			arg2 ++;
			arg3 ++;
		}

		if(LOCAL_1 == 45)
		{
			arg2 -= 2;
			arg3 -= 2;
		}

		if(LOCAL_1 >= arg4)
			break;
	}
}

void DoRanking(int arg1, int arg2, int arg3)
{
	if(arg1 == 0)
	{
		INT32 LOCAL_1 = 121;

		if(arg2 <= 0 || arg2 >= MAX_PLAYER)
			return;

		if(arg3 <= 0 || arg3 >= MAX_PLAYER)
			return;
		
		strncpy_s(sServer.Challanger.RankingName[0], 32, pMob[arg2].Mobs.Player.MobName, 16);
		strncpy_s(sServer.Challanger.RankingName[1], 32, pMob[arg3].Mobs.Player.MobName, 16);
		strncpy_s(sServer.Challanger.RankingName[2], 32, pMob[arg2].Mobs.Player.MobName, 16);
		strncpy_s(sServer.Challanger.RankingName[3], 32, pMob[arg3].Mobs.Player.MobName, 16);
		
		sServer.Challanger.RankingLevel[0] = pMob[arg2].Mobs.Player.CurrentScore.Level;
		sServer.Challanger.RankingLevel[1] = pMob[arg3].Mobs.Player.CurrentScore.Level;

		// 004617DA
		Teleportar(arg2, 147, 4045);
		SendClientMessage(arg2, g_pLanguageString[_NN_Battle_Started]);

		Teleportar(arg3, 189, 4045);
		SendClientMessage(arg3, g_pLanguageString[_NN_Battle_Started]);
		
		SendSignalParm(arg2, 0x7530, 0x3A1, LOCAL_1 + LOCAL_1 - 2);
		SendSignalParm(arg3, 0x7530, 0x3A1, LOCAL_1 + LOCAL_1 - 2);

		sServer.Challanger.RankingProcess = arg1 + 1;
		sServer.Challanger.Challanger1 = arg2;
		sServer.Challanger.Challanger2 = arg3;
		sServer.Challanger.Timer = LOCAL_1;

		return;
	}

	INT32 LOCAL_2 = 301;
	INT32 LOCAL_3 = pMob[arg2].Mobs.Player.Guild;
	INT32 LOCAL_4 = pMob[arg3].Mobs.Player.Guild;

	if(LOCAL_3 <= 0 || LOCAL_4 <= 0)
		return;
		
	sServer.Challanger.RankingProcess = arg1 + 1;
	sServer.Challanger.Challanger1 = LOCAL_3;
	sServer.Challanger.Challanger2 = LOCAL_4;
	sServer.Challanger.Timer = LOCAL_2;
	
	strncpy_s(sServer.Challanger.RankingName[0], 32, pMob[arg2].Mobs.Player.MobName, 16);
	strncpy_s(sServer.Challanger.RankingName[1], 32, pMob[arg3].Mobs.Player.MobName, 16);
	strncpy_s(sServer.Challanger.RankingName[2], 32, pMob[arg2].Mobs.Player.MobName, 16);
	strncpy_s(sServer.Challanger.RankingName[3], 32, pMob[arg3].Mobs.Player.MobName, 16);

	sServer.Challanger.RankingLevel[0] = pMob[arg2].Mobs.Player.CurrentScore.Level;
	sServer.Challanger.RankingLevel[1] = pMob[arg3].Mobs.Player.CurrentScore.Level;

	INT32 LOCAL_5 = 5;
	INT32 LOCAL_6 = 5;

	if(arg1 == 2)
	{
		LOCAL_5 = 10;
		LOCAL_6 = 10;
	}
	else if(arg1 == 3)
	{
		LOCAL_5 = MAX_PLAYER;
		LOCAL_6 = MAX_PLAYER;
	}

	if(arg1 == 3)
	{
		SummonGuild(LOCAL_3, 147, 4045, MAX_PLAYER, 3);
		SummonGuild(LOCAL_4, 189, 4045, MAX_PLAYER, 3);

		return;
	}

	for(INT32 LOCAL_7 = 1688; LOCAL_7 <= 1714; LOCAL_7++)
	{
		for(INT32 LOCAL_8 = 2564; LOCAL_8 <= 2579; LOCAL_8++)
		{
			INT32 LOCAL_9 = g_pMobGrid[LOCAL_7][LOCAL_8];
			if(LOCAL_9 <= 0 || LOCAL_9 >= MAX_PLAYER)
				continue;

			if(LOCAL_5 > 0 && pMob[LOCAL_9].Mobs.Player.Guild == LOCAL_3)
			{
				LOCAL_5 --;

				Teleportar(LOCAL_9, 147, 4045);
				SendClientMessage(LOCAL_9, g_pLanguageString[_NN_Battle_Started]);

				SendSignalParm(LOCAL_9, 0x7530, 0x3A1, (LOCAL_2 + LOCAL_2 - 2));
			}

			if(LOCAL_6 > 0 &&  pMob[LOCAL_9].Mobs.Player.Guild == LOCAL_4)
			{
				LOCAL_6 --;

				Teleportar(LOCAL_9, 189, 4045);
				SendClientMessage(LOCAL_9, g_pLanguageString[_NN_Battle_Started]);

				SendSignalParm(LOCAL_9, 0x7530, 0x3A1, (LOCAL_2 + LOCAL_2 - 2));
			}

			if(LOCAL_5 <= 0 && LOCAL_6 <= 0)
				break;
		}
	}
}

void ProcessRanking()
{
	if(sServer.Challanger.Timer > 0)
	{
		sServer.Challanger.Timer --;

		if(!(sServer.Challanger.Timer % 5))
		{
			INT32 LOCAL_1 = 0,
				  LOCAL_2 = 0;

			for(INT32 LOCAL_3 = 1; LOCAL_3 < MAX_PLAYER; LOCAL_3++)
			{
				if(pUser[LOCAL_3].Status != USER_PLAY)
					continue;

				if(pMob[LOCAL_3].Mobs.Player.CurrentScore.Hp <= 0)
					continue;

				INT32 LOCAL_4 = pMob[LOCAL_3].Target.X;
				INT32 LOCAL_5 = pMob[LOCAL_3].Target.Y;
				
				if(LOCAL_4 < 142 || LOCAL_4 > 195 || LOCAL_5 < 4007 || LOCAL_5 > 4082)
					continue;

				if(sServer.Challanger.RankingProcess == 1)
				{
					if(LOCAL_3 == sServer.Challanger.Challanger1)
						LOCAL_1++;

					if(LOCAL_3 == sServer.Challanger.Challanger2)
						LOCAL_2 ++;
				}
				else
				{
					if(pMob[LOCAL_3].Mobs.Player.Guild == sServer.Challanger.Challanger1)
						LOCAL_1 ++;

					if(pMob[LOCAL_3].Mobs.Player.Guild == sServer.Challanger.Challanger2)
						LOCAL_2 ++;
				}
			}

			if(LOCAL_1 == 0 || LOCAL_2 == 0)
				sServer.Challanger.Timer = 0;
		}

		if(sServer.Challanger.Timer < 60)
		{
			SendDamage(0x8E, 0xFA7, 0xC3, 0xFCA);
			SendDamage(0x8E, 0xFCE, 0xC3, 0xFF2);

			SendEnvEffect(0x8E, 0xFA7, 0xA8, 0xFB2, 0x20, 0);
			SendEnvEffect(0x8E, 0xFE7, 0xA8, 0xFF2, 0x20, 0);
			SendEnvEffect(0x8E, 0xFB3, 0xA8, 0xFBE, 0x20, 0);
			SendEnvEffect(0x8E, 0xFDB, 0xA8, 0xFE6, 0x20, 0);
			SendEnvEffect(0x8E, 0xFBF, 0xA8, 0xFCA, 0x20, 0);
			SendEnvEffect(0x8E, 0xFCF, 0xA8, 0xFDA, 0x20, 0);
			SendEnvEffect(0xA8, 0xFA7, 0xC3, 0xFB2, 0x20, 0);
			SendEnvEffect(0xA8, 0xFE7, 0xC3, 0xFF2, 0x20, 0);
			SendEnvEffect(0xA8, 0xFB3, 0xC3, 0xFBE, 0x20, 0);
			SendEnvEffect(0xA8, 0xFDB, 0xC3, 0xFE6, 0x20, 0);
			SendEnvEffect(0xA8, 0xFBF, 0xC3, 0xFCA, 0x20, 0);
			SendEnvEffect(0xA8, 0xFCF, 0xC3, 0xFDA, 0x20, 0);
		}
		else if(sServer.Challanger.Timer < 120)
		{
			SendDamage(0x8E, 0xFA7, 0xC3, 0xFC2);
			SendDamage(0x8E, 0xFD7, 0xC3, 0xFF2);

			SendEnvEffect(0x8E, 0xFA7, 0xA8, 0xFB2, 0x20, 0);
			SendEnvEffect(0x8E, 0xFE7, 0xA8, 0xFF2, 0x20, 0);
			SendEnvEffect(0x8E, 0xFB3, 0xA8, 0xFBE, 0x20, 0);
			SendEnvEffect(0x8E, 0xFDB, 0xA8, 0xFE6, 0x20, 0);
			SendEnvEffect(0xA8, 0xFA7, 0xC3, 0xFB2, 0x20, 0);
			SendEnvEffect(0xA8, 0xFE7, 0xC3, 0xFF2, 0x20, 0);
			SendEnvEffect(0xA8, 0xFB3, 0xC3, 0xFBE, 0x20, 0);
			SendEnvEffect(0xA8, 0xFDB, 0xC3, 0xFE6, 0x20, 0);
		}
		else if(sServer.Challanger.Timer < 180)
		{
			SendDamage(0x8E, 0xFA7, 0xC3, 0xFB3);
			SendDamage(0x8E, 0xFE6, 0xCE, 0xFF2);

			SendEnvEffect(0x8E, 0xFA7, 0xA8, 0xFB2, 0x20, 0);
			SendEnvEffect(0x8E, 0xFE7, 0xA8, 0xFF2, 0x20, 0);
			SendEnvEffect(0xA9, 0xFA7, 0xC3, 0xFB2, 0x20, 0);
			SendEnvEffect(0xA9, 0xFE7, 0xC3, 0xFF2, 0x20, 0);
		}
		
		if(sServer.Challanger.Timer <= 0)
		{
			INT32 LOCAL_6 = 0,
				  LOCAL_7 = 0;

			for(INT32 LOCAL_8 = 1; LOCAL_8 < MAX_PLAYER; LOCAL_8 ++)
			{
				if(pUser[LOCAL_8].Status != USER_PLAY)
					continue;;

				if(pMob[LOCAL_8].Mobs.Player.CurrentScore.Hp <= 0)
					continue;
				
				INT32 LOCAL_9 = pMob[LOCAL_8].Target.X;
				INT32 LOCAL_10 = pMob[LOCAL_8].Target.Y;
				
				if(LOCAL_9 < 142 || LOCAL_9 > 195 || LOCAL_10 < 4007 || LOCAL_10 > 4082)
					continue;

				if(sServer.Challanger.RankingProcess == 1)
				{
					if(LOCAL_8 == sServer.Challanger.Challanger1)
						LOCAL_6++;

					if(LOCAL_8 == sServer.Challanger.Challanger2)
						LOCAL_7 ++;
				}
				else
				{
					if(pMob[LOCAL_8].Mobs.Player.Guild == sServer.Challanger.Challanger1)
						LOCAL_6 ++;

					if(pMob[LOCAL_8].Mobs.Player.Guild == sServer.Challanger.Challanger2)
						LOCAL_7 ++;
				}
			}
			
			sServer.Challanger.RankingName[0][15] = 0;
			sServer.Challanger.RankingName[0][14] = 0;
			sServer.Challanger.RankingName[1][15] = 0;
			sServer.Challanger.RankingName[1][14] = 0;
			sServer.Challanger.RankingName[2][15] = 0;
			sServer.Challanger.RankingName[2][14] = 0;
			sServer.Challanger.RankingName[3][15] = 0;
			sServer.Challanger.RankingName[3][14] = 0;

			char szTMP[128];
			if(LOCAL_6 == LOCAL_7)
			{
				sprintf_s(szTMP, g_pLanguageString[_SS_S_S_Draw], sServer.Challanger.RankingName[0], sServer.Challanger.RankingName[1]);
				
				SendNoticeArea(szTMP, 0xA04, 0x698, 0xA13, 0x6B2);
				SendNoticeArea(szTMP, 0x8E, 0xFA7, 0xC3, 0xFF2);
			}
			else
			{
				if(LOCAL_6 > LOCAL_7)
					sprintf_s(szTMP, g_pLanguageString[_SS_S_WinBy_S],sServer.Challanger.RankingName[0], sServer.Challanger.RankingName[1]);
				else	
					sprintf_s(szTMP, g_pLanguageString[_SS_S_WinBy_S],sServer.Challanger.RankingName[1], sServer.Challanger.RankingName[0]);

				SendNoticeArea(szTMP, 0xA04, 0x698, 0xA13, 0x6B2);
				SendNoticeArea(szTMP, 0x8E, 0xFA7, 0xC3, 0xFF2);
			}

			sServer.Challanger.RankingProcess = 0;
			sServer.Challanger.Timer = 0;
			sServer.Challanger.Challanger1 = 0;
			sServer.Challanger.Challanger2 = 0;
			sServer.Challanger.RankingName[0][0] = 0;
			sServer.Challanger.RankingName[1][0] = 0;

			ClearAreaTeleport(0x8E, 0xFA7, 0xC3, 0xFF2, 0xA0C, 0x6D8);
		}
	}
}

void SetItemAmount(STRUCT_ITEM *item, int amount)
{
	int i = 0;
	for(; i < 3; i++)
	{
		if(item->stEffect[i].cEffect == EF_AMOUNT)
		{
			item->stEffect[i].cValue = amount;
			break;
		}
	}

	if(i == 3)
	{
		for(i = 0; i < 3; i++)
		{
			if(item->stEffect[i].cEffect == 0)
			{
				item->stEffect[i].cEffect = EF_AMOUNT;
				item->stEffect[i].cValue = amount;

				break;
			}
		}
	}
}
int CombineJeffi(int clientId, int searched, int earned)
{
	if (clientId < 0 || clientId >= MAX_PLAYER)
		return -1;

	int invAmount = GetInventoryAmount(clientId, searched);

	if (invAmount < 10)
		return -1;

	float bolsa1 = TimeRemaining(pMob[clientId].Mobs.Player.Inventory[60].EFV1, pMob[clientId].Mobs.Player.Inventory[60].EFV2, pMob[clientId].Mobs.Player.Inventory[60].EFV3 + 1900);
	float bolsa2 = TimeRemaining(pMob[clientId].Mobs.Player.Inventory[61].EFV1, pMob[clientId].Mobs.Player.Inventory[61].EFV2, pMob[clientId].Mobs.Player.Inventory[61].EFV3 + 1900);
	
	int totalPacks = 0;
	int aux = invAmount / 10; // pra cada PO/PL se usa 10 restos
	return 0;
}

void Combine(int clientId, int searched, int earned)
{
	INT32 LOCAL_1 = 0;
	if(clientId <= 0 || clientId >= MAX_PLAYER)
		return;

	INT32 LOCAL_2 = 0;
	
	LOCAL_1 = GetInventoryAmount(clientId, searched);

	if(LOCAL_1 < 10)
		return;

	INT32 LOCAL_4 = 0;
	for(LOCAL_2 = 0; LOCAL_2 < 60; LOCAL_2 ++)
	{
		INT32 LOCAL_5 = pMob[clientId].Mobs.Player.Inventory[LOCAL_2].sIndex;
		if(LOCAL_5 != searched)
			continue;

		while(pMob[clientId].Mobs.Player.Inventory[LOCAL_2].sIndex == searched)
		{
			AmountMinus(&pMob[clientId].Mobs.Player.Inventory[LOCAL_2]);
			
			LOCAL_4++;

			if(LOCAL_4 >= 10)
				break;
		}

		if(LOCAL_4 >= 10)
			break;
	}

	INT32 added = false;
	for(INT32 i = 0; i < 60; i++) 
	{
		STRUCT_ITEM *item = &pMob[clientId].Mobs.Player.Inventory[i];
		if(item->sIndex == earned)
		{
			INT32 amount = GetItemAmount(item);
			if(amount >= 120)
				continue;

			amount ++;
			int i = 0;
			for(; i < 3; i++)
			{
				if(item->stEffect[i].cEffect == EF_AMOUNT)
				{
					item->stEffect[i].cValue = amount;
					break;
				}
			}

			if(i == 3)
			{
				for(i = 0; i < 3; i++)
				{
					if(item->stEffect[i].cEffect == 0)
					{
						item->stEffect[i].cEffect = EF_AMOUNT;
						item->stEffect[i].cValue = amount;

						break;
					}
				}
			}

			if(i != 3)
			{
				SendItem(clientId, SlotType::Inv, i, &pMob[clientId].Mobs.Player.Inventory[i]);
				
				added = true;
				break;	
			}
		}
	}

	if(!added) 
	{
		STRUCT_ITEM LOCAL_7;
		memset(&LOCAL_7, 0, sizeof STRUCT_ITEM);
		LOCAL_7.sIndex = earned;

		INT32 LOCAL_8 = GetFirstSlot(clientId, 0);

		if(LOCAL_8 == -1)
		{
			INT32 LOCAL_13 = pMob[clientId].Target.X;
			INT32 LOCAL_14 = pMob[clientId].Target.Y;

			Log(clientId, LOG_COMP, "O item %s nao foi adicionado por falta de espaao [%d]", g_pItemList[earned].ItemName, earned);
		}
		else
		{
			pMob[clientId].Mobs.Player.Inventory[LOCAL_8] = LOCAL_7;

			SendItem(clientId, SlotType::Inv, LOCAL_8, &LOCAL_7);
		}
	}
}

float TimeRemaining(STRUCT_ITEMDATE date)
{
	time_t rawnow = time(NULL);
    struct tm now; localtime_s(&now, &rawnow);

	int month  = now.tm_mon; //0 Janeiro, 1 Fev
	int day    = now.tm_mday;
	int year   = now.tm_year;

	int dia_ = 0, mes_ = 0, ano_ = 0, hora_ = 0, min_ = 0, sec_;

	ano_  = date.Ano;
	dia_  = date.Dia;
	mes_  = date.Mes;

	hora_ = date.Hora;
	min_  = date.Minuto;
	sec_  = date.Segundo;

	struct std::tm a = {now.tm_sec, now.tm_min,now.tm_hour, day, month, year};
	struct std::tm b = {sec_, min_, hora_, dia_, mes_ - 1, ano_ - 1900};
	
	std::time_t x = std::mktime(&a);
	std::time_t y = std::mktime(&b);

	double timeDiv = 0;
	if ( x != (std::time_t)(-1) && y != (std::time_t)(-1) )
		timeDiv = (INT32)(std::difftime(y, x) / 8);

	return static_cast<float>(timeDiv);
}

int RandomItemEffect()
{
	INT32 rnd = Rand() % 105;
	if (rnd <= 35)
		return 0;
	else if (rnd > 35 && rnd <= 70)
		return 1;

	return 2;
}

void DoNightmare()
{
	for(INT32 i = 0 ; i < 3; i++)
	{
		// Checa se esta ligado - 0 = Desligado. 1 = 4 min de espera. 2 = em funcionamento
		if(sServer.Nightmare[i].CurrentScore == 0)
			continue;

		INT32 timer = sServer.Nightmare[i].TimeLeft - 2;

		// Checa se o tempo ja acabou e se acabou a 1a etapa
		// de preparaaao
		if(timer <= 0 && sServer.Nightmare[i].CurrentScore == 1)
		{
			for(int x = 0; x < 8; x++)
			{
				for(int y = 1 ; y < 4; y++)
				{
					int spawnId = g_pPesaGenerate[i][x][y];
					if(spawnId <= 0 || spawnId >= 8032)
						continue;

					GenerateMob(spawnId, 0, 0);
				}
			}

			for(INT32 x = 0; x < 40; x++)
			{

				INT32 memberId = sServer.Nightmare[i].Members[x];
				if(memberId <= 0 || memberId >= MAX_PLAYER)
					continue;

				if (pUser[memberId].Status != USER_PLAY)
				{
					sServer.Nightmare[i].Members[x] = 0;
					continue;
				}

				SendSignalParm(memberId, memberId, 0x3B0, 8);
				SendSignalParm(memberId, memberId, 0x3A1, 15 * 60);
			} 

			// Em processo normal de up
			sServer.Nightmare[i].CurrentScore = 2; 

			// Seta o novo tempo
			timer = 15 * 60;
		}

		if(sServer.Nightmare[i].CurrentScore == 2)
		{
			INT32 alive = 0;
			for(INT32 x = 0; x < 8; x++)
			{
				if(sServer.Nightmare[i].Alive[x])
					continue;

				int npcId = g_pPesaGenerate[i][x][0];
				if(npcId <= 0 || npcId > MAX_NPCGENERATOR)
					continue;

				if(mGener.pList[npcId].MobCount == 0)
				{
					sServer.Nightmare[i].Alive[x] = true;

					continue;
				}
					
				alive ++;

				for(int y = 1 ; y < 4; y++)
				{
					int spawnId = g_pPesaGenerate[i][x][y];
					if(spawnId <= 0 || spawnId > MAX_NPCGENERATOR)
						continue;

					GenerateMob(spawnId, 0, 0);
				}				
			}

			for(INT32 x = 0; x < 40; x++)
			{
				INT32 memberId = sServer.Nightmare[i].Members[x];
				if(memberId <= 0 || memberId >= MAX_PLAYER)
					continue;

				if(pUser[memberId].Status != USER_PLAY)
				{
					sServer.Nightmare[i].Members[x] = 0;

					continue;
				}

				SendSignalParm(memberId, memberId, 0x3B0, alive);

				if (pMob[memberId].Mobs.Player.Equip[0].EFV2 >= CELESTIAL && pMob[memberId].Mobs.Player.BaseScore.Level >= sServer.MaximumPesaLevel)
				{
					Log(memberId, LOG_INGAME, "Removido da area do pesadelo por atingir o limite de navel");

					DoRecall(memberId);
					sServer.Nightmare[i].Members[x] = 0;
					sServer.Nightmare[i].MembersName[x].clear();
				}
			} 

			sServer.Nightmare[i].NPCsLeft = alive;
		}

		if(timer <= 0)
		{
			for(int x = 1000; x < MAX_MOB; x++)
			{
				if(!pMob[x].Mode)
					continue;

				if(pMob[x].Target.X >= g_pPesaArea[i][0] && pMob[x].Target.X <= g_pPesaArea[i][2] && pMob[x].Target.Y >= g_pPesaArea[i][1] && pMob[x].Target.Y <= g_pPesaArea[i][3])
					DeleteMob(x, 1);
			}

			ClearArea(g_pPesaArea[i][0], g_pPesaArea[i][1], g_pPesaArea[i][2], g_pPesaArea[i][3]);

			sServer.Nightmare[i] = STRUCT_NIGHTMARE_INFO{};
			timer = 0;
		}
	
		sServer.Nightmare[i].TimeLeft = timer;
	}

}

void DoWater()
{
	static const INT32 waterId[3] = {3174, 778, 3183};
	
	for(INT32 i = 0; i < 3; i++)
	{
		stWater *water = sServer.pWater[i];
		for(INT32 x = 0; x < 9; x++)
		{
			INT32 timer = water[x].Time;
			if(timer == -1)
				continue;

			INT32 initial = PERGA_A;
			if(i == 0)
				initial = PERGA_N;
			else if(i == 1)
				initial = PERGA_M;

			// O tempo da agua acabou
			if(timer <= 0 && water[x].Mode != 2)
			{
				for(INT32 t = 1000; t < 30000; t++)
				{
					if(pMob[t].Target.X >= waterMaxMin[i][x][0] && pMob[t].Target.X <= waterMaxMin[i][x][2] && pMob[t].Target.Y >= waterMaxMin[i][x][1] && pMob[t].Target.Y <= waterMaxMin[i][x][3])
						MobKilled(t, t, 0, 0);
				}
				
				// Reseta a area e teleporta todos para fora da Zona Elemental da agua...
				ClearAreaTeleport(waterMaxMin[i][x][0], waterMaxMin[i][x][1], waterMaxMin[i][x][2], waterMaxMin[i][x][3], 1965, 1770);
				
				INT32 leader = water[x].Leader;
				
				if(x != 8 && leader != 0 && leader > 0 && leader < MAX_PLAYER && pUser[leader].Status == 22)
				{
					INT32 slotId = GetFirstSlot(leader, 0);
					if(slotId != -1)
					{
						memset(&pMob[leader].Mobs.Player.Inventory[slotId], 0, sizeof STRUCT_ITEM);

						pMob[leader].Mobs.Player.Inventory[slotId].sIndex = waterId[i] + x;
						SendItem(leader, SlotType::Inv, slotId, &pMob[leader].Mobs.Player.Inventory[slotId]);
					}
				}

				if(x == 8)
				{
					mGener.pList[initial + 8].MobCount = 0;
					mGener.pList[initial + 9].MobCount = 0;
					mGener.pList[initial + 10].MobCount = 0;
					mGener.pList[initial + 11].MobCount = 0;
				}
				else
					mGener.pList[initial + x].MobCount = 0;

				water[x].Time = -1;
				water[x].Mode = 0;
				continue;
			}

			if(timer <= 0 && water[x].Mode == 2)
			{
				// Reseta a area e teleporta todos para fora da Zona Elemental da agua...
				ClearAreaTeleport(waterMaxMin[i][x][0], waterMaxMin[i][x][1], waterMaxMin[i][x][2], waterMaxMin[i][x][3], 1965, 1770);

				water[x].Mode = 0;
				water[x].Leader = 0;
				water[x].Time = -1;
			}

			// Checa agora se o matou todos os mobs
			INT32 mobCount = mGener.pList[initial + x].MobCount;
			if(x == 8)
				mobCount = mGener.pList[initial + 8].MobCount + mGener.pList[initial + 9].MobCount + mGener.pList[initial + 10].MobCount + mGener.pList[initial + 11].MobCount;

			if(mobCount <= 1 && water[x].Mode == 1)
			{
				water[x].Mode = 2;
				water[x].Time = 15;
				timer = 18;

				INT32 leader = water[x].Leader;
				if(leader > 0 && leader < MAX_PLAYER && pUser[leader].Status == USER_PLAY)
				{
					for(INT32 p = 0; p < 12; p++)
					{
						INT32 party = pMob[leader].PartyList[p];
						if(party <= 0 || party >= MAX_PLAYER)
							continue;

						SendSignalParm(party, 0x7530, 0x3A1, 15);
					}
					
					INT32 slotId = GetFirstSlot(leader, 0);
					if(slotId != -1 && x != 8)
					{
						memset(&pMob[leader].Mobs.Player.Inventory[slotId], 0, sizeof STRUCT_ITEM);

						pMob[leader].Mobs.Player.Inventory[slotId].sIndex = waterId[i] + x;
						SendItem(leader, SlotType::Inv, slotId, &pMob[leader].Mobs.Player.Inventory[slotId]);
					}
				}
				
				SendSignalParm(leader, SERVER_SIDE, 0x3A1, 15);
			}
			else
			{
				INT32 leader = water[x].Leader;
				if(leader > 0 && leader < MAX_PLAYER && pUser[leader].Status == 22)
				{
					for(INT32 p = 0; p < 12; p++)
					{
						INT32 party = pMob[leader].PartyList[p];
						if(party <= 0 || party >= MAX_PLAYER)
							continue;

						SendSignalParm(party, SERVER_SIDE, 0x3B0, mobCount);
					}
					
					if(pUser[leader].Status == USER_PLAY)
						SendSignalParm(leader, SERVER_SIDE, 0x3B0, mobCount);
					else
						water[x].Leader = 0;
				}
			}

			if(water[x].Time != -1)
				water[x].Time = timer - 3;
		}
	}
}

void LogRune(int sala, int party, std::string message)
{
    if (sala < 0 || sala >= 10)
        return;

    if (party < 0 || party >= 3)
        return;

    const STRUCT_PISTA_DE_RUNAS *pista = &pPista[sala];
    for (int i = 0; i < 13; ++i)
    {
        int memberId = pista->Clients[party][i];
        if (memberId <= 0 || memberId >= MAX_PLAYER)
            continue;

        Log(memberId, LOG_INGAME, message.c_str());
    }
}

void MessageRune(int sala, int party, std::string message)
{
    if (sala < 0 || sala >= 10)
        return;

    if (party < 0 || party >= 3)
        return;

    const STRUCT_PISTA_DE_RUNAS *pista = &pPista[sala];
    for (int i = 0; i < 13; ++i)
    {
        int memberId = pista->Clients[party][i];
        if (memberId <= 0 || memberId >= MAX_PLAYER)
            continue;

        Log(memberId, LOG_INGAME, message.c_str());
    }
}

void GiveRuna(int sala, int party)
{
	STRUCT_PISTA_DE_RUNAS *pista = &pPista[sala];

	for (int iRunes = 0; iRunes < sServer.RunesPerSanc; iRunes++)
	{
		std::vector<const CUser*> receivedUsers;
		for (int i = 0; i < 13; i++)
		{
			int memberId = pista->Clients[party][i];
			if (memberId <= 0 || memberId >= MAX_PLAYER)
				continue;

			if (pMob[memberId].Target.X < g_pPistaCoords[sala][0] || pMob[memberId].Target.X > g_pPistaCoords[sala][2] || pMob[memberId].Target.Y < g_pPistaCoords[sala][1] || pMob[memberId].Target.Y > g_pPistaCoords[sala][3])
			{
				Log(memberId, LOG_INGAME, "Nao recebeu a runa pois estava fora da area. Sala %d. %ux %uy", sala, pMob[memberId].Target.X, pMob[memberId].Target.Y);
				LogPlayer(memberId, "Nao recebeu a Runa da sala %d pois estava fora da area", sala);

				continue;
			}

			if (pMob[memberId].Mobs.Player.CurrentScore.Hp <= 0)
			{
				Log(memberId, LOG_INGAME, "Nao recebeu a runa pois estava morto na area");
				LogPlayer(memberId, "Nao recebeu a Runa da sala %d pois estava fora da area", sala);

				continue;
			}

			int slotId = GetFirstSlot(memberId, 0);
			if (slotId == -1)
			{
				SendClientMessage(memberId, "Sem espaao no inventario para receber a Runa");

				Log(memberId, LOG_INGAME, "Nao recebeu runas pois estavam sem espaao");
				LogPlayer(memberId, "Nao recebeu a Runa da sala %d por nao possuir espaao no inventario", sala);
				continue;
			}

			const CUser* user = &pUser[memberId];
			auto isSameUser = [user](const CUser* receivedUser) {
				return memcmp(user->MacAddress, receivedUser->MacAddress, 8) == 0;
			};

			auto totalFound = std::count_if(std::begin(receivedUsers), std::end(receivedUsers), isSameUser);
			if (totalFound >= 2)
			{
				std::stringstream str;
				str << "Recebido runas pelas contas " << std::endl;
				for (auto receivedUser : receivedUsers)
				{
					if (isSameUser(receivedUser))
						str << receivedUser->User.Username << std::endl;
				}

				Log(user->clientId, LOG_INGAME, str.str().c_str());
				continue;
			}

			STRUCT_ITEM *item = &pMob[memberId].Mobs.Player.Inventory[slotId];

			auto runeRoom = Runes[sala];
			int itemIndex = 0;
			int tries = 0;
			while (tries++ < 30 && (itemIndex <= 0 || itemIndex >= MAX_ITEMLIST))
				itemIndex = *select_randomly(runeRoom.begin(), runeRoom.end());

			if (itemIndex <= 0 || itemIndex > 6500)
				itemIndex = runeRoom[0];

			memset(item, 0, sizeof STRUCT_ITEM);

			item->sIndex = itemIndex;

			SendItem(memberId, SlotType::Inv, slotId, item);

			Log(memberId, LOG_INGAME, "Recebeu a runa %s. Sala %d ( %hu )", g_pItemList[item->sIndex].ItemName, sala, item->sIndex);
			LogPlayer(memberId, "Recebeu a runa %s. Sala %d.", g_pItemList[item->sIndex].ItemName, sala);

			receivedUsers.push_back(user);
		}
	} 

	int leaderId = pista->Clients[party][12];
	int slotId = GetFirstSlot(leaderId, 0);

	if (pMob[leaderId].Target.X < g_pPistaCoords[sala][0] || pMob[leaderId].Target.X > g_pPistaCoords[sala][2] || pMob[leaderId].Target.Y < g_pPistaCoords[sala][1] || pMob[leaderId].Target.Y > g_pPistaCoords[sala][3])
	{
		Log(leaderId, LOG_INGAME, "Nao recebeu a Pista de Runas pois estava fora da area. Sala %d. %ux %uy", sala, pMob[leaderId].Target.X, pMob[leaderId].Target.Y);
		LogPlayer(leaderId, "Nao recebeu a Pista de Runas pois estava fora da area. Sala: %d", sala + 1);
		return;
	}

	if(sala == 6)
		return;



	if(slotId == -1)
	{
		SendClientMessage(leaderId, "!Nao foi possavel receber a Pista de Runas. Falta espaao no inventario");
		Log(leaderId, LOG_INGAME, "Nao foi possavel receber a pista de runas pois nao possuaa espaao no inventario. Sala: %d", sala);
	}
	else
	{
		memset(&pMob[leaderId].Mobs.Player.Inventory[slotId], 0, sizeof STRUCT_ITEM);

		pMob[leaderId].Mobs.Player.Inventory[slotId].sIndex = 5134;

		pMob[leaderId].Mobs.Player.Inventory[slotId].EF1 = EF_SANC;
		pMob[leaderId].Mobs.Player.Inventory[slotId].EFV1 = sala + 1;

		SendItem(leaderId, SlotType::Inv, slotId, &pMob[leaderId].Mobs.Player.Inventory[slotId]);
	}
}	

void DoRune()
{
	constexpr std::array positions =
	{
		STRUCT_POSITION { 3299, 1687 },
		STRUCT_POSITION { 3299, 1701 }
	};

	size_t positionIndex = 0;
	size_t totalReseted = 0;
	for(INT32 i = 0 ; i < MAX_ROOM; i++)
	{
		STRUCT_PISTA_DE_RUNAS *pista = &pPista[i];
		if(!pista->CurrentScore)
			continue;

		if(pista->Timer - 2 <= 0)
		{
			switch(i)
			{
				case 1: // 
				{
					//bool giveRuna = true;
					bool giveRuna[3] = { false, false, false};
					for (int x = 0; x < 3; x++)
					{
						if (mGener.pList[TORRE_ID + x].MobCount != 0)
							giveRuna[x] = true;

					}

					for (int x = 0; x < 3; x++) //Max party = 3
					{
						if (giveRuna[x])
							GiveRuna(i, x);
					}
				}
				break;
				case 3:
				{
					int morePoints[2] = {-1, -1};
					for(int i = 0 ; i < MAX_PARTYPISTA;i ++)
					{
						if(pista->inSec.Points[i] > morePoints[1])
						{
							morePoints[1] = pista->inSec.Points[i];
							morePoints[0] = i;
						}
					}

					if (morePoints[0] != -1)
						GiveRuna(i, morePoints[0]);
				}
				break;
			}

			auto minX = g_pPistaCoords[i][0];
			auto minY = g_pPistaCoords[i][1];
			auto maxX = g_pPistaCoords[i][2];
			auto maxY = g_pPistaCoords[i][3];

			// Limpa os mobs da area
			for(int x = 1000; x < 30000; x++)
			{
				if(pMob[x].Target.X >= minX && pMob[x].Target.X <= maxX && pMob[x].Target.Y >= minY && pMob[x].Target.Y <= maxY)
					DeleteMob(x, 1);
			}

			// Limpa a array de players e teleporta para o HallKefra
			for(int y = 0;y < MAX_PARTYPISTA; y++)
			{
				for(int z = 0;z<13;z++)
				{
					if(pista->Clients[y][z] == 0)
						continue;
					
					INT32 mobId = pista->Clients[y][z];
					if (pMob[mobId].Target.X >= minX && pMob[mobId].Target.X <= maxX && pMob[mobId].Target.Y >= minY && pMob[mobId].Target.Y <= maxY)
					{
						if (totalReseted != 0 && (totalReseted++ % 30) == 0)
							positionIndex++;

						if (positionIndex >= positions.size())
							positionIndex = 0;

						Teleportar(pista->Clients[y][z], positions[positionIndex].X, positions[positionIndex].Y);
					}
					else 
						Log(mobId, LOG_INGAME, "Nao retornou a area do Uxmal por nao estar dentro da area da pista. %u %u", pMob[mobId].Target.X, pMob[mobId].Target.Y);

					pista->Clients[y][z] = 0;

					totalReseted++;
				}
			}

			// Reseta por completo a area.
			std::string pistaName{ "Pista +" + std::to_string(i + 1) };
			ClearAreaTeleport(minX, minY, maxX, maxY, 3299, 1687, pistaName.c_str());

			*pista = STRUCT_PISTA_DE_RUNAS{};
		}
		else
		{
			pista->Timer -= 2;

			if(i == 2 || i == 6)
			{
				if(pista->Timer <= 600 && !pista->inSec.Born)
				{
					INT32 mobId = VALKY_ID;
					if(i == 6)
						mobId = COELHO_ID;
						
					GenerateMob(mobId, 0, 0);

					for(int y = 1000;y < 30000;y++)
					{
						if(pMob[y].GenerateID == mobId)
						{
							pista->inSec.BossID = y;

							break;
						}
					}

					pista->inSec.Born = true;

					for(int x = 0; x < 3; x++)
					{
						for(int y = 0; y < 13; y++)
						{
							int cId = pista->Clients[x][y];
							if(pUser[cId].Status != USER_PLAY)
							{
								pista->Clients[x][y] = 0;

								continue;
							}

							SendClientMessage(cId, "O Boss nasceu.");
						}
					}
				}
			}
			else if(i == 5)
			{
				for(INT32 room = 0; room < 4; room++)
				{
					for(int tele = 0; tele < g_pTeleBarlog[room][0][6];tele ++)
					{
						for(int posY = g_pTeleBarlog[room][tele][3]; posY <= g_pTeleBarlog[room][tele][1]; posY++)
						{
							for(int posX = g_pTeleBarlog[room][tele][2]; posX <= g_pTeleBarlog[room][tele][0]; posX++)
							{
								int mobId = g_pMobGrid[posY][posX];
								if(mobId == 0)
									continue;

								int party = -1;

								for(int liderId = 0; liderId < MAX_PARTYPISTA; liderId ++)
								{
									if(mobId == pista->Clients[liderId][12])
									{
										party = liderId;

										break;
									}
								}

								if(party == -1)
									continue;

								int semente = GetFirstSlot(mobId, 4032);
								if(semente == -1)
								{
									SendClientMessage(mobId, "Necessario Semente de Cristal para avanaar a sala");

									break;
								}

								int _rand = Rand() % 100;

								if(_rand <= 70)
								{
									for(int x = 0; x < 13; x++)
									{
										int memberId = pista->Clients[party][x];
										if(memberId <= 0 || memberId >= MAX_PLAYER)
											continue;

										Teleportar(memberId, g_pTeleBarlog[pista->inSec.Room[party] + 1][0][4], g_pTeleBarlog[pista->inSec.Room[party] + 1][0][5]);
									}

									Teleportar(mobId, g_pTeleBarlog[pista->inSec.Room[party] + 1][0][4], g_pTeleBarlog[pista->inSec.Room[party] + 1][0][5]);

									pista->inSec.Room[party] ++;

									if(pista->inSec.Room[party] == 4)
										GenerateMob(BARLOG, 0, 0);

									memset(&pMob[mobId].Mobs.Player.Inventory[semente], 0, sizeof STRUCT_ITEM);

									SendItem(mobId, SlotType::Inv, semente, &pMob[mobId].Mobs.Player.Inventory[semente]);

									Log(mobId, LOG_INGAME, "Teleportado BARLOG para sala %d", pista->inSec.Room[party]);
								}
								else if(_rand <= 90 && pista->inSec.Room[party] > 0)
								{
									for(int x = 0; x < 13; x++)
									{
										int memberId = pista->Clients[party][x];
										if(memberId <= 0 || memberId >= MAX_PLAYER)
											continue;

										Teleportar(memberId, g_pTeleBarlog[pista->inSec.Room[party] - 1][0][4], g_pTeleBarlog[pista->inSec.Room[party] - 1][0][5]);
									}

									Teleportar(mobId, g_pTeleBarlog[pista->inSec.Room[party] - 1][0][4], g_pTeleBarlog[pista->inSec.Room[party] - 1][0][5]);
								
									memset(&pMob[mobId].Mobs.Player.Inventory[semente], 0, sizeof STRUCT_ITEM);

									SendItem(mobId, SlotType::Inv, semente, &pMob[mobId].Mobs.Player.Inventory[semente]);

									Log(mobId, LOG_INGAME, "Teleportado BARLOG para sala %d - Voltou uma sala", pista->inSec.Room[party]);

									pista->inSec.Room[party] --;
								}
								else
								{
									memset(&pMob[mobId].Mobs.Player.Inventory[semente], 0, sizeof STRUCT_ITEM);
									SendItem(mobId, SlotType::Inv, semente, &pMob[mobId].Mobs.Player.Inventory[semente]);

									SendClientMessage(mobId, "Nada aconteceu, tente novamente...");

									Log(mobId, LOG_INGAME, "Tentou teleporte barlog. SALA %d", pista->inSec.Room[party]);
								}
							}
						}
					}
				}
			}
		}
	}
	
    time_t rawnow = time(NULL);
    struct tm now; localtime_s(&now, &rawnow);

	if(!(now.tm_min % 20))
	{
		for(INT32 i = 0 ; i < MAX_AVAIBLE_ROOM; i++)
		{
			STRUCT_PISTA_DE_RUNAS *pista = &pPista[i];
			if(pista->CurrentScore)
				continue;

			int x;
			for(x = 0; x < 3; x++)
				if(pista->Clients[x][12] > 0)
					break;

			if(x == 3)
			{
				pista->CurrentScore = false;
				pista->Timer = 0;

				continue;
			}

			for(x = 1000; x < 30000; x++)
			{
				if(pMob[x].Target.X >= g_pPistaCoords[i][0] && pMob[x].Target.X <= g_pPistaCoords[i][2] && pMob[x].Target.Y >= g_pPistaCoords[i][1] && pMob[x].Target.Y <= g_pPistaCoords[i][3])
					DeleteMob(x, 1);
			}

			std::stringstream str;
			if(i == 4)
			{
				int roomSorted = -1;
				for(int i = HELL; i < (HELL + 47); i++)
				{
					if(i == HELL_BOSS)
						continue;

					GenerateMob(i, 0, 0);
				}

				roomSorted = Rand() % 47;

				if (roomSorted >= 0 && roomSorted < 47)
				{
					pista->inSec.RoomSorted[roomSorted] = true;
					str << "Sala sorteada para ir ao boss: " << roomSorted << std::endl;
				}
				else
				{
					pista->inSec.RoomSorted[30] = true;
					str << "Sala sorteada para ir ao boss: " << 30 << std::endl;
				}
			}

			if(i != 2 && i != 4 && i != 5 && i != 6)
			{
				for(int x = 0 ; x < g_pGenerateLoops[i]; x++)
				{
					int generate = g_pGenerateIndexes[i] + x;
					if(generate <= 0 || generate >= MAX_NPCGENERATOR)
						continue;

					GenerateMob(generate, 0 ,0);
				}
			}

			for(x = 0; x < MAX_PARTYPISTA;x ++)
			{
				std::stringstream partyStr;
				partyStr << str.str();
				for(int y = 0; y < 13; y++)
				{
					int memberId = pista->Clients[x][y];
					if(memberId == 0)
						continue;

					// 3317 1674 3275 1712
					if(pMob[memberId].Target.X >= 3275 && pMob[memberId].Target.X <= 3317 && pMob[memberId].Target.Y >= 1674 && pMob[memberId].Target.Y <= 1712)
					{
						if (i == 1) // Pista + 1
						{
							// Teleporta o gp para a respectiva coordenada da torre
							// caso a torre que esteja do lado dele cair, ele se fode
							if (x == 0)
								Teleportar(memberId, 3387, 1554);
							else if (x == 1)
								Teleportar(memberId, 3418, 1577);
							else if (x == 2)
								Teleportar(memberId, 3358, 1578);
						}
						else
							Teleportar(memberId, g_pPistaCoords[i][4], g_pPistaCoords[i][5]);

						partyStr << "Grupo com " << pMob[memberId].Mobs.Player.MobName << "(" << pUser[memberId].User.Username << ") para a pista +" << i << std::endl;

						SendSignalParm(memberId, memberId, 0x3A1, 900);
						Log(memberId, LOG_INGAME, "Teleportado para pista %d - %ux %ux", i, g_pPistaCoords[i][4], g_pPistaCoords[i][5]);
					}
					else
						Log(memberId, LOG_INGAME, "Nao foi teleportado para pista pois estava fora da area de Kefra %hux %huy", pMob[memberId].Target.X, pMob[memberId].Target.Y);
				}

				for (int y = 0; y < 13; y++)
				{
					int memberId = pista->Clients[x][y];
					if (memberId == 0)
						continue;

					Log(memberId, LOG_INGAME, partyStr.str().c_str());
				}
			}

			pista->Timer = 900;
			pista->CurrentScore = true;
		}
	}
}

void LogPlayer(INT32 clientId, const char *msg, ...)
{
	return;
	if(clientId <= 0 || clientId >= MAX_PLAYER)
		return;

	FILE *pFile = NULL;
	
	time_t rawnow = time(NULL);
	struct tm now; localtime_s(&now, &rawnow);

	char first[16];
	BASE_GetFirstKey(pUser[clientId].User.Username, first);
	
	char szFileName[512];
	sprintf_s(szFileName, "..\\Logs\\Site\\%s\\%s - %02d-%02d-%02d.txt", first, pUser[clientId].User.Username, now.tm_mday, (now.tm_mon + 1), (now.tm_year - 100));
			
	// Abre o arquivo de log 
	fopen_s(&pFile, szFileName, "a+");

	if(pFile)
	{
		// Inicia a lista de argumentos
		va_list arglist;

		if(pUser[clientId].Status == USER_PLAY)
		{
			// Insere a hora no arquivo
			fprintf(pFile, "\n%02d:%02d:%02d %s : ",
					now.tm_hour, now.tm_min, now.tm_sec , pMob[clientId].Mobs.Player.MobName);
		}
		else
		{
			// Insere a hora no arquivo
			fprintf(pFile, "\n%02d:%02d:%02d : ",
					now.tm_hour, now.tm_min, now.tm_sec);
		}
		
		va_start(arglist, msg);
		vfprintf(pFile, msg, arglist);

		va_end(arglist);
		fclose(pFile);
	}
}

void Log(INT32 clientId, INT32 type, const char *msg, ...)
{
#if defined(_DEBUG)
	// Inicia a lista de argumentos
	va_list arglist;
	va_start(arglist, msg);

	printf("\n[%d-%d]:", clientId , type);
	vprintf(msg, arglist);
	printf("\n");

	va_end(arglist);
#endif
	try 
	{
		time_t rawnow = time(NULL);
		struct tm now; localtime_s(&now, &rawnow);

		char szFileName[512] = { 0 };
		if (clientId == SERVER_SIDE)
		{
			FILE *pFile = nullptr;

			std::lock_guard<std::recursive_mutex> mutex{ sServer.logMutex };

			if(type == LOG_HACK)
				sprintf_s(szFileName, "..\\Logs\\Servidor_%d\\Hack_%02d-%02d-%02d.txt", sServer.Channel, now.tm_mday, (now.tm_mon + 1), (now.tm_year - 100));
			else
				sprintf_s(szFileName, "..\\Logs\\Servidor_%d\\%02d-%02d-%02d.txt", sServer.Channel, now.tm_mday, (now.tm_mon + 1), (now.tm_year - 100));

			// Abre o arquivo de log 
			fopen_s(&pFile, szFileName, "at+");

			if (pFile)
			{
				// Inicia a lista de argumentos
				va_list arglist;

				// Insere a hora no arquivo
				fprintf(pFile, "\n%02d:%02d:%02d : ",
					now.tm_hour, now.tm_min, now.tm_sec);

				// Insere o log em si
				va_start(arglist, msg);
				vfprintf(pFile, msg, arglist);
				va_end(arglist);

				// Fecha o arquivo
				fclose(pFile);
			}

#if defined(_DEBUG)
			// Inicia a lista de argumentos
			va_list arglist;
			va_start(arglist, msg);

			printf("\n%02d:%02d:%02d : ", now.tm_hour, now.tm_min, now.tm_sec);
			vprintf(msg, arglist);
			printf("\n");

			va_end(arglist);
#endif
		}
		else if (clientId < MAX_PLAYER && clientId > 0)
		{
			if (!pUser[clientId].User.Username[0])
				return;

			std::lock_guard<std::recursive_mutex> mutex{ pUser[clientId].logMutex };
			if (type == LOG_HACK)
			{
				// Inicia a lista de argumentos
				va_list arglist;
				va_start(arglist, msg);

				std::string message;

				if (pUser[clientId].Status == USER_PLAY)
					message += "["s + pMob[clientId].Mobs.Player.MobName + "] - "s;

				message += msg;

				if (pUser[clientId].HackLog != nullptr)
					pUser[clientId].HackLog->Log(message.c_str(), arglist);

				va_end(arglist);
			}

			if (pUser[clientId].NormalLog != nullptr)
			{
				// Inicia a lista de argumentos
				va_list arglist;
				va_start(arglist, msg);

				std::string message;

				if (pUser[clientId].Status == USER_PLAY)
					message += "["s + pMob[clientId].Mobs.Player.MobName + "] - "s;

				message += msg;

				pUser[clientId].NormalLog->Log(message.c_str(), arglist);
				va_end(arglist);
			}
#if defined(_DEBUG)
			// Inicia a lista de argumentos
			va_list arglist;
			va_start(arglist, msg);

			printf("\n%02d:%02d:%02d %s : ", now.tm_hour, now.tm_min, now.tm_sec, pMob[clientId].Mobs.Player.MobName);
			vprintf(msg, arglist);
			printf("\n");

			va_end(arglist);
#endif
		}
		else
		{

		}
	}
	catch (std::exception& e)
	{
		static bool isOnError = false;

		if (!isOnError)
		{
			isOnError = true;
			::Log(SERVER_SIDE, LOG_INGAME, "Erro estranho no log. Mensagem: %s", e.what());
			isOnError = false;
		}
	}
	catch (...)
	{
		static bool isOnError = false;

		if (!isOnError)
		{
			isOnError = true;
			::Log(SERVER_SIDE, LOG_INGAME, "Erro estranho no log. Sem mensagme de erro. GetLastError() %d. errno %d", GetLastError(), errno);
			isOnError = false;
		}
	}
}

INT32 GetGuild(STRUCT_ITEM *item)
{
	return 0;
}

void DoSummon(int arg1, unsigned int arg2, unsigned int arg3)
{
	INT32 LOCAL_1 = GetEmptyMobGrid(arg1, &arg2, &arg3);
	if (!LOCAL_1)
	{
		if (arg1 < MAX_PLAYER)
			Log(arg1, LOG_INGAME, "DoSummon - Falha ao encontrar espaao vago no mapa para o usuario. Posiaao: %ux %uy", arg2, arg3);

		return;
	}

	p36C LOCAL_14;
	GetAction(arg1, arg2, arg3, &LOCAL_14);

	LOCAL_14.stEffect = 1;

	if(arg1 < MAX_PLAYER)
		pUser[arg1].AddMessage((BYTE*)&LOCAL_14, sizeof p36C);

	GridMulticast(arg1, arg2, arg3, (BYTE*)&LOCAL_14);
}

bool AddCrackError(int arg1, int point, int type)
{
	if(type!= 3 && type != 8 && type != 15)
		Log(arg1, LOG_HACK, "CrackError - Points: %d type: %d", point, type);

	pUser[arg1].CrackCount += point;

	if(pUser[arg1].CrackCount >= 30)
	{
		Log(arg1, LOG_HACK, "Desconectado - CrackCount: %d - point: %d - type: %d", pUser[arg1].CrackCount, point, type);

		pUser[arg1].CrackCount = 0;

		SendClientMessage(arg1, g_pLanguageString[_NN_Bad_Network_Packets]);

		if(pUser[arg1].Status != USER_SELCHAR)
		{
			CharLogOut(arg1);

			Log(arg1, LOG_HACK, "Enviado para a tela de personagem. CrackError atingiu o limite");
		}

		return true;
	}

	return false;
}

BOOL CheckPacket(PacketHeader *Header)
{
	if(Header == NULL)
		return false;

	// Cadigo - 1 = erro - 0 = ok
	BOOL code = TRUE;

	INT32 packetId = Header->PacketId;
	INT32 size = Header->Size;

	if(packetId == 0x20D && size != sizeof p20D)
		code = FALSE;
	else if(packetId == 0x20F && size != sizeof p20F)
		code = FALSE;
	else if(packetId == 0x36C && size != sizeof p36C)
		code = FALSE;
	else if(packetId == 0x366 && size != sizeof p36C)
		code = FALSE;
	else if(packetId == 0x368 && size != sizeof p36C)
		code = FALSE;
	else if(packetId == 0x37A && size != sizeof p37A)
		code = FALSE;
	//else if(packetId == 0x39E && (size != sizeof MSG_Attack && size != sizeof p367 && size != sizeof p39D + sizeof STRUCT_DAM))
	//	code = FALSE;
	//else if(packetId == 0x39D && (size != sizeof p39D && size != sizeof p367 && size != sizeof p39D + sizeof STRUCT_DAM))
	//	code = FALSE;
	//else if(packetId == 0x367 && (size != sizeof p39D && size != sizeof p367 && size != sizeof p39D + sizeof STRUCT_DAM))
		//code = FALSE;
	else if(packetId == 0x215 && size != sizeof PacketHeader)
		code = FALSE;
	else if(packetId == 0x333 && size != sizeof p333)
		code = FALSE;
	else if(packetId == 0x334 && size != sizeof p334)
		code = FALSE;
	else if(packetId == 0x376 && size != sizeof p376)
		code = FALSE;
	else if(packetId == 0x397 && size != sizeof p397)
		code = FALSE;
	else if(packetId == 0x398 && size != sizeof p398)
		code = FALSE;
	else if(packetId == 0x39A && size != sizeof p39A)
		code = FALSE;
	else if(packetId == 0x384 && size != 12)
		code = FALSE;
	else if(packetId == 0x291 && size != sizeof pMsgSignal)
		code = FALSE;
	else if(packetId == 0x277 && size != sizeof p277)
		code = FALSE;
	else if(packetId == 0x27B && size != sizeof p27B)
		code = FALSE;
	else if(packetId == 0x379 && size != sizeof p379)
		code = FALSE;
	else if(packetId == 0x374 && size != sizeof p374)
		code = FALSE;
	else if((packetId == 0x3A6 || packetId == 0x2C3 || packetId == 0x2D2 || packetId == 0x2D3 || packetId == 0x2C4 || packetId == 0x3BA || packetId == 0x3B5 || packetId == 0x3C0) && size != sizeof pCompor)
		code = FALSE;
	else if(packetId == 0x3D5 && size != sizeof pMsgSignal2)
		code = FALSE;
	else if(packetId == 0x28C && size != sizeof pMsgSignal)
		code = FALSE;
	else if(packetId == 0x378 && size != sizeof p378)
		code = FALSE;
	else if(packetId == 0x37F && size != sizeof p37F)
		code = FALSE;
	else if(packetId == 0x3AB && size != sizeof p3AB)
		code = FALSE;
	else if(packetId == 0x37E && size != sizeof p37E)
		code = FALSE;
	else if(packetId == 0x272 && size != sizeof p272)
		code = FALSE;
	else if(packetId == 0x2E4 && size != sizeof p2E4)
		code = FALSE;
	else if(packetId == 0x270 && size != sizeof p270)
		code = FALSE;
	else if(packetId == 0x399 && size != sizeof pMsgSignal)
		code = FALSE;
	else if(packetId == 0x383 && size != sizeof p383)
		code = FALSE;
	else if(packetId == 0x387 && size != sizeof pMsgSignal)
		code = FALSE;
	else if(packetId == 0x388 && size != sizeof pMsgSignal)
		code = FALSE;
	else if(packetId == 0x39F && size != sizeof p39F)
		code = FALSE;
	else if(packetId == 0x28B && size != sizeof p28B)
		code = FALSE;
	else if(packetId == 0x2E5 && size != sizeof p2E5)
		code = FALSE;
	else if (packetId == 0xE12 && size != sizeof pE12)
		code = FALSE;
	else if (packetId == 0x36A && size != sizeof p36A)
		code = FALSE;

	return code;
}

void DoAlly(INT32 guild, INT32 ally)
{
	if(guild <= 0 || ally < 0 || guild >= MAX_GUILD || ally >= MAX_GUILD)
		return;

	if (IsWarTime())
		return;

	INT32 allyId = g_pGuildAlly[guild]; // local 8
	if(allyId < 0 || allyId >= MAX_GUILD)
		allyId = 0;

	INT32 allyAlly = g_pGuildAlly[ally]; // LOCAL_9
	if(ally == 0)
	{
		allyId = g_pGuildAlly[guild];

		if(allyId > 0 && allyId < MAX_GUILD)
		{
			SendGuildNotice(guild, g_pLanguageString[_SS_Ally_Canceled], g_pGuild[guild].MobName.c_str() , g_pGuild[allyId].MobName.c_str());
			SendGuildNotice(allyId, g_pLanguageString[_SS_Ally_Canceled], g_pGuild[guild].MobName.c_str(), g_pGuild[allyId].MobName.c_str());
			
			g_pGuildAlly[guild] = 0;
		}
	}
	else if(allyId == 0 && ally != 0)
	{
		SendGuildNotice(guild, g_pLanguageString[_SS_Ally_Declared], g_pGuild[guild].MobName.c_str(), g_pGuild[ally].MobName.c_str());
		SendGuildNotice(ally, g_pLanguageString[_SS_Ally_Declared], g_pGuild[guild].MobName.c_str(), g_pGuild[ally].MobName.c_str());

		g_pGuildAlly[guild] = ally;
	}

	for(INT32 i = 1; i < MAX_PLAYER; i++)
	{
		if(pUser[i].Status != USER_PLAY)
			continue;

		if(pMob[i].Mobs.Player.Guild == guild)
			SendWarInfo(i, sServer.CapeWin);
	}
}

INT32 CombineTreasureMap(INT32 clientId)
{
	for(INT32 LOCAL_1 = 0; LOCAL_1 < 60; LOCAL_1++)
	{
		if(pMob[clientId].Mobs.Player.Inventory[LOCAL_1].sIndex == 788)
		{
			INT32 LOCAL_2 = LOCAL_1;
			LOCAL_2 = LOCAL_1 + 1;

			if(LOCAL_2 >= 60)
				continue;

			if(pMob[clientId].Mobs.Player.Inventory[LOCAL_2].sIndex != 789)
				continue;

			LOCAL_2 = LOCAL_1 + 9;
			if(LOCAL_2 >= 60)
				break;

			if(pMob[clientId].Mobs.Player.Inventory[LOCAL_2].sIndex != 790)
				continue;

			LOCAL_2 = LOCAL_1 + 10;
			if(LOCAL_2 >= 60)
				continue;

			if(pMob[clientId].Mobs.Player.Inventory[LOCAL_2].sIndex != 791)
				continue;

			// 00462654
			LOCAL_2 = LOCAL_1 + 18;
			if(LOCAL_2 >= 60)
				break;

			if(pMob[clientId].Mobs.Player.Inventory[LOCAL_2].sIndex != 792)
				continue;

			LOCAL_2 = LOCAL_1 + 19;
			if(LOCAL_2 >= 60)
				break;

			if(pMob[clientId].Mobs.Player.Inventory[LOCAL_2].sIndex != 793)
				continue;

			// envia pacote para DB - 004626BD
			LOCAL_2 = LOCAL_1;

			if(LOCAL_2 >= 60)
				continue;

			memset(&pMob[clientId].Mobs.Player.Inventory[LOCAL_2], 0, sizeof STRUCT_ITEM);

			SendItem(clientId, SlotType::Inv, LOCAL_2, &pMob[clientId].Mobs.Player.Inventory[LOCAL_2]);

			LOCAL_2 = LOCAL_1 + 1;
			if(LOCAL_2 >= 60)
				continue;

			memset(&pMob[clientId].Mobs.Player.Inventory[LOCAL_2], 0, sizeof STRUCT_ITEM);

			SendItem(clientId, SlotType::Inv, LOCAL_2, &pMob[clientId].Mobs.Player.Inventory[LOCAL_2]);
			
			LOCAL_2 = LOCAL_1 + 9;
			if(LOCAL_2 >= 60)
				continue;

			memset(&pMob[clientId].Mobs.Player.Inventory[LOCAL_2], 0, sizeof STRUCT_ITEM);

			SendItem(clientId, SlotType::Inv, LOCAL_2, &pMob[clientId].Mobs.Player.Inventory[LOCAL_2]);
			
			LOCAL_2 = LOCAL_1 + 10;
			if(LOCAL_2 >= 60)
				continue;

			memset(&pMob[clientId].Mobs.Player.Inventory[LOCAL_2], 0, sizeof STRUCT_ITEM);

			SendItem(clientId, SlotType::Inv, LOCAL_2, &pMob[clientId].Mobs.Player.Inventory[LOCAL_2]);
			
			LOCAL_2 = LOCAL_1 + 18;
			if(LOCAL_2 >= 60)
				continue;

			memset(&pMob[clientId].Mobs.Player.Inventory[LOCAL_2], 0, sizeof STRUCT_ITEM);

			SendItem(clientId, SlotType::Inv, LOCAL_2, &pMob[clientId].Mobs.Player.Inventory[LOCAL_2]);
			
			LOCAL_2 = LOCAL_1 + 19;
			if(LOCAL_2 >= 60)
				continue;

			memset(&pMob[clientId].Mobs.Player.Inventory[LOCAL_2], 0, sizeof STRUCT_ITEM);

			SendItem(clientId, SlotType::Inv, LOCAL_2, &pMob[clientId].Mobs.Player.Inventory[LOCAL_2]);
			
			SendClientMessage(clientId, g_pLanguageString[_NN_Treasure_Map]);
			return 1;
		}

		INT32 LOCAL_70 = 0;
		for(LOCAL_1 = 0; LOCAL_1 < 60 ;LOCAL_1++)
		{
			if(pMob[clientId].Mobs.Player.Inventory[LOCAL_1].sIndex >= 788 && pMob[clientId].Mobs.Player.Inventory[LOCAL_1].sIndex <= 793)
			{
				memset(&pMob[clientId].Mobs.Player.Inventory[LOCAL_1], 0, sizeof STRUCT_ITEM); // n tem

				pMob[clientId].Mobs.Player.Inventory[LOCAL_1].sIndex = 485;
				SendItem(clientId, SlotType::Inv, LOCAL_1, &pMob[clientId].Mobs.Player.Inventory[LOCAL_1]);

				LOCAL_70++;
			}
		}

		if(LOCAL_70 > 0)
		{
			SendClientMessage(clientId, g_pLanguageString[_NN_Congratulations]);

			return 1;
		}
		
		return 0;
	}

	return true;
}


INT32 PutItemArea(STRUCT_ITEM *item, unsigned int x1, unsigned int y1, unsigned int x2, unsigned int y2)
{
	int count = 0;
	for (int i = 0; i < MAX_PLAYER; i++)
	{
		if (pUser[i].Status != USER_PLAY)
			continue;

		if (pMob[i].Target.X >= x1 && pMob[i].Target.Y >= y1 &&
			pMob[i].Target.X <= x2 && pMob[i].Target.Y <= y2)
		{
			count++;
			PutItem(i, item);
		}
	}

	return count;
}

INT32 PutItem(INT32 clientId, STRUCT_ITEM *item)
{
	if(clientId <= 0 || clientId >= MAX_PLAYER)
		return 0;

	if(pUser[clientId].Status != USER_PLAY)
		return 0;

	INT32 slot = GetFirstSlot(clientId, 0);
	if(slot != -1)
	{
		memcpy(&pMob[clientId].Mobs.Player.Inventory[slot], item, sizeof STRUCT_ITEM);

		SendItem(clientId, SlotType::Inv, slot, item);
	}
	else
		return 0;

	return 1;
}

void CloseUser(INT32 clientId)
{
	INT32 LOCAL_1 = 0,
	      LOCAL_2 = 0;

	if(pUser[clientId].Status == USER_PLAY)
	{
		if(pMob[clientId].Target.X >= 0 && pMob[clientId].Target.X < 4096 && pMob[clientId].Target.Y >= 0 && pMob[clientId].Target.Y < 4096)
			g_pMobGrid[pMob[clientId].Target.Y][pMob[clientId].Target.X] = 0;
	}
	else
	{
		if(pMob[clientId].Target.X >= 0 && pMob[clientId].Target.X < 4096 && pMob[clientId].Target.Y >= 0 && pMob[clientId].Target.Y < 4096 && g_pMobGrid[pMob[clientId].Target.Y][pMob[clientId].Target.X] == clientId)
			g_pMobGrid[pMob[clientId].Target.Y][pMob[clientId].Target.X] = 0;
	}

	//7B3236C = 0; Provavelmente algo admin
	//7B322DC = 0; IsBillConnected

	pUser[clientId].AccessLevel				= 0;
	pUser[clientId].TokenOk					= 0;
	pUser[clientId].inGame.incorrectNumeric = 0;
	pUser[clientId].DropEvent.IsValid = false;
	pUser[clientId].DropEvent.Dropped = 0; 
	pUser[clientId].EventAutoTrade.IsValid = false;
	pUser[clientId].Arena.GroupIndex = MAXUINT32;

	// Remove o registro da quest atual
	pUser[clientId].QuestAccess = 0;

	std::fill(pUser[clientId].Repurchase.Items.begin(), pUser[clientId].Repurchase.Items.end(), STRUCT_ITEM{});
	pUser[clientId].Repurchase.LastIndex = 0;

	std::fill(pUser[clientId].Dropped.Items.begin(), pUser[clientId].Dropped.Items.end(), DroppedItem{});
	pUser[clientId].Dropped.LastIndex = 0;

	std::fill(pUser[clientId].AutoParty.Nicknames.begin(), pUser[clientId].AutoParty.Nicknames.end(), "");
	pUser[clientId].AutoParty.EnableAll = true;
	pUser[clientId].AutoParty.Password = "";
	
	sServer.Zakum.Unregister(clientId);
	std::fill(std::begin(pUser[clientId].TimeStamp.Skills), std::end(pUser[clientId].TimeStamp.Skills), std::chrono::steady_clock::time_point());

	pUser[clientId].invitedUsers.clear();

	pUser[clientId].CloseSocket();

 

	INT32 LOCAL_3 = pUser[clientId].Status;
	if(LOCAL_3 == USER_EMPTY || LOCAL_3 == USER_ACCEPT)
	{
		Log(clientId, LOG_INGAME, "Usuario desconectado com status %d", LOCAL_3);
		pUser[clientId].CloseUser_OL1();

		pUser[clientId].NormalLog.reset();
		pUser[clientId].HackLog.reset();
		pUser[clientId].User.Username[0] = '\0';
		return;
	}
	else if(LOCAL_3 == USER_PLAY || LOCAL_3 == USER_SAVING4QUIT)
	{
		Log(clientId, LOG_INGAME, "Personagem foi desconectado. Permaneceu %02d:%02d:%02d minutos online",
			(pUser[clientId].Time / 3600) % 24, (pUser[clientId].Time / 60) % 60, pUser[clientId].Time % 60);

		LogPlayer(clientId, "Personagem desconectado. Permaneceu %02d:%02d:%02d online",
			(pUser[clientId].Time / 3600) % 24, (pUser[clientId].Time / 60) % 60, pUser[clientId].Time % 60);

		LogGold(clientId);

		INT32 LOCAL_4 = pUser[clientId].Trade.ClientId;
		if(LOCAL_4 > 0 && LOCAL_4 < MAX_PLAYER && pUser[LOCAL_4].Status == USER_PLAY && pUser[LOCAL_4].Trade.ClientId == clientId)
			RemoveTrade(LOCAL_4);

		pUser[clientId].Trade.ClientId = 0;

		STRUCT_SAVECHARACTER packet;
		memset(&packet, 0, sizeof packet);

		if (clientId <= 0 || clientId >= MAX_PLAYER)
		{
			pUser[clientId].NormalLog.reset();
			pUser[clientId].HackLog.reset();
			return;
		}
		
		packet.CharSlot = pUser[clientId].inGame.CharSlot;
		if (packet.CharSlot < 0 || packet.CharSlot >= 4)
		{
			pUser[clientId].NormalLog.reset();
			pUser[clientId].HackLog.reset();
			return;
		}
		
		packet.Header.PacketId = 0x806;
		memcpy(&packet.Storage, &pUser[clientId].User.Storage.Item, sizeof STRUCT_ITEM * 128);
		memcpy(packet.SkillBar, pMob[clientId].Mobs.Player.ShortSkill, 4);
		memcpy(&packet.SkillBar[4], pMob[clientId].Mobs.SkillBar, 16);

		strncpy_s(packet.Pass, pUser[clientId].User.Block.Pass, 16);
		strncpy_s(packet.User, pUser[clientId].User.Username, 16);
		packet.Header.ClientId = clientId;
		
		packet.BanType = pUser[clientId].User.BanType;
		memcpy(&packet.Ban, &pUser[clientId].User.Ban, sizeof STRUCT_ITEMDATE);

		packet.Coin = pUser[clientId].User.Storage.Coin;
		packet.Cash = pUser[clientId].User.Cash;
		packet.Blocked = pUser[clientId].User.Block.Blocked;

		memcpy(&packet.Mob, &pMob[clientId].Mobs, sizeof STRUCT_CHARINFO);
 
		
		packet.Insignia = pUser[clientId].User.Insignias.Value;

		packet.Slot = -1;
 
		packet.Position.X = pMob[clientId].Target.X;
		packet.Position.Y = pMob[clientId].Target.Y;
		
 
		
		packet.Water.Day = pUser[clientId].User.Water.Day;
		packet.Water.Total = pUser[clientId].User.Water.Total;

		packet.Divina = pUser[clientId].User.Divina;
		packet.Sephira = pUser[clientId].User.Sephira;
 

		AddMessageDB((BYTE*)&packet, sizeof STRUCT_SAVECHARACTER);
		pUser[clientId].Status = USER_SAVING4QUIT;

		DeleteMob(clientId, 2);
		RemoveParty(clientId);
	}
	else
	{
		Log(clientId, LOG_INGAME, "Personagem foi desconectado. Permaneceu %02d:%02d:%02d minutos online",
			(pUser[clientId].Time / 3600) % 24, (pUser[clientId].Time / 60) % 60, pUser[clientId].Time % 60);

		LogPlayer(clientId, "Personagem desconectado. Permaneceu %02d:%02d:%02d online",
			(pUser[clientId].Time / 3600) % 24, (pUser[clientId].Time / 60) % 60, pUser[clientId].Time % 60);

		pMsgSignal packet;
		packet.Header.PacketId = 0x805;
		packet.Header.ClientId = clientId;

		AddMessageDB((BYTE*)&packet, 12);

		pMob[clientId].Mode = 0;
		pUser[clientId].CloseUser_OL1();
	}

	pUser[clientId].IsBanned = false;
	pUser[clientId].IsAutoTrading				= false;
	
	pUser[clientId].Socket.Error				= 0;
	pUser[clientId].AccessLevel					= 0;

	// Zera o acesso a quest
	pUser[clientId].QuestAccess					= 0;
	pUser[clientId].WolfEggEquipedTime			= 0;
	pUser[clientId].WolfEquipedTime				= 0;

	// Apaga o SND do usuario
	pUser[clientId].SNDMessage[0]				= 0;

	// Apaga a altima pessoa que o usuario conversou
	pUser[clientId].LastWhisper					= 0;
	pMob[clientId].Tab[0]						= 0;

	// Zera o tempo que o usuario esta online
	pUser[clientId].CharLoginTime				= 0;

	pUser[clientId].TimeStamp.LastReceiveTime   = 0;
	pUser[clientId].TimeStamp.LastAttack		= 0;
	pUser[clientId].TimeStamp.TimeStamp			= 0;
	pUser[clientId].inGame.incorrectNumeric		= 0;
	pUser[clientId].hashIncrement				= 0;

	pUser[clientId].Time = 0;

	memset(pUser[clientId].Keys, 0, 16);
	memset(pUser[clientId].MacAddress, 0, 8);

	pUser[clientId].WolfEggEquipedTime			= 0;
	pUser[clientId].WolfEquipedTime				= 0;
	pUser[clientId].WolfTotalTime				= 0;
	pUser[clientId].AlphaPotionRewardCounter	= 0;

	// :) (: 
	pUser[clientId].User.BanType				= 0;
 

	memset(&pUser[clientId].User.Ban, 0, sizeof STRUCT_ITEMDATE);
	
	pUser[clientId].aHack.Response = 0; // aguardando
	pUser[clientId].aHack.Question = 0;
	pUser[clientId].aHack.Next     = 0;
	pUser[clientId].aHack.Last     = 0;
	pUser[clientId].aHack.Error    = 0;

	pUser[clientId].PremierStore.CurrentScore = 0;
	pUser[clientId].PremierStore.Time   = 0;
	pUser[clientId].PremierStore.Wait   = 0;
	pUser[clientId].PremierStore.Count  = 0;
 
	pUser[clientId].User.Water.Day		= 0;
	pUser[clientId].User.Water.Total	= 0;

	pUser[clientId].MacIntegrity.IsChecked = false;
	pUser[clientId].MacIntegrity.WasWarned = false;

	pUser[clientId].User = STRUCT_ACCOUNT{};

	auto colosseum = std::find(sServer.Colosseum.clients.begin(), sServer.Colosseum.clients.end(), clientId);
	if (colosseum != sServer.Colosseum.clients.end())
		sServer.Colosseum.clients.erase(colosseum);

	 

	for (int i = 0; i < MAX_PLAYER; i++)
	{
		if (sServer.Zombie.Registered[i] == clientId)
		{
			sServer.Zombie.Registered[i] = 0;
			break;
		}
	}

	for(int i = 0; i < 3; ++i)
	{
		auto it = std::find(std::begin(sServer.Nightmare[i].Members), std::end(sServer.Nightmare[i].Members), clientId);
		if (it != std::end(sServer.Nightmare[i].Members))
		{
			*it = 0;
			Log(clientId, LOG_INGAME, "Removido do registro do Pesadelo");
		}
	}

	// Remove o usuario da pista se estiver cadastrado
	for (int ref = 0; ref < 6; ref++)
	{
		STRUCT_PISTA_DE_RUNAS *pista = &pPista[ref];

		for (int party = 0; party < MAX_PARTYPISTA; party++)
		{
			for (int member = 0; member < 13; member++)
			{
				if (pista->Clients[party][member] == clientId)
				{
					pista->Clients[party][member] = 0;

					party = MAX_PARTYPISTA;
					ref = 6;

					Log(clientId, LOG_INGAME, "O usuario foi removido da pista + %d pois foi desconectado", ref);
					break;
				}
			}
		}
	}

	pUser[clientId].NormalLog.reset();
	pUser[clientId].HackLog.reset();
}

void FinishCastleWar()
{
	Log(SERVER_SIDE, LOG_INGAME, "Guerra de Noatun finalizada. CastleState: 4");

	SetCastleDoor(1);
	ClearAreaGuild(0x40C, 0x688, 0x478, 0x6E4, ChargedGuildList[sServer.Channel - 1][4]);

	// Anteriormente o valor era 0
	// Agora seta como 4 para ser possavel o recolhimento dos impostos
	sServer.CastleState = 4;

	INT32 LOCAL_1 = 1;
	for(; LOCAL_1 < MAX_PLAYER; LOCAL_1++)
	{
		if(pUser[LOCAL_1].Status != USER_PLAY)
			continue;

		SendSignal(LOCAL_1, 0x7530, 0x3AC);
	}

	for(LOCAL_1 = 1; LOCAL_1 < MAX_PLAYER; LOCAL_1++)
	{
		if(pUser[LOCAL_1].Status != USER_PLAY)
			continue;

		SendSignalParm(LOCAL_1, 0x7530, 0x3AC, 0); // 0 = sServer.CastleState
	}

	for(LOCAL_1 = MAX_PLAYER; LOCAL_1 < MAX_MOB; LOCAL_1 ++)
	{
		if(pMob[LOCAL_1].Mode == 0)
			continue;

		if(pMob[LOCAL_1].Mobs.Player.Equip[0].sIndex == 219)
			DeleteMob(LOCAL_1, 2);
	}

	sServer.AltarId = 0;
	ClearArea(0x40C, 0x688, 0x478, 0x6E4);
	GuildZoneReport();
	
	for(INT32 i = 0; i < 4096; i++)
	{
		if(g_pInitItem[i].Item.sIndex <= 0 || g_pInitItem[i].Item.sIndex >= MAX_ITEMLIST)
			continue;

		if(g_pInitItem[i].Item.sIndex >= 3145 && g_pInitItem[i].Item.sIndex <= 3149)
		{
			g_pInitItem[i + 4].Item.sIndex = 3145 + g_pCityZone[4].win_count;
			
			g_pInitItem[i + 4].Item.EF1 = 56;
			g_pInitItem[i + 4].Item.EFV1 = ChargedGuildList[sServer.Channel - 1][4] / 257;

			g_pInitItem[i + 4].Item.EF2 = 57;
			g_pInitItem[i + 4].Item.EFV2 = ChargedGuildList[sServer.Channel - 1][4];

			g_pInitItem[i + 4].Item.EF3 = 59;
			g_pInitItem[i + 4].Item.EFV3 = Rand() % 255;

			Log(SERVER_SIDE, LOG_INGAME, "Setado o indice da torre de Noatun para %d. Index: %hu", i + 4, g_pInitItem[i + 4].Item.sIndex);
			break;
		}
	}

	// Reseta a guerra de torres : ) 
	sServer.TowerWar.Guild = 0;

	for(INT32 i = 0; i < MAX_GUILD; i++)
	{
		if(g_pGuild[i].MobName.empty())
			continue;

		SetGuildWin(i, 0);
	}
}

void DecideChallenger()
{
	for (auto cityIt = std::begin(sServer.ChallengerMoney); cityIt != std::end(sServer.ChallengerMoney); ++cityIt)
	{
		int cityIndex = std::distance(std::begin(sServer.ChallengerMoney), cityIt);
		auto& cityZone = g_pCityZone[cityIndex];
		auto& city = *cityIt;

		if (ChargedGuildList[sServer.Channel - 1][cityIndex] != 0)
		{
			auto& city = *cityIt;
			auto maxIt = std::max_element(std::begin(city), std::end(city), [](const STRUCT_CHALLENGEINFO& a, const STRUCT_CHALLENGEINFO& b) {
				return a.Value < b.Value;
			});

			if (maxIt == std::end(city))
				Log(SERVER_SIDE, LOG_INGAME, "Nao houve desafiantes na cidade de %s", szCitys[cityIndex]);
			else
			{
				const auto& guild = *maxIt;
				g_pCityZone[cityIndex].chall_index = guild.GuildId;

				Log(SERVER_SIDE, LOG_INGAME, "%s sera o desafiante da cidade %s", g_pGuild[guild.GuildId].MobName.c_str(), szCitys[cityIndex]);
			}
		}
		else
		{
			for (int i = 0; i < 2; ++i)
			{
				auto maxIt = std::max_element(std::begin(city), std::end(city), [](const STRUCT_CHALLENGEINFO& a, const STRUCT_CHALLENGEINFO& b) {
					return a.Value < b.Value;
				});

				if (maxIt == std::end(city))
				{
					if(i == 0)
						Log(SERVER_SIDE, LOG_INGAME, "Nao houve desafiantes na cidade de %s", szCitys[cityIndex]);
					else 
						Log(SERVER_SIDE, LOG_INGAME, "Nao houve um segundo desafiante na cidade de %s", szCitys[cityIndex]);

					continue;
				}

				const auto& guild = *maxIt;

				if (i == 0)
					g_pCityZone[cityIndex].chall_index = guild.GuildId;
				else
					g_pCityZone[cityIndex].chall_index_2 = guild.GuildId;

				Log(SERVER_SIDE, LOG_INGAME, "%s sera o desafiante da cidade %s (%d)", g_pGuild[guild.GuildId].MobName.c_str(), szCitys[cityIndex], i);
				city.erase(maxIt);
			}
		}
	}
}

void GuildProcess()	
{
	DoColosseum();

	time_t rawnow = time(NULL);
	struct tm now; localtime_s(&now, &rawnow);
	
	if(now.tm_wday >= SEGUNDA && now.tm_wday <= SEXTA && (now.tm_hour == 20 || now.tm_hour == 15) && sServer.NoviceChannel)
	{
		if((now.tm_min == 0 || now.tm_min == 30) && !sServer.Colosseum.inSec.closedGate)
		{
			sServer.Colosseum.Type = now.tm_min == 0 ? eColosseumType::Normal : eColosseumType::Mystic;

			sServer.Colosseum.inSec.closedGate = true;
			SetColoseumDoor(1);
			
			if(sServer.Colosseum.Type == eColosseumType::Normal)
				SendNotice("!Coliseu Normal sera iniciado em 3 minutos. Disponavel apenas para mortais.");
			else if (sServer.Colosseum.Type == eColosseumType::Mystic)
				SendNotice("!Coliseu Mastico sera iniciado em 3 minutos. Disponavel para Arch ou superior");

			SendNotice("!Localizaaao: Cidade de Arzan. 2615x 1724y");

			Log(SERVER_SIDE, LOG_INGAME, "Coliseu sera iniciado em 3 minutos");
		}
		else if((now.tm_min == 3 || now.tm_min == 33) && !sServer.Colosseum.inSec.closedWalls)
		{
			Log(SERVER_SIDE, LOG_INGAME, "Coliseu foi iniciado");
			sServer.Colosseum.inSec.closedWalls = true;

			// Seta como o 1a navel
			sServer.Colosseum.level = 1;
			
			SetColoseumDoor(3);
			SetColoseumDoor2(1);
			
			// Seta 5 minutos
			sServer.Colosseum.time = 300; 
			sServer.Colosseum.inSec.wasBorn = true;

			for(int i = 0; i < 9; i++)
				GenerateMob(COLOSSEUM_ID + 4 + i, 0, 0);

			for(int i = 1; i < MAX_PLAYER; i++)
			{
				if(pUser[i].Status != USER_PLAY)
					continue;

				if (pMob[i].Target.X >= 2604 && pMob[i].Target.X <= 2650 && pMob[i].Target.Y >= 1708 && pMob[i].Target.Y <= 1748)
				{
					if (pMob[i].Mobs.Player.Equip[0].EFV2 != MORTAL && sServer.Colosseum.Type == eColosseumType::Normal)
					{
						DoRecall(i);

						SendClientMessage(i, "Somente mortais podem participar do Coliseu");
						Log(i, LOG_INGAME, "Removido da area de Coliseu por nao ser mortal");
						continue;
					}
					
					if (pMob[i].Mobs.Player.Equip[0].EFV2 < ARCH && sServer.Colosseum.Type == eColosseumType::Mystic)
					{
						DoRecall(i);

						SendClientMessage(i, "Somente archs ou superior podem participar do Coliseu");
						Log(i, LOG_INGAME, "Removido da area de Coliseu por nao ser Arch");
						continue;
					}

					int sameComputerId = -1;
					for (const auto clientId : sServer.Colosseum.clients)
					{
						if (memcmp(&pUser[clientId].MacAddress, pUser[i].MacAddress, 8) == 0)
						{
							sameComputerId = i;
							break;
						}
					}
					
					if (sameComputerId != -1)
					{
						SendClientMessage(i, "Somente uma conta por computador");

						Log(i, LOG_INGAME, "O usuario nao foi registrado no Coliseu por ja ter uma conta registrada. Conta: %s", pUser[sameComputerId].User.Username);
						continue;
					}

					SendClientMessage(i, "Coliseu foi iniciado");
					SendSignalParm(i, SERVER_SIDE, 0x3A1, sServer.Colosseum.time);

					sServer.Colosseum.clients.push_back(i);

					Log(i, LOG_INGAME, "Registrado no coliseu. %ux %uy.", pMob[i].Target.X, pMob[i].Target.Y);
				}
			}
		}
		else if (sServer.Colosseum.level != 0)
		{
			for (int i = 1; i < MAX_PLAYER; i++)
			{
				if (pUser[i].Status != USER_PLAY || pUser[i].IsAdmin)
					continue;

				if (pMob[i].Target.X >= 2608 && pMob[i].Target.X <= 2647 && pMob[i].Target.Y >= 1708 && pMob[i].Target.Y <= 1748)
				{
					if (std::find(sServer.Colosseum.clients.cbegin(), sServer.Colosseum.clients.cend(), i) == sServer.Colosseum.clients.cend())
						DoRecall(i);
				}
			}
		}
	}

	if(sServer.WarChannel == 1 && now.tm_hour == sServer.CastleHour && now.tm_wday == DOMINGO)// && now.tm_wday == 0) // now.tm_hour == 19
	{
		if(sServer.CastleState == 0 && now.tm_min == 0)
		{
			SendNotice(g_pLanguageString[_DN_Castle_will_be_open]);

			sServer.CastleState = 1;
			for(INT32 LOCAL_13 = 1; LOCAL_13 < MAX_PLAYER; LOCAL_13++)
			{
				if(pUser[LOCAL_13].Status != USER_PLAY)
					continue;

				SendSignalParm(LOCAL_13, 0x7530, 0x3AC, sServer.CastleState);
			}

			Log(SERVER_SIDE, LOG_INGAME, "Portaes vao fechar em 5 minutos. CastleState: 1");
		}
		else if(sServer.CastleState == 1 && now.tm_min == 5)
		{
			ClearAreaGuild(0x40C, 0x688, 0x478, 0x6E4, ChargedGuildList[sServer.Channel - 1][4]);

			ClearAreaTeleport(0x0469, 0x6A9, 0x469, 0x6AD, 0x421, 0x6CE);
			ClearAreaTeleport(0x045C, 0x6A9, 0x45C, 0x6AD, 0x421, 0x6CE);
			ClearAreaTeleport(0x0446, 0x698, 0x446, 0x69C, 0x421, 0x6CE);
			ClearAreaTeleport(0x043F, 0x649, 0x43F, 0x6B1, 0x421, 0x6CE);
			ClearAreaTeleport(0x041A, 0x69A, 0x41A, 0x69A, 0x421, 0x6CE);
			ClearAreaTeleport(0x0416, 0x69A, 0x417, 0x49B, 0x421, 0x6CE);
			ClearAreaTeleport(0x0464, 0x6AC, 0x464, 0x6AC, 0x421, 0x6CE);

			SetCastleDoor(3);

			for(INT32 LOCAL_14 = 0; LOCAL_14 < 3; LOCAL_14 ++)
			{
				GenerateMob(LOCAL_14 + TORRE_NOATUN, 0, 0);

				sServer.LiveTower[LOCAL_14] = 1;
			}
			
			SendNotice(g_pLanguageString[_DN_Castle_opened]);
			sServer.CastleState = 2;
			
			Log(SERVER_SIDE, LOG_INGAME, "Guerra iniciada. CastleState: 2");
		}
		else if(sServer.CastleState == 2 && now.tm_min == 50)
		{
			SendNotice(g_pLanguageString[_DN_Castle_will_be_closed]);

			sServer.CastleState = 3;
			Log(SERVER_SIDE, LOG_INGAME, "Guerra ira acabar em 5minutos. CastleState: 3");
		}
		else if(sServer.CastleState == 3 && now.tm_min >= 55)
		{
			SendNotice(g_pLanguageString[_DN_Castle_closed]);

			FinishCastleWar();
		}

		if((sServer.CastleState == 2 || sServer.CastleState == 3) && now.tm_min < 55)
		{
			INT32 User = g_pMobGrid[1690][1046];
			
			if(User != sServer.AltarId)
			{
				if (User > 0 && User < MAX_PLAYER && pMob[User].Mobs.Player.GuildLevel == 9)
				{
					INT32 guildId = pMob[User].Mobs.Player.Guild;

					bool any = false;
					for(INT32 i = 0; i < 4; i++)
					{
						if(ChargedGuildList[sServer.Channel - 1][i] == guildId)
							any = true;
					}

					if(ChargedGuildList[sServer.Channel - 1][4] == guildId)
						SendClientMessage(User, "Voca ja a dono do Castelo");
					else if(any)
					{
						if (sServer.AltarId > 0 && sServer.AltarId < MAX_PLAYER)
							pUser[sServer.AltarId].TimerCount = 0;

						sServer.AltarId = User;
						pUser[User].TimerCount = 0;
					}
					else
						SendClientMessage(User, "Para conquistar o Castelo voca deve possuir pelo menos uma cidade");
				}
				else if(pMob[User].Mobs.Player.GuildLevel != 9 && User > 0 && User < MAX_PLAYER)
				{
					SendClientMessage(User, "Somente o lader da guilda pode conquistar o Castelo");
					sServer.AltarId = 0;
				}
				else
					sServer.AltarId = 0;
			}

			if(sServer.AltarId != 0)
			{
				if(pUser[User].TimerCount == 0)
					SendNotice(g_pLanguageString[_SN_S_is_charging_castle], pMob[User].Mobs.Player.MobName);

				p3AD LOCAL_5;
				LOCAL_5.Header.PacketId = 0x3AD;
				LOCAL_5.Header.Size = 16;
				LOCAL_5.User = User;
				LOCAL_5.Unknow = 1;

				GridMulticast_2(pMob[User].Target.X, pMob[User].Target.Y, (BYTE*)&LOCAL_5, 0);

				pUser[User].TimerCount ++;

				INT32 GuildID = pMob[User].Mobs.Player.Guild;
				if(pUser[User].TimerCount > 180)
				{
					SendNotice(g_pLanguageString[_SN_S_charge_castle], pMob[User].Mobs.Player.MobName);

					// Seta o novo lader da guild
					ChargedGuildList[sServer.Channel - 1][4] = GuildID;
					
					// Como houve uma nova pessoa conquistando, a contagem de Vitarias 
					// a zerada no ato ^^^ 
					g_pCityZone[4].win_count = 0;
					
					sServer.CapeWin = pMob[User].Mobs.Player.CapeInfo;

					FinishCastleWar();

					INT32 LOCAL_6 = 1;
					for(; LOCAL_6 < MAX_PLAYER; LOCAL_6++)
						ClearCrown(LOCAL_6); //remove a Coroa de quem estiver online do jogo
				}
			}
		}
	}

	// Ao terminar a guerra, sServer.CastleState a setado como 4
	// Quando esta setado como 4, o lader da guild dono de noatun pode efetuar
	// o recolhimento dos impostos. Caso continue 4 as 21h quer dizer que o mesmo
	// nao recolheu e sera setado automaticamente como 0
	if(sServer.WarChannel == 1 && now.tm_hour == sServer.CastleHour + 1 && now.tm_min >= 30 && now.tm_wday == DOMINGO && sServer.CastleState != 0)
	{
		sServer.CastleState = 0;

		Log(SERVER_SIDE, LOG_INGAME, "Setado CastleState = 0");
	}

	// 004585E6
	if(now.tm_hour == sServer.NewbieHour && now.tm_min == 54)
		sServer.PotionReady = 0;
	
	if(now.tm_hour == sServer.NewbieHour && now.tm_min == 55 && sServer.PotionReady == 0)
	{
		GenerateMob(22, 0, 0);

		sServer.PotionReady = 1;
	}

	INT32 LOCAL_24 = 5;
	// 0045864E
	//if(sServer.ForceWeekDay != -1)
	//	now.tm_wday = -1;

	if(sServer.WeekMode == 0)
	{
		if((now.tm_wday == DOMINGO && now.tm_hour >= sServer.WeekHour) || sServer.ForceWeekDay == 1)
		{
			SendNotice(g_pLanguageString[_NN_Guild_Battle_Notice1]);

			sServer.WeekMode = 1;
			return;
		}
	}

	// 04586A2
	if(sServer.WeekMode == 1)
	{
		if((now.tm_wday == DOMINGO && now.tm_hour >= sServer.WeekHour && now.tm_min == 3) || sServer.ForceWeekDay == 2)
		{
			ClearGuildPKZone();
			sServer.WeekMode = 2;

			INT32 LOCAL_25 = 0;
			for(; LOCAL_25 < 5; LOCAL_25 ++)
			{
				if(LOCAL_25 == 4)
					continue;

				if(ChargedGuildList[sServer.Channel - 1][LOCAL_25] == 0 && g_pCityZone[LOCAL_25].chall_index_2 == 0)
					continue;

				INT32 guildId = ChargedGuildList[sServer.Channel - 1][LOCAL_25];
				if (guildId == 0)
					guildId = g_pCityZone[LOCAL_25].chall_index_2;

				INT32 total = 0;
				for(INT32 LOCAL_2 = 1; LOCAL_2 < MAX_PLAYER; LOCAL_2++)
				{
					if(pUser[LOCAL_2].Status != USER_PLAY || pUser[LOCAL_2].IsAutoTrading)
						continue;

					if(pMob[LOCAL_2].Mode == 0)
						continue;

					if(pMob[LOCAL_2].Mobs.Player.Guild != guildId)
						continue;
		
					INT32 LOCAL_3 = GetVillage(pMob[LOCAL_2].Target.X, pMob[LOCAL_2].Target.Y);
					if (LOCAL_3 != LOCAL_25)
					{
						Log(LOCAL_2, LOG_INGAME, "Nao teleportado para guerra de %s por estar fora da cidade. Posiaao: %u %u", szCitys[LOCAL_25], pMob[LOCAL_2].Target.X, pMob[LOCAL_2].Target.Y);

						continue;
					}

					if ((LOCAL_25 == 0 || LOCAL_25 == 1) && pMob[LOCAL_2].Mobs.Player.Equip[0].EFV2 < CELESTIAL)
					{
						SendClientMessage(LOCAL_2, g_pLanguageString[_NN_3rd_village_limit]);
						Log(LOCAL_2, LOG_INGAME, "Nao teleportado para guerra de %s por nao ser celestial", szCitys[LOCAL_25]);

						continue;
					}
					if (LOCAL_25 == 2 && pMob[LOCAL_2].Mobs.Player.Equip[0].EFV2 != MORTAL)
					{
						SendClientMessage(LOCAL_2, g_pLanguageString[_NN_3rd_village_limit]);
						Log(LOCAL_2, LOG_INGAME, "Nao teleportado para guerra de %s por estar fora nao ser mortal", szCitys[LOCAL_25]);

						continue;
					}

					if (LOCAL_25 == 3 && pMob[LOCAL_2].Mobs.Player.Equip[0].EFV2 != ARCH)
					{
						SendClientMessage(LOCAL_2, g_pLanguageString[_NN_3rd_village_limit]);
						Log(LOCAL_2, LOG_INGAME, "Nao teleportado para guerra de %s por estar fora nao ser arch", szCitys[LOCAL_25]);

						continue;
					}

					pMob[LOCAL_2].Lifes = 5;

					total++;
					Teleportar(LOCAL_2, g_pCityZone[LOCAL_25].guilda_war_x, g_pCityZone[LOCAL_25].guilda_war_y);
					SendScore(LOCAL_2);

					Log(SERVER_SIDE, LOG_GUILD, "Teleportado %s (%u-%hhu) para a guerra de %s - Contagem: %d - DEFENSORA", pMob[LOCAL_2].Mobs.Player.MobName, pMob[LOCAL_2].Mobs.Player.BaseScore.Level,
						pMob[LOCAL_2].Mobs.Player.Equip[0].EFV2, szCitys[LOCAL_25], total);

					if(total >= 26)
						break;
				}
			}

			for(LOCAL_25 = 0; LOCAL_25 < 5; LOCAL_25 ++)
			{
				if(LOCAL_25 == 4)
					continue;

				if(g_pCityZone[LOCAL_25].chall_index == 0)
					continue;

				INT32 guildId = g_pCityZone[LOCAL_25].chall_index;
				INT32 total = 0;
				for(INT32 LOCAL_2 = 1; LOCAL_2 < MAX_PLAYER; LOCAL_2++)
				{
					if(pUser[LOCAL_2].Status != USER_PLAY || pUser[LOCAL_2].IsAutoTrading)
						continue;

					if(pMob[LOCAL_2].Mode == 0)
						continue;

					if(pMob[LOCAL_2].Mobs.Player.Guild != guildId)
						continue;
		
					INT32 LOCAL_3 = GetVillage(pMob[LOCAL_2].Target.X, pMob[LOCAL_2].Target.Y);
					if (LOCAL_3 != LOCAL_25)
					{
						Log(LOCAL_2, LOG_INGAME, "Nao teleportado para guerra de %s por estar fora da cidade. Posiaao: %u %u", szCitys[LOCAL_25], pMob[LOCAL_2].Target.X, pMob[LOCAL_2].Target.Y);

						continue;
					}

					if ((LOCAL_25 == 0 || LOCAL_25 == 1) && pMob[LOCAL_2].Mobs.Player.Equip[0].EFV2 < CELESTIAL)
					{
						SendClientMessage(LOCAL_2, g_pLanguageString[_NN_3rd_village_limit]);
						Log(LOCAL_2, LOG_INGAME, "Nao teleportado para guerra de %s por nao ser celestial", szCitys[LOCAL_25]);

						continue;
					}

					if (LOCAL_25 == 2 && pMob[LOCAL_2].Mobs.Player.Equip[0].EFV2 != MORTAL)
					{
						SendClientMessage(LOCAL_2, g_pLanguageString[_NN_3rd_village_limit]);
						Log(LOCAL_2, LOG_INGAME, "Nao teleportado para guerra de %s por estar fora nao ser mortal", szCitys[LOCAL_25]);

						continue;
					}

					if (LOCAL_25 == 3 && pMob[LOCAL_2].Mobs.Player.Equip[0].EFV2 != ARCH)
					{
						SendClientMessage(LOCAL_2, g_pLanguageString[_NN_3rd_village_limit]);
						Log(LOCAL_2, LOG_INGAME, "Nao teleportado para guerra de %s por estar fora nao ser arch", szCitys[LOCAL_25]);

						continue;
					}

					pMob[LOCAL_2].Lifes = 5;

					total++;
					Teleportar(LOCAL_2, g_pCityZone[LOCAL_25].guildb_war_x, g_pCityZone[LOCAL_25].guildb_war_y);

					Log(SERVER_SIDE, LOG_GUILD, "Teleportado %s (%u-%hhu) para a guerra de %s - Contagem: %d - DESAFIADOR", pMob[LOCAL_2].Mobs.Player.MobName, pMob[LOCAL_2].Mobs.Player.BaseScore.Level,
						pMob[LOCAL_2].Mobs.Player.Equip[0].EFV2, szCitys[LOCAL_25], total);

					if(total >= 26)
						break;
				}
			}

			return;
		}
	}
	else if(sServer.WeekMode == 2)
	{
		if((now.tm_wday == DOMINGO && now.tm_hour >= sServer.WeekHour && now.tm_min == 6) || sServer.ForceWeekDay == 3)
		{
			SendNotice(g_pLanguageString[_NN_Guild_Battle_Notice3]);

			sServer.WeekMode = 3;
			SetArenaDoor(1);
		}
	}
	else if(sServer.WeekMode == 3)
	{
		if((now.tm_wday == DOMINGO && now.tm_hour >= sServer.WeekHour && now.tm_min >= 30) || sServer.ForceWeekDay == 4)
		{
			SendNotice(g_pLanguageString[_NN_Guild_Battle_Notice4]);

			DecideWinner();
			ClearGuildPKZone();
			ClearChallanger();
			SaveGuildZone();
			SetArenaDoor(3);

			sServer.WeekMode = 4;
			GuildZoneReport();
		}
	}
	else if(sServer.WeekMode == 4)
	{
		if(now.tm_wday == SABADO || sServer.ForceWeekDay == 5)
		{
			SendNotice(g_pLanguageString[_NN_Guild_Battle_Notice5]);

			sServer.WeekMode = 5;
		}
	}
	else if(sServer.WeekMode == 5)
	{
		if(now.tm_wday == DOMINGO || sServer.ForceWeekDay == 0)
		{
			DecideChallenger();
			SendNotice(g_pLanguageString[_NN_Guild_Battle_Notice6]);

			sServer.WeekMode = 0;
		}
	}
}

void ClearAreaGuild(unsigned int minPosX, unsigned int minPosY, unsigned int maxPosX, unsigned int maxPosY, int guildId)
{
	for(size_t i = 1; i < MAX_PLAYER; i++)
	{
		if(pUser[i].Status != USER_PLAY)
			continue;

		if(pMob[i].Target.X < minPosX || pMob[i].Target.X > maxPosX || pMob[i].Target.Y < minPosY || pMob[i].Target.Y > maxPosY)
			continue;

		if(pMob[i].Mobs.Player.Guild == guildId && guildId == 0)
			continue;

		DoRecall(i);
	}
}

void SetCastleDoor(int mode)
{
	for(INT32 i = 0; i < 4; i++)
	{
		INT32 LOCAL_2 = i + 33;

		if(g_pInitItem[LOCAL_2].Item.sIndex <= 0 || g_pInitItem[LOCAL_2].Item.sIndex >= MAX_ITEMLIST)
			continue;

		if(g_pInitItem[LOCAL_2].CurrentScore != mode)
		{
			INT32 LOCAL_3;
			UpdateItem(LOCAL_2, mode, &LOCAL_3);

			p374 LOCAL_150;
			LOCAL_150.Header.PacketId = 0x374;
			LOCAL_150.Header.ClientId = 0x7530;
			LOCAL_150.gateId = LOCAL_2 + 10000;
			LOCAL_150.Header.Size = 20;
			LOCAL_150.status = g_pInitItem[LOCAL_2].CurrentScore;
			LOCAL_150.unknow = LOCAL_3;

			GridMulticast_2(g_pInitItem[LOCAL_2].PosX, g_pInitItem[LOCAL_2].PosY, (BYTE*)&LOCAL_150, 0);

			g_pInitItem[LOCAL_2].IsOpen = 0;
		}
	}
}

void SetColoseumDoor(int mode)
{
	for(INT32 i = 0; i < 2; i++)
	{
		INT32 LOCAL_2 = i + 13;

		if(g_pInitItem[LOCAL_2].Item.sIndex <= 0 || g_pInitItem[LOCAL_2].Item.sIndex >= MAX_ITEMLIST)
			continue;

		if(g_pInitItem[LOCAL_2].CurrentScore != mode)
		{
			INT32 LOCAL_3;
			UpdateItem(LOCAL_2, mode, &LOCAL_3);

			p374 LOCAL_150;
			LOCAL_150.Header.PacketId = 0x374;
			LOCAL_150.Header.ClientId = 0x7530;
			LOCAL_150.gateId = LOCAL_2 + 10000;
			LOCAL_150.Header.Size = 20;
			LOCAL_150.status = g_pInitItem[LOCAL_2].CurrentScore;
			LOCAL_150.unknow = (mode == 3) ? 18 : 0;

			GridMulticast_2(g_pInitItem[LOCAL_2].PosX, g_pInitItem[LOCAL_2].PosY, (BYTE*)&LOCAL_150, 0);

			g_pInitItem[LOCAL_2].IsOpen = 0;
		}
	}
}

void SetColoseumDoor2(int mode)
{
	for(INT32 i = 0; i < 5; i++)
	{
		INT32 LOCAL_2 = i + 15;

		if(g_pInitItem[LOCAL_2].Item.sIndex <= 0 || g_pInitItem[LOCAL_2].Item.sIndex >= MAX_ITEMLIST)
			continue;

		if(g_pInitItem[LOCAL_2].CurrentScore != mode)
		{
			INT32 LOCAL_3;
			UpdateItem(LOCAL_2, mode, &LOCAL_3);

			p374 LOCAL_150;
			LOCAL_150.Header.PacketId = 0x374;
			LOCAL_150.Header.ClientId = 0x7530;
			LOCAL_150.gateId = LOCAL_2 + 10000;
			LOCAL_150.Header.Size = 20;
			LOCAL_150.status = g_pInitItem[LOCAL_2].CurrentScore;
			LOCAL_150.unknow = (mode == 3) ? 18 : 0;

			GridMulticast_2(g_pInitItem[LOCAL_2].PosX, g_pInitItem[LOCAL_2].PosY, (BYTE*)&LOCAL_150, 0);

			g_pInitItem[LOCAL_2].IsOpen = 0;
		}
	}
}

void ClearAreaLevel(unsigned int minPosX, unsigned int minPosY, unsigned int maxPosX, unsigned int maxPosY, unsigned int min_level, unsigned int max_level)
{
	for(INT32 i = 1; i < MAX_PLAYER; i++)
	{
		if(pUser[i].Status != USER_PLAY)
			continue;

		if(pMob[i].Mode == 0)
			continue;

		if(pMob[i].Target.X < minPosX || pMob[i].Target.X > maxPosX || pMob[i].Target.Y < minPosY || pMob[i].Target.Y > maxPosY)
			continue;

		if(pMob[i].Mobs.Player.BaseScore.Level < min_level || pMob[i].Mobs.Player.BaseScore.Level > max_level)
			continue;

		if (pMob[i].Mobs.Player.BaseScore.Level >= 1010 || pUser[i].AccessLevel != 0)
			continue;

		DoRecall(i);
	}
}

void GenerateColoseum(int i)
{
	INT32 LOCAL_1 = Rand() & 0x80000003 + 4;

	for(INT32 i = 0; i < LOCAL_1; i++)
		GenerateMob(i, 0, 0);
}

void ClearGuildPKZone()
{
	UINT32 LOCAL_1 = 0x80,
		  LOCAL_2 = 0x80,
		  LOCAL_3 = 0x100,
		  LOCAL_4 = 0x100,
		  i = 0x1;

	for(; i < MAX_PLAYER; i++)
	{
		if(pUser[i].Status != USER_PLAY)
			continue;

		if(pMob[i].Mode == 0)
			continue;

		if(pMob[i].Target.X < LOCAL_1 || pMob[i].Target.X > LOCAL_3 || pMob[i].Target.Y < LOCAL_2 || pMob[i].Target.Y > LOCAL_4)
			continue;

		DoRecall(i);
	}
}

void SetArenaDoor(int mode)
{
	for(INT32 i = 0 ; i < 5; i++)
	{
		if(i == 4)
			continue;

		for(INT32 x = 0; x < 3; x++)
		{
			INT32 LOCAL_3 = i * 3 + x + 1;

			if(g_pInitItem[LOCAL_3].Item.sIndex <= 0 || g_pInitItem[LOCAL_3].Item.sIndex >= MAX_ITEMLIST)
				continue;

			INT32 LOCAL_4;
			UpdateItem(LOCAL_3, mode, &LOCAL_4);

			p374 LOCAL_150;
			LOCAL_150.Header.PacketId = 0x374;
			LOCAL_150.Header.ClientId = 0x7530;
			LOCAL_150.gateId = LOCAL_3 + 10000;
			LOCAL_150.Header.Size = 20;
			LOCAL_150.status = g_pInitItem[LOCAL_3].CurrentScore;
			LOCAL_150.unknow = LOCAL_4;

			GridMulticast_2(g_pInitItem[LOCAL_3].PosX, g_pInitItem[LOCAL_3].PosY, (BYTE*)&LOCAL_150, 0);

			g_pInitItem[LOCAL_3].IsOpen = 0;
		}
	}
}

void DecideWinner()
{
	for(INT32 i = 0; i < 5; i++)
	{
		if(i == 4)
			continue;

		INT32 LOCAL_2 = 0,
			  LOCAL_3 = 0,
			  LOCAL_4 = ChargedGuildList[sServer.Channel - 1][i],
			  LOCAL_5 = g_pCityZone[i].chall_index;

		if (LOCAL_4 == 0)
			LOCAL_4 = g_pCityZone[i].chall_index_2;

		if(LOCAL_5 == 0)
		{
			g_pCityZone[i].win_count++;

			if (g_pCityZone[i].win_count >= 5)
				g_pCityZone[i].win_count = 4;
			continue;
		}

		std::stringstream strOwner;
		strOwner << "[" << szCitys[i] << "] - Guild campea informaaaes:\n";

		std::stringstream strChall;
		strChall << "[" << szCitys[i] << "] - Guild desafiadora informaaaes:\n";

		for(UINT32 LOCAL_134 = g_pCityZone[i].war_min_y; LOCAL_134 < g_pCityZone[i].war_max_y; LOCAL_134++)
		{
			for(UINT32 LOCAL_135 = g_pCityZone[i].war_min_x; LOCAL_135 < g_pCityZone[i].war_max_x; LOCAL_135++)
			{
				INT32 mobId = g_pMobGrid[LOCAL_134][LOCAL_135]; // LOCAL_136
				if(mobId <= 0 || mobId >= MAX_PLAYER)
					continue;

				if(pMob[mobId].Mode == 0 || pMob[mobId].Mobs.Player.CurrentScore.Hp <= 0)
					continue;

				INT32 LOCAL_137 = pMob[mobId].Mobs.Player.Guild;
				UINT32 LOCAL_138 = pMob[mobId].Mobs.Player.CurrentScore.Level + 1;

				if (pMob[mobId].Mobs.Player.Equip[0].EFV2 >= CELESTIAL)
					LOCAL_138 += 400;

				if(LOCAL_137 == 0)
					continue;

				int lifesMultiplier = 1;
				if (pMob[mobId].Lifes > 0)
				{
					switch (pMob[mobId].Lifes)
					{
					case 5:
						lifesMultiplier = 10;
						break;
					case 4:
						lifesMultiplier = 8;
						break;
					case 3:
						lifesMultiplier = 5;
						break;
					case 2:
						lifesMultiplier = 3;
						break;
					case 1:
						break;
					}
				}

				int totalPoints = LOCAL_138 * lifesMultiplier;
				if (LOCAL_137 == LOCAL_4)
				{
					LOCAL_2 += LOCAL_138;
					strOwner << "Player " << pMob[mobId].Mobs.Player.MobName << " com " << pMob[mobId].Lifes << " totalizou " << totalPoints << " pontos. Total: " << LOCAL_2 << "\n";
				}
				else if (LOCAL_137 == LOCAL_5)
				{
					LOCAL_3 += LOCAL_138;
					strChall << "Player " << pMob[mobId].Mobs.Player.MobName << " com " << pMob[mobId].Lifes << " totalizou " << totalPoints << " pontos. Total: " << LOCAL_3 << "\n";
				}
			}
		}

		if(LOCAL_2 >= LOCAL_3)
		{
			SendGuildNotice(LOCAL_4, g_pLanguageString[_SSNN_GuildWarResult], g_pGuild[LOCAL_4].MobName.c_str(), g_pGuild[LOCAL_5].MobName.c_str(), LOCAL_2, LOCAL_3);
			SendGuildNotice(LOCAL_5, g_pLanguageString[_SSNN_GuildWarResult], g_pGuild[LOCAL_4].MobName.c_str(), g_pGuild[LOCAL_5].MobName.c_str(), LOCAL_2, LOCAL_3);
			
			if (ChargedGuildList[sServer.Channel - 1][i] == 0)
				g_pCityZone[i].win_count = -1;

			g_pCityZone[i].win_count ++;
			if(g_pCityZone[i].win_count >= 5)
				g_pCityZone[i].win_count = 4;

			ChargedGuildList[sServer.Channel - 1][i] = LOCAL_4;
		}
		else 
		{
			SendGuildNotice(LOCAL_4, g_pLanguageString[_SSNN_GuildWarResult], g_pGuild[LOCAL_5].MobName.c_str(), g_pGuild[LOCAL_4].MobName.c_str(), LOCAL_3, LOCAL_2);
			SendGuildNotice(LOCAL_5, g_pLanguageString[_SSNN_GuildWarResult], g_pGuild[LOCAL_5].MobName.c_str(), g_pGuild[LOCAL_4].MobName.c_str(), LOCAL_3, LOCAL_2);
			
			ChargedGuildList[sServer.Channel - 1][i] = LOCAL_5;

			g_pCityZone[i].win_count = 0;			
		}

		Log(SERVER_SIDE, LOG_INGAME, strOwner.str().c_str());
		Log(SERVER_SIDE, LOG_INGAME, strChall.str().c_str());

		Log(SERVER_SIDE, LOG_INGAME, "Guerra cidade %d terminada. %s %d %s %d", i, g_pGuild[LOCAL_4].MobName.c_str(), LOCAL_2, g_pGuild[LOCAL_5].MobName.c_str(), LOCAL_3);

		g_pCityZone[i].chall_index = 0;
		g_pCityZone[i].chall_index_2= 0;
	}
		
	INT32 castle = ChargedGuildList[sServer.Channel - 1][4];

	// Checa se ha algum dono de castelo
	if(castle != 0)
	{
		// Caso tenha um dono, checa se possui peolo menos uma cidade
		bool has = false;
		for(INT32 i = 0; i < 4; i++)
		{
			// Caso encontre uma cidade, a variavel HAS a setada como true
			if(ChargedGuildList[sServer.Channel - 1][i] == castle)
				has = true;
		}

		// CAso ainda esteja false, quer dizer que nenhuma cidade foi encontrada
		// Ou seja, Noatun sera removida
		if(!has)
		{
			ChargedGuildList[sServer.Channel - 1][4] = 0;
			g_pCityZone[4].win_count   = 0;
			castle = 0;

			Log(SERVER_SIDE, LOG_GUILD, "Rei do Castelo perdeu o Castelo por nao possuir nenhuma cidade");
		}
	}

	// Seta como true que tem todas as cidades
	bool hasAll = true;
	for(INT32 i = 1; i < 4; i++)
	{
		// Checa se o dono da cidade em questao a igual ao dono
		// da cidade de armia
		// Caos nao seja, a variavel a setada como false e nao sera entregue
		if(ChargedGuildList[sServer.Channel - 1][i] != ChargedGuildList[sServer.Channel - 1][0])
			hasAll = false;
	}

	// Caso a guilda seja dono de todas as cidades E o dono do castelo seja diferente do
	if(hasAll)
	{
		bool newOwner = ChargedGuildList[sServer.Channel - 1][4] != ChargedGuildList[sServer.Channel - 1][0];
		ChargedGuildList[sServer.Channel - 1][4] = ChargedGuildList[sServer.Channel - 1][0];

		if(newOwner)
			g_pCityZone[4].win_count = 0;

		Log(SERVER_SIDE, LOG_GUILD, "Entregue o Castelo para %d por possuir todas as cidades", ChargedGuildList[sServer.Channel - 1][4]);
	}

	UpdateCityTowers();
	SaveGuildZone();
}

void UpdateCityTowers()
{
	std::stringstream str;
	for (INT32 i = 0; i < 4096; i++)
	{
		if (g_pInitItem[i].Item.sIndex <= 0 || g_pInitItem[i].Item.sIndex >= MAX_ITEMLIST)
			continue;

		if (g_pInitItem[i].Item.sIndex >= 3145 && g_pInitItem[i].Item.sIndex <= 3149)
		{
			str << "Encontrado primeira torre em " << i << "\n";
			for (INT32 t = 0; t < 5; t++)
			{
				g_pInitItem[i + t].Item.sIndex = 3145 + g_pCityZone[t].win_count;

				g_pInitItem[i + t].Item.EF1 = 56;
				g_pInitItem[i + t].Item.EFV1 = ChargedGuildList[sServer.Channel - 1][t] / 257;

				g_pInitItem[i + t].Item.EF2 = 57;
				g_pInitItem[i + t].Item.EFV2 = ChargedGuildList[sServer.Channel - 1][t];

				g_pInitItem[i + t].Item.EF3 = 59;
				g_pInitItem[i + t].Item.EFV3 = Rand() % 255;

				str << "Torre com id " << (i + t) << " alterado para index " << g_pInitItem[i + t].Item.sIndex << "\n";
			}

			break;
		}
	}

	Log(SERVER_SIDE, LOG_INGAME, str.str().c_str());
}

void ClearChallanger()
{
	for(INT32 i = 0; i < 5; i++)
	{
		g_pCityZone[i].chall_index = 0;
		g_pCityZone[i].chall_index_2 = 0;

		for (auto& challenger : sServer.ChallengerMoney)
			challenger.clear();
	}
}

INT32 Challange(INT32 clientId, INT32 mobId, INT32 value)
{
	if(mobId < MAX_PLAYER || mobId >= MAX_MOB)
		return 0;

	INT32 cityId = pMob[mobId].Mobs.Player.BaseScore.Level,
		  guildId = pMob[clientId].Mobs.Player.Guild;

	if(cityId < 0 || cityId > 5)
		return 0;

	if(cityId == 4)
		return 0;

	if(pMob[clientId].Mobs.Player.GuildLevel != 9)
	{
		SendSay(mobId, g_pLanguageString[_NN_Only_Guild_Master_can]);

		return 0;
	}

	INT32 wins = g_pGuild[guildId].Wins;
	if(wins == 0)
	{
		SendSay(mobId, "Para apostar voca deve ter conquistado a Torre de Erion uma vez pelo menos");

		return 0;
	}

	if (g_pGuild[guildId].Fame < value || g_pGuild[guildId].Fame < 100)
	{
		SendSay(mobId, g_pLanguageString[_NN_Havent_Money_So_Much]);

		return 0;
	}
	
	for (int serverId = 0; serverId < 10; ++serverId)
	{
		if (serverId == sServer.Channel - 1)
			continue;

		for (int cityId = 0; cityId < 5; cityId++)
		{
			if (ChargedGuildList[serverId][cityId] == guildId)
			{
				SendSay(mobId, "Nao a possavel apostar enquanto possuir cidade em outro servidor");

				return 0;
			}
		}
	}

	if(value < 100)
	{
		SendSay(mobId, g_pLanguageString[_NN_Need_1000000_For_Challange]);

		return 0;
	}

	if(ChargedGuildList[sServer.Channel - 1][cityId] == guildId)
	{
		SendSay(mobId, g_pLanguageString[_NN_Champions_Cant_Challange]);

		return 0;
	}

	INT32 success = 1,
		  LOCAL_4 = sServer.ServerGroup,
		  LOCAL_5 = guildId >> 12,
		  LOCAL_6 = guildId % 0xFFF;

	if(LOCAL_6 < 0 || LOCAL_4 >= 10 || LOCAL_5 < 0 || LOCAL_5 >= 16 || LOCAL_6 < 0 || LOCAL_6 > 4096)
		success = 0;
	else if(!g_pGuild[guildId].MobName[0])
		success = 0;

	if(success == 0)
	{
		SendSay(mobId, g_pLanguageString[_NN_Only_Named_Guild]);

		return 0;
	}

	value = g_pGuild[guildId].Fame;
	SetGuildFame(guildId, value - 100);

	SetGuildWin(guildId, wins - 1);
	g_pGuild[guildId].Wins--;

	SendClientMessage(clientId, "Aposta realizada.");

	Log(SERVER_SIDE, LOG_INGAME, "Guild %s realizou aposta na cidade %d. Valor: %d", g_pGuild[guildId].MobName.c_str(), cityId, value);

	SendEtc(clientId);

	auto& cityChallenger = sServer.ChallengerMoney[cityId];
	auto guildIt = std::find_if(std::begin(cityChallenger), std::end(cityChallenger), [guildId](const STRUCT_CHALLENGEINFO& info) {
		return info.GuildId == guildId;
	});

	STRUCT_CHALLENGEINFO* guild = nullptr;
	if (guildIt == std::end(cityChallenger))
		guild = &cityChallenger.emplace_back();
	else
		guild = &*guildIt;

	if (guild == nullptr)
	{
		Log(SERVER_SIDE, LOG_INGAME, "Falha ao encontrar ponteiro de guild. GuildId: %d", guildId);

		return 0;
	}

	guild->GuildId = guildId;
	guild->Value = value;

	return 1;
}

void DoWar(int arg1, int arg2)
{
	INT32 LOCAL_1 = 65535;
	if(arg1 <= 0 || arg2 < 0 ||arg1 >= LOCAL_1 || arg2 >= LOCAL_1)
		return;

	INT32 LOCAL_8 = g_pGuildWar[arg1];
	if(LOCAL_8 < 0 || LOCAL_8 >= LOCAL_1)
		LOCAL_8 = 0;

	INT32 LOCAL_9 = g_pGuildWar[arg2];
	if(arg2 == 0)
	{
		if(LOCAL_8 <= 0 || LOCAL_8 >= LOCAL_1)
			return;

		LOCAL_9 = g_pGuildWar[LOCAL_8];
		if(LOCAL_9 == arg1)
		{
			g_pGuildWar[LOCAL_8] = 0;
			g_pGuildWar[arg1] = 0;

			for(INT32 LOCAL_10 = 1; LOCAL_10 < MAX_PLAYER; LOCAL_10++)
			{
				if(pUser[LOCAL_10].Status != USER_PLAY)
					continue;

				if(pMob[LOCAL_10].Mobs.Player.Guild != arg1 && pMob[LOCAL_10].Mobs.Player.Guild == LOCAL_8)
					continue;

				SendWarInfo(LOCAL_10, sServer.CapeWin);
			}
		}
		else
		{
			SendGuildNotice(arg1, g_pLanguageString[_SS_War_declare_canceled], g_pGuild[arg1].MobName.c_str(), g_pGuild[arg2].MobName.c_str());

			g_pGuildWar[arg1] = 0;
		}

		SendGuildNotice(arg1, g_pLanguageString[_SS_War_Canceled], g_pGuild[arg1].MobName.c_str(), g_pGuild[arg2].MobName.c_str());
		SendGuildNotice(LOCAL_8, g_pLanguageString[_SS_War_Canceled], g_pGuild[arg1].MobName.c_str(), g_pGuild[arg2].MobName.c_str());
	}
	else if(LOCAL_8 == 0 && arg2 != 0 && LOCAL_9 != arg1)
	{
		SendGuildNotice(arg1, g_pLanguageString[_SS_War_Declared], g_pGuild[arg1].MobName.c_str(), g_pGuild[arg2].MobName.c_str());
		SendGuildNotice(arg2, g_pLanguageString[_SS_War_Declared], g_pGuild[arg1].MobName.c_str(), g_pGuild[arg2].MobName.c_str());

		g_pGuildWar[arg1] = arg2;
	}
	else if(LOCAL_8 == 0 && arg2 != 0 && LOCAL_9 == arg1)
	{
		SendNotice(g_pLanguageString[_SS_War_Started], g_pGuild[arg1].MobName.c_str(), g_pGuild[arg2].MobName.c_str());

		g_pGuildWar[arg1] = arg2;

		for(INT32 LOCAL_11 = 1; LOCAL_11 < MAX_PLAYER; LOCAL_11++)
		{
			if(pUser[LOCAL_11].Status != USER_PLAY)
				continue;
			
			if(pMob[LOCAL_11].Mobs.Player.Guild != arg1 && pMob[LOCAL_11].Mobs.Player.Guild == arg2)
				continue;

			SendWarInfo(LOCAL_11, sServer.CapeWin);
		}
	}
}

void SetGuildFame(INT32 guildId, INT32 fame)
{
	MSG_ADDGUILD packet;
	memset(&packet, 0, sizeof packet);

	packet.Header.PacketId = MSG_ADDGUILD_OPCODE;
	packet.Header.Size = sizeof packet;

	packet.Type = 0;
	packet.Value = fame;
	packet.guildIndex = guildId;

	AddMessageDB((BYTE*)&packet, sizeof packet);
}

void SetGuildWin(INT32 guildId, INT32 win)
{
	MSG_ADDGUILD packet;
	memset(&packet, 0, sizeof packet);

	packet.Header.PacketId = MSG_ADDGUILD_OPCODE;
	packet.Header.Size = sizeof packet;

	packet.Type = 2;
	packet.Value = win;
	packet.guildIndex = guildId;

	AddMessageDB((BYTE*)&packet, sizeof packet);
}

void SaveUser(INT32 clientId, INT32 arg2)
{
	STRUCT_SAVECHARACTER packet{};
	packet.Header.PacketId = 0x807;
	packet.Header.Size = sizeof(STRUCT_SAVECHARACTER);

	packet.CharSlot = pUser[clientId].inGame.CharSlot;
	packet.Mob = pMob[clientId].Mobs;

	memcpy(packet.Storage, pUser[clientId].User.Storage.Item, 1024);

	packet.Coin = pUser[clientId].User.Storage.Coin;
	packet.Header.ClientId = clientId;
	packet.Arg2 = arg2;

	strncpy_s(packet.User, pUser[clientId].User.Username, 16);
	strncpy_s(packet.Pass, pUser[clientId].User.Block.Pass, 16);

	packet.Blocked = pUser[clientId].User.Block.Blocked;

	memcpy(packet.SkillBar, pMob[clientId].Mobs.Player.ShortSkill, 4);
	memcpy(&packet.SkillBar[4], pMob[clientId].Mobs.SkillBar, 16);

	packet.Cash = pUser[clientId].User.Cash;

	packet.BanType = pUser[clientId].User.BanType;
	memcpy(&packet.Ban, &pUser[clientId].User.Ban, sizeof STRUCT_ITEMDATE);

 
	packet.Insignia = pUser[clientId].User.Insignias.Value;

	packet.Position.X = pMob[clientId].Target.X;
	packet.Position.Y = pMob[clientId].Target.Y;

 
	packet.Slot = pUser[clientId].User.CharSlot;

 

	packet.Water.Day = pUser[clientId].User.Water.Day;
	packet.Water.Total = pUser[clientId].User.Water.Total;

	packet.Divina = pUser[clientId].User.Divina;
	packet.Sephira = pUser[clientId].User.Sephira;
 

	AddMessageDB((BYTE*)&packet, sizeof STRUCT_SAVECHARACTER);
}

void RemoveDefaultADD(STRUCT_ITEM *item, int index)
{
	__try
	{
		if(g_pItemList[item->sIndex].Pos >= 64)
			return;

		for(int i = 0; i < 3; i++)
		{
			switch(item->stEffect[i].cEffect)
			{
				case 43:
				case 0:
				case 116:
				case 117:
				case 118:	
				case 119:
				case 120:
				case 121:
				case 122:
				case 123:
				case 124:
				case 125:
					continue;
			}

			int value = GetEffectValueByIndex(item->sIndex, item->stEffect[i].cEffect); // 45
			int originalValue = GetEffectValueByIndex(index, item->stEffect[i].cEffect); // 35

			item->stEffect[i].cValue = (item->stEffect[i].cValue + originalValue) - value;
		}
	}
	__except(1)
	{
	}
}

void FormatIntToTime(int time, char *string)
{
	sprintf_s(string, 16, "%02d:%02d:%02d", time / 3600, (time / 60) % 60, time % 60);
}

INT32 ReadNPCQuest()
{
	// Busca por arquivos no diretorio dir
	HANDLE handle;
	WIN32_FIND_DATA win32_find_data;
	handle = FindFirstFile("Quests\\*.c", &win32_find_data);

	if(handle == INVALID_HANDLE_VALUE)
		return false;

	// Limpa a array
	memset(questNPC, 0, sizeof questNPC);

	while(true)
	{
		char *fileName = win32_find_data.cFileName;
		if(fileName[0] == '.')
		{
			if(FindNextFile(handle, &win32_find_data))
				break;

			continue;
		}

		char tmp[256];
		sprintf_s(tmp, "Quests\\%s", fileName);
		printf("Lendo arquivo: %s\n", fileName);

		FILE *pFile = nullptr;
		fopen_s(&pFile, tmp, "r");

		if(!pFile)
		{
			if(!FindNextFile(handle, &win32_find_data))
				break;

			continue;
		}

		while(fgets(tmp, 256, pFile))
		{
			char cmd1[96],
				 cmd2[96],
				 cmd3[96],
				 cmd4[96],
				 cmd5[96],
				 cmd6[96];

			INT32 ret = sscanf_s(tmp, "%s %s %s %s %s %s", cmd1, 96, cmd2, 96, cmd3, 96, cmd4, 96, cmd5, 96, cmd6, 96);

			INT32 questId = -1;

			if(sscanf_s(cmd1, "QUEST%d", &questId))
			{
				if(questId < 0 || questId >= MAX_NPCQUEST)
					questId = -1;
			}

			if(questId == -1)
				continue;

			STRUCT_NPCQUEST_CFILE *npc = &questNPC[questId];
			if(!_strnicmp(cmd2, "TAB", 3))
				strncpy_s(npc->Tab, cmd3, 28);
			else if(!_strnicmp(cmd2, "CONDITION", 9))
			{
				INT32 condition = -1;

				sscanf_s(cmd2, "CONDITION-%d", &condition);
				if(condition < 0 || condition >= MAX_NPCQUEST_CONDITION)
					continue;
				
				if(!_strnicmp(cmd3, "SPEECH", 6) && sscanf_s(cmd4, "%s", npc->Condition[condition].Speech, 96))
					continue;
				else if(!_strnicmp(cmd3, "LEVEL", 5) && sscanf_s(cmd4, "%d", &npc->Condition[condition].minLevel) && sscanf_s(cmd5, "%d", &npc->Condition[condition].maxLevel))
					continue;
				else if(!_strnicmp(cmd3, "EVOLUTION", 9) && sscanf_s(cmd4, "%d", &npc->Condition[condition].Evolution))
					continue;
				else if(!_strnicmp(cmd3, "EXP", 9) && sscanf_s(cmd4, "%d", &npc->Condition[condition].Exp))
					continue;
				else if(!_strnicmp(cmd3, "GOLD", 9) && sscanf_s(cmd4, "%d", &npc->Condition[condition].Coin))
					continue;
				else if(!_strnicmp(cmd3, "CLASS", 9) && sscanf_s(cmd4, "%d", &npc->Condition[condition].Class))
					continue;
				else if(!_strnicmp(cmd3, "EQITEM", 6))
				{
					INT32 pos = -1, 
						  itemId = -1;
					
					if(!sscanf_s(cmd4, "%d", &pos) || !sscanf_s(cmd5, "%d", &itemId))
						continue;
					
					npc->Condition[condition].Equip.Slot = pos;
					npc->Condition[condition].Equip.ItemID = itemId;
				}
				else if(!_strnicmp(cmd3, "ITEM", 4))
				{
					INT32 itemId = -1,
						  amount = -1;

					if(!sscanf_s(cmd4, "%d", &amount) || !sscanf_s(cmd5, "%d", &itemId))
						continue;

					if(itemId <= 0 || itemId >= 6500 || amount <= 0 || amount >= 200)
						continue;

					npc->Condition[condition].Item.Item = itemId;
					npc->Condition[condition].Item.Amount = amount;
				}
			}
			else if(!_strnicmp(cmd2, "REWARD", 6))
			{
				INT32 reward = -1;

				sscanf_s(cmd2, "REWARD-%d", &reward);
				if(reward < 0 || reward >= MAX_NPCQUEST_REWARD)
					continue;
				
				if(!_strnicmp(cmd3, "SPEECH", 6) && sscanf_s(cmd4, "%s", npc->Reward[reward].Speech, 96))
					continue;
				else if(!_strnicmp(cmd3, "EXP", 6) && sscanf_s(cmd4, "%d", &npc->Reward[reward].Exp))
					continue;
				else if(!_strnicmp(cmd3, "LEVEL", 6) && sscanf_s(cmd4, "%d", &npc->Reward[reward].Level))
					continue;
				else if(!_strnicmp(cmd3, "GOLD", 6) && sscanf_s(cmd4, "%d", &npc->Reward[reward].Coin))
					continue;
				else if(!_strnicmp(cmd3, "EQUIP", 6))
				{
					INT32 pos = -1;
					STRUCT_ITEM item;
					
					memset(&item, 0, sizeof STRUCT_ITEM);
					if(!sscanf_s(tmp, "%*s %*s %*s %d %hd %hhu %hhu %hhu %hhu %hhd %hhu", &pos, &item.sIndex, &item.EF1, &item.EFV1, &item.EF2, &item.EFV2, &item.EF3, &item.EFV3))
						continue;
					
					npc->Reward[reward].Equip.Slot = pos;
					memcpy(&npc->Reward[reward].Equip.Item, &item, sizeof STRUCT_ITEM);
				}
				else if(!_strnicmp(cmd3, "TELEPORT", 6))
				{
					INT32 posX = 0,
						  posY = 0;
				
					if(!sscanf_s(cmd4, "%d", &posX) || !sscanf_s(cmd5, "%d", &posY))
						continue;

					if(posX < 0 || posX >= 4096 || posY < 0 || posY >= 4096)
					{
						npc->Reward[reward].Teleport.X = 0;
						npc->Reward[reward].Teleport.Y = 0;
					
						printf("Fail : TELEPORT QUEST %d - %dx %dy\n", questId, posX, posY);
					}
					else
					{
						npc->Reward[reward].Teleport.X = posX;
						npc->Reward[reward].Teleport.Y = posY;
					}
					continue;
				}
				else if(!_strnicmp(cmd3, "REMOVEGOLD", 10))
				{
					INT32 gold = 0;

					if(!sscanf_s(cmd4, "%d", &gold))
						continue;

					npc->Remove.Coin = gold;
				}
				else if(!_strnicmp(cmd3, "REMOVEEXP", 10))
				{
					INT32 exp = 0;

					if(!sscanf_s(cmd4, "%d", &exp))
						continue;

					npc->Remove.Exp = exp;
				}
				else if(!_strnicmp(cmd3, "DELETEITEM", 10))
				{
					INT32 amount = 0,
						  itemId = 0,
						  slot   = 0;
				
					if(!sscanf_s(cmd4, "%d", &slot) || !sscanf_s(cmd5, "%d", &amount) || !sscanf_s(cmd6, "%d", &itemId))
						continue;

					npc->Remove.Item[slot].Item = itemId;
					npc->Remove.Item[slot].Amount = amount;
				}
				else if(!_strnicmp(cmd3, "EQDELETE", 8))
				{
					INT32 slotId = 0,
						  itemId = 0;
				
					if(!sscanf_s(cmd4, "%d", &slotId) || !sscanf_s(cmd5, "%d", &itemId))
						continue;

					npc->Remove.Equip.Slot = slotId;
					npc->Remove.Equip.Item = itemId;
				}
				else if(!_strnicmp(cmd3, "ITEM", 4))
				{
					INT32 slot = -1;

					sscanf_s(cmd3, "ITEM-%d", &slot);
					if(slot < 0 || slot >= MAX_NPCQUEST_REWARD_ITEM)
						continue;

					STRUCT_ITEM item;
					memset(&item, 0, sizeof STRUCT_ITEM);

					INT32 amount = -1;
					sscanf_s(tmp, "%*s %*s %*s %d %hd %hhu %hhu %hhu %hhu %hhu %hhu", &amount, &item.sIndex, &item.EF1, &item.EFV1, &item.EF2, &item.EFV2, &item.EF3, &item.EFV3);

					if(amount <= 0 || amount >= 100 || item.sIndex <= 0 || item.sIndex >= 6500)
						continue;

					memcpy(&npc->Reward[reward].Item[slot].Item, &item, sizeof STRUCT_ITEM);
					npc->Reward[reward].Item[slot].Amount = amount;
				}
			}
		}

		fclose(pFile);
		
		if(!FindNextFile(handle, &win32_find_data))
			break;
	}

	FindClose(handle);
	return true;
}

INT32 ReadMessages()
{
	char line[1024];
	memset(sServer.Messages, 0 , sizeof sServer.Messages);

	FILE *pFile;
	fopen_s(&pFile, "Messages.txt", "r");

	if(pFile) 
	{
		int index = 0;

		while((fscanf_s(pFile, "%[^\n]", line, 1024)) != EOF)
		{
			fgetc(pFile);

			if(*line == '#')
				continue;

			sscanf_s(line, "%hd %hd %hd %hd %[^\n]", &sServer.Messages[index].Hour, &sServer.Messages[index].Min, &sServer.Messages[index].Interval, &sServer.Messages[index].Repeat, sServer.Messages[index].msg, 128);

			index++;

			if(index == MAX_MESSAGE)
				break;
		}

		return true;
	}

	return false;
}

INT32 ReadNoCP( )
{
	FILE *pFile = nullptr;

	char szTMP[1024];
	sprintf_s(szTMP, "Data\\NonCP.txt");

	fopen_s(&pFile, szTMP, "r");
	if(!pFile)
		return false;
	
	memset(g_pPositionCP, 0, sizeof g_pPositionCP);
	int count = 0;
	while(fgets(szTMP, sizeof szTMP, pFile))
	{
		if (szTMP[0] == '#')
			continue;

		INT32 minPosX = 0,
			  maxPosX = 0,
			  minPosY = 0,
			  maxPosY = 0;

		INT32 ret = sscanf_s(szTMP, "%d %d %d %d", &minPosX, &minPosY, &maxPosX, &maxPosY);	

		if(minPosX < 0 || minPosY < 0 || minPosX >= 4096 || minPosY >= 4096 || maxPosX < 0 || maxPosY < 0 || maxPosX >= 4096 || maxPosY >= 4096)
			return false;
		
		g_pPositionCP[count].Min.X = minPosX;
		g_pPositionCP[count].Min.Y = minPosY;
		
		g_pPositionCP[count].Max.X = maxPosX;
		g_pPositionCP[count].Max.Y = maxPosY;

		count++;

		if(count >= MAX_MESSAGE)
			break;
	}

	fclose(pFile);
	return true;
}

INT32 ReadScheduled ( )
{
	const std::string filename = "Data\\Scheduled.xml";
	if (!std::filesystem::exists(filename))
		return false;

	pugi::xml_document doc;
	pugi::xml_parse_result result = doc.load_file(filename.c_str(), pugi::parse_full);
	if (!result)
	{
		std::stringstream str;
		str << "parsed with errors, attr value: [" << doc.child("node").attribute("attr").value() << "]\n";
		str << "Error description: " << result.description() << "\n";
		str << "Error offset: " << result.offset << " (error at [..." << (doc.text().as_string() + result.offset) << "]\n\n";

		Log(SERVER_SIDE, LOG_INGAME, "Falha ao ler arquivo Scheduled.xml. Erro:\n%s", str.str().c_str());
		return false;
	}

	sServer.STRUCT_SCHEDULED.clear();
	auto scheduledNode = doc.child("scheduled");
	for (auto commandNode = scheduledNode.child("command"); commandNode; commandNode = commandNode.next_sibling("command"))
	{
		auto& scheduled = sServer.STRUCT_SCHEDULED.emplace_back();
		scheduled.Month = std::stoi(commandNode.child_value("month"));
		scheduled.Day = std::stoi(commandNode.child_value("day"));
		scheduled.Hour = std::stoi(commandNode.child_value("hour"));
		scheduled.Min = std::stoi(commandNode.child_value("min"));
		scheduled.Command = commandNode.child_value("commandline");
	}

	return true;
}

void ClearCrown(int conn)
{
	if (pUser[conn].Status != USER_PLAY)
		return;

	int guildMemberType = pMob[conn].Mobs.Player.GuildLevel;
	int guildId = pMob[conn].Mobs.Player.Guild;

	if(guildId == 0)
		return;

	int isCrownGuild = FALSE;

	int i = 0;

	for (i = 0; i < 10; i++)
	{
		int crownGuild = ChargedGuildList[i][4];

		if ((guildId != 0) && (crownGuild != 0) && (crownGuild == guildId && guildMemberType == 9))
			isCrownGuild = TRUE;
	}

	if (isCrownGuild == 1)
	{
		int haveCrown = 0;
		int haveDrag = 0;

		for (i = 0; i < MAX_EQUIP; i++)
		{
			if (pMob[conn].Mobs.Player.Equip[i].sIndex == 747)
				haveCrown = 1;

			if (pMob[conn].Mobs.Player.Equip[i].sIndex != 3993 && pMob[conn].Mobs.Player.Equip[i].sIndex != 3994) // 747 = Crown on itemlist
				continue;

			haveDrag = 1;
		}

		for (i = 0; i < 64; i++)
		{
			if (pMob[conn].Mobs.Player.Inventory[i].sIndex == 747)
				haveCrown = 1;

			if (pMob[conn].Mobs.Player.Inventory[i].sIndex != 3993 && pMob[conn].Mobs.Player.Inventory[i].sIndex != 3994) // 747 = Crown on itemlist
				continue;

			haveDrag = 1;
		}

		for (i = 0; i < MAX_CARGO; i++)
		{
			if (pUser[conn].User.Storage.Item[i].sIndex == 747)
				haveCrown = 1;

			if (pUser[conn].User.Storage.Item[i].sIndex != 3993 && pUser[conn].User.Storage.Item[i].sIndex != 3994) // 747 = Crown on itemlist
				continue;

			haveDrag = 1;
		}

		if (haveCrown == 0)
		{
			STRUCT_ITEM Item;
			memset(&Item, 0, sizeof(STRUCT_ITEM));

			Item.sIndex = 747;
			Item.stEffect[0].cEffect = 43;
			Item.stEffect[0].cValue = 9;

			UINT32 guildFame = g_pGuild[guildId].Fame;
			if (guildFame >= 1000)
			{
				if (guildFame < 1500)
					Item.stEffect[0].cValue = 233;
				else if (guildFame < 3000)
					Item.stEffect[0].cValue = 237;
				else if (guildFame < 5000) 
					Item.stEffect[0].cValue = 241;
				else if (guildFame < 8000)
					Item.stEffect[0].cValue = 245;
				else if (guildFame < 11000)
					Item.stEffect[0].cValue = 249;
				else
					Item.stEffect[0].cValue = 253;
			}

			PutItem(conn, &Item);
		}

		if (haveDrag == 0)
		{
			STRUCT_ITEM Item;
			memset(&Item, 0, sizeof(STRUCT_ITEM));

			if (pMob[conn].Mobs.Player.CapeInfo == CAPE_RED)
				Item.sIndex = 3993;
			else if (pMob[conn].Mobs.Player.CapeInfo == CAPE_BLUE)
				Item.sIndex = 3994;


			PutItem(conn, &Item);
		}

		return;
	}

	for (i = 0; i < MAX_EQUIP; i++)
	{
		if (pMob[conn].Mobs.Player.Equip[i].sIndex != 747 && pMob[conn].Mobs.Player.Equip[i].sIndex != 3993 && pMob[conn].Mobs.Player.Equip[i].sIndex != 3994) // 747 = Crown on itemlist
			continue;

		//sprintf_s(temp, "etc,crown guild:%d level:%d charge:%d name:%s", Guild, GLevel, ChargedGuildList[ServerIndex][4], pMob[conn].MOB.ItemName);
		//Log(temp, pUser[conn].AccountName, pUser[conn].IP);

		memset(&pMob[conn].Mobs.Player.Equip[i], 0, sizeof(STRUCT_ITEM));
		SendItem(conn, SlotType::Equip, i, &pMob[conn].Mobs.Player.Equip[i]);
	}

	for (i = 0; i < MAX_INVEN; i++)
	{
		if (pMob[conn].Mobs.Player.Inventory[i].sIndex != 747 && pMob[conn].Mobs.Player.Inventory[i].sIndex != 3993 && pMob[conn].Mobs.Player.Inventory[i].sIndex != 3994) // 747 = Crown on itemlist
			continue;

		memset(&pMob[conn].Mobs.Player.Inventory[i], 0, sizeof(STRUCT_ITEM));
		SendItem(conn, SlotType::Inv, i, &pMob[conn].Mobs.Player.Inventory[i]);
	}

	for (i = 0; i < MAX_CARGO; i++)
	{
		if (pUser[conn].User.Storage.Item[i].sIndex != 747 && pUser[conn].User.Storage.Item[i].sIndex != 3993 && pUser[conn].User.Storage.Item[i].sIndex != 3994) // 747 = Crown on itemlist
			continue;

		//sprintf_s(temp, "etc,crown guild:%d level:%d charge:%d name:%s", Guild, GLevel, ChargedGuildList[ServerIndex][4], pMob[conn].MOB.ItemName);
		//Log(temp, pUser[conn].AccountName, pUser[conn].IP);

		memset(&pUser[conn].User.Storage.Item[i], 0, sizeof(STRUCT_ITEM));

		SendItem(conn, SlotType::Storage, i, &pUser[conn].User.Storage.Item[i]);
	}
}

void GuildZoneReport()
{	
	MSG_GuildZoneReport sm;
	memset(&sm, 0, sizeof(MSG_GuildZoneReport));

	sm.Header.PacketId = _MSG_GuildZoneReport;
	sm.Header.Size = sizeof(sm);
	sm.Header.ClientId = sServer.Channel - 1;

	for (int i = 0; i < 5; i++)
		sm.Guild[i] = ChargedGuildList[sServer.Channel - 1][i];
	
	AddMessageDB((BYTE*)&sm, sizeof(sm));
}

void ClearTowerArea(BYTE Citizenship)
{
	// Citizenship:
	// 0 -> Guerra iniciando (teleporta todos na area)
	// 1 -> Deixar somente cidadania 1
	// 2 -> Deixar somente cidadania 2
		
	for (int i = 0; i < MAX_PLAYER; i++)
	{
		CMob *player = &pMob[i];

		if (player->Target.X < 2447 || player->Target.X > 2545 ||
			player->Target.Y < 1857 || player->Target.Y > 1919)
			continue;

		DoRecall(i);
	}
}

void InitializeTowerWar()
{
	sServer.TowerWar.CurrentScore = 1;
	
	GenerateMob(TORRE_ERION, 0, 0);
	ClearTowerArea(0);
	
	SendNotice("A batalha do acampamento avanaado do reino foi iniciada.");
}

void FinalizeTowerWar()
{
	sServer.TowerWar.CurrentScore = 0;
	
	for (int i = MAX_PLAYER; i < MAX_MOB; i++)
	{
		if (pMob[i].GenerateID == TORRE_ERION)
		{
			// Mata a torre de thor de forma definitiva
			MobKilled(i, 29999, pMob[i].Target.X, pMob[i].Target.Y);

			break;
		}
	}
	
	INT32 guildId = sServer.TowerWar.Guild;
	if(guildId != 0)
	{
		INT32 fame = g_pGuild[guildId].Fame + 100;
		INT32 win = g_pGuild[guildId].Wins + 1;

		SetGuildFame(guildId, fame);
		Log(SERVER_SIDE, LOG_INGAME, "Guild %s [%d] conquistou a Torre de Erion - Fame: %d - Wins: %d", g_pGuild[guildId].MobName.c_str(), guildId, fame, win);

		SetGuildWin(guildId, win);
	}

	SendNotice("A batalha de acampamento avanaado do reino foi finalizada");
	
	ClearTowerArea(0);
}




INT32 ReadNPCDonate()
{
	FILE *pFile = nullptr;
	fopen_s(&pFile, "..\\DBsrv\\Donate.txt", "r");

	memset(g_pStore, 0, sizeof g_pStore);

	if(pFile)
	{
		char line[1024];

		while(fgets(line, sizeof line, pFile))
		{
			// Comentario
			if(*line == '#' || *line == '\n')
				continue; 

			INT32 store = 0,
				  index = 0,
				  avaible = -1,
				  price = 0,
				  loop = 0,
				  itemId = 0, ef1 = 0, ef2 = 0, ef3 = 0,
				  efv1 = 0, efv2 = 0, efv3 = 0;

			INT32 ret = sscanf_s(line, "%d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d", &store, &index, &avaible, &price, &loop,
				&itemId, &ef1, &efv1, &ef2, &efv2, &ef3, &efv3);

			if(ret < 6)
			{
				MessageBoxA(NULL, line, "ERROR", NULL);

				break;
			}

			if(store < 0 || store >= 5 || index >= 27 || index < 0)
				continue;

			g_pStore[store][index].Avaible = avaible;
			g_pStore[store][index].Loop = loop;
			g_pStore[store][index].Price = price;

			g_pStore[store][index].item.sIndex = itemId;
			g_pStore[store][index].item.EF1 = ef1;
			g_pStore[store][index].item.EF2 = ef2;
			g_pStore[store][index].item.EF3 = ef3;
			g_pStore[store][index].item.EFV1 = efv1;
			g_pStore[store][index].item.EFV2 = efv2;
			g_pStore[store][index].item.EFV3 = efv3;
		}

		fclose(pFile);
	}
	else
		return false;

	return true;
}

INT32 ReadPacItens()
{
	FILE *pFile;
	fopen_s(&pFile, "..\\DBsrv\\PacItem.txt", "r");
	memset(g_pPacItem, 0, sizeof g_pPacItem);

	if(!pFile)
		return false;

	char line[1024];
	while(fgets(line, sizeof line, pFile))
	{
		if(*line == '#' || *line == '\n')
			continue;

		STRUCT_ITEM item = {0, };
		INT32 index = -1;
		INT32 amount = 0;
		INT32 itemId = 0;
		INT32 ret = sscanf_s(line, "%d, %d, %d, %hd, %hhu, %hhu, %hhu, %hhu, %hhu, %hhu", &itemId, &index, &amount, &item.sIndex, &item.EF1, &item.EFV1, 
			&item.EF2, &item.EFV2, &item.EF3, &item.EFV3);

		if(ret < 3)
			continue;

		if(index < 0 || index >= MAX_PACITEM)
			continue;
		
		g_pPacItem[index].ItemId = itemId;

		INT32 t = 0;
		for(; t < 8; t++)
			if(g_pPacItem[index].Item[t].sIndex == 0)
				break;
		
		if(t == 8)
			continue;

		memcpy(&g_pPacItem[index].Item[t], &item, sizeof STRUCT_ITEM);
		g_pPacItem[index].Amount[t] = amount;
	}

	fclose(pFile);
	return true;
}

INT32 ReadBossQuest()
{
	memset(sServer.QuestsBosses, 0, sizeof STRUCT_QUESTS * 5);

	for (int i = 0; i < 5; i++)
	{
		FILE *fs = NULL;

		char Directory[1024];
		sprintf_s(Directory, "Data\\BOSS_QUEST_%d.txt", i);

		fopen_s(&fs, Directory, "rt");

		if (!fs)
			continue;

		char line[1024];
		int e = 0;

		while (fgets(line, sizeof(line), fs))
		{
			if (*line == '\n' || *line == '#')
				continue;

			if (e == 0)
				sscanf_s(line, "%d,%d", &sServer.QuestsBosses[i].Boss.CountToBorn, &sServer.QuestsBosses[i].Boss.gennerId);
			else if (e < 11)
			{
				int count = e - 1;
				sscanf_s(line, "%hud,%hhu,%hhu,%hhu,%hhu,%hhu,%hhu,%hhd", &sServer.QuestsBosses[i].Boss.Gifts[count].sIndex,
					&sServer.QuestsBosses[i].Boss.Gifts[count].stEffect[0].cEffect, &sServer.QuestsBosses[i].Boss.Gifts[count].stEffect[0].cValue,
					&sServer.QuestsBosses[i].Boss.Gifts[count].stEffect[1].cEffect, &sServer.QuestsBosses[i].Boss.Gifts[count].stEffect[1].cValue,
					&sServer.QuestsBosses[i].Boss.Gifts[count].stEffect[2].cEffect, &sServer.QuestsBosses[i].Boss.Gifts[count].stEffect[2].cValue,
					&sServer.QuestsBosses[i].Boss.Chances[count]);
			}
			else 
				break;

			e++;
		}

		fclose(fs);
	}

	return true;
}

INT32 ReadBlockedItem()
{
	FILE *pFile;
	fopen_s(&pFile, "BlockDropItem.txt", "r");
	if(!pFile)
		return false;

	char line[1024] = { 0 };
	for (int i = 0; i < MAX_BLOCKITEM; ++i)
		g_pBlockedItem[i] = 0;

	INT32 x = 0;
	while(fgets(line, sizeof line, pFile))
	{
		// Comentario
		if(*line == '#' || *line == '\n')
			continue; 

		INT32 itemId = -1;
		INT32 ret = sscanf_s(line, "%d,%*[^\n]", &itemId);
		if(ret != 1)
			continue;

		if(itemId <= 0 || itemId >= MAX_ITEMLIST)
			continue;

		g_pBlockedItem[x++] = itemId;
		if (x >= MAX_BLOCKITEM)
			break;
	}

	fclose(pFile);
	return true;
}

void RebornKefra()
{
	Log(SERVER_SIDE, LOG_INGAME, "Recebido sinal para renascimento do Kefra");

	if(!sServer.KefraDead)
		return;

	for(INT32 i = GUARDAS_KEFRA; i < GUARDAS_KEFRA + 18; i++)
	{
		// Faz o mob nao ficar nascendo apas morrer 
		mGener.pList[i].MinuteGenerate = 4;

		// Gera os mobs
		GenerateMob(i, 0, 0);
	}

	// Limpa as variaveis
	sServer.KefraDead = FALSE;
	sServer.KefraKiller = 0;

	// Gera o Kefra no devido local
	GenerateMob(KEFRA, 0, 0);

	// Envia mensagem para o canal completo com o renascimento do Kefra
	SendNotice(g_pLanguageString[_NN_Kefra_Reborn]);
		
	Log(SERVER_SIDE, LOG_ADMIN, "Kefra renasceu");

	// Limpa a area
	ClearArea(3200, 1664, 3328, 1791);
}

 

bool LoadNPCEvento() 
{ 
	FILE *pFile = nullptr;
	fopen_s(&pFile, "Data\\NPCEvento.txt", "r");

	// Apaga totalmente o NPC de evento
	memset(npcEvent, 0, sizeof npcEvent);

	if(pFile != NULL)
	{
		char Text[1024];

		// Contagem dos NPCs
		int npcId = -1;

		while(fgets(Text, 1024, pFile)) 
		{
			if(Text[0] == '#')
			{
				npcId ++;
				continue;
			}

			if(Text[0] == '/' && Text[1] == '/')
				continue;

			char cmd[32], val[512];
			// Comments
			int ret = sscanf_s(Text, "%[^=]=%[^\n]", cmd, 32, val, 512);

			if(ret != 2)
				continue;

			STRUCT_NPC_EVENT *ev = &npcEvent[npcId];
			if(!_strnicmp("itemRequired_", cmd, 13))
			{
				int itemR = 0;
				int ret = sscanf_s(cmd, "itemRequired_%d", &itemR);
				if(ret != 1)
				{
					MessageBoxA(NULL, "Falha do GM #1", "Falha do GM #1", 4096);

					continue;
				}

				STRUCT_ITEM *item = &ev->itemRequired[itemR];
				sscanf_s(val, "%hd %hhu %hhu %hhu %hhu %hhu %hhu", &item->sIndex, &item->EF1, &item->EFV1, &item->EF2, &item->EFV2, &item->EF3, &item->EFV3);
			}
			else if(!_strnicmp("goldEarned", cmd, 10))
			{
				int itemR = 0;
				int ret = sscanf_s(cmd, "goldEarned %d", &itemR);
				if(ret != 1)
				{
					MessageBoxA(NULL, "Falha do GM #1", "Falha do GM #1", 4096);

					continue;
				}
			}
			else if(!_strnicmp("amountRequired_", cmd, 15))
			{
				int itemR = 0;
				int ret = sscanf_s(cmd, "amountRequired_%d", &itemR);
				if(ret != 1)
				{
					MessageBoxA(NULL, "Falha do GM #1", "Falha do GM #1", 4096);

					continue;
				}

				ev->amountRequired[itemR] = atoi(val);
			}
			else if(!_strnicmp("goldRequired", cmd, 12))
				ev->goldRequired = atoi(val);

			else if(!_strnicmp("itemEarned_", cmd, 11))
			{
				int itemR = 0;
				int itemR2 = 0;
				int ret = sscanf_s(cmd, "itemEarned_%d_%d", &itemR, &itemR2);
				if(ret != 1 && ret != 2)
				{
					MessageBoxA(NULL, "Falha do GM #2", "Falha do GM #2", 4096);

					continue;
				}

				STRUCT_ITEM *item = &ev->itemEarned[itemR][itemR2];
				sscanf_s(val, "%hd %hhu %hhu %hhu %hhu %hhu %hhu", &item->sIndex, &item->EF1, &item->EFV1, &item->EF2, &item->EFV2, &item->EF3, &item->EFV3);
			}
			else if(!_strnicmp("itemRate_", cmd, 9))
			{
				int itemR = 0;
				int ret = sscanf_s(cmd, "itemRate_%d", &itemR);
				if(ret != 1)
				{
					MessageBoxA(NULL, "Falha do GM #3", "Falha do GM #3", 4096);

					continue;
				}

				ev->Rates[itemR] = atoi(val);
			}
			else if(!_strnicmp("teleport_", cmd, 9))
			{
				int itemR = 0;
				int ret = sscanf_s(cmd, "teleport_%d", &itemR);
				if(ret != 1)
				{
					MessageBoxA(NULL, "Falha do GM #4", "Falha do GM #4", 4096);

					continue;
				}

				sscanf_s(val, "%hud %hud", &ev->Pos[itemR].X, &ev->Pos[itemR].Y);
			}
			else if (!_strnicmp("teleMsg_", cmd, 8))
			{
				int itemR = 0;
				int ret = sscanf_s(cmd, "teleMsg_%d", &itemR);
				if (ret != 1)
				{
					MessageBoxA(NULL, "Falha do GM #5", "Falha do GM #5", 4096);

					continue;
				}

				strncpy_s(ev->msg[itemR], val, 108);
			}
			else if (!_strnicmp("npcName", cmd, 8))
				strncpy_s(ev->npcId, val, 108);
		}

		fclose(pFile);
	}
	else 
		return false;

	return true;
}

void NumberFormat(char*result)
{
	char st[32];
	memset(st, 0, sizeof st);

	strncpy_s(st, result, 32);

	int len = strlen(st);
	int sum = ((len - 1) / 3);
	
	for(int i = (len - 1), count = 0, index = (len - 1) + sum; i >= 0; i--, count++)
	{
		if(!(count % 3) && count != 0)
		{
			result[index] = ',';
			index--;
		}

		result[index] = st[i];

		count++;
		index--;
	}

	if(len + sum < 32)
		result[len + sum] = 0;
}

void FinishColosseum()
{
	sServer.Colosseum.level = 0;
	sServer.Colosseum.inSec.boss = false;
	sServer.Colosseum.inSec.closedGate = false;
	sServer.Colosseum.inSec.closedWalls = false;
	sServer.Colosseum.inSec.wasBorn = false;

	sServer.Colosseum.npc = 0;
	sServer.Colosseum.time = 0;

	sServer.Colosseum.clients.clear();

	// Portaes de entrada
	SetColoseumDoor(1);

	// Portaes do meio ^^
	SetColoseumDoor2(3);

	for(INT32 i = MAX_PLAYER; i < MAX_MOB; i++)
	{
		if(pMob[i].Target.X >= 2608 && pMob[i].Target.X <= 2647  && pMob[i].Target.Y >= 1708 && pMob[i].Target.Y <= 1748)
			DeleteMob(i, 1);
	}

	ClearArea(2608, 1708, 2647, 1748);
}

void DoColosseum()
{
    time_t rawnow = time(NULL);
    struct tm now; localtime_s(&now, &rawnow);

	// Coliseu nao esta ativo ainda
	if(sServer.Colosseum.level == 0)
		return;

	// Checa se as muralhas ja desceram ou nao
	if(sServer.Colosseum.inSec.closedWalls == 0) 
		return;

	char szTMP[256];

	sServer.Colosseum.time -= 1;

	if(sServer.Colosseum.time <= 0)
	{
		ClearArea(2647, 1748, 2608, 1708);
		memset(&sServer.Colosseum, 0, sizeof sServer.Colosseum);

		SendNotice("Coliseu finalizado. Nao foi derrotado todos os monstros dentro do tempo");

		FinishColosseum();
		Log(SERVER_SIDE, LOG_INGAME, "Coliseu finalizado. Nao foi derrotado todos os monstros dentro do tempo");
		return;
	}

	switch(sServer.Colosseum.level)
	{
		// Navel 1 do Coliseu
		case 1:
		// Navel 2 do Coliseu
		case 2:
		// Navel 3 do Coliseu
		case 3:
		{
			// Comeaa depois de todos os boss
			int BASE_MOB = (COLOSSEUM_ID + 4) + (9 * (sServer.Colosseum.level - 1));
			if(!sServer.Colosseum.inSec.wasBorn)
			{
				// TODO : GenerateMob dos mobs
				sServer.Colosseum.inSec.wasBorn = true;

				for(int i = 0; i < 9; i++)
					GenerateMob(BASE_MOB + i, 0, 0);
			}

			int totalMob = 0;
			// Alteraaao do tamanho do loop para adequar-se a quantidade de mobs gerados no local
			if(!sServer.Colosseum.inSec.boss)
				for(int i = 0; i < 9; i++)
					totalMob += mGener.pList[BASE_MOB + i].MobCount;
			else
				totalMob = mGener.pList[COLOSSEUM_ID + (sServer.Colosseum.level - 1)].MobCount; 

			// TODO : Alterar o valor do '4' para o na do loop pois cada vez que um grupo de mob acaba totalmente fica o valor '1'
			if((sServer.Colosseum.inSec.boss && totalMob == 0) || (!sServer.Colosseum.inSec.boss && totalMob == 0))
			{	
				if(sServer.Colosseum.inSec.boss)
				{
					if(sServer.Colosseum.level != 3)
						sprintf_s(szTMP, "Boss derrotado. Navel %d alcanaado...", (sServer.Colosseum.level + 1));
					else
						strncpy_s(szTMP, "Proteja Tyr para continuar sua missao! Caso ele morra, o Coliseu acaba!", 108);

					sServer.Colosseum.time = 300;
					sServer.Colosseum.level += 1;

					for(int i = 1; i < MAX_PLAYER ; i ++)
					{
						if(pUser[i].Status != 22)
							continue;
						
						if(pMob[i].Target.X >= 2608 && pMob[i].Target.X <= 2647 && pMob[i].Target.Y >= 1708 && pMob[i].Target.Y <= 1748) 
						{
							SendClientMessage(i, szTMP);
							SendSignalParm(i, SERVER_SIDE, 0x3A1, sServer.Colosseum.time);
						}
					}

					sServer.Colosseum.inSec.boss = false;
					
					// TODO : GenerateMob do praximo navel
					int baseMob = (COLOSSEUM_ID + 4) + (sServer.Colosseum.level - 1) * 9;
					for(int i = 0; i < 9; i++)
						GenerateMob(baseMob + i, 0, 0);

					if(sServer.Colosseum.level == 4)
						GenerateMob(COLOSSEUM_TYR, 0, 0);
				}
				else
				{
					sServer.Colosseum.time = 300;
					sServer.Colosseum.inSec.boss = true;

					for(int i = 0 ; i < MAX_PLAYER ; i ++)
					{
						if(pUser[i].Status != USER_PLAY)
							continue;
						
						if(pMob[i].Target.X >= 2608 && pMob[i].Target.X <= 2647 && pMob[i].Target.Y >= 1708 && pMob[i].Target.Y <= 1748) 
							SendClientMessage(i, "Boss nasceu!");
					}

					GenerateMob(COLOSSEUM_ID + (sServer.Colosseum.level - 1), 0, 0);
				}
			}
		}
		break;
		case 4:
		{
			if(mGener.pList[COLOSSEUM_TYR].MobCount == 0)
			{
				for(int i = 0 ; i < MAX_PLAYER ; i ++)
				{
					if(pUser[i].Status != USER_PLAY)
						continue;
						
					if(pMob[i].Target.X >= 2608 && pMob[i].Target.X <= 2647 && pMob[i].Target.Y >= 1708 && pMob[i].Target.Y <= 1748) 
						SendClientMessage(i, "Coliseu finalizado. Tyr foi derrotado!!");
				}
				
				FinishColosseum();
				return;
			}

			// Comeaa depois de todos os boss
			int BASE_MOB = (COLOSSEUM_ID + 4) + (9 * (sServer.Colosseum.level - 1));

			int totalMob = 0;
			// Alteraaao do tamanho do loop para adequar-se a quantidade de mobs gerados no local
			if(!sServer.Colosseum.inSec.boss)
				for(int i = 0; i < 9; i++)
					totalMob += mGener.pList[BASE_MOB + i].MobCount;
			else
				totalMob = mGener.pList[COLOSSEUM_ID + (sServer.Colosseum.level - 1)].MobCount; 
			
			// TODO : Alterar o valor do '4' para o na do loop pois cada vez que um grupo de mob acaba totalmente fica o valor '1'
			if((sServer.Colosseum.inSec.boss && totalMob == 0) || (!sServer.Colosseum.inSec.boss && totalMob == 0))
			{
				if(sServer.Colosseum.inSec.boss)
				{
					sServer.Colosseum.time = 300;
					sServer.Colosseum.level += 1;

					sServer.Colosseum.inSec.boss = false;
					FinishColosseum();
				}
				else
				{
					sServer.Colosseum.time = 300;
					sServer.Colosseum.inSec.boss = true;

					for(int i = 1 ; i < MAX_PLAYER; i ++)
					{
						if(pUser[i].Status != 22)
							continue;
						
						if(pMob[i].Target.X >= 2608 && pMob[i].Target.X <= 2647 && pMob[i].Target.Y >= 1708 && pMob[i].Target.Y <= 1748) 
							SendClientMessage(i, "altimo Boss nasceu!");
					}

					GenerateMob(COLOSSEUM_ID + (sServer.Colosseum.level - 1), 0, 0);
				}
			}

		}
		break;
	}
}

void DropEventOnHit(INT32 clientId, INT32 targetIdx) 
{
	if(!sServer.BossEvent.CurrentScore)
		return;
	
	INT32 rate  = sServer.BossEvent.Rate,
		  bonus = sServer.BossEvent.Bonus;

	for(INT32 i = 0; i < 32; i++)
	{
		if(pMob[clientId].Mobs.Affects[i].Index == 56)
		{
			rate += bonus;

			break;
		}
	}

	INT32 _rand = Rand() % 15000;
	if(_rand >= rate)
		return ;

	INT32 slotId = GetFirstSlot(clientId, 0);
	if(slotId == -1)
	{
		SendClientMessage(clientId, "!Sem espaao para receber o item do evento");

		return;
	}

	if(PutItem(clientId, &sServer.BossEvent.item))
	{
		Log(clientId, LOG_INGAME, "Recebeu o [%s] [%d] [%d %d %d %d %d %d] do evento. %d/%d", g_pItemList[sServer.BossEvent.item.sIndex].ItemName,
			sServer.BossEvent.item.sIndex, sServer.BossEvent.item.EF1, sServer.BossEvent.item.EFV1, sServer.BossEvent.item.EF2, sServer.BossEvent.item.EFV2,
			sServer.BossEvent.item.EF3, sServer.BossEvent.item.EFV3, _rand, rate);
	}
	else
	{
		SendClientMessage(clientId, "!Sem espaao para receber o item do evento");

		return;
	}
	
	sServer.BossEvent.Count ++;
}

void PremiumTime() 
{
	if(sServer.PremiumTime.Second <= 0 || sServer.PremiumTime.Interval <= 0)
		return;

	sServer.PremiumTime.Second ++;
	if(sServer.PremiumTime.Second - sServer.PremiumTime.Last < sServer.PremiumTime.Interval)
		return;

	sServer.PremiumTime.Last = sServer.PremiumTime.Second;

	INT16 players[MAX_PLAYER];
	memset(&players[0], 0, sizeof players);

	INT32 count = 0;
	for(INT32 i = 1; i < MAX_PLAYER; i++)
	{
		if(pUser[i].Status != USER_PLAY)
			continue;

		players[count] = i;
		count++;

		if(count >= MAX_PLAYER)
			break;
	}

	INT32 totalLoop = 0;
	while(true)
	{
		if(count <= 0)
			break;

		totalLoop ++;
		if(totalLoop >= 500)
			break;

		INT32 _rand = Rand() % (count + 1);
		if(_rand >= count)
			_rand = Rand() % count;

		INT32 clientId = players[_rand];
		if(pUser[clientId].Status != USER_PLAY)
			continue;

		STRUCT_MOB *player = &pMob[clientId].Mobs.Player;
		if(sServer.PremiumTime.Coin != 0)
		{
			if(sServer.PremiumTime.Coin + player->Coin > 2000000000)
				continue;
		}
				
		INT32 slotId = -1;
		if(sServer.PremiumTime.Item.sIndex != 0)
		{
			slotId = GetFirstSlot(clientId, 0);
			if(slotId == -1)
				continue;
		}

		char szTMP[256];

		player->Coin += sServer.PremiumTime.Coin;
		SendSignalParm(clientId, clientId, 0x3AF, player->Coin);

		if(slotId != -1)
		{
			memset(&player->Inventory[slotId], 0, sizeof STRUCT_ITEM);
			memcpy(&player->Inventory[slotId], &sServer.PremiumTime.Item, sizeof STRUCT_ITEM);
					
			SendItem(clientId, SlotType::Inv, slotId, &sServer.PremiumTime.Item);
		}

		if(sServer.PremiumTime.Coin != 0 && sServer.PremiumTime.Item.sIndex != 0)
			sprintf_s(szTMP, "O usuario %s ganhou %d de Gold e o item %s", player->MobName, sServer.PremiumTime.Coin, g_pItemList[sServer.PremiumTime.Item.sIndex].ItemName);
		else if(sServer.PremiumTime.Coin != 0)
			sprintf_s(szTMP, "O usuario %s ganhou %d de Gold", player->MobName, sServer.PremiumTime.Coin);
		else if(sServer.PremiumTime.Item.sIndex != 0)
			sprintf_s(szTMP, "O usuario %s ganhou o item %s", player->MobName, g_pItemList[sServer.PremiumTime.Item.sIndex].ItemName);

		SendNotice(szTMP);
		Log(SERVER_SIDE, LOG_INGAME, szTMP);
		break;
	}
}
 
INT32 RemoveAmount(INT32 clientId, INT32 itemId, INT32 amount) 
{
	INT32 retn = 0;
	for(INT32 LOCAL_832 = 0; LOCAL_832 < 60 ; LOCAL_832++)
	{
		if(pMob[clientId].Mobs.Player.Inventory[LOCAL_832].sIndex == itemId)
		{
			int totalRemoved = 0;
			while(pMob[clientId].Mobs.Player.Inventory[LOCAL_832].sIndex == itemId)
			{
				AmountMinus(&pMob[clientId].Mobs.Player.Inventory[LOCAL_832]);
				totalRemoved++;

				amount--;
				retn  ++;
				if(amount <= 0)
					break;
			}

			SendItem(clientId, SlotType::Inv, LOCAL_832, &pMob[clientId].Mobs.Player.Inventory[LOCAL_832]);
			Log(clientId, LOG_INGAME, "Removido %d unidade(s) de %s %s no slot %d", totalRemoved, g_pItemList[itemId].ItemName, pMob[clientId].Mobs.Player.Inventory[LOCAL_832].toString().c_str(), LOCAL_832);
		}

		if(amount <= 0)
			break;
	}

	return retn;
}

BOOL ReadMerchantStore()
{
	FILE *pFile = NULL;

	fopen_s(&pFile, "Data\\Store.txt", "r");
	memset(&sServer.PremierStore, 0, sizeof sServer.PremierStore);

	if (pFile)
	{
		char tmp[256];
		INT32 count = 0;
		while (fgets(tmp, sizeof(tmp), pFile))
		{
			if (tmp[0] == '#' || tmp[0] == '\n')
				continue;

			if (count >= MAX_PREMIERSTORE)
				break;

			STRUCT_ITEM item;
			memset(&item, 0, sizeof STRUCT_ITEM);

			INT32 price = 0;
			INT32 ret = sscanf_s(tmp, R"(%d,%hd,%hhu,%hhu,%hhu,%hhu,%hhu,%hhu)", &price, &item.sIndex, &item.EF1, &item.EFV1,
				&item.EF2, &item.EFV2, &item.EF3, &item.EFV3);

			if (price <= 0 || price >= 10000)
				continue;

			if (item.sIndex <= 0 || item.sIndex >= MAX_ITEMLIST)
				continue;

			memcpy(&sServer.PremierStore.item[count], &item, sizeof STRUCT_ITEM);
			sServer.PremierStore.Price[count] = price;

			count++;
		}

		fclose(pFile);
	}

	return false;
}

BOOL ReadArenaStore()
{
	FILE *pFile = NULL;

	fopen_s(&pFile, "Data\\ArenaStore.txt", "r");
	memset(&sServer.ArenaStore, 0, sizeof sServer.ArenaStore);

	if (pFile)
	{
		char tmp[256];
		INT32 count = 0;
		while (fgets(tmp, sizeof(tmp), pFile))
		{
			if (tmp[0] == '#' || tmp[0] == '\n')
				continue;

			if (count >= MAX_PREMIERSTORE)
				break;

			STRUCT_ITEM item;
			memset(&item, 0, sizeof STRUCT_ITEM);

			INT32 price = 0;
			INT32 ret = sscanf_s(tmp, R"(%d,%hd,%hhu,%hhu,%hhu,%hhu,%hhu,%hhu)", &price, &item.sIndex, &item.EF1, &item.EFV1,
				&item.EF2, &item.EFV2, &item.EF3, &item.EFV3);

			if (price <= 0 || price >= 10000)
				continue;

			if (item.sIndex <= 0 || item.sIndex >= MAX_ITEMLIST)
				continue;

			memcpy(&sServer.ArenaStore.item[count], &item, sizeof STRUCT_ITEM);
			sServer.ArenaStore.Price[count] = price;

			count++;
		}

		fclose(pFile);
	}

	return false;
}

void LevelItem(INT32 clientId) 
{
	INT32 level = pMob[clientId].Mobs.Player.CurrentScore.Level; 
	INT32 evolution = pMob[clientId].Mobs.Player.Equip[0].EFV2;

	for(auto const item : sServer.levelItem)
	{
		auto itemL = &item;
		if(itemL->item.sIndex <= 0 || itemL->item.sIndex >= MAX_ITEMLIST || itemL->Level != level || evolution != itemL->Evolution)
			continue;

		INT32 type   = itemL->Type,
			  classe = itemL->Classe; 

		if (evolution >= ARCH)
		{
			// Se for != -1, quer dizer que ele checa a classe
			if (type != -1 && classe != GetInfoClass(pMob[clientId].Mobs.Player.Equip[0].EF2))
				continue;
		}
		else
		{
			// Se for != -1, quer dizer que ele checa a classe
			if (type != -1 && classe != pMob[clientId].Mobs.Player.Class)
				continue;
		}
		if(type >= 1  && type <= 3)
		{
			if(type == 1)
			{
				if((pMob[clientId].Mobs.Player.CurrentScore.Str + pMob[clientId].Mobs.Player.CurrentScore.Dex) <= pMob[clientId].Mobs.Player.CurrentScore.Int)
					continue;
			}
			else if(type == 2)
			{
				if(pMob[clientId].Mobs.Player.CurrentScore.Int <= pMob[clientId].Mobs.Player.CurrentScore.Str + pMob[clientId].Mobs.Player.CurrentScore.Dex)
					continue;
			}
			else if(type == 3)
			{
				if(pMob[clientId].Mobs.Player.CurrentScore.Dex <= pMob[clientId].Mobs.Player.CurrentScore.Str)
					continue;
			}
		}

		INT32 slotId = GetFirstSlot(clientId, 0);
		bool isBank = false;
		if (slotId == -1)
		{
			isBank = true;

			for (int i = 0; i < 120; ++i) 
			{
				if (pUser[clientId].User.Storage.Item[i].sIndex <= 0)
				{
					slotId = i;

					break;
				}
			}
		}

		if(slotId == -1)
		{
			SendClientMessage(clientId, "!Sem espaao no banco para receber o item!");

			Log(clientId, LOG_INGAME, "Nao recebeu o item por level por falta de espaao no banco. ItemID: %s.", itemL->item.toString().c_str());
			continue;
		}

		Log(clientId, LOG_INGAME, "Recebeu o item %s %s. STR: %d. INT: %hu. DEX: %hu. CON: %hu",
			g_pItemList[itemL->item.sIndex].ItemName,
			itemL->item.toString().c_str(),
			pMob[clientId].Mobs.Player.CurrentScore.Str,
			pMob[clientId].Mobs.Player.CurrentScore.Int,
			pMob[clientId].Mobs.Player.CurrentScore.Dex,
			pMob[clientId].Mobs.Player.CurrentScore.Con
		);

		if (!isBank)
		{
			pMob[clientId].Mobs.Player.Inventory[slotId] = itemL->item;
			SendItem(clientId, SlotType::Inv, slotId, &pMob[clientId].Mobs.Player.Inventory[slotId]);

			LogPlayer(clientId, "Recebeu o item %s por chegar ao navel %d", g_pItemList[itemL->item.sIndex].ItemName, level + 1);
		}
		else
		{
			pUser[clientId].User.Storage.Item[slotId] = itemL->item;
			SendItem(clientId, SlotType::Storage, slotId, const_cast<STRUCT_ITEM*>(&itemL->item));

			LogPlayer(clientId, "Recebeu o item %s por chegar ao navel %d (recompensa chegou ao baa)", g_pItemList[itemL->item.sIndex].ItemName, level + 1);
		}

		SendClientMessage(clientId, "Chegou um item: [ %s ]", g_pItemList[itemL->item.sIndex].ItemName);
	}
}

BOOL ReadLevelItem() 
{
	FILE *pFile = NULL;
	std::fill(std::begin(sServer.levelItem), std::end(sServer.levelItem), STRUCT_LEVELITEM{});

	fopen_s(&pFile, "Data//LevelItem.txt", "r");
	if(pFile)
	{
		char tmp[256];

		while(fgets(tmp, sizeof tmp, pFile))
		{
			if(tmp[0] == '#' || tmp[0] == '\n')
				continue;

			INT32 evolution     = -1,
				  type   = -1,
				  classe = -1,
				  level  = -1,
				  itemId = -1,
				  ef1 = 0, efv1 = 0, ef2 = 0, efv2 = 0, ef3 = 0, efv3 = 0;

			STRUCT_ITEM item{};

			INT32 ret = sscanf_s(tmp, "%d %d %d %d %d %d %d %d %d %d %d", &evolution, &level, &classe, &type,
				&itemId, &ef1, &efv1, &ef2, &efv2, &ef3, &efv3);//&item.sIndex, &item.EF1, &item.EFV1, &item.EF2, &item.EFV2, &item.EF3, &item.EFV3);

			if(ret < 5)
				continue;

			sServer.levelItem.push_back(STRUCT_LEVELITEM{});

			auto& levelItem = sServer.levelItem.back();
			levelItem.Classe = classe;
			levelItem.Level  = level;
			levelItem.Type   = type;
			levelItem.Evolution   = evolution;
			
			item.sIndex					 = itemId;
			item.EF1					 = ef1;
			item.EFV1					 = efv1;
			item.EF2					 = ef2;
			item.EFV2					 = efv2;
			item.EF3					 = ef3;
			item.EFV3					 = efv3;

			levelItem.item = item;
		}

		fclose(pFile);
		return true;
	}

	return false;
}

 
 
bool ReadBoss()
{
	const std::string filename = "Data\\Boss.xml";
	if (!std::filesystem::exists(filename))
		return false;

	pugi::xml_document doc;
	pugi::xml_parse_result result = doc.load_file(filename.c_str(), pugi::parse_full);
	if (!result)
	{
		std::cout << "parsed with errors, attr value: [" << doc.child("node").attribute("attr").value() << "]\n";
		std::cout << "Error description: " << result.description() << "\n";
		std::cout << "Error offset: " << result.offset << " (error at [..." << (doc.text().as_string() + result.offset) << "]\n\n";

		return false;
	}

	sServer.Boss.fill(STRUCT_MOB_BOSS{});

	auto bossesNode = doc.child("bosses");

	{
		int count = 0;
		for (auto bossNode = bossesNode.child("boss"); bossNode; bossNode = bossNode.next_sibling("boss"))
		{
			auto& boss = sServer.Boss[count];

			boss.sIndex = count++;
			boss.Fame = std::stoi(bossNode.child_value("fame"));
			boss.TimeToReborn = std::stoi(bossNode.child_value("timeToReborn"));
			boss.MaxTimeIngame = std::stoi(bossNode.child_value("maxTimeIngame"));

			for (auto genersNode = bossNode.child("geners"); genersNode; genersNode = genersNode.next_sibling("geners"))
				boss.Geners.emplace_back(std::stoi(genersNode.child_value("gener")));

			if (count >= MaxBoss)
				break;
		}
	}

	return true;
}

void LogGold(INT32 clientId)
{
	if(clientId > 0 && clientId < MAX_PLAYER)
	{
		if(pUser[clientId].Coin == 0)
			return;;

		INT32 total = pUser[clientId].GoldCount;

		Log(clientId, LOG_INGAME, "Recebeu %d de gold em %d mobs no total. Total de gold: %d", pUser[clientId].Coin, total, pMob[clientId].Mobs.Player.Coin);
		LogPlayer(clientId, "Recebeu %d de gold matando %d mobs.", pUser[clientId].Coin, total);

		pUser[clientId].Coin = 0;
		pUser[clientId].GoldCount = 0;
	}
}

void GroupTransfer(INT32 clientId, INT32 mobId)
{
	CMob *newLider = &pMob[mobId];

	int clueParty{ 0 };
	int clueSanc{ 0 };
	bool isClueLeader{ false };
	// itera sobre todas as pistas
	for (int iPista = 0; iPista < 10; ++iPista)
	{
		// itera sobre todos os grupos
		for (int iParty = 0; iParty < MAX_PARTYPISTA; ++iParty)
		{
			auto& pista = pPista[iPista].Clients[iParty];

			int memberId = pista[12];
			if (memberId > 0 && memberId < MAX_PLAYER && clientId == memberId)
			{
				clueParty = iParty;
				clueSanc = iPista;

				isClueLeader = true;
				break;
			}
		}

		if (isClueLeader)
			break;
	}

	if (isClueLeader)
	{
		auto& pista = pPista[clueSanc].Clients[clueParty];
		pista[12] = mobId;

		for (int i = 0; i < 12; ++i)
		{
			if (pPista[clueSanc].Clients[clueParty][i] == mobId)
			{
				pPista[clueSanc].Clients[clueParty][i] = clientId;

				break;
			}
		}

		for (int i = 0; i < 13; ++i)
		{
			int memberId = pista[i];
			if (memberId <= 0 || memberId >= MAX_PLAYER)
				continue;

			SendClientMessage(memberId, "O lader do grupo da pista foi passado para %s", newLider->Mobs.Player.MobName);
			Log(memberId, LOG_INGAME, "Lader passado para o usuario %s (%s)", newLider->Mobs.Player.MobName, pUser[mobId].User.Username);
		}

        Log(clientId, LOG_INGAME, "O lader da pista foi passado para o usuario %s (%s)", newLider->Mobs.Player.MobName, pUser[mobId].User.Username);
	}

	// Salva a info do grupo no novo lader
	memcpy(newLider->PartyList, &pMob[clientId].PartyList[0], sizeof INT16 * 12);

	for(INT32 i = 0; i < 12; i++)
	{
		if(mobId == newLider->PartyList[i])
		{
			newLider->PartyList[i] = clientId;

			break;
		}
	}	

	// Apaga o grupo
	memset(pMob[clientId].PartyList, 0, sizeof INT16 * 12);

	// Informa o novo lider de sua condiaao
	newLider->Leader = 0;
	
	// Informa ao usuario ex-lader de quem a o novo lader
	pMob[clientId].Leader = mobId;

	// Adiciona ele de novo como lider, provavelmente.
	SendAddParty(mobId, mobId, 1);

	// Adiciona o antigo lader como membro normal no grupo
	SendAddParty(clientId, clientId, 0);
	SendAddParty(clientId, mobId, 1);
	SendAddParty(mobId, clientId, 0);
	for (INT32 iterator = 0; iterator < 12; iterator++)
	{
		int partyMob = newLider->PartyList[iterator];
		if (partyMob <= 0 || partyMob > MAX_MOB)
			continue;

		CMob *mob = &pMob[partyMob];

		mob->Leader = mobId;

		SendAddParty(partyMob, clientId, 0);
		SendAddParty(partyMob, mobId, 1);
	}
}

eSellItemResult SellItem(int clientId, STRUCT_ITEM* item)
{
	bool canSell = true;
	switch (item->sIndex)
	{
	case 509:
	case 3993:
	case 3994:
	case 747:
		canSell = false;
		break;
	}

	bool sellAmount = false;
	switch (item->sIndex)
	{
	case 419:
	case 420:
	case 412:
	case 413:
		sellAmount = true;
		break;
	}

	auto _volatile = GetItemAbility(item, EF_VOLATILE);
	if (!canSell || _volatile == 1 || _volatile == 191)
		return eSellItemResult::BlockedItem;

	INT32 cityZone = GetVillage(pMob[clientId].Target.X, pMob[clientId].Target.Y);
	if (cityZone == 5)
		cityZone = 4;

	INT32 perc_impost = g_pCityZone[cityZone].perc_impost;
	INT32 impost = 0;

	STRUCT_ITEMLIST* rItem = &g_pItemList[item->sIndex];
	INT32 itemPrice = (rItem->Price / 4);

	if (itemPrice >= 5001 && itemPrice <= 10000)
		itemPrice = itemPrice * 2 / 3;
	else if (itemPrice > 10000)
		itemPrice /= 2;

	if (sellAmount)
		itemPrice *= GetItemAmount(item);

	if (perc_impost != 0)
	{
		if (sellAmount)
			perc_impost /= 2;

		impost = (itemPrice * perc_impost / 100);
		itemPrice = (itemPrice - impost);
	}

	STRUCT_MOB* mob = &pMob[clientId].Mobs.Player;
	INT64 totalGold = static_cast<INT64>(mob->Coin) + static_cast<INT64>(itemPrice);
	if (totalGold > 2000000000 || totalGold < 0)
		return eSellItemResult::GoldLimit;

	// Arrecada o imposto da cidade
	g_pCityZone[cityZone].impost += impost;

	mob->Coin += itemPrice;

	Log(clientId, LOG_INGAME, "Vendeu o item %s [%d] [%d %d %d %d %d %d] por %d. Gold atual: %d. Valor do imposto: %d", rItem->ItemName, item->sIndex, item->EF1, item->EFV1, item->EF2, item->EFV2, item->EF3, item->EFV3, itemPrice, pMob[clientId].Mobs.Player.Coin, impost);
	LogPlayer(clientId, "Vendeu o item %s na loja %s por %d", rItem->ItemName, mob->MobName, itemPrice, impost);

	memset(item, 0, sizeof STRUCT_ITEM);

	if (impost >= 2000000)
	{
		Log(SERVER_SIDE, LOG_INGAME, "Imposto maior que 2milhaes.");
		Log(SERVER_SIDE, LOG_INGAME, "%s - Vendeu o item %s [%d] [%d %d %d %d %d %d] por %d. Gold atual: %d",pUser[clientId].User.Username, rItem->ItemName, item->sIndex, item->EF1, item->EFV1, item->EF2, item->EFV2, item->EF3, item->EFV3, itemPrice, pMob[clientId].Mobs.Player.Coin);
	}

	SendSignalParm(clientId, clientId, 0x3AF, pMob[clientId].Mobs.Player.Coin);
	return eSellItemResult::Success;
}

bool AgroupItem(int clientId, STRUCT_ITEM* srcItem, STRUCT_ITEM* dstItem)
{
	if (srcItem == NULL || dstItem == NULL || srcItem == dstItem)
		return false;

	if (srcItem->sIndex != dstItem->sIndex)
		return false;

	if (srcItem->sIndex == 4685)
	{
		dstItem->sIndex = 4641;

		Log(clientId, LOG_INGAME, "Juntou 2 %s por %s", g_pItemList[4685].ItemName, g_pItemList[4641].ItemName);
		*srcItem = STRUCT_ITEM{};
		return true;
	}

	int max = GetMaxAmountItem(srcItem);
	if (max <= 0)
		return false;

	if (srcItem->sIndex >= 2390 && srcItem->sIndex <= 2419)
	{
		for (int i = 0; i < 3; i++)
		{
			if (srcItem->stEffect[i].cEffect == 210)
				return false;

			if (dstItem->stEffect[i].cEffect == 210)
				return false;
		}
	}

	for (int i = 0; i < 3; i++)
	{
		if (srcItem->stEffect[i].cEffect == 200)
			return false;
		if (dstItem->stEffect[i].cEffect == 200)
			return false;
	}

	int amountSrc = 0;
	for (int i = 0; i < 3; i++)
		if (srcItem->stEffect[i].cEffect == EF_AMOUNT)
			amountSrc += srcItem->stEffect[i].cValue;

	int amountDst = 0;
	for (int i = 0; i < 3; i++)
		if (dstItem->stEffect[i].cEffect == EF_AMOUNT)
			amountDst += dstItem->stEffect[i].cValue;

	if (amountSrc >= max || amountDst >= max)
		return false;

	Log(clientId, LOG_INGAME, "Agrupando o item %s %s com o item %s %s", g_pItemList[srcItem->sIndex].ItemName, srcItem->toString().c_str(), g_pItemList[dstItem->sIndex].ItemName, dstItem->toString().c_str());

	if (amountSrc == 0)
		amountSrc = 1;

	if (amountDst == 0)
		amountDst = 1;

	int itemIndex = dstItem->sIndex;
	if ((amountSrc + amountDst) <= max)
	{
		memset(srcItem, 0, sizeof STRUCT_ITEM);

		STRUCT_ITEM item;
		memset(&item, 0, sizeof STRUCT_ITEM);

		item.sIndex = itemIndex;

		for (int i = 0; i < 3; i++)
		{
			if (dstItem->stEffect[i].cEffect == EF_AMOUNT)
				continue;
			if (dstItem->stEffect[i].cEffect == EF_UNIQUE)
				continue;

			item.stEffect[i].cEffect = dstItem->stEffect[i].cEffect;
			item.stEffect[i].cValue = dstItem->stEffect[i].cValue;
		}
		int i = 0;
		for (; i < 3; i++)
		{
			if (item.stEffect[i].cEffect != 0)
				continue;

			item.stEffect[i].cEffect = EF_AMOUNT;
			item.stEffect[i].cValue = (amountSrc + amountDst);
			break;
		}

		if (i == 3)
			return false;

		memcpy(dstItem, &item, sizeof STRUCT_ITEM);
	}
	else
	{
		int amount = max - amountDst;
		amountSrc -= amount;

		for (int i = 0; i < 3; i++)
		{
			if (dstItem->stEffect[i].cEffect == EF_AMOUNT)
			{
				dstItem->stEffect[i].cValue = max;
				break;
			}
		}

		int i = 0;
		for (i = 0; i < 3; i++)
		{
			if (srcItem->stEffect[i].cEffect == EF_AMOUNT)
			{
				srcItem->stEffect[i].cValue = amountSrc;
				break;
			}
		}

		if (i == 3)
		{
			for (i = 0; i < 3; i++)
			{
				if (srcItem->stEffect[i].cEffect == 59)
				{
					srcItem->stEffect[i].cEffect = EF_AMOUNT;
					srcItem->stEffect[i].cValue = amountSrc;
					break;
				}
			}
		}
	}

	return true;
}

int AbsorveDamageByPet(CMob* player, int damage) 
{
	if (!player->isPetAlive())
		return damage;

	const STRUCT_ITEM& item = player->Mobs.Player.Equip[14];
	int absPerc = 25;

	if (item.sIndex == 2376) // fenrir
		absPerc = 27;
	else if (item.sIndex == 2377) // dragao feio
		absPerc = 34;
	else if (item.sIndex == 2379) // tf
		absPerc = 35;
	else if (item.sIndex == 2380) // dragao verm.
		absPerc = 35;
	else if (item.sIndex == 2381 || item.sIndex == 2382 || item.sIndex == 2383) // unisus, uni, pegasus
		absPerc = 29;
	else if (item.sIndex == 2384 || item.sIndex == 2385 || item.sIndex == 2386)
		absPerc = 31;
	else if (item.sIndex == 2387 || item.sIndex == 2388)
		absPerc = 28;

	int level = item.stEffect[1].cEffect;
	if (level >= 120)
		absPerc += (level - 120);

	auto damageWithAbs = damage - (damage * absPerc / 100);
	if (damageWithAbs < 1)
		damageWithAbs = 1;

	return damageWithAbs;
}

void EnergizeEmptyRune(int clientId, const p376 *packet)
{
	STRUCT_ITEM* srcItem = GetItemPointer(clientId, packet->SrcType, packet->SrcSlot);
	STRUCT_ITEM* dstItem = GetItemPointer(clientId, packet->DstType, packet->DstSlot);

	if (srcItem == nullptr || dstItem == nullptr)
		return;

	if (srcItem->sIndex < 5110 || srcItem->sIndex > 5133 || dstItem->sIndex != 4854)
		return;

	int runeIndex = srcItem->sIndex - 5109;
	int learn = *(int*)&dstItem->stEffect[1].cEffect;
	if (learn & (1 << runeIndex))
	{
		SendClientMessage(clientId, "Esta Runa ja foi utilizada para energizaaao");

		return;
	}

	int energy = 0;
	for (INT8 i = 0; i < 3; i++)
	{
		if (dstItem->stEffect[i].cEffect == EF_AMOUNT)
		{
			energy = dstItem->stEffect[i].cValue;

			break;
		}
	}

	energy += 5;
	SetItemAmount(dstItem, energy);

	*(int*)&dstItem->stEffect[1].cEffect |= (1 << runeIndex);

	Log(clientId, LOG_INGAME, "Energizou a %s com a %s. Energia total: %d", g_pItemList[dstItem->sIndex].ItemName, g_pItemList[srcItem->sIndex].ItemName, energy);
	*srcItem = STRUCT_ITEM{};

	SendItem(clientId, (SlotType)packet->SrcType, packet->SrcSlot, srcItem);
	SendItem(clientId, (SlotType)packet->DstType, packet->DstSlot, dstItem);
}