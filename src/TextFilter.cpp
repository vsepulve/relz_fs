#include "TextFilter.h"

TextFilter::TextFilter(){
}

TextFilter::~TextFilter(){
}
	
// Retorna true para los chars validos para este TextFilter (No filtrados)
// Retoran false para todo char invalido (que sera filtrado)
bool TextFilter::validChar(char c){
	cerr<<"TextFilter::validChar - No Implementado\n";
	return false;
}

// Almacena el alfabeto valido completo en alphabet (si es NULL lo omite)
// Retorna el total de chars validos (incluso si alphabet == NULL)
unsigned int TextFilter::getAlphabet(vector<char> *alphabet){
	cerr<<"TextFilter::getAlphabet - No Implementado\n";
	return 0;
}

// Realiza la lectura y FILTRADO del texto y lo retorna como un c-string
// El llamador debe encargarse de liberar la memoria del texto retornado
// Escribe el largo real del texto en text_length (para ahorrar el strlen)
// Almacena los runs (pares ini, fin) de lowcase solo si lowcase_runs != NULL (de otro modo, lo omite)
char *TextFilter::readText(const char *in_file, unsigned long long &text_length, vector< pair<unsigned long long, unsigned long long> > *lowcase_runs){

	cout<<"TextFilter::readText - Inicio (leyendo \""<<in_file<<"\", lowcase_runs? "<<(lowcase_runs != NULL)<<")\n";
	
	NanoTimer timer;
	fstream lector(in_file, fstream::in);
	if(! lector.good() ){
		cerr<<"TextFilter::readText - Error en lectura\n";
		return NULL;
	}
	// Tomo el largo para cargarlo completo (notar que text_length sera menor a file_size por el filtrado)
	lector.seekg (0, lector.end);
	unsigned long long file_size = lector.tellg();
	lector.seekg (0, lector.beg);
	
//	cout<<"TextFilter::readText - Preparando buffer para archivo de "<<file_size<<" bytes\n";
	//Notar que se cargaran MENOS bytes (por el filtrado)
	char *text = new char[file_size + 1];
	
	text[0] = 0;
	text_length = 0;
	unsigned int max_read = 64*1024;
	char *buff = new char[max_read];
	unsigned long long total_read = 0;
	
	//Datos para runs de minusculas
	bool run = false;
	unsigned long long run_ini = 0;
	
	while( (total_read < file_size) && lector.good() ){
		lector.read(buff, max_read);
		unsigned int lectura = lector.gcount();
		total_read += lectura;
		for(unsigned int j = 0; j < lectura; ++j){
			// Omito chars invalidos y luego construyo lowcase run
			// De este modo, la verificacion de up/low case es independiente del filtrado
			char c = buff[j];
			if( ! validChar( toupper(c) ) ){
				continue;
			}
			if(c < 'a' || c > 'z'){
				// UPCASE
				// if run, cortar
				if(run){
					// El run es de [run_ini, text_length-1]
					if(lowcase_runs != NULL){
						lowcase_runs->push_back(pair<unsigned long long, unsigned long long>(run_ini, text_length-1));
					}
					//cout<<"\"\n";
					//cout<<"run["<<run_ini<<", "<<text_length-1<<"]\n";
					run = false;
				}
				text[text_length++] = c;
			}
			else{
				// LOWCASE
				// if run, sumar
				// else, iniciar
				if(run){
					// continuar run
				}
				else{
					// iniciar
					run_ini = text_length;
					run = true;
					//cout<<"\"";
				}
				//cout<<""<<c<<"";
				text[text_length++] = toupper(c);
			}
		}
	}
	// Si queda un run activo, cerrarlo
	if(run){
		// El run es de [run_ini, text_length-1]
		if(lowcase_runs != NULL){
			lowcase_runs->push_back(pair<unsigned long long, unsigned long long>(run_ini, text_length-1));
		}
		//cout<<"\"\n";
		//cout<<"run["<<run_ini<<", "<<text_length-1<<"]\n";
		run = false;
	}
	
	delete [] buff;
	
//	if(lowcase_runs != NULL){
//		cout<<"TextFilter::readText - lowcase_runs: "<<lowcase_runs->size()<<" pares\n";
//	}
	
	lector.close();
	text[text_length] = 0;
	cout<<"TextFilter::readText - Fin (chars: "<<text_length<<" en "<<timer.getMilisec()<<" ms)\n";
	
	return text;
	
}

unsigned int TextFilter::readReference(const char *in_file, char *text){

	cout<<"TextFilter::readReference - Inicio (leyendo \""<<in_file<<"\")\n";
	
	text[0] = 0;
	unsigned int text_length = 0;
	NanoTimer timer;
	fstream lector(in_file, fstream::in);
	if(! lector.good() ){
		cerr<<"TextFilter::readReference - Error en lectura\n";
		return 0;
	}
	//Tomo el largo para cargarlo completo
	lector.seekg (0, lector.end);
	unsigned int file_size = lector.tellg();
	lector.seekg (0, lector.beg);
	
	unsigned int max_read = 64*1024;
	char *buff = new char[max_read];
	unsigned int total_read = 0;
	
	while( (total_read < file_size) && lector.good() ){
		lector.read(buff, max_read);
		unsigned int lectura = lector.gcount();
		total_read += lectura;
		for(unsigned int j = 0; j < lectura; ++j){
			char c = toupper(buff[j]);
			if( c == 'A' || c == 'C' || c == 'G' || c == 'T'){
				text[text_length++] = c;
			}
		}
	}
	
	delete [] buff;
	lector.close();
	text[text_length] = 0;
	
	cout<<"TextFilter::readReference - Fin (chars: "<<text_length<<" en "<<timer.getMilisec()<<" ms)\n";
	return text_length;
	
}

//Este metodo y lo necesario para su aplicacion, quizas deberia ser estatico.
//En ese caso, headers no necesitaria guardar una instancia, simplemente llamar TextFilter::addNewLines() o algo asi.
unsigned long long TextFilter::filterNewLines(char *text, unsigned long long text_length, vector<unsigned long long> *nl_pos){
	
	if( text == NULL || text_length < 1 ){
		return 0;
	}
	
	// aacc0 0tt0  (9)
	// aacc   tt   (6)
	//     0 0  0  (3)    => <4, 5, 8>
	// aacc0[0tt0] (5, 4) => [5, 9)
	// aacc [ tt ] (4, 2) => [4, 6)
	
	// Notar que con este metodo, es requisito ajustar nl ANTES de ajustar minusculas
	// Es decir, orden inverso para la reconstruccion (de modo que las posiciones se traten en el nivel correcto)
	
	//  1.- convertir abs(ini, fin) en rel(ini, fin)
	//    - rel = abs
	//    - for... cada nl desde 0 hasta abs_fin
	//      - if(nl < abs_ini) --rel_ini
	//      - if(nl < abs_fin) --rel_fin
	//    -- Lo anterior se puede hacer en dos llamadas (para ini desde 0, y para fin desde el resultado anterior)
	//  2.- extraer texto rel(ini, fin)
	//    - read normal (rel_ini, rel_fin)
	//  3.- agregar nl al texto (y reconvertir rel en abs)
	//    - for... cada nl menor a rel_ini
	//      - ++rel_ini, ++rel_fin
	//    - for... cada nl menor o igual a rel_fin
	//      - agregar nl (esto implica mover o copiar en otro buff despues del nl)
	//      - ++rel_fin
	//  Se puede resolver esto con una estructura de rank/select ?
	//  Por ahora lo dejo en version descomprimida y directa, pero notando eso.
	
	unsigned int mov = 0;
	for(unsigned long long i = 0; i < text_length; ++i){
		if( text[i] == '\n' ){
			// NL encontrado, agregar pos absoluta
			if( nl_pos != NULL ){
				nl_pos->push_back(i);
//				cout<<"TextFilter::filterNewLines - "<<i<<"\n";
			}
			++mov;
		}
		else{
			// Mover char
			text[ i - mov ] = text[i];
		}
	}//for... cada char
	//Cortar la cola del string
	text[ text_length - mov ] = 0;
	return (text_length - mov);
}























