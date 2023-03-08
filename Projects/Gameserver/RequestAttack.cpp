#include "cServer.h"
#include "Basedef.h"
#include "SendFunc.h"
#include "GetFunc.h"
#include "CEventManager.h"

//func_40195B (targetIdx, 36, masteryTmp + 100, masteryTmp);
INT32 func_40195B(INT32 clientId, INT32 arg2, INT32 arg3, INT32 arg4)
{
	if (arg2 >= 104)
		return 0;

	if (pMob[clientId].Mobs.Player.Info.Merchant == 1 && clientId >= MAX_PLAYER)
		return 0;

	INT32 LOCAL_1 = SkillData[arg2].AffectType;
	INT32 LOCAL_2 = SkillData[arg2].Aggressive;
	INT32 LOCAL_3 = SkillData[arg2].Time;
	INT32 LOCAL_4 = (arg2 / 24) & 0x80000003;
	INT32 LOCAL_5 = (arg2 % 24) >> 3;
	INT32 LOCAL_6 = SkillData[arg2].TickType;
	INT32 LOCAL_7 = 0;

	// 004C4091
	if (arg2 == 47 && pMob[clientId].Mobs.AffectInfo & 0x200 && LOCAL_2 != 0)
		return 0;

	else if (arg2 == 47 && pMob[clientId].Mobs.AffectInfo & 0x200)
	{
		for (INT32 LOCAL_8 = 0; LOCAL_8 < 32; LOCAL_8++)
		{
			if (pMob[clientId].Mobs.Affects[LOCAL_8].Index == 19)
			{
				pMob[clientId].Mobs.Affects[LOCAL_8].Index = 0;
				pMob[clientId].Mobs.Affects[LOCAL_8].Time = 0;

				pMob[clientId].GetCurrentScore(clientId);

				SendScore(clientId);
				return 0;
			}
		}
	}

	if (arg2 == 102)
		LOCAL_7 = 15;
	else
	{
		LOCAL_7 = GetEmptyAffect(clientId, LOCAL_1);

		if (LOCAL_1 > 0)
			LOCAL_7 = GetEmptyAffect(clientId, LOCAL_1);

		if (LOCAL_6 > 0)
			LOCAL_7 = GetEmptyAffect(clientId, LOCAL_6);
	}

	// 0004C41D5
	if (LOCAL_7 >= 0 && LOCAL_7 < 32)
	{
		if (LOCAL_6 != 0)
		{
			pMob[clientId].Mobs.Affects[LOCAL_7].Index = LOCAL_6;
			pMob[clientId].Mobs.Affects[LOCAL_7].Value = SkillData[arg2].TickValue;
			pMob[clientId].Mobs.Affects[LOCAL_7].Time = arg3 * LOCAL_3 / 100;

			if (arg2 == 37 && pMob[clientId].Mobs.Player.LearnedSkill[0] & 0x8000)
				pMob[clientId].Mobs.Affects[LOCAL_7].Time += 3;

			if (arg3 >= 10000)
				pMob[clientId].Mobs.Affects[LOCAL_7].Time = 39;

			pMob[clientId].Mobs.Affects[LOCAL_7].Master = arg4;

			pMob[clientId].GetCurrentScore(clientId);
			return 1;
		}

		//4C42EB
		INT32 LOCAL_9 = pMob[clientId].Mobs.Affects[LOCAL_7].Index;

		pMob[clientId].Mobs.Affects[LOCAL_7].Index = LOCAL_1;
		pMob[clientId].Mobs.Affects[LOCAL_7].Value = SkillData[arg2].TickValue;

		INT32 LOCAL_10 = arg3 * LOCAL_3 / 100;

		//4C434C
		if (LOCAL_2 == 0 && arg2 <= 96)
		{
			if (pMob[clientId].Mobs.Player.LearnedSkill[0] & 0x80)
			{
				INT32 LOCAL_11 = pMob[clientId].Mobs.Player.Class; // ebp - 2Ch
				if (LOCAL_11 == 0)
				{
					if (LOCAL_4 == 0 && LOCAL_5 == 0)
						LOCAL_10 += 5;
				}
				else if (LOCAL_11 == 2)
				{
					if (LOCAL_4 == 2 && LOCAL_5 == 0)
						LOCAL_10 += 3;
				}
				else if (LOCAL_11 == 3)
				{
					if (LOCAL_4 == 3 && LOCAL_5 == 0)
						LOCAL_10 += 3;
				}
			}

			// 004C43E1
			if (pMob[clientId].Mobs.Player.LearnedSkill[0] & 0x8000)
			{
				INT32 LOCAL_12 = pMob[clientId].Mobs.Player.Class;

				if (LOCAL_12 > 3)
				{
					switch (LOCAL_12)
					{
					case 0:
						if (LOCAL_4 == 0 && LOCAL_5 == 1)
							LOCAL_10 += 5;
						break;
					case 1:
						if (LOCAL_4 == 1 && LOCAL_5 == 1)
							LOCAL_10 += 3;
						break;
					case 2:
						if (LOCAL_4 == 2 && LOCAL_5 == 1)
							LOCAL_10 += 5;
						break;
					}
				}
			}

			//004C4478
			if (pMob[clientId].Mobs.Player.LearnedSkill[0] & 0x800000)
			{
				INT32 LOCAL_13 = pMob[clientId].Mobs.Player.Class;

				if (LOCAL_13 == 1)
				{
					if (LOCAL_4 == 1 && LOCAL_5 == 2)
						LOCAL_10 += 3;
				}
				else if (LOCAL_13 == 2)
				{
					if (LOCAL_4 == 2 && LOCAL_5 == 2)
						LOCAL_10 += 3;
				}
				else if (LOCAL_13 == 3)
				{
					if (LOCAL_4 == 3 && LOCAL_5 == 2)
						LOCAL_10 += 3;
				}
			}
		}

		if (arg2 == 102)
		{
			if (arg4 == 1)
				LOCAL_10 = 50;
			else if (arg4 == 2)
				LOCAL_10 = 450;
			else if (arg4 == 3)
				LOCAL_10 = 100;
			else
			{
				LOCAL_10 = pMob[clientId].Mobs.Player.BaseScore.Level + 10;

				if (LOCAL_10 > 50)
					LOCAL_10 = 50;
			}
		}

		// 004C4550
		pMob[clientId].Mobs.Affects[LOCAL_7].Time = LOCAL_10;

		if (arg4 > 255)
			arg4 = 255;

		if (LOCAL_9 == LOCAL_1)
		{
			if (arg4 > pMob[clientId].Mobs.Affects[LOCAL_7].Master)
				pMob[clientId].Mobs.Affects[LOCAL_7].Master = arg4;
		}
		else
			pMob[clientId].Mobs.Affects[LOCAL_7].Master = arg4;

		if (arg3 >= 100000)
			pMob[clientId].Mobs.Affects[LOCAL_7].Time = 100000;

		pMob[clientId].GetCurrentScore(clientId);
		return 1;
	}

	return 0;
}

bool CUser::RequestAttack(PacketHeader *Header)
{
	p367 *p = (p367*)(Header);

	STRUCT_MOB *player = &pMob[clientId].Mobs.Player;
	if (pUser[clientId].Status != USER_PLAY)
	{
		AddCrackError(clientId, 3, CRACK_USER_STATUS);

		return false;
	}

	//  Player morto usando outra skill sem ser a ressureiaao.
	if (player->CurrentScore.Hp == 0 && p->skillId != 99)
	{
		SendHpMode(clientId);

		Log(clientId, LOG_HACK, "Usou uma skill estando com 0 de HP. SkillID: %d", p->skillId);
		return false;
	}

	DWORD clientTimeStamp = clock(); // local149
	if (pUser[clientId].TimeStamp.TimeStamp != 0x00E0A1ACA)
	{
		if (p->Header.TimeStamp == 0x0E0A1ACA)
		{
			Log(clientId, LOG_HACK, "Enviado pacote com TimeStamp 0x0E0A1ACA");

			return true;
		}

		if ((clientTimeStamp - pUser[clientId].TimeStamp.LastAttack) < 900)
		{	// addcrackerror 
			Log(clientId, LOG_HACK, "Ataque mais rapido que 900ms. Diferenaa %d", clientTimeStamp - pUser[clientId].TimeStamp.LastAttack);
			return true;
		}

		pUser[clientId].TimeStamp.LastAttack = clientTimeStamp;
	}

	int skillNum = p->skillId; // local151
	if (skillNum >= 0 && skillNum < 110 && SkillData[skillNum].Passive_Check == 1)
		return true;

	if (skillNum > 0 && skillNum < 97 && pUser[clientId].TimeStamp.TimeStamp != 0x00E0A1ACA)
	{
		//0042488F -> Checagem do tempo das skills
		//004249A3 -> Checagem da classe da skill

		if ((skillNum / 24) != pMob[clientId].Mobs.Player.Class) //local157
		{
			Log(clientId, LOG_HACK, "MSG_Attack, Request other class %d", p->skillId);

			AddCrackError(clientId, 10, CRACK_USER_PKTHACK);
			return true;
		}

		auto now = std::chrono::steady_clock::now();
		auto delay = SkillData[skillNum].Delay;
		if (player->AffectInfo.SkillDelay)
			delay--;

		int jewelIndex = pMob[clientId].Jewel;
		if (jewelIndex >= 0 && jewelIndex < 32 && (pMob[clientId].Mobs.Affects[jewelIndex].Value & 1) != 0)
			delay--;

		if (now - TimeStamp.Skills[skillNum] < std::chrono::seconds(delay) - 500ms)
		{
			Log(clientId, LOG_HACK, "MSG_Attack, Skill Delay. SkillId: %d. Delay original: %d. Delay com reduaaes: %d. Tempo: %lld", p->skillId, SkillData[skillNum].Delay, delay, std::chrono::duration_cast<std::chrono::milliseconds>((now - TimeStamp.Skills[skillNum])).count());

			return true;
		}

		TimeStamp.Skills[skillNum] = now;
	}

	INT32 userMastery = 0; // local158
	INT32 delay = 100; // local159
	INT32 LOCAL_160 = 0; // local160

	INT32 skillKind = 0; // local161
	INT32 learn = 0; // local162
	INT32 learnedSkill = 0; // local163
	if (skillNum >= 0 && skillNum < 96 && p->Header.TimeStamp != 0x0E0A1ACA)
	{
		skillKind = ((skillNum % 24) >> 3) + 1; // mastery
		learn = skillNum % 24;

		learnedSkill = 1 << (char)learn;

		if (skillKind <= 0 || skillKind > 3)
		{
			Log(clientId, LOG_HACK, "Tipo de habilidade fora do limite - MSG_Attack");
			return true;
		}
	}
	else if (skillNum >= 97 && skillNum <= 102 && p->Header.TimeStamp != 0x0E0A1ACA)
	{
		bool has = (pMob[clientId].Mobs.Player.LearnedSkill[0] & (1 << (24 + (skillNum - 96))));
		if (!has)
		{
			Log(clientId, LOG_HACK, "Enviado skill sephira que nao possui");

			return true;
		}
	}
	else if (skillNum >= 200 && skillNum <= 248)
	{
		Log(clientId, LOG_HACK, "Uso de skills nao habilitadas");
		Log(SERVER_SIDE, LOG_HACK, "[%s] %s Uso de skills nao habilitadas", User.Username, pMob[clientId].Mobs.Player.MobName);

		return true;
	}

	userMastery = player->CurrentScore.Special[skillKind];

	delay += userMastery;
	LOCAL_160 = userMastery;

	if (skillNum == 85) // escudo dourado
	{
		INT32 coin = userMastery * 100; // local167

		if (player->Coin < coin)
			return true;

		player->Coin -= coin;
		SendEtc(clientId);

		Log(clientId, LOG_INGAME, "Usado buff Escudo Dourado. Gold consumido: %d. Gold atual: %d", coin, player->Coin);
	}

	INT32 mp = player->CurrentScore.Mp; // local168
	INT32 reqMp = Potion.CountMp; // local169

	if (skillNum >= 0 && skillNum < 98 && p->Header.TimeStamp != 0x0E0A1ACA)
	{
		INT32 manaSpent = GetManaSpent(skillNum, player->SaveMana, userMastery); // local170

		if (skillNum == 31)
			manaSpent = mp;

		if (player->CurrentScore.Mp - manaSpent < 0)
		{
			SendSetHpMp(clientId);

			return true;
		}

		player->CurrentScore.Mp -= manaSpent;

		Potion.CountMp -= manaSpent;
		SetReqMp(clientId);
	}

	int master = 0; // local171
	if (!player->Class && !(pMob[clientId].Leader & 16384))
	{
		master = player->CurrentScore.Special[2] / 20;
		if (master < 0)
			master = 0;

		if (master > 15)
			master = 15;
	}
	else if (player->Class == 3 && p->Header.TimeStamp != 0x0E0A1ACA)
		DoRemoveHide(clientId);// DoRemoveHide -> Seria a funaao de retirar o invisavel

	//DoRemovePossuido(clientId);

	pMob[clientId].Motion = 0;

	INT32 userNewExp = 0; // local172
	INT32 userLevel = player->CurrentScore.Level; // local173

	eMapAttribute mapAtt = GetAttribute(pMob[clientId].Target.X, pMob[clientId].Target.Y); // local174

	INT32 hp = player->CurrentScore.Hp; // local175
	INT32 LOCAL_176 = 0; // local176
	unsigned char doubleCritical = 0; // local177
	INT32 iterator = 0; // local178

	int maxTarget = 0;
	if (p->Header.Size == sizeof p367)
		maxTarget = 13;
	else if (p->Header.Size == sizeof p39D)
		maxTarget = 1;
	else if (p->Header.Size == sizeof p39D + sizeof st_Target)
		maxTarget = 2;
	else
	{
		Log(clientId, LOG_HACK, "Enviado pacote de ataque com size diferente do esperado. Size enviado: %hu", p->Header.Size);

		AddCrackError(clientId, 1, CRACK_USER_PKTHACK);
		return true;
	}

	bool tryToSendSamePlayer = false;
	for (INT32 i = 0; i < maxTarget; i++)
	{
		if (p->Target[i].Index == 0)
			continue;

		for (INT32 t = (i + 1); t < maxTarget; t++)
		{
			if (p->Target[i].Index == p->Target[t].Index)
			{
				tryToSendSamePlayer = true;

				p->Target[t].Index = 0;
			}
		}
	}

	if (tryToSendSamePlayer)
		Log(clientId, LOG_HACK, "Tentou enviar o pacote de ataque com o mesmo clientid");

	INT32 maxTargetSkill = 1; // local195
	if (skillNum >= 0 && skillNum < 110)
		maxTargetSkill = SkillData[skillNum].Maxtarget;
	else if (skillNum >= 110 && skillNum <= 127)
	{
		Log(clientId, LOG_INGAME, "Enviou a skill %d na qual a invalida. ", skillNum);
		Log(SERVER_SIDE, LOG_INGAME, "Enviou a skill %d na qual a invalida. ", skillNum);

		AddCrackError(clientId, 15, CRACK_USER_PKTHACK);
		return false;
	}

	if (skillNum == 79)
	{
		for (INT32 i = 1; i < 6; i++)
		{
			p->Target[i].Index = p->Target[0].Index;
			p->Target[i].Damage = p->Target[0].Damage;
		}
	}

	for (iterator = 0; iterator < maxTarget; iterator++)
	{
		// 425044 ata 42506D nao feito
		INT32 targetIdx = p->Target[iterator].Index; // local179
		if (targetIdx <= 0 || targetIdx > MAX_MOB)
			continue;

		CMob *mob = &pMob[targetIdx];
		if (mob->Mode == 0)
		{
			SendRemoveMob(clientId, targetIdx, 0, 0);
			continue;
		}

		if (mob->Mobs.Player.CurrentScore.Hp <= 0)
		{
			if (skillNum != 99 && skillNum != 31 && skillNum != 29 && skillNum != 27)
			{
				p->Target[iterator].Index = 0;
				p->Target[iterator].Damage = 0;

				continue;
			}

			// Se for as skills de cura da FM e tal, sa continua pois eles estarao mortos
			if (skillNum == 29 || skillNum == 27)
			{
				p->Target[iterator].Index = 0;
				p->Target[iterator].Damage = 0;

				continue;
			}
		}

		// TORRES DA PISTA
		if (mob->GenerateID == 176 || mob->GenerateID == 177 || mob->GenerateID == 178)
		{
			int partyId = mob->GenerateID - 176;

			const auto& pista = pPista[1];
			if (!pista.CurrentScore)
			{
				Log(SERVER_SIDE, LOG_HACK, "Atacando mob da Pista (Torre) sem a pista estar ativa");
				p->Target[iterator].Index = 0;
				p->Target[iterator].Damage = 0;

				continue;
			}

			bool canAttack = true;
			for (int i = 0; i < 13; i++)
			{
				if (pista.Clients[partyId][i] == clientId)
				{
					canAttack = false;

					break;
				}
			}

			if (!canAttack)
			{
				p->Target[iterator].Index = 0;
				p->Target[iterator].Damage = 0;

				continue;
			}
		}

		if (targetIdx >= MAX_PLAYER && pMob[targetIdx].GenerateID == TORRE_ERION)
		{
			INT32 guildId = pMob[clientId].Mobs.Player.Guild;
			if (guildId == 0)
			{
				p->Target[iterator].Index = 0;
				p->Target[iterator].Damage = 0;

				continue;
			}

			if (pMob[targetIdx].Mobs.Player.Guild == guildId)
			{
				p->Target[iterator].Index = 0;
				p->Target[iterator].Damage = 0;

				continue;
			}

			INT32 ally = g_pGuildAlly[guildId];
			if (ally != 0 && ally == pMob[targetIdx].Mobs.Player.Guild)
			{
				p->Target[iterator].Index = 0;
				p->Target[iterator].Damage = 0;

				continue;
			}
		}

		if (clientId > 0 && clientId < MAX_PLAYER && sServer.RvR.CurrentScore)
		{
			if (targetIdx >= MAX_PLAYER && pMob[targetIdx].GenerateID == TORRE_RVR_BLUE && pMob[clientId].Mobs.Player.CapeInfo == CAPE_BLUE)
			{
				p->Target[iterator].Index = 0;
				p->Target[iterator].Damage = 0;

				continue;
			}
			else if (targetIdx >= MAX_PLAYER && pMob[targetIdx].GenerateID == TORRE_RVR_RED && pMob[clientId].Mobs.Player.CapeInfo == CAPE_RED)
			{
				p->Target[iterator].Index = 0;
				p->Target[iterator].Damage = 0;

				continue;
			}

			if (targetIdx >= MAX_PLAYER && pMob[targetIdx].Mobs.Player.CapeInfo == 4)
			{
				INT32 summoner = pMob[targetIdx].Summoner;
				if (summoner != 0 && pMob[summoner].Mobs.Player.CapeInfo == pMob[clientId].Mobs.Player.CapeInfo)
				{
					p->Target[iterator].Index = 0;
					p->Target[iterator].Damage = 0;

					continue;
				}
			}
		}

		INT32 dam = p->Target[iterator].Damage; // local180
		INT32 damTmp = p->Target[iterator].Damage;
		if (dam != -2 && dam != -1 && dam != 0)
		{
			p->Target[iterator].Damage = 0;

			Log(clientId, LOG_ERROR, "AddCrackError, dam!= -2 -1 0 : %d", p->Target[iterator].Damage);
			AddCrackError(clientId, 10, CRACK_USER_PKTHACK);
			continue;
		}
 

		INT32 leader = pMob[clientId].Leader; // local181
		if (leader == 0)
			leader = clientId;

		INT32 mobLeader = mob->Leader; // local182
		if (mobLeader == 0)
			mobLeader = targetIdx;

		INT32 guild = player->Guild; // local183

		if (pMob[clientId].GuildDisable != 0)
			guild = 0;

		if ((mob->Target.X / 128) == 1 && (mob->Target.Y / 128) == 4)
			guild = 0;	

		INT32 mobGuild = mob->Mobs.Player.Guild; // local184
		if (mob->GuildDisable != 0)
			mobGuild = 0;

		if (guild == 0 && mobGuild == 0)
			guild = -1;

		// CHECAGEM DE GUILDDISABLE
		// 8BF1828 -> Nao entendi essa parte, ja que esse address a desconhecido
		// 8BF185C

		INT32 playerCape = player->CapeInfo; // local187
		INT32 mobCape = mob->Mobs.Player.CapeInfo; //local188

		INT32 isFrag = 0; // local189
		if (playerCape == 7 && mobCape == 7)
			isFrag = 1;
		else if (playerCape == 8 && mobCape == 8)
			isFrag = 1;

		bool isMagicDamage = dam == -1;
		INT32 cancel = 0;
		if (dam == -2)
		{ // 0042540F
			INT32 distance = GetDistance(pMob[clientId].Target.X, pMob[clientId].Target.Y, mob->Target.X, mob->Target.Y); // local190
			INT32 distance2 = GetDistance(pMob[clientId].Target.X, pMob[clientId].Target.Y, mob->Last.X, mob->Last.Y);
			if ((distance > (pUser[clientId].Range + 3) && distance2 > pUser[clientId].Range) || distance > 23) 
			{
				Log(clientId, LOG_HACK, "Enviando ataque com o distancia maior que o range do ataque basico. Distancia: %d. Distancia 2: %d. Range: %d.", distance, distance2, pUser[clientId].Range);

				continue;
			}

			dam = 0;
			INT32 criticalDouble = 0; // local191
			if (iterator == 0)
				criticalDouble = GetDoubleCritical(&pMob[clientId], (short*)&pUser[clientId].AttackCount, &p->attackCount, &doubleCritical);

			dam = player->CurrentScore.Damage;
			if ((doubleCritical & 2))
			{
				if (targetIdx < MAX_PLAYER)
					dam = (((Rand() % 2) + 13) * dam) / 10;
				else
					dam = (((Rand() % 2) + 15) * dam) / 10;
			}

			INT32 defense = mob->Mobs.Player.CurrentScore.Ac; // local192
			if (targetIdx < MAX_PLAYER)
				defense = defense * 3;

			dam = GetDamage(dam, defense, master);
			if (iterator == 0 && p->Header.Size >= sizeof p39D && player->Class == 3 && (player->LearnedSkill[0] & 0x200000) && (Rand() % 4) == 0)
			{
				INT32 skillDam = (player->CurrentScore.Dex + player->CurrentScore.Special[3]) >> 2; // local193
				skillDam /= 2;
				// Ao possuir a oitava skill Invisibilidade, 
				if ((player->LearnedSkill[0] & 0x800000))
					skillDam += (skillDam * 5 / 100);

				UINT32 mobLearn = 0; // local194
				if (targetIdx >= MAX_PLAYER)
				{
					if (mob->Mobs.Player.CurrentScore.Level >= 300)
					{
						mobLearn = mob->Mobs.Player.LearnedSkill[0];
						skillDam = (skillDam * (100 - mobLearn) / 100); // -304
					}
				}

				p->Target[1].Index = 0;
				p->Target[1].Damage = skillDam;

				doubleCritical |= 4;
				dam = dam + skillDam;
			}

			if ((doubleCritical & 1))
				dam *= 2;

			p->doubleCritical = doubleCritical;
		}
		else if (dam == -1 && skillNum >= 0 && skillNum <= 256)
		{ // 00425782
			dam = 0;

			if (pUser[clientId].TimeStamp.TimeStamp != 0xE0A1ACA && iterator >= maxTargetSkill)
			{
				Log(clientId, LOG_HACK, "Enviando skill com o target maior que o da skill.");
				continue;
			}

			if (SkillData[skillNum].PartyCheck != 0 && leader != mobLeader && guild != mobGuild)
			{
				Log(clientId, LOG_HACK, "Enviando skill de grupo sem estar em grupo ou guild.");

				continue;
			}
			{
				int distance = GetDistance(pMob[clientId].Target.X, pMob[clientId].Target.Y, mob->Target.X, mob->Target.Y);
				int range = SkillData[skillNum].Range + 3;
				if (pMob[clientId].Mobs.Player.LearnedSkill[0] & 0x20000000)
					range++;

				if (distance > range && p->skillId != 42)
				{
					auto distance2 = GetDistance(pMob[clientId].Target.X, pMob[clientId].Target.Y, mob->Last.X, mob->Last.Y);
					if (distance2 > range)
					{
						Log(clientId, LOG_HACK, "Enviando skill com o distancia maior que o range da skill. Distancia: %d. Distancia 2: %d. Range: %d. SkillId: %d", distance, distance2, range, skillNum);

						if (p->skillId == 31)
							return true;

						continue;
					}
				}
			}
			INT32 _needUpdate = 0; // local196
			INT32 instanceType = SkillData[skillNum].InstanceType; // local197

			if (instanceType >= 1 && instanceType <= 5)
			{//00425903  |. 8B0D 1C18BF08  |MOV ECX,DWORD PTR DS:[8BF181C]
				INT32 weather = sServer.Weather; // local198
				INT32 applyWeather; // local2080

				if ((pMob[clientId].Target.X >> 7) < 12 && (pMob[clientId].Target.Y >> 7) < 25)
					applyWeather = 1;
				else
					applyWeather = 0;

				if (applyWeather != 0)
					weather = 0;

				if (pUser[clientId].TimeStamp.TimeStamp == 0xE0A1ACA && p->Motion == 254 && (p->skillId == 32 || p->skillId == 34 || p->skillId == 36))
				{
					INT32 _level = player->CurrentScore.Level; // local199

					INT32 familiarSanc = GetItemSanc(&player->Equip[13]); // local200
					if (p->skillId == 32)
						dam = familiarSanc * 200 + (_level * 8);
					else if (p->skillId == 34)
						dam = familiarSanc * 250 + (_level * 8);
					else if (p->skillId == 36)
						dam = familiarSanc * 300 + (_level * 8);
				}
				else
				{
					if(targetIdx < MAX_PLAYER)
						dam = GetSkillDamage_PvP(skillNum, &pMob[clientId], weather, pMob[clientId].WeaponDamage);
					else
						dam = GetSkillDamage_PvM(skillNum, &pMob[clientId], weather, pMob[clientId].WeaponDamage);
				}

				if (pUser[clientId].AttackCount++ >= 1023)
					pUser[clientId].AttackCount = 0;

				INT32 def = mob->Mobs.Player.CurrentScore.Ac; // local201
				if (targetIdx < MAX_PLAYER)
					def = (def * 3);

				//00425AF3  |. 83FA 01        |CMP EDX,1
/*
				if (mob->Mobs.Player.Class == 1)
					def = (def * 3 / 2);
*/
				//00425B41  |. 8985 30FDFFFF  |MOV [LOCAL.180],EAX
				dam = GetSkillDamage_2(dam, def, master);

				INT32 skind = 0; // local203
				INT32 mobResist = 0; // local204
				INT32 LOCAL_205 = 0;
				INT32 LOCAL_206 = 0;

				if (instanceType == 1)
				{
					mobResist = 0;

					if (targetIdx < MAX_PLAYER)
						mobResist = mob->Mobs.Player.Resist.Sagrado; // resist[0] ?
					else
						mobResist = 10;

					if (pMob[clientId].IgnoreResistance > 0 && mobResist > 0)
						mobResist -= (mobResist * pMob[clientId].IgnoreResistance / 100);

					if (targetIdx < MAX_PLAYER)
						mobResist /= 4;

					dam = (MAX_RESIST - mobResist) * dam / 100;
				}
				else if (instanceType >= 2 && instanceType <= 5)
				{
					skind = instanceType - 2;
					mobResist = *(BYTE*)((INT32)&mob->Mobs.Player.Resist.Fogo + skind);

					if (pMob[clientId].IgnoreResistance > 0 && mobResist > 0)
						mobResist -= (mobResist * pMob[clientId].IgnoreResistance / 100);

					dam = (MAX_RESIST - mobResist) * dam / 100;
				}

				if (skillNum == 79)
				{
					if(targetIdx < MAX_PLAYER)
						dam = GetSkillDamage_PvP(skillNum, &pMob[clientId], weather, pMob[clientId].WeaponDamage);
					else
						dam = GetSkillDamage_PvM(skillNum, &pMob[clientId], weather, pMob[clientId].WeaponDamage);

					int defense = mob->Mobs.Player.CurrentScore.Ac;

					if (targetIdx < MAX_PLAYER)
						defense *= 2;

					dam = GetDamage((int)dam, defense, master);
				}
				if (instanceType == 2)
				{
					LOCAL_205 = 10 - iterator;

					if (LOCAL_205 < 0)
					{
						LOCAL_205 = 0;

						p->Target[iterator].Index = 0;
					}

					dam = dam * LOCAL_205 / 10;
				}
				else if (instanceType == 4)
				{
					LOCAL_206 = 10 - (iterator << 1);

					if (LOCAL_206 < 0)
					{
						LOCAL_206 = 0;

						p->Target[iterator].Index = 0;
					}

					dam = dam * LOCAL_206 / 10;
				}
			}
			else if (instanceType == 6)
			{
				if (mob->Mobs.Player.CapeInfo == 4)
					continue;

				if (skillNum == 27)
					dam = SkillData[skillNum].InstanceValue + LOCAL_160 * 2;
				else
					dam = ((LOCAL_160 * 3) >> 1) + SkillData[skillNum].InstanceValue;

				dam = -dam;

				if (dam < 0 && dam > -6)
					dam = -6;

				if (targetIdx < MAX_PLAYER && p->Header.ClientId <= MAX_PLAYER)
				{
					INT32 _guilty = GetGuilty(targetIdx);
					if (_guilty != 0)
					{
						INT32 _myGuilty = GetGuilty(clientId);

						SetGuilty(clientId, 8);

						if (_myGuilty == 0)
						{
							p364 LOCAL_329;
							GetCreateMob(clientId, (BYTE*)&LOCAL_329);

							GridMulticast_2(pMob[clientId].Target.X, pMob[clientId].Target.Y, (BYTE*)&LOCAL_329, 0);
						}
					}
				}

				if (p->Header.ClientId < MAX_PLAYER)
				{
					if (pMob[clientId].Mobs.Player.Equip[0].EFV2 >= CELESTIAL)
					{
						dam += -150;

						if (pMob[clientId].Mobs.Player.LearnedSkill[0] & (1 << (31 % 24)))
							dam += -350;
					}
					else if (pMob[clientId].Mobs.Player.Equip[0].EFV2 == ARCH)
					{
						dam += -150;

						if (pMob[clientId].Mobs.Player.LearnedSkill[0] & (1 << (31 % 24)))
							dam += -100;
					}
				}

				/*for (INT32 i = 0; i < 32; i++)
				{
					if (mob->Mobs.Affects[i].Index == 55 && mob->Mobs.Affects[i].Time > 0)
					{
						dam *= 2;

						break;
					}
				}*/

				INT32 mobHp = mob->Mobs.Player.CurrentScore.Hp; // local208
				INT32 itemId = mob->Mobs.Player.Equip[13].sIndex;
				if (itemId == 786 || itemId == 1936 || itemId == 1937)
				{
					INT32 _sanc = GetItemSanc(&mob->Mobs.Player.Equip[13]); // local209
					if (_sanc < 2)
						_sanc = 2;

					INT32 multHP = 1;
					switch (itemId)
					{
					case 1936:
						multHP = 10;
						break;

					case 1937:
						multHP = 1000;
						break;
					}

					multHP *= _sanc;
					mob->Mobs.Player.CurrentScore.Hp = mob->Mobs.Player.CurrentScore.Hp - (dam / multHP);
				}
				else
					mob->Mobs.Player.CurrentScore.Hp -= dam;

				if (mob->Mobs.Player.CurrentScore.Hp > mob->Mobs.Player.CurrentScore.MaxHp)
					mob->Mobs.Player.CurrentScore.Hp = mob->Mobs.Player.CurrentScore.MaxHp;

				if (targetIdx > 0 && targetIdx < MAX_PLAYER)
					SetReqHp(targetIdx);

				INT32 mobCurHp = mob->Mobs.Player.CurrentScore.Hp; // local210
				INT32 calcExp = (mobCurHp - mobHp) >> 3; // local211

				if (calcExp > 120)
					calcExp = 120;

				if (calcExp > 0 && clientId != targetIdx && clientId > 0 && clientId < MAX_PLAYER)
				{
					INT32 village = GetVillage(mob->Target.X, mob->Target.Y); // local212
					if (village >= 0 || village < 5)
						LOCAL_176 = calcExp;
				}
				//0042602E  |> E9 C9090000    |JMP TMSRV.004269FC

			}
			else if (instanceType == 7)//00426033  |> 83BD ECFCFFFF >|CMP [LOCAL.197],7
			{
				mob->Mode = 4;
				mob->CurrentTarget = 0;

				for (INT32 i = 0; i < 4; i++) // local213
					mob->EnemyList[i] = 0;
			}
			else if (instanceType == 8) // 004260AE  |> 83BD ECFCFFFF >|CMP [LOCAL.197],8
			{
				INT32 needUpdate = 0; // local214
				INT32 k; // local215

				for (k = 0; k < 32; k++) // 4? kkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkk
				{
					INT32 affectId = mob->Mobs.Affects[k].Index; // local216
					if (affectId == 1 || affectId == 3 || affectId == 5 || affectId == 7 || affectId == 10 || affectId == 12 || affectId == 20 || ((pMob[clientId].Mobs.Player.LearnedSkill[0] & 0x80) && affectId == 32) || affectId == 56)
					{
						memset(&mob->Mobs.Affects[k], 0, sizeof STRUCT_AFFECT);

						needUpdate = 1;
					}
				}

				if (needUpdate != 0)
				{
					mob->GetCurrentScore(targetIdx);
					SendScore(targetIdx);
				}
			}
			else if (instanceType == 9)//004261B2  |> 83BD ECFCFFFF >|CMP [LOCAL.197],9
			{
				if (mob->Mobs.Player.CurrentScore.Hp <= 0)
				{ // 004261D6 
					SendClientMessage(clientId, g_pLanguageString[_NN_Cant_Summon_Dead_Person]);

					return true;
				}

				eMapAttribute getAtt = GetAttribute(pMob[clientId].Target.X, pMob[clientId].Target.Y); // local217
				if (getAtt.CantSummon && pMob[clientId].Mobs.Player.CurrentScore.Level < 1000)
				{
					SendClientMessage(clientId, g_pLanguageString[_NN_Summon_Not_Allowed_Here]);

					continue;
				}

				if (pMob[targetIdx].Mobs.Player.BaseScore.Level > (pMob[clientId].Mobs.Player.BaseScore.Level + userMastery + 30))
				{
					SendClientMessage(clientId, g_pLanguageString[_NN_Too_High_Level_To_Summon]);

					continue;
				}

				if (!(pMob[targetIdx].Target.X & 0x0FF00) && !(pMob[targetIdx].Target.Y & 0x0FF00))
				{
					SendClientMessage(clientId, g_pLanguageString[_NN_Cant_Use_That_Here]);

					return true;
				}

				if (targetIdx < MAX_PLAYER && pUser[clientId].Status == USER_PLAY)
				{
					p3B2 packet{};

					packet.Header.PacketId = 0x3B2;
					packet.Header.ClientId = 0x7530;

					int len = strlen(pMob[clientId].Mobs.Player.MobName);

					for (int i = 0; i < len; i++)
						packet.Nickname[i] = pMob[clientId].Mobs.Player.MobName[i];

					pUser[p->Target[iterator].Index].AddMessage(reinterpret_cast<BYTE*>(&packet), sizeof p3B2);

					SummonedUser = targetIdx;

					SendClientMessage(targetIdx, g_pLanguageString[_SN_Summoned_By_S], pMob[clientId].Mobs.Player.MobName);
				}
			}
			else if (instanceType == 10 && instanceType < MAX_PLAYER)//00426376  |> 83BD ECFCFFFF >|CMP [LOCAL.197],0A
			{
				for (INT32 j = MAX_PLAYER; j < 30000; j++) // local218
				{
					if (pMob[j].Mode != 5)
						continue;

					if (pMob[j].CurrentTarget != targetIdx)
						continue;

					for (INT32 LOCAL_219 = 0; LOCAL_219 < 4; LOCAL_219++)
					{
						if (pMob[j].EnemyList[LOCAL_219] != targetIdx)
							continue;

						pMob[j].EnemyList[LOCAL_219] = clientId;
					}

					pMob[j].CurrentTarget = clientId;
				}
			}
			else if (instanceType == 11)
			{
				int leaderId = pMob[clientId].Leader;
				if (leaderId == 0)
					leaderId = clientId;

				int generateIndex = p->skillId - 56;

				int _numberGenerated = 1;
				switch (generateIndex)
				{
				case 0:
					_numberGenerated = pMob[clientId].Mobs.Player.CurrentScore.Special[2] / 30;
					break;
				case 1:
					_numberGenerated = pMob[clientId].Mobs.Player.CurrentScore.Special[2] / 30;
					break;
				case 2:
					_numberGenerated = pMob[clientId].Mobs.Player.CurrentScore.Special[2] / 40;
					break;
				case 3:
					_numberGenerated = pMob[clientId].Mobs.Player.CurrentScore.Special[2] / 40;
					break;
				case 4:
					_numberGenerated = pMob[clientId].Mobs.Player.CurrentScore.Special[2] / 40;
					break;
				case 5:
					_numberGenerated = pMob[clientId].Mobs.Player.CurrentScore.Special[2] / 80;
					break;
				case 6:
					_numberGenerated = pMob[clientId].Mobs.Player.CurrentScore.Special[2] / 80;
					break;
				case 7:
					_numberGenerated = 2;
					break;
				}

				if (pMob[clientId].Target.X >= 322 && pMob[clientId].Target.X <= 352 && pMob[clientId].Target.Y >= 416 && pMob[clientId].Target.Y <= 454)
				{
					SendClientMessage(clientId, "Nao a possavel evocar nesta area");

					return true;
				}

				if (pMob[clientId].Target.X >= 1030 && pMob[clientId].Target.X <= 1150 && pMob[clientId].Target.Y >= 1409 && pMob[clientId].Target.Y <= 1535)
				{
					SendClientMessage(clientId, "Nao a possavel evocar nesta area");

					return true;
				}

				if (_numberGenerated == 0)
					_numberGenerated = 1;

				bool canSummon = true;
				int summonFace = NPCBase[generateIndex].Equip[0].sIndex;
				for (int i = 0; i < 12; i++)
				{
					int party = pMob[leaderId].PartyList[i];
					if (party < MAX_PLAYER)
						continue;

					int tmpFace = pMob[party].Mobs.Player.Equip[0].sIndex;
					if (pMob[party].Summoner != clientId)
					{
						if (tmpFace < 315 || tmpFace >= 345)
							canSummon = false;

						break;
					}

					if ((tmpFace >= 315 && tmpFace <= 345) || tmpFace == summonFace)
						continue;

					canSummon = false;
					break;
				}

				if (!canSummon)
					continue;

				int total = 0;
				for (int i = 0; i < 12; i++)
				{
					int party = pMob[leaderId].PartyList[i];
					if (party == 0)
						continue;

					if (party < MAX_PLAYER)
						continue;

					int tmpFace = pMob[party].Mobs.Player.Equip[0].sIndex;
					if (tmpFace == summonFace && pMob[party].Summoner == clientId && (tmpFace < 315 || tmpFace >= 345))
						total++;
				}

				if ((player->LearnedSkill[0] & (1 << 15)))
					_numberGenerated += 1;

				if (total != -1)
				{
					_numberGenerated -= total;

					for (int i = 0; i < _numberGenerated; i++)
						GenerateSummon(clientId, generateIndex, 0);
				}
			}
			else if (instanceType == 12) // Chamas Etereas -> Nao vou descompilar
			{
				if (pMob[clientId].Mobs.Player.Class != 2)
					continue;

				int _rand = Rand() % 200;
				int _rand_2 = Rand() % 200;

				INT32 rate = 15;
				if (pMob[clientId].Mobs.Player.LearnedSkill[0] & (1 << 7))
					rate += 10;

				// _rand_2 tem q estar dentro e _rand + 10
				if (_rand >= _rand_2 && _rand_2 <= (_rand + rate))
				{
					STRUCT_AFFECT *affect = pMob[targetIdx].Mobs.Affects;

					int i;
					for (i = 0; i < 32; i++)
					{
						int index = affect[i].Index;
						if (index == 18 || index == 16 || index == 19 || index == 24)
							break;
					}

					if (i != 32)
					{
						memset(&affect[i], 0, sizeof STRUCT_AFFECT);

						SendScore(targetIdx);
					}
				}
			}
			if (skillNum == 6) // Faria divina
			{
				if ((targetIdx < MAX_PLAYER || !mob->Mobs.Player.Info.Merchant) && mob->Mobs.Player.CapeInfo != 6)// 00426A27
				{
					if (pMob[targetIdx].Mobs.Player.Equip[0].sIndex != 219 && pMob[targetIdx].Mobs.Player.Equip[0].sIndex != 220 && pMob[targetIdx].Mobs.Player.Equip[0].sIndex != 357 &&
						pMob[targetIdx].Mobs.Player.Equip[0].sIndex != 362 && pMob[targetIdx].Mobs.Player.Equip[0].sIndex != 397 && pMob[targetIdx].GenerateID != 374)
					{
						mob->Route[0] = 0;

						UINT32 posX = pMob[clientId].Target.X; // local239
						if (posX < mob->Target.X)
							posX++;

						if (posX > mob->Target.X)
							posX--;

						UINT32 posY = pMob[clientId].Target.Y; // local240
						if (posY < mob->Target.Y)
							posY++;

						if (posY > mob->Target.Y)
							posY--;

						if (GetEmptyMobGrid(targetIdx, &posX, &posY))
						{
							INT32 mastery2 = player->CurrentScore.Special[1]; // local241
							INT32 calcMastery = mastery2 / 10 + 40; // local242

							if (targetIdx >= MAX_PLAYER)
								calcMastery = mastery2 / 5 + 60;

							INT32 levelPlayer = player->CurrentScore.Level;
							INT32 levelMob = mob->Mobs.Player.CurrentScore.Level;

							if (pMob[clientId].Mobs.Player.Equip[0].EFV2 >= CELESTIAL)
								levelPlayer += 300;
							
							if (targetIdx < MAX_PLAYER)
							{
								if (pMob[targetIdx].Mobs.Player.Equip[0].EFV2 >= CELESTIAL)
									levelMob += 300;

								if (pMob[targetIdx].Mobs.Info.Reset_1)
									levelMob += 50;
								if (pMob[targetIdx].Mobs.Info.Reset_2)
									levelMob += 50;
								if (pMob[targetIdx].Mobs.Info.Reset_3)
									levelMob += 50;

								if (pMob[clientId].Mobs.Info.Reset_1)
									levelPlayer += 50;
								if (pMob[clientId].Mobs.Info.Reset_2)
									levelPlayer += 50;
								if (pMob[clientId].Mobs.Info.Reset_3)
									levelPlayer += 50;
							}

							INT32 levelDif = levelPlayer - levelMob; // local243
							levelDif >>= 1;

							if (Rand() % 100 < (calcMastery + levelDif))
							{
								p36C LOCAL_256;

								GetAction(targetIdx, posX, posY, &LOCAL_256);

								LOCAL_256.stEffect = 2;
								LOCAL_256.Speed = 6;

								GridMulticast(targetIdx, posX, posY, (BYTE*)&LOCAL_256);

								if (targetIdx < MAX_PLAYER)
								{
									pUser[targetIdx].AddMessage((BYTE*)&LOCAL_256, sizeof p36C);

									auto guilty = GetGuilty(clientId);

									INT32 arena = GetArena(pMob[clientId].Target.X, pMob[clientId].Target.Y); // local292
									INT32 village = GetVillage(pMob[clientId].Target.X, pMob[clientId].Target.Y); // local293

									bool isWar = false;
									// Retira o PK de todas as guerras
									for (INT32 i = 0; i < 5; i++)
									{
										if (pMob[clientId].Target.X >= g_pCityZone[i].war_min_x && pMob[clientId].Target.X <= g_pCityZone[i].war_max_x && pMob[clientId].Target.Y >= g_pCityZone[i].war_min_y && g_pCityZone[i].war_max_y)
										{
											isWar = isWar;
											break;
										}
									}

									if (!isWar && village == 5 && arena == 5)
									{
										SetGuilty(clientId, 8);
										if (guilty == 0)
										{
											p364 LOCAL_329{};
											GetCreateMob(clientId, (BYTE*)&LOCAL_329);

											GridMulticast_2(pMob[clientId].Target.X, pMob[clientId].Target.Y, (BYTE*)&LOCAL_329, 0);
										}
									}
								}
							}
						}
					}
				}
			}
			else if (skillNum == 22) // Exterminar
			{
				INT32 userCurHp = player->CurrentScore.Mp; // local257

				player->CurrentScore.Mp = 0;
				pUser[clientId].Potion.CountMp = 0;
				p->currentMp = 0;

				INT32 mastery4 = player->CurrentScore.Special[3]; // local258
				INT32 userInt = player->CurrentScore.Int; // local259

				dam = dam + userCurHp + (userInt >> 1);

				UINT32 mobPosX = mob->Target.X; // local260
				UINT32 mobPosY = mob->Target.Y; // local261

				UINT32 _mobPosX = mobPosX; // local262
				UINT32 _mobPosY = mobPosY; // local263

				if (GetEmptyMobGrid(clientId, &_mobPosX, &_mobPosY))
				{
					if (mob->Mobs.Player.Equip[0].sIndex != 219 && mob->Mobs.Player.Equip[0].sIndex != 220 && pMob[targetIdx].GenerateID == 374)
					{
						p36C sm; // local276

						GetAction(targetIdx, _mobPosX, _mobPosY, &sm);
						sm.Speed = 2;
						sm.stEffect = 2;

						GridMulticast(targetIdx, _mobPosX, _mobPosY, (BYTE*)&sm);

						if (targetIdx < MAX_PLAYER)
							pUser[targetIdx].AddMessage((BYTE*)&sm, 52);

						GetAction(clientId, mobPosX, mobPosY, &sm);

						sm.Speed = 2;
						sm.stEffect = 2;

						GridMulticast(targetIdx, _mobPosX, _mobPosY, (BYTE*)&sm);

						if (targetIdx < MAX_PLAYER)
							pUser[targetIdx].AddMessage((BYTE*)&sm, sizeof p36C);
					}
				}
			}

			else if (skillNum == 102)
			{ // soul
				INT32 slotId = GetEmptyAffect(clientId, 29);
				if (slotId == -1)
					continue;

				INT32 ev = pMob[clientId].Mobs.Player.Equip[0].EFV2;
				if (ev == ARCH)
					continue;

				INT32 time = 50;
				if (ev >= CELESTIAL)
					time = (player->BaseScore.Level + 25) * 3 / 8;

				bool need = true;
				if (pMob[clientId].Mobs.Affects[slotId].Index == 29)
					need = false;

				pMob[clientId].Mobs.Affects[slotId].Index = 29;
				pMob[clientId].Mobs.Affects[slotId].Master = 29;
				pMob[clientId].Mobs.Affects[slotId].Value = 10;
				pMob[clientId].Mobs.Affects[slotId].Time = time;

				SendAffect(clientId);
			}
			else if (skillNum == 79) // Tempestade de Raios
			{
				if (targetIdx > MAX_PLAYER)
					dam /= 2;
			}
			else if (skillNum == 30) // Julgamento Divino
			{
				dam = dam + (hp * 125 / 100);

				player->CurrentScore.Hp = (player->CurrentScore.Hp << 1) / 3;
				pUser[clientId].Potion.CountHp = player->CurrentScore.Hp;
			}
			else if (skillNum == 41) // Teleporte
			{
				INT32 skillTarg = (userMastery / 25) + 2; // local277
				if (skillTarg >= 13)
					skillTarg = 13;

				if (skillTarg <= 1)
					skillTarg = 2;

				for (INT32 l = 0; l < skillTarg; l++) // local278
				{
					//TODO : CHECAGENS NaO FEITAS, POIS NaO SEI O QUE a LOCAL_141 AINDA - 00426FAB
					INT32 targIndex = p->Target[l].Index; // local279

					if (targIndex <= 0 || targIndex >= MAX_PLAYER)
						continue;

					if (pUser[targIndex].Status != USER_PLAY)
						continue;

					if (!SetAffect(targIndex, skillNum, delay, userMastery))
						continue;

					SendScore(targIndex);
				}
			}
			else if (skillNum == 98) // Muro de Espinhos
			{
				eMapAttribute aux = GetAttribute(p->targetPos.X, p->targetPos.Y);
				if (aux.Village)
					continue;

				INT32 mobId = CreateMob("Vinha", p->targetPos.X, p->targetPos.Y, "npc");
				if (mobId <= 0 || mobId >= MAX_MOB)
					continue;

				pMob[mobId].Mobs.Affects[0].Index = 24;
				pMob[mobId].Mobs.Affects[0].Time = 10;

				pMob[mobId].RouteType = 5;
			}
			else if (skillNum == 99) // Ressu
			{
				GridMulticast_2(pMob[clientId].Target.X, pMob[clientId].Target.Y, (BYTE*)p, 0);

				int rate = 50;
				if ((Rand() % 100) <= rate)
				{
					pMob[clientId].Mobs.Player.CurrentScore.Hp = pMob[clientId].Mobs.Player.CurrentScore.MaxHp / 10;
					pUser[clientId].Potion.CountHp = pMob[clientId].Mobs.Player.CurrentScore.Hp;

					Log(clientId, LOG_INGAME, "O usuario reviveu usando a habilidade Ressurreiaao");

					// Envia ele vivo pra todos 
					p364 packet;
					GetCreateMob(clientId, (BYTE*)&packet);

					GridMulticast_2(pMob[clientId].Target.X, pMob[clientId].Target.Y, (BYTE*)&packet, 0);
				}
				else
				{
					pMob[clientId].Mobs.Player.CurrentScore.Hp = 2;
					pUser[clientId].Potion.CountHp = 2;

					DoRecall(clientId);
					Log(clientId, LOG_INGAME, "Falhou ao renascer usando Ressurreiaao. Voltando para a cidade.");
				}

				SendSetHpMp(clientId);

				SendScore(clientId);
				return true;
			}
			else if (skillNum == 31)
			{
				if (pMob[targetIdx].Mobs.Player.CurrentScore.Hp)
				{
					SendClientMessage(clientId, "Este personagem esta vivo!");

					p->Target[iterator].Damage = 1;
					p364 packet;
					GetCreateMob(targetIdx, (BYTE*)&packet);

					GridMulticast_2(pMob[targetIdx].Target.X, pMob[targetIdx].Target.Y, (BYTE*)&packet, 0);

					p->currentExp = player->Exp;
					p->currentMp = player->CurrentScore.Mp;
					p->Hold = static_cast<int>(pMob[clientId].Mobs.Hold);

					GridMulticast_2(pMob[clientId].Target.X, pMob[clientId].Target.Y, (BYTE*)p, 0);
					return true;
				}

				INT32 leaderId = pMob[clientId].Leader;
				if (leaderId <= 0)
					leaderId = clientId;

				bool isParty = false;
				if (leaderId == targetIdx)
					isParty = true;
				else
				{
					for (INT32 i = 0; i < 12; i++)
					{
						INT32 memberId = pMob[leaderId].PartyList[i];
						if (memberId <= 0 || memberId >= MAX_PLAYER)
							continue;

						if (memberId == targetIdx)
						{
							isParty = true;

							break;
						}
					}
				}

				INT32 guildId = pMob[clientId].Mobs.Player.Guild;
				if (guildId == 0)
					guildId = -1;

				if (guildId == pMob[targetIdx].Mobs.Player.Guild)
					isParty = true;

				if (!isParty)
				{
					SendClientMessage(clientId, g_pLanguageString[_NN_Party_Only]);

					SendScore(targetIdx);
					p->Target[iterator].Damage = 1;

					p364 packet;
					GetCreateMob(targetIdx, (BYTE*)&packet);

					GridMulticast_2(pMob[targetIdx].Target.X, pMob[targetIdx].Target.Y, (BYTE*)&packet, 0);

					return true;
				}

				p->currentExp = player->Exp;
				p->currentMp = player->CurrentScore.Mp;
				p->Hold = static_cast<int>(pMob[clientId].Mobs.Hold);

				int rate = 50;
				int runeSanc = 0;
				if ((Rand() % 100) <= rate)
				{
					pMob[targetIdx].Mobs.Player.CurrentScore.Hp = (pMob[targetIdx].Mobs.Player.CurrentScore.MaxHp * 10 / 100);

					if (targetIdx > 0 && targetIdx < MAX_PLAYER)
					{
						pUser[targetIdx].Potion.CountHp = pMob[targetIdx].Mobs.Player.CurrentScore.Hp;
						Log(targetIdx, LOG_INGAME, "Sucesso no renascimento pelo usuario %s", pMob[clientId].Mobs.Player.MobName);
						Log(clientId, LOG_INGAME, "Revivo usuario %s", pMob[targetIdx].Mobs.Player.MobName);
					}
				}
				else
				{
					if (targetIdx > 0 && targetIdx < MAX_PLAYER)
					{
						Log(targetIdx, LOG_INGAME, "Falhou no renascimento pelo usuario %s", pMob[clientId].Mobs.Player.MobName);
						Log(clientId, LOG_INGAME, "Falha ao reviver usuario %s", pMob[targetIdx].Mobs.Player.MobName);
					}
				}

				GridMulticast_2(pMob[clientId].Target.X, pMob[clientId].Target.Y, (BYTE*)p, 0);

				pMob[targetIdx].GetCurrentScore(targetIdx);
				SendScore(targetIdx);

				p364 packet{};
				GetCreateMob(targetIdx, (BYTE*)&packet);

				GridMulticast_2(pMob[targetIdx].Target.X, pMob[targetIdx].Target.Y, (BYTE*)&packet, 0);
				return true;
			}
			else if (skillNum == 47) // Cancelamento
			{
				int rand = Rand() % 100;
				if (rand < 25)
				{
					p->Target[iterator].Damage = -3;
					continue;
				}

				if (pMob[targetIdx].Mobs.Player.Class == 3)
				{
					for (int a = 0; a < 32; a++)
					{
						// Remove a imundidade, caso a tenha.
						if (pMob[targetIdx].Mobs.Affects[a].Index == 19)
						{
                            pMob[targetIdx].Mobs.Affects[a] = STRUCT_AFFECT{};
							pMob[targetIdx].GetCurrentScore(targetIdx);

							cancel = 1;
							break;
						}
					}
				}
			}
			else if (skillNum == 44) // Arma Magica
			{
				INT32 skillTarg = userMastery / 25 + 2; // local280

				if (skillTarg >= 13)
					skillTarg = 13;

				if (skillTarg <= 1)
					skillTarg = 2;

				for (INT32 l = 0; l < skillTarg; l++) // LOCAL281
				{
					//TODO : CHECAGENS NaO FEITAS, POIS NaO SEI O QUE a LOCAL_141 AINDA - 00004270CE

					INT32 targIndex = p->Target[l].Index; // local282

					if (targIndex <= 0 || targIndex >= MAX_PLAYER)
						continue;

					if (pUser[targIndex].Status != USER_PLAY)
						continue;

					INT32 aux = 0;
					if (!(aux = SetAffect(targIndex, skillNum, delay, userMastery)))
						continue;

					SendScore(targIndex);
				}
			}

			INT32 agressive = SkillData[skillNum].Aggressive; // local283
			INT32 sameLeaderGuild = 1; // local284

			if (agressive != 0)
			{
				if (leader == mobLeader || guild == mobGuild)
					sameLeaderGuild = 0;

				INT32 affectResist = SkillData[skillNum].AffectResist; // local285

				INT32 idxLevel = mob->Mobs.Player.CurrentScore.Level - player->CurrentScore.Level; // local286
				idxLevel = idxLevel >> 1;

				if (affectResist >= 1 && affectResist <= 4)
				{
					INT32 rnd = Rand() % 100; // local287
					if (rnd < *(BYTE*)((INT32)&mob->Mobs.Player.Resist.Fogo + affectResist))
						sameLeaderGuild = 0;
				}

				if (mob->Mobs.AffectInfo & 2)
					sameLeaderGuild = 0;

				if (clientId < MAX_PLAYER)
					if (mob->Mobs.Player.CapeInfo == 6)
						sameLeaderGuild = 0;
			}

			if (sameLeaderGuild != 0 && cancel == 0)
			{
				INT32 p;
				if ((p = SetAffect(targetIdx, skillNum, delay, userMastery)))
					_needUpdate = 1;

				INT32 t;
				if ((t = SetTick(targetIdx, skillNum, delay, userMastery)))
					_needUpdate = 1;

				if (_needUpdate != 0)
				{
					if (skillNum == 64 || skillNum == 66 || skillNum == 68 || skillNum == 70 || skillNum == 71)
						SendEquip(targetIdx);

					pMob[targetIdx].GetCurrentScore(targetIdx);

					SendScore(targetIdx);
				}
			}

			if (cancel == 1)
			{
				pMob[targetIdx].GetCurrentScore(targetIdx);

				SendScore(targetIdx);
			}
		}
		else
		{ // Tipo de ataque invalido, mensagem de erro
			AddCrackError(clientId, 1, CRACK_USER_PKTHACK);
				
			return false;
		}

		INT32 LCOAL_506 = targetIdx;

		p->Target[iterator].Damage = dam;

		if (p->Target[iterator].Damage >= 0 && targetIdx >= MAX_PLAYER)
		{
			bool chicken = true;
			if (skillNum >= 0 && skillNum <= 104)
			{
				INT32 instanceType = SkillData[skillNum].InstanceType; // local197
				if (instanceType == 0)
					chicken = false;
			}

			if (chicken)
			{
				for (int i = 0; i < 32; i++)
				{
					if (pMob[clientId].Mobs.Affects[i].Index == 30)
					{
						int totalDamage = dam;
						switch (pMob[clientId].Mobs.Affects[i].Value)
						{
						case 1: // Frango
							totalDamage += 2000;
							break;

						case 2: // Elixir 500 dano
							totalDamage += 500;
							break;

						case 3: // Elixir 2000 dano
							totalDamage += 2000;
							break;
						}

						dam = totalDamage;
						break;
					}
				}

				if ((doubleCritical & 1) && dam > 32000)
					dam = 32000;
				else
				{
					if (dam > MAX_NORMAL_DAMAGE)
						dam = MAX_NORMAL_DAMAGE;
				}

				p->Target[iterator].Damage = dam;
			}
		}
 

		if (p->Target[iterator].Damage <= 0)
			continue;

		// 004273F6
		if (targetIdx < MAX_PLAYER || mob->Mobs.Player.CapeInfo == 4)
		{
			if (p->doubleCritical & 4)
			{
				INT32 newDamage = dam - p->Target[1].Damage; // local288
				newDamage >>= 2;

				dam = p->Target[1].Damage + newDamage;
			}
			else // tm 7556 (467334
				dam >>= 2;
		}

#pragma region __TMSRV 7556 
		/*
		Parte referente a TMsrv 7556
		Endereao: 00467365
		*/
		/*
		INT32 ebp_7FC = 0;
		if(skillNum < 0 || skillNum >= 104)
			ebp_7FC = 1;
		else if(SkillData[skillNum].InstanceValue < 0 || SkillData[skillNum].Aggressive == 1)
			ebp_7FC = 1;
		*/
		//else if(SkillData[skillNum].Unk != 0) - Parte da estrutura nao existente na 6xx
		//	ebp_7CF = 1;
#pragma endregion
		// 0042748E
		if (pMob[clientId].ForceDamage != 0) //&& ebp_7FC == 1)
		{
			if (dam == 1)
				dam = pMob[clientId].ForceDamage;
			else if (dam > 0)
				dam += pMob[clientId].ForceDamage;

			p->Target[iterator].Damage = dam;
		}

		// Parte nao inserida no emulador
		if (mobLeader == leader)
			dam = 0;

		if (mobGuild == guild)
			dam = 0;

		if (targetIdx >= MAX_PLAYER && isFrag != 0)
			dam = 0;

#pragma region __TMSRV 7556 
		INT32 ebp_800h = dam;
		if (targetIdx < MAX_PLAYER && skillNum != 104)
		{
			INT32 ebp_804h = pMob[clientId].Mobs.Player.CurrentScore.Dex / 5;

			if (pMob[clientId].Mobs.Player.LearnedSkill[0] & 0x10000000)
				ebp_804h += 100;

			// 1FDF350h -> Endereao que retorna o local de onde esta a Jaia da Precisao
			if(pMob[clientId].Jewel >= 0 && pMob[clientId].Jewel < 32 && pMob[clientId].Mobs.Affects[pMob[clientId].Jewel].Value & 0x40)
				ebp_804h += 500;

			if (damTmp == -2)
				ebp_804h += 100;

			//004675E0
		}
#pragma endregion
		// 0042753F
		INT32 summoner = targetIdx; // local289
		if (targetIdx >= MAX_PLAYER && mob->Mobs.Player.CapeInfo == 4 && mob->Summoner > 0 && mob->Summoner < MAX_PLAYER && pUser[mob->Summoner].Status == USER_PLAY)
			summoner = mob->Summoner;

		//004275CF
		if (summoner < MAX_PLAYER)
		{
			INT32 pointPk = GetPKPoint(clientId); // local290
			INT32 summonerPointPk = GetPKPoint(summoner); // local291
			// 00427603
			INT32 arena = GetArena(pMob[clientId].Target.X, pMob[clientId].Target.Y); // local292
			INT32 village = GetVillage(pMob[clientId].Target.X, pMob[clientId].Target.Y); // local293

			INT32 mapX = pMob[clientId].Target.X >> 7; // local294
			INT32 mapY = pMob[clientId].Target.Y >> 7; // local295

			INT32 mapPK = 0; // local296
			INT32 isWar = 0; // local297

			INT32 connGuild = player->Guild; // local298
			INT32 summonerGuild = pMob[summoner].Mobs.Player.Guild; // local299

			INT32 maxGuild = 65536; // local300
				/*
			if(connGuild > 0 && connGuild < maxGuild && summonerGuild > 0 && summonerGuild < maxGuild && g_pGuildAlly[connGuild] == summonerGuild && g_pGuildAlly[summonerGuild] == connGuild)
			{ // Aqui ha uma provavel checagem da alianaa das guildas
				isWar = 1;
			}*/

			// Retira o PK de todas as guerras
			for (INT32 i = 0; i < 5; i++)
			{
				if (pMob[clientId].Target.X >= g_pCityZone[i].war_min_x && pMob[clientId].Target.X <= g_pCityZone[i].war_max_x && pMob[clientId].Target.Y >= g_pCityZone[i].war_min_y && g_pCityZone[i].war_max_y)
				{
					isWar = 1;
					break;
				}
			}

			if (mapX == 1 && mapY == 31)
				mapPK = 1;

			if (arena == 5 && village == 5 && isWar == 0)
			{
				INT32 summonerGuilty = GetGuilty(summoner); // local301
				INT32 connGuilty = GetGuilty(clientId); // local302

				if (dam > 0)
				{
					SetGuilty(clientId, 8);
					if (connGuilty == 0)
					{
						p364 LOCAL_329;
						GetCreateMob(clientId, (BYTE*)&LOCAL_329);

						GridMulticast_2(pMob[clientId].Target.X, pMob[clientId].Target.Y, (BYTE*)&LOCAL_329, 0);
					}
				}
			}

			INT32 parry = GetParryRate(clientId, targetIdx, damTmp); // local330

			// 00467648h TM.7556
			if ((skillNum == 79 || skillNum == 22) && iterator < 3)
				parry = parry * 30 / 100;

			INT32 _rnd = Rand() % 1000 + 1; // local331
			if (_rnd < parry)
			{
				if (maxTargetSkill != 0)
					dam = -3;

				if ((mob->Mobs.AffectInfo & 128) && _rnd < 100)
					dam = -4;
			}
		}

		if (pMob[clientId].Mobs.Player.AffectInfo.Value & 0x04 && (skillNum != 79 || (skillNum == 79 && iterator == 0)) && !(pMob[targetIdx].Mobs.AffectInfo & 2))
		{
			INT32 wType = GetItemAbility(&player->Equip[6], EF_WTYPE); // ebp-820h

			if (wType == 101)
			{
				INT32 randTmp = Rand() % 4; // ebp-824h

				if (randTmp == 0)
				{
					INT32 masteryTmp = player->CurrentScore.Special[1];

					if (func_40195B(targetIdx, 36, (masteryTmp / 4) + 30, masteryTmp))
						SendScore(targetIdx);
				}
			}
		}

		if (pMob[targetIdx].BossInfoId < sServer.Boss.size())
		{
			INT32 summonerGuilty = GetGuilty(summoner); // local301
			INT32 connGuilty = GetGuilty(clientId); // local302

			if (dam > 0)
			{
				SetGuilty(clientId, 8);
				if (connGuilty == 0)
				{
					p364 LOCAL_329;
					GetCreateMob(clientId, (BYTE*)&LOCAL_329);

					GridMulticast_2(pMob[clientId].Target.X, pMob[clientId].Target.Y, (BYTE*)&LOCAL_329, 0);
				}
			}
		}

		//00467AEA
		if (pMob[clientId].Mobs.Player.AffectInfo.Value & 0x10 && !(pMob[targetIdx].Mobs.AffectInfo & 2))
		{
			INT32 wType = GetItemAbility(&player->Equip[6], EF_WTYPE); // ebp-820h

			if (wType == 41)
			{
				INT32 randTmp = Rand() & 0x80000001;

				if (randTmp == 0)
				{
					INT32 masteryTmp = player->CurrentScore.Special[2];

					if (func_40195B(targetIdx, 40, masteryTmp + 100, masteryTmp))
						SendScore(targetIdx);
				}
			}
		}

		if (targetIdx < MAX_PLAYER || mob->Mobs.Player.CapeInfo == 4 || mob->Mobs.Player.Info.Merchant)
		{ // CHECAGENS DE TERRENO E DO USUaRIO
			// 0x42798D
			eMapAttribute targetMapAttribute = GetAttribute(pMob[targetIdx].Target.X, pMob[targetIdx].Target.Y); //local332
			eMapAttribute connMapAttribute = GetAttribute(pMob[clientId].Target.X, pMob[clientId].Target.Y); // local333

			if (targetMapAttribute.Village || !targetMapAttribute.PvP)
				dam = 0;

			if (connMapAttribute.Village || !connMapAttribute.PvP)
				dam = 0;
		}

		// Tentando se atacar tendo a mesma capa no rvr
		if (targetIdx < MAX_PLAYER && clientId < MAX_PLAYER)
		{
			if (pMob[targetIdx].Target.X >= 1041 && pMob[targetIdx].Target.X <= 1248 &&
				pMob[targetIdx].Target.Y >= 1950 && pMob[targetIdx].Target.Y <= 2158 && sServer.RvR.CurrentScore == 1)
			{
				if (pMob[targetIdx].Mobs.Player.CapeInfo == pMob[clientId].Mobs.Player.CapeInfo)
					dam = 0;
			}
		}

		if (mob->Mobs.Player.CapeInfo == 6 && userLevel < 399) // MAX LEVEL
			dam = 0;

 

		p->Target[iterator].Damage = dam;

		//00427A94
		if (dam <= 0)
			continue;

		INT32 mobHp1;
		if (mob->Mobs.Player.CurrentScore.Hp < dam)
			mobHp1 = mob->Mobs.Player.CurrentScore.Hp;
		else
			mobHp1 = dam;

		INT32 mobNewHp = mobHp1;	// local336	/
		/*INT32 _calcExp = mob->Mobs.Player.Exp * mobNewHp / mob->Mobs.Player.CurrentScore.MaxHp; // local337

		_calcExp = GetExpApply(_calcExp, clientId, targetIdx);

		if(mob->Mobs.Player.CapeInfo == 4)
			_calcExp = 0;

		if(targetIdx >= MAX_PLAYER)
			userNewExp = userNewExp + _calcExp;
			*/

		if (targetIdx > 0 && targetIdx < MAX_PLAYER && dam > 0)
		{
			if (mob->ReflectDamage > 0)
				dam = dam - mob->ReflectDamage;

			if (dam <= 0)
			{
				dam = 0;

				p->Target[iterator].Damage = dam;
				continue;
			}
			else
				p->Target[iterator].Damage = dam;
		}

		INT32 _damage = dam; // local338
		INT32 _calcDamage = 0; // local339
		INT32 mountId = mob->Mobs.Player.Equip[14].sIndex; // local340

		if (targetIdx < MAX_PLAYER)
		{
			if (mob->isPetAlive())
			{
				if (mob->isNormalPet())
				{
					_damage = AbsorveDamageByPet(mob, _damage);
					_calcDamage = dam - _damage;
				}

				p->Target[iterator].Damage = _damage;
			}
		}

		// BENaaO DIVINA - Reduaao de dano
		if (pMob[targetIdx].Target.X >= 1664 && pMob[targetIdx].Target.X <= 1791 && pMob[targetIdx].Target.Y >= 1537 && pMob[targetIdx].Target.Y <= 1920 && targetIdx < MAX_PLAYER)
		{
			_damage /= 2;
			p->Target[iterator].Damage = _damage;
		}

		INT32 auxDam = p->Target[iterator].Damage;
		if (targetIdx < MAX_PLAYER)
		{
			for (int i = 0; i < 32; i++)
			{
				if (pMob[targetIdx].Mobs.Affects[i].Index == 18)
				{
					// 
					INT32 tmpDamage = auxDam * 80 / 100;
					if ((pMob[targetIdx].Mobs.Player.CurrentScore.Mp - tmpDamage) >= 300)
					{
						if (pUser[targetIdx].Potion.CountMp - tmpDamage >= 0)
							pUser[targetIdx].Potion.CountMp -= tmpDamage;
						else
							pUser[targetIdx].Potion.CountMp = 0;

						if (pMob[targetIdx].Mobs.Player.CurrentScore.Mp - tmpDamage >= 0)
							pMob[targetIdx].Mobs.Player.CurrentScore.Mp -= tmpDamage;
						else
							pMob[targetIdx].Mobs.Player.CurrentScore.Mp = 0;

						SetReqMp(targetIdx);
						SendScore(targetIdx);

						auxDam -= tmpDamage;
					}

					break;
				}
			}
		}

		 

		p->Target[iterator].Damage = auxDam;
		_damage = auxDam;

		// 00427D7E
		INT32 tDamage = _damage; // local341
		INT32 itemId = mob->Mobs.Player.Equip[13].sIndex;

		if (itemId == 786 || itemId == 1936 || itemId == 1937)
		{ // ITENS DE HP SaO INSERIDOS AQUI
			float hpItemSanc = static_cast<float>(GetItemSanc(&mob->Mobs.Player.Equip[13])); // local342
			if (hpItemSanc < 2.0)
				hpItemSanc = 2.0;

			float multHP = 1;
			switch (itemId)
			{
			case 1936:
				multHP = 10.0;
				break;

			case 1937:
				multHP = 1000.0;
				break;
			}

			multHP *= hpItemSanc;
			tDamage = _damage / multHP;

			if (tDamage > mob->Mobs.Player.CurrentScore.Hp)
				mob->Mobs.Player.CurrentScore.Hp = 0;
			else
				mob->Mobs.Player.CurrentScore.Hp -= tDamage;

#if defined(_DEBUG)
			Log(clientId, LOG_INGAME, "Removido o Hp do mob %s. HP removido: %d. HP Atual: %d", mob->Mobs.Player.MobName, tDamage, mob->Mobs.Player.CurrentScore.Hp);
#endif
		}
		else
		{
			if (tDamage > mob->Mobs.Player.CurrentScore.Hp)
				tDamage = mob->Mobs.Player.CurrentScore.Hp;

			mob->Mobs.Player.CurrentScore.Hp -= tDamage;
		}

		// 00427EB1
		if (_calcDamage > 0)
			ProcessAdultMount(targetIdx, _calcDamage);

		if (targetIdx > 0 && targetIdx < MAX_PLAYER)
		{
			pUser[targetIdx].Potion.CountHp -= tDamage;
			SetReqHp(targetIdx);
		}

		if (pMob[clientId].LifeSteal > 0 || pMob[clientId].Vampirism > 0)
		{
			int healPerc = 0;
			if (isMagicDamage)
				healPerc = pMob[clientId].Vampirism;
			else
				healPerc = pMob[clientId].LifeSteal;

			if (skillNum == 79 && iterator != 0)
				healPerc = 0;

			if (healPerc != 0)
			{
				int heal = _damage * healPerc / 100;
				if (targetIdx > MAX_PLAYER)
					heal /= 4;

				if (targetIdx > MAX_PLAYER && heal > 300)
					heal = 300;

				INT32 totalHp = pMob[clientId].Mobs.Player.CurrentScore.Hp + heal;
				if (totalHp > pMob[clientId].Mobs.Player.CurrentScore.MaxHp)
					totalHp = pMob[clientId].Mobs.Player.CurrentScore.MaxHp;

				pMob[clientId].Mobs.Player.CurrentScore.Hp = totalHp;
				pUser[clientId].Potion.CountHp += heal;

				p18A packet{};
				packet.Header.PacketId = 0x18A;
				packet.Header.Size = sizeof p18A;
				packet.Header.ClientId = clientId;

				packet.CurHP = pMob[clientId].Mobs.Player.CurrentScore.Hp;
				packet.Incress = heal;

				GridMulticast_2(pMob[clientId].Target.X, pMob[clientId].Target.Y, (BYTE*)& packet, 0);

				SetReqHp(clientId);
				SetReqMp(clientId);
			}
		}

		if (pMob[targetIdx].GenerateID == 374 && p->Header.TimeStamp != 0x00E0A1ACA)
			DropEventOnHit(clientId, targetIdx);

		if (pMob[clientId].Mobs.Player.Class == 0 && !(pUser[clientId].AttackCount % 5))
		{
			for (INT32 i = 0; i < 32; i++)
			{
				if (pMob[clientId].Mobs.Affects[i].Index == 17)
				{
					INT32 heal = _damage * 20 / 100;
					if (targetIdx >= MAX_PLAYER)
						heal /= 4;

					if (targetIdx && heal > 300)
						heal = 300;

					INT32 totalHp = pMob[clientId].Mobs.Player.CurrentScore.Hp + heal;
					if (totalHp > pMob[clientId].Mobs.Player.CurrentScore.MaxHp)
						totalHp = pMob[clientId].Mobs.Player.CurrentScore.MaxHp;

					pMob[clientId].Mobs.Player.CurrentScore.Hp = totalHp;
					pUser[clientId].Potion.CountHp += heal;

					p18A packet{};
					packet.Header.PacketId = 0x18A;
					packet.Header.Size = sizeof p18A;
					packet.Header.ClientId = clientId;

					packet.CurHP = pMob[clientId].Mobs.Player.CurrentScore.Hp;
					packet.Incress = heal;

					INT32 LOCAL_162 = pMob[clientId].Target.X;
					INT32 LOCAL_163 = pMob[clientId].Target.Y;

					GridMulticast_2(LOCAL_162, LOCAL_163, (BYTE*)&packet, 0);

					SetReqHp(clientId);
					SetReqMp(clientId);
					break;
				}
			}
		}

		if (targetIdx < MAX_PLAYER && pMob[targetIdx].Mobs.Player.Class == 2 && (pMob[targetIdx].Mobs.Player.LearnedSkill[0] & (1 << 19)) && (pMob[targetIdx].Mobs.Player.LearnedSkill[0] & (1 << 23)))
		{ // beastmaster com aden + escudo do tormento
			INT32 reflect = _damage / 10;
			if (reflect > 350)
				reflect = 350;

			if (skillNum == 79 && iterator != 0)
				reflect = 0;

			INT32 totalHp = pMob[clientId].Mobs.Player.CurrentScore.Hp - reflect;
			if (totalHp <= 0)
				totalHp = 0;

			pMob[clientId].Mobs.Player.CurrentScore.Hp = totalHp;
			pUser[clientId].Potion.CountHp -= reflect;

			if (reflect > 0)
			{
				p18A packet{};
				packet.Header.PacketId = 0x18A;
				packet.Header.Size = sizeof p18A;
				packet.Header.ClientId = clientId;

				packet.CurHP = pMob[clientId].Mobs.Player.CurrentScore.Hp;
				packet.Incress = -reflect;

				INT32 LOCAL_162 = pMob[clientId].Target.X;
				INT32 LOCAL_163 = pMob[clientId].Target.Y;

				GridMulticast_2(LOCAL_162, LOCAL_163, (BYTE*)&packet, 0);

				SetReqHp(clientId);
				SetReqMp(clientId);

				if (totalHp <= 0)
					MobKilled(clientId, targetIdx, 0, 0);
			}
		}

		// 00427F7F
		if (targetIdx >= MAX_PLAYER && mob->Mobs.Player.CapeInfo == 4)
			LinkMountHp(targetIdx);

		// 00427FAF
		if (mob->Mobs.Player.CurrentScore.Hp <= 0)
		{
			mob->Mobs.Player.CurrentScore.Hp = 0;

			p->Target[iterator].Damage = dam;
			continue;
		}

		// 00427FF5
		if (mob->Mode != 0 && mob->Mobs.Player.CurrentScore.Hp > 0)
		{
			SetBattle(targetIdx, clientId);

			pMob[clientId].CurrentTarget = targetIdx;

			INT32 _userLeader = pMob[clientId].Leader; // local343
			if (_userLeader <= 0)
				_userLeader = clientId;

			for (INT32 l = 0; l < 12; l++) // local344
			{
				INT32 partyIndex = pMob[_userLeader].PartyList[l]; // local345
				if (partyIndex <= MAX_PLAYER)
					continue;

				// 004280CD
				if (pMob[partyIndex].Mode == 0 || pMob[partyIndex].Mobs.Player.CurrentScore.Hp <= 0)
				{
					if (pMob[partyIndex].Mode != 0)
						DeleteMob(partyIndex, 1);

					pMob[_userLeader].PartyList[l] = 0;
					continue;
				}

				SetBattle(partyIndex, targetIdx);
			}

			// 0042815B
			_userLeader = mob->Leader;
			if (_userLeader <= 0)
				_userLeader = targetIdx;

			for (INT32 l = 0; l < 12; l++)
			{
				INT32 partyIndex = pMob[_userLeader].PartyList[l]; // local346
				if (partyIndex <= MAX_PLAYER)
					continue;

				if (pMob[partyIndex].Mode == 0 || pMob[partyIndex].Mobs.Player.CurrentScore.Hp <= 0)
				{
					if (pMob[partyIndex].Mode != 0)
						DeleteMob(partyIndex, 1);

					pMob[_userLeader].PartyList[l] = 0;
					continue;
				}

				SetBattle(partyIndex, clientId);
			}
		}
	}

	if ((pMob[clientId].Mobs.Player.LearnedSkill[0] & 0x20000000))
		p->doubleCritical = p->doubleCritical | 8;

	// 0042826B
	if (LOCAL_176 > 0)
		userNewExp = LOCAL_176;

	if (skillNum == 30) // JUlgamento divino
		SendSetHpMp(clientId);

	if (userNewExp <= 0)
		userNewExp = 0;

	if (userNewExp > 200)
		userNewExp = 200;

	//004282D5
	p->currentExp = player->Exp;
	p->currentMp = player->CurrentScore.Mp;
	p->Hold = static_cast<unsigned int>(pMob[clientId].Mobs.Hold);

	GridMulticast_2(pMob[clientId].Target.X, pMob[clientId].Target.Y, (BYTE*)p, 0);

	for (INT32 i = 0; i < maxTarget; i++)
	{
		for (INT32 t = (i + 1); t < maxTarget; t++)
		{
			if (p->Target[i].Index == p->Target[t].Index)
				p->Target[t].Index = 0;
		}
	}

	for (INT32 LOCAL_347 = 0; LOCAL_347 < maxTarget; LOCAL_347++)
	{
		if (p->Target[LOCAL_347].Index <= 0 || p->Target[LOCAL_347].Index >= 30000 || pMob[p->Target[LOCAL_347].Index].Mode == 0)
			continue;

		if (!pMob[p->Target[LOCAL_347].Index].Mobs.Player.CurrentScore.Hp)
			MobKilled(p->Target[LOCAL_347].Index, clientId, 0, 0);
		else
			pMob[p->Target[LOCAL_347].Index].GetCurrentScore(p->Target[LOCAL_347].Index);
	}

	pMob[clientId].CheckGetLevel();
	return true;
}