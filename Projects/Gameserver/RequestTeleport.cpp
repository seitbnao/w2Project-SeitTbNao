#include "cServer.h"
#include "Basedef.h"
#include "SendFunc.h"
#include "GetFunc.h"
#include "CEventManager.h"

bool CUser::RequestTeleport(PacketHeader *Header)
{
	int posX = pMob[clientId].Target.X,
		posY = pMob[clientId].Target.Y;

	for (int i = 0; i < MAX_TELEPORT; i++)
	{
		STRUCT_TELEPORT *m = &mTeleport[i];

		if (i == 44 || i == 45 || i == 46 || i == 47)
		{
			if (posX >= (m->SrcPos.X - 8) && posX <= (m->SrcPos.X + 8) &&
				posY >= (m->SrcPos.Y - 8) && posY <= (m->SrcPos.Y + 8))
			{
				 
			}

			continue;
		}

		if (posX >= (m->SrcPos.X - 3) && posX <= (m->SrcPos.X + 3) &&
			posY >= (m->SrcPos.Y - 3) && posY <= (m->SrcPos.Y + 3))
		{
			// Checa se a necessario gold para teleportar
			if (m->Price != 0)
			{
				// Casp seja, faz a checagem se possui gold ou nao
				int gold = pMob[clientId].Mobs.Player.Coin;
				if (gold < m->Price)
				{
					SendClientMessage(clientId, g_pLanguageString[_NN_NotEnoughGold_Teleport]);

					return true;
				}

				int destinyCity = GetVillage(m->DestPos.X, m->DestPos.Y);
				if (destinyCity != 5)
					g_pCityZone[destinyCity].impost += m->Price;

				// Retira o gold
				gold -= m->Price;

				// Seta o gold na estrutura do mob
				pMob[clientId].Mobs.Player.Coin = gold;

				// Atualiza o gold no personagem
				SendSignalParm(clientId, clientId, 0x3AF, gold);
			}
			// 38
			if (i == 14)
			{
				if (pMob[clientId].Mobs.HallEnter <= 0)
				{
					SendClientMessage(clientId, "Voca nao possui entradas");

					break;
				}

				pMob[clientId].Mobs.HallEnter--;
				SendClientMessage(clientId, g_pLanguageString[_NN_HallEnter], pMob[clientId].Mobs.HallEnter);
				Log(clientId, LOG_INGAME, "Teleportado para hallkefra. Entradas restantes: %u", pMob[clientId].Mobs.HallEnter);
			}
			else if (i == 6)
			{
				if (!sServer.RvR.CurrentScore)
				{
					SendClientMessage(clientId, "Somente as 22:00hrs de segunda a sabado.");

					return true;
				}

				INT32 cape = pMob[clientId].Mobs.Player.CapeInfo;
				if (cape != CAPE_BLUE && cape != CAPE_RED)
				{
					SendClientMessage(clientId, g_pLanguageString[_NN_Only_same_kingdom]);

					return true;
				}

				INT32 posX = 0,
					posY = 0;

				if (cape == CAPE_BLUE)
				{
					if (!(Rand() % 2))
					{
						posX = 1061 - Rand() % 5;
						posY = 2113 + Rand() % 5;
					}
					else
					{
						posX = 1091 - Rand() % 5;
						posY = 2140 + Rand() % 5;
					}
				}
				else
				{
					if (!(Rand() % 2))
					{
						posX = 1238 + Rand() % 5;
						posY = 1983 + Rand() % 5;
					}
					else
					{
						posX = 1211 + Rand() % 5;
						posY = 1955 + Rand() % 5;
					}
				}

				if (posX != 0)
				{
					Teleportar(clientId, posX, posY);

					Log(clientId, LOG_INGAME, "Solicitado teleporte na posiaao %ux %uy e teleportado para %ux %uy", posX, posY, pMob[clientId].Target.X, pMob[clientId].Target.Y);
					SendCounterMob(clientId, sServer.RvR.Points[1], sServer.RvR.Points[0]);
				}

				return true;
			}
			else if (i == 13)
			{
				if (!sServer.FirstKefra)
				{
					SendClientMessage(clientId, g_pLanguageString[_NN_TeleportDisable]);

					break;
				}
			}
			else if (i == 43)
			{
				if (pMob[clientId].Mobs.Player.Equip[13].sIndex != 3916 && pMob[clientId].Mobs.Player.Equip[13].sIndex != 3917)
				{
					SendClientMessage(clientId, "Necessario a Fada do Vale para ser teleportado");

					return true;
				}

				auto totalAccounts = GetSameMACUsers(pUser[clientId], [](CUser& user) -> bool {
					return pMob[user.clientId].IsInsideValley();
				}).size();

				if (totalAccounts != 0)
				{
					SendClientMessage(clientId, "Somente uma conta por computador");

					return true;
				}

				auto valley = GetValleyWithMinimum();
				if (valley == eValley::First)
					Teleportar(clientId, 2284 + (Rand() % 2), 3688 + (Rand() % 2));
				else
					Teleportar(clientId, 2284 + (Rand() % 2), 3816 + (Rand() % 2));

				Log(clientId, LOG_INGAME, "Solicitado teleporte na posiaao %ux %uy e teleportado para %ux %uy", posX, posY, pMob[clientId].Target.X, pMob[clientId].Target.Y);
				break;
			}

			Teleportar(clientId, m->DestPos.X + (Rand() % 2), m->DestPos.Y + (Rand() % 2));
			Log(clientId, LOG_INGAME, "Solicitado teleporte na posiaao %ux %uy e teleportado para %ux %uy", posX, posY, pMob[clientId].Target.X, pMob[clientId].Target.Y);

			pMob[clientId].GetCurrentScore(clientId);
			SendScore(clientId);
			break;
		}
	}

	return true;
}