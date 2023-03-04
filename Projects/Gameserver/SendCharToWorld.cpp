#include "cServer.h"
#include "Basedef.h"
#include "SendFunc.h"
#include "GetFunc.h"
#include "CEventManager.h"

bool CUser::SendCharToWorld(PacketHeader* Header)
{
	p114* LOCAL_215 = (p114*)Header;
	INT32 LOCAL_216 = 0;
	
	auto fixCostume = [](int clientId, STRUCT_ITEM* item) 
	{

		bool isUsingCostume = (item->sIndex >= 4151 && item->sIndex <= 4189) || (item->sIndex >= 4210 && item->sIndex <= 4229) || (item->sIndex >= 4230 && item->sIndex <= 4241);

		if (isUsingCostume && item->EF1 == 106 && item->EF2 == 110 && item->EF3 == 109)
		{
			Log(clientId, LOG_INGAME, "Removido os adicionais de tempo do traje %s %s", g_pItemList[item->sIndex].ItemName, item->toString().c_str());

			item->EF1 = 0;
			item->EF2 = 0;
			item->EF3 = 0;
		}
	};

	auto fixBoots = [](int clientId, STRUCT_ITEM* item) 
	{
		auto temporaryItem = *item;
		int totalDamage = GetItemAbilityNoSanc(item, EF_DAMAGE) + GetItemAbilityNoSanc(item, EF_DAMAGE2);

		int maxDamage = 30;
		if (GetItemAbility(item, EF_MOBTYPE) == 1)
			maxDamage = 42;

		if (totalDamage > maxDamage)
		{
			int i = 0;
			for (; i < 3; ++i)
			{
				if (item->stEffect[i].cEffect == EF_DAMAGE)
				{
					item->stEffect[i].cEffect = EF_DAMAGE2;
					item->stEffect[i].cValue = maxDamage;

					break;
				}
			}

			if (i != 3)
				Log(clientId, LOG_INGAME, "O item %s %s teve seu adicionaal corrigido. Adicional anterior: %d. Informaaaes do item: %s", g_pItemList[item->sIndex].ItemName, item->toString().c_str(), totalDamage, temporaryItem.toString().c_str());

			return i != 3;
		}

		return false;
	};

	for (; LOCAL_216 < 16; LOCAL_216++)
	{
		STRUCT_ITEM* LOCAL_217 = &LOCAL_215->Mob.Equip[LOCAL_216];
		if (LOCAL_217->sIndex <= 0 || LOCAL_217->sIndex > MAX_ITEMLIST)
			continue;

		INT32 LOCAL_218 = g_pItemList[LOCAL_217->sIndex].Pos;
		if (LOCAL_218 == 64 || LOCAL_218 == 192)
		{
			if (LOCAL_217->stEffect[0].cEffect == EF_DAMAGE2 || LOCAL_217->stEffect[0].cEffect == EF_DAMAGEADD)
				LOCAL_217->stEffect[0].cEffect = EF_DAMAGE;

			if (LOCAL_217->stEffect[1].cEffect == EF_DAMAGE2 || LOCAL_217->stEffect[1].cEffect == EF_DAMAGEADD)
				LOCAL_217->stEffect[1].cEffect = EF_DAMAGE;

			if (LOCAL_217->stEffect[2].cEffect == EF_DAMAGE2 || LOCAL_217->stEffect[2].cEffect == EF_DAMAGEADD)
				LOCAL_217->stEffect[2].cEffect = EF_DAMAGE;
		}

		if (LOCAL_218 <= 32)
		{ // Armaduras
			if (LOCAL_217->stEffect[0].cEffect == EF_CRITICAL)
			{
				LOCAL_217->stEffect[0].cEffect = EF_CRITICAL2;

				INT32 value = LOCAL_217->stEffect[0].cValue + GetEffectValueByIndex(LOCAL_217->sIndex, EF_CRITICAL);
				LOCAL_217->stEffect[0].cValue = value;
			}
			if (LOCAL_217->stEffect[1].cEffect == EF_CRITICAL)
			{
				LOCAL_217->stEffect[1].cEffect = EF_CRITICAL2;

				INT32 value = LOCAL_217->stEffect[1].cValue + GetEffectValueByIndex(LOCAL_217->sIndex, EF_CRITICAL);
				LOCAL_217->stEffect[1].cValue = value;
			}

			if (LOCAL_217->stEffect[2].cEffect == EF_CRITICAL)
			{
				LOCAL_217->stEffect[2].cEffect = EF_CRITICAL2;

				INT32 value = LOCAL_217->stEffect[2].cValue + GetEffectValueByIndex(LOCAL_217->sIndex, EF_CRITICAL);
				LOCAL_217->stEffect[2].cValue = value;
			}
		}

		bool isUsingCostume = (LOCAL_217->sIndex >= 4151 && LOCAL_217->sIndex <= 4189) || (LOCAL_217->sIndex >= 4210 && LOCAL_217->sIndex <= 4229) || (LOCAL_217->sIndex >= 4230 && LOCAL_217->sIndex <= 4241);
		if ((LOCAL_217->sIndex >= 3980 && LOCAL_217->sIndex <= 3999) || isUsingCostume)
		{
			if ((LOCAL_217->EF1 == 106 && LOCAL_217->EF2 == 110 && LOCAL_217->EF3 == 109) || (isUsingCostume && LOCAL_217->EFV1 != 0 && LOCAL_217->EFV2 != 0 && LOCAL_217->EFV3 != 0))
			{
				float difDays = TimeRemaining(LOCAL_217->EFV1, LOCAL_217->EFV2, (LOCAL_217->EFV3 + 2000));
				if (difDays <= 0.0f)
				{
					SendClientMessage(clientId, "Esfera / Traje expirou...");
					Log(clientId, LOG_INGAME, "Traje / Esfera expirado. Expira em: %d/%d/%d", LOCAL_217->EFV1, LOCAL_217->EFV2, LOCAL_217->EFV3);

					memset(LOCAL_217, 0, sizeof STRUCT_ITEM);
					continue;
				}
			}
		}

		fixCostume(clientId, LOCAL_217);

		if (LOCAL_218 == 32)
			fixBoots(clientId, LOCAL_217);
	}

	for (LOCAL_216 = 0; LOCAL_216 < 64; LOCAL_216++)
	{
		STRUCT_ITEM* LOCAL_219 = &LOCAL_215->Mob.Inventory[LOCAL_216];
		if (LOCAL_219->sIndex <= 0 || LOCAL_219->sIndex >= MAX_ITEMLIST)
			continue;

		INT32 LOCAL_220 = g_pItemList[LOCAL_219->sIndex].Pos;
		if (LOCAL_220 == 64 || LOCAL_220 == 192)
		{
			if (LOCAL_219->stEffect[0].cEffect == EF_DAMAGE2 || LOCAL_219->stEffect[0].cEffect == EF_DAMAGEADD)
				LOCAL_219->stEffect[0].cEffect = EF_DAMAGE;

			if (LOCAL_219->stEffect[1].cEffect == EF_DAMAGE2 || LOCAL_219->stEffect[1].cEffect == EF_DAMAGEADD)
				LOCAL_219->stEffect[1].cEffect = EF_DAMAGE;

			if (LOCAL_219->stEffect[2].cEffect == EF_DAMAGE2 || LOCAL_219->stEffect[2].cEffect == EF_DAMAGEADD)
				LOCAL_219->stEffect[2].cEffect = EF_DAMAGE;
		}

		if (LOCAL_220 <= 32)
		{ // Armaduras
			if (LOCAL_219->stEffect[0].cEffect == EF_CRITICAL)
			{
				LOCAL_219->stEffect[0].cEffect = EF_CRITICAL2;

				INT32 value = LOCAL_219->stEffect[0].cValue + GetEffectValueByIndex(LOCAL_219->sIndex, EF_CRITICAL);
				LOCAL_219->stEffect[0].cValue = value;
			}
			if (LOCAL_219->stEffect[1].cEffect == EF_CRITICAL)
			{
				LOCAL_219->stEffect[1].cEffect = EF_CRITICAL2;

				INT32 value = LOCAL_219->stEffect[1].cValue + GetEffectValueByIndex(LOCAL_219->sIndex, EF_CRITICAL);
				LOCAL_219->stEffect[1].cValue = value;
			}

			if (LOCAL_219->stEffect[2].cEffect == EF_CRITICAL)
			{
				LOCAL_219->stEffect[2].cEffect = EF_CRITICAL2;

				INT32 value = LOCAL_219->stEffect[2].cValue + GetEffectValueByIndex(LOCAL_219->sIndex, EF_CRITICAL);
				LOCAL_219->stEffect[2].cValue = value;
			}
		}

		if (LOCAL_220 == 32)
			fixBoots(clientId, LOCAL_219);

		fixCostume(clientId, LOCAL_219);
	}

	for (int i = 0; i < 120; ++i)
	{
		auto item = &User.Storage.Item[i];
		if (item->sIndex <= 0 || item->sIndex >= MAX_ITEMLIST || g_pItemList[item->sIndex].Pos != 32)
			continue;

		if (fixBoots(clientId, item))
			SendItem(clientId, SlotType::Storage, i, item);
	}

	// 0044D940
	if (true) // evOn
	{
		for (INT32 LOCAL_221 = 0; LOCAL_221 < 64; LOCAL_221++)
		{
			if (LOCAL_215->Mob.Inventory[LOCAL_221].sIndex == 470 || LOCAL_215->Mob.Inventory[LOCAL_221].sIndex == 500)
				LOCAL_215->Mob.Inventory[LOCAL_221].sIndex = 0;
		}
	}

	// Quando troca de servidor ja seta como true
	// para nao dar problema ao dar personagem
	// Sa a setado quando a realmente enviado para o game,
	// entao nao tem problema
	TokenOk = true;

	pMob[clientId].Mobs.Player = LOCAL_215->Mob;
	pMob[clientId].Mode = 2;

	LOCAL_215->Mob.Last.X = LOCAL_215->Mob.Last.X + (Rand() % 5) - 2;
	LOCAL_215->Mob.Last.Y = LOCAL_215->Mob.Last.Y + (Rand() % 5) - 2;

	LOCAL_215->Header.PacketId = 0x114;
	LOCAL_215->Header.ClientId = 0x7531;

	int cityId = pMob[clientId].Mobs.Player.Info.CityID;
	LOCAL_215->WorldPos.X = g_pCityZone[cityId].city_x + Rand() % 10;
	LOCAL_215->WorldPos.Y = g_pCityZone[cityId].city_y + Rand() % 10;

	//0044D885
	LOCAL_215->ClientIndex = clientId;

	pMob[clientId].Last.Time = CurrentTime;
	pMob[clientId].Target.X = LOCAL_215->Mob.Last.X;
	pMob[clientId].Last.X = pMob[clientId].Target.X;

	pMob[clientId].Target.Y = LOCAL_215->Mob.Last.Y;
	pMob[clientId].Last.Y = pMob[clientId].Target.Y;

	LOCAL_215->Mob.Equip[0].EFV2 = pMob[clientId].Mobs.Player.Equip[0].EFV2;

	pMob[clientId].clientId = clientId;

	pMob[clientId].Mobs.Player.Equip[0].EFV2 = pMob[clientId].Mobs.Player.Equip[0].EFV2;
	LOCAL_215->Mob.Equip[0].EFV2 = pMob[clientId].Mobs.Player.Equip[0].EFV2;

	if (pMob[clientId].Mobs.Player.Equip[0].EFV2 >= 3)
	{
		if (!(pMob[clientId].Mobs.Player.LearnedSkill[0] & 0x40000000ull))
			pMob[clientId].Mobs.Player.LearnedSkill[0] |= 0x40000000ull;
	}

	// 0044DC30
	if (!pMob[clientId].Mobs.Player.Inventory[63].sIndex)
	{
		memset(&pMob[clientId].Mobs.Player.Inventory[63], 0, 8);

		pMob[clientId].Mobs.Player.Inventory[63].sIndex = 547;

		pMob[clientId].Mobs.Player.Inventory[63].EF1 = 43;
		pMob[clientId].Mobs.Player.Inventory[63].EF2 = 76;
		pMob[clientId].Mobs.Player.Inventory[63].EF3 = 77;

		SetPKPoint(clientId, 75);
	}

	pUser[clientId].Challenger.Mode = 0;
	pUser[clientId].LastWhisper = 0;
	pUser[clientId].Socket.Error = 0;
	inGame.CharSlot = static_cast<char>(LOCAL_215->	SlotIndex);

	{
		auto now = std::chrono::steady_clock::now();
		pUser[clientId].MacIntegrity.loginTime = now;
		Times.LastDeletedItem = now;
		Times.LastUsedItem = now;
	}

	pUser[clientId].User.CharSlot = inGame.CharSlot;

	pUser[clientId].CrackCount = 0;
	//Users[clientId].Unknow_1796 = 0;
	pUser[clientId].Movement.PacketId = 0x366;
	pUser[clientId].Movement.TimeStamp = 0xE0A1ACA;
	//Users[clientId].Damage.LastType = 0;
	//Users[clientId].Damage.TimeStamp = 0xE0A1ACA;
	//Users[clientId].IlussionTime = 0xE0A1ACA;
	//Users[clientId].Challanger.sIndex = 0;
	//Users[clientId].Challanger.ClassId = 0;

	//*(DWORD*)&Users[clientId].Unknow_2744[0] = 0;
	//memset(&Users[clientId].Unknow[4], -1, 400);

	if (pMob[clientId].Mobs.Player.CurrentScore.Hp <= 0)
		pMob[clientId].Mobs.Player.CurrentScore.Hp = 2;

	memset(&pUser[clientId].AutoTrade, 0, sizeof pUser[clientId].AutoTrade);

	INT32 LOCAL_222 = 0;
	for (; LOCAL_222 < 15; LOCAL_222++)
		pUser[clientId].Trade.Slot[LOCAL_222] = -1;

	for (LOCAL_222 = 0; LOCAL_222 < 12; LOCAL_222++)
		pUser[clientId].AutoTrade.Slots[LOCAL_222] = -1;

	IsAutoTrading = 0;

	pMob[clientId].GuildDisable = 0;
	AllStatus.PK = 0;
	AllStatus.Chat = 0;
	AllStatus.Citizen = 0;
	AllStatus.Guild = 0;
	AllStatus.Kingdom = 0;
	AllStatus.Whisper = 0;

	UINT32 LOCAL_223 = LOCAL_215->WorldPos.X;
	UINT32 LOCAL_224 = LOCAL_215->WorldPos.Y;

	INT32 LOCAL_225 = pMob[clientId].Mobs.Player.Info.CityID;

	//0044DEBB
	LOCAL_223 = g_pCityZone[LOCAL_225].city_x + (Rand() % 14);
	LOCAL_224 = g_pCityZone[LOCAL_225].city_y + (Rand() % 14);

	INT32 LOCAL_226 = pMob[clientId].Mobs.Player.Guild;
	INT32 LOCAL_227 = pMob[clientId].Mobs.Player.Class;

	if (LOCAL_227 < 0 || LOCAL_227 > 3)
	{
		//TODO :  error, login undefined class
		Log(clientId, LOG_INGAME, "login undefined class %d", LOCAL_227);
		CloseUser(clientId);
		return false;
	}

	for (INT32 LOCAL_228 = 0; LOCAL_228 < 5; LOCAL_228++)
	{
		if (LOCAL_226 != 0 && LOCAL_226 == ChargedGuildList[sServer.Channel - 1][LOCAL_228])
		{
			LOCAL_223 = g_pCityZone[LOCAL_228].area_guild_x;
			LOCAL_224 = g_pCityZone[LOCAL_228].area_guild_y;

			break;
		}
	}

	if (pMob[clientId].Mobs.Player.Equip[0].EFV2 == MORTAL && pMob[clientId].Mobs.Player.BaseScore.Level < sServer.NewbieZone)
	{ // area de treinamento
		LOCAL_223 = 2112;
		LOCAL_224 = 2042;
	}

	INT32 LOCAL_229 = GetEmptyMobGrid(clientId, &LOCAL_223, &LOCAL_224);
	if (LOCAL_229 == 0)
	{
		// TODO : Error can't start can't get mobgrid
		Log(clientId, LOG_INGAME, "Nao encontrado espaao no grid em %dx %dy", LOCAL_223, LOCAL_224);

		CloseUser(clientId);
		return false;
	}

	LOCAL_215->WorldPos.X = LOCAL_223;
	LOCAL_215->WorldPos.Y = LOCAL_224;

	pMob[clientId].Target.X = LOCAL_223;
	pMob[clientId].Last.X = pMob[clientId].Target.X;

	pMob[clientId].Target.Y = LOCAL_224;
	pMob[clientId].Last.Y = pMob[clientId].Target.Y;

	this->Status = USER_PLAY;

	pUser[clientId].nTargetX = 0;
	pUser[clientId].nTargetY = 0;
	pUser[clientId].Trade.ClientId = 0;
	pMob[clientId].Lifes = 0;

	pMob[clientId].GetCurrentScore(clientId);

	lastLogEquipAndInventory = std::chrono::high_resolution_clock::now();
	LogEquipsAndInventory(true);

	//*(DWORD*)&LOCAL_215->unknow[0] = Users[clientId].inGame.MagicIncrement;

	AddMessage((BYTE*)LOCAL_215, sizeof p114);

	pUser[clientId].AttackCount = 0;

	pUser[clientId].GoldCount = 0;
	pUser[clientId].Coin = 0;

	pUser[clientId].PremierStore.CurrentScore = 0;
	pUser[clientId].PremierStore.Time = 0;
	pUser[clientId].PremierStore.Count = 0;
	pUser[clientId].PremierStore.Wait = 0;

	pUser[clientId].aHack.Question = -1;
	pUser[clientId].aHack.Response = 0;
	pUser[clientId].aHack.Error = 0;
	pUser[clientId].aHack.Next = 60;
	pUser[clientId].aHack.Last = sServer.SecCounter;

	SummonedUser = 0;
	pUser[clientId].Potion.CountHp = pMob[clientId].Mobs.Player.CurrentScore.Hp;
	pUser[clientId].Potion.CountMp = pMob[clientId].Mobs.Player.CurrentScore.Mp;
	pUser[clientId].Potion.bQuaff = 0;

	pUser[clientId].Damage.TorreErion = 0;

	pMob[clientId].SpawnType = 2;

	p364 LOCAL_256{};
	GetCreateMob(clientId, (BYTE*)&LOCAL_256);

	pMob[clientId].SpawnType = 0;

	g_pMobGrid[LOCAL_215->WorldPos.Y][LOCAL_215->WorldPos.X] = clientId;

	if (!pUser[clientId].IsBanned)
		GridMulticast_2(LOCAL_215->WorldPos.X, LOCAL_215->WorldPos.Y, (BYTE*)& LOCAL_256, 0);
	else
		AddMessage(reinterpret_cast<BYTE*>(&LOCAL_256), sizeof p364);

	SendGridMob(clientId);
	SendScore(clientId);
	SendEtc(clientId);
	SendAutoPartyInfo(clientId);

	Log(clientId, LOG_INGAME, "Personagem %s enviado ao mundo %dx %dy - Canal %d - Cash da conta: %d.", LOCAL_215->Mob.MobName, LOCAL_215->WorldPos.X, LOCAL_215->WorldPos.Y, sServer.Channel, User.Cash);
	Log(clientId, LOG_INGAME, "Gold atual: %d. Classe: %d Evoluaao: %d. Experiancia: %I64d. Navel: %d. Info: %I64d. GuildID: %d. CP: %d", LOCAL_215->Mob.Coin, pMob[clientId].Mobs.Player.Class, pMob[clientId].Mobs.Player.Equip[0].EFV2, LOCAL_215->Mob.Exp,
		LOCAL_215->Mob.BaseScore.Level, pMob[clientId].Mobs.Info.Value, pMob[clientId].Mobs.Player.Guild, GetPKPoint(clientId) - 75);

	LogPlayer(clientId, "Entrou no jogo com o personagem %s", LOCAL_215->Mob.MobName);
	
	pMob[clientId].Jewel = -1;

	for (int i = 0; i < 32; i++)
	{
		if (pMob[clientId].Mobs.Affects[i].Index == 0)
			continue;

		if (pMob[clientId].Mobs.Affects[i].Index == 30)
			Log(clientId, LOG_INGAME, "Logou no personagem %s com frango ativo. Tempo: %d", LOCAL_215->Mob.MobName, pMob[clientId].Mobs.Affects[i].Time);

		else if (pMob[clientId].Mobs.Affects[i].Index == 39)
			Log(clientId, LOG_INGAME, "Logou no personagem %s com baa de experiancia ativo. Tempo: %d", LOCAL_215->Mob.MobName, pMob[clientId].Mobs.Affects[i].Time);

		else if (pMob[clientId].Mobs.Affects[i].Index == 34)
			Log(clientId, LOG_INGAME, "Logou no personagem %s com poaao divina ativo. Validade: %d/%d/%d %d:%d:%d", LOCAL_215->Mob.MobName, pUser[clientId].User.Divina.Dia,
				pUser[clientId].User.Divina.Mes, pUser[clientId].User.Divina.Ano, pUser[clientId].User.Divina.Hora, pUser[clientId].User.Divina.Minuto, pUser[clientId].User.Divina.Segundo);

		else if (pMob[clientId].Mobs.Affects[i].Index == 51)
			Log(clientId, LOG_INGAME, "Logou no personagem %s com poaao revigorante ativo. Validade: %d/%d/%d %d:%d:%d", LOCAL_215->Mob.MobName, pMob[clientId].Mobs.Revigorante.Dia,
				pMob[clientId].Mobs.Revigorante.Mes, pMob[clientId].Mobs.Revigorante.Ano, pMob[clientId].Mobs.Revigorante.Hora, pMob[clientId].Mobs.Revigorante.Minuto, pMob[clientId].Mobs.Revigorante.Segundo);

		else if (pMob[clientId].Mobs.Affects[i].Index == 4)
			Log(clientId, LOG_INGAME, "Logou no personagem %s com poaao sephira ativo. Validade: %d/%d/%d %d:%d:%d", LOCAL_215->Mob.MobName, pUser[clientId].User.Sephira.Dia,
				pUser[clientId].User.Sephira.Mes, pUser[clientId].User.Sephira.Ano, pUser[clientId].User.Sephira.Hora, pUser[clientId].User.Sephira.Minuto, pUser[clientId].User.Sephira.Segundo);

		else if (pMob[clientId].Mobs.Affects[i].Index == 35)
			Log(clientId, LOG_INGAME, "Logou no personagem %s com poaao saade ativo. Validade: %d/%d/%d %d:%d:%d", LOCAL_215->Mob.MobName, pMob[clientId].Mobs.Saude.Dia,
				pMob[clientId].Mobs.Saude.Mes, pMob[clientId].Mobs.Saude.Ano, pMob[clientId].Mobs.Saude.Hora, pMob[clientId].Mobs.Saude.Minuto, pMob[clientId].Mobs.Saude.Segundo);

		else if (pMob[clientId].Mobs.Affects[i].Index == 8)
			pMob[clientId].Jewel = i;
	}

	SendWarInfo(clientId, sServer.CapeWin);

	if(sServer.CastleState)
		SendSignalParm(clientId, 0x7530, 0x3AC, 1);

	SendSignalParm(clientId, 0x7530, RefreshGoldPacket, User.Cash);

	ClearCrown(clientId);

	if (pMob[clientId].Mobs.Player.Equip[6].sIndex == 877)
	{
		memset(&pMob[clientId].Mobs.Player.Equip[6], 0, sizeof STRUCT_ITEM);

		SendItem(clientId, SlotType::Equip, 6, &pMob[clientId].Mobs.Player.Equip[6]);
	}
		
	RemoveParty(clientId);

	MountProcess(clientId, 0);

	INT32 guildId = pMob[clientId].Mobs.Player.Guild;
	if(guildId != 0)
	{
		INT32 capeInfo = pMob[clientId].Mobs.Player.CapeInfo;
		if(capeInfo != g_pGuild[guildId].Kingdom)
		{
			int capeId = pMob[clientId].Mobs.Player.Equip[15].sIndex;
			bool isWhite = std::find(std::begin(g_pCapesID[2]), std::end(g_pCapesID[2]), capeId) != std::end(g_pCapesID[2]);
			unsigned int _index = capeInfo - CAPE_BLUE;
			unsigned int otherIndex = g_pGuild[guildId].Kingdom - CAPE_BLUE;

			if (isWhite)
				_index = 2;

			if ((isWhite || capeId != 4006) && _index >= 0 && _index < g_pCapesID.size() && otherIndex >= 0 && otherIndex <= g_pCapesID.size())
			{
				for (INT32 i = 0; i < 10; i++)
				{
					if (capeId == g_pCapesID[_index][i])
					{
						pMob[clientId].Mobs.Player.Equip[15].sIndex = g_pCapesID[otherIndex][i];

						Log(clientId, LOG_INGAME, "Trocado a capa %d para %hu devido a guild ter reino diferente", capeId, pMob[clientId].Mobs.Player.Equip[15].sIndex);
						SendItem(clientId, SlotType::Equip, 15, &pMob[clientId].Mobs.Player.Equip[15]);
						break;
					}
				}
			}
		}
	}

    if (pMob[clientId].Mobs.Player.GetEvolution() == eClass::SubCelestial && pMob[clientId].Mobs.GetTotalResets() == 3 && pMob[clientId].Mobs.Info.Unlock200)
    {
        if (!pMob[clientId].Mobs.Sub.Info.Unlock200)
        {
            pMob[clientId].Mobs.Sub.Info.Unlock200 = 1;
            Log(clientId, LOG_INGAME, "Desbloqueado o navel 200 do personagem Celestial armazenado na Pedra Misteriosa");
        }
    }

	if (pMob[clientId].Mobs.Player.GetEvolution() >= Celestial && pMob[clientId].Mobs.Player.BaseScore.Level == 199 && pMob[clientId].Mobs.Player.BaseScore.Level < 399 && !pMob[clientId].Mobs.Info.Unlock200 && !pMob[clientId].Mobs.Info.LvBlocked && pMob[clientId].Mobs.GetTotalResets() == 3)
	{
		Log(clientId, LOG_INGAME, "Bloqueado o navel 200 do personagem devido a nao estar bloqueado e nao ter desbloqueado.");

		pMob[clientId].Mobs.Info.LvBlocked = 1;
		pMob[clientId].Mobs.Player.Exp = g_pNextLevel[pMob[clientId].Mobs.Player.Equip[0].EFV2][pMob[clientId].Mobs.Player.BaseScore.Level - 1];

		SendEtc(clientId);
	}

	time_t rawnow = time(NULL);
	struct tm now; localtime_s(&now, &rawnow);

	if (now.tm_yday != User.Water.Day)
	{
		Log(clientId, LOG_INGAME, "Resetado a contagem de entradas agua! Day: %d-%d. Total: %d", User.Water.Day, now.tm_yday, User.Water.Total);
		LogPlayer(clientId, "Resetado a contagem de entradas agua");

		User.Water.Day = now.tm_yday;
		User.Water.Total = 0;
	}

	if (pMob[clientId].Mobs.Player.Inventory[60].sIndex == 3467 && !pMob[clientId].isBagActive(eBag::FirstBag))
	{
		Log(clientId, LOG_INGAME, "Bolsa do Andarilho (1) expirou. %s", pMob[clientId].Mobs.Player.Inventory[60].toString().c_str());

		memset(&pMob[clientId].Mobs.Player.Inventory[60], 0, sizeof STRUCT_ITEM);
		SendItem(clientId, SlotType::Inv, 60, &pMob[clientId].Mobs.Player.Inventory[60]);
	}

	if (pMob[clientId].Mobs.Player.Inventory[61].sIndex == 3467 && !pMob[clientId].isBagActive(eBag::SecondBag))
	{
		Log(clientId, LOG_INGAME, "Bolsa do Andarilho (2) expirou. %s", pMob[clientId].Mobs.Player.Inventory[61].toString().c_str());

		memset(&pMob[clientId].Mobs.Player.Inventory[61], 0, sizeof STRUCT_ITEM);
		SendItem(clientId, SlotType::Inv, 61, &pMob[clientId].Mobs.Player.Inventory[61]);
	}

	if(!User.Block.Pass[0])
		SendSignal(clientId, clientId, 0x904);
	
	if(sServer.KefraKiller != 0)
		SendClientMessage(clientId, g_pLanguageString[_NN_Kefra_GuildKill], g_pGuild[sServer.KefraKiller].MobName.c_str());
	else if(sServer.KefraKiller == 0 && sServer.KefraDead)
		SendClientMessage(clientId, g_pLanguageString[_NN_Kefra_PlayerKill]);

	if (sServer.KingdomBattle.Winner == CAPE_BLUE || sServer.KingdomBattle.Winner == CAPE_RED)
		SendClientMessage(clientId, "O Reino %s a o atual dominante", sServer.KingdomBattle.Winner == CAPE_BLUE ? "Blue" : "Red");

	if(g_pGuildNotice[guildId][0])
		SendChatGuild(clientId, guildId, "--Aviso: %s", g_pGuildNotice[guildId]);

	SendKingdomBattleInfo(clientId, CAPE_BLUE, sServer.KingdomBattle.Info[0].CurrentScore);
	SendKingdomBattleInfo(clientId, CAPE_RED, sServer.KingdomBattle.Info[1].CurrentScore);
 
	auto& mob = pMob[clientId].Mobs;
	if (mob.Player.Equip[0].EFV2 >= CELESTIAL)
	{
		if (mob.Sub.CurrentScore == 1 || mob.Player.Equip[0].EFV2 == SUBCELESTIAL)
		{
			STRUCT_ITEM& capeOne = mob.Player.Equip[15];
			STRUCT_ITEM& capeTwo = mob.Sub.Equip[1];

			Log(clientId, LOG_INGAME, "Evoluaao logada: %d", (int)mob.Player.Equip[0].EFV2);
			Log(clientId, LOG_INGAME, "Dados da capa um: %s", capeOne.toString().c_str());
			Log(clientId, LOG_INGAME, "Dados da capa dois: %s", capeTwo.toString().c_str());

			if (GetItemSanc(&capeTwo) > GetItemSanc(&capeOne))
			{
				Log(clientId, LOG_INGAME, "Trocando para capa dois pois a refinaaao a maior.");
				capeOne = capeTwo;

				SendItem(clientId, SlotType::Equip, 15, &capeOne);
			}
			else
				Log(clientId, LOG_INGAME, "Nao foi efetuado a troca pois a refinaaao atual a maior");
		}
	}

	{
		int totalDivinas = 0;
		int totalSephiras = 0;
		for (int i = 0; i < 120; ++i)
		{
			STRUCT_ITEM* item = &User.Storage.Item[i];
			if (item->sIndex == 3379)
				totalDivinas++;
			else if (item->sIndex == 3361)
				totalSephiras++;
		}

		for (int i = 0; i < 60; ++i)
		{
			STRUCT_ITEM* item = &pMob[clientId].Mobs.Player.Inventory[i];
			if (item->sIndex == 3379)
				totalDivinas++;
			else if (item->sIndex == 3361)
				totalSephiras++;
		}

		if (totalDivinas >= 2 || totalSephiras >= 2)
		{
			Log(clientId, LOG_INGAME, "Foram encontrados um total de %d Divinas e %d Sephiras", totalDivinas, totalSephiras);

			for (int i = 0; i < 120; ++i)
			{
				STRUCT_ITEM* item = &User.Storage.Item[i];
				if (item->sIndex == 3379 && totalDivinas > 1)
				{
					Log(clientId, LOG_INGAME, "Removido %s (banco) do slot %d", item->toString().c_str(), i);

					*item = STRUCT_ITEM{};
					SendItem(clientId, SlotType::Storage, i, item);
					totalDivinas--;
				}
				else if (item->sIndex == 3361 && totalSephiras > 1)
				{
					Log(clientId, LOG_INGAME, "Removido %s (banco) do slot %d", item->toString().c_str(), i);
					*item = STRUCT_ITEM{};
					SendItem(clientId, SlotType::Storage, i, item);

					totalSephiras--;
				}
				else if (item->sIndex == 3379 || item->sIndex == 3361)
				{
					item->stEffect[0].cEffect = EF_NOTRADE;
					item->stEffect[0].cValue = 1;

					SendItem(clientId, SlotType::Storage, i, item);
				}
			}

			for (int i = 0; i < 60; ++i)
			{
				STRUCT_ITEM* item = &pMob[clientId].Mobs.Player.Inventory[i];
				if (item->sIndex == 3379 && totalDivinas > 1)
				{
					Log(clientId, LOG_INGAME, "Removido %s (inventario) do slot %d", item->toString().c_str(), i);

					*item = STRUCT_ITEM{};
					SendItem(clientId, SlotType::Inv, i, item);
					totalDivinas--;
				}
				else if (item->sIndex == 3361 && totalSephiras > 1)
				{
					Log(clientId, LOG_INGAME, "Removido %s (inventario) do slot %d", item->toString().c_str(), i);
					*item = STRUCT_ITEM{};
					SendItem(clientId, SlotType::Inv, i, item);

					totalSephiras--;
				}
				else if (item->sIndex == 3379 || item->sIndex == 3361)
				{
					item->stEffect[0].cEffect = EF_NOTRADE;
					item->stEffect[0].cValue = 1;

					SendItem(clientId, SlotType::Inv, i, item);
				}
			}
		}
	}

	std::string playerName{ mob.Player.MobName };
	for (int i = 0; i < 3; ++i)
	{
		const auto& nightmare = sServer.Nightmare[i];
		if(std::find(std::begin(nightmare.MembersName), std::end(nightmare.MembersName), playerName) == std::end(nightmare.MembersName))
			continue;

		SendSignalParm2(clientId, SERVER_SIDE, NightmareCanEnterWarnPacket, i, nightmare.TimeLeft);
	}

#if defined(_DEBUG)
	IsAdmin = true;
	AccessLevel = 100;
#endif

	return true;
}