#include "cServer.h"
#include "Basedef.h"
#include "SendFunc.h"
#include "GetFunc.h"
#include <tuple>

bool CUser::RequestBuyShop(PacketHeader *Header)
{
	p379 *p = (p379*)(Header);

	if (p->mobID >= MAX_MOB)
	{
		Log(clientId, LOG_INGAME, "Enviado pacote de compra de item com npcId %d", p->mobID);
		return false;
	}
	int mobId = p->mobID;
	if(p->sellSlot < 0 || p->sellSlot > 63 || p->invSlot < 0 || p->invSlot >= 60)
	{
		Log(clientId, LOG_ERROR, "Enviando slots com indices irregular.");
		return false;
	}

	// recompra de items
	if (mobId < 1000)
	{
		if (p->sellSlot == 27)
			p->sellSlot = 9;

		if (p->sellSlot >= static_cast<int>(Repurchase.Items.size()))
		{
			Log(clientId, LOG_ERROR, "Enviado pacote de recompra de item com index de %d", p->sellSlot);

			return false;
		}

		STRUCT_ITEM* item = &Repurchase.Items[p->sellSlot];

		// TODO : log error
		if (item->sIndex <= 0 || item->sIndex >= MAX_ITEMLIST)
			return false;

		long long price = static_cast<unsigned long long>(g_pItemList[item->sIndex].Price);
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

		if (sellAmount)
			price *= GetItemAmount(item);

		if (price > 2000000000LL)
		{
			SendClientMessage(clientId, "Nêo ê possêvel recomprar este item.");

			Log(clientId, LOG_HACK, "Tentativa de recomprar item com valor maior que 2bi. Valor: %lld. %s %s", price, g_pItemList[item->sIndex].ItemName, item->toString().c_str());
			Log(clientId, LOG_HACK, "%s Tentativa de recomprar item com valor maior que 2bi. Valor: %lld. %s %s", User.Username, price, g_pItemList[item->sIndex].ItemName, item->toString().c_str());
			return true;
		}

		if (pMob[clientId].Mobs.Player.Coin < price)
		{
			SendClientMessage(clientId, g_pLanguageString[_NN_NotEnoughGold_BuyShop]);

			return true;
		}

		int slotId = GetFirstSlot(clientId, 0);
		if (slotId == -1)
		{
			SendClientMessage(clientId, "Nêo ha espaêo no inventario");

			return true;
		}

		pMob[clientId].Mobs.Player.Coin -= static_cast<int>(price);
		SendSignalParm(clientId, clientId, 0x3AF, pMob[clientId].Mobs.Player.Coin);

		pMob[clientId].Mobs.Player.Inventory[slotId] = *item;
		SendItem(clientId, SlotType::Inv, slotId, item);

		Log(clientId, LOG_INGAME, "Recomprado o item %s %s pelo preêo de %d", g_pItemList[item->sIndex].ItemName, item->toString().c_str(), price);
		*item = STRUCT_ITEM{};

		SendRepurchase(clientId);
		return true;
	}

	STRUCT_MOB *spw = &pMob[mobId].Mobs.Player;
	STRUCT_MOB *mySpawn = &pMob[clientId].Mobs.Player;

	auto distance = GetDistance(pMob[mobId].Target.X, pMob[mobId].Target.Y, pMob[clientId].Target.X, pMob[clientId].Target.Y);
	if (distance > VIEWGRIDX / 2)
	{
		AddCrackError(clientId, 2, CRACK_USER_PKTHACK);

		Log(clientId, LOG_INGAME, "Uso do NPC %s fora do range para tentar comprar %ux %uy %ux %uy. Distência: %d", spw->MobName, pMob[mobId].Target.X, pMob[mobId].Target.Y, pMob[clientId].Target.X, pMob[clientId].Target.Y, distance);
		return false;
	}

	if(spw->BaseScore.Merchant.Merchant != 1)
	{
		Log(SERVER_SIDE, LOG_INGAME, "%s - Tentativa de comprar item de mob! Mob: %s. Posição: %dx %dy.", User.Username, spw->MobName, pMob[mobId].Target.X, pMob[mobId].Target.Y);

		return true;
	}

	STRUCT_ITEM *item = &spw->Inventory[p->sellSlot];
	if(item->sIndex <= 0 || item->sIndex >= MAX_ITEMLIST)
		return false;

	if(mySpawn->Inventory[p->invSlot].sIndex != 0)
	{
		SendItem(clientId, SlotType::Inv, p->invSlot, &mySpawn->Inventory[p->invSlot]);

		return true;
	}

	if(p->invSlot >= 30 && p->invSlot < 45)
	{
		float time = TimeRemaining(pMob[clientId].Mobs.Player.Inventory[60].EFV1, pMob[clientId].Mobs.Player.Inventory[60].EFV2, pMob[clientId].Mobs.Player.Inventory[60].EFV3 + 1900);
		if(time <= 0)
			return true;
	}

	else if(p->invSlot >= 45 && p->invSlot < 60)
	{
		float time = TimeRemaining(pMob[clientId].Mobs.Player.Inventory[61].EFV1, pMob[clientId].Mobs.Player.Inventory[61].EFV2, pMob[clientId].Mobs.Player.Inventory[61].EFV3 + 1900);
		if(time <= 0.0f)
			return true;
	}

	if(User.Block.Blocked)
	{
		SendClientMessage(clientId, "Desbloqueie seus itens para poder movimenta-los");

		return true;
	}

	STRUCT_ITEMLIST *itemList = &g_pItemList[item->sIndex];

	INT32 cityZone = GetVillage(pMob[mobId].Target.X, pMob[mobId].Target.Y);
	if(cityZone == 5)
		cityZone = 4;
	
	INT64 itemPrice = itemList->Price;

	INT32 perc_impost = g_pCityZone[cityZone].perc_impost;
	INT64 impost = 0;
	if(perc_impost != 0)
	{
		impost = (itemPrice * static_cast<INT64>(perc_impost) / 100LL);
		itemPrice = (itemPrice + impost);
	}

	if (itemPrice > 2000000000LL)
	{
		SendClientMessage(clientId, g_pLanguageString[_NN_NotEnoughGold_BuyShop]);

		return true;
	}
	
	if (mySpawn->Coin < itemPrice)
	{
		SendClientMessage(clientId, g_pLanguageString[_NN_NotEnoughGold_BuyShop]);

		return true;
	}
	
	if(impost >= 2000000LL)
	{
		Log(SERVER_SIDE, LOG_INGAME, "Imposto maior que 2milhêes.");
		Log(SERVER_SIDE, LOG_INGAME, "%s - Comprou o item %s na loja de %s. Preêo: %lld. Imposto: %lld. Gold: %d.", User.Username, itemList->ItemName, spw->MobName, itemPrice, impost, pMob[clientId].Mobs.Player.Coin);
	}

	// Arrecada o imposto da cidade
	g_pCityZone[cityZone].impost += impost;

	Log(clientId, LOG_INGAME, "Comprou o item %s %s na loja de %s. Preêo total: %lld. Imposto: %lld. Gold: %d anterior. Gold apês a compra: %d", itemList->ItemName, item->toString().c_str(), spw->MobName, itemPrice, impost, pMob[clientId].Mobs.Player.Coin, mySpawn->Coin - static_cast<int>(itemPrice));
	LogPlayer(clientId, "Comprou o item %s na loja de %s por %lld. Impostos: %lld", itemList->ItemName, spw->MobName, itemPrice, impost);

	// Retira o gold COM o imposto
	mySpawn->Coin -= static_cast<int>(itemPrice);

	SendSignalParm(clientId, clientId, 0x3AF, mySpawn->Coin);
	memcpy(&mySpawn->Inventory[p->invSlot], item, sizeof STRUCT_ITEM);

	SendItem(clientId, SlotType::Inv, p->invSlot, item);
	return true;
}
