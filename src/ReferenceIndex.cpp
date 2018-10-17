#include "ReferenceIndex.h"

const unsigned char ReferenceIndex::type = 0;

ReferenceIndex::ReferenceIndex(){}

ReferenceIndex::~ReferenceIndex(){}

void ReferenceIndex::find(const char *text, unsigned int size, unsigned int &position, unsigned int &length, bool verify_range, unsigned int min_pos, unsigned int max_pos) const{
	cerr << "ReferenceIndex::find - Error, Not Implemented.\n";
	position = 0;
	length = 0;
}

	
void ReferenceIndex::save(const char *ref_file){}

void ReferenceIndex::load(const char *ref_file){}

char *ReferenceIndex::loadText(const char *ref_file){
	cerr << "ReferenceIndex::loadText - Error, Not Implemented.\n";
	return NULL;
}

unsigned int ReferenceIndex::getLength(){
	cerr << "ReferenceIndex::getLength - Error, Not Implemented.\n";
	return 0;
}

const char *ReferenceIndex::getText() const{
	cerr << "ReferenceIndex::getText - Error, Not Implemented.\n";
	return NULL;
}

void ReferenceIndex::search(const char *text, unsigned int size, vector<unsigned int> &res) const{
	cerr << "ReferenceIndex::search - Error, Not Implemented.\n";
}
