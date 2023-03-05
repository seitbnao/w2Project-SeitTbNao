#include "Server.h"
#include "Basedef.h"

bool PacketDecrypt(BYTE* PacketBuffer)
{
	PacketHeader* Header = (PacketHeader*)PacketBuffer;
	UINT32 KeyResult = NULL;
	UINT32 KeyIncrement = KeyTable[(Header->Key & 255) * 2];

	for (INT32 i = 4; i < Header->Size; i++, KeyIncrement++)
	{
		INT32 keyResult = KeyTable[(((KeyIncrement & 255) & 0x800000FF) * 2) + 1];

		switch (i & 3)
		{
		case 00:
		{
			PacketBuffer[i] = PacketBuffer[i] + ((keyResult & 255) << 1);
			break;
		}
		case 01:
		{
			PacketBuffer[i] = PacketBuffer[i] - ((keyResult & 255) >> 3);
			break;
		}
		case 02:
		{
			PacketBuffer[i] = PacketBuffer[i] + ((keyResult & 255) << 2);
			break;
		}
		case 03:
		{
			PacketBuffer[i] = PacketBuffer[i] - ((keyResult & 255) >> 5);
			break;
		}
		}
	}
	return true;
}