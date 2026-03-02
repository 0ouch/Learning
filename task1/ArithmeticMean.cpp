//srednee arithmeticheskoe

#include <iostream>
#include<cmath>
using namespace std;
int mean=0;

void arithmeticmean(double* gradearr, int arrsize) {
	for (int i = 0; i< arrsize; i++) {
		mean = mean + gradearr[i];
	}
	mean = mean / arrsize;
	cout<<"Arithmetic mean: "<<mean << endl<<endl;
}