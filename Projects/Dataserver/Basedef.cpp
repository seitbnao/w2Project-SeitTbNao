#include "Basedef.h"
#include "Server.h"
#include <ctime>

STRUCT_MOB pBaseSet[4];
char	g_pServerList[MAX_SERVERGROUP][MAX_SERVERNUMBER][64];

INT16 g_pGuildAlly[65535];
INT16 g_pGuildWar[65535];
STRUCT_GUI_INFO GUI;
INT32 ChargedGuildList[MAX_SERVERGROUP][5];
STRUCT_GUILDINFO g_pGuild[MAX_GUILD];
stTowerWar g_pTowerWarState[MAX_SERVERGROUP];
STRUCT_STOREDONATE g_pStore[10][27];

BOOL SendTowerWarInfo(BYTE Info)
{/*
	MSG_STARTTOWERWAR packet;
	memset(&packet, 0, sizeof MSG_STARTTOWERWAR);

	packet.Header.PacketId = _MSG_STARTTOWERWAR;
	packet.Header.Size = sizeof(MSG_STARTTOWERWAR);

	packet.isStarting = Info;

	memcpy(packet.war, g_pTowerWarState, sizeof stTowerWar * 10);

	// Informa os canais das condiçães iniciais da guerra e libera o funcionamento
	for (int i = 0; i < MAX_SERVERGROUP; i++)
	{
		if (pUser[i].Mode == USER_EMPTY)
			continue;

		if (pUser[i].Sock.Sock == INVALID_SOCKET)
			continue;

		packet.Header.ClientId = i;
		pUser[i].Sock.SendOneMessage((char*)&packet, packet.Header.Size);
	}
	*/
	return TRUE;
}

BOOL ReadCharBase(INT32 index, const char *fileName)
{
	FILE *pFile = NULL;

	fopen_s(&pFile, fileName, "rb");
	if (!pFile)
		return NULL;

	fread(&pBaseSet[index], sizeof STRUCT_MOB, 1, pFile);
	fclose(pFile);

	return true;
}

BOOL InitializeServerList()
{
	FILE *pFile = NULL;
	fopen_s(&pFile, "serverlist.txt", "rt");

	if (!pFile)
		fopen_s(&pFile, "../../serverlist.txt", "rt");

	if (!pFile)
	{
		MessageBox(NULL, "Can't open serverlist.txt", "-system", MB_OK | MB_APPLMODAL);
		return false;
	}

	memset(g_pServerList, 0, sizeof g_pServerList);

	char str[256];
	char address[64];
	while (1)
	{
		char *ret = fgets(str, 255, pFile);
		if (ret == NULL)
			break;

		INT32 serverGroup = -1;
		INT32 serverNumber = -1;
		address[0] = 0;

		sscanf_s(str, "%d %d %s", &serverGroup, &serverNumber, address, 64);

		if (serverGroup < 0 || serverGroup >= MAX_SERVERGROUP || serverNumber < 0 || serverNumber >= MAX_SERVERNUMBER)
			continue;

		strncpy_s(g_pServerList[serverGroup][serverNumber], address, 64);
	}

	return true;
}

int Rand()
{
	static long long i = 115;

	i = ((i * 214013) + 253101111);
	return ((i >> 16) & 0x7FFF);
}

void GetFirstKey(const char *source, char *dest)
{
	if ((source[0] >= 'A'&& source[0] <= 'Z') || (source[0] >= 'a'&& source[0] <= 'z'))
	{
		dest[0] = source[0];
		dest[1] = 0;
	}
	else
		strncpy_s(dest, 4, "etc", 3);
}

INT32 GetUserFromSocket(INT32 soc)
{
	for (INT32 i = 0; i < MAX_SERVERGROUP; i++)
	{
		if (pUser[i].Sock.Sock == soc)
			return i;
	}

	return -1;
}

void ClearItem(STRUCT_ITEM *item)
{
	item->sIndex = 0;
	item->EF1 = 0;
	item->EF2 = 0;
	item->EF3 = 0;
	item->EFV1 = 0;
	item->EFV2 = 0;
	item->EFV3 = 0;
}

void ClearMob(STRUCT_MOB *player)
{
	memset(player, 0, sizeof STRUCT_MOB);

	player->Last.X = 2100;
	player->Last.Y = 2100;

	memset(&player->BaseScore, 0, sizeof STRUCT_SCORE);
	memset(&player->CurrentScore, 0, sizeof STRUCT_SCORE);

	for (INT32 i = 0; i < 16; i++)
		ClearItem(&player->Equip[i]);

	for (INT32 i = 0; i < 64; i++)
		ClearItem(&player->Inventory[i]);

	player->ShortSkill[0] = -1;
	player->ShortSkill[1] = -1;
	player->ShortSkill[2] = -1;
	player->ShortSkill[3] = -1;
}

INT32 InitializeBaseDef()
{
	memset(ChargedGuildList, 0, sizeof ChargedGuildList);

	InitializeServerList();
	return true;
}

INT32 ReadConfig()
{
	FILE *pFile = NULL;
	fopen_s(&pFile, "Config.txt", "rt");

	if (pFile)
	{
		char str[255] = { 0 };
		while (fgets(str, 255, pFile))
		{
			char cmd[64];
			int value = 0;
			if (fscanf_s(pFile, "%s %d", cmd, 64, &value) != 2)
				continue;

			std::string command{ cmd };
			if (command == "Sapphire")
				sServer.Sapphire = value;
			else if (command == "LastSeal")
				sServer.LastSeal = value;
		}

		fclose(pFile);
		return true;
	}

	return false;
}

INT32 WriteConfig()
{
	FILE *pFile = NULL;
	fopen_s(&pFile, "Config.txt", "w+");

	if (pFile)
	{
		fprintf(pFile, "Sapphire %d\r\n", sServer.Sapphire);
		fprintf(pFile, "LastSeal %d\r\n", sServer.LastSeal);
		fclose(pFile);
		return true;
	}

	return false;
}

INT32 ReadGuilds()
{
	FILE *pFile = NULL;
	fopen_s(&pFile, "guilds.txt", "rt");

	if (!pFile)
		fopen_s(&pFile, "..\\guilds.txt", "rt");

	if (!pFile)
	{
		MessageBox(NULL, "Can't open guilds.txt", "-system", MB_OK | MB_APPLMODAL);
		return false;
	}

	memset(g_pGuild, 0, sizeof g_pGuild);

	char str[256];
	while (true)
	{
		char *ret = fgets(str, 255, pFile);
		if (ret == NULL)
			break;

		if (str[0] == '\n' || str[0] == '#')
			continue;

		INT32 fameGuild = 0,
			guildId = 0,
			kingdom = 0,
			citizen = 0,
			wins = 0;

		char guildName[32],
			subName[3][32];

		guildName[0] = 0;
		memset(subName, 0, sizeof subName);

		INT32 rtn = sscanf_s(str, "%d, %d, %d, %d, %d, %[^,], %[^,], %[^,], %[^,]", &guildId, &fameGuild, &kingdom, &citizen, &wins,
			guildName, 16, subName[0], 32, subName[1], 32, subName[2], 32);

		if (rtn < 5)
		{
			MessageBoxA(NULL, "Can't parse strings on Guilds.txt", "error", MB_OK);
			continue;
		}

		if (guildId < 0 || guildId >= MAX_GUILD)
			continue;

		if (fameGuild < 0)
			fameGuild = 0;

		g_pGuild[guildId].Citizen = citizen;
		g_pGuild[guildId].Fame = fameGuild;
		g_pGuild[guildId].Kingdom = kingdom;
		g_pGuild[guildId].Wins = wins;

		g_pGuild[guildId].MobName = std::string(guildName);

		for (INT32 i = 0; i < 3; i++)
		{
			if (!subName[i][0])
				continue;

			g_pGuild[guildId].SubGuild[i] = std::string(subName[i]);
		}
	}

	fclose(pFile);
	return true;
}

INT32 WriteGuilds()
{
	FILE *pFile = NULL;
	fopen_s(&pFile, "guilds.txt", "w+");

	if (pFile)
	{
		fprintf(pFile, "#id, fame, kingdom, citizen, name, sub1, sub2, sub3\n");
		fprintf(pFile, "0, 1, 1, 1, 1, 1, 1, 1\n");

		for (INT32 i = 1; i < MAX_GUILD; i++)
		{
			if (g_pGuild[i].MobName.empty())
				continue;

			STRUCT_GUILDINFO *g = &g_pGuild[i];
			fprintf(pFile, "%d, %d, %d, %d, %d, %s, %s, %s, %s\n", i, g->Fame, g->Kingdom, g->Citizen, g->Wins, g->MobName.c_str(), g->SubGuild[0].c_str(), g->SubGuild[1].c_str(), g->SubGuild[2].c_str());
		}

		fclose(pFile);
		return true;
	}

	return false;
}

INT32 DecideWinnerTowerWar()
{/*
	for(INT32 conn = 0; conn < MAX_SERVERGROUP; conn += 2)
	{
		int otherConn = ((conn % 2) ? (conn - 1) : (conn + 1));

		INT32 winner = -1;

		// o canal esta avanêando no canal 1 e no canal 2 pois o TowerState dos dois canais sêo diferentes dos
		if(g_pTowerWarState[conn].TowerState == otherConn && g_pTowerWarState[otherConn].TowerState == otherConn)
			winner = conn;
		else if(g_pTowerWarState[conn].TowerState == conn && g_pTowerWarState[otherConn].TowerState == conn)
			winner = otherConn;

		_MSG_RESULTWARTOWER packet;
		memset(&packet, 0, sizeof packet);

		packet.Header.PacketId = MSG_RESULTWARTOWER_OPCODE;
		packet.Header.Size = sizeof _MSG_RESULTWARTOWER;

		packet.Winner = winner;

		pUser[conn].Sock.AddMessage((char*)(&packet), sizeof _MSG_RESULTWARTOWER);
		pUser[otherConn].Sock.AddMessage((char*)(&packet), sizeof _MSG_RESULTWARTOWER);
	}
	*/
	return true;
}

void SetFame(INT32 guildId, INT32 value)
{
	MSG_ADDGUILD packet;
	memset(&packet, 0, sizeof packet);

	packet.Header.PacketId = MSG_ADDGUILD_OPCODE;
	packet.Header.Size = sizeof MSG_ADDGUILD;

	packet.guildIndex = guildId;
	packet.Type = 0;
	packet.Value = value;

	for (INT32 i = 0; i < MAX_SERVERGROUP; i++)
	{
		if (pUser[i].Mode == 0 || pUser[i].Sock.Sock == 0)
			continue;

		pUser[i].Sock.SendOneMessage((char*)&packet, sizeof MSG_ADDGUILD);
	}

	g_pGuild[guildId].Fame = value;
}

INT32 WriteNPCDonate()
{
	// O NPC tambêm ê salvo caso  oservidor seja desligado
	// a quantidade de itens disponêveis na loja continue sempre o mesmo
	// Sussa? xD
	FILE *pFile = nullptr;

	fopen_s(&pFile, "Donate.txt", "w+");
	if (pFile)
	{
		fprintf(pFile, "# Loja\n# 0 = armia\n# 1 = arzan\n# 2 = erion\n# 3 = karden\n# 5 = noatun\n\n#loja, index, avaible, price, loop, item info\n");

		for (INT32 i = 0; i < 10; i++)
		{
			for (INT32 t = 0; t < 27; t++)
			{
				if (g_pStore[i][t].item.sIndex <= 0)
					continue;

				fprintf(pFile, "%d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d\n", i, t, g_pStore[i][t].Avaible, g_pStore[i][t].Price, g_pStore[i][t].Loop,
					g_pStore[i][t].item.sIndex, g_pStore[i][t].item.EF1, g_pStore[i][t].item.EFV1, g_pStore[i][t].item.EF2, g_pStore[i][t].item.EFV2, g_pStore[i][t].item.EF3, g_pStore[i][t].item.EFV3);
			}
		}

		fclose(pFile);
		return true;
	}

	return false;
}

INT32 ReadNPCDonate()
{
	FILE *pFile;
	fopen_s(&pFile, "Donate.txt", "r");

	memset(g_pStore, 0, sizeof g_pStore);

	if (pFile)
	{
		char line[1024];

		while (fgets(line, sizeof line, pFile))
		{
			if (*line == '#' || *line == '\n')
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

			if (ret < 6)
				continue;

			if (store < 0 || store >= 5 || index >= 27 || index < 0)
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

void Log(char *username, const char *msg, ...)
{
	FILE *pFile = NULL;

	char szFileName[256];
	__try
	{
		time_t rawnow = time(NULL);
		struct tm now;
		localtime_s(&now, &rawnow);

		char first[16];
		GetFirstKey(username, first);

		sprintf_s(szFileName, "..\\Logs\\Players\\%s\\%s - %02d-%02d-%02d.txt", first, username, now.tm_mday, (now.tm_mon + 1), (now.tm_year - 100));

		// Abre o arquivo de log 
		fopen_s(&pFile, szFileName, "a+");

		if (pFile)
		{
			// Inicia a lista de argumentos
			va_list arglist;

			// Insere a hora no arquivo
			fprintf(pFile, "\n%02d:%02d:%02d : [DBSRV] ",
				now.tm_hour, now.tm_min, now.tm_sec);

			va_start(arglist, msg);
			vfprintf(pFile, msg, arglist);

			va_end(arglist);
			fclose(pFile);
		}
	}
	__except (1)
	{
		if (pFile)
			fclose(pFile);
	}
}


void AppendStructure(pugi::xml_node node, const STRUCT_ITEM* item)
{
	node.append_attribute("itemId").set_value(item->sIndex);

	for (int i = 0; i < 3; i++)
	{
		std::string childEffectName = "ef" + std::to_string(i);
		std::string childEffectValueName = "efv" + std::to_string(i);

		node.append_attribute(childEffectName.c_str()).set_value(item->stEffect[i].cEffect);
		node.append_attribute(childEffectValueName.c_str()).set_value(item->stEffect[i].cValue);
	}
}

void AppendStructure(pugi::xml_node node, const STRUCT_POSITION* position)
{
	node.append_attribute("x").set_value(position->X);
	node.append_attribute("y").set_value(position->Y);
}

void AppendStructure(pugi::xml_node node, const STRUCT_ITEMDATE* date)
{
	node.append_child("day").append_child(pugi::node_pcdata).set_value(std::to_string(date->Dia).c_str());
	node.append_child("month").append_child(pugi::node_pcdata).set_value(std::to_string(date->Mes).c_str());
	node.append_child("year").append_child(pugi::node_pcdata).set_value(std::to_string(date->Ano).c_str());
	node.append_child("hour").append_child(pugi::node_pcdata).set_value(std::to_string(date->Hora).c_str());
	node.append_child("minute").append_child(pugi::node_pcdata).set_value(std::to_string(date->Minuto).c_str());
	node.append_child("second").append_child(pugi::node_pcdata).set_value(std::to_string(date->Segundo).c_str());
}

void AppendStructure(pugi::xml_node node, const STRUCT_AFFECT* affect)
{
	node.append_attribute("index").set_value(affect->Index);
	node.append_attribute("master").set_value(affect->Master);
	node.append_attribute("value").set_value(affect->Value);
	node.append_attribute("time").set_value(affect->Time);
}

void AppendStructure(pugi::xml_node mob, STRUCT_SCORE* status)
{
	mob.append_child("level").append_child(pugi::node_pcdata).set_value(std::to_string(status->Level).c_str());
	mob.append_child("defense").append_child(pugi::node_pcdata).set_value(std::to_string(status->Ac).c_str());
	mob.append_child("attack").append_child(pugi::node_pcdata).set_value(std::to_string(status->Damage).c_str());
	mob.append_child("merchant").append_child(pugi::node_pcdata).set_value(std::to_string(status->Merchant.Value).c_str());
	mob.append_child("move").append_child(pugi::node_pcdata).set_value(std::to_string(status->Move.Value).c_str());
	mob.append_child("maxHp").append_child(pugi::node_pcdata).set_value(std::to_string(status->MaxHp).c_str());
	mob.append_child("maxMp").append_child(pugi::node_pcdata).set_value(std::to_string(status->MaxMp).c_str());
	mob.append_child("curHp").append_child(pugi::node_pcdata).set_value(std::to_string(status->Hp).c_str());
	mob.append_child("curMp").append_child(pugi::node_pcdata).set_value(std::to_string(status->Mp).c_str());
	mob.append_child("str").append_child(pugi::node_pcdata).set_value(std::to_string(status->Str).c_str());
	mob.append_child("int").append_child(pugi::node_pcdata).set_value(std::to_string(status->Int).c_str());
	mob.append_child("des").append_child(pugi::node_pcdata).set_value(std::to_string(status->Dex).c_str());
	mob.append_child("con").append_child(pugi::node_pcdata).set_value(std::to_string(status->Con).c_str());

	auto mastery = mob.append_child("mastery");
	mastery.append_attribute("mastery0").set_value(status->Special[0]);
	mastery.append_attribute("mastery1").set_value(status->Special[1]);
	mastery.append_attribute("mastery2").set_value(status->Special[2]);
	mastery.append_attribute("mastery3").set_value(status->Special[3]);
}

void AppendStructure(pugi::xml_node mob, STRUCT_MOB* mobInfo)
{
	mob.append_child("name").append_child(pugi::node_pcdata).set_value(mobInfo->MobName);
	mob.append_child("capeInfo").append_child(pugi::node_pcdata).set_value(std::to_string(mobInfo->CapeInfo).c_str());
	mob.append_child("info").append_child(pugi::node_pcdata).set_value(std::to_string(mobInfo->Info.Value).c_str());
	mob.append_child("questInfo").append_child(pugi::node_pcdata).set_value(std::to_string(mobInfo->QuestInfo.Value).c_str());
	mob.append_child("guildIndex").append_child(pugi::node_pcdata).set_value(std::to_string(mobInfo->Guild).c_str());
	mob.append_child("classInfo").append_child(pugi::node_pcdata).set_value(std::to_string(mobInfo->Class).c_str());
	mob.append_child("affectInfo").append_child(pugi::node_pcdata).set_value(std::to_string(mobInfo->AffectInfo.Value).c_str());
	mob.append_child("gold").append_child(pugi::node_pcdata).set_value(std::to_string(mobInfo->Coin).c_str());
	mob.append_child("experience").append_child(pugi::node_pcdata).set_value(std::to_string(mobInfo->Exp).c_str());
	AppendStructure(mob.append_child("lastPosition"), &mobInfo->Last);

	{
		auto baseStatus = mob.append_child("baseStatus");
		auto currentStatus = mob.append_child("currentStatus");

		AppendStructure(baseStatus, &mobInfo->BaseScore);
		AppendStructure(currentStatus, &mobInfo->CurrentScore);
	}

	{
		auto equips = mob.append_child("equips");

		for (int i = 0; i < 16; i++)
		{
			auto equip = equips.append_child("item");
			equip.append_attribute("slot").set_value(i);

			AppendStructure(equip, &mobInfo->Equip[i]);
		}
	}

	{
		auto inventory = mob.append_child("inventory");

		for (int i = 0; i < 64; i++)
		{
			if (mobInfo->Inventory[i].sIndex == 0)
				continue;

			auto invItem = inventory.append_child("item");
			invItem.append_attribute("slot").set_value(i);

			AppendStructure(invItem, &mobInfo->Inventory[i]);
		}
	}

	mob.append_child("learn").append_child(pugi::node_pcdata).set_value(std::to_string(mobInfo->LearnedSkill[0]).c_str());
	mob.append_child("secLearn").append_child(pugi::node_pcdata).set_value(std::to_string(mobInfo->LearnedSkill[1]).c_str());
	mob.append_child("statusPoint").append_child(pugi::node_pcdata).set_value(std::to_string(mobInfo->ScoreBonus).c_str());
	mob.append_child("masterPoint").append_child(pugi::node_pcdata).set_value(std::to_string(mobInfo->SpecialBonus).c_str());
	mob.append_child("skillPoint").append_child(pugi::node_pcdata).set_value(std::to_string(mobInfo->SkillPoint).c_str());
	mob.append_child("critical").append_child(pugi::node_pcdata).set_value(std::to_string(mobInfo->Critical).c_str());
	mob.append_child("saveMana").append_child(pugi::node_pcdata).set_value(std::to_string(mobInfo->SaveMana).c_str());

	{
		auto skillbar1 = mob.append_child("skillbar1");
		for (int i = 0; i < 4; i++)
		{
			auto sk = skillbar1.append_child("sk");
			sk.append_attribute("index").set_value(i);
			sk.append_child(pugi::node_pcdata).set_value(std::to_string(mobInfo->ShortSkill[i]).c_str());
		}
	}

	mob.append_child("guildMemberType").append_child(pugi::node_pcdata).set_value(std::to_string(mobInfo->GuildLevel).c_str());
}

void AppendStructure(pugi::xml_node mob, STRUCT_CHARINFO* charInfo)
{
	{
		auto player = mob.append_child("player");
		AppendStructure(player, &charInfo->Player);
	}
	{
		auto affects = mob.append_child("affects");

		for (int i = 0; i < 32; i++)
		{
			auto affect = affects.append_child("affect");
			affect.append_attribute("skillIndex").set_value(i);

			AppendStructure(affect, &charInfo->Affects[i]);
		}
	}

	mob.append_child("info").append_child(pugi::node_pcdata).set_value(std::to_string(charInfo->Info.Value).c_str());
	mob.append_child("hallEnter").append_child(pugi::node_pcdata).set_value(std::to_string(charInfo->HallEnter).c_str());
	mob.append_child("fame").append_child(pugi::node_pcdata).set_value(std::to_string(charInfo->Fame).c_str());
	mob.append_child("pesaEnter").append_child(pugi::node_pcdata).set_value(std::to_string(charInfo->PesaEnter).c_str());
	mob.append_child("citizen").append_child(pugi::node_pcdata).set_value(std::to_string(charInfo->Citizen).c_str());
	mob.append_child("soul").append_child(pugi::node_pcdata).set_value(std::to_string(charInfo->Soul).c_str());
	mob.append_child("hold").append_child(pugi::node_pcdata).set_value(std::to_string(charInfo->Hold).c_str());

	auto skillBar = mob.append_child("skillbar2");
	for (int i = 0; i < 16; i++)
	{
		auto sk = skillBar.append_child("sk");
		sk.append_attribute("index").set_value(i);
		sk.append_child(pugi::node_pcdata).set_value(std::to_string(charInfo->SkillBar[i]).c_str());
	}

	mob.append_child("mortalSlot").append_child(pugi::node_pcdata).set_value(std::to_string(charInfo->MortalSlot).c_str());
	mob.append_child("rvrPoints").append_child(pugi::node_pcdata).set_value(std::to_string(charInfo->RvRPoints).c_str());

	AppendStructure(mob.append_child("saude"), &charInfo->Saude);
	AppendStructure(mob.append_child("escritura"), &charInfo->Escritura);
	AppendStructure(mob.append_child("revigorante"), &charInfo->Revigorante);

	AppendStructure(mob.append_child("lastKickOut"), &charInfo->LastGuildKickOut);

	// sub
	{
		auto sub = mob.append_child("sub");
		sub.append_attribute("status").set_value(charInfo->Sub.CurrentScore);

		AppendStructure(sub.append_child("status"), &charInfo->Sub.SubStatus);

		auto items = sub.append_child("equips");

		for (int i = 0; i < 2; i++)
		{
			auto item = items.append_child("equip");
			item.append_attribute("index").set_value(i);

			AppendStructure(item, &charInfo->Sub.Equip[i]);
		}

		sub.append_child("experience").append_child(pugi::node_pcdata).set_value(std::to_string(charInfo->Sub.Exp).c_str());
		sub.append_child("learn").append_child(pugi::node_pcdata).set_value(std::to_string(charInfo->Sub.LearnedSkill).c_str());
		sub.append_child("secLearn").append_child(pugi::node_pcdata).set_value(std::to_string(charInfo->Sub.SecLearn).c_str());

		auto skillbar = sub.append_child("skillbar");
		for (int i = 0; i < 20; i++)
		{
			auto sk = skillbar.append_child("sk");

			sk.append_attribute("index").set_value(i);
			sk.append_child(pugi::node_pcdata).set_value(std::to_string(charInfo->Sub.SkillBar[i]).c_str());
		}

		auto affects = sub.append_child("affects");
		for (int i = 0; i < 32; i++)
		{
			auto affect = affects.append_child("affect");
			affect.append_attribute("skillIndex").set_value(i);

			AppendStructure(affect, &charInfo->Sub.Affect[i]);
		}

		sub.append_child("questInfo").append_child(pugi::node_pcdata).set_value(std::to_string(charInfo->Sub.Info.Value).c_str());
		sub.append_child("soul").append_child(pugi::node_pcdata).set_value(std::to_string(charInfo->Sub.Soul).c_str());
	}

	{
		auto nightmares = mob.append_child("nightmares");

		for (int i = 0; i < 3; i++)
		{
			auto nightmare = nightmares.append_child("nightmare");
			nightmare.append_attribute("index").set_value(i);

			AppendStructure(nightmare, &charInfo->Nightmare[i]);
		}
	}
}

void AppendStructure(pugi::xml_node account, STRUCT_ACCOUNT* acc)
{
	account.append_child("username").append_child(pugi::node_pcdata).set_value(acc->Username);
	account.append_child("password").append_child(pugi::node_pcdata).set_value(acc->Password);
	account.append_child("cash").append_child(pugi::node_pcdata).set_value(std::to_string(acc->Cash).c_str());
	account.append_child("alreadyReseted").append_child(pugi::node_pcdata).set_value(std::to_string(acc->AlreadyReseted).c_str());

	{
		auto ban = account.append_child("ban");
		ban.append_child("type").append_child(pugi::node_pcdata).set_value(std::to_string(acc->BanType).c_str());

		AppendStructure(ban.append_child("date"), &acc->Ban);
	}

	if(acc->TempKey[0])
		account.append_child("tempkey").append_child(pugi::node_pcdata).set_value(acc->TempKey);

	account.append_child("token").append_child(pugi::node_pcdata).set_value(acc->SecondPass);
	account.append_child("access").append_child(pugi::node_pcdata).set_value(std::to_string(acc->AccessLevel).c_str());

	{
		for (int i = 0; i < 4; i++)
		{
			if (!acc->Mob[i].Player.MobName[0])
				continue;

			auto mob = account.append_child("mob");
			mob.append_attribute("index").set_value(i);

			AppendStructure(mob, &acc->Mob[i]);
		}
	}
	{
		auto storage = account.append_child("storage");
		storage.append_child("gold").append_child(pugi::node_pcdata).set_value(std::to_string(acc->Storage.Coin).c_str());

		auto storageItems = storage.append_child("items");
		for (int i = 0; i < 128; i++)
		{
			if (acc->Storage.Item[i].sIndex == 0)
				continue;

			auto storageItem = storageItems.append_child("item");
			storageItem.append_attribute("slot").set_value(i);

			AppendStructure(storageItem, &acc->Storage.Item[i]);
		}
	}
	{
		auto blockPassword = account.append_child("block");
		blockPassword.append_attribute("password").set_value(acc->Block.Pass);
		blockPassword.append_attribute("status").set_value(acc->Block.Blocked);
	}

	account.append_child("insignia").append_child(pugi::node_pcdata).set_value(std::to_string(acc->Insignias.Value).c_str());

	account.append_child("charslot").append_child(pugi::node_pcdata).set_value(std::to_string(acc->CharSlot).c_str());
	AppendStructure(account.append_child("position"), &acc->Position);

	{
		auto water = account.append_child("water");
		water.append_child("day").append_child(pugi::node_pcdata).set_value(std::to_string(acc->Water.Day).c_str());
		water.append_child("total").append_child(pugi::node_pcdata).set_value(std::to_string(acc->Water.Total).c_str());
	}

	AppendStructure(account.append_child("divina"), &acc->Divina);
	AppendStructure(account.append_child("sephira"), &acc->Sephira);
}


void XMLToStructure(pugi::xml_node node, STRUCT_AFFECT& affect)
{
	affect.Index = node.attribute("index").as_int();
	affect.Master = node.attribute("master").as_int();
	affect.Value = node.attribute("value").as_int();
	affect.Time = node.attribute("time").as_int();
}

void XMLToStructure(pugi::xml_node node, STRUCT_ITEM& item)
{
	item.sIndex = node.attribute("itemId").as_int();

	for (int i = 0; i < 3; i++)
	{
		std::string childEffectName = "ef" + std::to_string(i);
		std::string childEffectValueName = "efv" + std::to_string(i);

		item.stEffect[i].cEffect = node.attribute(childEffectName.c_str()).as_int();
		item.stEffect[i].cValue = node.attribute(childEffectValueName.c_str()).as_int();
	}
}

void XMLToStructure(pugi::xml_node node, STRUCT_SCORE& status)
{
	if (node == nullptr)
		return;

	status.Level = std::stoul(node.child_value("level"));
	status.Ac = std::stoul(node.child_value("defense"));
	status.Damage = std::stoul(node.child_value("attack"));
	status.Merchant.Value = std::stoul(node.child_value("merchant"));
	status.Move.Value = std::stoi(node.child_value("move"));
	status.MaxHp = std::stoi(node.child_value("maxHp"));
	status.MaxMp = std::stoi(node.child_value("maxMp"));
	status.Hp = std::stoi(node.child_value("curHp"));
	status.Mp = std::stoi(node.child_value("curMp"));
	status.Str = std::stoi(node.child_value("str"));
	status.Int = std::stoi(node.child_value("int"));
	status.Con = std::stoi(node.child_value("con"));
	status.Dex = std::stoi(node.child_value("des"));

	auto mastery = node.child("mastery");
	status.Special[0] = mastery.attribute("mastery0").as_int();
	status.Special[1] = mastery.attribute("mastery1").as_int();
	status.Special[2] = mastery.attribute("mastery2").as_int();
	status.Special[3] = mastery.attribute("mastery3").as_int();
}

void XMLToStructure(pugi::xml_node node, STRUCT_POSITION& position)
{
	position.X = node.attribute("x").as_int();
	position.Y = node.attribute("y").as_int();
}

void XMLToStructure(pugi::xml_node node, STRUCT_ITEMDATE& date)
{
	if (node == nullptr)
	{
		date = STRUCT_ITEMDATE{};
		return;
	}

	date.Dia = std::stoul(node.child_value("day"));
	date.Mes = std::stoul(node.child_value("month"));
	date.Ano = std::stoul(node.child_value("year"));
	date.Hora = std::stoul(node.child_value("hour"));
	date.Minuto = std::stoul(node.child_value("minute"));
	date.Segundo = std::stoul(node.child_value("second"));
}

void XMLToStructure(pugi::xml_node node, STRUCT_SUBINFO& sub)
{
	auto affects = node.child("affects");

	for (auto affect = affects.child("affect"); affect; affect = affect.next_sibling("affect"))
	{
		auto skillNode = affect.attribute("skillIndex");
		if (skillNode == nullptr)
			skillNode = affect.attribute("inex");

		if (skillNode == nullptr)
			continue;

		int skillIndex = skillNode.as_int();
		if (skillIndex < 0 || skillIndex > 32)
			continue;

		XMLToStructure(affect, sub.Affect[skillIndex]);
	}

	{
		auto equips = node.child("equips");

		for (auto equipItem = equips.child("equip"); equipItem; equipItem = equipItem.next_sibling("equip"))
		{
			int itemSlot = equipItem.attribute("index").as_int();
			if (itemSlot < 0 || itemSlot >= 1)
				continue;

			XMLToStructure(equipItem, sub.Equip[itemSlot]);
		}
	}

	sub.Exp = std::stoll(node.child_value("experience"));
	sub.Info.Value = std::stoll(node.child_value("questInfo"));
	sub.LearnedSkill = std::stoul(node.child_value("learn"));
	sub.SecLearn = std::stoul(node.child_value("secLearn"));

	{
		auto skillBar = node.child("skillbar");

		for (auto skill = skillBar.child("sk"); skill; skill = skill.next_sibling("sk"))
		{
			int index = skill.attribute("index").as_int();
			char value = std::stoi(skill.child_value());

			if (index < 0 || index >= 20)
				continue;

			sub.SkillBar[index] = value;
		}
	}

	sub.Soul = static_cast<unsigned char>(std::stoi(node.child_value("soul")));
	sub.CurrentScore = node.attribute("status").as_int();

	XMLToStructure(node.child("status"), sub.SubStatus);
}

void XMLToStructure(pugi::xml_node node, STRUCT_CHARINFO& charInfo)
{
	{
		STRUCT_MOB* player = &charInfo.Player;
		auto playerNode = node.child("player");

		strncpy_s(player->MobName, playerNode.child_value("name"), 15);
		player->CapeInfo = std::stoi(playerNode.child_value("capeInfo"));
		player->Info.Value = std::stoi(playerNode.child_value("info"));
		player->Guild = std::stoi(playerNode.child_value("guildIndex"));
		player->Class = std::stoi(playerNode.child_value("classInfo"));
		player->AffectInfo.Value = std::stoi(playerNode.child_value("affectInfo"));

		if (!std::string(playerNode.child_value("questInfo")).empty())
			player->QuestInfo.Value = std::stoi(playerNode.child_value("questInfo"));

		player->Exp = std::stoll(playerNode.child_value("experience"));
		player->Coin = std::stoi(playerNode.child_value("gold"));

		XMLToStructure(playerNode.child("lastPosition"), player->Last);
		XMLToStructure(playerNode.child("baseStatus"), player->BaseScore);
		XMLToStructure(playerNode.child("currentStatus"), player->CurrentScore);

		auto equips = playerNode.child("equips");
		for (auto equipItem = equips.child("item"); equipItem; equipItem = equipItem.next_sibling("item"))
		{
			int itemSlot = equipItem.attribute("slot").as_int();

			XMLToStructure(equipItem, player->Equip[itemSlot]);
		}

		auto inventory = playerNode.child("inventory");
		for (auto inventoryItem = inventory.child("item"); inventoryItem; inventoryItem = inventoryItem.next_sibling("item"))
		{
			int itemSlot = inventoryItem.attribute("slot").as_int();

			XMLToStructure(inventoryItem, player->Inventory[itemSlot]);
		}

		player->LearnedSkill[0] = std::stoul(playerNode.child_value("learn"));
		player->LearnedSkill[1] = std::stoul(playerNode.child_value("secLearn"));
		player->ScoreBonus = (unsigned short)std::stoul(playerNode.child_value("statusPoint"));
		player->SpecialBonus = (unsigned short)std::stoul(playerNode.child_value("masterPoint"));
		player->SkillPoint = (unsigned short)std::stoul(playerNode.child_value("skillPoint"));

		{
			auto skillBar = playerNode.child("skillbar1");

			for (auto skill = skillBar.child("sk"); skill; skill = skill.next_sibling("sk"))
			{
				int index = skill.attribute("index").as_int();
				char value = std::stoi(skill.child_value());

				if (index < 0 || index >= 4)
					continue;

				player->ShortSkill[index] = value;
			}
		}

		player->GuildLevel = std::stoi(playerNode.child_value("guildMemberType"));
	}

	{
		auto affects = node.child("affects");

		for (auto affect = affects.child("affect"); affect; affect = affect.next_sibling("affect"))
		{
			int skillIndex = affect.attribute("skillIndex").as_int();
			if (skillIndex < 0 || skillIndex > 32)
				continue;

			XMLToStructure(affect, charInfo.Affects[skillIndex]);
		}
	}

	charInfo.Info.Value = std::stoll(node.child_value("info"));
	charInfo.HallEnter = std::stoi(node.child_value("hallEnter"));
	charInfo.Fame = std::stoul(node.child_value("fame"));
	charInfo.PesaEnter = std::stoi(node.child_value("pesaEnter"));
	charInfo.Citizen = std::stoi(node.child_value("citizen"));
	charInfo.Soul = (unsigned char)std::stoul(node.child_value("soul"));
	charInfo.Hold = std::stoll(node.child_value("hold"));
	charInfo.MortalSlot = std::stoi(node.child_value("mortalSlot"));

	if (!std::string(node.child_value("rvrPoints")).empty())
		charInfo.RvRPoints = std::stoi(node.child_value("rvrPoints"));

	XMLToStructure(node.child("sub"), charInfo.Sub);

	{
		auto skillBar = node.child("skillbar2");

		for (auto skill = skillBar.child("sk"); skill; skill = skill.next_sibling("sk"))
		{
			int index = skill.attribute("index").as_int();
			char value = std::stoi(skill.child_value());

			if (index < 0 || index >= 16)
				continue;

			charInfo.SkillBar[index] = value;
		}
	}

	XMLToStructure(node.child("saude"), charInfo.Saude);
	XMLToStructure(node.child("escritura"), charInfo.Escritura);
	XMLToStructure(node.child("revigorante"), charInfo.Revigorante);
	XMLToStructure(node.child("lastKickOut"), charInfo.LastGuildKickOut);

	{
		auto nightmares = node.child("nightmares");

		for (auto nightmare = nightmares.child("nightmare"); nightmare; nightmare = nightmare.next_sibling("nightmare"))
		{
			int index = nightmare.attribute("index").as_int();
			if (index < 0 || index >= 3)
				continue;

			XMLToStructure(nightmare, charInfo.Nightmare[index]);
		}
	}
}


void XMLToStructure(pugi::xml_node accNode, STRUCT_ACCOUNT* file)
{
	strncpy_s(file->Username, accNode.child_value("username"), 15);
	strncpy_s(file->Password, accNode.child_value("password"), 15);

	if (accNode.child_value("alreadyReseted") != nullptr)
	{
		std::string alreadyReseted = accNode.child_value("alreadyReseted");
		if (!alreadyReseted.empty())
			file->AlreadyReseted = alreadyReseted == "0" ? false : true;
	}

	{
		auto tempKeyNode = accNode.child("tempkey");
		if (tempKeyNode != nullptr)
		{
			int len = strlen(tempKeyNode.child_value());
			strncpy_s(file->TempKey, tempKeyNode.child_value(), len);
		}
	}
	file->Cash = std::stoi(accNode.child_value("cash"));

	{
		auto ban = accNode.child("ban");
		file->BanType = std::stoi(ban.child_value("type"));

		XMLToStructure(ban.child("date"), file->Ban);
	}

	strncpy_s(file->SecondPass, accNode.child_value("token"), 16);
	file->AccessLevel = std::stoi(accNode.child_value("access"));

	for (auto mob = accNode.child("mob"); mob; mob = mob.next_sibling("mob"))
	{
		int index = mob.attribute("index").as_int();
		if (index < 0 || index >= 4)
			continue;

		XMLToStructure(mob, file->Mob[index]);
	}

	{
		auto block = accNode.child("block");

		strncpy_s(file->Block.Pass, block.attribute("password").as_string(), 15);
		file->Block.Blocked = block.attribute("status").as_int();
	}

	file->Insignias.Value = std::stoll(accNode.child_value("insignia"));
	
	

	{
		auto storage = accNode.child("storage");
		file->Storage.Coin = std::stoi(storage.child_value("gold"));

		auto items = storage.child("items");

		for (auto item = items.child("item"); item; item = item.next_sibling("item"))
		{
			int itemSlot = item.attribute("slot").as_int();

			XMLToStructure(item, file->Storage.Item[itemSlot]);
		}
	}

	{
		auto water = accNode.child("water");

		file->Water.Day = std::stoi(water.child_value("day"));
		file->Water.Total = std::stoi(water.child_value("total"));
	}


	XMLToStructure(accNode.child("divina"), file->Divina);
	XMLToStructure(accNode.child("sephira"), file->Sephira);
}