/*
 * Copyright (c) 2021, Rayope
 *
 * SPDX-License-Identifier: GPL-3.0-only
 */

#include "user_settings.h"

void UserSettingsManager::Load(const std::string & filename) {
	std::ifstream file(filename);
	std::string line;

	while (std::getline(file, line)) {
		std::istringstream current(line.substr(line.find("=") + 1));

		// Common
		LoadCommon(line, current, s.area.common, "area.");
		LoadCommon(line, current, s.audio.common, "audio.");
		LoadCommon(line, current, s.cheats.common, "cheats.");
		LoadCommon(line, current, s.comms.common, "comms.");
		LoadCommon(line, current, s.cutscene.common, "cutscene.");
		LoadCommon(line, current, s.handleHelper.common, "handleHelper.");
		LoadCommon(line, current, s.luaConsole.common, "luaConsole.");
		LoadCommon(line, current, s.memWatcher.common, "memWatcher.");
		LoadCommon(line, current, s.mission.common, "mission.");
		LoadCommon(line, current, s.scripts.common, "scripts.");
		LoadCommon(line, current, s.playerSwitch.common, "playerSwitch.");
		LoadCommon(line, current, s.syncScene.common, "syncScene.");

		// Area
		if (line.find("area.drawOffsetZ") != -1)
			current >> s.area.drawOffsetZ;
		else if (line.find("area.drawBox") != -1)
			current >> s.area.drawBox;
		// Audio
		// Cheats
		else if (line.find("cheats.enableInGameHUD") != -1)
			current >> s.cheats.enableInGameHUD;
		else if (line.find("cheats.showGeneralInfo") != -1)
			current >> s.cheats.showGeneralInfo;
		else if (line.find("cheats.showAvailableInputs") != -1)
			current >> s.cheats.showAvailableInputs;
		else if (line.find("cheats.floatingMenu") != -1)
			current >> s.cheats.floatingMenu;
		else if (line.find("cheats.displayKMH") != -1)
			current >> s.cheats.displayKMH;
		else if (line.find("cheats.showPosition") != -1)
			current >> s.cheats.showPosition;
		else if (line.find("cheats.showEntityHealth") != -1)
			current >> s.cheats.showEntityHealth;
		else if (line.find("cheats.showEntityArmour") != -1)
			current >> s.cheats.showEntityArmour;
		else if (line.find("cheats.showCurrentSpeed") != -1)
			current >> s.cheats.showCurrentSpeed;
		else if (line.find("cheats.showCurrentAcceleration") != -1)
			current >> s.cheats.showCurrentAcceleration;
		else if (line.find("cheats.showMaxSpeed") != -1)
			current >> s.cheats.showMaxSpeed;
		else if (line.find("cheats.showMaxAcceleration") != -1)
			current >> s.cheats.showMaxAcceleration;
		else if (line.find("cheats.showSpeedVector") != -1)
			current >> s.cheats.showSpeedVector;
		else if (line.find("cheats.showSprintTimeRemaining") != -1)
			current >> s.cheats.showSprintTimeRemaining;
		else if (line.find("cheats.showUnderwaterTimeRemaining") != -1)
			current >> s.cheats.showUnderwaterTimeRemaining;
		else if (line.find("cheats.showInvincible") != -1)
			current >> s.cheats.showInvincible;
		else if (line.find("cheats.showCanRagdoll") != -1)
			current >> s.cheats.showCanRagdoll;
		else if (line.find("cheats.showStreamingRequests") != -1)
			current >> s.cheats.showStreamingRequests;
		else if (line.find("cheats.showPedHandle") != -1)
			current >> s.cheats.showPedHandle;
		else if (line.find("cheats.showGameTime") != -1)
			current >> s.cheats.showGameTime;
		else if (line.find("cheats.showGameDateTime") != -1)
			current >> s.cheats.showGameDateTime;
		// Comms
		// Cutscene
		// HandleHelper
		else if (line.find("handleHelper.nearbyObjectMax") != -1)
			current >> s.handleHelper.nearbyObjectMax;
		else if (line.find("handleHelper.drawTrackedStats") != -1)
			current >> s.handleHelper.drawTrackedStats;
		else if (line.find("handleHelper.drawOffsetZ") != -1)
			current >> s.handleHelper.drawOffsetZ;
		else if (line.find("handleHelper.drawOnScreenEntityOnly") != -1)
			current >> s.handleHelper.drawOnScreenEntityOnly;
		else if (line.find("handleHelper.drawId") != -1)
			current >> s.handleHelper.drawId;
		else if (line.find("handleHelper.drawLife") != -1)
			current >> s.handleHelper.drawLife;
		// LuaConsole
		else if (line.find("luaConsole.showInfo") != -1)
			current >> s.luaConsole.showInfo;
		// MemWatcher
		else if (line.find("memWatcher.sortByName") != -1)
			current >> s.memWatcher.sortByName;
		else if (line.find("memWatcher.inputHexIndex") != -1)
			current >> s.memWatcher.inputHexIndex;
		else if (line.find("memWatcher.saveGlobals") != -1)
			current >> s.memWatcher.saveGlobals;
		else if (line.find("memWatcher.displayHudInfo") != -1)
			current >> s.memWatcher.displayHudInfo;
		// Mission
		// Scripts
		else if (line.find("scripts.sortByName") != -1)
			current >> s.scripts.sortByName;
		// PlayerSwitch
		// SyncScene
	}
}

void UserSettingsManager::Save(const std::string & filename) {
	std::ofstream file(filename);

	// Common
	SaveCommon(file, s.area.common, "area.");
	SaveCommon(file, s.audio.common, "audio.");
	SaveCommon(file, s.cheats.common, "cheats.");
	SaveCommon(file, s.comms.common, "comms.");
	SaveCommon(file, s.cutscene.common, "cutscene.");
	SaveCommon(file, s.handleHelper.common, "handleHelper.");
	SaveCommon(file, s.luaConsole.common, "luaConsole.");
	SaveCommon(file, s.mission.common, "mission.");
	SaveCommon(file, s.scripts.common, "scripts.");
	SaveCommon(file, s.playerSwitch.common, "playerSwitch.");
	SaveCommon(file, s.syncScene.common, "syncScene.");
	
	// Area
	file << "area.drawOffsetZ = " << s.area.drawOffsetZ << std::endl;
	file << "area.drawBox = " << s.area.drawBox << std::endl;
	// Audio
	// Cheats
	file << "cheats.enableInGameHUD = " << s.cheats.enableInGameHUD << std::endl;
	file << "cheats.showGeneralInfo = " << s.cheats.showGeneralInfo << std::endl;
	file << "cheats.showAvailableInputs = " << s.cheats.showAvailableInputs << std::endl;
	file << "cheats.floatingMenu = " << s.cheats.floatingMenu << std::endl;
	file << "cheats.displayKMH = " << s.cheats.displayKMH << std::endl;
	file << "cheats.showPosition = " << s.cheats.showPosition << std::endl;
	file << "cheats.showEntityHealth = " << s.cheats.showEntityHealth << std::endl;
	file << "cheats.showEntityArmour = " << s.cheats.showEntityArmour << std::endl;
	file << "cheats.showCurrentSpeed = " << s.cheats.showCurrentSpeed << std::endl;
	file << "cheats.showCurrentAcceleration = " << s.cheats.showCurrentAcceleration << std::endl;
	file << "cheats.showMaxSpeed = " << s.cheats.showMaxSpeed << std::endl;
	file << "cheats.showMaxAcceleration = " << s.cheats.showMaxAcceleration << std::endl;
	file << "cheats.showSpeedVector = " << s.cheats.showSpeedVector << std::endl;
	file << "cheats.showSprintTimeRemaining = " << s.cheats.showSprintTimeRemaining << std::endl;
	file << "cheats.showUnderwaterTimeRemaining = " << s.cheats.showUnderwaterTimeRemaining << std::endl;
	file << "cheats.showInvincible = " << s.cheats.showInvincible << std::endl;
	file << "cheats.showCanRagdoll = " << s.cheats.showCanRagdoll << std::endl;
	file << "cheats.showStreamingRequests = " << s.cheats.showStreamingRequests << std::endl;
	file << "cheats.showPedHandle = " << s.cheats.showPedHandle << std::endl;
	file << "cheats.showGameTime = " << s.cheats.showGameTime << std::endl;
	file << "cheats.showGameDateTime = " << s.cheats.showGameDateTime << std::endl;
	// Comms
	// Cutscene
	// HandleHelper
	file << "handleHelper.nearbyObjectMax = " << s.handleHelper.nearbyObjectMax << std::endl;
	file << "handleHelper.drawTrackedStats = " << s.handleHelper.drawTrackedStats << std::endl;
	file << "handleHelper.drawOffsetZ = " << s.handleHelper.drawOffsetZ << std::endl;
	file << "handleHelper.drawOnScreenEntityOnly = " << s.handleHelper.drawOnScreenEntityOnly << std::endl;
	file << "handleHelper.drawId = " << s.handleHelper.drawId << std::endl;
	file << "handleHelper.drawLife = " << s.handleHelper.drawLife << std::endl;
	// LuaConsole
	file << "luaConsole.showInfo = " << s.luaConsole.showInfo << std::endl;
	// MemWatcher
	file << "memWatcher.sortByName = " << s.memWatcher.sortByName << std::endl;
	file << "memWatcher.inputHexIndex = " << s.memWatcher.inputHexIndex << std::endl;
	file << "memWatcher.displayHudInfo = " << s.memWatcher.displayHudInfo << std::endl;
	file << "memWatcher.saveGlobals = " << s.memWatcher.saveGlobals << std::endl;
	// Mission
	// Scripts
	file << "scripts.sortByName = " << s.scripts.sortByName << std::endl;
	// PlayerSwitch
	// SyncScene
}

void UserSettingsManager::LoadCommon(std::string & line, std::istringstream & stream, CommonSettings & common, const std::string & prefix)
{
	if (line.find(prefix + "common.showInGame") != -1)
		stream >> common.showInGame;
	if (line.find(prefix + "common.autoUpdate") != -1)
		stream >> common.autoUpdate;
	else if (line.find(prefix + "common.menuFontSize") != -1)
		stream >> common.menuFontSize;
	else if (line.find(prefix + "common.contentFontSize") != -1)
		stream >> common.contentFontSize;
	else if (line.find(prefix + "common.inGameFontSize") != -1)
		stream >> common.inGameFontSize;
	else if (line.find(prefix + "common.columnSpacing") != -1)
		stream >> common.columnSpacing;
	else if (line.find(prefix + "common.inGameFontRed") != -1)
		stream >> common.inGameFontRed;
	else if (line.find(prefix + "common.inGameFontGreen") != -1)
		stream >> common.inGameFontGreen;
	else if (line.find(prefix + "common.inGameFontBlue") != -1)
		stream >> common.inGameFontBlue;
	else if (line.find(prefix + "common.inGameOffsetX") != -1)
		stream >> common.inGameOffsetX;
	else if (line.find(prefix + "common.inGameOffsetY") != -1)
		stream >> common.inGameOffsetY;
}

void UserSettingsManager::SaveCommon(std::ofstream & file, CommonSettings & common, const std::string & prefix)
{
	file << prefix + "common.showInGame = " << common.showInGame << std::endl;
	file << prefix + "common.autoUpdate = " << common.autoUpdate << std::endl;
	file << prefix + "common.menuFontSize = " << common.menuFontSize << std::endl;
	file << prefix + "common.contentFontSize = " << common.contentFontSize << std::endl;
	file << prefix + "common.inGameFontSize = " << common.inGameFontSize << std::endl;
	file << prefix + "common.columnSpacing = " << common.columnSpacing << std::endl;
	file << prefix + "common.inGameFontRed = " << common.inGameFontRed << std::endl;
	file << prefix + "common.inGameFontGreen = " << common.inGameFontGreen << std::endl;
	file << prefix + "common.inGameFontBlue = " << common.inGameFontBlue << std::endl;
	file << prefix + "common.inGameOffsetX = " << common.inGameOffsetX << std::endl;
	file << prefix + "common.inGameOffsetY = " << common.inGameOffsetY << std::endl;
}