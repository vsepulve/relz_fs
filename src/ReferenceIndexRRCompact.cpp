#include "ReferenceIndexRRCompact.h"

ReferenceIndexRRCompact::ReferenceIndexRRCompact(){
	texto = NULL;
	arr = NULL;
	largo_arr = 0;
}

ReferenceIndexRRCompact::ReferenceIndexRRCompact(const char *ref_file_rr){
	texto = NULL;
	arr = NULL;
	largo_arr = 0;
	load(ref_file_rr);
}

ReferenceIndexRRCompact::ReferenceIndexRRCompact(const char *ref_file, unsigned int distancia){

	fstream lector(ref_file, fstream::binary | fstream::in);
	
	//Lectura del texto
	unsigned int largo = 0;
	lector.read((char*)(&largo), sizeof(int));
	char *ref = new char[largo + 1];
	memset(ref, 0, largo + 1);
	lector.read((char*)ref, largo);
	
	texto = new CompactSequence(ref, largo);
	
	delete [] ref;
	
	//Preparacion de arr
	largo_arr = largo / distancia;
	if(distancia * largo_arr < largo){
		++largo_arr;
	}
	arr = new unsigned int[largo_arr];
	
	//Llenado de arr
	
	//v1: RR directo
	for(unsigned int i = 0; i < largo_arr; ++i){
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

ReferenceIndexRRCompact::~ReferenceIndexRRCompact(){
	if(texto != NULL){
		delete texto;
		texto = NULL;
	}
	if(arr != NULL){
		delete [] arr;
		arr = NULL;
	}
}

//Busca el "text" en el arreglo de sufijos
//Guarda la posicio y el largo del mayor prefijo comun
void ReferenceIndexRRCompact::find(const char *text, unsigned int size, unsigned int &position, unsigned int &length) const{
//	cout<<"ReferenceIndexRRCompact::find - inicio (largo "<<size<<")\n";
	position = 0;
	length = 0;
	
	//Notar que es trivial preparar limites ajustados para, por ejemplo, las primeras 2 letras de la query
	//Eso seria equivalente a evitar las 2 primeras iteraciones del for (las mas largas, obviamente)
	
	unsigned int limite_izq = 0;
	unsigned int limite_der = largo_arr - 1;
	unsigned int utimo_comun = 0xffffffff;
	
	for(unsigned int cur_pos = 0; cur_pos < size; ++cur_pos){
//		cout<<"ReferenceIndexRRCompact::find - cur_pos: "<<cur_pos<<" ("<<limite_izq<<", "<<limite_der<<")\n";
		//se busca el caracter text[cur_pos] en la posicion cur_pos de los sufijos
		
		//buscar limite izq

//		//version lineal
//		unsigned int izq = limite_izq;
//		while( (izq < largo) && (&(ref[ arr[izq] ]))[cur_pos] < text[cur_pos] ){
//			++izq;
//		}

		//Version binaria
		unsigned int l = limite_izq;
		unsigned int h = limite_der;
		unsigned int m;
		while(l < h){
			m = l + ((h-l)>>1);
//			if( (&(ref[ arr[m] ]))[cur_pos] < text[cur_pos] ){
//			if( arr[m] + cur_pos > largo || *(ref + arr[m] + cur_pos) < *(text + cur_pos) ){
			if( texto->getChar(arr[m] + cur_pos) < *(text + cur_pos) ){
				l = m+1;
			}
			else{
				h = m;
			}
		}
		unsigned int izq = h;
		
//		cout<<"ReferenceIndexRRCompact::find - izq: "<<izq<<"\n";
		
		//buscar limite der
		
//		//version lineal
//		unsigned int der = limite_der;
//		while( (der > 0) && (&(ref[ arr[der] ]))[cur_pos] > text[cur_pos] ){
//			--der;
//		}
		
		//Version binaria
		l = limite_izq;
		h = limite_der;
		while(l < h){
			m = l + ((h-l)>>1);
//			if( (&(ref[ arr[m] ]))[cur_pos] <= text[cur_pos] ){
//			if( arr[m] + cur_pos > largo || *(ref + arr[m] + cur_pos) <= *(text + cur_pos) ){
			if( texto->getChar(arr[m] + cur_pos) <= *(text + cur_pos) ){
				l = m+1;
			}
			else{
				h = m;
			}
		}
		unsigned int der = h;
//		if( (&(ref[ arr[der] ]))[cur_pos] > text[cur_pos] ){
//		if( *(ref + arr[der] + cur_pos) > *(text + cur_pos) ){
		if( texto->getChar(arr[der] + cur_pos) > *(text + cur_pos) ){
			--der;
		}
		
//		cout<<"ReferenceIndexRRCompact::find - der: "<<der<<"\n";
		
		//si limite izq > der, salir y usar el ultimo rango valido
		if(izq == der){
			limite_izq = izq;
			limite_der = der;
			//Extension de la cola (en el caso izq == der)
//			while( (cur_pos < size) && (&(ref[ arr[limite_izq] ]))[cur_pos] == text[cur_pos] ){
				
//			while( (cur_pos < size) 
//				&& (arr[limite_izq] + cur_pos < largo) 
//				&& *(ref + arr[limite_izq] + cur_pos) == *(text + cur_pos) ){
				
			while( (cur_pos < size) && texto->getChar(arr[limite_izq] + cur_pos) == *(text + cur_pos) ){
				
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
	
//	cout<<"ReferenceIndexRRCompact::find - limite_izq: "<<limite_izq<<"\n";
//	cout<<"ReferenceIndexRRCompact::find - limite_der: "<<limite_der<<"\n";
//	cout<<"ReferenceIndexRRCompact::find - ultimo char comun: "<<utimo_comun<<"\n";
	
	if(utimo_comun != 0xffffffff){
		
		//arr_uso[ arr[limite_izq] ]++;
		
		position = arr[limite_izq];
		length = 1 + utimo_comun;
//		cout<<"ReferenceIndexRRCompact::find - Prefijo mayor: \""<<(string(&(ref[position]), length))<<"\"\n";
	}
	
}

	
//Metodos de save para carga sin construccion
void ReferenceIndexRRCompact::save(const char *ref_file){
	cout<<"ReferenceIndexRRCompact::save - guardando en \""<<ref_file<<"\"\n";
	fstream escritor(ref_file, fstream::trunc | fstream::binary | fstream::out);
	escritor.write((char*)(&largo_arr), sizeof(int));
	escritor.write((char*)arr, largo_arr * sizeof(int));
	texto->save(&escritor);
	escritor.close();
	cout<<"ReferenceIndexRRCompact::save - fin\n";
}

//Metodos de carga sin construccion
void ReferenceIndexRRCompact::load(const char *ref_file){
	
	if(texto != NULL){
		delete texto;
		texto = NULL;
	}
	if(arr != NULL){
		delete [] arr;
		arr = NULL;
	}
	
	fstream lector(ref_file, fstream::binary | fstream::in);
	lector.read((char*)(&largo_arr), sizeof(int));
	arr = new unsigned int[largo_arr];
	lector.read((char*)arr, largo_arr * sizeof(int));
	texto = new CompactSequence();
	texto->load(&lector);
	lector.close();

}










