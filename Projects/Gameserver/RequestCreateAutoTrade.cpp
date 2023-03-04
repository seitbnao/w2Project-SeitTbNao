#include "cServer.h"
#include "Basedef.h"
#include "SendFunc.h"
#include "GetFunc.h"
#include <chrono>

bool CUser::RequestCreateAutoTrade(PacketHeader *Header)
{
	p397 *p = (p397*)(Header);
	
	if(!pMob[clientId].Mobs.Player.CurrentScore.Hp || CurrentScore != USER_PLAY)
	{
		SendHpMode(clientId);
		AddCrackError(clientId, 10, 88);

		RemoveTrade(clientId);
		return true;
	}

	if (pUser[clientId].Trade.ClientId)
	{
		RemoveTrade(clientId);
		return true;
	}
	
	if(User.Block.Blocked)
	{
		SendClientMessage(clientId, "Desbloqueie seus itens para poder movimenta-los");
		
		RemoveTrade(clientId);
		return true;
	}
		
	INT32 posX = pMob[clientId].Target.X;
	INT32 posY = pMob[clientId].Target.Y;
	INT32 villageId = GetVillage(posX, posY);

	if(villageId < 0 || villageId >= 5)
	{
		SendClientMessage(clientId, g_pLanguageString[_NN_OnlyVillage]);
		
		RemoveTrade(clientId);
		return true;
	}

	if(!sServer.NoviceChannel)
	{
		SendClientMessage(clientId, "Somente no canal novato para criação de lojas");
		
		RemoveTrade(clientId);
		return true;
	}

	if (clientId <= 0 || clientId >= MAX_PLAYER)
		return true;

	INT32 itemId = p->sIndex; // LOCAL_1204
	INT32 i = 0;
	int total = 0;
	for( ; i < 12; i ++)
	{
		if(p->Coin[i] < 0 || p->Coin[i] > 1999999999) 
			return false;

		if(p->Item[i].sIndex == 0 && p->Coin[i] != 0)
			return false;

		if(p->Item[i].sIndex == 0)
			continue;

		if(p->Item[i].sIndex == 508 || p->Item[i].sIndex == 509 || p->Item[i].sIndex == 522 || (p->Item[i].sIndex >= 526 && p->Item[i].sIndex <= 531) || p->Item[i].sIndex == 446
			|| p->Item[i].sIndex == 3993 || p->Item[i].sIndex == 3994)
			return false;

		if (GetItemAbility(&p->Item[i], EF_NOTRADE))
		{
			SendClientMessage(clientId, "Este item não é negociavel.");

			return false;
		}

		INT32 itemSlot = p->Slot[i]; // LOCAL1206
		if(itemSlot < 0 || itemSlot >= 120)
			return false;

		INT32 verify = memcmp(&User.Storage.Item[itemSlot], &p->Item[i], 8);
		if (verify != 0)
		{
			RemoveTrade(clientId);

			return true;
		}

		if (p->Coin[i] >= 10000000)
			total++;
	}

	p->Unknown = g_pCityZone[villageId].perc_impost;
	p->MobName[23] = '\0';
	p->MobName[22] = '\0';

	strncpy_s((char*)AutoTradeName, 24, p->MobName, 24);

	if (sServer.AutoTradeEvent.CurrentScore && strstr(AutoTradeName, "#EventoLoja") != nullptr)
	{
		CUser* userWithAutoTrade = nullptr;
		CUser* userWithEventOnline = nullptr;
		for (int i = 1; i < MAX_PLAYER; i++)
		{
			auto& user = pUser[i];
			if (user.CurrentScore != USER_PLAY || i == clientId)
				continue;

			if (memcmp(MacAddress, user.MacAddress, 8) != 0)
				continue;
 
			if (!user.IsAutoTrading || strstr(user.AutoTradeName, "#EventoLoja") == nullptr)
				continue;

			userWithAutoTrade = &user;
			break;
		}

		if (userWithAutoTrade != nullptr)
		{
			Log(clientId, LOG_INGAME, "Loja não valida para recebimento do item do evento pois a conta %s estava online também", userWithAutoTrade->User.Username);
			SendClientMessage(clientId, "!Esta loja não esta valida para recebimento do item do evento.");

			EventAutoTrade.IsValid = false;
		}
		else if (userWithEventOnline != nullptr)
		{
			Log(clientId, LOG_INGAME, "Loja não valida para recebimento do item do evento pois a conta %s estava com o evento online ativo", userWithEventOnline->User.Username);
			SendClientMessage(clientId, "!Esta loja não esta valida para recebimento do item do evento.");

			EventAutoTrade.IsValid = false;
		}
		else
		{
			Log(clientId, LOG_INGAME, "Esta loja valida para recebimento do item do evento");
			SendClientMessage(clientId, "!Loja valida para recebimento do item do evento");

			EventAutoTrade.IsValid = true;
			EventAutoTrade.LastTime = std::chrono::high_resolution_clock::now();

			GenerateNewAutoTradeTime();
		}
	}

	// Verifica se o total de itens com 10 milhaes de gold a maior que 5
	if (total >= 5)
	{
		bool has = false;
		for (int j = 1; j < MAX_PLAYER; j++)
		{
			if (pUser[j].CurrentScore != USER_PLAY || j == clientId || !pUser[j].IsAutoTrading || !pUser[j].PremierStore.CurrentScore)
				continue;

			if (!memcmp(pUser[j].MacAddress, pUser[clientId].MacAddress, 6))
			{
				has = true;

				break;
			}
		}

		if (!has)
		{
			if (PremierStore.Wait == 0 || (PremierStore.Wait != 0 && PremierStore.Wait > 300))
			{ // abre a loja normalmente com o status normal 
				PremierStore.CurrentScore = 1;
				PremierStore.Time = 0;
				PremierStore.Count = 0;

				SendClientMessage(clientId, "Loja premiada ativada!");
			}
			else
				PremierStore.Wait = 0;
		}
	}

	IsAutoTrading = true;

	memcpy(&AutoTrade, p, sizeof AutoTrade);

	p363 sm{};
	GetCreateMobTrade(clientId, (BYTE*)&sm);
	GridMulticast_2(posX, posY, (BYTE*)&sm, 0);
	std::stringstream str;
	str << "Informações dos itens da loja\n";
	str << "Nome da loja: " << p->MobName << "\n";
	str << "Posição criada: " << pMob[clientId].Target.X << "x " << pMob[clientId].Target.Y << "\n";
	str << "Total de itens com valor superior a 10 milhões: " << total << "\n";
	str << "[ITENS]" << "\n";

	for (int i = 0; i < 12; i++)
	{
		if (p->Item[i].sIndex == 0 && p->Coin[i] != 0)
			return false;

		if (p->Item[i].sIndex == 0)
			continue;

		str << "Slot [" << std::to_string(p->Slot[i]) << "] - " << g_pItemList[p->Item[i].sIndex].ItemName << " " << p->Item[i].toString().c_str() << ". Preço de " << p->Coin[i] << "\n";
	}

	Log(clientId, LOG_INGAME, str.str().c_str());
	return true;
}