/*
 * barcode_print.cpp
 *
 *  Created on: 2015年4月29日
 *      Author: welkinm
 */

#include "barcode_print.h"
#include "../Code128/Code128_fill.h"
#include "../EAN13/EAN13_fill.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>


#define BARCODE_PRINT_DEFAULT_DPI	300
#define BARCODE_PRINT_DEFAULT_DPI_PAGE	BARCODE_PRINT_DEFAULT_DPI	//默认文档分辨率
#define BARCODE_PRINT_DEFAULT_DPI_BMP BARCODE_PRINT_DEFAULT_DPI	//默认位图分辨率
//PJL PCL5文件头
#define COMMOM_PJL_HEAD "\x1b%-12345X@PJL JOB\n\
@PJL SET JOBNAME = \"barcode.bmp\"\n\
@PJL SET AUTOTRAYCHANGE = ON\n\
@PJL SET DUPLEX=OFF\n\
@PJL SET SMOOTHING=OFF\n\
@PJL SET ECONOMODE=OFF\n\
@PJL SET QTY = 1\n\
@PJL SET JOBOFFSET = OFF\n\
@PJL SET STAPLE=OFF\n\
@PJL SET PUNCH=OFF\n\
@PJL SET SLIPSHEETPRINT=OFF\n\
@PJL SET EDGETOEDGE=NO\n\
@PJL SET FOLD=OFF\n\
@PJL SET RESOLUTION=600\n\
@PJL ENTER LANGUAGE=PCL\n\
\x1b\x45"
#define COMMOM_PJL_HEAD_LEN sizeof(COMMOM_PJL_HEAD)-1
//PJL PCL5文件尾
#define COMMOM_PJL_TAIL "\x1b%-12345X@PJL EOJ NAME = \"barcode.bmp\"\n\
\x1b%-12345X"
#define COMMOM_PJL_TAIL_LEN sizeof(COMMOM_PJL_TAIL)-1
//选取字符集
#define BARCODE_PRINT_DEFAULT_FONT "\x1b(8U\x1b(s0P\x1b(s16H\x1b(s6V\x1b(s0S\x1b(s0B"
#define BARCODE_PRINT_DEFAULT_FONT_LEN sizeof(BARCODE_PRINT_DEFAULT_FONT)-1
//字体单位点大小
#define BARCODE_PRINT_POINT_SIZE ((double)1/72)//0.01389
//字体字高
#define BARCODE_PRINT_FONT_H 6
#define BARCODE_PRINT_TEMP_LEN 1028*128



int Barcode_Print_Fill_Buf(const char *barcode, unsigned int barcodeLen,
		unsigned char *buf, unsigned int bufLen,
		unsigned int dpiPage, unsigned int dpiBmp,
		unsigned int& w, unsigned int& h,
		unsigned char** bmpBegin, unsigned int& bmpLen,
		unsigned char** strBegin, unsigned int& strLen,
		unsigned int& bmpAndStrLen, unsigned int& fileLen) {
	int ret = 0;
	unsigned char *p = buf;
	char strTmp[128];
	if(bufLen<1024) return 1;
	//初始化默认DPI
	if(dpiPage == 0) dpiPage = BARCODE_PRINT_DEFAULT_DPI_PAGE;
	if(dpiBmp == 0) dpiBmp = BARCODE_PRINT_DEFAULT_DPI_BMP;
	//填充PUL PCL5头
	memcpy(p, COMMOM_PJL_HEAD, COMMOM_PJL_HEAD_LEN);
	p += COMMOM_PJL_HEAD_LEN;


	//设置分辨率
	ret = sprintf(strTmp, "\x1b&u%dD", dpiPage);
	memcpy(p, strTmp, ret);
	p += ret;
	//返回bmp开始位置
	*bmpBegin = p;
	//压光标栈，移动到左上角
	memcpy(p, "\x1b&f0S", sizeof("\x1b&f0S")-1);	//PUSH Position
	p += sizeof("\x1b&f0S")-1;
	memcpy(p, "\x1b*p0X", sizeof("\x1b*p0X")-1);	//X position is 0
	p += sizeof("\x1b*p0X")-1;
	memcpy(p, "\x1b*p0Y", sizeof("\x1b*p0Y")-1);	//Y position is 0
	p += sizeof("\x1b*p0Y")-1;
	memcpy(p, "\x1b*p-10000Y", sizeof("\x1b*p-10000Y")-1);	//Y position is top
	p += sizeof("\x1b*p-10000Y")-1;
	ret = sprintf(strTmp, "\x1b*p+%dY", dpiPage/10);
	memcpy(p, strTmp, ret);
	p += ret;
	//填充条码bmp到临时缓存中
	unsigned char *pBmpBuf = new unsigned char[BARCODE_PRINT_TEMP_LEN];
	ret = Code128B_Auto_Fill_Buf(barcode, barcodeLen, pBmpBuf, BARCODE_PRINT_TEMP_LEN, dpiBmp, w, h, bmpLen, false);
	if(ret!=0) {
		printf("Code128B_Auto_Fill_Buf err\n");
		return ret;
	}
	//构造条码bmp文件头，用于取得bmp的实际存储宽度
	BarCode_BMPHead_Type head;
	ret = BarCode_BMP_Build_Head(head, w, h, 1, BARCODE_BMP_COLOR_TABLE_1);
	if(ret!=0) {
		printf("BarCode_BMP_Build_Head err\n");
		return 1;
	}
	//bmp位图实际数据位移
	unsigned char *pBmpBegin = pBmpBuf + head.fh.bfOffBits;
	//准备打印位图，初始化位图打印信息
	ret = sprintf(strTmp, "\x1b*t%dR", dpiBmp);	//Raster Graphics Resolution
	memcpy(p, strTmp, ret);
	p += ret;
	ret = sprintf(strTmp, "\x1b*r%dF", 3);		//Presentation
	memcpy(p, strTmp, ret);
	p += ret;
	ret = sprintf(strTmp, "\x1b*r%dT", h);		//Height
	memcpy(p, strTmp, ret);
	p += ret;
	ret = sprintf(strTmp, "\x1b*r%dS", w);		//Width
	memcpy(p, strTmp, ret);
	p += ret;
	ret = sprintf(strTmp, "\x1b*r%dA", 1);		//Start Raster Graphics
	memcpy(p, strTmp, ret);
	p += ret;
	//开始打印位图，填充位图打印信息
	for(int i=h-1; i>=0; i--) {
		ret = sprintf(strTmp, "\x1b*b%dW", head.widthBypes);		//Transfer Raster Data
		memcpy(p, strTmp, ret);
		p += ret;
		memcpy(p, &pBmpBegin[i*head.widthBypes], head.widthBypes);
		p += head.widthBypes;
	}
	delete []pBmpBuf;
	//结束位图打印信息
	memcpy(p, "\x1b*rC", sizeof("\x1b*rC")-1);//End Raster Graphics
	p += sizeof("\x1b*rC")-1;
	//填充出栈原位置光标打印指令
	memcpy(p, "\x1b&f1S", sizeof("\x1b&f1S")-1);	//POP Position
	p += sizeof("\x1b&f1S")-1;
	//bmp长度
	bmpLen = p - *bmpBegin;

	//压光标栈，移动到左上角
	memcpy(p, "\x1b&f0S", sizeof("\x1b&f0S")-1);	//PUSH Position
	p += sizeof("\x1b&f0S")-1;
	memcpy(p, "\x1b*p0X", sizeof("\x1b*p0X")-1);	//X position is 0
	p += sizeof("\x1b*p0X")-1;
	memcpy(p, "\x1b*p0Y", sizeof("\x1b*p0Y")-1);	//Y position is 0
	p += sizeof("\x1b*p0Y")-1;
	memcpy(p, "\x1b*p-10000Y", sizeof("\x1b*p-10000Y")-1);	//Y position is top
	p += sizeof("\x1b*p-10000Y")-1;
	//移动光标到条码下面
	ret = sprintf(strTmp, "\x1b*p+%dY", dpiPage/10 + (dpiPage*h/dpiBmp)+(int)(BARCODE_PRINT_FONT_H*BARCODE_PRINT_POINT_SIZE*dpiPage));
	memcpy(p, strTmp, ret);
	p += ret;
	//选择打印字体，大小等
	memcpy(p, BARCODE_PRINT_DEFAULT_FONT, BARCODE_PRINT_DEFAULT_FONT_LEN);
	p += BARCODE_PRINT_DEFAULT_FONT_LEN;
	//开始填充文字
	*p = ' ';
	p++;
	*p = ' ';
	p++;
	*p = ' ';
	p++;

	//返回打印文字的开始位置
	*strBegin = p;
	for(unsigned int i = 0; i<barcodeLen; i++) {
		*p = barcode[i];
		p++;
	}
	//str长度
	fileLen = p - *strBegin;

	//填充出栈原位置光标打印指令
	memcpy(p, "\x1b&f1S", sizeof("\x1b&f1S")-1);	//POP Position
	p += sizeof("\x1b&f1S")-1;
	bmpAndStrLen = p - *bmpBegin;
	//填充PJL PCL5结束符
	memcpy(p, COMMOM_PJL_TAIL, COMMOM_PJL_TAIL_LEN);
	p += COMMOM_PJL_TAIL_LEN;
	fileLen = p - buf;
	return 0;
}

//int Barcode_Print_Fill_Buf(const char *barcode, unsigned int barcodeLen, unsigned char *buf, unsigned int bufLen, unsigned int dpi,
//		unsigned int& w, unsigned int& h, unsigned int& bmpLen, bool isColorExchange,
//		unsigned char** rasterGraphicsBegin, unsigned int& fileLen) {
//	char strTmp[128];
//	int ret = 0;
//	if(bufLen<1024) return 1;
//	if(dpi == 0) dpi = BARCODE_PRINT_DEFAULT_DPI;
//	unsigned char *p = buf;
//	memcpy(p, COMMOM_PJL_HEAD, COMMOM_PJL_HEAD_LEN);
//	p += COMMOM_PJL_HEAD_LEN;
//
//	*rasterGraphicsBegin = p;
//	memcpy(p, "\x1b&f0S", sizeof("\x1b&f0S")-1);	//PUSH Position
//	p += sizeof("\x1b&f0S")-1;
//	memcpy(p, "\x1b*p0X", sizeof("\x1b*p0X")-1);	//X position is 0
//	p += sizeof("\x1b*p0X")-1;
//	memcpy(p, "\x1b*p0Y", sizeof("\x1b*p0Y")-1);	//Y position is 0
//	p += sizeof("\x1b*p0Y")-1;
//	memcpy(p, "\x1b*p-10000Y", sizeof("\x1b*p-10000Y")-1);	//Y position is top
//	p += sizeof("\x1b*p-10000Y")-1;
//	ret = sprintf(strTmp, "\x1b*p+%dY", dpi/10);
//	memcpy(p, strTmp, ret);
//	p += ret;
//	unsigned char *pBmpBuf = new unsigned char[BARCODE_PRINT_TEMP_LEN];
//	ret = Code128B_Auto_Fill_Buf(barcode, barcodeLen, pBmpBuf, BARCODE_PRINT_TEMP_LEN, dpi, w, h, bmpLen, isColorExchange);
//	if(ret!=0) {
//		printf("Code128B_Auto_Fill_Buf err\n");
//		return ret;
//	}
//	BarCode_BMPHead_Type head;
//	ret = BarCode_BMP_Build_Head(head, w, h, 1, BARCODE_BMP_COLOR_TABLE_1);
//	if(ret!=0) {
//		printf("BarCode_BMP_Build_Head err\n");
//		return 1;
//	}
//	unsigned char *pBmpBegin = pBmpBuf + BARCODE_BMP_FILE_HEADER_LENGTH + BARCODE_BMP_INFORMATION_HEADER_LENGTH + 4*(1<<1);
//
//
//	ret = sprintf(strTmp, "\x1b*t%dR", dpi);	//Raster Graphics Resolution
//	memcpy(p, strTmp, ret);
//	p += ret;
//	ret = sprintf(strTmp, "\x1b*r%dF", 3);		//Presentation
//	memcpy(p, strTmp, ret);
//	p += ret;
//	ret = sprintf(strTmp, "\x1b*r%dT", h);		//Height
//	memcpy(p, strTmp, ret);
//	p += ret;
//	ret = sprintf(strTmp, "\x1b*r%dS", w);		//Width
//	memcpy(p, strTmp, ret);
//	p += ret;
//	ret = sprintf(strTmp, "\x1b*r%dA", 1);		//Start Raster Graphics
//	memcpy(p, strTmp, ret);
//	p += ret;
//
//	for(int i=h-1; i>=0; i--) {
//		ret = sprintf(strTmp, "\x1b*b%dW", head.widthBypes);		//Transfer Raster Data
//		memcpy(p, strTmp, ret);
//		p += ret;
//		memcpy(p, &pBmpBegin[i*head.widthBypes], head.widthBypes);
//		p += head.widthBypes;
//	}
//	delete []pBmpBuf;
//
//	memcpy(p, "\x1b*rC", sizeof("\x1b*rC")-1);//End Raster Graphics
//	p += sizeof("\x1b*rC")-1;
//
//	memcpy(p, "\x1b&f1S", sizeof("\x1b&f1S")-1);	//POP Position
//	p += sizeof("\x1b&f1S")-1;
//	bmpLen = p - *rasterGraphicsBegin;
//	memcpy(p, COMMOM_PJL_TAIL, COMMOM_PJL_TAIL_LEN);
//	p += COMMOM_PJL_TAIL_LEN;
//	fileLen = p - buf;
//	return 0;
//}
//
//
//
//
//
//int Barcode_Print_Fill_Buf_Str(const char *str, unsigned int strLen, unsigned char *buf, unsigned int bufLen,
//		unsigned int &retLen, unsigned int w, unsigned int h, unsigned int dpi) {
//	int ret = 0;
//	unsigned char *p = buf;
//	char strTmp[128];
//	memcpy(p, "\x1b&f0S", sizeof("\x1b&f0S")-1);	//PUSH Position
//	p += sizeof("\x1b&f0S")-1;
//	memcpy(p, "\x1b*p0X", sizeof("\x1b*p0X")-1);	//X position is 0
//	p += sizeof("\x1b*p0X")-1;
////	ret = sprintf(strTmp, "\x1b*p+%dx", w-80);
////	memcpy(p, strTmp, ret);	//X position is 0
////	p += ret;
//	memcpy(p, "\x1b*p0Y", sizeof("\x1b*p0Y")-1);	//Y position is 0
//	p += sizeof("\x1b*p0Y")-1;
//	memcpy(p, "\x1b*p-10000Y", sizeof("\x1b*p-10000Y")-1);	//Y position is top
//	p += sizeof("\x1b*p-10000Y")-1;
//	ret = sprintf(strTmp, "\x1b*p+%dY", dpi/10+h+(int)(BARCODE_PRINT_FONT_H*BARCODE_PRINT_POINT_SIZE*dpi));
//	memcpy(p, strTmp, ret);
//	p += ret;
//
//	memcpy(p, BARCODE_PRINT_DEFAULT_FONT, BARCODE_PRINT_DEFAULT_FONT_LEN);
//	p += BARCODE_PRINT_DEFAULT_FONT_LEN;
//	*p = ' ';
//	p++;
//	*p = ' ';
//	p++;
//	*p = ' ';
//	p++;
//	for(unsigned int i = 0; i<strLen; i++) {
//		*p = str[i];
//		p++;
//	}
//	memcpy(p, "\x1b&f1S", sizeof("\x1b&f1S")-1);	//POP Position
//	p += sizeof("\x1b&f1S")-1;
//	retLen = p - buf;
//	return 0;
//}





























