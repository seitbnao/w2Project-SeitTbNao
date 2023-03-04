#include <Windows.h>
#include "cServer.h"

#ifndef __SENDFUNC_H__
#define __SENDFUNC_H__


// Envia uma mensagem ao usuario
bool SendClientMessage(int clientId, const char *msg, ...);

// Envia um sigal
void SendSignalParm(INT32 toClientId, INT32 clientId, INT16 packetId, INT32 value);
void SendSignalParm2(INT32 toClientId, INT32 clientId, INT16 packetId, INT32 value, INT32 value2);
void SendSignalParm3(INT32 toClientId, INT32 clientId, INT16 packetId, INT32 value, INT32 value2, INT32 value3);
void SendSignal(INT32 toClientId, INT32 clientId, INT16 packetId);

// Envia o mob para o usuario
void SendCreateMob(int sendClientID, int createClientID, INT32 send = 0);

// Envia o mob para todos da area
void SendGridMob(int sIndex);

// Envia os buffs do usuario
void SendAffect(int clientId);

// Atualiza um slot do inventario do usuario
void SendItem(int clientId, SlotType invType, int slotId, STRUCT_ITEM *item);

// Atualiza os itens equipados no usuario
void SendEquip(int clientId);

// Atualiza os status do personagem
void SendScore(int clientIndex);
void SendEtc(int clientId);

// Envia a loja ao usuario
void SendAutoTrade(int sendClientId, int tradeClientId);

// Atualiza o gold do banco do usuario
void SendCargoCoin(int clientId);

// 
void DeleteMob(int clientId, int reason);

//
void SendSay(int clientId, const char *msg, ...);

// 
void SendRemoveMob(UINT16 killerId, UINT16 killedId,  UINT32 mType, INT32 send);
//
void SendEmotion(int clientId, int val1, int val2);

//
void SendNoticeArea(const char *Message, unsigned int x1, unsigned int y1, unsigned int x2, unsigned int y2);

//
void SendSetHpMp(int clientId);

void SendCreateItem(int toClientId, int initId, int unk);
void SendRemoveItem(int dest, int itemid, int bSend);

void SendAddParty(int target, int whom, int leader);
void SendRemoveParty(int target, int whom);

void SendHpMode(int clientId);

void SendCarry(int clientId);
void SendDamage(unsigned int min_x, unsigned int min_y, unsigned int max_x, unsigned int max_y);
void SendEnvEffect(int min_x, int min_y, int max_x, int max_y, int type1, int type2);

void SendNotice(const char*msg, ...);
void SendGuildNotice(INT32 guildId, const char *msg, ...);
void SendServerNotice(const char *msg, ...);
void SendWarInfo(INT32 arg1, INT32 arg2);

void SendGuildList(INT32 clientId);

void SendChatGuild(INT32 clientId, INT32 guildId, const char *msg, ...);

void SendRepurchase(int clientId);
void SendWeather();

void SendCounterMob(int clientId, short value, short total);
 
void SendQuiz(int clientId);
void SendQuiz(int clientId, const char* question, std::array<std::string, 4> answers);
 
void SendKingdomBattleInfo(int clientId, int kingdom, bool status);
 
void SendAutoPartyInfo(int clientId);
 

void MulticastGetCreateMob(int clientId);

void SendChatMessage(int clientId, int color, const char* message, ...);
void SendChatMessage(int color, const char* message, ...);

 

#endif