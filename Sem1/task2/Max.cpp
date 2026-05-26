//poisk max znachenia

#include<algorithm>
#include<iostream>
using namespace std;

void max(double* gradearr, int arrsize) {
	cout << "Maximum grade: " << *max_element(gradearr, gradearr+arrsize) << endl<<endl;
}