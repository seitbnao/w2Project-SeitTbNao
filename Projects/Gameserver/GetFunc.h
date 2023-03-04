#include <Windows.h>
#include "cServer.h"

#ifndef __GETFUNC_H__
#define __GETFUNC_H__

#include <tuple>
#include <array>
#include <functional>

void GetCreateMob(int clientId, BYTE *bufPak);
void GetCreateMobTrade(int clientId, BYTE *bufPak);

int GetAnctCode(STRUCT_ITEM *item, bool usingCostume);
int GetItemIDAndEffect(STRUCT_ITEM *Item, int mnt, bool usingCostume);

// Procura um lugar vazio dentro de uma area para o usuario
bool GetEmptyMobGrid(int sIndex, unsigned int *posX, unsigned int *posY);
bool GetEmptyItemGrid(int *posX, int *posY);

// Cria o pacote de movimento
void GetAction(int clientId, short posX, short posY, void *buf);

// Pega a refinação atual do item
int GetItemSanc(STRUCT_ITEM *item);

// Pega a classe do usuario baseado na face
int GetInfoClass (int face) ;


bool IsImpossibleToRefine(STRUCT_ITEM* item);

// Pega o valor do efeito baseado no sIndex 
short GetEffectValueByIndex(int ItemID, int sIndex);

eMapAttribute GetAttribute(int posX, int posY);

// Pega a velocidade atual do mob
int GetSpeed(STRUCT_SCORE *status);

// Pega a distância do usuario
int GetDistance(int x1,int y1,int x2,int y2);

// Pega a rota do usuario
INT32 GetRoute(unsigned int x, unsigned int y, unsigned int *targetX, unsigned int *targetY, char *route, int distance);
//INT32 GetRoute(int arg1,int arg2,int *arg3,int *arg4,char *arg5,int arg6,char *arg7);
// Dano do usuario
int GetDamage(int dam, int ac, int combat) ;
INT32 GetSkillDamage_PvP(INT32 skillId, CMob*mob, INT32 weather, INT32 weaponDamage);
INT32 GetSkillDamage_PvM(INT32 skillId, CMob*mob, INT32 weather, INT32 weaponDamage);
INT32 GetSkillDamage_2(INT32 damage, INT32 defenserAC, INT32 master);

// Evasão do usuario
INT32 GetParryRate(int clientId, int mobId, int type);

// Procura o slot vago nos affects
int GetEmptyAffect(int mobId, int buffId);

// Pk Point do usuario
int GetPKPoint(int sIndex);

// Pega a cidade e a arena
int GetVillage(unsigned int x, unsigned int y);
int GetArena(unsigned int x, unsigned int y);


int GetTotKill(int sIndex);
int GetGuilty(int sIndex);
int GetCurKill(int sIndex);

void GetHitPosition(int arg1,int arg2,int *arg3,int *arg4);

void GetAttack(int clientId, int mobId, p39D* p);
void GetMultiAttack(int attackerId, int *target, p367 *p);

STRUCT_ITEM *GetItemPointer(int clientIndex, int invType, int invSlot);

void GetCreateItem(int sIndex, p26E *p);

INT32 GetUserByName(char *name);

INT32 GetEmptyItem();

long long GetExpApply(long long exp, int attackerId, int targetId);
long long GetExpApply_2(long long exp, int receiver, int attackerId, bool useBoxExp);
 

INT32 GetManaSpent(INT32 skillId, INT32 saveMana, INT32 mastery);

unsigned long long GetFairyExpBonus(unsigned long long exp, INT32 fadaId);

INT32 GetCompounderDeal(INT32 clientId);
INT32 getWeek(INT32 day, INT32 month, INT32 year) ;

std::tuple<int, int> GetPriceAndImpost(STRUCT_ITEMLIST* item, int city);
 
int GetStaticItemAbility(STRUCT_ITEM *item, unsigned char Type);
int GetDamageByJewel(int clientId, int damage);

INT32 GetDoubleCritical(CMob *arg1, short *arg2, short *arg3, unsigned char *arg4);
INT32 GetItemAmount(STRUCT_ITEM *item);

int GetUserInArea(unsigned int x1, unsigned int y1, unsigned int x2, unsigned int y2, char* first);

INT32 GetInventoryAmount(int clientId, int itemId);
void GetGuild(int clientId);
UINT8 DayOfWeek();
unsigned int GetWeekNumber();

INT32 GetInHalf(INT32 clientId, INT32 mobId);

INT32 GetBonusSet(STRUCT_MOB *player, INT32 defense);

int ReturnChance(STRUCT_ITEM *item);
int GetMaxAmountItem(const STRUCT_ITEM* item);

bool IsWarTime();

int GetSpiritRessBonus(int sanc);
int GetSpiritHPMPBonus(int sanc);

bool IsCostume(const STRUCT_ITEM* item);

std::vector<CUser*> GetSameMACUsers(const CUser& user, std::function<bool(CUser& user)> function = nullptr);

eValley GetValleyWithMinimum();

std::string GetEvolutionName(eClass evolution);

#endif