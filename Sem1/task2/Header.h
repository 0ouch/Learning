#pragma once
#include<vector>

void inputofgrades(std::vector<std::vector<double>>& gradearr, const int studnum, const int subjnum);
void arithmeticmean(std::vector<std::vector<double>>& gradearr, const int studnum, const int subjnum);
void max(double* gradearr, int arrsize);
void min(double* gradearr, int arrsize);
void ceiling(double* gradearr, int arrsize);
int inputofN();
int inputofM();
