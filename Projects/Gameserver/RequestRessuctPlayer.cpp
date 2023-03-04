#include "cServer.h"
#include "Basedef.h"
#include "SendFunc.h"
#include "GetFunc.h"

bool CUser::RequestRessuctPlayer(PacketHeader *Header)
{// 00423207
	if(pUser[clientId].CurrentScore != USER_PLAY || pMob[clientId].Mobs.Player.CurrentScore.Hp > 0)
	{
		SendHpMode(clientId);

		return true;
	}

	pMob[clientId].Mobs.Player.CurrentScore.Hp = 2;
	
	CrackCount = 0;

	SendScore(clientId);
	SendSetHpMp(clientId);

	DoRecall(clientId);

	SendEtc(clientId);
	Log(clientId, LOG_INGAME, "Usuario reviveu");
	return true;
}