#include "cServer.h"
#include "Basedef.h"
#include "SendFunc.h"
#include "GetFunc.h"

bool CUser::RequestShany(PacketHeader *Header)
{
	pCompor *p = (pCompor*)Header;
	
	bool sucess = false;

	STRUCT_MOB *player = &pMob[clientId].Mobs.Player;

	for(int i = 0;i < 3;i++)
	{
		if(p->slot[i] < 0 || p->slot[i] >= 60)
		{
			Log(clientId, LOG_HACK, "Banido por enviar êndice invalido - NPC Shany - %d", p->slot[i]);
			Log(SERVER_SIDE, LOG_HACK, "%s - Banido por enviar êndice invalido - NPC Shany - %d", player->MobName, p->slot[i]);
			
			SendCarry(clientId);
			return true;
		}

		if(memcmp(&player->Inventory[p->slot[i]], &p->items[i], 8) != 0)
		{
			Log(clientId, LOG_HACK, "Banido por enviar item inexistente - NPC Ehre - %d", p->items[i].sIndex);
			Log(SERVER_SIDE, LOG_HACK, "%s - Banido por enviar item inexistente - NPC Shany - %d", player->MobName, p->items[i].sIndex);
			
			SendCarry(clientId);
			return true;
		}

		for(int y = 0; y < 3; y++)
		{
			if(y == i)
				continue;

			if(p->slot[i] == p->slot[y])
			{
				Log(clientId, LOG_HACK, "Banido por enviar item com mesmo slotId - NPC Ehre - %d", p->items[i].sIndex);
				Log(SERVER_SIDE, LOG_HACK, "%s - Banido por enviar item com mesmo slotId  - NPC Shany - %d", player->MobName, p->items[i].sIndex);
				
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
			Log(clientId, LOG_COMP, "Shany - %d - Sem item\n", i);

			continue;
		}

		Log(clientId, LOG_COMP, "Shany - %d - %s %s - %hhd", i, g_pItemList[p->items[i].sIndex].ItemName, p->items[i].toString().c_str(), p->slot[i]);
	}

	if (Trade.ClientId != 0)
	{
		RemoveTrade(clientId);
		AddCrackError(clientId, 1, CRACK_TRADE_NOTEMPTY);
		return true;
	}

	STRUCT_ITEM *items = (STRUCT_ITEM*)p->items;
	
	SendSignalParm(clientId, SERVER_SIDE, 0x3A7, 2);
	for(int i = 0; i < 3; i++)
	{
		int itemId = items[i].sIndex;
		if(itemId != 540 && itemId != 541)
		{
			SendClientMessage(clientId, "Insira a Pedra Espiritual [E] nos dois primeiros slots");

			return true;
		}

		if(i != 2)
		{
			int sanc = GetItemSanc(&items[i]);
			if(sanc != 9)
			{
				SendClientMessage(clientId, "Pedra Espiritual [E] deve estar +9");

				return true;
			}
		}
	}

	for(int i = 3; i < 7;i++)
	{
		if(items[i].sIndex != 413)
		{
			SendClientMessage(clientId, "Insira [04] Poeiras de Lactolerium");

			return true;
		}
	}

	if(player->Equip[0].EFV2 == 1 || (player->Equip[0].EFV2 == 2 && player->BaseScore.Level < 255))
	{
		SendClientMessage(clientId, "Necessario ser no mênimo arch nivel 256");

		return true;
	}

	int _rand = Rand() % 100;

	for(int i = 0; i < 7; i ++)
	{
		Log(clientId, LOG_COMP, "NPC Shany - Espiritual: %s [%d] [%d %d %d %d %d %d] - Slot %d", g_pItemList[items[i].sIndex].ItemName, items[i].sIndex, items[i].EF1, items[i].EFV1, items[i].EF2,
			items[i].EFV2, items[i].EF3, items[i].EFV3, i);
	}

	if(_rand <= 60)
	{
		for(int i = 2; i < 7; i++)
		{
			memset(&player->Inventory[p->slot[i]], 0, sizeof STRUCT_ITEM);

			SendItem(clientId, SlotType::Inv, p->slot[i], &player->Inventory[p->slot[i]]);
		}
		
		SendClientMessage(clientId, "Houve uma falha na composição");

		Log(clientId, LOG_COMP, "Falha na composiêaê Shany - %d", _rand);
		LogPlayer(clientId, "Falha na composição de Pedra Espiritual [F]");

		SaveUser(clientId, 0);
		return true;
	}

	SendClientMessage(clientId, "Sucesso na composição");

	if(player->Exp <= 2000000)
		player->Exp = 0;
	else
		player->Exp -= 2000000;

	if(player->Equip[0].EFV2 >= 3)
	{
		while(player->Exp < g_pNextLevel[2][player->BaseScore.Level - 1])
			player->BaseScore.Level --;
	}
	else
	{
		while(player->BaseScore.Level > 0 && player->Exp < g_pNextLevel[0][player->BaseScore.Level - 1])
			player->BaseScore.Level --;
	}

	for(int i = 3; i < 7; i++)
	{
		memset(&player->Inventory[p->slot[i]], 0, sizeof STRUCT_ITEM);
	
		SendItem(clientId, SlotType::Inv, p->slot[i], &player->Inventory[p->slot[i]]);
	}
	
	memset(&player->Inventory[p->slot[0]], 0, sizeof STRUCT_ITEM);
	SendItem(clientId, SlotType::Inv, p->slot[0], &player->Inventory[p->slot[0]]);
	
	memset(&player->Inventory[p->slot[1]], 0, sizeof STRUCT_ITEM);
	SendItem(clientId, SlotType::Inv, p->slot[1], &player->Inventory[p->slot[1]]);

	memset(&items[2], 0, sizeof STRUCT_ITEM);
	_rand = Rand() % 100;

	if(_rand <= 33)
		items[2].sIndex = 631;
	else if(_rand <= 66)
		items[2].sIndex = 632;
	else 
		items[2].sIndex = 633;

	memcpy(&player->Inventory[p->slot[2]], &items[2], sizeof STRUCT_ITEM);

 
	LogPlayer(clientId, "Sucesso na composição de Pedra Espiritual [F]");

	SendItem(clientId, SlotType::Inv, p->slot[2], &player->Inventory[p->slot[2]]);

	pMob[clientId].GetCurrentScore(clientId);
	SendEtc(clientId);
	SendScore(clientId);

	SaveUser(clientId, 0);
	return true;
}