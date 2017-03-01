#include "ReferenceIndexRR.h"

ReferenceIndexRR::ReferenceIndexRR(){
	arr = NULL;
	largo = 0;
	largo_arr = 0;
}

ReferenceIndexRR::ReferenceIndexRR(const char *ref_file_rr){
	arr = NULL;
	largo = 0;
	largo_arr = 0;
	load(ref_file_rr);
}

ReferenceIndexRR::ReferenceIndexRR(const char *ref_file, unsigned int distancia){
	arr = NULL;
	largo = 0;
	largo_arr = 0;

	fstream lector(ref_file, fstream::binary | fstream::in);
	
	if( ! lector.good() ){
		cerr<<"ReferenceIndexRR - Error al abrir archivo \""<<ref_file<<"\"\n";
		return;
	}
	
	cout<<"ReferenceIndexRR - Construyendo RR desde \""<<ref_file<<"\", dist: "<<distancia<<"\n";
	
	//Lectura del texto
	largo = 0;
	lector.read((char*)(&largo), sizeof(int));
	cout<<"ReferenceIndexRR - Cargando texto de "<<largo<<" chars\n";
	
	ref = new unsigned char[largo + 1];
	lector.read((char*)ref, largo);
	ref[largo] = 0;
	
	unsigned int protected_pos = 1024 + 256;
	
	//Preparacion de arr
	largo_arr = (largo-protected_pos) / distancia;
	if(distancia * largo_arr < (largo-protected_pos) ){
		++largo_arr;
	}
	largo_arr += protected_pos;
	
	cout<<"ReferenceIndexRR - Escogiendo "<<largo_arr<<" posiciones del SA (protected_pos: "<<protected_pos<<")\n";
	arr = new unsigned int[largo_arr];
	
	//Llenado de arr
	
	//Segmento protegido
	for(unsigned int i = 0; i < protected_pos; ++i){
		lector.read((char*)(arr + i), sizeof(int));
	}
	
	//v1: RR directo
	for(unsigned int i = protected_pos; i < largo_arr; ++i){
		lector.read((char*)(arr + i), sizeof(int));
		if(distancia > 1){
			lector.ignore( (distancia - 1) * sizeof(int) );
		}
	}
	
	//v2: al azar (con set temporal para posiciones seleccionadas)
	/*
	set<unsigned int> posiciones_tomadas;
	set<unsigned int>::iterator it_pos;
	unsigned int pos = 0;
	for(unsigned int i = 0; i < largo_arr; ++i){
		pos = rand() % largo;
		while( posiciones_tomadas.find(pos) != posiciones_tomadas.end() ){
			pos = rand() % largo;
		}
		posiciones_tomadas.insert(pos);
	}
	pos = 0;
	unsigned int *arr_pos = arr;
	for(it_pos = posiciones_tomadas.begin(); it_pos != posiciones_tomadas.end(); it_pos++){
		if( pos < *it_pos ){
			lector.ignore( (*it_pos - pos) * sizeof(int) );
			pos = *it_pos;
		}
		lector.read((char*)(arr_pos), sizeof(int));
		++arr_pos;
		++pos;
	}
	posiciones_tomadas.clear();
	*/
	
	lector.close();

}

ReferenceIndexRR::~ReferenceIndexRR(){
	if(ref != NULL){
		delete [] ref;
		ref = NULL;
	}
	if(arr != NULL){
		delete [] arr;
		arr = NULL;
	}
}

//Busca el "text" en el arreglo de sufijos
//Guarda la posicio y el largo del mayor prefijo comun
void ReferenceIndexRR::find(const char *text, unsigned int size, unsigned int &position, unsigned int &length) const{
	position = 0;
	length = 0;
	
//	cout<<"ReferenceIndexRR::find - inicio (largo "<<size<<", text: \""<<string(text, size)<<"\")\n";
	
	//Notar que es trivial preparar limites ajustados para, por ejemplo, las primeras 2 letras de la query
	//Eso seria equivalente a evitar las 2 primeras iteraciones del for (las mas largas, obviamente)
	
	unsigned int limite_izq = 0;
	unsigned int limite_der = largo_arr - 1;
	unsigned int utimo_comun = 0xffffffff;
	
	for(unsigned int cur_pos = 0; cur_pos < size; ++cur_pos){
//		cout<<"ReferenceIndexRR::find - cur_pos: "<<cur_pos<<" ("<<limite_izq<<", "<<limite_der<<")\n";
		//se busca el caracter text[cur_pos] en la posicion cur_pos de los sufijos
		
		//buscar limite izq

//		//version lineal
//		unsigned int izq = limite_izq;
//		while( (izq < largo_arr) && (
//			arr[izq] + cur_pos > largo || 
//			*(ref + arr[izq] + cur_pos) < text[cur_pos]
//			) ){
//			++izq;
//		}

		//Version binaria
		unsigned int l = limite_izq;
		unsigned int h = limite_der;
		unsigned int m;
		while(l < h){
			m = l + ((h-l)>>1);
			if( arr[m] + cur_pos > largo || *(ref + arr[m] + cur_pos) < (unsigned char)(*(text + cur_pos)) ){
				l = m+1;
			}
			else{
				h = m;
			}
		}
		unsigned int izq = h;
		
//		cout<<"ReferenceIndexRR::find - izq: "<<izq<<"\n";
		
		//buscar limite der
		
//		//version lineal
//		unsigned int der = limite_der;
//		while( (der > 0) && (arr[der] + cur_pos < largo) &&
//			*(ref + arr[der] + cur_pos) > text[cur_pos] 
//			){
//			--der;
//		}
		
		//Version binaria
		l = limite_izq;
		h = limite_der;
		while(l < h){
			m = l + ((h-l)>>1);
			if( arr[m] + cur_pos > largo || *(ref + arr[m] + cur_pos) <= (unsigned char)(*(text + cur_pos)) ){
				l = m+1;
			}
			else{
				h = m;
			}
		}
		unsigned int der = h;
		if( (arr[der] + cur_pos < largo) 
			&& *(ref + arr[der] + cur_pos) > (unsigned char)(*(text + cur_pos)) ){
			--der;
		}
		
//		cout<<"ReferenceIndexRR::find - der: "<<der<<"\n";
		
		//si limite izq > der, salir y usar el ultimo rango valido
		if(izq == der){
			limite_izq = izq;
			limite_der = der;
			//Extension de la cola (en el caso izq == der)
			while( (cur_pos < size) 
				&& (arr[limite_izq] + cur_pos < largo) 
				&& *(ref + arr[limite_izq] + cur_pos) == (unsigned char)(*(text + cur_pos)) ){
				utimo_comun = cur_pos;
				++cur_pos;
			}
			break;
		}
		else if(izq < der){
			limite_izq = izq;
			limite_der = der;
			utimo_comun = cur_pos;
		}
		else{
			break;
		}
		
	}
	
//	cout<<"ReferenceIndexRR::find - limite_izq: "<<limite_izq<<"\n";
//	cout<<"ReferenceIndexRR::find - limite_der: "<<limite_der<<"\n";
//	cout<<"ReferenceIndexRR::find - ultimo char comun: "<<utimo_comun<<"\n";
	
	if(utimo_comun != 0xffffffff){
		position = arr[limite_izq];
		length = 1 + utimo_comun;
//		cout<<"ReferenceIndexRR::find - Prefijo mayor: \""<<(string((char*)ref + position, length))<<"\"\n";
	}
	
}

//Metodos de save para carga sin construccion
void ReferenceIndexRR::save(const char *ref_file){
	cout<<"ReferenceIndexRR::save - guardando en \""<<ref_file<<"\"\n";
	fstream escritor(ref_file, fstream::trunc | fstream::binary | fstream::out);
	escritor.write((char*)(&largo), sizeof(int));
	escritor.write((char*)(&largo_arr), sizeof(int));
	
	escritor.write((char*)ref, largo);
	escritor.write((char*)arr, largo_arr * sizeof(int));
	
	escritor.close();
	cout<<"ReferenceIndexRR::save - fin\n";
}

//Metodos de carga sin construccion
void ReferenceIndexRR::load(const char *ref_file){
	if(arr != NULL){
		delete [] arr;
		arr = NULL;
	}
	
	fstream lector(ref_file, fstream::binary | fstream::in);
	lector.read((char*)(&largo), sizeof(int));
	lector.read((char*)(&largo_arr), sizeof(int));
	cout<<"ReferenceIndexRR::load - cargando "<<largo<<" chars y "<<largo_arr<<" ints desde \""<<ref_file<<"\"\n";
	
	ref = new unsigned char[largo+1];
	lector.read((char*)ref, largo);
	ref[largo] = 0;
	
	arr = new unsigned int[largo_arr];
	lector.read((char*)arr, largo_arr * sizeof(int));
	
	lector.close();

}










