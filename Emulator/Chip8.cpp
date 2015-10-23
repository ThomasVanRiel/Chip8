#include "Chip8.h"

using namespace std;

Chip8::Chip8() {
	U8 font[80] = {
		0xF0, 0x90, 0x90, 0x90, 0xF0,
		0x20, 0x60, 0x20, 0x20, 0x70,
		0xF0, 0x10, 0xF0, 0x80, 0xF0,
		0xF0, 0x10, 0xF0, 0x10, 0xF0,

		0x90, 0x90, 0xF0, 0x10, 0x10,
		0xF0, 0x80, 0xF0, 0x10, 0xF0,
		0xF0, 0x80, 0xF0, 0x90, 0xF0,
		0xF0, 0x10, 0x20, 0x40, 0x40,

		0xF0, 0x90, 0xF0, 0x90, 0xF0,
		0xF0, 0x90, 0xF0, 0x10, 0xF0,
		0xF0, 0x90, 0xF0, 0x90, 0x90,
		0xE0, 0x90, 0xE0, 0x90, 0xE0,

		0xF0, 0x80, 0x80, 0x80, 0xF0,
		0xE0, 0x90, 0x90, 0x90, 0xE0,
		0xF0, 0x80, 0xF0, 0x80, 0xF0,
		0xF0, 0x80, 0xF0, 0x80, 0x80
	};

	memcpy(m_Memory, font, 80);
}

void Chip8::LoadRom(std::string filePath) {

	//Store sprite font

	streampos size;
	ifstream file;
	file.open(filePath, ios::in | ios::binary | ios::ate);

	if (file.is_open()) {
		size = file.tellg();
		file.seekg(0, ios::beg);
		file.read((char*)m_Memory + 512, size);
		file.close();
	} else {
		cout << "File " << filePath << " not found." << endl;
	}

	//Set values to 0
	m_RegPC = 0x200;
	for (int i = 0; i < 0xF; i++) {
		m_Reg[i] = 0;
	}

	//Clear screen
	for (int i = 0; i < 64 * 32; i++) {
		m_Texture[i] = 0x000000ff;
	}

	m_StackPointer = 0;


	m_TimerDelay = 0;
	m_TimerSound = 0;
}



void Chip8::Loop() {

	m_DoRedraw = false;

	//Get opcode
	U16 OpCode = m_Memory[m_RegPC++];
	OpCode <<= 8;
	OpCode |= m_Memory[m_RegPC++];

	//std::cout << std::hex << OpCode << std::endl;

	switch (OpCode & 0xF000) {
		case 0x0000:
			if (OpCode == 0x00E0) {
				//00E0 - Clear screen
				for (int i = 0; i < 64 * 32; i++) {
					m_Texture[i] = 0x000000ff;
				}
				m_DoRedraw = true;
			} else if (OpCode == 0x00EE) {
				//00EE - return from subroutine
				m_RegPC = m_Stack[--m_StackPointer];
			}
			break;
		case 0x1000:
			//1NNN - Jumps to address NNN.
			m_RegPC = OpCode & 0x0FFF;
			break;
		case 0x2000:
			//2NNN - Calls subroutine at NNN.
			m_Stack[m_StackPointer++] = m_RegPC;
			m_RegPC = OpCode & 0x0FFF;
			break;
		case 0x3000:
		{
			//3XNN - Skips the next instruction if VX equals NN.
			U8 x = (OpCode & 0x0F00) >> 8;
			U8 value = OpCode & 0x00FF;
			if (m_Reg[x] == value) {
				m_RegPC += 2;
			}
		}
			break;
		case 0x4000:
		{
			//4XNN - Skips the next instruction if VX does not equal NN.
			U8 x = (OpCode & 0x0F00) >> 8;
			U8 value = OpCode & 0x00FF;
			if (m_Reg[x] != value) {
				m_RegPC += 2;
			}
		}
			break;
		case 0x5000:
		{
			//5XY0 - Skips the next instruction if VX equals VY.
			U8 x = (OpCode & 0x0F00) >> 8;
			U8 y = (OpCode & 0x00F0) >> 4;
			if (m_Reg[x] == m_Reg[y]) {
				m_RegPC += 2;
			}
		}
			break;
		case 0x6000:
		{
			//6XNN - Sets VX to NN.
			U8 x = (OpCode & 0x0F00) >> 8;
			U8 value = OpCode & 0x00FF;
			m_Reg[x] = value;
		}
			break;
		case 0x7000:
		{
			//7XNN - Adds NN to VX.	
			U8 x = (OpCode & 0x0F00) >> 8;
			U8 value = OpCode & 0x00FF;
			m_Reg[x] += value;
		}
			break;
		case 0x8000:
			switch (OpCode & 0x000F) {
				case 0x0:
				{
					//8XY0 - Sets VX to the value of VY.
					U8 x = (OpCode & 0x0F00) >> 8;
					U8 y = (OpCode & 0x00F0) >> 4;

					m_Reg[x] = m_Reg[y];
				}
					break;
				case 0x1:
				{
					//8XY1 - Sets VX to VX or VY.
					U8 x = (OpCode & 0x0F00) >> 8;
					U8 y = (OpCode & 0x00F0) >> 4;

					m_Reg[x] = m_Reg[x] | m_Reg[y];
				}
					break;
				case 0x2:
				{
					//8XY2 - Sets VX to VX and VY.
					U8 x = (OpCode & 0x0F00) >> 8;
					U8 y = (OpCode & 0x00F0) >> 4;

					m_Reg[x] = m_Reg[x] & m_Reg[y];
				}
					break;
				case 0x3:
				{
					//8XY3 - Sets VX to VX xor VY.
					U8 x = (OpCode & 0x0F00) >> 8;
					U8 y = (OpCode & 0x00F0) >> 4;

					m_Reg[x] = m_Reg[x] ^ m_Reg[y];
				}
					break;
				case 0x4:
				{
					//8XY4 - Adds VY to VX. VF is set to 1 when there's a carry, and to 0 when there isn't.
					U8 x = (OpCode & 0x0F00) >> 8;
					U8 y = (OpCode & 0x00F0) >> 4;

					U8 origRegX = m_Reg[x];
					m_Reg[x] += m_Reg[y];
					m_Reg[0xF] = 0;
					//if (m_Reg[x] < origRegX)
					//	m_Reg[0xF] = 1;
					if (m_Reg[y] > (0xFF - m_Reg[x])) {
						m_Reg[0xF] = 1;
					}

				}
					break;
				case 0x5:
				{
					//8XY5 - VY is subtracted from VX. VF is set to 0 when there's a borrow, and 1 when there isn't.
					U8 x = (OpCode & 0x0F00) >> 8;
					U8 y = (OpCode & 0x00F0) >> 4;

					U8 origRegX = m_Reg[x];
					m_Reg[x] -= m_Reg[y];
					m_Reg[0xF] = 1;
					//if (m_Reg[x] > origRegX)
					//	m_Reg[0xF] = 0;
					if (m_Reg[y] > (0xFF - m_Reg[x])) {
						m_Reg[0xF] = 0;
					}

				}
					break;
				case 0x6:
				{
					//8XY6 - Shifts VX right by one. VF is set to the value of the least significant bit of VX before the shift.
					//Store the value of register VY shifted right one bit in register VX Set register VF to the least significant bit prior to the shift
					U8 x = (OpCode & 0x0F00) >> 8;
					U8 y = (OpCode & 0x00F0) >> 4;
					//m_Reg[0xF] = m_Reg[y] & 1;
					//m_Reg[x] = m_Reg[y] >> 1;
					m_Reg[0xF] = m_Reg[x] & 1;
					m_Reg[x] = m_Reg[x] >> 1;
				}
					break;
				case 0x7:
				{
					//8XY7 - Sets VX to VY minus VX. VF is set to 0 when there's a borrow, and 1 when there isn't.
					U8 x = (OpCode & 0x0F00) >> 8;
					U8 y = (OpCode & 0x00F0) >> 4;

					U8 origRegX = m_Reg[x];
					m_Reg[x] = m_Reg[y] - m_Reg[x];
					m_Reg[0xF] = 1;
					//if (m_Reg[x] > m_Reg[y])
					//	m_Reg[0xF] = 0;
					if (m_Reg[y] < m_Reg[x]) {
						m_Reg[0xF] = 0;
					}
				}
					break;
				case 0xE:
				{
					//8XYE - Shifts VX left by one. VF is set to the value of the most significant bit of VX before the shift.
					U8 x = (OpCode & 0x0F00) >> 8;
					U8 y = (OpCode & 0x00F0) >> 4;
					//m_Reg[0xF] = m_Reg[y] >> 7;
					//m_Reg[x] = m_Reg[y] << 1;
					m_Reg[0xF] = m_Reg[x] >> 7;
					m_Reg[x] = m_Reg[x] << 1;
				}
					break;
				default:
					break;
			}
			break;
		case 0x9000:
		{
			//9XY0 - Skips the next instruction if VX doesn't equal VY.
			U8 x = (OpCode & 0x0F00) >> 8;
			U8 y = (OpCode & 0x00F0) >> 4;
			if (m_Reg[x] != m_Reg[y]) {
				m_RegPC += 2;
			}
		}
			break;
		case 0xA000:
			//ANNN - Sets I to the address NNN.
			m_RegI = OpCode & 0x0FFF;
			break;
		case 0xB000:
			//BNNN - Jumps to the address NNN plus V0.
			m_RegPC = (OpCode & 0xFFF) + m_Reg[0x0];
			break;
		case 0xC000:
		{
			//CXNN - Sets VX to the result of a bitwise and operation on a random number and NN.
			U8 x = (OpCode & 0x0F00) >> 8;
			U8 random = (rand() % 0xFF) & (OpCode & 0x00FF);
			m_Reg[x] = random;
		}
			break;
		case 0xD000:
		{
			//DXYN - Sprites stored in memory at location in index register (I), 8bits wide. Wraps around the screen.
			//If when drawn, clears a pixel, register VF is set to 1 otherwise it is zero. 
			//All drawing is XOR drawing (i.e. it toggles the screen pixels). Sprites are drawn starting at position VX, VY. 
			//N is the number of 8bit rows that need to be drawn. If N is greater than 1, second line continues at position VX, VY+1, and so on.

			U8 xInit = m_Reg[((OpCode & 0x0F00) >> 8)];
			U8 yInit = m_Reg[((OpCode & 0x00F0) >> 4)];
			U8 height = (OpCode & 0x000F);
			U8 sprite;

			m_Reg[0xF] = 0;
			for (int y = 0; y < height; y++) {
				sprite = m_Memory[m_RegI + y];
				for (int x = 0; x < 8; x++) {
					U8 test = (sprite & (0x80 >> x));
					if ((sprite & (0x80 >> x)) != 0) {
						U32 pixel = m_Texture[xInit + x + ((yInit + y) * 64)];
						if (m_Texture[xInit + x + ((yInit + y) * 64)] == 0xFFFFFFFF)
							m_Reg[0xF] = 1;
						m_Texture[xInit + x + ((yInit + y) * 64)] ^= 0xFFFFFF00;
					}
				}
			}

			m_DoRedraw = true;
		}
			break;
		case 0xE000:
		{
			U8 x = (OpCode & 0xF00) >> 8;
			if ((OpCode & 0x00FF) == 0x009E) {
				//EX9E - Skips the next instruction if the key stored in VX is pressed.
				if (((m_Key >> m_Reg[x]) & 1) != 0) {
					m_RegPC += 2;
				}

			} else if ((OpCode & 0x00FF) == 0x00A1) {
				//EXA1 - Skips the next instruction if the key stored in VX isn't pressed.
				if (((m_Key >> m_Reg[x]) & 1) == 0) {
					m_RegPC += 2;
				}
			}
		}
			break;
		case 0xF000:
			switch (OpCode & 0x00FF) {
				case 0x07:
				{
					//FX07 - Sets VX to the value of the delay timer.
					U8 x = (OpCode & 0x0F00) >> 8;

					m_Reg[x] = m_TimerDelay;
				}
					break;
				case 0x0A:
				{
					//FX0A - A key press is awaited, and then stored in VX.
					U8 x = (OpCode & 0xF00) >> 8;
					if (m_Key == 0) {
						m_RegPC -= 2;
					} else {
						for (int i = 0; i < 0xF; i++) {
							//U8 test = ((m_Key >> i) & 1);
							if (((m_Key >> i) & 1) == 1) {
								m_Reg[x] = i;
								break;
							}
						}
					}
				}
					break;
				case 0x15:
				{
					//FX15 - Sets the delay timer to VX.
					U8 x = (OpCode & 0x0F00) >> 8;

					m_TimerDelay = m_Reg[x];
				}

					break;
				case 0x18:
				{	//FX18 - Sets the sound timer to VX.
					U8 x = (OpCode & 0x0F00) >> 8;

					m_TimerSound = m_Reg[x];
				}
					break;
				case 0x1E:
					//FX1E - Adds VX to I.
					m_RegI += m_Reg[(OpCode & 0x0F00) >> 8];
					if (m_RegI > 0xFFF) {
						m_Reg[0xF] = 1;
					} else {
						m_Reg[0xF] = 0;
					}
					break;
				case 0x29:
					//FX29 - Sets I to the location of the sprite for the character in VX. Characters 0-F (in hexadecimal) are represented by a 4x5 font.
				{
					U8 x = (OpCode & 0x0F00) >> 8;

					m_RegI = m_Reg[x] * 5;
				}
					break;
				case 0x33:
					//FX33 - Stores the Binary-coded decimal representation of VX, with the most significant of three digits at the address in I, 
					//the middle digit at I plus 1, and the least significant digit at I plus 2. 
					//(In other words, take the decimal representation of VX, place the hundreds digit in memory at location in I, the tens digit at location I+1, and the ones digit at location I+2.)
				{
					U8 number = m_Reg[(OpCode & 0xF00) >> 8];
					U8 hundreds = 0, tens = 0, ones = 0;
					ones = number % 10;
					number /= 10;
					tens = number % 10;
					hundreds = number / 10;

					m_Memory[m_RegI] = hundreds;
					m_Memory[m_RegI + 1] = tens;
					m_Memory[m_RegI + 2] = ones;
				}
					break;
				case 0x55:
					//FX55 - Stores V0 to VX in memory starting at address I.
				{
					U8 x = (OpCode & 0x0F00) >> 8;

					for (int i = 0; i <= x; i++) {
						m_Memory[m_RegI + i] = m_Reg[i];
					}

					m_RegI += x + 1;
				}
					break;
				case 0x65:
					//FX65 - Fills V0 to VX with values from memory starting at address I.
				{
					U8 x = (OpCode & 0x0F00) >> 8;

					for (int i = 0; i <= x; i++) {
						m_Reg[i] = m_Memory[m_RegI + i];
					}
					m_RegI += x + 1;
				}
					break;
				default:
					break;
			}
			break;
		default:
			break;
	}

	//m_Key = 0;
}

void Chip8::DecreaseTimers() {
	--m_TimerDelay;
	if (m_TimerDelay < 0) {
		m_TimerDelay = 0;
	}

	--m_TimerSound;
	if (m_TimerSound < 0) {
		m_TimerSound = 0;
	}


}

