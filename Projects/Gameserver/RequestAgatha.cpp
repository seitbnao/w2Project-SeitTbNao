#include "cServer.h"
#include "Basedef.h"
#include "SendFunc.h"
#include "GetFunc.h"

bool CUser::RequestAgatha(PacketHeader *Header)
{
	pCompor *p = (pCompor*)Header;

	STRUCT_MOB *player = &pMob[clientId].Mobs.Player;

	for(int i = 0; i < 6;i++)
	{
		if(p->slot[i] == -1)
			continue;

		if(p->slot[i] < 0 || p->slot[i] > 60)
		{
			SendClientMessage(clientId, "Invalido");

			return true;
		}

		for(int y = 0; y < 6; y++)
		{
			if(y == i)
				continue;

			if(p->slot[i] == p->slot[y])
			{
				Log(clientId, LOG_HACK, " Banido por enviar item com mesmo slotId - NPC Agatha - %d", p->items[i].sIndex);
				Log(SERVER_SIDE, LOG_HACK, "%s - Banido por enviar item com mesmo slotId  - NPC Agatha - %d", player->MobName, p->items[i].sIndex);
				
				SendCarry(clientId);
				return true;
			}
		}

		if(!memcmp(&p->items[i], &player->Inventory[p->slot[i]], sizeof STRUCT_ITEM))
			continue;

		SendClientMessage(clientId, "Invalido");
		return true;
	}
	
	if(User.Block.Blocked)
	{
		SendClientMessage(clientId, "Desbloqueie seus itens para poder movimenta-los");

		return true;
	}

	for(int i = 0 ; i < 6; i ++)
	{
		Log(clientId, LOG_COMP, "Agatha - %d - %s [%d] [%d %d %d %d %d %d] - %d", i, g_pItemList[p->items[i].sIndex].ItemName, p->items[i].sIndex,
			p->items[i].stEffect[0].cEffect, p->items[i].stEffect[0].cValue, p->items[i].stEffect[1].cEffect, p->items[i].stEffect[1].cValue, p->items[i].stEffect[2].cEffect,
			p->items[i].stEffect[2].cValue, p->slot[i]);
	}

	if (Trade.ClientId != 0)
	{
		RemoveTrade(clientId);

		AddCrackError(clientId, 1, CRACK_TRADE_NOTEMPTY);
		return true;
	}

	STRUCT_ITEM *item = p->items;

	if(GetEffectValueByIndex(item[0].sIndex, EF_MOBTYPE) != 1)
	{
		SendClientMessage(clientId, "Utilize apenas itens archs na composição");
		return true;
	}

	int itemClass = GetEffectValueByIndex(item[1].sIndex, EF_UNKNOW1);
	if(itemClass < 4)
	{
		SendClientMessage(clientId, "Utilize apenas itens [D] ou superior");
		return true;
	}

	const STRUCT_ITEMLIST& itemArch = g_pItemList[item[0].sIndex];
	const STRUCT_ITEMLIST& itemMortal = g_pItemList[item[1].sIndex];

	if(GetItemSanc(&item[0]) >= 10)
	{
		SendClientMessage(clientId, "Nêo ê possêvel transferir para itens acima de +9");

		return true;
	}

	if(itemArch.Pos != itemMortal.Pos)
	{
		SendClientMessage(clientId, "Os itens devem ter o mesmo tipo");
		return true;
	}

	if(itemArch.Pos > 32)
	{
		SendClientMessage(clientId, "Posição do item invalido");

		return true;
	}
	
	int reqClassArch = GetEffectValueByIndex(item[0].sIndex, EF_CLASS);
	int reqClassMortal = GetEffectValueByIndex(item[1].sIndex, EF_CLASS);

	if(reqClassArch != reqClassMortal)
	{
		SendClientMessage(clientId, "Necessario ser da mesma classe");

		return true;
	}
	for(int i = 2;i < 6;i++)
	{
		if(item[i].sIndex != 3140)
		{
			SendClientMessage(clientId, "Utilize 4 Pedras da Luz");

			return true;
		}
	}
	
	int _rand = Rand() % 100;
	int totalChance = 12;
	int grade = g_pItemList[item[1].sIndex].nGrade;

	constexpr std::array rateByGrade = { 20, 25, 35, 45 };

	if (grade >= 1 && grade <= 4 && itemClass == 4)
		totalChance = rateByGrade[grade - 1];
	else if (itemClass == 5)
		totalChance = 26;

	SendSignalParm(clientId, SERVER_SIDE, 0x3A7, 2);

	if(_rand <= totalChance)
	{
		Log(clientId, LOG_COMP, "Agatha - Sucesso na transferência de add: %s [%d] [%d %d %d %d %d %d] para %s [%d] [%d %d %d %d %d %d] - %d/%d",
			g_pItemList[item[1].sIndex].ItemName, item[1].sIndex, item[1].EF1, item[1].EFV1, item[1].EF2, item[1].EFV2, item[1].EF3, item[1].EFV3, g_pItemList[item[0].sIndex].ItemName,
			item[0].sIndex, item[0].EF1, item[0].EFV1, item[0].EF2, item[0].EFV2, item[0].EF3, item[0].EFV3, _rand, totalChance);

		Log(SERVER_SIDE, LOG_COMP, "[%s] - Agatha - Sucesso na transferência de add: %s [%d] [%d %d %d %d %d %d] para %s [%d] [%d %d %d %d %d %d] - %d/%d",
			User.Username, g_pItemList[item[1].sIndex].ItemName, item[1].sIndex, item[1].EF1, item[1].EFV1, item[1].EF2, item[1].EFV2, item[1].EF3, item[1].EFV3, g_pItemList[item[0].sIndex].ItemName,
			item[0].sIndex, item[0].EF1, item[0].EFV1, item[0].EF2, item[0].EFV2, item[0].EF3, item[0].EFV3, _rand, totalChance);

		LogPlayer(clientId, "Sucesso na transferência de adicional de %s para %s", g_pItemList[item->sIndex].ItemName, g_pItemList[item->sIndex].ItemName);

		int sanc = GetItemSanc(&item[1]);
		for(int i = 0 ;i < 3;i++)
		{
			if(item[1].stEffect[i].cEffect == EF_SANC || (item[1].stEffect[i].cEffect >= 116 && item[1].stEffect[i].cEffect <= 125))
			{
				item[1].stEffect[i].cValue = 7;

				break;
			}
		}

		int index = item[1].sIndex;
		item[1].sIndex = item[0].sIndex;

		SendNotice(".%s transferiu com sucesso adicional para %s", player->MobName, g_pItemList[item[0].sIndex].ItemName);
		SendClientMessage(clientId, "Jogador %s obteve sucesso na composição de %s (%d/%d)", player->MobName, g_pItemList[item->sIndex].ItemName, _rand, totalChance);

		for(int i = 2;i < 6;i++)
		{
			memset(&player->Inventory[p->slot[i]], 0, sizeof STRUCT_ITEM);
			
			SendItem(clientId, SlotType::Inv, p->slot[i], &player->Inventory[p->slot[i]]);
		}

		memset(&player->Inventory[p->slot[1]], 0, sizeof STRUCT_ITEM);
		memcpy(&player->Inventory[p->slot[0]], &item[1], sizeof STRUCT_ITEM);
		
		SendItem(clientId, SlotType::Inv, p->slot[0], &player->Inventory[p->slot[0]]);
		SendItem(clientId, SlotType::Inv, p->slot[1], &player->Inventory[p->slot[1]]);
	}
	else
	{
		memset(&player->Inventory[p->slot[0]], 0, sizeof STRUCT_ITEM);

		SendNotice(".%s falhou na transferência de adicional para %s", player->MobName, g_pItemList[item->sIndex].ItemName);
		SendClientMessage(clientId, "Falha na composição do item. (%d/%d)", _rand, totalChance);

		for(int i = 2;i<6;i++)
		{
			memset(&player->Inventory[p->slot[i]], 0, sizeof STRUCT_ITEM);
			
			SendItem(clientId, SlotType::Inv, p->slot[i], &player->Inventory[p->slot[i]]);
		}
		
		SendItem(clientId, SlotType::Inv, p->slot[0], &player->Inventory[p->slot[0]]);

		LogPlayer(clientId, "Falha na transferência de adicional de %s para %s", g_pItemList[item->sIndex].ItemName, g_pItemList[item->sIndex].ItemName);
		Log(clientId, LOG_COMP, "Agatha - Falha na transferência de add: %s [%d] [%d %d %d %d %d %d] para %s [%d] [%d %d %d %d %d %d] - %d/%d",
			g_pItemList[item[1].sIndex].ItemName, item[1].sIndex, item[1].EF1, item[1].EFV1, item[1].EF2, item[1].EFV2, item[1].EF3, item[1].EFV3, g_pItemList[item[0].sIndex].ItemName,
			item[0].sIndex, item[0].EF1, item[0].EFV1, item[0].EF2, item[0].EFV2, item[0].EF3, item[0].EFV3, _rand, totalChance);

		Log(SERVER_SIDE, LOG_COMP, "[%s] - Agatha - Falha na transferência de add: %s [%d] [%d %d %d %d %d %d] para %s [%d] [%d %d %d %d %d %d] - %d/%d",
			User.Username, g_pItemList[item[1].sIndex].ItemName, item[1].sIndex, item[1].EF1, item[1].EFV1, item[1].EF2, item[1].EFV2, item[1].EF3, item[1].EFV3, g_pItemList[item[0].sIndex].ItemName,
			item[0].sIndex, item[0].EF1, item[0].EFV1, item[0].EF2, item[0].EFV2, item[0].EF3, item[0].EFV3, _rand, totalChance);
	}

	SaveUser(clientId, 0);
	return true;
}