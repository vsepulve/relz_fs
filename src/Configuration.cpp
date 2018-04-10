#include "Configuration.h"

Configuration::Configuration(){
	base_path = (char*)("./test_real");
	// ruta de la referencia
//	reference_file = (char*)("./data/ref.bin");
	ReferenceIndexBasic *ref = new ReferenceIndexBasic();
	ref->load( "./data/ref.bin" );
	references.push_back( pair<string, ReferenceIndexBasic *>("/", ref) );
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

unsigned int Configuration::nextLine(ifstream &lector, char *buff, unsigned int buff_size){
	unsigned int length = 0;
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
		length = strlen(buff);
		break;
	}
	return length;
}

ReferenceIndexBasic *Configuration::getReference(const char *path){
	// buscar en el vector de referencias desde el ultimo (subdir mas largo) hasta el primero
	// Es valido el primero en el que el subdir sea PREFIJO completo del path
	// Por ahora retorno el primero (que DEBE ser el de /)
	if( path == NULL ){
		return references[0].second;
	}
	string str_path(path);
	for( unsigned int i = references.size() - 1; i >= 0; --i ){
		if( str_path.length() > references[i].first.length() && str_path.find(references[i].first) == 0 ){
			cout << "Configuration::getReference - Reference found in \"" << references[i].first << "\" for path \"" << path << "\"\n";
			return references[i].second;
		}
	}
	cerr << "Configuration::getReference - Reference not found.\n";
	return NULL;
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
	
		unsigned int length = nextLine(lector, buff, buff_size);
		if( length == 0){
			break;
		}
		
		//Linea valida de largo > 0
//		cout << "Procesando \"" << buff << "\"\n";
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
//			toks >> value;
			cout << "Configuration::loadConfiguration - Value \"" << value << "\"\n";
			char *text = new char[ 1 + value.length() ];
			strcpy(text, value.c_str());
			base_path = text;
		}
		if( name.compare("host") == 0 ){
//			toks >> value;
			cout << "Configuration::loadConfiguration - Value \"" << value << "\"\n";
			char *text = new char[ 1 + value.length() ];
			strcpy(text, value.c_str());
			host = text;
		}
		else if( name.compare("port") == 0 ){
//			toks >> value;
			cout << "Configuration::loadConfiguration - Value \"" << value << "\"\n";
			port = atoi( value.c_str() );
		}
		else if( name.compare("user_id") == 0 ){
//			toks >> value;
			cout << "Configuration::loadConfiguration - Value \"" << value << "\"\n";
			user_id = atoi( value.c_str() );
		}
		else if( name.compare("n_ref") == 0 ){
//			toks >> value;
			cout << "Configuration::loadConfiguration - Value \"" << value << "\"\n";
//			compress_block_size = atoi( value.c_str() );
			// En este caso, hay que tomar las n referencias para la tabla
			unsigned int n_ref = atoi(value.c_str());
			set< pair<unsigned int, pair<string, string>> >  referencias_sort;
			for(unsigned int i = 0; i < n_ref; ++i){
				length = nextLine(lector, buff, buff_size);
				if( length == 0){
					cerr << "Configuration::loadConfiguration - Error, reference not found.\n";
					return;
				}
				string line_ref(buff);
				stringstream toks_ref(line_ref);
		
				name = "";
				mark = "";
				value = "";
		
				toks_ref >> name;
				toks_ref >> mark;
				toks_ref >> value;
		
				if( (mark.length() != 1) || (mark.compare("=") != 0) || (value.length() < 1) ){
					cerr << "Configuration::loadConfiguration - Error, reference not found.\n";
					return;
				}
				cout << "Configuration::loadConfiguration - Reference \"" << value << "\" for subdir \"" << name << "\"\n";
				referencias_sort.insert(
					pair<unsigned int, pair<string, string>>(
						name.length(),
						pair<string, string>(name, value)
					)
				);
			} // for... lectura de cada referncia
			for( auto it : referencias_sort ){
//				references.push_back( it.second );
				cout << "Configuration::loadConfiguration - Loading Reference \"" << it.second.second << "\"\n";
				ReferenceIndexBasic *ref = new ReferenceIndexBasic();
				ref->load( it.second.second.c_str() );
				references.push_back( pair<string, ReferenceIndexBasic *>(it.second.first, ref) );
			}
		}
		
//		else if( name.compare("reference_file") == 0 ){
//			toks >> value;
//			cout << "Configuration::loadConfiguration - Value \"" << value << "\"\n";
//			char *text = new char[ 1 + value.length() ];
//			strcpy(text, value.c_str());
//			reference_file = text;
//		}
		else if( name.compare("compress_block_size") == 0 ){
//			toks >> value;
			cout << "Configuration::loadConfiguration - Value \"" << value << "\"\n";
			compress_block_size = atoi( value.c_str() );
		}
		else if( name.compare("compress_max_threads") == 0 ){
//			toks >> value;
			cout << "Configuration::loadConfiguration - Value \"" << value << "\"\n";
			compress_max_threads = atoi( value.c_str() );
		}
		else if( name.compare("decompress_line_size") == 0 ){
//			toks >> value;
			cout << "Configuration::loadConfiguration - Value \"" << value << "\"\n";
			decompress_line_size = atoi( value.c_str() );
		}
		else if( name.compare("io_block_size") == 0 ){
//			toks >> value;
			cout << "Configuration::loadConfiguration - Value \"" << value << "\"\n";
			io_block_size = atoi( value.c_str() );
		}
		else{
			cout << "Configuration::loadConfiguration - Omitting value of \"" << name << "\"\n";
		}
	
	}

}








