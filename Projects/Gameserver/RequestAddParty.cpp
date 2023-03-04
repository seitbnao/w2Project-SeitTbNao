#include "cServer.h"
#include "Basedef.h"
#include "SendFunc.h"
#include "GetFunc.h"

bool CUser::RequestAddParty(PacketHeader *Header)
{
	p37F *p = (p37F*)Header;
	
	// Checagens de seguranêa
	if(p->targetId <= 0 || p->targetId >= MAX_PLAYER)
		return false;

	if(p->leaderId <= 0 || p->leaderId >= MAX_PLAYER)
		return false;

	if(pUser[p->leaderId].CurrentScore != USER_PLAY || pUser[p->targetId].CurrentScore != USER_PLAY)
		return true;

	CMob *liderSpw = &pMob[p->leaderId];
	CMob *targetSpw = &pMob[p->targetId];

	if(liderSpw->Leader != 0)
	{
		SendClientMessage(clientId, "Saia do grupo para criar um.");

		return true;
	}
	else if(targetSpw->Leader != 0)
	{
		SendClientMessage(clientId, "O outro jogador ja possui grupo.");

		return true;
	}

	if(targetSpw->Leader == 0)
	{
		bool has = false;
		for(int i = 0 ; i < 12; i++)
		{
			if(pMob[p->targetId].PartyList[i] != 0)
			{
				has = true;
				break;
			}
		}

		if(has)
		{
			SendClientMessage(clientId, "O outro jogador ja ê lêder de um grupo.");

			return true;
		}
	}

	p->partyId = 1;
	p->unk = 204;
	p->Header.ClientId = 0x7530;

	pUser[p->targetId].AddMessage((BYTE*)p, sizeof p37F);
	return true;
}