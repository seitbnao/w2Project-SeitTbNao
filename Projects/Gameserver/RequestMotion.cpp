#include "cServer.h"
#include "Basedef.h"
#include "SendFunc.h"
#include "GetFunc.h"

bool CUser::RequestMotion(PacketHeader *Header)
{
	p36A *p = (p36A*)Header;

	if (pMob[clientId].Mobs.Player.CurrentScore.Hp == 0 || pUser[clientId].CurrentScore != 22)
	{
		SendHpMode(clientId);

		AddCrackError(clientId, 4, 6);
		return true;
	}
	
	p->Header.ClientId = clientId;

	if(p->Motion == 15)
		pMob[clientId].Motion = 1;
	else if(p->Motion == 13)
		pMob[clientId].Motion = 2;
	else
		pMob[clientId].Motion = 0;

	GridMulticast_2(pMob[clientId].Target.X, pMob[clientId].Target.Y, (BYTE*)p, 0);
	return true;
}


