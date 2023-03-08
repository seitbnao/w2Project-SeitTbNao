#include "cServer.h"
#include "Basedef.h"
#include "SendFunc.h"
#include "GetFunc.h"

bool CUser::RequestOpenShop(PacketHeader *Header)
{
	p27B *p = (p27B*)Header;
	if(p->TargetID < 1000 || p->TargetID >= MAX_MOB)
		return false;

	STRUCT_MOB *spw = &pMob[p->TargetID].Mobs.Player;

	int merch = spw->BaseScore.Merchant.Merchant;
	if(merch != 1 && merch != 3 && merch != 19)
		return false;

	if (pMob[p->TargetID].GenerateID == 384 || pMob[p->TargetID].GenerateID == 385)
	{
		p17E packet{};
		packet.Header.PacketId = 0x17E;
		for (size_t i = 0; i < sServer.RvR.Items.size(); ++i)
		{
			memcpy(&packet.Item[i], &sServer.RvR.Items[i].Item, sizeof STRUCT_ITEM);
			packet.Price[i] = sServer.RvR.Items[i].Price;
		}

		AddMessage(reinterpret_cast<BYTE*>(&packet), sizeof packet);
		return true;
	}

	p17C packet;
	packet.Header.ClientId = 0x7530;
	packet.Header.PacketId = 0x17C;
	packet.Header.Size = sizeof p17C;

	INT32 cityZone = GetVillage(pMob[p->TargetID].Target.X, pMob[p->TargetID].Target.Y);
	if(cityZone == 5)
		cityZone = 4;

	packet.Taxes = g_pCityZone[cityZone].perc_impost;
	packet.ShopType = merch;

	for(int i = 0;i<27;i++)
		memcpy(&packet.Item[i], &spw->Inventory[(i % 9) + ((i / 9) * 27)], sizeof STRUCT_ITEM);

	if(pMob[p->TargetID].Mobs.Player.BaseScore.Int == 4001)
		packet.Taxes = 0;
	else
		packet.Taxes = g_pCityZone[cityZone].perc_impost;

	AddMessage((BYTE*)&packet, sizeof p17C);
	return true;
}