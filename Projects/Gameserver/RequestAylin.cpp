#include "cServer.h"
#include "Basedef.h"
#include "SendFunc.h"
#include "GetFunc.h"

bool CUser::RequestAylin(PacketHeader *Header)
{
	pCompor *p = (pCompor*)Header;
	
	STRUCT_MOB *player = &pMob[clientId].Mobs.Player;

	for(int i = 0;i<7;i++)
	{
		if(i >= 3 && i <= 6)
			if(p->slot[i] == -1)
				continue;

		if(p->slot[i] < 0 || p->slot[i] > 60)
		{			
			Log(clientId, LOG_HACK, "[HACK] Banido por enviar êndice invalido - NPC Aylin - %d", p->slot[i]);
			Log(SERVER_SIDE, LOG_HACK, "[HACK] %s - Banido por enviar êndice invalido - NPC Aylin - %d", player->MobName, p->slot[i]);
			
			SendCarry(clientId);

			return true;
		}

		if(memcmp(&player->Inventory[p->slot[i]], &p->items[i], 8) != 0)
		{
			Log(clientId, LOG_HACK, "Banido por enviar item inexistente - NPC Aylin - %d", p->items[i].sIndex);
			Log(SERVER_SIDE, LOG_HACK, "%s - Banido por enviar item inexistente - NPC Aylin - %d", player->MobName, p->items[i].sIndex);
			
			SendCarry(clientId);

			return true;
		}
		
		for(int y = 0; y < 7; y++)
		{
			if(y == i)
				continue;

			if(p->slot[i] == p->slot[y])
			{
				Log(clientId, LOG_HACK, "Banido por enviar item com mesmo slotId - NPC Aylin - %d", p->items[i].sIndex);
				Log(SERVER_SIDE, LOG_HACK, "%s - Banido por enviar item com mesmo slotId  - NPC Aylin - %d", player->MobName, p->items[i].sIndex);

				CloseUser(clientId);
				return true;
			}
		}
	}
	
	if(User.Block.Blocked)
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

	STRUCT_ITEM *items = p->items;

	if(g_pItemList[items[0].sIndex].Pos >= 256 || g_pItemList[items[1].sIndex].Pos == 0)
	{
		SendClientMessage(clientId, "Sê ê possêvel com armas ou sets");

		return true;
	}
	
	int sanc_1 = GetItemSanc(&items[0]);
	int sanc_2 = GetItemSanc(&items[1]);

	if(sanc_1 < 9 || sanc_2 < 9)
	{
		SendClientMessage(clientId, "Sê ê possêvel combinar com itens +9");

		return true;
	}

	for(int i = 0;i<2;i++)
	{
		if(g_pItemList[items[i].sIndex].Unique >= 41 && g_pItemList[items[i].sIndex].Unique <= 49)
		{
			if(g_pItemList[items[i].sIndex].nGrade < 5 || g_pItemList[items[i].sIndex].nGrade > 8)
			{
				SendClientMessage(clientId, "Utilize itens ancts na composição");
				return true;
			}
		}
	}

	if(items[0].sIndex != items[1].sIndex)
	{
		SendClientMessage(clientId, "Combinação incorreta.");

		return true;
	}

	if(player->Coin < 50000000)
	{
		SendClientMessage(clientId, "Sêo necessarios 50.000.000 de gold para continuar");

		return true;
	}

	int chance = 0;
	for(int i=3;i<7;i++)
	{
		if(items[i].sIndex >= 2441 && items[i].sIndex <= 2444)
			chance++;
		else 
		{
			SendClientMessage(clientId, "Incorreto");
			return true;
		}
	}

	if(chance == 4) 
	{
		if(items[3].sIndex != items[4].sIndex || items[3].sIndex != items[5].sIndex || items[3].sIndex != items[6].sIndex)
		{
			SendClientMessage(clientId, "As jêias devem ser iguais!");
			return true;
		}
	}
	else if(chance == 3) 
	{
		if(items[3].sIndex != items[4].sIndex || items[3].sIndex != items[5].sIndex)
		{
			SendClientMessage(clientId, "As jêias devem ser iguais!");
			return true;
		}
	} 
	else if(chance == 2)
	{
		if(items[3].sIndex != items[4].sIndex)
		{
			SendClientMessage(clientId, "As jêias devem ser iguais!");
			return true;
		}
	} 

	if(chance == 4)
		chance = 50;
	else if(chance == 3)
		chance = 32;
	else if(chance == 2)
		chance = 24;
	else if(chance == 1)
		chance = 12;
	else
		return true;

	int sanc = items[3].sIndex - 2211;
	if(sanc < 230 || sanc > 233)
	{
		SendClientMessage(clientId, "Invalidade . Contate a administração");

		return true;
	}
	
	chance += GetCompounderDeal(clientId);

	int _rand = (Rand() % 101);

	SendSignalParm(clientId, SERVER_SIDE, 0x3A7, 2);

	player->Coin -= 50000000;

	SendSignalParm(clientId, clientId, 0x3AF, player->Coin);
	
	for(int i = 0; i < 7;i++)
	{
		if(p->slot[i] == -1)
			continue;

		Log(clientId, LOG_COMP, "Aylin - Item %d %s %s", i, g_pItemList[items[i].sIndex].ItemName,  items[i].toString().c_str());
	}

	if(_rand > chance)
	{
		for(int i = 2;i < 7;i++)
		{
			memset(&player->Inventory[p->slot[i]], 0, sizeof STRUCT_ITEM);
			
			SendItem(clientId, SlotType::Inv, p->slot[i], &player->Inventory[p->slot[i]]);
		}

		SendClientMessage(clientId, "Houve uma falha na composição");
 
 		return true;
	}

	STRUCT_ITEM *Item1 = (STRUCT_ITEM*)&player->Inventory[p->slot[0]];
	STRUCT_ITEM *Item2 = (STRUCT_ITEM*)&player->Inventory[p->slot[1]];

	STRUCT_ITEMLIST itemData = g_pItemList[Item1->sIndex];
	int rand2 = Rand() % 50;

	STRUCT_ITEM *newItem;
	if(rand2 <= 25)
	{
		memcpy(Item1, Item2, sizeof STRUCT_ITEM);
		memset(Item2, 0, sizeof STRUCT_ITEM);
	
		newItem = Item1;
	}
	else
	{
		memcpy(Item2, Item1, sizeof STRUCT_ITEM);
		memset(Item1, 0, sizeof STRUCT_ITEM);

		newItem = Item2;
	}

	if(g_pItemList[newItem->sIndex].Pos >= 64 && g_pItemList[newItem->sIndex].Pos != 128)
	{
		int newGrade = sanc - 230;
		int baseId = newItem->sIndex - (g_pItemList[newItem->sIndex].nGrade - 5);

		newItem->sIndex = baseId + newGrade;
	}

	for(int i = 0;i<3;i++)
	{
		if(newItem->stEffect[i].cEffect == 43 || (newItem->stEffect[i].cEffect >= 116 && newItem->stEffect[i].cEffect <= 125))
		{
			newItem->stEffect[i].cValue = sanc;

			break;
		}
	}

	for(int i = 2;i < 7;i++)
	{
		memset(&player->Inventory[p->slot[i]], 0, sizeof STRUCT_ITEM);
		
		SendItem(clientId, SlotType::Inv, p->slot[i], &player->Inventory[p->slot[i]]);
	}
	
	SendItem(clientId, SlotType::Inv, p->slot[0], &player->Inventory[p->slot[0]]);
	SendItem(clientId, SlotType::Inv, p->slot[1], &player->Inventory[p->slot[1]]);

	char szTMP[1024];

	sprintf_s(szTMP, ".Jogador %s concluiu com sucesso a composição do item %s +10", player->MobName, g_pItemList[newItem->sIndex].ItemName);
	SendNotice(szTMP);

 
	SaveUser(clientId, 0);
	return true;
}