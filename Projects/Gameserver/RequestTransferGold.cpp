#include "cServer.h"
#include "Basedef.h"
#include "SendFunc.h"
#include "GetFunc.h"

// TODO: Verificar se ele esta perto do guarda cargas para passar o gold.
bool CUser::RequestTransferGoldToInv(PacketHeader *Header)
{
	pMsgSignal *p = (pMsgSignal*)(Header);

	if(pMob[clientId].Mobs.Player.CurrentScore.Hp <= 0 || Status != USER_PLAY)
	{
		SendHpMode(clientId);

		return true;
	}

		
	if(Trade.ClientId != 0)
		RemoveTrade(clientId);

	UINT32 gold = p->Value;
	if(User.Storage.Coin < gold || gold < 0 || gold > 2000000000)
	{
		SendClientMessage(clientId, g_pLanguageString[_NN_Cant_Withdraw_That_Much]);

		return true;
	}

	INT32 totalGold = pMob[clientId].Mobs.Player.Coin + gold;
	if(totalGold > 2000000000 || totalGold < 0)
	{
		SendClientMessage(clientId, g_pLanguageString[_NN_Cant_get_more_than_2G]);

		return true;
	}

	pMob[clientId].Mobs.Player.Coin = totalGold;
	User.Storage.Coin -= gold;

	p->Header.ClientId = 0x7530;

	AddMessage((BYTE*)p, sizeof pMsgSignal);
	SendCargoCoin(clientId);
	
	Log(clientId, LOG_INGAME, "Transferiu %d de gold para o inventario. %d gold no banco. %d no inventario", p->Value, User.Storage.Coin, totalGold);
	LogPlayer(clientId, "Transferiu %d de gold para o inventario. %d gold no banco. %d no inventario", p->Value, User.Storage.Coin, totalGold);

	SaveUser(clientId, 0);
	return true;
}


bool CUser::RequestTransferGoldToBank(PacketHeader *Header)
{
	pMsgSignal *p = (pMsgSignal*)(Header);

	if(pMob[clientId].Mobs.Player.CurrentScore.Hp <=0 || Status != USER_PLAY)
	{
		SendHpMode(clientId);

		return true;
	}

	if(Trade.ClientId != 0)
		RemoveTrade(clientId);

	INT32 gold = p->Value;

	if(pMob[clientId].Mobs.Player.Coin < gold || gold < 0 || gold > 2000000000)
	{
		SendClientMessage(clientId, g_pLanguageString[_NN_Cant_Deposit_That_Much]);

		return true;
	}

	INT32 totalGold = User.Storage.Coin + gold;
	if(totalGold < 0 || totalGold > 2000000000)
	{
		SendClientMessage(clientId, g_pLanguageString[_NN_Cant_get_more_than_2G]);

		return true;
	}

	pMob[clientId].Mobs.Player.Coin -= gold;

	User.Storage.Coin = totalGold;

	AddMessage((BYTE*)p, sizeof pMsgSignal);
		
	p->Header.ClientId = 0x7530;
	SendCargoCoin(clientId);
	
	Log(clientId, LOG_INGAME, "Transferiu %d de gold para o banco. %d gold no banco. %d no inventario", p->Value, totalGold, pMob[clientId].Mobs.Player.Coin);
	LogPlayer(clientId, "Transferiu %d de gold para o banco. %d gold no banco. %d no inventario", p->Value, totalGold, pMob[clientId].Mobs.Player.Coin);

	SaveUser(clientId, 0);
	return true;
}