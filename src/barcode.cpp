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
#include "Print/barcode_print.h"
#include <stdio.h>
#include <string.h>

#define LEN 2024*1024+100
unsigned char buf[LEN];

int main() {
	unsigned int imgLen = 0;

//	memset(buf, 0, LEN);
	int ret;
	unsigned int w = 370;
	unsigned int h = 80;


//	bool isColorExchange = true;
//	unsigned int x = 4, y = 4, width=2, height=220;
//	BarCode_BMPHead_Type head;
//	ret = BarCode_BMP_Build_Head(head, w, h, 24, BARCODE_BMP_COLOR_TABLE_1);
//	if(ret!=0) printf("BarCode_BMP_Build_Head");
//	ret = BarCode_BMP_Mem_Write_Head(head, buf, LEN);
//	if(ret!=0) printf("BarCode_BMP_Mem_Write_Head");
//	imgLen = head.fh.bfSize;
//	//背景色与线条色
//	BarCode_BMPColor_Type bgRGB;
//	BarCode_BMPColor_Type barRGB;
//	if(isColorExchange) {
//		bgRGB.rgb.r = 0xff;
//		bgRGB.rgb.g = 0xff;
//		bgRGB.rgb.b = 0xff;
//		bgRGB.rgb.a = 0;
//		barRGB.rgb.r = 255;
//		barRGB.rgb.g = 0;
//		barRGB.rgb.b = 0;
//		barRGB.rgb.a = 0;
//	} else {
//		bgRGB.rgb.r = 255;
//		bgRGB.rgb.g = 0;
//		bgRGB.rgb.b = 0;
//		bgRGB.rgb.a = 0;
//		barRGB.rgb.r = 0;
//		barRGB.rgb.g = 0;
//		barRGB.rgb.b = 0;
//		barRGB.rgb.a = 0;
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

	const char *code128Str = "abcdefghjklm123456789";
	unsigned int code128StrLen = sizeof("abcdefghjklm123456789")-1;
//	Code128B_Auto_Fill_Buf(code128Str, code128StrLen, buf, LEN, 600, w, h, imgLen, false);
//	Code128B_Fill_Buf(code128Str, code128StrLen, buf, LEN, 600, w, h, imgLen, false);
	unsigned char *p;
	unsigned int fileLen;
	Barcode_Print_Fill_Buf(code128Str, code128StrLen, buf, LEN, 600, w, h, imgLen, false, &p, fileLen);
	printf("%s",buf);


	FILE *fout = fopen("/home/welkinm/bmp.prn", "w");
    fwrite(buf,1, fileLen, fout);
    fclose(fout);

#if __BYTE_ORDER == __LITTLE_ENDIAN
	cout<< "Little" <<endl;
#elif __BYTE_ORDER == __BIG_ENDIAN
	count<< "Big" <<endl;
#endif
}
