#include "cServer.h"
#include "Basedef.h"
#include "SendFunc.h"
#include "GetFunc.h"

bool CUser::RequestBuyAutoTrade(PacketHeader *Header)
{
	p398 *p = (p398*)(Header);

	STRUCT_MOB *player = (STRUCT_MOB*)(&pMob[clientId].Mobs.Player);

	if(!player->CurrentScore.Hp || Status != USER_PLAY)
		return false;

	if(pUser[clientId].Trade.ClientId != 0)
	{
		RemoveTrade(clientId);
		
		AddCrackError(clientId, 1, CRACK_TRADE_NOTEMPTY);
		return true;
	}
	
	if(User.Block.Blocked)
	{
		SendClientMessage(clientId, "Desbloqueie seus itens para poder movimenta-los");

		return true;
	}

	INT32 mobId = p->TargetID;
	INT32 itemPrice = p->Price;
	INT32 itemTaxe = p->Tax;
	INT32 slotId = p->Pos;

	if(mobId <= 0 || mobId >= MAX_PLAYER)
		return false;
	
	STRUCT_MOB *mob = (STRUCT_MOB*)(&pMob[mobId].Mobs.Player);

	if(pUser[mobId].Status != USER_PLAY)
		return false;

	if (pMob[clientId].Target.X < pMob[mobId].Target.X - VIEWGRIDX || pMob[clientId].Target.X > pMob[mobId].Target.X + VIEWGRIDX || pMob[clientId].Target.Y < pMob[mobId].Target.Y - VIEWGRIDY || pMob[clientId].Target.Y > pMob[mobId].Target.Y + VIEWGRIDY)
	{
		SendClientMessage(clientId, g_pLanguageString[_NN_SoFar_BuyShop]);

		return true;
	}


	if(slotId < 0 || slotId >= 10)
		return false;

	INT32 slotBank = pUser[mobId].AutoTrade.CarryPos[slotId];
	if(slotBank < 0 || slotBank >= 128)
		return false;

	if(itemTaxe != pUser[mobId].AutoTrade.Tax)
		return false;

	if(itemPrice != pUser[mobId].AutoTrade.TradeMoney[slotId])
		return false;

	if(pUser[mobId].AutoTrade.Item[slotId].sIndex == 0)
	{
		SendClientMessage(clientId, g_pLanguageString[_NN_AlwaysBuy_BuyShop]);

		return true;
	}

	INT32 itemCheck = memcmp(&p->item, &pUser[mobId].AutoTrade.Item[slotId], 8);
	if(itemCheck != 0)
	{
		RemoveTrade(clientId);

		return true;
	}

	itemCheck = memcmp(&pUser[mobId].User.Storage.Item[slotBank], &pUser[mobId].AutoTrade.Item[slotId], 8);
	if(itemCheck != 0)
	{
		RemoveTrade(clientId);

		return true;
	}

	if(player->Coin < itemPrice)
	{ // Coin insuficiente
		SendClientMessage(clientId, g_pLanguageString[_NN_NotEnoughGold_BuyShop]);

		return true;
	}

	if(itemPrice <= 1000) 
	{
		SendClientMessage(clientId, "Este item esta em modo demonstração");

		return true;
	}

	STRUCT_ITEM *item = (STRUCT_ITEM*)&p->item;
	
	INT32 nullSlot = GetFirstSlot(clientId, 0);
	if(nullSlot < 0 || nullSlot >= MAX_INVEN)
	{//sem espaêo no inventario
		SendClientMessage(clientId, g_pLanguageString[_NN_You_Have_No_Space_To_Trade]);

		return true;
	}
	
	INT32 impost = 0;
	INT32 totalPrice = itemPrice; // local1190
	
	INT32 mobTargetX = pMob[mobId].Target.X; // local1178
	INT32 mobTargetY = pMob[mobId].Target.Y; // local1179
	INT32 villageId = GetVillage(mobTargetX, mobTargetY); // local1180

	if (villageId < 0 || villageId >= 5)
	{ // Permitido somente dentro da cidade
		SendClientMessage(clientId, g_pLanguageString[_NN_OnlyVillage]);
		return true;
	}

	INT32 tax = g_pCityZone[villageId].perc_impost; 
	if(itemPrice >= 100000)
	{
		impost = itemPrice / 100 * tax;

		totalPrice = itemPrice - impost;
	}

	UINT64 total = pUser[mobId].User.Storage.Coin + totalPrice;
	if(total > 2000000000)
	{
		SendClientMessage(clientId, g_pLanguageString[_NN_Cant_get_more_than_2G]);

		return true;
	}

	g_pCityZone[villageId].impost += impost;

	memcpy(&player->Inventory[nullSlot], item, 8);

	SendItem(clientId, SlotType::Inv, nullSlot, &player->Inventory[nullSlot]);

	pUser[mobId].AutoTrade.CarryPos[slotId] = -1; // -1;
	memset(&pUser[mobId].AutoTrade.Item[slotId], 0, 8);
	pUser[mobId].AutoTrade.TradeMoney[slotId] = 0;
	memset(&pUser[mobId].User.Storage.Item[slotBank], 0, 8);


	SendItem(mobId, SlotType::Storage, slotBank, &pUser[mobId].User.Storage.Item[slotBank]);

	player->Coin = player->Coin - itemPrice;

	if(pUser[mobId].User.Storage.Coin < 2000000000)
		pUser[mobId].User.Storage.Coin += totalPrice;
	
	SendSignalParm(clientId, clientId, 0x3AF, player->Coin);
	SendCargoCoin(mobId);

	p39B sm;
	sm.Header.PacketId = 0x39B;
	sm.Header.ClientId = 0x7530;

	sm.MobID = mobId;
	sm.SlotID = slotId;

	sm.Header.Size = 20;

	GridMulticast_2(mobTargetX, mobTargetY, (BYTE*)&sm, 0);

	SendClientMessage(p->TargetID, "!O personagem [%s] comprou o item %s por %d", player->MobName, g_pItemList[p->item.sIndex].ItemName, p->Price);
	
	Log(clientId, LOG_INGAME, "Comprou o item %s [%d] [%d %d %d %d %d %d] de %s (%s) por %d. Gold sobrando: %d - Impostos pago: %d", g_pItemList[item->sIndex].ItemName, item->sIndex, item->stEffect[0].cEffect,
		item->stEffect[0].cValue, item->stEffect[1].cEffect, item->stEffect[1].cValue, item->stEffect[2].cEffect, item->stEffect[2].cValue, mob->MobName, pUser[p->TargetID].User.Username, p->Price,
		player->Coin, impost);

	Log(p->TargetID, LOG_INGAME, "Vendeu o item %s [%d] [%d %d %d %d %d %d] para %s (%s) por %d. Gold banco: %d - Impostos: %d", g_pItemList[item->sIndex].ItemName, item->sIndex, item->stEffect[0].cEffect,
		item->stEffect[0].cValue, item->stEffect[1].cEffect, item->stEffect[1].cValue, item->stEffect[2].cEffect, item->stEffect[2].cValue, player->MobName, pUser[clientId].User.Username,  p->Price, pUser[p->TargetID].User.Storage.Coin,
		impost);

	LogPlayer(clientId, "Comprou o item %s da loja de %s por %d", g_pItemList[item->sIndex].ItemName, pMob[p->TargetID].Mobs.Player.MobName, p->Price);
	LogPlayer(p->TargetID, "Vendeu o item %s de sua loja para %s por %d. %d foram pagos de impostos", g_pItemList[item->sIndex].ItemName, pMob[clientId].Mobs.Player.MobName, p->Price, impost);

	if(impost > 50000000)
	{
		Log(SERVER_SIDE, LOG_INGAME, "Pagos um total de %d de impostos... Valor maior que 50milhêes.", impost);
		Log(SERVER_SIDE, LOG_INGAME, "[%s] Vendeu o item %s [%d] [%d %d %d %d %d %d] para %s (%s) por %d. Gold banco: %d - Impostos: %d", User.Username, g_pItemList[item->sIndex].ItemName, item->sIndex, item->stEffect[0].cEffect,
			item->stEffect[0].cValue, item->stEffect[1].cEffect, item->stEffect[1].cValue, item->stEffect[2].cEffect, item->stEffect[2].cValue, player->MobName, pUser[clientId].User.Username,  p->Price, pUser[p->TargetID].User.Storage.Coin,
			impost);
	}

	if (pUser[p->TargetID].PremierStore.CurrentScore && itemPrice >= 10000000)
	{
		pUser[p->TargetID].PremierStore.Time += (5 * 60);

		Log(clientId, LOG_INGAME, "Recebeu 5 minutos extras por vender item com preêo acima de 10 milhêes na loja!");
	}

	SaveUser(clientId, 0);
	SaveUser(p->TargetID, 0);
	return true;
}