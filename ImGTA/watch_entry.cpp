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
	else if(m_scriptRunning)
		m_value = GetDisplayForType(m_addressIndex, m_scriptHash, m_type);
}

void WatchEntry::UpdateValue(int scriptHash)
{
	m_value = GetDisplayForType(scriptHash, m_addressIndex, m_type);
}

std::string GetDisplayForType(uint64_t *globalAddr, WatchType type)
{
	if (globalAddr == nullptr)
		return std::string("NULL");

	char buf[112] = "";

	switch (type)
	{
	case WatchType::kInt:
		std::snprintf(buf, sizeof(buf), "%d", *(int *)globalAddr);
		break;
	case WatchType::kFloat:
		std::snprintf(buf, sizeof(buf), "%.4f", *(float *)globalAddr);
		break;
	case WatchType::kVector3:
		Vector3 vec = *(Vector3 *)globalAddr;
		std::snprintf(buf, sizeof(buf), "(%.4f, %.4f, %.4f)", vec.x, vec.y, vec.z);
		break;
	case WatchType::kString:
		std::snprintf(buf, sizeof(buf), "\"%s\"", (char *)globalAddr);
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