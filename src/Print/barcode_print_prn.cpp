/*
 * barcode_print.cpp
 *
 *  Created on: 2015年4月29日
 *      Author: welkinm
 */

#include "barcode_print_prn.h"

#include "../Code128/Code128_fill.h"
#include "../EAN13/EAN13_fill.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "../barcode.h"
#include "barcode_inject_prn.h"


#define BARCODE_PRINT_DEFAULT_DPI	300
#define BARCODE_PRINT_DEFAULT_DPI_PAGE	BARCODE_PRINT_DEFAULT_DPI	//默认文档分辨率
#define BARCODE_PRINT_DEFAULT_DPI_BMP BARCODE_PRINT_DEFAULT_DPI	//默认位图分辨率
//PJL PCL5文件头
#define COMMOM_PJL_PCL5_HEAD "\x1b%-12345X@PJL JOB\n\
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
#define COMMOM_PJL_PCL5_HEAD_LEN sizeof(COMMOM_PJL_PCL5_HEAD)-1
//PJL PCL5文件尾
#define COMMOM_PJL_TAIL "\x1b%-12345X@PJL EOJ NAME = \"barcode.bmp\"\n\
\x1b%-12345X"
#define COMMOM_PJL_TAIL_LEN sizeof(COMMOM_PJL_TAIL)-1
//选取字符集
#define BARCODE_PRINT_DEFAULT_FONT "\x1b(8U\x1b(s0P\x1b(s%dH\x1b(s%dV\x1b(s0S\x1b(s0B"
#define BARCODE_PRINT_DEFAULT_FONT_LEN sizeof(BARCODE_PRINT_DEFAULT_FONT)-1
//字体单位点大小
#define BARCODE_PRINT_POINT_SIZE ((double)1/72)//0.01389
#define BARCODE_PRINT_FONT_SPAN_PITCH 15
//字体字高
#define BARCODE_PRINT_FONT_H 6
#define BARCODE_PRINT_TEMP_LEN 1028*128



int Barcode_Print_Prn_Fill_Buf(const char *barcode, unsigned int barcodeLen, int barcodeType,
		unsigned char *buf, unsigned int bufLen,
		unsigned int dpiPage, unsigned int dpiBmp,
		unsigned int& w, unsigned int& h,
		unsigned char** bmpBegin, unsigned int& bmpLen,
		unsigned char** strBegin, unsigned int& strLen,
		unsigned int& bmpAndStrLen, unsigned int& fileLen) {
	if(!(barcodeType==BARCODE_TYPE_EAN13 || barcodeType==BARCODE_TYPE_CODE128)) return 1;
	int ret = 0;
	unsigned char *p = buf;
	char strTmp[128];
	if(bufLen<1024) return 1;
	//初始化默认DPI
	if(dpiPage == 0) dpiPage = BARCODE_PRINT_DEFAULT_DPI_PAGE;
	if(dpiBmp == 0) dpiBmp = BARCODE_PRINT_DEFAULT_DPI_BMP;
	//填充PUL PCL5头
	memcpy(p, COMMOM_PJL_PCL5_HEAD, COMMOM_PJL_PCL5_HEAD_LEN);
	p += COMMOM_PJL_PCL5_HEAD_LEN;

	//设置分辨率
	ret = sprintf(strTmp, "\x1b&u%dD", dpiPage);
	memcpy(p, strTmp, ret);
	p += ret;
	//返回bmp开始位置
	*bmpBegin = p;
	//压光标栈，移动到左上角
	memcpy(p, "\x1b&f0S", sizeof("\x1b&f0S")-1);	//PUSH Position
	p += sizeof("\x1b&f0S")-1;
	switch(barcodeType) {
	case BARCODE_TYPE_EAN13:
		ret = sprintf(strTmp, "\x1b*p%dX", (int)(dpiPage*1.35/BARCODE_PRINT_FONT_SPAN_PITCH));
		memcpy(p, strTmp, ret);
		p += ret;
//		memcpy(p, "\x1b*p0X", sizeof("\x1b*p0X")-1);	//X position is 0
//		p += sizeof("\x1b*p0X")-1;
		break;
	default:
		memcpy(p, "\x1b*p0X", sizeof("\x1b*p0X")-1);	//X position is 0
		p += sizeof("\x1b*p0X")-1;
	}
	memcpy(p, "\x1b*p0Y", sizeof("\x1b*p0Y")-1);	//Y position is 0
	p += sizeof("\x1b*p0Y")-1;
	memcpy(p, "\x1b*p-10000Y", sizeof("\x1b*p-10000Y")-1);	//Y position is top
	p += sizeof("\x1b*p-10000Y")-1;
	ret = sprintf(strTmp, "\x1b*p+%dY", dpiPage/6);
	memcpy(p, strTmp, ret);
	p += ret;
	//填充条码bmp到临时缓存中
	unsigned char *pBmpBuf = new unsigned char[BARCODE_PRINT_TEMP_LEN];
	switch(barcodeType) {
	case BARCODE_TYPE_EAN13:
		ret = EAN13_Fill_Buf((char *)barcode, pBmpBuf, BARCODE_PRINT_TEMP_LEN, dpiBmp, w, h, bmpLen, false);
		break;
	case BARCODE_TYPE_CODE128:
		ret = Code128B_Auto_Fill_Buf(barcode, barcodeLen, pBmpBuf, BARCODE_PRINT_TEMP_LEN, dpiBmp, w, h, bmpLen, false);
		break;
	}
	if(ret!=0) {
		delete []pBmpBuf;
		printf("barcode fill buf err\n");
		return ret;
	}
	//构造条码bmp文件头，用于取得bmp的实际存储宽度
	BarCode_BMPHead_Type head;
	ret = BarCode_BMP_Build_Head(head, w, h, 1, BARCODE_BMP_COLOR_TABLE_1);
	if(ret!=0) {
		delete []pBmpBuf;
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
//	ret = sprintf(strTmp, "\x1b*r%dT", h);		//Height
//	memcpy(p, strTmp, ret);
//	p += ret;
//	ret = sprintf(strTmp, "\x1b*r%dS", w);		//Width
//	memcpy(p, strTmp, ret);
//	p += ret;
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
	switch(barcodeType) {
	case BARCODE_TYPE_EAN13:
		ret = sprintf(strTmp, "\x1b*p+%dY", dpiPage/6 + (dpiPage*h/dpiBmp)+(int)(BARCODE_PRINT_FONT_H*BARCODE_PRINT_POINT_SIZE*dpiPage*3/5));
		break;
	default:
		ret = sprintf(strTmp, "\x1b*p+%dY", dpiPage/6 + (dpiPage*h/dpiBmp)+(int)(BARCODE_PRINT_FONT_H*BARCODE_PRINT_POINT_SIZE*dpiPage));
	}
	memcpy(p, strTmp, ret);
	p += ret;

	//选择打印字体，大小等
	ret = sprintf(strTmp, BARCODE_PRINT_DEFAULT_FONT, BARCODE_PRINT_FONT_SPAN_PITCH, BARCODE_PRINT_FONT_H);
	memcpy(p, strTmp, ret);
	p += ret;
//	memcpy(p, BARCODE_PRINT_DEFAULT_FONT, BARCODE_PRINT_DEFAULT_FONT_LEN);
//	p += BARCODE_PRINT_DEFAULT_FONT_LEN;

	//开始填充文字
	switch(barcodeType) {
	case BARCODE_TYPE_EAN13:
		*p = barcode[0];
		p++;
		*p = ' ';
		p++;
		for(unsigned int i = 1; i<7; i++) {
			*p = barcode[i];
			p++;
		}
		*p = ' ';
		p++;
		for(unsigned int i = 7; i<13; i++) {
			*p = barcode[i];
			p++;
		}
		break;
	default:
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
	}


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


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////



#define BARCODE_PRINT_DEFAULT_DPI_XL 600
#define BARCODE_PRINT_DEFAULT_DPI_PAGE_XL BARCODE_PRINT_DEFAULT_DPI_XL	//默认文档分辨率
#define BARCODE_PRINT_DEFAULT_DPI_BMP_XL BARCODE_PRINT_DEFAULT_DPI_XL	//默认位图分辨率
//PJL PCL5文件头
#define COMMOM_PJL_PCLXL_HEAD "\x1b%-12345X@PJL JOB NAME=\"barcode.bmp\"\n\
@PJL SET STRINGCODESET=UTF8\n\
@PJL SET RET=ON\n\
@PJL SET JOBNAME = \"barcode.bmp\"\n\
@PJL SET SEPARATORPAGE=OFF\n\
@PJL SET FOLD=\"\"\n\
@PJL SET PUNCH=OFF\n\
@PJL SET PROCESSINGACTION=APPEND\n\
@PJL SET PROCESSINGTYPE=\"PUNCH\"\n\
@PJL SET PROCESSINGOPTION=\"NONE\"\n\
@PJL SET PROCESSINGBOUNDARY=MOPY\n\
@PJL SET OUTBIN=AUTO\n\
@PJL SET PROCESSINGACTION=APPEND\n\
@PJL SET PROCESSINGTYPE=\"STAPLING\"\n\
@PJL SET PROCESSINGOPTION=\"NONE\"\n\
@PJL SET PROCESSINGBOUNDARY=MOPY\n\
@PJL SET ECONOMODE=OFF\n\
@PJL SET KEEPGLOSSMODE=UNDEFINED\n\
@PJL SET OUTPUTPROFILE=SHA\n\
@PJL SET RESOLUTION=600\n\
@PJL SET BITSPERPIXEL=8\n\
@PJL ENTER LANGUAGE=PCLXL\n\
) HP-PCL XL;3;0;Comment Copyright(c) 2015 Healgoo\n"
#define COMMOM_PJL_PCLXL_HEAD_LEN sizeof(COMMOM_PJL_PCLXL_HEAD)-1

int Barcode_Print_Prn_Fill_Buf_XL(const char *barcode, unsigned int barcodeLen, int barcodeType,
		unsigned char *buf, unsigned int bufLen,
		unsigned int dpiPage, unsigned int dpiBmp, int endian,
		unsigned int& w, unsigned int& h,
		unsigned char** bmpBegin, unsigned int& bmpLen,
		unsigned char** strBegin, unsigned int& strLen,
		unsigned int& bmpAndStrLen, unsigned int& fileLen) {
	if(!(barcodeType==BARCODE_TYPE_EAN13 || barcodeType==BARCODE_TYPE_CODE128)) return 1;
	int ret = 0, i;
	unsigned char *p = buf;
//	unsigned char temp[128];
//	char strTmp[128];
	if(bufLen<1024) return 1;

	//填充条码bmp到临时缓存中
	unsigned char *pBmpBuf = new unsigned char[BARCODE_PRINT_TEMP_LEN];
	switch(barcodeType) {
	case BARCODE_TYPE_EAN13:
		ret = EAN13_Fill_Buf((char *)barcode, pBmpBuf, BARCODE_PRINT_TEMP_LEN, dpiBmp, w, h, bmpLen, false);
		break;
	case BARCODE_TYPE_CODE128:
		ret = Code128B_Auto_Fill_Buf(barcode, barcodeLen, pBmpBuf, BARCODE_PRINT_TEMP_LEN, dpiBmp, w, h, bmpLen, false);
		break;
	}
	if(ret!=0) {
		delete []pBmpBuf;
		printf("barcode fill buf err\n");
		return ret;
	}
	//构造条码bmp文件头，用于取得bmp的实际存储宽度
	BarCode_BMPHead_Type head;
	ret = BarCode_BMP_Build_Head(head, w, h, 1, BARCODE_BMP_COLOR_TABLE_1);
	if(ret!=0) {
		delete []pBmpBuf;
		printf("BarCode_BMP_Build_Head err\n");
		return 1;
	}
	//bmp位图实际数据位移
	unsigned char *pBmpBegin = pBmpBuf + head.fh.bfOffBits;

	//初始化默认DPI
	if(dpiPage == 0) dpiPage = BARCODE_PRINT_DEFAULT_DPI_PAGE_XL;
	if(dpiBmp == 0) dpiBmp = BARCODE_PRINT_DEFAULT_DPI_BMP_XL;
	//填充PUL PCLXL头
	memcpy(p, COMMOM_PJL_PCLXL_HEAD, COMMOM_PJL_PCLXL_HEAD_LEN);
	p += COMMOM_PJL_PCLXL_HEAD_LEN;

	//BeginSession
	//UnitsPerMesaure
	*p = 0xd1;	p++;			//uint16_xy
	BarCode_Print_Prn_Mem_Write_uInt(&p, 0x02580258, endian);					//600 600
	BarCode_Print_Prn_Mem_Write_uShort(&p, (unsigned short)0x89f8, endian);	//UnitsPerMesaure
	//Mesaure
	*p = 0xc0; p++;				//ubyte
	*p = 0x00; p++;				//eInch
	BarCode_Print_Prn_Mem_Write_uShort(&p, (unsigned short)0x86f8, endian);	//Measure
	//ErrorReport
	*p = 0xc0; p++;				//ubyte
	*p = 0x03; p++;				//eBackChAndErrPage
	BarCode_Print_Prn_Mem_Write_uShort(&p, (unsigned short)0x8ff8, endian);	//ErrorReport
	//BeginSession
	*p = 0x41; p++;

	//OpenDataSource
	//SourceType
	*p = 0xc0; p++;				//ubyte
	*p = 0x00; p++;				//eDefaultDataSource
	BarCode_Print_Prn_Mem_Write_uShort(&p, (unsigned short)0x88f8, endian);	//SourceType
	//DataOrg
	*p = 0xc0; p++;				//ubyte
	*p = 0x01; p++;				//eBinaryLowByteFirst
	BarCode_Print_Prn_Mem_Write_uShort(&p, (unsigned short)0x82f8, endian);	//DataOrg
	//OpenDataSource
	*p = 0x48; p++;

	//BeginPage
	//Orientation
	*p = 0xc0; p++;				//ubyte
	*p = 0x00; p++;				//ePortraitOrientation
	BarCode_Print_Prn_Mem_Write_uShort(&p, (unsigned short)0x28f8, endian);	//Orientation
	//MediaSize
	*p = 0xc0; p++;				//ubyte
	*p = 0x02; p++;				//eA4Paper
	BarCode_Print_Prn_Mem_Write_uShort(&p, (unsigned short)0x25f8, endian);	//MediaSize
	//SimplexPageMode
	*p = 0xc0; p++;				//ubyte
	*p = 0x00; p++;				//eSimplexFrontSide
	BarCode_Print_Prn_Mem_Write_uShort(&p, (unsigned short)0x34f8, endian);	//SimplexPageMode
	//BeginPage
	*p = 0x43; p++;

	//返回bmp开始位置
	*bmpBegin = p;

	//PushGS
	++*p = 0x61;

	//SetPageOrigin
	//PageOrigin
	*p = 0xd1; p++;				//uint16_xy
	BarCode_Print_Prn_Mem_Write_uInt(&p, 0x00000000, endian);	//0 0
	BarCode_Print_Prn_Mem_Write_uShort(&p, (unsigned short)0x2af8, endian);	//PageOrigin
	//SetPageOrigin
	*p = 0x75; p++;

	//SetBrushSource
	//RGBColor
	*p = 0xc8; p++;				//ubyte_array
	*p = 0xc1; p++;				//uint16
	BarCode_Print_Prn_Mem_Write_uShort(&p, (unsigned short)0x03, endian);	//3
	*p = 0x00; p++;				//R
	*p = 0x00; p++;				//G
	*p = 0x00; p++;				//B
	BarCode_Print_Prn_Mem_Write_uShort(&p, (unsigned short)0x0bf8, endian);	//RGBColor
	//SetBrushSource
	*p = 0x63; p++;

	//SetPenSource
	//RGBColor
	*p = 0xc8; p++;				//ubyte_array
	*p = 0xc1; p++;				//uint16
	BarCode_Print_Prn_Mem_Write_uShort(&p, (unsigned short)0x03, endian);	//3
	*p = 0x00; p++;				//R
	*p = 0x00; p++;				//G
	*p = 0x00; p++;				//B
	BarCode_Print_Prn_Mem_Write_uShort(&p, (unsigned short)0x0bf8, endian);	//RGBColor
	//SetPenSource
	*p = 0x79; p++;

	//SetCursor
	//Point
	*p = 0xd3; p++;				//sint16_xy
	BarCode_Print_Prn_Mem_Write_uShort(&p, (unsigned short)(dpiPage/6), endian);				//x
	BarCode_Print_Prn_Mem_Write_uShort(&p, (unsigned short)(dpiPage/6), endian);		//y
	BarCode_Print_Prn_Mem_Write_uShort(&p, (unsigned short)0x4cf8, endian);	//Point
	//SetCursor
	*p = 0x6b; p++;

	//SetColorSpace
	//ColorSpace
	*p = 0xc0; p++;				//ubyte
	*p = 0x02; p++;				//eRGB
	BarCode_Print_Prn_Mem_Write_uShort(&p, (unsigned short)0x03f8, endian);	//ColorSpace
	//PaletteDepth
	*p = 0xc0; p++;				//ubyte
	*p = 0x02; p++;				//e8Bit
	BarCode_Print_Prn_Mem_Write_uShort(&p, (unsigned short)0x02f8, endian);	//PaletteDepth
	//PaletteData
	*p = 0xc8; p++;				//ubyte_array
	*p = 0xc1; p++;				//uint16
	BarCode_Print_Prn_Mem_Write_uShort(&p, (unsigned short)(head.ctCount*3), endian);
	for(i=0; i<head.ctCount; i++) {
		*p = head.ct[i].r; p++;	//R
		*p = head.ct[i].b; p++;	//G
		*p = head.ct[i].b; p++;	//B
	}
	BarCode_Print_Prn_Mem_Write_uShort(&p, (unsigned short)0x06f8, endian);	//PaletteData
	//SetColorSpace
	*p = 0x6a; p++;

	//BeginImage
	//ColorMapping
	*p = 0xc0; p++;				//ubyte
	*p = 0x01; p++;				//eIndexedPixel
	BarCode_Print_Prn_Mem_Write_uShort(&p, (unsigned short)0x64f8, endian);	//ColorMapping
	//ColorDepth
	*p = 0xc0; p++;				//ubyte
	*p = 0x00; p++;				//e1Bit
	BarCode_Print_Prn_Mem_Write_uShort(&p, (unsigned short)0x62f8, endian);	//ColorDepth
	//SourceWidth
	*p = 0xc1; p++;				//uint16
	BarCode_Print_Prn_Mem_Write_uShort(&p, (unsigned short)(head.ih.biWidth), endian);
	BarCode_Print_Prn_Mem_Write_uShort(&p, (unsigned short)0x6cf8, endian);	//SourceWidth
	//SourceHeight
	*p = 0xc1; p++;				//uint16
	BarCode_Print_Prn_Mem_Write_uShort(&p, (unsigned short)(head.ih.biHeight), endian);
	BarCode_Print_Prn_Mem_Write_uShort(&p, (unsigned short)0x6bf8, endian);	//SourceHeight
	//DestinationSize
	*p = 0xd1; p++;				//uint16_xy
	BarCode_Print_Prn_Mem_Write_uShort(&p, (unsigned short)(head.ih.biWidth), endian);
	BarCode_Print_Prn_Mem_Write_uShort(&p, (unsigned short)(head.ih.biHeight), endian);
	BarCode_Print_Prn_Mem_Write_uShort(&p, (unsigned short)0x67f8, endian);	//DestinationSize
	//BeginImage
	*p = 0xb0; p++;

	//ReadImage
	//StartLine
	*p = 0xc1; p++;				//uint16
	BarCode_Print_Prn_Mem_Write_uShort(&p, (unsigned short)0x0000, endian);	//0
	BarCode_Print_Prn_Mem_Write_uShort(&p, (unsigned short)0x6df8, endian);	//StartLine
	//BlockHeight
	*p = 0xc1; p++;				//uint16
	BarCode_Print_Prn_Mem_Write_uShort(&p, (unsigned short)(head.ih.biHeight), endian);	//0
	BarCode_Print_Prn_Mem_Write_uShort(&p, (unsigned short)0x63f8, endian);	//BlockHeight
	//CompressMode
	*p = 0xc0; p++;				//ubyte
	*p = 0x00; p++;				//eNoCompression
	BarCode_Print_Prn_Mem_Write_uShort(&p, (unsigned short)0x65f8, endian);	//CompressMode
	//ReadImage
	*p = 0xb1; p++;

	//embedded_data
	*p = 0xfa; p++;
	BarCode_Print_Prn_Mem_Write_uInt(&p, head.ih.biSizeImage, endian);//Embedded Len
	memcpy(p, pBmpBegin, head.ih.biSizeImage);
	delete []pBmpBuf;
	p += head.ih.biSizeImage;

	//EndImage
	*p = 0xb2; p++;

	//PopGS
	*p = 0x60; p++;

	//bmp长度
	bmpLen = p - *bmpBegin;

	//PageCopies
	*p = 0xc1; p++;				//uint16
	BarCode_Print_Prn_Mem_Write_uShort(&p, (unsigned short)0x0100, endian);	//1
	BarCode_Print_Prn_Mem_Write_uShort(&p, (unsigned short)0x31f8, endian);	//PageCopies

	//EndPage
	*p = 0x44; p++;

	//CloseDataSource
	*p = 0x49; p++;

	//EndSession
	*p = 0x42; p++;

	//填充PJL PCL5结束符
	memcpy(p, COMMOM_PJL_TAIL, COMMOM_PJL_TAIL_LEN);
	p += COMMOM_PJL_TAIL_LEN;

	fileLen = p - buf;
	return 0;
}


#if __BYTE_ORDER == __LITTLE_ENDIAN
	void BarCode_Print_Prn_Mem_Write(unsigned char **pRaw, unsigned char *pData, int len, int endian) {
		unsigned char *p = *pRaw;
		if(endian==PRINTER_PCLXL_ENDIAN_LITTLE) {
			for(int i=0; i<len; i++) {
				p[i] = pData[i];
			}
		} else {
			for(int i=0; i<len; i++) {
				p[i] = pData[len-i-1];
			}
		}
		*pRaw += len;
	}
	void BarCode_Print_Prn_Mem_Write_uInt(unsigned char **pRaw, unsigned int d, int endian) {
		unsigned char *p = *pRaw;
		if(endian==PRINTER_PCLXL_ENDIAN_LITTLE) {
			*((unsigned int *)p) = d;
		} else {
			p[0] = (d&0xff000000) >> 24;
			p[1] = (d&0x00ff0000) >> 16;
			p[2] = (d&0x0000ff00) >> 8;
			p[3] = (d&0x000000ff);
		}
		*pRaw += sizeof(unsigned int);
	}
	void BarCode_Print_Prn_Mem_Write_uShort(unsigned char **pRaw, unsigned short d, int endian) {
		unsigned char *p = *pRaw;
		if(endian==PRINTER_PCLXL_ENDIAN_LITTLE) {
			*((unsigned short *)p) = d;
		} else {
			p[0] = (d&0xff00) >> 8;
			p[1] = (d&0xff);
		}
		*pRaw += sizeof(unsigned short);
	}
#elif __BYTE_ORDER == __BIG_ENDIAN

	void BarCode_Print_Prn_Mem_Write(unsigned char **pRaw, unsigned char *pData, int len, int endian) {
		unsigned char *p = *pRaw;
		if(endian==PRINTER_PCLXL_ENDIAN_BIG) {
			for(int i=0; i<len; i++) {
				p[i] = pData[i];
			}
		} else {
			for(int i=0; i<len; i++) {
				p[i] = pData[len-i-1];
			}
		}
		*pRaw += len;
	}
	void BarCode_Print_Prn_Mem_Write_uInt(unsigned char **pRaw, unsigned int d, int endian) {
		unsigned char *p = *pRaw;
		if(endian==PRINTER_PCLXL_ENDIAN_BIG) {
			*((unsigned int *)p) = d;
		} else {
			p[0] = (d&0xff000000) >> 24;
			p[1] = (d&0x00ff0000) >> 16;
			p[2] = (d&0x0000ff00) >> 8;
			p[3] = (d&0x000000ff);
		}
		*pRaw += sizeof(unsigned int);
	}
	void BarCode_Print_Prn_Mem_Write_uShort(unsigned char **pRaw, unsigned short d, int endian) {
		unsigned char *p = *pRaw;
		if(endian==PRINTER_PCLXL_ENDIAN_BIG) {
			*((unsigned short *)p) = d;
		} else {
			p[0] = (d&0xff00) >> 8;
			p[1] = (d&0xff);
		}
		*pRaw += sizeof(unsigned short);
	}
#endif


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





























