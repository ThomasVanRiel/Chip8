#pragma once
#include <iostream>
#include <fstream>
#include <string>
#include <functional>

typedef unsigned char U8;
typedef unsigned short U16;
typedef unsigned int U32;

struct SuperChip {
	U8 m_Memory[4096];
	U8 m_Reg[16];
	U16 m_RegI;
	U16 m_RegPC;
	U8 m_RPLUserFlags[8];
	U32 m_Gfx[128 * 64];

	U8 m_TimerDelay;
	U8 m_TimerSound;

	U16 m_Key;

	U16 m_Stack[16];
	U8 m_StackPointer;

	bool m_DoRedraw;
	bool m_Extended = false;

	std::function<void(void)> m_ExitCallback;

	SuperChip();
	void LoadRom(std::string filePath);
	void Loop();
	void DecreaseTimers();
	void TestExit() { m_ExitCallback(); };

	void SetExitCallback(std::function<void(void)> callback) { m_ExitCallback = callback; }

public:
	static const U16 SUPERFONT_START = 80;
};