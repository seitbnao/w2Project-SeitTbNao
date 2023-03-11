#include "cServer.h"
#include "Basedef.h"
#include "SendFunc.h"
#include "GetFunc.h"
#include "CEventManager.h"
#include "CQuiz.h"

bool PacketControl(BYTE* pBuffer, INT32 size)
{
	PacketHeader *header = (PacketHeader*)pBuffer;

	int clientId = header->ClientId;

	switch (header->PacketId)
	{
	
	case MSG_PANELGUILD_GETLIST:
		pUser[clientId].AddMessage(pBuffer, size);
		break;

	
	
	case 0x381:
	{
		pMsgSignal2 *p = (pMsgSignal2*)(header);

		if (p->Value < 0 || p->Value >= MAX_STORE || p->Value2 < 0 || p->Value2 >= MAX_DONATEITEM)
			return true;

		STRUCT_STOREDONATE *store = &g_pStore[p->Value][p->Value2];

		if (store->Avaible != -1)
			store->Avaible--;
	}
	break;

	case 0x382:
	{
		pMsgSignal3 *p = (pMsgSignal3*)(header);

		if (p->Value < 0 || p->Value >= MAX_STORE || p->Value2 < 0 || p->Value2 >= MAX_DONATEITEM)
			return true;

		STRUCT_STOREDONATE *store = &g_pStore[p->Value][p->Value2];
		store->Avaible = p->Value3;
	}
	break;
 
	case 0xD1D:
		header->ClientId = 0x7530;

		for (INT32 i = 1; i < MAX_PLAYER; i++)
		{
			if (pUser[i].Status != USER_PLAY)
				continue;

			pUser[i].AddMessage((BYTE*)header, header->Size);
		}
		break;
	case 0xC0E:
	{
		pCOE *p = (pCOE*)(header);

		if (pUser[clientId].Status < USER_SELCHAR)
		{
			Log(clientId, LOG_ERROR, "Falha ao entregar cash ImportCash - Status < USER_SELCHAR - %d",
				p->Cash);

			break;
		}

		if (strncmp(p->Username, pUser[clientId].User.Username, 16))
		{
			Log(clientId, LOG_ERROR, "Falha ao entregar cash ImportCash - %s != %s - %d",
				p->Username, pUser[clientId].User.Username, p->Cash);

			break;
		}

		pUser[clientId].User.Cash += p->Cash;

		if (pUser[clientId].Status == USER_SELCHAR)
			SaveUser(clientId, 0);

		Log(clientId, LOG_INGAME, "Recebeu %d de Cash - Total %d - ImportCash", p->Cash, pUser[clientId].User.Cash);

		if (pUser[clientId].Status == USER_PLAY)
		{
			SendClientMessage(clientId, "!Chegou [ %d ] Cash", p->Cash);
			SendSignalParm(clientId, clientId, RefreshGoldPacket, pUser[clientId].User.Cash);
		}
	}
	break;
	case 0xC10:
	{
		pC10 *p = (pC10*)(header);

		if (strncmp(p->Username, pUser[clientId].User.Username, 16))
		{
			Log(clientId, LOG_ERROR, "Falha ao banir o usuario %s != %s - %d %d/%d/%d",
				p->Username, pUser[clientId].User.Username, p->BanType, p->Ban.Dia, p->Ban.Mes, p->Ban.Ano);

			break;
		}

		pUser[clientId].User.BanType = p->BanType;
		memcpy(&pUser[clientId].User.Ban, &p->Ban, sizeof(p->Ban));

		SendClientMessage(clientId, "Conta banida. Contate o suporte para maiores informaaaes.");

		SaveUser(clientId, 0);
		CloseUser(clientId);
	}
	break;
	case 0xC0F:
	{
		pCOF *p = (pCOF*)(header);

		if (pUser[clientId].Status < USER_SELCHAR)
		{
			Log(clientId, LOG_ERROR, "Falha ao receber item do ImportItem - Status < USER_SELCHAR - %d %d %d %d %d %d %d",
				p->item.sIndex, p->item.stEffect[0].cEffect, p->item.stEffect[0].cValue, p->item.stEffect[1].cEffect, p->item.stEffect[1].cValue, p->item.stEffect[2].cEffect, p->item.stEffect[2].cValue);

			break;
		}

		if (strncmp(p->Username, pUser[clientId].User.Username, 16))
		{
			Log(clientId, LOG_ERROR, "Falha ao receber item do ImportItem - %s != %s - %d %d %d %d %d %d %d",
				p->Username, pUser[clientId].User.Username, p->item.sIndex, p->item.stEffect[0].cEffect, p->item.stEffect[0].cValue, p->item.stEffect[1].cEffect, p->item.stEffect[1].cValue, p->item.stEffect[2].cEffect, p->item.stEffect[2].cValue);

			break;
		}

		for (INT32 i = 0; i < 120; i++)
		{
			if (pUser[clientId].User.Storage.Item[i].sIndex != 0)
				continue;

			pUser[clientId].User.Storage.Item[i] = p->item;
			SendItem(clientId, SlotType::Storage, i, &p->item);

			SendClientMessage(clientId, "!Chegou um item [ %s ]", g_pItemList[p->item.sIndex].ItemName);

			Log(clientId, LOG_INGAME, "Recebeu o item %s [%d] [%d %d %d %d %d %d]",
				g_pItemList[p->item.sIndex].ItemName, p->item.sIndex, p->item.stEffect[0].cEffect, p->item.stEffect[0].cValue, p->item.stEffect[1].cEffect, p->item.stEffect[1].cValue, p->item.stEffect[2].cEffect, p->item.stEffect[2].cValue);

			if (pUser[clientId].Status == USER_SELCHAR)
				SaveUser(clientId, 0);

			return true;
		}

		for (INT32 i = 119; i > 0; i--)
		{
			if (pUser[clientId].User.Storage.Item[i].sIndex != 0)
				continue;

			pUser[clientId].User.Storage.Item[i] = p->item;
			SendItem(clientId, SlotType::Storage, i, &p->item);

			SendClientMessage(clientId, "!Chegou um item [ %s ]", g_pItemList[p->item.sIndex].ItemName);

			Log(clientId, LOG_INGAME, "Recebeu o item %s [%d] [%d %d %d %d %d %d]",
				g_pItemList[p->item.sIndex].ItemName, p->item.sIndex, p->item.stEffect[0].cEffect, p->item.stEffect[0].cValue, p->item.stEffect[1].cEffect, p->item.stEffect[1].cValue, p->item.stEffect[2].cEffect, p->item.stEffect[2].cValue);

			if (pUser[clientId].Status == USER_SELCHAR)
				SaveUser(clientId, 0);

			return true;
		}

		SaveUser(clientId, 1);

		Log(clientId, LOG_ERROR, "Nao recebeu o item %s [%d] [%d %d %d %d %d %d] por falta de espaao",
			g_pItemList[p->item.sIndex].ItemName, p->item.sIndex, p->item.stEffect[0].cEffect, p->item.stEffect[0].cValue, p->item.stEffect[1].cEffect, p->item.stEffect[1].cValue, p->item.stEffect[2].cEffect, p->item.stEffect[2].cValue);

		SendClientMessage(clientId, "!Falta espaao para receber o item! Libere seu baa!");
	}
	break;
	case 0x52A:
		pUser[clientId].ChangeServer(header);
		break;

	case MSG_CREATEGUILD_OPCODE:
		return GetNewGuild(header);

	case MSG_ADDSUB_OPCODE:
		return AddSubLider(header);

	case MSG_ADDGUILD_OPCODE:
		return GetGuildInfo(header);

	case 0x899:
	{
		pMsgSignal *p = (pMsgSignal*)(header);

		sServer.RvR.Bonus = p->Value;
		Log(SERVER_SIDE, LOG_INGAME, "Reino campeao da RvR: %d", p->Value);
	}
	break;
	case 0x3409:
	{
		MSG_NPNotice *m = (MSG_NPNotice*)(header);

		if (m->Parm1 == 1)
			SendNotice(m->String);

	} break;

	case 0x101:
	{
		p101 *p = (p101*)(header);

		SendNotice(p->Msg);
	}
	break;
	case 0x428:
	{
		MSG_GuildReport *m = (MSG_GuildReport*)header;

		memcpy(ChargedGuildList, m->ChargedGuildList, sizeof(ChargedGuildList));
	} break;

	case 0x415:
	{
		p415 *p = (p415*)(header);

		pUser[clientId].AccessLevel = p->Access;


		pUser[clientId].User.Water.Day = p->Water.Day;
		pUser[clientId].User.Water.Total = p->Water.Total;

		pUser[clientId].User.Divina = p->Divina;
		pUser[clientId].User.Sephira = p->Sephira;

		memcpy(&pUser[clientId].User.Ban, &p->Ban, sizeof STRUCT_ITEMDATE);
		pUser[clientId].User.BanType = p->BanType;

		pUser[clientId].IsBanned = p->IsBanned == 1;
		pUser[clientId].User.Cash = p->Cash;
	}
	break;
	case 0x41D:
	{
		SendSignal(clientId, 0, 0x11A);

		pUser[clientId].Status = USER_SELCHAR;
	}
	break;
	case 0x41E:
	{
		SendSignal(clientId, 0, 0x11B);

		pUser[clientId].Status = USER_SELCHAR;
	}
	break;
	case 0x40A:
	{// 0044E5DD
		MSG_DBSavingQuit *p = (MSG_DBSavingQuit *)(header);
		if (pUser[clientId].Status != USER_PLAY && pUser[clientId].Status != USER_SAVING4QUIT)
		{
			pMsgSignal packet;
			packet.Header.PacketId = 0x805;
			packet.Header.ClientId = clientId;

			AddMessageDB((BYTE*)&packet, 12);
		}

		if (pUser[clientId].Status != USER_PLAY || pUser[clientId].Status == USER_SELCHAR)
		{
			if (p->Mode == 0)
				SendClientMessage(clientId, g_pLanguageString[_NN_Your_Account_From_Others]);
			else if (p->Mode == 1)
				SendClientMessage(clientId, g_pLanguageString[_NN_Disabled_Account]);

			pUser[clientId].SendMessageA();
		}

		CloseUser(clientId);
	}
	break;
	case 0x40B:
	{
		pMob[clientId].Mode = 0;

		pUser[clientId].Status = USER_ACCEPT;
		CloseUser(clientId);
	}
	break;
	case 0x420:
	{
		SendSignal(clientId, 0x7532, 0x11D);

		pUser[clientId].SendMessageA();
		CloseUser(clientId);
	}
	break;
	case 0x421:
	{
		SendClientMessage(clientId, g_pLanguageString[_NN_No_Account_With_That_Name]);

		pUser[clientId].SendMessageA();
		CloseUser(clientId);
	}
	break;

	case 0x422:
	{
		SendClientMessage(clientId, g_pLanguageString[_NN_Wrong_Password]);

		AddFailAcount(pUser[clientId].User.Username);

		LogPlayer(clientId, "Senha incorreta enviada");

		pUser[clientId].SendMessageA();
		CloseUser(clientId);
	}
	break;
	case 0x424:
	{
		SendClientMessage(clientId, g_pLanguageString[_NN_Blocked_Account]);

		pUser[clientId].SendMessageA();
		CloseUser(clientId);
	}
	break;

	case 0x419:
	{
		p112 *p = (p112*)(header);

		header->PacketId = 0x112;
		header->ClientId = 0x7531;

		pUser[clientId].AddMessage((BYTE*)header, header->Size);

		// Copia a estrutura da charlist para o User
		memcpy(&pUser[clientId].CharList, &p->CharList, sizeof p->CharList);

		pUser[clientId].Status = USER_SELCHAR;
	}
	break;
	case 0x426:
	{
		SendClientMessage(clientId, "Servidor em manutenaao. Verifique o site para maiores infos.");

		pUser[clientId].SendMessageA();
		CloseUser(clientId);
	}
	break;
	case 0x425:
	{
		SendClientMessage(clientId, g_pLanguageString[_NN_Disabled_Account]);

		pUser[clientId].SendMessageA();
		CloseUser(clientId);
	}
	break;
	case 0x41F:
	{
		//	SendSignal(clientId, 0x7532, 0x11C);
		SendClientMessage(clientId, g_pLanguageString[_NN_Wrong_Password]);

		pUser[clientId].Status = USER_SELCHAR;
		//	Users[clientId].SendMessageA();
		//	CloseUser(clientId);
	}
	break;
	case 0xFDF:
	{
		CUser *user = &pUser[clientId];

		user->inGame.incorrectNumeric++;
		if (user->inGame.incorrectNumeric >= 4)
		{
			CloseUser(clientId);

			return false;
		}

		header->ClientId = 0;

		Log(clientId, LOG_INGAME, "Errou senha numarica: %d.", user->inGame.incorrectNumeric);
		pUser[clientId].AddMessage(pBuffer, size);
	}
	break;

	case 0x820:
		return pUser[clientId].GetCharInfo(header);

	case 0x417:
		pUser[clientId].SendCharToWorld(header);
		break;

	case 0x80E:
	{
		pMsgSignal *p = (pMsgSignal*)(header);

		if (p->Value == 1)
			sServer.Sapphire <<= 1;
		else
			sServer.Sapphire >>= 1;

		if (sServer.Sapphire < 1)
			sServer.Sapphire = 1;

		if (sServer.Sapphire > 35)
			sServer.Sapphire = 35;
	}
	break;
	// SignalParam charlist
	case 0x423:
	{
		pMsgSignal2 *p = (pMsgSignal2*)(pBuffer);

		if (p->Value == 1)
			sServer.Sapphire = p->Value2;
		else if (p->Value == 2)
			sServer.FirstKefra = p->Value2;
	}
	break;
	case 0x427:
	{
		SendClientMessage(header->ClientId, g_pLanguageString[_NN_Wrong_Password]);

		return true;
	}
	break;

	case 0x416:
	{
		pUser[clientId].SendCharList(header);
		break;
	}
	case 0x418:
		pUser[clientId].ResendCharList(header);
		break;

	case 0xFDE:
		pUser[clientId].SendNumericToken(header);
		break;
	case SealInfoPacket:
	{
		pDC3 *packet = reinterpret_cast<pDC3*>(header);

		if (packet->Info.CurrentScore == -1)
		{
			CloseUser(clientId);

			return true;
		}

		auto sealIt = std::find_if(std::begin(sServer.SealCache), std::end(sServer.SealCache), [&](const InfoCache<STRUCT_SEALINFO> seal) {
			return seal.Info.CurrentScore == packet->Info.CurrentScore;
		});

		if (sealIt == std::end(sServer.SealCache))
		{
			InfoCache<STRUCT_SEALINFO> cache{};
			cache.Info = packet->Info;
			cache.Last = std::chrono::steady_clock::now();

			sServer.SealCache.push_back(cache);
		}
		else
		{
			sealIt->Last = std::chrono::steady_clock::now();
			sealIt->Info = packet->Info;
		}

		packet->Header.PacketId = 0xDC3;
		packet->Header.ClientId = SERVER_SIDE;

		pUser[clientId].AddMessage(reinterpret_cast<BYTE*>(packet), packet->Header.Size);
		return true;
	}
	break;
	case PutInSealSuccess:
		Log(clientId, LOG_INGAME, "Foi criado o Selo da Alma do personagem. Removendo todas as suas informaaaes");

		pMob[clientId].Mobs = STRUCT_CHARINFO{};
		break;
	case 0xE12:
	{
		pE12 *p = (pE12*)(header);
		if (IsWarTime())
		{
			SendClientMessage(clientId, "Nao a possavel realizar em horario de guerra");

			return true;
		}

		DoAlly(p->GuildIndex1, p->GuildIndex2);
		break;
	}
	/*
case MSG_REWARDWARTOWER_OPCODE:
{
	_MSG_REWARDWARTOWER *p = (_MSG_REWARDWARTOWER*)(header);

	g_pCityZone[4].impost += p->Coin;
	sServer.CitizenEXP.Bonus += p->Taxe;

	if(sServer.CitizenEXP.Bonus > MAX_CITIZENXP)
		sServer.CitizenEXP.Bonus = MAX_CITIZENXP;
}
break;

case _MSG_STARTTOWERWAR:
	{
		MSG_STARTTOWERWAR *p = (MSG_STARTTOWERWAR*)(header);

		// Pacote de inacio da guerra
		// Informa a situaaao e se esta comeaando
		if (!p->isStarting)
			FinalizeTowerWar(p->war);
		else if (p->isStarting == 1)
			InitializeTowerWar(p->war);
		else if(p->isStarting == 2)
			UpdateTowerWar(p->war);

		break;
	}

case MSG_UPDATEWARDECLARATION:
{
	_MSG_UPDATEWARANSWER *p = (_MSG_UPDATEWARANSWER*)(header);

	// Pega o canal atual
	// Deve ser decrementado um pois esta trabalhando de acordo com o 'conn' da DBsrv
	// que trabalha com andices de 0~9
	int thisChannel = sServer.Channel - 1;

	// Caso o canal que deve receber a informaaao seja o canal que recebeu este pacote
	// e a atual situaaao seja diferente, seta o novo valor
	// WarState = 1 : guerra declarada
	// WarState = 0 : guerra nao declarada
	if (p->receiver == thisChannel && sServer.TowerWar[sServer.Channel - 1].WarState != p->action)
		sServer.TowerWar[sServer.Channel - 1].WarState = p->action;

	char szMsg[120];

	// Soma somente para fins de mensagem
	BYTE declarant = p->declarant + 1,
		 receiver  = p->receiver  + 1;

	if (p->action)
		sprintf_s(szMsg, "O canal %d declara guerra ao canal %d!", declarant, receiver);
	else
		sprintf_s(szMsg, "O canal %d recua!", declarant);

	SendNotice(szMsg);
	break;
}*/

	case MSG_SEND_SERVER_NOTICE:
	{
		_MSG_SEND_SERVER_NOTICE *p = (_MSG_SEND_SERVER_NOTICE*)(header);

		if (strlen(p->Notice) > 0)
			SendNotice(p->Notice);

		break;
	}

	case MSG_FIRST_KEFRA_NOTIFY:
	{
		int thisChannel = sServer.Channel - 1;

		_MSG_FIRST_KEFRA_NOTIFY *p = (_MSG_FIRST_KEFRA_NOTIFY*)(header);
		if (thisChannel != p->Channel)
			sServer.FirstKefra = FALSE;
		else
			sServer.FirstKefra = TRUE;

		break;
	}

	case MSG_REBORN_KEFRA:
		RebornKefra();
		break;
	}
	return true;
}
/*
	if(Header->PacketId == 0x3c9)
	{
	char tmp [ 1024 ];
	sprintf_s(tmp, "packet//pacote_%0X.txt", Header->PacketId);

	FILE *pFile = NULL;
	fopen_s(&pFile, tmp, "a+");

	if(pFile)
	{
		fprintf(pFile, "Send -> Packet: 0x%03X Size : %d ClientId: %d\n",Header->PacketId,Header->Size,Header->ClientId);

		fprintf(pFile, "   %05d   ", 0);
		for(int i=0; i < Header->Size;i++)
		{
			fprintf(pFile, "%02X ", pBuffer[i]);
			if(i != 0 && (i + 1) % 10 == 0)
			{
				fprintf(pFile, "  ");
				for(int x = (i - 10); x < i; x++)
				{
					if(pBuffer[x]  == 0)
						fprintf(pFile, ".");
					else
						fprintf(pFile, "%c", pBuffer[x]);
				}

				fprintf(pFile, "\n");
				fprintf(pFile, "   %05d   ", (i + 1));
			}
		}

		fprintf(pFile, "\n\n");
		fclose(pFile);
	}
	}
	*/
bool CUser::PacketControl(BYTE *pBuffer, INT32 size)
{
	PacketHeader *Header = (PacketHeader*)pBuffer;

	if (!CheckPacket(Header) && Header->PacketId != 0x334)
	{
		AddCrackError(clientId, 4, CRACK_USER_PKTHACK);

		Log(clientId, LOG_HACK, "CheckPacket Fail: %X - %d", Header->PacketId, Header->Size);
		return true;
	}

	if(TimeStamp.TimeStamp != 0x0E0A1ACA && Header->TimeStamp != 0x0E0A1ACA)
		Header->ClientId = clientId;

	TimeStamp.LastReceiveTime = sServer.SecCounter;

	if (Header->PacketId == 0x3AE)
		return true;

	switch (Header->PacketId)
	{
	case 0x655:
	case 0x656:
		return RequestClientInfo(Header);

	case 0x3E8:
		return RequestRepurchase(Header);

	case 0x20D:
		return RequestLogin(Header);
	case 0x20F:
		return RequestCreateChar(Header);
	case 0x36C:
	case 0x366:
	case 0x368:
		return RequestAction(Header);
	case 0x37A:
		return RequestSellShop(Header);
	case 0x39E:
	case 0x39D:
	case 0x367:
		return RequestAttack(Header);

	case 0x2CB:
		return true;

	case 0x215:
		Log(clientId, LOG_INGAME, "Personagem deu personagem");

		CharLogOut(clientId);
		break;

	case 0x903:
		return RequestBlockItems(Header);
	case 0x333:
		return RequestChat(Header);
	case 0x334:
		return RequestCommand(Header);
	case 0x290:
		return RequestTeleport(Header);
	case 0x376:
		return RequestMoveItem(Header);
	case 0x397:
		return RequestCreateAutoTrade(Header);
	case 0x398:
		return RequestBuyAutoTrade(Header);
	case 0x39A:
		return RequestOpenAutoTrade(Header);
	case 0x384:
		return RequestCloseAutoTrade(Header);
	case 0x291:
		return RequestChangeCity(Header);
	case 0x277:
		return RequestAddPoint(Header);
	case 0x27B:
		return RequestOpenShop(Header);
	case 0x379:
		return RequestBuyShop(Header);
	case 0x374:
		return RequestOpenGate(Header);
	case 0x289:
		return RequestRessuctPlayer(Header);
	case 0x369:
	{
		pMsgSignal *p = (pMsgSignal*)(Header);
		INT32 mobId = p->Value;
		if (mobId <= 0 || mobId >= MAX_MOB)
			return true;

		if (pMob[mobId].Mode == 0)
		{
			SendRemoveMob(clientId, mobId, 0, 0);

			return true;
		}

		if (mobId < MAX_PLAYER && pUser[mobId].Status != USER_PLAY)
		{
			SendRemoveMob(clientId, mobId, 0, 0);

			return true;
		}

		if (!GetInHalf(mobId, clientId))
		{
			SendRemoveMob(clientId, mobId, 0, 0);

			return true;
		}

		SendCreateMob(clientId, mobId, 1);
	}
	break;
	case 0x36A:
		return RequestMotion(Header);

	case 0x3A6:
		return RequestCompounder(Header);

	case 0x2C3:
		return RequestLindy(Header);

	case 0x2D2:
		return RequestOdin(Header);

	case 0x2D3:
		return RequestEhre(Header);

	case 0x2C4:
		return RequestShany(Header);

	case 0x3BA:
		return RequestAgatha(Header);

	case 0x3B5:
		return RequestAylin(Header);

	case 0x3C0:
		return RequestTiny(Header);

	case 0x3D5:
		return RequestRecruitMember(Header);

	case 0x28C:
		return RequestKickMember(Header);

	case 0x213:
	{
#if !defined(_DEBUG)
		if (TokenOk)
		{
			if (Status != USER_SELCHAR)
				return true;

			Header->ClientId = clientId;
			Header->PacketId = 0x804;
			Log(clientId, LOG_INGAME, "Solicitou ir ao mundo...");

			return AddMessageDB((BYTE*)Header, sizeof p213);
		}
		else
		{
			SendSignal(clientId, 0x7530, 0xFDF);
			SendClientMessage(clientId, "Digite a senha numarica");
		}
		return true;
#else
		if (Status != USER_SELCHAR)
			return true;

		Header->PacketId = 0x804;
		Log(clientId, LOG_INGAME, "Solicitou ir ao mundo...");

		return AddMessageDB((BYTE*)Header, Header->Size);
#endif
	}

	case 0xFDE:
	{
		pFDE* p = (pFDE*)Header;
		Header->ClientId = clientId;

		if (p->RequestChange == 1)
		{
			if (TokenOk)
				AddMessageDB((BYTE*)Header, Header->Size);

			return true;
		}
		return AddMessageDB((BYTE*)Header, sizeof pFDE);
	}

	case 0x2D4:
		return RequestExtraction(Header);
	case 0x211:
		return RequestDeleteChar(Header);

	case 0x378:
		return RequestChangeSkillbar(Header);

	case 0x37F:
		return RequestAddParty(Header);

	case 0x3AB:
		return RequestAcceptParty(Header);

	case 0x37E:
		return RequestExitParty(Header);

	case 0x373:
		return RequestUseItem(Header);

	case 0x272:
		return RequestDropItem(Header);

	case 0x2E4:
		return RequestDeleteItem(Header);

	case 0x270:
		return RequestPickItem(Header);

	case 0x399:
	{
		pMsgSignal *p = (pMsgSignal*)(Header);

		AllStatus.PK = p->Value;

		if (Trade.ClientId)
		{
			if (pUser[Trade.ClientId].Status == USER_PLAY && pUser[Trade.ClientId].Trade.ClientId == clientId)
			{
				SendClientMessage(clientId, "Nao a possavel trocar com o modo PvP ativo");
				SendClientMessage(Trade.ClientId, "O outro jogador esta com o modo PvP ativo");

				RemoveTrade(Trade.ClientId);
				RemoveTrade(clientId);
			}
		}

	}
	break;

	case 0x383:
		return RequestTrade(Header);

	case 0x387:
		return RequestTransferGoldToInv(Header);

	case 0x388:
		return RequestTransferGoldToBank(Header);

	case 0x39F:
		return RequestDuel(Header);

	case 0x28B:
		return RequestMerchantNPC(Header);

	case 0x2E5:
		return RequestUngroupItem(Header);

	case 0xAD9:
		return RequestGriffinMaster(Header);

	case 0xE12:
		return RequestAlly(Header);

	case 0x3A0:
		break;

	case 0x28E:
		return RequestReqChallange(Header);

		// Provavelmente desativado
		//case 0x18D:
			//return RequestChallange(Header);

	case MSG_PANELGUILD_GETLIST:
		AddMessageDB((BYTE*)Header, size);
		break;
	case 0xED7:
		return RequestDeclareWar(Header);

	case 0x2E1:
		return RequestAlchemy(Header);

	case 0x2C7:
		return static_cast<CQuiz*>(CEventManager::GetInstance().GetEvent(eEventType::Quiz))->HandlePacket(*this, reinterpret_cast<MSG_QUIZ_ANSWER*>(Header));



	case 0x2CD:
		return RequestSealInfo(Header);

	case 0x3CC:
		return RequestPutOutSeal(Header);

	case UseEssencePacket:
		return RequestEssenceUse(Header);

	case RecruitAcceptPacket:
		return RequestAcceptRecruit(Header);

	case 0x3776:
	{
		p376* p = reinterpret_cast<p376*>(Header);
		AddMessage(reinterpret_cast<BYTE*>(Header), Header->Size);
		break;
	}
	case NightmareAcceptPacket:
		return RequestNightmareAccept(Header);
	case RedeemGriffinPacket:
		return RequestRedeemGriffin(Header);
	case WPEOnWarnPacket:
		CloseUser(clientId);
		break;

	default:
		printf("Pacote desconhecido recebido - 0x%X - Size: %d (%s)\n", Header->PacketId, Header->Size, User.Username);
		break;
}

	return true;
}