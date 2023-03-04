#include <Windows.h>
#include <filesystem>
#include <stdio.h>
#include "CSealSoul.h"
#include "Basedef.h"

bool ReadSealInfo(int id, STRUCT_SEALFILE& info)
{
	char temp[MAX_PATH] = { 0 };
	sprintf_s(temp, "./seal/%d.xml", id);

	if (!std::filesystem::exists(temp))
		return false;

	pugi::xml_document doc;
	pugi::xml_parse_result result = doc.load_file(temp, pugi::parse_full);
	if (!result)
	{
		std::cout << "parsed with errors, attr value: [" << doc.child("node").attribute("attr").value() << "]\n";
		std::cout << "Error description: " << result.description() << "\n";
		std::cout << "Error offset: " << result.offset << " (error at [..." << (doc.text().as_string() + result.offset) << "]\n\n";

		return false;
	}

	auto sealNode = doc.child("seal");
	info.Seal.CurrentScore = id;

	info.Seal.Con = static_cast<short>(std::stoi(sealNode.child_value("CON")));
	info.Seal.Str = static_cast<short>(std::stoi(sealNode.child_value("STR")));
	info.Seal.Dex = static_cast<short>(std::stoi(sealNode.child_value("DEX")));
	info.Seal.Int = static_cast<short>(std::stoi(sealNode.child_value("INT")));

	info.Seal.Face = static_cast<short>(std::stoi(sealNode.child_value("face")));
	info.Seal.Level = static_cast<short>(std::stoi(sealNode.child_value("level")));
	info.Seal.QuestInfo = static_cast<short>(std::stoi(sealNode.child_value("quest")));
	info.Seal.Evolution = static_cast<short>(std::stoi(sealNode.child_value("evolution")));
	info.Seal.CapeId = static_cast<short>(std::stoi(sealNode.child_value("cape")));
	info.Seal.Unk_3 = static_cast<short>(std::stoi(sealNode.child_value("unk3")));
	
	auto skills = sealNode.child("skills");
	int i = 0;
	for (auto skill = skills.child("skill"); skill; skill = skill.next_sibling("skill"))
	{
		int index = skill.attribute("index").as_int();
		if (index < 0 || index >= 9)
			continue;

		auto value = static_cast<short>(skill.attribute("value").as_int());
		if (value == -1)
			continue;

		info.Seal.Skills[i++] = value;
	}

	XMLToStructure(sealNode.child("mob"), info.Mob);
	return true;
}

bool WriteSealInfo(int id, const STRUCT_SEALFILE& info)
{
	char temp[MAX_PATH] = { 0 };
	sprintf_s(temp, "./seal/%d.xml", id);

	pugi::xml_document doc;
	auto sealNode = doc.append_child("seal");
	
	sealNode.append_child("CON").append_child(pugi::node_pcdata).set_value(std::to_string(info.Seal.Con).c_str());
	sealNode.append_child("STR").append_child(pugi::node_pcdata).set_value(std::to_string(info.Seal.Str).c_str());
	sealNode.append_child("DEX").append_child(pugi::node_pcdata).set_value(std::to_string(info.Seal.Dex).c_str());
	sealNode.append_child("INT").append_child(pugi::node_pcdata).set_value(std::to_string(info.Seal.Int).c_str());

	sealNode.append_child("face").append_child(pugi::node_pcdata).set_value(std::to_string(info.Seal.Face).c_str());
	sealNode.append_child("level").append_child(pugi::node_pcdata).set_value(std::to_string(info.Seal.Level).c_str());
	sealNode.append_child("quest").append_child(pugi::node_pcdata).set_value(std::to_string(info.Seal.QuestInfo).c_str());
	sealNode.append_child("evolution").append_child(pugi::node_pcdata).set_value(std::to_string(info.Seal.Evolution).c_str());

	sealNode.append_child("cape").append_child(pugi::node_pcdata).set_value(std::to_string(info.Seal.CapeId).c_str());
	sealNode.append_child("unk3").append_child(pugi::node_pcdata).set_value(std::to_string(info.Seal.Unk_3).c_str());

	auto skillsNode = sealNode.append_child("skills");
	for (int i = 0; i < 9; i++)
	{
		auto skillNode = skillsNode.append_child("skill");

		skillNode.append_attribute("index").set_value(i);
		skillNode.append_attribute("value").set_value(info.Seal.Skills[i]);
	}

	AppendStructure(sealNode.append_child("mob"), const_cast<STRUCT_CHARINFO*>(&info.Mob));
	doc.save_file(temp);

	return true;
}