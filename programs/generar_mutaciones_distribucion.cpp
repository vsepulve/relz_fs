
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <sstream>
#include <fstream>
#include <string.h>

#include <algorithm>
#include <vector>
#include <map>

using namespace std;

// Notar que este programa es mas adecuado con un texto base limpio (solo sdn y en mayusculas)
// Las mutaciones en caracteres no validos (como '\n') simplemente son omitidas
// Eso y la repeticion de posiciones causan que el numero de mutaciones efectivas sea ligeramente menor a lo pedido

int main(int argc, char *argv[]){
	
	if(argc != 7){
		cout<<"\nModo de Uso: generar_mutaciones entrada distribucion min_mut max_mut base_salida n_archivos\n";
		cout<<"\n";
		return 0;
	}
	
	const char *archivo_entrada = argv[1];
	const char *archivo_dist = argv[2];
	unsigned int min_mut = atoi(argv[3]);
	unsigned int max_mut = atoi(argv[4]);
	
	const char *base_salida = argv[5];
	unsigned int n_archivos = atoi(argv[6]);
	
	char archivo_mut[128];
	
	cout<<"Inicio (de \""<<archivo_entrada<<"\" con dist "<<archivo_dist<<", n_mut: ["<<min_mut<<", "<<max_mut<<"])\n";
	
	fstream lector(archivo_entrada, fstream::in);
	fstream lector_dist(archivo_dist, fstream::in);
	fstream escritor;
	
	if( !lector.good() || !lector_dist.good() ){
		cerr<<"Problemas al abrir archivos\n";
		return 1;
	}
	
	// Cargar distribucion
	vector<unsigned int> dist;
	cout<<"Cargando distribucion desde \""<<archivo_dist<<"\"\n";
	unsigned int line_size = 128;
	char line[line_size];
	unsigned int lectura = 0;
	while( lector_dist.good() ){
		lector_dist.getline(line, line_size);
		lectura = lector_dist.gcount();
//		cout<<"line: \""<<line<<"\"\n";
		if( (lectura == 0) || strlen(line) < 1 ){
			break;
		}
		dist.push_back( atoi(line) );
	}
	lector_dist.close();
	cout<<"Valores leidos: "<<dist.size()<<"\n";
	
	lector.seekg(0, lector.end);
	unsigned long long text_size = lector.tellg();
	lector.seekg(0, lector.beg);
	
	char *text = new char[text_size + 1];
	unsigned int block_size = 1024*1024;
	char *block = new char[block_size + 1];
	
	cout<<"Cargando "<<text_size<<" chars\n";
	
	unsigned long long total_leido = 0;
	while( total_leido < text_size ){
		lector.read( text + total_leido, block_size );
		lectura = lector.gcount();
		if( lectura == 0 ){
			break;
		}
		total_leido += lectura;
	}
	lector.close();
	text[total_leido] = 0;
	text_size = total_leido;
	
	cout<<"Chars cargados: "<<text_size<<"\n";
	
	map<char, char*> mutacion;
	char arr_a[] = {'T', 'C', 'G'};
	char arr_t[] = {'A', 'C', 'G'};
	char arr_c[] = {'A', 'T', 'G'};
	char arr_g[] = {'A', 'T', 'C'};
	mutacion['A'] = arr_a;
	mutacion['T'] = arr_t;
	mutacion['C'] = arr_c;
	mutacion['G'] = arr_g;
	
	// la proxima posicion a mutar (notar que puede ser ll)
	unsigned long long next = 0;
	unsigned int pos = 0;
	char c = 0;
	
	// Mapa de pos->char para mutaciones
	// Al final igual las ordeno, por si usara algun mapa no ordenado
	map<unsigned int, char> mapa_mut;
	// Vector para el sort
	vector< pair<unsigned int, char> > vector_mut;
	
	// Desde aqui habria que repetir por cada archivo
	for( unsigned int k = 0; k < n_archivos; ++k ){
		
		unsigned int n_mut = min_mut + rand() % (1 + max_mut - min_mut);
		
		cout<<"Construyendo archivo "<<k<<" con "<<n_mut<<" mutaciones\n";
		
		for( unsigned int i = 0; i < n_mut; ++i ){
			pos = rand() % dist.size();
			next += dist[pos];
			// Si llego al final, simplemente escogo otro punto de inicio aleatorio
			if( next >= text_size ){
				next = rand() % text_size;
			}
			// Por ahora guardo la mutacion en el texto y en el mapa
			// En la generacion multile, NO DEBE modifcarse el texto, solo el mapa
			c = toupper( text[next] );
			if( c == 'A' || c == 'T' || c == 'C' || c == 'G' ){
				//text[next] = mutacion[c][ pos % 3 ];
				//mapa_mut[next] = text[next];
				mapa_mut[next] = mutacion[c][ pos % 3 ];
			}
		}
	
		// Escritura del archivo de mutaciones
		// text_size / mapa_mut.size / pares <pos, char> crecientemente
		cout<<"Preparando archivo de mutaciones\n";
		for( map<unsigned int, char>::iterator it = mapa_mut.begin(); it != mapa_mut.end(); it++ ){
			vector_mut.push_back( pair<unsigned int, char>(it->first, it->second) );
		}
		sort( vector_mut.begin(), vector_mut.end() );
		unsigned int n = vector_mut.size();
		sprintf(archivo_mut, "%s_%d.mut", base_salida, k);
		escritor.open(archivo_mut, fstream::binary | fstream::out | fstream::trunc);
		if(! escritor.good() ){
			cerr<<"Problemas al abrir archivo mut\n";
		}
		else{
			cout<<"Escribiendo archivo \""<<archivo_mut<<"\"\n";
			escritor.write( (char*)&text_size, sizeof(int) );
			escritor.write( (char*)&n, sizeof(int) );
			for(unsigned int i = 0; i < n; ++i){
				pos = vector_mut[i].first;
				c = vector_mut[i].second;
				escritor.write( (char*)&pos, sizeof(int) );
				escritor.write( &c, 1 );
			}
			escritor.close();
		}
		
		/*
		// Escritura de archivo de texto
		// Omitir esto para archivos multiples
		cout<<"Escribiendo archivo de texto\n";
		sprintf(archivo_mut, "%s_%d.txt", base_salida, k);
		escritor.open(archivo_mut, fstream::out | fstream::trunc);
		unsigned long long total_escrito = 0;
		unsigned int pos_vector = 0;
		while( escritor.good() && (total_escrito < text_size) ){
			if( total_escrito +  block_size > text_size ){
				block_size = text_size - total_escrito;
			}
			memcpy(block, text + total_escrito, block_size);
			while( (pos_vector < vector_mut.size()) && 
				(vector_mut[pos_vector].first < block_size + total_escrito) ){
				block[ vector_mut[pos_vector].first - total_escrito ] = vector_mut[pos_vector].second;
				++pos_vector;
			}
//			for(unsigned int i = 0; i < block_size; ++i){
//				if( mapa_mut.find( i + total_escrito ) != mapa_mut.end() ){
//					block[i] = mapa_mut[ i + total_escrito ];
//				}
//			}
			escritor.write(block, block_size);
			// Aqui estoy asumiendo que la escritura siempre es exitosa
			total_escrito += block_size;
		}
		escritor.close();
		*/
		
		cout<<"Archivo terminado (mutaciones: "<<n<<" / "<<((long double)n/text_size)<<")\n";
		vector_mut.clear();
		mapa_mut.clear();
		
		// Hasta aqui el proceso de archivos multiples
	}
	
	delete [] text;
	delete [] block;
	
	cout<<"Fin\n";
	
	return 0;
}













