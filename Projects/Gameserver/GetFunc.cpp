#include "cServer.h"
#include "Basedef.h"
#include "GetFunc.h"
#include "SendFunc.h"
#include <array>
 

int	g_pDistanceTable[8][8]=
{	0,1,2,3,4,5,6,7,
	1,1,2,3,4,5,6,7,
	2,2,3,4,4,5,6,7,
	3,3,4,4,5,5,6,7,
	4,4,4,5,5,5,6,7,
	5,5,5,5,5,6,6,7,
	6,6,6,6,6,6,7,7,
	7,7,7,7,7,7,7,7
};

unsigned int g_pArenaArea[5][4] = 
{
	{2052, 2171, 2052, 2163},
	{2432, 2675, 1672, 1767},
	{2448, 2476, 1966, 2024},
	{1070, 1110, 1130, 1158},
	{1036, 1072, 1700, 1760}
};

void GetCreateMob(int clientId, BYTE *bufPak)
{
	memset(bufPak, 0x00, sizeof p364);

	CMob *mob = (CMob*)&pMob[clientId];

	p364 *p = (p364*)(bufPak);

	p->Header.ClientId = 0x7530;
	p->Header.PacketId = 0x364;
	p->Header.Size = sizeof p364;

	memcpy(&p->CurrentScore, &mob->Mobs.Player.CurrentScore, sizeof STRUCT_SCORE);

	int len = strlen(mob->Mobs.Player.MobName);
	if(clientId >= MAX_PLAYER)
	{
		if(len >= 16)
			len = 15;

		p364 *packetMob = reinterpret_cast<p364*>(bufPak);
		strncpy_s(packetMob->MobName, 16, mob->Mobs.Player.MobName, 16);

		for(INT32 i = len; i > 0; i--)
		{
			if(packetMob->MobName[i] != ' ' && packetMob->MobName[i] != '_' && packetMob->MobName[i] != 0)
				break;

			packetMob->MobName[i] = 0;
		}

		packetMob->MobName[len] = 0;

		if(pMob[clientId].Mobs.Player.CapeInfo == 4)
			packetMob->CurrentScore.Ac = 0;
		else
			packetMob->CurrentScore.Ac = 1;
	}
	else
	{
		if (len >= 12)
		{
			p364* packetMob = reinterpret_cast<p364*>(bufPak);

			strncpy_s(packetMob->MobName, 16, mob->Mobs.Player.MobName, 16);
		}
		else
			strncpy_s(p->MobName, 12, mob->Mobs.Player.MobName, 16);
	}
	p->Spawn.Type = mob->SpawnType;
	
	if(pMob[clientId].Motion != 0)
		p->Spawn.Type |= (pMob[clientId].Motion << 4);

	p->sIndex = clientId;

	p->Current.X = pMob[clientId].Target.X;
	p->Current.Y = pMob[clientId].Target.Y;
	p->Header.TimeStamp = CurrentTime;

	bool isUsingCostume = pMob[clientId].Mobs.Player.Equip[12].sIndex != 0;
	for(int i = 0; i < 16; i++)
	{
		short effValue = 0;
		STRUCT_ITEM eqItem = mob->Mobs.Player.Equip[i];

		if(i == 14)
		{
			if(eqItem.sIndex >= 2360 && eqItem.sIndex <= 2390)
			{
				if(*(INT16*)&eqItem.stEffect[0].cEffect <= 0)
				{
					p->Item_Refine[i] = 0;
					p->pAnctCode[i] = 0;
					
					continue;
				}
			}
		}

		if (i == 0)
		{
			// se tiver um traje equipado
			if (pMob[clientId].Mobs.Player.Equip[12].sIndex != 0 && pMob[clientId].Mobs.Player.Class == 2)
				eqItem.sIndex = eqItem.EF2;
		}

		p->Item_Refine[i] = GetItemIDAndEffect(&eqItem, i, isUsingCostume);
		p->pAnctCode[i] = GetAnctCode(&eqItem, isUsingCostume);
	}

	for(int i = 0 ; i < 32; i++)
	{
		if(mob->Mobs.Affects[i].Index == 0 || mob->Mobs.Affects[i].Time <= 0)
		{
			p->Affect[i].Index = 0;
			p->Affect[i].Time = 0;
			continue;
		}

		p->Affect[i].Index = mob->Mobs.Affects[i].Index & 255;
		p->Affect[i].Time = mob->Mobs.Affects[i].Time & 255;
	}
	
	p->Guild = mob->Mobs.Player.Guild;
	p->GuildLevel = mob->Mobs.Player.GuildLevel;

	if(pMob[clientId].GuildDisable == 1)
	{
		p->GuildLevel = 0;
		p->Guild = 0;
	}

	if (mob->Mobs.Player.GuildLevel == 9)
		p->Spawn.Type |= 0x80;

	else if (mob->Mobs.Player.GuildLevel >= 6)
		p->Spawn.Type |= 0x40;

	if(clientId < MAX_PLAYER)
	{
		p->ChaosPoints = GetPKPoint(clientId);
		p->CurrentKill = GetCurKill(clientId);
		p->TotalKill = GetTotKill(clientId);

		if(GetGuilty(clientId) > 0)
			p->ChaosPoints = 0;

		if(pMob[clientId].Mobs.Player.Info.Merchant & 1)
			p->CurrentScore.Merchant.Merchant = 1;

		p->Life = pMob[clientId].Lifes;
	}
	
#if !defined(_DEBUG)
	if((pMob[clientId].Mobs.Player.Status.Level >= 69 && pMob[clientId].Mobs.Player.Equip[0].EFV2 == MORTAL) || pMob[clientId].Mobs.Player.Equip[0].EFV2 >= ARCH)
		strncpy_s(p->pTab, mob->Tab, 26);
#else
	sprintf_s(p->pTab, "index = %d", clientId);
#endif

	if(pMob[clientId].Target.X >= 2604 && pMob[clientId].Target.Y >= 1708 && pMob[clientId].Target.X <= 2648 && pMob[clientId].Target.Y <= 1744)
	{
		p->Guild = 0;
		p->GuildLevel = 0;

		p->Item_Refine[15] = 0;
		p->pAnctCode[15]   = 0;
	}
}

void GetCreateMobTrade(int clientId, BYTE *bufPak)
{
	memset(bufPak, 0x00, sizeof p363);

	CMob *mob = (CMob*)&pMob[clientId];
	p363 *p = (p363*)(bufPak);

	p->Header.Size = sizeof p363;
	p->Header.ClientId = 0x7530;
	p->Header.PacketId = 0x363;

	memcpy(&p->CurrentScore, &mob->Mobs.Player.CurrentScore, sizeof STRUCT_SCORE);
	
	for (int i = 0; i < 12; i++)
		p->MobName[i] = mob->Mobs.Player.MobName[i];

	p->sIndex = clientId;
	
	p->Spawn.Type = mob->SpawnType;
	
	p->Guild = mob->Mobs.Player.Guild;
	p->GuildLevel = mob->Mobs.Player.GuildLevel;

	p->Current.X = mob->Target.X;
	p->Current.Y = mob->Target.Y;

	bool isUsingCostume = pMob[clientId].Mobs.Player.Equip[12].sIndex != 0;
	for(int i = 0;i < 16; i++)
	{
		short effValue = 0;
		STRUCT_ITEM eqItem{ mob->Mobs.Player.Equip[i] };

		if(i == 14)
			if(eqItem.sIndex >= 2360 && eqItem.sIndex <= 2390)
				if(*(short*)&eqItem.stEffect[0].cEffect <= 0)
					eqItem.sIndex = 0;
		
		p->Item_Refine[i] = GetItemIDAndEffect(&eqItem, i, isUsingCostume);
		p->pAnctCode[i] = GetAnctCode(&eqItem, isUsingCostume);
	}

	for(int i = 0 ; i < 32; i++)
	{
		if(mob->Mobs.Affects[i].Index == 0 || mob->Mobs.Affects[i].Time <= 0)
		{
			p->Affect[i].Index = 0;
			p->Affect[i].Time = 0;
			continue;
		}
		
		p->Affect[i].Index = mob->Mobs.Affects[i].Index;
		p->Affect[i].Time = mob->Mobs.Affects[i].Index & 255;
	}

	p->ChaosPoints = GetPKPoint(clientId);
	p->CurrentKill = GetCurKill(clientId);
	p->TotalKill = GetTotKill(clientId);

	if (GetGuilty(clientId) > 0)
		p->ChaosPoints = 0;

	if (pMob[clientId].Mobs.Player.Info.Merchant & 1)
		p->CurrentScore.Merchant.Merchant = 1;
	
	strncpy_s(p->pTab, mob->Tab, 26);
	strncpy_s(p->StoreName, pUser[clientId].AutoTradeName, 24);
}

int GetAnctCode(STRUCT_ITEM *item, bool usingCostume)
{
	int value = 0;

	if(item->sIndex >= 2360 && item->sIndex < 2389)
	{
		if(item->EFV3 >= 11)
			return item->EFV3;

		return 0;
	}
    if(item->EF1 == 43)
		value = item->EFV1;
    else if(item->EF2 == 43)
		value = item->EFV2;
    else if(item->EF3 == 43)
		value = item->EFV3;
	else if (item->EF1 >= 116 && item->EF1 <= 125)
		return usingCostume ? item->EFV1 : item->EF1;
	else if(item->EF2 >= 116 && item->EF2 <=125)
		return usingCostume ? item->EFV2 : item->EF2;
	else if(item->EF3 >= 116 && item->EF3 <=125)
		return usingCostume ? item->EFV3 : item->EF3;

    if(value == 0)
		return 0;
    if(value < 230)
		return 43;

    switch(value % 4)
    {
        case 0:  
			return 0x30;
        case 1:  
			return 0x40;
        case 2:  
			return 0x10;
        case 3:  
			return 0x20;
		default: 
			return 0x20;
    }
}

int GetItemIDAndEffect(STRUCT_ITEM* Item, int mnt, bool usingCostume)
{
	int value;
	bool colored = false;

	if(mnt == 14)
	{
		return Item->sIndex | (Item->EF2 / 10 * 4096);
	}
	else
	{
		if (Item->EF1 >= 116 && Item->EF1 <=  125)
		{
			if (!usingCostume)
			{
				value = Item->EF1;
				colored = true;
			}
			else
				value = Item->EFV1;
		}
		else if (Item->EF2 >= 116 && Item->EF2 <=  125)
		{
			if (!usingCostume)
			{
				value = Item->EF2;
				colored = true;
			}
			else
				value = Item->EFV2;
		}
		else if (Item->EF3 >= 116 && Item->EF3 <=  125)
        {
			if (!usingCostume)
			{
				value = Item->EF3;
				colored = true;
			}
			else
				value = Item->EFV3;
		}
		else if (Item->EF1 == 43)
			value = Item->EFV1;
		else if (Item->EF2 == 43)
			value = Item->EFV2;
		else if (Item->EF3 == 43)
			value = Item->EFV3;
		else
			return Item->sIndex;
	}

	if(value > 9 && mnt != 14 && !colored)
	{
		if (value < 234)
			value = 10;
		else if (value < 238)
			value = 11;
		else if (value < 242)
			value = 12;
		else if (value < 246)
			value = 13;
		else if (value < 250)
			value = 14;
		else if (value < 254)
			value = 15;
		else
			value = 16;
    }
	else if(colored)
		if(value >= 9) 
			value = 9;
	
	return Item->sIndex | (value * 0x1000);
}
bool GetEmptyMobGrid(int sIndex, unsigned int *posX, unsigned int *posY)
{
	if (*posX < 0 || *posX >= 4096 || *posY < 0 || *posY >= 4096)
		return false;

	if (g_pMobGrid[*posY][*posX] == sIndex)
		return true;
	
	int LOCAL1 = g_pMobGrid[*posY][*posX];

	if (LOCAL1 < 0 || LOCAL1 >= 30000)
	{
		g_pMobGrid[*posY][*posX] = 0;
		LOCAL1 = 0;
	}

	if (pMob[LOCAL1].Mode == 0)
	{
		g_pMobGrid[*posY][*posX] = 0;
		LOCAL1 = 0;
	}

	if (LOCAL1 == 0)
	{
		if (g_pHeightGrid[*posY][*posX] != 127)
			return true;
	}

	for (unsigned int nY = (*posY - 1); nY <= (*posY + 1); nY++)
	{
		for (unsigned int nX = (*posX - 1); nX <= (*posX + 1); nX++)
		{
			if (nX >= 4096 || nY >= 4096)
				continue;

			if (g_pMobGrid[nY][nX] == 0)
			{
				if (g_pHeightGrid[nY][nX] != 127)
				{
					*posX = nX;
					*posY = nY;
					return true;
				}
			}
		}
	}

	for (unsigned int nY = (*posY - 2); nY <= (*posY + 2); nY++)
	{
		for (unsigned int nX = (*posX - 2); nX <= (*posX + 2); nX++)
		{
			if (nX >= 4096 || nY >= 4096)
				continue;

			if (g_pMobGrid[nY][nX] == 0)
			{
				if (g_pHeightGrid[nY][nX] != 127)
				{
					*posX = nX;
					*posY = nY;
					return true;
				}
			}
		}
	}

	for (unsigned int nY = (*posY - 3); nY <= (*posY + 3); nY++)
	{
		for (unsigned int nX = (*posX - 3); nX <= (*posX + 3); nX++)
		{
			if (nX < 0 || nY < 0 || nX >= 4096 || nY >= 4096)
				continue;

			if (g_pMobGrid[nY][nX] == 0)
			{
				if (g_pHeightGrid[nY][nX] != 127)
				{
					*posX = nX;
					*posY = nY;
					return true;
				}
			}
		}
	}

	for (unsigned int nY = (*posY - 4); nY <= (*posY + 4); nY++)
	{
		for (unsigned int nX = (*posX - 4); nX <= (*posX + 4); nX++)
		{
			if (nX < 0 || nY < 0 || nX >= 4096 || nY >= 4096)
				continue;

			if (g_pMobGrid[nY][nX] == 0)
			{
				if (g_pHeightGrid[nY][nX] != 127)
				{
					*posX = nX;
					*posY = nY;
					return true;
				}
			}
		}
	}

	for (unsigned int nY = (*posY - 5); nY <= (*posY + 5); nY++)
	{
		for (unsigned int nX = (*posX - 5); nX <= (*posX + 5); nX++)
		{
			if (nX < 0 || nY < 0 || nX >= 4096 || nY >= 4096)
				continue;

			if (g_pMobGrid[nY][nX] == 0)
			{
				if (g_pHeightGrid[nY][nX] != 127)
				{
					*posX = nX;
					*posY = nY;
					return true;
				}
			}
		}
	}

	return false;
}

/*
bool GetEmptyMobGrid_7556(int clientId, int *posX, int *posY)
{
	if (*posX < 0 || *posX >= 4096 || *posY < 0 || *posY >= 4096)
	{
		Log(clientId, LOG_ERROR, "GetEmptyMobGrid of Range %d %d", posX, posY);

		return false;
	}

	if (g_pMobGrid[*posY][*posX] == clientId)
		return true;
	
	int LOCAL1 = g_pMobGrid[*posY][*posX];

	if (LOCAL1 <= 0 || LOCAL1 >= 12800)
	{
		g_pMobGrid[*posY][*posX] = 0;
		LOCAL1 = 0;
	}

	if (pMob[LOCAL1].Mode == 0)
	{
		g_pMobGrid[*posY][*posX] = 0;
		LOCAL1 = 0;
	}

	if (LOCAL1 == 0)
	{
		if (g_pHeightGrid[*posY][*posX] != 127)
			return true;
	}
	
	int LOCAL2 = 0;
	int LOCAL3 = 0;

	LOCAL2 = *posY - 1;

	for (; LOCAL2 < (*posY + 1); LOCAL2++)
	{
		LOCAL3 = *posX - 1;
		for (; LOCAL3 < (*posX + 1); LOCAL3++)
		{
			if (LOCAL3 < 0 || LOCAL2 < 0 || LOCAL3 >= 4096 || LOCAL2 >= 4096)
				continue;

			if (g_pMobGrid[LOCAL2][LOCAL3] == 0)
			{
				if (g_pHeightGrid[LOCAL2][LOCAL3] != 127)
				{
					*posY = LOCAL2;
					*posX = LOCAL3;

					return true;
				}
			}
		}
	}

	LOCAL2 = *posY - 2;

	for (; LOCAL2 < (*posY + 2); LOCAL2++)
	{
		LOCAL3 = *posX - 2;
		for (; LOCAL3 < (*posX + 2); LOCAL3++)
		{
			if (LOCAL3 < 0 || LOCAL2 < 0 || LOCAL3 >= 4096 || LOCAL2 >= 4096)
				continue;

			if (g_pMobGrid[LOCAL2][LOCAL3] == 0)
			{
				if (g_pHeightGrid[LOCAL2][LOCAL3] != 127)
				{
					*posY = LOCAL2;
					*posX = LOCAL3;

					return true;
				}
			}
		}
	}

	LOCAL2 = *posY - 3;

	for (; LOCAL2 < (*posY + 3); LOCAL2++)
	{
		LOCAL3 = *posX - 3;
		for (; LOCAL3 < (*posX + 3); LOCAL3++)
		{
			if (LOCAL3 < 0 || LOCAL2 < 0 || LOCAL3 >= 4096 || LOCAL2 >= 4096)
				continue;

			if (g_pMobGrid[LOCAL2][LOCAL3] == 0)
			{
				if (g_pHeightGrid[LOCAL2][LOCAL3] != 127)
				{
					*posY = LOCAL2;
					*posX = LOCAL3;

					return true;
				}
			}
		}
	}

	return false;
}
*/

void GetAction(int clientId, short posX, short posY, void *buf)
{
	p36C *p = (p36C*)buf;

	CMob *mob = (CMob*)(&pMob[clientId]);

	p->Header.ClientId = clientId;
	p->Header.PacketId = 0x36C;
	p->Header.Size = sizeof p36C;

	p->LastPos.X = mob->Target.X;
	p->LastPos.Y = mob->Target.Y;

	p->Destiny.X = posX;
	p->Destiny.Y = posY;

	p->MoveSpeed = GetSpeed(&mob->Mobs.Player.CurrentScore);
	p->MoveType = 0;
	
	memcpy(&p->Command[0], &pMob[clientId].Route, 24);
}


int GetItemSanc(STRUCT_ITEM *item)
{
    int value = 0;

    if(item->sIndex >= 2360 && item->sIndex <= 2389)
    {
        //Montarias.
        value = (item->stEffect[2].cEffect / 10);

        if(value > 9)
            value = 9;

        return value;
    }

    if(item->sIndex >= 2330 && item->sIndex <= 2359)
    {
        //Crias.
        return 0;
    }

    if(item->stEffect[0].cEffect == 43)
        value = item->stEffect[0].cValue;
    else if(item->stEffect[1].cEffect == 43)
        value = item->stEffect[1].cValue;
    else if(item->stEffect[2].cEffect == 43)
        value = item->stEffect[2].cValue;
	else
	{
		for(INT32 i = 0; i < 3; i++)
		{
			if(item->stEffect[i].cEffect >= 116 && item->stEffect[i].cEffect <= 125)
			{
				value = item->stEffect[i].cValue;

				break;
			}
		}
	}

    if(value >= 230)
    {
        value = 10 + ((value - 230) / 4);
        if(value > 15)
            value = 15;
    }
    else
        value %= 10;

    return value;
}

bool IsImpossibleToRefine(STRUCT_ITEM* item)
{
	if (item->sIndex <= 0 || item->sIndex >= MAX_ITEMLIST)
		return false;

	bool haveRef = false;
	for (int i = 0; i < 3; i++)
	{
		if (item->stEffect[i].cEffect == 0)
			return false;

		if (item->stEffect[i].cEffect == 43 || (item->stEffect[i].cEffect >= 116 && item->stEffect[i].cEffect <= 125))
			return false;
	}

	return true;
}

int GetInfoClass (int face) 
{
	if(face < 10)
		return 0;
	else if(face < 20)
		return 1;
	else if(face < 30 || face == 32)
		return 2;
	else if(face < 40)
		return 3;

	return 4;
}

short GetEffectValueByIndex(int ItemID, int sIndex)
{
	STRUCT_ITEMLIST *item = &g_pItemList[ItemID];
	for(int i = 0; i < 12; i++)
	{
		if(item->stEffect[i].sEffect == sIndex)
			return item->stEffect[i].sValue;
	}
	return 0;
}


eMapAttribute GetAttribute(int posX, int posY)
{
	if (posX < 0 || posX > 4096 || posY < 0 || posY > 4096)
		return g_pAttributeMap[0][0];

	int gridX = (posX >> 2) & 0x3FF;
	int gridY = (posY >> 2) & 0x3FF;

	return g_pAttributeMap[gridY][gridX];
}

int GetSpeed(STRUCT_SCORE *status)
{
	int speed = status->Move.Speed;

	if(speed < 1)
		speed = 1;
	else if(speed > 6)
		speed = 6;

	return speed;
}

int GetDistance(int x1,int y1,int x2,int y2)
{
	int x, y;
	if(x1 > x2)
		x = x1 - x2;
	else
		x = x2 - x1;

	if(y1 > y2)
		y = y1 - y2;
	else
		y = y2 - y1;

	if(x <= 7 && y <= 7)
		return g_pDistanceTable[y][x]; // array com distências

	if(x > y)
		return x + 1;

	return y + 1;
}

/*
INT32 GetRoute(int arg1,int arg2,int *arg3,int *arg4,char *arg5,int arg6,char *arg7)
{
	int LOCAL_1 = arg1;
	int LOCAL_2 = arg2;
	int LOCAL_3 = *arg3;
	int LOCAL_4 = *arg4;

	memset(arg5, 0, 24);
	int LOCAL_5 = 0;
	for( ; LOCAL_5 <= arg6 || LOCAL_5 < 23; LOCAL_5++)
	{
		if((arg1 - g_HeightPosX) < 1 || (arg2 - g_HeightPosY) < 1 || (arg1 - g_HeightPosX) > (g_HeightWidth - 2) || (arg2 - g_HeightPosY) > (g_HeightHeight - 2))
		{
			arg5[LOCAL_5] = 0;

			break;
		}

		// 0040C544
		INT32  LOCAL_6 = arg7[arg2 * 4096 + arg1],
			  LOCAL_7 = 0;

		if(LOCAL_6 > 8)
			LOCAL_7 = 0;
		
		INT32 LOCAL_8   =  arg7[(arg2 - 1) * 4096  + arg1];	//	nw
        INT32 LOCAL_9   =  arg7[(arg2 - 1) * 4096  + arg1 + 1];   // n
        INT32 LOCAL_10   = arg7[(arg2      * 4096) + arg1 + 1];   // ne
		INT32 LOCAL_11   = arg7[(arg2 + 1) * 4096  + arg1 + 1];   // w
        INT32 LOCAL_12   = arg7[(arg2 + 1) * 4096  + arg1];   // e
		INT32 LOCAL_13   = arg7[(arg2 + 1) * 4096  + arg1 - 1];   // sw
		INT32 LOCAL_14   = arg7[(arg2      * 4096) + arg1 - 1];   // s
        INT32 LOCAL_15   = arg7[(arg2 - 1) * 4096  + arg1 - 1];   // se
		
		if (LOCAL_3==arg1 && LOCAL_4==arg2) {
			arg5[LOCAL_5]=0;
			break;
		}

		if(LOCAL_3 == arg1 && LOCAL_4 < arg2 && LOCAL_8 < (LOCAL_6 + 8) && LOCAL_8 > (LOCAL_6 - 8))
		{
			arg5[LOCAL_5] = '2'; // 0x32
			arg2 --;
			continue;
		}

		// 0040C70F
		if(LOCAL_3 > arg1 && LOCAL_4 < arg2 && LOCAL_9 < (LOCAL_6 + 8) && LOCAL_9 > (LOCAL_6 - 8))
		{
			arg5[LOCAL_5] = '3';
			arg2 --;
			continue;
		}

		// 0040C755
		if(LOCAL_3 > arg1 && LOCAL_4 == arg2 && LOCAL_10 < (LOCAL_6 + 8) && LOCAL_10 > (LOCAL_6 - 8))
		{
			arg5[LOCAL_5] = '6';
			arg1 ++;
			continue;
		}

		// 00040C792
		if(LOCAL_3 > arg1 && LOCAL_4 > arg2 && LOCAL_11 < (LOCAL_6 + 8) && LOCAL_11 > (LOCAL_6 - 8))
		{
			arg5[LOCAL_5] = '9';
			arg1++;
			arg2++;
			continue;
		}

		// 0040C7D8
		if(LOCAL_3 == arg1 && LOCAL_4 > arg2 && LOCAL_12 < (LOCAL_6 + 8) && LOCAL_12 > (LOCAL_6 - 8))
		{
			arg5[LOCAL_5] = '8';
			arg2++;
			continue;
		}
		
		// 0040C815
		if(LOCAL_3 < arg1 && LOCAL_4 > arg2 && LOCAL_13 < (LOCAL_6 + 8) && LOCAL_13 > (LOCAL_6 - 8))
		{
			arg5[LOCAL_5] = '7';
			arg1 --;
			arg2 ++;
			continue;
		}

		// 0040C85B
		if(LOCAL_3 < arg1 && LOCAL_4 == arg2 && LOCAL_14 < (LOCAL_6 + 8) && LOCAL_14 > (LOCAL_6 - 8))
		{
			arg5[LOCAL_5] = '4';
			arg1--;
			continue;
		}

		// 0040C898
		if(LOCAL_3 < arg1 && LOCAL_4 < arg2 && LOCAL_15 < (LOCAL_6 + 8) && LOCAL_15 > (LOCAL_6 - 8))
		{
			arg5[LOCAL_5] = '1';
			arg1--;
			arg2--;
			continue;
		}

		//0040C8DE
		if(LOCAL_3 > arg1 && LOCAL_4 < arg2 && LOCAL_10 < (LOCAL_6 + 8) && LOCAL_10 > (LOCAL_6 - 8))
		{
			arg5[LOCAL_5] = '6';
			arg1++;
			continue;
		}

		// 0040C91B
		if(LOCAL_3 > arg1 && LOCAL_4 < arg2 && LOCAL_8 < (LOCAL_6 + 8) && LOCAL_8 > (LOCAL_6 - 8))
		{
			arg5[LOCAL_5] = '2';
			arg2--;
			continue;
		}

		// 0040C958
		if(LOCAL_3 > arg1 && LOCAL_4 > arg2 && LOCAL_10 < (LOCAL_6 + 8) && LOCAL_10 > (LOCAL_6 - 8))
		{
			arg5[LOCAL_5] = '6';
			arg1++;
			continue;
		}

		// 0040C995
		if(LOCAL_3 > arg1 && LOCAL_4 > arg2 && LOCAL_12 < (LOCAL_6 + 8) && LOCAL_12 > (LOCAL_6 - 8))
		{
			arg5[LOCAL_5] = '8';
			arg2++;
			continue;
		}

		//0040C9D2
		if(LOCAL_3 < arg1 && LOCAL_4 > arg2 && LOCAL_14 < (LOCAL_6 + 8) && LOCAL_14 > (LOCAL_6 - 8))
		{
			arg5[LOCAL_5] = '4';
			arg1 --;
			continue;
		}

		//0040CA0F
		if(LOCAL_3 < arg1 && LOCAL_4 > arg2 && LOCAL_12 < (LOCAL_6 + 8) && LOCAL_12 > (LOCAL_6 - 8))
		{
			arg5[LOCAL_5] = '8';
			arg2++;
			continue;
		}

		// 0040CA4C
		if(LOCAL_3 < arg1 && LOCAL_4 < arg2 && LOCAL_14 < (LOCAL_6 + 8) &&LOCAL_14 > (LOCAL_6 - 8))
		{
			arg5[LOCAL_5] = '4';
			arg1--;
			continue;
		}

		//0040CA89
		if(LOCAL_3 < arg1 && LOCAL_4 < arg2 && LOCAL_8 < (LOCAL_6 + 8) && LOCAL_8 > (LOCAL_6 - 8))
		{
			arg5[LOCAL_5] = '2';
			arg2--;
			continue;
		}

		// 0040CAC6
		if(LOCAL_3 == (arg1 + 1) || LOCAL_4 == (arg2 + 1) || LOCAL_3 == (arg1 - 1) || LOCAL_4 == (arg2 - 1))
		{
			arg5[LOCAL_5] = 0;

			break;
		}

		// 0040CB00
		if(LOCAL_3 == arg1 && LOCAL_4 > arg2 && LOCAL_11 < (LOCAL_6 + 8) && LOCAL_11 > (LOCAL_6 - 8))
		{
			arg5[LOCAL_5] = '9';
			arg1++;
			arg2++;
			continue;
		}

		// 0040CB46
		if(LOCAL_3 == arg1 && LOCAL_4 > arg2 && LOCAL_13 < (LOCAL_6 + 8) && LOCAL_13 > (LOCAL_6 - 8))
		{
			arg5[LOCAL_5] = '7';
			arg1--;
			arg2++;
			continue;
		}

		//0040CB8C
		if(LOCAL_3 == arg1 && LOCAL_4 < arg2 && LOCAL_9 < (LOCAL_6 + 8) && LOCAL_9 > (LOCAL_6 - 8))
		{
			arg5[LOCAL_5] = '3';
			arg1++;
			arg2--;
			continue;
		}

		// 0040CBD2
		if(LOCAL_3 == arg1 && LOCAL_4 < arg2 && LOCAL_15 < (LOCAL_6 + 8) && LOCAL_15 > (LOCAL_6 - 8))
		{
			arg5[LOCAL_5] = '1';
			arg1--;
			arg2--;
			continue;
		}

		// 0040CC18
		if(LOCAL_3 < arg1 && LOCAL_4 == arg2 && LOCAL_13 < (LOCAL_6 + 8) && LOCAL_13 > (LOCAL_6 - 8))
		{
			arg5[LOCAL_5] = '7';
			arg1--;
			arg2++;
			continue;
		}

		//0040CC5E
		if(LOCAL_3 < arg1 && LOCAL_4 == arg2 && LOCAL_15 < (LOCAL_6 + 8) && LOCAL_15 > (LOCAL_6 - 8))
		{
			arg5[LOCAL_5] = '1';
			arg1--;
			arg2--;
			continue;
		}

		// 0040CCA4
		if(LOCAL_3 > arg1 && LOCAL_4 == arg2 && LOCAL_11 < (LOCAL_6 + 8) && LOCAL_11 > (LOCAL_6 - 8))
		{
			arg5[LOCAL_5] = '9';
			arg1++;
			arg2++;
			continue;
		}

		// 0040CCEA
		if(LOCAL_3 > arg1 && LOCAL_4 == arg2 && LOCAL_9 < (LOCAL_6 + 8) && LOCAL_9 > (LOCAL_6 - 8))
		{
			arg5[LOCAL_5] = '3';
			arg1++;
			arg2--;
			continue;
		}
	}

	arg5[LOCAL_5] = 0;

	if(LOCAL_1 == arg1 && LOCAL_2 == arg2)
		return 0;

	INT32 LOCAL_16 = 0;

	*arg3 = arg1;
	*arg4 = arg2;
	
     if (LOCAL_1 == arg1 && LOCAL_2 == arg2) 
		 return 0;

	 return 1;
}
*/

INT32 GetRoute(unsigned int x, unsigned int y, unsigned int *targetX, unsigned int *targetY, char *route, int distance)
{
	unsigned int lastX = x; //local1
	unsigned int lastY = y; //local2
	unsigned int tX = *targetX; //local3
	unsigned int tY = *targetY; //local4

	memset(route, 0, 24);

	int i = 0; // LOCAL5
	for (; i < distance || i < 23; i++)
	{
		if ((x - g_HeightPosX) < 1 || (y - g_HeightPosY) < 1 || (x - g_HeightPosX) > (g_HeightWidth - 2) || (y - g_HeightPosY) > (g_HeightHeight - 2))
		{
			route[i] = 0;

			break;
		}

		// 0040C544
		INT32 cul = g_pHeightGrid[y - g_HeightPosY][x - g_HeightPosX], // local6
			  kk  = 0; // local7

		if (cul > 8)
			kk = 0;

		INT32 nw = g_pHeightGrid[(y - g_HeightPosY - 1)][x - g_HeightPosX - 1];	  //nw
		INT32 n = g_pHeightGrid [(y - g_HeightPosY - 1)][x - g_HeightPosX    ];   // n
		INT32 ne = g_pHeightGrid[(y - g_HeightPosY - 1)][x - g_HeightPosX + 1];   // ne
		INT32 w = g_pHeightGrid [(y - g_HeightPosY)]    [x - g_HeightPosX - 1];   // w
		INT32 e = g_pHeightGrid [(y - g_HeightPosY)]    [x - g_HeightPosX + 1];   // e
		INT32 sw = g_pHeightGrid[(y - g_HeightPosY + 1)][x - g_HeightPosX - 1];   // sw
		INT32 s = g_pHeightGrid [(y - g_HeightPosY + 1)][x - g_HeightPosX    ];   // s
		INT32 se = g_pHeightGrid[(y - g_HeightPosY + 1)][x - g_HeightPosX + 1];   // se LOCAL15

		if (tX == x && tY == y)
		{
			route[i] = 0; 
			break;
		}
		if (tX == x && tY < y && n < cul + 8 && n > cul - 8) 
		{
			route[i] = '2'; 
			y--; 
			continue;
		}
		if (tX > x && tY < y && ne < cul + 8 && ne > cul - 8) 
		{
			route[i] = '3'; 
			x++; 
			y--; 
			continue;
		}
		if (tX > x && tY == y && e < cul + 8 && e > cul - 8) 
		{
			route[i] = '6'; 
			x++; 
			continue;
		}
		if (tX > x && tY > y && se < cul + 8 && se > cul - 8) 
		{
			route[i] = '9';
			x++;	
			y++;	
			continue;
		}
		if (tX == x && tY > y && s <cul + 8 && s > cul - 8) 
		{
			route[i] = '8';
			y++; 
			continue;
		}
		if (tX < x && tY > y && sw < cul + 8 && sw > cul - 8) 
		{
			route[i] = '7';
			x--;	
			y++; 
			continue;
		}
		if (tX < x && tY == y && w < cul + 8 && w > cul - 8) 
		{
			route[i] = '4'; 
			x--; 
			continue;
		}
		if (tX < x && tY < y && nw < cul + 8 && nw > cul - 8) 
		{
			route[i] = '1';
			x--; 
			y--; 
			continue;
		}
		if (tX > x && tY < y && e < cul + 8 && e > cul - 8) 
		{
			route[i] = '6'; 
			x++; 
			continue;
		}
		if (tX > x && tY < y && n < cul + 8 && n > cul - 8) 
		{
			route[i] = '2'; 
			y--; 
			continue;
		}
		if (tX > x && tY > y && e < cul + 8 && e > cul - 8) 
		{
			route[i] = '6'; 
			x++; 
			continue;
		}
		if (tX > x && tY > y && s < cul + 8 && s > cul - 8) 
		{
			route[i] = '8'; 
			y++; 
			continue;
		}
		if (tX < x && tY > y && w < cul + 8 && w > cul - 8) 
		{
			route[i] = '4'; 
			x--; 
			continue;
		}
		if (tX < x && tY > y && s < cul + 8 && s > cul - 8)		
		{
			route[i] = '8'; 
			y++; 
			continue;
		}
		if (tX < x && tY < y && w < cul + 8 && w > cul - 8)
		{
			route[i] = '4';
			x--; 
			continue;
		}
		if (tX < x && tY < y && n < cul + 8 && n > cul - 8) 
		{
			route[i] = '2';
			y--;
			continue;
		}
		if (tX == x + 1 || tY == y + 1 || tX == x - 1 || tY == y - 1) 
		{
			route[i] = 0; 
			break;
		}

		//0040CAC6
		if (tX == x && tY > y && se < cul + 8 && se > cul - 8) 
		{
			route[i] = '9';
			x++; 
			y++;
			continue;
		}
		if (tX == x && tY > y && sw <cul + 8 && sw > cul - 8) 
		{ 
			route[i] = '7'; 
			x--; 
			y++;
			continue; 
		}
		if (tX == x && tY < y && ne < cul + 8 && ne > cul - 8) 
		{ 
			route[i] = '3';
			x++;
			y--;
			continue; 
		}
		if (tX == x && tY < y && nw < cul + 8 && nw > cul - 8)
		{ 
			route[i] = '1';
			x--;
			y--;
			continue; 
		}
		if (tX < x && tY == y && sw < cul + 8 && sw > cul - 8) 
		{ 
			route[i] = '7'; 
			x--;
			y++;
			continue; 
		}
		if (tX < x && tY == y && nw  < cul + 8 && nw > cul - 8)
		{ 
			route[i] = '1';
			x--;
			y--;
			continue; 
		}
		if (tX > x && tY == y && se < cul + 8 && se > cul - 8)
		{ 
			route[i] = '9';
			x++; 
			y++;
			continue; 
		}
		if (tX > x && tY == y && ne < cul + 8 && ne > cul - 8)
		{ 
			route[i] = '3';
			x++; 
			y--; 
			continue; 
		}
		
		break;
	}

	route[i] = 0;

	if (lastX == x && lastY == y)
		return 0;

	*targetX = x;
	*targetY = y;

	if (lastX == x && lastY == y)
		return 0;

	return 1;
}

int GetDamage(int dam, int ac, int combat) 
{
	int calcDmg = dam - (ac >> 1);
	combat >>= 1;
    if(combat > 7)
        combat = 7;
 
    int calcRand = 12 - combat;
    int incDmg = (Rand() % calcRand) + combat + 99;
 
    calcDmg = (calcDmg * incDmg) / 100;
    if(calcDmg < -50)
		calcDmg = 0;
    else if(calcDmg >= -50 && calcDmg < 0)
		calcDmg = (calcDmg + 50) / 7;
    else if(calcDmg >= 0 && calcDmg <= 50)
		calcDmg = ((calcDmg * 5) >> 2) + 7;
 
    if(calcDmg <= 0)
		calcDmg = 1;
 
    return calcDmg;
}
/*
INT32 GetParryRate(CMob *mob, INT32 bonus, INT32 dex, INT32 *info)
{
	if(bonus > 100)
		bonus = 100;

	if(bonus < 0)
		bonus = 0;

	INT32 LOCAL_1 = mob->Mobs.Player.CurrentScore.Dex;
	if(LOCAL_1 > 1000)
		LOCAL_1 = 1000;

	INT32 LOCAL_2 = mob->Mobs.Player.CurrentScore.Dex - 1000;
	if(LOCAL_2 < 0)
		LOCAL_2 = 0;

	if(LOCAL_2 > 2000)
		LOCAL_2 = 2000;

	INT32 LOCAL_3 = mob->Mobs.Player.CurrentScore.Dex - 3000;
	if(LOCAL_3 < 0)
		LOCAL_3 = 0;

	INT32 LOCAL_4 = (LOCAL_1 >> 1) + ((LOCAL_2 >> 2) + bonus) + (LOCAL_3 >> 3);
	LOCAL_4 -= dex;

	INT32 LOCAL_5 = *info;
	if(LOCAL_5 & 20)
		LOCAL_4 += 100;

	if(LOCAL_5 & 0x80)
		LOCAL_4 += 50;

	if(LOCAL_5 & 200)
		LOCAL_5 += 50;

	if(LOCAL_4 > 650)
		LOCAL_4 = 650;

	if(LOCAL_4 < 1)
		LOCAL_4 = 1;
,}*/

INT32 GetParryRate(int clientId, int mobId, int type)
{
	constexpr std::array pEvadeEsfera = 
	{
		0, 10, 0, 0, 10, 0, 0, 10, 0, 16, 10, 0, 0, 60, 60, 50, 60, 0, 0, 0
	};
	
	INT32 dex = pMob[clientId].Mobs.Player.CurrentScore.Dex / 5; // arg3?
	if(pMob[clientId].Mobs.Player.AffectInfo.Evasion)
		dex += 150;

	if(type == -2)
		dex += 300;
	
	if(pMob[clientId].Mobs.Player.AffectInfo.SpeedMov)
		dex += 50;
	
	INT32 targetDex = pMob[mobId].Mobs.Player.CurrentScore.Dex; // local1
	if(targetDex > 1000)
		targetDex = 1000;

	INT32 targetDexCalc1 = pMob[mobId].Mobs.Player.CurrentScore.Dex - 1000; // local2
	if(targetDexCalc1 < 0)
		targetDexCalc1 = 0;

	if(targetDexCalc1 > 2000)
		targetDexCalc1 = 2000;

	INT32 targetDexCalc2 = pMob[mobId].Mobs.Player.CurrentScore.Dex - 3000;
	if(targetDexCalc2 < 0)
		targetDexCalc2 = 0;

	// ECX = LOCAL_1 >> 1
	// ESI = LOCAL_2 >> 2 + arg2
	// EAX = LOCAL_3 >> 3
	// LOCAL_4 = ESI + ECX + EAX
	INT32 calcParry = (targetDex >> 1) + (targetDexCalc1 >> 2) + (targetDexCalc2 >> 3);//(LOCAL_3 >> 3) + ((LOCAL_2 + (LOCAL_1 >> 1)) >> 2 + player[clientId].Parry);
	calcParry -= dex;

	if (pMob[mobId].Mobs.Player.Equip[14].sIndex >= 3980 && pMob[mobId].Mobs.Player.Equip[14].sIndex <= 3999)
	{
		int itemId = pMob[mobId].Mobs.Player.Equip[14].sIndex - 3980;
		calcParry += pEvadeEsfera[itemId];
	}

	if (pMob[clientId].Mobs.Player.Equip[14].sIndex >= 3980 && pMob[clientId].Mobs.Player.Equip[14].sIndex <= 3999)
	{
		int itemId = pMob[clientId].Mobs.Player.Equip[14].sIndex - 3980;
		calcParry += pEvadeEsfera[itemId];
	}

	if (calcParry > 750)
		calcParry = 750;

	if (pMob[clientId].Mobs.Player.Class == 1 && (pMob[clientId].Mobs.Player.LearnedSkill[0] & (1 << (47 % 24))) && type == -2)
		calcParry -= 50;

	if (pMob[clientId].Mobs.Player.Class == 0 && ((pMob[clientId].Mobs.Player.LearnedSkill[0] & (1 << 7)) || (pMob[clientId].Mobs.Player.LearnedSkill[0] & (1 << 23))) && type == -1)
		calcParry -= 75;

	calcParry += pMob[mobId].Parry;

	int jewelIndex = pMob[clientId].Jewel;
	if (jewelIndex >= 0 && jewelIndex < 32 && (pMob[clientId].Mobs.Affects[jewelIndex].Value & 64) != 0)
		calcParry -= 50;

	if ((pMob[clientId].Mobs.Player.LearnedSkill[0] & (1 << 28)))
		calcParry -= 75;

	calcParry -= pMob[clientId].HitRate;

	if(calcParry < 1)
		calcParry = 1;

	return calcParry;
}

int GetPKPoint(int sIndex)
{
	return pMob[sIndex].Mobs.Player.Inventory[63].stEffect[0].cEffect;
}

INT32 GetSkillDamage_PvP(INT32 skillId, CMob *player, INT32 weather, INT32 weaponDamage)
{ //7DCDD8
	INT32 value[5];
	INT32 damage[2];

	INT32 type = SkillData[skillId].InstanceType;  // local1

	auto mob = &player->Mobs.Player;
	INT32 mod = skillId % 0x18;
	value[0] = (mod >> 3) + 1; // local2  - 3 

	value[1] = mob->CurrentScore.Level; // local3  - 199
	if (mob->Equip[0].EFV2 >= CELESTIAL)
		value[1] += 400;

	if (value[1] < 0)
		value[1] = 0;

	if (value[1] > 600)
		value[1] = 600;

	value[2] = mob->CurrentScore.Special[value[0]]; //local4

	damage[0] = SkillData[skillId].InstanceValue; //local5
	damage[1] = SkillData[skillId].AffectValue; //local6 - TickValue old

	INT32 retn = 0; //local7

	if (type == 0)
	{
		switch (skillId)
		{

		case 11:
			retn = value[2] / 10 + damage[1];
			break;
		case 13:
			retn = ((value[2] * 3) >> 2) + damage[1];
			break;
		case 41:
			retn = value[2] / 25 + 2;
			break;
		case 43:
			retn = value[2] / 3 + damage[1];
			break;
		case 44:
			retn = ((value[2] * 3) / 20 + damage[1]) << 1;
			break;
		case 45:
			retn = (value[2] / 10 + damage[1]);
			break;
			//	default:
			//		return retn;
		}
	}
	else if (type >= 1 && type <= 5)
	{
		int local8 = skillId >> 3;

		if (skillId == 97)
			retn = value[1] * 15 + damage[0];
		else if (mob->Class == 0 && local8 == 1) // TK
			retn = (damage[0] + value[2]) + (value[1] >> 1) + (mob->CurrentScore.Dex * 2) + (weaponDamage * 3);
		else if (mob->Class == 0 && local8 != 1)
		{
			retn = (damage[0] + value[2]) + (value[1] >> 1) + weaponDamage + (mob->CurrentScore.Int / 2);

			if (skillId == 7 || skillId >= 17 && skillId <= 23 && (mob->LearnedSkill[0] & 0x800000))
				retn = (damage[0] + value[2]) + (value[1] >> 1) + weaponDamage + ((mob->CurrentScore.Int / 2) * 110 / 100);

			if (skillId == 7)
				retn += (retn * 10 / 100);
		}
		else if (mob->Class == 1)
		{
			retn = (damage[0] + value[2]) + (value[1] >> 1) + (mob->CurrentScore.Int / 2) + weaponDamage;

			if (skillId == 28) // CHOQUE DIVINO
				retn += (mob->CurrentScore.Hp * 10 / 100);
		}
		else if (mob->Class == 2)
		{
			retn = (damage[0] + value[2]) + (value[1] >> 1) + (mob->CurrentScore.Int / 2);

			// Com oitava habilidade
			if (skillId == 48 || skillId == 50 || skillId == 52 || skillId == 55 && (mob->LearnedSkill[0] & 0x80))
				retn = (damage[0] + value[2]) + (value[1] >> 1) + weaponDamage + ((mob->CurrentScore.Int / 2) * 108 / 100);

			retn += (retn * 5 / 100);
		}
		else if (mob->Class == 3)
			retn = (damage[0] + value[2]) + (value[1] >> 1) + (mob->CurrentScore.Str * 3) + (weaponDamage * 3);

		if (weather == 1)
		{
			if (skillId == 39 || skillId == 48)
				retn = retn * 90 / 100;
			else if (skillId == 17)
				retn = retn * 90 / 100;
			else if (skillId == 33)
				retn = retn * 150 / 100;
		}
		else if (weather == 2 && type == 3)
			retn = retn * 120 / 100;

		if (skillId == 97)
		{
			retn = retn;
			return retn;
		}
		else if (mob->Class || local8 != 1)
		{
			if (mob->Class == 3)
				retn = (retn * 5 / 4);
			else
			{
				retn = ((player->MagicIncrement * 2) + 100) * retn / 100;
				retn += (retn * 10 / 100);
			}
		}
		else
			return (retn * 5 / 4);
	}
	else if (type == 6)
		retn = (value[2] * 3 + damage[0]);
	else if (type == 11)
	{
		retn = damage[0];
		return retn;
	}
	else
		retn = player->MagicIncrement;

	if (skillId == 79)
		retn = mob->CurrentScore.Damage * 85 / 100;

	return retn;
}

INT32 GetSkillDamage_PvM(INT32 skillId, CMob *player, INT32 weather, INT32 weaponDamage)
{ //7DCDD8
	INT32 value[5];
	INT32 damage[2];

	INT32 type = SkillData[skillId].InstanceType;  // local1

	auto mob = &player->Mobs.Player;

	INT32 mod = skillId % 0x18;
	value[0] = (mod >> 3) + 1; // local2  - 3 
	value[1] = mob->CurrentScore.Level; // local3  - 199

	if (mob->Equip[0].EFV2 >= CELESTIAL)
		value[1] += 400;

	if (value[1] < 0)
		value[1] = 0;

	if (value[1] > 600)
		value[1] = 600;

	value[2] = mob->CurrentScore.Special[value[0]]; //local4

	damage[0] = SkillData[skillId].InstanceValue; //local5
	damage[1] = SkillData[skillId].AffectValue; //local6 - TickValue old

	INT32 retn = 0; //local7

	if (type == 0)
	{
		switch (skillId)
		{

		case 11:
			retn = value[2] / 10 + damage[1];
			break;
		case 13:
			retn = ((value[2] * 3) >> 2) + damage[1];
			break;
		case 41:
			retn = value[2] / 25 + 2;
			break;
		case 43:
			retn = value[2] / 3 + damage[1];
			break;
		case 44:
			retn = ((value[2] * 3) / 20 + damage[1]) << 1;
			break;
		case 45:
			retn = (value[2] / 10 + damage[1]);
			break;
			//	default:
			//		return retn;
		}
	}
	else if (type >= 1 && type <= 5)
	{
		int local8 = skillId >> 3;

		if (skillId == 97)
			retn = value[1] * 15 + damage[0];
		else if (mob->Class == 0 && local8 == 1) // TK
			retn = (damage[0] + value[2]) + (value[1] >> 1) + (mob->CurrentScore.Dex) + (weaponDamage * 3);
		else if (mob->Class == 0 && local8 != 1)
		{
			retn = (damage[0] + value[2]) + (value[1] >> 1) + weaponDamage + (mob->CurrentScore.Int / 2);

			if (skillId == 7 || skillId >= 17 && skillId <= 23 && (mob->LearnedSkill[0] & 0x800000))
				retn = (damage[0] + value[2]) + (value[1] >> 1) + weaponDamage + ((mob->CurrentScore.Int / 2) * 108 / 100);

			if (skillId == 7)
				retn += (retn * 10 / 100);
		}
		else if (mob->Class == 1)
		{
			retn = (damage[0] + value[2]) + (value[1] >> 1) + (mob->CurrentScore.Int / 2) + weaponDamage;

			if (skillId >= 32 && skillId <= 39 && (mob->LearnedSkill[0] & 0x8000))
				retn = (damage[0] + value[2]) + (value[1] >> 1) + weaponDamage + ((mob->CurrentScore.Int / 2) * 115 / 100);

			if (skillId == 40 && (mob->LearnedSkill[0] & 0x800000))
				retn = (damage[0] + value[2]) + (value[1] >> 1) + weaponDamage + ((mob->CurrentScore.Int / 2) * 115 / 100);

			if (skillId == 28) // CHOQUE DIVINO
				retn += (mob->CurrentScore.Hp * 10 / 100);
		}
		else if (mob->Class == 2)
		{
			retn = (damage[0] + value[2]) + (value[1] >> 1) + (mob->CurrentScore.Int / 2);

			// Com oitava habilidade
			if (skillId == 48 || skillId == 50 || skillId == 52 || skillId == 55 && (mob->LearnedSkill[0] & 0x80))
				retn = (damage[0] + value[2]) + (value[1] >> 1) + weaponDamage + ((mob->CurrentScore.Int / 2) * 115 / 100);

			retn += (retn * 8 / 100);
		}
		else if (mob->Class == 3)
			retn = (damage[0] + value[2]) + (value[1] >> 1) + (mob->CurrentScore.Str * 3) + (weaponDamage * 3);

		if (weather == 1)
		{
			if (skillId == 39 || skillId == 48)
				retn = retn * 90 / 100;
			else if (skillId == 17)
				retn = retn * 90 / 100;
			else if (skillId == 33)
				retn = retn * 150 / 100;
		}
		else if (weather == 2 && type == 3)
			retn = retn * 120 / 100;

		if (skillId == 97)
		{
			retn = retn;
			return retn;
		}
		else if (mob->Class || local8 != 1)
		{
			if (mob->Class == 3)
				retn = (retn * 5 / 4);
			else
			{
				retn = ((player->MagicIncrement * 2) + 100) * retn / 100;
				retn = retn * 5 / 4;
			}
		}
		else
			return (retn * 5 / 4);
	}
	else if (type == 6)
		retn = (value[2] * 3 + damage[0]);
	else if (type == 11)
	{
		retn = damage[0];
		return retn;
	}
	else
		retn = player->MagicIncrement;

	if (skillId == 79)
		retn = mob->CurrentScore.Damage;

	return retn;
}

INT32 GetSkillDamage_2(INT32 damage, INT32 defenserAC, INT32 master)
{
	int calcDmg = damage - (defenserAC >> 1); // local_1

	if(master > 15)
		master = 15;

	int delta =		21 - master;
	int rnd = (Rand() % delta) +  master + 90; // local3

	calcDmg = calcDmg * rnd / 100;

	if(calcDmg < -50)
		calcDmg = 0;
	else if(calcDmg >= -50 && calcDmg <= 0)
		calcDmg = (calcDmg + 50) / 10;
	else if(calcDmg >= 0 && calcDmg <= 45)
		calcDmg = ((calcDmg * 5) >> 2) + 5;
	else if(calcDmg <= 0)
		calcDmg = 1;

	return calcDmg;
}

int GetVillage(unsigned int x, unsigned int y)
{
	for(int i = 0; i < 5;i++)
	{
		if(x >= g_pCityZone[i].city_min_x && x <= g_pCityZone[i].city_max_x &&
			y >= g_pCityZone[i].city_min_y && y <= g_pCityZone[i].city_max_y)
			return i;
	}
	return 5;
}

int GetArena(unsigned int x, unsigned int y)
{
	for(int i = 0; i < 4; i++)
	{
		if(x >= g_pArenaArea[i][0] && x <= g_pArenaArea[i][1] && y >= g_pArenaArea[i][2] && y <= g_pArenaArea[i][3])
			return i;
	}

	return 5;
}

int GetEmptyAffect(int mobId, int buffId)
{
	for(int i = 0; i < 32; i++)
		if(pMob[mobId].Mobs.Affects[i].Index == buffId)
			return i;

	for(int i = 0; i < 32; i++)
		if(!pMob[mobId].Mobs.Affects[i].Index)
			return i;

	return -1;
}

int GetCurKill(int sIndex)
{
	return pMob[sIndex].Mobs.Player.Inventory[63].EFV1;
}

int GetTotKill(int sIndex)
{
	unsigned char f_tFrag =  pMob[sIndex].Mobs.Player.Inventory[63].EFV2;
	unsigned char s_tFrag =  pMob[sIndex].Mobs.Player.Inventory[63].EFV3;
	return f_tFrag + (s_tFrag << 8);
}

int GetGuilty(int sIndex)
{// 0x004012CB
	int guilty = pMob[sIndex].Mobs.Player.Inventory[63].EF2; 
	int value = guilty & 0xFF;

	if(value > 50)
	{
        pMob[sIndex].Mobs.Player.Inventory[63].EF2 = 0; // 0041E775  |. C680 906C5C01 >MOV BYTE PTR DS:[EAX+15C6C90],0
		value = 0;
	}

	return value;
}

void GetHitPosition(int arg1,int arg2,int *arg3,int *arg4)
{
	if(arg1 == *arg3 && arg2 == *arg4)
		return;

	if(arg1 == 0 || arg2 == 0 || *arg3 == 0 || *arg4 == 0)
		return;

	INT32 LOCAL_1, LOCAL_2, LOCAL_3, LOCAL_4, LOCAL_5, LOCAL_6;
	//0040D0E5
	if(arg1 > *arg3)
		LOCAL_1 = arg1 - *arg3;
	else
		LOCAL_1 = *arg3 - arg1;

	if(arg2 > *arg4)
		LOCAL_2 = arg2 - *arg4;
	else
		LOCAL_2 = *arg4 - arg2;

	INT32 LOCAL_7 = GetDistance(arg1, arg2, *arg3, *arg4);

	if(LOCAL_7 <= 1)
		return;
	
	INT32 LOCAL_8;
	if(LOCAL_1 > LOCAL_2)
	{
		if(!(*arg3 - arg1))
			return;

		// 0040D170
		LOCAL_5 = ((*arg4 - arg2) * 1000) / (*arg3 - arg1);
		LOCAL_6 = (arg2 * 1000) - (LOCAL_5 * arg1);

		if(arg1 < *arg3)
			LOCAL_3 = 1;
		else
			LOCAL_3 = -1;

		arg1 = arg1 + LOCAL_3;
		if(LOCAL_1 == LOCAL_2)
		{
			if(arg2 < *arg4)
				arg2++;
			else
				arg2--;
		}

		INT32 LOCAL_9 = LOCAL_5 * arg1 + LOCAL_6;
		INT32 LOCAL_10 = LOCAL_9 / 1000;

		LOCAL_8 = g_pHeightGrid[LOCAL_10 - g_HeightPosY][arg1 - g_HeightPosX]; // look

		if(LOCAL_8 == 127)
		{
			*arg3 = 0;
			*arg4 = 0;

			return;
		}

		INT32 LOCAL_11 = LOCAL_1;
		for(INT32 LOCAL_12 = arg1; (LOCAL_12 != *arg3); LOCAL_12 += LOCAL_3)
		{
			if(LOCAL_12 == arg1)
			{
				LOCAL_11 = LOCAL_11 - 1;
				if(LOCAL_11 < 1)
					break;

				continue;
			}

			LOCAL_9 = LOCAL_5 * LOCAL_12 + LOCAL_6;
			LOCAL_10 = LOCAL_9 / 1000;

			LOCAL_4 = LOCAL_8;
			
			LOCAL_8 = g_pHeightGrid[LOCAL_10 - g_HeightPosY][LOCAL_12 - g_HeightPosX];
			if(LOCAL_8 == 127)
			{
				*arg3 = 0;
				*arg4 = 0;

				return;
			}

			if(LOCAL_8 > (LOCAL_4 + 8) || LOCAL_8 < LOCAL_4 - 8)
			{
				*arg3 = LOCAL_12;
				*arg4 = LOCAL_10;

				return;
			}
			LOCAL_11 = LOCAL_11 - 1;

			if(LOCAL_11 < 1)
				break;
		}
	}

	else
	{
		if(!(*arg4 - arg2))
			return;

		LOCAL_5 = (*arg3 - arg1) * 1000 / (*arg4 - arg2);
		LOCAL_6 = (arg1 * 1000) - (LOCAL_5 * arg2);

		if(arg2 < *arg4)
			LOCAL_3 = 1;
		else
			LOCAL_3 = -1;

		arg2 = arg2 + LOCAL_3;
		if(LOCAL_1 == LOCAL_2)
		{
			if(arg1 < *arg3)
				arg1++;
			else
				arg1--;
		}

		INT32 LOCAL_13 = LOCAL_5 * arg2 + LOCAL_6;
		INT32 LOCAL_14 = LOCAL_13 / 1000;
		
		LOCAL_8 = g_pHeightGrid[arg2 - g_HeightPosY][LOCAL_14 - g_HeightPosX];

		if(LOCAL_8 == 127)
		{
			*arg3 = 0;
			*arg4 = 0;

			return;
		}

		INT32 LOCAL_15 = LOCAL_2;
		for(INT32 LOCAL_16 = arg2; LOCAL_16 != *arg4; LOCAL_16 += LOCAL_3)
		{
			if(LOCAL_16 == arg2)
			{
				LOCAL_15 --;

				if (LOCAL_15 < 1)
					break;

				continue;
			}

			LOCAL_13 = LOCAL_5 * LOCAL_16 + LOCAL_6;
			LOCAL_14 = LOCAL_13 / 1000;
			
			LOCAL_4 = LOCAL_8;
			LOCAL_8 = g_pHeightGrid[LOCAL_16 - g_HeightPosY][LOCAL_14 - g_HeightPosX];
			if(LOCAL_8 == 127)
			{
				*arg3 = 0;
				*arg4 = 0;

				return;
			}

			if(LOCAL_8 > (LOCAL_4 + 8) || LOCAL_8 < LOCAL_4 - 8)
			{
				*arg3 = LOCAL_14;
				*arg4 = LOCAL_16;

				return;
			}

			LOCAL_15 --;
			if(LOCAL_15 < 1)
				break;
		}
	}
}

void GetMultiAttack(int attackerId, int *target, p367 *p)
{
	INT32 first = 0;
	for(INT32 i = 0; i < 13 ;i++)
	{
		if(target[i] > 0 && target[i] < MAX_MOB)
		{
			first = target[i];
			break;
		}
	}

	if(first == 0)
		return;

	p->Header.ClientId = 0x7530;
	p->attackerId = attackerId;
	p->Header.TimeStamp = clock();

	p->attackerPos.X = pMob[attackerId].Target.X;
	p->attackerPos.Y = pMob[attackerId].Target.Y;
	
	p->targetPos.X = pMob[first].Target.X;
	p->targetPos.Y = pMob[first].Target.Y;

	p->Header.Size = sizeof p367;
	p->Header.PacketId = 0x367;

	p->doubleCritical = 0;
	p->currentMp = 0;
	p->currentExp = -1;

	p->skillParm = 0;
	p->skillId = -1;
	p->reqMP = 0;
	p->Motion = -1;

	if(attackerId >= MAX_PLAYER)
	{
		INT32 _rand = Rand() % 100;
		if(pMob[attackerId].Mobs.Player.ShortSkill[0] != -1 && _rand >= 25 && _rand <= 40)
			p->skillId = pMob[attackerId].Mobs.Player.ShortSkill[0];
		else if(pMob[attackerId].Mobs.Player.ShortSkill[1] != -1 && _rand >= 0 && _rand <= 49)
			p->skillId = pMob[attackerId].Mobs.Player.ShortSkill[1];
		else if(pMob[attackerId].Mobs.Player.ShortSkill[2] != -1 && _rand >= 50 && _rand <= 84)
			p->skillId = pMob[attackerId].Mobs.Player.ShortSkill[2];
		else if(pMob[attackerId].Mobs.Player.ShortSkill[3] != -1 && _rand >= 85 && _rand <= 99)
			p->skillId = pMob[attackerId].Mobs.Player.ShortSkill[3];

		if(p->skillId != -1)
		{
			//Skill recuperar
			if(p->skillId == 29) 
			{
				INT32 leader = pMob[attackerId].Leader;
				if(leader <= 0)
					leader = attackerId;

				// Pega 5% do HP do leader
				INT32 heal = pMob[leader].Mobs.Player.CurrentScore.MaxHp  * 5 / 100;
				
				p->Target[0].Index = leader;
				p->Target[0].Damage = -heal;
				return;
			}
		}
	}

	INT32 total = 0;
	for(INT32 i = 0 ; i < 13; i++)
	{
		if(p->skillId == -1 && total == 1)
			break;
		
		INT32 mobId = target[i];
		if(mobId <= 0 || mobId >= MAX_PLAYER)
			continue;

		INT32 damage = 0;
		if(p->skillId != -1)
		{
			damage = GetSkillDamage_PvM(p->skillId, &pMob[attackerId], sServer.Weather, pMob[attackerId].WeaponDamage);

			damage = GetSkillDamage_2(damage, pMob[attackerId].Mobs.Player.CurrentScore.Ac, 15);
		}
		else
			damage = GetDamage(pMob[attackerId].Mobs.Player.CurrentScore.Damage, pMob[mobId].Mobs.Player.CurrentScore.Ac, 0);

		if(damage > 0 && mobId < MAX_PLAYER && pMob[mobId].ReflectDamage > 0)
		{
			damage -= pMob[mobId].ReflectDamage;

			if(damage < 0)
				damage = 0;
		}

		p->Target[total].Index = mobId;
		p->Target[total].Damage = damage;

		total ++;
	}
}

void GetAttack(int clientId, int mobId, p39D* p)
{
	memset(p, 0, sizeof p39D);

	p->Header.ClientId = 0x7531;
	p->attackerId = clientId;
	p->Header.TimeStamp = clock();

	p->attackerPos.X = pMob[clientId].Target.X;
	p->attackerPos.Y = pMob[clientId].Target.Y;
	
	p->targetPos.X = pMob[mobId].Target.X;
	p->targetPos.Y = pMob[mobId].Target.Y;

	//0041D27A
	p->Header.Size = sizeof p39D;
	p->Header.PacketId = 0x39D;

	p->doubleCritical = 0;
	p->currentMp = -1;
	p->currentExp = -1;

	p->Target.Damage = 0;

	//p->unknow_2 = -2;
	p->Target.Index = mobId;

	INT32 LOCAL_1 = -1;
	p->skillParm = 0;
	p->skillId = -1;

	p->Motion = Rand() % 3 + 4;

	if(clientId >= MAX_PLAYER)
	{
		INT32 LOCAL_2 = GetDistance(pMob[clientId].Target.X, pMob[clientId].Target.Y, pMob[mobId].Target.X, pMob[mobId].Target.Y);

		INT16 LOCAL_3;
		INT32 LOCAL_4;
		if(LOCAL_2 < 3)
		{
			LOCAL_3 = pMob[clientId].Mobs.Player.CurrentScore.Special[0];
			LOCAL_4 = pMob[clientId].Mobs.Player.CurrentScore.Special[1];
		}
		else
		{
			LOCAL_3 = pMob[clientId].Mobs.Player.CurrentScore.Special[2];
			LOCAL_4 = pMob[clientId].Mobs.Player.CurrentScore.Special[3];
		}

		p->Motion= 0;

		if((LOCAL_3 & 0xFF) == 0xFF)
			LOCAL_3 = 0xFF;

		// 0041D3B1
		//LOCAL_3 &= 0xFF;

		if(LOCAL_3 == 0x79)
		{
			p->skillId = 103;
			p->skillParm = 5;
		}
		else if(LOCAL_3 == 0x7A)
		{
			p->skillId = 0x69;
			p->skillParm = 1;
		}
		else if(LOCAL_3 == 0x7B)
		{
			p->skillId = 0x65;
			p->skillParm = 1;
		}
		else if(LOCAL_3 == 0x7C)
		{
			p->skillId = 0x65;
			p->skillParm = 2;
		}
		else if(LOCAL_3 == 0x7D)
		{
			p->skillId = 0x28;
			p->skillParm = 1;
		}
		else if(LOCAL_3 == 0x7E)
		{
			p->skillId = 0x28;
			p->skillParm = 2;
		}
		
		else if(LOCAL_3 == 0x7F)
		{
			p->skillId = 0x28;
			p->skillParm = 3;
		}
		else if(LOCAL_3 == 0x80)
		{
			p->skillId = 0x21;
			p->skillParm = 0xFC;
			p->Motion = 0xFC;
		}
		else
			p->skillId = LOCAL_3;

		if(LOCAL_4 != 0)
		{
			p->Motion = 4;

			INT32 LOCAL_5 = Rand() & 0x80000003;
			if(LOCAL_4 == 3)
			{
				if(LOCAL_5 <= 1)
					p->Motion++;
			}
			else if(LOCAL_4 == 6)
			{
				if(LOCAL_5 <= 1)
					p->Motion++;
				else
					p->Motion += 2;
			}
			else if(LOCAL_4 == 7)
			{
				if(LOCAL_5 == 1)
					p->Motion++;
				else
					p->Motion += 2;
			}
			else if(LOCAL_4 == 15)
			{
				if(LOCAL_5 == 1)
					p->Motion++;
				else if(LOCAL_5 == 2)
					p->Motion += 2;
				else if(LOCAL_5 == 3)
					p->Motion += 3;
			}
			else if(LOCAL_4 == 0x18)
			{
				if(LOCAL_5 <= 1)
					p->Motion += 3;
				else 
					p->Motion += 4;
			}
			else if(LOCAL_4 == 0x1B)
			{
				if(LOCAL_5 == 1)
					p->Motion ++;
				else if(LOCAL_5 == 2)
					p->Motion += 3;
				else if(LOCAL_5 == 3)
					p->Motion += 4;
			}
			else if(LOCAL_4 == 0x17)
			{
				if(LOCAL_5 == 1)
					p->Motion ++;
				else if(LOCAL_5 == 2)
					p->Motion += 2;
				else if(LOCAL_5 == 3)
					p->Motion += 4;
			}
		}

		INT32 LOCAL_6 = Rand() % 100;
		if(pMob[clientId].Mobs.Player.ShortSkill[3] != -1 && LOCAL_6 >= 0x19 && LOCAL_6 <= 40)
		{// skillData base = 7DCDD8
			INT32 LOCAL_7 = pMob[clientId].Mobs.Player.ShortSkill[3];
			INT32 LOCAL_8 = SkillData[LOCAL_7].InstanceType;
			INT32 LOCAL_9 = pMob[clientId].Leader;

			if(LOCAL_9 <= 0)
				LOCAL_9 = clientId;

			INT32 LOCAL_10 = pMob[clientId].Mobs.Player.CurrentScore.Hp;
			LOCAL_10 = LOCAL_10 * 10;
			LOCAL_10 = LOCAL_10 / pMob[clientId].Mobs.Player.CurrentScore.MaxHp;
			INT32 LOCAL_11 = pMob[LOCAL_9].Mobs.Player.CurrentScore.Hp;
			LOCAL_11 = LOCAL_11 * 10;
			LOCAL_11 = LOCAL_11 / pMob[clientId].Mobs.Player.CurrentScore.MaxHp;

			if(LOCAL_8 == 6 && LOCAL_10 <= 8 && LOCAL_11 <= 8)
			{
				p->skillId = LOCAL_7;

				INT32 LOCAL_12 = clientId;
				if(LOCAL_10 > LOCAL_11)
					LOCAL_12 = LOCAL_9;

				INT32 LOCAL_13 = pMob[LOCAL_12].Mobs.Player.CurrentScore.Hp / 10;

				p->Target.Index = LOCAL_12;
				p->Target.Damage = -LOCAL_13;

				return;
			}
		}
		
		if(pMob[clientId].Mobs.Player.ShortSkill[0] != -1 && LOCAL_6 >= 0 && LOCAL_6 <= 25)
		{
			INT32 LOCAL_14 = pMob[clientId].Mobs.Player.ShortSkill[0];
			INT32 LOCAL_15 = SkillData[LOCAL_14].InstanceType;

			LOCAL_1 = LOCAL_15 - 2;

			p->skillId = LOCAL_14;
		}
		else if(pMob[clientId].Mobs.Player.ShortSkill[1] != -1 && LOCAL_6 >= 26 && LOCAL_6 <= 50)
		{
			INT32 LOCAL_16 = pMob[clientId].Mobs.Player.ShortSkill[1];
			INT32 LOCAL_17 = SkillData[LOCAL_16].InstanceType;

			LOCAL_1 = LOCAL_17 - 2;

			p->skillId = LOCAL_16;
		}
		else if(pMob[clientId].Mobs.Player.ShortSkill[2] != -1 && LOCAL_6 >= 51 && LOCAL_6 <= 75)
		{
			INT32 LOCAL_16 = pMob[clientId].Mobs.Player.ShortSkill[2];
			INT32 LOCAL_17 = SkillData[LOCAL_16].InstanceType;

			LOCAL_1 = LOCAL_17 - 2;

			p->skillId = LOCAL_16;
		}
		else if(pMob[clientId].Mobs.Player.ShortSkill[3] != -1 && LOCAL_6 >= 51 && LOCAL_6 <= 75)
		{
			INT32 LOCAL_16 = pMob[clientId].Mobs.Player.ShortSkill[3];
			INT32 LOCAL_17 = SkillData[LOCAL_16].InstanceType;

			LOCAL_1 = LOCAL_17 - 2;

			p->skillId = LOCAL_16;
		}
	}

	INT32 LOCAL_20 = pMob[clientId].Mobs.Player.CurrentScore.Damage;

	LOCAL_20 = GetDamage(LOCAL_20, pMob[mobId].Mobs.Player.CurrentScore.Ac, 0);

	if(LOCAL_1 >= 0 && LOCAL_1 <= 3)
		LOCAL_20 = LOCAL_20 * (100 - *(BYTE*)(&pMob[mobId].Mobs.Player.Resist + LOCAL_1)) / 100;

	if(LOCAL_20 > 0 && mobId < MAX_PLAYER && pMob[mobId].ReflectDamage > 0)
	{
		LOCAL_20 -= pMob[mobId].ReflectDamage;

		if(LOCAL_20 < 0)
			LOCAL_20 = 0;
	}

	if(LOCAL_20 > 0)
	{
		if(clientId < MAX_PLAYER || pMob[clientId].Mobs.Player.CapeInfo == 4)
		{
			if(mobId < MAX_PLAYER || pMob[mobId].Mobs.Player.CapeInfo == 4)
			{
				LOCAL_20 = LOCAL_20 * 3 / 10;
				
				eMapAttribute LOCAL_21 = GetAttribute(pMob[mobId].Target.X, pMob[mobId].Target.Y);
				eMapAttribute LOCAL_22 = GetAttribute(pMob[clientId].Target.X, pMob[clientId].Target.Y);

				if(LOCAL_21.Village || !LOCAL_21.PvP)
					LOCAL_20 = 0;

				if(LOCAL_22.Village || !LOCAL_22.PvP)
					LOCAL_20 = 0;
			}
		}
		
		INT32 LOCAL_23 = pMob[clientId].Leader;
		if(LOCAL_23 == 0)
			LOCAL_23 = clientId;

		INT32 LOCAL_24= pMob[mobId].Leader;
		if(LOCAL_24 == 0)
			LOCAL_24 = mobId;

		INT32 LOCAL_25 = pMob[clientId].Mobs.Player.Guild; 
		INT32 LOCAL_26 = pMob[mobId].Mobs.Player.Guild;    // quem esta sendo atacado
		INT32 ally = g_pGuildAlly[LOCAL_25];
		if(ally == 0)
			ally = -1;

		if(LOCAL_25 == 0 && LOCAL_26 == 0)
			LOCAL_25 = -1;

		if(mobId < MAX_PLAYER && pMob[mobId].GuildDisable)
			LOCAL_26 = -2;

		if (pMob[LOCAL_23].Target.X >= 1041 && pMob[LOCAL_23].Target.X <= 1248 &&
			pMob[LOCAL_23].Target.Y >= 1950 && pMob[LOCAL_23].Target.Y <= 2158 && sServer.RvR.CurrentScore == 1)
		{ // se estiver dentro da area
			if(pMob[LOCAL_23].Mobs.Player.CapeInfo == pMob[LOCAL_24].Mobs.Player.CapeInfo)
			{
				LOCAL_20 = 0;
				pMob[clientId].CurrentTarget = 0;
			}
		}

		if(LOCAL_23 == LOCAL_24 || LOCAL_25 == LOCAL_26 || ally == LOCAL_26)
		{
			LOCAL_20 = 0;
			pMob[clientId].CurrentTarget = 0;
		}
	}

	p->Target.Damage = LOCAL_20;
}

STRUCT_ITEM *GetItemPointer(int clientIndex, int invType, int invSlot)
{
	if(clientIndex <= 0 || clientIndex >= MAX_MOB)
		return NULL;

	switch(invType)
	{
		case 0:
			return &pMob[clientIndex].Mobs.Player.Equip[invSlot];
		case 1:
			return &pMob[clientIndex].Mobs.Player.Inventory[invSlot];
		case 2:
		{
			if (clientIndex < MAX_PLAYER)
				return &pUser[clientIndex].User.Storage.Item[invSlot];

			return NULL;
		}
	}

	return NULL;
}

bool GetEmptyItemGrid(int *posX, int *posY)
{
    if(g_pItemGrid[*posY][*posX] == 0)
        if(g_pHeightGrid[*posY][*posX] != 127)
            return true;

    for(int nY = (*posY - 1); nY <= (*posY + 1); nY++)
    {
        for(int nX = (*posX - 1); nX <= (*posX + 1); nX++)
        {
            if(nX < 0 || nY < 0 || nX >= 4096 || nY >= 4096)
                continue;

            if(g_pItemGrid[nY][nX] == 0)
            {
                if(g_pHeightGrid[nY][nX] != 127)
                {
                    *posX = nX;
                    *posY = nY;
                    return true;
                }
            }
        }
    }

    return false;
}

void GetCreateItem(int sIndex, p26E *p)
{
	const STRUCT_INITITEM *init = &g_pInitItem[sIndex];

	p->Header.Size = sizeof p26E;
	p->Header.PacketId = 0x26E;
	p->Header.ClientId = 0x7530;

	p->Init.X = init->PosX;
	p->Init.Y = init->PosY;

	p->sIndex = (sIndex + 10000);

	memcpy(&p->Item, &init->Item, 8);

	p->Rotation = init->Rotation;
	p->CurrentScore = init->CurrentScore;
	p->Unknow = 0;
	p->HeightGrid = init->HeightGrid;
}
 
INT32 GetUserByName(char *name)
{
	for(INT32 i = 1; i < MAX_PLAYER; i++)
	{
		if(pUser[i].Status != USER_PLAY)
			continue;

		if(!strcmp(pMob[i].Mobs.Player.MobName, name))
			return i;
	}

	return 0;
}

INT32 GetEmptyItem()
{
	for(int i = 1; i < 4096; i++)
	{
		if(g_pInitItem[i].Open == 0)
			return i;
	}

	return 0;
}
 
long long GetExpApply(long long exp, int attackerId, int targetId)
{
	CMob *spwAttacker = &pMob[attackerId];
	CMob *spwTarget   = &pMob[ targetId ];
	
	long long attacker = spwAttacker->Mobs.Player.CurrentScore.Level;
	long long target = spwTarget->Mobs.Player.CurrentScore.Level;
	long long ev = spwAttacker->Mobs.Player.Equip[0].EFV2;
	if((ev <= ARCH && attacker >= MAX_LEVEL) || (ev >= CELESTIAL && attacker >= MAX_LEVEL_CELESTIAL))
		return 0;

	if(attacker >= 400 || target >= 400 || attacker < 0 || target < 0)
		return exp;

	attacker++;
	target++;

	if (ev >= CELESTIAL)
		attacker += (400 + (ev - ARCH) * 50);

	long long	multiexp = target * 100 / attacker;
	if(multiexp < 80 && attacker >= 50)
		multiexp = ((multiexp  - 80) * 2) + 100;
	else if(multiexp > 200)			
		multiexp = 200;
	else if(multiexp < 0)		
		multiexp = 0;

	exp = exp * multiexp / 100;
	if(exp <= 0)
		return 0;

	INT32 posX = spwAttacker->Target.X,
		  posY = spwAttacker->Target.Y;
	
	if(posX >= 1152 && posX <= 1282 && posY >= 130 && posY <= 217)
	{
		INT32 count = 0;
		for(INT32 x = 0; x < 8; x++)
		{
			if(sServer.Nightmare[2].Alive[x])
				continue;
			
			count ++;
		}		
		
		if(ev == MORTAL)
			exp += exp * 20ULL / 100ULL;
		else if(ev == ARCH)
			exp += exp * 15ULL / 100ULL;

		count = 8 - count;

		if(count != 0)
			exp -= static_cast<long long>((exp * (((count * 7) / 0.70f) / 100.0f)));
	}
	else if(posX >= 1049 && posX <= 1130 && posY >= 272 && posY <= 334)
	{
		INT32 count = 0;
		for(INT32 x = 0; x < 8; x++)
		{
			if(sServer.Nightmare[1].Alive[x])
				continue;
			
			count ++;
		}		
		
		if(ev == MORTAL)
			exp += exp * 20ULL / 100ULL;
		else if(ev == ARCH)
			exp += exp * 15ULL / 100ULL;

		count = 8 - count;

		if(count != 0)
			exp -= static_cast<long long>((exp * (((count * 7) / 0.70) / 100)));
	}

	if(spwAttacker->ExpBonus != 0)
		exp = exp + (exp * static_cast<long long>(spwAttacker->ExpBonus) / 100);

	if (sServer.KefraDead)
		exp += (exp / 100ULL) * 50ULL;

	return exp ;
}

long long GetExpApply_2(long long exp, int receiver, int attackerId, bool useBoxExp)
{
	if(exp == 0)
		return 0;
	
	long long level = pMob[receiver].Mobs.Player.CurrentScore.Level;
	eClass classMaster = pMob[receiver].Mobs.Player.GetEvolution();

	auto checkIsBlocked = [](int receiver) -> bool {
		long long level = pMob[receiver].Mobs.Player.CurrentScore.Level;
		eClass classMaster = pMob[receiver].Mobs.Player.GetEvolution();

		bool isBlocked = false;
		bool isBlockedAndNeedToCompleteExperience = false;
		if (pMob[receiver].Mobs.Info.LvBlocked)
			return (classMaster >= eClass::Celestial && (level == 39 || level == 89 || level == 199 || level == 209)) || (classMaster == eClass::Arch && (level == 354 || level == 369));

		return false;
	};


	if (checkIsBlocked(attackerId))
		return 0;

	bool isBlocked = checkIsBlocked(receiver);
	if (classMaster >= eClass::Celestial && level >= MAX_LEVEL_CELESTIAL)
		return 0;

	// Somente batedores celestiais ou superior podem dar experiência para celestiais ou superior 200+
	auto attackerClassMaster = pMob[attackerId].Mobs.Player.GetEvolution();
	if (classMaster >= CELESTIAL && level >= 199 && attackerClassMaster < eClass::Celestial)
		return 0;

	if (classMaster >= CELESTIAL && pMob[receiver].Mobs.GetTotalResets() < 3 && level == 199)
		return 0;

	if (isBlocked)
		return 0;

	if (classMaster == eClass::Celestial)
		exp -= ((exp * (level + 100LL) / 10LL) / 100LL);
	else if (classMaster == eClass::SubCelestial)
		exp -= ((exp * (level + 200LL) / 10LL) / 100LL);

	if (classMaster == eClass::Mortal)
		exp += (exp * 100LL / 100LL);
	else if (classMaster == eClass::Arch)
	{
		if (level <= 320)
			exp = exp;
		else if (level <= 354)
			exp /= 2LL;
		else
			exp /= 4LL;
	}
	else if (classMaster == eClass::Celestial)
	{
		if (level < 39)
			exp = exp / 10LL;
		else if (level < 89)
			exp = exp / 20LL;
		else if (level < 170)
			exp = exp / 50LL;
		else if (level < 181)
			exp = exp / 200LL;
		else if(level < 199)
			exp = exp / 250LL;
		else if (level < 206)
			exp = exp / 450LL;
		else
			exp = exp / 900LL;

	}
	else if (classMaster == eClass::SubCelestial)
	{
		if (level < 39)
			exp = exp / 25LL;
		else if (level < 89)
			exp = exp / 35LL;
		else if (level < 120)
			exp = exp / 50LL;
		else if (level < 170)
			exp = exp / 70LL;
		else if (level < 194)
			exp = exp / 200LL;
		else if (level < 199)
			exp = exp / 350LL;
		else if (level < 206)
			exp = exp / 450LL;
		else
			exp = exp / 900LL;
	}

	auto posX = pMob[receiver].Target.X;
	auto posY = pMob[receiver].Target.Y;
	if ((posX >= 1049 && posX <= 1130 && posY >= 272 && posY <= 334) ||
		(posX >= 1152 && posX <= 1282 && posY >= 130 && posY <= 217))
	{
		if (classMaster >= eClass::Celestial && level >= 120)
			exp = exp * 60LL / 100LL;
	}

	if (useBoxExp)
	{
		for (INT32 i = 0; i < 32; i++)
		{
			if (pMob[attackerId].Mobs.Affects[i].Index == 39) // baê de experiência
			{
				exp *= 2LL;
				break;
			}
		}
	}

	if ((classMaster == eClass::Celestial && pMob[receiver].Mobs.Player.BaseScore.Level >= 64) || classMaster >= eClass::SubCelestial)
	{
		for (int t = 0; t < 9; t++)
		{
			if (pMob[receiver].Target.X >= waterMaxMin[1][t][0] && pMob[receiver].Target.X <= waterMaxMin[1][t][2] && pMob[receiver].Target.Y >= waterMaxMin[1][t][1] && pMob[receiver].Target.Y <= waterMaxMin[1][t][3])
			{
				exp /= 4LL;

				break;
			}
		}
	}

	if (classMaster >= eClass::Celestial && pMob[receiver].Mobs.Player.BaseScore.Level >= 199)
	{
		for (int t = 0; t < 9; t++)
		{
			if (pMob[receiver].Target.X >= waterMaxMin[2][t][0] && pMob[receiver].Target.X <= waterMaxMin[2][t][2] && pMob[receiver].Target.Y >= waterMaxMin[2][t][1] && pMob[receiver].Target.Y <= waterMaxMin[2][t][3])
			{
				exp /= 4LL;

				break;
			}
		}
	}

	bool onNightmareArea{ false };
	for (int i = 0; i < 3; ++i)
	{
		if (pMob[attackerId].Target.X >= g_pPesaArea[i][0] && pMob[attackerId].Target.X <= g_pPesaArea[i][2] && pMob[attackerId].Target.Y >= g_pPesaArea[i][1] && pMob[attackerId].Target.Y <= g_pPesaArea[i][3])
		{
			onNightmareArea = true;

			break;
		}
	}

	if (!onNightmareArea && sServer.BonusEXP != 0)
		exp += (exp * sServer.BonusEXP / 100LL);

 

	exp += GetFairyExpBonus(exp, pMob[attackerId].Mobs.Player.Equip[13].sIndex);

	if (pMob[receiver].IndividualExpBonus > 0 && pMob[receiver].IndividualExpBonus < 100)
		exp += (exp * static_cast<long long>(pMob[receiver].IndividualExpBonus) / 100LL);

	return exp;
}

unsigned long long GetFairyExpBonus(unsigned long long exp, INT32 fadaId)
{
	if (fadaId == 3900 || fadaId == 3906 || fadaId == 3914)
		exp = (exp * 16 / 100);
	else if (fadaId == 3915)
		exp = exp * 18 / 100;
	else if(fadaId == 3902 || fadaId == 3908)
		exp = (exp * 32 / 100);
	else
		exp = 0;

	return exp;
}

INT32 GetManaSpent(INT32 skillId, INT32 saveMana, INT32 mastery)
{
	INT32 manaSpent = SkillData[skillId].Mana; // -4

	manaSpent = (((mastery >> 1) + 100) * manaSpent) / 100;
	manaSpent = ((100 - saveMana) * manaSpent) / 100;

	return manaSpent;
}

INT32 GetDoubleCritical(CMob *arg1, short *arg2, short *arg3, unsigned char *arg4)
{
	*arg4 = 0;

	if (arg3 == 0)
		return 0;

	if (*arg3 >= 1024)
		*arg3 = *arg3 & 0x800003FF;

	if (arg2 != 0 && *arg2 >= 1024)
		*arg2 = *arg2 & 0x800003FF;

	INT32 LOCAL_1 = 1;
	INT32 LOCAL_3[2] = { (((arg1->AttackSpeed >> 4) - 5) * 100), arg1->Mobs.Player.Critical };
	INT32 LOCAL_4;
	if (arg2 != 0 && arg3 != 0 && *arg3 != *arg2)
	{
		*arg2 = *arg3;
		LOCAL_4 = *arg3;

		if (LOCAL_4 < *arg2)
			LOCAL_4 += 256;

		if (LOCAL_4 < *arg2 && LOCAL_4 < *arg2 + 5)
			LOCAL_1 = 1;
	}

	INT32 LOCAL_5 = g_pHitRate[*arg3];
	for (INT32 LOCAL_6 = 0; LOCAL_6 < 2; LOCAL_6++)
	{
		INT32 LOCAL_7 = 0;
		if (LOCAL_6 == 0 && LOCAL_5 < LOCAL_3[LOCAL_6])
			LOCAL_7 = 1;

		if (LOCAL_6 == 1)
		{
			if ((Rand() % 250) < LOCAL_3[LOCAL_6])
				LOCAL_7 = 1;
		}

		*arg4 = *arg4 | (LOCAL_7 << LOCAL_6);
	}

	if (arg2 != 0)
		*arg2 = *arg2 + 1;

	*arg3 = *arg3 + 1;

	return LOCAL_1;
}
INT32 GetItemAmount(STRUCT_ITEM *item)
{
	INT32 amount = 1;
	
	for(INT8 i = 0; i < 3; i++)
	{
		if(item->stEffect[i].cEffect == EF_AMOUNT)
		{
			amount = item->stEffect[i].cValue;
			if(amount == 0)
				amount = 1;

			break;
		}
	}

	return amount;
}

int GetUserInArea(unsigned int x1, unsigned int y1, unsigned int x2, unsigned int y2, char* first)
{ //401348
	INT32 LOCAL_1 = 0,
		  LOCAL_2 = 1;

	for(; LOCAL_2 < MAX_PLAYER; LOCAL_2 ++)
	{
		if(pUser[LOCAL_2].Status != USER_PLAY)
			continue;

		if(pMob[LOCAL_2].Mode == 0)
			continue;

		if(pMob[LOCAL_2].Target.X < x1 || pMob[LOCAL_2].Target.X > x2 || pMob[LOCAL_2].Target.Y < y1 || pMob[LOCAL_2].Target.Y > y2)
			continue;

		if(LOCAL_1 == 0)
			strncpy_s(first, 16, pMob[LOCAL_2].Mobs.Player.MobName, 12);

		LOCAL_1++;
	}

	return LOCAL_1;
}

INT32 GetInventoryAmount(int clientId, int itemId)
{
	STRUCT_ITEM *item = pMob[clientId].Mobs.Player.Inventory;
	
	INT32 total = 0;
	for(INT32 i = 0; i < 30; i++)
	{
		if(item[i].sIndex == itemId)
			total += GetItemAmount(&item[i]);
	}
	
	double bolsa1 = TimeRemaining(item[60].EFV1, item[60].EFV2, item[60].EFV3 + 1900);
	double bolsa2 = TimeRemaining(item[61].EFV1, item[61].EFV2, item[61].EFV3 + 1900);

	if(bolsa1 > 0.0f)
	{
		for(INT32 i = 30; i < 45; i++)
		{
			if(item[i].sIndex == itemId)
				total += GetItemAmount(&item[i]);
		}
	}

	if(bolsa2 > 0.0f)
	{
		for(INT32 i = 45; i < 60; i++)
		{
			if(item[i].sIndex == itemId)
				total += GetItemAmount(&item[i]);
		}
	}

	return total;
}

void GetGuild(int clientId)
{
	STRUCT_ITEM *LOCAL_1 = &pMob[clientId].Mobs.Player.Equip[12];

/*	if(pMob[clientId].Mobs.MedalId == 509)
		pMob[clientId].Mobs.Player.GuildLevel = 9;
	else if(pMob[clientId].Mobs.MedalId >= 526 && pMob[clientId].Mobs.MedalId <= 531)
		pMob[clientId].Mobs.Player.GuildLevel = (3 + pMob[clientId].Mobs.MedalId - 526);
	else if(pMob[clientId].Mobs.MedalId == 508)
		pMob[clientId].Mobs.Player.GuildLevel = 1;
	else 
		pMob[clientId].Mobs.Player.GuildLevel = 0;*/
}

unsigned int GetWeekNumber()
{
	time_t now;
	time(&now);

	unsigned int week = 86400;

	__int64 ret = static_cast<unsigned int>(now) / week - 3;

	return static_cast<unsigned int>(ret);
}

UINT8 DayOfWeek()
{
	// 0 = segunda .... 6 = domingo
	static const int szTable[12] = {-1, 2, 2, 5, 0, 3, 5, 1, 4, -1, 2, 4};
    time_t rawnow = time(NULL);
    struct tm now; localtime_s(&now, &rawnow);
	int cDay = now.tm_mday;
	int cMon = now.tm_mon;
	int cYear = now.tm_year;

	return (cYear + ((cYear / 4) + szTable[cMon] + (cDay - 1))) % 7;
}

INT32 GetInHalf(INT32 clientId, INT32 mobId)
{ // 401159
	if (pMob[mobId].Target.X - HALFGRIDX > pMob[clientId].Target.X)
		return false;

	if (pMob[mobId].Target.X + HALFGRIDX < pMob[clientId].Target.X)
		return false;

	if (pMob[mobId].Target.Y - HALFGRIDY > pMob[clientId].Target.Y)
		return false;

	if (pMob[mobId].Target.Y + HALFGRIDY < pMob[clientId].Target.Y)
		return false;

	return true;
}

INT32 GetBonusSet(STRUCT_MOB *player, INT32 defense)
{
	INT32 start = -1;
	STRUCT_ITEM *item;
	if(player->Equip[0].EFV2 >= 2) 
	{
		start = 3;
		item = (&player->Equip[2]);
	}
	else
	{
		start = 2;
		item = &player->Equip[1];
	}

	// O primeiro item ja nêo existe, logo, nêo ha porque aplicar o bênus
	if(item->sIndex <= 0 || item->sIndex >= MAX_ITEMLIST)
		return 0;

	INT32 mobType = GetEffectValueByIndex(item->sIndex, EF_MOBTYPE),
		  _class  = GetEffectValueByIndex(item->sIndex, EF_CLASS),
		  letter  = GetEffectValueByIndex(item->sIndex, EF_UNKNOW1);

	INT32 i;
	for(i = start; i < 6; i++)
	{
		STRUCT_ITEM *tmpItem = &player->Equip[i];
		if(tmpItem->sIndex <= 0 || tmpItem->sIndex >= 6500)
			break;

		if(mobType != GetEffectValueByIndex(tmpItem->sIndex, EF_MOBTYPE) ||
			_class != GetEffectValueByIndex(tmpItem->sIndex, EF_CLASS) ||
			letter != GetEffectValueByIndex(tmpItem->sIndex, EF_UNKNOW1))
			break;
	}

	if(i == 6)
		return (defense * 5 / 100);

	return 0;
}

int GetStaticItemAbility(STRUCT_ITEM *item, unsigned char Type)
{
	int value = 0;
	int idx = item->sIndex;

	if(idx <= 0 || idx > MAX_ITEMLIST)
		return value;

	int nPos = g_pItemList[idx].Pos;

	if(Type == EF_LEVEL && idx >= 2330 && idx < 2360)
		value = item->stEffect[1].cEffect - 1;
	else if(Type == EF_LEVEL)
		value += g_pItemList[idx].Level;

	if(Type == EF_REQ_STR)
		value += g_pItemList[idx].Str;

	if(Type == EF_REQ_INT)
		value += g_pItemList[idx].Int;

	if(Type == EF_REQ_DEX)
		value += g_pItemList[idx].Dex;

	if(Type == EF_REQ_CON)
		value += g_pItemList[idx].Con;

	if(Type == EF_POS)
		value += g_pItemList[idx].Pos;

	if(Type != EF_INCUBATE)
	{
		for(int i = 0; i < 12; i++)
		{
			if(g_pItemList[idx].stEffect[i].sEffect != Type)
                		continue;

			int tvalue = g_pItemList[idx].stEffect[i].sValue;

            if(Type == EF_ATTSPEED && tvalue == 1)
                tvalue = 10;

            value += tvalue;
		}
	}

	if(Type == EF_RESIST1 || Type == EF_RESIST2 || Type == EF_RESIST3 || Type == EF_RESIST4)
	{
		for(int i = 0; i < 12; i++)
		{
			if(g_pItemList[idx].stEffect[i].sEffect == EF_RESISTALL)
				value += g_pItemList[idx].stEffect[i].sValue;
		}

		for(int i = 0; i < 3; i++)
		{
			if(item->stEffect[i].cEffect == EF_RESISTALL)
				value += item->stEffect[i].cValue;
		}
	}

	if(idx >= 2330 && idx < 2390)
	{
		if(Type == EF_MOUNTHP)
			return item->stEffect[0].cValue;

		else if(Type == EF_MOUNTSANC)
			return item->stEffect[1].cEffect;

		else if(Type == EF_MOUNTLIFE)
			return item->stEffect[1].cValue;

		else if(Type == EF_MOUNTFEED)
			return item->stEffect[2].cEffect;

		else if(Type == EF_MOUNTKILL)
			return item->stEffect[2].cValue;

		if(idx < 2362 || idx >= 2390 || item->stEffect[0].cValue <= 0)
			return value;

		int lv = item->stEffect[1].cEffect;
		int cd = item->sIndex - 2360;

		if(Type == EF_DAMAGE)
			return (lv + 20) * mMont[cd].atkFisico / 100; //Retorna o DN da montaria no level atual

		else if(Type == EF_MAGIC)
			return (lv + 15) * mMont[cd].atkMagico / 100;

		else if(Type == EF_PARRY)
			return mMont[cd].Evasion;

		else if(Type == EF_RESIST1 || Type == EF_RESIST2 || Type == EF_RESIST3 || Type == EF_RESIST4)
			return mMont[cd].Resist;
		else
			return value;
	}

	int sanc = GetItemSanc(item);

	if(sanc == 9 && (nPos & 0xF00) != 0)
		sanc = 10;

	if(Type != EF_GRID && Type != EF_CLASS && Type != EF_POS && Type != EF_WTYPE && Type != EF_RANGE && Type != EF_LEVEL && 
		Type != EF_REQ_STR && Type != EF_REQ_INT && Type != EF_REQ_DEX && Type != EF_REQ_CON && Type != EF_INCUBATE && Type != EF_INCUDELAY)
	{
		value *= sanc + 10;
		value /= 10;
	}

	if(Type == EF_RUNSPEED)
	{
		if(value >= 3)
			value = 2;

		if(value > 0 && sanc == 9)
			value++;
	}

	if(Type == EF_HWORDGUILD || Type == EF_LWORDGUILD)
	{
		unsigned char v = value;

		value = v;
	}

	return value;
}

INT32 GetCompounderDeal(INT32 clientId)
{
	INT32 slotId = GetFirstSlot(clientId, 4529);
	if(slotId == -1)
		return 0;

	AmountMinus(&pMob[clientId].Mobs.Player.Inventory[slotId]);
	SendItem(clientId, SlotType::Inv, slotId, &pMob[clientId].Mobs.Player.Inventory[slotId]);

	INT32 value = ((Rand() % 5) + 15);
	Log(clientId, LOG_INGAME, "Utilizado o item Escritura de Composição - %d", value);
	return value;
}

INT32 getWeek(INT32 day, INT32 month, INT32 year) 
{
	time_t rawnow = time(NULL);
	struct tm now;
	localtime_s(&now, &rawnow);
	
	char buffer[16];
	strftime (buffer,4,"%W", &now);   // '%W' = week number of the year, eg 1/1/09 == 1
	
	return (atoi(buffer));
}

std::tuple<int, int> GetPriceAndImpost(STRUCT_ITEMLIST* item, int city)
{
	int itemPrice = item->Price;
	int perc_impost = g_pCityZone[city].perc_impost;
	int impost = 0;
	if (perc_impost != 0)
		impost = (itemPrice * perc_impost / 100);

	return std::make_tuple(itemPrice, impost);
}

 

int GetDamageByJewel(int clientId, int damage)
{
	if (clientId < MAX_PLAYER)
		return damage;

	INT32 itemId = pMob[clientId].Mobs.Player.Equip[13].sIndex;
	if (itemId == 786 || itemId == 1936 || itemId == 1937)
	{
		INT32 sanc = GetItemSanc(&pMob[clientId].Mobs.Player.Equip[13]); // local209
		if (sanc < 2)
			sanc = 2;

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

		multHP *= sanc;
		damage /= multHP;
	}

	return damage;
}

int ReturnChance(STRUCT_ITEM *item)
{
	constexpr std::array refs{ 2, 3, 4, 5, 6, 7 , 8, 10, 12, 15 };
	if (item->EF1 == 43)
		return refs[item->EFV1];
	else if (item->EF2 == 43)
		return refs[item->EFV2];
	else if (item->EF3 == 43)
		return refs[item->EFV3];

	return 2;
}

int GetMaxAmountItem(const STRUCT_ITEM* item)
{
	switch (item->sIndex)
	{
	case 412:
	case 413:
	case 3200:
	case 3201:
	case 3202:
	case 3203:
	case 3204:
	case 3205:
	case 3206:
	case 3207:
	case 3208:
	case 3214:
	case 3209:
	case 2390:
	case 2391:
	case 2392:
	case 2393:
	case 2394:
	case 2395:
	case 2396:
	case 2397:
	case 2398:
	case 2399:
	case 2400:
	case 2401:
	case 2402:
	case 2403:
	case 2404:
	case 2405:
	case 2406:
	case 2407:
	case 2408:
	case 2409:
	case 2410:
	case 2411:
	case 2412:
	case 2413:
	case 2414:
	case 2415:
	case 2416:
	case 2417:
	case 2418:
	case 2419:
	case 415:
	case 419:
	case 420:
	case 4599:
	case 4601:
	case 4603:
	case 4547:
	case 4528:
	case 1739:
	case 4641:
	case 4719:
		return 120;

	case 4140:
	case 4548:
	case 4549:
	case 3314:
	case 4016:
	case 4017:
	case 4018:
	case 4019:
	case 4020:
	case 777:
	case 3182:
	case 3252:
	case 3253:
		return 120;

	case 4850:
		return 150;
	}

	return 0;
}

bool IsWarTime()
{
	return (sServer.TowerWar.CurrentScore || sServer.WeekMode == 3 || sServer.ForceWeekDay == 3 || sServer.CastleState != 0);

}

int GetSpiritHPMPBonus(int sanc)
{
	if (sanc == 0)
		sanc = 1;

	if (sanc > 9)
		sanc = 9;

	return 10 * sanc;
}

int GetSpiritRessBonus(int sanc)
{
	if (sanc == 0)
		sanc = 1;

	if (sanc > 9)
		sanc = 9;

	int total = 10 * sanc;
	if (total > 60)
		total = 60;

	return total;
}

bool IsCostume(const STRUCT_ITEM* item)
{
	return (item->sIndex >= 4151 && item->sIndex <= 4189) || (item->sIndex >= 4210 && item->sIndex <= 4229) || (item->sIndex >= 4230 && item->sIndex <= 4247);
}

std::vector<CUser*> GetSameMACUsers(const CUser& thisUser, std::function<bool(CUser& user)> function)
{
	std::vector<CUser*> users;
	for (auto& user : pUser)
	{
		if (&thisUser == &user || user.Status != USER_PLAY || memcmp(user.MacAddress, thisUser.MacAddress, 8) != 0 || (function != nullptr && !function(user)))
			continue;

		users.push_back(&user);
	}

	return users;
}

eValley GetValleyWithMinimum()
{
	std::array<int, 2> total{ 0, 0 };

	for (const auto& user : pUser)
	{
		if (user.Status != USER_PLAY)
			continue;

		auto& mob = pMob[user.clientId];
		if (mob.Target.X >= 2176 && mob.Target.X <= 2304 && mob.Target.Y >= 3585 && mob.Target.Y <= 3711)
			total[0]++;
		else if (mob.Target.X >= 2176 && mob.Target.X <= 2304 && mob.Target.Y >= 3712 && mob.Target.Y <= 3839)
			total[1]++;
	}

	return total[0] > total[1] ? eValley::First : eValley::Second;
}

std::string GetEvolutionName(eClass evolution)
{
    constexpr std::array evolutionName = { "Unknown", "Mortal", "Arch", "Celestial", "Subcelestial" };

    if (evolution > eClass::SubCelestial || evolution < eClass::Mortal)
        return evolutionName[0];

    return evolutionName[evolution];
}