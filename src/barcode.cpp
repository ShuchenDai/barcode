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
#include <stdio.h>
#include <string.h>

#define LEN 2024*1024+100
unsigned char buf[LEN];

int main() {
	unsigned int imgLen = 0;

//	memset(buf, 0, LEN);
	int ret;
	unsigned int w = 132;
	unsigned int h = 80;

//	bool isColorExchange = true;
//	unsigned int x = 1, y = 1, width=1, height=1;
//	BarCode_BMPHead_Type head;
//	int ret = BarCode_BMP_Build_Head(head, w, h, 1, BARCODE_BMP_COLOR_TABLE_1);
//	if(ret!=0) printf("BarCode_BMP_Build_Head");
//	ret = BarCode_BMP_Mem_Write_Head(head, buf, LEN);
//	if(ret!=0) printf("BarCode_BMP_Mem_Write_Head");
//	imgLen = head.fh.bfSize;
//	//背景色与线条色
//	BarCode_BMPColor_Type bgRGB;
//	BarCode_BMPColor_Type barRGB;
//	if(isColorExchange) {
//		bgRGB.idx.i = 1;
//		barRGB.idx.i = 0;
//	} else {
//		bgRGB.idx.i = 0;
//		barRGB.idx.i = 1;
//	}
//	ret = BarCode_BMP_Mem_Wrire_BK_Color(head, buf, bgRGB);
//	if(ret!=0) printf("BarCode_BMP_Mem_Wrire_BK_Color");
//	ret = BarCode_BMP_Mem_Fill_Rect(head, buf, x, y, width, height, barRGB);
//	if(ret!=0) printf("BarCode_BMP_Mem_Fill_Rect");

//	char ean13Str[32] = "6923450659328";
//	ret = EAN13_Fill_Buf(ean13Str, buf, LEN, w, h, imgLen, false);
//	if(ret!=0) printf("EAN13_Fill_Buf");


//	const char *code128Str = "A";//
//	unsigned int code128StrLen = sizeof("A")-1;//

	const char *code128Str = "ab:cd:ef:GH:JK:LM_123456789_98473230";
	unsigned int code128StrLen = sizeof("ab:cd:ef:GH:JK:LM_123456789_98473230")-1;
	Code128B_Auto_Fill_Buf(code128Str, code128StrLen, buf, LEN, 600, w, h, imgLen, false);
//	Code128B_Fill_Buf(code128Str, code128StrLen, buf, LEN, 600, w, h, imgLen, false);

	FILE *fout = fopen("/home/welkinm/EAN13_10", "w");
    fwrite(buf,1, imgLen, fout);
    fclose(fout);
#if __BYTE_ORDER == __LITTLE_ENDIAN
	cout<< "Little" <<endl;
#elif __BYTE_ORDER == __BIG_ENDIAN
	count<< "Big" <<endl;
#endif
}
