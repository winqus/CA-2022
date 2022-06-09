#include <algorithm>
#include <array>
#include <cmath>
#include <emmintrin.h>
#include <iostream>
#include <fstream>
#include <stdexcept>
#include <stdint.h>
#include <string.h>
#include <stdio.h>

using namespace std;


bool DEBUGMODE = false;
const char COMMA = ',';

int main(int argc, char **argv) {	
    float x[4] __attribute__ ((aligned (16))); 
    float y[4] __attribute__ ((aligned (16)));
    float z[4] __attribute__ ((aligned (16)));
    float outs[4] __attribute__ ((aligned (16)));

    ofstream fout("pitOut.csv");
    fout << "x" << COMMA << "y" << COMMA << "z\n";

    //// SSE
	for(int i = 1; i <= 1000; i++) {
        for(int j = i; j <= 1000; j+=4) {

            for(int m=0; m<4; m++) { 
                x[m] = (float)i; 
                y[m] = (float)(j+m); 

                if(y[m] > 1000)
                    y[m] = 1.; 
            }

            asm (
                "MOVAPS xmm0, [%2];"
                "MOVAPS xmm1, [%3];"

                "MULPS xmm0, xmm0;"             // square Xs
                "MULPS xmm1, xmm1;"             // square Ys
                "ADDPS xmm0, xmm1;"             // square sums
                "MOVAPS xmm4, xmm0;"            // square sums copy
                "SQRTPS xmm4, xmm4;"            // root of sums
                "MOVAPS xmm2, xmm4;"            // root of sums copy (to z)
                "CVTPS2DQ xmm4, xmm4;"          // roots(floats) to ints
                "CVTDQ2PS xmm4, xmm4;"          // ints to floats
                "MULPS xmm4, xmm4;"             // square floats
                "SUBPS xmm4, xmm0;"             // float and sum difference

                "MOVAPS [%0], xmm4;" 
                "MOVAPS [%1], xmm2;"
                : "=m"(outs),"=m"(z)
                : "m"(x), "m"(y)
                : "eax", "ebx"
            );

            for(int v = 0; v < 4; v++)
                if(outs[v] == 0)
                    fout << x[v] << COMMA << y[v] << COMMA << z[v] << '\n';

        }
    }
	return 0;
}