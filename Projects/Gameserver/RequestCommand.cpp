#include "cServer.h"
#include "Basedef.h"
#include "SendFunc.h"
#include "GetFunc.h"
#include "CNPCGener.h"
#include <ctime>
#include <algorithm>
#include <sstream>
using namespace std::chrono_literals;
bool CUser::RequestCommand(PacketHeader *Header)
{
	p334 *p = (p334*)(Header);
	p->eCommand[15] = '\0';
	p->eValue[99] = '\0';

	char szTMP[128];

	Log(clientId, LOG_INGAME, "Comando \"/%s %s\"", p->eCommand, p->eValue);

	if (Status != USER_PLAY && clientId != SCHEDULE_ID)
	{
		Log(clientId, LOG_INGAME, "Enviado comando estando fora da sessêo de jogo... Sessêo atual: %d", Status);

		return true;
	}

	CMob *mob = &pMob[clientId];
	if (!strcmp(p->eCommand, "day"))
		SendClientMessage(clientId, "!#11 2");
 
	 
	else if (!strncmp(p->eCommand, "tgp", 3))
	{
		INT32 mobId = GetUserByName(p->eValue);
		if (mobId <= 0)
		{
			SendClientMessage(clientId, "O usuario nêo esta conectado.");

			return true;
		}

		if (pMob[clientId].Leader != 0)
		{
			SendClientMessage(clientId, g_pLanguageString[_NN_Party_Leader_Only]);

			return true;
		}

		INT32 i = 0;
		for (; i < 12; i++)
		{
			if (pMob[clientId].PartyList[i] == mobId)
				break;
		}

		if (i == 12)
		{
			SendClientMessage(clientId, "O membro deve ser do seu grupo");

			return true;
		}

		GroupTransfer(clientId, mobId);
	}
	else if (!strcmp(p->eCommand, "srv"))
	{
		INT32 channel = atoi(p->eValue);

		if (sServer.Channel == channel)
		{
			SendClientMessage(clientId, "Você ja esta neste canal");

			return true;
		}

		MSG_DBServerChange packet;
		memset(&packet, 0, sizeof packet);

		packet.Header.PacketId = 0x814;
		packet.Header.Size = sizeof MSG_DBServerChange;
		packet.Header.ClientId = clientId;

		packet.NewServerID = channel;
		packet.Slot = inGame.CharSlot;

		AddMessageDB((BYTE*)&packet, sizeof MSG_DBServerChange);

		Log(clientId, LOG_INGAME, "Solicitou trocar para o servidor %d", channel);
		return true;
	}
	else if (!strcmp(p->eCommand, "handover"))
	{
		// Retorna o tempo para o pesadelo
		time_t nowraw;
		struct tm now;

		nowraw = time(NULL);
		localtime_s(&now, &nowraw);

		int guildId = pMob[clientId].Mobs.Player.Guild;
		if (!guildId)
		{
			SendClientMessage(clientId, "Necessario possuir uma guild.");
			return true;
		}

		int mobId = GetUserByName(p->eValue);
		if (mobId == 0)
		{
			SendClientMessage(clientId, "O usuario nêo esta conectado.");
			return true;
		}

		if (pMob[mobId].Mobs.Player.Guild != guildId)
		{
			SendClientMessage(clientId, "Necessario ser da mesma guilda.");
			return true;
		}

		if (now.tm_wday == DOMINGO)
		{
			SendClientMessage(clientId, "Nêo ê possêvel transferir no domingo");

			return true;
		}

		if (pMob[clientId].Mobs.Player.GuildLevel != 9)
		{
			SendClientMessage(clientId, "Transferência habilitada apenas para lêderes de guild.");

			return true;
		}

		SetGuildFame(guildId, 0);

		auto memberType = pMob[mobId].Mobs.Player.GuildLevel;
		std::swap(pMob[clientId].Mobs.Player.GuildLevel, pMob[mobId].Mobs.Player.GuildLevel);

		SendClientMessage(clientId, "Medalha transferida");
		SendClientMessage(mobId, "Medalha transferida");

		Log(clientId, LOG_INGAME, "Medalha transferida para %s GuildID : %d. Fame perdida.", pMob[mobId].Mobs.Player.MobName, guildId);
		Log(mobId, LOG_INGAME, "Medalha recebida de %s - GuildID: %d. Fame perdida", pMob[clientId].Mobs.Player.MobName, guildId);

		MulticastGetCreateMob(clientId);
		MulticastGetCreateMob(mobId);
	}
	else if (!strcmp(p->eCommand, "fimirma"))
	{
		INT32 guildId = pMob[clientId].Mobs.Player.Guild;
		if (guildId <= 0 || guildId >= MAX_GUILD || pMob[clientId].Mobs.Player.GuildLevel != 9)
		{
			SendClientMessage(clientId, g_pLanguageString[_NN_Only_Guild_Master_can]);

			return true;
		}

		INT32 guildAlly = g_pGuildAlly[guildId];
		if (guildAlly <= 0 || guildAlly >= MAX_GUILD)
		{
			SendClientMessage(clientId, "Você nêo possui alianêa");

			return true;
		}

		if (IsWarTime())
		{
			SendClientMessage(clientId, "Nêo ê possêvel realizar em horario de guerra");

			return true;
		}

		pE12 packet;
		memset(&packet, 0, sizeof packet);

		packet.Header.PacketId = 0xE12;
		packet.Header.Size = sizeof pE12;

		packet.GuildIndex1 = guildId;
		packet.GuildIndex2 = 0;

		AddMessageDB((BYTE*)&packet, sizeof pE12);
	}
	else if (!strncmp(p->eCommand, "war", 3))
	{/*
		INT32 conn		= sServer.Channel - 1;
		INT32 otherConn = ((conn % 2) ? (conn - 1) : (conn + 1));

		if(sServer.TowerWar[conn].WarState == 1)
			SendClientMessage(clientId, "O canal %d esta em guerra com o canal %d", otherConn + 1, conn + 1);
		else if(sServer.TowerWar[conn].WarState == 2)
			SendClientMessage(clientId, "O canal %d precisa reconquistar o prêprio canal", conn);

		if(sServer.TowerWar[otherConn].WarState == 1)
			SendClientMessage(clientId, "O canal %d esta em guerra com o canal %d", conn + 1, otherConn + 1);
		else if(sServer.TowerWar[otherConn].WarState == 2)
			SendClientMessage(clientId, "O canal %d precisa reconquistar o prêprio canal", otherConn);*/
	}
	else if (!strcmp(p->eCommand, "rvr"))
	{
		SendClientMessage(clientId, "Você possui %d pontos", pMob[clientId].Mobs.RvRPoints);
	}
	else if (!strcmp(p->eCommand, "info"))
	{
		if (pMob[clientId].Mobs.Player.Equip[0].EFV2 <= 2)
			return true;

		sprintf_s(szTMP, "Você esta no %s e possui %d resets.", (pMob[clientId].Mobs.Player.Equip[0].EFV2 == 4) ? "Subcelestial" : "Celestial", pMob[clientId].Mobs.GetTotalResets());
		SendClientMessage(clientId, szTMP);
	}
	/*else if (!strncmp(p->eCommand, "lock", 4))
	{
		if (strcmp(p->eValue, User.Block.Pass))
		{
			SendClientMessage(clientId, "Senha incorreta");

			return true;
		}

		User.Block.Blocked = !User.Block.Blocked;
		SendClientMessage(clientId, (User.Block.Blocked) ? "Seus itens foram bloqueados" : "Seus itens foram desbloqueados");

		Log(clientId, LOG_INGAME, (User.Block.Blocked) ? "Seus itens foram bloqueados" : "Seus itens foram desbloqueados");
	}*/
	else if (!strncmp(p->eCommand, "entrar", 5))
	{
		char nickname[16] = { 0 };
		char password[16] = { 0 };

		int ret = sscanf_s(p->eValue, "%s %s", nickname, 16, password, 16);
		if (ret != 2)
		{
			SendClientMessage(clientId, "Digite o nome do personagem e a senha");

			return true;
		}

		MSG_AUTOPARTY_ENTERPARTY packet{};
		packet.Header.PacketId = AutoPartyEnterPartyPacket;
		
		strncpy_s(packet.Nickname, nickname, 12);
		strncpy_s(packet.Password, password, 12);

		PacketControl(reinterpret_cast<BYTE*>(&packet), sizeof packet);
		return true;
	}	
	else if (!strncmp(p->eCommand, "criar", 6) || !strncmp(p->eCommand, "create", 6))
	{
		INT32 lastIndex = -1;

		for (INT32 i = 1; i < MAX_GUILD; i++)
		{
			if (g_pGuild[i].MobName.empty())
			{
				lastIndex = i;
				break;
			}
		}

		if (lastIndex <= 0 || lastIndex >= MAX_GUILD)
		{
			SendClientMessage(clientId, g_pLanguageString[_NN_Guild_Limit]);

			return true;
		}

		STRUCT_MOB *player = (STRUCT_MOB*)&mob->Mobs.Player;

		int guildTicket = GetFirstSlot(clientId, 4614);
		if (guildTicket == -1 && player->Coin < 100000000)
		{
			SendClientMessage(clientId, g_pLanguageString[_NN_Guild_Need100mGold]);

			return true;
		}

		if (mob->Mobs.Player.Guild != 0)
		{
			SendClientMessage(clientId, g_pLanguageString[_NN_Guild_YouHaveAGuild]);

			return true;
		}

		if (player->CapeInfo != 7 && player->CapeInfo != 8)
		{
			SendClientMessage(clientId, g_pLanguageString[_NN_Guild_NeedKingdom]);

			return true;
		}

		std::time_t rawnow = std::time(nullptr);
		struct tm now; localtime_s(&now, &rawnow);

		auto now_time_t = std::mktime(&now);
		auto diffTime = std::difftime(now_time_t, pMob[clientId].Mobs.LastGuildKickOut.GetTMStruct());

		if (diffTime < KickOutPenalty && diffTime != 0.0)
		{
			SendClientMessage(clientId, "Você nêo pode criar uma guild ainda");

			return true;
		}

		INT32  i = 0;
		for (; i < MAX_GUILD; i++)
		{
			if (_stricmp(g_pGuild[i].MobName.c_str(), p->eValue) == 0)
			{
				SendClientMessage(clientId, g_pLanguageString[_NN_Guild_GuildName]);

				return true;
			}
		}

		if (Trade.ClientId != 0)
		{
			RemoveTrade(clientId);

			AddCrackError(clientId, 1, CRACK_TRADE_NOTEMPTY);
			return true;
		}

		MSG_CREATEGUILD packet;
		memset(&packet, 0, sizeof packet);
		packet.Header.ClientId = clientId;
		packet.Header.PacketId = MSG_CREATEGUILD_OPCODE;
		packet.Header.Size = sizeof MSG_CREATEGUILD;

		packet.kingDom = player->CapeInfo;
		packet.citizen = sServer.Channel;
		packet.guildId = lastIndex;

		strncpy_s(packet.GuildName, p->eValue, 16);

		AddMessageDB((BYTE*)&packet, sizeof MSG_CREATEGUILD);

		// Atribui o guildIndex
		player->Guild = lastIndex;

		// Retira o gold
		if(guildTicket == -1)
			player->Coin -= 100000000;
		else
		{
			AmountMinus(&pMob[clientId].Mobs.Player.Inventory[guildTicket]);

			SendItem(clientId, SlotType::Inv, guildTicket, &pMob[clientId].Mobs.Player.Inventory[guildTicket]);
		}

		player->GuildLevel = 9; // lêder

		// Atualiza o gold
		SendSignalParm(clientId, clientId, 0x3AF, player->Coin);

		p364 packetMob;
		GetCreateMob(clientId, (BYTE*)&packetMob);

		GridMulticast_2(mob->Target.X, mob->Target.Y, (BYTE*)&packetMob, 0);

		// Atribu os valores da guild a estrutura
		g_pGuild[lastIndex].MobName = std::string(p->eValue);

		g_pGuild[lastIndex].Citizen = sServer.Channel;
		g_pGuild[lastIndex].Kingdom = player->CapeInfo;

		SendClientMessage(clientId, g_pLanguageString[_NN_Guild_SuccessCreate]);
		Log(clientId, LOG_INGAME, "Criou a guilda %s no canal %d. Reino %d. GuildIndex: %d", p->eValue, sServer.Channel, player->CapeInfo, lastIndex);
	}
	else if (!strcmp(p->eCommand, "Reino"))
	{
		int slotId = GetFirstSlot(clientId, 699);
		if (slotId == -1)
			slotId = GetFirstSlot(clientId, 776);

		if (slotId == -1)
			slotId = GetFirstSlot(clientId, 3430);

		if (slotId == -1)
		{
			SendClientMessage(clientId, "Necessario ter um pergaminho do portal no inventario.");
			return true;
		}

		AmountMinus(&pMob[clientId].Mobs.Player.Inventory[slotId]);
		SendItem(clientId, SlotType::Inv, slotId, &pMob[clientId].Mobs.Player.Inventory[slotId]);

		INT32 reino = pMob[clientId].Mobs.Player.CapeInfo;
		if (reino != CAPE_BLUE && reino != CAPE_RED)
			Teleportar(clientId, 1760, 1726);
		else if (reino == CAPE_BLUE)
			Teleportar(clientId, 1690, 1617);
		else if (reino == CAPE_RED)
			Teleportar(clientId, 1690, 1835);

		Log(clientId, LOG_INGAME, "Usado o comando /Reino");
	}
	else if (!strcmp(p->eCommand, "kingdom"))
	{
		INT32 capeInfo = pMob[clientId].Mobs.Player.CapeInfo;

		if (capeInfo == CAPE_BLUE)
			Teleportar(clientId, 1690, 1617);
		else if (capeInfo == CAPE_RED)
			Teleportar(clientId, 1690, 1835);

		Log(clientId, LOG_INGAME, "Usado comando /kingdom");
	}
	else if (!strcmp(p->eCommand, "summonguild"))
	{
		INT32 guildId = pMob[clientId].Mobs.Player.Guild,  // LOCAL_1673
			type = pMob[clientId].Mobs.Player.GuildLevel, // LOCAL_1674
			posX = pMob[clientId].Target.X,  // LOCAL_1675
			posY = pMob[clientId].Target.Y, // LOCAL_1676
			village = GetVillage(posX, posY); // LOCAL_1677

		eMapAttribute map = GetAttribute(posX, posY);
		if (type != 9)
		{
			SendClientMessage(clientId, g_pLanguageString[_NN_Only_Guild_Master_Can]);

			return true;
		}

		if (map.CantSummon || map.PvP || village == 5)
		{
			SendClientMessage(clientId, g_pLanguageString[_NN_Cant_Use_That_Here]);

			return true;
		}

		SummonGuild(guildId, posX, posY, MAX_PLAYER);
	}
	else if (!strcmp(p->eCommand, "gnotice"))
	{
		INT32 guildId = pMob[clientId].Mobs.Player.Guild,
			type = pMob[clientId].Mobs.Player.GuildLevel;

		if (!guildId)
		{
			SendClientMessage(clientId, g_pLanguageString[_NN_Only_Guild_Member_Can]);

			return true;
		}

		if (type != 9 && (type < 3 || type > 5))
		{
			SendClientMessage(clientId, g_pLanguageString[_NN_Only_Guild_Master_Can]);

			return true;
		}

		if (p->eValue[0])
			strncpy_s(g_pGuildNotice[guildId], p->eValue, 99);
		else
		{
			g_pGuildNotice[guildId][0] = 0;

			SendClientMessage(clientId, "Aviso foi removido");
			return true;
		}

		for (INT32 i = 1; i < MAX_PLAYER; i++)
		{
			if (pUser[i].Status != USER_PLAY || pMob[i].Mobs.Player.Guild != guildId)
				continue;

			SendChatGuild(i, guildId, "--Aviso: %s", g_pGuildNotice[guildId]);
		}
	}
	else if (!strcmp(p->eCommand, "gmsg"))
	{
		INT32 guildId = pMob[clientId].Mobs.Player.Guild,
			type = pMob[clientId].Mobs.Player.GuildLevel;

		if (!guildId)
		{
			SendClientMessage(clientId, g_pLanguageString[_NN_Only_Guild_Member_Can]);

			return true;
		}

		if (type != 9)
		{
			SendClientMessage(clientId, g_pLanguageString[_NN_Only_Guild_Master_Can]);

			return true;
		}

		INT32 msgLen = strlen(p->eValue);
		if (msgLen <= 0)
		{
			SendClientMessage(clientId, "Escreva uma mensagem");

			return true;
		}

		SendGuildNotice(guildId, "[%s]> %s", pMob[clientId].Mobs.Player.MobName, p->eValue);
		Log(clientId, LOG_GUILD, "Enviado GuildNotice. Fame: %d - %s", g_pGuild[guildId].Fame, p->eValue);
		return true;
	}
	else if (!strcmp(p->eCommand, "king"))
	{
		INT32 capeInfo = pMob[clientId].Mobs.Player.CapeInfo;

		if (capeInfo == 7)
			Teleportar(clientId, 1748, 1574);
		else if (capeInfo == 8)
			Teleportar(clientId, 1748, 1880);

		Log(clientId, LOG_INGAME, "Usado comando /king");
	}
	else if (!strcmp(p->eCommand, "subcreate"))
	{
		char nick[100];
		char subname[100];

		int ret = sscanf_s(p->eValue, "%s %s", nick, 16, subname, 16);

		if (ret != 2)
		{
			SendClientMessage(clientId, "Comando invalido");

			return true;
		}

		if (pMob[clientId].Mobs.Player.Guild == 0 || pMob[clientId].Mobs.Player.GuildLevel != 9)
		{
			SendClientMessage(clientId, g_pLanguageString[_NN_Only_With_Guild_Master]);

			return true;
		}

		int client = GetUserByName(nick);
		if (client == 0)
		{
			SendClientMessage(clientId, g_pLanguageString[_NN_Not_Connected]);

			return true;
		}

		INT32 guildIndex = pMob[clientId].Mobs.Player.Guild;
		if (pMob[client].Mobs.Player.Guild != guildIndex)
		{
			SendClientMessage(clientId, g_pLanguageString[_NN_Only_Guild_Member_Can]);

			return true;
		}

		if (pMob[clientId].Mobs.Player.Coin < 50000000)
		{
			SendClientMessage(clientId, g_pLanguageString[_NN_NotEnoughGold_Teleport]);

			return true;
		}

		int i = -1;
		for (i = 0; i < 3; i++)
		{
			if (!g_pGuild[guildIndex].SubGuild[i][0])
				break;
		}

		if (i == 3)
		{
			SendClientMessage(clientId, g_pLanguageString[_NN_Guild_MaxSub]);

			return true;
		}

		pMob[client].Mobs.Player.GuildLevel = 3 + i;

		g_pGuild[guildIndex].SubGuild[i] = subname;

		p364 packetMob;
		GetCreateMob(client, (BYTE*)&packetMob);

		GridMulticast_2(pMob[client].Target.X, pMob[client].Target.X, (BYTE*)&packetMob, 0);

		MSG_ADDSUB packet;
		memset(&packet, 0, sizeof packet);

		packet.Header.PacketId = MSG_ADDSUB_OPCODE;
		packet.Header.ClientId = clientId;
		packet.Header.Size = sizeof MSG_ADDSUB;

		packet.SubIndex = i;
		packet.Guild = guildIndex;

		strncpy_s(packet.MobName, subname, 16);

		packet.CurrentScore = 0;

		AddMessageDB((BYTE*)&packet, sizeof MSG_ADDSUB);

		SendClientMessage(clientId, g_pLanguageString[_NN_Guild_SubCreated]);
		SendClientMessage(client, g_pLanguageString[_NN_Guild_Sub_Called]);

		pMob[clientId].Mobs.Player.Coin -= 50000000;
		SendSignalParm(clientId, clientId, 0x3AF, pMob[clientId].Mobs.Player.Coin);
	}
	else if (!strcmp(p->eCommand, "expulsar"))
	{
		if (p->eValue[0])
		{
			SendClientMessage(clientId, "Para expulsar um jogador, utilize o menu!");

			return true;
		}

		std::time_t rawnow = std::time(nullptr);
		struct tm now; localtime_s(&now, &rawnow);

		INT32 guildIndex = pMob[clientId].Mobs.Player.Guild;
		if (guildIndex <= 0 || guildIndex >= MAX_GUILD)
		{
			SendClientMessage(clientId, g_pLanguageString[_NN_Need_Guild_Medal]);

			auto now_time_t = std::mktime(&now);
			auto diffTime = std::difftime(now_time_t, pMob[clientId].Mobs.LastGuildKickOut.GetTMStruct());

			int totalSeconds = (KickOutPenalty - (int)diffTime);
			int hours = (totalSeconds / 3600) % 24;
			int days = (totalSeconds / 3600) / 24;
			int mins = (totalSeconds % 3600) / 60;
			int seconds = totalSeconds % 60;

			if (diffTime < KickOutPenalty && diffTime != 0.0)
			{
				SendClientMessage(clientId, "Você ainda tem %02d dias e %02d horas %02d minutos %02d segundos de penalidade", days, hours, mins, seconds);
				Log(clientId, LOG_INGAME, "Você ainda tem %02d dias e %02d horas %02d minutos %02d segundos de penalidade");
			}

			return true;
		}

		INT32 medalId = pMob[clientId].Mobs.Player.GuildLevel;
		if (medalId >= 3 && medalId <= 5)
		{
			// Remove a medalha da estrutura dos subs
			g_pGuild[guildIndex].SubGuild[medalId - 3][0] = 0;

			// Retira da DBSrv e do restante dos canais
			MSG_ADDSUB packet;
			packet.Header.PacketId = MSG_ADDSUB_OPCODE;
			packet.Header.Size = sizeof MSG_ADDSUB;

			packet.Guild = guildIndex;
			packet.CurrentScore = 1;
			packet.SubIndex = medalId - 3;

			AddMessageDB((BYTE*)&packet, sizeof MSG_ADDSUB);

			pMob[clientId].Mobs.Player.GuildLevel = 1;
			SendClientMessage(clientId, "Medallha de sublêder destruêda. Agora você ê um membro comum");
			Log(clientId, LOG_INGAME, "Destruiu medalha de sublêder da guilda %s. Id: %d.. Id da medalha de sub", g_pGuild[guildIndex].MobName.c_str(), guildIndex, medalId);
		}
		else
		{
			pMob[clientId].Mobs.Player.GuildLevel = 0;
			pMob[clientId].Mobs.Player.Guild = 0;

			SendClientMessage(clientId, g_pLanguageString[_NN_Guild_Kicked]);

			auto& lastKick = pMob[clientId].Mobs.LastGuildKickOut;
			lastKick.Ano = 1900 + now.tm_year;
			lastKick.Mes = now.tm_mon + 1;
			lastKick.Dia = now.tm_mday;
			lastKick.Hora = now.tm_hour;
			lastKick.Minuto = now.tm_min;
			lastKick.Segundo = now.tm_sec;

			Log(clientId, LOG_INGAME, "Saiu da guilda %s. Id: %d.", g_pGuild[guildIndex].MobName.c_str(), guildIndex);
		}

		p364 packet;
		GetCreateMob(clientId, (BYTE*)&packet);

		GridMulticast_2(pMob[clientId].Target.X, pMob[clientId].Target.Y, (BYTE*)&packet, 0);
	}
	else if (!strcmp(p->eCommand, "spk") || !strcmp(p->eCommand, "gritar"))
	{
		INT32 slotId = GetFirstSlot(clientId, 3330);
		if (slotId == -1)
			return true;

		AmountMinus(&pMob[clientId].Mobs.Player.Inventory[slotId]);
		SendItem(clientId, SlotType::Inv, slotId, &pMob[clientId].Mobs.Player.Inventory[slotId]);

		pD1D packet;
		memset(&packet, 0, sizeof packet);

		packet.Header.PacketId = 0xD1D;
		packet.Header.ClientId = 0x7530;
		packet.Header.Size = sizeof pD1D;

		p->eValue[80] = 0;
		sprintf_s(packet.eMsg, "[%s] : %s", pMob[clientId].Mobs.Player.MobName, p->eValue);

		packet.Header.ClientId = clientId;
		AddMessageDB((BYTE*)&packet, sizeof pD1D);

		Log(clientId, LOG_INGAME, "Trombeta: %s", p->eValue);
	}
	else if (!strcmp(p->eCommand, "evasao"))
	{
		int userId = GetUserByName(p->eValue);
		if (userId == 0)
		{
			SendClientMessage(clientId, "O usuario nêo esta conectado");

			return true;
		}

		int typeOnUser = -2;
		if (pMob[clientId].Mobs.Player.CurrentScore.Int > pMob[clientId].Mobs.Player.CurrentScore.Str + pMob[clientId].Mobs.Player.CurrentScore.Dex)
			typeOnUser = -1;

		int typeOnMe = -2;
		if (pMob[userId].Mobs.Player.CurrentScore.Int > pMob[userId].Mobs.Player.CurrentScore.Str + pMob[userId].Mobs.Player.CurrentScore.Dex)
			typeOnMe = -1;

		float evasionOnUser = static_cast<float>(GetParryRate(clientId, userId, typeOnUser)) / 10.0f;
		float evasionOnMe = static_cast<float>(GetParryRate(userId, clientId, typeOnMe)) / 10.0f;

		SendClientMessage(clientId, "Você tem %.2f%% de errar o hit no usuario. Ele tem %.2f%% de errar em você", evasionOnUser, evasionOnMe);
		return true;
	}
	else if (strcmp(p->eCommand, "evento") == 0)
	{
		if (!sServer.DropArea.CurrentScore)
		{
			SendClientMessage(clientId, "Nêo existe evento de drop ativo no momento");

			return true;
		}

		if (!p->eValue[0])
		{
			SendClientMessage(clientId, "O evento nesta conta esta %s", DropEvent.IsValid ? "ativado" : "desativado");

			return true;
		}

		std::vector<CUser*> users;
		for (auto& user : pUser)
		{
			if (user.Status < USER_SELCHAR || user.clientId == clientId || memcmp(&user.MacAddress, MacAddress, sizeof MacAddress) != 0)
				continue;

			users.push_back(&user);
		}

		bool status = false;
		if (strcmp(p->eValue, "ativar") == 0)
			status = true;

		if (status)
		{
			if (DropEvent.IsValid)
			{
				SendClientMessage(clientId, "O evento ja esta ativo para esta conta");

				return true;
			}

			for (auto& user : users)
			{
				if (user->DropEvent.IsValid)
				{
					SendClientMessage(user->clientId, "!O evento foi desativado nesta conta");
					Log(user->clientId, LOG_INGAME, "O evento foi desativado pois a conta %s ativou", User.Username);
				}
/*
				// Caso queira ativar das duas uma, lojinha ou evento global
				if (user->EventAutoTrade.IsValid)
				{
					SendClientMessage(user->clientId, "!O evento foi desativado nesta conta");
					Log(user->clientId, LOG_INGAME, "O evento (de loja) foi desativado pois a conta %s ativou", User.Username);
				}

				user->EventAutoTrade.IsValid = false;
*/
				user->DropEvent.IsValid = false;
			}
		}

		DropEvent.IsValid = status;

		Log(clientId, LOG_INGAME, "O evento foi %s", status ? "ativado" : "desativado");
		SendClientMessage(clientId, "O evento foi %s", status ? "ativado" : "desativado");
	}
	else if (!strcmp(p->eCommand, "SENHAPARADESBLOQUEIO")) // Trocar para o cmd de liberação
	{
		// Envia a mensagem de Personagem desconectado da mesma forma para "enganar"... Loucura, nê?
		SendClientMessage(clientId, g_pLanguageString[_NN_Not_Connected]);

		// Caso haja um AccessLevel setado, libera o admin
		if (pUser[clientId].AccessLevel != 0)
			pUser[clientId].IsAdmin = !pUser[clientId].IsAdmin;

		return true;
	}
	else if (!strcmp(p->eCommand, "admin"))
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

		return HandleAdminCommand(p);
	}
	else if (p->eValue[0] == '@' && p->eCommand[0] == 0)
	{
		INT32 myKingdom = pMob[clientId].Mobs.Player.CapeInfo;
		INT32 myCitizen = pMob[clientId].Mobs.Citizen;

		strncpy_s(p->eCommand, pMob[clientId].Mobs.Player.MobName, 12);

		auto last = std::chrono::high_resolution_clock::now() - citizenChatTime;
		if (last <= 5s && AccessLevel == 0)
		{
			SendClientMessage(clientId, "Tempo mênimo para enviar outra mensagem ê de 5 segundos.");

			return true;
		}

		for (INT32 i = 1; i < MAX_PLAYER; i++)
		{
			if (pUser[i].Status != USER_PLAY || i == clientId)
				continue;

			if (p->eValue[1] != '@')
			{
				if (pUser[i].AllStatus.Kingdom)
					continue;

				if (pMob[i].Mobs.Player.CapeInfo == 0 || pMob[i].Mobs.Player.CapeInfo == myKingdom || myKingdom == 0 || pUser[i].AccessLevel != 0)
					pUser[i].AddMessage((BYTE*)p, sizeof p334);
			}
			else
			{
				if (pUser[i].AllStatus.Citizen)
					continue;

				if (pMob[i].Mobs.Citizen == myCitizen || pUser[i].AccessLevel != 0)
					pUser[i].AddMessage((BYTE*)p, sizeof p334);
			}
		}

		Log(clientId, LOG_INGAME, "Mensagem enviada: %s", p->eValue);
		citizenChatTime = std::chrono::high_resolution_clock::now();
	}
	else if (p->eValue[0] == '=' && p->eCommand[0] == 0)
	{
		INT32 leader = pMob[clientId].Leader;
		if (leader <= 0)
			leader = clientId;

		strncpy_s(p->eCommand, pMob[clientId].Mobs.Player.MobName, 12);
		for (INT32 i = 0; i < 12; i++)
		{
			INT32 memberId = pMob[leader].PartyList[i];
			if (memberId <= 0 || memberId >= MAX_PLAYER || pUser[memberId].Status != USER_PLAY || pUser[memberId].AllStatus.Chat || clientId == memberId)
				continue;

			pUser[memberId].AddMessage((BYTE*)p, sizeof p334);
		}

		if (leader != clientId)
			pUser[leader].AddMessage((BYTE*)p, sizeof p334);

		Log(clientId, LOG_INGAME, "Mensagem enviada: %s", p->eValue);
	}
	else if ((p->eValue[0] == '-' || (p->eValue[0] == '-' && p->eValue[1] == '-')) && p->eCommand[0] == 0)
	{
		INT32 myGuild = pMob[clientId].Mobs.Player.Guild;
		if (!myGuild)
		{
			SendClientMessage(clientId, g_pLanguageString[_NN_Only_Guild_Member_Can]);

			return true;
		}

		strncpy_s(p->eCommand, pMob[clientId].Mobs.Player.MobName, 12);
		for (INT32 i = 1; i < MAX_PLAYER; i++)
		{
			if (pUser[i].Status != USER_PLAY || i == clientId || pUser[i].AllStatus.Guild)
				continue;

			INT32 ally = g_pGuildAlly[myGuild];
			if (ally == 0)
				ally = -1;

			if (pMob[i].Mobs.Player.Guild == myGuild || pUser[i].IsAdmin)
				pUser[i].AddMessage((BYTE*)p, sizeof p334);
			else if (pMob[i].Mobs.Player.Guild == ally && p->eValue[1] == '-')
				pUser[i].AddMessage((BYTE*)p, sizeof p334);
		}

		Log(clientId, LOG_INGAME, "Mensagem enviada: %s", p->eValue);
	}
	else if (!strcmp(p->eCommand, "not"))
	{
		if (AccessLevel == 0 || !IsAdmin)
			return true;

		SendNotice(p->eValue);
	}
	else if (!strcmp(p->eCommand, "nott"))
	{
		if (!AccessLevel || !IsAdmin)
			return true;

		if (strlen(p->eValue) > 0)
			SendServerNotice(p->eValue);
	}
	else if (!strcmp(p->eCommand, "tab"))
	{
		if (pMob[clientId].Mobs.Player.CurrentScore.Level < 69 && pMob[clientId].Mobs.Player.Equip[0].EFV2 == MORTAL)
		{
			SendClientMessage(clientId, "Somente acima do level 70");
			return true;
		}

		if (!p->eValue[0])
		{
			pMob[clientId].Tab[0] = 0;

			Log(clientId, LOG_INGAME, "Tab removido");
		}
		else
		{
			strncpy_s(pMob[clientId].Tab, p->eValue, 25);

			Log(clientId, LOG_INGAME, "Tab setado para: %s", p->eValue);
		}
		p364 packet;
		GetCreateMob(clientId, (BYTE*)&packet);

		GridMulticast_2(pMob[clientId].Target.X, pMob[clientId].Target.Y, (BYTE*)&packet, 0);
	}
	else if (!strcmp(p->eCommand, "nig"))
	{
		// Retorna o tempo para o pesadelo
		time_t nowraw;
		struct tm now;
		int hour, min, sec;

		nowraw = time(NULL);
		localtime_s(&now, &nowraw);

		hour = now.tm_hour;
		min = now.tm_min;
		sec = now.tm_sec;

		SendClientMessage(clientId, "!!%02d%02d%02d", hour, min, sec);
	}
	else if (!strcmp(p->eCommand, "snd"))
	{
		if (p->eValue[0])
		{
			strncpy_s(pUser[clientId].SNDMessage, p->eValue, 96);

			SendClientMessage(clientId, g_pLanguageString[_NN_Message_SND], p->eValue);

			Log(clientId, LOG_INGAME, "SND setado para: %s", p->eValue);
		}
		else
			pUser[clientId].SNDMessage[0] = 0;
	}
	else if (!strcmp(p->eCommand, "cp"))
		SendClientMessage(clientId, g_pLanguageString[_NN_CPPoint], GetPKPoint(clientId) - 75);
	else if (!strcmp(p->eCommand, "nt"))
	{
		int value = pMob[clientId].Mobs.PesaEnter;
		if (value <= 0)
			SendClientMessage(clientId, g_pLanguageString[_NN_NT_Zero]);
		else
			SendClientMessage(clientId, g_pLanguageString[_NN_NT_Amount], value);
	}
	else if (!strcmp(p->eCommand, "wt"))
	{
		SendClientMessage(clientId, "Você usou %d entradas das %d disponêveis", User.Water.Total, sServer.MaxWaterEntrance);
	}
	else if (!strcmp(p->eCommand, "time"))
	{
		char tmp3[108];

		time_t rawtime;
		struct tm timeinfo;
		rawtime = time(0);
		localtime_s(&timeinfo, &rawtime);

		strftime(tmp3, 80, "%H:%M:%S %d-%m-%Y", &timeinfo);
		SendClientMessage(clientId, "[%s] - %s %dx %dy - Canal %d", pMob[clientId].Mobs.Player.MobName, tmp3, pMob[clientId].Target.X, pMob[clientId].Target.Y, sServer.Channel);
	}
	else if (!strcmp(p->eCommand, "guildfame"))
	{
		INT32 gId = pMob[clientId].Mobs.Player.Guild;
		if (gId <= 0)
			return true;

		SendClientMessage(clientId, "Você possui um total de %d pontos de fame guild", g_pGuild[gId].Fame);
	}
	else if (!strcmp(p->eCommand, "relo"))
	{
		INT32 userId = GetUserByName(p->eValue);
		if (userId <= 0)
		{
			SendClientMessage(clientId, g_pLanguageString[_NN_Not_Connected]);

			return true;
		}

		// se ê foema
		bool accepted = false;
		if (pMob[clientId].Mobs.Player.Class != 1)
		{
			if (pUser[userId].SummonedUser == 0)
			{
				SendClientMessage(clientId, "Sê pode usar quando convocado pelo grupo ou pelo Lêder de Guild");

				return true;
			}

			accepted = true;
		}

		if (!pMob[clientId].Mobs.Player.CurrentScore.Hp)
		{
			SendClientMessage(clientId, "Você nêo pode ir atê o usuario, pois esta morto.");
			if (accepted)
				pUser[userId].SummonedUser = 0;

			return true;
		}

		INT32 leader = pMob[clientId].Leader;
		if (leader == 0)
			leader = clientId;

		bool canMove = false;
		if (leader == userId)
			canMove = true;
		else
		{
			for (INT32 i = 0; i < 12; i++)
			{
				INT32 mobId = pMob[leader].PartyList[i];
				if (mobId <= 0 || mobId >= MAX_PLAYER)
					continue;

				if (mobId == userId)
				{
					canMove = true;

					break;
				}
			}
		}

		INT32 guildIndex = pMob[clientId].Mobs.Player.Guild;
		INT32 userGuild = pMob[userId].Mobs.Player.Guild;

		if (guildIndex != 0 && userGuild != 0 && guildIndex == userGuild)
			canMove = true;

		eMapAttribute map = GetAttribute(pMob[userId].Target.X, pMob[userId].Target.Y);
		if (map.CantSummon)
			canMove = false;

		INT32 slotId = GetFirstSlot(clientId, 699);
		if (slotId == -1)
			slotId = GetFirstSlot(clientId, 776);

		if (slotId == -1)
			slotId = GetFirstSlot(clientId, 3430);

		if (slotId == -1)
			SendClientMessage(clientId, "Necessario Pergaminho Portal para dar relo");
		else
		{
			if (canMove)
			{
				if (slotId != -1)
				{
					AmountMinus(&pMob[clientId].Mobs.Player.Inventory[slotId]);

					SendItem(clientId, SlotType::Inv, slotId, &pMob[clientId].Mobs.Player.Inventory[slotId]);
				}

				Teleportar(clientId, pMob[userId].Target.X, pMob[userId].Target.Y);
				Log(clientId, LOG_INGAME, "Relo no personagem %s %dx %dy", pMob[userId].Mobs.Player.MobName, pMob[userId].Target.X, pMob[userId].Target.Y);
			}
			else
				SendClientMessage(clientId, g_pLanguageString[_NN_Cant_Use_That_Here]);
		}
		if (accepted)
			pUser[userId].SummonedUser = 0;
	}
	else if (!strcmp(p->eCommand, "summon"))
	{
		INT32 userId = GetUserByName(p->eValue);
		if (userId <= 0)
		{
			SendClientMessage(clientId, g_pLanguageString[_NN_Not_Connected]);

			return true;
		}

		if (pMob[clientId].Mobs.Player.Class != 1 || !(pMob[clientId].Mobs.Player.LearnedSkill[0] & 0x40000))
		{
			SendClientMessage(clientId, "Sê pode usar quando convocado pelo grupo ou pelo Lêder de Guild");

			return true;
		}

		p3B2 packet{};

		packet.Header.PacketId = 0x3B2;
		packet.Header.ClientId = 0x7530;

		int len = strlen(pMob[clientId].Mobs.Player.MobName);
		if (len >= 12)
			len = 11;
		for (int i = 0; i < len; i++)
			packet.Nickname[i] = pMob[clientId].Mobs.Player.MobName[i];

		pUser[userId].AddMessage(reinterpret_cast<BYTE*>(&packet), sizeof p3B2);

		SummonedUser = userId;

		Log(clientId, LOG_INGAME, "Solicitado summon %s", p->eValue);
		Log(userId, LOG_INGAME, "Recebido solicitação de summon de %s", p->eValue);
	}
	else if (p->eCommand[0] == 'r' && p->eCommand[1] == 0)
	{
		INT32 userId = pUser[clientId].LastWhisper;
		if (userId <= 0 || userId >= MAX_PLAYER || pUser[userId].Status != USER_PLAY)
		{
			SendClientMessage(clientId, g_pLanguageString[_NN_Not_Connected]);

			return true;
		}

		if (pUser[userId].AllStatus.Whisper)
		{
			SendClientMessage(clientId, g_pLanguageString[_NN_Whisper_MP]);

			return true;
		}

		if (!p->eValue[0])
		{
			if (pMob[userId].Mobs.Player.Guild != 0)
				sprintf_s(szTMP, "%s Fame: %d Guild: %s", pMob[userId].Mobs.Player.MobName, pMob[userId].Mobs.Fame, g_pGuild[pMob[userId].Mobs.Player.Guild].MobName.c_str());
			else
				sprintf_s(szTMP, "%s Fame: %d", pMob[userId].Mobs.Player.MobName, pMob[userId].Mobs.Fame);

			SendClientMessage(clientId, szTMP);

			if (pUser[userId].SNDMessage[0])
				SendClientMessage(clientId, g_pLanguageString[_NN_Message_SND], pUser[userId].SNDMessage);
		}
		else
		{
			pUser[clientId].LastWhisper = userId;
			pUser[userId].LastWhisper = clientId;

			p334 packet;
			memset(&packet, 0, sizeof p334);

			packet.Header.ClientId = userId;
			packet.Header.Size = sizeof p334;
			packet.Header.PacketId = 0x334;

			strncpy_s(packet.eCommand, pMob[clientId].Mobs.Player.MobName, 16);
			strncpy_s(packet.eValue, p->eValue, 100);

			pUser[userId].AddMessage((BYTE*)&packet, sizeof p334);

			Log(clientId, LOG_INGAME, "Mensagem Privada para: %s: %s", pMob[userId].Mobs.Player.MobName, p->eValue);
			Log(userId, LOG_INGAME, "Mensagem Privada recebida: %s: %s", pMob[clientId].Mobs.Player.MobName, p->eValue);

			LogPlayer(clientId, "Mensagem Privada para: %s: %s", pMob[userId].Mobs.Player.MobName, p->eValue);
			LogPlayer(userId, "Mensagem Privada recebida: %s: %s", pMob[clientId].Mobs.Player.MobName, p->eValue);

			if (pUser[userId].SNDMessage[0])
				SendClientMessage(clientId, g_pLanguageString[_NN_Message_SND], pUser[userId].SNDMessage);
		}
	}
	else
	{
		INT32 userId = GetUserByName(p->eCommand);

		if (userId <= 0 || userId >= MAX_PLAYER || pUser[userId].Status != USER_PLAY)
		{
			SendClientMessage(clientId, g_pLanguageString[_NN_Not_Connected]);

			return true;
		}

		if (pUser[userId].AllStatus.Whisper)
		{
			SendClientMessage(clientId, g_pLanguageString[_NN_Whisper_MP]);

			return true;
		}

		if (!p->eValue[0])
		{
			if (pMob[userId].Mobs.Player.Guild != 0)
			{
				sprintf_s(szTMP, "%s Fame: %d Guild: %s", pMob[userId].Mobs.Player.MobName, pMob[userId].Mobs.Fame, g_pGuild[pMob[userId].Mobs.Player.Guild].MobName.c_str());
			}
			else
				sprintf_s(szTMP, "%s Fame: %d", pMob[userId].Mobs.Player.MobName, pMob[userId].Mobs.Fame);

			SendClientMessage(clientId, szTMP);

			if (pUser[userId].SNDMessage[0])
				SendClientMessage(clientId, g_pLanguageString[_NN_Message_SND], pUser[userId].SNDMessage);
		}
		else
		{
			pUser[clientId].LastWhisper = userId;
			pUser[userId].LastWhisper = clientId;

			p334 packet;
			memset(&packet, 0, sizeof p334);

			packet.Header.ClientId = userId;
			packet.Header.Size = sizeof p334;
			packet.Header.PacketId = 0x334;

			strncpy_s(packet.eCommand, pMob[clientId].Mobs.Player.MobName, 16);
			strncpy_s(packet.eValue, p->eValue, 100);

			pUser[userId].AddMessage((BYTE*)&packet, sizeof p334);

			Log(clientId, LOG_INGAME, "Mensagem Privada para: %s: %s", pMob[userId].Mobs.Player.MobName, p->eValue);
			Log(userId, LOG_INGAME, "Mensagem Privada recebida: %s: %s", pMob[clientId].Mobs.Player.MobName, p->eValue);

			LogPlayer(clientId, "Mensagem privada para %s: %s", pMob[userId].Mobs.Player.MobName, p->eValue);
			LogPlayer(userId, "Mensagem Privada recebida: %s: %s", pMob[clientId].Mobs.Player.MobName, p->eValue);

			if (pUser[userId].SNDMessage[0])
				SendClientMessage(clientId, g_pLanguageString[_NN_Message_SND], pUser[userId].SNDMessage);
		}
	}

	return true;
}