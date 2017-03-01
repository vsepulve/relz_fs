#include "LinearPatternMatching.h"

LinearPatternMatching::LinearPatternMatching(){
}

LinearPatternMatching::~LinearPatternMatching(){
}

// delta1 table: delta1[c] contains the distance between the last
// character of pat and the rightmost occurrence of c in pat.
// If c does not occur in pat, then delta1[c] = patlen.
// If c is at string[i] and c != pat[patlen-1], we can
// safely shift i over by delta1[c], which is the minimum distance
// needed to shift pat forward to get string[i] lined up 
// with some character in pat.
// this algorithm runs in alphabet_len+patlen time.
void LinearPatternMatching::make_delta1(unsigned char *pat, int patlen) {
	int i;
	for (i = 0; i < ALPHABET_LEN; ++i) {
		delta1[i] = NOT_FOUND;
	}
	for (i = 0; i < patlen-1; ++i) {
		delta1[pat[i]] = patlen-1 - i;
	}
}

// true if the suffix of word starting from word[pos] is a prefix 
// of word
int LinearPatternMatching::is_prefix(unsigned char *word, int wordlen, int pos) {
	int i;
	int suffixlen = wordlen - pos;
	// could also use the strncmp() library function here
	for (i = 0; i < suffixlen; ++i) {
		if (word[i] != word[pos+i]) {
			return 0;
		}
	}
	return 1;
}

// length of the longest suffix of word ending on word[pos].
// suffix_length("dddbcabc", 8, 4) = 2
int LinearPatternMatching::suffix_length(unsigned char *word, int wordlen, int pos) {
	int i;
	// increment suffix length i to the first mismatch or beginning
	// of the word
	for (i = 0; (word[pos-i] == word[wordlen-1-i]) && (i < pos); ++i);
	return i;
}

// delta2 table: given a mismatch at pat[pos], we want to align 
// with the next possible full match could be based on what we
// know about pat[pos+1] to pat[patlen-1].
//
// In case 1:
// pat[pos+1] to pat[patlen-1] does not occur elsewhere in pat,
// the next plausible match starts at or after the mismatch.
// If, within the substring pat[pos+1 .. patlen-1], lies a prefix
// of pat, the next plausible match is here (if there are multiple
// prefixes in the substring, pick the longest). Otherwise, the
// next plausible match starts past the character aligned with 
// pat[patlen-1].
// 
// In case 2:
// pat[pos+1] to pat[patlen-1] does occur elsewhere in pat. The
// mismatch tells us that we are not looking at the end of a match.
// We may, however, be looking at the middle of a match.
// 
// The first loop, which takes care of case 1, is analogous to
// the KMP table, adapted for a 'backwards' scan order with the
// additional restriction that the substrings it considers as 
// potential prefixes are all suffixes. In the worst case scenario
// pat consists of the same letter repeated, so every suffix is
// a prefix. This loop alone is not sufficient, however:
// Suppose that pat is "ABYXCDBYX", and text is ".....ABYXCDEYX".
// We will match X, Y, and find B != E. There is no prefix of pat
// in the suffix "YX", so the first loop tells us to skip forward
// by 9 characters.
// Although superficially similar to the KMP table, the KMP table
// relies on information about the beginning of the partial match
// that the BM algorithm does not have.
//
// The second loop addresses case 2. Since suffix_length may not be
// unique, we want to take the minimum value, which will tell us
// how far away the closest potential match is.
//void make_delta2(int *delta2, uint8_t *pat, int32_t patlen) {
void LinearPatternMatching::make_delta2(int *delta2, unsigned char *pat, int patlen) {
	int p;
	int last_prefix_index = patlen-1;

	// first loop
	for (p = patlen-1; p >= 0; --p) {
		if (is_prefix(pat, patlen, p+1)) {
			last_prefix_index = p+1;
		}
		delta2[p] = last_prefix_index + (patlen-1 - p);
	}

	// second loop
	for (p=0; p < patlen-1; ++p) {
		int slen = suffix_length(pat, patlen, p);
		if (pat[p - slen] != pat[patlen-1 - slen]) {
			delta2[patlen-1 - slen] = patlen-1 - p + slen;
		}
	}
}

// Boyer Moore modificado para continuar la busqueda y agregar todos los resultados a res
// Esta version almacena TODAS las ocurrencias (incluidos overlap)
unsigned int LinearPatternMatching::search(unsigned char *text, unsigned int stringlen, 
											unsigned char *pat, unsigned int patlen, vector<unsigned int> &res) {

//	cout<<"LinearPatternMatching::search - Inicio ("<<stringlen<<", "<<patlen<<")\n";
			
	// The empty pattern must be considered specially
	if (patlen == 0) {
		return 0;
	}
	
	// El espacio de delta1 puede ser reusado (debe ser de la instancia)
	// delta2 podría ser reusado, pero no creo que sea necesario (al menos para patrones cortos)
//	int delta1[ALPHABET_LEN];
	int delta2[patlen];
	
//	cout<<"LinearPatternMatching::search - make_delta1...\n";
	make_delta1(pat, patlen);
//	cout<<"LinearPatternMatching::search - make_delta2...\n";
	make_delta2(delta2, pat, patlen);
	
	unsigned int i;
	unsigned int n_pos = 0;

//	cout<<"LinearPatternMatching::search - Buscando\n";
	
	i = patlen-1;
	while (i < stringlen) {
		int j = patlen-1;
		while (j >= 0 && (text[i] == pat[j])) {
			--i;
			--j;
		}
		if (j < 0) {
			
//			cout<<"Pos: "<<(i+1)<<" (\""<<string((const char*)text+(i+1), patlen)<<"\")\n";
			res.push_back(i+1);
			++n_pos;
			
			// Reinitialize j to restart search
			j = patlen-1;
			
			// Reinitialize i to start at i+1+patlen
			i += patlen +1; // (not completely sure of that +1)
			
			continue;
		}

		i += max(delta1[text[i]], delta2[j]);
	}
	
//	cout<<"LinearPatternMatching::search - Fin ("<<n_pos<<")\n";
	return n_pos;
}

// Boyer Moore modificado para continuar la busqueda y agregar todos los resultados a res
// Version No Overlap
unsigned int LinearPatternMatching::searchNO(unsigned char *text, unsigned int stringlen, 
											unsigned char *pat, unsigned int patlen, vector<unsigned int> &res) {

	// The empty pattern must be considered specially
	if (patlen == 0) {
		return 0;
	}
	
	// El espacio de delta1 puede ser reusado (debe ser de la instancia)
	// delta2 podría ser reusado, pero no creo que sea necesario (al menos para patrones cortos)
//	int delta1[ALPHABET_LEN];
	int delta2[patlen];
	
	unsigned int i;
	make_delta1(pat, patlen);
	make_delta2(delta2, pat, patlen);
	unsigned int n_pos = 0;

	i = patlen-1;
	while (i < stringlen) {
		int j = patlen-1;
		while (j >= 0 && (text[i] == pat[j])) {
			--i;
			--j;
		}
		if (j < 0) {
			
//			if ( (n_pos > 0) && (res.back() + patlen < i+1)){
			if ( (n_pos > 0) && (res.back() + patlen > i+1)){
				// Overlap, no almacenar
//				cout<<"Pos Omitida: "<<(i+1)<<" (\""<<string((const char*)text+(i+1), patlen)<<"\") ("<<res.back()<<" + "<<patlen<<" !< "<<i+1<<")\n";
			}
			else{
//				cout<<"Pos: "<<(i+1)<<" (\""<<string((const char*)text+(i+1), patlen)<<"\")\n";
				res.push_back(i+1);
				++n_pos;
			}
			
			// Reinitialize j to restart search
			j = patlen-1;
			
			// Reinitialize i to start at i+1+patlen
			i += patlen+1; // (not completely sure of that +1)
			
			continue;
		}

		i += max(delta1[text[i]], delta2[j]);
	}
	return n_pos;
}





