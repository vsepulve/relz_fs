#include "ReferenceIndexBasic.h"
	
//funcion (global por el momento) para threads
void f_sort(unsigned int id, unsigned int **shared_arrs, unsigned int *n_largos, unsigned int n_arrs, mutex *shared_mutex, unsigned int *shared_pos, SAComparatorN3 *shared_comp){
	
//	shared_mutex->lock();
//	cout<<"Thread ["<<id<<"] - Inicio\n";
//	shared_mutex->unlock();
	
	NanoTimer timer;
	unsigned int pos_local = 0;
	unsigned int partes_procesadas = 0;
	while(true){
		shared_mutex->lock();
		pos_local = *shared_pos;
		++(*shared_pos);
//		cout<<"Thread ["<<id<<"] - Procesando bucket "<<pos_local<<"\n";
		shared_mutex->unlock();
		if(pos_local >= n_arrs){
//			shared_mutex->lock();
//			cout<<"Thread ["<<id<<"] - Saliendo\n";
//			shared_mutex->unlock();
			break;
		}
		//Solo es necesario ordenar buckets de 2 o mas elementos
		if( n_largos[pos_local] > 1 && shared_arrs[pos_local] != NULL){
//			shared_mutex->lock();
//			cout<<"Thread ["<<id<<"] - arr largo "<<n_largos[pos_local]<<"\n";
//			shared_mutex->unlock();
//			sort(shared_arrs[pos_local], shared_arrs[pos_local]+n_largos[pos_local], *shared_comp);
			stable_sort(shared_arrs[pos_local], shared_arrs[pos_local]+n_largos[pos_local], *shared_comp);
			++partes_procesadas;
		}
	}
	
	shared_mutex->lock();
	cout<<"Thread ["<<id<<"] - Fin ("<<partes_procesadas<<" partes procesadas en "<<timer.getMilisec()<<" ms)\n";
	shared_mutex->unlock();
	
	return;
}

ReferenceIndexBasic::ReferenceIndexBasic(){
	ref = NULL;
	arr = NULL;
	largo = 0;
}
	
ReferenceIndexBasic::ReferenceIndexBasic(const char *_referencia, unsigned int n_threads){
	
	cout<<"ReferenceIndexBasic - inicio\n";
	
	largo = strlen(_referencia);
	arr = new unsigned int[largo];
	
	ref = new unsigned char[largo + 1];
	memset(ref, 0, largo + 1);
	sprintf((char*)ref, "%s", _referencia);
	
//	cout<<"ReferenceIndexBasic - Texto: \""<<(char*)ref<<"\"\n";
	
	for(unsigned int i = 0; i < largo; ++i){
		arr[i] = i;
//		cout<<"arr["<<i<<"]: "<<arr[i]<<" (\""<<&(ref[ arr[i] ])<<"\", largo "<<largo-arr[i]<<", char[0]: "<<(unsigned int)(ref[ arr[i]])<<")\n";
	}
	cout<<"ReferenceIndexBasic - arr de largo "<<largo<<"\n";
	
	//V4: 3 nivel de bucket (125) y sort
	//uso un par de atajos para acelerar el codigo
	
	cout<<"ReferenceIndexBasic - preparando datos\n";
	
	unsigned int ***index = new unsigned int**[256];
	for(unsigned int i = 0; i < 256; ++i){
		index[i] = new unsigned int*[256];
		for(unsigned int j = 0; j < 256; ++j){
			index[i][j] = new unsigned int[256];
			memset(index[i][j], 0, 256*sizeof(int));
		}
	}
	
	cout<<"ReferenceIndexBasic - preparando vocabulario\n";
	
	//En lugar de fijar el voc aca, uso todo el voc de la referencia
	//Notar que esto SIEMPRE omitira el char '\0' pues se uso strlen
//	vector<unsigned char> vocabulary = {'A', 'C', 'G', 'N', 'T'};

	set<unsigned char> voc_usado;
	set<unsigned char>::iterator it_voc;
	for(unsigned int i = 0; i < largo; ++i){
		voc_usado.insert(ref[i]);
	}
	vector<unsigned char> vocabulary;
	for(it_voc = voc_usado.begin(); it_voc != voc_usado.end(); it_voc++){
		if(*it_voc != 0){
//			cout<<"ReferenceIndexBasic - Agregando \'"<<(*it_voc)<<"\' ("<<(unsigned int)(*it_voc)<<")\n";
			vocabulary.push_back(*it_voc);
		}
	}
	
	cout<<"ReferenceIndexBasic - preparando buckets\n";
	
	unsigned int n_buckets = vocabulary.size() * vocabulary.size() * vocabulary.size();
	unsigned int contador = 0;
	for(unsigned int i = 0; i < vocabulary.size(); ++i){
		for(unsigned int j = 0; j < vocabulary.size(); ++j){
			for(unsigned int k = 0; k < vocabulary.size(); ++k){
//				cout<<"{"<<vocabulary[i]<<", "<<vocabulary[j]<<", "<<vocabulary[k]<<"} : "<<contador<<"\n";
				index[ vocabulary[i] ][ vocabulary[j] ][ vocabulary[k] ] = contador++;
			}
		}
	}
	
	cout<<"ReferenceIndexBasic - contando ("<<n_buckets<<" buckets)\n";
	unsigned int *n = new unsigned int[n_buckets];
	memset(n, 0, n_buckets * sizeof(int));
	
	//Casos de Borde
	//El caso de la ultima letra en ref[largo-1] debe ser tratado igual al primer par
	//ref[largo-1]['A'], la primera letra del vocabulario
	++(n[ index[ ref[largo-2] ][ ref[largo-1] ][ vocabulary[0] ] ]);
	++(n[ index[ ref[largo-1] ][ vocabulary[0] ][ vocabulary[0] ] ]);
	for(unsigned int i = 0; i < largo-2; ++i){
//		cout<<" n["<<ref[i]<<"]["<<ref[i+1]<<"]["<<ref[i+2]<<"]++ ("<<index[ref[i]][ref[i+1]][ref[i+2]]<<")\n";
		++(n[ index[ref[i]][ref[i+1]][ref[i+2]] ]);
	}
	
	cout<<"ReferenceIndexBasic - creando buckets\n";
	unsigned int **bucket = new unsigned int *[n_buckets];
	for(unsigned int i = 0; i < n_buckets; ++i){
		if( n[i] > 0){
			
//			//Convierto posicion en tripleta que lo genero para confirmar resultados
//			unsigned int p1 = i / (vocabulary.size() * vocabulary.size());
//			unsigned int imod = i % (vocabulary.size() * vocabulary.size());
//			unsigned int p2 = imod / vocabulary.size();
//			unsigned int p3 = imod % vocabulary.size();
//			cout<<"Bucket ["<<i<<"]: "<<n[i]<<" elements (["<<vocabulary[p1]<<"]["<<vocabulary[p2]<<"]["<<vocabulary[p3]<<"])\n";
			
			bucket[i] = new unsigned int[n[i]];
		}
		else{
			bucket[i] = NULL;
		}
	}
	
	cout<<"ReferenceIndexBasic - llenando buckets\n";
	unsigned int *pos = new unsigned int[n_buckets];
	memset(pos, 0, n_buckets * sizeof(int));
	
	//Agrego el ultimo sufijo como el primero de su bucket, mismo caso de borde anterior
	bucket[ index[ ref[largo-2] ][ ref[largo-1] ][ vocabulary[0] ] ][ pos[ index[ ref[largo-2] ][ ref[largo-1] ][ vocabulary[0] ] ]++ ] = largo-2;
	bucket[ index[ ref[largo-1] ][ vocabulary[0] ][ vocabulary[0] ] ][ pos[ index[ ref[largo-1] ][ vocabulary[0] ][ vocabulary[0] ] ]++ ] = largo-1;
	for(unsigned int i = 0; i < largo-2; ++i){
		bucket[ index[ref[i]][ref[i+1]][ref[i+2]] ][ pos[ index[ref[i]][ref[i+1]][ref[i+2]] ]++ ] = i;
	}
	
	cout<<"ReferenceIndexBasic - Ordenando buckets\n";
	SAComparatorN3 comp(ref, (unsigned int)largo);

	//Version Secuencial
//	for(unsigned int i = 0; i < n_buckets; ++i){
//		sort(bucket[i], bucket[i]+pos[i], comp);
//	}
	
	//Version paralela
	vector<std::thread> sort_threads;
	mutex shared_mutex;
	unsigned int shared_pos = 0;
	
	for(unsigned int i = 0; i < n_threads; ++i){
		sort_threads.push_back( std::thread(f_sort, i, bucket, pos, n_buckets, &shared_mutex, &shared_pos, &comp) );
	}
	for(unsigned int i = 0; i < n_threads; ++i){
		sort_threads[i].join();
	}
	
	//Copia de resultados al arreglo final
	//Notar que esto SE PUEDE hacer en los threads
	//Solo lo dejo asi por legibilidad
	unsigned int pos_arr = 0;
	for(unsigned int i = 0; i < n_buckets; ++i){
		for(unsigned int j = 0; j < pos[i]; ++j){
			arr[pos_arr++] = bucket[i][j];
		}
	}
	
	//Borrado
	if( index != NULL ){
		for(unsigned int i = 0; i < 256; ++i){
			if(index[i] != NULL){
				for(unsigned int j = 0; j < 256; ++j){
					if( index[i][j] != NULL ){
						delete [] index[i][j];
					}
				}
				delete [] index[i];
			}
		}
		delete [] index;
	}
	if( bucket != NULL ){
		for(unsigned int i = 0; i < n_buckets; ++i){
			if( bucket[i] != NULL){
				delete [] bucket[i];
			}
		}
		delete [] bucket;
	}
	if( pos != NULL ){
		delete [] pos;
	}
	if( n != NULL ){
		delete [] n;
	}
	
//	for(unsigned int i = 0; i < largo; ++i){
//		cout<<"arr["<<i<<"]: "<<arr[i]<<" (\""<<&(ref[ arr[i] ])<<"\", largo "<<largo-arr[i]<<", char[0]: "<<(unsigned int)(ref[ arr[i]])<<")\n";
//	}
	
	cout<<"ReferenceIndexBasic - fin\n";
}

ReferenceIndexBasic::~ReferenceIndexBasic(){
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
void ReferenceIndexBasic::find(const char *text, unsigned int size, unsigned int &position, unsigned int &length) const{
	position = 0;
	length = 0;
	
//	cout<<"ReferenceIndexBasic::find - inicio (largo "<<size<<", text: \""<<string(text, size)<<"\")\n";
	
	//Notar que es trivial preparar limites ajustados para, por ejemplo, las primeras 2 letras de la query
	//Eso seria equivalente a evitar las 2 primeras iteraciones del for (las mas largas, obviamente)
	
	unsigned int limite_izq = 0;
	unsigned int limite_der = largo-1;
	unsigned int utimo_comun = 0xffffffff;
	
	for(unsigned int cur_pos = 0; cur_pos < size; ++cur_pos){
//		cout<<"ReferenceIndexBasic::find - cur_pos: "<<cur_pos<<" ("<<limite_izq<<", "<<limite_der<<")\n";
		//se busca el caracter text[cur_pos] en la posicion cur_pos de los sufijos
		
		//buscar limite izq

//		//version lineal
//		unsigned int izq = limite_izq;
//		while( (izq < largo) && (
//			arr[izq] + cur_pos >= largo || 
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
		// Ajuste adicional (esto es porque h no puede ser mayor a limite_der para romper igualdad)
		if( (arr[izq] + cur_pos < largo) 
			&& *(ref + arr[izq] + cur_pos) < (unsigned char)(*(text + cur_pos)) ){
			++izq;
		}
		
//		cout<<"ReferenceIndexBasic::find - izq: "<<izq<<"\n";
		
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
		
//		cout<<"ReferenceIndexBasic::find - der: "<<der<<"\n";
		
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
	
//	cout<<"ReferenceIndexBasic::find - limite_izq: "<<limite_izq<<"\n";
//	cout<<"ReferenceIndexBasic::find - limite_der: "<<limite_der<<"\n";
//	cout<<"ReferenceIndexBasic::find - ultimo char comun: "<<utimo_comun<<"\n";
	
	if(utimo_comun != 0xffffffff){
		position = arr[limite_izq];
		length = 1 + utimo_comun;
//		cout<<"ReferenceIndexBasic::find - Prefijo mayor: \""<<(string((char*)ref + position, length))<<"\"\n";
	}
	
}

// Similar al anterior, pero solo permite sufijos en un rango determinado (de ref, no de arr)
// Tambien considera un cierto min_length para considerar las busquedas validas
// Si no encuentra un sufijo comun de largo min_length o mas, omite el resultado
// Mientras busca el sufijo, comienza a probar el rango valido solo si ya tiene min_length o mas encontrado
void ReferenceIndexBasic::find(const char *text, unsigned int size, unsigned int &position, unsigned int &length, unsigned int min_length, bool *arr_valid) const{
	
	position = 0;
	length = 0;
	
//	cout<<"ReferenceIndexBasic::find - inicio (largo "<<size<<", text: \""<<string(text, size)<<"\", min_length: "<<min_length<<")\n";
//	cout<<"ReferenceIndexBasic::find - inicio (largo "<<size<<", min_length: "<<min_length<<")\n";
	
//	cout<<"ReferenceIndexBasic::find - arr_valid: ";
//	for(unsigned int i = 0; i < largo; ++i){
//		if( arr_valid[i] ){
//			cout<<"1";
//		}
//		else{
//			cout<<"0";
//		}
//	}
//	cout<<"\n";
	
	//Notar que es trivial preparar limites ajustados para, por ejemplo, las primeras 2 letras de la query
	//Eso seria equivalente a evitar las 2 primeras iteraciones del for (las mas largas, obviamente)
	
	unsigned int limite_izq = 0;
	unsigned int limite_der = largo-1;
	unsigned int utimo_comun = 0xffffffff;
	
	for(unsigned int cur_pos = 0; cur_pos < size; ++cur_pos){
//		cout<<"ReferenceIndexBasic::find - cur_pos: "<<cur_pos<<" ("<<limite_izq<<", "<<limite_der<<"), utimo_comun: "<<utimo_comun<<"\n";
		//se busca el caracter text[cur_pos] en la posicion cur_pos de los sufijos
		
		//buscar limite izq

//		//version lineal
//		unsigned int izq = limite_izq;
//		while( (izq < largo) && (
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
		// Ajuste adicional (esto es porque h no puede ser mayor a limite_der para romper igualdad)
		if( (arr[izq] + cur_pos < largo) 
			&& *(ref + arr[izq] + cur_pos) < (unsigned char)(*(text + cur_pos)) ){
			++izq;
		}
		
//		cout<<"ReferenceIndexBasic::find - izq: "<<izq<<"\n";
		
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
		
//		cout<<"ReferenceIndexBasic::find - der: "<<der<<"\n";
		
		//Verificacion de rango
		if( cur_pos >= min_length-1 ){
//			cout<<"ReferenceIndexBasic::find - verificando range\n";
			//Basta con que haya alguna posicion valida para seguir (solo paro si no quedan)
			bool valid = false;
			for(unsigned int i = izq; i <= der; ++i){
				//(arr[i] + cur_pos) debe estar en rango valido
				if( arr_valid[ (arr[i] + cur_pos) ] ){
					valid = true;
					break;
				}
			}
			if(! valid){
				break;
			}
		}
		
		//si limite izq > der, salir y usar el ultimo rango valido
		if(izq == der){
			limite_izq = izq;
			limite_der = der;
			//Extension de la cola (en el caso izq == der)
			while( (cur_pos < size) 
				&& (arr[limite_izq] + cur_pos < largo) 
				&& arr_valid[ (arr[limite_izq] + cur_pos) ] 
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
	
//	cout<<"ReferenceIndexBasic::find - limite_izq: "<<limite_izq<<"\n";
//	cout<<"ReferenceIndexBasic::find - limite_der: "<<limite_der<<"\n";
//	cout<<"ReferenceIndexBasic::find - ultimo char comun: "<<utimo_comun<<" (de "<<min_length<<")\n";
	
	if( utimo_comun != 0xffffffff && utimo_comun + 1 >= min_length ){
		position = arr[limite_izq];
		length = 1 + utimo_comun;
//		cout<<"ReferenceIndexBasic::find - Prefijo mayor: \""<<(string((char*)ref + position, length))<<"\"\n";
	}
	
}


	
//Metodos de save para carga sin construccion
void ReferenceIndexBasic::save(const char *ref_file){
	fstream escritor(ref_file, fstream::trunc | fstream::binary | fstream::out);
	if( ! escritor.good() ){
		cerr<<"ReferenceIndexBasic::save - Error al abrir archivo\n";
		return;
	}
	escritor.write((char*)(&largo), sizeof(int));
	escritor.write((char*)ref, largo);
	escritor.write((char*)arr, largo * sizeof(int));
	escritor.close();
}

//Metodos de carga sin construccion
void ReferenceIndexBasic::load(const char *ref_file){
	if(ref != NULL){
		delete [] ref;
		ref = NULL;
	}
	if(arr != NULL){
		delete [] arr;
		arr = NULL;
	}
	largo = 0;
	
	fstream lector(ref_file, fstream::binary | fstream::in);
	if( ! lector.good() ){
		cerr<<"ReferenceIndexBasic::load - Error al abrir archivo\n";
		return;
	}
	
	lector.read((char*)(&largo), sizeof(int));
	cout<<"ReferenceIndexBasic::load - cargando referencia de "<<largo<<" chars desde \""<<ref_file<<"\"\n";
	
	ref = new unsigned char[largo + 1];
	lector.read((char*)ref, largo);
	ref[largo] = 0;
	arr = new unsigned int[largo];
	
//	cout<<"ReferenceIndexBasic::load - Texto: \""<<(char*)ref<<"\"\n";
	
	lector.read((char*)arr, largo * sizeof(int));
	lector.close();
	
//	for(unsigned int i = 0; i < largo; ++i){
//		cout<<"arr["<<i<<"]: "<<arr[i]<<" (\""<<&(ref[ arr[i] ])<<"\", largo "<<largo-arr[i]<<", char[0]: "<<(unsigned int)(ref[ arr[i]])<<")\n";
//	}
	
//	cout<<"ReferenceIndexBasic::load - Carga Terminada\n";

}

// Notar que este metodo es static
char *ReferenceIndexBasic::loadText(const char *ref_file){
	char *text = NULL;
	unsigned int text_size = 0;
	
	fstream reader(ref_file, fstream::binary | fstream::in);
	if( ! reader.good() ){
		cerr<<"ReferenceIndexBasic::loadText - Error al abrir archivo.\n";
		return NULL;
	}
	
	reader.read((char*)(&text_size), sizeof(int));
	cout<<"ReferenceIndexBasic::loadText - cargando referencia de "<<text_size<<" chars desde \""<<ref_file<<"\"\n";
	
	text = new char[text_size + 1];
	reader.read(text, text_size);
	text[text_size] = 0;
	
	reader.close();
	
	return text;
}

void ReferenceIndexBasic::search(const char *text, unsigned int size, vector<unsigned int> &res) const{

	cout<<"ReferenceIndexBasic::search - Inicio (text \""<<text<<"\")\n";
	
	unsigned int limite_izq = 0;
	unsigned int limite_der = largo-1;
	
	for(unsigned int cur_pos = 0; cur_pos < size; ++cur_pos){
//		cout<<"ReferenceIndexBasic::search - cur_pos: "<<cur_pos<<" (\""<<(text + cur_pos)<<"\", "<<limite_izq<<", "<<limite_der<<")\n";
		// Se busca el caracter text[cur_pos] en la posicion cur_pos de los sufijos
		
		// buscar limite izq

//		// version lineal
//		unsigned int izq = limite_izq;
//		while( (izq < largo) && (
//			arr[izq] + cur_pos >= largo || 
//			*(ref + arr[izq] + cur_pos) < text[cur_pos]
//			) ){
//			++izq;
//		}
		
		// Version binaria
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
		// Ajuste adicional (esto es porque h no puede ser mayor a limite_der para romper igualdad)
		if( (arr[izq] + cur_pos < largo) 
			&& *(ref + arr[izq] + cur_pos) < (unsigned char)(*(text + cur_pos)) ){
			++izq;
		}
		
//		cout<<"ReferenceIndexBasic::search - izq: "<<izq<<"\n";
		
		// Buscar limite der
		
//		// Version lineal
//		unsigned int der = limite_der;
//		while( (der > 0) && (arr[der] + cur_pos < largo) &&
//			*(ref + arr[der] + cur_pos) > text[cur_pos] 
//			){
//			--der;
//		}
		
		// Version binaria
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
		
//		cout<<"ReferenceIndexBasic::search - der: "<<der<<"\n";
		
		limite_izq = izq;
		limite_der = der;
		
		if(izq > der){
//			cout<<"ReferenceIndexBasic::search - Sin resultados validos, cancelando\n";
			break;
		}
		
	}//for... cur_pos
	
	while(limite_izq <= limite_der){
//		cout<<"ReferenceIndexBasic::search - Agregando res: "<<arr[limite_izq]<<"\n";
		res.push_back( arr[limite_izq] );
		++limite_izq;
	}
	
	sort(res.begin(), res.end());
	
}










