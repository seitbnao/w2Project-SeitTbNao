#include "cServer.h"
#include "Basedef.h"
#include "SendFunc.h"
#include "GetFunc.h"


#pragma optimize( "", off )
bool CUser::RequestAddPoint(PacketHeader *header)
{
	//00436263 

	p277 *p = (p277*)header;
	if (p->Header.Size != sizeof p277)
	{
		Log(SERVER_SIDE, LOG_INGAME, "Error: p is size different from p277 %hu", p->Header.Size);

		return true;
	}
	if (header->ClientId <= 0 || header->ClientId >= MAX_PLAYER)
		return true;

	int clientId = p->Header.ClientId;

	if (pMob[clientId].Mobs.Player.CurrentScore.Hp == 0 || pUser[clientId].CurrentScore != USER_PLAY)
	{
		SendHpMode(clientId);
		AddCrackError(clientId, 10, 20);

		return true;
	}

	if (p == nullptr)
	{
		Log(SERVER_SIDE, LOG_INGAME, "Error: p is null %s %s", __FILE__, __FUNCTION__);

		return true;
	}

	if (p->Mode == 0)
	{
		if (pMob[clientId].Mobs.Player.ScoreBonus <= 0)
		{
			SendEtc(clientId);
			return true;
		}

		if (p->Info < 0 || p->Info > 3)
		{
			Log(clientId, LOG_ERROR, "Error no ApplyBonus, info fora de indice");

			return true;
		}

		int addPoints = 1;
		if (pMob[clientId].Mobs.Player.ScoreBonus > 200)
			addPoints = 100;

		pMob[clientId].Mobs.Player.ScoreBonus -= addPoints;

		if (p->Info == 0)
			pMob[clientId].Mobs.Player.BaseScore.Str += addPoints;

		if (p->Info == 1)
			pMob[clientId].Mobs.Player.BaseScore.Int += addPoints;

		if (p->Info == 2)
			pMob[clientId].Mobs.Player.BaseScore.Dex += addPoints;

		if (p->Info == 3)
			pMob[clientId].Mobs.Player.BaseScore.Con += addPoints;

		pMob[clientId].GetCurrentScore(clientId);
		SendScore(clientId);
		SendEtc(clientId);
		return true;
	}
	else if (p->Mode == 1)
	{
		if (pMob[clientId].Mobs.Player.SpecialBonus <= 0)
		{
			SendEtc(clientId);

			return true;
		}

		if (p->Info < 0 || p->Info > 3)
		{
			Log(clientId, LOG_ERROR, "ApplyBonus Special, Info fora de indice.");

			return true;
		}

		// info mastery[0] level
		if (pMob[clientId].Mobs.Player.BaseScore.Special[p->Info] >= (((pMob[clientId].Mobs.Player.BaseScore.Level + 1) * 3) >> 1) && pMob[clientId].Mobs.Player.Equip[0].EFV2 < CELESTIAL)
		{
			SendClientMessage(clientId, g_pLanguageString[_NN_Maximum_Point_Now]);
			return true;
		}

		int max = 0;
		if (pMob[clientId].Mobs.Player.Equip[0].EFV2 >= CELESTIAL)
			max = 200;

		int has = pMob[clientId].Mobs.Player.LearnedSkill[0] & (1 << (8 * p->Info - 1));
		max = (has) ? 255 : 200;

		if (pMob[clientId].Mobs.Player.BaseScore.Special[p->Info] >= max)
		{
			SendClientMessage(clientId, g_pLanguageString[_NN_Maximum_Point_200_Now]);
			return true;
		}

		pMob[clientId].Mobs.Player.SpecialBonus -= 1;
		pMob[clientId].Mobs.Player.BaseScore.Special[p->Info] += 1;

		pMob[clientId].GetCurrentScore(clientId);
		SendScore(clientId);
		SendEtc(clientId);
		return true;
	}

	else if (p->Mode == 2)
	{
		int classInfo = pMob[clientId].Mobs.Player.Class;

		int skillClass = (p->Info - 5000) / 24;
		int skillPos = (p->Info - 5000) % 24;

		int mobId = p->unk;

		if (p->Info < 5000 || p->Info > 5095)
		{
			Log(clientId, LOG_ERROR, "ApplyPoint RequestSkill out of bound");
			return true;
		}

		if (mobId < 0 || mobId >= 30000)
		{
			Log(clientId, LOG_ERROR, "ApplyBonus RequestTargetId is out of bound");
			return true;
		}

		if (classInfo != skillClass)
		{
			SendClientMessage(clientId, g_pLanguageString[_NN_Cant_Learn_Other_Class_Skill]);

			return true;
		}

		int skillBonus = SkillData[skillClass * 24 + skillPos].Points;
		if (skillBonus > pMob[clientId].Mobs.Player.SkillPoint)
		{
			SendClientMessage(clientId, g_pLanguageString[_NN_Not_Enough_Skill_Point]);
			return true;
		}

		unsigned int learned = 1 << skillPos;
		if ((learned & pMob[clientId].Mobs.Player.LearnedSkill[0]) != 0)
		{
			SendClientMessage(clientId, g_pLanguageString[_NN_Already_Learned_It]);
			return true;
		}

		unsigned int level = g_pItemList[p->Info].Level;
		if (level > pMob[clientId].Mobs.Player.CurrentScore.Level && pMob[clientId].Mobs.Player.Equip[0].EFV2 < CELESTIAL)
		{
			SendClientMessage(clientId, g_pLanguageString[_NN_Need_More_Level_To_Learn]);
			return true;
		}

		if (pMob[clientId].Mobs.Player.CurrentScore.Special[1] < g_pItemList[p->Info].Int ||
			pMob[clientId].Mobs.Player.CurrentScore.Special[2] < g_pItemList[p->Info].Dex ||
			pMob[clientId].Mobs.Player.CurrentScore.Special[3] < g_pItemList[p->Info].Con)
		{
			SendClientMessage(clientId, g_pLanguageString[_NN_Need_More_Mastery_To_Learn]);
			return true;
		}

		static int gold[4] =
		{
			5000000,
			10000000,
			50000000,
			50000000,
		};

		if ((skillPos % 24) % 8 == 7)
		{
			for (int i = 1; i < 8; i++)
			{
				int have = pMob[clientId].Mobs.Player.LearnedSkill[0] & (1 << (skillPos - i));
				if (!have)
				{
					SendClientMessage(clientId, g_pLanguageString[_NN_Need_All_Skill_Lineage]);

					return true;
				}
			}

			int skillDiv = ((p->Info - 5000) % 24 / 8) + 1;
			for (int i = 1; i < 4; i++)
			{
				if (i == skillDiv)
					continue;

				int have = pMob[clientId].Mobs.Player.LearnedSkill[0] & (1 << ((i * 8) - 1));
				if (have)
				{
					SendClientMessage(clientId, g_pLanguageString[_NN_Only_A_8th_Per_Char]);

					return true;
				}
			}

			if (pMob[clientId].Mobs.Player.Coin < gold[pMob[clientId].Mobs.Player.Equip[0].EFV2 - 1])
			{
				SendClientMessage(clientId, g_pLanguageString[_NN_Need_X_Gold_To_Buy_Skill]);

				return true;
			}
		}

		Log(clientId, LOG_INGAME, "Comprou uma skill %s. Learn antigo: %llu, Learn atual: %llu. Learn da skill: %d.", g_pItemList[p->Info].ItemName,
			pMob[clientId].Mobs.Player.LearnedSkill, pMob[clientId].Mobs.Player.LearnedSkill[0] | learned, learned);

		pMob[clientId].Mobs.Player.LearnedSkill[0] = pMob[clientId].Mobs.Player.LearnedSkill[0] | learned;
		pMob[clientId].Mobs.Player.SkillPoint -= skillBonus;

		if ((skillPos % 24) % 8 == 7)
			pMob[clientId].Mobs.Player.Coin -= gold[pMob[clientId].Mobs.Player.Equip[0].EFV2 - 1];

		pMob[clientId].GetCurrentScore(clientId);
		SendScore(clientId);
		SendEtc(clientId);
		return true;
	}
	else
	{
		Log(clientId, LOG_ERROR, "ApplyBonus out of range.");

		return true;
	}
	return true; 
}
#pragma optimize("", on)

