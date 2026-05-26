//main.cpp
//Печать на английском потому что моя система не печатает нормально русский

#include <iostream>
#include<cmath>
#include "Header.h"
#include<vector>

int main()
{
	using namespace std;
	const int N=inputofN();	//ввод кол-ва студентов
	const int M = inputofM();	//ввод кол-ва предметов

	//double* grades = new double[N];	//массив оценок
	vector< vector<double>> grades(N, vector<double>(M));

	inputofgrades(grades, N,M);
	arithmeticmean(grades, N,M);
	//max(grades, N);
	//min(grades, N);
	//ceiling(grades, N);

	//delete[] grades;	//освобождаем память и зануляем указатель
	//grades = nullptr;
	return 0;
}