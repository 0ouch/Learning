//first v of input

#include <iostream>
#include <typeinfo>
	using namespace std;

void input1(double* gradearr, int arrsize) {
	
	for (int i = 0; i < arrsize; i++) {
		cout << "Input grade of the student number " << i+1 << ": ";
		int a;	//ввод оценки
		cin >> a;
		if (a >= 0 && a <= 5 && typeid(a) == typeid(int))	// проверка ввода
			gradearr[i] = a;
		else {
			cout << "Input must be an integer between 0 and 5, try again" << endl;
			i--;
		}

	}
	cout << endl<< "Grades:" << endl;	//печать введенных значений
	for (int i = 0; i < arrsize; i++) {
		cout << "Student number "<<i+1<<": " << gradearr[i]<<endl;
	}
	cout << endl;
}