#include "cServer.h"
#include "Basedef.h"
#include "SendFunc.h"
#include "GetFunc.h"

bool CUser::RequestOpenAutoTrade(PacketHeader *Header)
{
	p39A *p = (p39A*)(Header);

	INT32 idx = p->sIndex;
	if (pMob[clientId].Mobs.Player.CurrentScore.Hp == 0 || pUser[clientId].Status != USER_PLAY)
	{
		SendHpMode(clientId);
		AddCrackError(clientId, 10, 87);
		return true;
	}

	if (idx <= 0 || idx >= MAX_PLAYER)
		return false;

	if (!pUser[idx].IsAutoTrading)
		return false;

	SendClientMessage(idx, "%s abriu sua loja", pMob[clientId].Mobs.Player.MobName);

	SendAutoTrade(clientId, idx);
	return true;
}