#include <algorithm>
#include <array>
#include <cmath>
#include <iostream>
#include <fstream>
#include <stdexcept>
#include <stdint.h>
#include <string.h>
#include <stdio.h>

using namespace std;


bool DEBUGMODE = false;

int main(int argc, char **argv) {	
	
	int len{}, iOut{};
	char* line(nullptr);

	//// INPUT
	if(argc > 1) {
		try {
			line = argv[1];
			if(argc > 2) {
				if(string(argv[2]) == "-d")
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
		line = (char*)"tekstas";
		cout << "Need a positive integer as argument.\n";
		return 0;
	}
	char c;

	//// ASM
	asm (
		"mov esi, %2;"
		"xor ecx, ecx;"
		"xor ebx, ebx;"
		"dec esi;"
		"dec ecx;"

		"loop:"
			"inc ecx;"
			"inc esi;"
			"xor eax, eax;"
			"mov al, byte ptr [esi];"
			"cmp al, 0;"
				"jz exit_loop;"
			"cmp al, '0';"
				"jl handle_nan;"
			"cmp al, '9';"
				"jg handle_nan;"
			"sub eax, 48;"
			"imul ebx, 10;"
			"jo handle_of;"
			"add ebx, eax;"
			"jmp loop;"
		"handle_nan:"
			"mov ebx, -2;"
			"jmp exit_loop;"
		"handle_of:"
			"mov ebx, -1;"
		"exit_loop:"
			"mov [%0], ecx;"
			"mov [%1], ebx;"
		
		: "=m"(len), "=m"(iOut)
		: "m"(line)
		: "eax", "ebx", "ecx", "esi"
	);
	//// OUTPUT
	if (DEBUGMODE) cout << "Loops: " << len << endl;
	if(iOut  >= 0)
		cout << "int: <" << iOut << ">" << endl;
	else if(iOut == -1)
		cout << "number is too large...\n";
	else if(iOut == -2)
		cout << "not a positive integer...\n";
	else 
		cout << "an error occured..\n";
		
	return 0;
}