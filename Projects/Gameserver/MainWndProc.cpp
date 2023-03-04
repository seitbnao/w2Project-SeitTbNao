#include "cServer.h"
#include "Basedef.h"
#include "SendFunc.h"
#include <new>          // std::bad_alloc
#include <chrono>
LONG APIENTRY WndProc(HWND hWnd, UINT Message, WPARAM wParam, LPARAM lParam)
{
	try
	{
		CUser* user;
		switch (Message)
		{
		case WM_TIMER:
		{
			if (wParam == TIMER_SEC)
				ProcessSecTimer();
			else if (wParam == TIMER_MIN)
				ProcessMinTimer();
			else if (wParam == TIMER_HOUR)
				ProcessHourTimer();
			else if (wParam == TIMER_SEND)
				ProcessSendTimer();
		}
		break;

		case WSA_READDB:
		{
			if (WSAGETSELECTEVENT(lParam) != FD_READ)
			{
				closesocket(sData.Socket);

				return true;
			}

			int ret = Receive();
			if (ret == FALSE)
			{
				ret = WSAGetLastError();

				break;
			}

			else if (ret == -1)
			{
				ret = WSAGetLastError();

				break;
			}

			int Error = FALSE;
			int ErrorCode = FALSE;
			while (TRUE)
			{
				char* Msg = ReadMessageDB(&Error, &ErrorCode);
				if (Msg == NULL)
					break;

				PacketControl((BYTE*)Msg, ErrorCode);
			}

		}
		break;
		// Aceitação de um usuario ao tentar-se conectar
		case WSA_ACCEPT:
		{
			if (WSAGETSELECTERROR(lParam) == 0)
			{
				int userId = GetEmptyUser();
				if (userId == 0)
				{
					Log(SERVER_SIDE, LOG_INGAME, "Cant find a slot to player");

					break;
				}

				int ret = pUser[userId].AcceptUser(sServer.Socket);
				if (userId >= MAX_PLAYER - 10)
				{
					SendClientMessage(userId, "Servidor cheio. Tente novamente mais tarde");
					pUser[userId].SendMessageA();

					CloseUser(userId);
					return true;
				}
				
				pUser[userId].clientId = userId;
				pUser[userId].LastReceive = sServer.ServerTime;
			}
		}
		break;

		case WSA_READ:
		{
			user = GetUserBySocket(wParam);
			if (user == NULL)
			{
				closesocket(wParam);
				break;
			}

			if (WSAGETSELECTEVENT(lParam) != FD_READ)
			{
				Log(user->clientId, LOG_INGAME, "Recebeu lParam (WndProc) como %hu. Erro possêvel: %hu. Desconectando o usuario.", WSAGETSELECTEVENT(lParam), WSAGETSELECTERROR(lParam));

				CloseUser(user->clientId);
				break;
			}

			int ret = user->Receive();
			if (ret == FALSE)
			{
				ret = WSAGetLastError();

				if (user->User.Username[0])
					Log(user->clientId, LOG_INGAME, "Usuario foi desconectado. Recebeu o erro %d no Receive", ret);
				else
					Log(SERVER_SIDE, LOG_INGAME, "Um usuario foi desconectado pois recebeu o erro %d", ret);

				CloseUser(user->clientId);
				break;
			}
			else if (ret == -1)
			{
				ret = WSAGetLastError();

				if (user->User.Username[0])
					Log(user->clientId, LOG_INGAME, "Usuario foi desconectado. Recebeu o erro %d no Receive", ret);
				else
					Log(SERVER_SIDE, LOG_INGAME, "Um usuario foi desconectado pois recebeu o erro %d", ret);

				CloseUser(user->clientId);
				break;
			}

			int Error = FALSE;
			int ErrorCode = FALSE;
			while (TRUE)
			{
				char* Msg = user->ReadMessage(&Error, &ErrorCode);
				if (Msg == NULL)
					break;

				if (Error == 2)
				{
					CloseUser(user->clientId);

					Log(user->clientId, LOG_INGAME, "Desconectado por Error 2 - ReadMessage");
					break;
				}

				user->PacketControl((BYTE*)Msg, ErrorCode);
			}
		}
		break;

		case WM_COMMAND:
		{
			if ((HWND)lParam == GUI.hButton[0])
			{
				bool success = true;

				if (!LoadConfig())
					success = false;
				else if (!LoadDataServer())
					success = false;
				/*else if (!ReadMySQL())
					success = false;
				else if (!ConnectSQL())
					success = false;*/
				else if (!CreateServer())
					success = false;
				else if (!ConnectDB())
					success = false;

				if (success)
				{
					BASE_InitializeHitRate();
					ReadAttributeMap();
					ReadHeightMap();
					ReadLanguageFile();
					ReadItemList();
					ReadSkillData();
					ReadTeleport();
					ReadGameConfig();
					ReadNPCBase();
					ReadGuild();
					ReadInitItem();
					LoadGuild();

					for (UINT32 LOCAL_5 = 0; LOCAL_5 < sServer.InitCount; LOCAL_5++)
					{
						STRUCT_ITEM LOCAL_7;
						memset(&LOCAL_7, 0, sizeof STRUCT_ITEM);

						LOCAL_7.sIndex = g_pInitItemFile[LOCAL_5].sIndex;

						INT32 LOCAL_8 = CreateItem(g_pInitItem[LOCAL_5].PosX, g_pInitItem[LOCAL_5].PosY, &LOCAL_7, g_pInitItemFile[LOCAL_5].Rotate, 3);
						if (LOCAL_8 >= 4096 || LOCAL_8 <= 0)
							continue;

						INT32 LOCAL_9 = GetItemAbility(&LOCAL_7, EF_KEYID);
						if (LOCAL_9 != 0 && LOCAL_9 < 15)
						{
							int LOCAL_10;
							UpdateItem(LOCAL_8, 3, &LOCAL_10);
						}
					}

					// Cria as torres doidonas
					for (INT32 i = 0; i < 5; i++)
					{
						STRUCT_GUILDZONE* zone = &g_pCityZone[i];

						STRUCT_ITEM item;
						memset(&item, 0, sizeof STRUCT_ITEM);

						item.sIndex = 3145 + (zone->win_count);

						if (ChargedGuildList[sServer.Channel - 1][i] != 0)
						{
							item.EF1 = 56;
							item.EFV1 = zone->owner_index / 257;

							item.EF2 = 57;
							item.EFV2 = zone->owner_index;

							item.EF3 = 59;
							item.EFV3 = Rand() % 255;
						}

						CreateItem(zone->tower_x, zone->tower_y, &item, 3, 0);
					}

					SetArenaDoor(3);

					// Lê os arquivos de itens
					mGener.ReadNPCGener();
				}
				else
				{
					if (sServer.Socket != INVALID_SOCKET)
						closesocket(sServer.Socket);

					WSACleanup();
				}
			}
		}
		break;

		// Event to destroy GUI
		case WM_CLOSE:
		case WM_DESTROY:
			PostQuitMessage(0);
			break;
			// Default value - Probably when user use mouse in form
		default:
			break;
		}
	}
	catch (std::exception& e)
	{
		printf("Excecao em WndProc %s", e.what());
	}   
	
	return DefWindowProc(hWnd, Message, wParam, lParam);

}