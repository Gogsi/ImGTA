/*
 * Copyright (c) 2021, James Puleo <james@jame.xyz>
 * Copyright (c) 2021, Rayope
 *
 * SPDX-License-Identifier: GPL-3.0-only
 */

#include "utils.h"

#include "mod.h"

#include "natives.h"

#include <bitset>

ThreadBasket threadBasket;
int textDrawCount = 0;

BOOL IsMainWindow(HWND handle)
{
	return GetWindow(handle, GW_OWNER) == (HWND)0 && IsWindowVisible(handle);
}

BOOL CALLBACK EnumWindowsCallback(HWND handle, LPARAM lParam)
{
	handle_data& data = *(handle_data*)lParam;
	unsigned long process_id = 0;
	GetWindowThreadProcessId(handle, &process_id);
	if (data.processID != process_id || !IsMainWindow(handle))
		return TRUE;
	data.windowHandle = handle;
	return FALSE;
}

// https://stackoverflow.com/questions/1888863/how-to-get-main-window-handle-from-process-id
HWND FindMainWindow(unsigned long process_id)
{
	handle_data data;
	data.processID = process_id;
	data.windowHandle = 0;
	EnumWindows(EnumWindowsCallback, (LPARAM)&data);
	return data.windowHandle;
}

void ResetTextDrawCount()
{
	textDrawCount = 0;
}

int GetTextDrawCount()
{
	return textDrawCount;
}

float TextFontHeight(float size, eFont font)
{
	return HUD::GET_RENDERED_CHARACTER_HEIGHT(size, font);
}

void DrawTextToScreen(const char* text, float x, float y, float scale, eFont font, bool alignRight, int red, int green, int blue)
{
	// WARNING: Only the first 100 calls to this function are displayed!
	HUD::SET_TEXT_FONT((int)font);
	HUD::SET_TEXT_SCALE(scale, scale);
	HUD::SET_TEXT_COLOUR(red, green, blue, 255);
	HUD::SET_TEXT_WRAP(0.0, 1.0);
	HUD::SET_TEXT_CENTRE(false);
	HUD::SET_TEXT_RIGHT_JUSTIFY(alignRight);
	HUD::SET_TEXT_DROPSHADOW(2, 2, 0, 0, 0);
	HUD::SET_TEXT_EDGE(1, 0, 0, 0, 205);
	HUD::BEGIN_TEXT_COMMAND_DISPLAY_TEXT((char*)"STRING");
	HUD::ADD_TEXT_COMPONENT_SUBSTRING_KEYBOARD_DISPLAY((char*)text);
	HUD::END_TEXT_COMMAND_DISPLAY_TEXT(x, y, 0);
	textDrawCount++;
}

void ClipInt(int& value, int min, int max)
{
	if (value < min)
		value = min;
	else if (value > max)
	{
		if (max > min)
			value = max;
		else
			value = min;
	}
}

void ClipFloat(float& value, float min, float max)
{
	if (value < min)
		value = min;
	else if (value > max)
	{
		if (max > min)
			value = max;
		else
			value = min;
	}
}

const char* BoolToStr(bool value)
{
	return value ? "True" : "False";
}

Vector3 InitVector3(float value)
{
	Vector3 vec;
	vec.x = value;
	vec.y = value;
	vec.z = value;

	return vec;
}

bool IsVersionSupportedForGlobals(eGameVersion ver)
{
	// Note: Assumed globals are the same in steam and nonsteam
	return ver == eGameVersion::VER_1_0_372_2_STEAM || ver == eGameVersion::VER_1_0_372_2_NOSTEAM;
}

bool InitThreadBasket()
{
	// 0x9B5BCD: offset to get to the mov instruction that contains the offset to the threads structure
	// 0x2A07D38: Offset to get the pointer to the ThreadBasket structure

	// (*(srcThreads))[0] -> Address to first thread script
	// (*srcThreads)[1] -> Address to second thread script

	// (*(*srcThreads)[0]) -> First script thread

	// (*(*srcThreads)[0]).pStack -> First script stack start address
	// *(*(*srcThreads)[0]).pStack + index) -> iLocal_index
	if (IsVersionSupportedForGlobals(getGameVersion()))
	{
		PVOID baseAddress = GetModuleHandleA("GTA5.exe");
		if (baseAddress == NULL)
			return false;

		// Offset to the thread basket address, valid for build 372 only
		PVOID offsetAddress = (PVOID)((char*)baseAddress + 0x2A07D38);

		DWORD d, ds;

		// Make the memory readable
		if (!VirtualProtect(offsetAddress, sizeof(ThreadBasket), PAGE_EXECUTE_READ, &d))
			return false;

		// Copy the ThreadBasket
		if (!memcpy(&threadBasket, offsetAddress, sizeof(ThreadBasket)))
			return false;

		// Return the previous rights to the memory
		if (!VirtualProtect(offsetAddress, sizeof(ThreadBasket), d, &ds))
			return false;
		return true;
	}
	else
		return false;
}

std::string GetGameVersionString() {
	return std::to_string(getGameVersion());
}

uint64_t* GetThreadAddress(int localId, int scriptHash)
{
	if (localId < 0 || scriptHash == 0)
		return nullptr;

	bool threadBasketLoaded = false;
	if (threadBasket.srcThreads == nullptr)
		threadBasketLoaded = InitThreadBasket();
	else
		threadBasketLoaded = true;

	uint64_t* localAddress = nullptr;
	if (threadBasketLoaded)
	{
		for (unsigned short i = 0; i < threadBasket.threadCount; i++)
		{
			ScrThread* scrThread = threadBasket.srcThreads[i];
			if (scrThread != nullptr && scrThread->pStack != nullptr)
			{
				if (scrThread->scriptHash == scriptHash)
				{
					localAddress = scrThread->pStack + localId;
					break;
				}
			}
		}
	}

	return localAddress;
}

uint64_t* GetGlobalPtr(int globalId)
{
	if (globalId < 0)
		return nullptr;
	return getGlobalPtr(globalId);
}

char* GetCommsCharacterName(int characterI)//Position - 0xDBDC
{
	switch (characterI)
	{
	case 16:
		return "ABIGAIL";
		break;

	case 95:
		return "KIDNAPPEDFEMALE";
		break;

	case 17:
		return "AMANDA";
		break;

	case 21:
		return "CHENG";
		break;

	case 35:
		return "CHENGSR";
		break;

	case 30:
		return "DAVE";
		break;

	case 29:
		return "DEVIN";
		break;

	case 36:
		return "FRIEDLANDER";
		break;

	case 28:
		return "ESTATEAGENT";
		break;

	case 53:
		return "HAO";
		break;

	case 54:
		return "CLETUS";
		break;

	case 14:
		return "JIMMY";
		break;

	case 55:
		return "JIMMYBOSTON";
		break;

	case 19:
		return "LAMAR";
		break;

	case 12:
		return "LESTER";
		break;

	case 31:
		return "MARTIN";
		break;

	case 39:
		return "ONEIL";
		break;

	case 34:
		return "OSCAR";
		break;

	case 40:
		return "PATRICIA";
		break;

	case 20:
		return "NERVOUSRON";
		break;

	case 18:
		return "SIMEON";
		break;

	case 26:
		return "SOLOMON";
		break;

	case 23:
		return "STEVE";
		break;

	case 37:
		return "STRETCH";
		break;

	case 43:
		return "TANISHA";
		break;

	case 15:
		return "TRACEY";
		break;

	case 24:
		return "WADE";
		break;

	case 104:
		return "JULIET";
		break;

	case 105:
		return "NIKKI";
		break;

	case 106:
		return "CHASTITY";
		break;

	case 107:
		return "CHEETAH";
		break;

	case 108:
		return "SAPPHIRE";
		break;

	case 109:
		return "INFERNUS";
		break;

	case 110:
		return "FUFU";
		break;

	case 111:
		return "PEACH";
		break;

	case 94:
		return "BDOWNHOTCHICK";
		break;

	case 112:
		return "TaxiLiz";
		break;

	case 93:
		return "REHH2Hiker";
		break;

	case 46:
		return "LIEngineer";
		break;

	case 50:
		return "BEVERLY";
		break;

	case 51:
		return "CRIS";
		break;

	case 58:
		return "JOSH";
		break;

	case 62:
		return "MAUDE";
		break;

	case 113:
		return "TaxiDispatch";
		break;

	case 123:
		return "MANAGER";
		break;

	case 116:
		return "MANAGER";
		break;

	case 103:
		return "TONYA";
		break;

	default:
		break;
	}
	return "ERROR";
}

char* GetTextMessageName(int textId) {
	switch (textId)
	{
	case 1834414893:
		return "AH_1_U";
		break;

	case 2114893093:
		return "AH_2_U";
		break;

	case 230961098:
		return "AH_P_U";
		break;

	case 639116137:
		return "AH_GETA_R";
		break;

	case 1943637475:
		return "AH3B_UNLK";
		break;

	case 1635608802:
		return "ASS1_MIS";
		break;

	case -799417230:
		return "ASS_FINA";
		break;

	case -2135245515:
		return "ASS_FINB";
		break;

	case -1982006572:
		return "ARM2_END";
		break;

	case 866793964:
		return "ARM3_END2";
		break;

	case 410510653:
		return "ARM3_END1";
		break;

	case 341684477:
		return "CAR1_ALL";
		break;

	case 1328556918:
		return "CAR2_DONE";
		break;

	case 1023767:
		return "CAR3_F_REM";
		break;

	case 190444893:
		return "CAR3_MT_REM";
		break;

	case 1761525528:
		return "MIC2_MIS";
		break;

	case 1674644829:
		return "CHOP_UNLOCK";
		break;

	case 152157591:
		return "CITY_RON";
		break;

	case 736324744:
		return "CHEF_U";
		break;

	case 839361606:
		return "DOCKS1_F_SHT";
		break;

	case 411396589:
		return "DCKP1_U";
		break;

	case -631719301:
		return "DCKP2B_U";
		break;

	case -549935988:
		return "DCKP2B_R";
		break;

	case 1759805585:
		return "DOCKS1_M_FLY";
		break;

	case 1269300253:
		return "DCK2_U";
		break;

	case 610159002:
		return "EXIL2_U";
		break;

	case -1281173598:
		return "EXIL2_OSC";
		break;

	case -1293573249:
		return "RBH_PAT";
		break;

	case 1328243545:
		return "EXIL_WAD";
		break;

	case 1400447159:
		return "EXL_HUNT_PIE";
		break;

	case 981904836:
		return "EXL_JIM_MCAR";
		break;

	case -1428654185:
		return "FAM1_END";
		break;

	case -1608680660:
		return "FAM3_INIT";
		break;

	case 25048086:
		return "FAM5_END";
		break;

	case 6861963:
		return "FBI4I_U";
		break;

	case -655205392:
		return "FB4_RM_M";
		break;

	case 1266526796:
		return "FB4_RM_FT";
		break;

	case -1652588035:
		return "FBI4_UM";
		break;

	case -125167689:
		return "FBI4_UFT";
		break;

	case 1323596299:
		return "FBI4_ASS";
		break;

	case -1205448796:
		return "FBI4_MTASS";
		break;

	case -1942568571:
		return "FIH_PA_U";
		break;

	case -408163515:
		return "FIH_PB_U";
		break;

	case 2073240496:
		return "FIH_PD_U";
		break;

	case -1216150262:
		return "FIH_GETA_R";
		break;

	case 704657460:
		return "FIH_KILLM";
		break;

	case -1198893762:
		return "FIH_KILLT";
		break;

	case 497850717:
		return "FORSALE_TXT";
		break;

	case -1501908698:
		return "FRAN0_END";
		break;

	case 943848816:
		return "FRA1_END";
		break;

	case 689367973:
		return "JP1A_U";
		break;

	case -666454256:
		return "JH1A_FRNK";
		break;

	case -270587643:
		return "JP2A_U";
		break;

	case -1641645609:
		return "JP1B_U";
		break;

	case 1471029448:
		return "JP1B_HOST";
		break;

	case -1879623490:
		return "JH2_P";
		break;

	case -1816533207:
		return "JOSH1_TXT";
		break;

	case -474700046:
		return "MAR1_U";
		break;

	case -1577577773:
		return "MAR_CASH1";
		break;

	case -1235391473:
		return "MAR_CASH2";
		break;

	case 251204761:
		return "MAR_TREV";
		break;

	case -1207693656:
		return "ME_A_FAIL";
		break;

	case 809731919:
		return "ME_A_FAIL2";
		break;

	case -149728591:
		return "ME_J_FAIL";
		break;

	case 1245740196:
		return "ME_T_FAIL";
		break;

	case -1589024007:
		return "MIC4_M";
		break;

	case -658009473:
		return "MIC4_F";
		break;

	case 772720529:
		return "PLTSC_U";
		break;

	case 902928934:
		return "RBH_U";
		break;

	case -831144864:
		return "RBH_P_U";
		break;

	case 486824538:
		return "SHOOT_U";
		break;

	case -384575792:
		return "SXT_";
		break;

	case -1924990311:
		return "FTX_";
		break;

	case 2091854273:
		return "FTX_";
		break;

	case 328868333:
		return "FTX_";
		break;

	case -1813399915:
		return "FTX_";
		break;

	case 465306446:
		return "FTX_";
		break;

	case -816460512:
		return "FTX_";
		break;

	case -702667427:
		return "FTX_";
		break;

	case 271682265:
		return "PRM";
		break;

	case 1694883968:
		return "BSTXT_";
		break;

	case -181320640:
		return "FGTXT_M";
		break;

	case 1418815087:
		return "FGTXT_F";
		break;

	case 2087297077:
		return "FGTXT_T";
		break;

	case -1789721694:
		return "FGTXT_H";
		break;

	case 483349085:
		return "BARR3_U";
		break;

	case 1800466587:
		return "BARR4_U";
		break;

	case 2106164812:
		return "EPSI6_U";
		break;

	case -1783809705:
		return "EXTR2_U";
		break;

	case -1159983966:
		return "EXTR3_U";
		break;

	case -1134717682:
		return "HUNT1_U";
		break;

	case -1444331296:
		return "PAP3_U";
		break;

	case 969002696:
		return "TON3_U";
		break;

	case 1794975438:
		return "TON4_U";
		break;

	case 1988415324:
		return "RE_BUR_MIS";
		break;

	case 1234461962:
		return "SOL2_PASS";
		break;

	case 1516526930:
		return "DRF3_U";
		break;

	case -685399607:
		return "DRF4_U";
		break;

	case -527573502:
		return "SR_AIRPORT";
		break;

	case 277048894:
		return "SR_FREEWAY";
		break;

	case 477030223:
		return "SR_CANALS";
		break;

	case -1900821691:
		return "MGTNS_U";
		break;

	case -1845612607:
		return "MGTNS_U2";
		break;

	case -815326385:
		return "T_LOST_HANGER";
		break;

	case 367701416:
		return "OMEGA_TXT1";
		break;

	case 65881433:
		return "BLAZER3_U";
		break;

	case 1256519626:
		return "ABI1_MISS";
		break;

	case 1324777792:
		return "HAO1_ENDT";
		break;

	case 552744224:
		return "PHT_WLD_BEV1";
		break;

	case 1751306471:
		return "MNKY_CAR_UNLK";
		break;

	case -899711929:
		return "MNKY_LAMAR";
		break;

	case 1139718847:
		return "CRACE_UNLK";
		break;
	}
	return "UNKNOWN";
}

char* GetCallName(int callID) {
	switch (callID)
	{
	case -1323332922:
		return "F3A_INTALT";
		break;

	case -651018670:
		return "AHF_C4";
		break;

	case -236506853:
		return "AHF_C7";
		break;

	case -518176220:
		return "AHF_C5";
		break;

	case -1889998615:
		return "AHF_C6";
		break;

	case 777679877:
		return "AHF_C8";
		break;

	case -1547927413:
		return "AHF_C9";
		break;

	case 1770589463:
		return "AH3A_PASS";
		break;

	case -1358914512:
		return "AH3B_PASS";
		break;

	case 1611093726:
		return "ARFL_CALL1";
		break;

	case 742578279:
		return "ARFL_CALL_CT";
		break;

	case -1753640864:
		return "ARFL_CALL2";
		break;

	case 24012281:
		return "ASFL_CALL1";
		break;

	case 343235192:
		return "OJAS_HOTEL_C";
		break;

	case 687625470:
		return "CSFL_CALL0";
		break;

	case -1134988206:
		return "MIFL_CALL2";
		break;

	case -1155501333:
		return "CHFL_CALL1";
		break;

	case -951496:
		return "DHF_C3";
		break;

	case -1108940541:
		return "DHF_C4";
		break;

	case 1372476011:
		return "DHF_C3B";
		break;

	case -452599906:
		return "DHF_WP1";
		break;

	case 944724110:
		return "DHF_WP2";
		break;

	case -749738207:
		return "DHF_C2A";
		break;

	case -1478217267:
		return "EXL1_CALLM";
		break;

	case -1451910289:
		return "EXIL_CALL1";
		break;

	case -1151582404:
		return "EXIL_CALL2";
		break;

	case 1628462442:
		return "EXIL_MAR09";
		break;

	case 1791324372:
		return "EXIL_MAR10";
		break;

	case -240877892:
		return "EXIL_MAR11";
		break;

	case -1141042322:
		return "EXIL_MAR05";
		break;

	case 1806999335:
		return "EXIL_MAR12";
		break;

	case 2054503592:

		return "EXIL_MAR13";
		break;

	case -2009081795:

		return "EXIL_MAR14";
		break;

	case 520422857:

		return "EXIL_MAR07";
		break;

	case 1645231:

		return "FMFL_AM4";
		break;

	case -769656371:

		return "EXL3_STEVE";
		break;

	case -694220424:

		return "EPSD_CRIS";
		break;

	case -885867010:

		return "EPSD_FIN";
		break;

	case -632277372:

		return "EPS8_CF1";
		break;

	case -1763624884:

		return "EPS8_CF3";
		break;

	case 1995383583:

		return "FMFL_TRFAM4";
		break;

	case -881570909:

		return "FMFL_TRCHILL";
		break;

	case -444489072:

		return "FOFL_CALL1A";

		break;

	case 154402960:

		return "FOFL_CALL1B";
		break;

	case 795730787:

		return "FOFL_CALL4";
		break;

	case -1847734803:

		return "FBIP1_MENDC";
		break;

	case -714760066:

		return "FBIP1_FENDC";
		break;

	case -1198055521:

		return "FBIP1_TENDC";
		break;

	case 1374342572:

		return "FBIP2_MENDC";
		break;

	case 530956160:

		return "FBIP2_FENDC";
		break;

	case 240475766:

		return "FBIP2_TENDC";
		break;

	case 580731697:

		return "FBIP4_MENDC";
		break;

	case 728176806:

		return "FBIP4_FENDC";
		break;

	case 910240872:

		return "FBIP4_TENDC";
		break;

	case 74540475:

		return "FBIP5_MENDC";
		break;

	case -1200353264:

		return "FBIP5_FENDC";
		break;

	case 801340541:

		return "FBIP5_TENDC";
		break;

	case 1527885205:

		return "FBI4_READY";
		break;

	case -224691627:

		return "FBI4_WAIT";
		break;

	case -2014002000:

		return "FBI4_READY";
		break;

	case -314546970:

		return "FBI4_ITSONF";
		break;

	case -1306479777:

		return "FBI4_ITSONT";
		break;

	case -621899663:

		return "FBI4_ISAGO";
		break;

	case -1615086084:

		return "FNFL_CHC1";
		break;

	case -1989308064:

		return "FNFL_CHC1B";
		break;

	case -1970925435:

		return "FNFL_CHC2";
		break;

	case -1060930305:

		return "FNFL_CHC2B";
		break;

	case -1817481777:

		return "FNFL_CHC3";
		break;

	case -2099041387:

		return "FNFL_AMAN";
		break;

	case -1468151569:

		return "FNFL_DAVE";
		break;

	case -121056641:

		return "FNFL_DEV09";
		break;

	case -579786137:

		return "FNFL_LAM22";
		break;

	case -1398697867:

		return "FNFL_MDEAD1";
		break;

	case 466185907:

		return "FNFL_MDEAD2";
		break;

	case 185453884:

		return "FNFL_MDEAD3";
		break;

	case 1972824823:

		return "FNFL_STE01";
		break;

	case -288258721:

		return "FNFL_TDEAD1";
		break;

	case -47439340:

		return "FNFL_TDEAD2";
		break;

	case -553769964:

		return "FHF_STN1";
		break;

	case -1434443191:

		return "FHF_STN2";
		break;

	case -2032593637:

		return "FHF_STN3";
		break;

	case -458789713:

		return "FHF_CUT1E";
		break;

	case 321648831:

		return "FHF_CUT2E";
		break;

	case 312697495:

		return "FHF_CUT3E";
		break;

	case -1980743701:

		return "FHF_MPC1";
		break;

	case 1567521709:

		return "FHF_FPC1";
		break;

	case -1687842043:

		return "FHF_TPC1";
		break;

	case -1631047976:

		return "FHF_MPC2";
		break;

	case 962970051:

		return "FHF_FPC2";
		break;

	case -1323797481:

		return "FHF_TPC2";
		break;

	case 2095586439:

		return "FHF_CAR1";
		break;

	case 1608868018:

		return "FHF_CAR2";
		break;

	case 1134611425:

		return "FHF_CAR3";
		break;

	case 187813079:

		return "FHF_TRN1";
		break;

	case -39544602:

		return "FHF_TRN2";
		break;

	case -997367701:

		return "FHF_TRN3";
		break;

	case -128912482:

		return "FHF_GETA1";
		break;

	case -131238069:

		return "FHF_GETA2";
		break;

	case -330732224:

		return "FHF_GETA3";
		break;

	case -1334144471:

		return "FHF_RDY";
		break;

	case -211946295:

		return "FHF_PREP";
		break;

	case 1679209251:

		return "FHF_STNG";
		break;

	case -1394105734:

		return "FHF_GETA";
		break;

	case 94848458:

		return "FHF_DRILL";
		break;

	case -1106471007:

		return "FHF_TRN";
		break;

	case -268883259:

		return "FHF_GAU";
		break;

	case 546458037:

		return "FRFL_1LC";
		break;

	case -2068958976:

		return "LSFL_ENG01";
		break;

	case -1366489923:

		return "LSFL_ENG02";
		break;

	case 406771743:

		return "LSFL_ENG03";
		break;

	case -837794877:

		return "LSFL_ENG04";
		break;

	case -78001721:

		return "JHF_C1";
		break;

	case 327673956:

		return "JHF_C3A";
		break;

	case -1746082591:

		return "JHF_P10A";
		break;

	case -1945164276:

		return "JHF_P10B";
		break;

	case -1052097629:

		return "JHF_BZD1";
		break;

	case 1595158098:

		return "JHF_TRANSM";
		break;

	case -420969532:

		return "JHF_TRANSF";
		break;

	case -441095892:

		return "MIFL_DAVE";
		break;

	case 153235461:

		return "MIFL_TR1";
		break;

	case -375893868:

		return "MIFL_PREM";
		break;

	case -67922520:

		return "MIFL_AM1";
		break;

	case 707131591:

		return "PAP3A_ENDCAL";
		break;

	case 2033185762:

		return "PAFL_01";
		break;

	case 1801771084:

		return "PAFL_02";
		break;

	case -1716308760:

		return "PAFL_03";
		break;

	case -2116452823:

		return "RHF_C1";
		break;

	case -1141220854:

		return "RHF_C1A";
		break;

	case 1880611494:

		return "RHF_C2";
		break;

	case -1276059507:

		return "RHF_C2A";
		break;

	case -1434698871:

		return "RHF_C3";
		break;

	case 1080702705:

		return "FNFL_TANISHA";
		break;

	case 1632742627:

		return "FMFL_CALL2B";
		break;

	case -1074970768:

		return "SLFL_MAR04";
		break;

	case 643619115:

		return "SOL3_PASS";
		break;

	case 720676600:

		return "DIVE_PHONE";
		break;

	case 1303349750:

		return "SONAR_PHONE";
		break;

	case -1343914045:

		return "TRV5_LESCALL";
		break;

	case -1199050901:

		return "REBU2_CALLM";
		break;

	case 1082655975:

		return "REBU2_CALLF";
		break;

	case -2100435596:

		return "REBU2_CALLT";
		break;

	case -974942855:

		return "HAO1_PCW";
		break;

	case -577064562:

		return "TOW_CALLSALE";
		break;

	case 2025906743:

		return "PRP_INTRO";
		break;

	case -1404384954:

		return "PRP_INTROM";
		break;

	case -112860461:

		return "PRP_INTROF2";
		break;

	case 339632201:

		return "PMPLA_MAN";
		break;

	case 453095043:

		return "PRP_INTROF";
		break;

	case 313472619:

		return "FMM_GRGa";
		break;

	case 2038672434:

		return "FMM_GRRa";
		break;

	case 1666308520:

		return "FMM_GRCa";
		break;

	case 1894462438:

		return "FMM_GRHa";
		break;

	case 1728635625:

		return "FMF_GRGa";
		break;

	case 975196153:

		return "FMF_GRRa";
		break;

	case -710274964:

		return "FMF_GRCa";
		break;

	case 1289879258:

		return "FMF_GRB";
		break;

	case 1621076324:

		return "FMF_GRHa";
		break;

	case 1005256598:

		return "FMT_GRGa";
		break;

	case 1127548000:

		return "FMT_GRRa";
		break;

	case -1291788156:

		return "FMT_GRCa";
		break;

	case 1993031175:

		return "FMT_GRHa";
		break;

	default:
		switch (callID)
		{
		case 2111803439:

			return "FMFL_AM1";
			break;

		case 1290939985:

			return "FMFL_AM2";
			break;

		case -1690219790:

			return "FMFL_AM3";
			break;

		case 1766909710:

			return "FMFL_AM4";
			break;

		case -358013836:

			return "HTAMB_PH1";
			break;

		case -589035286:

			return "HTAMB_PH2";
			break;

		case -1842374536:

			return "HTAMB_PH3";
			break;

		case -418954710:

			return "HTAMB_PH4";
			break;

		case -1783816333:

			return "HTAMB_PH5";
			break;

		case 1438498239:

			return "JHF_DAVE";
			break;

		case -2139605007:
			return "CHATC_DEV03";
			break;

		case 1906711117:
			return "CHATC_DEV04";
			break;

		case -1949184344:
			return "CHATC_DEV05";
			break;

		case 2065444157:
			return "CHATC_DEV06";
			break;

		case 624853379:
			return "CHATC_DEV07";
			break;

		case -1766792090:
			return "CHATC_DEV08";
			break;

		case -1878340166:
			return "CHATC_JIM05";
			break;

		case 279793409:
			return "CHATC_JIM06";
			break;

		case 460547213:
			return "CHATC_JIM07";
			break;

		case -330856906:
			return "CHATC_JIM08";
			break;

		case -690594988:
			return "CHATC_JIM09";
			break;

		case -349863294:
			return "CHATC_JIM10";
			break;

		case 424926942:
			return "CHATC_JIM11";
			break;

		case 1890717037:
			return "CHATC_JIM12";
			break;

		case -1636767510:
			return "CHATC_JIM13";
			break;

		case -1893250473:
			return "CHATC_JIM14";
			break;

		case -1160994395:
			return "CHATC_JIM15";
			break;

		case 963157723:
			return "CHATC_JIM16";
			break;

		case 1730771548:
			return "CHATC_JIM17";
			break;

		case 1410028576:
			return "CHATC_JIM18";
			break;

		case -2082393141:
			return "CHATC_JIM19";
			break;

		case -1497396589:
			return "CHATC_JIM20";
			break;

		case -1196085634:
			return "CHATC_JIM21";
			break;

		case 170512742:
			return "CHATC_JIM22";
			break;

		case 469005563:
			return "CHATC_JIM23";
			break;

		case -306898819:
			return "CHATC_JIM24";
			break;

		case 82970585:

			return "LAM_06";
			break;

		case 1867406480:
			return "CHATC_LAM08";
			break;

		case 1502097668:
			return "CHATC_LAM09";
			break;

		case -1621970384:
			return "CHATC_LAM10";
			break;

		case 1031237239:
			return "CHATC_LAM11";
			break;

		case 194808514:
			return "CHATC_LAM12";
			break;

		case 1509435260:
			return "CHATC_LAM13";
			break;

		case 673727449:
			return "CHATC_LAM14";
			break;

		case 71040001:
			return "CHATC_LAM15";
			break;

		case -755689100:
			return "CHATC_LAM16";
			break;

		case 556840426:
			return "CHATC_LAM17";
			break;

		case -177054098:
			return "CHATC_LAM18";
			break;

		case -835252192:
			return "CHATC_LAM19";
			break;

		case 616513703:
			return "CHATC_LAM20";
			break;

		case 2064117051:

			return "FNFL_LAM21";
			break;

		case -1758542647:
			return "CHATC_RON03";
			break;

		case 1089542219:
			return "CHATC_RON04";
			break;

		case 675079903:
			return "CHATC_RON05";
			break;

		case -1613846760:
			return "CHATC_SOL01";
			break;

		case 518202687:
			return "CHATC_SOL02";
			break;

		case 946886745:
			return "CHATC_SOL03";
			break;

		case 1229388294:
			return "CHATC_SOL04";
			break;

		case 522335044:
			return "CHATC_STR01";
			break;

		case 2097475336:
			return "CHATC_STR02";
			break;

		case -108894207:
			return "CHATC_STR03";
			break;

		case -950205513:
			return "CHATC_STR04";
			break;

		case -703684326:
			return "CHATC_STR05";
			break;

		case -1982920067:
			return "CHATC_TEXIL1";
			break;

		case 1319343143:
			return "CHATC_TEXIL2";
			break;

		case -1118768768:
			return "CHATC_TEXIL3";
			break;

		case -705125709:
			return "CHATC_TEXIL4";
			break;

		case -782984853:
			return "CHATC_TEXIL5";
			break;

		case -92968020:
			return "CHATC_TEXIL6";
			break;

		case -406305198:
			return "CHATC_TEXIL7";
			break;

		case -1130010274:

			return "FMFL_TNOT";
			break;

		case 684213521:
			return "CHATC_TFAM";
			break;

		case 1388615949:
			return "CHATC_TFAM4";
			break;

		case -2062910056:
			return "CHATC_TFAM5";
			break;

		case -643694497:
			return "CHATC_WADE01";
			break;

		case -278713375:
			return "CHATC_WADE02";
			break;

		case -1105442476:
			return "CHATC_WADE04";
			break;

		case -805704433:
			return "CHATC_WADE05";
			break;

		case -1565848713:
			return "CHATC_MAR01";
			break;

		case -1351801605:
			return "CHATC_MAR02";
			break;

		case 1311040108:
			return "CHATC_MAR03";
			break;

		case 1562602199:
			return "CHATC_CHG1";
			break;

		case 1113666899:
			return "CHATC_CHG2";
			break;

		case 814649774:
			return "CHATC_CHG3";
			break;

		case 225734768:
			return "CHATC_HOSP";
			break;

		default:
			break;
		}
		break;
	}

	return "UNKNOWN";
}