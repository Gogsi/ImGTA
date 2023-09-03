/*
 * Copyright (c) 2021, James Puleo <james@jame.xyz>
 * Copyright (c) 2021, Rayope
 *
 * SPDX-License-Identifier: GPL-3.0-only
 */

#include "watch_entry.h"

#include "utils.h"

#include "types.h"
#include "main.h"

#include <bitset>
#include <cstdio>

void WatchEntry::UpdateValue()
{
	if (IsGlobal())
		m_value = GetDisplayForType(m_addressIndex, m_type);
	else if (m_scriptRunning)
		m_value = GetDisplayForType(m_addressIndex, m_scriptHash, m_type);

	if (m_type == kArray) {
		for (int i = 0; i < std::stoi(m_value); i++)
		{
			if (m_arrayWatches.size() <= i)
			{
				if (IsGlobal())
					m_arrayWatches.push_back(WatchEntry(m_addressIndex + 1 + i * m_itemSizeQWORD + m_arrayIndexInItem, m_arrayItemType, kInt, "Global", 0, "", 0, 0, true));
				else
					m_arrayWatches.push_back(WatchEntry(m_addressIndex + 1 + i * m_itemSizeQWORD + m_arrayIndexInItem, m_arrayItemType, kInt, m_scriptName, m_scriptHash, "", 0, 0, true));
			}
		}
	}

	for (auto& watch : m_arrayWatches)
		watch.UpdateValue();
}

void WatchEntry::UpdateValue(int scriptHash)
{
	m_value = GetDisplayForType(scriptHash, m_addressIndex, m_type);
}

std::string GetDisplayForType(uint64_t* globalAddr, WatchType type)
{
	if (globalAddr == nullptr)
		return std::string("NULL");

	char buf[112] = "";

	switch (type)
	{
	case WatchType::kInt:
	case WatchType::kArray:
		std::snprintf(buf, sizeof(buf), "%d", *(int*)globalAddr);
		break;
	case WatchType::kFloat:
		std::snprintf(buf, sizeof(buf), "%.4f", *(float*)globalAddr);
		break;
	case WatchType::kVector3:
		Vector3 vec = *(Vector3*)globalAddr;
		std::snprintf(buf, sizeof(buf), "(%.4f, %.4f, %.4f)", vec.x, vec.y, vec.z);
		break;
	case WatchType::kString:
		std::snprintf(buf, sizeof(buf), "\"%s\"", (char*)globalAddr);
		break;
	case WatchType::kBitfield32:
		std::snprintf(buf, sizeof(buf), "%s", std::bitset<32>(*globalAddr).to_string().c_str());
	}

	return std::string(buf);
}

std::string GetDisplayForType(int addrId, WatchType type)
{
	return GetDisplayForType(GetGlobalPtr(addrId), type);
}

std::string GetDisplayForType(int addrId, int scriptHash, WatchType type)
{
	return GetDisplayForType(GetThreadAddress(addrId, scriptHash), type);
}