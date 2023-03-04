#include "cServer.h"
#include "Basedef.h"

bool CUser::RequestCreateChar(PacketHeader *Header)
{
	p20F *p = (p20F*)Header;

	// Faz as checagens de segurança, somente após isto
	// É feito as checagens de índice invalido

	if (p->ClassID < 0 || p->ClassID > 3)
		return false;

	if (p->SlotID < 0 || p->SlotID > 3)
		return false;

	// BUG Da própria TMsrv
	// Checagem se o usuario esta logado
	// Caso não esteja, ele retorna falso pois não é possível criar personagem ingame.
	if(CurrentScore != USER_SELCHAR)
		return false;

	// Null character
	p->Nick[12] = '\0';

	p->Header.PacketId = 0x802;

	AddMessageDB((BYTE*)Header, sizeof p20F);

	// Seta como quem esta criando um personagem
	CurrentScore = USER_CREWAIT;

	return true;
}