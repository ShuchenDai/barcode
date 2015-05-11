/*
 * barcode_bmp.cpp
 *
 *  Created on: 2015年4月22日
 *      Author: welkinm
 */

#include "barcode_bmp.h"
#include <string.h>
#include <stdio.h>

BarCode_BMPRGBQuad_Type BARCODE_BMP_COLOR_TABLE_1[2] = {
		{0XFF, 0XFF, 0XFF, 0X00},
		{0X00, 0X00, 0X00, 0X00},
};

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////// --------- 内部函数申明 ---------- /////////////////////////////////////////////////////////////////////////////
/*
 * 向buf写入Int或Short，不同处理器有不同endian
 */
void BarCode_BMP_Mem_Write_uInt(unsigned char **pRaw, unsigned int d);
void BarCode_BMP_Mem_Write_uShort(unsigned char **pRaw, unsigned short d);
/*
 * 在1位位图中画一个点
 */
void BarCode_BMP_Mem_Wrire_Pixel_1(BarCode_BMPHead_Type &head, unsigned char *pRaw, unsigned int x, unsigned int y, BarCode_BMPColor_Type &color);
/*
 * 在4位位图中画一个点
 */
void BarCode_BMP_Mem_Wrire_Pixel_4(BarCode_BMPHead_Type &head, unsigned char *pRaw, unsigned int x, unsigned int y, BarCode_BMPColor_Type &color);
/*
 * 在8位位图中画一个点
 */
void BarCode_BMP_Mem_Wrire_Pixel_8(BarCode_BMPHead_Type &head, unsigned char *pRaw, unsigned int x, unsigned int y, BarCode_BMPColor_Type &color);
/*
 * 在16位位图中画一个点
 */
void BarCode_BMP_Mem_Wrire_Pixel_16(BarCode_BMPHead_Type &head, unsigned char *pRaw, unsigned int x, unsigned int y, BarCode_BMPColor_Type &color);
/*
 * 在24位位图中画一个点
 */
void BarCode_BMP_Mem_Wrire_Pixel_24(BarCode_BMPHead_Type &head, unsigned char *pRaw, unsigned int x, unsigned int y, BarCode_BMPColor_Type &color);
/*
 * 在32位位图中画一个点
 */
void BarCode_BMP_Mem_Wrire_Pixel_32(BarCode_BMPHead_Type &head, unsigned char *pRaw, unsigned int x, unsigned int y, BarCode_BMPColor_Type &color);




/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////// --------- 外部函数实现 ---------- /////////////////////////////////////////////////////////////////////////////
/*
 * 根据位图的像素宽度和每行像素数，计算每行实际存储所需的字节数
 */
unsigned int BarCode_BMP_Get_Width_Bypes(unsigned int pixelsCount, unsigned short bitCount) {
	return (((unsigned int)pixelsCount*bitCount+31)>>5)<<2;//((pixelsCount*bitCount+31)/32)*4;
}

/*
 * 根据位图的像素宽和像素高、每像素位数
 * 如果每像素位数为1，4，8，需提供颜色表，颜色表的长度满足: len(colorTable) ＝ pow(2,bitCount)
 */
int BarCode_BMP_Build_Head(BarCode_BMPHead_Type &head,
		unsigned int w, unsigned int h, unsigned short bitCount, BarCode_BMPRGBQuad_Type *colorTable) {
	if(!(bitCount==1 || bitCount==4 || bitCount==8 || bitCount==16 || bitCount==24 || bitCount==32)) return 1;
	//位图每行的实际存储所需的字节数
	head.widthBypes = BarCode_BMP_Get_Width_Bypes(w, bitCount);
	//颜色表
	if(bitCount<24) {
		if(colorTable==NULL) {
			return 1;
		}
		head.ct = colorTable;
		head.ctCount = 1 << bitCount;
	} else {
		head.ct = NULL;
		head.ctCount = 0;
	}
	//BMP文件头数据结构
	head.fh.bfType = BARCODE_BMP_TYPE;
	head.fh.bfSize = BARCODE_BMP_FILE_HEADER_LENGTH + BARCODE_BMP_INFORMATION_HEADER_LENGTH + head.ctCount*sizeof(BarCode_BMPColor_Type) + head.widthBypes*h;
	head.fh.bfReserved1 = 0;
	head.fh.bfReserved2 = 0;
	head.fh.bfOffBits = BARCODE_BMP_FILE_HEADER_LENGTH + BARCODE_BMP_INFORMATION_HEADER_LENGTH + head.ctCount*sizeof(BarCode_BMPColor_Type);
	//BMP位图信息
	head.ih.biSize = BARCODE_BMP_INFORMATION_HEADER_LENGTH;
	head.ih.biWidth = w;
	head.ih.biHeight = h;
	head.ih.biPlanes = 1;
	head.ih.biBitCount = bitCount;
	head.ih.biCompression = 0;
	head.ih.biSizeImage = head.widthBypes*h;
	head.ih.biXPelsPerMeter = 0x0b12;
	head.ih.biYPelsPerMeter = 0x0b12;
	head.ih.biClrUsed = 0;
	head.ih.biClrImportant = 0;
	return 0;
}

/*
 * 将BMP相关描述信息写入指定buf
 */
int BarCode_BMP_Mem_Write_Head(BarCode_BMPHead_Type &head, unsigned char *buf, unsigned int bufLen) {
	unsigned char *p = buf;
	//检查类型
	if(head.fh.bfType!=BARCODE_BMP_TYPE || head.ih.biCompression!=0) return 1;
	//检查缓冲区是否足够
	if(bufLen<head.fh.bfSize) return 1;
	//BMP文件头数据结构
	BarCode_BMP_Mem_Write_uShort(&p, head.fh.bfType);
	BarCode_BMP_Mem_Write_uInt(&p, head.fh.bfSize);
	BarCode_BMP_Mem_Write_uShort(&p, head.fh.bfReserved1);
	BarCode_BMP_Mem_Write_uShort(&p, head.fh.bfReserved2);
	BarCode_BMP_Mem_Write_uInt(&p, head.fh.bfOffBits);
	//BMP位图信息
	BarCode_BMP_Mem_Write_uInt(&p, head.ih.biSize);
	BarCode_BMP_Mem_Write_uInt(&p, head.ih.biWidth);
	BarCode_BMP_Mem_Write_uInt(&p, head.ih.biHeight);
	BarCode_BMP_Mem_Write_uShort(&p, head.ih.biPlanes);
	BarCode_BMP_Mem_Write_uShort(&p, head.ih.biBitCount);
	BarCode_BMP_Mem_Write_uInt(&p, head.ih.biCompression);
	BarCode_BMP_Mem_Write_uInt(&p, head.ih.biSizeImage);
	BarCode_BMP_Mem_Write_uInt(&p, head.ih.biXPelsPerMeter);
	BarCode_BMP_Mem_Write_uInt(&p, head.ih.biYPelsPerMeter);
	BarCode_BMP_Mem_Write_uInt(&p, head.ih.biClrUsed);
	BarCode_BMP_Mem_Write_uInt(&p, head.ih.biClrImportant);
	//颜色表
	if(head.ctCount>0) {
		memcpy(p, head.ct, head.ctCount*sizeof(BarCode_BMPColor_Type));
	}
	return 0;
}

/*
 * 填充背景色
 */
int BarCode_BMP_Mem_Wrire_BK_Color(BarCode_BMPHead_Type &head, unsigned char *buf, BarCode_BMPColor_Type &bgColor) {
	//填充背景色
	unsigned char *p = buf+head.fh.bfOffBits;
	unsigned char c;
	unsigned short s;
	unsigned int i=0, j=0, k=0;
	unsigned int h = head.ih.biHeight;
	unsigned int w = head.ih.biWidth;
	switch(head.ih.biBitCount) {
	case 1:
		c = bgColor.idx.i&1? 0xFF: 0;
		memset(buf+head.fh.bfOffBits, c, head.ih.biSizeImage);
		break;
	case 4:
		c = bgColor.idx.i&0x0F;
		c |= c<<4;
		memset(buf+head.fh.bfOffBits, c, head.ih.biSizeImage);
		break;
	case 8:
		c = bgColor.idx.i&0xFF;
		memset(buf+head.fh.bfOffBits, c, head.ih.biSizeImage);
		break;
	case 16:
		s = bgColor.idx.i;
		for(i=0; i<head.ih.biSizeImage; i+=2) {
			BarCode_BMP_Mem_Write_uShort(&p, s);
		}
		break;
	case 24:
		for(i=0; i<h; i++) {
			for(j=0; j<w; j++) {
				k = i*head.widthBypes + j*3;
				p[k++] = bgColor.rgb.b;
				p[k++] = bgColor.rgb.g;
				p[k] = bgColor.rgb.r;
			}
		}
		break;
	case 32:
		for(i=0; i<h; i++) {
			for(j=0; j<w; j++) {
				k = i*head.widthBypes + j*4;
				p[k++] = bgColor.rgb.b;
				p[k++] = bgColor.rgb.g;
				p[k++] = bgColor.rgb.r;
				p[k] = bgColor.rgb.a;
			}
		}
		break;
	default:
		return 1;
	}
	return 0;
}

/*
 * 方便初始化位图
 */
int BarCode_BMP_Mem_Write_Default(BarCode_BMPHead_Type &head, unsigned char *buf, unsigned int bufLen,
		unsigned int& w, unsigned int& h, unsigned short bitCount, BarCode_BMPRGBQuad_Type *colorTable,
		BarCode_BMPColor_Type &bgColor) {
	//设置BMP文件头信息
	int ret = BarCode_BMP_Build_Head(head, w, h, bitCount, colorTable);
	if(ret!=0) return 1;
	//写入BMP头信息
	ret = BarCode_BMP_Mem_Write_Head(head, buf, bufLen);
	if(ret!=0) return 1;
	//填充背景色
	ret = BarCode_BMP_Mem_Wrire_BK_Color(head, buf, bgColor);
	return ret;
}

/*
 * 画一个像素
 */
int BarCode_BMP_Mem_Wrire_Pixel(BarCode_BMPHead_Type &head, unsigned char *buf,
		unsigned int x, unsigned int y, BarCode_BMPColor_Type &color) {
	y = head.ih.biHeight - y;
	unsigned char *p = buf+head.fh.bfOffBits;
	switch(head.ih.biBitCount) {
		case 1:
			BarCode_BMP_Mem_Wrire_Pixel_1(head, p, x, y, color);
			break;
		case 4:
			BarCode_BMP_Mem_Wrire_Pixel_4(head, p, x, y, color);
			break;
		case 8:
			BarCode_BMP_Mem_Wrire_Pixel_8(head, p, x, y, color);
			break;
		case 16:
			BarCode_BMP_Mem_Wrire_Pixel_16(head, p, x, y, color);
			break;
		case 24:
			BarCode_BMP_Mem_Wrire_Pixel_24(head, p, x, y, color);
			break;
		case 32:
			BarCode_BMP_Mem_Wrire_Pixel_32(head, p, x, y, color);
			break;
		default:
			return 1;
	}
	return 0;
}

int BarCode_BMP_Mem_Fill_Rect(BarCode_BMPHead_Type &head, unsigned char *buf, unsigned int x, unsigned int y,
		unsigned int w, unsigned int h, BarCode_BMPColor_Type &color) {
	if(x>=head.ih.biWidth) return 1;
	if(y>=head.ih.biHeight) return 1;
	if(x+w>head.ih.biWidth) return 1;
	if(y+h>head.ih.biHeight) return 1;
	unsigned char *p = buf+head.fh.bfOffBits;
	unsigned int xBegin = x;
	unsigned int xEnd = x+w;
	unsigned int yBegin = head.ih.biHeight-(y+h);
	unsigned int yEnd = yBegin+h;
	unsigned  i,j;
	switch(head.ih.biBitCount) {
		case 1:
			for(i=yBegin; i<yEnd; i++) {
				for(j=xBegin; j<xEnd; j++) {
					BarCode_BMP_Mem_Wrire_Pixel_1(head, p, j, i, color);
				}
			}
			break;
		case 4:
			for(i=yBegin; i<yEnd; i++) {
				for(j=xBegin; j<xEnd; j++) {
					BarCode_BMP_Mem_Wrire_Pixel_4(head, p, j, i, color);
				}
			}
			break;
		case 8:
			for(i=yBegin; i<yEnd; i++) {
				for(j=xBegin; j<xEnd; j++) {
					BarCode_BMP_Mem_Wrire_Pixel_8(head, p, j, i, color);
				}
			}
			break;
		case 16:
			for(i=yBegin; i<yEnd; i++) {
				for(j=xBegin; j<xEnd; j++) {
					BarCode_BMP_Mem_Wrire_Pixel_16(head, p, j, i, color);
				}
			}
			break;
		case 24:
			for(i=yBegin; i<yEnd; i++) {
				for(j=xBegin; j<xEnd; j++) {
					BarCode_BMP_Mem_Wrire_Pixel_24(head, p, j, i, color);
				}
			}
			break;
		case 32:
			for(i=yBegin; i<yEnd; i++) {
				for(j=xBegin; j<xEnd; j++) {
					BarCode_BMP_Mem_Wrire_Pixel_32(head, p, j, i, color);
				}
			}
			break;
		default:
			return 1;
	}
	return 0;
}








//int BarCode_BMP_Mem_Turn_24_To_1(BarCode_BMPHead_Type &head, unsigned char *buf, BarCode_BMPHead_Type &headNew, unsigned char *bufNew, int bgColor) {
//	int w = head.ih.biWidth;
//	int h = head.ih.biHeight;
//	unsigned int wNew;
//	BarCode_BMP_Build_Normal_Head_1(&headNew, wNew, h);
//	BarCode_BMP_Mem_Wrire_BK_Color_1(headNew, bufNew, bgColor);
//	unsigned char *p = buf + BARCODE_BMP_FILE_HEADER_LENGTH + BARCODE_BMP_INFORMATION_HEADER_LENGTH;
//	unsigned char *pNew = bufNew + BARCODE_BMP_FILE_HEADER_LENGTH + BARCODE_BMP_INFORMATION_HEADER_LENGTH;
//	int k = 0;
//	unsigned char *pC;
//	for(int i=0; i<h; i++) {
//		for(int j=0; j<w; j++) {
//			k = (i*head.alignedWidth+j)*3;
//			pC = pNew[i*headNew.alignedWidth + j/8];
//			if(p[k]+p[k+1]+p[k+2] > 382) {
//				pNew[i*j/8] = pNew[j/8]|(1<<(8-(j%8)));
//			}
//			else
////			    		p[k++] = rgb.rgbRed;
////						p[k++] = rgb.rgbGreen;
////						p[k] = rgb.rgbBlue;
//		}
//	}
//	return 0;
//}
//




/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////// --------- 辅助函数 ---------- /////////////////////////////////////////////////////////////////////////////////
/*
 * 在4位位图中画一个点
 */
void BarCode_BMP_Mem_Wrire_Pixel_1(BarCode_BMPHead_Type &head, unsigned char *pRaw,
		unsigned int x, unsigned int y, BarCode_BMPColor_Type &color) {
//	y = head.ih.biHeight-y;
	unsigned int pos = y*head.widthBypes + x/8;
	unsigned char mask = 1<<(7-x%8);
	if(color.idx.i&1) {
		pRaw[pos] = pRaw[pos]|mask;
	} else {
		pRaw[pos] = pRaw[pos]&(~mask);
	}
}
/*
 * 在4位位图中画一个点
 */
void BarCode_BMP_Mem_Wrire_Pixel_4(BarCode_BMPHead_Type &head, unsigned char *pRaw,
		unsigned int x, unsigned int y, BarCode_BMPColor_Type &color) {
	unsigned int pos = y*head.widthBypes + x/2;
	unsigned char mask = 0xF<<((1-(x%2))<<2);
	pRaw[pos] = (pRaw[pos]&(~mask)) | (color.idx.i&mask);
}
/*
 * 在8位位图中画一个点
 */
void BarCode_BMP_Mem_Wrire_Pixel_8(BarCode_BMPHead_Type &head, unsigned char *pRaw,
		unsigned int x, unsigned int y, BarCode_BMPColor_Type &color) {
	unsigned int pos = y*head.widthBypes + x;
	pRaw[pos] = (unsigned char)color.idx.i;
}
/*
 * 在16位位图中画一个点
 */
void BarCode_BMP_Mem_Wrire_Pixel_16(BarCode_BMPHead_Type &head, unsigned char *pRaw,
		unsigned int x, unsigned int y, BarCode_BMPColor_Type &color) {
//	unsigned int pos = y*head.widthBypes + x*2;
	BarCode_BMP_Mem_Write_uShort(&pRaw, color.idx.i);
}
/*
 * 在24位位图中画一个点
 */
void BarCode_BMP_Mem_Wrire_Pixel_24(BarCode_BMPHead_Type &head, unsigned char *pRaw,
		unsigned int x, unsigned int y, BarCode_BMPColor_Type &color) {
	unsigned int pos = y*head.widthBypes + x*3;
	pRaw[pos++] = color.rgb.b;
	pRaw[pos++] = color.rgb.g;
	pRaw[pos] = color.rgb.r;
}
/*
 * 在32位位图中画一个点
 */
void BarCode_BMP_Mem_Wrire_Pixel_32(BarCode_BMPHead_Type &head, unsigned char *pRaw,
		unsigned int x, unsigned int y, BarCode_BMPColor_Type &color) {
	unsigned int pos = y*head.widthBypes + x*4;
	pRaw[pos++] = color.rgb.b;
	pRaw[pos++] = color.rgb.g;
	pRaw[pos++] = color.rgb.r;
	pRaw[pos] = color.rgb.a;
}




#if __BYTE_ORDER == __LITTLE_ENDIAN

void BarCode_BMP_Mem_Write_uInt(unsigned char **pRaw, unsigned int d) {
	unsigned char *p = *pRaw;
	*((unsigned int *)p) = d;
	*pRaw += sizeof(unsigned int);
}
void BarCode_BMP_Mem_Write_uShort(unsigned char **pRaw, unsigned short d) {
	unsigned char *p = *pRaw;
	*((unsigned short *)p) = d;
	*pRaw += sizeof(unsigned short);
}

#elif __BYTE_ORDER == __BIG_ENDIAN

void static BarCode_BMP_Mem_Write_uInt(unsigned char **pRaw, unsigned int d) {
	unsigned char *p = *pRaw;
	p[0] = (d&0xff000000) >> 24;
	p[1] = (d&0x00ff0000) >> 16;
	p[2] = (d&0x0000ff00) >> 8;
	p[3] = (d&0x000000ff);
	*pRaw += sizeof(unsigned int);
}
void static BarCode_BMP_Mem_Write_uShort(unsigned char **pRaw, unsigned short d) {
	unsigned char *p = *pRaw;
	p[0] = (d&0xff00) >> 8;
	p[1] = (d&0xff);
	*pRaw += sizeof(unsigned short);
}

#endif

