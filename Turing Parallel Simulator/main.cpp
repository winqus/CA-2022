#include <algorithm>
#include <fstream>
#include <iostream>
#include <unistd.h>
#include <map>
#include <mutex>
#include <thread>
#include <stdexcept>
#include <stdio.h>
#include <string>
#include <sstream>
#include <vector>

using namespace std;

//// CONFIGURATION VARIABLES
useconds_t USLEEPTIME = 1000;
bool FINITE = false;
bool OUTPUTCHANGE = true;
bool CHECKINF = false;
bool FASTMODE = false;

//// ANSI COLORS
const string red = "\033[1;4;31m", yellow = "\033[1;33m", magenta = "\033[1;35m", cyan = "\033[1;36m", cyanU = "\033[1;4;36m", green = "\033[1;32m";
const string endColor = "\033[0m";

struct NewSymDirState {
	char newSymbol;
	char direction; // L or R
	string newState;
};

typedef map<string, map<char, NewSymDirState>> msmcE;
typedef map<char, NewSymDirState> mcE;

mutex mut;

string RecolorCharOfString(string s, const int i, const string color) {
	s.insert(i,color);
	s.insert(i+color.length()+1,endColor);
	return s;
}
string InsertStringAroundStringElement(string s, const int i, const string ins) {
	s.insert(i,ins);
	s.insert(i+ins.length()+1,ins);
	return s;
}
int ReadFile(string filePath, string &tape, int &position, msmcE &newSDSs) {
	ifstream infile;
	infile.open(filePath, ios::in);
	if(!infile.is_open())
		return 1; // Failed to open file
	auto is_empty = [](ifstream &f)->bool{return f.peek() == ifstream::traits_type::eof();};
	if(is_empty(infile))
		return 99;
	//// Reading and checking the tape
	getline(infile, tape, '\n');
	if(tape.find(" ") != string::npos)
		return 2; // Illegal chars (e. x.: spaces)
	
	if(is_empty(infile))
		return 3;
	
	//// Position input handling
	string textLine = "";
	while(textLine == "")
		getline(infile, textLine, '\n');
	stringstream sstmp(textLine);
	string word;
	sstmp >> word;
	try {
		position = stoi(word);
		if(position < 1)
			throw invalid_argument("less than 1 value");
		else if(position > tape.length())
			throw out_of_range("input position is out of tape range");
	}
	catch(const invalid_argument& e) {
		return 99; // Wrong input.
	}
	catch(const out_of_range& e) {
		return 9; // Out of range.
	}
	catch(const exception &e) {
		return -1; // Something went wrong... 
	}
	//// State line reading
	while(!infile.eof()) {
		getline(infile, textLine, '\n');
		if(textLine == "")
			continue;
		if(count(textLine.begin(), textLine.end(), ' ') < 4)
			return 3; // Wrong parameter count 
		
		stringstream ss(textLine);
		string state; ss >> state;
		if(state == "")
			return 4; //Wrong state
		
		string tmp; ss >> tmp;
		char symbol;
		if(tmp.length() == 1 && tmp != "")
			symbol = tmp[0];
		else return 5; //Wrong symbol

		//// New symbol, direction, state parsing
		NewSymDirState newSDS;
		ss >> tmp;
		if(tmp.length() == 1 && tmp != "")
			newSDS.newSymbol = tmp[0];
		else return 6; //Wrong new symbol
		ss >> tmp;
		if(tmp == "L" || tmp == "R")
			newSDS.direction = tmp[0];
		else return 7; //Wrong direction symbol
		ss >> newSDS.newState;
		if(newSDS.newState == "")
			return 8; //Wrong new state

		//// Updating the map
		newSDSs[state][symbol] = newSDS;
	}
	infile.close();
	return 0; //Success
}
void Turing(const string filePath, const int row) {
	mut.lock();
	
	string tape;
	int position;
	map<string, map<char, NewSymDirState>> newSDSs;
	int ret = ReadFile(filePath, tape, position, newSDSs);
	printf("\033[%d;0H", row);
	if(ret != 0){
		string msg = "<Error>";
		switch(ret) {
			case 1:
				msg += "Failed to open file.";
				break;
			case 2:
				msg += "Illegal chars (e.x. spaces) in input tape.";
				break;
			case 3:
				msg += "Wrong parameter count.";
				break;
			case 4:
				msg += "Wrong state.";
				break;
			case 5:
				msg += "Wrong symbol.";
				break;
			case 6:
				msg += "Wrong new symbol.";
				break;
			case 7:
				msg += "Wrong direction symbol.";
				break;
			case 8:
				msg += "Wrong new state.";
				break;
			case 9:
				msg += "Input position is out of tape range.";
				break;
			case 99:
				msg += "Wrong input.";
				break;
			default:
				msg += "Something went wrong...";
				break;
		}
		cout << "<" << filePath << ">: " << msg << '\n';
		mut.unlock();
		return;
	}
	//// Position check
	int i = (position > 0 && position <= tape.length()) ? (position-1) : 0;

	//// Let other threads read input files.
	mut.unlock();

	//// Print file name on specific row.
	printf("\033[%d;0H", row);
	if(!OUTPUTCHANGE) cout << "<" << filePath << ">\t" << tape << "<IN PROGRESS>\n";
	else cout << "<" << filePath << ">\t" << tape << "\n";
	
	//// Tracker variables
	int steps = 1;
	map<string, map<char, map<int, map<string, bool>>>> mhistory;
	bool infinite = false;
	string state = "0";

	//// Turing code execution
	while(true) {

		string outtape(tape);

		//// Stop if finite
		if(infinite && FINITE)
		{
			mut.lock();
			printf("\033[%d;0H", row);
			cout << "<" << filePath << ">\t" << magenta << "HALT_INFINITE_RUN_DETECTED" << endColor << ", tape: " << RecolorCharOfString(outtape,i,yellow) << " <steps: " << steps << ">\n";
			mut.unlock();
			break;
		}
		msmcE::iterator itr1 = newSDSs.find(state);
		//// No defined state found
		if(itr1 == newSDSs.end())
		{
			mut.lock();
			printf("\033[%d;0H", row);
			cout << "<" << filePath << ">\t" << cyan << "HALT_UNDEFINED_STATE" << endColor << " <"<< state << ">, tape: " << RecolorCharOfString(outtape,i,cyanU) << " <steps: " << steps << ">\n";
			mut.unlock();
			break;
		}
		char symbol = tape[i];
		//// Symbol is undefined
		mcE::iterator itr2 = (itr1->second).find(symbol);
		if(itr2 == (itr1->second).end())
		{
			mut.lock();
			printf("\033[%d;0H", row);
			cout << "<" << filePath << ">\t" << yellow << "HALT_ERROR_UNDEFINED_SYMBOL" << endColor << " <" << symbol << ">, tape: " << tape << " <steps: " << steps << ">\n";
			break;
		}
		//// Get new symbol, direction, state
		NewSymDirState newSDS = newSDSs[state][symbol];

		//// Infinite run check
		if(!infinite && CHECKINF) {
			if(mhistory[state][symbol][i][tape] == true)
				infinite = true;
			else
				mhistory[state][symbol][i][tape] = true;
		}

		//// Tape and state change
		tape[i] = newSDS.newSymbol;
		state = newSDS.newState;

		//// Tape change output
		if(OUTPUTCHANGE) {
			mut.lock();
			printf("\033[%d;0H", row);
			cout << "<" << filePath << ">\t" << RecolorCharOfString(outtape,i,red) << " <steps: " << steps << ">";
			if(!infinite)
				cout << "\n";
			else
				cout << magenta << " <INF>" << endColor << "\n";
			mut.unlock();
		}
		else if (steps % 1000000 == 0) {
			mut.lock();
			printf("\033[%d;0H", row);
			cout << "<" << filePath << ">\t" << RecolorCharOfString(outtape,i,red) << " <steps: " << steps << ">\n";
			mut.unlock();
		}
		//// Move the head
		if(newSDS.direction == 'L' && i-1 >= 0)
			i--;
		else if(newSDS.direction == 'R' && i+1 < tape.size())
			i++;
		else {
			mut.lock();
			printf("\033[%d;0H", row);
			cout << "<" << filePath << ">\t" << "HALT_POSITION_ERROR, tape: " << RecolorCharOfString(tape,i,red) << " <steps: " << steps << ">\n";
			break;
		}
		steps++;
		
		if(!FASTMODE)
			usleep(10);
	}
	mut.unlock();
}
int main(int argc, char **argv)
{
	vector<string> inputFiles;
	//// Arguments
	if(argc <= 1) {
		cout << "No arguments given..\n";
		return 0;
	}
	for(int i = 1; i < argc; i++)
	{
		string s = argv[i];
		if(s == "--help" || s == "-h") {
			cout << "--finite or -f       :(end execution on detected infinite run);" << endl;
			cout << "--infcheck           :(checks for infinite run);" << endl;
			cout << "--utime <useconds_t> :(specify the execution pause time; default is 1000 us);" << endl;
			cout << "--speed or -s        :(fast, finite and few tape changes are printed);" << endl;
			cout << "--many or -m         :(runs with several default input files);" << endl;
			cout << "Other arguments will be each considered an input text file;" << endl;
			return 0;
		}
		else if(s == "--finite" || s == "-f") {
			FINITE = true;
			CHECKINF = true;
		}
		else if(s == "--infcheck") {
			CHECKINF = true;
		}
		else if(s == "--speed" || s == "-s") {
			OUTPUTCHANGE = false;
			FINITE = true;
			USLEEPTIME = 10;
			FASTMODE = true;
		}
		else if(s == "--utime" || s == "-u") {
			try {
				USLEEPTIME = stoi(argv[++i]);
			}
			catch(const exception &e) {
				cout << "Illegal argument\n";
				return 0;
			}
		}
		else if(s == "--many" || s == "-m") {
			if(inputFiles.size() == 0) {
				inputFiles.push_back("Input/1.txt");
				inputFiles.push_back("Input/2.txt");
				inputFiles.push_back("Input/3.txt");
				inputFiles.push_back("Input/4.txt");
				inputFiles.push_back("Input/6 66.txt");
				inputFiles.push_back("Input/0.txt");
				inputFiles.push_back("Input/3 (short).txt");
			}
		}
		else {
			inputFiles.push_back(s);
		}
	}
	
	if(inputFiles.size() > thread::hardware_concurrency()) {
		cout << "Based on current hardware the maximum input file count is " << thread::hardware_concurrency() << ".\n";
		return 0;
	}
	
	printf("\033[2J"); // Clear console window
	//// Multithreading
	vector<thread> threads;
	for(int i = 0; i < inputFiles.size(); i++)
		threads.push_back(thread(Turing, inputFiles[i], i+1));

	//// Converge threads
	for(thread &t : threads)
		if(t.joinable())
			t.join();

	//// Finished executing all input file codes
	printf("\033[%d;0H", int(inputFiles.size()+2));
	cout << "------FINISHED\n";
	return 0;
}