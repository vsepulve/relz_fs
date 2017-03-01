
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <sstream>
#include <fstream>
#include <string.h>

#include <sys/types.h>
#include <dirent.h>

#include "FilesTree.h"

using namespace std;

int main(int argc, char *argv[]){
	
	FilesTree arbol;
	
	cout<<"Prueba de Color\n";
	
	for(unsigned int i = 0 ; i < 256; ++i){
		cout<<"\033[1;38;5;"<<i<<"m Texto Color "<<i<<"\033[0m\n";
	}
	
//	FileNode raiz("/", true);
//	loadNode(&raiz, "test/");
//	raiz.print();
	
	arbol.loadDirectory("test/");
	arbol.print();
	
	arbol.remove("test.txt");
	arbol.print();
	cout<<"-----\n";
	arbol.remove("/test.txt");
	arbol.print();
	cout<<"-----\n";
	arbol.remove("/dirA/dirAA/");
	arbol.print();
	cout<<"-----\n";
	
	return 0;
}













