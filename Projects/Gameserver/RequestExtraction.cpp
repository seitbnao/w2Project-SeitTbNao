#include "cServer.h"
#include "Basedef.h"
#include "SendFunc.h"
#include "GetFunc.h"

bool CUser::RequestExtraction(PacketHeader *Header)
{
	p2D4 *p = (p2D4*)(Header);

	INT32 slotId = p->slotId;
	if(slotId < 0 || slotId >= 64)
		return true;

	STRUCT_ITEM *item = &pMob[clientId].Mobs.Player.Inventory[slotId];
	if(item->sIndex <= 0 || item->sIndex >= MAX_ITEMLIST)
		return true;

	INT32 unique = g_pItemList[item->sIndex].Pos;
	if(unique == 2) 
		unique = 3021;
	else if(unique == 4) 
		unique = 3022;
	else if(unique == 8 ) 
		unique = 3023;
	else if(unique == 16) 
		unique = 3024;
	else if(unique == 32) 
		unique = 3025;

	if(unique < 3021 || unique >= 3026)
	{
		SendClientMessage(clientId, "Sê ê possêvel utilizar em equipamentos.");

		return true;
	}
	
	INT32 wiseStone = GetFirstSlot(clientId, 1774);
	if(wiseStone == -1)
	{
		SendClientMessage(clientId, "Para realizar a composição ê necessario [01] Pedra do Sabio");

		return true;
	}
	
	if(g_pItemList[item->sIndex].Unique >= 41 && g_pItemList[item->sIndex].Unique <= 49)
	{
		if(g_pItemList[item->sIndex].nGrade >= 5 && g_pItemList[item->sIndex].nGrade <= 8)
		{
			SendClientMessage(clientId, "Nêo ê possêvel usar em itens ancts");

			return true;
		}
	}
	
	if(GetItemSanc(item) > 9)
	{
		SendClientMessage(p->Header.ClientId, "Nêo ê possêvel tirar extração de itens acima de +9");

		return true;
	}

	if (IsImpossibleToRefine(item))
	{
		SendClientMessage(clientId, "Item impossêvel de refinar");

		return true;
	}

	INT32 _rand  = Rand() % 100;
	INT32 chance = 3 + (pMob[clientId].Mobs.Player.CurrentScore.Special[2] * 2 / 11);
	if ((pMob[clientId].Mobs.Player.LearnedSkill[0] & 0x8000))
		chance += 10;

	if(_rand > chance)
	{
		Log(clientId, LOG_INGAME, "EXTRAçãO - Destruiu o item %s [%d] [%d %d %d %d %d %d] - %d/%d", g_pItemList[item->sIndex].ItemName,
			item->sIndex, item->EF1, item->EFV1, item->EF2, item->EFV2, item->EF3, item->EFV3, _rand, chance);

		memset(item, 0, sizeof item);
		memset(&pMob[clientId].Mobs.Player.Inventory[wiseStone], 0, sizeof STRUCT_ITEM);

		SendItem(clientId, SlotType::Inv, slotId, item);
		SendItem(clientId, SlotType::Inv, wiseStone, &pMob[clientId].Mobs.Player.Inventory[wiseStone]);

		SendClientMessage(clientId, "Item destruêdo");
		return true;
	}
	
	Log(clientId, LOG_INGAME, "EXTRAçãO - Usado no item %s [%d] [%d %d %d %d %d %d] - %d/%d", g_pItemList[item->sIndex].ItemName,
		item->sIndex, item->EF1, item->EFV1, item->EF2, item->EFV2, item->EF3, item->EFV3, _rand, chance);

	BOOL success = false;
	for(int i = 0; i < 3;i++)
	{
		if(item->stEffect[i].cEffect == 43 || (item->stEffect[i].cEffect >= 116 && item->stEffect[i].cEffect <= 125))
		{
			int value   = GetEffectValueByIndex(item->sIndex, EF_UNKNOW1);
			int mobtype = GetEffectValueByIndex(item->sIndex, EF_MOBTYPE);

			int sanc = GetItemSanc(item);
			if(value <= 5 && mobtype == 0)
			{ // Itens <= [E] e ê item mortal
				value = value;
			}
			
			if(value == 0)
			{
				SendClientMessage(clientId, "Necessario que o item possua alguma classe ([A],[B],[C] ....)");
				
				return true;
			}

			// Calculo realizado - Item entregue
			item->stEffect[i].cEffect = 87;
			item->stEffect[i].cValue = value;
			success = true;
			break;
		}
	}

	if(!success)
	{
		for(int i = 0; i < 3; i++)
		{
			if(item->stEffect[i].cEffect == 0)
			{
				int value   = GetEffectValueByIndex(item->sIndex, EF_UNKNOW1);
				int mobtype = GetEffectValueByIndex(item->sIndex, EF_MOBTYPE);
				int sanc    = GetItemSanc(item);
				if(value <= 5 && mobtype == 0)
				{ // Itens <= [E] e ê item mortal
					value = value;
				}

				if(value == 0)
				{
					SendClientMessage(clientId, "Necessario que o item possua alguma classe ([A],[B],[C] ....)");

					return true;
				}

				// Calculo realizado - Item entregue
				item->stEffect[i].cEffect = 87;
				item->stEffect[i].cValue = value;
				success = true;
				break;
			}
		}
	}


	if(!success)
	{
		SendClientMessage(clientId, "Item impossêvel de refinar");
		return true;
	}
	
	if(item->stEffect[1].cEffect == EF_DAMAGE)
		item->stEffect[1].cValue += GetStaticItemAbility(item, EF_DAMAGE);
	
	if(item->stEffect[2].cEffect == EF_DAMAGE)
		item->stEffect[2].cValue += GetStaticItemAbility(item, EF_DAMAGE);
	
	item->sIndex = unique;

	Log(clientId, LOG_INGAME, "EXTRAçãO - Resultado %s [%d] [%d %d %d %d %d %d] - %d/%d", g_pItemList[item->sIndex].ItemName,
		item->sIndex, item->EF1, item->EFV1, item->EF2, item->EFV2, item->EF3, item->EFV3, _rand, chance);

	
	memset(&pMob[clientId].Mobs.Player.Inventory[wiseStone], 0, sizeof STRUCT_ITEM);

	SendItem(clientId, SlotType::Inv, slotId, item);
	SendItem(clientId, SlotType::Inv, wiseStone, &pMob[clientId].Mobs.Player.Inventory[wiseStone]);
	
	SendClientMessage(clientId, "Extração criada");

	Log(clientId, LOG_INGAME, "Extração criada : %s [%d] [%d %d %d %d %d %d]", g_pItemList[item->sIndex].ItemName,
		item->sIndex, item->EF1, item->EFV1, item->EF2, item->EFV2, item->EF3, item->EFV3);
	return true;
}