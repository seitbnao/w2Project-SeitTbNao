#include "cServer.h"
#include "Basedef.h"
#include "SendFunc.h"
#include "GetFunc.h"

bool CUser::RequestOpenGate(PacketHeader *Header)
{ // 00430C81
	p374 *p = (p374*)Header; // LOCAL1154

	if(!pMob[clientId].Mobs.Player.CurrentScore.Hp || pUser[clientId].Status != USER_PLAY)
	{
		SendHpMode(clientId);

		return true;
	}

	INT32 gateId = p->gateId - 10000; // LOCAL1155
	if(gateId < 0 || gateId >= 4096)
		return false;

	if (gateId == 24)
	{
		SendClientMessage(clientId, "Monte o seu grupo e utilize o Zakum para ter acesso a quest.");

		return true;
	}

	INT32 packetStatus = p->status ; // LOCAL1156
	INT32 initItemStatus = g_pInitItem[gateId].CurrentScore; // LOCAL1157

	if (initItemStatus == 3 || packetStatus == 3)
	{
		INT32 itemAbility = GetItemAbility(&g_pInitItem[gateId].Item, EF_KEYID); // LOCAL1159
		INT32 keySlot = 0; // LOCAL1160

		if (itemAbility != 0)
		{
			for(INT32 i = 0; i < 64; i++) // LOCAL1161
			{
				keySlot = GetItemAbility(&pMob[clientId].Mobs.Player.Inventory[i], EF_KEYID);
				if (keySlot == itemAbility)
				{
					memset(&pMob[clientId].Mobs.Player.Inventory[i], 0, sizeof STRUCT_ITEM);

					SendItem(clientId, SlotType::Inv, i, &pMob[clientId].Mobs.Player.Inventory[i]);
					break;
				}
			}

			// 00430E1A
			if (keySlot != itemAbility)
			{
				if(g_pInitItem[gateId].Item.sIndex != 773)
					SendClientMessage(clientId, g_pLanguageString[_NN_No_Key]);

				return true;
			}
		}
	}

	INT32 height = 0; // LOCAL1162

	INT32 updateItem = UpdateItem(gateId, packetStatus, &height); // LOCAL1163
	if (updateItem == 0)
		return true;

	p->unknow = height;

	GridMulticast_2(g_pInitItem[gateId].PosX, g_pInitItem[gateId].PosY, (BYTE*)p, 0);

	return true;
}