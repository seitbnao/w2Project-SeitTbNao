#include "Basedef.h"
#include "SendFunc.h"
#include "CDuelLetter.h"
#include <map>
#include <vector>
#include <utility>

constexpr int DefaultTime = 220;

std::map<eDuelLetter_Room, std::pair<int, int>> roomPositions =
{
	{ eDuelLetter_Room::FirstRoom, { 786, 3680 } },
	{ eDuelLetter_Room::SecondRoom, { 844, 3688 } },
	{ eDuelLetter_Room::ThirdRoom, { 844, 3632 } },
	{ eDuelLetter_Room::FourthRoom, { 786, 3639 } },
	{ eDuelLetter_Room::BossRoom, { 0, 0 } },
};

std::map<eDuelLetter_Room, std::vector<std::pair<int, int>>> runePositions =
{
	{ 
		eDuelLetter_Room::FirstRoom, {
		{ 787, 3676 }
	}},
	{
		eDuelLetter_Room::SecondRoom, {
		{ 845, 3676 },
		{ 844, 3676 }
	}},
	{ eDuelLetter_Room::ThirdRoom, {
		{ 845, 3620 },
		{ 844, 3621 },
		{ 845, 3621 }
	}},
	{ eDuelLetter_Room::FourthRoom, {
		{ 787, 3634},
		{ 786, 3634 },
		{ 787, 3635 },
		{ 786, 3635 },
	}},
	{ eDuelLetter_Room::BossRoom, {
		{ 787, 3634 },
		{ 786, 3639 },
		{ 887, 3635 }
	}}
};

void CDuelLetter::Work()
{
	if (!_status)
		return;

	_timer--;
	if (_timer <= 0)
	{
		Finish(true);

		return;
	}

	CheckUsers();

	if (_users.size() == 0)
	{
		Finish(false);

		return;
	}

	int totalMob = GetMobCountOnRoom();
	int index = GetInitialIndex();
	if (totalMob != 0)
	{
		for (const auto& user : _users)
			SendSignalParm(user->clientId, 0x7530, 0x3B0, totalMob);

		return;
	}

	if (_room == eDuelLetter_Room::BossRoom)
		return;

	// Muda a sala do menino
	auto temporaryRoom = GetNextRoom();
	if (temporaryRoom == eDuelLetter_Room::FourthRoom)
	{
		if (!_haveCourageSymbol)
			return;
	}

	_room = temporaryRoom;
	if (_room != eDuelLetter_Room::BossRoom)
		_timer = DefaultTime;

	GenerateRoom();

	if (_room != eDuelLetter_Room::BossRoom)
	{
		auto position = roomPositions[_room];
		totalMob = GetMobCountOnRoom();

		for (const auto& user : _users)
		{
			SendSignalParm(user->clientId, 0x7530, 0x3B0, totalMob);
			SendSignalParm(user->clientId, 0x7530, 0x3A1, _timer);

			Teleportar(user->clientId, position.first, position.second);
			LogGroup("Teleportado para a sala %dy %dx", position.first, position.second);
		}
	}
}

void CDuelLetter::LogGroup(const char* message, ...)
{
	/* Arglist */
	char buffer[150];
	va_list arglist;
	va_start(arglist, message);
	vsprintf_s(buffer, message, arglist);
	va_end(arglist);
	/* Fim arlist */

	for (const auto& user : _users)
		Log(user->clientId, LOG_INGAME, buffer);
}

void CDuelLetter::CheckUsers()
{
	_users.erase(std::remove_if(_users.begin(), _users.end(), [](const CUser* user) 
	{
		if (user->Status != USER_PLAY)
			return true;

		CMob& mob = pMob[user->clientId];
		return mob.Target.X < 782 || mob.Target.X > 886 || mob.Target.Y < 3598 || mob.Target.Y > 3695;
	}), _users.end());

	for (int i = 1; i < MAX_PLAYER; i++)
	{
		if (pMob[i].Mode == 0 || pUser[i].IsAdmin)
			continue;

		if (pMob[i].Target.X >= 782 && pMob[i].Target.X <= 886 && pMob[i].Target.Y >= 3598 && pMob[i].Target.Y <= 3695)
		{
			if (std::find_if(_users.begin(), _users.end(), [&](CUser* user) {
				return user->clientId == i;
			}) == _users.end())
			{
				Log(i, LOG_INGAME, "Dentro da area sem estar presente na lista de membros da Carta de Duelo");
				DoRecall(i);
			}
		}
	}
}

bool CDuelLetter::CanRegister(CUser& user)
{
	return !this->_status;
}

bool CDuelLetter::Register(CUser& user, STRUCT_ITEM* item)
{
	if (item == nullptr)
		throw std::invalid_argument("Need to have an associate item");

	if (!CanRegister(user))
	{
		Log(user.clientId, LOG_INGAME, "Register da Carta de Duelo chamada sem ser possavel de registrar");

		return false;
	}

	// Por precausao, limpamos a area por completo
	Finish(false);

	// Adicionamos o praprio lader aqui
	_users.push_back(&user);

	for (int i = 0; i < 12; i++)
	{
		auto mobId = pMob[user.clientId].PartyList[i];
		if (mobId <= 0 || mobId >= MAX_PLAYER)
			continue;

		_users.push_back(&pUser[mobId]);

		Log(user.clientId, LOG_INGAME, "Carta de Duelo N - Grupo %s (%d)", pMob[mobId].Mobs.Player.MobName, i);
	}

	_room = eDuelLetter_Room::FirstRoom;
	_timer = DefaultTime;
	_type = GetType(item);

	if (pMob[user.clientId].Mobs.Player.Equip[10].sIndex == 1732)
	{
		_haveCourageSymbol = true;
		Log(user.clientId, LOG_INGAME, "Consumido Sambolo da Coragem");

		memset(&pMob[user.clientId].Mobs.Player.Equip[10], 0, sizeof STRUCT_ITEM);
		SendItem(user.clientId, SlotType::Equip, 10, &pMob[user.clientId].Mobs.Player.Equip[10]);
	}
	else
		Log(user.clientId, LOG_INGAME, "Nao possuaa o Sambolo da Coragem");

	GenerateRoom();

	int totalMob = GetMobCountOnRoom();

	const auto& position = roomPositions[_room];
	for (const auto& users : _users)
	{
		Teleportar(users->clientId, position.first, position.second);

		SendSignalParm(users->clientId, 0x7530, 0x3A1, DefaultTime);
		SendSignalParm(users->clientId, 0x7530, 0x3B0, totalMob);

		Log(users->clientId, LOG_INGAME, "Teleportado para quest %s no grupo de %s", g_pItemList[item->sIndex].ItemName, pMob[user.clientId].Mobs.Player.MobName);
	}

	_status = true;
	return true;
}

eDuelLetterType CDuelLetter::GetType(STRUCT_ITEM* item) const
{
	eDuelLetterType type = eDuelLetterType::Arcane;
	if (item->sIndex == 3172)
		type = eDuelLetterType::Normal;
	else if (item->sIndex == 3171)
		type = eDuelLetterType::Mystical;

	return type;
}

int CDuelLetter::GetInitialIndex() const
{
	if (_type == eDuelLetterType::Mystical)
		return DuelLetter_Mystical_Index;
	else if (_type == eDuelLetterType::Arcane)
		return DuelLetter_Arcane_Index;

	return DuelLetter_Normal_Index;
}

void CDuelLetter::GenerateRune()
{
	// Na altima sala nao gera runa alguma
	if (_room == eDuelLetter_Room::BossRoom)
		return;

	STRUCT_ITEM rune{};
	if (!(Rand() % 15))
		rune.sIndex = 1736;
	// Repetir a 1a runa gerada
	if (!(Rand() % 6) && _runesId.size() != 0)
		rune.sIndex = _runesId.begin()->second;
	else
	{
		int missingRune = 1733;

		if (_runesId.size() != 0)
		{
			for (int i = 1733; i < 1736; i++)
			{
				if (std::find_if(_runesId.begin(), _runesId.end(), [&](std::pair<eDuelLetter_Room, int> rune) { return rune.second == i; }) != std::end(_runesId))
					continue;

				rune.sIndex = i;
			}
		}
		
		if(rune.sIndex == 0)
			rune.sIndex = 1733 + Rand() % 3;
	}

	_runesId[_room] = rune.sIndex;
	LogGroup("Runa %hu gerada na sala %d", rune.sIndex, static_cast<int>(_room));

	std::vector<eDuelLetter_Room> rooms;
	if (_room == eDuelLetter_Room::FirstRoom)
		rooms.push_back(eDuelLetter_Room::FirstRoom);
	
	if (_room == eDuelLetter_Room::SecondRoom)
	{
		rooms.push_back(eDuelLetter_Room::FirstRoom);
		rooms.push_back(eDuelLetter_Room::SecondRoom);
	}

	if (_room == eDuelLetter_Room::ThirdRoom)
	{
		rooms.push_back(eDuelLetter_Room::FirstRoom);
		rooms.push_back(eDuelLetter_Room::SecondRoom);
		rooms.push_back(eDuelLetter_Room::ThirdRoom);
	}

	if (_room == eDuelLetter_Room::FourthRoom)
	{
		rooms.push_back(eDuelLetter_Room::FirstRoom);
		rooms.push_back(eDuelLetter_Room::SecondRoom);
		rooms.push_back(eDuelLetter_Room::ThirdRoom);
		rooms.push_back(eDuelLetter_Room::FourthRoom);
	}

	// Itera sobre todas as poisaaes que a runa deve ser criada
	for (size_t index = 0; index < runePositions[_room].size(); index++)
	{
		const auto& runePos = runePositions[_room][index];

		STRUCT_ITEM item{};
		item.sIndex = _runesId[rooms[index]];

		int initIndex = CreateItem(runePos.first, runePos.second, &item, 0, 0, 0);
		if (initIndex == 0)
		{
			Log(SERVER_SIDE, LOG_INGAME, "Falha ao gerar o item no chao");

			continue;
		}

		_initRunes[_room].push_back(initIndex);
	}
}

bool CDuelLetter::CanGenerateBoss() const
{
	std::array haveRunes{ false, false, false };

	for (const auto& [room, rune] : _runesId)
	{
		if (rune >= 1733 && rune <= 1735)
			haveRunes[rune - 1733] = true;
	}

	return std::find(haveRunes.begin(), haveRunes.end(), false) == haveRunes.end();
}

void CDuelLetter::GenerateRoom()
{
	int index = GetInitialIndex();

	int initialSum;
	int totalLoop;
	std::tie(initialSum, totalLoop) = GetGenerIndexes();

	if (_room == eDuelLetter_Room::BossRoom)
	{
		if (CanGenerateBoss())
		{
			if (_type == eDuelLetterType::Normal)
				GenerateMob(index + 11, 0, 0);
			else
				GenerateMob(index + 12, 0, 0);
		}

		return;
	}

	for (int i = 0; i < totalLoop; i++)
	{
		GenerateMob(index + i + initialSum, 0, 0);
		Log(SERVER_SIDE, LOG_INGAME, "Gerado o gener %d", index + i + initialSum);
	}

	GenerateRune();
}

std::tuple<int, int> CDuelLetter::GetGenerIndexes() const
{
	if (_type == eDuelLetterType::Normal)
	{
		int initialSum = 0;
		int totalLoop = 3;

		if (_room == eDuelLetter_Room::SecondRoom)
		{
			initialSum = 3;
			totalLoop = 2;
		}

		if (_room == eDuelLetter_Room::ThirdRoom)
			initialSum = 5;

		if (_room == eDuelLetter_Room::FourthRoom)
		{
			initialSum = 8;
			totalLoop = 2;
		}

		if (_room == eDuelLetter_Room::BossRoom)
		{
			totalLoop = 1;
			initialSum = 11;
		}

		return std::make_tuple(initialSum, totalLoop);
	}

	if (_type == eDuelLetterType::Mystical)
	{
		int initialSum = 0;
		int totalLoop = 3;

		if (_room == eDuelLetter_Room::SecondRoom)
			initialSum = 3;

		if (_room == eDuelLetter_Room::ThirdRoom)
			initialSum = 6;

		if (_room == eDuelLetter_Room::FourthRoom)
		{
			initialSum = 9;
			totalLoop = 2;
		}

		if (_room == eDuelLetter_Room::BossRoom)
		{
			totalLoop = 1;
			initialSum = 12;
		}

		return std::make_tuple(initialSum, totalLoop);
	}

	if (_type == eDuelLetterType::Arcane)
	{
		int initialSum = 0;
		int totalLoop = 3;

		if (_room == eDuelLetter_Room::SecondRoom)
			initialSum = 3;

		if (_room == eDuelLetter_Room::ThirdRoom)
			initialSum = 6;

		if (_room == eDuelLetter_Room::FourthRoom)
		{
			initialSum = 9;
			totalLoop = 2;
		}

		if (_room == eDuelLetter_Room::BossRoom)
		{
			totalLoop = 1;
			initialSum = 12;
		}

		return std::make_tuple(initialSum, totalLoop);
	}

	return std::make_tuple(0, 0);
}

int CDuelLetter::GetMobCountOnRoom() const
{
	int initialIndex = GetInitialIndex();

	int initialSum;
	int totalLoop;
	std::tie(initialSum, totalLoop) = GetGenerIndexes();

	int totalMob = 0;
	for (int i = 0; i < totalLoop; i++)
		totalMob += mGener.pList[initialIndex + initialSum + i].MobCount;

	return totalMob;
}

eDuelLetter_Room CDuelLetter::GetNextRoom() const
{
	if (_room == eDuelLetter_Room::FirstRoom)
		return eDuelLetter_Room::SecondRoom;

	if (_room == eDuelLetter_Room::SecondRoom)
		return eDuelLetter_Room::ThirdRoom;

	if (_room == eDuelLetter_Room::ThirdRoom)
		return eDuelLetter_Room::FourthRoom;

	if (_room == eDuelLetter_Room::FourthRoom)
		return eDuelLetter_Room::BossRoom;

	throw std::runtime_error("Invalid room");
}

void CDuelLetter::Finish(bool outOfTime)
{
	for (int i = 1000; i < 30000; i++)
	{
		if (pMob[i].GenerateID >= DuelLetter_Normal_Index && pMob[i].GenerateID <= DuelLetter_Arcane_Index + 12)
			DeleteMob(i, 1);
	}

	for (const auto& user : _users)
	{
		DoRecall(user->clientId);

		if (outOfTime)
		{
			SendClientMessage(user->clientId, "Tempo esgotado");
			Log(user->clientId, LOG_INGAME, "Jogado para fora da Carta de Duelo pois o tempo acabou");
		}
	}

	ClearArea(786, 3656, 826, 3695);
	ClearArea(782, 3599, 830, 3643);
	ClearArea(841, 3655, 886, 3695);
	ClearArea(841, 3598, 886, 3639);

	_room = eDuelLetter_Room::InvalidRoom;
	_timer = 0;
	_status = false;

	_users.clear();

	// Tiramos os itens do chao
	for (auto& [room, runesList] : _initRunes)
	{
		for (const auto rune : runesList)
		{
			g_pInitItem[rune].Open = 0;
			g_pItemGrid[g_pInitItem[rune].PosY][g_pInitItem[rune].PosX] = 0;

			g_pInitItem[rune].Item = STRUCT_ITEM{};
		}
	}

	_runesId.clear();
	_initRunes.clear();
}