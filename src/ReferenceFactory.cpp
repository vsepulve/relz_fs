#include "ReferenceFactory.h"

ReferenceFactory::ReferenceFactory(){}

ReferenceFactory::~ReferenceFactory(){}

ReferenceIndex *ReferenceFactory::loadInstance(const char *ref_file){
	
	fstream reader(ref_file, fstream::binary | fstream::in);
	if( ! reader.good() ){
		cerr << "ReferenceIndexRR::load - Error opening file \"" << ref_file << "\"\n";
		return NULL;
	}
	
	unsigned char type = 0;
	reader.read((char*)&type, 1);
	reader.close();
	
	ReferenceIndex *ref = NULL;
	
	if( type == 1 ){
		cout << "ReferenceFactory - Loading ReferenceIndexBasic\n";
		ref = new ReferenceIndexBasic();
		ref->load(ref_file);
	}
	else if( type == 2 ){
		cout << "ReferenceFactory - Loading ReferenceIndexRR\n";
		ref = new ReferenceIndexRR();
		ref->load(ref_file);
	}
	else{
		cerr << "ReferenceFactory - Unexpected Reference Type (" << (unsigned int)type << ")\n";
	}
	
	return ref;
}

// Notar que este metodo es static
char *ReferenceFactory::loadText(const char *ref_file){
	char *text = NULL;
	unsigned char type = 0;
	unsigned int type_flags = 0;
	unsigned int text_size = 0;
	unsigned int arr_size = 0;
	
	fstream reader(ref_file, fstream::binary | fstream::in);
	if( ! reader.good() ){
		cerr<<"ReferenceFactory::loadText - Error al abrir archivo.\n";
		return NULL;
	}
	
	reader.read((char*)&type, 1);
	reader.read((char*)(&type_flags), sizeof(int));
	reader.read((char*)(&text_size), sizeof(int));
	reader.read((char*)(&arr_size), sizeof(int));
	cout<<"ReferenceFactory::loadText - Loading reference text (" << text_size << " chars) from \"" << ref_file << "\" (type: " << (unsigned int)type << ")\n";
	
	text = new char[text_size + 1];
	reader.read(text, text_size);
	text[text_size] = 0;
	
	reader.close();
	
	return text;
}
