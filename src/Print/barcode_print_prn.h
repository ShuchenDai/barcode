/*
 * barcode_print.h
 *
 *  Created on: 2015年4月29日
 *      Author: welkinm
 */

#ifndef PRINT_BARCODE_PRINT_PRN_H_
#define PRINT_BARCODE_PRINT_PRN_H_



int Barcode_Print_Prn_Fill_Buf(const char *barcode, unsigned int barcodeLen, int barcodeType,
		unsigned char *buf, unsigned int bufLen,
		unsigned int dpiPage, unsigned int dpiBmp,
		unsigned int& w, unsigned int& h,
		unsigned char** bmpBegin, unsigned int& bmpLen,
		unsigned char** strBegin, unsigned int& strLen,
		unsigned int& bmpAndStrLen, unsigned int& fileLen);

int Barcode_Print_Prn_Fill_Buf_XL(const char *barcode, unsigned int barcodeLen, int barcodeType,
		unsigned char *buf, unsigned int bufLen,
		unsigned int dpiPage, unsigned int dpiBmp, int endian,
		unsigned int& w, unsigned int& h,
		unsigned char** bmpBegin, unsigned int& bmpLen,
		unsigned char** strBegin, unsigned int& strLen,
		unsigned int& bmpAndStrLen, unsigned int& fileLen);

void BarCode_Print_Prn_Mem_Write(unsigned char **pRaw, unsigned char *pData, int len, int endian);
void BarCode_Print_Prn_Mem_Write_uInt(unsigned char **pRaw, unsigned int d, int endian);
void BarCode_Print_Prn_Mem_Write_uShort(unsigned char **pRaw, unsigned short d, int endian);

#endif /* PRINT_BARCODE_PRINT_PRN_H_ */
