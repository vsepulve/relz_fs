#include "MetadataFasta.h"

MetadataFasta::MetadataFasta(){
	metadata_text = NULL;
	metadata_length = 0;
}

MetadataFasta::~MetadataFasta(){
	if( metadata_text != NULL ){
		delete [] metadata_text;
		metadata_text = NULL;
	}
	metadata_length = 0;
}
	
unsigned char* MetadataFasta::CompressWithLzma(const char *text, size_t length, int level, size_t &output_length){
//std::string MetadataFasta::CompressWithLzma(const std::string& in, int level) {
	
	size_t worst_space = length + (length >> 2) + 128;
	unsigned char *tmp_buff = new unsigned char[worst_space];
	cout << "CompressWithLzma - worst_space: " << worst_space << "\n";
	output_length = 0;
	int res = lzma_easy_buffer_encode(
			level, LZMA_CHECK_CRC32, NULL,
//			static_cast<const unsigned char*>(text), 
			(const unsigned char*)text, 
			length,
			tmp_buff, 
			&output_length, 
			worst_space
	);
	if( res != LZMA_OK ){
		cerr << "CompressWithLzma - Error while compressing\n";
		delete [] tmp_buff;
		output_length = 0;
		return NULL;
	}
	cout << "CompressWithLzma - final space: " << output_length << "\n";
	unsigned char *result = new unsigned char[output_length + 1];
	memcpy(result, tmp_buff, output_length);
	result[output_length] = 0;
	delete [] tmp_buff;
	return result;

//  std::string result;
//  result.resize(in.size() + (in.size() >> 2) + 128);
//  cout << "CompressWithLzma - worst out_pos: " << (in.size() + (in.size() >> 2) + 128) << "\n";
//  size_t out_pos = 0;
//  if (LZMA_OK != lzma_easy_buffer_encode(
//      level, LZMA_CHECK_CRC32, NULL,
//      reinterpret_cast<uint8_t*>(const_cast<char*>(in.data())), in.size(),
//      reinterpret_cast<uint8_t*>(&result[0]), &out_pos, result.size()))
//    abort();
//  cout << "CompressWithLzma - out_pos: " << out_pos << "\n";
//  result.resize(out_pos);
//	return result;
}

//std::string MetadataFasta::DecompressWithLzma(const std::string& in) {
char * MetadataFasta::DecompressWithLzma(const unsigned char *input, size_t length, size_t &output_length) {
	static const size_t kMemLimit = 1 << 30;  // 1 GB.
	lzma_stream strm = LZMA_STREAM_INIT;
	std::string result;
	result.resize(8192);
	size_t result_used = 0;
	lzma_ret ret;
	ret = lzma_stream_decoder(&strm, kMemLimit, LZMA_CONCATENATED);
	if (ret != LZMA_OK){
		abort();
	}
	size_t avail0 = result.size();
//	strm.next_in = reinterpret_cast<const uint8_t*>(in.data());
	strm.next_in = input;
//	strm.avail_in = in.size();
	strm.avail_in = length;
	strm.next_out = reinterpret_cast<uint8_t*>(&result[0]);
	strm.avail_out = avail0;
	while (true) {
		ret = lzma_code(&strm, strm.avail_in == 0 ? LZMA_FINISH : LZMA_RUN);
		if (ret == LZMA_STREAM_END) {
			result_used += avail0 - strm.avail_out;
			// Guaranteed by lzma_stream_decoder().
			if (0 != strm.avail_in)  {
				abort();
			}
			result.resize(result_used);
			lzma_end(&strm);
//			return result;
			output_length = result.size();
			char *res = new char[output_length + 1];
			memcpy(res, &result[0], output_length);
			res[output_length] = 0;
			return res;
		}
		if (ret != LZMA_OK){
			abort();
		}
		if (strm.avail_out == 0) {
			result_used += avail0 - strm.avail_out;
			result.resize(result.size() << 1);
			strm.next_out = reinterpret_cast<uint8_t*>(&result[0] + result_used);
			strm.avail_out = avail0 = result.size() - result_used;
		}
	}
}

unsigned long long MetadataFasta::filterMetadata(char *text, unsigned long long text_length){
	
	cout << "MetadataFasta::filterMetadata - Start (text_length: " << text_length << ")\n";
	
	// Por ahora preparo dos buffers internos de todo el texto para separar y despues copiar de vuelta el texto filtrado
	char *buff_text = new char[text_length + 1];
	buff_text[0] = 0;
	char *buff_metadata = new char[text_length + 1];
	buff_metadata[0] = 0;
	
	unsigned int buff_size = 64*1024;
	char buff[buff_size];
	
	unsigned long long read_pos = 0;
	unsigned long long text_pos = 0;
	unsigned long long metadata_pos = 0;
	
//	vector<unsigned int> pos_text;
//	vector<unsigned int> pos_storage;
//	vector<unsigned int> length_line
	
	while( read_pos < text_length ){
		
		cout << "MetadataFasta::filterMetadata - Reading from " << read_pos << " / " << text_length << "\n";
		
		// getline
		unsigned long long line_start = read_pos;
		unsigned int buff_pos = 0;
		for( ; read_pos < text_length; ++read_pos ){
			buff[buff_pos++] = text[read_pos];
			if( text[read_pos] == '\n' ){
				++read_pos;
				break;
			}
		}
		buff[buff_pos] = 0;
//		cout << "MetadataFasta::filterMetadata - Line \"" << buff << "\"\n";
		
		if( (buff_pos > 0) && (buff[0] == ';' || buff[0] == '>' || buff[0] == '#') ){
//			cout << "MetadataFasta::filterMetadata - Adding to metadata\n";
			if( buff[buff_pos-1] == '\n' ){
				--buff_pos;
				buff[buff_pos] = 0;
			}
			
			pos_text.push_back(line_start);
			pos_storage.push_back(metadata_pos);
			length_line.push_back(buff_pos);
			
			memcpy(buff_metadata + metadata_pos, buff, buff_pos);
			metadata_pos += buff_pos;
			buff_metadata[metadata_pos] = 0;
		}
		else{
//			cout << "MetadataFasta::filterMetadata - Adding to text\n";
			memcpy(buff_text + text_pos, buff, buff_pos);
			text_pos += buff_pos;
			buff_text[text_pos] = 0;
		}
		
	}
	
//	cout << "MetadataFasta::filterMetadata - Resulting metadata (" << metadata_pos << "): \"" << buff_metadata << "\"\n";
//	cout << "MetadataFasta::filterMetadata - Resulting text (" << text_pos << "): \"" << buff_text << "\"\n";
	
	cout << "MetadataFasta::filterMetadata - Stored lines: " << pos_text.size() << "\n";
	for( unsigned int i = 0; i < ( (pos_text.size()<20)?pos_text.size():20 ); ++i ){
		cout << "MetadataFasta::filterMetadata - Metadata[" << i << "]: (" << pos_text[i] << ", " << pos_storage[i] << ", " << length_line[i] << ")\n";
	}
	
	// Devolver texto filtrado y borrar buffer
	memcpy( text, buff_text, text_pos );
	text[text_pos] = 0;
	delete [] buff_text;
	
	// Almacenar texto de metadata y borrar buffer
	metadata_length = metadata_pos;
	metadata_text = new char[metadata_length + 1];
	memcpy( metadata_text, buff_metadata, metadata_length );
	metadata_text[metadata_length] = 0;
	delete [] buff_metadata;
	
	return text_pos;
}
	
void MetadataFasta::save(fstream *writer){

	cout << "MetadataFasta::save - Start\n";
	
	unsigned int n_lines = pos_text.size();
	writer->write((char*)&n_lines, sizeof(int));
	
	BitsUtils utils;
	unsigned long long last;
	unsigned int delta;
	
	// El worst case es el mismo para los 3 casos: 5 bytes por delta
	unsigned int worst_case = 5*n_lines;
	unsigned char *buff = new unsigned char[worst_case];
	unsigned int cur_byte;
	
	last = 0;
	cur_byte = 0;
	for( unsigned int i = 0; i < n_lines; ++i ){
		delta = (unsigned int)(pos_text[i] - last);
		last = pos_text[i];
		cur_byte += utils.write_varbyte(buff + cur_byte, delta);
	}
	writer->write((char*)&cur_byte, sizeof(int));
	writer->write((char*)buff, cur_byte);
	
	last = 0;
	cur_byte = 0;
	for( unsigned int i = 0; i < n_lines; ++i ){
		delta = (unsigned int)(pos_storage[i] - last);
		last = pos_storage[i];
		cur_byte += utils.write_varbyte(buff + cur_byte, delta);
	}
	writer->write((char*)&cur_byte, sizeof(int));
	writer->write((char*)buff, cur_byte);
	
	cur_byte = 0;
	for( unsigned int i = 0; i < n_lines; ++i ){
		cur_byte += utils.write_varbyte(buff + cur_byte, length_line[i]);
	}
	writer->write((char*)&cur_byte, sizeof(int));
	writer->write((char*)buff, cur_byte);
	
	delete [] buff;
	
	char compression_mark = 0;
	size_t compressed_size = 0;
	unsigned char *compressed = CompressWithLzma(metadata_text, (size_t)metadata_length, 6, compressed_size);
	if( compressed == NULL || (compressed_size > (size_t)metadata_length) ){
		cout << "MetadataFasta::save - Saving metadata text UNCOMPRESSED\n";
		writer->write(&compression_mark, 1);
		writer->write((char*)&metadata_length, sizeof(long long));
		writer->write(metadata_text, metadata_length);
	}
	else{
		cout << "MetadataFasta::save - Saving metadata text LZMA\n";
		compression_mark = 1;
		writer->write(&compression_mark, 1);
		writer->write((char*)&compressed_size, sizeof(size_t));
		writer->write((char*)compressed, compressed_size);
	}
	if( compressed != NULL ){
		delete [] compressed;
	}
	
	cout << "MetadataFasta::save - End\n";
	
}

void MetadataFasta::load(fstream *reader){
	cout << "MetadataFasta::load - Start\n";
	
	unsigned int n_lines = 0;
	reader->read((char*)&n_lines, sizeof(int));
	cout << "MetadataFasta::load - n_lines: " << n_lines << "\n";
	
//	cout << "MetadataFasta::load - Loading pos_text\n";
	
	BitsUtils utils;
	unsigned long long acum;
	unsigned int delta;
	
	// El worst case es el mismo para los 3 casos: 5 bytes por delta
	unsigned int worst_case = 5*n_lines;
	unsigned char *buff = new unsigned char[worst_case];
	unsigned int cur_byte;
	unsigned int n_bytes;
	
	n_bytes = 0;
	reader->read((char*)&n_bytes, sizeof(int));
	if( n_bytes > worst_case ){
		cerr << "MetadataFasta::load - Error, n_bytes > worst_case (" << n_bytes << " / " << worst_case << ")\n";
		return;
	}
	reader->read((char*)buff, n_bytes);
	cur_byte = 0;
	acum = 0;
	for( unsigned int i = 0; i < n_lines; ++i ){
		cur_byte += utils.read_varbyte(buff + cur_byte, delta);
		acum += delta;
		pos_text.push_back(acum);
	}
	
	n_bytes = 0;
	reader->read((char*)&n_bytes, sizeof(int));
	if( n_bytes > worst_case ){
		cerr << "MetadataFasta::load - Error, n_bytes > worst_case (" << n_bytes << " / " << worst_case << ")\n";
		return;
	}
	reader->read((char*)buff, n_bytes);
	cur_byte = 0;
	acum = 0;
	for( unsigned int i = 0; i < n_lines; ++i ){
		cur_byte += utils.read_varbyte(buff + cur_byte, delta);
		acum += delta;
		pos_storage.push_back(acum);
	}
	
	n_bytes = 0;
	reader->read((char*)&n_bytes, sizeof(int));
	if( n_bytes > worst_case ){
		cerr << "MetadataFasta::load - Error, n_bytes > worst_case (" << n_bytes << " / " << worst_case << ")\n";
		return;
	}
	reader->read((char*)buff, n_bytes);
	cur_byte = 0;
	for( unsigned int i = 0; i < n_lines; ++i ){
		cur_byte += utils.read_varbyte(buff + cur_byte, delta);
		length_line.push_back(delta);
	}
	
//	cout << "MetadataFasta::load - Preparing Metadata text\n";
	char compression_mark = 0;
	metadata_length = 0;
	if(metadata_text != NULL){
		delete [] metadata_text;
	}
	reader->read(&compression_mark, 1);
	if( compression_mark == 1 ){
		cout << "MetadataFasta::load - Reading text LZMA\n";
		size_t compressed_size = 0;
		reader->read((char*)&compressed_size, sizeof(size_t));
		cout << "MetadataFasta::load - compressed_size: " << compressed_size << "\n";
		unsigned char *compressed = new unsigned char[compressed_size + 1];
		reader->read((char*)compressed, compressed_size);
		compressed[compressed_size] = 0;
		size_t uncompressed_size = 0;
		cout << "MetadataFasta::load - Decompressing...\n";
		metadata_text = DecompressWithLzma(compressed, compressed_size, uncompressed_size);
		metadata_length = uncompressed_size;
		cout << "MetadataFasta::load - Deleting compressed buffer\n";
		delete [] compressed;
	}
	else{
		cout << "MetadataFasta::load - Reading text UNCOMPRESSED\n";
		reader->read((char*)&metadata_length, sizeof(long long));
		cout << "MetadataFasta::load - metadata_length: " << metadata_length << "\n";
		metadata_text = new char[metadata_length + 1];
		reader->read(metadata_text, metadata_length);
		metadata_text[metadata_length] = 0;
	}
	
	
//	cout << "MetadataFasta::load - Resulting metadata (" << metadata_length << "): \"" << metadata_text << "\"\n";
//	cout << "MetadataFasta::load - Resulting text (" << text_pos << "): \"" << buff_text << "\"\n";
	
	cout << "MetadataFasta::load - Stored lines: " << pos_text.size() << "\n";
	for( unsigned int i = 0; i < ( (pos_text.size()<20)?pos_text.size():20 ); ++i ){
		cout << "MetadataFasta::load - Metadata[" << i << "]: (" << pos_text[i] << ", " << pos_storage[i] << ", " << length_line[i] << ")\n";
	}
	
//	cout << "MetadataFasta::load - Testing countTextBin\n";
//	for(unsigned int i = 0; i < 100000; ++i){
//		unsigned long long n1 = countText(i);
//		unsigned long long n2 = countTextBin(i);
//		if( n1 != n2 ){
//			cout << "MetadataFasta::load - Error (" << n2 << " != " << n1 << ")\n";
//			exit(0);
//		}
//	}
	
	cout << "MetadataFasta::load - End\n";
	
}
	
unsigned int MetadataFasta::size(){
	
	unsigned int size_bytes = 0;
	
	// n_lines
	size_bytes += sizeof(int);
	
	BitsUtils utils;
	unsigned long long last;
	unsigned int delta;
	
	// pos_text
//	size_bytes += pos_text.size() * sizeof(long long);
	// => n_bytes + sum bytes de cada delta de pos_text
	size_bytes += sizeof(int);
	last = 0;
	for( unsigned int i = 0; i < pos_text.size(); ++i ){
		delta = (unsigned int)(pos_text[i] - last);
		last = pos_text[i];
		size_bytes += utils.size_varbyte(delta);
	}
	
	
	// pos_storage
//	size_bytes += pos_storage.size() * sizeof(long long);
	// => n_bytes + sum bytes de cada delta de pos_storage
	size_bytes += sizeof(int);
	last = 0;
	for( unsigned int i = 0; i < pos_storage.size(); ++i ){
		delta = (unsigned int)(pos_storage[i] - last);
		last = pos_storage[i];
		size_bytes += utils.size_varbyte(delta);
	}
	
	// length_line
//	size_bytes += length_line.size() * sizeof(int);
	// => n_bytes + sum bytes de cada length_line
	size_bytes += sizeof(int);
	for( unsigned int i = 0; i < length_line.size(); ++i ){
		size_bytes += utils.size_varbyte(length_line[i]);
	}
	
	unsigned int size_bytes_nums = size_bytes;
	
	// compression_mark
	size_bytes += 1;
	
	size_t compressed_size = 0;
	unsigned char *compressed = CompressWithLzma(metadata_text, (size_t)metadata_length, 6, compressed_size);
	if( compressed == NULL || (compressed_size > (size_t)metadata_length) ){
		cout << "MetadataFasta::size - UNCOMPRESSED\n";
		
		// metadata_length
		size_bytes += sizeof(long long);
		
		// metadata_text
		size_bytes += metadata_length;
	}
	else{
		cout << "MetadataFasta::size - LZMA\n";
		
		// compressed_size
		size_bytes += sizeof(size_t);
		
		// compressed
		size_bytes += compressed_size;
	}
	if( compressed != NULL ){
		delete [] compressed;
	}
	
	cout << "MetadataFasta::size - Total Size: " << size_bytes << " (numbers: " << size_bytes_nums << " (" << (double)size_bytes_nums/(3*pos_text.size()) << " bytes/num), text: " << (size_bytes - size_bytes_nums) << ")\n";
	
	return size_bytes;
}
	
	
void MetadataFasta::adjustText(char *out_buff, unsigned long long pos_ini, unsigned int copied_chars, char *adjust_buffer){
	
	cout << "MetadataFasta::adjustText - Start (pos_ini: " << pos_ini << ", copied_chars: " << copied_chars << ")\n";
	
	cout << "MetadataFasta::adjustText - Original text: \"" << out_buff << "\"\n";
	
	unsigned int write_pos = 0;
	unsigned int read_pos = 0;
	bool first = true;
	unsigned int special_copy = 0;
	
	
//	// Version Binaria
//	unsigned int l = 0;
//	unsigned int h = pos_text.size() - 1;
//	unsigned int m;
//	while(l < h){
//		m = l + ((h-l)>>1);
//		if( pos_text[m] < pos_ini ){
//			l = m+1;
//		}
//		else{
//			h = m;
//		}
//	}
//	unsigned int line_pos = h;
	
	for( unsigned int line_pos = 0; line_pos < pos_text.size(); ++line_pos ){
//	for( ; line_pos < pos_text.size(); ++line_pos ){
		// Obviamente el primer paso se saca de una busqueda binaria simple
		// Aqui estoy perdiendo la cola de potenciales metadatos previos, hay que considerar el largo tambien
		
		if( pos_text[line_pos] < pos_ini ){
			if( pos_text[line_pos] + length_line[line_pos] >= pos_ini ){
				// Caso especial, escribir la cola de una linea de metadata cortada
				cout << "MetadataFasta::adjustText - Special case, adding truncated metadata\n";
				special_copy = pos_text[line_pos] + length_line[line_pos] - pos_ini;
				unsigned long long pos = pos_storage[line_pos] + length_line[line_pos] - special_copy;
				cout << "MetadataFasta::adjustText - Adding " << special_copy << " chars from metadata (+1 newline) from pos " << pos << " (" << pos_storage[line_pos] << " + " << length_line[line_pos] << " - " << special_copy << ")\n";
				memcpy(adjust_buffer + write_pos, metadata_text + pos, special_copy);
				write_pos += special_copy;
				adjust_buffer[write_pos++] = '\n';
				++special_copy;
			}
			continue;
		}
		if( pos_text[line_pos] >= pos_ini + copied_chars ){
			break;
		}
		
		// Copia de chars del texto previo al metadata
		unsigned int copy_len = pos_text[line_pos];
		cout << "MetadataFasta::adjustText - Initial copy_len: " << pos_text[line_pos] << "\n";
		if( first ){
			if( pos_ini > copy_len ){
				copy_len = 0;
			}
			else{
				copy_len -= pos_ini;
			}
			if( special_copy > copy_len ){
				copy_len = 0;
			}
			else{
				copy_len -= special_copy;
			}
			cout << "MetadataFasta::adjustText - Mod copy_len: " << pos_text[line_pos] << " - " << pos_ini << " - " << special_copy << "\n";
		}

//		unsigned int copy_len = pos_text[line_pos];
//		cout << "MetadataFasta::adjustText - Initial copy_len: " << copy_len << "\n";
		if( ! first ){
			// Descuento la linea anterior
			copy_len -= (pos_text[line_pos-1] + length_line[line_pos-1] + 1);
			cout << "MetadataFasta::adjustText - Adjusted copy_len: " << copy_len << " (ajuste: - " << pos_text[line_pos-1] << " - " << length_line[line_pos-1] << " - 1)\n";
		}
		
		first = false;
		
		cout << "MetadataFasta::adjustText - Adding " << copy_len << " chars from original text\n";
		memcpy(adjust_buffer + write_pos, out_buff + read_pos, copy_len);
		write_pos += copy_len;
		read_pos += copy_len;
		cout << "MetadataFasta::adjustText - Adding " << length_line[line_pos] << " chars from metadata (+1 newline)\n";
		memcpy(adjust_buffer + write_pos, metadata_text + pos_storage[line_pos], length_line[line_pos]);
		write_pos += length_line[line_pos];
		adjust_buffer[write_pos++] = '\n';
		adjust_buffer[write_pos] = 0;
		// Condicion de salida si ya escribio lo suficiente?
		// Quizas recibir el largo esperado del texto de salida
	}
	
//	cout << "MetadataFasta::adjustText - adjust_buffer: \"" << adjust_buffer << "\" (write_pos: " << write_pos << ", strlen: " << strlen(adjust_buffer) << ")\n";
	
	// Agergar la cola del texto
	if( write_pos < copied_chars ){
		cout << "MetadataFasta::adjustText - Adding " << (copied_chars - write_pos) << " chars from original text to finish\n";
		memcpy(adjust_buffer + write_pos, out_buff + read_pos, copied_chars - write_pos);
		write_pos = copied_chars;
		adjust_buffer[write_pos] = 0;
	}
	
//	cout << "MetadataFasta::adjustText - adjust_buffer (final): \"" << adjust_buffer << "\"\n";
	
	// Si habiamos escrito mas, basta con desecharlo
	write_pos = copied_chars;
	
	// Devolver texto a la salida
	memcpy(out_buff, adjust_buffer, write_pos);
	out_buff[write_pos] = 0;
	
	cout << "MetadataFasta::adjustText - Resulting text: \"" << out_buff << "\"\n";
	
	
	cout << "MetadataFasta::adjustText - End\n";

}

unsigned long long MetadataFasta::countText(unsigned long long pos){

//	cout << "MetadataFasta::countText - Start (pos: " << pos << ")\n";
	
	unsigned long long res = 0;
	for( unsigned int i = 0; i < pos_text.size(); ++i ){
		if( pos_text[i] >= pos ){
			break;
		}
		unsigned long long ini = pos_text[i];
		// agreggo el newline
		unsigned int len = length_line[i] + 1;
		
		// pos: 10, ini: 7, len: 6 => len deseado 3
		if( ini + len > pos ){
			len = pos - ini;
		}
		res += len;
	}
	
//	cout << "MetadataFasta::countText - End (res: " << res << ")\n";
	
	return res;
}

unsigned long long MetadataFasta::countTextBin(unsigned long long pos){

//	cout << "MetadataFasta::countTextBin - Start (pos: " << pos << ")\n";
	
	if( pos >= (pos_text.back() + length_line.back() + length_line.size()) ){
//		cout << "MetadataFasta::countTextBin - Greater than collection\n";
		return (pos_storage.back() + length_line.back() + length_line.size());
	}
	
	// Version Binaria
	unsigned int l = 0;
	unsigned int h = pos_text.size() - 1;
	unsigned int m;
	while(l < h){
		m = l + ((h-l)>>1);
		if( pos_text[m] < pos ){
			l = m+1;
		}
		else{
			h = m;
		}
	}
//	cout<<"MetadataFasta::countTextBin - BB finished (h: " << h << " por " << pos_text[h] << " / " << pos << ")\n";
	if( (h > 0) && (pos_text[h] + 1) > pos ){
//		cout<<"MetadataFasta::countTextBin - Adjusting\n";
		--h;
	}
	
//	cout<<"MetadataFasta::countTextBin - h: " << h << ", pos_text: " << pos_text[h] << ", pos_storage: " << pos_storage[h] << ", length_line: " << length_line[h] << "\n";
	
	unsigned long long res = pos_storage[h] + h;
	
//	cout<<"MetadataFasta::countTextBin - " << pos << " < " << (pos_text[h] + length_line[h] + h + 1) << "? por h: " << h << "\n";
	if( pos < (pos_text[h] + length_line[h] + 1) ){
//		cout<<"MetadataFasta::countTextBin - Adding " << (pos - pos_text[h]) << "\n";
		res +=  (pos - pos_text[h]);
	}
	else{
		res += length_line[h] + 1;
	}
	
//	cout << "MetadataFasta::countTextBin - End (res: " << res << ")\n";
	
	return res;
}















