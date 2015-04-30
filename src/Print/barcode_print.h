/*
 * barcode_print.h
 *
 *  Created on: 2015年4月29日
 *      Author: welkinm
 */

#ifndef PRINT_BARCODE_PRINT_H_
#define PRINT_BARCODE_PRINT_H_



int Barcode_Print_Fill_Buf(const char *barcode, unsigned int barcodeLen,
		unsigned char *buf, unsigned int bufLen,
		unsigned int dpiPage, unsigned int dpiBmp,
		unsigned int& w, unsigned int& h,
		unsigned char** bmpBegin, unsigned int& bmpLen,
		unsigned char** strBegin, unsigned int& strLen,
		unsigned int& fileLen);

#endif /* PRINT_BARCODE_PRINT_H_ */
