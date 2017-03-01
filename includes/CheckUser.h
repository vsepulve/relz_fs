#ifndef _CHECK_USER_H
#define _CHECK_USER_H


#include <iostream>
#include <fstream>
#include <string>
//for time
#include <ctime>
//for strcat
#include <stdio.h>
#include <string.h>
//for string stream
#include <sstream>

using namespace std;

class CheckUser {
private: 
	char * db_name;
	
public: 
	CheckUser(string db_name);
	~CheckUser();
	bool valid(unsigned int user, char* passwd);
};
#endif //_CHECK_USER_H
