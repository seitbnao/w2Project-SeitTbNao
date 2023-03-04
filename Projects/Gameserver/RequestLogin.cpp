#include "cServer.h"
#include "Basedef.h"
#include "SendFunc.h"
#include "CGUID.h"
#include <sstream>
void EncryptVersion(UINT32 *pVersion)
{
	INT32 random = (Rand() % 7) + 1;

	INT32 randIterator = Rand() & 0x80000003;
	if (randIterator < 0)
	{
		randIterator--;
		randIterator |= 0xFFFFFFFC;
		randIterator++;
	}

	*pVersion = ((*pVersion << (byte)(random + 5)) | (random << 2)) | randIterator;
}

void DecryptVersion(UINT32 *pVersion)
{
	*pVersion >>= (byte)(((*pVersion & 28) >> 2) + 5);
}

bool CUser::RequestLogin(PacketHeader *Header)
{
	p20D *p = (p20D*)Header;

	// Decripta a versao enviada pelo cliente
	//DecryptVersion(&p->CliVer);

	p->Login[15] = 0;
	p->Password[11] = 0;

#ifdef _DEBUG
	// Checa com a versao do servidor
	if(p->CliVer != sServer.CliVer)
	{
		//SendClientMessage(clientId, g_pLanguageString[_NN_Version_Not_Match_Rerun]);
		SendClientMessage(clientId, "Versao de cliente %d incompativel!", p->CliVer);
		Log(clientId, LOG_INGAME, "Tentativa de logar com cliver incorreto. Cliver: %u", p->CliVer);
		SendMessageA();
		return true;
	}
#endif 

	// Caso ele nao esteja no modo correto
	if(CurrentScore != USER_ACCEPT)
	{
		SendClientMessage(clientId, "Login now, wait a moment.");

		SendMessageA();
		return true;
	}

	std::vector<CUser*> users;
	INT32 total = 0;
	for(INT32 i = 1; i < MAX_PLAYER; i++)
	{
		if(pUser[i].CurrentScore < USER_SELCHAR || i == clientId)
			continue;

		if (memcmp(pUser[i].MacAddress, p->Mac, 8) == 0)
		{
			total++;

			users.push_back(&pUser[i]);
		}
	}

	std::stringstream str;
	for (const auto& user : users)
	{
		str << "Conta " << user->User.Username << " logado na conta\n";
		str << "Status da conta: ";
		if (user->CurrentScore == USER_PLAY)
		{
			str << "JOGANDO\n";
			str << "Personagem: " << pMob[user->clientId].Mobs.Player.MobName;
		}
		else if (user->CurrentScore == USER_SELCHAR)
			str << "SELEaaO DE PERSONAGEM";
		else
			str << "Outro (" << user->CurrentScore << ")";

		str << "\n";

		Log(user->clientId, LOG_INGAME, "O login %s tentou logar na conta com o mesmo mac.", p->Login);
	}

#if !defined(_DEBUG)
	if (total >= 10)
	{
		SendClientMessage(clientId, "Limite de 10 contas por computador");

		SendMessageA();
		return true;
	}
#endif

	memcpy(pUser[clientId].MacAddress, p->Mac, 8);

	// Caso exista aspas ou espaao no login, excluira
	for(INT32 i = 0; i < 16;i ++)
	{
		if(p->Login[i] == '\'')
			p->Login[i] = 0;

		if(p->Login[i] == ' ')
			p->Login[i] = 0;
	}

	_strupr_s(p->Login);
	strncpy_s(User.Username, p->Login, 16);

	NormalLog = std::make_unique<CLog>("..\\Logs\\Players", User.Username);
	HackLog = std::make_unique<CLog>("..\\Logs\\Hack", User.Username);

	Log(clientId, LOG_INGAME, "=========================== NOVA SESSaO ==========================\nTentativa de logar na conta - IP: %s Mac: (%02X:%02X:%02X:%02X:%02X:%02X). ClientId: %d", IP, (int)p->Mac[0], (int)p->Mac[1], (int)p->Mac[2],
		(int)p->Mac[3], (int)p->Mac[4], (int)p->Mac[5], clientId);

	if(!str.str().empty())
		Log(clientId, LOG_INGAME, str.str().c_str());

	// Checa se a senha ja foi errada mais de tras vezes
	// 00422D52
	INT32 checkFail = CheckFailAccount(p->Login); // local29
	if (checkFail >= 3)
	{
		SendClientMessage(clientId, g_pLanguageString[_NN_3_Tims_Wrong_Pass]);

		SendMessageA();
		return true;
	}
	
	// Envia o pacote a DBsrv
	p->Header.ClientId = clientId;
	p->Header.PacketId = 0x803;

	AddMessageDB((BYTE*)Header, sizeof p20D);

	CurrentScore = USER_LOGIN;
	pMob[clientId].Mode = 0;
	pMob[clientId].clientId = clientId;
	return true;
}