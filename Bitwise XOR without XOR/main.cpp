#include <algorithm>
#include <cmath>
#include <iostream>
#include <fstream>
#include <stdexcept>
#include <string>

using namespace std;

int XOR1(int a, int b) {
	return ( (~a & b) | (a & ~b) );
}
int XOR2(int a, int b) {
	return ( (a | b) & ~(a & b) );
}
int XOR3(int a, int b) {
	return ( (a | b) & (~a | ~b) );
}
int XOR4(int a, int b) {
	return ~( ~(a & ~(a & b) ) & ~(~(a & b) & b) );
}
int HalfAdder(int a, int b, int &carry) {
	carry = a & b;
	return XOR1(a, b);
}
int FullAdder(int a, int b, int carryIn, int&carryOut) {
	int carry1(0), carry2(0);
	int sum1 = HalfAdder(a, b, carry1);
	int sum = HalfAdder(sum1, carryIn, carry2);
	carryOut = carry1 | carry2;
	return sum;
}
int BinarySum(int a, int b) {
	if(b > a)
		swap(a, b);
	int sum(0), carryIn(0), carryOut(0);
	int i(0);
	for(; a != 0; i++) {
		sum |= FullAdder(a&1, b&1, carryIn, carryOut) << i;
		carryIn = carryOut;
		a >>= 1;
		b >>= 1;
	}
	if(carryOut == 1)
		sum |= 1 << i;
	
	return sum;
}
int BinaryMultiply(int a, int b) {
	bool isNeg = false;
	if((a < 0) ^ (b < 0))
		isNeg = true;
	if(a < 0) 
		a = ~a + 1;
	if(b < 0)
		b = ~b + 1;
	if(b > a)
		swap(a, b);
	int sum(0);
	for(int i = 0; b != 0; i++) {
		if((b&1) != 0)
			sum = BinarySum(sum, a << i);
		b >>= 1;
	}
	if(isNeg)
		return ~sum + 1;
	return sum;
}

int main(int argc, char **argv) {
	int num1(0), num2(0); // E.X.: 1010 (10) XOR 1100 (12) = 0110 (6)
	
	//// INPUT
	if(argc >= 2) {
		try {
			num1 = stoi(argv[1]);
			num2 = stoi(argv[2]);
		}
		catch(const exception &e) {
				cout << "Wrong argument.\n";
				return 0;
		}
	}
	else {
		cout << "Need two decimals as arguments.\n";
		return 0;
	}
	
	//// XOR
	int x1(XOR1(num1, num2)),
			x2(XOR2(num1, num2)),
			x3(XOR3(num1, num2)),
			x4(XOR4(num1, num2));
	cout << "XOR1: " << x1 << endl;
	cout << "XOR2: " << x2 << endl;
	cout << "XOR3: " << x3 << endl;
	cout << "XOR4: " << x4 << endl;

	//// MULTIPLICATION 
	int mul(0);
	mul = BinaryMultiply(num1, num2);
	cout << "Multiplication: " << mul << endl;

	//// CSV OUTPUT
	ofstream fout("output.csv");
	const char COMMA = ';';
	fout << "A" << COMMA << "B" << COMMA << "XOR1" << COMMA << "XOR2" << COMMA << "XOR3" << COMMA << "XOR4" << COMMA << "MULTIPLICATION" << "\n";
	fout << num1 << COMMA << num2 << COMMA << x1 << COMMA << x2 << COMMA << x3 << COMMA << x4 << COMMA << mul << "\n";
	fout.close();
	
	return 0;
}