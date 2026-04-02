//kol-vo vyshe potolka

#include <iostream>
#include<cmath>
using namespace std;

void ceiling(double* gradearr, int arrsize) {
	int num=0;
	int a;
	cout << "Input the grade ceiling: ";
	cin >> a;
	for (int i = 0; i < arrsize; i++) {
		if (gradearr[i] > a)
			num++;
	}
	cout << "Number of students with grade above the celing: " << num << endl << endl;
}