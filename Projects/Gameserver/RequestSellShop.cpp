#include "cServer.h"
#include "Basedef.h"
#include "SendFunc.h"
#include "GetFunc.h"
#include <algorithm>

bool CUser::RequestSellShop(PacketHeader *Header)
{
	p37A *p = (p37A*)Header;

	int fairyId = pMob[clientId].Mobs.Player.Equip[13].sIndex;

	int mobId = p->npcId;
	if (mobId >= MAX_MOB || mobId < 0)
	{
		Log(clientId, LOG_ERROR, "Pacote RequestSellShop enviado com ID de mob %d", mobId);

		return false;
	}

	if((mobId < 1000 || mobId >= MAX_MOB) && fairyId != 3914 && fairyId != 3915)
	{ 
		AddCrackError(clientId, 2, CRACK_USER_PKTHACK);

		return false;
	}

	if (p->sellSlot < 0 || p->sellSlot >= 60)
	{
		AddCrackError(clientId, 2, CRACK_USER_PKTHACK);

		return false;
	}

	CMob *mob = nullptr;
	if (mobId != 0)
	{
		mob = &pMob[mobId];
		auto distance = GetDistance(mob->Target.X, mob->Target.Y, pMob[clientId].Target.X, pMob[clientId].Target.Y);
		if (distance > VIEWGRIDX / 2 && fairyId != 3914 && fairyId != 3915)
		{
			AddCrackError(clientId, 2, CRACK_USER_PKTHACK);

			Log(clientId, LOG_INGAME, "Uso do NPC %s fora do range para tentar vender %dx %dy %dx %dy. Distência: %d", mob->Mobs.Player.MobName, mob->Target.X, mob->Target.Y, pMob[clientId].Target.X, pMob[clientId].Target.Y, distance);
			return false;
		}
	}

	STRUCT_ITEM *item = &pMob[clientId].Mobs.Player.Inventory[p->sellSlot];
	if(item->sIndex <= 0 || item->sIndex > MAX_ITEMLIST)
	{
		AddCrackError(clientId, 2, CRACK_USER_PKTHACK);

		return false;
	}

	if(User.Block.Blocked)
	{
		SendClientMessage(clientId, "Desbloqueie seus itens para poder movimenta-los");

		return true;
	}

	STRUCT_ITEMLIST *rItem = &g_pItemList[item->sIndex];
	INT32 cityZone = 5;
	if (mob != nullptr)
		cityZone = GetVillage(mob->Target.X, mob->Target.Y);

	STRUCT_ITEM temporaryItem{ *item };
	auto sellItemResult = SellItem(clientId, item);

	if(sellItemResult == eSellItemResult::GoldLimit)
	{
		SendClientMessage(clientId, g_pLanguageString[_NN_Cant_get_more_than_2G]);

		SendItem(clientId, (SlotType)p->type, p->sellSlot, item);
		return true;
	}

	if (sellItemResult == eSellItemResult::BlockedItem)
	{
		SendClientMessage(clientId, "Nêo ê possêvel vender este item");

		SendItem(clientId, (SlotType)p->type, p->sellSlot, item);
		return true;
	}

	auto found = std::find_if(Repurchase.Items.begin(), Repurchase.Items.end(), [](const STRUCT_ITEM& item) {
		return item.sIndex <= 0 || item.sIndex >= MAX_ITEMLIST;
	});

	if(found == Repurchase.Items.end())
	{
		if (++Repurchase.LastIndex >= Repurchase.Items.size())
			Repurchase.LastIndex = 0;

		found = Repurchase.Items.begin() + Repurchase.LastIndex;
	}

	(*found) = temporaryItem;

	if(mob != nullptr)
		Log(clientId, LOG_INGAME, "Vendido item no NPC %s", mob->Mobs.Player.MobName);

	SendItem(clientId, SlotType::Inv, p->sellSlot, item);
	return true;
}