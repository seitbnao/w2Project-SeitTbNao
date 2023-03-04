#include "cServer.h"
#include "Basedef.h"
#include "SendFunc.h"
#include "GetFunc.h"
#include <algorithm>
#include <sstream>
#include <iomanip>
using namespace std::chrono_literals;


std::array<CUser, MAX_PLAYER> pUser;

CUser::CUser()
{
	CurrentScore = USER_EMPTY;
	Socket.Socket = 0;

	hashIncrement = 0;
	Keys[0] = 0;

	clientId = 0;

	IsAdmin = false;
	IsAutoTrading = false;
	AccountID = 0;
	AccessLevel = 0;

	CrackCount = 0;

	Socket.recvBuffer = NULL;
	Socket.sendBuffer = NULL;
	Arena.GroupIndex = MAXUINT32;
}

CUser::~CUser()
{
	if (Socket.recvBuffer != NULL)
	{
		delete[] Socket.recvBuffer;
		Socket.recvBuffer = NULL;
	}

	if (Socket.sendBuffer != NULL)
	{
		delete[] Socket.sendBuffer;
		Socket.sendBuffer = NULL;
	}

	Socket.SizeOfSend = 0;
	CurrentScore = USER_EMPTY;
}

BYTE CUser::GetHashKey()
{
	if(*Keys == 0)
		return -1;

	BYTE key = 0;
	if (hashIncrement <= 15)
	{
		key = Keys[hashIncrement];

		hashIncrement++;
	}
	else
	{
		switch (Keys[15] & 0x80000001)
		{
		case 0:
			key = (BYTE)(Keys[1] + Keys[3] + Keys[5] - 87);
			break;
		case 1:
			key = (BYTE)(Keys[13] + Keys[11] - Keys[9] + 4);
			break;
		}
	}

	return BYTE(key ^ 255);
}

/*
bool CUser::CloseUser()
{
	// Usuario esta ingame
	if(CurrentScore == USER_PLAY)
	{
		RemoveParty(clientId, 0);
		
		DeleteMob(clientId, 0);
		// Checa se a posição ê valida
		// Se o usuario estiver dentro de uma posição valida e se ê o usuario que esta registrado, 
		// apaga ele do local onde lee esta
		// Para os outros usuarios entêo nêo verem ele
		INT32 posX = pMob[clientId].Target.X;
		INT32 posY = pMob[clientId].Target.Y;
		if(posX >= 0 && posX < 4096 && posY >= 0 && posY < 4096 && g_pMobGrid[posY][posX] == clientId)
			g_pMobGrid[posY][posX] = 0;
		
		DeleteMob(clientId, 2);
		
		STRUCT_SAVECHARACTER packet;
		memset(&packet, 0, sizeof packet);

		packet.Header.PacketId = 0x800;
		packet.Header.ClientId = clientId;

		packet.CharSlot = inGame.CharSlot;
		packet.Coin = User.Storage.Coin;

		memcpy(&packet.Storage, &User.Storage.Item, sizeof STRUCT_ITEM * 128);
		memcpy(&packet.Mob, &pMob[clientId].Mobs, sizeof STRUCT_CHARINFO);
		memcpy(packet.SkillBar, &pMob[clientId].Mobs.Player.ShortSkill, 4);
		memcpy(&packet.SkillBar[4], pMob[clientId].Mobs.SkillBar, 16);
		strncpy_s(packet.User, this->User.Username, 16);

		memcpy(packet.Affect, pMob[clientId].Mobs.Affects, sizeof STRUCT_AFFECT * 32);

		packet.Arg2 = 2;

		AddMessageDB((BYTE*)&packet, sizeof STRUCT_SAVECHARACTER);
	}
	else if(CurrentScore > 0)
	{
		STRUCT_SAVECHARACTER packet;
		memset(&packet, 0, sizeof packet);

		packet.Header.PacketId = 0x800;
		packet.Header.ClientId = clientId;

		packet.CharSlot = -1;
		packet.Coin = User.Storage.Coin;

		memcpy(&packet.Storage, &User.Storage.Item, sizeof STRUCT_ITEM * 128);
		strncpy_s(packet.User, this->User.Username, 16);

		packet.Arg2 = 2;

		AddMessageDB((BYTE*)&packet, sizeof STRUCT_SAVECHARACTER);
	}
	
	Log(clientId, LOG_INGAME, "Personagem finalizado.");

	IsAutoTrading = false;
	
	// Zera o acesso a quest
	QuestAccess = 0;

	// Apaga o SND do usuario
	SNDMessage[0] = 0;
	// Apaga a êltima pessoa que o usuario conversou
	LastWhisper = 0;

	// 
	pMob[clientId].Tab[0] = 0;

	// Zera o tempo que o usuario esta online
	CharLoginTime = 0;

	TimeStamp.LastReceiveTime = 0;
	TimeStamp.LastAttack = 0;

	inGame.incorrectNumeric = 0;

	Socket.SizeOfSend = 0;
	CurrentScore = USER_EMPTY;
	
	hashIncrement = 0;
	memset(&Keys[0], 0, 16);

	TokenOk = false;
		
	Socket.Init = 0;
	closesocket(Socket.Socket);
	return true;2
}*/

bool CUser::CloseSocket()
{
	std::lock_guard<decltype(CUser::messageMutex)> mutex{ messageMutex };

	Socket.SizeOfSend = 0;
	Socket.Init = 0;
	Socket.nProcPosition = 0;
	Socket.nRecvPosition = 0;

	if (Socket.Socket)
	{
		closesocket(Socket.Socket);
		Socket.Socket = 0;
	}

	if (Socket.recvBuffer != nullptr)
	{
		delete[] Socket.recvBuffer;
		Socket.recvBuffer = nullptr;
	}

	if (Socket.sendBuffer != nullptr)
	{
		delete[] Socket.sendBuffer;
		Socket.sendBuffer = nullptr;
	}
	
	Socket.Socket = 0;

	return 1;
}

bool CUser::CloseUser_OL1()
{
	CloseSocket();

	Socket.Socket = 0;
	CurrentScore = USER_EMPTY;
	User.Username[0] = 0;
	return 1;
}

void CUser::RefreshSendBuffer()
{
	INT32 LOCAL_2 = Socket.nSendPosition - Socket.nSentPosition;
	if(LOCAL_2 <= 0)
		return;

	memcpy(Socket.sendBuffer, Socket.sendBuffer + Socket.nSentPosition, LOCAL_2);
	Socket.nSentPosition = 0;
	Socket.nSendPosition -= LOCAL_2;
}

bool CUser::canConnectionContinue(int error) const
{
	constexpr std::array allowedErrors{ 10035 };
	
	return std::find(allowedErrors.begin(), allowedErrors.end(), error) != allowedErrors.end();
}

BOOL CUser::SendMessageA(void)
{
	if(Socket.Socket <= 0)
	{
		Socket.nSendPosition = 0;
		Socket.nSentPosition = 0;
		
		return false;
	}

	//std::lock_guard<decltype(messageMutex)> mutex{ messageMutex };

	if(Socket.nSentPosition > 0)
		RefreshSendBuffer();

	if(Socket.nSendPosition > MAX_BUFFER || Socket.nSendPosition < 0)
	{
		Log(clientId, LOG_INGAME, "Buffer invalido ao tentar enviar pacote. nSendPosition: %d", Socket.nSendPosition);
		Socket.nSendPosition = 0;
		Socket.nSentPosition = 0;

		return true;
	}

	if(Socket.nSentPosition > Socket.nSendPosition || Socket.nSentPosition > MAX_BUFFER || Socket.nSentPosition < 0)
	{
		Log(clientId, LOG_INGAME, "Buffer invalido ao tentar enviar pacote. nSendPosition: %d. nSentPosition: %d", Socket.nSendPosition, Socket.nSentPosition);
		Socket.nSendPosition = 0;
		Socket.nSentPosition = 0;
	}

	INT32 err = 0;
	for(INT32 i = 0; i < 1; i++)
	{
		INT32 LOCAL_68 = Socket.nSendPosition - Socket.nSentPosition;

		INT32 LOCAL_69 = send(Socket.Socket, (char*)Socket.sendBuffer + Socket.nSentPosition, LOCAL_68, 0);
		if(LOCAL_69 != -1)
			Socket.nSentPosition += LOCAL_69;
		else
			err = WSAGetLastError();
	}

	if (err != 0)
	{
		if (!canConnectionContinue(err))
		{
			Log(clientId, LOG_INGAME, "Erro no envio do pacote. Desconectando usuario. ErrorCode: %d", err);
			return false;
		}

		CheckIdle(clientId);

		Socket.Error++;
		if (Socket.Error < 10)
			Log(clientId, LOG_INGAME, "Erro no envio do pacote. Total a ser enviado: %d. Error: %d", Socket.nSendPosition, err);
	}
	else if (Socket.Error != 0)
		Socket.Error = 0;

	if(Socket.nSentPosition >= Socket.nSendPosition || err != 0)
	{
		Socket.nSendPosition = 0;
		Socket.nSentPosition = 0;

		return true;
	}

	if (Socket.nSendPosition > MAX_BUFFER)
	{
		Log(clientId, LOG_INGAME, "Buffer de envio cheio");

		return false;
	}

	return true;
}

BOOL CUser::SendOneMessage(BYTE *packet, INT32 size)
{
	AddMessage(packet, size);

	return SendMessageA();
}

std::string CUser::LogSameAccounts() const
{
	std::stringstream str;
	auto users = GetSameMACUsers(*this);
	
	str << "[USUaRIOS LOGADOS]" << std::endl;

	if (users.size() > 0)
	{
		for (const auto& user : users)
		{
			str << "[" << user->User.Username << "]\t";

			if (user->CurrentScore == USER_SELCHAR)
				str << "SELCHAR\t";
			else if (user->CurrentScore == USER_PLAY)
			{
				const auto& mob = pMob[user->clientId].Mobs.Player;
				str << "USER_PLAY\t" << "[" << pMob[user->clientId].Mobs.Player.MobName << "] (level: " << mob.CurrentScore.Level << " ev: " << (int)mob.GetEvolution() << ")\t";
			}

			str << std::endl;
		}
	}
	else
		str << "Nenhuma outra conta logada " << std::endl;;


	return str.str().c_str();
}

void CUser::LogEquipsAndInventory(bool force) const
{
	if (std::chrono::high_resolution_clock::now() - lastLogEquipAndInventory < 30min && !force)
		return;

	std::stringstream strItems;
	if (CurrentScore == USER_SELCHAR)
	{
		strItems << "[INFORMAçãES DA CONTA]\n";
		strItems << "Personagem na seleção de personagem. Sem informaçães no momento\n";
		strItems << "Tempo online atê o momento: "
			<< std::setfill('0') << std::setw(2) << (pUser[clientId].Time / 86400) << " dias e "
			<< std::setfill('0') << std::setw(2) << (pUser[clientId].Time / 3600) % 24
			<< ":" << std::setfill('0') << std::setw(2) << (pUser[clientId].Time / 60) % 60
			<< ":" << std::setfill('0') << std::setw(2) << (pUser[clientId].Time % 60) << " horas\n";
	}
	else if(CurrentScore == USER_PLAY)
	{
        auto evolution = pMob[clientId].Mobs.Player.GetEvolution();
		strItems << "[INFORMAçãES DA CONTA]\n";
		strItems << "Personagem online: " << pMob[clientId].Mobs.Player.MobName << "\n";
		strItems << "Gold atual: " << pMob[clientId].Mobs.Player.Coin << "\n";
		strItems << "Level atual: " << pMob[clientId].Mobs.Player.BaseScore.Level << "\n";
		strItems << "Experiência atual: " << pMob[clientId].Mobs.Player.Exp << "\n";
		strItems << "Evolução atual: " << GetEvolutionName(evolution) << "\n";
		strItems << "Tempo online atê o momento: "
			<< std::setfill('0') << std::setw(2) << (pUser[clientId].Time / 86400) << " dias e "
			<< std::setfill('0') << std::setw(2) << (pUser[clientId].Time / 3600) % 24
			<< ":" << std::setfill('0') << std::setw(2) << (pUser[clientId].Time / 60) % 60
			<< ":" << std::setfill('0') << std::setw(2) << (pUser[clientId].Time % 60) << " horas\n";
		strItems << "Posição atual: " << pMob[clientId].Target.X << "x " << pMob[clientId].Target.Y << "y\n\n";
		strItems << "Pontos NT: " << ((int)pMob[clientId].Mobs.PesaEnter) << "\n";
		strItems << "Entradas Hall: " << pMob[clientId].Mobs.HallEnter << "\n";
		strItems << "Info: " << pMob[clientId].Mobs.Info.Value << "\n";
		strItems << "Soul: " << ((int)pMob[clientId].Mobs.Soul) << "\n";
		strItems << "Divina: " << pMob[clientId].Mobs.Divina.toString().c_str() << "\n";
		strItems << "Sephira: " << pMob[clientId].Mobs.Sephira.toString().c_str() << "\n";
		strItems << "Saude: " << pMob[clientId].Mobs.Saude.toString().c_str() << "\n";
		strItems << "Escritura: " << pMob[clientId].Mobs.Escritura.toString().c_str() << "\n";
		strItems << "Revigorante: " << pMob[clientId].Mobs.Revigorante.toString().c_str() << "\n";
		strItems << "LastGuildKickout: " << pMob[clientId].Mobs.LastGuildKickOut.toString().c_str() << "\n";

        // verifica se esta no sub ou se tem subcelestial
        if (evolution == eClass::SubCelestial || pMob[clientId].Mobs.Sub.CurrentScore == 1)
        {
            const auto& sub = pMob[clientId].Mobs.Sub;
            strItems << "\n[PEDRA MISTERIOSA]\n";
            strItems << "Evolução: " << GetEvolutionName(static_cast<eClass>(sub.Equip[0].EFV2)) << "\n";
            strItems << "Level atual: " << sub.SubStatus.Level << "\n";
            strItems << "Experiência atual: " << sub.Exp << "\n";
            strItems << "Learn: " << sub.LearnedSkill << "\n";
            strItems << "SecLearn: " << sub.SecLearn << "\n";
            strItems << "Info: " << sub.Info.Value << "\n";
            strItems << "Soul: " << ((int)sub.Soul) << "\n\n";
        }

		strItems << "[EQUIPAMENTOS]\n";

		for (int i = 0; i < 16; i++)
		{
			STRUCT_ITEM* item = &pMob[clientId].Mobs.Player.Equip[i];
			if (item->sIndex <= 0 || item->sIndex > MAX_ITEMLIST)
				continue;

			strItems << "[" << std::setw(2) << i << "] - " << g_pItemList[item->sIndex].ItemName << item->toString().c_str() << "\n";
		}

		strItems << "\n[INVENTaRIO]\n";

		for (int i = 0; i < 64; i++)
		{
			STRUCT_ITEM* item = &pMob[clientId].Mobs.Player.Inventory[i];
			if (item->sIndex <= 0 || item->sIndex > MAX_ITEMLIST)
				continue;

			strItems << "[" << std::setw(2) << i << "] - " << g_pItemList[item->sIndex].ItemName << item->toString().c_str() << "\n";
		}
	}
	else
	{
		strItems << "[INFORMAçãES DA CONTA]\n";
		strItems << "Personagem com status \"" << CurrentScore << "\". Sem informaçães no momento\n";
		strItems << "Tempo online atê o momento: "
			<< std::setfill('0') << std::setw(2) << (pUser[clientId].Time / 86400) << " dias e "
			<< std::setfill('0') << std::setw(2) << (pUser[clientId].Time / 3600) % 24
			<< ":" << std::setfill('0') << std::setw(2) << (pUser[clientId].Time / 60) % 60
			<< ":" << std::setfill('0') << std::setw(2) << (pUser[clientId].Time % 60) << " horas\n";
	}

	strItems << std::endl << LogSameAccounts();

	Log(clientId, LOG_INGAME, strItems.str().c_str());
	lastLogEquipAndInventory = std::chrono::high_resolution_clock::now();
}

std::tuple<bool, std::chrono::milliseconds> CUser::CheckIfIsTooFast(const STRUCT_ITEM* item, int slotId) 
{
	// Verifica na lista de itens recentes deletados se consta o mesmo
	auto result = std::find_if(std::begin(Dropped.Items), std::end(Dropped.Items), [&](DroppedItem& droppedItem) {
		return memcmp(&droppedItem.Item, item, sizeof STRUCT_ITEM) == 0 && droppedItem.SlotId == slotId;
	});

	if (result != std::end(Dropped.Items))
	{
		auto time = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - result->Time);

		result->Item = STRUCT_ITEM{};
		return std::make_pair(time < 700ms, time);
	}

	return std::make_pair(false, std::chrono::milliseconds(0));
}

void CUser::GenerateNewAutoTradeTime()
{
	bool hasPotion = false;
	for (int i = 0; i < 32; i++)
	{
		if (pMob[clientId].Mobs.Affects[i].Index == 59)
		{
			hasPotion = true;

			break;
		}
	}

	int time;
	if (!hasPotion)
		time = (Rand() % 11) + 15;
	else
		time = (Rand() % 8) + 7;

	EventAutoTrade.TimeToWin = std::chrono::milliseconds(time * 1000 * 60);
	Log(clientId, LOG_INGAME, "Espera aproximadamente para receber item do evento: %d minutos.", time);
}