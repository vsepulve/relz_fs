#ifndef _COMPARATOR_UTILS_H
#define _COMPARATOR_UTILS_H

#include <stdio.h>
#include <stdlib.h>
#include <iostream>

using namespace std;

//Coparador Basico de texto para Suffix Array
class SAComparator : public std::binary_function<unsigned int, unsigned int, bool> {
private:
	unsigned char *ref;
	unsigned int largo;
public:
	SAComparator(){
		ref = NULL;
		largo = 0;
	}
	SAComparator(unsigned char *_ref, unsigned int _largo){
		ref = _ref;
		largo = _largo;
	}
	inline bool operator()(const unsigned int a, const unsigned int b){
		//return lexicographical_compare(&(ref[a]), &(ref[a])+(largo-a), &(ref[b]), &(ref[b])+(largo-b));
		if( a > largo ){
			return true;
		}
		if( b > largo ){
			return false;
		}
		unsigned char *p1 = ref + a;
		unsigned char *p2 = ref + b;
		while( *p1 != 0 && *p1 == *p2 ){
			++p1;
			++p2;
		}
		return (*p1 < *p2);
	}
};

//Comparador para el primer nivel de buckets (omite 1 caracter)
class SAComparatorN1 : public std::binary_function<unsigned int, unsigned int, bool> {
private:
	unsigned char *ref;
	unsigned int largo;
public:
	SAComparatorN1(){
		ref = NULL;
		largo = 0;
	}
	SAComparatorN1(unsigned char *_ref, unsigned int _largo){
		ref = _ref;
		largo = _largo;
	}
	inline bool operator()(const unsigned int a, const unsigned int b){
//		return lexicographical_compare(&(ref[a])+1, &(ref[a])+(largo-a), &(ref[b])+1, &(ref[b])+(largo-b));
		//En este caso debe verificarse si hay string nulos
		if( a > largo-1 ){
			return true;
		}
		if( b > largo-1 ){
			return false;
		}
		unsigned char *p1 = ref + a + 1;
		unsigned char *p2 = ref + b + 1;
		while( *p1 != 0 && *p1 == *p2 ){
			++p1;
			++p2;
		}
		return (*p1 < *p2);
	}
};

//Comparador para el segundo nivel de buckets (omite 2 caracter)
class SAComparatorN2 : public std::binary_function<unsigned int, unsigned int, bool> {
private:
	unsigned char *ref;
	unsigned int largo;
public:
	SAComparatorN2(){
		ref = NULL;
		largo = 0;
	}
	SAComparatorN2(unsigned char *_ref, unsigned int _largo){
		ref = _ref;
		largo = _largo;
	}
	inline bool operator()(const unsigned int a, const unsigned int b){
		//return lexicographical_compare(&(ref[a])+2, &(ref[a])+(largo-a), &(ref[b])+2, &(ref[b])+(largo-b));
		//En este caso debe verificarse si hay string nulos
		if( a > largo-2 ){
			return true;
		}
		if( b > largo-2 ){
			return false;
		}
		unsigned char *p1 = ref + a + 2;
		unsigned char *p2 = ref + b + 2;
		while( *p1 != 0 && *p1 == *p2 ){
			++p1;
			++p2;
		}
		return (*p1 < *p2);
	}
};

//Comparador para el tercer nivel de buckets (omite 3 caracter)
class SAComparatorN3 : public std::binary_function<unsigned int, unsigned int, bool> {
private:
	unsigned char *ref;
	unsigned int largo;
public:
	SAComparatorN3(){
		ref = NULL;
		largo = 0;
	}
	SAComparatorN3(unsigned char *_ref, unsigned int _largo){
		ref = _ref;
		largo = _largo;
	}
	inline bool operator()(const unsigned int a, const unsigned int b){
		//return lexicographical_compare(&(ref[a])+2, &(ref[a])+(largo-a), &(ref[b])+2, &(ref[b])+(largo-b));
		//En este caso debe verificarse si hay string nulos
		if( a > largo-3 ){
			return true;
		}
		if( b > largo-3 ){
			return false;
		}
		unsigned char *p1 = ref + a + 3;
		unsigned char *p2 = ref + b + 3;
		while( *p1 != 0 && *p1 == *p2 ){
			++p1;
			++p2;
		}
		return (*p1 < *p2);
	}
};






#endif //_COMPARATOR_UTILS_H

