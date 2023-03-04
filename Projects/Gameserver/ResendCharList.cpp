#include "cServer.h"
#include "Basedef.h"
 
bool CUser::ResendCharList(PacketHeader *Header)
{
	p112 *p = (p112*)(Header);
	
	// Copia a estrutura da charlist para o User
	memcpy(&this->CharList, &p->CharList, sizeof p->CharList);

	for (int iChar = 0; iChar < 4; ++iChar)
	{
		if (!p->CharList.MobName[iChar][0])
			continue;

		if (p->CharList.Equip[iChar][12].sIndex == 0)
			continue;

		for (int i = 1; i < 11; ++i)
		{
			for (int iEffect = 0; iEffect < 3; ++iEffect)
			{
				if (p->CharList.Equip[iChar][i].stEffect[iEffect].cEffect >= 116 && p->CharList.Equip[iChar][i].stEffect[iEffect].cEffect <= 125)
				{
					p->CharList.Equip[iChar][i].stEffect[iEffect].cEffect = EF_SANC;

					break;
				}
			}
		}
	}

	// Seta como se o usuari oestivesse na charList
	CurrentScore = USER_SELCHAR;
	
	// ClientID do pacote
	p->Header.PacketId = 0x110;
	p->Header.ClientId = 0x7531;

	AddMessage((BYTE*)p, sizeof p112);
	return true;
}