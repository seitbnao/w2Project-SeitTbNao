#include "cServer.h"
#include "Basedef.h"
#include "SendFunc.h"
#include "GetFunc.h"
#include <vector>

bool CUser::RequestAlchemy(PacketHeader* Header)
{
	struct Item
	{
		int sIndex{ 0 };
		short Sanc{ 0 };
		short Amount{ 1 };
	};

	struct AlchemyItem
	{
		std::array<Item, 7> Required;

		int Coin;
		int Earned;
	};

	static const std::vector<AlchemyItem> compositionItems
	{
		{
			{
				{
					{413, 0},
					{2441, 0},
					{2442, 0},
					{0, 0},
					{0, 0},
					{0, 0},
					{0, 0},
				},
			},
			20000000,
			3200
		},
		{
			{
				{
					{413, 0},
					{2443, 0},
					{2442, 0},
					{0, 0},
					{0, 0},
					{0, 0},
					{0, 0},
				},
			},
			20000000,
			3201
		},
		{
			{
				{
					{4127, 0},
					{4127, 0},
					{4127, 0},
					{0, 0},
					{0, 0},
					{0, 0},
					{0, 0},
				},
			},
			20000000,
			3202
		},
		{
			{
				{
					{4127, 0},
					{4127, 0},
					{697, 0},
					{0, 0},
					{0, 0},
					{0, 0},
					{0, 0},
				},
			},
			20000000,
			3203
		},
		{
			{
				{
					{412, 0},
					{2441, 0},
					{2444, 0},
					{0, 0},
					{0, 0},
					{0, 0},
					{0, 0},
				},
			},
			20000000,
			3204
		},
		{
			{
				{
					{412, 0},
					{2444, 0},
					{2443, 0},
					{0, 0},
					{0, 0},
					{0, 0},
					{0, 0},
				},
			},
			20000000,
			3205
		},
		{
			{
				{
					{4127, 0},
					{4127, 0},
					{4127, 0},
					{413, 0, 10},
					{3221, 0},
					{0, 0},
					{0, 0},
				},
			},
			20000000,
			3214
		},
		// Joias Palito
		// Joia do Poder
		{
			{
				{
					{413, 0, 60},
					{4127, 0 },
					{4127, 0 },
					{4127, 0 },
					{614, 9},
					{3221, 0},
					{0, 0},
				},
			},
			100000000,
			3206
		},
		/*{
			{
				{
					{612, 0},
					{613, 0},
					{614, 0},
					{615, 0},
					{0, 0},
					{0, 0},
					{0, 0},
				},
			},
			0,
			3207
		},*/
		// Precisêo
		{
			{
				{
					{413, 0, 60},
					{4127, 0 },
					{4127, 0 },
					{4127, 0 },
					{612, 9},
					{3221, 0},
					{0, 0},
				},
			},
			100000000,
			3208
		},	
		// Magia
		{
			{
				{
					{413, 0, 60},
					{4127, 0 },
					{4127, 0 },
					{4127, 0 },
					{613, 9},
					{3221, 0},
					{0, 0},
				},
			},
			100000000,
			3209
		}
	};

	STRUCT_MOB* player = &pMob[clientId].Mobs.Player;
	pCompor* p = (pCompor*)Header;

	for (int i = 0; i < 7; i++)
	{
		if (p->slot[i] == -1)
		{
			memset(&p->items[i], 0, sizeof STRUCT_ITEM);
			continue;
		}

		if (p->slot[i] < 0 || p->slot[i] >= 60)
		{
			SendClientMessage(clientId, "Invalido");

			return true;
		}

		for (int y = 0; y < 7; y++)
		{
			if (y == i)
				continue;

			if (p->slot[i] == p->slot[y])
			{
				Log(clientId, LOG_HACK, "Banido por enviar item com mesmo slotId - NPC Alchemy - %d", p->items[i].sIndex);
				Log(SERVER_SIDE, LOG_HACK, "%s - Banido por enviar item com mesmo slotId  - NPC Alchemy - %d", player->MobName, p->items[i].sIndex);

				SendCarry(clientId);
				return true;
			}
		}

		if (!memcmp(&p->items[i], &player->Inventory[p->slot[i]], sizeof STRUCT_ITEM))
			continue;

		SendClientMessage(clientId, "Invalido");
		return true;
	}

	if (User.Block.Blocked)
	{
		SendClientMessage(clientId, "Desbloqueie seus itens para poder movimenta-los");

		return true;
	}

	for (int i = 0; i < 7; i++)
	{
		if (p->items[i].sIndex == 0)
			continue;

		Log(clientId, LOG_COMP, "Alchemy  - %d - %s %s - %d", i, g_pItemList[p->items[i].sIndex].ItemName, p->items[i].toString().c_str(), p->slot[i]);
	}

	if (Trade.ClientId != 0)
	{
		RemoveTrade(clientId);

		AddCrackError(clientId, 1, CRACK_TRADE_NOTEMPTY);
		return true;
	}

	const AlchemyItem* alchemy = nullptr;
	for (const auto& comp : compositionItems)
	{
		bool itsThatComp = true;
		for (int i = 0; i < 7; i++) 
		{
			if (comp.Required[i].sIndex == 0)
				continue;

			if (comp.Required[i].sIndex != p->items[i].sIndex || GetItemSanc(&p->items[i]) != comp.Required[i].Sanc || GetItemAmount(&p->items[i]) != comp.Required[i].Amount)
			{
				itsThatComp = false;

				break;
			}
		}

		if (itsThatComp)
		{
			alchemy = &comp;
			break;
		}
	}

	if (alchemy == nullptr)
	{
		SendClientMessage(clientId, g_pLanguageString[_NN_IncorrectComp]);

		return true;
	}

	if (alchemy->Coin != 0 && player->Coin < alchemy->Coin)
	{
		SendClientMessage(clientId, g_pLanguageString[_NN_IncorrectComp]);

		return true;
	}

	INT32 chance = 3 + (pMob[clientId].Mobs.Player.CurrentScore.Special[2] * 2 / 10);
	if ((pMob[clientId].Mobs.Player.LearnedSkill[0] & 0x8000))
		chance += 20;

	int rand = Rand() % 100;
	for (int i = 0; i < 7; i++)
	{
		char slotId = p->slot[i];
		if (slotId == -1)
			continue;

		Log(clientId, LOG_INGAME, "Removido o item %s %s. Slot: %d", g_pItemList[p->items[i].sIndex].ItemName, p->items[i].toString().c_str(), (int)p->slot[i]);
		memset(&player->Inventory[slotId], 0, sizeof STRUCT_ITEM);
		SendItem(clientId, SlotType::Inv, p->slot[i], &player->Inventory[p->slot[i]]);
	}

	if (alchemy->Coin != 0)
	{
		player->Coin -= alchemy->Coin;
	
		Log(clientId, LOG_INGAME, "Removido %d gold do personagem. Gold restante:", alchemy->Coin, player->Coin);
		SendSignalParm(clientId, clientId, 0x3AF, player->Coin);
	}

	// Fecha o inventario
	SendSignalParm(clientId, SERVER_SIDE, 0x3A7, 2);

	if (rand > chance)
	{
		Log(clientId, LOG_COMP, "Falha na composição na skill de Alquimia");

		SendClientMessage(clientId, g_pLanguageString[_NN_CombineFailed]);
	}
	else
	{
		// Seta a Pedra Secreta
		player->Inventory[p->slot[0]].sIndex = alchemy->Earned;

		SendItem(clientId, SlotType::Inv, p->slot[0], &player->Inventory[p->slot[0]]);

		// Envia a mensagem de sucesso
		SendClientMessage(clientId, g_pLanguageString[_NN_Success_Comp]);

		Log(clientId, LOG_COMP, "Sucesso na composição do item %s [%d]", g_pItemList[alchemy->Earned].ItemName, alchemy->Earned);
	}

	SaveUser(clientId, 0);
	return true;
}