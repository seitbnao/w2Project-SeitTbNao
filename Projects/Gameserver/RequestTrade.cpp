#include "cServer.h"
#include "Basedef.h"
#include "SendFunc.h"
#include "GetFunc.h"
using namespace std::chrono_literals;

void SortTradeItem(STRUCT_ITEM *item, char type)
{
	INT32 iterator; //LOCAL_1
	INT32 buffer [15]; //LOCAL9
	memset(buffer, -1, sizeof buffer);

	for(iterator = 0; iterator < 15 ; iterator ++)
	{
		if (item[iterator].sIndex == 0)
		{
			buffer[iterator] = -1;

			continue;
		}

		buffer[iterator] = GetItemAbility(&item[iterator], type);
	}

	STRUCT_ITEM itemTemp[15]; // LOCAL25
	memset(itemTemp, 0, sizeof itemTemp);

	for(iterator = 0; iterator < 15; iterator++)
	{
		INT32 maxBufferIndex = 0; //LOCAL26
		INT32 maxBuffer = -1; // LOCAL27

		for (INT32 i = 0; i < 15; i++) // local28
		{
			if (buffer[i] > maxBuffer)
			{
				maxBufferIndex = i;
				maxBuffer = buffer[i];
			}
		}

		if (maxBuffer == -1)
			break;

		buffer[maxBufferIndex] = -1;
		memcpy(&itemTemp[iterator], &item[maxBufferIndex], sizeof STRUCT_ITEM);
	}

	memcpy(item, itemTemp, sizeof STRUCT_ITEM * 15);
}

INT32 CanTrade(STRUCT_ITEM *dest, STRUCT_ITEM *inv, char* myTrade, STRUCT_ITEM *opponentTrade)
{
	STRUCT_ITEM opponentTemp[15]; //local16
	memcpy(&opponentTemp, opponentTrade, sizeof STRUCT_ITEM * 15);

	memcpy(dest, inv, sizeof STRUCT_ITEM * 64);

	for(INT32 i = 0; i < 15; i ++) // local17
	{
		INT32 slot = myTrade[i]; // local18
		if (slot == -1)
			continue;

		memset(&dest[slot], 0, sizeof STRUCT_ITEM);
	}

	// 00409BE6
	SortTradeItem(opponentTemp, EF_GRID);
	for (INT32 i = 0; i < 15; i++) // local17
	{
		if (opponentTemp[i].sIndex == 0)
			continue;

		INT32 freeSlot = -1; // LOCAL19
		for(int i = 0 ; i < 30; i++)
		{
			if(dest[i].sIndex == 0)
			{
				freeSlot = i;

				break;
			}
		}

		if(freeSlot == -1 && dest[60].sIndex == 3467)
		{
			float remainig = TimeRemaining(dest[60].EFV1, dest[60].EFV2, dest[60].EFV3 + 1900);
			if(remainig > 0.0f)
			{
				for(int i = 30; i < 45; i++)
				{
					if(dest[i].sIndex == 0)
					{
						freeSlot = i;
						break;
					}
				}
			}
		}

		if(freeSlot == -1 && dest[61].sIndex == 3467)
		{
			float remainig = TimeRemaining(dest[61].EFV1, dest[61].EFV2, dest[61].EFV3 + 1900);
			if(remainig > 0.0f)
			{
				for(int i = 45; i < 60; i++)
				{
					if(dest[i].sIndex == 0)
					{
						freeSlot = i;
						break;
					}
				}
			}
		} 


		if(freeSlot != -1)
		{
			memcpy(&dest[freeSlot], &opponentTemp[i], sizeof STRUCT_ITEM);

			continue;
		}

		return false;
	}

	return true;
}

bool CUser::RequestTrade(PacketHeader *Header)
{
	p383 *msg = (p383*)(Header); // LOCAL1348

	if(pMob[clientId].Mobs.Player.CurrentScore.Hp <= 0 || pUser[clientId].CurrentScore != USER_PLAY)
	{
		Log(clientId, LOG_ERROR, "Tentando usar o trade sem estar no jogo ou com o HP zerado.");
		SendHpMode(clientId);
		RemoveTrade(clientId);
		return true;
	}
	
	INT32 opponentId = msg->ClientId; // LOCAL1349
	if(opponentId <= 0 || opponentId >= MAX_PLAYER)
	{
		Log(clientId, LOG_ERROR, "Enviando oponente com indice irregular.");

		RemoveTrade(clientId);
		return true;
	}

	if(pUser[opponentId].CurrentScore != USER_PLAY)
	{
		RemoveTrade(clientId);

		return true;
	}

	if(User.Block.Blocked)
	{
		SendClientMessage(clientId, "Desbloqueie seus itens para poder movimenta-los");

		return true;
	}

	if(msg->Coin < 0 || msg->Coin > pMob[clientId].Mobs.Player.Coin)
	{
		RemoveTrade(clientId);
		RemoveTrade(opponentId);

		return true;
	}
	
	for(INT32 i = 0; i < 15; i ++) // local1350
	{
		if(msg->Item[i].sIndex != 0)
		{
			INT32 slot = msg->Slot[i]; // local1351
			if (slot < 0 || slot >= 60)
				return true;
			
			INT32 verify = memcmp(&pMob[clientId].Mobs.Player.Inventory[slot], &msg->Item[i], 8); // local1352
			if (verify != 0)
			{
				Log(clientId, LOG_ERROR, "Retirou ou modificou o item enquanto estava no trade.");
				RemoveTrade(clientId);
				RemoveTrade(opponentId);
				return true;
			}	
		}
		/// 43262B
		if(pUser[opponentId].Trade.Item[i].sIndex != 0)
		{
			INT32 slot = pUser[opponentId].Trade.Slot[i]; // local1353
			if (slot < 0 || slot > 60)
				return true;

			INT32 verify = memcmp(&pMob[opponentId].Mobs.Player.Inventory[slot], &pUser[opponentId].Trade.Item[i], 8); // local1354
			if (verify != 0)
			{
				Log(opponentId, LOG_ERROR, "Retirou ou modificou o item enquanto estava no trade.");

				RemoveTrade(clientId);
				RemoveTrade(opponentId);
				return true;
			}
		}
	}

	INT32 lastOpponent = pUser[clientId].Trade.ClientId; //local1355
	if(lastOpponent == opponentId)
	{
		for(INT32 i = 0; i < 15; i++) // local1356
		{
			INT32 verify = 0; // local1357
			if(pUser[clientId].Trade.Item[i].sIndex != 0)
				verify = memcmp(&pUser[clientId].Trade.Item[i], &msg->Item[i], 8);

			if(pUser[clientId].Trade.Coin  != 0 && msg->Coin != pUser[clientId].Trade.Coin)
				verify = 1;

			if (verify != 0)
			{
				RemoveTrade(clientId);
				RemoveTrade(opponentId);
				return true;
			}
		}
	}
	else
	{
		if(lastOpponent != 0)
		{
			SendClientMessage(clientId,  g_pLanguageString[_NN_Already_Trading]);

			RemoveTrade(clientId);
			return true;
		}
	}
	
	if (pUser[opponentId].AllStatus.PK)
	{
		SendClientMessage(clientId, "O outro jogador esta com o modo PvP ativo");
		
		RemoveTrade(opponentId);
		RemoveTrade(clientId);
		return true;
	}

	if (pUser[clientId].AllStatus.PK)
	{
		SendClientMessage(clientId, "Nêo ê possêvel trocar com o modo PvP ativo");
		
		RemoveTrade(opponentId);
		RemoveTrade(clientId);
		return true;
	}

	if (pUser[opponentId].AllStatus.Whisper)
	{
		SendClientMessage(clientId, "O outro jogador esta com o chat desativado");
		
		RemoveTrade(opponentId);
		RemoveTrade(clientId);
		return true;
	}

	if (pUser[clientId].AllStatus.Whisper)
	{
		SendClientMessage(clientId, "Nêo ê possêvel trocar com o chat desativado");

		RemoveTrade(opponentId);
		RemoveTrade(clientId);
		return true;
	}

	INT32 clientGuildId = pMob[clientId].Mobs.Player.Guild; // local1358
	INT32 opponentGuildId = pMob[opponentId].Mobs.Player.Guild; // local1359

	// 004328C3
	INT32 clientMemberType = pMob[clientId].Mobs.Player.GuildLevel; // local1360
	INT32 opponentMemberType = pMob[opponentId].Mobs.Player.GuildLevel; // local1361

	for(INT32 i = 0; i < 15; i ++) // local1362
	{
		if(msg->Item[i].sIndex == 747 || msg->Item[i].sIndex == 3993 || msg->Item[i].sIndex == 3994)
		{
			SendClientMessage(clientId, g_pLanguageString[_NN_Only_With_Guild_Master]);
			SendClientMessage(opponentId, g_pLanguageString[_NN_Only_With_Guild_Master]);

			RemoveTrade(clientId);
			if(pUser[opponentId].Trade.ClientId == clientId)
				RemoveTrade(opponentId);

			return true;
		}
		/*
		if(msg->Item[i].cEffect == 508 || msg->Item[i].cEffect == 522 || 
			(msg->Item[i].cEffect >= 506 && msg->Item[i].cEffect < 537) || msg->Item[i].cEffect == 446)
		{
			INT32 guild = GetGuild(&msg->Item[i]); // local1363

			if (guild == clientGuildId && clientMemberType != 0)
				continue;

			if (guild == opponentGuildId && opponentMemberType != 0)
				continue;

			SendClientMessage(clientId, g_pLanguageString[_NN_Only_With_Guild_Master]);
			SendClientMessage(opponentId, g_pLanguageString[_NN_Only_With_Guild_Master]);

			RemoveTrade(clientId);

			if(Users[opponentId].Trade.ClientId == clientId)
				RemoveTrade(opponentId);

			return true;
		}
		*/
		if (msg->Item[i].sIndex && GetItemAbility(&msg->Item[i], EF_NOTRADE))
		{
			SendClientMessage(clientId, "Este item nêo pode ser trocado.");
			SendClientMessage(opponentId, "Este item nêo pode ser trocado.");

			RemoveTrade(clientId);
			if (pUser[opponentId].Trade.ClientId == clientId)
				RemoveTrade(opponentId);

			return true;
		}
	}
	// 00432AC0

	if(pUser[opponentId].Trade.ClientId == 0)
	{
		auto now = std::chrono::steady_clock::now();
		if (now - LastTrade <= 1s)
		{
			SendClientMessage(clientId, "Aguarde para enviar novamente");

			LastTrade = now;
			return true;
		}

		memcpy(&pUser[clientId].Trade, msg, sizeof p383);

		pUser[clientId].Trade.Confirm = 0;
		pUser[clientId].Trade.Coin = 0;

		msg->Header.ClientId = opponentId;
		msg->ClientId = clientId;

		pUser[opponentId].AddMessage((BYTE*)msg, sizeof p383);

		pUser[opponentId].Trade.Confirm = 0;
		pUser[opponentId].Trade.Coin = 0;

		LastTrade = now;
		return true;
	}

	if(pUser[opponentId].Trade.ClientId != clientId)
	{
		SendClientMessage(clientId, g_pLanguageString[_NN_Already_Trading]);

		RemoveTrade(clientId);
		return true;
	}

	// 00432BCF
	for(INT32 i = 0; i < 15; i++) // local1364
	{
		INT32 slot = msg->Slot[i]; // local1365
		if(slot == -1)
		{
			memset(&msg->Item[i], 0, sizeof STRUCT_ITEM);

			continue;
		}

		if(slot < -1  || slot >= 60)
		{
			SendClientMessage(clientId, g_pLanguageString[_NN_Wrong_Trade_Packet1]);
			RemoveTrade(clientId);

			return true;
		}

		for(INT32 j = 0; j < 15; j ++) // local1366
		{
			if(j == i)
				continue;

			if(slot == msg->Slot[j])
			{
				SendClientMessage(clientId, g_pLanguageString[_NN_Wrong_Trade_Packet1]);
				RemoveTrade(clientId);

				return true;
			}
		}

		memcpy(&msg->Item[i], &pMob[clientId].Mobs.Player.Inventory[slot], sizeof STRUCT_ITEM);
	}

	if(msg->Confirm == 1)
	{
		pUser[clientId].Trade.Confirm = true;
		pUser[clientId].Trade.Header.Key = msg->Header.Key;
		pUser[clientId].Trade.Header.CheckSum = msg->Header.CheckSum;
		pUser[clientId].Trade.Header.TimeStamp = msg->Header.TimeStamp;

		int verify = memcmp(&pUser[clientId].Trade, msg, sizeof p383); // local1367
		if (verify != 0)
		{
			RemoveTrade(clientId);
			RemoveTrade(opponentId);

			Log(clientId, LOG_ERROR, "Tentou tirar/mudar o item enquanto estava com trade.");
			return false;
		}

		if(pUser[opponentId].Trade.Confirm == 1)
		{
			if(pUser[clientId].Trade.Coin < 0 || pUser[clientId].Trade.Coin > 2000000000)
			{
				RemoveTrade(clientId);
				RemoveTrade(opponentId);
			}

			if(pUser[opponentId].Trade.Coin < 0 || pUser[opponentId].Trade.Coin > 2000000000)
			{
				RemoveTrade(clientId);
				RemoveTrade(opponentId);
			}

			INT32 totalGoldClient = pMob[clientId].Mobs.Player.Coin - pUser[clientId].Trade.Coin + pUser[opponentId].Trade.Coin; // local1368
			if (totalGoldClient > 2000000000 || totalGoldClient < 0)
			{
				SendClientMessage(clientId, g_pLanguageString[_NN_Cant_get_more_than_2G]);
				
				RemoveTrade(clientId);
				RemoveTrade(opponentId);
				return true;
			}

			INT32 totalGoldOpponent = pMob[opponentId].Mobs.Player.Coin - pUser[opponentId].Trade.Coin + pUser[clientId].Trade.Coin; // local1369
			if (totalGoldOpponent > 2000000000 || totalGoldOpponent < 0)
			{
				SendClientMessage(clientId, g_pLanguageString[_NN_Cant_get_more_than_2G]);
				
				RemoveTrade(clientId);
				RemoveTrade(opponentId);
				return true;
			}

			if(pUser[clientId].Trade.Coin > pMob[clientId].Mobs.Player.Coin)
			{
				SendClientMessage(clientId, g_pLanguageString[_NN_Havent_Money_So_Much]);
				SendClientMessage(opponentId, g_pLanguageString[_NN_Opponent_Havent_Money]);

				return true;
			}

			if(pUser[opponentId].Trade.Coin > pMob[opponentId].Mobs.Player.Coin)
			{
				SendClientMessage(opponentId, g_pLanguageString[_NN_Havent_Money_So_Much]);
				SendClientMessage(clientId, g_pLanguageString[_NN_Opponent_Havent_Money]);

				return true;
			}

			STRUCT_ITEM clientDest[64]; // local1497
			STRUCT_ITEM opponentDest[64]; // local1625
			// 00433036
			INT32 clientTrade = CanTrade(clientDest, pMob[clientId].Mobs.Player.Inventory, pUser[clientId].Trade.Slot, pUser[opponentId].Trade.Item); // local1626
			INT32 opponentTrade = CanTrade(opponentDest, pMob[opponentId].Mobs.Player.Inventory, pUser[opponentId].Trade.Slot, pUser[clientId].Trade.Item);; // BASE_CanTrade local1627

			if (clientTrade == 0 && opponentTrade == 0)
			{
				SendClientMessage(clientId, g_pLanguageString[_NN_Each_Of_You_Havent_Space]);
				SendClientMessage(opponentId, g_pLanguageString[_NN_Each_Of_You_Havent_Space]);

				return true;
			}

			if (clientTrade == 0)
			{
				SendClientMessage(clientId, g_pLanguageString[_NN_You_Have_No_Space_To_Trade]);
				SendClientMessage(opponentId, g_pLanguageString[_NN_Opponent_Have_No_Space_To]);

				return true;
			}

			if (opponentTrade == 0)
			{
				SendClientMessage(opponentId, g_pLanguageString[_NN_You_Have_No_Space_To_Trade]);
				SendClientMessage(clientId, g_pLanguageString[_NN_Opponent_Have_No_Space_To]);

				return true;
			}
			
			Log(clientId, LOG_INGAME, "Trade com conta %s", pUser[opponentId].User.Username);
			Log(opponentId, LOG_INGAME, "Trade com conta %s", pUser[clientId].User.Username);

			for(INT32 i = 0 ; i < 15; i++)
			{
				STRUCT_ITEM *item = &pUser[opponentId].Trade.Item[i];
				if(item->sIndex > 0 && item->sIndex < 6500)
				{ // Esta enviando item
					Log(opponentId, LOG_INGAME, "Trade - %d - Enviado %s para %s [%d] [%d %d %d %d %d %d]", i, g_pItemList[item->sIndex].ItemName, pMob[clientId].Mobs.Player.MobName, item->sIndex, item->EF1, item->EFV1,
						item->EF2, item->EFV2, item->EF3, item->EFV3);
					Log(clientId, LOG_INGAME, "Trade - %d - Recebido %s de %s [%d] [%d %d %d %d %d %d]", i, g_pItemList[item->sIndex].ItemName, pMob[opponentId].Mobs.Player.MobName, item->sIndex, item->EF1, item->EFV1,
						item->EF2, item->EFV2, item->EF3, item->EFV3);
					
					LogPlayer(opponentId, "Enviado o item %s para %s por trade.", g_pItemList[item->sIndex].ItemName, pMob[clientId].Mobs.Player.MobName);
					LogPlayer(clientId, "Recebido o item %s para %s por trade.", g_pItemList[item->sIndex].ItemName, pMob[opponentId].Mobs.Player.MobName);
				}
			}

			for(INT32 i = 0 ; i < 15; i++)
			{
				STRUCT_ITEM *item = &pUser[clientId].Trade.Item[i];
				if(item->sIndex > 0 && item->sIndex < 6500)
				{ // Esta enviando item
					Log(clientId, LOG_INGAME, "Trade - %d - Enviado %s para %s [%d] [%d %d %d %d %d %d]", i, g_pItemList[item->sIndex].ItemName, pMob[opponentId].Mobs.Player.MobName, item->sIndex, item->EF1, item->EFV1,
						item->EF2, item->EFV2, item->EF3, item->EFV3);
					Log(opponentId, LOG_INGAME, "Trade - %d - Recebido %s de %s [%d] [%d %d %d %d %d %d]", i, g_pItemList[item->sIndex].ItemName, pMob[clientId].Mobs.Player.MobName, item->sIndex, item->EF1, item->EFV1,
						item->EF2, item->EFV2, item->EF3, item->EFV3);
					
					LogPlayer(opponentId, "Recebido o item %s para %s por trade.", g_pItemList[item->sIndex].ItemName, pMob[clientId].Mobs.Player.MobName);
					LogPlayer(clientId, "Enviado o item %s para %s por trade.", g_pItemList[item->sIndex].ItemName, pMob[opponentId].Mobs.Player.MobName);
				}
			}

			// 00433169
			memcpy(pMob[clientId].Mobs.Player.Inventory, clientDest, sizeof STRUCT_ITEM * 64);
			memcpy(pMob[opponentId].Mobs.Player.Inventory, opponentDest, sizeof STRUCT_ITEM * 64);
			
			Log(clientId, LOG_INGAME, "Trade - Enviado %d de gold para o usuario", pUser[clientId].Trade.Coin);
			Log(opponentId, LOG_INGAME, "Trade - Recebido %d de gold do usuario", pUser[clientId].Trade.Coin);

			Log(opponentId, LOG_INGAME, "Trade - Enviado %d de gold para o usuario", pUser[opponentId].Trade.Coin);
			Log(clientId, LOG_INGAME, "Trade - Recebido %d de gold do usuario", pUser[opponentId].Trade.Coin);

			LogPlayer(clientId, "Enviado %d de gold para o usuario", pUser[clientId].Trade.Coin);
			LogPlayer(opponentId, "Recebido %d de gold do usuario", pUser[clientId].Trade.Coin);

			LogPlayer(opponentId, "Enviado %d de gold para o usuario", pUser[opponentId].Trade.Coin);
			LogPlayer(clientId, "Recebido %d de gold do usuario", pUser[opponentId].Trade.Coin);

			pMob[clientId].Mobs.Player.Coin = totalGoldClient;
			pMob[opponentId].Mobs.Player.Coin = totalGoldOpponent;

			SendCarry(clientId);
			SendCarry(opponentId);

			RemoveTrade(clientId);
			RemoveTrade(opponentId);

			SaveUser(clientId, 0);
			SaveUser(opponentId, 0);

			return true;
		}
		
		SendSignal(clientId, clientId, 0x386);

		msg->ClientId = clientId;
		msg->Header.ClientId = opponentId;

		pUser[opponentId].AddMessage((BYTE*)msg, sizeof p383);
		return true;
	}

	memcpy(&pUser[clientId].Trade, msg, sizeof p383);

	pUser[clientId].Trade.Confirm = false;
	pUser[opponentId].Trade.Confirm = false;

	
	msg->Header.ClientId = opponentId;
	msg->ClientId = clientId;

	pUser[opponentId].AddMessage((BYTE*)msg, sizeof p383);
	return true;
}