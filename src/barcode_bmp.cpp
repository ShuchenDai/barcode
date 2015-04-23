/*
 * barcode_bmp.cpp
 *
 *  Created on: 2015年4月22日
 *      Author: welkinm
 */

#include "barcode_bmp.h"

int BarCode_BMP_Build_Normal_Head(BarCode_BMPHead_Type *head, unsigned int& w, unsigned int h) {
	if(!head) return 1;
	head->alignedWidth = (w * 8 + 31) / 32 * 4;
	w = head->alignedWidth;
	head->curPadNum = head->alignedWidth - w;

	head->fh.bfType = BARCODE_BMP_TYPE;
	head->fh.bfSize = BARCODE_BMP_FILE_HEADER_LENGTH + BARCODE_BMP_INFORMATION_HEADER_LENGTH + head->alignedWidth*h*3;
	head->fh.bfReserved1 = 0;
	head->fh.bfReserved2 = 0;
	head->fh.bfOffBits = BARCODE_BMP_FILE_HEADER_LENGTH + BARCODE_BMP_INFORMATION_HEADER_LENGTH;

	head->ih.biSize = BARCODE_BMP_INFORMATION_HEADER_LENGTH;
	head->ih.biWidth = head->alignedWidth;
	head->ih.biHeight = h;
	head->ih.biPlanes = 1;
	head->ih.biBitCount = 24;
	head->ih.biCompression = 0;
	head->ih.biSizeImage = w * h * 3;// + head->filePadNum;
	head->ih.biXPelsPerMeter = 0x0b12;
	head->ih.biYPelsPerMeter = 0x0b12;
	head->ih.biClrUsed = 0;
	head->ih.biClrImportant = 0;
	return 0;
}

int BarCode_BMP_Mem_Write_Head(BarCode_BMPHead_Type *fh, unsigned char *buf, unsigned int len) {
	unsigned char *p = buf;
	if(!fh) return 1;
	if(len<BARCODE_BMP_FILE_HEADER_LENGTH + BARCODE_BMP_INFORMATION_HEADER_LENGTH) return 1;
	BarCode_BMP_Mem_Write_uShort(&p, fh->fh.bfType);
	BarCode_BMP_Mem_Write_uInt(&p, fh->fh.bfSize);
	BarCode_BMP_Mem_Write_uShort(&p, fh->fh.bfReserved1);
	BarCode_BMP_Mem_Write_uShort(&p, fh->fh.bfReserved2);
	BarCode_BMP_Mem_Write_uInt(&p, fh->fh.bfOffBits);

	BarCode_BMP_Mem_Write_uInt(&p, fh->ih.biSize);
	BarCode_BMP_Mem_Write_uInt(&p, fh->ih.biWidth);
	BarCode_BMP_Mem_Write_uInt(&p, fh->ih.biHeight);
	BarCode_BMP_Mem_Write_uShort(&p, fh->ih.biPlanes);
	BarCode_BMP_Mem_Write_uShort(&p, fh->ih.biBitCount);
	BarCode_BMP_Mem_Write_uInt(&p, fh->ih.biCompression);
	BarCode_BMP_Mem_Write_uInt(&p, fh->ih.biSizeImage);
	BarCode_BMP_Mem_Write_uInt(&p, fh->ih.biXPelsPerMeter);
	BarCode_BMP_Mem_Write_uInt(&p, fh->ih.biYPelsPerMeter);
	BarCode_BMP_Mem_Write_uInt(&p, fh->ih.biClrUsed);
	BarCode_BMP_Mem_Write_uInt(&p, fh->ih.biClrImportant);
	return 0;
}


int BarCode_BMP_Mem_Wrire_BK_Color(BarCode_BMPHead_Type &head, unsigned char *buf, BarCode_BMRGBQuad_Type &rgb)
{
    unsigned char *p = buf + BARCODE_BMP_FILE_HEADER_LENGTH + BARCODE_BMP_INFORMATION_HEADER_LENGTH;
//    unsigned int len = head.ih.biSizeImage;
    unsigned int h, w;
    unsigned int i=0, j=0, k=0;
    h = head.ih.biHeight;
    w = head.ih.biWidth;
    for (i=0; i<h; i++) {
    	for(j=0; j<w; j++) {
    		k = (i*head.alignedWidth+j)*3;
    		p[k++] = rgb.rgbRed;
			p[k++] = rgb.rgbGreen;
			p[k] = rgb.rgbBlue;
    	}
    }
    return 0;
}

int BarCode_BMP_Mem_Wrire_Pixel(BarCode_BMPHead_Type &head, unsigned char *buf,
		unsigned int x, unsigned int y, BarCode_BMRGBQuad_Type &rgb) {
	unsigned char *p = buf + BARCODE_BMP_FILE_HEADER_LENGTH + BARCODE_BMP_INFORMATION_HEADER_LENGTH;
	unsigned int k = y*head.ih.biWidth+x;
	p[k] = rgb.rgbRed;
	p[k] = rgb.rgbGreen;
	p[k] = rgb.rgbBlue;
	return 0;
}

int BarCode_BMP_Mem_Fill_Rect(BarCode_BMPHead_Type &head, unsigned char *buf, unsigned int x, unsigned int y,
		unsigned int w, unsigned int h, BarCode_BMRGBQuad_Type &rgb) {
	if(x>=head.ih.biWidth) return 1;
	if(y>=head.ih.biHeight) return 1;
	if(x+w>head.ih.biWidth) return 1;
	if(y+h>head.ih.biHeight) return 1;
	unsigned char *p = buf + BARCODE_BMP_FILE_HEADER_LENGTH + BARCODE_BMP_INFORMATION_HEADER_LENGTH;
	unsigned  i,j,k;
	unsigned yEnd = y+h;
	unsigned xEnd = x+w;
	for(i=y; i<yEnd; i++) {
		for(j=x; j<xEnd; j++) {
			k=3*(i*head.alignedWidth+j);
			p[k+0] = rgb.rgbRed;
			p[k+1] = rgb.rgbGreen;
			p[k+2] = rgb.rgbBlue;
		}
	}
	return 0;
}





#if __BYTE_ORDER == __LITTLE_ENDIAN

void BarCode_BMP_Mem_Write_uInt(unsigned char **buf, unsigned int d) {
	unsigned char *p = *buf;
	*((unsigned int *)p) = d;
	*buf += sizeof(unsigned int);
}
void BarCode_BMP_Mem_Write_uShort(unsigned char **buf, unsigned short d) {
	unsigned char *p = *buf;
	*((unsigned short *)p) = d;
	*buf += sizeof(unsigned short);
}

#elif __BYTE_ORDER == __BIG_ENDIAN

void static BarCode_BMP_Mem_Write_uInt(unsigned char **buf, unsigned int d) {
	unsigned char *p = *buf;
	buf[0] = (d&0xff000000) >> 24;
	buf[1] = (d&0x00ff0000) >> 16;
	buf[2] = (d&0x0000ff00) >> 8;
	buf[3] = (d&0x000000ff);
	*buf += sizeof(unsigned int);
}
void static BarCode_BMP_Mem_Write_uShort(unsigned char **buf, unsigned short d) {
	unsigned char *p = *buf;
	buf[0] = (d&0xff00) >> 8;
	buf[1] = (d&0xff);
	*buf += sizeof(unsigned short);
}

#endif

