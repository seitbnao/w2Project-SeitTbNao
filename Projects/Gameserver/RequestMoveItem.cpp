#include "cServer.h"
#include "Basedef.h"
#include "SendFunc.h"
#include "GetFunc.h"

void GiveTime(p376 *p)
{
	INT32 clientId = p->Header.ClientId;
	// Serve para quando equipar a Esfera dê a validade ao item
	int days = 0;

	if (p->SourType != (unsigned char)SlotType::Equip && p->DestType != (unsigned char)SlotType::Equip)
		return;

	STRUCT_ITEM* item = nullptr;
	SlotType type;
	int slot;
	if (p->DestType == (unsigned char)SlotType::Equip)
	{
		item = GetItemPointer(clientId, p->DestType, p->DestSlot);
		type = (SlotType)p->SourType;
		slot = p->SourSlot;
	}
	else if (p->SourType == (unsigned char)SlotType::Equip)
	{
		item = GetItemPointer(clientId, p->SourType, p->SourSlot);
		type = (SlotType)p->DestType;
		slot = p->DestSlot;
	}

	if (item == nullptr)
		return;

	switch (item->sIndex)
	{
	case 3916:
	case 3917:
		days = 1;
		break;

	case 3980:
	case 3981:
	case 3982:
	case 3900:
	case 3901:
	case 3902:
	case 3998:
		days = 3;
		break;
	case 3903:
	case 3904:
	case 3905:
	case 3990: // Grande lobo
		days = 5;
		break;
	case 3906:
	case 3907:
	case 3908:
	case 3911:
	case 3997:
		days = 7;
		break;
	case 3983:
	case 3984:
	case 3985:
	case 3912:
	case 3996:
		days = 15;
		break;
	case 3986:
	case 3987:
	case 3988:
	case 3989:
	case 3913:
	case 4151:
		//case 4152: // valq
		//case 4153: // esqueleto
	case 4154:
		//case 4155:
		//case 4156:
	case 4157:
	case 4158:
	case 4159:
	case 4160:
	case 4161:
	case 4162:
	case 4163:
	case 4164:
	case 4165:
	case 4166:
	case 4167:
	case 4168:
	case 4169:
	case 4170:
	case 4171:
	case 4172:
	case 4173:
	case 4174:
	case 4175:
	case 4176:
	case 4177:
	case 4178:
	case 4179:
	case 4180:
	case 4181:
	case 4182:
	case 4183:
	case 4184:
	case 4185:
	case 4186:
	case 4187:
	case 4188:
	case 4189:
	case 4190:
	case 4191:
	case 4192:
	case 3995:
	case 4229:
	case 4240:
	case 4241:
	case 4235:
	case 4236:
	case 4237:
	case 4233:
	case 4224:
	case 4231:
	case 4230:
	case 4243:
	case 4242:
	case 4244:
	case 4225:
	case 4226:
	case 4227:
    case 4245:
    case 4246:
    case 4247:
		days = 30;
		break;

	default:
		return;
	}

	time_t rawnow = time(NULL);
	struct tm now;
	localtime_s(&now, &rawnow);

	int cMes = now.tm_mon + 1; //0 Janeiro, 1 Fev
	int cDia = now.tm_mday;
	int cAno = now.tm_year;;

	int mes = 0, dia = 0, ano = 0;

	mes = cMes;
	dia = cDia + days;
	ano = cAno + 1900;

	if (dia > dias_mes[mes])
	{
		dia -= dias_mes[mes];
		mes += 1;
	}

	if (mes > 12)
	{
		mes -= 12;
		ano += 1;
	}

	bool isCostume = IsCostume(item);
	if ((item->sIndex >= 3980 && item->sIndex <= 3999) || (item->sIndex >= 3995 && item->sIndex <= 3995) || isCostume)
	{
		if ((item->EF1 == 106 && item->EF2 == 110 && item->EF3 == 109) || (isCostume && item->EFV1 != 0 && item->EFV2 != 0 && item->EFV3 != 0))
		{
			float difDays = TimeRemaining(item->EFV1, item->EFV2, (item->EFV3 + 2000));
			if (difDays <= 0.0f)
			{
				SendClientMessage(p->Header.ClientId, "Esfera / Traje expirou...");
				Log(clientId, LOG_INGAME, "Traje / Esfera expirado. Expira em: %d/%d/%d", item->EFV1, item->EFV2, item->EFV3);

				memset(item, 0, sizeof STRUCT_ITEM);

				SendItem(clientId, type, slot, item);
				return;
			}
			else
			{
				if (isCostume && item->EF1 == 106 && item->EF2 == 110 && item->EF3 == 109)
				{
					Log(clientId, LOG_INGAME, "Removido os adicionais de tempo do traje %s %s", g_pItemList[item->sIndex].ItemName, item->toString().c_str());

					item->EF1 = 0;
					item->EF2 = 0;
					item->EF3 = 0;
				}
			}

			return;
		}
	}
	else if ((item->sIndex >= 3900 && item->sIndex <= 3908) || item->sIndex == 3916 || item->sIndex == 3917)
	{
		if (item->EF1 != 0)
			return;

		item->EF1 = 106;
		item->EFV1 = days;

		SendItem(clientId, type, slot, item);
		return;
	}

	if (!isCostume)
	{
		item->EF1 = 106;
		item->EFV1 = dia;

		item->EF2 = 110;
		item->EFV2 = mes;

		item->EF3 = 109;
		item->EFV3 = (ano - 2000);
	}
	else
	{
		item->EFV1 = dia;
		item->EFV2 = mes;
		item->EFV3 = (ano - 2000);
	}

	SendItem(clientId, type, slot, item);
	Log(clientId, LOG_INGAME, "Aplicado tempo no item %s %s - %d/%d/%d", g_pItemList[item->sIndex].ItemName, item->toString().c_str(), dia, mes, ano);
}

bool CUser::RequestMoveItem(PacketHeader *header)
{
	static const char *szSlotType[] = { "EQUIPAMENTO", "INVENTaRIO", "BANCO", "RUNA" };

	p376* p = (p376*)header;
	if (!pMob[clientId].Mobs.Player.CurrentScore.Hp || Status != USER_PLAY)
	{
		SendHpMode(clientId);

		AddCrackError(clientId, 1, CRACK_USER_STATUS);
		return true;
	}

	if (pUser[clientId].Trade.ClientId != MOB_EMPTY)
	{
		RemoveTrade(clientId);

		AddCrackError(clientId, 1, CRACK_TRADE_NOTEMPTY);
		return true;
	}

	if (IsAutoTrading)
	{
		SendClientMessage(clientId, g_pLanguageString[_NN_CantWhenAutoTrade]);

		return true;
	}

	if (User.Block.Blocked)
	{
		SendClientMessage(clientId, "Desbloqueie seus itens para poder movimenta-los");	

		return true;
	}

	p->Header.ClientId = clientId;
	STRUCT_ITEM *mountSlot = &pMob[clientId].Mobs.Player.Equip[14]; // LOCAL1684

	STRUCT_ITEM *equipDestSlot; // LOCAL1649
	STRUCT_ITEM *equipSourSlot; // LOCAL1650
	INT32 needSendEquip = 0; // LOCAL1651
	INT32 needSendScore = 0; // LOCAL1652

	if (p->DestType == (unsigned char)SlotType::Equip)
	{
		if (p->DestSlot <= 0 || p->DestSlot > 15)
		{
			AddCrackError(clientId, 2, CRACK_USER_PKTHACK);

			return false;
		}

		if (p->DestSlot == 15)
		{
			AddCrackError(clientId, 2, CRACK_USER_PKTHACK);

			Log(clientId, LOG_HACK, "Tentativa de mover a capa do personagem");
			return false;
		}

		equipDestSlot = &pMob[clientId].Mobs.Player.Equip[p->DestSlot];

		needSendEquip = 1;
		needSendScore = 1;
	}
	else if (p->DestType == (unsigned char)SlotType::Inv)
	{
		if (p->DestSlot < 0 || p->DestSlot >= 60)
		{
			AddCrackError(clientId, 2, CRACK_USER_PKTHACK);

			return false;
		}

		if (p->DestSlot >= 30 && p->DestSlot < 45)
		{
			if (TimeRemaining(pMob[clientId].Mobs.Player.Inventory[60].EFV1, pMob[clientId].Mobs.Player.Inventory[60].EFV2, pMob[clientId].Mobs.Player.Inventory[60].EFV3 + 1900) <= 0)
				return false;
		}
		else if (p->DestSlot >= 45 && p->DestSlot < 60)
		{
			if (TimeRemaining(pMob[clientId].Mobs.Player.Inventory[61].EFV1, pMob[clientId].Mobs.Player.Inventory[61].EFV2, pMob[clientId].Mobs.Player.Inventory[61].EFV3 + 1900) <= 0)
				return false;
		}

		equipDestSlot = &pMob[clientId].Mobs.Player.Inventory[p->DestSlot];
	}
	else if (p->DestType == (unsigned char)SlotType::Storage)
	{
		if (p->DestSlot < 0 || p->DestSlot >= 120)
		{
			AddCrackError(clientId, 2, CRACK_USER_PKTHACK);

			return false;
		}

		equipDestSlot = &pUser[clientId].User.Storage.Item[p->DestSlot];
	}
	else
	{
		AddCrackError(clientId, 2, CRACK_USER_PKTHACK);

		return false;
	}

	if (p->SourType == (unsigned char)SlotType::Equip)
	{
		if (p->SourSlot <= 0 || p->SourSlot > 15)
		{
			AddCrackError(clientId, 2, CRACK_USER_PKTHACK);

			return false;
		}

		if (p->SourSlot == 15)
		{
			AddCrackError(clientId, 2, CRACK_USER_PKTHACK);

			Log(clientId, LOG_HACK, "Tentativa de mover a capa do personagem");
			return false;
		}

		equipSourSlot = &pMob[clientId].Mobs.Player.Equip[p->SourSlot];

		needSendEquip = 1;
		needSendScore = 1;
	}
	else if (p->SourType == (unsigned char)SlotType::Inv)
	{
		if (p->SourSlot < 0 || p->SourSlot >= 60)
		{
			AddCrackError(clientId, 2, CRACK_USER_PKTHACK);

			return false;
		}
		/*
		if (p->SourSlot >= 30 && p->SourSlot < 45)
		{
			if (TimeRemaining(pMob[clientId].Mobs.Player.Inventory[60].EFV1, pMob[clientId].Mobs.Player.Inventory[60].EFV2, pMob[clientId].Mobs.Player.Inventory[60].EFV3 + 1900) <= 0)
				return false;
		}
		else if (p->SourSlot >= 45 && p->SourSlot < 60)
		{
			if (TimeRemaining(pMob[clientId].Mobs.Player.Inventory[61].EFV1, pMob[clientId].Mobs.Player.Inventory[61].EFV2, pMob[clientId].Mobs.Player.Inventory[61].EFV3 + 1900) <= 0)
				return false;
		}
		*/
		equipSourSlot = &pMob[clientId].Mobs.Player.Inventory[p->SourSlot];
	}
	else if (p->SourType == (unsigned char)SlotType::Storage)
	{
		if (p->SourSlot < 0 || p->SourSlot >= 120)
		{
			AddCrackError(clientId, 2, CRACK_USER_PKTHACK);

			return false;
		}

		equipSourSlot = &pUser[clientId].User.Storage.Item[p->SourSlot];
	}
	else
	{
		AddCrackError(clientId, 2, CRACK_USER_PKTHACK);

		return false;
	}

	// Testa se sêo os mesmos itens
	if (equipSourSlot->sIndex == equipDestSlot->sIndex)
	{
		STRUCT_ITEM* srcItem = GetItemPointer(p->Header.ClientId, p->SourType, p->SourSlot);
		STRUCT_ITEM* dstItem = GetItemPointer(p->Header.ClientId, p->DestType, p->DestSlot);

		if (AgroupItem(clientId, srcItem, dstItem))
		{
			Log(clientId, LOG_INGAME, "O agrupamento foi efetuado de slot %d tipo %s para slot %d tipo %s", p->SourSlot, szSlotType[p->SourType], p->DestSlot, szSlotType[p->DestType]);

			SendItem(p->Header.ClientId, (SlotType)p->SourType, p->SourSlot, srcItem);
			SendItem(p->Header.ClientId, (SlotType)p->DestType, p->DestSlot, dstItem);

			return true;
		}
	}

	if ((equipSourSlot->sIndex >= 5110 && equipSourSlot->sIndex <= 5133) && equipDestSlot->sIndex == 4854)
	{
		EnergizeEmptyRune(clientId, p);

		return true;
	}

	if (equipDestSlot->sIndex == 747 || equipSourSlot->sIndex == 747 ||
		equipDestSlot->sIndex == 3993 || equipSourSlot->sIndex == 3993 ||
		equipDestSlot->sIndex == 3994 || equipSourSlot->sIndex == 3994)
	{
		if (p->DestType == 2 || p->SourType == 2)
		{
			INT32 guildId = pMob[clientId].Mobs.Player.Guild;
			if (guildId <= 0)
				return false;

			for (INT32 i = 0; i < 10; i++)
			{
				if (ChargedGuildList[i][4] == guildId)
					return true;
			}

			if (IsAdmin)
				return true;

			return false;
		}
	}

	if (equipDestSlot->sIndex == 877 || equipSourSlot->sIndex == 877)
		return true;

	STRUCT_ITEM newEquipSourSlot; // LOCAL1655
	STRUCT_ITEM newEquipDestSlot; // LOCAL1657
	memcpy(&newEquipSourSlot, equipSourSlot, sizeof STRUCT_ITEM);
	memcpy(&newEquipDestSlot, equipDestSlot, sizeof STRUCT_ITEM);

	memset(equipSourSlot, 0, sizeof STRUCT_ITEM);
	memset(equipDestSlot, 0, sizeof STRUCT_ITEM);

	// 00433D7B
	INT32 canMove1 = 1; // LOCAL1659
	INT32 canMove2 = 1; // LOCAL1660

	if (newEquipDestSlot.sIndex != 0)
	{
		INT32 error = -2; // LOCAL1661
		if (p->SourType == (unsigned char)SlotType::Equip)
			canMove1 = CanEquip(&newEquipDestSlot, &pMob[clientId].Mobs.Player, p->SourSlot, pMob[clientId].Mobs.Player.Class, p, pMob[clientId].Mobs.Player.Equip[0].EFV2);

		else if (p->SourType == (unsigned char)SlotType::Inv)
		{
			canMove1 = CanCarry(&newEquipDestSlot, pMob[clientId].Mobs.Player.Inventory, p->SourSlot % 9, p->SourSlot / 9, &error);

			if (canMove1 == 0 && error > 0 && error <= 64)
			{
				error--;

				SendItem(clientId, SlotType::Inv, error, &pMob[clientId].Mobs.Player.Inventory[error]);
			}
		}

		else if (p->SourType == (unsigned char)SlotType::Storage)
			canMove1 = CanCargo(&newEquipDestSlot, pUser[clientId].User.Storage.Item, (p->SourSlot % 9), (p->SourSlot / 9));
	}

	if (newEquipSourSlot.sIndex != 0)
	{
		INT32 error = -2; // LOCAL1662
		if (p->DestType == (unsigned char)SlotType::Equip)
			canMove2 = CanEquip(&newEquipSourSlot, &pMob[clientId].Mobs.Player, p->DestSlot, pMob[clientId].Mobs.Player.Class, p, pMob[clientId].Mobs.Player.Equip[0].EFV2);

		else if (p->DestType == (unsigned char)SlotType::Inv)
		{
			canMove2 = CanCarry(&newEquipSourSlot, pMob[clientId].Mobs.Player.Inventory, p->DestSlot % 9, p->DestSlot / 9, &error);

			if (canMove2 == 0 && error > 0 && error < 60)
			{
				error--;

				SendItem(clientId, SlotType::Inv, error, &pMob[clientId].Mobs.Player.Inventory[error]);
			}
		}

		else if (p->DestType == (unsigned char)SlotType::Storage)
			canMove2 = CanCargo(&newEquipSourSlot, pUser[clientId].User.Storage.Item, (p->DestSlot % 9), (p->DestSlot / 9));
	}

	if (canMove1 == 0 || canMove2 == 0)
	{
		memcpy(equipSourSlot, &newEquipSourSlot, 8);
		memcpy(equipDestSlot, &newEquipDestSlot, 8);

		return true;
	}
	else
	{
		// Checa se esta movendo fora de uma cidade
		if ((p->DestType == (unsigned char)SlotType::Storage || p->SourType == (unsigned char)SlotType::Storage) && GetVillage(pMob[clientId].Target.X, pMob[clientId].Target.Y) == 5)
		{
			auto posX = pMob[clientId].Target.X;
			auto posY = pMob[clientId].Target.Y;

			bool canMove = false;
			if ((posX >= 2085 && posX <= 2150 && posY >= 2016 && posY <= 2051) ||
				(posX >= 3209 && posX <= 3273 && posY >= 1672 && posY <= 1712))
				canMove = true;

			if (!canMove)
			{
				memcpy(equipSourSlot, &newEquipSourSlot, 8);
				memcpy(equipDestSlot, &newEquipDestSlot, 8);

				Log(SERVER_SIDE, LOG_HACK, "[%s] - O usuario moveu o item %s fora da cidade. Posição: %ux %uy", User.Username, equipSourSlot->toString().c_str(), pMob[clientId].Target.X, pMob[clientId].Target.Y);
				Log(clientId, LOG_HACK, "[%s] - O usuario moveu o item %s fora da cidade. Posição: %ux %uy", User.Username, equipSourSlot->toString().c_str(), pMob[clientId].Target.X, pMob[clientId].Target.Y);
				return true;
			}
		}

		memcpy(equipSourSlot, &newEquipDestSlot, 8);
		memcpy(equipDestSlot, &newEquipSourSlot, 8);

		Log(p->Header.ClientId, LOG_INGAME, "Moveu item %s [%d] [%d %d %d %d %d %d] do tipo %s do slot %d para tipo %s slot %d", g_pItemList[equipDestSlot->sIndex].ItemName, equipDestSlot->sIndex, equipDestSlot->EF1,
			equipDestSlot->EFV1, equipDestSlot->EF2, equipDestSlot->EFV2, equipDestSlot->EF3, equipDestSlot->EFV3, szSlotType[p->SourType], p->SourSlot, szSlotType[p->DestType], p->DestSlot);

		GiveTime(p);
	}

	if (equipSourSlot->sIndex <= 40)
		memset(equipSourSlot, 0, 8);
	if (equipDestSlot->sIndex <= 40)
		memset(equipDestSlot, 0, 8);

	pMob[clientId].GetCurrentScore(clientId);

	AddMessage((BYTE*)p, sizeof p376);

	//434182
	if (pMob[clientId].Mobs.Player.Equip[6].sIndex == 0 && pMob[clientId].Mobs.Player.Equip[7].sIndex != 0)
	{
		INT32 itemAbility = GetItemAbility(&pMob[clientId].Mobs.Player.Equip[7], 17); // LOCAL1663

		if (itemAbility != 128)
		{
			p->DestType = 0;
			p->DestSlot = 6;
			p->SourType = 0;
			p->SourSlot = 7;

			AddMessage((BYTE*)p, sizeof p376);

			memcpy(&pMob[clientId].Mobs.Player.Equip[6], &pMob[clientId].Mobs.Player.Equip[7], 8);
			memset(&pMob[clientId].Mobs.Player.Equip[7], 0, 8);
		}
	}

	if (needSendScore == 1)
		SendScore(clientId);

	if (needSendEquip == 1)
		SendEquip(clientId);

	MountProcess(clientId, mountSlot);
	return true;
}
