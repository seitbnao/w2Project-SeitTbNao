#include "cServer.h"
#include "Basedef.h"
#include "SendFunc.h"
#include "GetFunc.h"
#include "CNPCGener.h"

bool CUser::RequestDeleteChar(PacketHeader* header)
{
	p211 *p = (p211*)(header);
	p->MobName[15] = '\0';
	p->Pwd[11] = '\0';

	STRUCT_MOB* mob = &pMob[clientId].Mobs.Player;
	if (Status != USER_SELCHAR)
	{
		SendClientMessage(clientId, "Deleting Character, wait a moment");

		Log(clientId, LOG_HACK, "Tentativa de deletar personagem enquanto nêo na CharList. Status atual: %d", Status);
		return true;
	}

	for (int i = 1; i < 15; i++)
	{
		if (mob->Equip[i].sIndex <= 0 || mob->Equip[i].sIndex >= MAX_ITEMLIST)
			continue;

		SendClientMessage(clientId, "Desequipe todos os itens do personagem para excluir");
		return true;
	}

	if (pMob[clientId].Mobs.Player.Coin != 0)
	{
		SendClientMessage(clientId, "Nêo ê possêvel excluir enquanto possuir gold no inventario");
		return true;
	}

	Log(clientId, LOG_INGAME, "Solicitado deletar personagem %s", p->MobName);

	Status = USER_DELWAIT;

	header->PacketId = 0x809;
	return AddMessageDB((BYTE*)header, sizeof p211);
}