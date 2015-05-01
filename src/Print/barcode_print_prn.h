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

#endif /* PRINT_BARCODE_PRINT_PRN_H_ */
