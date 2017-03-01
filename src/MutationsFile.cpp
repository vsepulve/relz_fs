#include "MutationsFile.h"

// Constructor vacio para agregar mutaciones o cargar archivo despues
MutationsFile::MutationsFile(){
	text_size = 0;
	base_text = NULL;
	n_mut = 0;
	arr_mut = NULL;
}

// El puntero a base_text es compartido
// Asume que el archivo esta en el formato correcto y ordenado
// Reliza un par de verificaciones para asegurarlo (tamaño y orden de posiciones)
// También verifico que el tamaño del texto base sea correcto
MutationsFile::MutationsFile(const char *file, char *_base_text, unsigned int _text_size){
	text_size = 0;
	base_text = NULL;
	n_mut = 0;
	arr_mut = NULL;
	load(file, _base_text, _text_size);
}

MutationsFile::~MutationsFile(){
	if( arr_mut != NULL ){
		delete [] arr_mut;
		arr_mut = NULL;
	}
	text_size = 0;
	base_text = NULL;
	n_mut = 0;
}

bool MutationsFile::load(const char *file, char *_base_text, unsigned int _text_size){
	cout<<"MutationsFile::load - Inicio\n";
	
	// Borrar datos previos
	// Basta con limpiar arr_mut, los demas datos son sobreescritos
	if( arr_mut != NULL ){
		delete [] arr_mut;
		arr_mut = NULL;
	}
	
	fstream lector(file, fstream::binary | fstream::in);
	if(! lector.good() ){
		cerr<<"MutationsFile::load - Error en lectura\n";
		return false;
	}
	
	lector.seekg (0, lector.end);
	unsigned int file_size = lector.tellg();
	lector.seekg (0, lector.beg);
	
	// file_size debe ser: (text_size + n_mut + n_mut * (pos, char))
	
	unsigned int saved_text_size = 0;
	lector.read( (char*)&saved_text_size, sizeof(int) );
	lector.read( (char*)&n_mut, sizeof(int) );
	
	cout<<"MutationsFile::load - saved_text_size: "<<saved_text_size<<", n_mut: "<<n_mut<<"\n";
	
	if( file_size != ( 2*sizeof(int) + n_mut*(sizeof(int) + 1) ) ){
		cerr<<"MutationsFile::load - Error, archivo de tamaño incorrecto\n";
		return false;
	}
	if( saved_text_size != _text_size ){
		cerr<<"MutationsFile::load - Error, text_size incorrecto\n";
		return false;
	}
	
	text_size = _text_size;
	base_text = _base_text;
	
	if( n_mut > 0 ){
		cout<<"MutationsFile::load - Carga Datos\n";
		arr_mut = new pair<unsigned int, char>[n_mut];
		unsigned int pos = 0;
		char c = 0;
		unsigned int last = 0;
		bool error = false;
		for(unsigned int i = 0; i < n_mut; ++i){
			lector.read( (char*)&pos, sizeof(int) );
			lector.read( &c, 1 );
			if(last >= pos){
				cerr<<"MutationsFile::load - Error, posicion no creciente\n";
				error = true;
				break;
			}
			arr_mut[i].first = pos;
			arr_mut[i].second = c;
		}
		if(error){
			n_mut = 0;
			delete [] arr_mut;
			arr_mut = NULL;
		}
	}
	
	cout<<"MutationsFile::load - Carga Terminada\n";
	
	return true;
}// fin load

// Notar que este read siempre es exitoso (aunque puede parecer un archivo vacio)
// Tambien notar que solo escribe size bytes en el buff, SIN AGREGAR un 0 final (para c-string)
// Retorna el numero de bytes escritos en buff
unsigned int MutationsFile::read(char *buff, unsigned int size, unsigned long long offset){
//	cout<<"MutationsFile::read - size: "<<size<<", offset: "<<offset<<", text_size: "<<text_size<<"\n";
	
	if( offset >= text_size || size == 0 || base_text == NULL){
		return 0;
	}
	
	if( offset + size > text_size ){
		size = text_size - offset;
	}
	
	// Primero memcpy del texto base
	// Luego aplico mutaciones
//	cout<<"MutationsFile::read - memcpy(buff, base_text + "<<offset<<", "<<size<<")\n";
	memcpy(buff, base_text + offset, size);
	
	// Notar la tercera condicion, que exista alguna mutacion en rango valido
	if( (n_mut > 0) && (arr_mut != NULL) && (arr_mut[n_mut-1].first >= offset) ){
		// Aplicar mutaciones
//		cout<<"MutationsFile::read - Buscando mutaciones en ["<<offset<<", "<<(offset+size)<<"]\n";
		
		// Buscar binariamente offset en arr
		// Necesito la primera pos >= off
		unsigned int l = 0;
		unsigned int h = n_mut-1;
		unsigned int m;
		while(l < h){
			m = l + ((h-l)>>1);
			if( arr_mut[m].first < offset ){
				l = m+1;
			}
			else{
				h = m;
			}
		}
		// arr[h] >= off
		// Puede que NO haya mutaciones validas, en ese caso h puede ser insficiente
		// Esto PUEDE verificarse en el if anterior a la BB
		while( (h < n_mut) && (arr_mut[h].first < offset + size ) ){
//			cout<<"MutationsFile::read - mutacion["<<h<<"]: ("<<arr_mut[h].first<<", "<<arr_mut[h].second<<")\n";
			buff[ arr_mut[h].first - offset ] = arr_mut[h].second;
			++h;
		}
	}// if... hay mutaciones
	
	return size;
	
}// fin read

// Retorna el tamaño del texto base
unsigned int MutationsFile::size(){
	return text_size;
}


