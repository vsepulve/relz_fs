#include "Metadata.h"

Metadata::Metadata(){
	save_mode = 0;
	lowcase_runs = NULL;
	nl_pos = NULL;
	nl_skip = NULL;
	
//	nl_ex = NULL;
//	nl_skip_ex = NULL;
//	nl_norm = 0;
//	nl_total = 0;
	
}

Metadata::Metadata(unsigned char _save_mode, vector< pair<unsigned long long, unsigned long long> > *_lowcase_runs, vector<unsigned long long> *_nl_pos){
	save_mode = _save_mode;
	lowcase_runs = _lowcase_runs;
	cout<<"Metadata - lowcase_runs: "<<(lowcase_runs!=NULL)<<"? "<< ((lowcase_runs==NULL)?0:lowcase_runs->size()) <<"\n";
	nl_pos = _nl_pos;
	cout<<"Metadata - nl_pos: "<<(nl_pos!=NULL)<<"? "<< ((nl_pos==NULL)?0:nl_pos->size()) <<"\n";
	nl_skip = NULL;
	if( nl_pos != NULL ){
		prepareNewLinesSkip();
	}
	
//	nl_ex = NULL;
//	nl_skip_ex = NULL;
//	nl_norm = 0;
//	nl_total = 0;
	
}

/*
Metadata::Metadata(unsigned char _save_mode, vector< pair<unsigned long long, unsigned long long> > *_lowcase_runs, unsigned int _nl_total, unsigned int _nl_norm, vector< pair<unsigned long long, unsigned long long> > *_nl_ex){
	save_mode = _save_mode;
	lowcase_runs = _lowcase_runs;
	cout<<"Metadata - lowcase_runs: "<<(lowcase_runs!=NULL)<<"? "<< ((lowcase_runs==NULL)?0:lowcase_runs->size()) <<"\n";
	nl_pos = NULL;
//	cout<<"Metadata - nl_pos: "<<(nl_pos!=NULL)<<"? "<< ((nl_pos==NULL)?0:nl_pos->size()) <<"\n";
	nl_skip = NULL;
//	if( nl_pos != NULL ){
//		prepareNewLinesSkip();
//	}
	
	nl_norm = _nl_norm;
	nl_total = _nl_total;
	nl_ex = _nl_ex;
	nl_skip_ex = NULL;
	if( nl_ex != NULL ){
//		prepareNewLinesSkipEx();
	}
}
*/

Metadata::~Metadata(){
	deleteData();
}

void Metadata::deleteData(){
	if(lowcase_runs != NULL){
		lowcase_runs->clear();
		delete lowcase_runs;
		lowcase_runs = NULL;
	}
	if(nl_pos != NULL){
		nl_pos->clear();
		delete nl_pos;
		nl_pos = NULL;
	}
	if(nl_skip != NULL){
		nl_skip->clear();
		delete nl_skip;
		nl_skip = NULL;
	}
//	if(nl_ex != NULL){
//		nl_ex->clear();
//		delete nl_ex;
//		nl_ex = NULL;
//	}
//	if(nl_skip_ex != NULL){
//		nl_skip_ex->clear();
//		delete nl_skip_ex;
//		nl_skip_ex = NULL;
//	}
}

void Metadata::save(fstream *escritor){
	if( escritor == NULL || ! escritor->good() ){
		cerr<<"Metadata::save - Error en escritor\n";
		return;
	}
	cout<<"Metadata::save - Inicio\n";
	unsigned char marca = 0;
	if( lowcase_runs != NULL && nl_pos == NULL ){
		if( save_mode == META_SAVE_UNC ){
			marca = 1;
		}
		else if( save_mode == META_SAVE_VBYTE ){
			marca = 4;
		}
		else{
			cerr<<"Metadata::save - Tipo desconocido "<<(unsigned int)save_mode<<", usando META_SAVE_UNC\n";
			marca = 1;
		}
	}
	else if( lowcase_runs == NULL && nl_pos != NULL ){
		if( save_mode == META_SAVE_UNC ){
			marca = 2;
		}
		else if( save_mode == META_SAVE_VBYTE ){
			marca = 5;
		}
		else{
			cerr<<"Metadata::save - Tipo desconocido "<<(unsigned int)save_mode<<", usando META_SAVE_UNC\n";
			marca = 2;
		}
	}
	else if( lowcase_runs != NULL && nl_pos != NULL ){
		if( save_mode == META_SAVE_UNC ){
			marca = 3;
		}
		else if( save_mode == META_SAVE_VBYTE ){
			marca = 6;
		}
		else{
			cerr<<"Metadata::save - Tipo desconocido "<<(unsigned int)save_mode<<", usando META_SAVE_UNC\n";
			marca = 3;
		}
	}
	cout<<"Metadata::save - marca: "<<(unsigned int)marca<<" (save_mode: "<<(unsigned int)save_mode<<")\n";
	escritor->write((char*)&marca, 1);
	if( lowcase_runs != NULL ){
		if( save_mode == META_SAVE_UNC ){
			saveLowcaseUncompress(escritor);
		}
		else if( save_mode == META_SAVE_VBYTE ){
			saveLowcaseVarByte(escritor);
		}
		else{
			cerr<<"Metadata::save - Tipo desconocido "<<(unsigned int)save_mode<<", usando META_SAVE_UNC\n";
			saveLowcaseUncompress(escritor);
		}
	}
	if( nl_pos != NULL ){
		if( save_mode == META_SAVE_UNC ){
			saveNewLinesUncompress(escritor);
		}
		else if( save_mode == META_SAVE_VBYTE ){
//			saveNewLinesVarByte(escritor);
			saveNewLinesVarByteEx(escritor);
		}
		else{
			cerr<<"Metadata::save - Tipo desconocido "<<(unsigned int)save_mode<<", usando META_SAVE_UNC\n";
			saveNewLinesUncompress(escritor);
		}
	}
	cout<<"Metadata::save - Fin\n";
	return;
}

void Metadata::load(fstream *lector){
	if( lector == NULL || ! lector->good() ){
		cerr<<"Metadata::load - Error en lector\n";
		return;
	}
	cout<<"Metadata::load - Inicio\n";
	deleteData();
	unsigned char marca = 0;
	lector->read((char*)&marca, 1);
	cout<<"Metadata::load - Marca: "<<(unsigned int)marca<<"\n";
	if( marca == 0 ){
		cout<<"Metadata::load - Sin Datos\n";
	}
	else if( marca == 1 ){
		save_mode = META_SAVE_UNC;
		loadLowcaseUncompress(lector);
	}
	else if( marca == 2 ){
		save_mode = META_SAVE_UNC;
		loadNewLinesUncompress(lector);
	}
	else if( marca == 3 ){
		save_mode = META_SAVE_UNC;
		loadLowcaseUncompress(lector);
		loadNewLinesUncompress(lector);
	}
	else if( marca == 4 ){
		save_mode = META_SAVE_VBYTE;
		loadLowcaseVarByte(lector);
	}
	else if( marca == 5 ){
		save_mode = META_SAVE_VBYTE;
//		loadNewLinesVarByte(lector);
		loadNewLinesVarByteEx(lector);
	}
	else if( marca == 6 ){
		save_mode = META_SAVE_VBYTE;
		loadLowcaseVarByte(lector);
//		loadNewLinesVarByte(lector);
		loadNewLinesVarByteEx(lector);
	}
	cout<<"Metadata::load - Fin (save_mode: "<<(unsigned int)save_mode<<")\n";
}

void Metadata::load(BytesReader *lector){
	if( lector == NULL || ! lector->good() ){
		cerr<<"Metadata::load - Error en lector\n";
		return;
	}
	cout<<"Metadata::load - Inicio\n";
	deleteData();
	unsigned char marca = 0;
	lector->read((char*)&marca, 1);
//	cout<<"Metadata::load - Marca: "<<(unsigned int)marca<<"\n";
	if( marca == 0 ){
		cout<<"Metadata::load - Sin Datos\n";
	}
	else if( marca == 1 ){
		save_mode = META_SAVE_UNC;
		loadLowcaseUncompress(lector);
	}
	else if( marca == 2 ){
		save_mode = META_SAVE_UNC;
		loadNewLinesUncompress(lector);
	}
	else if( marca == 3 ){
		save_mode = META_SAVE_UNC;
		loadLowcaseUncompress(lector);
		loadNewLinesUncompress(lector);
	}
	else if( marca == 4 ){
		save_mode = META_SAVE_VBYTE;
		loadLowcaseVarByte(lector);
	}
	else if( marca == 5 ){
		save_mode = META_SAVE_VBYTE;
		loadNewLinesVarByte(lector);
	}
	else if( marca == 6 ){
		save_mode = META_SAVE_VBYTE;
		loadLowcaseVarByte(lector);
		loadNewLinesVarByte(lector);
	}
//	cout<<"Metadata::load - Fin\n";
}

unsigned int Metadata::size(){
	unsigned int size = 1;
	if( lowcase_runs != NULL ){
		size += sizeLowcase();
	}
	if( nl_pos != NULL ){
		size += sizeNewLines();
	}
//	cout<<"Metadata::size - size: "<<size<<" (save_mode: "<<(unsigned int)save_mode<<")\n";
	return size;
}

unsigned int Metadata::sizeLowcase(){
	unsigned int size = 0;
	if( save_mode == META_SAVE_UNC ){
		//uncompress (largo, mas 2 enteros por par)
		size += sizeof(int);
		size += lowcase_runs->size() * 2 * sizeof(long long);
		cout<<"Metadata::sizeLowcase - unc, size: "<<size<<" ("<<lowcase_runs->size()<<" pares)\n";
	}
	else if( save_mode == META_SAVE_VBYTE ){
		//vbyte (largo, n_bytes, y los bytes usados para cada numero)
		size += 2 * sizeof(int);
		BitsUtils utils;
		unsigned long long ini, fin;
		unsigned long long last = 0;
		for(unsigned int i = 0; i < lowcase_runs->size(); ++i){
			ini = lowcase_runs->at(i).first - last;
			last = lowcase_runs->at(i).first;
			fin = lowcase_runs->at(i).second - last;
			last = lowcase_runs->at(i).second;
			
//			if( ini > max_int || fin > max_int ){
//				cerr<<"Metadata::sizeLowcase - Error, delta > 32 bits\n";
//				break;
//			}
//			size += utils.size_varbyte((unsigned int)ini);
//			size += utils.size_varbyte((unsigned int)fin);

			size += utils.size_varbyte(ini);
			size += utils.size_varbyte(fin);
		}
		cout<<"Metadata::sizeLowcase - vbyte, size: "<<size<<"\n";
	}
	else{
		cerr<<"Metadata::sizeLowcase - Tipo desconocido "<<(unsigned int)save_mode<<"\n";
	}
	return size;
}

unsigned int Metadata::sizeNewLines(){
	unsigned int size = 0;
	if( save_mode == META_SAVE_UNC ){
		//uncompress (largo, mas 1 entero por numero)
		size += sizeof(int);
		size += nl_pos->size() * sizeof(long long);
//		cout<<"Metadata::sizeNewLines - unc, size: "<<size<<" ("<<nl_pos->size()<<" pos)\n";
	}
	else if( save_mode == META_SAVE_VBYTE ){
	
	
		/*
		//vbyte (largo, n_bytes, y los bytes usados para cada numero)
		size += 2 * sizeof(int);
		BitsUtils utils;
		unsigned int pos;
		unsigned int last = 0;
		for(unsigned int i = 0; i < nl_pos->size(); ++i){
			pos = nl_pos->at(i) - last;
			last = nl_pos->at(i);
			size += utils.size_varbyte(pos);
		}
//		cout<<"Metadata::sizeNewLines - vbyte, size: "<<size<<"\n";
		*/
		
		//Tamaño de la version norm + ex
		
//		cout<<"Metadata::sizeNewLines - Calculando tamaño de vbyte EX\n";
		
		//4 enteros (size_nl, nl_norm, size_ex, n_bytes) mas los que use vbyte
		size += 4 * sizeof(int);
		
		unsigned int size_nl = nl_pos->size();
		map<unsigned int, unsigned int> mapa_deltas;
		map<unsigned int, unsigned int>::iterator it_deltas;
		unsigned long long pos;
		unsigned long long last = 0;
		for(unsigned int i = 0; i < size_nl; ++i){
			pos = nl_pos->at(i) - last;
			last = nl_pos->at(i);
			if( pos > max_int ){
				cerr<<"Metadata::sizeNewLines - Error, delta > 32 bits\n";
				break;
			}
			mapa_deltas[(unsigned int)pos]++;
		}
		unsigned int nl_norm = 0;
		unsigned int mejor_conteo = 0;
		for( it_deltas = mapa_deltas.begin(); it_deltas != mapa_deltas.end(); it_deltas++ ){
			if( it_deltas->second > mejor_conteo ){
				nl_norm = it_deltas->first;
				mejor_conteo = it_deltas->second;
			}
		}
//		cout<<"Metadata::sizeNewLines - nl_norm: "<<nl_norm<<", preparando excepciones\n";
		
		vector< pair<unsigned int, unsigned int> > nl_ex;
		last = 0;
		for(unsigned int i = 0; i < size_nl; ++i){
			pos = nl_pos->at(i) - last;
			last = nl_pos->at(i);
			if( pos > max_int ){
				cerr<<"Metadata::sizeNewLines - Error, delta > 32 bits\n";
				break;
			}
			if(pos != nl_norm){
				nl_ex.push_back( pair<unsigned int, unsigned int>(i, (unsigned int)pos) );
			}
		}
		
//		cout<<"Metadata::sizeNewLines - guardando datos previos\n";
		
//		cout<<"Metadata::sizeNewLines - Preparando guardado de excepciones\n";
		unsigned int size_ex = nl_ex.size();
		BitsUtils utils;
//		cout<<"Metadata::sizeNewLines - guardando "<<size_ex<<" excepciones\n";
		
		//En este ciclo, los valores (segundo elemento de nl_ex) ya esta en delta
		//Sin embargo, las posiciones tambien pueden guardarse en delta (pues son siempre crecientes)
		last = 0;
		for(unsigned int i = 0; i < size_ex; ++i){
			pos = nl_ex[i].first - last;
			last = nl_ex[i].first;
			
//			if( pos > max_int ){
//				cerr<<"Metadata::sizeNewLines - Error, delta > 32 bits\n";
//				break;
//			}
//			size += utils.size_varbyte((unsigned int)pos);
			
			size += utils.size_varbyte(pos);
			size += utils.size_varbyte(nl_ex[i].second);
		}
//		cout<<"Metadata::sizeNewLines - total bytes: "<<size<<"\n";
		
	}
	else{
		cerr<<"Metadata::sizeNewLines - Tipo desconocido "<<(unsigned int)save_mode<<"\n";
	}
	return size;
}

void Metadata::adjustCase(char *buff, unsigned long long ini, unsigned int length){
	if(lowcase_runs == NULL || lowcase_runs->size() == 0){
		return;
	}
	
//	cout<<"Metadata::adjustCase - Inicio (ini: "<<ini<<", length: "<<length<<")\n";
	
	unsigned long long fin = ini + length;
	unsigned long long par_ini = 0;
	unsigned long long par_fin = 0;
	
//	//Descarte izq (Secuencial)
//	unsigned int par = 0;
//	while( (par < lowcase_runs->size()) && (lowcase_runs->at(par).second < ini) ){
//		++par;
//	}
	
	//Descarte izq (Binario)
	unsigned int l = 0;
	unsigned int h = lowcase_runs->size() - 1;
	unsigned int m;
	while(l < h){
		m = l + ((h-l)>>1);
		if( lowcase_runs->at(m).second < ini ){
			l = m+1;
		}
		else{
			h = m;
		}
	}
	unsigned int par = h;
	//El ciclo anterior PUEDE deja a par justo en la posicion anterior a la correcta
	if( lowcase_runs->at(par).second < ini ){
		++par;
	}
	
	//Procesamiento (secuencial en el tamaño del read)
	while( (par < lowcase_runs->size()) && (lowcase_runs->at(par).first < fin) ){
		par_ini = lowcase_runs->at(par).first;
		par_fin = lowcase_runs->at(par).second;
		if(par_ini < ini){
			par_ini = ini;
		}
		if(par_fin > fin){
			par_fin = fin;
		}
		par_ini -= ini;
		par_fin -= ini;
//		cout<<"Metadata::adjustCase - Ajustando ["<<par_ini<<", "<<par_fin<<"]\n";
		for(unsigned int i = (unsigned int)par_ini; i <= (unsigned int)par_fin; ++i){
			buff[i] = tolower(buff[i]);
		}
		++par;
	}
}

unsigned int Metadata::totalNewLines(){
	if(nl_pos == NULL ){
		return 0;
	}
	return nl_pos->size();
}

unsigned int Metadata::countNewLines(unsigned long long pos){
	
	if( nl_pos == NULL || nl_pos->size() < 1 ){
		return 0;
	}
	
//	cout<<"Metadata::countNewLines - Inicio (pos: "<<pos<<")\n";
	
	// Version binaria y sin necesidad de nl_skip (solo si nl_pos es absoluta)
	
	unsigned int l = 0;
	unsigned int h = nl_pos->size() - 1;
	unsigned int m;
	while(l < h){
		m = l + ((h-l)>>1);
		if( nl_pos->at(m) < pos ){
			l = m+1;
		}
		else{
			h = m;
		}
	}
//	cout<<"Metadata::countNewLines - BB terminada (total: "<<h<<")\n";
	while( (h < nl_pos->size()) && (nl_pos->at(h) < pos) ){
		++h;
	}
//	cout<<"Metadata::countNewLines - Fin (total: "<<h<<")\n";
	return h;
	
	
	/*
	unsigned int cur_pos = 0;
	//Solo usar skip list si esta definida, omitirla de otro modo
	if( nl_skip != NULL ){
		while( (cur_pos+1 < nl_skip->size()) && (nl_skip->at(cur_pos+1) < pos) ){
			++cur_pos;
		}
		//Convertir cur_pos de skip a cur_pos real (x step_size)
//		cout<<"Metadata::countNewLines - nl_skip["<<cur_pos<<"] = "<< ((nl_skip->size()==0)?0:nl_skip->at(cur_pos)) <<"\n";
//		cur_pos *= step_size;
		cur_pos <<= step_size_bits;
	}
	//Revision secuencial del resto
	while( (cur_pos < nl_pos->size()) && (nl_pos->at(cur_pos) < pos) ){
		++cur_pos;
	}
//	cout<<"Metadata::countNewLines - Fin (total: "<<cur_pos<<")\n";
	return cur_pos;
	*/
	
	
	
	/*
	//Version norm + ex
	if( nl_total == 0 ){
		return 0;
	}
	cout<<"Metadata::countNewLines - Inicio (pos: "<<pos<<")\n";
	unsigned int cur_pos = 0;
	unsigned int cur_nl = 0;
	unsigned int cur_ex = 0;
	//Solo usar skip list si esta definida, omitirla de otro modo
	//Creo que en esta version nl_skip puede usarse exactamente del mismo modo
	//Basta con que nl_skip sea construido correctamente (usando norm + ex)
	//La excepcion es la actualizacion de cur_nl y de cur_ex
	if( (nl_skip != NULL) 
		&& (nl_skip_ex != NULL) 
		&& (nl_skip->size() == nl_skip_ex->size()) 
		){
		while( (cur_pos+1 < nl_skip->size()) && (nl_skip->at(cur_pos+1) < pos) ){
			++cur_pos;
		}
		cur_nl = nl_skip->at(cur_pos);
		cur_ex = nl_skip_ex->at(cur_pos);
		//Convertir cur_pos de skip a cur_pos real (x step_size)
		cout<<"Metadata::countNewLines - nl_skip["<<cur_pos<<"] = "<< ((nl_skip->size()==0)?0:nl_skip->at(cur_pos)) <<"\n";
//		cur_pos *= step_size;
		cur_pos <<= step_size_bits;
	}
	//Revision secuencial del resto
	//En esta version, asumo que HAY ex
	//Esto es valido para el primer nl
	if( cur_pos == nl_ex->at(cur_ex).first ){
		cur_nl += nl_ex->at(cur_ex).second;
		++cur_ex;
	}
	else{
		cur_nl += nl_norm;
	}
	while( (cur_pos < nl_total) && (cur_nl < pos) ){
		++cur_pos;
		//actualizar cur_nl
		if( cur_pos == nl_ex->at(cur_ex).first ){
			cur_nl += nl_ex->at(cur_ex).second;
			++cur_ex;
		}
		else{
			cur_nl += nl_norm;
		}
	}
	cout<<"Metadata::countNewLines - Fin (total: "<<cur_pos<<")\n";
	return cur_pos;
	*/
	
}

//Asume que la pos de inicio ya esta ajustada para ser absoluta (ini = rel_pos + nl_izq) al menos por ahora
void Metadata::adjustNewLines(char *buff, unsigned long long ini, unsigned int length, unsigned int nl_izq, unsigned int nl_med, char *copy_buff){
	if( buff == NULL || nl_pos == NULL || nl_izq + nl_med > nl_pos->size() || nl_med == 0 ){
		return;
	}
//	cout<<"Metadata::adjustNewLines - Inicio (ini: "<<ini<<", length: "<<length<<", nl_izq: "<<nl_izq<<", nl_med: "<<nl_med<<")\n";
	
	//Aqui ya no necesita descartar los nl previos, eso es nl_izq
	//Parte desde ahi y agrega nl_med '\n'
	
	unsigned int buff_size = length + nl_med;
	bool borrar_buff = false;
	if( copy_buff == NULL ){
		copy_buff = new char[buff_size + 1];
		borrar_buff = true;
	}
	
	unsigned long long pos = 0;
	//Notar que ini se usa como si hubiese habido un NL justo antes
	unsigned long long last_pos = ini - 1;
	unsigned int cur_read = 0;
	unsigned int cur_write = 0;
	for(unsigned int i = nl_izq; i < nl_med + nl_izq; ++i){
		pos = nl_pos->at(i);
//		cout<<"Metadata::adjustNewLines - pos: "<<pos<<", ini: "<<ini<<", cur_read: "<<cur_read<<", cur_write: "<<cur_write<<"\n";
		//agregar el texto hasta pos y luego el '\n'
		if( pos - last_pos - 1 > max_int ){
			cerr<<"Metadata::adjustNewLines - Error, valor > 32bits\n";
			break;
		}
		unsigned int len_cpy = (unsigned int)(pos - last_pos - 1);
		last_pos = pos;
		if(len_cpy > buff_size - cur_write){
			len_cpy = buff_size - cur_write;
		}
//		cout<<"Metadata::adjustNewLines - Copiando desde ["<<cur_read<<" len "<<len_cpy<<"] en "<<cur_write<<" (max: "<<buff_size<<")\n";
		memcpy(copy_buff + cur_write, buff + cur_read, len_cpy);
		cur_read += len_cpy;
		cur_write += len_cpy;
		copy_buff[cur_write++] = '\n';
		if( cur_write > buff_size ){
			cerr<<"Metadata::adjustNewLines - Error de buffer (cur_write: "<<cur_write<<" / "<<buff_size<<")\n";
		}
	}
	
	//Procesar un posible resto (caracteres posteriores al ultimo NL considerado)
	if( cur_write < buff_size ){
//		cout<<"Metadata::adjustNewLines - Copia final (cur_write: "<<cur_write<<", buff_size: "<<buff_size<<", cur_read: "<<cur_read<<", last_pos: "<<last_pos<<")\n";
		
		unsigned int len_cpy = buff_size - cur_write;
		if( len_cpy > length - cur_read ){
			len_cpy = length - cur_read;
		}
//		cout<<"Metadata::adjustNewLines - Copiando (resto) desde ["<<cur_read<<" len "<<len_cpy<<"] en "<<cur_write<<" (max: "<<buff_size<<")\n";
		memcpy(copy_buff + cur_write, buff + cur_read, len_cpy);
		cur_read += len_cpy;
		cur_write += len_cpy;
	}
	copy_buff[cur_write] = 0;
	
//	cout<<"Metadata::adjustNewLines - Comparando:\n";
//	cout<<"Metadata::adjustNewLines - \n["<<buff<<"]\n";
//	cout<<"Metadata::adjustNewLines - \n["<<copy_buff<<"]\n";
	
	//Notar que (length + nl_med) debe ser valido para el llamador (pues su buff era suficiente para el length absoluto)
	memcpy(buff, copy_buff, buff_size);
	buff[buff_size] = 0;
	
	copy_buff[0] = 0;
	if( borrar_buff ){
		delete [] copy_buff;
	}
}

void Metadata::filterNewText(const char *in_buff, unsigned int original_length, unsigned long long original_pos_ini, char *out_buff, unsigned int &adjusted_length, unsigned long long &adjusted_pos_ini){
//void Metadata::filterNewText(const char *in_buff, unsigned int original_length, unsigned int original_pos_ini, char *out_buff, unsigned int &adjusted_length, unsigned int &adjusted_pos_ini){
	
	if( in_buff == NULL || out_buff == NULL || original_length == 0 || nl_pos == NULL ){
		return;
	}
	
	
	// ADVERTENCIA CON ESTO
	// CODIGO QUE NO HA SIDO PROBADO DETALLADAMENTE
	
	
	// Iterar por el texto completo en busca de NL para almacenar y filtrar
	// Tabien hay que buscar runs de case para almacenar (quizas junto con lo anterior)
	
	// Lo corto pues puede ser un buffer sin 0 final
	cout<<"Metadata::filterNewText - Texto original: \""<<string(in_buff, original_length)<<"\"\n";
	
	// Version dummy que no hace nada
	memcpy(out_buff, in_buff, original_length);
	out_buff[original_length] = 0;
	adjusted_length = original_length;
	adjusted_pos_ini = original_pos_ini;
	
	// Version real
	
	// Limpiar rango de nuevo texto (luego se agregaran los NL nuevos)
	// contar el numero de NL de cada extremo para el remove
	// Notar que esto tambien implica ajustar la skip list (por ahora, simplemente la reconstruyo al final)
	unsigned int r_ini = countNewLines(original_pos_ini);
	unsigned int r_fin = countNewLines(original_pos_ini + original_length);
	cout<<"Metadata::filterNewText - Borrando rango NL ("<<r_ini<<", "<<r_fin<<")\n";
	if(r_fin > r_ini){
		nl_pos->erase( nl_pos->begin() + r_ini, nl_pos->begin() + r_fin );
	}
//	prepareNewLinesSkip();
	vector<unsigned long long>::iterator it = nl_pos->begin() + r_ini;
	
	unsigned int mov = 0;
	for(unsigned int i = 0; i < original_length; ++i){
		if( in_buff[i] == '\n' ){
			// NL encontrado, agregar pos absoluta
			// Ajustar nl_pos (o guardar para ajustar despues)
			
			while( (it != nl_pos->end()) && (*it < i + original_pos_ini) ){
				it++;
			}
			// if... it == end => push_back o algo asi
			if(it == nl_pos->end()){
				nl_pos->push_back(i + original_pos_ini);
				// No estoy seguro si es necesario redefinir end luego de un push_back
				it = nl_pos->end();
			}
			else if( *it != i + original_pos_ini ){
				cout<<"Metadata::filterNewText - Agregando "<<(i + original_pos_ini)<<"\n";
				nl_pos->insert(it, i + original_pos_ini);
			}
			
			++mov;
		}
		else{
			// Mover char
			out_buff[ i - mov ] = in_buff[i];
		}
	}//for... cada char
	//Cortar la cola del string
	out_buff[ original_length - mov ] = 0;
	
	cout<<"Metadata::filterNewText - Texto filtrado: \""<<out_buff<<"\"\n";
	
	adjusted_length = original_length - mov;
	adjusted_pos_ini = original_pos_ini - countNewLines(original_pos_ini);
	
	cout<<"Metadata::filterNewText - nl_pos resultante:\n";
	for( it = nl_pos->begin(); it != nl_pos->end(); it++ ){
		cout<<"nl_pos[]: "<<(*it)<<"\n";
	}
	
	prepareNewLinesSkip();
	
}

void Metadata::saveLowcaseUncompress(fstream *escritor){
	if( escritor == NULL || ! escritor->good() ){
		cerr<<"Metadata::saveLowcaseUncompress - Error en escritor\n";
		return;
	}
//	cout<<"Metadata::saveLowcaseUncompress - Inicio\n";
	
	unsigned int size = lowcase_runs->size();
	escritor->write((char*)&size, sizeof(int));
	//Version Directa
	unsigned long long ini, fin;
//	cout<<"Metadata::saveLowcaseUncompress - Guardando "<<size<<" pares\n";
	for(unsigned int i = 0; i < size; ++i){
		ini = lowcase_runs->at(i).first;
		fin = lowcase_runs->at(i).second;
		escritor->write((char*)&ini, sizeof(long long));
		escritor->write((char*)&fin, sizeof(long long));
//		cout<<"["<<ini<<", "<<fin<<"]\n";
	}
//	cout<<"Metadata::saveLowcaseUncompress - Fin\n";
}
	
void Metadata::loadLowcaseUncompress(fstream *lector){
	if( lector == NULL || ! lector->good() ){
		cerr<<"Metadata::loadLowcaseUncompress - Error en lector\n";
		return;
	}
//	cout<<"Metadata::loadLowcaseUncompress - Inicio\n";
	
	//Datos
	lowcase_runs = new vector< pair<unsigned long long, unsigned long long> >();
	unsigned int size = 0;
	lector->read((char*)&size, sizeof(int));
	//Version Directa
	unsigned long long ini, fin;
//	cout<<"Metadata::loadLowcaseUncompress - Cargando "<<size<<" pares\n";
	for(unsigned int i = 0; i < size; ++i){
		lector->read((char*)&ini, sizeof(long long));
		lector->read((char*)&fin, sizeof(long long));
		lowcase_runs->push_back( pair<unsigned long long, unsigned long long>(ini, fin) );
//		cout<<"["<<ini<<", "<<fin<<"]\n";
	}
//	cout<<"Metadata::loadLowcaseUncompress - Fin\n";
}

void Metadata::saveLowcaseVarByte(fstream *escritor){
	if( escritor == NULL || ! escritor->good() ){
		cerr<<"Metadata::saveLowcaseVarByte - Error en escritor\n";
		return;
	}
	cout<<"Metadata::saveLowcaseVarByte - Inicio\n";
	
	unsigned int size = lowcase_runs->size();
	escritor->write((char*)&size, sizeof(int));
	//Version VarByte
	//Buffer para vbyte (uso el peor caso, 5 bytes por cada numero por par)
	unsigned int worst_case = 5 * 2 * size;
	unsigned char *buff = new unsigned char[ worst_case ];
	unsigned int cur_byte = 0;
	BitsUtils utils;
	unsigned long long ini, fin;
	unsigned long long last = 0;
//	cout<<"Metadata::saveLowcaseVarByte - Preparando "<<size<<" pares\n";
	for(unsigned int i = 0; i < size; ++i){
		ini = lowcase_runs->at(i).first - last;
		last = lowcase_runs->at(i).first;
		fin = lowcase_runs->at(i).second - last;
		last = lowcase_runs->at(i).second;
		
//		if( ini > max_int || fin > max_int ){
//			cerr<<"Metadata::saveLowcaseVarByte - Error, delta > 32bits\n";
//			break;
//		}
//		cur_byte += utils.write_varbyte(buff + cur_byte, (unsigned int)ini);
//		cur_byte += utils.write_varbyte(buff + cur_byte, (unsigned int)fin);
		
		cur_byte += utils.write_varbyte(buff + cur_byte, ini);
		cur_byte += utils.write_varbyte(buff + cur_byte, fin);
//		if(i < 3) cout<<"["<<lowcase_runs->at(i).first<<", "<<lowcase_runs->at(i).second<<"] -> ["<<ini<<", "<<fin<<"]\n";
	}
//	cout<<"Metadata::saveLowcaseVarByte - Guardando "<<cur_byte<<" bytes ("<<((unsigned int*)buff)[0]<<")\n";
	escritor->write((char*)&cur_byte, sizeof(int));
	escritor->write((char*)buff, cur_byte);
	delete [] buff;
	cout<<"Metadata::saveLowcaseVarByte - Fin\n";
}

void Metadata::loadLowcaseVarByte(fstream *lector){
	if( lector == NULL || ! lector->good() ){
		cerr<<"Metadata::loadLowcaseVarByte - Error en lector\n";
		return;
	}
	cout<<"Metadata::loadLowcaseVarByte - Inicio\n";
	
	//Datos
	lowcase_runs = new vector< pair<unsigned long long, unsigned long long> >();
	unsigned int size = 0;
	lector->read((char*)&size, sizeof(int));
	//Version VarByte
	unsigned int n_bytes = 0;
	lector->read((char*)&n_bytes, sizeof(int));
//	cout<<"Metadata::loadLowcaseVarByte - Cargando "<<n_bytes<<" bytes\n";
	unsigned char *buff = new unsigned char[n_bytes];
	lector->read((char*)buff, n_bytes);
	unsigned int cur_byte = 0;
	BitsUtils utils;
	unsigned long long ini, fin;
//	unsigned int delta_ini, delta_fin;
	unsigned long long sum = 0;
//	cout<<"Metadata::loadLowcaseVarByte - Preparando "<<size<<" pares ("<<((unsigned int*)buff)[0]<<")\n";
	for(unsigned int i = 0; i < size; ++i){
		cur_byte += utils.read_varbyte(buff + cur_byte, ini);
		cur_byte += utils.read_varbyte(buff + cur_byte, fin);
		ini += sum;
		fin += ini;
		sum = fin;
		lowcase_runs->push_back( pair<unsigned long long, unsigned long long>(ini, fin) );
//		if(i < 3) cout<<"["<<ini<<", "<<fin<<"]\n";
	}
//	cout<<"Metadata::loadLowcaseVarByte - "<<cur_byte<<" de "<<n_bytes<<" bytes usados\n";
	delete [] buff;
	cout<<"Metadata::loadLowcaseVarByte - Fin\n";
}

void Metadata::saveNewLinesUncompress(fstream *escritor){
	if( escritor == NULL || ! escritor->good() ){
		cerr<<"Metadata::saveNewLinesUncompress - Error en escritor\n";
		return;
	}
//	cout<<"Metadata::saveNewLinesUncompress - Inicio\n";
	
	unsigned int size = nl_pos->size();
	escritor->write((char*)&size, sizeof(int));
	//Version Directa
	unsigned long long pos;
	cout<<"Metadata::saveNewLinesUncompress - Guardando "<<size<<" posiciones\n";
	for(unsigned int i = 0; i < size; ++i){
		pos = nl_pos->at(i);
		escritor->write((char*)&pos, sizeof(long long));
		//cout<<"["<<pos<<"]\n";
	}
//	cout<<"Metadata::saveNewLinesUncompress - Fin\n";
}

void Metadata::loadNewLinesUncompress(fstream *lector){
	if( lector == NULL || ! lector->good() ){
		cerr<<"Metadata::loadNewLinesUncompress - Error en lector\n";
		return;
	}
//	cout<<"Metadata::loadNewLinesUncompress - Inicio\n";
	
	//Datos
	nl_pos = new vector<unsigned long long>();
	unsigned int size = 0;
	lector->read((char*)&size, sizeof(int));
	//Version Directa
	unsigned long long pos;
//	cout<<"Metadata::loadNewLinesUncompress - Cargando "<<size<<" posiciones\n";
	for(unsigned int i = 0; i < size; ++i){
		lector->read((char*)&pos, sizeof(long long));
		nl_pos->push_back( pos );
//		cout<<"["<<pos<<"]\n";
	}
	//Construir nl_skip
	prepareNewLinesSkip();
//	cout<<"Metadata::loadNewLinesUncompress - Fin\n";
}

void Metadata::saveNewLinesVarByte(fstream *escritor){
	if( escritor == NULL || ! escritor->good() ){
		cerr<<"Metadata::saveNewLinesVarByte - Error en escritor\n";
		return;
	}
//	cout<<"Metadata::saveNewLinesVarByte - Inicio\n";
	
	unsigned int size = nl_pos->size();
	escritor->write((char*)&size, sizeof(int));
	//Version VarByte
	//Buffer para vbyte (uso el peor caso, 5 bytes por numero)
	unsigned int worst_case = 5 * size;
	unsigned char *buff = new unsigned char[ worst_case ];
	unsigned int cur_byte = 0;
	BitsUtils utils;
	unsigned long long pos;
	unsigned long long last = 0;
//	cout<<"Metadata::saveNewLinesVarByte - Preparando "<<size<<" posiciones\n";
	for(unsigned int i = 0; i < size; ++i){
		pos = nl_pos->at(i) - last;
		last = nl_pos->at(i);
		
//		if( pos > max_int ){
//			cerr<<"Metadata::saveNewLinesVarByte - Error, delta > 32 bits\n";
//			break;
//		}
//		cur_byte += utils.write_varbyte(buff + cur_byte, (unsigned int)pos);
		
		cur_byte += utils.write_varbyte(buff + cur_byte, pos);
//		cout<<"["<<nl_pos->at(i)<<"] -> ["<<pos<<"]\n";
	}
//	cout<<"Metadata::saveNewLinesVarByte - Guardando "<<cur_byte<<" bytes ("<<((unsigned int*)buff)[0]<<")\n";
	escritor->write((char*)&cur_byte, sizeof(int));
	escritor->write((char*)buff, cur_byte);
	delete [] buff;
	cout<<"Metadata::saveNewLinesVarByte - Fin\n";
}

void Metadata::loadNewLinesVarByte(fstream *lector){
	if( lector == NULL || ! lector->good() ){
		cerr<<"Metadata::loadNewLinesVarByte - Error en lector\n";
		return;
	}
//	cout<<"Metadata::loadNewLinesVarByte - Inicio\n";
	
	//Datos
	nl_pos = new vector<unsigned long long>();
	unsigned int size = 0;
	lector->read((char*)&size, sizeof(int));
	//Version VarByte
	unsigned int n_bytes = 0;
	lector->read((char*)&n_bytes, sizeof(int));
//	cout<<"Metadata::loadNewLinesVarByte - Cargando "<<n_bytes<<" bytes\n";
	unsigned char *buff = new unsigned char[n_bytes];
	lector->read((char*)buff, n_bytes);
	unsigned int cur_byte = 0;
	BitsUtils utils;
	unsigned int delta_pos;
	unsigned long long pos;
	unsigned long long sum = 0;
//	cout<<"Metadata::loadNewLinesVarByte - Preparando "<<size<<" pares ("<<((unsigned int*)buff)[0]<<")\n";
	for(unsigned int i = 0; i < size; ++i){
		cur_byte += utils.read_varbyte(buff + cur_byte, delta_pos);
		pos = delta_pos + sum;
		sum = pos;
		nl_pos->push_back( pos );
//		cout<<"["<<pos<<"]\n";
	}
//	cout<<"Metadata::loadNewLinesVarByte - "<<cur_byte<<" de "<<n_bytes<<" bytes usados\n";
	delete [] buff;
	//Construir nl_skip
	prepareNewLinesSkip();
//	cout<<"Metadata::loadNewLinesVarByte - Fin\n";
}

void Metadata::saveNewLinesVarByteEx(fstream *escritor){
	if( escritor == NULL || ! escritor->good() ){
		cerr<<"Metadata::saveNewLinesVarByte - Error en escritor\n";
		return;
	}
	cout<<"Metadata::saveNewLinesVarByteEx - Inicio\n";
	
	//Revisar nl_pos para encontrar norm
	//Por ahora, uso un map <delta, cantidad>
	//Luego lo itero para encontrar el delta mas comun
//	cout<<"Metadata::saveNewLinesVarByteEx - calculando deltas\n";
	unsigned int size = nl_pos->size();
	map<unsigned int, unsigned int> mapa_deltas;
	map<unsigned int, unsigned int>::iterator it_deltas;
	unsigned long long pos;
	unsigned long long last = 0;
	for(unsigned int i = 0; i < size; ++i){
		pos = nl_pos->at(i) - last;
		last = nl_pos->at(i);
		if( pos > max_int ){
			cerr<<"Metadata::saveNewLinesVarByteEx - Error, delta > 32 bits\n";
			break;
		}
		mapa_deltas[(unsigned int)pos]++;
//		cout<<"["<<nl_pos->at(i)<<"]\n";
	}
	unsigned int nl_norm = 0;
	unsigned int mejor_conteo = 0;
	for( it_deltas = mapa_deltas.begin(); it_deltas != mapa_deltas.end(); it_deltas++ ){
//		cout<<"Metadata::saveNewLinesVarByteEx - delta <"<<it_deltas->first<<", "<<it_deltas->second<<">\n";
		if( it_deltas->second > mejor_conteo ){
			nl_norm = it_deltas->first;
			mejor_conteo = it_deltas->second;
		}
	}
//	cout<<"Metadata::saveNewLinesVarByteEx - nl_norm: "<<nl_norm<<", preparando excepciones\n";
	
	vector< pair<unsigned int, unsigned int> > nl_ex;
	last = 0;
	for(unsigned int i = 0; i < size; ++i){
		pos = nl_pos->at(i) - last;
		last = nl_pos->at(i);
		if( pos > max_int ){
			cerr<<"Metadata::saveNewLinesVarByteEx - Error, delta > 32 bits\n";
			break;
		}
		if(pos != nl_norm){
//			cout<<"Metadata::saveNewLinesVarByteEx - ex: "<<pos<<" en linea "<<i<<"\n";
			nl_ex.push_back( pair<unsigned int, unsigned int>(i, (unsigned int)pos) );
		}
	}
	
//	cout<<"Metadata::saveNewLinesVarByteEx - guardando datos previos\n";
	escritor->write((char*)&size, sizeof(int));
	escritor->write((char*)&nl_norm, sizeof(int));
	
//	cout<<"Metadata::saveNewLinesVarByteEx - Preparando guardado de excepciones\n";
	unsigned int size_ex = nl_ex.size();
	unsigned int worst_case = 5 * size_ex;
	unsigned char *buff = new unsigned char[ worst_case ];
	unsigned int cur_byte = 0;
	BitsUtils utils;
//	cout<<"Metadata::saveNewLinesVarByteEx - guardando "<<size_ex<<" excepciones\n";
	escritor->write((char*)&size_ex, sizeof(int));
	//En este ciclo, los valores (segundo elemento de nl_ex) ya esta en delta
	//Sin embargo, las posiciones tambien pueden guardarse en delta (pues son siempre crecientes)
	last = 0;
	for(unsigned int i = 0; i < size_ex; ++i){
		pos = nl_ex[i].first - last;
		last = nl_ex[i].first;
		cur_byte += utils.write_varbyte(buff + cur_byte, (unsigned int)pos);
		cur_byte += utils.write_varbyte(buff + cur_byte, nl_ex[i].second);
//		cout<<"save Ex ["<<nl_ex[i].first<<", "<<nl_ex[i].second<<"] -> ["<<pos<<", ]\n";
	}
//	cout<<"Metadata::saveNewLinesVarByteEx - Guardando "<<cur_byte<<" bytes ("<<((unsigned int*)buff)[0]<<")\n";
	escritor->write((char*)&cur_byte, sizeof(int));
	escritor->write((char*)buff, cur_byte);
	delete [] buff;
	
	cout<<"Metadata::saveNewLinesVarByteEx - Fin\n";
}

void Metadata::loadNewLinesVarByteEx(fstream *lector){
	if( lector == NULL || ! lector->good() ){
		cerr<<"Metadata::loadNewLinesVarByteEx - Error en lector\n";
		return;
	}
	cout<<"Metadata::loadNewLinesVarByteEx - Inicio\n";
	
	//Datos
	unsigned int size_nl = 0;
	unsigned int size_ex = 0;
	unsigned int nl_norm = 0;
	lector->read((char*)&size_nl, sizeof(int));
	lector->read((char*)&nl_norm, sizeof(int));
	lector->read((char*)&size_ex, sizeof(int));
	
	unsigned int n_bytes = 0;
	lector->read((char*)&n_bytes, sizeof(int));
//	cout<<"Metadata::loadNewLinesVarByteEx - Cargando "<<n_bytes<<" bytes\n";
	unsigned char *buff = new unsigned char[n_bytes];
	lector->read((char*)buff, n_bytes);
	unsigned int cur_byte = 0;
	BitsUtils utils;
	unsigned int pos_ex;
	unsigned int val_ex;
	unsigned int sum = 0;
	vector< pair<unsigned int, unsigned int> > nl_ex;
//	cout<<"Metadata::loadNewLinesVarByteEx - Preparando "<<size_ex<<" exs\n";
	for(unsigned int i = 0; i < size_ex; ++i){
		cur_byte += utils.read_varbyte(buff + cur_byte, pos_ex);
		pos_ex += sum;
		sum = pos_ex;
		cur_byte += utils.read_varbyte(buff + cur_byte, val_ex);
		nl_ex.push_back( pair<unsigned int, unsigned int>(pos_ex, val_ex) );
//		cout<<"load Ex ["<<pos_ex<<", "<<val_ex<<"]\n";
	}
//	cout<<"Metadata::loadNewLinesVarByteEx - "<<cur_byte<<" de "<<n_bytes<<" bytes usados\n";
	delete [] buff;
	
//	cout<<"Metadata::loadNewLinesVarByteEx - Reconstruyendo nl_pos con "<<size_nl<<" deltas, nl_norm: "<<nl_norm<<"\n";
	nl_pos = new vector<unsigned long long>();
	for(unsigned int i = 0; i < size_nl; ++i){
		nl_pos->push_back( nl_norm );
	}
//	cout<<"Metadata::loadNewLinesVarByteEx - Ajustando con "<<size_ex<<" excepciones\n";
	for(unsigned int i = 0; i < size_ex; ++i){
		pos_ex = nl_ex[i].first;
		val_ex = nl_ex[i].second;
		nl_pos->at(pos_ex) = val_ex;
	}
//	cout<<"Metadata::loadNewLinesVarByteEx - Acumulando deltas\n";
//	if( size_nl > 0 ){
//		cout<<"["<<nl_pos->at(0)<<"]\n";
//	}
	for(unsigned int i = 1; i < size_nl; ++i){
		nl_pos->at(i) += nl_pos->at(i - 1);
//		cout<<"["<<nl_pos->at(i)<<"]\n";
	}
	
	//Construir nl_skip
	prepareNewLinesSkip();
	cout<<"Metadata::loadNewLinesVarByteEx - Fin\n";
}

void Metadata::prepareNewLinesSkip(){
	if( nl_pos == NULL ){
		return;
	}
	if( nl_skip != NULL ){
		nl_skip->clear();
		delete nl_skip;
	}
	nl_skip = new vector<unsigned long long>();
	unsigned int pos = 0;
	while( ++pos * step_size < nl_pos->size() ){
		nl_skip->push_back( nl_pos->at( pos * step_size ) );
//		cout<<"Metadata::prepareNewLinesSkip - skip_list["<<(nl_skip->size()-1)<<"]: "<<nl_skip->back()<<"\n";
	}
}

void Metadata::loadLowcaseUncompress(BytesReader *lector){
	if( lector == NULL || ! lector->good() ){
		cerr<<"Metadata::loadLowcaseUncompress - Error en lector\n";
		return;
	}
//	cout<<"Metadata::loadLowcaseUncompress - Inicio (BytesReader)\n";
	
	//Datos
	lowcase_runs = new vector< pair<unsigned long long, unsigned long long> >();
	unsigned int size = 0;
	lector->read((char*)&size, sizeof(int));
	//Version Directa
	unsigned long long ini, fin;
//	cout<<"Metadata::loadLowcaseUncompress - Cargando "<<size<<" pares\n";
	for(unsigned int i = 0; i < size; ++i){
		lector->read((char*)&ini, sizeof(long long));
		lector->read((char*)&fin, sizeof(long long));
		lowcase_runs->push_back( pair<unsigned long long, unsigned long long>(ini, fin) );
//		cout<<"["<<ini<<", "<<fin<<"]\n";
	}
//	cout<<"Metadata::loadLowcaseUncompress - Fin\n";
}

void Metadata::loadLowcaseVarByte(BytesReader *lector){
	if( lector == NULL || ! lector->good() ){
		cerr<<"Metadata::loadLowcaseVarByte - Error en lector\n";
		return;
	}
//	cout<<"Metadata::loadLowcaseVarByte - Inicio (BytesReader)\n";
	
	//Datos
	lowcase_runs = new vector< pair<unsigned long long, unsigned long long> >();
	unsigned int size = 0;
	lector->read((char*)&size, sizeof(int));
	//Version VarByte
	unsigned int n_bytes = 0;
	lector->read((char*)&n_bytes, sizeof(int));
//	cout<<"Metadata::loadLowcaseVarByte - Cargando "<<n_bytes<<" bytes\n";
	unsigned char *buff = new unsigned char[n_bytes];
	lector->read((char*)buff, n_bytes);
	unsigned int cur_byte = 0;
	BitsUtils utils;
	unsigned int delta_ini, delta_fin;
	unsigned long long ini, fin;
	unsigned long long sum = 0;
//	cout<<"Metadata::loadLowcaseVarByte - Preparando "<<size<<" pares ("<<((unsigned int*)buff)[0]<<")\n";
	for(unsigned int i = 0; i < size; ++i){
		cur_byte += utils.read_varbyte(buff + cur_byte, delta_ini);
		cur_byte += utils.read_varbyte(buff + cur_byte, delta_fin);
		ini = delta_ini + sum;
		fin = delta_fin + ini;
		sum = fin;
		lowcase_runs->push_back( pair<unsigned long long, unsigned long long>(ini, fin) );
//		cout<<"["<<ini<<", "<<fin<<"]\n";
	}
//	cout<<"Metadata::loadLowcaseVarByte - "<<cur_byte<<" de "<<n_bytes<<" bytes usados\n";
	delete [] buff;
//	cout<<"Metadata::loadLowcaseVarByte - Fin\n";
}

void Metadata::loadNewLinesUncompress(BytesReader *lector){
	if( lector == NULL || ! lector->good() ){
		cerr<<"Metadata::loadNewLinesUncompress - Error en lector\n";
		return;
	}
//	cout<<"Metadata::loadNewLinesUncompress - Inicio (BytesReader)\n";
	
	//Datos
	nl_pos = new vector<unsigned long long>();
	unsigned int size = 0;
	lector->read((char*)&size, sizeof(int));
	//Version Directa
	unsigned long long pos;
//	cout<<"Metadata::loadNewLinesUncompress - Cargando "<<size<<" posiciones\n";
	for(unsigned int i = 0; i < size; ++i){
		lector->read((char*)&pos, sizeof(long long));
		nl_pos->push_back( pos );
//		cout<<"["<<pos<<"]\n";
	}
	//Construir nl_skip
	prepareNewLinesSkip();
//	cout<<"Metadata::loadNewLinesUncompress - Fin\n";
}

void Metadata::loadNewLinesVarByte(BytesReader *lector){
	if( lector == NULL || ! lector->good() ){
		cerr<<"Metadata::loadNewLinesVarByte - Error en lector\n";
		return;
	}
//	cout<<"Metadata::loadNewLinesVarByte - Inicio (BytesReader)\n";
	
	//Datos
	nl_pos = new vector<unsigned long long>();
	unsigned int size = 0;
	lector->read((char*)&size, sizeof(int));
	//Version VarByte
	unsigned int n_bytes = 0;
	lector->read((char*)&n_bytes, sizeof(int));
//	cout<<"Metadata::loadNewLinesVarByte - Cargando "<<n_bytes<<" bytes\n";
	unsigned char *buff = new unsigned char[n_bytes];
	lector->read((char*)buff, n_bytes);
	unsigned int cur_byte = 0;
	BitsUtils utils;
	unsigned int delta_pos;
	unsigned long long pos;
	unsigned long long sum = 0;
//	cout<<"Metadata::loadNewLinesVarByte - Preparando "<<size<<" pares ("<<((unsigned int*)buff)[0]<<")\n";
	for(unsigned int i = 0; i < size; ++i){
		cur_byte += utils.read_varbyte(buff + cur_byte, delta_pos);
		pos = delta_pos + sum;
		sum = pos;
		nl_pos->push_back( pos );
//		cout<<"["<<pos<<"]\n";
	}
//	cout<<"Metadata::loadNewLinesVarByte - "<<cur_byte<<" de "<<n_bytes<<" bytes usados\n";
	delete [] buff;
	//Construir nl_skip
	prepareNewLinesSkip();
//	cout<<"Metadata::loadNewLinesVarByte - Fin\n";
}









