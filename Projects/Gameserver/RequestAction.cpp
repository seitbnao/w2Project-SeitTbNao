#include "cServer.h"
#include "Basedef.h"
#include "SendFunc.h"
#include "GetFunc.h"
#include <sstream>

bool CUser::RequestAction(PacketHeader *Header)
{ // 0042351A
	p36C *p = (p36C*)(Header);
	
	if(pUser[clientId].Status != USER_PLAY)
	{
		SendHpMode(clientId);

		return true;
	}

	if(pMob[clientId].Mobs.Player.CurrentScore.Hp == 0)
	{
		SendHpMode(clientId);

		Log(clientId, LOG_INGAME, "Tentativa de mover enquanto morto. Posição: %ux %uy", pMob[clientId].Target.X, pMob[clientId].Target.Y);
		return true;
	}

	if (pUser[clientId].Trade.ClientId != 0)
	{
		RemoveTrade(pUser[clientId].Trade.ClientId);
		RemoveTrade(clientId);
	}

	UINT32 moveTime = p->Header.TimeStamp, // local38
		  checkTime; // local39

	if(CurrentTime >= 120000)
		checkTime = CurrentTime - 120000;
	else
		checkTime = 0;

	pMob[clientId].Motion = 0;
	
	if(p->Header.PacketId == 0x368)
	{
		if(pMob[clientId].Mobs.Player.Class != 3 || !(pMob[clientId].Mobs.Player.LearnedSkill[0] & 2))
		{
			if(pMob[clientId].Mobs.Player.BaseScore.Level < 400)
				if(AddCrackError(clientId, 10, 28))
					return true;
		}

		INT32 mp = SkillData[73].Mana; // local40

		if(pMob[clientId].Mobs.Player.CurrentScore.Mp < mp)
		{
			SendSetHpMp(clientId);

			return true;
		}

		pMob[clientId].Mobs.Player.CurrentScore.Mp -= mp;
		pUser[clientId].Potion.CountMp -= mp;
		/*
		if(Users[clientId].IllusionTime != 0xE0A1ACA && moveTime < Users[clientId].IllusionTime + 900)
		{
			Log(clientId, LOG_ERROR, "err,illusion %d %d 900 ms limit", moveTime, Users[clientId].IllusionTime);

			AddCrackError(clientId, 1, 105);
			return true;
		}
		*/
		if(moveTime > CurrentTime + 15000 || moveTime < checkTime)
		{
			Log(clientId, LOG_ERROR, "err,illusion %d %d 15000 ms limit", moveTime, CurrentTime);

			AddCrackError(clientId, 1, 104);
			return true;
		}

		pUser[clientId].IllusionTime = moveTime;
	}

	else if(p->Header.PacketId == 0x36C)
	{
		if(pUser[clientId].Movement.TimeStamp != 0xE0A1ACA && moveTime < pUser[clientId].Movement.TimeStamp + 900)
		{
			Log(clientId, LOG_ERROR, "err,action %d %d 900 ms limit", moveTime, pUser[clientId].Movement.TimeStamp);

			AddCrackError(clientId, 1, 103);
			return true;
		}
		
		if(moveTime > CurrentTime + 15000 || moveTime < checkTime)
		{
			Log(clientId, LOG_ERROR, "err,action %d %d 15000 ms limit", moveTime, CurrentTime);

			AddCrackError(clientId, 1, 102);
			return true;
		}

		pUser[clientId].Movement.TimeStamp = moveTime;
	}
	else if(p->Header.PacketId == 0x366)
	{
		if (pUser[clientId].Movement.PacketId == 0x366)
		{
			Log(clientId, LOG_ERROR, "err, can't send MSG_Stop continuosly");

			AddCrackError(clientId, 1, 101);
			return true;
		}
	}

	if(p->Speed > pMob[clientId].Mobs.Player.CurrentScore.Move.Speed)
	{
		// Retira o CrackError ja que o Speed correto ê setado novamente abaixo.
		//AddCrackError(clientId, 5, 4);

		std::stringstream str;
		str << "Informaçães do pacote:\n";
		str << "MoveType: " << p->stEffect << "\n";
		str << "MoveSpeed: " << p->Speed << "\n";
		str << "DestinyX: " << p->TargetX << "\n";
		str << "DestinyY: " << p->TargetY << "\n";
		str << "DestinyX: " << p->PosX << "\n";
		str << "DestinyY: " << p->PosY << "\n";

		str << "etc,diffrent movement. Speed que era para ser: " << (pMob[clientId].Mobs.Player.CurrentScore.Move.Value & 15);

		Log(clientId, LOG_ERROR, str.str().c_str());
		p->Speed = pMob[clientId].Mobs.Player.CurrentScore.Move.Speed; 
	}

	//00423AB3
	INT32 tgtX = pMob[clientId].Target.X; // local41
	INT32 tgtY = pMob[clientId].Target.Y; // local42

	// Aqui ha uma checagem do tipo de movimento
	// Quando ê 1 ou 2, naturalmente a TMsrv nêo faz a checagem de distência. Ridêculo, nêo? :)
	if (p->TargetX < tgtX - VIEWGRIDX || p->TargetX > tgtX + VIEWGRIDX || p->TargetY < tgtY - VIEWGRIDY || p->TargetY > tgtY + VIEWGRIDY)
	{
		if (p->TargetX < tgtX - (VIEWGRIDX * 2) || p->TargetX > tgtX + VIEWGRIDX * 2 || p->TargetY < tgtY - VIEWGRIDY * 2 || p->TargetY > tgtY + VIEWGRIDY * 2)
		{
			p36C sm; // local55
			memset(&sm, 0, sizeof p36C);
			
			GetAction(clientId, tgtX, tgtY, &sm);
			sm.Header.PacketId = 0x368;
			sm.Speed = 6;

			pUser[clientId].SendOneMessage((BYTE*)&sm, sizeof p36C);
		}

		AddCrackError(clientId, 1, 5);
		return true;
	}

	if(tgtX >= 3329 && tgtX <= 3452 && tgtY >= 1408 && tgtY <= 1462 && !IsAdmin)
	{
		STRUCT_PISTA_DE_RUNAS *pista = &pPista[2];
		if(!pista->CurrentScore)
		{
			DoRecall(clientId);

			return true;
		}

		if(pista->inSec.Born)
		{
			INT32 dist = GetDistance(tgtX, tgtY, pMob[pista->inSec.BossID].Target.X, pMob[pista->inSec.BossID].Target.Y);
			if(dist <= 14)
			{
				INT32 x;
				for(x = 0; x < 3; x++)
				{
					if(pista->Clients[x][12] == clientId)
						break;
				}

				if(x == 3)
				{
					Teleportar(clientId, 3418, 1451);
				
					return true;
				}
			}
		}
	}

	if (tgtX >= 2447 && tgtX <= 2545 && tgtY >= 1857 && tgtY <= 1919)
	{
		time_t rawnow = time(NULL);
		struct tm now; localtime_s(&now, &rawnow);
		/*
		if (now.tm_hour == 23 && now.tm_wday != 0 && now.tm_wday != 6)
		{
			// Caso a guerra esteja recusada somente os membros do canal podem subir 
			// Caso a guerra esteja no outro canal somente os membros do canal podem subir
			if (pMob[clientId].Mobs.Citizen != sServer.Channel && (!sServer.TowerWar[sServer.Channel - 1].WarState || sServer.TowerWar[sServer.Channel - 1].WarState == 2) && !IsAdmin)
			{
				DoRecall(clientId);

				return true;
			}
		}*/
	}

	int cape = pMob[clientId].Mobs.Player.CapeInfo;
	/*
	// RvR
	if (p->TargetX >= 1710 && p->TargetX <= 1715 && p->TargetY >= 1969 && tgtY <= p->TargetY && sServer.RvR.CurrentScore == 1)
	{
		if (cape != CAPE_RED)
		{
			p36C sm; // local55
			memset(&sm, 0, sizeof p36C);
			
			GetAction(clientId, tgtX, tgtY, &sm);
			sm.Header.PacketId = 0x368;
			sm.Speed = p->Speed;

			Users[clientId].SendOneMessage((BYTE*)&sm, sizeof p36C);
			return true;
		}
	}
	else if (p->TargetX >= 1748 && p->TargetX <= 1754 && p->TargetY >= 1969 && p->TargetY <= 1994 && sServer.RvR.CurrentScore == 1)
	{
		if (cape != CAPE_BLUE)
		{
			p36C sm; // local55
			memset(&sm, 0, sizeof p36C);
			
			GetAction(clientId, tgtX, tgtY, &sm);
			sm.Header.PacketId = 0x368;
			sm.Speed = p->Speed;

			Users[clientId].SendOneMessage((BYTE*)&sm, sizeof p36C);
			return true;
		}
	}*/

	if(p->TargetX <= 0 || p->TargetX >= 4096 || p->TargetY <= 0 || p->TargetY >= 4096)
	{
		Log(clientId, LOG_ERROR, "err,action - viewgrid %s", pMob[clientId].Mobs.Player.MobName);

		return true;
	}

	if (p->TargetX != pMob[clientId].Target.X || p->TargetY != pMob[clientId].Target.Y)
	{
		// 00423CEC
		eMapAttribute attribute = GetAttribute(p->TargetX, p->TargetY); // local56
		if ((attribute.Newbie && pMob[clientId].Mobs.Player.CurrentScore.Level > sServer.NewbieZone && pMob[clientId].Mobs.Player.CurrentScore.Level < 1000) && !pUser[clientId].IsAdmin)
		{
			SendClientMessage(clientId, g_pLanguageString[_NN_Newbie_zone]);

			DoRecall(clientId);
			return true;
		}

		if (attribute.Newbie && pMob[clientId].Mobs.Player.Equip[0].EFV2 != MORTAL && !pUser[clientId].IsAdmin)
		{
			SendClientMessage(clientId, g_pLanguageString[_NN_Newbie_zone]);

			DoRecall(clientId);
			return true;
		}

		if (attribute.Guild && pMob[clientId].Mobs.Player.CurrentScore.Level < 400 && !pUser[clientId].IsAdmin)
		{
			INT32 village = GetVillage(p->TargetX, p->TargetY); // local57

			if (village >= 0 && village < 5)
			{
				if (pMob[clientId].Mobs.Player.Guild != ChargedGuildList[sServer.Channel - 1][village])
				{
					SendClientMessage(clientId, g_pLanguageString[_NN_Only_Guild_Members]);

					DoRecall(clientId);
					return true;
				}
			}
		}

		pUser[clientId].Movement.PacketId = p->Header.PacketId;
		INT32 LOCAL_58 = 0, // ?
			posX = p->TargetX,
			posY = p->TargetY;

		char pCmd[256];
		strncpy_s(pCmd, p->Route, 24);

		if (g_pMobGrid[posY][posX] != 0 && g_pMobGrid[posY][posX] != clientId)
		{
			unsigned int tmpPosX = p->TargetX;
			unsigned int tmpPosY = p->TargetY;
			size_t strsize = strlen(p->Route);

			int j;
			
			for (j = strsize; j >= 0; j--)
			{
				GetRoute(p->PosX, p->PosY, &tmpPosX, &tmpPosY, p->Route, j); // (char*)g_pHeightGrid);
				if (g_pMobGrid[p->TargetY][p->TargetX] != 0 && g_pMobGrid[p->TargetY][p->TargetX] != clientId)
					continue;

				break;
			}

			if (j == -1)
			{
				p->TargetX = pMob[clientId].Target.X;
				p->TargetY = pMob[clientId].Target.Y;

				p->Route[0] = 0;

				AddMessage((BYTE*)p, sizeof p);
				return true;
			}

			p->TargetX = tmpPosX;
			p->TargetY = tmpPosY;

			AddMessage((BYTE*)p, sizeof p);
		}

		memcpy(pMob[clientId].Route, p->Route, 24);

		GridMulticast(clientId, posX, posY, (BYTE*)p);

		if (p->Header.PacketId == 0x368)
		{
			AddMessage((BYTE*)p, sizeof p36C);
			SendSetHpMp(clientId);
		}
		// 004241BB
		INT32 routeSize = strlen((char*)pMob[clientId].Route); // local129
		BYTE LOCAL_130 = 0; // EBP - 208h ?? - direction? 

		if (routeSize > 0 && routeSize < 24)
		{
			LOCAL_130 = pMob[clientId].Route[routeSize - 1];
			LOCAL_130 = LOCAL_130 - 0x30;

			if (LOCAL_130 >= 1 && LOCAL_130 <= 9)
				LOCAL_130 = LOCAL_130 << 4;
			else
				LOCAL_130 = 0;
		}

		pMob[clientId].Mobs.Player.CurrentScore.Merchant.Value = pMob[clientId].Mobs.Player.CurrentScore.Merchant.Value | LOCAL_130 & 0xF0;
		pMob[clientId].Mobs.Player.BaseScore.Merchant.Value = pMob[clientId].Mobs.Player.CurrentScore.Merchant.Value;

		//if (sServer.BRState && sServer.BRItem > 0)
		//{
		//	INT32 tX = pMob[clientId].Target.X;
		//	INT32 tY = pMob[clientId].Target.Y;
		//	INT32 level = pMob[clientId].Mobs.Player.CurrentScore.Level;

		//	if (tX >= 2604 && tY >= 1708 && tX <= 2648 && tY <= 1744)
		//	{
		//		if (!sServer.BRGrid && level >= 100 && level < 1000)
		//			DoRecall(clientId);

		//		if (sServer.BRGrid == 1 && level >= 200 && level < 1000)
		//			DoRecall(clientId);
		//	}
		//}

		if (sServer.Colo150Limit)
		{
			INT32 level = pMob[clientId].Mobs.Player.CurrentScore.Level;
			if (level >= 150)
			{
				INT32 tX = pMob[clientId].Target.X;
				INT32 tY = pMob[clientId].Target.Y;

				if (tX >= 2604 && tY >= 1708 && tX <= 2648 && tY <= 1744)
					DoRecall(clientId);
			}
		}
	}

#if defined(_DEBUG)
	printf("Pacote recebido. Posicao: %hux %huy\n", p->TargetX, p->TargetY);
#endif

	return true;
}


// CMOB -> 1FDECA0
// CUSER-> 752BAD8
//	int32_t PosX; // 1FDF1F4
// int32_t PosY; // 01FDF1F8
//local198 = buffer
// TMSRV 756
bool RequestAction2(PacketHeader *Header)
{
	int conn = Header->ClientId;

	p36C *p = (p36C*)Header;

	int LOCAL199 = pMob[conn].Last.X; // local199
	int posY = pMob[conn].Last.Y; // local200

	int destX = p->TargetX; // local201
	int destY = p->TargetY; // local202

	int lastX = p->PosX; // local203
	int lastY = p->PosY; // local204

	if (destX <= 0 || destX >= 4096 || destY <= 0 || destY >= 4096)
	{
		// LOG ERROR

		return true;
	}

	if (lastX <= 0 || lastX >= 4096 || lastY <= 0 || lastY >= 4096)
	{
		// LOG ERROR

		return true;
	}

	if (LOCAL199 <= 0 || LOCAL199 >= 4096 || posY <= 0 || posY >= 4096)
	{
		// LOG ERROR

		return true;
	}

	// local205
	int heightPos1 = (g_pHeightGrid[posY][LOCAL199]) - (g_pHeightGrid[destY][destX]);
	
	if (heightPos1 < 0)
	{
		heightPos1 = -heightPos1; // NEG ECX
	}

	// local206
	int heightPos2 = (g_pHeightGrid[posY][LOCAL199]) - (g_pHeightGrid[lastY][lastX]);

	if (heightPos2 < 0)
	{
		heightPos2 = -heightPos2;
	}

	if (g_pHeightGrid[destY][destX] == 127 || g_pHeightGrid[lastY][lastX] == 127 || heightPos1 > 30 || heightPos2 > 30)
	{
		// CREATE MOB
		// LOG ERROR
		p364 msg;
		GetCreateMob(conn, (BYTE*)&msg);

		GridMulticast_2(pMob[conn].Target.X, pMob[conn].Target.Y, (BYTE*)&msg, 0);

		// LOG
		return true;
	}

	if (pUser[conn].Status != 22)
	{
		SendHpMode(conn);

		return true;
	}

	int LOCAL251 = pMob[conn].Mobs.Player.CurrentScore.Hp;
	if (!LOCAL251)
	{		
		SendHpMode(conn);
		return true;
	}

	if (p->Header.TimeStamp != 0x0E0A1ACA)
	{
		int LOCAL252 = 0;
		// 0x752C474 = Logout
		// 0x752C478 = Recall
		// 0x752C47C = Restart
		if (pUser[conn].Movement7556.Logout != 0)
		{
			if (pUser[conn].Movement7556.Logout >= CurrentTime - 5000)
				LOCAL252 = 1;
		}

		if (pUser[conn].Movement7556.Recall != 0)
		{
			if (pUser[conn].Movement7556.Recall >= CurrentTime - 5000)
				LOCAL252 = 1;
		}

		if (pUser[conn].Movement7556.Restart != 0)
		{
			if (pUser[conn].Movement7556.Restart >= CurrentTime - 5000)
				LOCAL252 = 1;
		}
		if (LOCAL252 != 0)
		{
			// LOG Action While delay_mode

			pUser[conn].Movement7556.Logout = 0;
			pUser[conn].Movement7556.Recall = 0;
			pUser[conn].Movement7556.Restart = 0;
		}
	}

	unsigned int LOCAL253 = p->Header.TimeStamp;
	unsigned int LOCAL254 = 0;
	if (CurrentTime > 20000)
		LOCAL254 = CurrentTime - 20000;
	else
		LOCAL254 = 0;

	if (p->Header.PacketId == 0x368)
	{
		if (pMob[conn].Mobs.Player.Class != 3 || !(pMob[conn].Mobs.Player.LearnedSkill[0] & 2))
		{
			if (pMob[conn].Mobs.Player.CurrentScore.Level < 400)
			{
				// LOG Request Illusion
				return true;
			}
		}
		// fmaster - savemana
		int LOCAL_255 = SkillData[73].Mana;

		if (pMob[conn].Mobs.Player.CurrentScore.Mp < LOCAL_255)
		{
			SendSetHpMp(conn);
			return false;
		}

		pMob[conn].Mobs.Player.CurrentScore.Mp -= LOCAL_255;
		pUser[conn].Potion.CountMp -= LOCAL_255; // 752C438 
		// 0x752C0C0
		if (pUser[conn].IllusionTime != 0x0E0A1ACA)
		{
			if (LOCAL253 < pUser[conn].IllusionTime + 900)
			{
				// LOG illusion 900ms
				return true;
			}
		}

		if (LOCAL253 > CurrentTime + 3000 || LOCAL253 > LOCAL254)
		{
			// LOCAL illusion 3000ms
			CurrentTime = GetTickCount();

			return true;
		}

		pUser[conn].IllusionTime = LOCAL253;
	}
	else if (p->Header.PacketId == 0x36C)
	{
		if (pUser[conn].Movement.TimeStamp != 0x0E0A1ACA)
		{
			if (LOCAL253 < pUser[conn].Movement.TimeStamp + 900)
			{
				// LOCAL action 900ms

				return true;
			}
		}
		
		if (LOCAL253 > CurrentTime + 3000 || LOCAL253 > LOCAL254)
		{
			if (LOCAL253 > CurrentTime + 3000)
			{
				// LOG action 3000ms
			}
			else
			{
				// Log action 2000ms
			}

			// log action faster 3000ms - verificar os JNB
			if (LOCAL253 > CurrentTime + 6000 || LOCAL253 > LOCAL254)
			{
				CloseUser(conn);

				// action faster 6000ms - closeuser

				return true;
			}

			CurrentTime = GetTickCount();
			return true;
		}

		pUser[conn].Movement.TimeStamp = LOCAL253;
	}
	else if (p->Header.PacketId == 0x366)
	{
		if (pUser[conn].Movement.PacketId == 0x366)
		{
			// LOG cant stop MSG_STOP continuously
			return true;
		}
	}
	else
		return true;

	pMob[conn].GetCurrentScore(conn);

	unsigned int LOCAL_256 = GetSpeed(&pMob[conn].Mobs.Player.CurrentScore);

	if (p->Speed > LOCAL_256)
	{
		// log different moviemtn

		p->Speed = pMob[conn].Mobs.Player.CurrentScore.Move.Value & 15;
	}

	/* if (p->stEffect != 1 && p->stEffect != 2)
	{
		if (p->TargetX < LOCAL199 - VIEWGRIDX || p->TargetX > LOCAL199 + VIEWGRIDX || p->TargetY < posY - VIEWGRIDY || p->TargetY > posY + VIEWGRIDY)
		{
			if (p->TargetX < LOCAL199 - (VIEWGRIDX * 2) || p->TargetX > LOCAL199 + VIEWGRIDX * 2 || p->TargetY < posY - VIEWGRIDY * 2 || p->TargetY > posY + VIEWGRIDY * 2)
			{
				p36C sm; // local55
				memset(&sm, 0, sizeof p36C);

				GetAction(conn, LOCAL199, posY, &sm);
				sm.Header.PacketId = 0x368;
				sm.Speed = 6;

				Users[conn].SendOneMessage((BYTE*)&sm, sizeof p36C);
				return true;
			}
		}
	}
	*/

	//004615B8  |. 83C4 0C        ADD ESP,0C


	return true;
}
