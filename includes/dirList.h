#ifndef dirList_H
#define dirList_H


#include <iostream>
#include <fstream>
#include <string>
#include <string.h>
//for time
#include <ctime>
//for strcat
#include <stdio.h>
#include <vector>
//string stream
#include <sstream>
using namespace std;

class dirList {
private: 
	vector<pair<string,int>> *files;
	vector<string> *directories;
public: 
	dirList();
	dirList(char * serial);
	~dirList();
	void add_file(pair<string,int> file);
	void add_dir(string dir);
	void print(ostream * out);
	void serialize(char * ret);
	int size();
};


#endif //dirList_Hr_H
