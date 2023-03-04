#include "cServer.h"
#include "Basedef.h"
#include "SendFunc.h"
#include "GetFunc.h"

bool CUser::RequestTiny(PacketHeader *Header)
{
	pCompor *p = (pCompor*)Header;
	
	STRUCT_MOB *player = &pMob[clientId].Mobs.Player;
	for(int i = 0;i<3;i++)
	{
		if(p->slot[i] == -1)
		{
			SendClientMessage(clientId, "Coloque todos os itens");
			return true;
		}

		if((p->slot[i] < 0 || p->slot[i] >= 60))
		{
			Log(clientId, LOG_HACK, "Banido por enviar êndice invalido - NPC Tiny - %d", p->slot[i]);
			Log(SERVER_SIDE, LOG_HACK, "%s - Banido por enviar êndice invalido - NPC Tiny - %d", player->MobName, p->slot[i]);
			
			SendCarry(clientId);
			return true;
		}
		if(memcmp(&player->Inventory[p->slot[i]], &p->items[i], 8) != 0)
		{
			Log(clientId, LOG_HACK, "Banido por enviar item inexistente - NPC Tiny - %d", p->items[i].sIndex);
			Log(SERVER_SIDE, LOG_HACK, "%s - Banido por enviar item inexistente - NPC Tiny - %d", player->MobName, p->items[i].sIndex);
			
			SendCarry(clientId);
			return true;
		}
		for(int y = 0; y < 3; y++)
		{
			if(y == i)
				continue;

			if(p->slot[i] == p->slot[y])
			{
				Log(clientId, LOG_HACK, "Banido por enviar item com mesmo slotId - NPC Tiny - %d", p->items[i].sIndex);
				Log(SERVER_SIDE, LOG_HACK, "%s - Banido por enviar item com mesmo slotId  - NPC Tiny - %d", player->MobName, p->items[i].sIndex);
				
				SendCarry(clientId);
				return true;
			}
		}
	}
	
	if(User.Block.Blocked)
	{
		SendClientMessage(clientId, "Desbloqueie seus itens para poder movimenta-los");

		return true;
	}

	for(int i = 0 ; i < 3; i ++)
	{
		if (p->slot[i] == -1)
		{
			Log(clientId, LOG_COMP, "Tiny - %d - Sem item\n", i);

			continue;
		}

		Log(clientId, LOG_COMP, "Tiny - %d - %s %s - %hhd", i, g_pItemList[p->items[i].sIndex].ItemName, p->items[i].toString().c_str(), p->slot[i]);
	}

	if (Trade.ClientId)
	{
		RemoveTrade(clientId);
		AddCrackError(clientId, 1, CRACK_TRADE_NOTEMPTY);

		return true;
	}

	STRUCT_ITEM *items = p->items;
	
	SendSignalParm(clientId, SERVER_SIDE, 0x3A7, 2);

	int chanceTotal = 25;
	if (p->slot[2] != -1)
	{
		STRUCT_ITEMLIST *itemD = (STRUCT_ITEMLIST*)&g_pItemList[items[2].sIndex];

		int itemType = GetEffectValueByIndex(items[2].sIndex, EF_UNKNOW1);
		if (itemType < 4)
		{
			SendClientMessage(clientId, "Utilize um item [D] ou superior na composição!");
			return true;
		}

		chanceTotal += ((itemType - 3) * 4);
	}

	for(int i = 0; i < 2; i++)
	{		
		STRUCT_ITEMLIST *item = (STRUCT_ITEMLIST*)&g_pItemList[items[i].sIndex];
		if(item->nGrade < 5 || item->nGrade > 8)
		{
			SendClientMessage(clientId, "Utilize itens ancts na composição");

			return true;
		}

		int ref = GetItemSanc(&player->Inventory[p->slot[i]]);
		if(ref < 9)
		{
			SendClientMessage(clientId, "Utilize itens +9 na composição");

			return true;
		}
	}

	if(player->Coin < 100000000)
	{
		SendClientMessage(clientId, "Sêo necessarios 100 milhêes de gold para a composição");

		return true;
	}

	int _rand = Rand() % 100;	
	player->Coin -= 100000000;

	SendSignalParm(clientId, clientId, 0x3AF, player->Coin);

	if (p->slot[2] != -1)
	{
		memset(&player->Inventory[p->slot[2]], 0, sizeof STRUCT_ITEM);
		SendItem(clientId, SlotType::Inv, p->slot[2], &player->Inventory[p->slot[2]]);
	}

	if(_rand <= chanceTotal)
	{
		//Sucesso na composição
		STRUCT_ITEM *itemArch = (STRUCT_ITEM*)&player->Inventory[p->slot[0]];
		STRUCT_ITEM *itemMortal = (STRUCT_ITEM*)&player->Inventory[p->slot[1]];

		Log(clientId, LOG_COMP, "Tiny - Sucesso na transferência de add para %s [%d] [%d %d %d %d %d %d]", g_pItemList[itemArch->sIndex].ItemName, itemMortal->sIndex, itemMortal->EF1, itemMortal->EFV1,
			itemMortal->EF2, itemMortal->EFV2, itemMortal->EF3, itemMortal->EFV3);
		Log(SERVER_SIDE, LOG_COMP, "[%s] - Tiny - Sucesso na transferência de add para %s [%d] [%d %d %d %d %d %d]", 
			User.Username, g_pItemList[itemArch->sIndex].ItemName, itemMortal->sIndex, itemMortal->EF1, itemMortal->EFV1, itemMortal->EF2, itemMortal->EFV2, itemMortal->EF3, itemMortal->EFV3);
		LogPlayer(clientId, "Sucesso na transferência de adicional de %s para %s", g_pItemList[itemMortal->sIndex].ItemName, g_pItemList[itemArch->sIndex].ItemName);

		SendNotice(".%s transferiu com sucesso o adicional para %s", player->MobName, g_pItemList[itemArch->sIndex].ItemName);
		SendClientMessage(clientId, "Sucesso na transferência de adicional de %s para %s (%d/%d)", g_pItemList[itemMortal->sIndex].ItemName, g_pItemList[itemArch->sIndex].ItemName, _rand, chanceTotal);
		itemMortal->sIndex = itemArch->sIndex;

		SetItemSanc(itemMortal, 7, 0);

		memset(itemArch, 0, sizeof STRUCT_ITEM);

		SendItem(clientId, SlotType::Inv, p->slot[0], &player->Inventory[p->slot[0]]);
		SendItem(clientId, SlotType::Inv, p->slot[1], &player->Inventory[p->slot[1]]);
	}
	else
	{
		 
		SendClientMessage(clientId, "Falha na transferência de adicional (%d/%d)", _rand, chanceTotal);
 
	}

	SaveUser(clientId, 0);
	return true;
}