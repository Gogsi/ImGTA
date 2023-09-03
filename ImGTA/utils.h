/*
 * Copyright (c) 2021, James Puleo <james@jame.xyz>
 * Copyright (c) 2021, Rayope
 *
 * SPDX-License-Identifier: GPL-3.0-only
 */

#pragma once
#include "types.h"
#include "enums.h"
#include "main.h"

#include <Windows.h>
#include <cstdint>
#include <string>

typedef unsigned long long QWORD;

struct handle_data
{
	unsigned long processID;
	HWND windowHandle;
};


struct ScrThread {
	uint64_t field_0; // 0
	int thread_id; // 8
	int scriptHash; // 12
	int field_3; // 16
	int field_4; // 20
	uint64_t frameSP; // 24
	uint64_t _padding[18]; // 32
	uint64_t* pStack = nullptr; // 176
	int _padding1; // 184
	int nParametersSize; // 188
	int nArgsOffset; // 192
	int _padding2; // 196
	uint64_t _padding3; // 200
	char scriptName[64]; // 208
	uint64_t _padding4[9]; // 272
}; // Size 344 Bytes


struct ThreadBasket {
	ScrThread** srcThreads = nullptr; // 0
	unsigned short threadCount = 0; // 8
	unsigned short threadCapacity = 0; // 12
}; // 16 Bytes

template<typename T>
struct GlobalArray {
	GlobalArray(int sizeAddr) : sizeAddr{ sizeAddr }, startAddr{ sizeAddr + 1 }
	{

	}

	void LoadElement()
	{
		if (sizeAddr != 0)
		{
			size = *(int*)GetGlobalPtr(sizeAddr);
			if (size > 0 && id < size)
			{
				int offset = startAddr + id * sizeof(T) / 8;

				uint64_t* p = GetGlobalPtr(offset);
				arr = *(T*)p;
				ptr = (T*)p;
			}
		}
	}

	T arr;
	T* ptr; // pointer, editable
	int id = 0;
	int size = 0;

private:
	int sizeAddr = 0;
	int startAddr = 0;
};

struct PaddedInt {
	int val = 0;
	DWORD _padding;
};


BOOL IsMainWindow(HWND handle);
BOOL CALLBACK EnumWindowsCallback(HWND handle, LPARAM lParam);
HWND FindMainWindow(unsigned long processID);
void ResetTextDrawCount();
int GetTextDrawCount();
float TextFontHeight(float size, eFont font);
// WARNING DrawTextToScreen: Only the first 100 calls to this function are displayed!
void DrawTextToScreen(const char* text, float x, float y, float scale, eFont font, bool alignRight = false, int red = 255, int green = 255, int blue = 255);
void ClipInt(int& value, int min, int max);
void ClipFloat(float& value, float min, float max);
const char* BoolToStr(bool value);
Vector3 InitVector3(float value);
bool IsVersionSupportedForGlobals(eGameVersion ver);

char* GetCommsCharacterName(int characterId);
char* GetTextMessageName(int textId);
char* GetCallName(int callID);
// Thanks Parik (explanation on where and how to find addresses as well as thread structures...
//				 everything you see in these two functions)
// Thanks Gogsi123 (how to get the value from addresses in C++)
bool InitThreadBasket();
uint64_t* GetThreadAddress(int localId, int scriptHash);
uint64_t* GetGlobalPtr(int globalId);
std::string GetGameVersionString();