//main.cpp
//Печать на английском потому что моя система не печатает нормально русский

#include <iostream>
#include<cmath>
#include "Header.h"

int main()
{
	using namespace std;
	int N=inputofN();	//ввод кол-ва студентов

	double* grades = new double[N];	//массив оценок

	input1(grades, N);
	arithmeticmean(grades, N);
	max(grades, N);
	min(grades, N);
	ceiling(grades, N);

	delete[] grades;	//освобождаем память и зануляем указатель
	grades = nullptr;
	return 0;
}