#include "ReferenceIndex.h"

ReferenceIndex::ReferenceIndex(){}

ReferenceIndex::~ReferenceIndex(){}

void ReferenceIndex::find(const char *text, unsigned int size, unsigned int &position, unsigned int &length) const{
	position = 0;
	length = 0;
}

	
void ReferenceIndex::save(const char *ref_file){}

void ReferenceIndex::load(const char *ref_file){}

char *ReferenceIndex::loadText(const char *ref_file){
	cerr<<"ReferenceIndex::loadText\n";
	return NULL;
}

unsigned int ReferenceIndex::getLength(){
	return 0;
}

const char *ReferenceIndex::getText() const{
	return NULL;
}

void ReferenceIndex::search(const char *text, unsigned int size, vector<unsigned int> &res) const{
}
