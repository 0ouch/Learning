//vvod chisla studentof
#include <iostream>
int N;
using namespace std;

int inputofN() {
	cout << "Input the number of students, number must be > 0: ";
	do {
		cin >> N;	//ввод кол-ва студентов
		if (N <= 0)
			cout << "Try again!" << endl;
	} while (N <= 0);
	cout << "Number of students: " << N << endl<<endl;
	return N;
}