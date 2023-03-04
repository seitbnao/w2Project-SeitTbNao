#pragma once
#include <SharedStructs.h>
 
#include <map>
#include <tuple>
#include <sstream>

struct STRUCT_ITEM;

class CResetAccount
{
public:
	CResetAccount(STRUCT_ACCOUNT*);

	STRUCT_ACCOUNT ResetAccount();
	std::string GetReport() const
	{
		return report.str();
	}
private:
	STRUCT_ACCOUNT _account;

	void LoadDonatePrice();

	std::map<short, int> donateStore;

	int GetItemPrice(const STRUCT_ITEM* item);
	void Log_Text(const char* msg, ...);

	bool isInitialPackage(const STRUCT_ITEM* item) const;
	bool isExpired(const STRUCT_ITEM* item) const;
	int GetItemAmount(const STRUCT_ITEM* item) const;
	float TimeRemaining(int dia, int mes, int ano) const; 
	std::tuple<bool, STRUCT_ITEM>  isMountWithCostume(const STRUCT_ITEM* item);


	std::stringstream report;
};

