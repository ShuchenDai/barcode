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
#include "barcode_bmp.h"
#include "Print/barcode_print_prn.h"
#include "Print/barcode_inject_prn.h"
#include "barcode.h"
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>

#define LEN 2024*1024+100
unsigned char buf[LEN];
unsigned char buf2[LEN*10];
unsigned char buf3[1024];



int main(int argv, char **args) {
	int ret;
	FILE *fin, *fout;

	char code128Str[24] = "6219839849828abcdefgh";
	unsigned int code128StrLen = sizeof("6219839849828abcdefgh")-1;
	unsigned int dpiPage = 600, dpiBmp = 300;
	unsigned int w = 370;
	unsigned int h = 80;

//	ret = Barcode_Print_Fill_Buf(code128Str, code128StrLen, buf, LEN, dpiPage, dpiBmp, w, h, &bmpBegin, bmpLen, &strBegin,  strLen, fileLen);
//	if(ret!=0) {
//		printf("%s\n", "Barcode_Print_Fill_Buf err!");
//		return 1;
//	}

//	//输出单独的位图文件
//	fout = fopen("/home/welkinm/bmp.prn", "w");
//	fwrite(buf,1, fileLen, fout);
//	fclose(fout);

	ret = Barcode_Inject_Prn("/home/welkinm/basereport_2010_001.pcl", "/home/welkinm/barcode.prn",
			BARCODE_TYPE_EAN13, code128Str, code128StrLen);
	if(ret!=0) {
		printf("%s\n", "Barcode_Print_Fill_Buf err!");
		return 1;
	}

//    if(len!=readLen) {
//    	printf("Read err: %d(%s)\n", errno, strerror(errno));
//    	return 1;
//    }
//    fwrite(buf2, 1, readLen, fout);//写源PRN开始部份
//    fwrite(p, 1, imgLen, fout);//写入条形码
//    fwrite(buf3, 1, strLen, fout);//写入字
//    readLen = fread(buf2, 1, 1024*1024, fin);//写源PRN结尾部份
//    while(readLen>0) {
//    	fwrite(buf2, 1, readLen, fout);
//    	readLen = fread(buf2, 1, 1024*1024, fin);
//    }
//    fclose(fin);
//    fclose(fout);

#if __BYTE_ORDER == __LITTLE_ENDIAN
	cout<< "Little" <<endl;
#elif __BYTE_ORDER == __BIG_ENDIAN
	count<< "Big" <<endl;
#endif
}
