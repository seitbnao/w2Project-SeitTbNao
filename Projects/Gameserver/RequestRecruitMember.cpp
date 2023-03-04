#include "cServer.h"
#include "Basedef.h"
#include "SendFunc.h"
#include "GetFunc.h"

bool CUser::RequestRecruitMember(PacketHeader *Header)
{ // p3D5
	pMsgSignal2 *p = (pMsgSignal2*)(Header);

	if (p->Value <= 0 || p->Value >= MAX_PLAYER)
		return false;

	time_t rawnow = time(NULL);
	struct tm now; localtime_s(&now, &rawnow);

	INT32 week = now.tm_wday;
	if (week == DOMINGO)
	{
		SendClientMessage(clientId, g_pLanguageString[_NN_Guild_CantRecruit_Day]);

		return true;
	}

	INT32 memberType = pMob[clientId].Mobs.Player.GuildLevel;
	if (memberType != 9 && (memberType < 3 || memberType > 9))
	{
		SendClientMessage(clientId, g_pLanguageString[_NN_Guild_CantKick_Guild]);

		return true;
	}

	INT32 userId = p->Value;
	if (std::find(std::begin(g_pCapesID[2]), std::end(g_pCapesID[2]), pMob[userId].Mobs.Player.Equip[15].sIndex) != std::end(g_pCapesID[2]))
	{
		SendClientMessage(clientId, g_pLanguageString[_NN_Guild_CantRecruit_Kingdom]);

		return true;
	}

	if (pMob[userId].Mobs.Player.CapeInfo != 0 && pMob[clientId].Mobs.Player.CapeInfo != pMob[userId].Mobs.Player.CapeInfo)
	{
		SendClientMessage(clientId, g_pLanguageString[_NN_Guild_CantRecruit_Kingdom]);

		return true;
	}

	INT32 guildIndex = pMob[clientId].Mobs.Player.Guild;
	if (pMob[p->Value].Mobs.Player.Guild != 0)
	{
		SendClientMessage(clientId, "Usuario ja possui uma guild");

		return true;
	}

	if (pMob[clientId].Mobs.Player.Coin < 4000000)
	{
		SendClientMessage(clientId, g_pLanguageString[_NN_Guild_CantRecruit_Gold], 4000000);

		return true;
	}

	auto now_time_t = std::mktime(&now);
	auto diffTime = std::difftime(now_time_t, pMob[userId].Mobs.LastGuildKickOut.GetTMStruct());
 	if (diffTime < KickOutPenalty && diffTime != 0.0)
	{
		SendClientMessage(clientId, "O usuario ainda nêo pode ser recrutado por penalidade de sair da guild");

		return true;
	}

	MSG_RECRUITREQUEST packet{};
	packet.Header.PacketId = RecruitRequestPacket;
	packet.Header.Size = sizeof packet;

	packet.ClientId = clientId;
	packet.GuildId = guildIndex;
	strncpy_s(packet.Nickname, pMob[clientId].Mobs.Player.MobName, 12);
	strncpy_s(packet.GuildName, g_pGuild[guildIndex].MobName.c_str(), 16);

	pUser[userId].AddMessage(reinterpret_cast<BYTE*>(&packet), sizeof packet);
	
	invitedUsers.push_back(pMob[clientId].Mobs.Player.MobName);

		/*
	//pMob[userId].Mobs.MedalId = 508;
	pMob[userId].Mobs.Player.Guild = guildIndex;
	pMob[userId].Mobs.Player.GuildLevel = 1;

	//GetGuild(userId);

	pMob[clientId].Mobs.Player.Coin -= 4000000;
	SendSignalParm(clientId, clientId, 0x3AF, pMob[clientId].Mobs.Player.Coin);

	SendClientMessage(userId, g_pLanguageString[_NN_Guild_Recruit_GuildEnter]);
	SendClientMessage(clientId, g_pLanguageString[_NN_Guild_Recruit], pMob[userId].Mobs.Player.ItemName);

	p364 packetMob;
	GetCreateMob(userId, (BYTE*)&packetMob);

	GridMulticast_2(pMob[userId].Target.X, pMob[userId].Target.Y, (BYTE*)&packetMob, 0);

	Log(clientId, LOG_INGAME, "Recrutou %s", pMob[userId].Mobs.Player.ItemName);
	Log(userId, LOG_INGAME, "Foi recrutado por %s. Guildname: %s. sIndex: %d", pMob[clientId].Mobs.Player.ItemName, g_pGuild[guildIndex].ItemName.c_str(), guildIndex);

	LogPlayer(clientId, "Você recrutou %s para sua guild", pMob[userId].Mobs.Player.ItemName);
	LogPlayer(userId, "Você foi recrutado por %s para a guild %s", pMob[clientId].Mobs.Player.ItemName, g_pGuild[guildIndex].ItemName.c_str());
	*/
	return true;
}