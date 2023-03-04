#include "cServer.h"
#include "Basedef.h"
#include "SendFunc.h"
#include "GetFunc.h"
using namespace std::chrono_literals;
bool CUser::RequestSealInfo(PacketHeader* header)
{
	pMsgSignal *p = reinterpret_cast<pMsgSignal*>(header);

	auto sealIt = std::find_if(std::begin(sServer.SealCache), std::end(sServer.SealCache), [&](const InfoCache<STRUCT_SEALINFO> seal) {
		return seal.Info.CurrentScore == p->Value;
	});

	if (sealIt != std::end(sServer.SealCache))
	{
		if (std::chrono::steady_clock::now() - sealIt->Last < 5min)
		{
			pDC3 packet{};
			packet.Header.PacketId = 0xDC3;
			packet.Header.Size = sizeof packet;
			packet.Info = sealIt->Info;

			AddMessage(reinterpret_cast<BYTE*>(&packet), sizeof packet);
			return true;
		}
	}

	p->Header.PacketId = SealInfoPacket;
	p->Header.ClientId = clientId;
	p->Header.Size = sizeof pMsgSignal;

	AddMessageDB(reinterpret_cast<BYTE*>(p), sizeof pMsgSignal);
	return true;
}	