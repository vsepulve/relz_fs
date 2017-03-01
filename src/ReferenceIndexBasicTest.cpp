#include "ReferenceIndexBasicTest.h"
	
//funcion (global por el momento) para threads
void f_sort_test(unsigned int id, unsigned int **shared_arrs, unsigned int *n_largos, unsigned int n_arrs, mutex *shared_mutex, unsigned int *shared_pos, SAComparatorN2 *shared_comp){
	
	shared_mutex->lock();
	cout<<"Thread ["<<id<<"] - Inicio\n";
	shared_mutex->unlock();
	
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
//		shared_mutex->lock();
//		cout<<"Thread ["<<id<<"] - arr largo "<<n_largos[pos_local]<<"\n";
//		shared_mutex->unlock();
		sort(shared_arrs[pos_local], shared_arrs[pos_local]+n_largos[pos_local], *shared_comp);
		++partes_procesadas;
	}
	
	shared_mutex->lock();
	cout<<"Thread ["<<id<<"] - Fin ("<<partes_procesadas<<" partes procesadas)\n";
	shared_mutex->unlock();
	
	return;
}

ReferenceIndexBasicTest::ReferenceIndexBasicTest(){
	ref = NULL;
	arr = NULL;
}
	
ReferenceIndexBasicTest::ReferenceIndexBasicTest(const char *_referencia){
	
	cout<<"ReferenceIndexBasicTest - inicio\n";
	
	//Notar que si se pide el arreglo para el texto con el doble del tamaño
	//se puede copiar el string y llenar el resto con ceros
	//De ese modo, se pueden hacer las busquedas posteriores sin revisar el largo de cada sufijo
	//Tienen que ser ceros para que la busqueda sea lexicograficamente correcta
	
	unsigned int distancia = 2;
	
	largo = strlen(_referencia);
	largo_arr = largo/distancia;
	if(distancia * largo_arr < largo){
		++largo_arr;
	}
	cout<<"ReferenceIndexBasicTest - Largo de texto: "<<largo<<", largo de arr: "<<largo_arr<<"\n";
	arr = new unsigned int[largo_arr];
	
	ref = new unsigned char[largo + 1];
	memset(ref, 0, largo + 1);
	sprintf((char*)ref, "%s", _referencia);
	
	for(unsigned int i = 0; i < largo_arr; ++i){
		arr[i] = distancia * i;
//		cout<<"arr["<<i<<"]: "<<arr[i]<<" (\""<<&(ref[ arr[i] ])<<"\", largo "<<largo-arr[i]<<", char[0]: "<<(unsigned int)(ref[ arr[i]])<<")\n";
	}
	
//	//V1: Full sort
//	cout<<"ReferenceIndexBasicTest - Ordenando Arreglo\n";
//	SAComparator comp(ref, largo);
//	std::sort(arr, arr + largo_arr, comp);
	
	
	
	
	//V3: 2 nivel de bucket (16) y sort
	//uso un par de atajos para acelerar el codigo
	cout<<"ReferenceIndexBasicTest - preparando datos\n";
	unsigned char **index = new unsigned char*[256];
	for(unsigned int i = 0; i < 256; ++i){
		index[i] = new unsigned char[256];
		memset(index[i], 0, 256);
	}
	
	vector<unsigned char> vocabulary = {'A', 'C', 'G', 'N', 'T'};
	unsigned int n_buckets = vocabulary.size() * vocabulary.size();
	unsigned int contador = 0;
	for(unsigned int i = 0; i < vocabulary.size(); ++i){
		for(unsigned int j = 0; j < vocabulary.size(); ++j){
			index[ vocabulary[i] ][ vocabulary[j] ] = contador++;
		}
	}
	
	cout<<"ReferenceIndexBasicTest - contando ("<<n_buckets<<" buckets)\n";
	unsigned int n[n_buckets];
	memset(n, 0, n_buckets * sizeof(int));
	
	//Casos de Borde
	//El caso de la ultima letra en ref[largo-1] debe ser tratado igual al primer par
	//ref[largo-1]['A'], la primera letra del vocabulario
//	++(n[ index[ref[largo-1]][ vocabulary[0] ]  ]);
//	for(unsigned int i = 0; i < largo-1; ++i){
//		++(n[ index[ref[i]][ref[i+1]]  ]);
//	}
	//Esto SOLO es valido si la ultima posicion usada en arr es exactamente la ultima letra del texto
	//Es decir, si es necesario ordenar con una ultima letra "ficticia" (igual a vocabulary[0])
	//Si no es el caso, debe usarse el bucket adecuado (que sera valido, pues habra mas letras en el texto)
	if( largo - 1 == 2 * (largo_arr - 1) ){
		++(n[ index[ref[largo - 1]][ vocabulary[0] ]  ]);
		for(unsigned int i = 0; i < largo_arr-1; ++i){
			++(n[ index[ref[2*i]][ref[2*i+1]]  ]);
		}
	}
	else{
		for(unsigned int i = 0; i < largo_arr; ++i){
			++(n[ index[ref[2*i]][ref[2*i+1]]  ]);
		}
	}
	cout<<"ReferenceIndexBasicTest - creando buckets\n";
	unsigned int **bucket = new unsigned int *[n_buckets];
	for(unsigned int i = 0; i < n_buckets; ++i){
		bucket[i] = new unsigned int[n[i]];
	}
	
	cout<<"ReferenceIndexBasicTest - llenando buckets\n";
	unsigned int *pos = new unsigned int[n_buckets];
	memset(pos, 0, n_buckets * sizeof(int));
	
	//Agrego el ultimo sufijo como el primero de su bucket, mismo caso de borde anterior
//	bucket[ index[ref[largo-1]][ vocabulary[0] ] ][ pos[ index[ref[largo-1]][ vocabulary[0] ] ]++ ] = largo-1;
//	for(unsigned int i = 0; i < largo-1; ++i){
//		bucket[ index[ref[i]][ref[i+1]] ][ pos[ index[ref[i]][ref[i+1]] ]++ ] = i;
//	}
	if( largo - 1 == 2 * (largo_arr - 1) ){
		bucket[ index[ref[largo-1]][ vocabulary[0] ] ][ pos[ index[ref[largo-1]][ vocabulary[0] ] ]++ ] = largo-1;
		for(unsigned int i = 0; i < largo_arr-1; ++i){
			bucket[ index[ref[2*i]][ref[2*i+1]] ][ pos[ index[ref[2*i]][ref[2*i+1]] ]++ ] = 2*i;
		}
	}
	else{
		for(unsigned int i = 0; i < largo_arr; ++i){
			//cout<<"sufijo "<<2*i<<" ("<<(ref+2*i)<<") a bucket["<<ref[2*i]<<"]["<<ref[2*i+1]<<"] ("<<(unsigned int)index[ref[2*i]][ref[2*i+1]]<<" en pos "<<pos[ index[ref[2*i]][ref[2*i+1]] ]<<")\n";
			bucket[ index[ref[2*i]][ref[2*i+1]] ][ pos[ index[ref[2*i]][ref[2*i+1]] ]++ ] = 2*i;
		}
	}
	
	cout<<"ReferenceIndexBasicTest - Ordenando buckets\n";
	SAComparatorN2 comp(ref, largo);

//	//Version Secuencial
//	for(unsigned int i = 0; i < n_buckets; ++i){
//		sort(bucket[i], bucket[i]+pos[i], comp);
//	}
	
	//Version paralela (por ahora, n_threads es fijo)
	unsigned int n_threads = 4;
	vector<std::thread> sort_threads;
	mutex shared_mutex;
	unsigned int shared_pos = 0;
	
	for(unsigned int i = 0; i < n_threads; ++i){
		sort_threads.push_back( std::thread(f_sort_test, i, bucket, pos, n_buckets, &shared_mutex, &shared_pos, &comp) );
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
//	cout<<"Elementos copiados de buckets a arr: "<<pos_arr<<"\n";
	
	//Borrado
	for(unsigned int i = 0; i < 255; ++i){
		delete [] index[i];
	}
	delete [] index;
	
	for(unsigned int i = 0; i < n_buckets; ++i){
		delete [] bucket[i];
	}
	delete [] bucket;
	
	delete [] pos;
	
	
//	for(unsigned int i = 0; i < largo_arr; ++i){
//		if(largo-arr[i] < 10){
//			cout<<"arr["<<i<<"]: "<<arr[i]<<" (\""<<ref+arr[i]<<"\", largo "<<largo-arr[i]<<", char[0]: "<<(unsigned int)(ref[ arr[i]])<<")\n";
//		}
//		else{
//			cout<<"arr["<<i<<"]: "<<arr[i]<<" (\""<<string((const char*)(ref+arr[i]), 10)<<"\", largo "<<largo-arr[i]<<", char[0]: "<<(unsigned int)(ref[ arr[i]])<<")\n";
//		}
//	}
	
	cout<<"ReferenceIndexBasicTest - fin\n";
}

ReferenceIndexBasicTest::~ReferenceIndexBasicTest(){
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
void ReferenceIndexBasicTest::find(const char *text, unsigned int size, unsigned int &position, unsigned int &length) const{
//	cout<<"ReferenceIndexBasicTest::find - inicio (largo "<<size<<")\n";
	position = 0;
	length = 0;
	
	unsigned int limite_izq = 0;
	unsigned int limite_der = largo_arr-1;
	unsigned int utimo_comun = 0xffffffff;
	
	for(unsigned int cur_pos = 0; cur_pos < size; ++cur_pos){
//		cout<<"ReferenceIndexBasicTest::find - cur_pos: "<<cur_pos<<" ("<<limite_izq<<", "<<limite_der<<")\n";
		//se busca el caracter text[cur_pos] en la posicion cur_pos de los sufijos
		
		//buscar limite izq

//		//version lineal
//		unsigned int izq = limite_izq;
//		while( (izq < largo_arr) && (&(ref[ arr[izq] ]))[cur_pos] < text[cur_pos] ){
//			++izq;
//		}

		//Version binaria
		unsigned int l = limite_izq;
		unsigned int h = limite_der;
		unsigned int m;
		while(l < h){
			m = l + ((h-l)>>1);
			if( (&(ref[ arr[m] ]))[cur_pos] < text[cur_pos] ){
				l = m+1;
			}
			else{
				h = m;
			}
		}
		unsigned int izq = h;
		
//		cout<<"ReferenceIndexBasicTest::find - izq: "<<izq<<"\n";
		
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
			if( (&(ref[ arr[m] ]))[cur_pos] <= text[cur_pos] ){
				l = m+1;
			}
			else{
				h = m;
			}
		}
		unsigned int der = h;
		if( (&(ref[ arr[der] ]))[cur_pos] > text[cur_pos] ){
			--der;
		}
		
//		cout<<"ReferenceIndexBasicTest::find - der: "<<der<<"\n";
		
		//si limite izq > der, salir y usar el ultimo rango valido
		if(izq == der){
			limite_izq = izq;
			limite_der = der;
			//Extension de la cola (en el caso izq == der)
			while( (cur_pos < size) && (&(ref[ arr[limite_izq] ]))[cur_pos] == text[cur_pos] ){
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
	
//	cout<<"ReferenceIndexBasicTest::find - limite_izq: "<<limite_izq<<"\n";
//	cout<<"ReferenceIndexBasicTest::find - limite_der: "<<limite_der<<"\n";
//	cout<<"ReferenceIndexBasicTest::find - ultimo char comun: "<<utimo_comun<<"\n";
	
	if(utimo_comun != 0xffffffff){
		position = arr[limite_izq];
		length = 1+utimo_comun;
//		cout<<"ReferenceIndexBasicTest::find - Prefijo mayor: \""<<(string((const char*)(ref + position), length))<<"\"\n";
	}
	
}

	
//Metodos de save para carga sin construccion
void ReferenceIndexBasicTest::save(const char *ref_file){

	fstream escritor(ref_file, fstream::trunc | fstream::binary | fstream::out);
	escritor.write((char*)(&largo), sizeof(int));
	escritor.write((char*)(&largo_arr), sizeof(int));
	escritor.write((char*)ref, largo);
	escritor.write((char*)arr, largo_arr * sizeof(int));
	escritor.close();
	
}

//Metodos de carga sin construccion
void ReferenceIndexBasicTest::load(const char *ref_file){

	if(ref != NULL){
		delete [] ref;
		ref = NULL;
	}
	if(arr != NULL){
		delete [] arr;
		arr = NULL;
	}
	
	fstream lector(ref_file, fstream::binary | fstream::in);
	lector.read((char*)(&largo), sizeof(int));
	lector.read((char*)(&largo_arr), sizeof(int));
	cout<<"ReferenceIndexBasicTest::load - cargando referencia de "<<largo<<" chars\n";
	ref = new unsigned char[2 * largo];
	memset(ref, 0, 2 * largo);
	lector.read((char*)ref, largo);
	arr = new unsigned int[largo_arr];
	lector.read((char*)arr, largo_arr * sizeof(int));
	lector.close();

}










