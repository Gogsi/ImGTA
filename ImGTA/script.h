/*
 * Copyright (c) 2021, James Puleo <james@jame.xyz>
 * Copyright (c) 2021, Rayope
 *
 * SPDX-License-Identifier: GPL-3.0-only
 */

#pragma once
#include "lua_engine.h"
#include "user_settings.h"

#include <Windows.h>
#include <functional>
#include <mutex>
#include <chrono>

struct IDXGISwapChain;
class Mod;
class MissionMod;

class DLLObject
{
public:
	DLLObject();
	~DLLObject();

	void InitMods();
	void Update();
	void UpdateWindows();

	// Relying on this function is not recommended!
	void RunOnNativeThread(std::function<void()> func);

	void SetAllWindowCollapsed(bool collapse);
	void SetShowAllInGame(bool show);
	void SetTextDrawMaxWarning(bool toggle);
	void SetAllFontSize(float menuSize, float contentSize, float ingameSize);
	void SetAllInGameFontColor(int red, int green, int blue);
	void Load();
	void Unload();
	void ToggleOpen();
	void PassCurrentMission();
	void OnPresent(IDXGISwapChain *swap);
	
	std::string GetSettingsFolder() { return m_settingsFolder; }
	bool HasInitializedImGui() { return m_hasInitializedImgui; }
	LONG_PTR GetOldProc() { return m_oldProc; }
	void SetOldProc(LONG_PTR proc) { m_oldProc = proc; }
	UserSettings & GetUserSettings() { return m_userSettings.Get(); }
	bool GetIsOpen() { return m_isOpen; }
	void SetIsOpen(bool open) { m_isOpen = open; }
	bool GetFloatingMenu() { return m_floatingMenu; }
	void SetFloatingMenu(bool isFloating) { m_floatingMenu = isFloating; }
	void SetEnableHUD(bool enable) { m_enableHUD = enable; }
	bool GetEnableHUD() { return m_enableHUD; }
	bool m_updatedPauseMenuOn = false;
	bool m_updatedPauseMenuOff = false;
	std::string m_settingsFolder = "ImGTA/";

private:
	std::string m_fileImGuiIni = m_settingsFolder + "Settings_imgui.ini";
	std::string m_fileImGuiLog = m_settingsFolder + "Log_imgui.txt";
	std::string m_userSettingsFile = m_settingsFolder + "Settings_HUD.ini";
	UserSettingsManager m_userSettings;
	std::vector<Mod *> m_modsLoaded;
	MissionMod* missionMod;
	bool m_hasInitializedImgui;
	LONG_PTR m_oldProc = 0;
	bool m_isOpen = false;
	bool m_isLoaded = false;
	bool m_floatingMenu = false;
	bool m_enableHUD = true;

	// TODO: Update this bad design for ON/OFF
	bool m_updatedTextDrawMaxWarningOn = false;
	bool m_updatedTextDrawMaxWarningOff = false;

	std::vector<std::function<void()>> m_toRun;
	std::mutex m_toRunMutex;
	LuaEngine m_luaEngine{m_settingsFolder};
};
