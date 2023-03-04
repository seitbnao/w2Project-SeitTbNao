#include "cServer.h"
#include "Basedef.h"
#include "SendFunc.h"
#include "GetFunc.h"

constexpr std::array blockedItems = { 1104,1116,1128,1140,1152,1416,1419,1422,1425,1428,1254,1266,1278,1290,1302,1553,1569,1572,1575,1578,938,939,940,941,942,943 }; // Armas Sephira

int GetMatchCombine(STRUCT_ITEM *item)
{
	short itemid = item[0].sIndex;
	short stone = item[1].sIndex;

	if (itemid <= 0 || itemid >= MAX_ITEMLIST)
		return 0;

	if (stone <= 0 || stone >= MAX_ITEMLIST)
		return 0;

	short unique = g_pItemList[itemid].Unique;
	if (unique < 41 || unique > 49)
		return 0; // Apenas armas podem ser compostas 

	short itemanctid = g_pItemList[itemid].Extreme;
	if (itemanctid == 0)
		return 0; // Armas de eventos nêo podem ser Anct

	int type = GetEffectValueByIndex(item[0].sIndex, EF_UNKNOW1);
	int refine = 1;
	for (int i = 2; i < 8; i++)
	{
		short index = item[i].sIndex;
		if (index <= 0 || index >= MAX_ITEMLIST)
			continue;

		short pos = GetItemAbility(&item[i], EF_POS);
		if (pos == 0)
			return 0;

		if (std::find(std::begin(blockedItems), std::end(blockedItems), index) != std::end(blockedItems))
			return 0;

		short sanc = GetItemSanc(&item[i]);
		if (sanc == 7)
			refine += 2;
		else if (sanc == 8)
			refine += 4;
		else if (sanc == 9)
			refine += 10;
		else
			refine += sanc + 2;

		int ef_type = GetEffectValueByIndex(index, EF_UNKNOW1);
		if (ef_type == type)
			continue;

		if (ef_type <= (type - 2))
			return 0;

		if (ef_type < type)
			refine -= 2;
		else if (ef_type > type)
			refine += 2;
	}

	return refine > 100 ? 100 : refine;
}

bool CUser::RequestCompounder(PacketHeader *Header)
{
	pCompor *p = (pCompor*)Header;

	STRUCT_MOB *player = (STRUCT_MOB*)(&pMob[clientId].Mobs.Player);

	for (int i = 0; i < 8; i++)
	{
		if (p->items[i].sIndex <= 0 || p->items[i].sIndex >= MAX_ITEMLIST)
			continue;

		short slot = p->slot[i];
		if (slot < 0 || slot >= 60)
		{
			SendSignalParm(clientId, SERVER_SIDE, 0x3A7, 2);

			return false;
		}

		if (memcmp(&p->items[i], &player->Inventory[p->slot[i]], sizeof STRUCT_ITEM))
		{
			SendSignalParm(clientId, SERVER_SIDE, 0x3A7, 2);

			return false;
		}

		for (int y = 0; y < 8; y++)
		{
			if (y == i)
				continue;

			if (p->slot[i] == p->slot[y])
			{
				Log(clientId, LOG_HACK, "Banido por enviar item com mesmo slotId - NPC Compounder - %d", p->items[i].sIndex);
				Log(SERVER_SIDE, LOG_HACK, "%s - Banido por enviar item com mesmo slotId  - NPC Compounder - %d", player->MobName, p->items[i].sIndex);

				SendCarry(clientId);
				return false;
			}
		}
	}

	for (int i = 0; i < 8; i++)
	{
		if (p->slot[i] == -1)
		{
			Log(clientId, LOG_COMP, "Compounder - %d - Sem item\n", i);

			continue;
		}

		Log(clientId, LOG_COMP, "Compounder - %d - %s %s - %hhd", i, g_pItemList[p->items[i].sIndex].ItemName, p->items[i].toString().c_str(), p->slot[i]);
	}

	if (User.Block.Blocked)
	{
		SendClientMessage(clientId, "Desbloqueie seus itens para poder movimenta-los");

		return true;
	}

	if (Trade.ClientId != 0)
	{
		RemoveTrade(clientId);
		AddCrackError(clientId, 1, CRACK_TRADE_NOTEMPTY);

		return true;
	}

	int chance = GetMatchCombine(p->items);
	if (chance <= 0)
	{
		SendClientMessage(p->Header.ClientId, "Composição incorreta");

		return false;
	}

	short slot = p->slot[0];
	if (slot < 0 || slot >= 64)
	{
		Log(clientId, LOG_INGAME, "Envio de slot %d em Anct", slot);
		return false;
	}

	int stone = (p->items[1].sIndex - 2441);
	if (stone < 0 || stone > 3)
	{
		SendClientMessage(clientId, "Jêia incorreta na composição do item.");

		SendSignalParm(clientId, SERVER_SIDE, 0x3A7, 2);
		return true;
	}

	// Checagem de classe do item
	int type = GetEffectValueByIndex(p->items[0].sIndex, EF_UNKNOW1);
	for (int i = 2; i < 6; i++)
	{
		int _tmp_type = GetEffectValueByIndex(p->items[0].sIndex, EF_UNKNOW1);
		if (type >= _tmp_type)
			continue;

		if (type - 1 < _tmp_type)
		{
			SendClientMessage(clientId, "Itens tem de ser de atê uma classe a menos");

			SendSignalParm(p->Header.ClientId, SERVER_SIDE, 0x3A7, 2);
			return true;
		}
	}

	for (int i = 0; i < 8; i++)
	{
		if (p->items[i].sIndex <= 0 || p->items[i].sIndex >= MAX_ITEMLIST)
			continue;

		short slot = p->slot[i];
		if (slot < 0 || slot >= 60)
			continue;

		memset(&player->Inventory[slot], 0, sizeof(STRUCT_ITEM));
		SendItem(clientId, SlotType::Inv, slot, &player->Inventory[slot]);
	}

	chance += GetCompounderDeal(clientId);

	int _rand = (Rand() % 115);
	if (_rand > 100)
		_rand -= 15;

	for (int i = 0; i < 8; i++)
	{
		if (p->slot[i] == -1)
		{
			Log(clientId, LOG_COMP, "Compor Anct - %d - Sem item", i);
			continue;
		}

		Log(clientId, LOG_COMP, "Compor Anct - %d - %s [%d] [%d %d %d %d %d %d] %d - Slot: %d", i, g_pItemList[p->items[i].sIndex].ItemName, p->items[i].sIndex, p->items[i].EF1, p->items[i].EFV1,
			p->items[i].EF2, p->items[i].EFV2, p->items[i].EF3, p->items[i].EFV3, _rand, p->slot[i]);
	}

	if (_rand > chance)
	{
		SendClientMessage(clientId, "Falha na composição do item. (%d/%d)", _rand, chance);

		SendSignalParm(clientId, SERVER_SIDE, 0x3A7, 2);
 
		return true;
	}

	SendEmotion(clientId, 100, Rand() % 8);
	SendEmotion(clientId, 100, Rand() % 8);
	SendEmotion(clientId, 100, Rand() % 8);

	int itemid = p->items[0].sIndex;
	memcpy(&player->Inventory[slot], &p->items[0], sizeof STRUCT_ITEM);

	player->Inventory[slot].sIndex = (g_pItemList[itemid].Extreme + stone);

	SetItemSanc(&player->Inventory[slot], 7, 0);

	SendItem(clientId, SlotType::Inv, slot, &player->Inventory[slot]);

	 
	SendSignalParm(clientId, SERVER_SIDE, 0x3A7, 2);
	return true;
}