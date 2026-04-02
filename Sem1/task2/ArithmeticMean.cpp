//srednee arithmeticheskoe

#include <iostream>
#include<cmath>
#include<vector>
#include<algorithm>
using namespace std;

void arithmeticmean(vector<vector<double>>& gradearr, const int studnum, const int subjnum) {
	double mean = 0;
	int choice = 0;
	do {
		cout << "What kind of mean are you looking for?" << endl << "For student mean press 1,"<<endl;
		cout << "for subject mean press 2," << endl;
		cout<<"to find the highest mean among students press 3,"<<endl;
		cout<<"to exit press anything else: ";
		cin >> choice;
		vector<double> means(studnum, 0);	//для поиска наибольшего среднего, кейс 3
		switch (choice) {
		case 1: {
			for (int i = 0; i < studnum; i++) {
				cout << "Student number " << i + 1 << " mean is: ";
				for (int j = 0; j < subjnum; j++) {
					mean = mean + gradearr[i][j];
				}
				mean = mean / subjnum;
				cout << mean << endl;
				mean = 0;
			}
			break;
		}
		case 2: {
			for (int j = 0; j < subjnum; j++) {
				cout << "Subject number " << j + 1 << " mean is: ";
				for (int i = 0; i < studnum; i++) {
					mean = mean + gradearr[i][j];
				}
				mean = mean / studnum;
				cout << mean << endl;
				mean = 0;
			}
			break;
		}
		case 3: {
			for (int i = 0; i < studnum; i++) {
				int j;
				for (j = 0; j < subjnum; j++) {
					mean = mean + gradearr[i][j];
				}
				mean = mean / subjnum;
				means[i] = mean;
				mean = 0;
			}
			auto max = max_element(means.begin(), means.end());
			cout << "Maximum mean among students is: " << *max << endl; ;
			break;
		}
		default:
			cout << "Exiting";
		}
		cout << endl;
	} while (choice == 1 || choice==2|| choice==3);
	cout << endl;
}