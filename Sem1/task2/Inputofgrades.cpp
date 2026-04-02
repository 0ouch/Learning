//input grades

#include <iostream>
#include<vector>
	using namespace std;

void inputofgrades(vector<vector<double>>& gradearr, const int studnum,const int subjnum) {
	
	for (int i = 0; i < studnum; i++) {
		for (int j = 0; j < subjnum; j++) {
			cout << "Input grade for subject number " << j + 1 << " of the student number " << i + 1 << ": ";
			int a;	//ввод оценки
			cin >> a;
			if (a >= 0 && a <= 5)	// проверка ввода
				gradearr[i][j] = a;
			else {
				cout << "Input must be an integer between 0 and 5, try again" << endl;
				j--;
			}
		}
			cout << endl;
	}
	cout << endl<< "Grades:" << endl;	//печать введенных значений
	for (int i = 0; i < studnum; i++) {
		cout << "Student number " << i + 1 << ": " << endl;
		for (int j = 0; j < subjnum; j++) {
			cout<<"Subject "<<j+1<<", grade is " << gradearr[i][j] << endl;
		}
		cout << endl;
	}
	cout << endl;
}