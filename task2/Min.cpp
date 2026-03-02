//poisk min znachenia

#include<algorithm>
#include<iostream>
using namespace std;

void min(double* gradearr, int arrsize) {
	cout << "Minimum grade: " << *min_element(gradearr, gradearr + arrsize) << endl << endl;
}