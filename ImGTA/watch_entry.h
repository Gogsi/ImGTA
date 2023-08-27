/*
 * Copyright (c) 2021, James Puleo <james@jame.xyz>
 * Copyright (c) 2021, Rayope
 *
 * SPDX-License-Identifier: GPL-3.0-only
 */

#pragma once
#include <string>
#include <json.hpp>
using json = nlohmann::json;

enum WatchType
{
	kInt,
	kFloat,
	kString,
	kVector3,
	kBitfield32
};

NLOHMANN_JSON_SERIALIZE_ENUM(WatchType, {
	{kInt, "int"},
	{kFloat, "float"},
	{kString, "string"},
	{kVector3, "vector3"},
	{kBitfield32, "bitfield32"},
	})

class WatchEntry
{
public:
	WatchEntry(int addressIndex, WatchType type, std::string scriptName, int scriptHash, std::string info = std::string("")) : m_addressIndex(addressIndex), m_type(type), m_scriptName(scriptName), m_scriptHash(scriptHash), m_info(info), m_showInGame(true)
	{}

	WatchEntry() = default;
	int m_addressIndex;
	std::string m_scriptName;
	int m_scriptHash = 0;
	bool m_scriptRunning = true;
	WatchType m_type;
	std::string m_info;
	std::string m_value;
	bool m_showInGame;

	bool IsGlobal() { return m_scriptHash == 0; }
	void UpdateValue();
	void UpdateValue(int scriptHash);
	bool operator==(WatchEntry* other)
	{
		return (this->m_addressIndex == other->m_addressIndex
			&& this->m_scriptHash == other->m_scriptHash
			&& this->m_type == other->m_type);
	}
	NLOHMANN_DEFINE_TYPE_INTRUSIVE(WatchEntry, m_addressIndex, m_type, m_scriptName, m_scriptHash, m_info, m_value, m_showInGame)
};

std::string GetDisplayForType(uint64_t* globalAddr, WatchType type);
std::string GetDisplayForType(int addrId, WatchType type);
std::string GetDisplayForType(int addrId, int scriptHash, WatchType type);