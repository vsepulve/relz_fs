#ifndef _REFERENCE_FACTORY_H
#define _REFERENCE_FACTORY_H

#include <stdio.h>
#include <stdlib.h>
#include <iostream>

#include <algorithm>
#include <vector>

using namespace std;

#include "NanoTimer.h"

#include "ReferenceIndex.h"
#include "ReferenceIndexBasic.h"
#include "ReferenceIndexRR.h"

class ReferenceFactory{

private: 

public: 
	ReferenceFactory();
	virtual ~ReferenceFactory();
	
	// Reads the type so it can load and return the right instance of Reference
	static ReferenceIndex *loadInstance(const char *ref_file);
	
	// Reads and returns only the text from a reference file
	// It will reserve the necesary memory
	static char *loadText(const char *ref_file);
	
	static unsigned int loadFlags(const char *ref_file);
	
};

#endif //_REFERENCE_FACTORY_H

