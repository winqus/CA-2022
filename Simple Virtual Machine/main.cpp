
/* EXAMPLE CONSOLE COMMANDS
*		./main "Testing files/decryptor.bin" "Testing files/q1_encr.txt"
*/

#include <algorithm>
#include <array>
#include <cmath>
#include <iostream>
#include <fstream>
#include <stdexcept>
#include <string>

using namespace std;

const unsigned int REG_COUNT = 16;
const unsigned int MEM_SIZE = 256;

int ReadToMem(string filePath, unsigned char arr[]) {
	ifstream infile(filePath, ios::binary);

	auto is_empty = [](ifstream &f)->bool{return f.peek() == ifstream::traits_type::eof();};
	if(!infile.is_open() || is_empty(infile)) {
		cout << "Failed to open file..\n";
		return -1;
	}
	int i(0);
	for(i = 0; !infile.eof(); i++){
		if(i >= MEM_SIZE-1) {
			cout << "Not enough memomory...\n";
			return -1;
		}
		infile.get((char&)arr[i]);
	}
	infile.close();
	return 0; //Success
}

int main(int argc, char **argv) {	
	bool DEBUGMODE = false;
	string binFile, textFile; 
	
	// INPUT
	if(argc > 2) {
		try {
			binFile = argv[1];
			textFile = argv[2];
			if(argc > 3) {
				if(string(argv[3]) == "-d")
					DEBUGMODE = true;
					cout << "DEBUG MODE IS ON!\n";
			}
		}
		catch(const exception &e) {
				cout << "Wrong argument.\n";
				return 0;
		}
	}
	else {
		binFile = "decryptor.bin";
		textFile = "q1_encr.txt";
		cout << "Need two files as arguments (<bin> <textFile>).\n";
		cout << "Auto-run with <" << binFile << "> and <" << textFile << ">\n";
	}
	
	unsigned char regs[REG_COUNT] = {};
	unsigned char prog_mem[MEM_SIZE] = {};
	int flag{}; // 0x01 bit is EOF flag, 0x02 bit is ZERO flag.
	
	if(ReadToMem(binFile, prog_mem) != 0) {
		return 0;
	}

	ifstream infile(textFile, ios::binary);
	auto is_empty = [](ifstream &f)->bool{return f.peek() == ifstream::traits_type::eof();};
	if(!infile.is_open() || is_empty(infile)) {
		cout << "Failed to open file <" << textFile << ">\n";
		return 0;
	}
	
	ofstream fout("file.out", ios::binary);
	auto regsCout = [&regs]()->void{for(int i = 0; i < REG_COUNT; i++) cout << (int)regs[i] << ' '; cout << '\n';};
	
	// 0x01 bit is EOF flag, 0x02 bit is ZERO flag.
	int i(0);
	bool isRet = false, inc_i;
	while(!isRet) {
		inc_i = true;
		unsigned char code, op, rx, ry;
		if(i+1 < MEM_SIZE) {
			code = prog_mem[i]; 
			op   = prog_mem[i+1];
			rx = op&0x0F;
			ry = (op&0xF0) >> 4;
		}	
		else {
			cout << "Seg-fault...\n";
			break;
		}
		// LAMBDA FUNCTIONS
		auto setZeroFlag = [&flag](const int reg)->void{ 
			if(reg==0) {flag|=0x02;} 
			else flag&=0x01; 
		};
		auto doJMP = [&]()->void {
			i = i + char(op);
			inc_i = false;
		};
		
		if(code == 0x01) { // INC (Rx)
			regs[rx]++;
			setZeroFlag(regs[rx]);
		}
		else if(code == 0x02) { // DEC (Rx)
			regs[rx]--;
			setZeroFlag(regs[rx]);
		}
		else if(code == 0x03) { // MOV (Ry->Rx)
			regs[rx] = regs[ry];
		}
		else if(code == 0x04) { // MOVC (op->R0)
			regs[0] = op;
		}
		else if(code == 0x05) { // LSL (Rx<<1)
			regs[rx] <<= 1;
			setZeroFlag(regs[rx]);
		}
		else if(code == 0x06) { // LSR (Rx>>1)
			regs[rx] >>= 1;
			setZeroFlag(regs[rx]);
		}
		else if(code == 0x07) { // JMP (i+op)
			if(DEBUGMODE) cout << "cur i is " << i << ",";
			doJMP();
			if(DEBUGMODE) { cout << "JMP to <" << i << "> "; }
		}
		else if(code == 0x08) { // JZ (i+addr if zero flag)
			if(flag&0x02) {
				doJMP();
				if(DEBUGMODE) { cout << "JZ to <" << i << ">\n"; }
			}
		}
		else if(code == 0x09) { // JNZ (i+addr if no zero flag)
			if(!(flag&0x02)) {
				doJMP();
				if(DEBUGMODE) { cout << "JNZ to <" << i << ">\n"; }
			}
		}
		else if(code == 0x0A) { // JFE (i+addr if eof flag)
			if(flag&0x01) {
				doJMP();
				if(DEBUGMODE) { cout << "JFE to <" << i << ">\n"; }
			}
		}
		else if(code == 0x0B) { // RET (exit)
			isRet = true;
		}
		else if(code == 0x0C) { // ADD (Rx+=Ry)
			regs[rx] += regs[ry];
			setZeroFlag(regs[rx]);
		}
		else if(code == 0x0D) { // SUB (Rx-=Ry)
			regs[rx] -= regs[ry];
			setZeroFlag(regs[rx]);
		}
		else if(code == 0x0E) { // XOR (Rx^=Ry)
			regs[rx] ^= regs[ry];
			setZeroFlag(regs[rx]);
		}
		else if(code == 0x0F) { // OR  (Rx|=Ry)
			regs[rx] |= regs[ry];
			setZeroFlag(regs[rx]);
		}
		else if(code == 0x10) { // IN  (ReadByte->Rx and set flag if eof)
			unsigned char tmp_byte;
			infile.get((char&)tmp_byte);
			regs[rx] = tmp_byte;
			if(infile.eof())
				flag |= 0x01;
		}
		else if(code == 0x11) { // OUT (Rx->OutputFile)
			fout << regs[rx];
			if(DEBUGMODE) { cout << "char <" << (int)regs[rx] << "," << regs[rx] << ">, "; }
		}
		else {
			cout << "i=" << i << ", unknown command code <" << (int)code << ">\n";
			break;
		}
		if(DEBUGMODE) {cout << "code <" << (int)code << ">, op <" << (int)op << ">, flag<" << (int)flag << ">,\tregs: ";regsCout();}
		
		if(inc_i) i+=2;
	}
	cout << "VM exited." << endl;
	return 0;
}