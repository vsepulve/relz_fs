#include "RelzIndexSecondary.h"

RelzIndexSecondary::RelzIndexSecondary(){
	reference = NULL;
}

RelzIndexSecondary::RelzIndexSecondary(ReferenceIndex *_reference){
	reference = _reference;
}

RelzIndexSecondary::~RelzIndexSecondary(){
	reference = NULL;
	segments.clear();
}

void RelzIndexSecondary::search(const char *text, unsigned int size, vector<unsigned int> &res) const{
	
	// Creo que no se necesita la referencia en esta etapa
//	if(reference == NULL){
//		cerr<<"RelzIndexSecondary::search - Error, reference NULL\n";
//		return;
//	}
	
	cout<<"RelzIndexSecondary::search - Inicio (text: "<<text<<", size: "<<size<<", segmentos: "<<segments.size()<<")\n";
	
	// Primera version, textos descomprimidos con tabla de ajuste de posiciones
	// Iterar por cada segmento
	// Buscar el texto en cada segmento linealmente
	// Usar el inicio y fin del segmento para ajustar las posiciones encontradas
	// Agregar las posiciones ajustadas directamente
	// En esta version ni siquiera es necesario ordenarlas, pues los segmentos estan ordenados
	// Se pueden omitir segmentos de largo menor a size
	// vector< <pair< text, pair<ini, fin> > > > (Creo que no se necesita, de hecho, la posicion de termino, solo ini)
	// vector< pair<ini, text> >
	unsigned int pos = 0;
	for(unsigned int i = 0; i < segments.size(); ++i){
		unsigned int ini = segments[i].first;
		// Por seguridad usare directamente second
		// string seg = segments[i].second;
		
		// Aqui habria que considerar tambien los caracteres previos al segmento (de la referencia)
		// Lo ideal seria buscar directamente en esos caracteres, pero para eso habria que ser capaz de buscar parcialmente
		// Quizas sea una opcion mas simple copiar los caracteres antes y despues del segmento
		// Para realizar eso de modo eficiente, habria que guardar los segmentos con espacio para la copia
		// Esto es porque no es viable copiar tambien los segmentos (eso seria memcpy de TODO el texto secundario por cada busqueda)
		
		// Hay que considerar, eso si, que el texto adicional que se agrega es exactamente del largo del patron -1
		// No es una busqueda cualquiera, es un caso mas acotado, pero en que SI importan los match con overlap
		// Quizas sea mas facil de lo que estoy pensando, quizas sea viable una busqueda secuencial mas lenta que para el resto del texto
		//
		
		if( segments[i].second.length() < size ){
			continue;
		}
		cout<<"RelzIndexSecondary::search - Procesando segmento "<<i<<"\n";
		while( true ){
//			pos = segments[i].second.find(text, pos + 1);
			pos = segments[i].second.find(text, pos + size);
			if(pos == string::npos || pos > segments[i].second.length()){
				break;
			}
			else{
				// Agregar pos ajustada
				cout<<"RelzIndexSecondary::search - Agregando "<<(pos + ini)<<" ("<<pos<<" + "<<ini<<")\n";
				res.push_back(pos + ini);
			}
		}// while... next pos
		
		
	}// for... cada segmento
	
	
}

void RelzIndexSecondary::indexFactors(DecoderBlocksRelz &decoder, unsigned int min_len){

	segments.clear();
	
	unsigned int n_blocks = decoder.getNumBlocks();
	unsigned int block_size = decoder.getBlockSize();
	
	cout<<"RelzIndexSecondary::indexFactors - Inicio ("<<n_blocks<<" blocks de largo "<<block_size<<")\n";
	
	char *buff = new char[block_size + 1];
	// unsigned int *buff_pos = NULL;
	unsigned int *buff_len = NULL;
	unsigned int n_factors = 0;
	unsigned int len;
	
	// Iterar por cada bloque
	// Para cada bloque, iterar por cada factor
	// Si el factor es largo, iniciar corte (guardar segmento, continuar contando largo para proximo ini, resetear segmento)
	// Si es corto, continuar segmento (agregar texto, continuar contando largo)
	// Agregar ultimo segmento si existe
	
	// Notar que falta agregar los bordes de overlap
	
	cout<<"RelzIndexSecondary::indexFactors - Buscando factores...\n";
	unsigned int seq_pos = 0;
	unsigned int block_pos = 0;
	unsigned int segment_start = 0;
	char *buff_seg = new char[block_size + 1];
	buff_seg[0] = 0;
	unsigned int seg_len = 0;
	for(unsigned int i = 0; i < n_blocks; ++i){
		decoder.decodeBlock(i, buff);
		// buff_pos = decoder.getBuffPos();
		buff_len = decoder.getBuffLen();
		n_factors = decoder.getNumFactors();
		block_pos = 0;
		cout<<"RelzIndexSecondary::indexFactors - Block "<<i<<", "<<n_factors<<" factores\n";
		for(unsigned int j = 0; j < n_factors; ++j){
			// pos = buff_pos[j];
			len = buff_len[j];
			seq_pos += len;
			if( (len >= min_len) && seg_len > 0){
//				// Punto de corte, agregar segmento si es valido
				cout<<"RelzIndexSecondary::indexFactors - Agregando segmento <"<<segment_start<<", \""<<buff_seg<<"\">\n";
				segments.push_back( pair< unsigned int, string >( segment_start, string(buff_seg) ) );
				segment_start = seq_pos;
				buff_seg[0] = 0;
				seg_len = 0;
			}
			else{
				cout<<"RelzIndexSecondary::indexFactors - Sumando "<<string(buff + block_pos, len)<<" ("<<len<<") a segmento\n";
				// Notar que escribo el 0 ANTES del memcpy (para no pisar el 0 actual para strlen)
//				buff_seg[ strlen(buff_seg) + len ] = 0;
//				memcpy(buff_seg + strlen(buff_seg), buff + block_pos, len);
				
				memcpy(buff_seg + seg_len, buff + block_pos, len);
				seg_len += len;
				buff_seg[seg_len] = 0;
				
				cout<<"RelzIndexSecondary::indexFactors - Res: \""<<buff_seg<<"\" ("<<seg_len<<")\n";
			}
			block_pos += len;
		}// for... cada factor del bloque
	}// for... cada bloque
	if(seg_len > 0){
		// Ultimo segmento
		cout<<"RelzIndexSecondary::indexFactors - Agregando segmento <"<<segment_start<<", \""<<buff_seg<<"\">\n";
		segments.push_back( pair< unsigned int, string >( segment_start, string(buff_seg) ) );
		segment_start = seq_pos;
		buff_seg[0] = 0;
		seg_len = 0;
	}
	
	delete [] buff;
	delete [] buff_seg;
	
}








