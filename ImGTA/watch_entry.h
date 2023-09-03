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
	kBitfield32,
	kArray
};

NLOHMANN_JSON_SERIALIZE_ENUM(WatchType, {
	{kInt, "int"},
	{kFloat, "float"},
	{kString, "string"},
	{kVector3, "vector3"},
	{kBitfield32, "bitfield32"},
	{kArray, "array"},
	})

class WatchEntry
{
public:
	WatchEntry(int addressIndex, WatchType type, WatchType arrayItemtype, std::string scriptName,
		int scriptHash, std::string info = std::string(""), int itemSizeQWORD = 0, int indexInItem = 0,
		bool isArrayItem = false) : m_addressIndex(addressIndex), m_type(type), m_arrayItemType(arrayItemtype),
		m_scriptName(scriptName), m_scriptHash(scriptHash), m_info(info), m_showInGame(true),
		m_itemSizeQWORD(itemSizeQWORD), m_isArrayItem(isArrayItem), m_arrayIndexInItem(indexInItem)
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
	//array
	int m_itemSizeQWORD = 0;
	int m_arrayIndexInItem = 0;
	WatchType m_arrayItemType = kInt;
	bool m_isArrayItem = false;
	std::vector<WatchEntry> m_arrayWatches;

	bool IsGlobal() { return m_scriptHash == 0; }
	void UpdateValue();
	void UpdateValue(int scriptHash);
	bool operator==(WatchEntry* other)
	{
		return (this->m_addressIndex == other->m_addressIndex
			&& this->m_scriptHash == other->m_scriptHash
			&& this->m_type == other->m_type);
	}
	NLOHMANN_DEFINE_TYPE_INTRUSIVE_WITH_DEFAULT(WatchEntry, m_addressIndex, m_type, m_scriptName, m_scriptHash, m_info, m_value, m_showInGame,
		m_itemSizeQWORD, m_arrayIndexInItem, m_arrayItemType, m_arrayWatches, m_isArrayItem)
};

std::string GetDisplayForType(uint64_t* globalAddr, WatchType type);
std::string GetDisplayForType(int addrId, WatchType type);
std::string GetDisplayForType(int addrId, int scriptHash, WatchType type);