#include "CheckUser.h"

CheckUser::CheckUser(string _db_name){
	db_name = new char[_db_name.size() + 1];
	strcpy(db_name, _db_name.c_str());
}

CheckUser::~CheckUser(){
	delete[] db_name;
}

bool CheckUser::valid(unsigned int user, char* passwd){
	return true;	
}
