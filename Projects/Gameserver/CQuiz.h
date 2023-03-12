#pragma once

#include <vector>
#include <map>
#include "Struct.h"
#include "CEvent.h"

using milliseconds = std::chrono::duration<unsigned int, std::milli>;

class CQuiz : public CEventItem
{
	enum class eQuizQuest
	{
		LanHouseN,
		LanHouseM,
		LanHouseA
	};

	struct SQuizUserInfo
	{
		CUser* User;

		time_point lastInteraction;
		std::chrono::milliseconds nextIteraction;

		eQuizQuest quest;

		bool isWaitingResponse{ false };
		bool firstInteraction{ false };
		int correctIndex;
	};

public:
	CQuiz(std::chrono::milliseconds interval)
		: CEventItem(interval)
	{
	}

	virtual bool Register(CUser&, STRUCT_ITEM*);
	virtual void Unregister(CUser&)
	{
	}

	virtual bool CanRegister(CUser&);

	bool GetStatus() const noexcept;
	bool HandlePacket(CUser& user, pMsgSignal* packet);

protected:
	virtual void Work();

private:
	bool _status{ true };

	std::chrono::milliseconds GetNextQuiz() const;
	std::vector<SQuizUserInfo> _users;

	constexpr std::chrono::milliseconds GetWaitResponseTime() const;
};