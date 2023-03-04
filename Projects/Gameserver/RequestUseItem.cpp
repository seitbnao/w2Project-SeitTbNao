#include "cServer.h"
#include "Basedef.h"
#include "SendFunc.h"
#include "GetFunc.h"
#include "CEventManager.h"
#include "CDuelLetter.h"
#include "CNPCGener.h"
#include "ItemsAction.h"
#include <algorithm>
#include <random>

bool CUser::RequestUseItem(PacketHeader* Header)
{
	static const INT32 g_pItemRest[7][6] =
	{
		{2363, 2364, 2365, 0, 0, 0},
		{2366, 2367, 2368, 2372, 2373, 2371},
		{2369, 2370, 2374, 2375, 2387, 0},
		{2377, 2376, 2378, 0, 0, 0},
		{2381, 2382, 2383, 2388, 0, 0},
		{2384, 2385, 2386, 0, 0, 0},
		{2379, 2380, 0, 0, 0, 0}
	};

	p373* p = (p373*)(Header);
	if (p->SrcSlot < 0 || p->SrcSlot > 60 || p->DstSlot < 0 || p->DstSlot > 60 || p->DstType < 0 || p->DstType > 2 || p->SrcType < 0 || p->SrcType > 2)
		return false;

	STRUCT_ITEM* srcItem = GetItemPointer(clientId, p->SrcType, p->SrcSlot);
	if (srcItem == NULL)
		return false;

	if (CurrentScore != USER_PLAY)
	{
		Log(clientId, LOG_INGAME, "Enviado pacote 0x373 (RequestUseItem) nao estando dentro do jogo. Status: %d", CurrentScore);
		return false;
	}
	if (pMob[clientId].Mobs.Player.CurrentScore.Hp <= 0)
	{
		SendItem(clientId, (SlotType)p->SrcType, p->SrcSlot, srcItem);
		return false;
	}

	if (p->PosX < 0 || p->PosX >= 4096 || p->PosY < 0 || p->PosY >= 4096)
	{
		Log(clientId, LOG_HACK, "Enviado pacote com posiaao %hux %huy", p->PosX, p->PosY);
		Log(SERVER_SIDE, LOG_HACK, "[%s] -  Enviado pacote com posiaao %hux %huy", User.Username, p->PosX, p->PosY);

		return false;
	}

	INT32 _volatile = GetItemAbility(srcItem, EF_VOLATILE); // LOCAL_929
	INT32 itemAmount = GetItemAmount(srcItem); // local930

	// Evita usar um item que esta no trade
	if (Trade.ClientId != 0)
		RemoveTrade(clientId);

	if (_volatile != 2 && _volatile != 15)
	{
		if (User.Block.Blocked)
		{
			SendClientMessage(clientId, "Desbloqueie seus itens para poder movimenta-los");

			SendItem(clientId, (SlotType)p->SrcType, p->SrcSlot, srcItem);
			return true;
		}
	}

	if (srcItem->sIndex <= 0 || srcItem->sIndex >= MAX_ITEMLIST)
	{
		SendItem(clientId, (SlotType)p->SrcType, p->SrcSlot, srcItem);

		return true;
	}

	if (_volatile != 1 && _volatile != 15 && _volatile != 191 && srcItem->sIndex != 0)
	{
		Log(clientId, LOG_INGAME, "Item %s [%d] [%d %d %d %d %d %d] utilizado em %hux %huy", g_pItemList[srcItem->sIndex].ItemName, srcItem->sIndex, srcItem->EF1, srcItem->EFV1, srcItem->EF2, srcItem->EFV2, srcItem->EF3, srcItem->EFV3,
			p->PosX, p->PosY);
		LogPlayer(clientId, "Item %s utilizado", g_pItemList[srcItem->sIndex].ItemName);

		auto now = std::chrono::steady_clock::now();
		auto timeInMs = std::chrono::duration_cast<std::chrono::milliseconds>(now - Times.LastUsedItem);
		if (timeInMs < 169ms)
		{
			Log(clientId, LOG_HACK, "O usuario usou itens muito rapidamente. Tempo para usar o item: %lld. Item: %s", timeInMs.count(), g_pItemList[srcItem->sIndex].ItemName);
			Log(SERVER_SIDE, LOG_HACK, "[%s] - O usuario usou itens muito rapidamente. Tempo para usar o item: %lld. Item: %s", User.Username, timeInMs.count(), g_pItemList[srcItem->sIndex].ItemName);
		}

		Times.LastUsedItem = now;

		auto[isFast, time] = CheckIfIsTooFast(srcItem, p->SrcSlot);
		if (isFast)
		{
			Log(clientId, LOG_HACK, "O usuario usou o item %s em %dms.", srcItem->toString().c_str(), time);
			Log(SERVER_SIDE, LOG_HACK, "O usuario %s usou o item %s em %dms.", User.Username, srcItem->toString().c_str(), time);
		}
	}

	INT32 itemId = srcItem->sIndex;
#pragma region PAC ITENS
	if (_volatile == 210)
	{
		INT32 index = -1;
		for (INT32 i = 0; i < MAX_PACITEM; i++)
		{
			if (g_pPacItem[i].ItemId == srcItem->sIndex)
			{
				index = i;
				break;
			}
		}

		if (index < 0 || index >= MAX_PACITEM)
		{
			SendItem(clientId, SlotType::Inv, p->SrcSlot, srcItem);

			return true;
		}

		STRUCT_ITEMPAC* pac = &g_pPacItem[index];
		INT32 total = 0;

		for (INT32 i = 0; i < 8; i++)
			total += pac->Amount[i];

		if (GetInventoryAmount(clientId, 0) < total)
		{
			SendClientMessage(clientId, g_pLanguageString[_NN_You_Have_No_Space_To_Trade]);

			SendItem(clientId, SlotType::Inv, p->SrcSlot, srcItem);
			return true;
		}

		// apaga
		AmountMinus(srcItem);

		for (INT32 i = 0; i < 8; i++)
		{
			if (pac->Item[i].sIndex <= 0 || pac->Item[i].sIndex >= MAX_ITEMLIST || pac->Amount[i] <= 0)
				continue;

			for (INT32 t = 0; t < pac->Amount[i]; t++)
			{
				INT32 slotId = GetFirstSlot(clientId, 0);
				if (slotId == -1)
				{
					Log(clientId, LOG_INGAME, "Nao foi possavel encontrar espaao para o pacIndex %d. I: %d. Amount atual: %d", index, i, t);

					break;
				}

				memcpy(&pMob[clientId].Mobs.Player.Inventory[slotId], &pac->Item[i], sizeof STRUCT_ITEM);
			}
		}

		SendCarry(clientId);
	}
#pragma endregion
#pragma region POTS
	else if (_volatile == 1) //0042DE14
	{
		for (int i = 0; i < 32; i++)
		{
			if (pMob[clientId].Mobs.Affects[i].Index == 32)
			{
				SendItem(clientId, (SlotType)p->SrcType, p->SrcSlot, srcItem);

				return true;
			}
		}

		INT32 countHeal; // LOCAL931
		INT32 hpHeal = GetItemAbility(srcItem, EF_HP); // LOCAL932

		int potionBonus = pMob[clientId].PotionBonus;
		if (potionBonus != 0)
			hpHeal += (hpHeal * potionBonus / 100);

		if (hpHeal != 0)
		{
			countHeal = pUser[clientId].Potion.CountHp;
			countHeal += hpHeal;

			if (countHeal > pMob[clientId].Mobs.Player.CurrentScore.MaxHp)
				countHeal = pMob[clientId].Mobs.Player.CurrentScore.MaxHp;

			pUser[clientId].Potion.CountHp = countHeal;
		}

		INT32 mpHeal = GetItemAbility(srcItem, EF_MP);
		if (potionBonus != 0)
			mpHeal += (mpHeal * potionBonus / 100);

		if (mpHeal != 0)
		{
			countHeal = pUser[clientId].Potion.CountMp;
			countHeal += mpHeal;

			if (countHeal > pMob[clientId].Mobs.Player.CurrentScore.MaxMp)
				countHeal = pMob[clientId].Mobs.Player.CurrentScore.MaxMp;

			pUser[clientId].Potion.CountMp = countHeal;
		}

		pUser[clientId].Potion.bQuaff = 1;

		SendSetHpMp(clientId);
		AmountMinus(srcItem);

		return true;
	}
#pragma endregion
#pragma region POEIRAS
	else if ((_volatile == 4 || _volatile == 5) && srcItem->sIndex != 4141)
	{ // poeira - 0042DFB2
		static const int Rate[2][7][9] =
		{
			{
				{100, 100, 100, 100, 90, 60, 0, 0, 0 }, // A
				{100, 100, 100, 100, 90, 50, 0, 0, 0 }, // B
				{100, 100, 100, 80, 75, 50, 0, 0, 0 }, // C
				{100, 100, 95, 85, 80, 80, 0, 0, 0 }, // D
				{100, 95, 95, 85, 80, 80, 0, 0, 0 }, // E
				{100, 90, 80, 80, 80, 80, 0, 0, 0 }, // F
				{100, 90, 80, 80, 80, 80, 0, 0, 0 }, // Itens archs
			},
			{
				{100, 100, 100, 100, 100, 100, 75, 55, 40}, // A
				{100, 100, 100, 100, 100, 100, 75, 45, 35}, // B
				{100, 100, 100, 100, 100, 100, 75, 40, 30}, // C
				{100, 100, 100, 100, 100, 100, 75, 50, 45}, // D
				{100, 100, 100, 100, 100, 100, 65, 50, 45}, // E
				{100, 100, 100, 100, 100, 100, 55, 45, 45}, // F
				{100, 100, 100, 100, 100, 100, 55, 45, 45}, // Itens archs
			}
		};

		static const int Fail[9] = {
			0, 5, 5, 4, 4, 3, 3, 2, 1
		};

		static const int RateCele[15] = { 90, 85, 85, 80, 70, 60, 50, 45, 40, 35, 25, 15, 10, 7, 5 };
		static const short RateClosed[9] = { 90, 90, 80, 70, 65, 60, 55, 50, 30 };

		STRUCT_MOB* player = &pMob[clientId].Mobs.Player;

		STRUCT_ITEM* dstItem = GetItemPointer(clientId, p->DstType, p->DstSlot);
		if (p->DstSlot == 15 && p->DstType == 0)
		{
			if (pMob[clientId].Mobs.Player.Equip[0].EFV2 >= CELESTIAL)
			{
				SendItem(clientId, (SlotType)p->SrcType, p->SrcSlot, srcItem);

				SendClientMessage(clientId, g_pLanguageString[_NN_CantRefine]);
				return true;
			}
		}

		// Tentando refinar a face.
		if (p->DstSlot == 0 && p->DstType == 0)
			return true;

		if (dstItem->sIndex == 769)
		{
			for (int i = 0; i < 5; i++)
			{
				if (pMob[clientId].Target.X >= g_pCityZone[i].war_min_x && pMob[clientId].Target.X <= g_pCityZone[i].war_max_x && pMob[clientId].Target.Y >= g_pCityZone[i].war_min_y && pMob[clientId].Target.Y <= g_pCityZone[i].war_max_y)
				{
					SendItem(clientId, (SlotType)p->SrcType, p->SrcSlot, srcItem);

					SendClientMessage(clientId, g_pLanguageString[_NN_CantRefine_War]);
					return true;
				}
			}
		}

		if (p->DstType == (unsigned int)SlotType::Equip && p->DstSlot == 12)
		{
			SendItem(clientId, (SlotType)p->SrcType, p->SrcSlot, srcItem);

			return true;
		}

		switch (dstItem->sIndex)
		{
		case 3994:
		case 3993:
		{
			SendClientMessage(clientId, g_pLanguageString[_NN_CantRefine]);

			SendItem(clientId, (SlotType)p->SrcType, p->SrcSlot, srcItem);
		}
		break;
		}

		if (dstItem->sIndex == NUCLEO_LAC_ITEMID && _volatile == 4)
		{
			SendClientMessage(clientId, "Somente com poeiras de lac.");

			SendItem(clientId, (SlotType)p->SrcType, p->SrcSlot, srcItem);
			SendItem(clientId, (SlotType)p->DstType, p->DstSlot, dstItem);
			return true;
		}

		// Nacleo de lac
		if (dstItem->sIndex == NUCLEO_LAC_ITEMID && _volatile == 5)
		{
			// Inicializa o nacleo
			if (dstItem->stEffect[0].cEffect != EF_AMOUNT)
			{
				dstItem->stEffect[0].cEffect = EF_AMOUNT;
				dstItem->stEffect[0].cValue = 0;
			}

			// Nacleo ja incializado.
			if (dstItem->stEffect[0].cEffect == EF_AMOUNT)
			{
				if (dstItem->stEffect[0].cValue >= 100)
				{
					SendClientMessage(clientId, "O nacleo ja esta totalmente carregado!");

					SendItem(clientId, (SlotType)p->DstType, p->DstSlot, dstItem);
					SendItem(clientId, (SlotType)p->SrcType, p->SrcSlot, srcItem);
					return true;
				}

				dstItem->stEffect[0].cValue += 2;
				SendClientMessage(clientId, "Nacleo refinado com sucesso!");

				SendItem(clientId, (SlotType)p->DstType, p->DstSlot, dstItem);

				SendEmotion(clientId, 14, 3);
				AmountMinus(srcItem);
				return true;
			}
		}

		if (p->DstSlot == 13)
		{
			if (dstItem->sIndex >= 3397 && dstItem->sIndex <= 3406)
			{
				dstItem->sIndex += 10;

				SendItem(clientId, (SlotType)p->DstType, p->DstSlot, dstItem);

				AmountMinus(srcItem);
				return true;
			}
		}

		if (p->DstSlot == 14 && p->DstType == 0)
		{ // Ovo 
			if (player->Equip[p->DstSlot].sIndex >= 2300 && player->Equip[p->DstSlot].sIndex < 2330)
			{
				int add = player->Equip[14].EFV3;

				if (add != 0)
				{
					SendItem(clientId, (SlotType)p->SrcType, p->SrcSlot, srcItem);
					SendClientMessage(clientId, g_pLanguageString[_NN_Incu_Wait_More]);

					return true;
				}

				if (player->Equip[14].EFV1 <= 1)
				{
					player->Equip[p->DstSlot].sIndex += 30;

					player->Equip[p->DstSlot].EF2 = 1;
					player->Equip[p->DstSlot].EFV2 = (Rand() % 20 + 10);

					player->Equip[p->DstSlot].EF3 = 30;
					player->Equip[p->DstSlot].EFV1 = 100;
					player->Equip[p->DstSlot].EF1 = 100;
					player->Equip[p->DstSlot].EFV3 = 1;

					MountProcess(clientId, 0);

					SendClientMessage(clientId, g_pLanguageString[_NN_INCUBATED]);
				}
				else
				{
					if (player->Equip[14].EFV1 > 1)
						player->Equip[14].EFV1--;

					player->Equip[14].EF3 = 84;
					player->Equip[14].EFV3 = Rand() % 8 + 4;

				}

				SendItem(clientId, (SlotType)p->DstType, p->DstSlot, dstItem);
				SendEmotion(clientId, 14, 3);

				AmountMinus(srcItem);
			}
			else
			{
				SendClientMessage(clientId, g_pLanguageString[_NN_Mount_Not_Match]);

				SendItem(clientId, (SlotType)p->SrcType, p->SrcSlot, srcItem);
			}

			return true;
		}

		if (p->DstType == 0 && p->DstSlot == 11)
		{
			if (dstItem->sIndex >= 1752 && dstItem->sIndex < 1760)
			{
				int _rand = Rand() % 101;
				if (_rand <= 80)
				{
					dstItem->sIndex -= 8;

					Log(clientId, LOG_INGAME, "Pedra %s foi aberta", g_pItemList[dstItem->sIndex + 8].ItemName);
					SendClientMessage(clientId, g_pLanguageString[_NN_Refine_Success]);
				}
				else
				{
					Log(clientId, LOG_INGAME, "Pedra %s falhou para abrir", g_pItemList[dstItem->sIndex + 8].ItemName);

					SendClientMessage(clientId, g_pLanguageString[_NN_Fail_To_Refine]);
				}

				SendItem(clientId, (SlotType)p->DstType, p->DstSlot, dstItem);
				SendEmotion(clientId, 14, 3);

				AmountMinus(srcItem);

				return true;
			}
		}

		int sanc = GetItemSanc(dstItem);
		if (p->DstType == 0 && p->DstSlot == 8 && _volatile == 5)
		{
			if (dstItem->sIndex >= 591 && dstItem->sIndex <= 595)
			{
				if (sanc >= 9 && sanc <= 14)
				{
					int _rand = Rand() % 101;
					SendEmotion(clientId, 14, 3);

					int protectionSlot = GetFirstSlot(clientId, 4611);

					int rate = 90;
					if (sanc >= 11)
						rate = 95;

					if (_rand > rate)
					{
						SendClientMessage(clientId, g_pLanguageString[_NN_Refine_Success]);

						Log(clientId, LOG_INGAME, "Sucesso - Item %s para %d %s %d/%d", g_pItemList[dstItem->sIndex].ItemName, sanc + 1, dstItem->toString().c_str(), _rand, rate);

						SetItemSanc(dstItem, sanc + 1, 0);
					}
					else
					{
						SendClientMessage(clientId, g_pLanguageString[_NN_Fail_To_Refine]);

						if (protectionSlot == -1)
						{
							Log(clientId, LOG_INGAME, "Destruado - Item %s para %d %s %d/%d", g_pItemList[dstItem->sIndex].ItemName, sanc + 1, dstItem->toString().c_str(), _rand, rate);

							memset(dstItem, 0, sizeof STRUCT_ITEM);
						}
						else
							Log(clientId, LOG_INGAME, "Falhou - Item %s para %d %s %d/%d", g_pItemList[dstItem->sIndex].ItemName, sanc + 1, dstItem->toString().c_str(), _rand, rate);
					}

					if (protectionSlot != -1)
					{
						AmountMinus(&pMob[clientId].Mobs.Player.Inventory[protectionSlot]);

						SendItem(clientId, SlotType::Inv, protectionSlot, &pMob[clientId].Mobs.Player.Inventory[protectionSlot]);
						Log(clientId, LOG_INGAME, "Consumido Emblema da Proteaao (Brinco), evitando a perda do item");
					}

					SendItem(clientId, (SlotType)p->DstType, p->DstSlot, dstItem);
					AmountMinus(srcItem);
					return true;
				}
			}
		}

		// Cythera Desencantada - Se falhar, volta pra +0
		if (dstItem->sIndex == 3508)
		{
			if (_volatile != 5)
			{
				SendItem(clientId, (SlotType)p->SrcType, p->SrcSlot, srcItem);
				return true;
			}

			INT32 sanc = GetItemSanc(dstItem);
			if (sanc >= 9)
			{
				SendClientMessage(clientId, g_pLanguageString[_NN_CantRefine]);

				SendItem(clientId, (SlotType)p->SrcType, p->SrcSlot, srcItem);
				return true;
			}

			static INT32 g_pCytheraRate[] = { 80, 70, 60, 50, 50, 50, 50, 40, 30, 30 };

			INT32 _rate = g_pCytheraRate[sanc],
				_rand = Rand() % 100;

			if (_rand <= _rate)
			{
				SetItemSanc(dstItem, sanc + 1, 0);

				SendItem(clientId, (SlotType)p->DstType, p->DstSlot, dstItem);
				SendClientMessage(clientId, g_pLanguageString[_NN_Refine_Success]);

				if (sanc == 8)
					SendNotice(".%s refinou Cythera Desencantada para +9", pMob[clientId].Mobs.Player.MobName);

				Log(clientId, LOG_INGAME, "Sucesso na refinaaao de %s para %d - [%d] [%d %d %d %d %d %d] ", g_pItemList[dstItem->sIndex].ItemName, sanc + 1, dstItem->sIndex, dstItem->stEffect[0].cEffect, dstItem->stEffect[0].cValue,
					dstItem->stEffect[1].cEffect, dstItem->stEffect[1].cValue, dstItem->stEffect[2].cEffect, dstItem->stEffect[2].cValue);
			}
			else if (sanc != 6)
			{
				SendClientMessage(clientId, g_pLanguageString[_NN_Fail_To_Refine]);

				SetItemSanc(dstItem, 0, 0);
				SendItem(clientId, (SlotType)p->DstType, p->DstSlot, dstItem);

				Log(clientId, LOG_INGAME, "Falha na refinaaao de %s para %d - [%d] [%d %d %d %d %d %d] ", g_pItemList[dstItem->sIndex].ItemName, sanc + 1, dstItem->sIndex, dstItem->stEffect[0].cEffect, dstItem->stEffect[0].cValue,
					dstItem->stEffect[1].cEffect, dstItem->stEffect[1].cValue, dstItem->stEffect[2].cEffect, dstItem->stEffect[2].cValue);
			}
			else
			{
				memset(dstItem, 0, sizeof STRUCT_ITEM);
				SendItem(clientId, (SlotType)p->DstType, p->DstSlot, dstItem);

				SendClientMessage(clientId, "Item destruado");

				Log(clientId, LOG_INGAME, "Falha na refinaaao de %s para %d - [%d] [%d %d %d %d %d %d] - ITEM DESTRUaDO", g_pItemList[dstItem->sIndex].ItemName, sanc + 1, dstItem->sIndex, dstItem->stEffect[0].cEffect, dstItem->stEffect[0].cValue,
					dstItem->stEffect[1].cEffect, dstItem->stEffect[1].cValue, dstItem->stEffect[2].cEffect, dstItem->stEffect[2].cValue);

				LogPlayer(clientId, "Destruiu o item %s tentando refinar para +7", g_pItemList[dstItem->sIndex].ItemName);
			}

			AmountMinus(srcItem);
			return true;
		}

		if (((dstItem->sIndex >= 762 && dstItem->sIndex <= 768)  || (dstItem->sIndex >= 788 && dstItem->sIndex <= 791)) && (sanc == 9 || sanc == 10) && _volatile == 5)
		{
			int rate = 30;
			if (sanc == 10)
				rate = 15;

			if (dstItem->sIndex >= 788 && dstItem->sIndex <= 791)
			{
				rate = 15;
				if (sanc == 10)
					rate = 6;
			}

			int _rand = Rand() % 100;
			if (_rand < rate)
			{
				SetItemSanc(dstItem, sanc + 1, 0);

				SendItem(clientId, (SlotType)p->DstType, p->DstSlot, dstItem);
				SendClientMessage(clientId, g_pLanguageString[_NN_Refine_Success]);

				SendNotice(".%s refinou %s para +%d", pMob[clientId].Mobs.Player.MobName, g_pItemList[dstItem->sIndex].ItemName, sanc + 1);

				Log(clientId, LOG_INGAME, "Sucesso na refinaaao de %s para %d - %s ", g_pItemList[dstItem->sIndex].ItemName, sanc + 1, dstItem->toString().c_str());
			}
			else
			{
				SetItemSanc(dstItem, 0, 0);

				SendItem(clientId, (SlotType)p->DstType, p->DstSlot, dstItem);
				SendClientMessage(clientId, g_pLanguageString[_NN_Fail_To_Refine]);

				SendNotice(".%s falhou %s para +%d", pMob[clientId].Mobs.Player.MobName, g_pItemList[dstItem->sIndex].ItemName, sanc + 1);

				Log(clientId, LOG_INGAME, "Falha na refinaaao de %s para %d  %s", g_pItemList[dstItem->sIndex].ItemName, sanc + 1, dstItem->toString().c_str());
			}

			AmountMinus(srcItem);
			return true;
		}

		int _type = GetEffectValueByIndex(player->Equip[p->DstSlot].sIndex, EF_MOBTYPE);
		if (_volatile == 5 && _type == 3)
		{
			if (player->Equip[p->DstSlot].sIndex)
			{
				if (sanc == 15)
				{
					SendClientMessage(clientId, g_pLanguageString[_NN_CantRefine]);

					SendItem(clientId, (SlotType)p->SrcType, p->SrcSlot, srcItem);
					return true;
				}

				int _rate = Rand() % 101;

				SendEmotion(clientId, 14, 3);

				if (_rate <= RateCele[sanc])
				{
					SetItemSanc(dstItem, sanc + 1, 0);

					SendClientMessage(clientId, g_pLanguageString[_NN_Refine_Success]);

					Log(clientId, LOG_INGAME, "Refinado %s (%d %d %d %d %d %d) para %d - %d", g_pItemList[dstItem->sIndex].ItemName, dstItem->EF1, dstItem->EFV1, dstItem->EF2,
						dstItem->EFV2, dstItem->EF3, dstItem->EFV3, sanc + 1, _rate);
				}
				else
				{
					if (sanc != 0)
						SetItemSanc(dstItem, sanc - 1, 0);

					SendClientMessage(clientId, g_pLanguageString[_NN_Fail_To_Refine]);
				}

				AmountMinus(srcItem);
				SendItem(clientId, (SlotType)p->DstType, p->DstSlot, dstItem);

				return true;
			}
		}

		if (p->DstSlot <= 7 && p->DstType == (unsigned int)SlotType::Equip && _volatile == 5 && sanc == 10)
		{
			INT32 _rate = Rand() % 101;

			AmountMinus(srcItem);
			if (_rate > 96)
			{
				INT32 i = 0;
				for (; i < 3; i++)
				{
					if (dstItem->stEffect[i].cEffect == EF_SANC || (dstItem->stEffect[i].cEffect >= 116 && dstItem->stEffect[i].cEffect <= 125))
						break;
				}

				if (i != 3)
				{
					dstItem->stEffect[i].cValue += 4;

					SendEmotion(clientId, 14, 3);

					SendClientMessage(clientId, g_pLanguageString[_NN_Refine_Success]);

					Log(clientId, LOG_INGAME, "Item %s refinado com sucesso para %d - [%d] [%d %d %d %d %d %d]", g_pItemList[dstItem->sIndex].ItemName, sanc + 1, dstItem->sIndex, dstItem->stEffect[0].cEffect, dstItem->stEffect[0].cValue,
						dstItem->stEffect[1].cEffect, dstItem->stEffect[1].cValue, dstItem->stEffect[2].cEffect, dstItem->stEffect[2].cValue);
				}
				else
					Log(clientId, LOG_ERROR, "Nao encontrado o ef 43 para refinar para +11");
			}
			else
			{
				SendClientMessage(clientId, g_pLanguageString[_NN_Fail_To_Refine]);

				Log(clientId, LOG_INGAME, "Falha na refinaaao de %s para %d - [%d] [%d %d %d %d %d %d]", g_pItemList[dstItem->sIndex].ItemName, sanc + 1, dstItem->sIndex, dstItem->stEffect[0].cEffect, dstItem->stEffect[0].cValue,
					dstItem->stEffect[1].cEffect, dstItem->stEffect[1].cValue, dstItem->stEffect[2].cEffect, dstItem->stEffect[2].cValue);
			}

			SendItem(clientId, (SlotType)p->DstType, p->DstSlot, dstItem);
			return true;
		}

		if ((_volatile == 4 && sanc >= 6) || (_volatile == 5 && sanc >= 9))
		{
			SendClientMessage(clientId, g_pLanguageString[_NN_Cant_Refine_More]);

			SendItem(clientId, (SlotType)p->SrcType, p->SrcSlot, srcItem);
			return true;
		}

		int i = 0;
		for (i = 0; i < 3; i++)
		{
			if (dstItem->stEffect[i].cEffect == 0 || dstItem->stEffect[i].cEffect == 43 || (dstItem->stEffect[i].cEffect >= 116 && dstItem->stEffect[i].cEffect <= 125))
				break;
		}

		if (i == 3)
		{
			SendClientMessage(clientId, g_pLanguageString[_NN_CantRefine]);

			SendItem(clientId, (SlotType)p->SrcType, p->SrcSlot, srcItem);
			return true;
		}

		int blocked = GetEffectValueByIndex(dstItem->sIndex, 125);
		if (blocked == 1)
		{
			SendClientMessage(clientId, g_pLanguageString[_NN_CantRefine]);

			SendItem(clientId, (SlotType)p->SrcType, p->SrcSlot, srcItem);
			return true;
		}


		int _pos = g_pItemList[dstItem->sIndex].Pos;
		if (_pos == 0)
		{
			SendClientMessage(clientId, g_pLanguageString[_NN_CantRefine]);

			SendItem(clientId, (SlotType)p->SrcType, p->SrcSlot, srcItem);
			return true;
		}

		int fails = 0;
		for (i = 0; i < 3; i++)
		{
			if (dstItem->stEffect[i].cEffect == 43 || (dstItem->stEffect[i].cEffect >= 116 && dstItem->stEffect[i].cEffect <= 125))
			{
				if (dstItem->stEffect[i].cValue != 0)
				{
					fails = dstItem->stEffect[i].cValue / 10;
					break;
				}
			}
		}

		int type = GetEffectValueByIndex(dstItem->sIndex, EF_UNKNOW1);
		if (type < 0 || type >= 6)
			type = 5;

		int _Fails = Fail[sanc] * fails;
		int rate = Rate[_volatile - 4][type][sanc] + _Fails;

		if (_type == 1)
			rate = Rate[_volatile - 4][6][sanc];

		int _rate = Rand() % 101;

		STRUCT_ITEMLIST* item = (STRUCT_ITEMLIST*)& g_pItemList[dstItem->sIndex];

		if (_rate > rate)
		{
			SendClientMessage(clientId, g_pLanguageString[_NN_Fail_To_Refine]);

			Log(clientId, LOG_INGAME, "Falha na refinaaao de %s para %d - %s %d/%d", g_pItemList[dstItem->sIndex].ItemName, sanc + 1, dstItem->toString().c_str(), _rate, rate);

			LogPlayer(clientId, "Falhou ao tentar refinar %s para %d", g_pItemList[dstItem->sIndex].ItemName, sanc + 1);
			SetItemSanc(dstItem, sanc, _Fails);

			SendEmotion(clientId, 15, 1);
		}
		else
		{
			SendClientMessage(clientId, g_pLanguageString[_NN_Refine_Success]);

			Log(clientId, LOG_INGAME, "Item %s refinado com sucesso para %d - %s %d/%d", g_pItemList[dstItem->sIndex].ItemName, sanc + 1, dstItem->toString().c_str(), _rate, rate);
			LogPlayer(clientId, "Sucesso na refinaaao de %s para %d", g_pItemList[dstItem->sIndex].ItemName, sanc + 1);

			SetItemSanc(dstItem, sanc + 1, _Fails);
		}

		SendItem(clientId, (SlotType)p->DstType, p->DstSlot, dstItem);

		if (_rate > rate)
			SendEmotion(clientId, 15, 1);
		else
			SendEmotion(clientId, 14, 3);

		pMob[clientId].GetCurrentScore(clientId);
		SendScore(clientId);

		AmountMinus(srcItem);

		SendEquip(clientId);
		return true;
	}
#pragma endregion
#pragma region PILULA DO PODER MaGICO
	else if (_volatile == 6) // pilula do poder magico
	{ // 0042E9BF
		// Retira no inacio do cadigo 
		// Pois mesmo que nao seja possavel utilizar, o item a usado
		AmountMinus(srcItem);

		if (pMob[clientId].Mobs.Info.Pilula)
		{
			SendClientMessage(clientId, g_pLanguageString[_NN_Youve_Done_It_Already]);

			return true;
		}

		pMob[clientId].Mobs.Info.Pilula = true;
		pMob[clientId].GetCurrentScore(clientId);

		SendClientMessage(clientId, g_pLanguageString[_NN_Get_Skill_Point]);

		SendEtc(clientId);
		SendScore(clientId);
	}
#pragma endregion
#pragma region POEIRA DA FADA
	else if (_volatile == 7) // poeira da fada
	{ // 0042EA46
		pMob[clientId].Mobs.Player.Exp = 4000000000;

		SendClientMessage(clientId, "Recebeu 4.000.000.000 de experiancia");
		pMob[clientId].GetCurrentScore(clientId);
		AmountMinus(srcItem);
		SendScore(clientId);
		SendEtc(clientId);
	}
#pragma endregion
#pragma region OLHO CRESCENTE
	else if (_volatile == 8) // olho crescente
	{ // 0042EBDD
		pMob[clientId].Mobs.Player.Exp += 3000;

		SendClientMessage(clientId, g_pLanguageString[_DN_get_D_exp], 3000);
		SendEmotion(clientId, 20, 0);

		AmountMinus(srcItem);

		pMob[clientId].CheckGetLevel();
	}
#pragma endregion
#pragma region PEDRA LE
	else if (_volatile == 9) // pedras LE
	{
		STRUCT_ITEM* dstItem = GetItemPointer(clientId, p->DstType, p->DstSlot);

		int itemIndex = srcItem->sIndex;
		itemIndex = itemIndex - 575;

		if (itemIndex < 0 || itemIndex > 4)
		{
			SendItem(clientId, (SlotType)p->SrcType, p->SrcSlot, srcItem);
			return true;
		}

		int itemPos = g_pItemList[dstItem->sIndex].Pos;
		if (itemPos < 2 || itemPos > 32)
		{
			SendClientMessage(clientId, "Utilize em equipamentos");

			SendItem(clientId, (SlotType)p->SrcType, p->SrcSlot, srcItem);
			return true;
		}

		int itemAb = GetItemAbility(dstItem, EF_UNKNOW1);
		if (itemAb == 5)
		{
			SendClientMessage(clientId, "Nao a possavel utilizar nesse equipamento.");

			SendItem(clientId, (SlotType)p->SrcType, p->SrcSlot, srcItem);
			return true;
		}

		if (GetItemSanc(dstItem) > 9)
		{
			SendClientMessage(clientId, "Nao a possavel utilizar em itens superiores a +9");

			SendItem(clientId, (SlotType)p->SrcType, p->SrcSlot, srcItem);
			return true;
		}

		int UniqueType = -1;
		int nUnique = g_pItemList[dstItem->sIndex].Unique;

		if (nUnique == 5 || nUnique == 14 || nUnique == 24 || nUnique == 34)
			UniqueType = 0;

		if (nUnique == 6 || nUnique == 15 || nUnique == 25 || nUnique == 35)
			UniqueType = 1;

		if (nUnique == 7 || nUnique == 16 || nUnique == 26 || nUnique == 36)
			UniqueType = 2;

		if (nUnique == 8 || nUnique == 17 || nUnique == 27 || nUnique == 37)
			UniqueType = 3;

		if (nUnique == 10 || nUnique == 20 || nUnique == 30 || nUnique == 40)
			UniqueType = 3;

		if (UniqueType != itemIndex)
		{
			SendClientMessage(clientId, "Item inadequado");

			SendItem(clientId, (SlotType)p->SrcType, p->SrcSlot, srcItem);
			return true;
		}

		int itemExtreme = g_pItemList[dstItem->sIndex].Extreme;
		if (itemExtreme <= 0 || itemExtreme >= MAX_ITEMLIST)
		{
			SendItem(clientId, (SlotType)p->SrcType, p->SrcSlot, srcItem);
			return true;
		}

		if ((Rand() % 100) < 50)
		{
			SendClientMessage(clientId, g_pLanguageString[_NN_Fail_To_Refine]);

			Log(clientId, LOG_INGAME, "Item %s utilizado em %s - FALHOU %d/%d", g_pItemList[srcItem->sIndex].ItemName, dstItem->toString().c_str());

			AmountMinus(srcItem);
			return true;
		}

		int dstIndex = dstItem->sIndex;
		dstItem->sIndex = itemExtreme;

		Log(clientId, LOG_INGAME, "Item %s utilizado em %s %d [%d %d %d %d %d %d]", g_pItemList[srcItem->sIndex].ItemName, g_pItemList[dstIndex].ItemName, dstItem->sIndex, dstItem->EF1, dstItem->EFV1, dstItem->EF2, dstItem->EFV2, dstItem->EF3, dstItem->EFV3);

		SendEquip(clientId);
		AmountMinus(srcItem);

		pMob[clientId].GetCurrentScore(clientId);
		SendScore(clientId);

		SendItem(clientId, SlotType::Equip, p->DstSlot, dstItem);
	}

#pragma endregion
#pragma region ANTIDOTO / POaaO KAPPA
	else if (_volatile == 10) // antidoto ou poaao kappa
	{ // 0042F2B4
		STRUCT_AFFECT* affects = pMob[clientId].Mobs.Affects;

		int affectIndex = -1;
		for (int i = 0; i < 32; i++)
		{
			if (affects[i].Index == 4 && (affects[i].Value == 4 || affects[i].Value == 5))
				continue;

			if (affects[i].Index == 4)
			{
				affectIndex = i;
				break;
			}
		}

		if (affectIndex == -1)
			affectIndex = GetEmptyAffect(clientId, 0);

		if (affectIndex == -1)
		{
			SendItem(clientId, (SlotType)p->SrcType, p->SrcSlot, srcItem);

			return true;
		}

		int value = 6;
		int time = 15;
		if (srcItem->sIndex == 1764)
			value = 7;
		else if (srcItem->sIndex == 600)
		{
			value = 8;
			time = 15 * 60 / 8;
		}

		if (affects[affectIndex].Index != 0 && affects[affectIndex].Value != value)
		{
			SendClientMessage(clientId, "Voca ja possui uma poaao de outro tipo ativa");
			SendItem(clientId, (SlotType)p->SrcType, p->SrcSlot, srcItem);

			return true;
		}

		affects[affectIndex].Index = 4;
		affects[affectIndex].Value = value;
		affects[affectIndex].Time = time;

		SendAffect(clientId);
		SendScore(clientId);

		p364 packet;
		GetCreateMob(clientId, (BYTE*)& packet);

		GridMulticast_2(pMob[clientId].Target.X, pMob[clientId].Target.Y, (BYTE*)& packet, 0);

		AmountMinus(srcItem);
	}
#pragma endregion
#pragma region PERGAMINHO DO RETORNO
	else if (_volatile == 11) // pergaminho do retorno
	{ //0042F32D
		eMapAttribute map = GetAttribute(pMob[clientId].Target.X, pMob[clientId].Target.Y);

		int posX = pMob[clientId].Target.X;
		int posY = pMob[clientId].Target.Y;

		bool isHallKefra = posX >= 3199 && posX <= 3324 && posY >= 1663 && posY <= 1714;
		if (map.Village && !isHallKefra)
		{
			if (nTargetX == 0 && nTargetY == 0)
			{
				SendClientMessage(clientId, g_pLanguageString[_NN_No_Return_Point]);

				SendItem(clientId, (SlotType)p->SrcType, p->SrcSlot, srcItem);
				return true;
			}

			map = GetAttribute(nTargetX, nTargetY);
			if (map.CantSummon && pMob[clientId].Mobs.Player.BaseScore.Level < 1000)
			{
				SendClientMessage(clientId, g_pLanguageString[_NN_Cant_Use_That_Here]);

				SendItem(clientId, (SlotType)p->SrcType, p->SrcSlot, srcItem);
				return true;
			}

			Teleportar(clientId, nTargetX, nTargetY);
		}
		else
		{
			nTargetX = pMob[clientId].Target.X;
			nTargetY = pMob[clientId].Target.Y;

			DoRecall(clientId);
		}

		AmountMinus(srcItem);
	}
#pragma endregion
#pragma region GEMA ESTELAR
	else if (_volatile == 12)
	{ // 0042F533
		eMapAttribute map = GetAttribute(pMob[clientId].Target.X, pMob[clientId].Target.Y);

		if (map.CantSummon && pMob[clientId].Mobs.Player.BaseScore.Level < 1000)
		{
			for (INT32 i = 0; i < 3; i++)
			{
				if (pMob[clientId].Target.X >= g_pPesaArea[i][0] && pMob[clientId].Target.X <= g_pPesaArea[i][2] && pMob[clientId].Target.Y >= g_pPesaArea[i][1] && pMob[clientId].Target.Y <= g_pPesaArea[i][3])
				{
					INT32 face = pMob[clientId].Mobs.Player.Equip[0].sIndex / 10; // local960
					if (face == 0)
						SendEmotion(clientId, 23, 0);
					else if (face >= 1 && face <= 3)
						SendEmotion(clientId, 15, 0);

					pMob[clientId].Mobs.Nightmare[i].X = pMob[clientId].Target.X;
					pMob[clientId].Mobs.Nightmare[i].Y = pMob[clientId].Target.Y;

					Log(clientId, LOG_INGAME, "Armazenado Gema Estelar em %dx %dy", pMob[clientId].Target.X, pMob[clientId].Target.Y);

					AmountMinus(srcItem);

					SendClientMessage(clientId, g_pLanguageString[_NN_Set_Warp]);
					return true;
				}
			}

			SendClientMessage(clientId, g_pLanguageString[_NN_Cant_Use_That_Here]);

			SendItem(clientId, (SlotType)p->SrcType, p->SrcSlot, srcItem);
			return true;
		}

		INT32 arena = GetArena(pMob[clientId].Target.X, pMob[clientId].Target.Y); // local958
		INT32 village = GetVillage(pMob[clientId].Target.X, pMob[clientId].Target.Y); // local959
		if (arena < 5 || village < 5)
		{
			SendClientMessage(clientId, g_pLanguageString[_NN_Cant_Use_That_Here]);

			SendItem(clientId, (SlotType)p->SrcType, p->SrcSlot, srcItem);
			return true;
		}

		INT32 face = pMob[clientId].Mobs.Player.Equip[0].sIndex / 10; // local960
		if (face == 0)
			SendEmotion(clientId, 23, 0);
		else if (face >= 1 && face <= 3)
			SendEmotion(clientId, 15, 0);

		pMob[clientId].Mobs.Player.Last.X = pMob[clientId].Target.X;
		pMob[clientId].Mobs.Player.Last.Y = pMob[clientId].Target.Y;

		Log(clientId, LOG_INGAME, "Armazenado Gema Estelar em %dx %dy", pMob[clientId].Target.X, pMob[clientId].Target.Y);

		AmountMinus(srcItem);

		SendClientMessage(clientId, g_pLanguageString[_NN_Set_Warp]);
	}
#pragma endregion
#pragma region PERGAMINHO DO TELEPORTE
	else if (_volatile == 13) // pergaminho do teleporte
	{ //0042F7A1
		Teleportar(clientId, pMob[clientId].Mobs.Player.Last.X, pMob[clientId].Mobs.Player.Last.Y);

		AmountMinus(srcItem);
	}
#pragma endregion
#pragma region UNKNOWN
	else if (_volatile == 14) // evocaaao neses (N)
	{ //0042F824
	}
#pragma endregion 
#pragma region RAaaES
	else if (_volatile == 15) // raaaes
	{ //0042F919
		if (p->DstType != 0 || p->DstSlot != 14)
		{
			SendItem(clientId, (SlotType)p->SrcType, p->SrcSlot, srcItem);

			return true;
		}

		STRUCT_ITEM* mont = &pMob[clientId].Mobs.Player.Equip[14];
		if (mont->sIndex < 2330 || mont->sIndex > 2390)
		{
			SendItem(clientId, (SlotType)p->SrcType, p->SrcSlot, srcItem);

			return true;
		}

		INT32 baseId = 0;
		if (mont->sIndex >= 2330 && mont->sIndex < 2360)
			baseId = 2330;

		if (mont->sIndex >= 2360 && mont->sIndex < 2390)
			baseId = 2360;

		INT32 mountId = (mont->sIndex - baseId) % 30; // local1028
		INT32 feedId = (srcItem->sIndex - 2420);	// LOCAL1029

		if (mountId >= 6 && mountId <= 15 || mountId == 27)
			mountId = 6;

		if (mountId == 19)
			mountId = 7;

		if (mountId == 20)
			mountId = 8;

		if (mountId == 21 || mountId == 22 || mountId == 23 || mountId == 28)
			mountId = 9;

		if (mountId == 24 || mountId == 25 || mountId == 26)
			mountId = 10;

		if (mountId == 29)
			mountId = 19;

		if (mountId != feedId)
		{
			SendClientMessage(clientId, g_pLanguageString[_NN_Mount_Not_Match]);

			SendItem(clientId, (SlotType)p->SrcType, p->SrcSlot, srcItem);
			return true;
		}

		if (*(short*)& mont->stEffect[0].cEffect <= 0)
		{
			SendItem(clientId, (SlotType)p->SrcType, p->SrcSlot, srcItem);

			return true;
		}

		STRUCT_ITEM* item = &pMob[clientId].Mobs.Player.Equip[14];

		INT32 index = item->sIndex - baseId + 10;
		INT32 hp = *(short*)& item->stEffect[0].cEffect + 5000;
		INT32 maxHp = NPCBase[index].BaseScore.MaxHp;

		if (hp > maxHp)
			hp = maxHp;

		*(short*)& item->stEffect[0].cEffect = hp;

		INT32 LOCAL_1032 = item->stEffect[2].cEffect + 2; // local1032
		if (LOCAL_1032 > 100)
			LOCAL_1032 = 100;

		item->stEffect[2].cEffect = LOCAL_1032;

		if (mont->sIndex >= 2330 && mont->sIndex < 2360)
			MountProcess(clientId, 0);

		if (mont->sIndex >= 2360 && mont->sIndex < 2390)
			ProcessAdultMount(clientId, 0);

		AmountMinus(srcItem);

		SendSignalParm(clientId, 0x7530, 0x3A3, 0x10E);
		SendItem(clientId, (SlotType)p->DstType, p->DstSlot, mont);
		return true;
	}
#pragma endregion
#pragma region PERGAMINHO aGUA + PERGAMINHO GUILDA
	else if (_volatile == 17) // pergaminho da guilda
	{ // 00430286
	}
	else if ((_volatile >= 21 && _volatile <= 30) || (_volatile >= 131 && _volatile <= 140) || (_volatile >= 161 && _volatile <= 170))
	{// 00430306

		STRUCT_MOB* player = &pMob[clientId].Mobs.Player;

		char tmp[1024];

		INT32 minus = 3182;
		INT32 type = 2;
		if (_volatile >= 21 && _volatile <= 30)
		{
			minus = 777;
			type = 1;
		}
		else if (_volatile >= 131 && _volatile <= 140)
		{
			minus = 3173;
			type = 0;
		}

		int sIndex = srcItem->sIndex - minus;

		if (sServer.pWater[type][sIndex].Time != -1)
		{
			sprintf_s(tmp, "Sala ocupada por %s e seu grupo. Tempo restante: %d seg.", pMob[sServer.pWater[type][sIndex].Leader].Mobs.Player.MobName, sServer.pWater[type][sIndex].Time);

			SendClientMessage(clientId, tmp);
			SendItem(clientId, SlotType::Inv, p->SrcSlot, srcItem);
			return true;
		}

		if (pMob[clientId].Leader != 0 && pMob[clientId].Leader != -1)
		{
			SendClientMessage(clientId, "Apenas o lader do grupo pode utilizar!");

			SendItem(clientId, SlotType::Inv, p->SrcSlot, srcItem);
			return TRUE;
		}

		if (User.Water.Total >= sServer.MaxWaterEntrance)
		{
			SendClientMessage(clientId, "Voca atingiu o limite de entradas diaria!");

			SendItem(clientId, SlotType::Inv, p->SrcSlot, srcItem);
			return TRUE;
		}

		INT32 t = 0;
		for (; t < 9; t++)
		{
			if (pMob[clientId].Target.X >= waterMaxMin[type][t][0] && pMob[clientId].Target.X <= waterMaxMin[type][t][2] && pMob[clientId].Target.Y >= waterMaxMin[type][t][1] && pMob[clientId].Target.Y <= waterMaxMin[type][t][3])
				break;
		}

		INT32 initial = PERGA_A;
		if (_volatile >= 21 && _volatile <= 30)
			initial = PERGA_M;
		else if (_volatile >= 131 && _volatile <= 140)
			initial = PERGA_N;

		if ((pMob[clientId].Target.X > 1961 && pMob[clientId].Target.X < 1970 && pMob[clientId].Target.Y > 1770 && pMob[clientId].Target.Y < 1778) || (t != 9 && sServer.pWater[type][t].Mode == 2))
		{
			if (t == sIndex)
			{
				SendItem(clientId, SlotType::Inv, p->SrcSlot, srcItem);
				return true;
			}

			if (t != 9 && sServer.pWater[type][t].Mode == 2)
			{
				for (INT32 u = 1000; u < 30000; u++)
				{
					if (pMob[u].Target.X >= waterMaxMin[type][t][0] && pMob[u].Target.X <= waterMaxMin[type][t][2] && pMob[u].Target.Y >= waterMaxMin[type][t][1] && pMob[u].Target.Y <= waterMaxMin[type][t][3])
					{
						if (pMob[t].Summoner == 0)
							MobKilled(u, u, 0, 0);
					}
				}

				if (t == 8)
				{
					mGener.pList[initial + 8].MobCount = 0;
					mGener.pList[initial + 9].MobCount = 0;
					mGener.pList[initial + 10].MobCount = 0;
					mGener.pList[initial + 11].MobCount = 0;
				}
				else
					mGener.pList[initial + t].MobCount = 0;

				sServer.pWater[type][t].Time = -1;
				sServer.pWater[type][t].Mode = 0;
			}

			sServer.pWater[type][sIndex].Time = 120;

			int Generated = initial + sIndex;

			if (sIndex == 8)
			{
				int chance = Rand() % 100;
				if (chance >= 75)
					Generated = initial + 8;
				else if (chance >= 50)
					Generated = initial + 9;
				else if (chance >= 25)
					Generated = initial + 10;
				else
					Generated = initial + 11;
			}

			for (int i = 1000; i < 30000; i++)
			{
				if (pMob[i].GenerateID == Generated)
					MobKilled(i, i, 0, 0);
			}

			GenerateMob(Generated, 0, 0);

			Teleportar(clientId, waterPlace[type][sIndex][0], waterPlace[type][sIndex][1]);
			SendSignalParm(clientId, 0x7530, 0x3A1, 120);
			SendSignalParm(clientId, clientId, 0x3B0, Generated);

			Log(clientId, LOG_INGAME, "agua %d - Grupo teleportado para a area. Entradas restantes: %d", srcItem->sIndex, sServer.MaxWaterEntrance - User.Water.Total);

			for (INT32 i = 0; i < 12; i++)
			{
				INT32 mobId = pMob[clientId].PartyList[i];
				if (mobId <= 0 || mobId >= MAX_PLAYER)
					continue;

				if (pUser[mobId].CurrentScore != USER_PLAY)
					continue;

				if (pUser[mobId].User.Water.Total >= sServer.MaxWaterEntrance)
				{
					SendClientMessage(mobId, "Voca atingiu o limite de entradas diaria!");

					Log(mobId, LOG_INGAME, "agua %d - Nao teleportado por falta de entradas agua", pUser[mobId].User.Water.Total);
					continue;
				}

				Teleportar(mobId, waterPlace[type][sIndex][0], waterPlace[type][sIndex][1]);

				Log(clientId, LOG_INGAME, "agua %d - Grupo: %d - Nome: %s", srcItem->sIndex, i, pMob[mobId].Mobs.Player.MobName);
				Log(mobId, LOG_INGAME, "agua %d - Grupo de %s. Entradas restantes: %d", srcItem->sIndex, pMob[clientId].Mobs.Player.MobName, sServer.MaxWaterEntrance - pUser[mobId].User.Water.Total);

				LogPlayer(mobId, "%s usou a entrada da agua. Entradas restantes: %d", pMob[clientId].Mobs.Player.MobName, sServer.MaxWaterEntrance - pUser[mobId].User.Water.Total);

				SendSignalParm(mobId, mobId, 0x3B0, Generated);
				SendSignalParm(mobId, 0x7530, 0x3A1, 120);

				pUser[mobId].User.Water.Total++;
			}

			sServer.pWater[type][sIndex].Leader = clientId;
			sServer.pWater[type][sIndex].Mode = 1;
			sServer.pWater[type][sIndex].Time = 120;

			User.Water.Total++;

			AmountMinus(srcItem);

		}
		else
		{
			SendClientMessage(clientId, "Use na Zona Elemental da agua!");

			SendItem(clientId, SlotType::Inv, p->SrcSlot, srcItem);
			return true;
		}
	}
#pragma endregion
#pragma region PESADELOS
	else if (_volatile == 173) // Pesadelo N/Grupo
	{
		STRUCT_MOB* plaer = &pMob[clientId].Mobs.Player;
		int village = GetVillage(pMob[clientId].Target.X, pMob[clientId].Target.Y);

		if (village != 2)
		{
			SendClientMessage(clientId, g_pLanguageString[_NN_Only_Erion]);

			SendItem(clientId, SlotType::Inv, p->SrcSlot, srcItem);
			return true;
		}

		if (pMob[clientId].Leader != 0)
		{
			SendClientMessage(clientId, g_pLanguageString[_NN_Party_Leader_Only]);

			SendItem(clientId, SlotType::Inv, p->SrcSlot, srcItem);
			return true;
		}

		if (sServer.Nightmare[0].CurrentScore != 1 || sServer.Nightmare[0].TimeLeft <= 0)
		{
			SendClientMessage(clientId, g_pLanguageString[_NN_CantEnter]);

			SendItem(clientId, SlotType::Inv, p->SrcSlot, srcItem);
			return true;
		}

		std::vector<unsigned int> players;
		for (int i = 0; i < 13; i++)
		{
			int memberId = 0;
			if (i != 12)
				memberId = pMob[clientId].PartyList[i];
			else
				memberId = clientId;

			if (memberId <= 0 || memberId >= MAX_PLAYER)
				continue;

			if (pMob[memberId].Mobs.Player.Equip[0].EFV2 >= CELESTIAL)
			{
				if (pMob[memberId].Mobs.PesaEnter <= 0)
				{
					SendClientMessage(memberId, g_pLanguageString[_NN_NT_Zero]);

					continue;
				}
				else if (pMob[memberId].Mobs.Player.CurrentScore.Level >= sServer.MaximumPesaLevel)
				{
					SendClientMessage(memberId, g_pLanguageString[_NN_Level_NotAllowed], 1, sServer.MaximumPesaLevel);

					continue;
				}
			}

			players.push_back(memberId);
		}

		size_t remaining = 0;
		for (int i = 0; i < 40; i++)
		{
			if (sServer.Nightmare[0].Members[i] == 0)
				remaining++;
		}

		if (players.size() > remaining)
		{
			SendClientMessage(clientId, g_pLanguageString[_NN_NoSpace_AllGroup]);

			SendItem(clientId, SlotType::Inv, p->SrcSlot, srcItem);
			return true;
		}

		if (players.size() == 0)
		{
			SendClientMessage(clientId, "Ninguam no grupo esta habilitado a entrar no Pesadelo");

			SendItem(clientId, SlotType::Inv, p->SrcSlot, srcItem);
			return true;
		}
		
		for (size_t i = 0; i < players.size(); i++)
		{
			int memberId = players[i];
			int x = 0;
			for (; x < 40; x++)
			{
				if (sServer.Nightmare[0].Members[x] == 0)
					break;
			}

			if (pMob[memberId].Mobs.Player.Equip[0].EFV2 >= CELESTIAL)
			{
				pMob[memberId].Mobs.PesaEnter--;
				Log(memberId, LOG_INGAME, "Consumiu NT. Restante: %d", (int)pMob[memberId].Mobs.PesaEnter);
			}

			sServer.Nightmare[0].Members[x] = memberId;
			Teleportar(memberId, 1210, 180);

			Log(clientId, LOG_INGAME, "Pesadelo N - Slot: %d - Name: %s", i, pMob[memberId].Mobs.Player.MobName);
			Log(memberId, LOG_INGAME, "Pesadelo N - Grupo de %s", pMob[clientId].Mobs.Player.MobName);

			int _rand = Rand() % 3;
			if (_rand >= 3)
				_rand = 2;

			int posX = CoordsPesaN[_rand][0];
			int posY = CoordsPesaN[_rand][1];
			if (pMob[memberId].Mobs.Nightmare[0].X != 0 && pMob[memberId].Mobs.Nightmare[0].Y != 0)
			{
				posX = pMob[memberId].Mobs.Nightmare[0].X;
				posY = pMob[memberId].Mobs.Nightmare[0].Y;
			}

			Teleportar(memberId, posX, posY);

			SendSignalParm(memberId, SERVER_SIDE, 0x3A7, 2);
			SendSignalParm(memberId, memberId, 0x3A1, sServer.Nightmare[0].TimeLeft);
		}

		AmountMinus(srcItem);
	}
	else if (_volatile == 174) // Pesadelo M/Grupo
	{
		STRUCT_MOB* player = &pMob[clientId].Mobs.Player;
		int village = GetVillage(pMob[clientId].Target.X, pMob[clientId].Target.Y);

		if (village != 0)
		{
			SendClientMessage(clientId, g_pLanguageString[_NN_Only_Armia]);

			SendItem(clientId, SlotType::Inv, p->SrcSlot, srcItem);
			return true;
		}

		if (pMob[clientId].Leader != 0)
		{
			SendClientMessage(clientId, g_pLanguageString[_NN_Party_Leader_Only]);

			SendItem(clientId, SlotType::Inv, p->SrcSlot, srcItem);
			return true;
		}

		if (sServer.Nightmare[1].CurrentScore != 1 || sServer.Nightmare[1].TimeLeft <= 0)
		{
			SendClientMessage(clientId, g_pLanguageString[_NN_CantEnter]);

			SendItem(clientId, SlotType::Inv, p->SrcSlot, srcItem);
			return true;
		}

		std::vector<unsigned int> players;
		for (int i = 0; i < 13; i++)
		{
			int memberId = 0;
			if (i != 12)
				memberId = pMob[clientId].PartyList[i];
			else
				memberId = clientId;

			if (memberId <= 0 || memberId >= MAX_PLAYER)
				continue;

			if (pMob[memberId].Mobs.Player.Equip[0].EFV2 >= CELESTIAL)
			{
				if (pMob[memberId].Mobs.PesaEnter <= 0)
				{
					SendClientMessage(memberId, g_pLanguageString[_NN_NT_Zero]);

					continue;
				}
				else if (pMob[memberId].Mobs.Player.CurrentScore.Level >= sServer.MaximumPesaLevel)
				{
					SendClientMessage(memberId, g_pLanguageString[_NN_Level_NotAllowed], 1, sServer.MaximumPesaLevel);

					continue;
				}
			}

			players.push_back(memberId);
		}

		size_t remaining = 0;
		for (int i = 0; i < 40; i++)
		{
			if (sServer.Nightmare[1].Members[i] == 0)
				remaining++;
		}

		if (players.size() > remaining)
		{
			SendClientMessage(clientId, g_pLanguageString[_NN_NoSpace_AllGroup]);

			SendItem(clientId, SlotType::Inv, p->SrcSlot, srcItem);
			return true;
		}

		if (players.size() == 0)
		{
			SendClientMessage(clientId, "Ninguam no grupo esta habilitado a entrar no Pesadelo");

			SendItem(clientId, SlotType::Inv, p->SrcSlot, srcItem);
			return true;
		}

		for (size_t i = 0; i < players.size(); i++)
		{
			auto memberId = players[i];
			int x = 0;
			for (; x < 40; x++)
			{
				if (sServer.Nightmare[1].Members[x] == 0)
					break;
			}

			if (pMob[memberId].Mobs.Player.Equip[0].EFV2 >= CELESTIAL)
			{
				pMob[memberId].Mobs.PesaEnter--;
				Log(memberId, LOG_INGAME, "Consumiu NT. Restante: %d", (int)pMob[memberId].Mobs.PesaEnter);
			}

			sServer.Nightmare[1].Members[x] = memberId;

			Teleportar(memberId, 1210, 180);

			Log(clientId, LOG_INGAME, "Pesadelo M - Slot: %d - Name: %s", i, pMob[memberId].Mobs.Player.MobName);
			Log(memberId, LOG_INGAME, "Pesadelo M - Grupo de %s", pMob[clientId].Mobs.Player.MobName);

			int _rand = Rand() % 6;
			if (_rand >= 6)
				_rand = 5;

			int posX = CoordsPesaM[_rand][0];
			int posY = CoordsPesaM[_rand][1];

			if (pMob[memberId].Mobs.Nightmare[1].X != 0 && pMob[memberId].Mobs.Nightmare[1].Y != 0)
			{
				posX = pMob[memberId].Mobs.Nightmare[1].X;
				posY = pMob[memberId].Mobs.Nightmare[1].Y;
			}

			Teleportar(memberId, posX, posY);

			SendSignalParm(memberId, SERVER_SIDE, 0x3A7, 2);
			SendSignalParm(memberId, memberId, 0x3A1, sServer.Nightmare[1].TimeLeft);
		}

		AmountMinus(srcItem);
	}
	else if (_volatile == 175) // Pesadelo A/Grupo
	{
		STRUCT_MOB* player = &pMob[clientId].Mobs.Player;
		int village = GetVillage(pMob[clientId].Target.X, pMob[clientId].Target.Y);
		if (village != 1)
		{
			SendClientMessage(clientId, g_pLanguageString[_NN_Only_Arzam]);

			SendItem(clientId, SlotType::Inv, p->SrcSlot, srcItem);
			return true;
		}

		if (pMob[clientId].Leader != 0)
		{
			SendClientMessage(clientId, g_pLanguageString[_NN_Party_Leader_Only]);

			SendItem(clientId, SlotType::Inv, p->SrcSlot, srcItem);
			return true;
		}

		if (sServer.Nightmare[2].CurrentScore != 1 || sServer.Nightmare[2].TimeLeft <= 0)
		{
			SendClientMessage(clientId, g_pLanguageString[_NN_CantEnter]);

			SendItem(clientId, SlotType::Inv, p->SrcSlot, srcItem);
			return true;
		}

		std::stringstream str;
		std::vector<unsigned int> players;
		for (int i = 0; i < 13; i++)
		{
			int memberId = 0;
			if (i != 12)
				memberId = pMob[clientId].PartyList[i];
			else
				memberId = clientId;

			if (memberId <= 0 || memberId >= MAX_PLAYER)
				continue;

			if (pMob[memberId].Mobs.Player.Equip[0].EFV2 >= CELESTIAL)
			{
				if (pMob[memberId].Mobs.PesaEnter <= 0)
				{
					SendClientMessage(memberId, g_pLanguageString[_NN_NT_Zero]);

					str << "O jogador " << pMob[memberId].Mobs.Player.MobName << "(" << pUser[memberId].User.Username << ") nao entrara por falta de NT" << std::endl;
					continue;
				}
				else if (pMob[memberId].Mobs.Player.CurrentScore.Level >= sServer.MaximumPesaLevel)
				{
					SendClientMessage(memberId, g_pLanguageString[_NN_Level_NotAllowed], 1, sServer.MaximumPesaLevel);

					str << "O jogador " << pMob[memberId].Mobs.Player.MobName << "(" << pUser[memberId].User.Username << ") nao entrara por conta do limite de navel" << std::endl;
					continue;
				}
			}

			players.push_back(memberId);
			str << "O jogador " << pMob[memberId].Mobs.Player.MobName << "(" << pUser[memberId].User.Username << ") foi adicionado a lista" << std::endl;
		}

		size_t remaining = 0;
		for (int i = 0; i < 40; i++)
		{
			if (sServer.Nightmare[2].Members[i] == 0)
				remaining++;
		}

		if (players.size() > remaining)
		{
			SendClientMessage(clientId, g_pLanguageString[_NN_NoSpace_AllGroup]);

			SendItem(clientId, SlotType::Inv, p->SrcSlot, srcItem);
			return true;
		}

		if (players.size() == 0)
		{
			SendClientMessage(clientId, "Ninguam no grupo esta habilitado a entrar no Pesadelo");

			SendItem(clientId, SlotType::Inv, p->SrcSlot, srcItem);
			return true;
		}

		str << "[GRUPO]" << std::endl;

		for (size_t i = 0; i < players.size(); i++)
		{
			int memberId = players[i];
			int x = 0;
			for (; x < 40; x++)
			{
				if (sServer.Nightmare[2].Members[x] == 0)
					break;
			}

			if (pMob[memberId].Mobs.Player.Equip[0].EFV2 >= CELESTIAL)
			{
				pMob[memberId].Mobs.PesaEnter--;
				Log(memberId, LOG_INGAME, "Consumiu NT. Restante: %d", (int)pMob[memberId].Mobs.PesaEnter);
			}

			sServer.Nightmare[2].Members[x] = memberId;
			sServer.Nightmare[2].MembersName[x] = pMob[memberId].Mobs.Player.MobName;

			INT32 posX = 1210,
				posY = 180;
			if (pMob[memberId].Mobs.Nightmare[2].X != 0 && pMob[memberId].Mobs.Nightmare[2].Y != 0)
			{
				posX = pMob[memberId].Mobs.Nightmare[2].X;
				posY = pMob[memberId].Mobs.Nightmare[2].Y;
			}

			Teleportar(memberId, posX, posY);

			str << "[Pesadelo A] - " << pUser[memberId].User.Username << " - " << pMob[memberId].Mobs.Player.MobName << std::endl;

			SendSignalParm(memberId, SERVER_SIDE, 0x3A7, 2);
			SendSignalParm(memberId, memberId, 0x3A1, sServer.Nightmare[2].TimeLeft);
		}

		AmountMinus(srcItem);

		for (auto player : players)
			Log(player, LOG_INGAME, str.str().c_str());
	}
#pragma endregion
#pragma region PEDRA IDEAL
	else if (_volatile == 211)
	{
		INT32 classMaster = pMob[clientId].Mobs.Player.Equip[0].EFV2;
		if (classMaster >= eClass::Celestial && pMob[clientId].Mobs.GetTotalResets() == 3 && pMob[clientId].Mobs.Player.BaseScore.Level >= 199 && pMob[clientId].Mobs.Info.LvBlocked)
		{
			if (pMob[clientId].Mobs.Player.BaseScore.Level == 199 && pMob[clientId].Mobs.Sub.SubStatus.Level == 199)
			{
				int realSealSlot = GetFirstSlot(clientId, 4683);
				if (realSealSlot == -1)
				{
					SendClientMessage(clientId, "Necessario Selo Real para destravar o navel");

					SendItem(clientId, (SlotType)p->SrcType, p->SrcSlot, srcItem);
					return true;
				}

				if (pMob[clientId].Mobs.Fame < 1000)
				{
					SendClientMessage(clientId, "Necessario 1000 pontos de fama para destravar o navel");

					return true;
				}

				pMob[clientId].Mobs.Player.Inventory[realSealSlot] = STRUCT_ITEM{};
				pMob[clientId].Mobs.Fame -= 1000;

				SendItem(clientId, SlotType::Inv, realSealSlot, &pMob[clientId].Mobs.Player.Inventory[realSealSlot]);

				pMob[clientId].Mobs.Info.LvBlocked = 0;
				pMob[clientId].Mobs.Info.Unlock200 = 1;

                auto& sub = pMob[clientId].Mobs.Sub;
                sub.Info.LvBlocked = 0;
                sub.Info.Unlock200 = 1;

				Log(clientId, LOG_INGAME, "Navel desbloqueado com sucesso. Evoluaao atual: %d. Consumido Selo Real do slot %d", pMob[clientId].Mobs.Player.GetEvolution(), realSealSlot);
					
				SendClientMessage(clientId, "Navel desbloqueado");

				AmountMinus(srcItem);
				return true;
			}

			SendItem(clientId, (SlotType)p->SrcType, p->SrcSlot, srcItem);
			return true;
		}

		if (classMaster == ARCH)
		{ // Transformar-se em celestial
			INT32 level = pMob[clientId].Mobs.Player.CurrentScore.Level;
			if (level < 354)
			{
				//TODO : HardCore
				SendClientMessage(clientId, "Funaao ainda nao implementada!");

				SendItem(clientId, (SlotType)p->SrcType, p->SrcSlot, srcItem);
				return true;
			}

			UINT16
				points = 0,
				cythera = 0;

			UNION_QUESTINFO quest = pMob[clientId].Mobs.Info;
			if (quest.Elime)
				points += 100;
			if (quest.Sylphed)
				points += 100;
			if (quest.Noas)
				points += 100;
			if (quest.Thelion)
				points += 100;

			if (level >= 355 && level <= 369)
				cythera = 3500;
			else if (level >= 370 && level <= 379)
				cythera = 3500;
			else if (level >= 380 && level <= 397)
				cythera = 3501;
			else if (level == 398)
				cythera = 3501;
			else if (level == 399)
				cythera = 3502;
			else
			{
				SendClientMessage(clientId, "Level inadequado!");

				SendItem(clientId, (SlotType)p->SrcType, p->SrcSlot, srcItem);
				return true;
			}

			int capeId = 0;
			switch (pMob[clientId].Mobs.Player.CapeInfo)
			{
			case CAPE_BLUE:
				capeId = 3197;
				break;
			case CAPE_RED:
				capeId = 3198;
				break;
			case CAPE_WHITE:
				capeId = 3199;
				break;
			}

			if (capeId == 0)
			{
				SendClientMessage(clientId, "Necessario possuir um Reino para criaaao do celestial");

				SendItem(clientId, (SlotType)p->SrcType, p->SrcSlot, srcItem);
				return true;
			}

			INT32 levelMortal = pUser[clientId].CharList.CurrentScore[pMob[clientId].Mobs.MortalSlot].Level;
			if (levelMortal != 399)
			{
				SendClientMessage(clientId, "Necessario que o Mortal seja navel 400");

				SendItem(clientId, (SlotType)p->SrcType, p->SrcSlot, srcItem);
				return true;
			}

			if (pMob[clientId].Mobs.Player.Equip[1].sIndex != 0)
			{
				SendClientMessage(clientId, "Desequipe sua cythera para continuar");

				SendItem(clientId, (SlotType)p->SrcType, p->SrcSlot, srcItem);
				return true;
			}

			int face = pMob[clientId].Mobs.Player.Equip[0].EF2;
			int points_tk[4] = { 8, 4, 7, 6 };
			int points_ht[4] = { 8, 9, 13, 6 };
			int points_bm[4] = { 6, 6, 9, 5 };
			int points_fm[4] = { 5, 8, 5, 5 };

			if (face > 31)
			{
				pMob[clientId].Mobs.Player.BaseScore.Str = points_ht[0];
				pMob[clientId].Mobs.Player.BaseScore.Int = points_ht[1];
				pMob[clientId].Mobs.Player.BaseScore.Dex = points_ht[2];
				pMob[clientId].Mobs.Player.BaseScore.Con = points_ht[3];
			}
			else if (face > 21)
			{
				pMob[clientId].Mobs.Player.BaseScore.Str = points_bm[0];
				pMob[clientId].Mobs.Player.BaseScore.Int = points_fm[1];
				pMob[clientId].Mobs.Player.BaseScore.Dex = points_fm[2];
				pMob[clientId].Mobs.Player.BaseScore.Con = points_fm[3];
			}
			else if (face > 11)
			{
				pMob[clientId].Mobs.Player.BaseScore.Str = points_fm[0];
				pMob[clientId].Mobs.Player.BaseScore.Int = points_fm[1];
				pMob[clientId].Mobs.Player.BaseScore.Dex = points_fm[2];
				pMob[clientId].Mobs.Player.BaseScore.Con = points_fm[3];
			}
			else if (face > 1)
			{
				pMob[clientId].Mobs.Player.BaseScore.Str = points_tk[0];
				pMob[clientId].Mobs.Player.BaseScore.Int = points_tk[1];
				pMob[clientId].Mobs.Player.BaseScore.Dex = points_tk[2];
				pMob[clientId].Mobs.Player.BaseScore.Con = points_tk[3];
			}

			if (level >= 355 && level <= 369)
				pMob[clientId].Mobs.Info.Level355 = 1;
			else if (level >= 370 && level <= 379)
				pMob[clientId].Mobs.Info.Level370 = 1;
			else if (level >= 380 && level <= 397)
				pMob[clientId].Mobs.Info.Level380 = 1;
			else if (level == 398)
				pMob[clientId].Mobs.Info.Level398 = 1;
			else if (level == 399)
				pMob[clientId].Mobs.Info.Level399 = 1;
			else
			{
				SendItem(clientId, (SlotType)p->SrcType, p->SrcSlot, srcItem);
				return true;
			}

			Log(clientId, LOG_INGAME, "Celestial criado. Level: %d - Cythera: %d. Points %d. Cape %d", level, cythera, points, capeId);

			SendServerNotice("Parabans para %s pela criaaao do seu Celestial", pMob[clientId].Mobs.Player.MobName);

			memset(&pMob[clientId].Mobs.Player.ShortSkill, -1, 4);
			memset(&pMob[clientId].Mobs.SkillBar, -1, 16);

			memset(&pMob[clientId].Mobs.Player.Equip[1], 0, sizeof STRUCT_ITEM);

			pMob[clientId].Mobs.Player.LearnedSkill[0] = (1 << 30);

			pMob[clientId].Mobs.Player.Equip[0].EFV2 = CELESTIAL;
			pMob[clientId].Mobs.Player.Equip[1] = STRUCT_ITEM{ cythera };

			pMob[clientId].Mobs.Player.BaseScore.Level = 0;
			pMob[clientId].Mobs.Player.CurrentScore.Level = 0;
			pMob[clientId].Mobs.Player.Exp = 0;

			for (size_t i = 0; i < 4; i++)
				pMob[clientId].Mobs.Player.BaseScore.Special[i] = 0;

			memset(&pMob[clientId].Mobs.Player.Equip[15], 0, sizeof STRUCT_ITEM);
			pMob[clientId].Mobs.Player.Equip[15].sIndex = capeId;

			SendItem(clientId, SlotType::Equip, 15, &pMob[clientId].Mobs.Player.Equip[15]);
			SendItem(clientId, SlotType::Equip, 1, &pMob[clientId].Mobs.Player.Equip[1]);
			SendScore(clientId);

			AmountMinus(srcItem);

			CharLogOut(clientId);
			SendSignalParm(clientId, clientId, 0x3B4, pUser[clientId].inGame.CharSlot);

			SendClientMessage(clientId, "Que Deus abenaoe seu caminho!");
		}
		else if (classMaster == CELESTIAL)
		{
			if (pMob[clientId].Mobs.Sub.CurrentScore == 1)
			{
				SendClientMessage(clientId, "Voca ja possui subcelestial");

				SendItem(clientId, (SlotType)p->SrcType, p->SrcSlot, srcItem);
				return true;
			}

			if (pMob[clientId].Mobs.Player.CurrentScore.Level < 119)
			{
				SendClientMessage(clientId, "Sa a possavel criar Subcelestial a partir do level 120");

				SendItem(clientId, (SlotType)p->SrcType, p->SrcSlot, srcItem);
				return true;
			}

			if (pMob[clientId].Mobs.Player.Equip[11].sIndex < 1760 || pMob[clientId].Mobs.Player.Equip[11].sIndex > 1763)
			{
				SendClientMessage(clientId, "a necessario uma Sephirot para a criaaao do Sub Celestial");

				SendItem(clientId, (SlotType)p->SrcType, p->SrcSlot, srcItem);
				return true;
			}

			p830 packet;
			memset(&packet, 0, sizeof p830);

			packet.Header.PacketId = 0x830;
			packet.Header.Size = sizeof packet;
			packet.Header.ClientId = clientId;

			packet.CharPos = pUser[clientId].inGame.CharSlot;
			packet.Class = (pMob[clientId].Mobs.Player.Equip[11].sIndex - 1760);
			packet.LearnedSkill = pMob[clientId].Mobs.Player.LearnedSkill[0];
			packet.Face = pMob[clientId].Mobs.Player.Equip[0].EF2;

			int capeId = 0;
			switch (pMob[clientId].Mobs.Player.CapeInfo)
			{
			case CAPE_BLUE:
				capeId = 3197;
				break;
			case CAPE_RED:
				capeId = 3198;
				break;
			default:
				capeId = 3199;
				break;
			}

			packet.Mantle = capeId;

			strncpy_s(packet.MobName, pMob[clientId].Mobs.Player.MobName, 12);

			AmountMinus(srcItem);
			memset(&pMob[clientId].Mobs.Player.Equip[11], 0, sizeof STRUCT_ITEM);

			//-------------------------
			// Cria o subcele
			unsigned int learn = 0;
			for (int i = 0; i < 8; i++)
			{
				if (24 + i == 30)
					continue;

				int has = (pMob[clientId].Mobs.Player.LearnedSkill[0] & (1 << (24 + i)));
				if (has)
					learn |= (1 << (24 + i));
			}

			learn |= (1 << 30);

			INT32 baseFace = 0;
			if (packet.Face < 10)
				baseFace = 6;
			else if (packet.Face < 20)
				baseFace = 16;
			else if (packet.Face < 30)
				baseFace = 26;
			else if (packet.Face < 40)
				baseFace = 36;

			memset(&pMob[clientId].Mobs.Sub, 0, sizeof STRUCT_SUBINFO);

			pMob[clientId].Mobs.Sub.Equip[0].sIndex = baseFace + packet.Class;
			pMob[clientId].Mobs.Sub.Equip[0].EF2 = baseFace + packet.Class;
			pMob[clientId].Mobs.Sub.Equip[0].EFV2 = SUBCELESTIAL;

			pMob[clientId].Mobs.Sub.Exp = 0;
			pMob[clientId].Mobs.Sub.LearnedSkill = learn;
			pMob[clientId].Mobs.Sub.CurrentScore = 1;

			SendServerNotice("Parabans para %s pela criaaao do seu SubCelestial", pMob[clientId].Mobs.Player.MobName);

			Log(clientId, LOG_INGAME, "Subcelestial criado. Classe %d - Level cele: %d - Reino: %d", packet.Class, pMob[clientId].Mobs.Player.CurrentScore.Level, capeId);

			memset(&pMob[clientId].Mobs.Sub.SkillBar[0], -1, sizeof(char) * 20);
			AmountMinus(srcItem);

			CharLogOut(clientId);
			AddMessageDB((BYTE*)& packet, sizeof p830);

			SendSignalParm(clientId, clientId, 0x3B4, pUser[clientId].inGame.CharSlot);

			SendClientMessage(clientId, "Que Deus abenaoe seu caminho!");
		}
		else
			SendItem(clientId, (SlotType)p->SrcType, p->SrcSlot, srcItem);
	}
#pragma endregion
#pragma region ERVA DE CURA
	else if (_volatile == 243)
	{
		bool any = false;
		for (int i = 0; i < 32; i++)
		{
			INT32 affectId = pMob[clientId].Mobs.Affects[i].Index;
			if (affectId == 1 || affectId == 3 || affectId == 5 || affectId == 7 || affectId == 10 || affectId == 12 || affectId == 20 || affectId == 56)
			{
				memset(&pMob[clientId].Mobs.Affects[i], 0, sizeof STRUCT_AFFECT);

				any = true;
			}
		}

		int heal = pUser[clientId].Potion.CountHp + 25;
		if (heal > pMob[clientId].Mobs.Player.CurrentScore.MaxHp)
			heal = pMob[clientId].Mobs.Player.CurrentScore.MaxHp;

		pUser[clientId].Potion.CountHp = heal;
		pUser[clientId].Potion.bQuaff = 1;

		if (any)
		{
			pMob[clientId].GetCurrentScore(clientId);
			SendScore(clientId);
		}

		SendSetHpMp(clientId);
		AmountMinus(srcItem);
	}
#pragma endregion
#pragma region FEIJaES + REMOVEDOR
	else if (_volatile == 186)
	{
		if (p->DstSlot < 1 || p->DstSlot > 5)
		{
			SendClientMessage(clientId, g_pLanguageString[_NN_Only_To_Equips]);

			SendItem(clientId, (SlotType)p->SrcType, p->SrcSlot, srcItem);
			return true;
		}

		STRUCT_ITEM* dstItem = GetItemPointer(clientId, p->DstType, p->DstSlot);
		if (dstItem == NULL || dstItem->sIndex == 0)
		{
			SendItem(clientId, (SlotType)p->SrcType, p->SrcSlot, srcItem);

			return true;
		}

		bool any = false;
		if (srcItem->sIndex != 3417)
		{
			int efAdd = 116 + (srcItem->sIndex - 3407);
			for (int i = 0; i < 3; i++)
			{
				if (dstItem->stEffect[i].cEffect == EF_SANC)
				{
					dstItem->stEffect[i].cEffect = efAdd;

					any = true;
					break;
				}
			}

			if (!any)
			{
				SendClientMessage(clientId, g_pLanguageString[_NN_HasColor]);

				SendItem(clientId, (SlotType)p->SrcType, p->SrcSlot, srcItem);
				return true;
			}
		}
		else
		{
			for (int i = 0; i < 3; i++)
			{
				if (dstItem->stEffect[i].cEffect >= 116 && dstItem->stEffect[i].cEffect <= 125)
				{
					dstItem->stEffect[i].cEffect = EF_SANC;

					any = true;
					break;
				}
			}

			if (!any)
			{
				SendClientMessage(clientId, g_pLanguageString[_NN_NoColor]);

				SendItem(clientId, (SlotType)p->SrcType, p->SrcSlot, srcItem);
				return true;
			}
		}

		if (any)
		{
			SendClientMessage(clientId, g_pLanguageString[_NN_Refine_Success]);

			AmountMinus(srcItem);

			SendEquip(clientId);
			SendEmotion(clientId, 14, 3);
		}
	}
#pragma endregion
#pragma region CLASSES (REPLATIONS)
		else if (itemId >= 4016 && itemId <= 4020)
		{
			if (p->DstType != (unsigned int)SlotType::Inv)
			{
				SendClientMessage(clientId, g_pLanguageString[_NN_Only_To_Equips]);

				SendItem(clientId, (SlotType)p->SrcType, p->SrcSlot, srcItem);
				return true;
			}

			STRUCT_ITEM* dstItem = GetItemPointer(clientId, p->DstType, p->DstSlot);
			if (dstItem == nullptr || dstItem->sIndex == 0)
			{
				SendItem(clientId, (SlotType)p->SrcType, p->SrcSlot, srcItem);

				return true;
			}

			INT32 itemType = GetEffectValueByIndex(dstItem->sIndex, EF_UNKNOW1);
			INT32 mobType = GetEffectValueByIndex(dstItem->sIndex, EF_MOBTYPE);
			if (itemType != (srcItem->sIndex - 4015) || mobType == 1 || g_pItemList[dstItem->sIndex].Pos > 32)
			{
				SendClientMessage(clientId, g_pLanguageString[_NN_Item_NotMatch]);

				SendItem(clientId, (SlotType)p->SrcType, p->SrcSlot, srcItem);
				return true;
			}

			if (GetItemSanc(dstItem) > 6)
			{
				SendClientMessage(clientId, "Somente em itens menor ou igual a +6.");

				SendItem(clientId, (SlotType)p->SrcType, p->SrcSlot, srcItem);
				return true;
			}

			Log(clientId, LOG_INGAME, "Replation usado em %s %s", g_pItemList[dstItem->sIndex].ItemName, dstItem->toString().c_str());

			if (IsImpossibleToRefine(dstItem))
			{
				for (int i = 0; i < 3; i++)
				{
					dstItem->stEffect[i].cEffect = 0;
					dstItem->stEffect[i].cValue = 0;
				}
			}

			if (dstItem->stEffect[0].cEffect != 43 && (dstItem->stEffect[0].cEffect < 116 || dstItem->stEffect[0].cEffect > 125))
			{
				int sanc = GetItemSanc(dstItem);
				if (sanc != 0)
				{
					if ((dstItem->stEffect[1].cEffect == 43 || (dstItem->stEffect[1].cEffect >= 116 && dstItem->stEffect[1].cEffect <= 125)) ||
						(dstItem->stEffect[2].cEffect == 43 || (dstItem->stEffect[2].cEffect >= 116 && dstItem->stEffect[2].cEffect <= 125)))
					{
						std::swap(dstItem->stEffect[1], dstItem->stEffect[2]);
					}
				}
			}

			for (int i = 1; i < 3; i++)
			{
				dstItem->stEffect[i].cEffect = 0;
				dstItem->stEffect[i].cValue = 0;
			}

			SetItemBonus(dstItem);

			Log(clientId, LOG_INGAME, "Replation gerou o adicional %s", dstItem->toString().c_str());
			AmountMinus(srcItem);

			SendItem(clientId, (SlotType)p->DstType, p->DstSlot, dstItem);
			SendEmotion(clientId, 14, 3);
		}
#pragma endregion
#pragma region BENaaO DE aMAGO
	else if (itemId == 3250)
	{
		auto dstItem = GetItemPointer(clientId, p->DstType, p->DstSlot);
		if (dstItem == nullptr)
		{
			SendItem(clientId, SlotType::Inv, p->SrcSlot, srcItem);
			return true;
		}

		if (dstItem->sIndex < 2400 || dstItem->sIndex > 2418)
		{
			SendClientMessage(clientId, g_pLanguageString[_NN_Mount_Not_Match]);

			SendItem(clientId, SlotType::Inv, p->SrcSlot, srcItem);
			return true;
		}

		int essenceBlessed = 0;
		switch (dstItem->sIndex)
		{
		case 2400:
			essenceBlessed = 3230;
			break;
		case 2405:
			essenceBlessed = 3231;
			break;
		case 2406:
			essenceBlessed = 3232;
			break;
		case 2407:
			essenceBlessed = 3233;
			break;
		case 2409:
			essenceBlessed = 3234;
			break;
		case 2410:
			essenceBlessed = 3235;
			break;
		case 2411:
		case 2412:
		case 2413:
			essenceBlessed = 3236 + (dstItem->sIndex - 2411);
			break;
		case 2414:
		case 2415:
		case 2416:
			essenceBlessed = 3239 + (dstItem->sIndex - 2414);
			break;
		case 2417:
			essenceBlessed = 3242;
			break;
		case 2418:
			essenceBlessed = 3243;
			break;
		}

		if (essenceBlessed == 0)
		{
			SendClientMessage(clientId, g_pLanguageString[_NN_Mount_Not_Match]);

			SendItem(clientId, SlotType::Inv, p->SrcSlot, srcItem);
			return true;
		}

		if (GetItemAmount(dstItem) != 120)
		{
			SendClientMessage(clientId, g_pLanguageString[_NN_Mount_Not_Match]);

			SendItem(clientId, SlotType::Inv, p->SrcSlot, srcItem);
			return true;
		}

		*dstItem = STRUCT_ITEM{};
		dstItem->sIndex = essenceBlessed;

		AmountMinus(srcItem);

		SendItem(clientId, (SlotType)p->DstType, p->DstSlot, dstItem);
		return true;
	}
#pragma endregion
#pragma region BARRA MYTRIL
	else if (_volatile >= 235 && _volatile <= 238)
	{
		static const INT16 g_pAddMyt[4][9] =
		{// MaxAddSet , MaxAddArma, MinAddSet, MinAddArma, SumSet, SumArma, EFFECT
			{42, 81, 0, 0, 6, 9, EF_DAMAGE, EF_DAMAGE2, 0},
			{14, 36,  0, 0, 2, 4, EF_MAGIC, 0, 0},
			{40,  0, 0,  0, 5, 0, EF_AC, EF_ACADD, EF_ACADD2},
			{90,  0, 0,  0, 10, 0, EF_CRITICAL, EF_CRITICAL2, 0}
		};

		STRUCT_ITEM* dstItem = GetItemPointer(clientId, p->DstType, p->DstSlot);

		INT32
			sanc = GetItemSanc(dstItem),
			mobType = GetEffectValueByIndex(dstItem->sIndex, EF_MOBTYPE),
			type = _volatile - 235;

		if (sanc < 11 || mobType != 1)
		{
			SendItem(clientId, (SlotType)p->SrcType, p->SrcSlot, srcItem);

			return true;
		}

		INT32 pos = g_pItemList[dstItem->sIndex].Pos,
			totalAdd = 0;

		int addFound = 0;
		for (INT32 i = 0; i < 3; i++)
		{
			if (dstItem->stEffect[i].cEffect != 0 && dstItem->stEffect[i].cEffect == g_pAddMyt[type][6] || dstItem->stEffect[i].cEffect == g_pAddMyt[type][7] || dstItem->stEffect[i].cEffect == g_pAddMyt[type][8])
			{
				totalAdd += dstItem->stEffect[i].cValue;
				addFound = dstItem->stEffect[i].cEffect;
			}
		}

		if (g_pAddMyt[type][6] == EF_CRITICAL)
		{
			totalAdd += GetEffectValueByIndex(dstItem->sIndex, EF_CRITICAL);

			for (INT32 i = 0; i < 3; i++)
			{
				if (dstItem->stEffect[i].cEffect == EF_CRITICAL2)
				{
					totalAdd = dstItem->stEffect[i].cValue;

					break;
				}
			}
		}
		else if (g_pAddMyt[type][7] == EF_ACADD && addFound != EF_ACADD2)
			totalAdd += GetEffectValueByIndex(dstItem->sIndex, EF_ACADD);

		INT32 maxAdd = g_pAddMyt[type][0],
			minAdd = g_pAddMyt[type][2],
			sumAdd = g_pAddMyt[type][4];

		if (pos > 32)
		{
			maxAdd = g_pAddMyt[type][1];
			minAdd = g_pAddMyt[type][3];
			sumAdd = g_pAddMyt[type][5];
		}

		if (pos == 16 && type == 2)
			maxAdd = 70;

		if (totalAdd <= minAdd || totalAdd >= maxAdd)
		{
			SendItem(clientId, (SlotType)p->SrcType, p->SrcSlot, srcItem);

			return true;
		}

		INT32 _rand = Rand() % 100;
		if (_rand >= 70)
		{
			for (INT32 i = 0; i < 3; i++)
			{
				if (dstItem->stEffect[i].cEffect == g_pAddMyt[type][6] || dstItem->stEffect[i].cEffect == g_pAddMyt[type][7] || dstItem->stEffect[i].cEffect == g_pAddMyt[type][8])
				{
					dstItem->stEffect[i].cValue += sumAdd;
					break;
				}
			}

			Log(clientId, LOG_INGAME, "Adicional de %s [%d] [%d %d %d %d %d %d] aumentou.", g_pItemList[dstItem->sIndex].ItemName, dstItem->sIndex, dstItem->EF1, dstItem->EFV1, dstItem->EF2, dstItem->EFV2, dstItem->EF3, dstItem->EFV3);
		}
		else if (_rand <= 20)
		{
			for (INT32 i = 0; i < 3; i++)
			{
				if (dstItem->stEffect[i].cEffect == g_pAddMyt[type][6] || dstItem->stEffect[i].cEffect == g_pAddMyt[type][7] || dstItem->stEffect[i].cEffect == g_pAddMyt[type][8])
				{
					if (dstItem->stEffect[i].cValue - sumAdd < 0)
						dstItem->stEffect[i].cValue = 0;
					else
						dstItem->stEffect[i].cValue -= sumAdd;

					break;
				}
			}

			Log(clientId, LOG_INGAME, "Adicional de %s [%d] [%d %d %d %d %d %d] reduziu.", g_pItemList[dstItem->sIndex].ItemName, dstItem->sIndex, dstItem->EF1, dstItem->EFV1, dstItem->EF2, dstItem->EFV2, dstItem->EF3, dstItem->EFV3);
		}
		else
			Log(clientId, LOG_INGAME, "Adicional de %s [%d] [%d %d %d %d %d %d] nao aconteceu nada.", g_pItemList[dstItem->sIndex].ItemName, dstItem->sIndex, dstItem->EF1, dstItem->EFV1, dstItem->EF2, dstItem->EFV2, dstItem->EF3, dstItem->EFV3);

		SendItem(clientId, (SlotType)p->DstType, p->DstSlot, dstItem);
		AmountMinus(srcItem);
		return true;
	}
#pragma endregion
	else
	{
#pragma region _BOLSA DO ANDARILHO
		if (itemId == 3467)
		{ // Bolsa do Andarilho
			INT32 slot = -1;
			if (pMob[clientId].Mobs.Player.Inventory[60].sIndex == 0)
				slot = 60;
			else if (pMob[clientId].Mobs.Player.Inventory[61].sIndex == 0)
				slot = 61;
			else if (TimeRemaining(pMob[clientId].Mobs.Player.Inventory[60].EFV1, pMob[clientId].Mobs.Player.Inventory[60].EFV2, pMob[clientId].Mobs.Player.Inventory[60].EFV3 + 1900) <= 0)
				slot = 60;
			else if (TimeRemaining(pMob[clientId].Mobs.Player.Inventory[61].EFV1, pMob[clientId].Mobs.Player.Inventory[61].EFV2, pMob[clientId].Mobs.Player.Inventory[61].EFV3 + 1900) <= 0)
				slot = 61;

			if (slot == -1)
			{
				SendItem(clientId, (SlotType)p->SrcType, p->SrcSlot, srcItem);
				SendClientMessage(clientId, g_pLanguageString[_NN_MaxBag_Actived]);

				return true;
			}

			time_t rawnow = time(NULL);
			struct tm now; localtime_s(&now, &rawnow);

			int month = now.tm_mon; //0 Janeiro, 1 Fev
			int day = now.tm_mday;
			int year = now.tm_year;

			int mes = 0, dia = 0, ano = 0;
			int days = 30;

			mes = month + 1;
			dia = day + days;
			ano = year + 1900;

			if (dia > dias_mes[mes])
			{
				dia -= dias_mes[mes];
				mes += 1;
			}

			if (mes > 12)
			{
				mes -= 12;
				ano += 1;
			}

			pMob[clientId].Mobs.Player.Inventory[slot].sIndex = 3467;
			pMob[clientId].Mobs.Player.Inventory[slot].EF1 = 106;
			pMob[clientId].Mobs.Player.Inventory[slot].EFV1 = dia;
			pMob[clientId].Mobs.Player.Inventory[slot].EF3 = 109;
			pMob[clientId].Mobs.Player.Inventory[slot].EFV3 = (ano - 1900);
			pMob[clientId].Mobs.Player.Inventory[slot].EF2 = 110;
			pMob[clientId].Mobs.Player.Inventory[slot].EFV2 = mes;

			Log(clientId, LOG_INGAME, "Bolsa do Andarilho usado. Ata dia %02d/%02d/%04d", dia, mes, ano);
			LogPlayer(clientId, "Bolsa do Andarilho utilizado durando ata o dia %02d/%02d/%04d", dia, mes, ano);

			AmountMinus(srcItem);
			SendItem(clientId, SlotType::Inv, slot, &pMob[clientId].Mobs.Player.Inventory[slot]);
		}
#pragma endregion
#pragma region CAIXA DE JOIA
		else if (itemId == 4528)
		{
			int slotId = -1;
			bool remove = true;
			if (GetItemAmount(srcItem) == 1)
			{
				slotId = p->SrcSlot;
				remove = false;
			}
			else
				slotId = GetFirstSlot(clientId, 0);

			if (slotId == -1)
			{
				SendClientMessage(clientId, "Sem espaao no inventario");

				SendItem(clientId, SlotType::Inv, p->SrcSlot, srcItem);
				return true;
			}

			STRUCT_ITEM* item = &pMob[clientId].Mobs.Player.Inventory[slotId];
			*item = STRUCT_ITEM{};
			item->sIndex = 2441 + Rand() % 4;
			SendItem(clientId, (SlotType)p->SrcType, slotId, item);

			SendClientMessage(clientId, "!Chegou um item [%s]", g_pItemList[item->sIndex].ItemName);
			Log(clientId, LOG_INGAME, "Abriu Caixa de Joia e recebeu: %s", g_pItemList[item->sIndex].ItemName);

			if (remove)
				AmountMinus(srcItem);
		}
#pragma endregion
#pragma region PEDRA DA FURIA
		else if (itemId == 3020)
		{
			INT32 evId = pMob[clientId].Mobs.Player.Equip[0].EFV2;
			INT32 level = pMob[clientId].Mobs.Player.BaseScore.Level;

			if (evId == SUBCELESTIAL || evId == CELESTIAL)
			{
				if (evId == SUBCELESTIAL && (pMob[clientId].Mobs.Info.Arcana || pMob[clientId].Mobs.Sub.Info.Arcana))
				{
					if (level >= 199)
					{
						if (pMob[clientId].Mobs.Player.Exp < 4000000000)
						{
							SendClientMessage(clientId, g_pLanguageString[_NN_CantTele_Nip]);

							SendItem(clientId, SlotType::Inv, p->SrcSlot, srcItem);
							return true;
						}

						if (pMob[clientId].Mobs.Fame < 1000)
						{
							SendClientMessage(clientId, g_pLanguageString[_NN_Need_XX_Fame], 1000);

							SendItem(clientId, SlotType::Inv, p->SrcSlot, srcItem);
							return true;
						}

						int realSealSlot = GetFirstSlot(clientId, 4683);
						if (realSealSlot == -1)
						{
							SendClientMessage(clientId, "Voca precisa de um Selo Real");

							SendItem(clientId, SlotType::Inv, p->SrcSlot, srcItem);
							return true;
						}

						if (GetInventoryAmount(clientId, 4011) < 5)
						{
							SendClientMessage(clientId, "Voca precisa de 5 bilhaes de gold (em barras)");

							SendItem(clientId, SlotType::Inv, p->SrcSlot, srcItem);
							return true;
						}

						if (pMob[clientId].Mobs.GetTotalResets() >= 3)
						{
							SendClientMessage(clientId, g_pLanguageString[_NN_Youve_Done_It_Already]);

							SendItem(clientId, SlotType::Inv, p->SrcSlot, srcItem);
							return true;
						}

						if (!pMob[clientId].Mobs.Info.Reset_1)
						{
							Log(clientId, LOG_INGAME, "O personagem %s completou o primeiro reset do celestial.", pMob[clientId].Mobs.Player.MobName);
							pMob[clientId].Mobs.Info.Reset_1 = 1;
							pMob[clientId].Mobs.Sub.Info.Reset_1 = 1;
						}
						else if (!pMob[clientId].Mobs.Info.Reset_2)
						{
							Log(clientId, LOG_INGAME, "O personagem %s completou o segundo reset do celestial.", pMob[clientId].Mobs.Player.MobName);
							pMob[clientId].Mobs.Info.Reset_2 = 1;
							pMob[clientId].Mobs.Sub.Info.Reset_2 = 1;
						}
						else if (!pMob[clientId].Mobs.Info.Reset_3)
						{
							Log(clientId, LOG_INGAME, "O personagem %s completou o terceiro reset do celestial.", pMob[clientId].Mobs.Player.MobName);
							pMob[clientId].Mobs.Info.Reset_3 = 1;
							pMob[clientId].Mobs.Sub.Info.Reset_3 = 1;
						}
						else
						{
							SendClientMessage(clientId, g_pLanguageString[_NN_Youve_Done_It_Already]);

							SendItem(clientId, SlotType::Inv, p->SrcSlot, srcItem);
							return true;
						}

						RemoveAmount(clientId, 4011, 5);
						pMob[clientId].Mobs.Fame -= 1000;
						AmountMinus(srcItem);
						AmountMinus(&pMob[clientId].Mobs.Player.Inventory[realSealSlot]);
						SendItem(clientId, SlotType::Inv, realSealSlot, &pMob[clientId].Mobs.Player.Inventory[realSealSlot]);

						pMob[clientId].Mobs.Player.Exp -= 200000000LL;

						while (pMob[clientId].Mobs.Player.BaseScore.Level > 0 && pMob[clientId].Mobs.Player.Exp < g_pNextLevel[pMob[clientId].Mobs.Player.Equip[0].EFV2][level - 1])
						{
							pMob[clientId].Mobs.Player.BaseScore.Level--;
							level--;
						}

						pMob[clientId].GetCurrentScore(clientId);
						SendScore(clientId);
						SendEtc(clientId);

						SendClientMessage(clientId, g_pLanguageString[_NN_Success_Comp]);
						return true;
					}
				}
				if (level == 199 && pMob[clientId].Mobs.Player.Equip[1].sIndex == 3502)
				{
					if (pMob[clientId].Mobs.Info.Arcana || pMob[clientId].Mobs.Sub.Info.Arcana)
					{
						SendClientMessage(clientId, g_pLanguageString[_NN_Youve_Done_It_Already]);

						SendItem(clientId, SlotType::Inv, p->SrcSlot, srcItem);
						return true;
					}

					if (pMob[clientId].Mobs.Fame < 500)
					{
						SendClientMessage(clientId, g_pLanguageString[_NN_Need_XX_Fame], 500);

						SendItem(clientId, SlotType::Inv, p->SrcSlot, srcItem);
						return true;
					}

					if (pMob[clientId].Mobs.Player.Equip[1].sIndex == 3502)
					{
						int szStones[4] = { 0, 0, 0, 0 };
						for (int i = 0; i < 4; i++)
						{
							int Slot = GetFirstSlot(clientId, 5334 + i);
							if (Slot == -1)
							{
								SendItem(clientId, SlotType::Inv, p->SrcSlot, srcItem);

								return true;
							}

							szStones[i] = Slot;
						}

						for (int i = 0; i < 4; i++)
						{
							memset(&pMob[clientId].Mobs.Player.Inventory[szStones[i]], 0, sizeof STRUCT_ITEM);

							SendItem(clientId, SlotType::Inv, szStones[i], &pMob[clientId].Mobs.Player.Inventory[szStones[i]]);
						}

						// Retirar a fame
						pMob[clientId].Mobs.Fame -= 500;

						AmountMinus(srcItem);

						int _rand = Rand() % 100;
						int _add = Rand() % 50;
						if (_rand + _add > 100)
							_rand += _add - 50;

						if (_rand < 50)
						{
							pMob[clientId].Mobs.Player.Equip[1].sIndex = 3507;

							SendItem(clientId, SlotType::Equip, 1, &pMob[clientId].Mobs.Player.Equip[1]);
							SendClientMessage(clientId, g_pLanguageString[_NN_Success_Comp]);

							pMob[clientId].Mobs.Info.Arcana = 1;
							pMob[clientId].Mobs.Sub.Info.Arcana = 1;

							Log(clientId, LOG_INGAME, "Sucesso ao criar Cythera Arcana");
							SendNotice(".O jogador %s concluiu com sucesso a criaaaoo da Cythera Arcana", pMob[clientId].Mobs.Player.MobName);
						}
						else
						{
							SendClientMessage(clientId, g_pLanguageString[_NN_CombineFailed]);

							Log(clientId, LOG_INGAME, "Falha ao criar Cythera Arcana");
							SendNotice(".O jogador %s falhou a criaaao da Cythera Arcana", pMob[clientId].Mobs.Player.MobName);
						}
					}
					else if (level == 199 && evId == 4)
					{
						SendClientMessage(clientId, "Desativado...");

						SendItem(clientId, SlotType::Inv, p->SrcSlot, srcItem);
						return true;
					}
				}
			}

			if (evId != CELESTIAL || level != 89 || !pMob[clientId].Mobs.Info.LvBlocked || pMob[clientId].Mobs.Info.Unlock89)
			{
				SendItem(clientId, SlotType::Inv, p->SrcSlot, srcItem);

				return true;
			}

			if (pMob[clientId].Mobs.Fame < 500)
			{
				SendClientMessage(clientId, g_pLanguageString[_NN_Need_XX_Fame], 500);

				SendItem(clientId, SlotType::Inv, p->SrcSlot, srcItem);
				return true;
			}

			int _rand = Rand() % 101;
			pMob[clientId].Mobs.Fame -= 500;

			if (_rand <= 95)
			{
				SendClientMessage(clientId, g_pLanguageString[_NN_Success_Comp]);

				SendEmotion(clientId, 14, 3);

				INT32 slotId = GetFirstSlot(clientId, 0);
				if (slotId == -1)
				{
					SendClientMessage(clientId, g_pLanguageString[_NN_NoSpace_Item], g_pItemList[3502].ItemName);

					Log(clientId, LOG_INGAME, "Desbloqueio 90: Sem espaao para receber cythera");
				}
				else
				{
					Log(clientId, LOG_INGAME, "Desbloqueio 90:  recebeu cythera mastica");

					// Apaga o item
					memset(&pMob[clientId].Mobs.Player.Inventory[slotId], 0, sizeof STRUCT_ITEM);

					// Atualiza o inventario
					pMob[clientId].Mobs.Player.Inventory[slotId].sIndex = 3502;

					SendItem(clientId, SlotType::Inv, slotId, &pMob[clientId].Mobs.Player.Inventory[slotId]);
				}

				pMob[clientId].Mobs.Info.LvBlocked = 0;
				pMob[clientId].Mobs.Info.Unlock89 = 1;
			}
			else
			{
				SendClientMessage(clientId, "Houve falha no destravamento do navel %d/95", _rand);

				Log(clientId, LOG_INGAME, "Desbloqueio 90: FALHA");
			}

			AmountMinus(srcItem);
		}
#pragma endregion
#pragma region PEDIDO DE CAaA
		else if (itemId >= 3432 && itemId <= 3437)
		{
			INT32 warp = p->warp;
			if (warp > 10)
				return false;

			if (warp - 1 < 0)
				return false;

			warp--;

			int ID = itemId - 3432;
			int tX = HuntingScrolls[ID][warp][0];
			int tY = HuntingScrolls[ID][warp][1];

			AmountMinus(srcItem); // Removes 1 if the item has unities, deletes it if not.
			Teleportar(clientId, tX, tY);
		}
#pragma endregion
#pragma region PERGAMINHO DO PERDaO
		else if (itemId == 3343)
		{
			if (GetPKPoint(clientId) - 75 > 60)
			{
				SendClientMessage(clientId, g_pLanguageString[_NN_CantReset_ChaosPoint]);
				SendItem(clientId, (SlotType)p->SrcType, p->SrcSlot, srcItem);

				return true;
			}

			Log(clientId, LOG_INGAME, "Perdao utilizado. PK antigo: %d.", GetPKPoint(clientId) - 75);

			SetPKPoint(clientId, GetPKPoint(clientId) + 75);

			SendClientMessage(clientId, g_pLanguageString[_NN_CPPoint], GetPKPoint(clientId) - 75);

			p364 packet;
			GetCreateMob(clientId, (BYTE*)& packet);

			GridMulticast_2(pMob[clientId].Target.X, pMob[clientId].Target.Y, (BYTE*)& packet, 0);

			AmountMinus(srcItem);
		}
#pragma endregion
#pragma region FOGOS DE ARTIFICIO
		else if (itemId == 1728)
		{
			SendEmotion(clientId, 100, Rand() % 8);

			AmountMinus(srcItem);
			return true;
		}
#pragma endregion
#pragma region BAa DE EXPERIaNCIA
		else if (itemId == 4140 || itemId == 4548 || itemId == 4549)
		{
			INT32 slot = -1;
			for (INT32 i = 0; i < 32; i++)
			{
				if (pMob[clientId].Mobs.Affects[i].Index == 39)
				{
					slot = i;

					break;
				}
			}

			if (slot == -1)
			{
				for (INT32 i = 0; i < 32; i++)
				{
					if (pMob[clientId].Mobs.Affects[i].Index == 0)
					{
						slot = i;

						break;
					}
				}
			}

			if (slot == -1)
			{
				SendItem(clientId, (SlotType)p->SrcType, p->SrcSlot, srcItem);

				return true;
			}

			INT32 time = 900;
			if (itemId == 4548)
				time = 225;
			else if (itemId == 4549)
				time = 450;

			pMob[clientId].Mobs.Affects[slot].Index = 39;

			if (pMob[clientId].Mobs.Affects[slot].Time > 0)
				pMob[clientId].Mobs.Affects[slot].Time += time;
			else
				pMob[clientId].Mobs.Affects[slot].Time = time;

			SendAffect(clientId);

			p364 packet;
			GetCreateMob(clientId, (BYTE*)& packet);

			GridMulticast_2(pMob[clientId].Target.X, pMob[clientId].Target.Y, (BYTE*)& packet, 0);

			AmountMinus(srcItem);
			return true;
		}
#pragma endregion
#pragma region CHOCOLATE DO AMOR
		else if (itemId == 1739)
		{
			SetAffect(clientId, 41, 500, 125);
			SetAffect(clientId, 43, 500, 125);
			SetAffect(clientId, 44, 500, 125);
			SetAffect(clientId, 45, 500, 125);
			SendAffect(clientId);

			AmountMinus(srcItem);
		}
#pragma endregion
#pragma region ESCRITURAS
		else if (itemId == 3445 || itemId == 3446)
		{
			INT32 searched = 412 + (itemId - 3445);
			INT32 totalAmount = GetInventoryAmount(clientId, searched);

			if (totalAmount > 10)
				totalAmount = 10;

			if (totalAmount == 0)
			{
				SendClientMessage(clientId, "Nao ha %s para juntar", g_pItemList[searched].ItemName);

				SendItem(clientId, (SlotType)p->SrcType, p->SrcSlot, srcItem);
				return true;
			}

			int slotId = GetFirstSlot(clientId, 0);
			if (slotId == -1)
			{
				SendClientMessage(clientId, g_pLanguageString[_NN_CarryFull]);

				SendItem(clientId, (SlotType)p->SrcType, p->SrcSlot, srcItem);
				return true;
			}

			INT32 total = totalAmount;
			for (INT32 i = 0; i < 60; i++)
			{
				if (pMob[clientId].Mobs.Player.Inventory[i].sIndex == searched)
				{
					while (pMob[clientId].Mobs.Player.Inventory[i].sIndex == searched)
					{
						AmountMinus(&pMob[clientId].Mobs.Player.Inventory[i]);

						totalAmount--;
						if (totalAmount <= 0)
							break;
					}
				}

				if (totalAmount <= 0)
					break;
			}

			STRUCT_ITEM* item = &pMob[clientId].Mobs.Player.Inventory[slotId];
			item->sIndex = 3447 + (itemId - 3445);

			item->EF1 = EF_AMOUNT;
			item->EFV1 = total;

			AmountMinus(srcItem);
			SendCarry(clientId);
		}

#pragma endregion
#pragma region FRANGO E POaaES DA CORAGEM
		else if (itemId == 3314 || itemId == 646 || itemId == 647)
		{
			int index = 0;
			int time = 0;
			if (itemId == 3314)
			{
				index = 1;
				time = 1800;
			}
			else if (itemId == 646 || itemId == 647)
			{
				index = 2 + (itemId - 646);

				time = 450;
			}

			INT32 slot = -1;
			for (INT32 i = 0; i < 32; i++)
			{
				if (pMob[clientId].Mobs.Affects[i].Index == 30)
				{
					slot = i;

					break;
				}
			}

			if (slot == -1)
			{
				for (INT32 i = 0; i < 32; i++)
				{
					if (pMob[clientId].Mobs.Affects[i].Index == 0)
					{
						slot = i;

						break;
					}
				}
			}

			if (slot == -1)
			{
				SendItem(clientId, (SlotType)p->SrcType, p->SrcSlot, srcItem);

				return true;
			}

			pMob[clientId].Mobs.Affects[slot].Index = 30;
			pMob[clientId].Mobs.Affects[slot].Time = time;
			pMob[clientId].Mobs.Affects[slot].Value = index;

			SendAffect(clientId);

			p364 packet;
			GetCreateMob(clientId, (BYTE*)& packet);

			GridMulticast_2(pMob[clientId].Target.X, pMob[clientId].Target.Y, (BYTE*)& packet, 0);

			AmountMinus(srcItem);
		}
#pragma endregion
#pragma region BOOKS SEPHIRA
		else if (itemId >= 667 && itemId <= 671)
		{
			// SkillPoint necessario
			const WORD BooksSkill[5] = { 0, 36, 35, 40, 39 };

			int _index = srcItem->sIndex - 666;

			if (_index <= 0 || _index > 5)
				return true;

			int has = (pMob[clientId].Mobs.Player.LearnedSkill[0] & (1 << (24 + _index)));
			if (has)
			{
				SendClientMessage(clientId, "Voca ja aprendeu esta skill");

				SendItem(clientId, (SlotType)p->SrcType, p->SrcSlot, srcItem);
				return true;
			}

			if (pMob[clientId].Mobs.Player.SkillPoint < BooksSkill[_index - 1])
			{
				SendClientMessage(clientId, "Voca nao tem pontos de Skill suficiente");

				SendItem(clientId, (SlotType)p->SrcType, p->SrcSlot, srcItem);
				return true;
			}

			// Checagem da classe do personagem : <= arch , celestiais podem adquirir em qualquer level
			if (pMob[clientId].Mobs.Player.Equip[0].EFV2 == MORTAL)
			{
				if (pMob[clientId].Mobs.Player.BaseScore.Level < 255)
				{
					SendClientMessage(clientId, "Level inadequado");

					SendItem(clientId, (SlotType)p->SrcType, p->SrcSlot, srcItem);
					return true;
				}
			}

			Log(clientId, LOG_INGAME, "Aprendido skill %s", g_pItemList[srcItem->sIndex].ItemName);
			LogPlayer(clientId, "Aprendeu a skill sephira %s", g_pItemList[srcItem->sIndex].ItemName);

			pMob[clientId].Mobs.Player.SkillPoint -= BooksSkill[_index - 1];
			pMob[clientId].Mobs.Player.LearnedSkill[0] |= (1 << (_index + 24));
			pMob[clientId].GetCurrentScore(clientId);

			SendScore(clientId);
			SendEtc(clientId);

			AmountMinus(srcItem);
		}
#pragma endregion
#pragma region POaaO REVIGORANTE XD
		else if (itemId >= 3210 && itemId <= 3213)
		{
			int days = 7;
			switch (itemId)
			{
			case 3210:
				days = 1;
				break;
			case 3211:
				days = 3;
				break;
			case 3212:
				days = 7;
				break;
			case 3213:
				days = 30;
				break;
			}

			INT32 slot = -1;
			for (INT32 i = 0; i < 32; i++)
			{
				if (pMob[clientId].Mobs.Affects[i].Index == 51)
				{
					slot = i;

					break;
				}
			}

			if (slot == -1)
			{
				for (INT32 i = 0; i < 32; i++)
				{
					if (pMob[clientId].Mobs.Affects[i].Index == 0)
					{
						slot = i;

						break;
					}
				}
			}

			if (slot == -1)
			{
				SendItem(clientId, (SlotType)p->SrcType, p->SrcSlot, srcItem);

				return true;
			}

			time_t rawnow = time(NULL);
			struct tm now; localtime_s(&now, &rawnow);

			int month = now.tm_mon + 1; //0 Janeiro, 1 Fev
			int day = now.tm_mday + days;
			int year = now.tm_year + 1900;
			int hour = now.tm_hour;
			int min = now.tm_min;
			int sec = now.tm_sec;

			if (day > dias_mes[month])
			{
				day -= dias_mes[month];
				month += 1;
			}

			if (month > 12)
			{
				month -= 12;
				year += 1;
			}

			pMob[clientId].Mobs.Revigorante.Dia = day;
			pMob[clientId].Mobs.Revigorante.Mes = month;
			pMob[clientId].Mobs.Revigorante.Ano = year;
			pMob[clientId].Mobs.Revigorante.Hora = hour;
			pMob[clientId].Mobs.Revigorante.Minuto = min;
			pMob[clientId].Mobs.Revigorante.Segundo = sec;


			pMob[clientId].Mobs.Affects[slot].Index = 51;
			pMob[clientId].Mobs.Affects[slot].Time = 99;
			pMob[clientId].Mobs.Affects[slot].Master = 1;

			p364 packet;
			GetCreateMob(clientId, (BYTE*)& packet);

			GridMulticast_2(pMob[clientId].Target.X, pMob[clientId].Target.Y, (BYTE*)& packet, 0);

			SendAffect(clientId);
			AmountMinus(srcItem);

			Log(clientId, LOG_INGAME, "Poaao Revigorante utilizada ata dia %02d/%02d/%02d %02d:%02d:%02d", day, month, year + 1900, hour, min, sec);
		}
#pragma endregion
#pragma region POaaO DIVINA
		else if (itemId >= 3379 && itemId <= 3381)
		{
			int days = 7;
			switch (itemId)
			{
			case 3379:
				days = 7;
				break;
			case 3380:
				days = 15;
				break;
			case 3381:
				days = 30;
				break;
			}

			INT32 slot = -1;
			for (INT32 i = 0; i < 32; i++)
			{
				if (pMob[clientId].Mobs.Affects[i].Index == 34)
				{
					slot = i;

					break;
				}
			}

			if (slot == -1)
			{
				for (INT32 i = 0; i < 32; i++)
				{
					if (pMob[clientId].Mobs.Affects[i].Index == 0)
					{
						slot = i;

						break;
					}
				}
			}

			if (slot == -1)
			{
				SendItem(clientId, (SlotType)p->SrcType, p->SrcSlot, srcItem);

				return true;
			}

			time_t rawnow = time(NULL);
			struct tm now; localtime_s(&now, &rawnow);

			int month = now.tm_mon + 1; //0 Janeiro, 1 Fev
			int day = now.tm_mday + days;
			int year = now.tm_year + 1900;
			int hour = now.tm_hour;
			int min = now.tm_min;
			int sec = now.tm_sec;

			if (day > dias_mes[month])
			{
				day -= dias_mes[month];
				month += 1;
			}

			if (month > 12)
			{
				month -= 12;
				year += 1;
			}

			pUser[clientId].User.Divina.Dia = day;
			pUser[clientId].User.Divina.Mes = month;
			pUser[clientId].User.Divina.Ano = year;
			pUser[clientId].User.Divina.Hora = hour;
			pUser[clientId].User.Divina.Minuto = min;
			pUser[clientId].User.Divina.Segundo = sec;

			pMob[clientId].Mobs.Affects[slot].Index = 34;
			pMob[clientId].Mobs.Affects[slot].Time = 99;
			pMob[clientId].Mobs.Affects[slot].Master = 1;

			p364 packet;
			GetCreateMob(clientId, (BYTE*)& packet);

			GridMulticast_2(pMob[clientId].Target.X, pMob[clientId].Target.Y, (BYTE*)& packet, 0);

			SendAffect(clientId);
			AmountMinus(srcItem);

			Log(clientId, LOG_INGAME, "Poaao Divina utilizada ata dia %02d/%02d/%02d %02d:%02d:%02d", day, month, year, hour, min, sec);
		}
#pragma endregion
#pragma region POaaO SEPHIRA
		else if (itemId >= 3361 && itemId <= 3363)
		{
			int days = 7;
			switch (itemId)
			{
			case 3361:
				days = 7;
				break;
			case 3362:
				days = 15;
				break;
			case 3363:
				days = 30;
				break;
			}

			INT32 slot = -1;
			for (INT32 i = 0; i < 32; i++)
			{
				if (pMob[clientId].Mobs.Affects[i].Index == 4)
				{
					slot = i;

					break;
				}
			}

			if (slot == -1)
			{
				for (INT32 i = 0; i < 32; i++)
				{
					if (pMob[clientId].Mobs.Affects[i].Index == 0)
					{
						slot = i;

						break;
					}
				}
			}

			if (slot == -1)
			{
				SendItem(clientId, (SlotType)p->SrcType, p->SrcSlot, srcItem);

				return true;
			}

			time_t rawnow = time(NULL);
			struct tm now; localtime_s(&now, &rawnow);

			int month = now.tm_mon + 1; //0 Janeiro, 1 Fev
			int day = now.tm_mday + days;
			int year = now.tm_year + 1900;
			int hour = now.tm_hour;
			int min = now.tm_min;
			int sec = now.tm_sec;

			if (day > dias_mes[month])
			{
				day -= dias_mes[month];
				month += 1;
			}

			if (month > 12)
			{
				month -= 12;
				year += 1;
			}

			pUser[clientId].User.Sephira.Dia = day;
			pUser[clientId].User.Sephira.Mes = month;
			pUser[clientId].User.Sephira.Ano = year;
			pUser[clientId].User.Sephira.Hora = hour;
			pUser[clientId].User.Sephira.Minuto = min;
			pUser[clientId].User.Sephira.Segundo = sec;

			pMob[clientId].Mobs.Affects[slot].Index = 4;
			pMob[clientId].Mobs.Affects[slot].Master = 1;
			pMob[clientId].Mobs.Affects[slot].Value = 4; // Poaao Sephira = 4
			pMob[clientId].Mobs.Affects[slot].Time = 999;

			p364 packet;
			GetCreateMob(clientId, (BYTE*)& packet);

			GridMulticast_2(pMob[clientId].Target.X, pMob[clientId].Target.Y, (BYTE*)& packet, 0);

			SendAffect(clientId);

			AmountMinus(srcItem);
			Log(clientId, LOG_INGAME, "Poaao Sephira utilizada ata dia %02d/%02d/%02d %02d:%02d:%02d", day, month, year + 1900, hour, min, sec);
		}
#pragma endregion
#pragma region POaaO SAaDE
		else if (itemId >= 3364 && itemId <= 3366)
		{
			INT32 slot = -1;
			for (INT32 i = 0; i < 32; i++)
			{
				if (pMob[clientId].Mobs.Affects[i].Index == 35)
				{
					slot = i;

					break;
				}
			}

			if (slot == -1)
			{
				for (INT32 i = 0; i < 32; i++)
				{
					if (pMob[clientId].Mobs.Affects[i].Index == 0)
					{
						slot = i;

						break;
					}
				}
			}

			if (slot == -1)
			{
				SendItem(clientId, (SlotType)p->SrcType, p->SrcSlot, srcItem);

				return true;
			}

			int days = 7;
			switch (itemId)
			{
			case 3364:
				days = 7;
				break;
			case 3365:
				days = 15;
				break;
			case 3366:
				days = 30;
				break;
			}

			time_t rawnow = time(NULL);
			struct tm now; localtime_s(&now, &rawnow);

			int month = now.tm_mon + 1; //0 Janeiro, 1 Fev
			int day = now.tm_mday + days;
			int year = now.tm_year + 1900;
			int hour = now.tm_hour;
			int min = now.tm_min;
			int sec = now.tm_sec;

			if (day > dias_mes[month])
			{
				day -= dias_mes[month];
				month += 1;
			}

			if (month > 12)
			{
				month -= 12;
				year += 1;
			}

			pMob[clientId].Mobs.Saude.Dia = day;
			pMob[clientId].Mobs.Saude.Mes = month;
			pMob[clientId].Mobs.Saude.Ano = year;
			pMob[clientId].Mobs.Saude.Hora = hour;
			pMob[clientId].Mobs.Saude.Minuto = min;
			pMob[clientId].Mobs.Saude.Segundo = sec;

			pMob[clientId].Mobs.Affects[slot].Index = 35;
			pMob[clientId].Mobs.Affects[slot].Master = 1;
			pMob[clientId].Mobs.Affects[slot].Value = 1; // Poaao Saade = 1 . Poaao Vigor = 2
			pMob[clientId].Mobs.Affects[slot].Time = 999;

			p364 packet;
			GetCreateMob(clientId, (BYTE*)& packet);

			GridMulticast_2(pMob[clientId].Target.X, pMob[clientId].Target.Y, (BYTE*)& packet, 0);

			SendAffect(clientId);

			AmountMinus(srcItem);
			Log(clientId, LOG_INGAME, "Poaao Saade utilizada ata dia %02d/%02d/%02d %02d:%02d:%02d", day, month, year + 1900, hour, min, sec);
		}
#pragma endregion
#pragma region ESCRITURA DE PESADELO
		else if (itemId == 5137)
		{
			if (pMob[clientId].Mobs.Player.Equip[0].EFV2 <= ARCH)
			{
				SendClientMessage(clientId, g_pLanguageString[_NN_Need_CelestialSub]);

				SendItem(clientId, (SlotType)p->SrcType, p->SrcSlot, srcItem);
				return true;
			}

			time_t rawnow = time(NULL);
			struct tm now; localtime_s(&now, &rawnow);

			int month = now.tm_mon; //0 Janeiro, 1 Fev
			int day = now.tm_mday;
			int year = now.tm_year;

			auto now_time_t = std::mktime(&now);
			auto diffTime = std::difftime(now_time_t, pMob[clientId].Mobs.Escritura.GetTMStruct());
			if (pMob[clientId].Mobs.Escritura.Ano == 0)
				diffTime = ScrollWaitTime;

			if (diffTime < ScrollWaitTime)
			{
				int totalSeconds = (ScrollWaitTime - (int)diffTime);
				int hours = (totalSeconds / 3600) % 24;
				int mins = (totalSeconds % 3600) / 60;
				int seconds = totalSeconds % 60;

				SendClientMessage(clientId, "Voca precisa aguardar %02d horas %02d minutos %02d segundos ata o praximo uso", hours, mins, seconds);

				SendItem(clientId, (SlotType)p->SrcType, p->SrcSlot, srcItem);
				return true;
			}

			INT32 value = pMob[clientId].Mobs.PesaEnter;
			if (value >= 36)
			{
				SendClientMessage(clientId, g_pLanguageString[_NN_Max36_Enter]);

				SendItem(clientId, (SlotType)p->SrcType, p->SrcSlot, srcItem);
				return true;
			}

			month = now.tm_mon + 1; //0 Janeiro, 1 Fev
			day = now.tm_mday;
			year = now.tm_year + 1900;
			int hour = now.tm_hour;
			int min = now.tm_min;
			int sec = now.tm_sec;

			pMob[clientId].Mobs.Escritura.Ano = year;
			pMob[clientId].Mobs.Escritura.Mes = month;
			pMob[clientId].Mobs.Escritura.Dia = day;
			pMob[clientId].Mobs.Escritura.Hora = hour;
			pMob[clientId].Mobs.Escritura.Minuto = min;
			pMob[clientId].Mobs.Escritura.Segundo = sec;

			value += 12;
			if (value > 36)
				value = 36;

			pMob[clientId].Mobs.PesaEnter = value;

			AmountMinus(srcItem);

			Log(clientId, LOG_INGAME, "Escritura de Pesadelo utilizado e tempo atribuado");
		}
#pragma endregion
#pragma region BARRAS GOLD
		else if (itemId == 4010 || itemId == 4011 || (itemId >= 4026 && itemId <= 4029))
		{
			if (!GoldBar(clientId, (SlotType)p->SrcType, p->SrcSlot, srcItem))
				SendItem(clientId, (SlotType)p->SrcType, p->SrcSlot, srcItem);
		}
#pragma endregion
#pragma region GEMA ANCT
		else if (itemId >= 3386 && itemId <= 3389)
		{
			STRUCT_ITEMLIST itemData = g_pItemList[pMob[clientId].Mobs.Player.Equip[p->DstSlot].sIndex];

			if (p->DstType != (unsigned int)SlotType::Equip)
			{
				SendItem(clientId, (SlotType)p->SrcType, p->SrcSlot, srcItem);

				return true;
			}

			if (itemData.Pos == 0)
			{
				SendItem(clientId, (SlotType)p->SrcType, p->SrcSlot, srcItem);

				return true;
			}

			if (itemData.Pos >= 64 && itemData.Pos <= 2048)
			{
				if ((itemData.nGrade < 5 || itemData.nGrade > 8) && GetItemSanc(&pMob[clientId].Mobs.Player.Equip[p->DstSlot]) < 10)
				{
					SendItem(clientId, (SlotType)p->SrcType, p->SrcSlot, srcItem);

					return true; // O item nao a ancient
				}
			}

			INT32 actual = 0,
				stone = pMob[clientId].Mobs.Player.Inventory[p->SrcSlot].sIndex - 3386;

			if (itemData.Pos <= 32)
			{
				INT32 sanctmp = GetItemSanc(&pMob[clientId].Mobs.Player.Equip[p->DstSlot]);
				if (GetItemSanc(&pMob[clientId].Mobs.Player.Equip[p->DstSlot]) < 10)
				{
					SendItem(clientId, (SlotType)p->SrcType, p->SrcSlot, srcItem);

					return true; // Set +9 ou menor ._.
				}

				INT32 value = -1;
				for (INT32 t = 0; t < 3; t++)
				{
					if (pMob[clientId].Mobs.Player.Equip[p->DstSlot].stEffect[t].cEffect == 43 || (pMob[clientId].Mobs.Player.Equip[p->DstSlot].stEffect[t].cEffect >= 116 && pMob[clientId].Mobs.Player.Equip[p->DstSlot].stEffect[t].cEffect <= 125))
					{
						value = pMob[clientId].Mobs.Player.Equip[p->DstSlot].stEffect[t].cValue;
						break;
					}
				}

				value = (value - 230) % 4;
				if (value == stone)
				{
					SendItem(clientId, (SlotType)p->SrcType, p->SrcSlot, srcItem);

					return true;
				}
			}
			else
			{
				actual = itemData.nGrade - 5;

				// Checa se os dois nao sao os mesmos -> GARNET PARA GARNET
				if (stone == actual)
				{
					SendItem(clientId, (SlotType)p->SrcType, p->SrcSlot, srcItem);

					return true;
				}
			}

			STRUCT_ITEM* dstItem = GetItemPointer(clientId, p->DstType, p->DstSlot);
			if (!dstItem)
			{
				SendItem(clientId, (SlotType)p->SrcType, p->SrcSlot, srcItem);

				return true;
			}

			// Para armas
			if (itemData.Pos >= 64 && itemData.Pos <= 2048)
			{
				for (int i = 0; i < 3; i++)
				{
					if (pMob[clientId].Mobs.Player.Equip[p->DstSlot].stEffect[i].cEffect == 43 || (pMob[clientId].Mobs.Player.Equip[p->DstSlot].stEffect[i].cEffect >= 116 && pMob[clientId].Mobs.Player.Equip[p->DstSlot].stEffect[i].cEffect <= 125))
					{
						actual = pMob[clientId].Mobs.Player.Equip[p->DstSlot].stEffect[i].cValue;

						break;
					}
				}

				if (actual > 9)
				{
					int sanc = actual;
					// Pega o valor da refinaaao base 
					if (sanc < 234)
						sanc = 230;
					else if (sanc < 238)
						sanc = 234;
					else if (sanc < 242)
						sanc = 238;
					else if (sanc < 246)
						sanc = 242;
					else if (sanc < 250)
						sanc = 246;
					else
						sanc = 250;

					actual = actual - sanc;

					for (int i = 0; i < 3; i++)
					{
						if (pMob[clientId].Mobs.Player.Equip[p->DstSlot].stEffect[i].cEffect == 43 || (pMob[clientId].Mobs.Player.Equip[p->DstSlot].stEffect[i].cEffect >= 116 && pMob[clientId].Mobs.Player.Equip[p->DstSlot].stEffect[i].cEffect <= 125))
						{
							pMob[clientId].Mobs.Player.Equip[p->DstSlot].stEffect[i].cValue += (stone - actual);

							break;
						}
					}
				}

				if (itemData.Pos != 128 && (itemData.nGrade >= 5 && itemData.nGrade <= 8))
				{
					stone = pMob[clientId].Mobs.Player.Inventory[p->SrcSlot].sIndex - 3386;

					actual = itemData.nGrade - 5;
					pMob[clientId].Mobs.Player.Equip[p->DstSlot].sIndex += (stone - actual);
				}
			}
			else if (itemData.Pos <= 32) {
				// Pega a refinaaao atual do item
				for (int i = 0; i < 3; i++)
				{
					if (pMob[clientId].Mobs.Player.Equip[p->DstSlot].stEffect[i].cEffect == 43 || (pMob[clientId].Mobs.Player.Equip[p->DstSlot].stEffect[i].cEffect >= 116 && pMob[clientId].Mobs.Player.Equip[p->DstSlot].stEffect[i].cEffect <= 125))
					{
						actual = pMob[clientId].Mobs.Player.Equip[p->DstSlot].stEffect[i].cValue;

						break;
					}
				}

				int sanc = actual;
				// Pega o valor da refinaaao base 
				if (sanc < 234)
					sanc = 230;
				else if (sanc < 238)
					sanc = 234;
				else if (sanc < 242)
					sanc = 238;
				else if (sanc < 246)
					sanc = 242;
				else if (sanc < 250)
					sanc = 246;
				else
					sanc = 250;

				actual = actual - sanc;
				for (int i = 0; i < 3; i++)
				{
					if (pMob[clientId].Mobs.Player.Equip[p->DstSlot].stEffect[i].cEffect == 43 || (pMob[clientId].Mobs.Player.Equip[p->DstSlot].stEffect[i].cEffect >= 116 && pMob[clientId].Mobs.Player.Equip[p->DstSlot].stEffect[i].cEffect <= 125))
					{
						pMob[clientId].Mobs.Player.Equip[p->DstSlot].stEffect[i].cValue += (stone - actual);

						break;
					}
				}
			}
			else
			{
				SendItem(clientId, (SlotType)p->SrcType, p->SrcSlot, srcItem);

				return true;
			}

			SendItem(clientId, (SlotType)p->DstType, p->DstSlot, &pMob[clientId].Mobs.Player.Equip[p->DstSlot]);
			AmountMinus(srcItem);

			Log(clientId, LOG_INGAME, "Usado Gema no item %s [%d] [%d %d %d %d %d %d]", g_pItemList[dstItem->sIndex].ItemName, dstItem->sIndex,
				dstItem->EF1, dstItem->EFV1, dstItem->EF2, dstItem->EFV2, dstItem->EF3, dstItem->EFV3);
		}
#pragma endregion
#pragma region LANHOUSE (MASMORRA)
		else if (itemId == 4111)
		{
			if (GetVillage(pMob[clientId].Target.X, pMob[clientId].Target.Y) == 5)
			{
				SendClientMessage(clientId, "Somente utilize em uma cidade!");
				SendItem(clientId, (SlotType)p->SrcType, p->SrcSlot, srcItem);

				return true;
			}

			if (pMob[clientId].Mobs.Player.Equip[0].EFV2 != MORTAL)
			{
				SendClientMessage(clientId, "Disponavel apenas para mortais");

				SendItem(clientId, (SlotType)p->SrcType, p->SrcSlot, srcItem);
				return true;
			}

			INT32 total = 0;
			for (INT32 i = 1; i < MAX_PLAYER; i++)
			{
				if (pUser[i].CurrentScore != USER_PLAY)
					continue;

				if ((pMob[i].Target.X >= 3732 && pMob[i].Target.X <= 3816 && pMob[i].Target.Y >= 3476 && pMob[i].Target.Y <= 3562) ||
					(pMob[i].Target.X >= 3600 && pMob[i].Target.X <= 3700 && pMob[i].Target.Y >= 3600 && pMob[i].Target.Y <= 3700) ||
					(pMob[i].Target.X >= 3840 && pMob[i].Target.X <= 3967 && pMob[i].Target.Y >= 3584 && pMob[i].Target.Y <= 3712))
				{
					if (memcmp(pUser[i].MacAddress, pUser[clientId].MacAddress, 8) == 0)
						total++;
				}
			}

			if (total >= 1)
			{
				SendClientMessage(clientId, "Disponavel apenas uma conta por computador na area");

				SendItem(clientId, (SlotType)p->SrcType, p->SrcSlot, srcItem);
				return true;
			}

			if (static_cast<CEventItem*>(CEventManager::GetInstance().GetEvent(eEventType::Quiz))->Register(*this, srcItem))
			{
				Teleportar(clientId, 3655, 3640);

				auto now = std::chrono::steady_clock::now();
				if (now - Times.LastLanHouseN <= 2s)
				{
					auto time = std::chrono::duration_cast<std::chrono::milliseconds>(now - Times.LastLanHouseN);

					Log(clientId, LOG_HACK, "O usuario entrou na LanHouse (N) muito rapidamente. Tempo: %lld", time.count());
					Log(SERVER_SIDE, LOG_HACK, "[%s] - O usuario entrou na LanHouse (N) muito rapidamente. Tempo: %lld", User.Username, time.count());
				}

				AmountMinus(srcItem);
			}
		}
#pragma endregion
#pragma region LANHOUSE (MASMORRA M)
		else if (itemId == 4112)
		{
			if (GetVillage(pMob[clientId].Target.X, pMob[clientId].Target.Y) == 5)
			{
				SendClientMessage(clientId, "Somente utilize em uma cidade!");
				SendItem(clientId, (SlotType)p->SrcType, p->SrcSlot, srcItem);

				return true;
			}

			if (pMob[clientId].Mobs.Player.Equip[0].EFV2 != ARCH)
			{
				SendClientMessage(clientId, "Disponavel apenas para archs");

				SendItem(clientId, (SlotType)p->SrcType, p->SrcSlot, srcItem);
				return true;
			}

			INT32 total = 0;
			for (INT32 i = 1; i < MAX_PLAYER; i++)
			{
				if (pUser[i].CurrentScore != USER_PLAY)
					continue;

				if ((pMob[i].Target.X >= 3732 && pMob[i].Target.X <= 3816 && pMob[i].Target.Y >= 3476 && pMob[i].Target.Y <= 3562) ||
					(pMob[i].Target.X >= 3600 && pMob[i].Target.X <= 3700 && pMob[i].Target.Y >= 3600 && pMob[i].Target.Y <= 3700) ||
					(pMob[i].Target.X >= 3840 && pMob[i].Target.X <= 3967 && pMob[i].Target.Y >= 3584 && pMob[i].Target.Y <= 3712))
				{
					if (memcmp(pUser[i].MacAddress, pUser[clientId].MacAddress, 8) == 0)
						total++;
				}
			}

			if (total >= 1)
			{
				SendClientMessage(clientId, "Disponavel apenas uma conta por computador na area");

				SendItem(clientId, (SlotType)p->SrcType, p->SrcSlot, srcItem);
				return true;
			}

			if (static_cast<CEventItem*>(CEventManager::GetInstance().GetEvent(eEventType::Quiz))->Register(*this, srcItem))
			{
				Teleportar(clientId, 3768, 3513);

				auto now = std::chrono::steady_clock::now();
				if (now - Times.LastLanHouseM <= 2s)
				{
					auto time = std::chrono::duration_cast<std::chrono::milliseconds>(now - Times.LastLanHouseM);

					Log(clientId, LOG_HACK, "O usuario entrou na LanHouse (M) muito rapidamente. Tempo: %lld", time.count());
					Log(SERVER_SIDE, LOG_HACK, "[%s] - O usuario entrou na LanHouse (M) muito rapidamente. Tempo: %lld", User.Username, time.count());
				}

				AmountMinus(srcItem);
			}
		}
#pragma endregion
#pragma region LANHOUSE (MASMORRA M)
		else if (itemId == 4113)
		{
			if (GetVillage(pMob[clientId].Target.X, pMob[clientId].Target.Y) == 5)
			{
				SendClientMessage(clientId, "Somente utilize em uma cidade!");
				SendItem(clientId, (SlotType)p->SrcType, p->SrcSlot, srcItem);

				return true;
			}

			if (pMob[clientId].Mobs.Player.Equip[0].EFV2 <= ARCH)
			{
				SendClientMessage(clientId, "Disponavel apenas para CELESTIAL");

				SendItem(clientId, (SlotType)p->SrcType, p->SrcSlot, srcItem);
				return true;
			}

			INT32 total = 0;
			for (INT32 i = 1; i < MAX_PLAYER; i++)
			{
				if (pUser[i].CurrentScore != USER_PLAY)
					continue;

				if ((pMob[i].Target.X >= 3713 && pMob[i].Target.X <= 3838 && pMob[i].Target.Y >= 3459 && pMob[i].Target.Y <= 3582) ||
					(pMob[i].Target.X >= 3600 && pMob[i].Target.X <= 3700 && pMob[i].Target.Y >= 3600 && pMob[i].Target.Y <= 3700) ||
					(pMob[i].Target.X >= 3840 && pMob[i].Target.X <= 3967 && pMob[i].Target.Y >= 3584 && pMob[i].Target.Y <= 3712))
				{
					if (memcmp(pUser[i].MacAddress, pUser[clientId].MacAddress, 8) == 0)
						total++;
				}
			}

			if (total >= 1)
			{
				SendClientMessage(clientId, "Disponavel apenas uma conta por computador na area");

				SendItem(clientId, (SlotType)p->SrcType, p->SrcSlot, srcItem);
				return true;
			}

			if (static_cast<CEventItem*>(CEventManager::GetInstance().GetEvent(eEventType::Quiz))->Register(*this, srcItem))
			{
				Teleportar(clientId, 3896, 3654);

				auto now = std::chrono::steady_clock::now();
				if (now - Times.LastLanHouseA <= 2s)
				{
					auto time = std::chrono::duration_cast<std::chrono::milliseconds>(now - Times.LastLanHouseA);

					Log(clientId, LOG_HACK, "O usuario entrou na LanHouse (A) muito rapidamente. Tempo: %lld", time.count());
					Log(SERVER_SIDE, LOG_HACK, "[%s] - O usuario entrou na LanHouse (A) muito rapidamente. Tempo: %lld", User.Username, time.count());
				}

				AmountMinus(srcItem);
			}
		}
#pragma endregion
#pragma region LACTOLERIUM 100
		else if (itemId == 4141)
		{
			STRUCT_MOB* player = &pMob[clientId].Mobs.Player;
			if (p->DstType == 1)
			{
				int _type = GetEffectValueByIndex(player->Inventory[p->DstSlot].sIndex, EF_SELADO);
				int sanc = GetItemSanc(&player->Inventory[p->DstSlot]);

				if (sanc == 9)
				{
					SendClientMessage(clientId, g_pLanguageString[_NN_CantRefine]);

					SendItem(clientId, (SlotType)p->SrcType, p->SrcSlot, srcItem);
					return true;
				}

				if (_type == 1)
				{
					STRUCT_ITEM* item = &player->Inventory[p->DstSlot];
					SetItemSanc(item, sanc + 1, 0);

					SendClientMessage(clientId, g_pLanguageString[_NN_Refine_Success]);

					Log(clientId, LOG_INGAME, "Poeira 100 usado em %s [%d] [%d %d %d %d %d %d]", g_pItemList[item->sIndex].ItemName, item->sIndex, item->EF1, item->EF1, item->EFV1,
						item->EF2, item->EFV2, item->EF3, item->EFV3);
				}
				else
				{
					SendClientMessage(clientId, g_pLanguageString[_NN_CantRefine]);

					SendItem(clientId, (SlotType)p->SrcType, p->SrcSlot, srcItem);
					return true;
				}
			}
			else
			{
				int _type = GetEffectValueByIndex(player->Equip[p->DstSlot].sIndex, EF_MOBTYPE);
				if (_type == 3)
				{
					SendClientMessage(clientId, g_pLanguageString[_NN_CantRefine_ItemCele]);

					SendItem(clientId, (SlotType)p->SrcType, p->SrcSlot, srcItem);
					return true;
				}

				int sanc = GetItemSanc(&player->Equip[p->DstSlot]);
				if (sanc <= 8 || sanc == 10)
				{
					if (p->DstSlot == 15)
					{
						SendItem(clientId, (SlotType)p->SrcType, p->SrcSlot, srcItem);

						return true;
					}
					// Tentando refinar a face			
					if (p->DstSlot == 0)
						return true;

					STRUCT_ITEM* item = &player->Equip[p->DstSlot];

					SetItemSanc(item, sanc + 1, 0);

					SendClientMessage(clientId, g_pLanguageString[_NN_Refine_Success]);

					Log(clientId, LOG_INGAME, "Poeira 100 usado em %s [%d] [%d %d %d %d %d %d]", g_pItemList[item->sIndex].ItemName, item->sIndex, item->EF1, item->EF1, item->EFV1,
						item->EF2, item->EFV2, item->EF3, item->EFV3);
				}
				else
				{
					SendClientMessage(clientId, g_pLanguageString[_NN_CantRefine]);

					SendItem(clientId, (SlotType)p->SrcType, p->SrcSlot, srcItem);
					return true;
				}

				SendItem(clientId, (SlotType)p->DstType, p->DstSlot, &pMob[clientId].Mobs.Player.Equip[p->DstSlot]);
				AmountMinus(srcItem);

				SendEmotion(clientId, 14, 3);
			}
		}

#pragma endregion
#pragma region SELO DO GUERREIRO
		else if (itemId == 4146)
		{
			INT32 capeId = pMob[clientId].Mobs.Player.Equip[15].sIndex;

			// O que ser isso?
			if (pMob[clientId].Mobs.Player.CurrentScore.Level >= 354 && capeId != 3191 && capeId != 3192 && capeId != 3193)
			{
				INT32 newCape = -1;
				if (pMob[clientId].Mobs.Player.CapeInfo == 7)
					newCape = 3191;
				else if (pMob[clientId].Mobs.Player.CapeInfo == 8)
					newCape = 3192;
				else if (pMob[clientId].Mobs.Player.CapeInfo == CAPE_WHITE)
					newCape = 3193;

				if (newCape != -1)
				{
					memset(&pMob[clientId].Mobs.Player.Equip[15], 0, sizeof STRUCT_ITEM);

					pMob[clientId].Mobs.Player.Equip[15].sIndex = newCape;
					SendItem(clientId, SlotType::Equip, 15, &pMob[clientId].Mobs.Player.Equip[15]);
				}
			}

			pMob[clientId].Mobs.Fame += 10;

			AmountMinus(srcItem);
			SendEmotion(clientId, 14, 3);
		}
#pragma endregion
#pragma region ITENS QUEST
		else if (itemId >= 4117 && itemId <= 4121)
		{
			static DWORD expValue[5] = { 100000, 400000, 1000000, 2000000, 3000000 };
			static DWORD goldValue[5] = { 6000, 10000, 14000, 10000, 15000 };

			STRUCT_MOB* player = &pMob[clientId].Mobs.Player;
			int level = player->BaseScore.Level;

			int questIndex = srcItem->sIndex - 4117;
			if (questIndex < 0 || questIndex > 4)
				return true;

			AmountMinus(srcItem);

			if (level < g_pQuestLevel[questIndex][0] || level > g_pQuestLevel[questIndex][1])
			{
				SendClientMessage(clientId, g_pLanguageString[_NN_Level_NotAllowed], g_pQuestLevel[questIndex][0] + 1, g_pQuestLevel[questIndex][1] + 1);

				return true;
			}

			if (pMob[clientId].Mobs.Player.Equip[0].EFV2 != MORTAL && pMob[clientId].Mobs.Player.Equip[0].EFV2 != ARCH)
			{
				SendClientMessage(clientId, g_pLanguageString[_NN_OnlyMortal]);

				return true;
			}

			int gold = goldValue[questIndex];
			int exp = expValue[questIndex];

			if (player->Coin + gold > 2000000000)
				gold = (2000000000 - player->Coin);

			player->Coin += gold;

			player->Exp += exp;

			INT32 leader = pMob[clientId].Leader;
			if (leader == 0)
				leader = clientId;

			for (INT32 i = 0; i < 12; i++)
			{
				INT32 memberId = pMob[leader].PartyList[i];
				if (memberId <= 0 || memberId >= MAX_PLAYER || memberId == clientId)
					continue;

				if (pMob[memberId].Mobs.Player.Equip[0].EFV2 == MORTAL || pMob[memberId].Mobs.Player.Equip[0].EFV2 == ARCH)
				{
					if (pMob[memberId].Mobs.Player.CurrentScore.Level > g_pQuestLevel[questIndex][1])
						continue;

					pMob[memberId].Mobs.Player.Exp += (exp / 10);

					SendClientMessage(memberId, ".+++ %d de Experiancia +++", exp / 10);

					SendEtc(memberId);
					SendEmotion(clientId, 14, 3);
				}
			}

			if (leader != 0 && clientId != leader && leader > 0 && leader <= MAX_PLAYER)
			{
				if (pMob[leader].Mobs.Player.Equip[0].EFV2 == MORTAL || pMob[leader].Mobs.Player.Equip[0].EFV2 == ARCH)
				{
					if (pMob[leader].Mobs.Player.CurrentScore.Level < g_pQuestLevel[questIndex][1])
					{
						pMob[leader].Mobs.Player.Exp += (exp / 10);

						SendClientMessage(leader, ".+++ %d de Experiancia +++", exp / 10);

						SendEtc(leader);
						SendEmotion(clientId, 14, 3);
					}
				}
			}

			SendEmotion(clientId, 14, 3);

			pMob[clientId].GetCurrentScore(clientId);
			pMob[clientId].CheckGetLevel();

			SendScore(clientId);
			SendEtc(clientId);

			SendSignalParm(clientId, clientId, 0x3AF, player->Coin);

			SendClientMessage(clientId, ".+++ %d de Experiancia +++", exp);
		}
#pragma endregion
#pragma region PEDRA MISTERIOSA
		else if (itemId == 4148)
		{
            static clock_t delay[MAX_PLAYER] = { 0 };
			if (clock() - delay[clientId] < 5 * CLOCKS_PER_SEC)
			{
				SendClientMessage(clientId, "Aguarde alguns segundos para utilizar novamente...");

				SendItem(clientId, (SlotType)p->SrcType, p->SrcSlot, srcItem);
				return true;
			}

			if (!GetAttribute(pMob[clientId].Target.X, pMob[clientId].Target.Y).Village)
			{
				SendClientMessage(clientId, "Nao a possavel utilizar fora da cidade");

				SendItem(clientId, (SlotType)p->SrcType, p->SrcSlot, srcItem);
				return true;
			}

			if ((pMob[clientId].Mobs.Player.Equip[0].EFV2 == CELESTIAL && pMob[clientId].Mobs.Sub.CurrentScore == 1) || (pMob[clientId].Mobs.Player.Equip[0].EFV2 == SUBCELESTIAL))
			{
                LogEquipsAndInventory(true);

				STRUCT_SUBINFO* sub = &pMob[clientId].Mobs.Sub;
				STRUCT_MOB* player = &pMob[clientId].Mobs.Player;

				STRUCT_SCORE tmpStatus;
				memcpy(&tmpStatus, &player->BaseScore, sizeof STRUCT_SCORE);
				memcpy(&player->BaseScore, &sub->SubStatus, sizeof STRUCT_SCORE);
				memcpy(&sub->SubStatus, &tmpStatus, sizeof STRUCT_SCORE);

				STRUCT_ITEM item[2];
				memcpy(&item[0], &player->Equip[0], sizeof STRUCT_ITEM);
				memcpy(&item[1], &player->Equip[15], sizeof STRUCT_ITEM);

				memcpy(&player->Equip[0], &sub->Equip[0], sizeof STRUCT_ITEM);
				memcpy(&sub->Equip[0], &item[0], sizeof STRUCT_ITEM);

				INT64 exp = player->Exp;
				player->Exp = sub->Exp;
				sub->Exp = exp;

				STRUCT_ITEMDATE date;
				memcpy(&date, &pMob[clientId].Mobs.Escritura, sizeof STRUCT_ITEMDATE);

				INT32 baseFace = player->Equip[0].EF2;
				INT32 classInfo = 0;
				if (baseFace < 10)
					classInfo = (baseFace - 6);
				else if (baseFace < 20)
					classInfo = (baseFace - 16);
				else if (baseFace < 30)
					classInfo = (baseFace - 26);
				else if (baseFace < 40)
					classInfo = (baseFace - 36);

				player->Class = classInfo;

				Log(clientId, LOG_INGAME, "Classe %d gerada atravas da face %d", classInfo, baseFace);

				auto learn = player->LearnedSkill[0];
				auto secLearn = player->LearnedSkill[1];
				player->LearnedSkill[0] = sub->LearnedSkill;
				player->LearnedSkill[1] = sub->SecLearn;

				sub->LearnedSkill = learn;
				sub->SecLearn = secLearn;

				INT32 soul = pMob[clientId].Mobs.Soul;
				pMob[clientId].Mobs.Soul = sub->Soul;
				sub->Soul = soul;

				UNION_QUESTINFO tmpInfo;
				tmpInfo.Value = pMob[clientId].Mobs.Info.Value;
				pMob[clientId].Mobs.Info.Value = sub->Info.Value;
				sub->Info.Value = tmpInfo.Value;

				STRUCT_AFFECT affect[32];
				memcpy(affect, pMob[clientId].Mobs.Affects, sizeof STRUCT_AFFECT * 32);
				memcpy(pMob[clientId].Mobs.Affects, sub->Affect, sizeof STRUCT_AFFECT * 32);
				memcpy(sub->Affect, affect, sizeof STRUCT_AFFECT * 32);

				unsigned char skillBar[20];
				memset(&skillBar, 255, sizeof skillBar);

				memcpy(skillBar, player->ShortSkill, 4);
				memcpy(&skillBar[4], &pMob[clientId].Mobs.SkillBar[0], 16);

				memcpy(player->ShortSkill, sub->SkillBar, 4);
				memcpy(pMob[clientId].Mobs.SkillBar, &sub->SkillBar[4], 16);

				memcpy(sub->SkillBar, skillBar, 20);

				sub->CurrentScore = 1;

				if (pMob[clientId].Mobs.Player.Equip[0].EFV2 == 3)
					pMob[clientId].Mobs.Player.Equip[0].EFV2 = 3;
				else
					pMob[clientId].Mobs.Player.Equip[0].EFV2 = 4;

				pMob[clientId].GetCurrentScore(clientId);

				if (pMob[clientId].Mobs.Player.GetEvolution() >= Celestial && pMob[clientId].Mobs.Player.BaseScore.Level == 199 && pMob[clientId].Mobs.Player.BaseScore.Level < 399 && !pMob[clientId].Mobs.Info.Unlock200 && !pMob[clientId].Mobs.Info.LvBlocked && pMob[clientId].Mobs.GetTotalResets() == 3)
				{
					Log(clientId, LOG_INGAME, "Bloqueado o navel 200 do personagem devido a nao estar bloqueado e nao ter desbloqueado.");

					pMob[clientId].Mobs.Info.LvBlocked = 1;
					pMob[clientId].Mobs.Player.Exp = g_pNextLevel[pMob[clientId].Mobs.Player.Equip[0].EFV2][pMob[clientId].Mobs.Player.BaseScore.Level - 1];
				}

				p364 packet;
				GetCreateMob(clientId, (BYTE*)& packet);

				GridMulticast_2(pMob[clientId].Target.X, pMob[clientId].Target.Y, (BYTE*)& packet, 0);

				SendItem(clientId, SlotType::Equip, 0, &pMob[clientId].Mobs.Player.Equip[0]);

				SendScore(clientId);
				SendEtc(clientId);

				p378 skill;
				skill.Header.PacketId = 0x378;
				skill.Header.Size = sizeof p378;
				skill.Header.ClientId = clientId;

				memcpy(skill.ShortSkill, player->ShortSkill, 4);
				memcpy(skill.SkillBar2, pMob[clientId].Mobs.SkillBar, 16);

				AddMessage((BYTE*)& skill, sizeof p378);

				SaveUser(clientId, 0);
				AmountMinus(srcItem);

				delay[clientId] = clock();
			}
			else
				SendItem(clientId, (SlotType)p->SrcType, p->SrcSlot, srcItem);
		}
#pragma endregion
#pragma region POaaO DO VIGOR
		else if (itemId == 3313)
		{
			INT32 slot = -1;
			for (INT32 i = 0; i < 32; i++)
			{
				if (pMob[clientId].Mobs.Affects[i].Index == 35)
				{
					slot = i;

					break;
				}
			}

			if (slot == -1)
			{
				for (INT32 i = 0; i < 32; i++)
				{
					if (pMob[clientId].Mobs.Affects[i].Index == 0)
					{
						slot = i;

						break;
					}
				}
			}

			if (slot == -1)
			{
				SendItem(clientId, (SlotType)p->SrcType, p->SrcSlot, srcItem);

				return true;
			}

			pMob[clientId].Mobs.Affects[slot].Index = 35;
			pMob[clientId].Mobs.Affects[slot].Value = 2;
			pMob[clientId].Mobs.Affects[slot].Time = 900;

			SendAffect(clientId);

			p364 packet;
			GetCreateMob(clientId, (BYTE*)& packet);

			GridMulticast_2(pMob[clientId].Target.X, pMob[clientId].Target.Y, (BYTE*)& packet, 0);

			AmountMinus(srcItem);
		}
#pragma endregion
#pragma region JaIAS
		else if (itemId >= 3200 && itemId <= 3209)
		{
			INT32 index = itemId - 3200;

			INT32 value = 0;
			switch (index)
			{
			case 0:
				value = 1;
				break;
			case 1:
				value = 2;
				break;
			case 2:
				value = 4;
				break;
			case 3: // Jaia da Recuperaaao
				break;
			case 4:
				value = 8;
				break;
			case 5:
				value = 16;
				break;
			case 6:
				value = 32;
				break;
			case 7: // Jaia armazenagem
				break;
			case 8:
				value = 64;
				break;
			case 9:
				value = 128;
				break;
			}

			if (value == 0)
			{
				if (index == 3)
				{
					INT32 i = 0;
					for (; i < 32; i++)
					{
						if (pMob[clientId].Mobs.Affects[i].Index == 32) // Cancel
						{
							memset(&pMob[clientId].Mobs.Affects[i], 0, sizeof STRUCT_AFFECT);

							SendAffect(clientId);

							p364 packet;
							GetCreateMob(clientId, (BYTE*)& packet);

							GridMulticast_2(pMob[clientId].Target.X, pMob[clientId].Target.Y, (BYTE*)& packet, 0);
							break;
						}
					}

					if (i == 32)
					{
						SendItem(clientId, (SlotType)p->SrcType, p->SrcSlot, srcItem);

						return true;
					}
				}

				AmountMinus(srcItem);
				return true;
			}

			INT32 slot = GetEmptyAffect(clientId, 8);

			if (slot == -1)
			{
				SendItem(clientId, (SlotType)p->SrcType, p->SrcSlot, srcItem);

				return true;
			}

			pMob[clientId].Mobs.Affects[slot].Index = 8;
			pMob[clientId].Mobs.Affects[slot].Time = 450;

			if (!(pMob[clientId].Mobs.Affects[slot].Value & value))
				pMob[clientId].Mobs.Affects[slot].Value |= value;

			pMob[clientId].Jewel = slot;

			SendAffect(clientId);

			p364 packet;
			GetCreateMob(clientId, (BYTE*)& packet);

			GridMulticast_2(pMob[clientId].Target.X, pMob[clientId].Target.Y, (BYTE*)& packet, 0);

			AmountMinus(srcItem);
		}
#pragma endregion
#pragma region ABRIR PACLAC
		else if (itemId == 3447 || itemId == 3448)
		{
			INT32 amount = GetItemAmount(srcItem);

			INT32 i;
			for (i = 0; i < amount; i++)
			{
				INT32 slotId = GetFirstSlot(clientId, 0);
				if (slotId == -1)
				{
					SendClientMessage(clientId, g_pLanguageString[_NN_You_Have_No_Space_To_Trade]);

					break;
				}
				else
				{
					memset(&pMob[clientId].Mobs.Player.Inventory[slotId], 0, sizeof STRUCT_ITEM);

					pMob[clientId].Mobs.Player.Inventory[slotId].sIndex = (412 + (srcItem->sIndex - 3447));
					SendItem(clientId, SlotType::Inv, slotId, &pMob[clientId].Mobs.Player.Inventory[slotId]);

					AmountMinus(srcItem);
				}
			}

			SendItem(clientId, (SlotType)p->SrcType, p->SrcSlot, srcItem);
		}
#pragma endregion
#pragma region SELO DO GUERREIRO
		else if (itemId == 4146)
		{
			pMob[clientId].Mobs.Fame += 10;

			if (pMob[clientId].Mobs.Player.Equip[15].sIndex != 3191 && pMob[clientId].Mobs.Player.Equip[15].sIndex != 3192 && pMob[clientId].Mobs.Player.BaseScore.Level >= 354)
			{
				INT32 cape = 3193,
					capeInfo = pMob[clientId].Mobs.Player.CapeInfo;

				if (capeInfo == CAPE_BLUE)
					cape = 3191;
				else if (capeInfo == CAPE_RED)
					cape = 3192;
				else
					cape = 3193;

				STRUCT_ITEM* item = &pMob[clientId].Mobs.Player.Equip[15];
				memset(item, 0, sizeof STRUCT_ITEM);

				item->sIndex = cape;

				// Seta a refinaaao como +0
				SetItemSanc(item, 0, 0);

				SendItem(clientId, SlotType::Equip, 15, item);
				SendEquip(clientId);

				Log(clientId, LOG_INGAME, "Adquirido capa %d com Selo do Guerreiro", cape);
			}

			Log(clientId, LOG_INGAME, "Adquirido fame pelo Selo do Guerreiro - %d fame", pMob[clientId].Mobs.Fame);

			AmountMinus(srcItem);
			SendEmotion(clientId, 14, 3);
		}
#pragma endregion
#pragma region CRISTAIS ARCH
		else if (itemId >= 4106 && itemId <= 4109)
		{
			INT32 classMaster = pMob[clientId].Mobs.Player.Equip[0].EFV2,
				level = pMob[clientId].Mobs.Player.CurrentScore.Level;

			INT32 index = itemId - 4106;
			BOOL conc = FALSE,
				anterior = TRUE;

			if (index == 0)
				conc = pMob[clientId].Mobs.Info.Elime;
			else if (index == 1)
			{
				if (!pMob[clientId].Mobs.Info.Elime)
					anterior = FALSE;
				conc = pMob[clientId].Mobs.Info.Sylphed;
			}
			else if (index == 2)
			{
				if (!pMob[clientId].Mobs.Info.Elime)
					anterior = FALSE;
				else if (!pMob[clientId].Mobs.Info.Sylphed)
					anterior = FALSE;

				conc = pMob[clientId].Mobs.Info.Thelion;
			}
			else if (index == 3)
			{
				if (!pMob[clientId].Mobs.Info.Elime)
					anterior = FALSE;
				else if (!pMob[clientId].Mobs.Info.Sylphed)
					anterior = FALSE;
				else if (!pMob[clientId].Mobs.Info.Thelion)
					anterior = FALSE;

				conc = pMob[clientId].Mobs.Info.Noas;
			}

			if (pMob[clientId].Mobs.Player.Exp < 100000000)
			{
				SendClientMessage(clientId, "Experiancia insuficiente.");
				SendItem(clientId, (SlotType)p->SrcType, p->SrcSlot, srcItem);
				return true;
			}

			if (conc)
			{
				SendClientMessage(clientId, g_pLanguageString[_NN_Youve_Done_It_Already]);

				SendItem(clientId, (SlotType)p->SrcType, p->SrcSlot, srcItem);
				return true;
			}

			if (!anterior)
			{
				SendClientMessage(clientId, "Conclua a quest anterior para continuar");

				SendItem(clientId, (SlotType)p->SrcType, p->SrcSlot, srcItem);
				return true;
			}
			if (classMaster == ARCH)
			{
				if (level < 354)
				{
					SendClientMessage(clientId, g_pLanguageString[_NN_CantTele_Nip]);

					SendItem(clientId, (SlotType)p->SrcType, p->SrcSlot, srcItem);
					return true;
				}
			}
			else if (classMaster == CELESTIAL)
			{
				if (pMob[clientId].Mobs.Fame < 100)
				{
					SendClientMessage(clientId, g_pLanguageString[_NN_Need_XX_Fame], 100);

					SendItem(clientId, (SlotType)p->SrcType, p->SrcSlot, srcItem);
					return true;
				}

				if (level < 39)
				{
					SendClientMessage(clientId, g_pLanguageString[_NN_CantTele_Nip]);

					SendItem(clientId, (SlotType)p->SrcType, p->SrcSlot, srcItem);
					return true;
				}
			}
			else
			{
				SendItem(clientId, (SlotType)p->SrcType, p->SrcSlot, srcItem);
				return true;
			}

			if (index == 0)
				pMob[clientId].Mobs.Info.Elime = 1;
			else if (index == 1)
				pMob[clientId].Mobs.Info.Sylphed = 1;
			else if (index == 2)
				pMob[clientId].Mobs.Info.Thelion = 1;
			else if (index == 3)
				pMob[clientId].Mobs.Info.Noas = 1;

			pMob[clientId].Mobs.Player.Exp -= 100000000;

			switch (classMaster)
			{

			case ARCH:
				while (pMob[clientId].Mobs.Player.Exp < g_pNextLevel[classMaster][level - 1])
				{
					pMob[clientId].Mobs.Player.BaseScore.Level--;
					level--;
				}
				break;

			case CELESTIAL:
				while (level > 0 && pMob[clientId].Mobs.Player.Exp < g_pNextLevel[classMaster][level - 1])
				{
					pMob[clientId].Mobs.Player.BaseScore.Level--;
					level--;
				}
				break;
			}

			pMob[clientId].GetCurrentScore(clientId);

			SendScore(clientId);
			SendEtc(clientId);

			AmountMinus(srcItem);

			SendEmotion(clientId, 14, 3);
		}
#pragma endregion
#pragma region  ACELERADOR DE NASCIMENTO
		else if (itemId == 3438)
		{
			STRUCT_ITEM* dstItem = GetItemPointer(clientId, p->DstType, p->DstSlot);

			if (dstItem->sIndex < 2300 || dstItem->sIndex >= 2330)
			{
				SendItem(clientId, (SlotType)p->SrcType, p->SrcSlot, srcItem);

				return true;
			}

			int totalIncubate = GetItemAbility(dstItem, EF_INCUBATE) - 1;
			if (totalIncubate <= 0)
			{
				INT32 mountIndex = dstItem->sIndex - 2300;
				dstItem->sIndex = 2330 + mountIndex;

				*(short*)& dstItem->EF1 = 5000;

				dstItem->EF2 = 1;
				dstItem->EFV2 = (Rand() % 20 + 10);

				dstItem->EF3 = 30;
				dstItem->EFV3 = 1;

				SendClientMessage(clientId, g_pLanguageString[_NN_INCUBATED]);

				Log(clientId, LOG_INGAME, "Montaria cresceu com acelerador de nascimento: %s [%d] [%d %d %d %d %d %d]", g_pItemList[dstItem->sIndex].ItemName, dstItem->sIndex, dstItem->EF1,
					dstItem->EFV1, dstItem->EF2, dstItem->EFV2, dstItem->EFV2, dstItem->EF3, dstItem->EFV3);

				MountProcess(clientId, 0);
			}
			else
			{
				for (int i = 0; i < 3; i++)
				{
					if (dstItem->stEffect[i].cEffect == EF_INCUBATE)
					{
						dstItem->stEffect[i].cValue = totalIncubate;

						break;
					}
				}

				SendClientMessage(clientId, "A incubaaao foi reduzida");
				Log(clientId, LOG_INGAME, "Diminuado incubaaao do item %s %s", g_pItemList[dstItem->sIndex].ItemName, dstItem->toString().c_str());
			}

			SendItem(clientId, (SlotType)p->DstType, p->DstSlot, &pMob[clientId].Mobs.Player.Equip[14]);
			AmountMinus(srcItem);
		}
#pragma endregion
#pragma region CATALISADORES
		else if (itemId >= 3344 && itemId <= 3350)
		{
			INT32 index = itemId - 3344;
			STRUCT_ITEM* dstItem = GetItemPointer(clientId, p->DstType, p->DstSlot);

			INT32 mountId = dstItem->sIndex;
			INT32 i = 0;
			for (; i < 6; i++)
			{
				if (dstItem->sIndex == (g_pItemRest[index][i] - 30))
					break;
			}

			if (i == 6)
			{
				SendClientMessage(clientId, g_pLanguageString[_NN_Mount_Not_Match]);

				SendItem(clientId, (SlotType)p->SrcType, p->DstType, srcItem);
				return true;
			}

			pMob[clientId].Mobs.Player.Equip[14].sIndex = mountId + 30;

			*(short*)& pMob[clientId].Mobs.Player.Equip[14].EF1 = 5000;

			pMob[clientId].Mobs.Player.Equip[14].EF2 = 1;
			pMob[clientId].Mobs.Player.Equip[14].EFV2 += Rand() % 16 + 5;

			pMob[clientId].Mobs.Player.Equip[14].EF3 = 100;
			pMob[clientId].Mobs.Player.Equip[14].EFV3 = 0;

			SendClientMessage(clientId, g_pLanguageString[_NN_INCUBATED]);

			Log(clientId, LOG_INGAME, "Montaria cresceu : %s [%d] [%d %d %d %d %d %d]", g_pItemList[pMob[clientId].Mobs.Player.Equip[14].sIndex].ItemName, pMob[clientId].Mobs.Player.Equip[14].sIndex, pMob[clientId].Mobs.Player.Equip[14].EF1,
				pMob[clientId].Mobs.Player.Equip[14].EFV1, pMob[clientId].Mobs.Player.Equip[14].EF2, pMob[clientId].Mobs.Player.Equip[14].EFV2, pMob[clientId].Mobs.Player.Equip[14].EFV2, pMob[clientId].Mobs.Player.Equip[14].EF3,
				pMob[clientId].Mobs.Player.Equip[14].EFV3);

			MountProcess(clientId, 0);

			SendClientMessage(clientId, g_pLanguageString[_NN_INCUBATED]);

			SendItem(clientId, (SlotType)p->DstType, p->DstSlot, dstItem);
			SendEmotion(clientId, 14, 3);

			AmountMinus(srcItem);
		}
#pragma endregion
#pragma region RESTAURADORES
		else if (itemId >= 3351 && itemId <= 3357)
		{
			INT32 index = itemId - 3351;
			STRUCT_ITEM* dstItem = GetItemPointer(clientId, p->DstType, p->DstSlot);

			if (dstItem->sIndex <= 2360 || dstItem->sIndex >= 2390 || index < 0 || index > 6)
			{
				SendItem(clientId, (SlotType)p->SrcType, p->DstType, srcItem);

				return true;
			}

			if (dstItem->EFV2 >= 60)
			{
				SendClientMessage(clientId, "Limite de 60 pontos de vitalidade");

				SendItem(clientId, (SlotType)p->SrcType, p->DstType, srcItem);
				return true;
			}

			INT32 i = 0;
			for (; i < 6; i++)
			{
				if (dstItem->sIndex == g_pItemRest[index][i])
					break;
			}

			if (i == 6)
			{
				SendClientMessage(clientId, g_pLanguageString[_NN_Mount_Not_Match]);

				SendItem(clientId, (SlotType)p->SrcType, p->DstType, srcItem);
				return true;
			}

			int rand = (Rand() % 3) + 1;
			int vit = dstItem->EFV2 + rand;
			if (vit > 60)
				vit = 60;

			dstItem->EFV2 = vit;

			Log(clientId, LOG_INGAME, "Utilizado item %s [%d] em %s [%d] [%d %d %d %d %d %d]. Vitalidade aumentada em %d", g_pItemList[itemId].ItemName, itemId, g_pItemList[dstItem->sIndex].ItemName, dstItem->sIndex, dstItem->EF1, dstItem->EFV1, dstItem->EF2, dstItem->EFV2, dstItem->EF3, dstItem->EFV3,
				rand);

			SendItem(clientId, (SlotType)p->DstType, p->DstSlot, dstItem);
			AmountMinus(srcItem);

			SendEmotion(clientId, 14, 3);
		}
#pragma endregion
#pragma region EXTRAaaES
		else if (itemId >= 3021 && itemId <= 3026)
		{
			int destino = srcItem->sIndex - 3020;
			if (pMob[clientId].Mobs.Player.Equip[destino].sIndex == 0)
			{
				SendItem(clientId, (SlotType)p->SrcType, p->SrcSlot, srcItem);
				return true;
			}

			STRUCT_ITEM* dstItem = GetItemPointer(clientId, (int)SlotType::Equip, destino);
			if (dstItem->sIndex <= 0 || dstItem->sIndex >= MAX_ITEMLIST)
			{
				SendItem(clientId, (SlotType)p->SrcType, p->SrcSlot, srcItem);
				return true;
			}

			INT32 itemType = -1;
			for (INT32 i = 0; i < 3; i++)
			{
				if (srcItem->stEffect[i].cEffect == 87)
				{
					itemType = srcItem->stEffect[i].cValue;

					break;
				}
			}

			if (itemType == -1)
			{
				SendClientMessage(clientId, g_pLanguageString[_NN_Bad_Network_Packets]);

				SendItem(clientId, (SlotType)p->SrcType, p->SrcSlot, srcItem);
				return true;
			}

			INT32 ability = GetItemAbility(dstItem, EF_UNKNOW1);
			if (ability != itemType && itemType <= 5)
			{
				SendClientMessage(clientId, g_pLanguageString[_NN_Items_Same_Class]);

				SendItem(clientId, (SlotType)p->SrcType, p->SrcSlot, srcItem);
				return true;
			}

			INT32 sanc = GetItemSanc(dstItem);

			INT32 mobType = GetEffectValueByIndex(dstItem->sIndex, EF_MOBTYPE);
			if (itemType == 5)
			{
				if (mobType == 1)
				{
					SendClientMessage(clientId, g_pLanguageString[_NN_Items_JustMortal]);

					SendItem(clientId, (SlotType)p->SrcType, p->SrcSlot, srcItem);
					return true;
				}
			}
			else if (itemType >= 0 && itemType <= 4)
			{
				if (sanc > 9)
				{
					SendClientMessage(clientId, g_pLanguageString[_NN_Items_Smaller9]);

					SendItem(clientId, (SlotType)p->SrcType, p->SrcSlot, srcItem);
					return true;
				}
			}
			else if (itemType > 5)
			{
				if (itemType == 11 || itemType == 10)
				{// 11 = Arma arch +10+ Anct
				 // 10 = Arma arch +9- Anct
					INT32 grade = g_pItemList[dstItem->sIndex].nGrade;
					if (grade < 5 || grade > 8)
					{
						SendClientMessage(clientId, g_pLanguageString[_NN_Items_Anct]);

						SendItem(clientId, (SlotType)p->SrcType, p->SrcSlot, srcItem);
						return true;
					}

					if (mobType != 1)
					{
						SendClientMessage(clientId, g_pLanguageString[_NN_Items_Arch]);

						SendItem(clientId, (SlotType)p->SrcType, p->SrcSlot, srcItem);
						return true;
					}

					INT32 tmpSanc = GetItemSanc(dstItem);
					if (itemType == 10)
					{
						if (tmpSanc > 9)
						{
							SendClientMessage(clientId, g_pLanguageString[_NN_Items_Smaller9]);

							SendItem(clientId, (SlotType)p->SrcType, p->SrcSlot, srcItem);
							return true;
						}
					}
					else
					{
						if (tmpSanc != 11)
						{
							SendClientMessage(clientId, g_pLanguageString[_NN_Items_Plus11]);

							SendItem(clientId, (SlotType)p->SrcType, p->SrcSlot, srcItem);
							return true;
						}

						sanc = 11;
					}
				}

				else if (itemType == 9 || itemType == 8)
				{
					INT32 grade = g_pItemList[dstItem->sIndex].nGrade;
					if (grade >= 5 && grade <= 8)
					{
						SendClientMessage(clientId, g_pLanguageString[_NN_Items_NotAnct]);

						SendItem(clientId, (SlotType)p->SrcType, p->SrcSlot, srcItem);
						return true;
					}

					if (mobType != 1)
					{
						SendClientMessage(clientId, g_pLanguageString[_NN_Items_Arch]);

						SendItem(clientId, (SlotType)p->SrcType, p->SrcSlot, srcItem);
						return true;
					}

					INT32 tmpSanc = GetItemSanc(dstItem);
					if (itemType == 9)
					{
						if (tmpSanc != 11)
						{
							SendClientMessage(clientId, g_pLanguageString[_NN_Items_Smaller9]);

							SendItem(clientId, (SlotType)p->SrcType, p->SrcSlot, srcItem);
							return true;
						}

						sanc = 11;
					}
					else
					{
						if (tmpSanc > 9)
						{
							SendClientMessage(clientId, g_pLanguageString[_NN_Items_Smaller9]);

							SendItem(clientId, (SlotType)p->SrcType, p->SrcSlot, srcItem);
							return true;
						}
					}
				}
			}

			INT32 ref = 0;
			for (INT32 i = 0; i < 3; i++)
			{
				if (dstItem->stEffect[i].cEffect == EF_SANC || (dstItem->stEffect[i].cEffect >= 116 && dstItem->stEffect[i].cEffect <= 125))
				{
					ref = dstItem->stEffect[i].cEffect;

					if (sanc == 0)
						sanc = dstItem->stEffect[i].cValue;

					break;
				}
			}

			for (INT32 i = 0; i < 3; i++)
			{
				if (srcItem->stEffect[i].cEffect == 87)
				{
					srcItem->stEffect[i].cEffect = ref;

					break;
				}
			}

			Log(clientId, LOG_INGAME, "Utilizou extraaao no item %s [%d] [%d %d %d %d %d %d]", g_pItemList[dstItem->sIndex].ItemName, dstItem->sIndex, dstItem->EF1, dstItem->EFV1, dstItem->EF2, dstItem->EFV2, dstItem->EF3, dstItem->EFV3);

			SetItemSanc(srcItem, sanc, 0);

			srcItem->sIndex = dstItem->sIndex;

			INT32 index = dstItem->sIndex;
			memcpy(dstItem, srcItem, sizeof STRUCT_ITEM);

			dstItem->sIndex = index;

			AmountMinus(srcItem);

			SendItem(clientId, (SlotType)p->SrcType, p->SrcSlot, srcItem);
			SendItem(clientId, SlotType::Equip, destino, dstItem);

		}
#pragma endregion
#pragma region COMPOSTO DO EQUILIBRIO
		else if (itemId == 4126)
		{
			if (pMob[clientId].Mobs.Player.BaseScore.Level < 119 ||
				pMob[clientId].Mobs.Player.BaseScore.Level > 124)
			{
				SendClientMessage(clientId, "Level inadequado.");

				return true;
			}

			if (pMob[clientId].Mobs.Player.Equip[0].EFV2 != MORTAL)
			{
				SendClientMessage(clientId, "Somente para mortais.");

				return true;
			}

			if (pMob[clientId].Mobs.Info.BalanceQuest != 0)
			{
				SendClientMessage(clientId, "Quest ja realizada.");

				return true;
			}

			INT32 totalMasterPoint = pMob[clientId].Mobs.Player.SpecialBonus;
			for (INT32 i = 0; i < 4; i++)
			{
				totalMasterPoint += pMob[clientId].Mobs.Player.BaseScore.Special[i];

				pMob[clientId].Mobs.Player.BaseScore.Special[i] = 0;
			}

			pMob[clientId].Mobs.Player.SpecialBonus = totalMasterPoint;

			for (int i = 0; i < 3; i++)
			{
				int initial = (i * 8);
				for (int a = initial; a < initial + 8; a++)
				{
					int has = (pMob[clientId].Mobs.Player.LearnedSkill[0] & (1 << a));
					if (has != 0)
						pMob[clientId].Mobs.Player.LearnedSkill[0] -= (1 << a);
				}
			}

			pMob[clientId].GetCurrentScore(clientId);

			SendScore(clientId);
			SendAffect(clientId);

			AmountMinus(srcItem);

			SendItem(clientId, (SlotType)p->SrcType, p->SrcSlot, srcItem);

			pMob[clientId].Mobs.Info.BalanceQuest = 1;

			SendClientMessage(clientId, "Processo concluido.");

			return true;
		}
#pragma endregion
#pragma region MOLAR DE GaRGULA
		else if (itemId == 4122)
		{
			if (pMob[clientId].Mobs.Player.BaseScore.Level < 199 || pMob[clientId].Mobs.Player.BaseScore.Level > 254)
			{
				SendClientMessage(clientId, g_pLanguageString[_NN_Level_NotAllowed], 200, 255);

				SendItem(clientId, SlotType::Inv, p->SrcSlot, srcItem);
				return true;
			}

			if (pMob[clientId].Mobs.Info.MolarGargula)
			{
				SendClientMessage(clientId, g_pLanguageString[_NN_Youve_Done_It_Already]);

				SendItem(clientId, SlotType::Inv, p->SrcSlot, srcItem);
				return true;
			}

			pMob[clientId].Mobs.Info.MolarGargula = 1;

			for (INT32 i = 1; i < 6; i++)
			{
				STRUCT_ITEM* item = &pMob[clientId].Mobs.Player.Equip[i];
				if (item->sIndex <= 0 || item->sIndex >= MAX_ITEMLIST)
				{
					Log(clientId, LOG_INGAME, "Molar de Gargula - Slot %d : nao possuia item equipado", i);
					continue;
				}

				Log(clientId, LOG_INGAME, "Molar de Gargula - Slot %d : %s [%d] [%d %d %d %d %d %d]", i, g_pItemList[item->sIndex].ItemName,
					item->sIndex, item->EF1, item->EFV1, item->EF2, item->EFV2, item->EF3, item->EFV3);

				INT32 sanc = GetItemSanc(item);
				if (sanc >= 6)
				{
					Log(clientId, LOG_INGAME, "Molar de Gargula - Slot %d : %s item nao refinado por ser +%d", i, g_pItemList[item->sIndex].ItemName, sanc);
					continue;
				}

				sanc += 4;
				if (sanc > 6)
					sanc = 6;

				SetItemSanc(item, sanc, 0);
				SendItem(clientId, SlotType::Equip, i, item);
			}

			AmountMinus(srcItem);
			Log(clientId, LOG_INGAME, "%s finalizou a quest do molar do gargula.", pMob[clientId].Mobs.Player.MobName);
		}
#pragma endregion
#pragma region COMPOSTO DE CHANCE
		else if (itemId == 4124)
		{
			if (pMob[clientId].Mobs.Player.BaseScore.Level < 69 ||
				pMob[clientId].Mobs.Player.BaseScore.Level > 74)
			{
				SendClientMessage(clientId, "Level inadequado.");

				SendItem(clientId, SlotType::Inv, p->SrcSlot, srcItem);
				return true;
			}

			if (pMob[clientId].Mobs.Player.Equip[0].EFV2 != MORTAL)
			{
				SendClientMessage(clientId, "Somente para mortais.");

				SendItem(clientId, SlotType::Inv, p->SrcSlot, srcItem);
				return true;
			}

			if (pMob[clientId].Mobs.Info.GodBless != 0)
			{
				SendClientMessage(clientId, "Quest ja realizada.");

				SendItem(clientId, SlotType::Inv, p->SrcSlot, srcItem);
				return true;
			}

			INT32 totalMasterPoint = pMob[clientId].Mobs.Player.SpecialBonus;
			for (INT32 i = 0; i < 4; i++)
			{
				if (pMob[clientId].Mobs.Player.BaseScore.Special[i] >= 50)
				{
					totalMasterPoint += pMob[clientId].Mobs.Player.BaseScore.Special[i];

					pMob[clientId].Mobs.Player.BaseScore.Special[i] -= 50;
				}
			}

			pMob[clientId].Mobs.Player.SpecialBonus = totalMasterPoint;

			for (int i = 0; i < 3; i++)
			{
				int initial = (i * 8);
				for (int a = initial; a < initial + 8; a++)
				{
					int has = (pMob[clientId].Mobs.Player.LearnedSkill[0] & (1 << a));
					if (has != 0)
						pMob[clientId].Mobs.Player.LearnedSkill[0] -= (1 << a);
				}
			}

			pMob[clientId].GetCurrentScore(clientId);

			SendScore(clientId);
			SendAffect(clientId);
			SendEtc(clientId);

			pMob[clientId].Mobs.Info.GodBless = 1;

			AmountMinus(srcItem);
			SendItem(clientId, (SlotType)p->SrcType, p->SrcSlot, srcItem);

			return true;
		}
#pragma endregion
#pragma region RECUSA DE GUERRA
		else if (itemId == 4031)
		{
			if (pMob[clientId].Mobs.Player.GuildLevel != 9 && !IsAdmin)
			{
				SendClientMessage(clientId, "Somente para lader de guild.");

				SendItem(clientId, (SlotType)p->SrcType, p->SrcSlot, srcItem);

				return true;
			}

			if (ChargedGuildList[sServer.Channel - 1][4] != pMob[clientId].Mobs.Player.Guild && !IsAdmin) // Somente caso possua a coroa
			{
				SendClientMessage(clientId, "Necessario possuir coroa.");

				SendItem(clientId, (SlotType)p->SrcType, p->SrcSlot, srcItem);

				return true;
			}

	 

			AmountMinus(srcItem);
			SendItem(clientId, (SlotType)p->SrcType, p->SrcSlot, srcItem);
		}
#pragma endregion

#pragma region MOEDA DE CASH
		else if (itemId == 4533 || itemId == 4534 || itemId == 4535)
		{
			INT32 cash = (itemId - 4533);

			if (cash == 0)
				cash = 200;
			else if (cash == 1)
				cash = 500;
			else if (cash == 2)
				cash = 1000;

			pUser[clientId].User.Cash += cash;

			SendClientMessage(clientId, "Foram adicionados %d Cash em sua conta. Saldo total: %u", cash, pUser[clientId].User.Cash);
			AmountMinus(srcItem);

			SendSignalParm(clientId, 0x7530, RefreshGoldPacket, User.Cash);
		}
#pragma endregion
#pragma region NUCLEO DE LAC
		else if (itemId == NUCLEO_LAC_ITEMID)
		{
			if (srcItem->stEffect[0].cEffect != EF_AMOUNT)
			{
				SendItem(clientId, (SlotType)p->SrcType, p->SrcSlot, srcItem);

				SendClientMessage(clientId, "Nacleo de Lac nao inicializado.");
				return true;
			}

			int rand = Rand() % 100;
			if (rand > srcItem->stEffect[0].cValue && srcItem->stEffect[0].cValue != 100)
			{
				memset(srcItem, 0, sizeof(STRUCT_ITEM));

				SendItem(clientId, (SlotType)p->SrcType, p->SrcSlot, srcItem);
				SendClientMessage(clientId, "Falha na composiaao.");
				return true;
			}

			// Deleta o nacleo.
			memset(srcItem, 0, sizeof(STRUCT_ITEM));
			SendItem(clientId, (SlotType)p->SrcType, p->SrcSlot, srcItem);

			// Entrega o lac 100%
			STRUCT_ITEM itemLac100;
			memset(&itemLac100, 0, sizeof(STRUCT_ITEM));

			itemLac100.sIndex = 4141;

			PutItem(clientId, &itemLac100);

			SendClientMessage(clientId, "Item composto com sucesso!");

			return true;
		}
#pragma endregion
#pragma region FLASH DO VALE
		else if (itemId == 4688)
		{
			int fadaId = pMob[clientId].Mobs.Player.Equip[13].sIndex;
			if (fadaId != 3916)
			{
				SendClientMessage(clientId, "Somente disponavel para Fada do Vale");

				SendItem(clientId, SlotType::Inv, p->SrcSlot, srcItem);
				return true;
			}

			Log(clientId, LOG_INGAME, "Recebido %s por usar o %s em %s (%s)", g_pItemList[3917].ItemName, g_pItemList[itemId].ItemName, g_pItemList[fadaId].ItemName, pMob[clientId].Mobs.Player.Equip[13].toString().c_str());
			pMob[clientId].Mobs.Player.Equip[13].sIndex = 3917;

			SendItem(clientId, SlotType::Equip, 13, &pMob[clientId].Mobs.Player.Equip[13]);

			AmountMinus(srcItem);
			return true;
		}
#pragma endregion
#pragma region FLASH PRATEADO / DOURADO
		else if (itemId == 3451 || itemId == 3452)
		{
			int fadaId = pMob[clientId].Mobs.Player.Equip[13].sIndex;
			if (fadaId != 3902 && fadaId != 3905 && fadaId != 3908)
			{
				SendClientMessage(clientId, "Somente disponavel para Fada Vermelha");

				SendItem(clientId, SlotType::Inv, p->SrcSlot, srcItem);
				return true;
			}

			int destinyId = 3914;
			if (srcItem->sIndex == 3452)
				destinyId = 3915;

			Log(clientId, LOG_INGAME, "Recebido %s por usar o %s em %s (%s)", g_pItemList[destinyId].ItemName, g_pItemList[itemId].ItemName, g_pItemList[fadaId].ItemName, pMob[clientId].Mobs.Player.Equip[13].toString().c_str());
			pMob[clientId].Mobs.Player.Equip[13].sIndex = destinyId;

			SendItem(clientId, SlotType::Equip, 13, &pMob[clientId].Mobs.Player.Equip[13]);

			AmountMinus(srcItem);
			return true;
		}
#pragma endregion
#pragma region BAa MERCANTIL
		else if (itemId == 4545)
		{
			INT32 key = GetFirstSlot(clientId, 4546);
			if (key == -1)
			{
				if (pMob[clientId].Mobs.Player.Coin < 10000000)
				{
					SendClientMessage(clientId, "Necessario 10 milhaes de gold ou Chave Mercantil!");

					SendItem(clientId, SlotType::Inv, p->SrcSlot, srcItem);
					return true;
				}

				pMob[clientId].Mobs.Player.Coin -= 10000000;
				SendSignalParm(clientId, clientId, 0x3AF, pMob[clientId].Mobs.Player.Coin);
			}
			else
			{
				AmountMinus(&pMob[clientId].Mobs.Player.Inventory[key]);

				SendItem(clientId, SlotType::Inv, key, &pMob[clientId].Mobs.Player.Inventory[key]);
			}

			INT32 _rand = Rand() % 100;

			STRUCT_ITEM item;
			memset(&item, 0, sizeof item);

			if (_rand <= 50)
			{
				_rand = Rand() % 2;

				if (_rand == 0)
					item.sIndex = 4547;
				else if (_rand == 1)
				{
					item.sIndex = 412;

					item.EF1 = EF_AMOUNT;
					item.EFV1 = 5;
				}
			}
			else if (_rand <= 90)
			{
				_rand = Rand() % 3;

				if (_rand == 0)
				{
					item.sIndex = 413;

					item.EF1 = EF_AMOUNT;
					item.EFV1 = 2;
				}
				else if (_rand == 1)
				{
					item.sIndex = 404;

					item.EF1 = EF_AMOUNT;
					item.EFV1 = 50;
				}
				else if (_rand == 2)
				{
					item.sIndex = 409;

					item.EF1 = EF_AMOUNT;
					item.EFV1 = 50;
				}
			}
			else
			{
				_rand = Rand() % 1;

				if (_rand == 0)
				{
					item.sIndex = 4547;

					item.EF1 = EF_AMOUNT;
					item.EFV1 = 3;
				}
			}

			AmountMinus(srcItem);

			if (PutItem(clientId, &item))
			{
				SendClientMessage(clientId, "!Chegou um item: [ %s ]", g_pItemList[item.sIndex].ItemName);

				Log(clientId, LOG_INGAME, "Recebeu o item %s [%d] da Caixa Mercantil!", g_pItemList[item.sIndex].ItemName, item.sIndex);
			}
			else
			{
				SendClientMessage(clientId, "!Nao recebeu o item por falta de espaao!");

				Log(clientId, LOG_INGAME, "NaO Recebeu o item %s [%d] da Caixa Mercantil! - FALTA DE ESPAaO NO INVENTaRIO", g_pItemList[item.sIndex].ItemName, item.sIndex);
			}
		}
#pragma endregion 
#pragma region TROCA DE CORPOS
		else if (itemId == 4649)
		{
			auto& player = pMob[clientId].Mobs.Player;
			for(int i = 1; i < 8; ++i)
			{
				if (player.Equip[i].sIndex != 0)
				{
					SendClientMessage(clientId, "Necessario desequipar todos os seus itens");

					SendItem(clientId, (SlotType)p->SrcType, p->SrcSlot, srcItem);
					return true;
				}
			}

			if (player.Equip[0].EFV2 <= ARCH)
			{
				SendClientMessage(clientId, "Disponavel apenas para celestiais ou superior");

				SendItem(clientId, (SlotType)p->SrcType, p->SrcSlot, srcItem);
				return true;
			}

			int sephirotId = player.Equip[11].sIndex;
			if (sephirotId < 1760 || sephirotId > 1763)
			{
				SendClientMessage(clientId, "Necessario a sephirot da classe que deseja o novo corpo");

				SendItem(clientId, (SlotType)p->SrcType, p->SrcSlot, srcItem);
				return true;
			}

			sephirotId -= 1760;

			{
				int newFace = (sephirotId * 10) + (player.Class + 6);
				int oldFace = player.Equip[0].sIndex;

				if (newFace == oldFace)
				{
					SendClientMessage(clientId, "Nao a possavel trocar para a mesma classe");

					SendItem(clientId, (SlotType)p->SrcType, p->SrcSlot, srcItem);
					return true;
				}

				player.Equip[0].sIndex = newFace;
				player.Equip[0].EF2 = newFace;
				Log(clientId, LOG_INGAME, "trocou o corpo de %d para %d (%hhu) - Evoluaao: %hhu", oldFace, newFace, pMob[clientId].Mobs.Player.Class, pMob[clientId].Mobs.Player.Equip[0].EFV2);
			}

			bool hasSub = pMob[clientId].Mobs.Sub.CurrentScore == 1 || player.Equip[0].EFV2 == SUBCELESTIAL;
			if (hasSub)
			{
				auto& sub = pMob[clientId].Mobs.Sub;

				int baseFace = sub.Equip[0].EF2;
				int classInfo = 0;
				if (baseFace < 10)
					classInfo = (baseFace - 6);
				else if (baseFace < 20)
					classInfo = (baseFace - 16);
				else if (baseFace < 30)
					classInfo = (baseFace - 26);
				else if (baseFace < 40)
					classInfo = (baseFace - 36);

				int newFace = (sephirotId * 10) + (classInfo + 6);
				int oldFace = sub.Equip[0].sIndex;

				sub.Equip[0].sIndex = newFace;
				sub.Equip[0].EF2 = newFace;

				Log(clientId, LOG_INGAME, "trocou o corpo de %d para %d (%hhu) - Evoluaao: %hhu", oldFace, newFace, classInfo, sub.Equip[0].EFV2);
			}

			AmountMinus(srcItem);
			player.Equip[11] = STRUCT_ITEM{};

			SendClientMessage(clientId, "Que os Deuses continuem te abenaoando");
			CharLogOut(clientId);

			SendSignalParm(clientId, clientId, 0x3B4, pUser[clientId].inGame.CharSlot);
			return true;
		}
#pragma endregion
#pragma region TROCA DE CLASSE 
		else if (itemId == 3486)
		{
			if (pMob[clientId].Mobs.Player.Equip[0].EFV2 < ARCH)
			{
				SendClientMessage(clientId, "Nao disponavel para mortais!");

				SendItem(clientId, (SlotType)p->SrcType, p->SrcSlot, srcItem);
				return true;
			}

			STRUCT_ITEM* seph = &pMob[clientId].Mobs.Player.Equip[11];
			if (seph->sIndex < 1760 || seph->sIndex > 1763)
			{
				SendClientMessage(clientId, "Necessario o sephirot da classe que deseja.");

				SendItem(clientId, (SlotType)p->SrcType, p->SrcSlot, srcItem);
				return true;
			}

			INT32 newClass = seph->sIndex - 1760;
			INT32 myFace = pMob[clientId].Mobs.Player.Equip[0].EF2 - pMob[clientId].Mobs.Player.Class;

			switch (myFace)
			{
			case 6:
			case 16:
			case 26:
			case 36:
				break;
			default:
				SendClientMessage(clientId, "Incorreto. Contate a administraaao #0001");

				SendItem(clientId, (SlotType)p->SrcType, p->SrcSlot, srcItem);
				return true;
			}

			int newFace = myFace + newClass;
			int oldFace = pMob[clientId].Mobs.Player.Equip[0].sIndex;
			int oldClass = pMob[clientId].Mobs.Player.Class;

			if (newFace == oldFace)
			{
				SendClientMessage(clientId, "Nao a possavel trocar para a mesma classe");

				SendItem(clientId, (SlotType)p->SrcType, p->SrcSlot, srcItem);
				return true;
			}

			pMob[clientId].Mobs.Player.Class = newClass;

			pMob[clientId].Mobs.Player.Equip[0].sIndex = newFace;
			pMob[clientId].Mobs.Player.Equip[0].EF2 = newFace;

			memset(seph, 0, sizeof STRUCT_ITEM);
			AmountMinus(srcItem);

			Log(clientId, LOG_INGAME, "%s trocou a classe de %d (%d) para %d (%hhu) - %hhu", pMob[clientId].Mobs.Player.MobName, oldFace, oldClass, newFace, pMob[clientId].Mobs.Player.Class, pMob[clientId].Mobs.Player.Equip[0].EFV2);

			SendClientMessage(clientId, "Que os Deuses continuem te abenaoando");
			CharLogOut(clientId);

			SendSignalParm(clientId, clientId, 0x3B4, pUser[clientId].inGame.CharSlot);
			return true;
		}
#pragma endregion
#pragma region CARTA DE DUELO
		else if (itemId == 3172 || itemId == 1731 || itemId == 3171)
		{
			auto event = static_cast<CEventItem*>(CEventManager::GetInstance().GetEvent(eEventType::DuelLetter));
			if (event == nullptr)
			{
				SendItem(clientId, (SlotType)p->SrcType, p->SrcSlot, srcItem);

				return true;
			}

			if (pMob[clientId].Target.X < 1045 || pMob[clientId].Target.X > 1047 || pMob[clientId].Target.Y < 1689 || pMob[clientId].Target.Y > 1691)
			{
				SendClientMessage(clientId, "Utilize no Altar de Thor!");

				SendItem(clientId, (SlotType)p->SrcType, p->SrcSlot, srcItem);
				return true;
			}

			if (!event->CanRegister(*this))
			{
				SendClientMessage(clientId, "Quest ja ocupada");

				SendItem(clientId, (SlotType)p->SrcType, p->SrcSlot, srcItem);
				return true;
			}

			if (pMob[clientId].Leader != 0)
			{
				SendClientMessage(clientId, "Somente o lader do grupo pode utilizar");

				SendItem(clientId, (SlotType)p->SrcType, p->SrcSlot, srcItem);
				return true;
			}

			if (event->Register(*this, srcItem))
			{
				Log(clientId, LOG_INGAME, "Registrado na Carta de duelo");
				AmountMinus(srcItem);
			}
		}
#pragma endregion
#pragma region REMOVEDOR DE TRAJE DE MONTARIA
		else if (itemId == 4595)
		{
			STRUCT_ITEM& item = pMob[clientId].Mobs.Player.Equip[14];
			INT32 traje = item.stEffect[2].cValue;

			if (traje < 11 || item.sIndex < 2360 || item.sIndex >= 2390)
			{
				SendClientMessage(clientId, "Utilize com uma montaria utilizando traje");

				SendItem(clientId, (SlotType)p->SrcType, p->SrcSlot, srcItem);
				return true;
			}

			if (traje != 12 && traje != 13 && traje != 14 && traje != 15 && traje != 16 && traje != 20)
			{
				SendClientMessage(clientId, "Voca nao pode remover este traje");

				SendItem(clientId, (SlotType)p->SrcType, p->SrcSlot, srcItem);
				return true;
			}

			int slotId = -1;
			bool remove = true;
			if (GetItemAmount(srcItem) == 1)
			{
				slotId = p->SrcSlot;
				remove = false;
			}
			else
				slotId = GetFirstSlot(clientId, 0);

			if (slotId == -1)
			{
				SendClientMessage(clientId, "Sem espaao no inventario");

				SendItem(clientId, SlotType::Inv, p->SrcSlot, srcItem);
				return true;
			}

			Log(clientId, LOG_INGAME, "Removido traje da montaria. Index do traje que havia: %d. Item info: %s", traje, item.toString().c_str());

			item.stEffect[2].cValue = 0;

			memset(&pMob[clientId].Mobs.Player.Inventory[slotId], 0, sizeof STRUCT_ITEM);
			pMob[clientId].Mobs.Player.Inventory[slotId].sIndex = 4190 + (traje - 11);

			SendItem(clientId, SlotType::Equip, 14, &pMob[clientId].Mobs.Player.Equip[14]);
			SendItem(clientId, SlotType::Inv, slotId, &pMob[clientId].Mobs.Player.Inventory[slotId]);

			if (remove)
				AmountMinus(srcItem);

			return true;
		}
#pragma endregion
#pragma region POaaO GELADA
		else if (itemId == 4624)
		{
			INT32 slot = -1;
			for (INT32 i = 0; i < 32; i++)
			{
				if (pMob[clientId].Mobs.Affects[i].Index == 54)
				{
					slot = i;

					break;
				}
			}

			if (slot == -1)
			{
				for (INT32 i = 0; i < 32; i++)
				{
					if (pMob[clientId].Mobs.Affects[i].Index == 0)
					{
						slot = i;

						break;
					}
				}
			}

			if (slot == -1)
			{
				SendItem(clientId, (SlotType)p->SrcType, p->SrcSlot, srcItem);

				return true;
			}

			pMob[clientId].Mobs.Affects[slot].Index = 54;
			pMob[clientId].Mobs.Affects[slot].Time = 2700; // = 6hrs
			pMob[clientId].Mobs.Affects[slot].Value = 1;

			p364 packet;
			GetCreateMob(clientId, (BYTE*)&packet);

			GridMulticast_2(pMob[clientId].Target.X, pMob[clientId].Target.Y, (BYTE*)&packet, 0);

			SendAffect(clientId);
			AmountMinus(srcItem);
		}
#pragma endregion
#pragma region POaaO ANNUBIS
		else if (itemId == 4668)
		{
			INT32 slot = -1;
			for (INT32 i = 0; i < 32; i++)
			{
				if (pMob[clientId].Mobs.Affects[i].Index == 59)
				{
					slot = i;

					break;
				}
			}

			if (slot == -1)
			{
				for (INT32 i = 0; i < 32; i++)
				{
					if (pMob[clientId].Mobs.Affects[i].Index == 0)
					{
						slot = i;

						break;
					}
				}
			}

			if (slot == -1)
			{
				SendItem(clientId, (SlotType)p->SrcType, p->SrcSlot, srcItem);

				return true;
			}

			pMob[clientId].Mobs.Affects[slot].Index = 59;
			pMob[clientId].Mobs.Affects[slot].Time = 2700; // = 6hrs
			pMob[clientId].Mobs.Affects[slot].Value = 1;

			p364 packet;
			GetCreateMob(clientId, (BYTE*)&packet);

			GridMulticast_2(pMob[clientId].Target.X, pMob[clientId].Target.Y, (BYTE*)&packet, 0);

			SendAffect(clientId);
			AmountMinus(srcItem);
		}
#pragma endregion
#pragma region PANQUECA
		else if (itemId == 4639)
		{
			int slot = GetEmptyAffect(clientId, 55);
			if (slot == -1)
			{
				SendItem(clientId, (SlotType)p->SrcType, p->SrcSlot, srcItem);

				return true;
			}

			pMob[clientId].Mobs.Affects[slot] = STRUCT_AFFECT{};
			pMob[clientId].Mobs.Affects[slot].Index = 55;
			pMob[clientId].Mobs.Affects[slot].Time = 900; // = 6hrs
			pMob[clientId].Mobs.Affects[slot].Value = 1;

			p364 packet{};
			GetCreateMob(clientId, (BYTE*)&packet);

			GridMulticast_2(pMob[clientId].Target.X, pMob[clientId].Target.Y, (BYTE*)&packet, 0);

			SendAffect(clientId);
			AmountMinus(srcItem);
		}
#pragma endregion
#pragma region BAa CONGELADO
		else if (itemId == 4541)
		{
			struct FrozenBox
			{
				int ItemIndex;
				int Amount;
				bool Splitted{ false };
			};

			if (GetInventoryAmount(clientId, 0) < 3)
			{
				SendClientMessage(clientId, "No manimo 3 slots vagos no inventario para usar este item");

				Log(clientId, LOG_INGAME, "Sem espaao no inventario para recebimento. Manimo de 3 itens");
				SendItem(clientId, SlotType::Inv, p->SrcSlot, srcItem);
				return true;
			}

			int slotId = -1;

			bool remove = true;
			if (GetItemAmount(srcItem) == 1)
			{
				slotId = p->SrcSlot;
				remove = false;
			}
			else
				slotId = GetFirstSlot(clientId, 0);

			if (slotId == -1)
			{
				SendClientMessage(clientId, "Sem espaao no inventario");

				Log(clientId, LOG_INGAME, "Sem espaao no inventario para recebimento");
				SendItem(clientId, SlotType::Inv, p->SrcSlot, srcItem);
				return true;
			}

			std::vector<FrozenBox> items
			{
				{
				{ 4542, 1 },
				{ 4140, 5 },
				{ 3314, 3 },
				{ 4538, 1 },
				{ 4522, 3, true },
				{ 4028, 1 }
				}
			};

			std::random_device rd;
			std::mt19937 g(rd());
			std::shuffle(items.begin(), items.end(), g);

			auto& earnedItem = items.front();
			STRUCT_ITEM* item = &pMob[clientId].Mobs.Player.Inventory[slotId];

			*item = STRUCT_ITEM{};
			item->sIndex = earnedItem.ItemIndex;

			if (earnedItem.Amount > 1)
			{
				if (!earnedItem.Splitted)
				{
					item->stEffect[0].cEffect = EF_AMOUNT;
					item->stEffect[0].cValue = earnedItem.Amount;
				}

				item->stEffect[1].cEffect = EF_NOTRADE;
				item->stEffect[1].cValue = 1;

				if (earnedItem.Splitted)
				{
					for (int i = 0; i < earnedItem.Amount - 1; i++)
					{
						int otherSlotId = GetFirstSlot(clientId, 0);
						if (otherSlotId == -1)
						{
							Log(clientId, LOG_INGAME, "Nao encontrou slot para entrega do item");

							SendClientMessage(clientId, "Um problema aconteceu. Contate a administraaao");
							continue;
						}

						STRUCT_ITEM* otherItem = &pMob[clientId].Mobs.Player.Inventory[otherSlotId];

						*otherItem = STRUCT_ITEM{};
						otherItem->sIndex = earnedItem.ItemIndex;

						otherItem->stEffect[0].cEffect = EF_NOTRADE;
						otherItem->stEffect[0].cValue = 1;

						Log(clientId, LOG_INGAME, "Recebido item %s %s no slot %d", g_pItemList[otherItem->sIndex].ItemName, otherItem->toString().c_str(), otherSlotId);
						SendItem(clientId, SlotType::Inv, otherSlotId, otherItem);
					}
				}
			}
			else
			{
				if (item->sIndex != 4542)
				{
					item->stEffect[0].cEffect = EF_NOTRADE;
					item->stEffect[0].cValue = 1;
				}
			}

			SendItem(clientId, SlotType::Inv, slotId, item);
			Log(clientId, LOG_INGAME, "Recebido item %s %s", g_pItemList[item->sIndex].ItemName, item->toString().c_str());

			if (remove)
				AmountMinus(srcItem);

			return true;
		}
#pragma endregion
#pragma region LIVRO AMALDIaOADO CAVEIRA
		else if (itemId == 4623)
		{
			struct CursedBook
			{
				int ItemIndex;
				int Amount;
			};

			int slotId = -1;

			bool remove = true;
			if (GetItemAmount(srcItem) == 1)
			{
				slotId = p->SrcSlot;
				remove = false;
			}
			else
				slotId = GetFirstSlot(clientId, 0);

			if (slotId == -1)
			{
				SendClientMessage(clientId, "Sem espaao no inventario");

				Log(clientId, LOG_INGAME, "Sem espaao no inventario para recebimento");
				SendItem(clientId, SlotType::Inv, p->SrcSlot, srcItem);
				return true;
			}

			std::vector<CursedBook> items =
			{
				{
					{ 4522, 3},
					{ 4140, 5},
					{ 3314, 3},
					{ 4627, 1},
					{ 4624, 1},
					{ 777, 5},
					{ 4028, 1},
					{ 4633, 1}
				}
			};

			std::random_device rd;
			std::mt19937 g(rd());
			std::shuffle(items.begin(), items.end(), g);

			auto& earnedItem = items.front();
			STRUCT_ITEM* item = &pMob[clientId].Mobs.Player.Inventory[slotId];

			*item = STRUCT_ITEM{};
			item->sIndex = earnedItem.ItemIndex;

			if (earnedItem.Amount > 1)
			{
				item->stEffect[0].cEffect = EF_AMOUNT;
				item->stEffect[0].cValue = earnedItem.Amount;

				item->stEffect[1].cEffect = EF_NOTRADE;
				item->stEffect[1].cValue = 1;
			}
			else
			{
				item->stEffect[0].cEffect = EF_NOTRADE;
				item->stEffect[0].cValue = 1;
			}

			SendItem(clientId, SlotType::Inv, slotId, item);
			Log(clientId, LOG_INGAME, "Recebido item %s %s", g_pItemList[item->sIndex].ItemName, item->toString().c_str());

			if (remove)
				AmountMinus(srcItem);

		}
#pragma endregion
#pragma region BAa DO SUKUR
		else if (itemId == 4647)
		{
			struct CursedBook
			{
				int ItemIndex;
				int Amount;
			};

			int slotId = -1;

			bool remove = true;
			if (GetItemAmount(srcItem) == 1)
			{
				slotId = p->SrcSlot;
				remove = false;
			}
			else
				slotId = GetFirstSlot(clientId, 0);

			if (slotId == -1)
			{
				SendClientMessage(clientId, "Sem espaao no inventario");

				Log(clientId, LOG_INGAME, "Sem espaao no inventario para recebimento");
				SendItem(clientId, SlotType::Inv, p->SrcSlot, srcItem);
				return true;
			}

			std::vector<CursedBook> items =
			{
				{
					{ 2411, 20},
					{ 2412, 20},
					{ 2413, 20},
					{ 3182, 10},
					{ 3182, 10},
					{ 3326, 1},
					{ 4140, 10},
					{ 4140, 10},
					{ 671, 1},
					{ 4633, 1}
				}
			};

			std::random_device rd;
			std::mt19937 g(rd());
			std::shuffle(items.begin(), items.end(), g);

			auto& earnedItem = items.front();
			STRUCT_ITEM* item = &pMob[clientId].Mobs.Player.Inventory[slotId];

			*item = STRUCT_ITEM{};
			item->sIndex = earnedItem.ItemIndex;

			if (earnedItem.Amount > 1)
			{
				item->stEffect[0].cEffect = EF_AMOUNT;
				item->stEffect[0].cValue = earnedItem.Amount;

				item->stEffect[1].cEffect = EF_NOTRADE;
				item->stEffect[1].cValue = 1;
			}
			else
			{
				item->stEffect[0].cEffect = EF_NOTRADE;
				item->stEffect[0].cValue = 1;
			}

			SendItem(clientId, SlotType::Inv, slotId, item);
			Log(clientId, LOG_INGAME, "Recebido item %s %s", g_pItemList[item->sIndex].ItemName, item->toString().c_str());

			if (remove)
				AmountMinus(srcItem);
		}
#pragma endregion
#pragma region LIVRO ANNUBIS
		else if (itemId == 4657)
		{
			struct AnnubisBook
			{
				int ItemIndex;
				int Amount;
			};

			int slotId = -1;

			bool remove = true;
			if (GetItemAmount(srcItem) == 1)
			{
				slotId = p->SrcSlot;
				remove = false;
			}
			else
				slotId = GetFirstSlot(clientId, 0);

			if (slotId == -1)
			{
				SendClientMessage(clientId, "Sem espaao no inventario");

				Log(clientId, LOG_INGAME, "Sem espaao no inventario para recebimento");
				SendItem(clientId, SlotType::Inv, p->SrcSlot, srcItem);
				return true;
			}

			std::vector<AnnubisBook> items =
			{
				{
					{ 2411, 30 },
					{ 2412, 30 },
					{ 2413, 30 },
					{ 3326, 1 },
					{ 4633, 2 },
					{ 4140, 10 },
					{ 4140, 10 },
					{ 4854, 1 },
					{ 3210, 1 },
					{ 4665, 10 },
					{ 633, 1 }
				}
			};

			std::random_device rd;
			std::mt19937 g(rd());
			std::shuffle(items.begin(), items.end(), g);

			auto& earnedItem = items.front();
			STRUCT_ITEM* item = &pMob[clientId].Mobs.Player.Inventory[slotId];

			*item = STRUCT_ITEM{};
			item->sIndex = earnedItem.ItemIndex;

			if (earnedItem.Amount > 1)
			{
				item->stEffect[0].cEffect = EF_AMOUNT;
				item->stEffect[0].cValue = earnedItem.Amount;

				item->stEffect[1].cEffect = EF_NOTRADE;
				item->stEffect[1].cValue = 1;
			}
			else
			{
				item->stEffect[0].cEffect = EF_NOTRADE;
				item->stEffect[0].cValue = 1;
			}

			SendItem(clientId, SlotType::Inv, slotId, item);
			Log(clientId, LOG_INGAME, "Recebido item %s %s", g_pItemList[item->sIndex].ItemName, item->toString().c_str());

			if (remove)
				AmountMinus(srcItem);
		}
#pragma endregion
#pragma region LIVRO POSEIDON
		else if (itemId == 4667)
		{
			struct PoseidonBook
			{
				int ItemIndex;
				int Amount;
			};

			int slotId = -1;

			bool remove = true;
			if (GetItemAmount(srcItem) == 1)
			{
				slotId = p->SrcSlot;
				remove = false;
			}
			else
				slotId = GetFirstSlot(clientId, 0);

			if (slotId == -1)
			{
				SendClientMessage(clientId, "Sem espaao no inventario");

				Log(clientId, LOG_INGAME, "Sem espaao no inventario para recebimento");
				SendItem(clientId, SlotType::Inv, p->SrcSlot, srcItem);
				return true;
			}

			std::vector<PoseidonBook> commonItems =
			{
				{ 4633, 3 },
				{ 2411, 30 },
				{ 2412, 30 },
				{ 2413, 30 },
				{ 4672, 1 },
			};

			std::vector<PoseidonBook> unusualItems =
			{
				{2414, 15},
				{2415, 15},
				{2416, 15},
				{4665, 10},
			};

			std::vector<PoseidonBook> rareItems =
			{
				{4679, 1},
				{633, 1},
				{671, 1},
				{4673, 1},
			};

			std::vector<PoseidonBook> epicItems =
			{
				{4854, 1},
			};

			std::vector<PoseidonBook> sortedItems;
			int rand = Rand() % 100;
			if (rand <= 50)
				sortedItems = commonItems;
			else if (rand <= 80)
				sortedItems = unusualItems;
			else if (rand <= 95)
				sortedItems = rareItems;
			else
				sortedItems = epicItems;

			std::random_device rd;
			std::mt19937 g(rd());
			std::shuffle(sortedItems.begin(), sortedItems.end(), g);

			auto& earnedItem = sortedItems.front();
			STRUCT_ITEM* item = &pMob[clientId].Mobs.Player.Inventory[slotId];

			*item = STRUCT_ITEM{};
			item->sIndex = earnedItem.ItemIndex;

			if (earnedItem.Amount > 1)
			{
				item->stEffect[0].cEffect = EF_AMOUNT;
				item->stEffect[0].cValue = earnedItem.Amount;
			}

			SendItem(clientId, SlotType::Inv, slotId, item);
			Log(clientId, LOG_INGAME, "Recebido item %s %s", g_pItemList[item->sIndex].ItemName, item->toString().c_str());

			if (remove)
				AmountMinus(srcItem);
		}
	#pragma endregion
#pragma region LIVRO JUNINO
		else if (itemId == 4715)
		{
			struct JuninoBook
			{
				int ItemIndex;
				int Amount;
				int Ef1;
				int Efv1;
			};

			int slotId = -1;

			bool remove = true;
			if (GetItemAmount(srcItem) == 1)
			{
				slotId = p->SrcSlot;
				remove = false;
			}
			else
				slotId = GetFirstSlot(clientId, 0);

			if (slotId == -1)
			{
				SendClientMessage(clientId, "Sem espaao no inventario");

				Log(clientId, LOG_INGAME, "Sem espaao no inventario para recebimento");
				SendItem(clientId, SlotType::Inv, p->SrcSlot, srcItem);
				return true;
			}

			std::vector<JuninoBook> commonItems =
			{
				{ 2414, 30 },
				{ 2415, 30 },
				{ 2416, 30 },
				{ 4708, 1 },
			};

			std::vector<JuninoBook> unusualItems =
			{
				{4679, 1 },
				{4851, 1 },
				{3338, 1 },
				{4665, 10 },
			};

			std::vector<JuninoBook> rareItems =
			{
				{633, 1, EF_HPADD2, 8},
				{633, 1, EF_MPADD2, 8},
				{633, 1, EF_CRITICAL, 40},
				{4709, 1},
			};

			std::vector<JuninoBook> epicItems =
			{
				{4854, 1},
			};

			std::vector<JuninoBook> sortedItems;
			int rand = Rand() % 100;
			if (rand <= 50)
				sortedItems = commonItems;
			else if (rand <= 80)
				sortedItems = unusualItems;
			else if (rand <= 95)
				sortedItems = rareItems;
			else
				sortedItems = epicItems;

			std::random_device rd;
			std::mt19937 g(rd());
			std::shuffle(sortedItems.begin(), sortedItems.end(), g);

			auto& earnedItem = sortedItems.front();
			STRUCT_ITEM* item = &pMob[clientId].Mobs.Player.Inventory[slotId];

			*item = STRUCT_ITEM{};
			item->sIndex = earnedItem.ItemIndex;

			if (earnedItem.Amount > 1)
			{
				item->stEffect[0].cEffect = EF_AMOUNT;
				item->stEffect[0].cValue = earnedItem.Amount;
			}

			if (earnedItem.Ef1 != 0)
			{
				item->stEffect[0].cEffect = earnedItem.Ef1;
				item->stEffect[0].cValue = earnedItem.Efv1;
			}

			SendItem(clientId, SlotType::Inv, slotId, item);
			Log(clientId, LOG_INGAME, "Recebido item %s %s", g_pItemList[item->sIndex].ItemName, item->toString().c_str());

			SendClientMessage(clientId, "!Chegou um item: [%s]", g_pItemList[item->sIndex].ItemName);
			if (remove)
				AmountMinus(srcItem);
		}
#pragma endregion
#pragma region LIVRO DO BONE DEMON
        else if (itemId == 4724)
        {
            struct BoneBook
            {
                int ItemIndex;
                int Amount;
                int Ef1;
                int Efv1;
            };

            int slotId = -1;

            bool remove = true;
            if (GetItemAmount(srcItem) == 1)
            {
                slotId = p->SrcSlot;
                remove = false;
            }
            else
                slotId = GetFirstSlot(clientId, 0);

            if (slotId == -1)
            {
                SendClientMessage(clientId, "Sem espaao no inventario");

                Log(clientId, LOG_INGAME, "Sem espaao no inventario para recebimento");
                SendItem(clientId, SlotType::Inv, p->SrcSlot, srcItem);
                return true;
            }

            if (GetInventoryAmount(clientId, 0) < 2)
            {
                SendClientMessage(clientId, "Voca precisa de no manimo dois slots vagos no inventario");

                Log(clientId, LOG_INGAME, "Sem espaao no inventario para recebimento");
                SendItem(clientId, SlotType::Inv, p->SrcSlot, srcItem);
                return true;
            }

            std::vector<BoneBook> commonItems =
            {
                { 2414, 30 },
                { 2415, 30 },
                { 2416, 30 },
                { 1731, 1 },
                { 4727, 1 },
            };

            std::vector<BoneBook> unusualItems =
            {
                {3338, 1, 43, 1 },
                {4665, 10 },
                {4864, 1, 43, 1},
                {4679, 1 },
                {3182, 15}
            };

            std::vector<BoneBook> rareItems =
            {
                {633, 1, EF_HPADD2, 8},
                {633, 1, EF_MPADD2, 8},
                {633, 1, EF_CRITICAL, 40},
                {4728, 1},
            };

            std::vector<BoneBook> epicItems =
            {
                {3338, 1, 43, 9},
                {3338, 2, 43, 4},
            };

            std::vector<BoneBook> sortedItems;
            int rand = Rand() % 100;
            if (rand <= 50)
                sortedItems = commonItems;
            else if (rand <= 80)
                sortedItems = unusualItems;
            else if (rand <= 95)
                sortedItems = rareItems;
            else
                sortedItems = epicItems;

            std::random_device rd;
            std::mt19937 g(rd());
            std::shuffle(sortedItems.begin(), sortedItems.end(), g);

            auto& earnedItem = sortedItems.front();
            STRUCT_ITEM* item = &pMob[clientId].Mobs.Player.Inventory[slotId];

            *item = STRUCT_ITEM{};
            item->sIndex = earnedItem.ItemIndex;

            if (earnedItem.ItemIndex != 3338)
            {
                if (earnedItem.Amount > 1)
                {
                    item->stEffect[0].cEffect = EF_AMOUNT;
                    item->stEffect[0].cValue = earnedItem.Amount;
                }

                if (earnedItem.Ef1 != 0)
                {
                    item->stEffect[0].cEffect = earnedItem.Ef1;
                    item->stEffect[0].cValue = earnedItem.Efv1;
                }
            }
            else
            {
                for (int i = 0; i < earnedItem.Amount; ++i)
                {
                    if (i == 0)
                    {
                        item->stEffect[0].cEffect = earnedItem.Ef1;
                        item->stEffect[0].cValue = earnedItem.Efv1;
                    }
                    else
                    {
                        int otherSlotId = GetFirstSlot(clientId, 0);
                        auto otherItem = &pMob[clientId].Mobs.Player.Inventory[otherSlotId];

                        *otherItem = STRUCT_ITEM{};
                        otherItem->sIndex = 3338;
                        otherItem->stEffect[0].cEffect = earnedItem.Ef1;
                        otherItem->stEffect[0].cValue = earnedItem.Efv1;

                        SendItem(clientId, SlotType::Inv, otherSlotId, otherItem);
                    }
                }
            }
            SendItem(clientId, SlotType::Inv, slotId, item);
            Log(clientId, LOG_INGAME, "Recebido item %s %s", g_pItemList[item->sIndex].ItemName, item->toString().c_str());

            SendClientMessage(clientId, "!Chegou um item: [%s]", g_pItemList[item->sIndex].ItemName);
            if (remove)
                AmountMinus(srcItem);
        }
#pragma endregion
#pragma region CAIXA MYTRIL
		else if (itemId == 4633)
		{
			int slotId = -1;

			bool remove = true;
			if (GetItemAmount(srcItem) == 1)
			{
				slotId = p->SrcSlot;
				remove = false;
			}
			else
				slotId = GetFirstSlot(clientId, 0);

			if (slotId == -1)
			{
				SendClientMessage(clientId, "Sem espaao no inventario");

				SendItem(clientId, SlotType::Inv, p->SrcSlot, srcItem);
				return true;
			}

			STRUCT_ITEM* item = &pMob[clientId].Mobs.Player.Inventory[slotId];

			*item = STRUCT_ITEM{};
			item->sIndex = 3027 + (Rand() % 4);

			SendItem(clientId, SlotType::Inv, slotId, item);

			Log(clientId, LOG_INGAME, "Recebido o item %s %s", g_pItemList[item->sIndex].ItemName, item->toString().c_str());
			if (remove)
				AmountMinus(srcItem);
		}
#pragma endregion
#pragma region CAIXA DE ARMAS D
		else if (itemId == 4631)
		{
			int slotId = -1;

			bool remove = true;
			if (GetItemAmount(srcItem) == 1)
			{
				slotId = p->SrcSlot;
				remove = false;
			}
			else
				slotId = GetFirstSlot(clientId, 0);

			if (slotId == -1)
			{
				SendClientMessage(clientId, "Sem espaao no inventario");

				SendItem(clientId, SlotType::Inv, p->SrcSlot, srcItem);
				return true;
			}

			std::array damageWeapons = { 824, 825, 839, 840, 870, 884, 885, 911, 809, 810 };
			std::array magicWeapons = { 854, 855, 902, 900 };

			int type = Rand() % 50;
			int addId = 0;
			int addValue = 0;
			int itemId = 0;

			if (type <= 25)
			{
				std::array damageValue = { 54, 54, 54, 54, 63, 63, 63, 72 };

				std::random_device rd;
				std::mt19937 g(rd());
				std::shuffle(damageValue.begin(), damageValue.end(), g);
				addId = EF_DAMAGE;
				addValue = damageValue.front();

				std::shuffle(damageWeapons.begin(), damageWeapons.end(), g);
				itemId = damageWeapons.front();
			}
			else
			{
				std::array magicValue = { 24, 24, 24, 24, 28, 28, 28, 32 };

				std::random_device rd;
				std::mt19937 g(rd());
				std::shuffle(magicValue.begin(), magicValue.end(), g);

				addId = EF_MAGIC;
				addValue = magicValue.front();

				std::shuffle(magicWeapons.begin(), magicWeapons.end(), g);
				itemId = magicWeapons.front();
			}

			STRUCT_ITEM* item = &pMob[clientId].Mobs.Player.Inventory[slotId];

			*item = STRUCT_ITEM{};
			item->sIndex = itemId;
			item->stEffect[0].cEffect = EF_SANC;
			item->stEffect[0].cValue = 0;
			item->stEffect[1].cEffect = addId;
			item->stEffect[1].cValue = addValue;

			SendItem(clientId, SlotType::Inv, slotId, item);

			Log(clientId, LOG_INGAME, "Recebido o item %s %s", g_pItemList[item->sIndex].ItemName, item->toString().c_str());
			if (remove)
				AmountMinus(srcItem);

			return true;
		}
#pragma endregion
#pragma region CAIXA DE ARMAS E
		else if (itemId == 4632)
		{
			int slotId = -1;

			bool remove = true;
			if (GetItemAmount(srcItem) == 1)
			{
				slotId = p->SrcSlot;
				remove = false;
			}
			else
				slotId = GetFirstSlot(clientId, 0);

			if (slotId == -1)
			{
				SendClientMessage(clientId, "Sem espaao no inventario");

				SendItem(clientId, SlotType::Inv, p->SrcSlot, srcItem);
				return true;
			}

			std::array damageWeapons = { 3551, 3556, 3561, 3571, 3576, 3591, 3596 };
			std::array magicWeapons = { 3566, 3581, 3582 };

			int type = Rand() % 50;
			int addId = 0;
			int addValue = 0;
			int itemId = 0;

			if (type <= 25)
			{
				std::array damageValue = { 54, 54, 54, 54, 63, 63, 63, 72, 72, 72 };

				std::random_device rd;
				std::mt19937 g(rd());
				std::shuffle(damageValue.begin(), damageValue.end(), g);
				addId = EF_DAMAGE;
				addValue = damageValue.front();

				std::shuffle(damageWeapons.begin(), damageWeapons.end(), g);
				itemId = damageWeapons.front();
			}
			else
			{
				std::array magicValue = { 24, 24, 24, 24, 28, 28, 28, 32, 32, 32 };

				std::random_device rd;
				std::mt19937 g(rd());
				std::shuffle(magicValue.begin(), magicValue.end(), g);

				addId = EF_MAGIC;
				addValue = magicValue.front();

				std::shuffle(magicWeapons.begin(), magicWeapons.end(), g);
				itemId = magicWeapons.front();
			}

			STRUCT_ITEM* item = &pMob[clientId].Mobs.Player.Inventory[slotId];

			*item = STRUCT_ITEM{};
			item->sIndex = itemId;
			item->stEffect[0].cEffect = EF_SANC;
			item->stEffect[0].cValue = 0;
			item->stEffect[1].cEffect = addId;
			item->stEffect[1].cValue = addValue;

			SendItem(clientId, SlotType::Inv, slotId, item);

			Log(clientId, LOG_INGAME, "Recebido o item %s %s", g_pItemList[item->sIndex].ItemName, item->toString().c_str());
			if (remove)
				AmountMinus(srcItem);

			return true;
		}
#pragma endregion
#pragma region CAIXA AMULETO ARCANO
		else if (itemId == 4634)
		{
			int arcaneId = 567 + (Rand() % 4);

			STRUCT_ITEM newItem{};
			newItem.sIndex = 567 + Rand() % 4;
			newItem.stEffect[0].cEffect = EF_SANC;
			newItem.stEffect[0].cValue = 0;

			int type = Rand() % 3;
			if (type == 0)
			{
				newItem.stEffect[1].cEffect = EF_HPADD2;
				newItem.stEffect[1].cValue = 10;
				newItem.stEffect[2].cEffect = EF_MAGIC;
				newItem.stEffect[2].cValue = 16;
			}
			else if(type == 1)
			{
				newItem.stEffect[1].cEffect = EF_MPADD2;
				newItem.stEffect[1].cValue = 10;
				newItem.stEffect[2].cEffect = EF_MAGIC;
				newItem.stEffect[2].cValue = 16;
			}
			else 
			{
				newItem.stEffect[1].cEffect = EF_HPADD2;
				newItem.stEffect[1].cValue = 10;
				newItem.stEffect[2].cEffect = EF_DAMAGE;
				newItem.stEffect[2].cValue = 40;
			}

			int slotId = -1;

			bool remove = true;
			if (GetItemAmount(srcItem) == 1)
			{
				slotId = p->SrcSlot;
				remove = false;
			}
			else
				slotId = GetFirstSlot(clientId, 0);

			if (slotId == -1)
			{
				SendClientMessage(clientId, "Sem espaao no inventario");

				SendItem(clientId, SlotType::Inv, p->SrcSlot, srcItem);
				return true;
			}

			STRUCT_ITEM* item = &pMob[clientId].Mobs.Player.Inventory[slotId];
			*item = newItem;

			SendItem(clientId, SlotType::Inv, slotId, item);

			Log(clientId, LOG_INGAME, "Recebido o item %s %s", g_pItemList[item->sIndex].ItemName, item->toString().c_str());

			if (remove)
				AmountMinus(srcItem);
		}
#pragma endregion
#pragma region REPLATION (GRIUPAN IMP)
		else if (itemId == 4686 || itemId == 4687)
		{
			if (p->DstType != (int)SlotType::Inv)
			{
				SendClientMessage(clientId, g_pLanguageString[_NN_Only_To_Equips]);

				SendItem(clientId, (SlotType)p->SrcType, p->SrcSlot, srcItem);
				return true;
			}

			STRUCT_ITEM* dstItem = GetItemPointer(clientId, p->DstType, p->DstSlot);
			if (dstItem == nullptr || dstItem->sIndex == 0)
			{
				SendItem(clientId, (SlotType)p->SrcType, p->SrcSlot, srcItem);

				return true;
			}

			if ((itemId == 4686 && dstItem->sIndex != 753) || (itemId == 4687 && dstItem->sIndex != 1726))
			{
				if(itemId == 4686)
					SendClientMessage(clientId, "Somente utilizado em auxiliares Imp");
				else
					SendClientMessage(clientId, "Somente utilizado em auxiliares Griupan");

				SendItem(clientId, (SlotType)p->SrcType, p->SrcSlot, srcItem);
				return true;
			}

			Log(clientId, LOG_INGAME, "Utilizado no item %s %s", g_pItemList[dstItem->sIndex].ItemName, dstItem->toString().c_str());

			for (int i = 0; i < 1; i++)
			{
				if (dstItem->stEffect[i].cEffect == EF_SANC)
					continue;

				dstItem->stEffect[i].cEffect = 0;
				dstItem->stEffect[i].cValue = 0;
			}

			int addEf = 0;
			int addEfv = 0;
			if (itemId == 4686)
			{
				int rand = Rand() % 100;
				if (rand < 50)
				{
					addEf = EF_DAMAGE;
					addEfv = 27 + ((Rand() % 7) * 3);
				}
				else
				{
					addEf = EF_MAGIC;
					addEfv = 5 + (Rand() % 6);
				}
			}
			else
			{
				int type = Rand() % 4;
				if (type == 0)
				{
					addEf = EF_AC;
					addEfv = 70 + ((Rand() % 4) * 10);
				}
				else if (type == 1)
				{
					addEf = EF_RESISTALL;
					if (Rand() % 11 <= 5)
						addEfv = 8;
					else
						addEfv = 10;
				}
				else if (type == 2 || type == 3)
				{
					if (type == 2)
						addEf = EF_HPADD2;
					else if (type == 3)
						addEf = EF_MPADD2;

					addEfv = 4 + ((Rand() % 3) * 2);
				}
			}

			int oldItemId = dstItem->sIndex;
			int sanc = GetItemSanc(dstItem);

			*dstItem = STRUCT_ITEM{};
			dstItem->sIndex = oldItemId;
			dstItem->stEffect[0].cEffect = EF_SANC;
			dstItem->stEffect[0].cValue = sanc;
			dstItem->stEffect[1].cEffect = addEf;
			dstItem->stEffect[1].cValue = addEfv;
			dstItem->stEffect[2].cEffect = EF_NOTRADE;
			dstItem->stEffect[2].cValue = 1;

			Log(clientId, LOG_INGAME, "Gerado o adicional: %s", dstItem->toString().c_str());
			SendItem(clientId, (SlotType)p->DstType, p->DstSlot, dstItem);

			AmountMinus(srcItem);
			return true;
		}
#pragma endregion
#pragma region REPLATION (BRINCO)
		else if (itemId == 4610)
		{
			if (p->DstType != (int)SlotType::Inv)
			{
				SendClientMessage(clientId, g_pLanguageString[_NN_Only_To_Equips]);

				SendItem(clientId, (SlotType)p->SrcType, p->SrcSlot, srcItem);
				return true;
			}

			STRUCT_ITEM* dstItem = GetItemPointer(clientId, p->DstType, p->DstSlot);
			if (dstItem == nullptr || dstItem->sIndex == 0)
			{
				SendItem(clientId, (SlotType)p->SrcType, p->SrcSlot, srcItem);

				return true;
			}

			if (dstItem->sIndex < 590 || dstItem->sIndex > 595)
			{
				SendClientMessage(clientId, "Somente utilizado em Brincos");

				SendItem(clientId, (SlotType)p->SrcType, p->SrcSlot, srcItem);
				return true;
			}

			if (GetItemSanc(dstItem) > 9)
			{
				SendClientMessage(clientId, "Somente utilizado em Brincos igual ou inferior a +9");

				SendItem(clientId, (SlotType)p->SrcType, p->SrcSlot, srcItem);
				return true;
			}

			Log(clientId, LOG_INGAME, "Utilizado no item %s %s", g_pItemList[dstItem->sIndex].ItemName, dstItem->toString().c_str());

			for (int i = 0; i < 1; i++)
			{
				if (dstItem->stEffect[i].cEffect == EF_SANC)
					continue;

				dstItem->stEffect[i].cEffect = 0;
				dstItem->stEffect[i].cValue = 0;
			}

			int effectIndex = 0;
			int effectValue = 0;
			int rand = Rand() % 100;
			if (rand <= 25)
			{
				effectIndex = EF_HPADD2;
				effectValue = Rand() % 4 + 3;
			}
			else if (rand <= 50)
			{
				effectIndex = EF_MPADD2;
				effectValue = Rand() % 4 + 3;
			}
			else if (rand <= 75)
			{
				effectIndex = EF_DAMAGE;
				effectValue = Rand() % 26 + 15;
			}
			else
			{
				effectIndex = EF_MAGIC;
				effectValue = Rand() % 7 + 4;
			}

			for (int i = 1; i < 3; i++)
			{
				if (dstItem->stEffect[i].cEffect == EF_SANC)
					continue;

				dstItem->stEffect[i].cEffect = effectIndex;
				dstItem->stEffect[i].cValue = effectValue;
				break;
			}

			Log(clientId, LOG_INGAME, "Gerado o adicional: %s", dstItem->toString().c_str());
			SendItem(clientId, (SlotType)p->DstType, p->DstSlot, dstItem);

			AmountMinus(srcItem);
			return true;
		}
#pragma endregion
#pragma region OVO DA SORTE
		else if (itemId == 4615 || itemId == 4642)
		{
			int slotId = -1;

			bool remove = true;
			if (GetItemAmount(srcItem) == 1)
			{
				slotId = p->SrcSlot;
				remove = false;
			}
			else
				slotId = GetFirstSlot(clientId, 0);

			if (slotId == -1)
			{
				SendClientMessage(clientId, "Sem espaao no inventario");

				Log(clientId, LOG_INGAME, "Sem espaao no inventario para recebimento");
				SendItem(clientId, SlotType::Inv, p->SrcSlot, srcItem);
				return true;
			}

			STRUCT_ITEM item{};

			if (itemId == 4615)
				item.sIndex = 2360 + (Rand() % 6);
			else
			{
				constexpr std::array mountId = { 2366, 2367, 2368, 2371, 2372, 2373 };
				item.sIndex = *select_randomly(std::begin(mountId), std::end(mountId));
			}

			*(WORD*)&item.stEffect[0].cEffect = 5000;
			item.stEffect[1].cEffect = 120;
			item.stEffect[1].cValue = 10;
			item.stEffect[2].cEffect = 100;

			pMob[clientId].Mobs.Player.Inventory[slotId] = item;
			SendItem(clientId, SlotType::Inv, slotId, &item);

			if (remove)
				AmountMinus(srcItem);

			Log(clientId, LOG_INGAME, "Recebido o item %s %s", g_pItemList[item.sIndex].ItemName, item.toString().c_str());
			SendClientMessage(clientId, "!Chegou um item: [%s]", g_pItemList[item.sIndex].ItemName);
		}
#pragma endregion
#pragma region SELO DAS ALMAS
		else if (itemId == 3443)
		{
			if (srcItem->stEffect[0].cEffect != 0)
				return true;

			for (int i = 1; i < 15; i++)
			{
				if (pMob[clientId].Mobs.Player.Equip[i].sIndex > 0)
				{
					SendClientMessage(clientId, "Desequipe todos os itens do seu personagem para sela-lo");

					Log(clientId, LOG_INGAME, "Nao a possavel selar enquanto possuir itens equipados");
					SendItem(clientId, (SlotType)p->SrcType, p->SrcSlot, srcItem);
					return true;
				}
			}

			for (int i = 0; i < 60; i++)
			{
				if (i == p->SrcSlot)
					continue;

				if (pMob[clientId].Mobs.Player.Inventory[i].sIndex > 0)
				{
					SendClientMessage(clientId, "Deixe o inventario vazio para selar seu personagem");

					Log(clientId, LOG_INGAME, "Nao a possavel selar enquanto possuir itens no inventario");
					SendItem(clientId, (SlotType)p->SrcType, p->SrcSlot, srcItem);
					return true;
				}
			}

			bool haveStorageSlot = false;
			for (int i = 0; i < 120; i++)
			{
				if (User.Storage.Item[i].sIndex == 0)
					haveStorageSlot = true;
			}

			if (!haveStorageSlot)
			{
				SendClientMessage(clientId, "Necessario ter um slot vago no seu Guarda Carga");

				Log(clientId, LOG_INGAME, "Sem slot vago na seleaao de personagem");
				SendItem(clientId, (SlotType)p->SrcType, p->SrcSlot, srcItem);
				return true;
			}

			if (pMob[clientId].Mobs.Player.GuildLevel != 0 && pMob[clientId].Mobs.Player.GuildLevel != 1)
			{
				SendClientMessage(clientId, "Nao a possavel selar personagens laderes/subladeres de guild");

				Log(clientId, LOG_INGAME, "Nao a possavel selar personagens laderes/subladeres de guild");
				SendItem(clientId, (SlotType)p->SrcType, p->SrcSlot, srcItem);
				return true;
			}

			if (pMob[clientId].Mobs.Player.Coin != 0)
			{
				SendClientMessage(clientId, "Nao a possavel selar personagem com gold no inventario");

				Log(clientId, LOG_INGAME, "Nao a possavel selar personagem com gold no inventario");
				SendItem(clientId, (SlotType)p->SrcType, p->SrcSlot, srcItem);
				return true;
			}

			if (pMob[clientId].Mobs.Player.Equip[0].EFV2 == MORTAL)
			{
				SendClientMessage(clientId, "Nao a possavel selar personagem mortal");

				Log(clientId, LOG_INGAME, "Nao a possavel selar personagem mortal");
				SendItem(clientId, (SlotType)p->SrcType, p->SrcSlot, srcItem);
				return true;
			}

			int capeIndex = pMob[clientId].Mobs.Player.CapeInfo - CAPE_BLUE;
			if (capeIndex == 0 || capeIndex == 1)
			{
				for (INT32 i = 0; i < 10; i++)
				{
					if (pMob[clientId].Mobs.Player.Equip[15].sIndex == g_pCapesID[capeIndex][i])
					{
						Log(clientId, LOG_INGAME, "Alterado a capa de %hu para %hu", pMob[clientId].Mobs.Player.Equip[15].sIndex, g_pCapesID[2][i]);
						pMob[clientId].Mobs.Player.Equip[15].sIndex = g_pCapesID[2][i];

						break;
					}
				}

				if ((pMob[clientId].Mobs.Player.Equip[0].EFV2 == CELESTIAL && pMob[clientId].Mobs.Sub.CurrentScore == 1) || (pMob[clientId].Mobs.Player.Equip[0].EFV2 == SUBCELESTIAL))
				{
					for (int i = 0; i < 10; i++)
					{
						if (pMob[clientId].Mobs.Sub.Equip[1].sIndex == g_pCapesID[capeIndex][i])
						{
							Log(clientId, LOG_INGAME, "Alterado a capa do Sub/cele %hu para %hu", pMob[clientId].Mobs.Sub.Equip[1].sIndex, g_pCapesID[2][i]);
							pMob[clientId].Mobs.Sub.Equip[1].sIndex = g_pCapesID[2][i];

							break;
						}
					}

					SetItemSanc(&pMob[clientId].Mobs.Sub.Equip[1], 0, 0);
				}
			}

			SetItemSanc(&pMob[clientId].Mobs.Player.Equip[15], 0, 0);
			AmountMinus(srcItem);

			CharLogOut(clientId);

			pMsgSignal packet{};
			packet.Header.PacketId = PutInSealPacket;
			packet.Header.Size = sizeof packet;
			packet.Header.ClientId = clientId;

			packet.Value = inGame.CharSlot;

			AddMessageDB(reinterpret_cast<BYTE*>(&packet), sizeof packet);

			return true;
		}
#pragma endregion
#pragma region COMPACTADOR DE GOLD
		else if (itemId == 4572)
		{
			auto gold = pMob[clientId].Mobs.Player.Coin;
			if (gold < 1000000000)
			{
				SendClientMessage(clientId, "O gold manimo para compactar a de 1 bilhao");

				SendItem(clientId, (SlotType)p->SrcType, p->SrcSlot, srcItem);
				return true;
			}

			int slotId = -1;

			bool remove = true;
			if (GetItemAmount(srcItem) == 1)
			{
				slotId = p->SrcSlot;
				remove = false;
			}
			else
				slotId = GetFirstSlot(clientId, 0);

			if (slotId == -1)
			{
				SendClientMessage(clientId, "Sem espaao no inventario");

				Log(clientId, LOG_INGAME, "Sem espaao no inventario para recebimento");
				SendItem(clientId, SlotType::Inv, p->SrcSlot, srcItem);
				return true;
			}

			Log(clientId, LOG_INGAME, "Gerado Barra de 1bi. Gold atual do inventario: %d", pMob[clientId].Mobs.Player.Coin);

			STRUCT_ITEM* item = &pMob[clientId].Mobs.Player.Inventory[slotId];
			*item = STRUCT_ITEM{};
			item->sIndex = 4011;

			pMob[clientId].Mobs.Player.Coin -= 1000000000;
			SendSignalParm(clientId, clientId, 0x3AF, pMob[clientId].Mobs.Player.Coin);

			if (remove)
				AmountMinus(srcItem);

			SendClientMessage(clientId, "!Chegou um item: [%s]", g_pItemList[4011].ItemName);
			SendItem(clientId, SlotType::Inv, slotId, item);
		}
#pragma endregion
#pragma region BAa DE RUNAS
		else if (itemId == 4665)
		{
		int slotId = -1;

		bool remove = true;
		if (GetItemAmount(srcItem) == 1)
		{
			slotId = p->SrcSlot;
			remove = false;
		}
		else
			slotId = GetFirstSlot(clientId, 0);

		if (slotId == -1)
		{
			SendClientMessage(clientId, "Sem espaao no inventario");

			Log(clientId, LOG_INGAME, "Sem espaao no inventario para recebimento");
			SendItem(clientId, SlotType::Inv, p->SrcSlot, srcItem);
			return true;
		}


		STRUCT_ITEM* item = &pMob[clientId].Mobs.Player.Inventory[slotId];
		*item = STRUCT_ITEM{};
		item->sIndex = 5110 + (Rand() % 24);

		SendItem(clientId, SlotType::Inv, slotId, item);

		if (remove)
			AmountMinus(srcItem);

		SendClientMessage(clientId, "!Chegou um item: [%s]", g_pItemList[item->sIndex].ItemName);
		Log(clientId, LOG_INGAME, "Recebeu o item [%s]", g_pItemList[item->sIndex].ItemName);
		}
#pragma endregion
#pragma region CAIXA DE SECRETAS
		else if (itemId == 4676)
		{
			int slotId = -1;

			bool remove = true;
			if (GetItemAmount(srcItem) == 1)
			{
				slotId = p->SrcSlot;
				remove = false;
			}
			else
				slotId = GetFirstSlot(clientId, 0);

			if (slotId == -1)
			{
				SendClientMessage(clientId, "Sem espaao no inventario");

				Log(clientId, LOG_INGAME, "Sem espaao no inventario para recebimento");
				SendItem(clientId, SlotType::Inv, p->SrcSlot, srcItem);
				return true;
			}

			STRUCT_ITEM* item = &pMob[clientId].Mobs.Player.Inventory[slotId];
			*item = STRUCT_ITEM{};
			item->sIndex = 5334 + (Rand() % 4);

			SendItem(clientId, SlotType::Inv, slotId, item);

			if (remove)
				AmountMinus(srcItem);

			SendClientMessage(clientId, "!Chegou um item: [%s]", g_pItemList[item->sIndex].ItemName);
			Log(clientId, LOG_INGAME, "Recebeu o item [%s]", g_pItemList[item->sIndex].ItemName);
		}
#pragma endregion
#pragma region JOIA DA ALMA
		else if (itemId == 3214)
		{
			if (!(pMob[clientId].Mobs.Player.LearnedSkill[0] & 0x40000000))
			{
				SendClientMessage(clientId, "Voca nao possui a habilidade Limite de Alma");

				SendItem(clientId, SlotType::Inv, p->SrcSlot, srcItem);
				return true;
			}

			SendSignal(clientId, SERVER_SIDE, ResetSoulDelayPacket);

			Log(clientId, LOG_INGAME, "Limite da Alma resetada com sucesso");
			TimeStamp.Skills[102] = std::chrono::steady_clock::time_point();

			AmountMinus(srcItem);
		}
#pragma endregion
#pragma region aMAGO ABENaOADO
		else if (itemId >= 3230 && itemId <= 3244)
		{
			STRUCT_ITEM* dstItem = GetItemPointer(clientId, (int)SlotType::Equip, 14);
			if (dstItem->sIndex < 2330 || dstItem->sIndex > 2389)
			{
				SendClientMessage(clientId, g_pLanguageString[_NN_Mount_Not_Match]);

				SendItem(clientId, SlotType::Inv, p->SrcSlot, srcItem);
				return true;
			}

			int level = dstItem->stEffect[1].cEffect;
			if (level < 120)
			{
				SendClientMessage(clientId, "O navel manimo a 120");

				SendItem(clientId, SlotType::Inv, p->SrcSlot, srcItem);
				return true;
			}

			if (level >= 130)
			{
				SendClientMessage(clientId, "O navel maximo a 130");

				SendItem(clientId, SlotType::Inv, p->SrcSlot, srcItem);
				return true;
			}

			int amagoId = -1;
			switch (dstItem->sIndex)
			{
			case 2370:
				amagoId = 3230;
				break;
			case 2375:
				amagoId = 3231;
				break;
			case 2376:
				amagoId = 3232;
				break;
			case 2377:
				amagoId = 3233;
				break;
			case 2379:
				amagoId = 3234;
				break;
			case 2380:
				amagoId = 3235;
				break;
			case 2381:
			case 2382:
			case 2383:
				amagoId = 3236 + (dstItem->sIndex - 2381);
				break;
			case 2384:
			case 2385:
			case 2386:
				amagoId = 3239 + (dstItem->sIndex - 2384);
				break;
			case 2388:
				amagoId = 3243;
				break;
			case 2387:
				amagoId = 3242;
				break;	
			}

			if (srcItem->sIndex != amagoId)
			{
				SendClientMessage(clientId, g_pLanguageString[_NN_Mount_Not_Match]);

				SendItem(clientId, SlotType::Inv, p->SrcSlot, srcItem);
				return true;
			}

			int rand = Rand() % 100;
			int rate = 50;
			rate = rate - ((level - 120) * 2);

			if (rand <= rate)
			{
				dstItem->stEffect[1].cEffect++;
				SendItem(clientId, SlotType::Equip, 14, dstItem);

				SendClientMessage(clientId, "Sucesso na refinaaao (%d/%d)", rand, rate);

				Log(clientId, LOG_INGAME, "Sucesso no uso do %s. Chance: %d/%d", g_pItemList[amagoId].ItemName, rand, rate);
			}
			else
			{
				SendClientMessage(clientId, "Falha na refinaaao (%d/%d)", rand, rate);

				Log(clientId, LOG_INGAME, "Falhou no uso do %s. Chance: %d/%d", g_pItemList[amagoId].ItemName, rand, rate);
			}

			AmountMinus(srcItem);
		}
#pragma endregion
#pragma region FUMAaA CaSMICA
		else if (itemId == 792)
		{
			auto dstItem = GetItemPointer(clientId, (int)SlotType::Equip, 11);
			if (dstItem == nullptr)
			{
				SendItem(clientId, SlotType::Inv, p->SrcSlot, srcItem);
				return true;
			}

			if (dstItem->sIndex < 762 || dstItem->sIndex > 765)
			{
				SendClientMessage(clientId, "Equipe um planeta no slot adequado");

				SendItem(clientId, SlotType::Inv, p->SrcSlot, srcItem);
				return true;
			}

			if (GetItemSanc(dstItem) != 11)
			{
				SendClientMessage(clientId, "O planeta precisa estar +11 para composiaao");

				SendItem(clientId, SlotType::Inv, p->SrcSlot, srcItem);
				return true;
			}

			auto mob = &pMob[clientId].Mobs;
			if (mob->Fame < 500)
			{
				SendClientMessage(clientId, "Voca precisa de 500 de fama para composiaao");

				SendItem(clientId, SlotType::Inv, p->SrcSlot, srcItem);
				return true;
			}

			int totalSealed = GetInventoryAmount(clientId, 4127);
			if (totalSealed < 10)
			{
				SendClientMessage(clientId, "Voca precisa de 10 Pergaminho Selado");

				SendItem(clientId, SlotType::Inv, p->SrcSlot, srcItem);
				return true;
			}

			for (int i = 0; i < 10; ++i)
			{
				int slotId = GetFirstSlot(clientId, 4127);
				if (slotId != -1)
				{
					Log(clientId, LOG_INGAME, "Removido item %s do slot %d", g_pItemList[mob->Player.Inventory[slotId].sIndex].ItemName, slotId);
					mob->Player.Inventory[slotId] = STRUCT_ITEM{};

					SendItem(clientId, SlotType::Inv, slotId, &mob->Player.Inventory[slotId]);
				}
			}

			mob->Fame -= 500;
			AmountMinus(srcItem);

			dstItem->sIndex += 26;

			SetItemSanc(dstItem, 0, 0);
			SendItem(clientId, SlotType::Equip, 11, dstItem);
		}
#pragma endregion
#pragma region ACELERADOR DE ESCRITURA
		else if (itemId == 4682)
		{
			if (pMob[clientId].Mobs.Player.Equip[0].EFV2 <= ARCH)
			{
				SendClientMessage(clientId, "Disponavel apenas para celestiais ou superior");

				SendItem(clientId, SlotType::Inv, p->SrcSlot, srcItem);
				return true;
			}

			time_t rawnow = time(NULL);
			struct tm now; localtime_s(&now, &rawnow);

			auto now_time_t = std::mktime(&now);
			auto diffTime = std::difftime(now_time_t, pMob[clientId].Mobs.Escritura.GetTMStruct());
			if (pMob[clientId].Mobs.Escritura.Ano == 0)
				diffTime = ScrollWaitTime;

			if (diffTime >= ScrollWaitTime)
			{
				SendClientMessage(clientId, "Voca nao esta com tempo de espera para usar um praximo Escritura de Pesadelo");

				SendItem(clientId, SlotType::Inv, p->SrcSlot, srcItem);
				return true;
			}

			int month = pMob[clientId].Mobs.Escritura.Mes;
			int day = pMob[clientId].Mobs.Escritura.Dia;
			int year = pMob[clientId].Mobs.Escritura.Ano;
			int hour = pMob[clientId].Mobs.Escritura.Hora - 10;
			int min = pMob[clientId].Mobs.Escritura.Minuto;
			int sec = pMob[clientId].Mobs.Escritura.Segundo;

			if (hour < 0)
			{
				hour += 24;
				day--;
			}

			if (day <= 0)
			{
				month--;
				day = dias_mes[month];
			}

			if (month <= 0)
			{
				month = 12;
				dias_mes[month];
				year--;
			}

			pMob[clientId].Mobs.Escritura.Ano = year;
			pMob[clientId].Mobs.Escritura.Mes = month;
			pMob[clientId].Mobs.Escritura.Dia = day;
			pMob[clientId].Mobs.Escritura.Hora = hour;
			pMob[clientId].Mobs.Escritura.Minuto = min;
			pMob[clientId].Mobs.Escritura.Segundo = sec;

			AmountMinus(srcItem);
			return true;
		}
#pragma endregion
 
#pragma region Pa DE BRINCO
		else if (itemId == 4719)
		{
			int amount = GetItemAmount(srcItem);
			if (amount != 120)
			{
				SendClientMessage(clientId, "Necessario 120 unidades de Pa de Brinco");
			
				SendItem(clientId, SlotType::Inv, p->SrcSlot, srcItem);
				return true;
			}

			if (pMob[clientId].Mobs.Player.Coin < 100000000)
			{
				SendClientMessage(clientId, "Necessario 100 milhaes de gold");

				SendItem(clientId, SlotType::Inv, p->SrcSlot, srcItem);
				return true;
			}

			pMob[clientId].Mobs.Player.Coin -= 100000000;
			SendSignalParm(clientId, clientId, 0x3AF, pMob[clientId].Mobs.Player.Coin);

			*srcItem = STRUCT_ITEM{};
			srcItem->sIndex = 4611;

			SendItem(clientId, SlotType::Inv, p->SrcSlot, srcItem);

			SendClientMessage(clientId, "!Chegou um item: [%s]", g_pItemList[4611].ItemName);
			Log(clientId, LOG_INGAME, "Recebeu Emblema da Proteaao (Brinco)");
			return true;
		}
#pragma endregion
	}
	pMob[clientId].GetCurrentScore(clientId);
	SendScore(clientId);
	SendEquip(clientId);
	return true;
}