#include "Configuration.h"

Configuration::Configuration(){
	base_path = (char*)("./test_real");
	// ruta de la referencia
	reference_file = (char*)("./data/ref.bin");
	// block_size para la compression
	compress_block_size = 100000;
	// numero maximo de threads a ser usados para comprimir
	compress_max_threads = 4;
	// largo de bloque para descomprimir (se pide/borra esa ram por cada descompresion completa)
	decompress_line_size = 65536;
	// Tamaño preferido para operaciones IO (por ejemplo, write)
	// Para usarlo apropiadamente, agrego "-o big_writes" al demonio
	io_block_size = 131072;
}

Configuration::Configuration(string &filename) : Configuration() {
	loadConfiguration(filename);
}

Configuration::~Configuration(){
}

void Configuration::loadConfiguration(string &filename){
	
	ifstream lector(filename, ifstream::in);
	if( ! lector.is_open() ){
		cout << "Configuration::loadConfiguration - Can't open file \"" << filename << "\"\n";
		return;
	}
	
	unsigned int buff_size = 1024;
	char buff[buff_size];
	memset(buff, 0, buff_size);
	
	string line, name, mark, value;
	
	while( lector.good() ){
		lector.getline(buff, buff_size);
		unsigned int lectura = lector.gcount();
		unsigned int pos = 0;
		while( (pos < lectura) && buff[pos] == ' ' ){
			++pos;
		}
		if( (pos + 1 >= lectura) || (buff[pos] == '#') ){
			continue;
		}
		//Linea valida de largo > 0
//		cout<<"Procesando \""<<buff<<"\" (pos: "<<pos<<" / "<<lectura<<")\n";
		string line(buff);
		stringstream toks(line);
		
		name = "";
		mark = "";
		value = "";
		
		toks >> name;
		toks >> mark;
		toks >> value;
		
		if( (mark.length() != 1) || (mark.compare("=") != 0) || (value.length() < 1) ){
			continue;
		}
		
		cout << "Configuration::loadConfiguration - Setting \"" << name << "\"\n";
		
		if( name.compare("base_path") == 0 ){
			toks >> value;
			cout << "Configuration::loadConfiguration - Value \"" << value << "\"\n";
			char *text = new char[ 1 + value.length() ];
			strcpy(text, value.c_str());
			base_path = text;
		}
		else if( name.compare("reference_file") == 0 ){
			toks >> value;
			cout << "Configuration::loadConfiguration - Value \"" << value << "\"\n";
			char *text = new char[ 1 + value.length() ];
			strcpy(text, value.c_str());
			reference_file = text;
		}
		else if( name.compare("compress_block_size") == 0 ){
			toks >> value;
			cout << "Configuration::loadConfiguration - Value \"" << value << "\"\n";
			compress_block_size = atoi( value.c_str() );
		}
		else if( name.compare("compress_max_threads") == 0 ){
			toks >> value;
			cout << "Configuration::loadConfiguration - Value \"" << value << "\"\n";
			compress_max_threads = atoi( value.c_str() );
		}
		else if( name.compare("decompress_line_size") == 0 ){
			toks >> value;
			cout << "Configuration::loadConfiguration - Value \"" << value << "\"\n";
			decompress_line_size = atoi( value.c_str() );
		}
		else if( name.compare("io_block_size") == 0 ){
			toks >> value;
			cout << "Configuration::loadConfiguration - Value \"" << value << "\"\n";
			io_block_size = atoi( value.c_str() );
		}
		else{
			cout << "Configuration::loadConfiguration - Omitting value of \"" << name << "\"\n";
		}
	
	}

}








