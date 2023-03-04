#include "cServer.h"
#include "Basedef.h"
#include "GetFunc.h"
#include "SendFunc.h"

bool GoldBar(int clientId, SlotType slotType, int srcSlot, STRUCT_ITEM* srcItem)
{
	int sumGold = 0;
	switch (srcItem->sIndex)
	{
	case 4010:
		sumGold = 100000000;
		break;

	case 4011:
		sumGold = 1000000000;
		break;

	case 4026:
		sumGold = 1000000;
		break;

	case 4027:
		sumGold = 5000000;
		break;

	case 4028:
		sumGold = 10000000;
		break;

	case 4029:
		sumGold = 50000000;
		break;

	default:
		return false;
	}

	UINT64 goldTotal = (UINT64)pMob[clientId].Mobs.Player.Coin + (UINT64)sumGold;
	if (goldTotal > 2000000000)
	{
		SendClientMessage(clientId, g_pLanguageString[_NN_Cant_get_more_than_2G]);

		SendItem(clientId, slotType, srcSlot, srcItem);
		return false;
	}

	Log(clientId, LOG_INGAME, "Gold atual: %d.", pMob[clientId].Mobs.Player.Coin);
	LogPlayer(clientId, "%s utilizado. Gold atual no inventario: %d", g_pItemList[srcItem->sIndex].ItemName, pMob[clientId].Mobs.Player.Coin);

	pMob[clientId].Mobs.Player.Coin += sumGold;

	SendSignalParm(clientId, clientId, 0x3AF, pMob[clientId].Mobs.Player.Coin);

	AmountMinus(srcItem);
	return true;
}