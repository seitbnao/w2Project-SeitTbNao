#include "cServer.h"
#include "Basedef.h"
#include "SendFunc.h"
#include "GetFunc.h"

int GetItemType(int ItemID)
{
	int unique = g_pItemList[ItemID].Pos;
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
	else if(unique == 64 || unique == 192) 
		unique = 3026;

	return unique;
}

bool CUser::RequestOdin(PacketHeader *Header)
{
	pCompor *p = (pCompor*)Header;

	STRUCT_MOB *player = &pMob[clientId].Mobs.Player;

	for(int i = 0;i < 7; i++)
	{
		if(p->slot[i] < 0 || p->slot[i] >= 60)
		{			
			Log(clientId, LOG_HACK, "[HACK] Banido por enviar êndice invalido - NPC Lindy - %d", p->slot[i]);
			Log(SERVER_SIDE, LOG_HACK, "[HACK] %s - Banido por enviar êndice invalido - NPC Lindy - %d", player->MobName, p->slot[i]);
			
			SendCarry(clientId);

			return true;
		}

		if(memcmp(&player->Inventory[p->slot[i]], &p->items[i], 8) != 0)
		{
			Log(clientId, LOG_HACK, "Banido por enviar item inexistente - NPC Lindy - %d", p->items[i].sIndex);
			Log(SERVER_SIDE, LOG_HACK, "%s - Banido por enviar item inexistente - NPC Lindy - %d", player->MobName, p->items[i].sIndex);
			
			SendCarry(clientId);

			return true;
		}
		
		for(int y = 0; y < 7; y++)
		{
			if(y == i)
				continue;

			if(p->slot[i] == p->slot[y])
			{
				Log(clientId, LOG_HACK, "Banido por enviar item com mesmo slotId - NPC Lindy - %d", p->items[i].sIndex);
				Log(SERVER_SIDE, LOG_HACK, "%s - Banido por enviar item com mesmo slotId  - NPC Lindy- %d", player->MobName, p->items[i].sIndex);

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

	for(int i = 0 ; i < 7; i ++)
	{
		if (p->slot[i] == -1)
		{
			Log(clientId, LOG_COMP, "Alq. Odin - %d - Sem item", i);

			continue;
		}

		Log(clientId, LOG_COMP, "Alq. Odin - %d - %s %s - %hhd", i, g_pItemList[p->items[i].sIndex].ItemName, p->items[i].toString().c_str(), p->slot[i]);
	}

	// Fecha o trade caso esteja aberto
	if(Trade.ClientId != 0)
	{
		RemoveTrade(Trade.ClientId);
		AddCrackError(clientId, 1, CRACK_TRADE_NOTEMPTY);
	}
	
	// Fecha o inventario
	SendSignalParm(clientId, SERVER_SIDE, 0x3A7, 2);
	
	if((p->items[0].sIndex == 4043 && p->items[1].sIndex == 4043) || (p->items[0].sIndex == 413 && GetItemAmount(&p->items[0]) == 10 && p->items[1].sIndex == 413 && GetItemAmount(&p->items[1]) == 10))
	{
		INT32 sanc = GetItemSanc(&p->items[2]);
		if(sanc >= 12) //Limite de refinação = 13
		{
			SendClientMessage(clientId, g_pLanguageString[_NN_Cant_Refine_More]);

			return true;
		}
		
		if(sanc <= 10) 
		{
			SendClientMessage(clientId, g_pLanguageString[_NN_Minimum_AlqOdin_Ref_Is_10]);

			return true;
		}
		
		int mobType = GetEffectValueByIndex(p->items[2].sIndex, EF_MOBTYPE);
		if(mobType == 3)
		{
			SendClientMessage(clientId, g_pLanguageString[_NN_Cant_Refine_With_Me]);

			return true;
		}
		
		int goldRequired = 1000000000;
		if(pMob[clientId].Mobs.Player.Coin < goldRequired)
		{
			SendClientMessage(clientId, "Gold insuficiente");

			return true;
		}

		bool canBreak = true;
		bool extracao = false;
		int rate = 0;

		if (sanc == 11)
			rate = 4;
		else if (sanc == 12)
			rate = 3;
		else if (sanc == 13)
			rate = 2;
		else
			rate = 1;

		if (p->items[0].sIndex == 4043 && p->items[1].sIndex == 4043)
		{
			extracao = true;

			rate += 2;
		}

		if(p->items[2].sIndex >= 3500 && p->items[2].sIndex <= 3507)
		{
			extracao = false;
			canBreak = false;
		}

		if(g_pItemList[p->items[2].sIndex].Pos == 128)
		{
			extracao = false;
			canBreak = false;
		}

		bool secrets{ false };
		for(int i = 0; i < 4; i++)
		{
			if(p->items[3 + i].sIndex >= 5334 && p->items[3 + i].sIndex <= 5337)
			{
				rate += 1;

				secrets = true;
				continue;
			}

			if (p->items[3 + i].sIndex != 3338)
				continue;

			if (secrets)
			{
				SendClientMessage(clientId, g_pLanguageString[_NN_IncorrectComp]);

				return true;
			}

			rate += ReturnChance(&p->items[3 + i]);

			int tmpSanc = GetItemSanc(&p->items[3 + i]);
			if(tmpSanc != 0)
			{
				canBreak = false;
				extracao = false;
			}
		}
		
		int pos = g_pItemList[p->items[2].sIndex].Pos;
		if(pos > 192)
		{
			SendClientMessage(clientId, g_pLanguageString[_NN_CantRefine]);

			return true;
		}

		if(rate > 100) //padrêo : 65
		{
			SendClientMessage(clientId, g_pLanguageString[_NN_Bad_Network_Packets]);

			return true;
		}
		
		if(rate < 0)
			rate = 4;

		pMob[clientId].Mobs.Player.Coin -= goldRequired;
		SendSignalParm(clientId, clientId, 0x3AF, player->Coin);

		STRUCT_ITEM *item = player->Inventory;
		int _rand = Rand() % 100;
		if(_rand <= rate)
		{ // Sucesso na composição
			
			for(int i = 0 ; i < 3; i++)
			{
				if(item[p->slot[2]].stEffect[i].cEffect == 43 || (item[p->slot[2]].stEffect[i].cEffect >= 116 && item[p->slot[2]].stEffect[i].cEffect <= 125))
				{
					item[p->slot[2]].stEffect[i].cValue += 4;
					break;
				}
			}
			
			char tmp[128];
			sprintf_s(tmp, g_pLanguageString[_NN_Odin_CombineSucceed], player->MobName, sanc + 1);
			SendServerNotice(tmp);	

			Log(clientId, LOG_COMP, "Alquimista Odin - Refinado com sucesso %s para %d (%d/%d)", g_pItemList[item[p->slot[2]].sIndex].ItemName, sanc + 1, _rand, rate);
			LogPlayer(clientId, "Refinado com sucesso %s para +%d", g_pItemList[item[p->slot[2]].sIndex].ItemName, sanc + 1);
		}
		else
		{
			SendServerNotice("%s falhou a refinação de %s para %d", player->MobName, g_pItemList[item[p->slot[2]].sIndex].ItemName, sanc + 1);

			if(canBreak && !(_rand % 5)) 
			{ // Se pode quebrar, vemos a chance para tal acontecer
				if(extracao)
				{
					int value = 0;
					for(int i = 0; i < 3; i++)
						{
						if(item[p->slot[2]].stEffect[i].cEffect == 43 || (item[p->slot[2]].stEffect[i].cEffect >= 116 && item[p->slot[2]].stEffect[i].cEffect <= 125))
						{
							value = GetEffectValueByIndex(item[p->slot[2]].sIndex, EF_UNKNOW1);
							int mobtype = GetEffectValueByIndex(item[p->slot[2]].sIndex, EF_MOBTYPE);

							if(value <= 5 && mobtype == 0)
							{ // Itens <= [E] e ê item mortal
								value = value;
							}
							else
							{
								if(value == 6)
								{
									if(sanc <= 9 && mobtype == 1)
										value = 10;
									else if(mobtype == 1 && sanc >= 10) // Item arch e +9 ou superior
										value = 11;
									else
										value = 6 ; // Item apenas anct
								}
								else if(mobtype == 1) // Item arch nêo anct
								{
									if(sanc >= 10)
										value = 9;
									else
										value = 8;
								}
								else
									NULL;
							}

							// Calculo realizado - Item entregue
							item[p->slot[2]].stEffect[i].cEffect = 87;
							item[p->slot[2]].stEffect[i].cValue = value;
							break;
						}
					}

					for(int i = 0; i < 3; i++)
					{
						if(item[p->slot[2]].stEffect[i].cEffect == 43 || (item[p->slot[2]].stEffect[i].cEffect >= 116 && item[p->slot[2]].stEffect[i].cEffect <= 125))
							continue;

						if(item[p->slot[2]].stEffect[i].cEffect == 0)
							continue;

						if(item[p->slot[2]].stEffect[i].cEffect == 87)
							continue;

						if(g_pItemList[item[p->slot[2]].sIndex].Pos > 32)
							continue;

						int value = GetEffectValueByIndex(item[p->slot[2]].sIndex, item[p->slot[2]].stEffect[i].cEffect);

						item[p->slot[2]].stEffect[i].cValue += value;
					}

					item[p->slot[2]].sIndex = GetItemType(item[p->slot[2]].sIndex);	

					Log(clientId, LOG_COMP, "Alquimista Odin - Extração criada. Tipo: %d", value);
					LogPlayer(clientId, "Extração criada no Alquimista Odin com a falha na composição de %s para +%d", g_pItemList[item[p->slot[2]].sIndex].ItemName, sanc + 1);
				}
				else
				{
					memset(&item[p->slot[2]], 0, sizeof STRUCT_ITEM);

					Log(clientId, LOG_COMP, "Alquimista Odin - Item quebrado, malz fera. %d/%d", _rand, rate);
				}
			}
			else
			{ // Falhou apenas, refinação volta
				for(int i = 0 ; i < 3; i++)
				{
					if(item[p->slot[2]].stEffect[i].cEffect == 43 || (item[p->slot[2]].stEffect[i].cEffect >= 116 && item[p->slot[2]].stEffect[i].cEffect <= 125))
					{
						item[p->slot[2]].stEffect[i].cValue -= 4;

						break;
					}
				}
			}
		}
			
		for(int i = 0; i < 7; i++) 
		{
			if(i == 2)
			{
				SendItem(clientId, SlotType::Inv, p->slot[i], &item[p->slot[i]]);

				continue;
			}

			memset(&item[p->slot[i]], 0, sizeof STRUCT_ITEM);	
			SendItem(clientId, SlotType::Inv, p->slot[i], &item[p->slot[i]]);
		}

		SaveUser(clientId, 0);
	}
	else if(p->items[0].sIndex == 4127 && p->items[1].sIndex == 4127 && p->items[2].sIndex == 5135 && p->items[3].sIndex == 5113 &&
		p->items[4].sIndex == 5129 && p->items[5].sIndex == 5112 && p->items[6].sIndex == 5110)
	{
		if(pMob[clientId].Mobs.Player.Equip[0].EFV2 != CELESTIAL || player->BaseScore.Level != 39 || !pMob[clientId].Mobs.Info.LvBlocked || pMob[clientId].Mobs.Info.Unlock39)
		{
			SendClientMessage(clientId, g_pLanguageString[_NN_IncorrectComp]);

			return true;
		}

		// Remove os itens, independente se deu certo ou nêo
		for(int i = 0 ; i < 7 ; i++)
		{
			memset(&player->Inventory[p->slot[i]], 0, sizeof STRUCT_ITEM);

			SendItem(clientId, SlotType::Inv, p->slot[i], &player->Inventory[p->slot[i]]);
		}

		INT32 _rand = Rand() % 100;
		if(_rand <= 95)
		{
			pMob[clientId].Mobs.Info.LvBlocked = false;
			pMob[clientId].Mobs.Info.Unlock39  = true;

			SendClientMessage(clientId, g_pLanguageString[_NN_Success_Comp]);
			Log(clientId, LOG_COMP, "Composição de desbloqueio 40 efetuada com SUCESSO");
		}
		else
		{
			SendClientMessage(clientId, g_pLanguageString[_NN_CombineFailed]);

			Log(clientId, LOG_COMP, "Composição de desbloqueio 40 fetuada com FALHA");
		}

		SaveUser(clientId, 0);
	}
	else if(p->items[0].sIndex == 5125 && p->items[1].sIndex == 5115 && p->items[2].sIndex == 5111 && p->items[3].sIndex == 5112 && p->items[4].sIndex == 5120 && 
		p->items[5].sIndex == 5128 && p->items[6].sIndex == 5119)
	{
		int _rand = Rand() % 100;
		
		// Remove os itens, independente se deu certo ou nêo
		for(int i = 0 ; i < 7 ; i++)
		{
			memset(&player->Inventory[p->slot[i]], 0, sizeof STRUCT_ITEM);

			SendItem(clientId, SlotType::Inv, p->slot[i], &player->Inventory[p->slot[i]]);
		}

		SendClientMessage(clientId, g_pLanguageString[_NN_Success_Comp]);

		// Seta o item no inventario
		player->Inventory[p->slot[0]].sIndex = 3020;

		// Atualiza o inventario
		SendItem(clientId, SlotType::Inv, p->slot[0], &player->Inventory[p->slot[0]]);

		Log(clientId, LOG_INGAME, "Sucesso na composição de Pedra da Fêria");
		LogPlayer(clientId, "Sucesso na composição da pedra da Fêria");

		SaveUser(clientId, 0);
	}
	else if(p->items[0].sIndex == 4127 && p->items[1].sIndex == 4127 && p->items[2].sIndex == 5135 && p->items[3].sIndex == 413 &&
		p->items[4].sIndex == 413 && p->items[5].sIndex == 413 && p->items[6].sIndex == 413)
	{
		if(pMob[clientId].Mobs.Player.Equip[0].EFV2 < CELESTIAL)
		{
			SendClientMessage(clientId, g_pLanguageString[_NN_IncorrectComp]);

			return true;
		}

		INT32 sanc = GetItemSanc(&player->Equip[15]);
		if(sanc >= 9)
		{
			SendClientMessage(clientId, g_pLanguageString[_NN_Cant_Refine_More]);

			return true;
		}
		
		// Remove os itens, independente se deu certo ou nêo
		for(int i = 0 ; i < 7 ; i++)
		{
			memset(&player->Inventory[p->slot[i]], 0, sizeof STRUCT_ITEM);

			SendItem(clientId, SlotType::Inv, p->slot[i], &player->Inventory[p->slot[i]]);
		}

		SetItemSanc(&player->Equip[15], sanc + 1, 0);
			
		SendClientMessage(clientId, g_pLanguageString[_NN_Success_Comp]);
		SendItem(clientId, SlotType::Equip, 15, &player->Equip[15]);

		Log(clientId, LOG_COMP, "Refinação da capa obtida com sucesso: %d - Capa: %d. Ev: %d", sanc + 1, player->Equip[15].sIndex, pMob[clientId].Mobs.Player.Equip[0].EFV2);

		pMob[clientId].GetCurrentScore(clientId);
		SendScore(clientId);

		SaveUser(clientId, 0);
	}
	else if(p->items[0].sIndex == 413 && p->items[1].sIndex == 413 && p->items[2].sIndex == 413 && p->items[3].sIndex == 413 && p->items[4].sIndex == 413 && p->items[5].sIndex == 413 && 
		p->items[6].sIndex == 413)
	{
		// Remove os itens, independente se deu certo ou nêo
		for(int i = 0 ; i < 7 ; i++)
		{
			memset(&player->Inventory[p->slot[i]], 0, sizeof STRUCT_ITEM);

			SendItem(clientId, SlotType::Inv, p->slot[i], &player->Inventory[p->slot[i]]);
		}
		
		// Seta o item no inventario
		player->Inventory[p->slot[0]].sIndex = 5134;

		// Atualiza o inventario
		SendItem(clientId, SlotType::Inv, p->slot[0], &player->Inventory[p->slot[0]]);
		
		SendClientMessage(clientId, g_pLanguageString[_NN_Success_Comp]);

		Log(clientId, LOG_INGAME, "Sucesso na composição de Pista de Runas");
		LogPlayer(clientId, "Sucesso na composição da Pista de Runas");

		SaveUser(clientId, 0);
		return true;
	}
	else if(p->items[0].sIndex == 674) 
	{
		// Composição da PEdra de Kersef
		INT32 nail		= GetInventoryAmount(clientId, 674); // 5x unha de Kefra
		INT32 heart		= GetInventoryAmount(clientId, 675); // 2x Coração de Sombra Negra
		INT32 hair		= GetInventoryAmount(clientId, 676); // 3x Cabelo do Beriel Amald
		INT32 heartBer	= GetInventoryAmount(clientId, 677); // 01x Coração do Beriel
		INT32 seal		= GetInventoryAmount(clientId, 4127); // 2x Pergaminho Selado
		INT32 leaf		= GetInventoryAmount(clientId, 770); // 5x Folha de Mandragora

		if(nail < 5 || heart < 2 || hair < 3 || heartBer < 1 || seal < 2 || leaf < 5)
		{
			SendClientMessage(clientId, g_pLanguageString[_NN_IncorrectComp]);

			return true;
		}

		if(pMob[clientId].Mobs.Player.Coin < 100000000)
		{
			SendClientMessage(clientId, "Gold insuficiente!");

			return true;
		}

		INT32 slotId = GetFirstSlot(clientId, 0);
		if(slotId == -1)
			return false;

		pMob[clientId].Mobs.Player.Coin -= 100000000;

		INT32 _rand = Rand() % 100;
		if(_rand >= 80)
		{
			INT32 totalRemoved = 0;

			while(totalRemoved != 5)
			{
				INT32 itemId = 0;
				_rand = Rand() % 6;
				if(_rand == 0)
					itemId = 674;
				else if(_rand == 1)
					itemId = 675;
				else if(_rand == 2)
					itemId = 676;
				else if(_rand == 3)
					itemId = 677;
				else if(_rand == 4)
					itemId = 4127;
				else if(_rand == 5)
					itemId = 770;

				totalRemoved++;
				RemoveAmount(clientId, itemId, 1);

				Log(clientId, LOG_INGAME, "Removido %s (%d) por falha na composição", g_pItemList[itemId].ItemName);
			}
		
			SendClientMessage(clientId, g_pLanguageString[_NN_CombineFailed]);
			Log(clientId, LOG_INGAME, "Falha na composição da Pedra de Kersef lv0. %d/80", _rand);
			return true;
		}
		
		// removetodos os itens
		RemoveAmount(clientId, 674, 5);
		RemoveAmount(clientId, 675, 2);
		RemoveAmount(clientId, 676, 3);
		RemoveAmount(clientId, 677, 1);
		RemoveAmount(clientId, 4127, 2);
		RemoveAmount(clientId, 770, 5);

		memset(&pMob[clientId].Mobs.Player.Inventory[slotId], 0, sizeof STRUCT_ITEM);

		pMob[clientId].Mobs.Player.Inventory[slotId].sIndex = 4552;
		SendItem(clientId, SlotType::Inv, slotId, &pMob[clientId].Mobs.Player.Inventory[slotId]);
	
		SendClientMessage(clientId, g_pLanguageString[_NN_Success_Comp]);

		Log(clientId, LOG_INGAME, "Composto com sucesso Pedra de Kersef (lv0)");
		return true;
	}
	else
	{
		bool any = false;
		constexpr int secretStone [4][7] = 
		{
			{5126,5127,5121,5114,5125,5111,5118},
			{5131,5113,5115,5116,5125,5112,5114},
			{5110,5124,5117,5129,5114,5125,5128},
			{5122,5119,5132,5120,5130,5133,5123}
		};

		for(int y = 0 ; y < 4; y++)
		{
			if(p->items[0].sIndex == secretStone[y][0] && p->items[1].sIndex == secretStone[y][1] && p->items[2].sIndex == secretStone[y][2] && p->items[3].sIndex == secretStone[y][3] &&
				p->items[4].sIndex == secretStone[y][4] && p->items[5].sIndex == secretStone[y][5] && p->items[6].sIndex == secretStone[y][6])
			{
				if(player->Coin < 2000000)
				{
					SendClientMessage(clientId, "Sêo necessarios 2 milhêes de godl");

					return true;
				}

				// Remove os itens, independente se deu certo ou nêo
				for(int i = 0 ; i < 7 ; i++)
				{
					memset(&player->Inventory[p->slot[i]], 0, sizeof STRUCT_ITEM);

					SendItem(clientId, SlotType::Inv, p->slot[i], &player->Inventory[p->slot[i]]);
				}

				// Retira o gold
				player->Coin -= 2000000;

				// Atualiza o gold
				SendSignalParm(clientId, clientId, 0x3AF, player->Coin);

				any = true;

				int rand = Rand() % 101;
				if (rand > 95)
				{
					SendClientMessage(clientId, "Falha na composição %d/95", rand);

					Log(clientId, LOG_INGAME, "Combinação falhou de secreta. A secreta que deveria vir era: %s", g_pItemList[5334 + y].ItemName);
					SendNotice(".%s falhou na composiêaê de %s", pMob[clientId].Mobs.Player.MobName, g_pItemList[5334 + y].ItemName);
				}
				else
				{
					// Seta a Pedra Secreta
					player->Inventory[p->slot[0]].sIndex = (5334 + y);

					SendItem(clientId, SlotType::Inv, p->slot[0], &player->Inventory[p->slot[0]]);

					// Envia a mensagem de sucesso
					SendClientMessage(clientId, "Composição concluêda %d/95", rand);

					Log(clientId, LOG_INGAME, "Composto com sucesso %s. %d/95", g_pItemList[5334 + y].ItemName, rand);
					LogPlayer(clientId, "Composto com sucesso %s", g_pItemList[5334 + y].ItemName);

					SendNotice(".%s compês com sucesso a %s", pMob[clientId].Mobs.Player.MobName, g_pItemList[5334 + y].ItemName);
				}

				SaveUser(clientId, 0);
				return true;
			}
		}

		bool allIsRune = true;
		for (int i = 0; i < 7; i++)
		{
			if (p->items[i].sIndex < 5110 || p->items[i].sIndex > 5133)
				allIsRune = false;
		}

		// Tentando gerar uma Secreta aleatoriamente
		if (allIsRune)
		{
			if (player->Coin < 2000000)
			{
				SendClientMessage(clientId, "Sêo necessarios 2 milhêes de godl");

				return true;
			}

			// Remove os itens, independente se deu certo ou nêo
			for (int i = 0; i < 7; i++)
			{
				memset(&player->Inventory[p->slot[i]], 0, sizeof STRUCT_ITEM);

				SendItem(clientId, SlotType::Inv, p->slot[i], &player->Inventory[p->slot[i]]);
			}

			// Retira o gold
			player->Coin -= 2000000;

			// Atualiza o gold
			SendSignalParm(clientId, clientId, 0x3AF, player->Coin);

			int rand = Rand() % 101;
			if (rand <= 5)
			{
				int secretId = (5334 + (Rand() % 4));
				player->Inventory[p->slot[0]].sIndex = secretId;

				SendItem(clientId, SlotType::Inv, p->slot[0], &player->Inventory[p->slot[0]]);

				// Envia a mensagem de sucesso
				SendClientMessage(clientId, "Composição concluêda %d/05", rand);

				Log(clientId, LOG_INGAME, "Composto com sucesso %s usando Runas aleatêrias", g_pItemList[secretId].ItemName);
				LogPlayer(clientId, "Composto com sucesso %s usando Runas aleatêrias", g_pItemList[secretId].ItemName);

				SendNotice(".%s compês com sucesso a %s", pMob[clientId].Mobs.Player.MobName, g_pItemList[secretId].ItemName);
			}
			else
			{
				SendClientMessage(clientId, "Houve uma falha na composição do item %d/05", rand);

				Log(clientId, LOG_INGAME, "Combinação falhou de secreta usando Runas aleatêrias");
				SendNotice(".%s falhou na composição da Pedra Secreta", pMob[clientId].Mobs.Player.MobName);
			}

			return true;
		}

		if(!any)
			SendClientMessage(clientId, g_pLanguageString[_NN_IncorrectComp]);
	}

	return true;
}