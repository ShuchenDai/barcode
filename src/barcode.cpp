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
#include <errno.h>
#include <stdlib.h>

#define LEN 2024*1024+100
unsigned char buf[LEN];
unsigned char buf2[LEN*10];
unsigned char buf3[1024];

int main(int argv, char **args) {
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
	int dpi = 300;
	unsigned char *p;
	unsigned int fileLen;
	ret = Barcode_Print_Fill_Buf(code128Str, code128StrLen, buf, LEN, dpi, w, h, imgLen, false, &p, fileLen);
	if(ret!=0) {
		printf("%s\n", "Barcode_Print_Fill_Buf err!");
		return 1;
	}
	unsigned int strLen;
	ret = Barcode_Print_Fill_Buf_Str(code128Str, code128StrLen, buf3, 1024, strLen, w, h, dpi);
	if(ret!=0) {
		printf("%s\n", "Barcode_Print_Fill_Buf_Str err!");
		return 1;
	}

	buf3[strLen] = 0;
	printf("Str: %s\n", buf3);

	//输出单独的位图文件
	FILE *fin, *fout;
	fout = fopen("/home/welkinm/bmp.prn", "w");
    fwrite(buf,1, fileLen, fout);
    fclose(fout);

	printf("%d\n", argv);
	if(argv<3) return 0;

    fin = fopen(args[1], "r");
    if(fin<=0) {
    	printf("%d(%s)\n", errno, strerror(errno));
    	return 1;
    }
    fout = fopen("/home/welkinm/out.prn", "w");

    int len=atoi(args[2]);
    int readLen = 0;
    readLen = fread(buf2, 1, len, fin);
    if(len!=readLen) {
    	printf("Read err: %d(%s)\n", errno, strerror(errno));
    	return 1;
    }
    fwrite(buf2, 1, readLen, fout);//写源PRN开始部份
    fwrite(p, 1, imgLen, fout);//写入条形码
    fwrite(buf3, 1, strLen, fout);//写入字


    readLen = fread(buf2, 1, 1024*1024, fin);//写源PRN结尾部份
    while(readLen>0) {
    	fwrite(buf2, 1, readLen, fout);
    	readLen = fread(buf2, 1, 1024*1024, fin);
    }
    fclose(fin);
    fclose(fout);

#if __BYTE_ORDER == __LITTLE_ENDIAN
	cout<< "Little" <<endl;
#elif __BYTE_ORDER == __BIG_ENDIAN
	count<< "Big" <<endl;
#endif
}


















