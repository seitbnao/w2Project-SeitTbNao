#ifndef __SHAREDSTRUCT_H__
#define __SHAREDSTRUCT_H__


#include <Windows.h>
#include <chrono>
#include <vector>
#include <string>
#include <iostream>
#include <array>
#include <sstream>
#include <ctime>
#include <cstdlib>

union STRUCT_BONUSEFFECT
{
	struct
	{
		unsigned char cEffect;
		unsigned char cValue;
	};
	short sValue;
};


struct STRUCT_ITEM
{
	WORD sIndex;
 
	STRUCT_BONUSEFFECT stEffect[3];

	std::string toString() const
	{
		std::stringstream str{};
		str << "[" << sIndex << "]" << " [";

		for (int i = 0; i < 3; i++)
		{
			str << std::to_string(stEffect[i].cEffect) << " " << std::to_string(stEffect[i].cValue);

			if (i != 2)
				str << " ";
		}
		str << "]";

		return str.str();
	}

	bool hasAdd(BYTE add, BYTE& value) const
	{
		for (int i = 0; i < 3; i++)
		{
			if (stEffect[i].cEffect == add)
			{
				value = stEffect[i].cValue;

				return true;
			}
		}

		return false;
	}
};

#define EF1 stEffect[0].cEffect
#define EFV1 stEffect[0].cValue
#define EF2 stEffect[1].cEffect
#define EFV2 stEffect[1].cValue
#define EF3 stEffect[2].cEffect
#define EFV3 stEffect[2].cValue



enum eClass
{
	Mortal = 1,
	Arch,
	Celestial,
	SubCelestial,
	UnknownClass = -1
};

// ------------------------------
// Structs
// ------------------------------
typedef struct
{
	WORD Size;
	BYTE Key;
	BYTE CheckSum;
	WORD PacketId;
	WORD ClientId;
	DWORD TimeStamp;
} PacketHeader;

typedef struct {
	PacketHeader Header;
	char Password[12];
	char Login[16];

    char Unknow[52];
	UINT32 CliVer;
	UINT32 Unknow_84;
	unsigned char Mac[8];
	char Keys[12];
} p20D;


struct STRUCT_POSITION
{
	WORD X, Y;

	friend std::ostream& operator<<(std::ostream& os, const STRUCT_POSITION& pos)
	{
		os << pos.X << "x" << pos.Y << "y";

		return os;
	}

	bool operator>=(const STRUCT_POSITION& rhs)
	{
		return X >= rhs.X && Y >= rhs.Y;
	}

	bool operator<=(const STRUCT_POSITION& rhs)
	{
		return X <= rhs.X && Y <= rhs.Y;
	}

	bool operator>(const STRUCT_POSITION& rhs)
	{
		return X > rhs.X && Y > rhs.Y;
	}

	bool operator<(const STRUCT_POSITION& rhs)
	{
		return X < rhs.X && Y < rhs.Y;
	}
};

struct STRUCT_SCORE
{
 
	DWORD Level;
	DWORD Ac;
	DWORD Damage;

	union
	{
		struct {
			BYTE Merchant : 4;
			BYTE Direction : 4;
		};
		BYTE Value;
	} Merchant;

	union {
		struct {
			BYTE Speed : 4;
			BYTE ChaosRate : 4;
		};
		BYTE Value;
	} Move;

	INT32 MaxHp, MaxMp;
	INT32 Hp, Mp;

	WORD Str, Int;
	WORD Dex, Con;

	WORD Special[4];
};

struct STRUCT_MOB
{
	char MobName[16]; // 0 - 15
	char CapeInfo; // 16 

	union
	{
		struct
		{
			BYTE Merchant : 6;
			BYTE CityID : 2;
		};
		BYTE Value;
	} Info; // 17

	WORD Guild; // 18 19
	BYTE Class; // 20

	union
	{
		struct
		{
			BYTE SlowMov : 1;
			BYTE DrainHP : 1;
			BYTE VisionDrop : 1;
			BYTE Evasion : 1;
			BYTE Snoop : 1;
			BYTE SpeedMov : 1;
			BYTE SkillDelay : 1;
			BYTE Resist : 1;
		};

		BYTE Value;
	} AffectInfo; // 21

	union
	{
		struct
		{
			WORD Mystical_GetQuest : 1; // 1
			WORD Mystical_CanGetAmuleto : 1; // 2
			WORD Mystical_GetAmuleto : 1; // 4
			WORD Reset50 : 1; // 8
			WORD Reset100 : 1; // 16
			WORD MestreHab : 1; // primeiro reset free? 32
			WORD Treinadores : 4; // 64 128 256 512
			WORD All : 6;
		};

		WORD Value;
	} QuestInfo;

	int Coin; // 24 - 27
	long long Exp; // 32 - 39


	STRUCT_POSITION Last; // 40 - 43
	STRUCT_SCORE BaseScore; // 44 - 91
	STRUCT_SCORE CurrentScore; // 92 - 139

	STRUCT_ITEM Equip[16]; // 140 - 267
	STRUCT_ITEM Inventory[64]; // 268 - 779

	unsigned int LearnedSkill[2]; // 780 - 783
	WORD ScoreBonus; // 788 - 789
	WORD SpecialBonus; // 790 - 791
	WORD SkillPoint; //792 - 793

	BYTE Critical; // 794
	BYTE SaveMana; // 795

	unsigned char ShortSkill[4]; // 796 - 799
	char GuildLevel; // 800

	BYTE MagicIncrement; // 801
	BYTE RegenHP; // 802 
	BYTE RegenMP; // 803
	union
	{
		char Resistence[4];
		struct
		{
			BYTE Fogo;
			BYTE Gelo;
			BYTE Sagrado;
			BYTE Trovao;
		} Resist; // 804 - 807

	};
	
	bool HasSoulSkill() const
	{
		return false;
	}
	eClass GetEvolution() const
	{
		int value = Equip[0].EFV2;
		if (value <= 0 || value >= 5)
			return eClass::UnknownClass;

		return (eClass)value;
	}
};

struct STRUCT_ITEMDATE
{
	BYTE Dia;
	BYTE Mes;
	WORD Ano;

	BYTE Hora;
	BYTE Minuto;
	BYTE Segundo;

	time_t GetTMStruct() const
	{
		struct std::tm b = { Segundo, Minuto, Hora, Dia, Mes - 1, Ano - 1900 };

		return std::mktime(&b);
	}

	auto GetDiffFromNow() const
	{
		time_t rawnow = time(NULL);
		struct tm now; localtime_s(&now, &rawnow);

		auto now_time_t = std::mktime(&now);
		auto diffTime = std::difftime(GetTMStruct(), now_time_t);
		if (diffTime <= 0.0)
			diffTime = -1.0;

		return diffTime;
	}

	std::string toString() const
	{
		std::stringstream str;
		str << Dia << "/" << Mes << "/" << Ano << " " << Hora << ":" << Minuto << ":" << Segundo;

		return str.str();
	}
};


union UNION_QUESTINFO
{
	struct
	{
		// Level feito o celestial
		INT64 Level355 : 1;
		INT64 Level370 : 1;
		INT64 Level380 : 1;
		INT64 Level398 : 1;
		INT64 Level399 : 1;


		// Cristais Arch
		INT64 Elime : 1;
		INT64 Sylphed : 1;
		INT64 Thelion : 1;
		INT64 Noas : 1;

		// Arcana
		INT64 Arcana : 1;

		// Adicionais SubCele
		INT64 Add120 : 1;
		INT64 Add151 : 1;
		INT64 Add180 : 1;
		INT64 Add199 : 1;

		// Resets Subcelestial
		INT64 Reset_1 : 1;
		INT64 Reset_2 : 1;
		INT64 Reset_3 : 1;
		INT64 Reset_4 : 1;

		// Treinadores
		INT64 Treinador_1 : 1;
		INT64 Treinador_2 : 1;
		INT64 Treinador_3 : 1;

		INT64 Unicornio : 1;
		INT64 MolarGargula : 1;

		INT64 Reset_50 : 1;
		INT64 Pilula : 1;

		INT64 Conj_Iniciante : 1;

		INT64 BemVindo : 1;
		INT64 Quest_1_de_5 : 1;
		INT64 Quest_2_de_5 : 1;
		INT64 Quest_3_de_5 : 1;
		INT64 Quest_4_de_5 : 1;
		INT64 Quest_5_de_5 : 1;

		INT64 LvBlocked : 1;
		INT64 BalanceQuest : 1;
		INT64 GodBless : 1;

		INT64 Unlock354 : 1;
		INT64 Unlock369 : 1;
		INT64 Unlock39 : 1;
		INT64 Unlock89 : 1;

		INT64 QuestRune_1 : 1;
		INT64 QuestRune_2 : 1;
		INT64 QuestRune_3 : 1;

		INT64 Unlock200 : 1;
		INT64 Unlock210 : 1;
		INT64 Unlock220 : 1;
		INT64 Unlock230 : 1;
	};
	INT64 Value;
};

struct STRUCT_AFFECT
{
	BYTE Index;
	BYTE Master;

	WORD Value;
	DWORD Time;
};

 
struct STRUCT_SUBINFO
{
	INT32 CurrentScore;

	STRUCT_SCORE SubStatus;
	STRUCT_ITEM Equip[2];

	unsigned char SkillBar[20];

	INT64 Exp;

	unsigned int LearnedSkill;
	unsigned int SecLearn;
	STRUCT_AFFECT Affect[32];

	UNION_QUESTINFO Info;

	UINT8 Soul;
};

struct STRUCT_CHARINFO
{
	STRUCT_MOB Player; // 0 - 807

	STRUCT_AFFECT Affects[32]; // 808 - 1063
	UNION_QUESTINFO Info; // 1064 - 1071

	UINT32 HallEnter; // Entradas para Kefra // 1072 - 1075
	UINT32 Fame; // Fama do personagem // 1076 - 1079

	INT8 PesaEnter; // Entrada Pesadelo // 1080

	UINT8 Citizen; // Cidadania // 1081
	UINT8 Soul; // Configuração da Soul // 1082

	long long Hold;
	unsigned char SkillBar[16];

	UINT32 MagicIncrement;
	INT32 MortalSlot;

	STRUCT_ITEMDATE Divina;
	STRUCT_ITEMDATE Sephira;
	STRUCT_ITEMDATE Saude;
	STRUCT_ITEMDATE Escritura;
	STRUCT_ITEMDATE Revigorante;

	STRUCT_SUBINFO Sub;

	// & 0x01 = Buff 18 -> 0x0040B0BA -> controle de mana
	// & 0x02 = Imunidade (skill HT)
	// & 0x04 = SlowMov
	// & 0x08 = poção ataque + samaritano 
	// & 0x10 = Buff 36 -> 0x0040BB0C -> veneno
	// & 0x20 = Buff 26 -> 0x0040B346 -> evasao
	// & 0x40 = Buff 28 -> 0x0040B393 -> invisibilidade
	// & 0x80 = SpeedMov;
	INT32 AffectInfo; // 1FDF34C (byte 1708)
	//DWORD Cash;

	STRUCT_POSITION Nightmare[3];


	STRUCT_ITEMDATE LastGuildKickOut;

	int RvRPoints;

	int GetTotalResets() const;
};

struct STRUCT_ACCOUNT
{
	char Username[16]; // Nome de usuario
	char Password[36]; // Senha da conta

	WORD Cash;

	WORD BanType;
	STRUCT_ITEMDATE Ban;
	char SecondPass[16]; // Segunda senha

	WORD AccessLevel;

	int Year;
	int YearDay;

	struct
	{
		DWORD Coin;
		STRUCT_ITEM Item[128];
	} Storage; // Banco do usuario

	STRUCT_CHARINFO Mob[4];

	struct
	{
		char Pass[16] = { 0 };
		WORD Blocked;
	} Block;

	// Estas sêo as insignias da conta e cada personagem tem uma estrutura prêpria
	union Insignias
	{
		struct
		{
			INT64 FirstDay : 1; // primeiro dia de servidor ficou online
			INT64 Novice : 1;
			INT64 All : 62;
		};

		INT64 Value;
	} Insignias;

	char TempKey[52] = { 0 };

	INT16 CharSlot;
	STRUCT_POSITION Position;

	struct
	{
		INT32 Day;
		INT32 Total;
	} Water;

	STRUCT_ITEMDATE Divina;
	STRUCT_ITEMDATE Sephira;



	bool AlreadyReseted;
};


struct STRUCT_STATICEFFECT
{
	short sEffect;
	short sValue;
};

 
struct STRUCT_ITEMLIST
{
	char ItemName[64];

	short IndexMesh;
	short nIndexTexture;
	short nIndexVisualEffect;
	short Level;
	short Str;
	short Int;
	short Dex;
	short Con;

	STRUCT_STATICEFFECT stEffect[12];

	int Price;
	short Unique;
	short Pos;
	short Extreme;
	short nGrade;
};

struct STRUCT_SKILLDATA
{
	int Points;
	int Target;
	int Mana;
	int Delay;
	int Range;
	int InstanceType; // Affect[0].cEffect
	int InstanceValue; // Affect[0].Value
	int TickType; // Affect[1].cEffect
	int TickValue; // Affect[1].Value
	int AffectType; // Affect[2].cEffect
	int AffectValue; // Affect[2].Value
	int Time;
	char Act[16];
	int InstanceAttribute;
	int TickAttribute;
	int Aggressive;
	int Maxtarget;
	int PartyCheck;
	int AffectResist;
	int Passive_Check;
	int ForceDamage;
};

struct STRUCT_TELEPORT
{
	INT32 Price;

	STRUCT_POSITION SrcPos;
	STRUCT_POSITION DestPos;
};

struct STRUCT_GUILDINFO
{
	std::string MobName;
	std::array<std::string, 3> SubGuild;

	// Reino das Guilds
	BYTE Kingdom;

	// Fame das guilds
	int Fame;

	// Cidadania da guilda
	int Citizen;

	INT32 Wins;
};

struct STRUCT_STOREDONATE
{
	UINT32 Price;
	INT32 Loop;
	INT32 Avaible;

	STRUCT_ITEM item;
};

struct STRUCT_SEALINFO
{
	INT32 CurrentScore; //B5C - 0 - 3
	INT16 Face; //B60 -  4 - 5
	INT16 Level; // B62 - 6 - 7
	INT16 Str; // B64 - 8 - 9
	INT16 Int; // B66 - 10 - 11
	INT16 Dex; // B68 - 12 - 13
	INT16 Con; // B6A - 14 - 15
	INT16 Evolution; // B6C
	INT16 CapeId; // B6E
	INT16 Unk_3; // B70
	INT16 Unk_4;
	INT16 Skills[9];
	INT16 QuestInfo; // 01 = 1 quest , 0x11 = 2 quests, 0x111 = 3 quests, 0x1111 = 4quests
};

struct STRUCT_SEALFILE
{
	STRUCT_SEALINFO Seal;
	STRUCT_CHARINFO Mob;
};

struct p114
{
	PacketHeader Header; // 0 - 11
	STRUCT_POSITION WorldPos; // 12 - 15
	STRUCT_MOB Mob; // 16 - 823
	unsigned short CurrentKill;
	unsigned short TotalKill;
	short SlotIndex; // 1040 - 1041
	short ClientIndex; // 1042 - 1043
	short Weather; // 1044 - 1045
	char SkillBar2[16]; // 1046 - 1061
	STRUCT_AFFECT Affect[32];
};


// Spawn Info
struct p364
{
	PacketHeader Header; 
	struct
	{
		short X, Y;
	} Current; 
	short sIndex; 
	char MobName[12]; 
	unsigned char ChaosPoints; 
	unsigned char CurrentKill; 
	unsigned short TotalKill; 
	WORD Item_Refine[16];
	struct
	{
		BYTE Time;
		BYTE Index;
	} Affect[32];
	WORD Guild; 
	char GuildLevel;
	STRUCT_SCORE CurrentScore;
	struct
	{
		WORD Type;
	} Spawn;
	char pAnctCode[16];
	char pTab[26];
	short Life;
	short Server;
};
struct p363 : p364
{
	char StoreName[27];
};







#pragma pack(push, 1)
struct p336
{
	PacketHeader Header;
	STRUCT_SCORE Score;
	char Critical;
	char SaveMana;
	unsigned short Affect[32];
	unsigned short Guild;
	unsigned short GuildLevel;
	char Resist[4];
	int ReqHp;
	int ReqMp;
	unsigned short Magic;
	unsigned short Rsv;
	char LearnedSkill;
};
#pragma pack(pop)

typedef struct
{
	PacketHeader Header;
	DWORD Hold;
	UINT64 Exp;
	UINT32 LearnedSkill[2];
	WORD pStatus;
	WORD pMaster;
	WORD pSkills;
	BYTE Magic; 
	DWORD Coin;
} p337;
typedef struct
{
	PacketHeader Header;

	unsigned short ItemEff[16];
	char pAnctCode[16];
} p36B;

typedef struct {
	PacketHeader Header;

	INT32 CurHP;
	INT16 Status;
}  p292;


typedef struct
{
	PacketHeader Header;

	INT32 Hp;
	INT32 Mp;
	INT32 MaxHp;
	INT32 MaxMp;
} p181;

typedef struct
{
	PacketHeader Header;
	short BonusType;
	short Detail;
	unsigned short TargetID;
}p277;


typedef struct
{
	PacketHeader Header; // 0 - 11
	short PosX;
	short PosY;
	UINT32 stEffect; // 16 - 19
	UINT32 Speed;  // 20 - 23
	char Route[24];  // 23 - 47
	unsigned short TargetX;
	unsigned short TargetY;
} p36C;
#endif //__SHAREDSTRUCT_H__