#include <algorithm>
#include <cmath>
#include <fstream>
#include <iostream>
#include <map>
#include <stdexcept>
#include <string>
#include <sstream>

using namespace std;

int IntToUtf8(int num) {
	int utf8(0), mask(0);
	if(num <= 0x7F) {
		utf8 = num;
	}
	else if(num <= 0x7FF) {
		utf8 = 0b1100000010000000;
		utf8 = utf8 |  (num & 0b111111);
		num = num >> 6;
		utf8 = utf8 | ((num & 0b11111) << 8);
	}
	else if(num <= 0xFFFF) {
		utf8 = 0b111000001000000010000000;
		utf8 = utf8 |  (num & 0b111111);
		num = num >> 6;
		utf8 = utf8 | ((num & 0b111111) << 8);
		num = num >> 6;
		utf8 = utf8 | ((num & 0b1111) << 16);
	}
	else if(num <= 0x10FFFF) {
		utf8 = 0b11110000100000001000000010000000;
		utf8 = utf8 |  (num & 0b111111);
		num = num >> 6;
		utf8 = utf8 | ((num & 0b111111) << 8);
		num = num >> 6;
		utf8 = utf8 | ((num & 0b111111) << 16);
		num = num >> 6;
		utf8 = utf8 | ((num & 0b111) << 24);
	}
	return utf8;
}
int ReadFile(string filePath, map<int,int> &dict) {
	ifstream infile;
	infile.open(filePath, ios::in);
	if(!infile.is_open())
		return 1; // Failed to open file
	auto is_empty = [](ifstream &f)->bool{return f.peek() == ifstream::traits_type::eof();};
	if(is_empty(infile))
		return 99;
	string textLine;
	//// State line reading
	while(!infile.eof()) {
		getline(infile, textLine, '\n');
		if(textLine == "" || textLine[0] == '#' || textLine.length() < 11)
			continue;
		
		stringstream ss(textLine);
		int cp437, unicode;
		string strhex;
		try {
			ss >> strhex;
			cp437 = stoi(strhex,0,16);
			ss >> strhex;
			unicode = stoi(strhex,0,16);
			if(cp437 < 0 || cp437 > 0xFF || unicode < 0)
				throw invalid_argument("value out of range");
			dict[cp437] = unicode;
		}
		catch(const exception &e) {
				cout << "Wrong argument.\n";
				return 0;
		}
	}
	infile.close();
	return 0; //Success
}

int main(int argc, char **argv) {
	string filePath, tableFile;
	if(argc > 2) {
		filePath = argv[1];
		tableFile = argv[2];
	}
	else {
		cout << "Missing args: <intelFile> <cp437>\n";
		cout << "Auto run with \"386intel.txt\" and \"CP437.TXT\"\n";
		filePath = "386intel.txt";
		tableFile = "CP437.TXT";
	}
	map<int,int> dict;
	int res = ReadFile(tableFile, dict);
	if(res != 0) {
		cout << "Encountered an error #" << res << endl;
		return 0;
	}

	ifstream intelFile(filePath, ios::binary);
	ofstream outfile("output.txt", ios::binary);

	auto is_empty = [](ifstream &f)->bool{return f.peek() == ifstream::traits_type::eof();};
	if(!intelFile.is_open() || is_empty(intelFile)) {
		cout << "Failed to open file..\n";
		return 0;
	}

	while(!intelFile.eof()){
		char a;
		unsigned char b;
		intelFile.get(a);
		b = a;
		unsigned int u8, mask = 0xFF000000;
		auto itr = dict.find(b);
		if(itr != dict.end())
			u8 = IntToUtf8(dict[b]);
		else
			u8 = IntToUtf8(b);
		bool nulls(true);
		int i = 0;
		for(int j = 0; j < 4; j++) {
			unsigned char b = ((u8 & (mask >> (8*j))) >> (8*(3-j)));
			if(b != 0 || !nulls) {
				if(nulls)
					nulls = false;
				outfile << b;
				i++;
			}
		}
	}
	
	intelFile.close();
	outfile.close();
	
	return 0;
}