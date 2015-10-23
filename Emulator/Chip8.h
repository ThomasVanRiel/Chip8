#include <iostream>
#include <fstream>
#include <string>

typedef unsigned char U8;
typedef unsigned short U16;
typedef unsigned int U32;

struct Chip8 {
	U8 m_Memory[4096];
	U8 m_Reg[16];
	U16 m_RegI;
	U16 m_RegPC;
	U32 m_Texture[64 * 32];

	U8 m_TimerDelay;
	U8 m_TimerSound;

	U16 m_Key;

	U16 m_Stack[16];
	U8 m_StackPointer; 

	bool m_DoRedraw;

	Chip8();
	void LoadRom(std::string filePath);
	void Loop();
	void DecreaseTimers();
};