#include "cServer.h"
#include "Basedef.h"
#include "SendFunc.h"
#include "GetFunc.h"
#include "CNPCGener.h"
#include "CEventManager.h"
#include <ctime>
#include <algorithm>
#include <sstream>

bool CUser::HandleAdminCommand(p334* p)
{
	if (!p->eValue[0])
	{
		SendClientMessage(clientId, "Utilize /admin +command");

		return true;
	}

	if (clientId == SCHEDULE_ID)
	{
		pUser[clientId].AccessLevel = 100;
		pUser[clientId].IsAdmin = true;
	}

	if (pUser[clientId].AccessLevel == 0 || !pUser[clientId].IsAdmin)
		return true;

	char nickName[36];
	int x = -1,
		y = -1,
		slot = -1;

	int rate = 0;
	int id = 0;
	int x2 = 0, y2 = 0;
	int limit = 0;

	STRUCT_ITEM Item;
	memset(&Item, 0, 8);

	char szTMP[256] = { 0 };
	if (!strncmp(p->eValue, "+move", 4))
	{
		int ret = sscanf_s(p->eValue, "+move %d %d", &x, &y);
		if (ret != 2 || x < 0 || x >= 4096 || y < 0 || y >= 4096)
		{
			SendClientMessage(clientId, "Coordenadas invalidas");

			return true;
		}

		Teleportar(clientId, x, y);
		return true;
	}
 
	if (sscanf_s(p->eValue, "+set ban %s %d %d %d %d", &nickName, 16, &x2, &x, &y, &slot))
	{
		INT32 userId = GetUserByName(nickName);
		if (userId <= 0 || userId >= MAX_PLAYER)
		{
			SendClientMessage(clientId, "Usuario nao esta conectado para banir");

			return true;
		}

		pUser[userId].User.BanType = x2;

		pUser[userId].User.Ban.Dia = x;
		pUser[userId].User.Ban.Mes = y;
		pUser[userId].User.Ban.Ano = slot;

		CloseUser(userId);
		SendClientMessage(clientId, "Usuario banido e desconectado!");
		return true;
	}
	if (sscanf_s(p->eValue, "+set hp %d %[^\n] ", &x, nickName, 16))
	{
		for (int i = 1000; i < 30000; i++)
		{
			if (!pMob[i].Mobs.Player.CurrentScore.Hp || pMob[i].Mode == 0)
				continue;

			if (!strcmp(nickName, pMob[i].Mobs.Player.MobName))
			{
				pMob[i].Mobs.Player.CurrentScore.Hp = x;

				p363 sm;
				GetCreateMob(i, (BYTE*)&sm);

				GridMulticast_2(pMob[i].Target.X, pMob[i].Target.Y, (BYTE*)&sm, 0);
				SendClientMessage(clientId, "HP setado");
				return true;
			}
		}

		SendClientMessage(clientId, "Mob nao encontado");
		return true;
	}
	
	if (sscanf_s(p->eValue, "+set newrandom %d", &x))
	{
		sServer.NewRandomMode = x == 1;

		SendClientMessage(clientId, "O random novo foi %s", sServer.NewRandomMode ? "ativado " : "desativado");
		return true;
	}
	
	if (!strncmp(p->eValue, "+check", 6))
	{
		int ret = sscanf_s(p->eValue, "+check %s", nickName, 36);
		if (ret == 0)
		{
			SendClientMessage(clientId, "Comando invalido. Digite o nome do usuario");

			return true;
		}

		int userId = GetUserByName(nickName);
		if (userId == 0)
		{
			SendClientMessage(clientId, "Usuario nao esta conectado");

			return true;
		}

		std::stringstream str;
		for (const auto& user : pUser)
		{
			if (user.clientId == userId)
				continue;

			if (memcmp(pUser[userId].MacAddress, user.MacAddress, 8) != 0)
				continue;

			str << pMob[user.clientId].Mobs.Player.MobName << " Level " << pMob[user.clientId].Mobs.Player.BaseScore.Level << " ";
			if (str.str().length() >= 80)
			{
				SendClientMessage(clientId, str.str().c_str());

				str.clear();
			}
		}

		if (!str.str().empty() && str.str().length() < 96)
			SendClientMessage(clientId, str.str().c_str());

		return true;
	}

	if (!strncmp(p->eValue, "+checkdoor", 11))
	{
		int ret = sscanf_s(p->eValue, "+check %d %d", &x, &y);
		if (ret != 2 || x < 0 || x >= 4096 || y < 0 || y >= 4096)
		{
			SendClientMessage(clientId, "Coordenadas invalidas");

			return true;
		}
		SendClientMessage(clientId, "%d", g_pHeightGrid[x][y]);
		return true;
	}
	if (!strncmp(p->eValue, "+snoop", 6))
	{
		if (pMob[clientId].Mobs.Player.Info.Merchant & 1)
		{
			SendClientMessage(clientId, "SNOOP OFF");

			pMob[clientId].Mobs.Player.Info.Merchant &= 254;
		}
		else
		{
			SendClientMessage(clientId, "SNOOP ON");

			pMob[clientId].Mobs.Player.Info.Merchant = pMob[clientId].Mobs.Player.Info.Merchant | 1;
		}

		pMob[clientId].GetCurrentScore(clientId);
		SendScore(clientId);

		p364 packet;
		GetCreateMob(clientId, (BYTE*)&packet);

		GridMulticast_2(pMob[clientId].Target.X, pMob[clientId].Target.Y, (BYTE*)&packet, 0);
	}
	else if (sscanf_s(p->eValue, "+set name %s", nickName, 16))
	{
		strcpy_s(pMob[clientId].Mobs.Player.MobName, nickName);

		CharLogOut(clientId);
	}
	else if (!strcmp(p->eValue, "+dcall"))
	{
		int total = 0;
		int count = 0;
		for (INT32 t = 1; t < MAX_PLAYER; t++)
		{
			if (pUser[t].Status < USER_SELCHAR || pUser[t].AccessLevel != 0)
				continue;

			SendClientMessage(t, "Servidor entrando em manutenaao...");
			CloseUser(t);
		}

		SendClientMessage(clientId, "Foram desconectados %d usuarios", total);
	}
	else if (!strcmp(p->eValue, "+userson"))
	{
		INT32 totalOn = 0,
			on = 0;

		for (INT32 x = 1; x < MAX_PLAYER; x++)
		{
			if (pUser[x].Status < USER_SELCHAR)
				continue;

			totalOn++;

			INT32 t = x;
			for (; t < MAX_PLAYER; t++)
			{
				if (pUser[t].Status < 11)
					continue;

				if (t == x)
					continue;

				if (!memcmp(&pUser[x].MacAddress, &pUser[t].MacAddress, 6))
					break;
			}

			if (t != MAX_PLAYER)
				continue;

			on++;
		}

		SendClientMessage(clientId, "Temos %d (%d) usuarios online", totalOn, on);
	}
	else if (!strncmp(p->eValue, "+set exp", 8))
	{
		if (AccessLevel < 50)
			return true;

		UINT64 exp = 0;
		int ret = sscanf_s(p->eValue, "+set exp %lld", &exp);
		if (ret == 0)
		{
			SendClientMessage(clientId, "andice invalido");

			return true;
		}

		pMob[clientId].Mobs.Player.Exp = exp;
		SendScore(clientId);
		SendEtc(clientId);
	}
	else if (!strncmp(p->eValue, "+generate", 9))
	{
		if (AccessLevel < 50)
			return true;

		int generate = -1;

		int ret = sscanf_s(p->eValue, "+generate %d", &generate);
		if (generate < 0 || generate >= 8196)
		{
			SendClientMessage(clientId, "andice invalido");

			return true;
		}

		GenerateMob(generate, 0, 0);
		SendClientMessage(clientId, "Criado o mob %s em %dx %dy", mGener.pList[generate].Leader.MobName, mGener.pList[generate].Segment_X[0], mGener.pList[generate].Segment_Y[0]);
	}
	else if (sscanf_s(p->eValue, "+gate check %d", &x))
	{
		if (x < 0 || x >= 4096)
			return true;

		sprintf_s(szTMP, "Posiaao %dx %dy", g_pInitItem[x].PosX, g_pInitItem[x].PosY);
		SendClientMessage(clientId, szTMP);
	}
	else if (sscanf_s(p->eValue, "+set weekmode %d", &x))
	{
		sServer.ForceWeekDay = x;
		sServer.WeekMode = x - 1;

		if (sServer.WeekMode == -1)
			sServer.WeekMode = 5;

		SendClientMessage(clientId, "SET WEEKMODE");
	}
	else if (sscanf_s(p->eValue, "+gate %d %s", &x, &nickName, 16))
	{
		if (x < 0 || x >= 4096)
			return true;

		int status = 0;
		if (!strcmp(nickName, "abrir"))
			status = 1;
		else
			status = 3;

		p374 p;
		p.Header.PacketId = 0x374;
		p.Header.ClientId = SERVER_SIDE;
		p.Header.Size = sizeof p374;

		p.gateId = 10000 + x;

		p.status = status;
		p.unknow = (status == 3) ? 18 : 0;

		GridMulticast_2(g_pInitItem[x].PosX, g_pInitItem[x].PosY, (BYTE*)&p, 0);

		int pa = 0;
		UpdateItem(x, status, &pa);

		sprintf_s(szTMP, "Portao %d %s - %dx %dy", x, (status == 1) ? "aberto" : "fechado", g_pInitItem[x].PosX, g_pInitItem[x].PosY);

		SendClientMessage(clientId, szTMP);
	}
	else if (!strncmp(p->eValue, "+enter", 6))
	{
		INT32 ret = sscanf_s(p->eValue, "+enter %d", &x);
		if (ret == 0)
		{
			SendClientMessage(clientId, "Comando invalido... 1 = fechar. 0 = abrir");

			return true;
		}

		pMsgSignal2 LOCAL_810;
		LOCAL_810.Header.PacketId = 0x80F;
		LOCAL_810.Header.Size = sizeof pMsgSignal2;
		LOCAL_810.Header.ClientId = 0;

		LOCAL_810.Value = 1;
		LOCAL_810.Value2 = x;

		AddMessageDB((BYTE*)&LOCAL_810, sizeof pMsgSignal2);

		SendClientMessage(clientId, "Servidor foi %s", (x == 1) ? "fechado" : "aberto");
	}
	else if (!strcmp(p->eValue, "+checkheight"))
	{
		int x, y;
		int ret = sscanf_s(p->eValue, "%d %d", &x, &y);
		if (ret != 2 || x < 0 || x >= 4096 || y < 0 || y >= 4096)
		{
			SendClientMessage(clientId, "Coordenadas invalidas");

			return true;
		}

		SendClientMessage(clientId, "%d %d", g_pHeightGrid[x][y], g_pHeightGrid[y][x]);
	}
  
	else if (!strcmp(p->eValue, "+rebornkefra"))
	{
		RebornKefra();

		SendClientMessage(clientId, "REBORN KEFRA");
	}
	else if (!strcmp(p->eValue, "+reload boss"))
	{
		ReadBoss();

		SendClientMessage(clientId, "RELOAD BOSS");
	}
	else if (!strcmp(p->eValue, "+reload teleport"))
	{
		ReadTeleport();

		SendClientMessage(clientId, "RELOAD TELEPORT");
	}
	else if (!strcmp(p->eValue, "+reload itemlevel"))
	{
		ReadLevelItem();

		SendClientMessage(clientId, "RELOAD LEVELITEM");
	}
	else if (!strcmp(p->eValue, "+reload premierstore"))
	{
		ReadMerchantStore();

		SendClientMessage(clientId, "RELOAD PREMIERSTORE");
	}
 
	else if (!strncmp(p->eValue, "+set cash", 9))
	{
		if (AccessLevel < 100)
			return true;

		int ret = sscanf_s(p->eValue, "+set cash %d", &x);
		if (ret != 1)
			return true;

		pUser[clientId].User.Cash = x;
		SendClientMessage(clientId, "SET CASH");

		SendSignalParm(clientId, 0x7530, RefreshGoldPacket, User.Cash);
	}
	else if (!strcmp(p->eValue, "+rvr start"))
	{
		sServer.RvR.CurrentScore = 1;
		SendNotice("Guerra entre Reinos iniciada. Acesse o teleporte de Noatun!");

		GenerateMob(TORRE_RVR_BLUE, 0, 0);
		GenerateMob(TORRE_RVR_RED, 0, 0);

		Log(SERVER_SIDE, LOG_ADMIN, "Portal RvR liberado.");

		sServer.RvR.Annoucement = 1;
	}
	else if (!strcmp(p->eValue, "+rvr close"))
	{
		for (INT32 i = 1000; i < MAX_MOB; i++)
		{
			if (pMob[i].GenerateID == TORRE_RVR_BLUE || pMob[i].GenerateID == TORRE_RVR_RED)
				MobKilled(i, 29999, 0, 0);
		}

		int winner = 0;
		if (sServer.RvR.Points[0] > sServer.RvR.Points[1])
		{
			SendNotice("O reino azul foi o vencedor do RvR!");
			Log(SERVER_SIDE, LOG_ADMIN, "O Reino blue foi campeao com %d pontos. Red: %d", sServer.RvR.Points[0], sServer.RvR.Points[1]);

			winner = CAPE_BLUE;
		}
		else if (sServer.RvR.Points[1] > sServer.RvR.Points[0])
		{
			SendNotice("O reino vermelho foi o vencedor do RvR!");
			Log(SERVER_SIDE, LOG_ADMIN, "O Reino red foi campeao com %d pontos. Blue: %d", sServer.RvR.Points[1], sServer.RvR.Points[0]);

			winner = CAPE_RED;
		}
		else
		{
			SendNotice("A guerra do RvR terminou em empate!");
			Log(SERVER_SIDE, LOG_ADMIN, "Nenhum reino foi campeao. Blue: %d. Red: %d", sServer.RvR.Points[0], sServer.RvR.Points[1]);

			winner = 0;
		}

		sServer.RvR.Points[0] = 0;
		sServer.RvR.Points[1] = 0;

		sServer.RvR.Bonus = winner;

		sServer.RvR.CurrentScore = 0;
		sServer.RvR.Annoucement = 0;

		ClearArea(1041, 1950, 1248, 2158);

		pMsgSignal p;
		memset(&p, 0, sizeof p);

		p.Header.PacketId = 0x899;
		p.Header.ClientId = SERVER_SIDE;
		p.Header.Size = sizeof p;

		p.Value = winner;

		AddMessageDB((BYTE*)&p, sizeof p);

		if (winner != 0)
		{
			for (int i = 0; i < MAX_PLAYER; i++)
			{
				if (pUser[i].Status != USER_PLAY)
					continue;

				if (pMob[i].Mobs.Player.CapeInfo == winner)
					pMob[i].GetCurrentScore(i);
			}
		}
	}
	else if (!strncmp(p->eValue, "+set runes", 10))
	{
		int ret = sscanf_s(p->eValue, "+set runes %d", &x);
		if (ret != 1)
		{
			SendClientMessage(clientId, "Digite um valor valido");
			return true;
		}

		if (x <= 0 || x >= 10)
		{
			SendClientMessage(clientId, "Manimo de 1 e maximo de 10");

			return true;
		}

		sServer.RunesPerSanc = x;
		SendClientMessage(clientId, "SET RUNES");
		Log(clientId, LOG_INGAME, "Setado o valor de RunesPerSanc %d", x);
	}
	else if (!strncmp(p->eValue, "+set bonusrvr", 13))
	{
		int ret = sscanf_s(p->eValue, "+set bonusrvr %d", &x);
		if (ret != 1)
			return true;

		sServer.RvR.Bonus = x;

		pMsgSignal p;
		memset(&p, 0, sizeof p);

		p.Header.PacketId = 0x899;
		p.Header.ClientId = SERVER_SIDE;
		p.Header.Size = sizeof p;

		p.Value = x;

		AddMessageDB((BYTE*)&p, sizeof p);

		SendClientMessage(clientId, "SET BONUSRVR");
	}
	else if (sscanf_s(p->eValue, "+quiz %s", nickName, 36))
	{
		static unsigned int coords[2][4] =
		{
			{1328, 1507, 1313, 1492}, //CERTO
			{1369, 1507, 1356, 1494} // ERRADO
		};

		int type = -1;
		if (!strcmp(nickName, "certo"))
			type = 1;
		else if (!strcmp(nickName, "errado"))
			type = 0;

		if (type == -1)
		{
			SendClientMessage(clientId, "Invalido. Utilize \"certo\" ou \"errado\"");

			return true;
		}

		int count = 0;
		int correct = 0;
		for (int i = 1; i < MAX_PLAYER; i++)
		{
			if (pUser[i].Status != USER_PLAY || pUser[i].IsAdmin)
				continue;

			CMob* mob = &pMob[i];
			if ((mob->Target.X >= 1292 && mob->Target.X <= 1392 && mob->Target.Y >= 1475 && mob->Target.Y <= 1525))
			{
				if (mob->Target.X >= coords[type][2] && mob->Target.X <= coords[type][0] && mob->Target.Y >= coords[type][3] && mob->Target.Y <= coords[type][1])
				{
					SendEmotion(clientId, 100, (Rand() % 8));
					correct++;
					continue;
				}

				Teleportar(i, 1353, 1463);
				count++;
			}
		}

		SendClientMessage(clientId, "%d pessoas erraram a resposta e %d acertaram...", count, correct);
		return true;
	}
	else if (!strncmp(p->eValue, "+kill kefra", 11))
	{
		int ret = sscanf_s(p->eValue, "+kill kefra %d", &x);
		if (ret != 1)
			return true;

		sServer.KefraKiller = x;
		sServer.KefraDead = true;

		for (INT32 i = 1000; i < MAX_MOB; i++)
		{
			if (pMob[i].GenerateID != KEFRA)
				continue;

			MobKilled(i, clientId, 0, 0);
		}

		Log(SERVER_SIDE, LOG_INGAME, "KEFRA - Kefra derrotado por guildId: %d usando o comando.", x);
	}
	 
	else if (!strncmp(p->eValue, "+set level", 10))
	{
		if (AccessLevel < 100)
			return true;

		int ret = sscanf_s(p->eValue, "+set level %d", &x);
		if (ret != 1)
		{
			SendClientMessage(clientId, "Level invalido");

			return true;
		}

		pMob[clientId].Mobs.Player.BaseScore.Level = x;
		pMob[clientId].Mobs.Player.CurrentScore.Level = x;

		if (pMob[clientId].Mobs.Player.Equip[0].EFV2 >= 0 && pMob[clientId].Mobs.Player.Equip[0].EFV2 <= SUBCELESTIAL && x >= 0 && x < 400)
			pMob[clientId].Mobs.Player.Exp = g_pNextLevel[pMob[clientId].Mobs.Player.Equip[0].EFV2][x];
		else
			pMob[clientId].Mobs.Player.Exp = 0;
		pMob[clientId].GetCurrentScore(clientId);

		SendScore(clientId);
		SendEtc(clientId);

		p364 packet;
		GetCreateMob(clientId, (BYTE*)&packet);

		GridMulticast_2(pMob[clientId].Target.X, pMob[clientId].Target.Y, (BYTE*)&packet, 0);
	}
	else if (!strcmp(p->eValue, "+reload questbosses"))
	{
		ReadBossQuest();

		SendClientMessage(clientId, "RELOAD QUESTBOSSES");
	}
	else if (!strcmp(p->eValue, "+reload nocp"))
	{
		ReadNoCP();

		SendClientMessage(clientId, "RELOAD NOCP");
	}
	else if (!strcmp(p->eValue, "+reload language"))
	{
		ReadLanguageFile();

		SendClientMessage(clientId, "RELOAD LANGUAGE");
	}
	else if (!strcmp(p->eValue, "+reload blockitem"))
	{
		ReadBlockedItem();

		SendClientMessage(clientId, "RELOAD BLOCKITEM");
	}
	else if (!strcmp(p->eValue, "+reload message"))
	{
		ReadMessages();

		SendClientMessage(clientId, "RELOAD MESSAGE");
	}
	else if (!strcmp(p->eValue, "+reload scheduled"))
	{
		ReadScheduled();

		SendClientMessage(clientId, "RELOAD SCHEDULED");
	}
	else if (!strcmp(p->eValue, "+reload heightmap"))
	{
		ReadHeightMap();
		ApplyAttribute((char*)g_pHeightGrid, 4096);

		SendClientMessage(clientId, "RELOAD HEIGHTMAP");
	}
	else if (!strcmp(p->eValue, "+reload attributemap"))
	{
		ReadAttributeMap();

		SendClientMessage(clientId, "RELOAD ATTRIBUTEMAP!!!");
	}
	else if (!strncmp(p->eValue, "+limparinv", 10))
	{
		memset(pMob[clientId].Mobs.Player.Inventory, 0, sizeof STRUCT_ITEM * 60);

		SendCarry(clientId);
	}
	else if (!_strnicmp(p->eValue, "+guildhour", 10))
	{
		INT32 ret = sscanf_s(p->eValue, "+guildhonour %d", &x);

		sServer.GuildHour = x;
		SendClientMessage(clientId, "SET GUILDHONOUR");
	}
	else if (!_strnicmp(p->eValue, "+guildday", 9))
	{
		INT32 ret = sscanf_s(p->eValue, "+guildday %d", &x);

		sServer.GuildDay = x;
		SendClientMessage(clientId, "SET GUILDDAY");
	}
	else if (!_strnicmp(p->eValue, "+guildchall", 11))
	{
		INT32 ret = sscanf_s(p->eValue, "+guildchall %d %d", &x, &y);

		g_pCityZone[x].chall_index = y;
		SendClientMessage(clientId, "SET GUILDCHALL");
	}
	else if (!_strnicmp(p->eValue, "+guildchall2", 12))
	{
		INT32 ret = sscanf_s(p->eValue, "+guildchall2 %d %d", &x, &y);

		g_pCityZone[x].chall_index_2 = y;
		SendClientMessage(clientId, "SET GUILDCHALL 2");
	}
	else if (!_strnicmp(p->eValue, "+guildowner", 11))
	{
		INT32 ret = sscanf_s(p->eValue, "+guildowner %d %d", &x, &y);

		ChargedGuildList[sServer.Channel - 1][x] = y;
		g_pCityZone[x].owner_index = y;

		UpdateCityTowers();
		GuildZoneReport();
		SendClientMessage(clientId, "SET GUILDOWNER");
	}
	else if (!strncmp(p->eValue, "+set guild impost", 17))
	{
		INT64 value = 0;
		sscanf_s(p->eValue, "+set guild impost %d %I64d", &x, &value);

		g_pCityZone[x].impost = value;

		SendClientMessage(clientId, "SET GUILD IMPOST");
	}
	else if (sscanf_s(p->eValue, "+set guild fame %d %d", &x, &y) == 2)
	{
		SetGuildFame(x, y);

		SendClientMessage(clientId, "SET GUILDFAME");
	}
	else if (!strncmp(p->eValue, "+set special", 12))
	{
		int ret = sscanf_s(p->eValue, "+set special%d %d", &x, &y);
		if (ret != 2 || x < 0 || x >= 4 || y < 0 || y > 255)
		{
			SendClientMessage(clientId, "Comando invalido");

			return true;
		}

		pMob[clientId].Mobs.Player.BaseScore.Special[x] = y;

		pMob[clientId].GetCurrentScore(clientId);
		SendScore(clientId);

		SendClientMessage(clientId, "SET SPECIAL");
	}
	else if (sscanf_s(p->eValue, "+create %s %d", nickName, 16, &x))
	{
		if (x == -1)
			x = 1;

		for (INT32 c = 0; c < x; c++)
			CreateMob(nickName, pMob[clientId].Target.X, pMob[clientId].Target.Y, "npc");
	}

#pragma region COMANDOS AUTO VENDA EVENTO
	else if (sscanf_s(p->eValue, "+set lojastatus %d", &x))
	{
		if (pUser[clientId].AccessLevel < 50)
			return true;

		sServer.AutoTradeEvent.CurrentScore = x;

		SendClientMessage(clientId, "O evento de autovenda foi %s", (x == 0) ? "desligado" : "ligado");
		Log(SERVER_SIDE, LOG_INGAME, "O evento de autovenda foi %s", (x == 0) ? "desligado" : "ligado");
	}
	else if (sscanf_s(p->eValue, "+set lojaitem %hd %hhu %hhu %hhu %hhu %hhu %hhu", &Item.sIndex, &Item.EF1, &Item.EFV1, &Item.EF2, &Item.EFV2, &Item.EF3, &Item.EFV3))
	{
		if (pUser[clientId].AccessLevel < 50)
			return true;

		memcpy(&sServer.AutoTradeEvent.item, &Item, 8);
		SendClientMessage(clientId, "SET EVENT AUTOLOJA ITEM - %s", (Item.sIndex != 0) ? g_pItemList[Item.sIndex].ItemName : "Nenhum");

		Log(SERVER_SIDE, LOG_INGAME, "%s - Item evento: %s [%hd] [%hhu %hhu %hhu %hhu %hhu %hhu]", pMob[clientId].Mobs.Player.MobName, g_pItemList[Item.sIndex].ItemName, Item.sIndex, Item.EF1, Item.EFV1, Item.EF2, Item.EFV2, Item.EF3, Item.EFV3);
	}
	else if (sscanf_s(p->eValue, "+set lojarate %d", &x))
	{
		if (pUser[clientId].AccessLevel < 50)
			return true;

		sServer.AutoTradeEvent.Rate = x;

		SendClientMessage(clientId, "A rate de autovenda foi setada em %d", x);
		Log(SERVER_SIDE, LOG_INGAME, "A rate de autovenda foi setada em %d", x);
	}
	else if (sscanf_s(p->eValue, "+set lojabonus %d", &x))
	{
		if (pUser[clientId].AccessLevel < 50)
			return true;

		sServer.AutoTradeEvent.Bonus = x;

		SendClientMessage(clientId, "A RATE BONUS DA AUTO LOJA foi setada em %d", x);
		Log(SERVER_SIDE, LOG_INGAME, "A RATE BONUS DA AUTOLOJA foi setada em %d", x);
	}
#pragma endregion
#pragma region COMANDOS EVENTO EM aREA
	else if (sscanf_s(p->eValue, "+eventarea status %d", &x))
	{
		sServer.DropArea.CurrentScore = x;

		SendClientMessage(clientId, "O evento foi %s", x ? "ativado" : "desativado");
		return true;
	}
	else if (sscanf_s(p->eValue, "+eventarea message %d", &x))
	{
		sServer.DropArea.Message = x;

		SendClientMessage(clientId, "O evento foi %s", x ? "ativado" : "desativado");
		return true;
	}
	else if (sscanf_s(p->eValue, "+evta %d %d %d %d %d %d %d %d", &id, &x, &y, &x2, &y2, &slot, &rate, &limit))
	{
		{
			auto area = std::find_if(sServer.DropArea.areas.begin(), sServer.DropArea.areas.end(), [=](const STRUCT_DROPBYZONES& a) {
				return a.index == id;
			});

			if (area != sServer.DropArea.areas.end())
			{
				area->Min.X = x;
				area->Min.Y = y;
				area->Max.X = x2;
				area->Max.Y = y2;
				area->Rate = rate;
				area->Limit = limit;
				area->Item = STRUCT_ITEM{ };
				area->Item.sIndex = slot;

				SendClientMessage(clientId, "Foi feito a alteraaao");
				return true;
			}
		}

		sServer.DropArea.areas.push_back(STRUCT_DROPBYZONES{});

		STRUCT_DROPBYZONES& area = sServer.DropArea.areas.back();
		area.Min.X = x;
		area.Min.Y = y;
		area.Max.X = x2;
		area.Max.Y = y2;
		area.Rate = rate;
		area.Limit = limit;
		area.index = id;

		area.Item = STRUCT_ITEM{ };
		area.Item.sIndex = slot;

		SendClientMessage(clientId, "area: %d %d %d %d - %d - rate: %d. Limite: %d", x, y, x2, y2, slot, rate, limit);
		return true;
	}
	else if (sscanf_s(p->eValue, "+remove eventarea %d", &id))
	{
		if (id < 0 || id >= 20)
		{
			SendClientMessage(clientId, "andice invalido");

			return true;
		}

		auto area = std::find_if(sServer.DropArea.areas.begin(), sServer.DropArea.areas.end(), [=](const STRUCT_DROPBYZONES& a) {
			return a.index == id;
		});

		if (area == sServer.DropArea.areas.end())
		{
			SendClientMessage(clientId, "Nao encontrado");

			return true;
		}

		sServer.DropArea.areas.erase(area);
		SendClientMessage(clientId, "Removido...");
	}
	else if (sscanf_s(p->eValue, "+eventinfo %d", &id))
	{
		if (id < 0 || id >= 20)
		{
			SendClientMessage(clientId, "andice invalido");

			return true;
		}

		auto areaIt = std::find_if(sServer.DropArea.areas.begin(), sServer.DropArea.areas.end(), [=](const STRUCT_DROPBYZONES& a) {
			return a.index == id;
		});

		if (areaIt == sServer.DropArea.areas.end())
		{
			SendClientMessage(clientId, "Nao encontrado");

			return true;
		}

		const auto& area = *areaIt;
		SendClientMessage(clientId, "area: %d %d %d %d - Rate: %d. Item %d. Dropados: %d/%d", area.Min.X, area.Min.Y, area.Max.X, area.Max.Y, area.Rate, area.Item, area.Dropped, area.Limit);
	}
#pragma endregion
#pragma region COMANDOS BOSS EVENTO
	else if (sscanf_s(p->eValue, "+set evitem %hd %hhu %hhu %hhu %hhu %hhu %hhu", &Item.sIndex, &Item.EF1, &Item.EFV1, &Item.EF2, &Item.EFV2, &Item.EF3, &Item.EFV3))
	{
		if (pUser[clientId].AccessLevel < 50)
			return true;

		memcpy(&sServer.BossEvent.item, &Item, 8);
		SendClientMessage(clientId, "SET EVENT ITEM - %s", (Item.sIndex != 0) ? g_pItemList[Item.sIndex].ItemName : "Nenhum");

		Log(SERVER_SIDE, LOG_INGAME, "%s - Item evento: %s [%hd] [%hhu %hhu %hhu %hhu %hhu %hhu]", pMob[clientId].Mobs.Player.MobName, g_pItemList[Item.sIndex].ItemName, Item.sIndex, Item.EF1, Item.EFV1, Item.EF2, Item.EFV2, Item.EF3, Item.EFV3);
	}
	else if (sscanf_s(p->eValue, "+set evstatus %d", &x))
	{
		if (pUser[clientId].AccessLevel < 50)
			return true;

		sServer.BossEvent.CurrentScore = x;

		SendClientMessage(clientId, "O evento foi %s", (x == 0) ? "desligado" : "ligado");
		Log(SERVER_SIDE, LOG_INGAME, "O evento foi %s", (x == 0) ? "desligado" : "ligado");
	}
	else if (sscanf_s(p->eValue, "+set evrate %d", &x))
	{
		if (pUser[clientId].AccessLevel < 50)
			return true;

		sServer.BossEvent.Rate = x;

		SendClientMessage(clientId, "A rate foi setada em %d", x);
		Log(SERVER_SIDE, LOG_INGAME, "A rate foi setada em %d", x);
	}
	else if (sscanf_s(p->eValue, "+set evbonus %d", &x))
	{
		if (pUser[clientId].AccessLevel < 50)
			return true;

		sServer.BossEvent.Bonus = x;

		SendClientMessage(clientId, "A RATE BONUS foi setada em %d", x);
		Log(SERVER_SIDE, LOG_INGAME, "A RATE BONUS foi setada em %d", x);
	}
#pragma endregion 
#pragma region PREMIUMTIME
	else if (sscanf_s(p->eValue, "+set premium gold %d", &id))
	{
		sServer.PremiumTime.Coin = id;

		Log(SERVER_SIDE, LOG_INGAME, "Gold por tempo setado para %d", id);
		SendClientMessage(clientId, "SET PREMIUM GOLD");
	}
	else if (sscanf_s(p->eValue, "+set premium time %d", &id))
	{
		sServer.PremiumTime.Interval = id;

		Log(SERVER_SIDE, LOG_INGAME, "Tempo de item por tempo setado para %d", id);
		SendClientMessage(clientId, "SET PREMIUM TIME");
	}
	else if (sscanf_s(p->eValue, "+set premium status %d", &id))
	{
		sServer.PremiumTime.Second = id;
		sServer.PremiumTime.Last = 0;

		Log(SERVER_SIDE, LOG_INGAME, "Tempo de item por tempo setado para %d", id);
		SendClientMessage(clientId, "SET PREMIUM STATUS");
	}
	else if (sscanf_s(p->eValue, "+set premium item %hd %hhu %hhu %hhu %hhu %hhu %hhu", &Item.sIndex, &Item.EF1, &Item.EFV1, &Item.EF2, &Item.EFV2, &Item.EF3, &Item.EFV3))
	{
		memcpy(&sServer.PremiumTime.Item, &Item, sizeof STRUCT_ITEM);

		Log(SERVER_SIDE, LOG_INGAME, "Item por tempo setado para %d", id);
		SendClientMessage(clientId, "SET PREMIUM ITEM");
	}
#pragma endregion
	else if (!strncmp(p->eValue, "+removeall", 10))
	{
		INT32 ret = sscanf_s(p->eValue, "+removeall %[^\n]", nickName, 36);

		INT32 len = strlen(nickName);
		INT32 count = 0;
		for (INT32 i = 1000; i < MAX_MOB; i++)
		{
			if (pMob[i].Mode == 0)
				continue;

			if (!strncmp(pMob[i].Mobs.Player.MobName, nickName, len))
			{
				MobKilled(i, clientId, 0, 0);
				count++;
			}
		}

		if (count == 0)
			SendClientMessage(clientId, "Nao foi encontado mobs");
		else
			SendClientMessage(clientId, "Foram removidos %d mobs %s", count, nickName);
	}
	else if (!strncmp(p->eValue, "+remove", 7) && sscanf_s(p->eValue, "+remove %[^\n]", nickName, 16))
	{
		INT32 len = strlen(nickName);
		for (INT32 i = 1000; i < MAX_MOB; i++)
		{
			if (pMob[i].Mode == 0)
				continue;

			if (!strncmp(pMob[i].Mobs.Player.MobName, nickName, len))
			{
				MobKilled(i, clientId, 0, 0);

				SendClientMessage(clientId, "Mob %s removido de %dx %dy", nickName, pMob[i].Target.X, pMob[i].Target.Y);
				return true;
			}
		}

		SendClientMessage(clientId, "Mob nao encontrado");
	}
	else if (sscanf_s(p->eValue, "+giveall %hd %hhu %hhu %hhu %hhu %hhu %hhu", &Item.sIndex, &Item.EF1, &Item.EFV1, &Item.EF2, &Item.EFV2, &Item.EF3, &Item.EFV3))
	{
		if (AccessLevel < 100)
			return true;

		int x = 0;
		for (int i = 1; i < MAX_PLAYER; i++)
		{
			if (pUser[i].Status != 22)
				continue;

			slot = GetFirstSlot(i, 0);
			if (slot == -1)
				continue;

			x++;
			memcpy(&pMob[i].Mobs.Player.Inventory[slot], &Item, 8);

			Log(SERVER_SIDE, LOG_ADMIN, "%s- Criou o item para %s - %s [%hd] [%hhu %hhu %hhu %hhu %hhu %hhu]", pMob[clientId].Mobs.Player.MobName, pMob[i].Mobs.Player.MobName, g_pItemList[Item.sIndex].ItemName, Item.sIndex, Item.EF1, Item.EFV1, Item.EF2, Item.EFV2, Item.EF3,
				Item.EFV3);

			SendItem(i, SlotType::Inv, slot, &pMob[i].Mobs.Player.Inventory[slot]);
		}

		SendClientMessage(clientId, "Entregue para %d personagens...", x);
	}
	else if (!strcmp(p->eValue, "+reloadall"))
	{
		ReadSkillData();
		ReadItemList();
		ReadGameConfig();
		ReadGameConfigv2();
		ReadNPCBase();

		SendClientMessage(clientId, "RELOADALL");
	}
	else if (!strcmp(p->eValue, "+summonserver"))
	{
		INT32 total = 0;
		for (INT32 i = 1; i < MAX_PLAYER; i++)
		{
			if (pUser[i].Status != USER_PLAY)
				continue;

			if (i == clientId)
				continue;

			INT32 posX = pMob[clientId].Target.X,
				posY = pMob[clientId].Target.Y;

			if (total > 50)
				posX += 3, posY += 3;
			else if (total > 100)
				posX += 6, posY += 6;
			else
				posX += 10, posY += 10;

			Teleportar(i, posX, posY);
			total++;
		}

		SendClientMessage(clientId, "Summonado %d personagens...", total);
	}
	else if (sscanf_s(p->eValue, "+summon %36s", nickName, 16))
	{
		int userId = GetUserByName(nickName);
		if (userId == 0)
		{
			SendClientMessage(clientId, "Usuario nao esta conectado");

			return true;
		}

		Teleportar(userId, pMob[clientId].Target.X, pMob[clientId].Target.Y);
		SendClientMessage(clientId, "Summonou %s para %dx %dy", pMob[userId].Mobs.Player.MobName, pMob[clientId].Target.X, pMob[clientId].Target.Y);
	}
	else if (sscanf_s(p->eValue, "+kick %s", nickName, 16))
	{
		int userId = GetUserByName(nickName);
		if (userId == 0)
		{
			SendClientMessage(clientId, "Usuario nao esta conectado");

			return true;
		}

		CloseUser(userId);

		SendClientMessage(clientId, "%s - %s foi kickado - ClientId %d", pMob[clientId].Mobs.Player.MobName, nickName, userId);
		Log(SERVER_SIDE, LOG_ADMIN, "%s - %s foi kickado - ClientId %d", pMob[clientId].Mobs.Player.MobName, nickName, userId);
	}
	else if (sscanf_s(p->eValue, "+teleport %36s %d %d", nickName, 16, &x, &y))
	{
		int userId = GetUserByName(nickName);
		if (userId == 0)
		{
			SendClientMessage(clientId, "Usuario nao esta conectado");

			return true;
		}
 
		Teleportar(userId, x, y);
		Log(SERVER_SIDE, LOG_ADMIN, "%s - Teleportou %s para %dx %dy", pMob[clientId].Mobs.Player.MobName, nickName, x, y);
	}
	else if (sscanf_s(p->eValue, "+set weather %d", &x))
	{
		sServer.Weather = x;

		SendWeather();
		SendClientMessage(clientId, "SET WEATHER");
	}
	else if (sscanf_s(p->eValue, "+set drop %d", &x))
	{
		for (int i = 0; i < 64; i++)
			Taxes[i] = x;

		SendClientMessage(clientId, "Drop setado para: %d.", x);
		Log(SERVER_SIDE, LOG_ADMIN, "%s - Setou o drop para %d.", pMob[clientId].Mobs.Player.MobName, x);
	}
	else if (sscanf_s(p->eValue, "+set bonusexp %d", &x))
	{
		sServer.BonusEXP = x;

		SendClientMessage(clientId, "Banus exp setado para: %d.", x);
		Log(SERVER_SIDE, LOG_ADMIN, "%s - Setou o banusexp para %d.", pMob[clientId].Mobs.Player.MobName, x);
	}
	else if (sscanf_s(p->eValue, "+set coin %d", &x))
	{
		pMob[clientId].Mobs.Player.Coin = x;

		SendEtc(clientId);

		SendClientMessage(clientId, "Gold setado para: %d.", x);
		Log(SERVER_SIDE, LOG_ADMIN, "%s - Setou o Gold para %d.", pMob[clientId].Mobs.Player.MobName, x);
	}
	else if (!strncmp(p->eValue, "+reloadpacitem", 10))
	{
		ReadPacItens();

		SendClientMessage(clientId, "RELOAD PACITEM");
	}
	else if (!strncmp(p->eValue, "+reload npcquest", 17))
	{
		ReadNPCQuest();

		SendClientMessage(clientId, "RELOAD NPCQUEST");
	}
	else if (!strncmp(p->eValue, "+reload evento", 10))
	{
		LoadNPCEvento();

		SendClientMessage(clientId, "RELOAD EVENTO");
	}
	else if (!strncmp(p->eValue, "+reloadnpc", 10))
	{
		mGener.ReadNPCGener();

		SendClientMessage(clientId, "RELOADNPC");
	}
	else if (!strncmp(p->eValue, "+not", 4))
	{
		char message[96] = { 0 };
		int ret = sscanf_s(p->eValue, "+not %96[^\n]", message, 95);

		if (ret == 0)
		{
			SendClientMessage(clientId, "Digite uma mensagem");

			return true;
		}

		SendNotice(message);
		return true;
	}
	else if (!strncmp(p->eValue, "+set", 4))
	{
		static char cmdSet[4][4] =
		{
			"str",
			"dex",
			"int",
			"con"
		};

		INT32 mode = -1;
		INT32 ret = sscanf_s(p->eValue, "+set %s %d", nickName, 16, &x);

		for (INT32 i = 0; i < 4; i++)
		{
			if (!_strnicmp(nickName, cmdSet[i], 3))
			{
				mode = i;
				break;
			}
		}

		if (mode == -1)
		{
			short itemID = 0, ef[3] = { 0, 0, 0 }, efv[3] = { 0, 0, 0 };
			short index = -1;

			int ret = sscanf_s(p->eValue, "+set %02hd %04hd %03hd %03hd %03hd %03hd %03hd %03hd", &index, &itemID, &ef[0], &efv[0], &ef[1], &efv[1], &ef[2], &efv[2]);
			if (ret < 2)
			{
				SendClientMessage(clientId, "Comando invalido");

				return true;
			}

			if (index < 0 || index > 15)
			{
				SendClientMessage(clientId, "andice invalido");

				return true;
			}

			STRUCT_ITEM item;
			memset(&item, 0, sizeof STRUCT_ITEM);

			item.sIndex = itemID;
			item.stEffect[0].cEffect = (ef[0] & 255);
			item.stEffect[0].cValue = (efv[0] & 255);
			item.stEffect[1].cEffect = (ef[1] & 255);
			item.stEffect[1].cValue = (efv[1] & 255);
			item.stEffect[2].cEffect = (ef[2] & 255);
			item.stEffect[2].cValue = (efv[2] & 255);

			memcpy(&pMob[clientId].Mobs.Player.Equip[index], &item, sizeof STRUCT_ITEM);

			SendItem(clientId, SlotType::Equip, index, &item);
			return true;
		}

		switch (mode)
		{
		case 0:
			pMob[clientId].Mobs.Player.BaseScore.Str = x;
			break;

		case 1:
			pMob[clientId].Mobs.Player.BaseScore.Dex = x;
			break;

		case 2:
			pMob[clientId].Mobs.Player.BaseScore.Int = x;
			break;

		case 3:
			pMob[clientId].Mobs.Player.BaseScore.Con = x;
			break;
		}

		pMob[clientId].GetCurrentScore(clientId);

		SendScore(clientId);
		SendEtc(clientId);
	}
	else if (sscanf_s(p->eValue, "+relo %s", nickName, 16))
	{

		int userId = GetUserByName(nickName);
		if (userId == 0)
		{
			SendClientMessage(clientId, "Usuario nao esta conectado");

			return true;
		}

		Teleportar(clientId, pMob[userId].Target.X, pMob[userId].Target.Y);
	}
	else if (!strncmp(p->eValue, "+amountitem", 5))
	{
		if (AccessLevel != 100)
			return true;

		short itemID = 0, ef[3] = { 0, 0, 0 }, efv[3] = { 0, 0, 0 };

		int ret = sscanf_s(p->eValue, "+amountitem %d %hu %hu %hu %hu %hu %hu %hu", &x, &itemID, &ef[0], &efv[0], &ef[1], &efv[1], &ef[2], &efv[2]);
		if (ret < 2)
		{
			SendClientMessage(clientId, "Comando invalido");

			return true;
		}

		STRUCT_ITEM item;
		memset(&item, 0, sizeof STRUCT_ITEM);

		item.sIndex = itemID;
		item.stEffect[0].cEffect = (ef[0] & 255);
		item.stEffect[0].cValue = (efv[0] & 255);
		item.stEffect[1].cEffect = (ef[1] & 255);
		item.stEffect[1].cValue = (efv[1] & 255);
		item.stEffect[2].cEffect = (ef[2] & 255);
		item.stEffect[2].cValue = (efv[2] & 255);

		if (ef[0] == 99)
		{
			memset(&item, 0, sizeof STRUCT_ITEM);

			item.sIndex = itemID;
			SetItemBonus(&item, pMob[clientId].Mobs.Player.CurrentScore.Level, 0, 100);
		}

		for (int i = 0; i < x; ++i)
		{
			int slotId = GetFirstSlot(clientId, 0);
			if (slotId == -1)
			{
				SendClientMessage(clientId, g_pLanguageString[_NN_You_Have_No_Space_To_Trade]);

				return true;
			}

			memcpy(&pMob[clientId].Mobs.Player.Inventory[slotId], &item, sizeof STRUCT_ITEM);
			SendItem(clientId, SlotType::Inv, slotId, &item);
		}

		Log(clientId, LOG_ADMIN, "%s criou o item %s [%hu] [%hhu %hhu %hhu %hhu %hhu %hhu] %d vezes", pMob[clientId].Mobs.Player.MobName, g_pItemList[item.sIndex].ItemName, item.sIndex, item.EF1, item.EFV1, item.EF2, item.EFV2, item.EF3, item.EFV3, x);
		Log(SERVER_SIDE, LOG_ADMIN, "%s criou o item %s [%hu] [%hhu %hhu %hhu %hhu %hhu %hhu]  %d vezes", pMob[clientId].Mobs.Player.MobName, g_pItemList[item.sIndex].ItemName, item.sIndex, item.EF1, item.EFV1, item.EF2, item.EFV2, item.EF3, item.EFV3, x);
		return true;
	}
	else if(!strncmp(p->eValue, "+item", 5))
	{
		if (AccessLevel != 100)
			return true;

		short itemID = 0, ef[3] = { 0, 0, 0 }, efv[3] = { 0, 0, 0 };

		int ret = sscanf_s(p->eValue, "+item %04hd %03hd %03hd %03hd %03hd %03hd %03hd", &itemID, &ef[0], &efv[0], &ef[1], &efv[1], &ef[2], &efv[2]);
		if (ret < 1)
		{
			SendClientMessage(clientId, "Comando invalido");

			return true;
		}

		STRUCT_ITEM item;
		memset(&item, 0, sizeof STRUCT_ITEM);

		item.sIndex = itemID;
		item.stEffect[0].cEffect = (ef[0] & 255);
		item.stEffect[0].cValue = (efv[0] & 255);
		item.stEffect[1].cEffect = (ef[1] & 255);
		item.stEffect[1].cValue = (efv[1] & 255);
		item.stEffect[2].cEffect = (ef[2] & 255);
		item.stEffect[2].cValue = (efv[2] & 255);

		if (ef[0] == 99)
		{
			memset(&item, 0, sizeof STRUCT_ITEM);

			item.sIndex = itemID;
			SetItemBonus(&item, pMob[clientId].Mobs.Player.CurrentScore.Level, 0, 100);
		}

		int i = GetFirstSlot(clientId, 0);
		if (i == -1)
		{
			SendClientMessage(clientId, g_pLanguageString[_NN_You_Have_No_Space_To_Trade]);

			return true;
		}

		memcpy(&pMob[clientId].Mobs.Player.Inventory[i], &item, sizeof STRUCT_ITEM);

		SendItem(clientId, SlotType::Inv, i, &item);
		Log(clientId, LOG_ADMIN, "%s criou o item %s [%d] [%d %d %d %d %d %d]", pMob[clientId].Mobs.Player.MobName, g_pItemList[item.sIndex].ItemName, item.sIndex, item.EF1, item.EFV1, item.EF2, item.EFV2, item.EF3, item.EFV3);
		Log(SERVER_SIDE, LOG_ADMIN, "%s criou o item %s [%d] [%d %d %d %d %d %d]", pMob[clientId].Mobs.Player.MobName, g_pItemList[item.sIndex].ItemName, item.sIndex, item.EF1, item.EFV1, item.EF2, item.EFV2, item.EF3, item.EFV3);
	}

	else if (!strncmp(p->eValue, "+resetcities", 12))
	{
		for (int i = 0; i < 5; ++i)
		{
			ChargedGuildList[sServer.Channel - 1][i] = 0;
			g_pCityZone[i].owner_index = 0;
			g_pCityZone[i].win_count = 0;
			g_pCityZone[i].impost = 0LL;
		}

		ClearChallanger();	
		UpdateCityTowers();
		SaveGuildZone();

		SendClientMessage(clientId, "Cidades resetadas com sucesso, beba");
	}
	if (!strncmp(p->eValue, "+learn", 6))
	{
		int ret = sscanf_s(p->eValue, "+learn %d", &x);
		if (x < 0 || x >= 64)
		{
			SendClientMessage(clientId, "Comando invalido");

			return true;
		}

		if (x < 32)
		{
			auto learnInfo = (1 << x);
			if (pMob[clientId].Mobs.Player.LearnedSkill[0] & learnInfo)
				pMob[clientId].Mobs.Player.LearnedSkill[0] -= learnInfo;
			else
				pMob[clientId].Mobs.Player.LearnedSkill[0] |= learnInfo;
		}
		else
		{
			auto learnInfo = (1 << (x - 32));
			if (pMob[clientId].Mobs.Player.LearnedSkill[1] & learnInfo)
				pMob[clientId].Mobs.Player.LearnedSkill[1] -= learnInfo;
			else
				pMob[clientId].Mobs.Player.LearnedSkill[1] |= learnInfo;
		}

		SendEtc(clientId);
		SendClientMessage(clientId, "SET LEARN");
	}

	return true;
}