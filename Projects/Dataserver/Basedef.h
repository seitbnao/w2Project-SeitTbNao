
#ifndef __BASEDEF_H__
#define __BASEDEF_H__

#include <Windows.h>
#include <fstream>
#include <SharedStructs.h>
#include "CUser.h"
 
#include "Keytable.h"
#include "pugixml.hpp"
 
// Adiciona a library da winsock
#pragma comment (lib, "WS2_32.lib")

#define MAX_SERVER			2
#define MAX_SERVERGROUP		10
#define MAX_GUILD 65535
#define MAX_SERVERNUMBER	(MAX_SERVER + 3)
#define	MAX_ADMIN				10 
#define MAX_PLAYER			1000
#define MAX_DBACCOUNT		(MAX_PLAYER * MAX_SERVER)
#define		ACCOUNTNAME_LENGTH		16		// Max length of the account login
#define		ACCOUNTPASS_LENGTH		12      // Max length of the account password
#define TIMER_SEC 0

struct STRUCT_GUI_INFO
{
	HWND hGUI,
	hButton[1],
	hLabel[1];

	HFONT Font;
	ATOM Class;
};

// Estrutura do banco de dados
struct STRUCT_SERVER_INFO
{
	struct
	{
		UINT32 Porta;
		char IP[32];
	} Config;

	INT32 SecCounter;
	INT32 MinCounter;
	INT32 HourCounter;

	INT32 CurrentTime;
	INT32 LastSendTime;

	UINT32 hServer;
	SOCKADDR_IN Service;

	INT32 ServerIndex;

	bool CurrentScore;

	// Safiras do servidor
	UINT32 Sapphire;

	// Primeiro canal que matou Kefra
	UINT32 FirstKefra;
	char   KefraKiller[16];

	UINT32 Staff; //
	UINT32 LastGuild;

	INT32 LastSeal{ 1 };
} ;

// Estrutura da guerra de torres
typedef struct
{
	INT16 TowerState; // Indica qual canal esta avançando neste momento
	INT16 WarState;   // Indica se a guerra esta declarada ou recusada
	INT16 TMP;        // Guarda o valor antigo
} stTowerWar;

static int dias_mes[13] = {0, 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
extern INT32 ChargedGuildList[MAX_SERVERGROUP][5];
extern STRUCT_SERVER_INFO sServer;
extern STRUCT_GUI_INFO GUI;
extern STRUCT_MOB pBaseSet[4];
extern INT16 g_pGuildWar[MAX_GUILD];
extern STRUCT_GUILDINFO g_pGuild[MAX_GUILD];
extern INT32 g_pGuildPoint [MAX_GUILD];
extern INT16 g_pGuildAlly[MAX_GUILD];
extern char	g_pServerList[MAX_SERVERGROUP][MAX_SERVERNUMBER][64];
extern stTowerWar g_pTowerWarState[MAX_SERVERGROUP];
extern STRUCT_STOREDONATE g_pStore[10][27];

INT32 InitializeBaseDef();

INT32 ReadNPCDonate();
INT32 WriteNPCDonate();

void Log(char *username, const char *msg, ...);

INT32 WriteConfig();
INT32 WriteGuilds();
INT32 ReadGuilds();
INT32 ReadConfig();

INT32 DecideWinnerTowerWar();

void SetFame(INT32 guildId, INT32 value);

BOOL SendTowerWarInfo(BYTE Info);
BOOL ReadCharBase(INT32 index, const char *fileName);
BOOL InitializeServerList();
int Rand();

void ClearItem(STRUCT_ITEM *item);
void ClearMob(STRUCT_MOB *player);
INT32 GetUserFromSocket(INT32 soc);

void GetFirstKey(const char *source, char *dest);


void AppendStructure(pugi::xml_node mob, STRUCT_CHARINFO* charInfo);
void AppendStructure(pugi::xml_node mob, STRUCT_MOB* mobInfo);
void AppendStructure(pugi::xml_node mob, STRUCT_SCORE* status);
void AppendStructure(pugi::xml_node node, const STRUCT_AFFECT* affect);
void AppendStructure(pugi::xml_node node, const STRUCT_ITEMDATE* date);
void AppendStructure(pugi::xml_node node, const STRUCT_POSITION* position);
void AppendStructure(pugi::xml_node node, const STRUCT_ITEM* item);
void AppendStructure(pugi::xml_node account, STRUCT_ACCOUNT* acc);

void XMLToStructure(pugi::xml_node node, STRUCT_CHARINFO& charInfo);
void XMLToStructure(pugi::xml_node node, STRUCT_SUBINFO& sub);
void XMLToStructure(pugi::xml_node node, STRUCT_ITEMDATE& date);
void XMLToStructure(pugi::xml_node node, STRUCT_POSITION& position);
void XMLToStructure(pugi::xml_node node, STRUCT_SCORE& status);
void XMLToStructure(pugi::xml_node node, STRUCT_ITEM& item);
void XMLToStructure(pugi::xml_node node, STRUCT_AFFECT& affect);
void XMLToStructure(pugi::xml_node accNode, STRUCT_ACCOUNT* file);

#define FLAG_GAME2CLIENT 0x0100
#define FLAG_CLIENT2GAME 0x0200
#define FLAG_DB2GAME     0x0400
#define FLAG_GAME2DB	 0x0800



#define _MSG_MessagePanel                 ( 1 | FLAG_GAME2CLIENT)
#define _MSG_DBNewCharacter         ( 2 | FLAG_GAME2DB)
#define _MSG_DBAccountLogin			( 3 | FLAG_GAME2DB) // login
#define _MSG_DBCharacterLogin       ( 4 | FLAG_GAME2DB)
#define _MSG_DBCNFCharacterLogin	0x417
#define _MSG_DBCNFCharacterLoginExtra 0x820
#define _MSG_DBNoNeedSave           ( 5 | FLAG_GAME2DB) 
#define _MSG_DBUpdateSapphire		0x80E
#define _MSG_DBStaffMode			0x80F
#define _MSG_DBSaveMobQuit           ( 6 | FLAG_GAME2DB) 
#define _MSG_DBSaveMob              ( 7 | FLAG_GAME2DB)
#define	_MSG_DBDeleteCharacter		( 9 | FLAG_GAME2DB)
#define _MSG_STARTTOWERWAR          ( 15 | FLAG_DB2GAME)
#define _MSG_DBCNFNewCharacter			0x418
#define _MSG_DBCNFAccountLogin			0x416
#define _MSG_DBCNFDeleteCharacter		0x419
#define _MSG_DBReloadDonateList			0x80C
#define _MSG_DBBuyStoreSaveList			0x380
#define _MSG_DBBroadcastStoreBuy		0x381
#define _MSG_DBBroadcastSetAvaible		0x382
#define _MSG_DBBroadcastChannelChat		0xD1D
#define _MSG_DBRequestNumericPassword	0xFDE
#define _MSG_DBCNFRequestNumericPass    0xFDE
#define _MSG_DBRequestCreateSubCele     0x830
struct MSG_MessagePanel
{        
	PacketHeader Header;
	int nID;
	char String[92];  
};

#define _MSG_DBSavingQuit         ( 10 | FLAG_DB2GAME)
struct  MSG_DBSavingQuit 
{       
	PacketHeader Header; // 0 -11
	char AccountName[16]; // 12 - 27
	int  Mode; // 28 - 31
};

struct		 MSG_DBCNFServerChange
{
	PacketHeader Header;

	char AccountName[ACCOUNTNAME_LENGTH];
	char Enc[52];
};

struct		 MSG_DBServerChange
{
	PacketHeader Header;

	int NewServerID;
	int Slot;
};

#define _MSG_DBNewArch         ( 11 | FLAG_GAME2DB)
typedef struct
{
	PacketHeader Header;

	INT32 PosID;
	char MobName[16];
	INT32 ClassID;
	INT32 Class;
	INT32 MortalSlot;
} MSG_DBNewArch;

#define _MSG_GuildZoneReport		( 13 | FLAG_GAME2DB)
#define _MSG_DBCNFChargedList       0x428
struct	MSG_GuildZoneReport
{
	PacketHeader Header;
	INT32 Guild[5];
};

struct MSG_ChargedGuildList
{
	PacketHeader Header;

	INT32 ChargedGuildList[MAX_SERVERGROUP][5];
};

struct  MSG_STANDARDPARM1
{  	    PacketHeader Header;
        int Parm1;
};

struct  MSG_STANDARDPARM2
{  	    PacketHeader Header;
        int Parm1;
		int Parm2;
};

struct  MSG_STANDARDPARM3
{  	    PacketHeader Header;
        int Parm1;
		int Parm2;
		int Parm3;
};




struct st_CharList
{
	INT16 PositionX[4];
	INT16 PositionY[4];

	char MobName[4][16];

	STRUCT_SCORE CurrentScore[4];
	STRUCT_ITEM Equip[4][16];

	UINT16 GuildId[4];

	int Coin[4];
	INT64 Exp[4];
}; // 744

struct p213 {
	PacketHeader Header;
	int CharIndex;
	char Zero[18];
};

struct p416 
{
	PacketHeader Header; // 0 - 11 

	BYTE Keys[16];

	INT32 Unkw;

	st_CharList CharList; // 12 - 755
	
	STRUCT_ITEM Storage[128]; // 756 - 1779 
	UINT32 GoldStorage; // 1880 - 1883

	char UserName[16]; // 1884 - 1900 
	BYTE Unknow[12];
};

struct p418
{
	PacketHeader Header; // 0 - 11 
	st_CharList CharList; // 12 - 755
};

struct p20F
{
	PacketHeader Header;
	UINT32 SlotID;
	char Nick[16];
	UINT32 ClassID;
};

 

typedef struct
{
    PacketHeader Header;
    int SlotIndex;
    char MobName[16];
    char Pwd[12];
} p211; 

typedef struct
{
	PacketHeader Header; // 0 - 11

	UINT32 CharSlot; // 12 - 15
	STRUCT_CHARINFO Mob; // 16 - 771
	STRUCT_ITEM Storage[128]; // 772 - 1795
	INT32 Coin; // 1796 - 1799
	char SkillBar[20]; // 1800 - 1807
	char User[16]; // 1808 - 1823
	UINT32 Arg2; // 1824 - 1827
	char Pass[16];
	INT32 Blocked;
	INT32 Cash;
	INT64 Insignia;
	UINT32 BanType;
	STRUCT_ITEMDATE Ban;

	STRUCT_POSITION Position;
	INT16 Slot;
	INT64 Unique;
 
	
	struct
	{
		INT32 Day;
		INT32 Total;
	} Water;

	STRUCT_ITEMDATE Divina;
	STRUCT_ITEMDATE Sephira;

	UINT64 SingleGift;
} p807;

typedef struct 
{
	PacketHeader Header;

	INT32 Val;
	char Username[16];
	STRUCT_ITEM item;
} pCOF;

typedef struct
{
	PacketHeader Header;

	INT32 Val;
	char Username[16];
	INT32 Cash;
} pCOE;

typedef struct
{
	PacketHeader Header;

	char Username[16];
	INT32 BanType;
	STRUCT_ITEMDATE Ban;
} pC10;

typedef struct {
	PacketHeader Header;

	INT32 Value;
} pMsgSignal;

typedef struct {
	PacketHeader Header;

	INT32 Value;
	INT32 Value2;
} pMsgSignal2;

typedef struct {
	PacketHeader Header;

	INT32 Value;
	INT32 Value2;
	INT32 Value3;
} pMsgSignal3;

#define _MSG_DBGuildAlly 0xE12
#define _MSG_DBGuildWar 0xE0E
typedef struct {
	PacketHeader Header;
	DWORD GuildIndex1;
	DWORD GuildIndex2;
} pE12;

typedef struct 
{
	PacketHeader Header;
	char num[16];

	int RequestChange;
} pFDE; 

typedef struct
{
	PacketHeader Header;

	STRUCT_CHARINFO Mob;
	INT32 Cash;
	char Pass[16];
	INT32 Blocked;
} p820;


typedef struct
{ 
	PacketHeader Header;

	INT32 Access;
	INT64 Unique;
 

	struct
	{
		INT32 Day;
		INT32 Total;
	} Water;

	STRUCT_ITEMDATE Divina;
	STRUCT_ITEMDATE Sephira;

 

	STRUCT_ITEMDATE Ban;
	UINT32 BanType;

	INT32 IsBanned;
	INT32 Cash;
} p415;

typedef struct
{ 
	PacketHeader Header;

	UINT32 Level;
	UINT32 Class;
	UINT32 LearnedSkill;
	UINT32 Mantle;
	UINT32 Face;
	UINT32 CharPos;

	char MobName[16];
} p830;

typedef struct
{
	PacketHeader Header;
	
	UINT32 Class;
	UINT32 LearnedSkill;
	UINT64 Exp;

	STRUCT_ITEMDATE Escritura;
	UNION_QUESTINFO Info;

	STRUCT_ITEM Item[2];
	STRUCT_SCORE CurrentScore;
}p432;

struct MSG_STARTTOWERWAR
{
	PacketHeader Header;

	BYTE isStarting;

	stTowerWar war[10];
};

typedef struct
{
	PacketHeader Header;
	char eMsg[96];
} p101;
// NPKO

const int FLAG_DB2NP = 0x1000;
const int FLAG_NP2DB = 0x2000;

const short  _MSG_NPReqIDPASS = (1 | FLAG_DB2NP);
const short  _MSG_NPIDPASS = (2 | FLAG_NP2DB);
struct		  MSG_NPIDPASS
{
	PacketHeader Header;
	char Account[16];
	int  Encode1;
	char Pass[12];
	int  Encode2;
};

const short  _MSG_NPReqAccount = (3 | FLAG_NP2DB);
struct		  MSG_NPReqAccount
{
	PacketHeader Header;
	char Account[16];
	char Char[16];
};

const short  _MSG_NPNotFound = (4 | FLAG_DB2NP); //   Signal

const short  _MSG_NPAccountInfo = (5 | FLAG_DB2NP);
struct		  MSG_NPAccountInfo
{
	PacketHeader Header;
	STRUCT_ACCOUNT account;
	short Session;
	short State;  // 0:Normal  1:Blocked= (@);  2:Deleted= (_);  3:Disabled= (#);
};

const short  _MSG_NPReqSaveAccount = (6 | FLAG_NP2DB); //   MSG_NPAccountInfo

const short  _MSG_NPDisable = (7 | FLAG_NP2DB | FLAG_DB2NP);

const short  _MSG_NPEnable = (8 | FLAG_NP2DB | FLAG_DB2NP); //   NPEnableParm 
struct		  MSG_NPEnable
{
	PacketHeader Header;
	char AccountName[16];
	int Year;
	int YearDay;
};

const short  _MSG_NPNotice = (9 | FLAG_NP2DB | FLAG_DB2NP | FLAG_DB2GAME); //   Parm 
struct		  MSG_NPNotice
{
	PacketHeader Header;
	int  Parm1;
	int  Parm2;
	char AccountName[16];
	char String[96];
};

const short  _MSG_NPState = (10 | FLAG_NP2DB | FLAG_DB2NP); //   Parm 

const short _MSG_NPCreateCharacter = (11 | FLAG_NP2DB | FLAG_DB2NP);
struct		 MSG_NPCreateCharacter
{
	PacketHeader Header;
	int  Slot;
	char Account[16];
	STRUCT_MOB Mob;
};

const short _MSG_NPCreateCharacter_Reply = (12 | FLAG_DB2NP | FLAG_NP2DB);
struct		 MSG_NPCreateCharacter_Reply
{
	PacketHeader Header;
	int  Slot;
	char Account[16];
	int  Result;
	char MobName[16];
};

const short  _MSG_NPDonate = (13 | FLAG_NP2DB | FLAG_DB2NP);
struct		  MSG_NPDonate
{
	PacketHeader Header;
	char AccountName[16];
	int Donate;
};

const short _MSG_NPAppeal = (16 | FLAG_DB2GAME | FLAG_GAME2DB | FLAG_DB2NP);

#define MSG_CREATEGUILD_OPCODE 0x310
struct MSG_CREATEGUILD
{ 
	PacketHeader Header;
	char GuildName[16];

	int kingDom;
	int citizen;
	int guildId;
};

#define MSG_ADDGUILD_OPCODE 0x313
struct MSG_ADDGUILD
{ 
	PacketHeader Header;
	int Type;
	int Value;
	int guildIndex;
};

#define MSG_ADDSUB_OPCODE 0x315
struct MSG_ADDSUB
{ 
	PacketHeader Header;

	INT16 Guild;
	INT16 SubIndex;
	char MobName[16];
	INT16 CurrentScore;
};


#define MSG_UPDATETOWERINFO 0x999
struct _MSG_UPDATETOWERINFO
{
	PacketHeader Header;

	int  KillerId; // Guild que matou a torre
};

#define MSG_UPDATEWARDECLARATION 0x998
struct _MSG_UPDATEWARDECLARATION
{
	PacketHeader Header;

	BYTE newInfo;
};

struct _MSG_UPDATEWARANSWER
{
	PacketHeader Header;

	BYTE action,
		 declarant,
		 receiver;
};

#define MSG_RESULTWARTOWER_OPCODE 0x997
typedef struct
{
	PacketHeader Header;

	INT32 Winner;
} _MSG_RESULTWARTOWER;

#define MSG_REWARDWARTOWER_OPCODE 0x996
typedef struct
{
	PacketHeader Header;

	INT32 Server;
	INT64 Coin;
	INT32 Taxe;
} _MSG_REWARDWARTOWER;


#define MSG_SEND_SERVER_NOTICE 0x908
typedef struct
{
	PacketHeader Header;

	char Notice[96];
} _MSG_SEND_SERVER_NOTICE;

#define MSG_NOTIFY_KEFRA_DEATH 0x909
typedef struct
{
	PacketHeader Header;

	char MobName[16];
} _MSG_NOTIFY_KEFRA_DEATH;

#define MSG_FIRST_KEFRA_NOTIFY 0x90A
#define MSG_REBORN_KEFRA 0x90B
typedef struct
{
	PacketHeader Header;

	int Channel;
} _MSG_FIRST_KEFRA_NOTIFY;

#define MSG_PANELGUILD_GETLIST 0x766
typedef struct 
{
	PacketHeader Header;

	STRUCT_GUILDINFO Guild[40];
} _MSG_PANELGUILD_GETLIST;

constexpr auto OutSealPacket = 3132;
constexpr auto PutInSealPacket = 3133;
constexpr auto SealInfoPacket = 3134;
constexpr auto PutInSealSuccess = 3135;

struct MSG_PUTOUTSEAL
{
	PacketHeader Header;
	int      SrcType;
	int      SrcSlot;
	int      DstType;
	int      DstSlot;
	unsigned short GridX, GridY;
	unsigned short WarpID;

	char MobName[16];
};

// PAcket 0x2CD  - Size 16 vem o pedido de colocar o mouse no item
struct MSG_SEALINFO
{
	PacketHeader Header;
	STRUCT_SEALINFO Info;
};

class CLog;

constexpr int LogPacket = 0x8580;
#endif