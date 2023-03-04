#include "cServer.h"
#include "Basedef.h"
#include "SendFunc.h"
#include "GetFunc.h"

bool CUser::RequestLindy(PacketHeader *Header)
{
	pCompor *p = (pCompor*)Header;

	STRUCT_MOB *player = &pMob[clientId].Mobs.Player;

	for (int i = 0; i < 7; i++)
	{
		if (p->slot[i] < 0 || p->slot[i] > 60)
		{
			Log(clientId, LOG_HACK, "[HACK] Banido por enviar êndice invalido - NPC Lindy - %d", p->slot[i]);
			Log(SERVER_SIDE, LOG_HACK, "[HACK] %s - Banido por enviar êndice invalido - NPC Lindy - %d", player->MobName, p->slot[i]);

			SendCarry(clientId);

			return true;
		}

		if (memcmp(&player->Inventory[p->slot[i]], &p->items[i], 8) != 0)
		{
			Log(clientId, LOG_HACK, "Banido por enviar item inexistente - NPC Lindy - %d", p->items[i].sIndex);
			Log(SERVER_SIDE, LOG_HACK, "%s - Banido por enviar item inexistente - NPC Lindy - %d", player->MobName, p->items[i].sIndex);

			SendCarry(clientId);

			return true;
		}

		for (int y = 0; y < 7; y++)
		{
			if (y == i)
				continue;

			if (p->slot[i] == p->slot[y])
			{
				Log(clientId, LOG_HACK, "Banido por enviar item com mesmo slotId - NPC Lindy - %d", p->items[i].sIndex);
				Log(SERVER_SIDE, LOG_HACK, "%s - Banido por enviar item com mesmo slotId  - NPC Lindy- %d", player->MobName, p->items[i].sIndex);

				CloseUser(clientId);
				return true;
			}
		}
	}

	if (User.Block.Blocked)
	{
		SendClientMessage(clientId, "Desbloqueie seus itens para poder movimenta-los");

		return true;
	}


	for (int i = 0; i < 7; i++)
	{
		Log(clientId, LOG_COMP, "Lindy - %d - %s [%d] [%d %d %d %d %d %d] - %d", i, g_pItemList[p->items[i].sIndex].ItemName, p->items[i].sIndex,
			p->items[i].stEffect[0].cEffect, p->items[i].stEffect[0].cValue, p->items[i].stEffect[1].cEffect, p->items[i].stEffect[1].cValue, p->items[i].stEffect[2].cEffect,
			p->items[i].stEffect[2].cValue, p->slot[i]);
	}

	if (Trade.ClientId != 0)
	{
		RemoveTrade(Trade.ClientId);

		AddCrackError(clientId, 1, CRACK_TRADE_NOTEMPTY);
		return true;
	}

	if (((player->BaseScore.Level == 354 && !pMob[clientId].Mobs.Info.Unlock354) || (player->BaseScore.Level == 369 && !pMob[clientId].Mobs.Info.Unlock369)) && pMob[clientId].Mobs.Info.LvBlocked)
	{
		if (p->items[0].sIndex == 413 && p->items[1].sIndex == 413 && p->items[2].sIndex == 4127 && p->items[3].sIndex == 413 && p->items[4].sIndex == 413 &&
			p->items[5].sIndex == 413 && p->items[6].sIndex == 413)
		{
			// Fecha o inventario
			SendSignalParm(clientId, SERVER_SIDE, 0x3A7, 2);

			// Checa se o item possui 10 unidades
			INT32 amount_1 = GetItemAmount(&p->items[0]);
			INT32 amount_2 = GetItemAmount(&p->items[1]);

			if (amount_1 != 10 || amount_2 != 10 || !pMob[clientId].Mobs.Info.LvBlocked || (player->BaseScore.Level == 369 && pMob[clientId].Mobs.Fame < 1))
			{
				SendClientMessage(clientId, g_pLanguageString[_NN_IncorrectComp]);

				return true;
			}

			// Remove os itens, independente se deu certo ou nêo
			for (int i = 0; i < 7; i++)
			{
				memset(&player->Inventory[p->slot[i]], 0, sizeof STRUCT_ITEM);

				SendItem(clientId, SlotType::Inv, p->slot[i], &player->Inventory[p->slot[i]]);
			}

			if (player->BaseScore.Level == 369)
				pMob[clientId].Mobs.Fame -= 1;

			INT32 _rand = Rand() % 100;

			// Desbloqueou com sucesso, amigo.
			if (_rand <= 90)
			{
				pMob[clientId].Mobs.Info.LvBlocked = false;

				if (pMob[clientId].Mobs.Player.BaseScore.Level == 354)
					pMob[clientId].Mobs.Info.Unlock354 = TRUE;
				else
					pMob[clientId].Mobs.Info.Unlock369 = TRUE;

				INT32 capeId = pMob[clientId].Mobs.Player.Equip[15].sIndex;
				if ((capeId != 3191 && capeId != 3192 && capeId != 3193) || capeId == 0)
				{
					INT32 newCape = 0;
					if (pMob[clientId].Mobs.Player.CapeInfo == 7)
						newCape = 3191;
					else if (pMob[clientId].Mobs.Player.CapeInfo == 8)
						newCape = 3192;
					else if (pMob[clientId].Mobs.Player.CapeInfo == 9 || capeId == 0)
						newCape = 3193;

					memset(&pMob[clientId].Mobs.Player.Equip[15], 0, sizeof STRUCT_ITEM);

					pMob[clientId].Mobs.Player.Equip[15].sIndex = newCape;
					pMob[clientId].Mobs.Player.Equip[15].stEffect[0].cEffect = EF_SANC;
					pMob[clientId].Mobs.Player.Equip[15].stEffect[0].cValue = 0;

					Log(clientId, LOG_INGAME, "Alterado a capa para %d", newCape);
				}

				if (player->BaseScore.Level == 369)
				{
					pMob[clientId].Mobs.Player.Equip[15].stEffect[1].cEffect = EF_RESISTALL;
					pMob[clientId].Mobs.Player.Equip[15].stEffect[1].cValue = 10;

					Log(clientId, LOG_INGAME, "Adicionado adicional de resistência na capa");
				}

				SendClientMessage(clientId, g_pLanguageString[_NN_Success_Comp]);
				Log(clientId, LOG_COMP, "Composição de desbloqueio 355 efetuada com SUCESSO 0- %d/90", _rand);

				SendItem(clientId, SlotType::Equip, 15, &pMob[clientId].Mobs.Player.Equip[15]);
			}
			else
			{
				SendClientMessage(clientId, g_pLanguageString[_NN_CombineFailed]);

				Log(clientId, LOG_COMP, "Composição de desbloqueio 355 efetuada com FALHA - %d/90", _rand);
			}
		}
	}
	else
		SendClientMessage(clientId, g_pLanguageString[_NN_IncorrectComp]);

	SaveUser(clientId, 0);
	return true;
}