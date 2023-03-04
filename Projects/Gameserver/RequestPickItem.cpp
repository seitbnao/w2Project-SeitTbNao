#include "cServer.h"
#include "Basedef.h"
#include "SendFunc.h"
#include "GetFunc.h"
#include "CNPCGener.h"
#include "CEventManager.h"

bool CUser::RequestPickItem(PacketHeader *Header)
{
	p270 *p = (p270*)Header;

	constexpr std::array blockedToPick = { 1727, 4700, 4701, 794, 795, 796, 797, 798,  4703, 4705, 4704};
	if(pMob[clientId].Mobs.Player.CurrentScore.Hp <= 0 || pUser[clientId].CurrentScore != USER_PLAY)
	{
		AddCrackError(clientId, 1, CRACK_USER_STATUS);

		return true;
	}

	if(p->invType != (int)SlotType::Inv)
	{
		AddCrackError(clientId, 1, CRACK_USER_STATUS);

		return true;
	}
	
	INT32 initItemId = p->initID - 10000; // LOCAL843

	if (initItemId < 0 || initItemId > 4096)
		return true;

	if (initItemId < 0 || initItemId >= 4096 || g_pInitItem[initItemId].Open == 0)
	{
		if (g_pInitItem[initItemId].Open != 0)
		{
			Log(SERVER_SIDE, LOG_ERROR, "PickItem - Mudanaa de status inesperado - OPEN.");
			// 0042CA41
		}

		p16F packet;
		memset(&packet, 0, sizeof p16F);

		packet.Header.PacketId = 0x16F;
		packet.Header.Size = sizeof p16F;

		packet.initID = p->initID;

		AddMessage((BYTE*)&packet, sizeof p16F);
		return true;
	}

	// 0042CAF0
	if (pMob[clientId].Target.X < g_pInitItem[initItemId].PosX - 3 || pMob[clientId].Target.Y < g_pInitItem[initItemId].PosY - 3 || pMob[clientId].Target.X > g_pInitItem[initItemId].PosX + 3 || pMob[clientId].Target.Y > g_pInitItem[initItemId].PosY + 3)
	{
		Log(clientId, LOG_ERROR, "Tentando pegar o item do chao muito longe.");
		return true;
	}

	// 0042CC2C
	STRUCT_ITEM *item = &g_pInitItem[initItemId].Item; // LOCAL848

	// 0042CBFF
	if (std::find(blockedToPick.begin(), blockedToPick.end(), item->sIndex) != std::end(blockedToPick) && !IsAdmin)
		return true;

	INT32 itemIndex = item->sIndex;
	if(itemIndex <= 0 || itemIndex >= MAX_ITEMLIST)
		return true;

	if(item->sIndex == 470)
	{
		if(pMob[clientId].Mobs.Info.Pilula)
		{
			SendClientMessage(clientId, g_pLanguageString[_NN_Youve_Done_It_Already]);

			return true;
		}

		//0042CCEA
		SendClientMessage(clientId, g_pLanguageString[_NN_Get_Skill_Point]);

		pMob[clientId].Mobs.Info.Pilula = 1;

		INT32 initItemPosX = g_pInitItem[initItemId].PosX; // LOCAL852
		INT32 initItemPosY = g_pInitItem[initItemId].PosY; // LOCAL853

		memset(item, 0, sizeof STRUCT_ITEM);

		g_pItemGrid[initItemPosY][initItemPosX] = 0;

		g_pInitItem[initItemId].Open = 0;

		p16F packet;
		memset(&packet, 0, sizeof p16F);

		packet.Header.PacketId = 0x16F;
		packet.Header.Size = sizeof p16F;

		packet.initID = initItemId + 10000;

		AddMessage((BYTE*)&packet, sizeof p16F);

		GridMulticast_2(initItemPosX, initItemPosY, (BYTE*)&packet, 0);
		
		SendEmotion(clientId, 14, 3);
		return true;
	}

	if(itemIndex >= 490 && itemIndex <= 500)
	{
		INT32 initItemPosX = g_pInitItem[initItemId].PosX; // LOCAL858
		INT32 initItemPosY = g_pInitItem[initItemId].PosY; // LOCAL859

		g_pItemGrid[initItemPosY][initItemPosX] = 0;
		g_pInitItem[initItemId].Open = 0;

		p16F packet;
		memset(&packet, 0, sizeof p16F);

		packet.Header.PacketId = 0x16F;
		packet.Header.Size = sizeof p16F;

		packet.initID = initItemId + 10000;

		AddMessage((BYTE*)&packet, sizeof p16F);

		GridMulticast_2(initItemPosX, initItemPosY, (BYTE*)&packet, 0);
	}

	
	INT32 initItemPosX = g_pInitItem[initItemId].PosX;
	INT32 initItemPosY = g_pInitItem[initItemId].PosY;
	
	p16F sm; // LOCAL872
	sm.Header.PacketId = 0x16F;
	sm.Header.Size = sizeof p16F;
	sm.Header.ClientId = 0x7530;
	sm.initID = p->initID;

	if(initItemPosX < 0 || initItemPosX >= 4096 || initItemPosY <= 0 || initItemPosY >= 4096)
	{
		AddMessage((BYTE*)&sm, sizeof p16F);
		
		g_pInitItem[initItemId].Open = 0;
		return true;
	}

	if (g_pItemGrid[initItemPosY][initItemPosX] != initItemId)
	{
		AddMessage((BYTE*)&sm, sizeof p16F);

		if(g_pItemGrid[initItemPosY][initItemPosX] == 0)
			g_pItemGrid[initItemPosY][initItemPosX] = initItemId;

		return true;
	}

	// 0042D06F
	if(p->posX != initItemPosX || p->posY != initItemPosY)
	{
		AddMessage((BYTE*)&sm, sizeof p16F);

		return true;
	}

	INT32 itemAbility = GetItemAbility(item, EF_VOLATILE); // local880

	int slotId = GetFirstSlot(clientId, 0);
	if (slotId == -1)
	{
		SendClientMessage(clientId, "Sem espaao no inventario");

		return true;
	}

	if (itemAbility == 2)
	{
		// 0042D0E8
		// Nao feito, nao cai mais gold no chao
	}
	else
	{
		STRUCT_ITEM *invSlotItem = &pMob[clientId].Mobs.Player.Inventory[slotId]; // LOCAL864

		memcpy(invSlotItem, item, sizeof STRUCT_ITEM);
		memset(item, 0, sizeof STRUCT_ITEM);

		SendItem(clientId, SlotType::Inv, slotId, invSlotItem);
		Log(clientId, LOG_INGAME, "Pegou o item %s %s do chao", g_pItemList[invSlotItem->sIndex].ItemName, invSlotItem->toString().c_str());
	}

	GridMulticast_2(initItemPosX, initItemPosY, (BYTE*)&sm, 0);

	g_pItemGrid[initItemPosY][initItemPosX] = 0;
	g_pInitItem[initItemId].Open = 0;

	return true;
}