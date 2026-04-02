//ввод числа предметов
#include <iostream>
int M;
using namespace std;

int inputofM() {
	cout << "Input the number of subjects, number must be > 0: ";
	do {
		cin >> M;	//ввод кол-ва предметов
		if (M <= 0)
			cout << "Try again!" << endl;
	} while (M <= 0);
	cout << "Number of subjects: " << M << endl << endl;
	return M;
}