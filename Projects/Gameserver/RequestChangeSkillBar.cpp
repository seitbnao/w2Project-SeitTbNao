#include "cServer.h"
#include "Basedef.h"
#include "SendFunc.h"
#include "GetFunc.h"

bool CUser::RequestChangeSkillbar(PacketHeader *Header)
{
	p378 *p = (p378*)Header;

	if(Status != USER_PLAY)
		return false;
	
#if defined(_DEBUG)	
	for (int o = 0; o < 4; o++)
	{
		auto skillId = p->ShortSkill[o];
		if (skillId != 255 && skillId >= 105 && skillId < 153)
			skillId -= 12 * pMob[clientId].Mobs.Player.Class;

		pMob[clientId].Mobs.Player.ShortSkill[o] = skillId;
		p->ShortSkill[o] = skillId;
	}

	for (int o = 0; o < 16; o++)
	{
		auto skillId = p->SkillBar2[o];
		if (skillId != 255 && skillId >= 105 && skillId < 153)
			skillId -= 12 * pMob[clientId].Mobs.Player.Class;

		pMob[clientId].Mobs.SkillBar[o] = skillId;
		p->SkillBar2[o] = skillId;
	}

	AddMessage((BYTE*)Header, p->Header.Size);
	return true;
#endif

	// Anti skillbar hack
	// Checa se existe a skill realmente no personagem para coloca-la em sua barra de skills
	for(INT32 i = 0; i < 4; i++)
	{
		unsigned char skillId = p->ShortSkill[i];
		if (skillId >= 104)
		{
			p->ShortSkill[i] = 255;

			continue;
		}
		 
		if(skillId < 96)
		{
			if(!(pMob[clientId].Mobs.Player.LearnedSkill[0] & (1 << (skillId % 24))))
				p->ShortSkill[i] = 255;
		}
		else if(skillId >= 96 && skillId < 105)
		{
			if(!(pMob[clientId].Mobs.Player.LearnedSkill[0] & (1 << (24 + skillId - 96))))
				p->ShortSkill[i] = 255;
		}

	}

	for(INT32 i = 0; i < 16; i++)
	{
		unsigned char skillId = p->SkillBar2[i];
		if(skillId < 0 || skillId >= 104)
		{
			p->SkillBar2[i] = 255;

			continue;
		}

		if(skillId < 96)
		{
			if(!(pMob[clientId].Mobs.Player.LearnedSkill[0] & (1 << (skillId % 24))))
				p->SkillBar2[i] = 255;
		}
		else if (skillId >= 96 && skillId < 105)
		{
			if(!(pMob[clientId].Mobs.Player.LearnedSkill[0] & (1 << (24 + skillId - 96))))
				p->SkillBar2[i] = 255;
		}

	}

	memcpy(pMob[clientId].Mobs.Player.ShortSkill, p->ShortSkill, 4);
	memcpy(pMob[clientId].Mobs.SkillBar, p->SkillBar2, 16);

	AddMessage((BYTE*)Header, p->Header.Size);
	return true;
}