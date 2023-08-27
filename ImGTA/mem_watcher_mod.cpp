/*
 * Copyright (c) 2021, James Puleo <james@jame.xyz>
 * Copyright (c) 2021, Rayope
 *
 * SPDX-License-Identifier: GPL-3.0-only
 */

#include "mem_watcher_mod.h"

#include "script.h"
#include "utils.h"
#include "global_id.h"

#include "natives.h"

#include "imgui.h"
#include "imgui_extras.h"

#include <vector>
#include <cstdio>
#include <inttypes.h>
#include <mutex>
#include <algorithm>
#include <bitset>

const char* watchTypeNames[] = { "Int", "Float", "String", "Vector3", "Bitfield32" };

void MemWatcherMod::Load()
{
	m_settings = m_dllObject.GetUserSettings().memWatcher;
	m_onlineVersion = NETWORK::_GET_ONLINE_VERSION();
	LoadWatches();
}

void MemWatcherMod::Unload()
{
	m_dllObject.GetUserSettings().memWatcher = m_settings;
	SaveWatches();
}

std::string MemWatcherMod::GetMemWatchFilePath()
{
	return m_dllObject.m_settingsFolder + m_fileMemWatch;
}

json GetDefaultJson() {
	json j{ {GetGameVersionString(), std::vector<WatchEntry>()} };
	return j;
}

void MemWatcherMod::LoadWatches()
{
	std::ifstream f_in(GetMemWatchFilePath());
	if (!f_in)
	{
		//create new file with default empty values
		std::ofstream f_out(GetMemWatchFilePath());
		f_out << GetDefaultJson();
		f_in.open(GetMemWatchFilePath());
	}
	if (json::accept(f_in))
	{
		//reset stream
		f_in.clear();
		f_in.seekg(0);
		//parse to m_watches
		json j = json::parse(f_in);
		m_watches = j.value(GetGameVersionString(), std::vector<WatchEntry>());
	}
}

void MemWatcherMod::SaveWatches()
{
	if (m_settings.saveGlobals)
	{
		std::ifstream f_in(GetMemWatchFilePath());
		std::ofstream f_out(GetMemWatchFilePath());
		json jsonNew{ { GetGameVersionString(), m_watches } };
		if (json::accept(f_in))
		{
			f_in.clear();
			f_in.seekg(0);
			//get previous json data and update with the current one to 
			//prevent losing watches for other game versions
			json jsonOld = json::parse(f_in);
			jsonOld.update(jsonNew);
			f_out << jsonOld;
		}
		else {
			//fail safe if user fucked the json
			f_out << jsonNew;
		}
	}
}

void MemWatcherMod::ClearSavedWatches()
{
	std::ofstream f_out(GetMemWatchFilePath());
	f_out << GetDefaultJson();
}

void MemWatcherMod::Think()
{
	if (m_watches.size() > 0)
	{
		std::lock_guard<std::mutex> lock(m_watchesMutex);

		char buf[112] = "";
		float xOff = m_settings.common.inGameOffsetX;
		float yOff = m_settings.common.inGameOffsetY;

		m_scriptHash = MISC::GET_HASH_KEY(m_scriptName.c_str());
		// Check if script is still running
		if (SCRIPT::_GET_NUMBER_OF_REFERENCES_OF_SCRIPT_WITH_NAME_HASH(m_scriptHash) > 0)
			m_scriptRunning = true;
		else
			m_scriptRunning = false;

		const float step = 1.2f * TextFontHeight(m_settings.common.inGameFontSize, m_font);
		const char* strFormat = m_settings.inputHexIndex ? "%s%s (0x%x) %s: %s" : "%s%s (%d) %s: %s";
		std::string bufferLines;
		const int bufferLinesCount = 2;
		int i = 0;
		yOff -= step * (bufferLinesCount - 1);
		for (auto& w : m_watches)
		{
			// Re-check if script is still running
			if (!w.IsGlobal())
			{
				if (SCRIPT::_GET_NUMBER_OF_REFERENCES_OF_SCRIPT_WITH_NAME_HASH(w.m_scriptHash) > 0)
					w.m_scriptRunning = true;
				else
					w.m_scriptRunning = false;
			}

			w.UpdateValue();

			if (m_dllObject.GetEnableHUD() && m_settings.common.showInGame && w.m_showInGame)
			{
				if (i % bufferLinesCount == 0)
					bufferLines = "";

				std::string infoDetail = (m_settings.displayHudInfo && w.m_info.size() > 0) ? (" (" + w.m_info + ")") : "";
				std::snprintf(buf, sizeof(buf), strFormat,
					w.m_scriptRunning ? "" : "(STOPPED) ",
					w.m_scriptName.c_str(),
					w.m_addressIndex,
					infoDetail.c_str(),
					w.m_value.c_str());
				bufferLines += std::string(buf) + "\n";

				if (i % bufferLinesCount == (bufferLinesCount - 1))
					DrawTextToScreen(bufferLines.c_str(), xOff, yOff, m_settings.common.inGameFontSize, m_font, false, m_settings.common.inGameFontRed, m_settings.common.inGameFontGreen, m_settings.common.inGameFontBlue);

				// Change column
				if (i % 30 == 29)
				{
					xOff += (m_settings.common.columnSpacing + step);
					yOff -= step * 30;
				}

				yOff += step;
				i++;
			}
		}
		if (m_dllObject.GetEnableHUD() && m_settings.common.showInGame)
		{
			if (i % bufferLinesCount == (bufferLinesCount - 1))
				DrawTextToScreen(bufferLines.c_str(), xOff, yOff, m_settings.common.inGameFontSize, m_font, false, m_settings.common.inGameFontRed, m_settings.common.inGameFontGreen, m_settings.common.inGameFontBlue);
		}

	}
}

void MemWatcherMod::SortWatches()
{
	std::lock_guard<std::mutex> lock(m_watchesMutex);
	std::sort(m_watches.begin(), m_watches.end(), CompareWatch);
}

void MemWatcherMod::ShowAddAddress(bool isGlobal)
{
	if (m_settings.inputHexIndex)
	{
		if (ImGui::InputInt("Hex Index##AddAddress", &m_inputAddressIndex, 1, 100, ImGuiInputTextFlags_CharsHexadecimal))
		{
			ClipInt(m_inputAddressIndex, 0, 999999);
			m_indexRange = 1;
			m_inputsUpdated = true;
		}
	}
	else
	{
		if (ImGui::InputInt("Decimal Index##AddAddress", &m_inputAddressIndex, 1, 100))
		{
			ClipInt(m_inputAddressIndex, 0, 999999);
			m_indexRange = 1;
			m_inputsUpdated = true;
		}
	}

	if (ImGui::InputInt("Range size##AddAddress", &m_indexRange))
		ClipInt(m_indexRange, 1, 100);

	if (ImGui::Combo("Type##AddAddress", &m_inputType, watchTypeNames, IM_ARRAYSIZE(watchTypeNames)))
		m_inputsUpdated = true;

	if (!isGlobal)
	{
		if (ImGui::InputText("Script Name##AddAddress", m_scriptNameBuf, sizeof(m_scriptNameBuf)))
		{
			m_scriptName = std::string(m_scriptNameBuf);
			m_dllObject.RunOnNativeThread([&]
				{
					m_scriptHash = MISC::GET_HASH_KEY(m_scriptName.c_str());
					if (SCRIPT::_GET_NUMBER_OF_REFERENCES_OF_SCRIPT_WITH_NAME_HASH(m_scriptHash) > 0)
						m_scriptRunning = true;
					else
						m_scriptRunning = false;
				});
			m_inputsUpdated = true;
		}
	}

	if (ImGui::InputText("Info##AddAddress", m_watchInfoBuf, sizeof(m_watchInfoBuf)))
		m_watchInfo = std::string(m_watchInfoBuf);

	if (isGlobal || (!isGlobal && m_scriptRunning))
	{
		if (ImGui::Button("Add##AddAddress"))
		{
			m_addressAvailable = true;
			if (isGlobal)
			{
				if (GetGlobalPtr(m_inputAddressIndex) == nullptr)
					m_addressAvailable = false;
			}
			else
			{
				if (GetThreadAddress(m_inputAddressIndex, m_scriptHash) == nullptr)
					m_addressAvailable = false;
			}

			if (m_addressAvailable)
			{
				std::lock_guard<std::mutex> lock(m_watchesMutex);

				// Check if the address is already watched
				int tmpScriptHash = isGlobal ? 0 : m_scriptHash;
				m_variableAlreadyWatched = false;
				for (const auto& watch : m_watches)
				{
					if (watch.m_addressIndex == m_inputAddressIndex
						&& watch.m_scriptHash == tmpScriptHash
						&& watch.m_type == m_inputType)
					{
						m_variableAlreadyWatched = true;
						break;
					}
				}

				if (!m_variableAlreadyWatched)
				{
					for (int i = 0; i < m_indexRange; i++)
					{
						if (isGlobal)
							m_watches.push_back(WatchEntry(m_inputAddressIndex + i, (WatchType)m_inputType, "Global", 0, m_watchInfo));
						else
							m_watches.push_back(WatchEntry(m_inputAddressIndex + i, (WatchType)m_inputType, m_scriptName, m_scriptHash, m_watchInfo));
					}
					m_autoScrollDown = true;
				}
			}

			// Reset error messages
			m_inputsUpdated = false;
		}

	}
	else // If a local index and script is not running
	{
		ImGui::TextColored(ImVec4(255, 0, 0, 255), "Script '%', is not running", m_scriptName.c_str());
	}
	// Error messages
	if (!m_inputsUpdated)
	{
		if (!m_addressAvailable)
			ImGui::TextColored(ImVec4(255, 0, 0, 255), "Cannot get memory address");

		if (m_variableAlreadyWatched)
			ImGui::TextColored(ImVec4(255, 0, 0, 255), "This variable is already on the watch list");
	}
}

void MemWatcherMod::ShowSelectedPopup()
{
	if (ImGui::BeginPopup("PopupEntryProperties"))
	{
		ImGui::Combo("Type##EntryProperties", (int*)&m_selectedEntry->m_type, watchTypeNames, IM_ARRAYSIZE(watchTypeNames));
		ImGui::Checkbox("Show Ingame##EntryProperties", &m_selectedEntry->m_showInGame);


		if (ImGui::InputText("Info##AddAddress", m_watchInfoModifyBuf, sizeof(m_watchInfoModifyBuf)))
			m_selectedEntry->m_info = std::string(m_watchInfoModifyBuf);
		else if (std::string(m_watchInfoModifyBuf) != m_selectedEntry->m_info)
			strncpy_s(m_watchInfoModifyBuf, sizeof(m_watchInfoModifyBuf), m_selectedEntry->m_info.c_str(), sizeof(m_watchInfoModifyBuf));

		uint64_t* val = nullptr;
		if (m_selectedEntry->IsGlobal())
		{
			val = GetGlobalPtr(m_selectedEntry->m_addressIndex);
		}
		else
		{
			val = GetThreadAddress(m_selectedEntry->m_addressIndex, m_selectedEntry->m_scriptHash);

			if (ImGui::InputText("Script Name##EntryProperties", m_scriptNameBuf, sizeof(m_scriptNameBuf)))
			{
				m_scriptName = std::string(m_scriptNameBuf);
				m_dllObject.RunOnNativeThread([&]
					{
						m_scriptHash = MISC::GET_HASH_KEY(m_scriptName.c_str());
						if (SCRIPT::_GET_NUMBER_OF_REFERENCES_OF_SCRIPT_WITH_NAME_HASH(m_scriptHash) > 0)
							m_selectedWatchScriptRunning = true;
						else
							m_selectedWatchScriptRunning = false;
					});
			}

			if (m_selectedWatchScriptRunning)
			{
				if (std::string(m_scriptNameBuf) != m_selectedEntry->m_scriptName)
					strncpy_s(m_scriptNameBuf, sizeof(m_scriptNameBuf), m_selectedEntry->m_scriptName.c_str(), sizeof(m_scriptNameBuf));

				m_selectedEntry->m_scriptName = m_scriptName;
				m_selectedEntry->m_scriptHash = m_scriptHash;
				m_selectedWatchScriptRunning = false;
			}
		}

		if (val)
		{
			switch (m_selectedEntry->m_type)
			{
			case WatchType::kBitfield32:
				ImGuiExtras::BitField("Value##WatchValueBitfield", (unsigned int*)val, nullptr);
				if (ImGui::Button("LS<<##WatchLBitshift"))
					*val = *val << 1;
				if (ImGui::Button(">>RS##WatchRBitshift"))
					*val = *val >> 1;
				break;
			case WatchType::kInt:
				ImGui::InputInt("Value##WatchValue", (int*)val);
				break;
			case WatchType::kFloat:
				ImGui::InputFloat("Value##WatchValue", (float*)val, 0.0f, 0.0f, "%.4f");
				break;
			case WatchType::kVector3:
				ImGuiExtras::InputVector3("WatchValue", (Vector3*)val);
				break;
			case WatchType::kString:
				ImGui::TextDisabled("Cannot edit string.");
				break;
			}
		}

		if (ImGui::Button("Remove##EntryProperties"))
		{
			std::lock_guard<std::mutex> lock(m_watchesMutex);
			m_watches.erase(std::remove(m_watches.begin(), m_watches.end(), m_selectedEntry), m_watches.end());
			ImGui::CloseCurrentPopup();
		}

		ImGui::EndPopup();
	}
}

void MemWatcherMod::DrawMenuBar()
{
	bool openPopup = false;
	if (ImGui::BeginMenuBar())
	{
		if (ImGui::BeginMenu("Watch"))
		{
			if (ImGui::BeginMenu("Add Global Index"))
			{
				ShowAddAddress(true);
				ImGui::EndMenu();
			}
			if (m_supportGlobals)
			{
				if (ImGui::BeginMenu("Add Local Index"))
				{
					ShowAddAddress(false);
					ImGui::EndMenu();
				}
			}
			if (ImGui::MenuItem("Sort all watches"))
				SortWatches();

			if (ImGui::MenuItem("Clear"))
			{
				std::lock_guard<std::mutex> lock(m_watchesMutex);
				m_watches.clear();
			}

			if (ImGui::MenuItem("Clear JSON"))
				openPopup = true;

			ImGui::EndMenu();
		}

		//https://github.com/ocornut/imgui/issues/331#issuecomment-140055181
		if (openPopup)
			ImGui::OpenPopup("Are you sure?");

		if (ImGui::BeginPopupModal("Are you sure?", nullptr, ImGuiWindowFlags_AlwaysAutoResize))
		{
			ImGui::Text("Are you sure you want to clear JSON?");
			if (ImGui::Button("Yes"))
			{
				ClearSavedWatches();
				ImGui::CloseCurrentPopup();
			}
			ImGui::SameLine();
			if (ImGui::Button("No"))
				ImGui::CloseCurrentPopup();
			ImGui::EndPopup();
		}

		ImGui::Separator();
		ImGui::Checkbox("##Enable HUD", &m_settings.common.showInGame);

		if (ImGui::BeginMenu("HUD"))
		{
			DrawCommonSettingsMenus(m_settings.common);

			ImGui::Separator();
			ImGui::MenuItem("Hexadecimal index", NULL, &m_settings.inputHexIndex);
			ImGui::MenuItem("Display information detail", NULL, &m_settings.displayHudInfo);

			ImGui::EndMenu();
		}
		ImGui::Checkbox("Save to JSON", &m_settings.saveGlobals);

		ImGui::EndMenuBar();
	}
}

bool MemWatcherMod::Draw()
{
	ImGui::SetWindowFontScale(m_settings.common.menuFontSize);
	DrawMenuBar();

	ImGui::SetWindowFontScale(m_settings.common.contentFontSize);
	ImGui::TextColored(ImVec4(255, 0, 0, 255), "Game online version: %s. "
		"Variable indexes are dependent on the game version.", m_onlineVersion.c_str());

	char buf[112] = "";
	const char* indexFormat = m_settings.inputHexIndex ? "0x%x##%d%d" : "%d##%d%d";

	ImGui::Columns(5);
	ImGui::Separator();
	ImGui::Text("Index"); ImGui::NextColumn();
	ImGui::Text("Type"); ImGui::NextColumn();
	ImGui::Text("Script (Hash)"); ImGui::NextColumn();
	ImGui::Text("Info"); ImGui::NextColumn();
	ImGui::Text("Value"); ImGui::NextColumn();
	ImGui::Separator();

	m_watchesMutex.lock();
	if (m_watches.size() > 0)
	{
		for (auto& w : m_watches)
		{
			std::snprintf(buf, sizeof(buf), indexFormat, w.m_addressIndex, w.m_addressIndex, w.m_scriptHash, w.m_type);

			if (ImGui::Selectable(buf, false, ImGuiSelectableFlags_SpanAllColumns))
			{
				m_selectedEntry = &w;
				ImGui::OpenPopup("PopupEntryProperties");
			}

			ImGui::NextColumn();
			ImGui::Text("%s", watchTypeNames[w.m_type]); ImGui::NextColumn();
			ImGui::Text("%s (%d)", w.m_scriptName.c_str(), w.m_scriptHash); ImGui::NextColumn();
			ImGui::Text("%s", w.m_info.c_str()); ImGui::NextColumn();
			ImGui::Text("%s", w.m_value.c_str()); ImGui::NextColumn();
		}
		ImGui::Columns(1);
		ImGui::Separator();
	}
	m_watchesMutex.unlock();

	if (m_autoScrollDown)
	{
		ImGui::SetScrollHereY(1.0f);
		m_autoScrollDown = false;
	}

	ShowSelectedPopup();
	return true;
}

bool CompareWatch(WatchEntry a, WatchEntry b)
{
	bool smaller = false;
	// Make sure globals are at the top
	if (a.m_scriptName == "Global")
		a.m_scriptName = "000Global";
	if (b.m_scriptName == "Global")
		b.m_scriptName = "000Global";

	// Order by script name
	if (a.m_scriptName < b.m_scriptName)
		smaller = true;
	else if (a.m_scriptName == b.m_scriptName)
	{
		// If equal, order by index
		if (a.m_addressIndex < b.m_addressIndex)
			smaller = true;
		else if (a.m_addressIndex == b.m_addressIndex)
		{
			// If equal, order by type
			if (a.m_type < b.m_type)
				smaller = true;
		}
	}
	return smaller;
}
