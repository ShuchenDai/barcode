//============================================================================
// Name        : barcode.cpp
// Author      : welkinm
// Version     :
// Copyright   : GPL v2.1
// Description : Hello World in C++, Ansi-style
//============================================================================

#include <iostream>
using namespace std;

#include "EAN13/EAN13_fill.h"
#include "Code128/Code128_fill.h"
#include <stdio.h>
#include <string.h>

int main() {
#define LEN 1024*1024+100
	unsigned char buf[LEN];
	unsigned int imgLen = 0;

	memset(buf, 0, LEN);
	unsigned int w = 640;
	unsigned int h = 60;
//	const char *ean13Str = "6923450659328";
//	unsigned int ean13StrLen = sizeof("6923450659328")-1;
//	EAN13_Fill_Buf("6923450659328", buf, LEN, w, h, imgLen, false);

//	const char *code128Str = "A";//
//	unsigned int code128StrLen = sizeof("A")-1;//

	const char *code128Str = "08:00:27:ec:a5:7c_0000000001_102398941";
	unsigned int code128StrLen = sizeof("08:00:27:ec:a5:7c_0000000001_102398941")-1;

	Code128B_Auto_Fill_Buf(code128Str, code128StrLen, buf, LEN, w, h, imgLen, false);

	FILE *fout = fopen("/home/welkinm/ean13_4", "w");
    fwrite(buf,1, imgLen, fout);
    fclose(fout);
#if __BYTE_ORDER == __LITTLE_ENDIAN
	cout<< "Little" <<endl;
#elif __BYTE_ORDER == __BIG_ENDIAN
	count<< "Big" <<endl;
#endif
}
