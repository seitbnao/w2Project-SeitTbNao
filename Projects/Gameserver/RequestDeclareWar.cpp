#include "cServer.h"
#include "Basedef.h"
#include "SendFunc.h"
#include "GetFunc.h"

bool CUser::RequestDeclareWar(PacketHeader *Header)
{
	pED7 *p = (pED7*)(Header);

	if(p->server > sServer.TotalServer)
	{
		AddCrackError(clientId, 5, CRACK_USER_PKTHACK);
		return true;
	}

	if(pMob[clientId].Mobs.Player.GuildLevel != 9 && !IsAdmin)
		return true;

	if (ChargedGuildList[sServer.Channel - 1][4] != pMob[clientId].Mobs.Player.Guild && !IsAdmin) // Somente caso possua a coroa
		return true;

	int slot = GetFirstSlot(clientId, 4030);
	if (slot == -1) // Mandou o pacote sem possuir declaração
		return true;

	AmountMinus(&pMob[clientId].Mobs.Player.Inventory[slot]);
	SendItem(clientId, SlotType::Inv, slot, &pMob[clientId].Mobs.Player.Inventory[slot]);

	 

	return true;
}