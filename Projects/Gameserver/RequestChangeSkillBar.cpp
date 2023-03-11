#include "cServer.h"
#include "Basedef.h"
#include "SendFunc.h"
#include "GetFunc.h"

bool CUser::RequestChangeSkillbar(PacketHeader* Header)
{
	p378* p = (p378*)Header;

	if (Status != USER_PLAY)
		return false;


	for (int o = 0; o < 4; o++)
	{
		auto skillId = p->ShortSkill[o];
		if (skillId != -1 && skillId >= 105 && skillId < 153)
			skillId -= 12 * pMob[clientId].Mobs.Player.Class;

		pMob[clientId].Mobs.Player.ShortSkill[o] = skillId;
		p->ShortSkill[o] = skillId;
	}

	for (int o = 0; o < 16; o++)
	{
		auto skillId = p->SkillBar2[o];
		if (skillId != -1 && skillId >= 105 && skillId < 153)
			skillId -= 12 * pMob[clientId].Mobs.Player.Class;

		pMob[clientId].Mobs.SkillBar[o] = skillId;
		p->SkillBar2[o] = skillId;
	}

	AddMessage((BYTE*)Header, p->Header.Size);
	return true;
}