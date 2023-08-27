/*
 * Copyright (c) 2021, James Puleo <james@jame.xyz>
 * Copyright (c) 2021, Rayope
 *
 * SPDX-License-Identifier: GPL-3.0-only
 */

#include "script.h"

#include "mod.h"
#include "mem_watcher_mod.h"
#include "cheats_mod.h"
#include "cutscene_mod.h"
#include "audio_mod.h"
#include "handle_helper_mod.h"
#include "scripts_mod.h"
#include "sync_scene_mod.h"
#include "player_switch_mod.h"
#include "comms_mod.h"
#include "mission_mod.h"
#include "area_mod.h"
#include "test_mod.h"
#include "lua_console_mod.h"
#include "utils.h"

#include "natives.h"

#include "imgui.h"
#include "imgui_internal.h"
#include "imgui_impl_win32.h"
#include "imgui_impl_dx11.h"

#include <d3d11.h>
#include <vector>
#include <mutex>
#include <cstdio>
#include <iostream>

LRESULT __stdcall WndProc(HWND hand, UINT msg, WPARAM wParam, LPARAM lParam);

DLLObject::DLLObject()
{
	
}

DLLObject::~DLLObject()
{
	// Make sure it's unloaded
	Unload();
}

void DLLObject::InitMods()
{
	bool supportGlobals = IsVersionSupportedForGlobals(getGameVersion());

	m_modsLoaded.push_back(new MemWatcherMod(*this, supportGlobals));
	m_modsLoaded.push_back(new CheatsMod(*this, supportGlobals));
	m_modsLoaded.push_back(new CutsceneMod(*this, supportGlobals));
	m_modsLoaded.push_back(new AudioMod(*this, supportGlobals));
	m_modsLoaded.push_back(new ScriptsMod(*this, supportGlobals));
	m_modsLoaded.push_back(new HandleHelperMod(*this, supportGlobals));
	m_modsLoaded.push_back(new SyncSceneMod(*this, supportGlobals));
	m_modsLoaded.push_back(new PlayerSwitchMod(*this, supportGlobals));
	m_modsLoaded.push_back(new AreaMod(*this, supportGlobals));
	m_modsLoaded.push_back(new LuaConsoleMod(*this, supportGlobals, m_luaEngine));
	if (supportGlobals)
	{
		m_modsLoaded.push_back(new CommsMod(*this, supportGlobals));
		m_modsLoaded.push_back(missionMod = new MissionMod(*this, supportGlobals));
	}
	//modsLoaded.push_back(new TestMod(supportGlobals));
}

void DLLObject::Update()
{
	ResetTextDrawCount();
	if (m_isOpen)
	{
		if (m_floatingMenu)
			PAD::ENABLE_ALL_CONTROL_ACTIONS(0);
		else
			PAD::DISABLE_ALL_CONTROL_ACTIONS(0);
	}

	m_toRunMutex.lock();
	for (auto &f : m_toRun)
		f();
	m_toRun.clear();
	m_toRunMutex.unlock();

	for (Mod * m : m_modsLoaded)
		m->Think();

	// Display a warning message if too much calls to DrawTextToScreen
	if (GetTextDrawCount() > 100)
	{
		if (!m_updatedTextDrawMaxWarningOn)
		{
			SetTextDrawMaxWarning(true);
			m_updatedTextDrawMaxWarningOn = true;
			m_updatedTextDrawMaxWarningOff = false;
		}
	}
	else
	{
		if (!m_updatedTextDrawMaxWarningOff)
		{
			SetTextDrawMaxWarning(false);
			m_updatedTextDrawMaxWarningOff = true;
			m_updatedTextDrawMaxWarningOn = false;
		}
	}
}

void DLLObject::UpdateWindows()
{
	ImGui_ImplDX11_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();

	for (auto &m : m_modsLoaded)
	{
		if (m->HasWindow())
			ImGui::Begin(m->GetName().c_str(), nullptr, m->m_windowFlags);
		if (m->Draw() && m->HasWindow())
			ImGui::End();
	}

	ImGui::Render();
	ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
}

void DLLObject::RunOnNativeThread(std::function<void()> func)
{
	m_toRun.push_back(func);
}

void DLLObject::SetAllWindowCollapsed(bool collapse)
{
	for (auto &m : m_modsLoaded)
		m->SetWindowCollapsed(collapse);
}

void DLLObject::SetShowAllInGame(bool show)
{
	for (auto &m : m_modsLoaded)
		m->SetShowInGame(show);
}

void DLLObject::SetTextDrawMaxWarning(bool toggle)
{
	for (auto &m : m_modsLoaded)
		m->SetTextDrawMaxWarning(toggle);
}

void DLLObject::SetAllFontSize(float menuSize, float contentSize, float inGameSize)
{
	for (auto &m : m_modsLoaded)
		m->SetFontSize(menuSize, contentSize, inGameSize);
}

void DLLObject::SetAllInGameFontColor(int red, int green, int blue)
{
	for (auto &m : m_modsLoaded)
		m->SetInGameFontColor(red, green, blue);
}


void DLLObject::Load()
{
	if (!m_isLoaded)
	{
		m_userSettings.Load(m_userSettingsFile);
		InitMods();
		for (auto &m : m_modsLoaded)
			m->Load();
		MISC::SET_THIS_SCRIPT_CAN_BE_PAUSED(false);
		m_isLoaded = true;
	}
		
	// After loading a save, ScriptHookV does not call ProcessDetach,
	// but ProcessAttach again

	while (true)
	{
		Update();
		WAIT(0);
	}
}

void DLLObject::Unload()
{
	if (m_isLoaded)
	{
		m_isOpen = false;

		// Unload mods
		for (auto &m : m_modsLoaded)
		{
			m->Unload();
			delete m;
		}
		m_modsLoaded.clear();

		// Save settings
		m_userSettings.Save(m_userSettingsFile);

		ImGui_ImplDX11_Shutdown();
		ImGui_ImplWin32_Shutdown();
		ImGui::DestroyContext();
		if (m_oldProc)
			SetWindowLongPtr(FindMainWindow(GetCurrentProcessId()), GWLP_WNDPROC, m_oldProc);
		m_isLoaded = false;
	}
}

void DLLObject::ToggleOpen()
{
	m_isOpen = !m_isOpen;
	ImGui::GetIO().MouseDrawCursor = m_isOpen;
}

void DLLObject::PassCurrentMission()
{
	if(missionMod != nullptr)
		missionMod->PassCurrentMission();
}

void DLLObject::OnPresent(IDXGISwapChain *swap)
{
	if (!m_hasInitializedImgui)
	{
		m_hasInitializedImgui = true;
		ID3D11Device *device;
		ID3D11DeviceContext *context;

		swap->GetDevice(__uuidof(ID3D11Device), (void **)&device);
		device->GetImmediateContext(&context);

		ImGui::CreateContext();
		ImGui::StyleColorsDark();
		ImGui::GetIO().ConfigFlags = ImGuiConfigFlags_NavEnableGamepad;
		ImGui::GetIO().IniFilename = m_fileImGuiIni.c_str();
		ImGui::GetIO().LogFilename = m_fileImGuiLog.c_str();

		HWND window = FindMainWindow(GetCurrentProcessId());
		ImGui_ImplWin32_Init(window);
		ImGui_ImplDX11_Init(device, context);

		m_oldProc = SetWindowLongPtr(window, GWLP_WNDPROC, (LONG_PTR)WndProc);
	}

	if (m_isOpen)
		UpdateWindows();
}
