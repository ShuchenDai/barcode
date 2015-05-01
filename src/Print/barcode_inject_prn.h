/*
 * barcode_inject.h
 *
 *  Created on: 2015年5月1日
 *      Author: welkinm
 */

#ifndef PRINT_BARCODE_INJECT_PRN_H_
#define PRINT_BARCODE_INJECT_PRN_H_

//打印指令类型
enum Printer_CMD_Type_TypeDef {
	PRINTER_CMD_NULL = 0,
	PRINTER_CMD_PJL = 1,
	PRINTER_CMD_PCL
};
//打印指令类型
enum Printer_CMD_SubType_TypeDef {
	PRINTER_SUBCMD_NULL = 0,
	PRINTER_SUBCMD_PCL5,
	PRINTER_SUBCMD_PCLXL,
};

int Barcode_Inject_Prn(const char * prnIn, const char *prnOut,
		int barcodeType, const char *barcode, unsigned int barcodeLen);

/*
 * 判断当前包是否为打印文件头
 */
bool Barcode_Inject_Prn_Pkg_Is_Head(unsigned char * buf, unsigned int len, int &cmdType);
int Barcode_Inject_Prn_Find_Str(const unsigned char* data,unsigned int dLen,
		const unsigned char *bStr,unsigned  int bStrLen,
		const unsigned char *eStr,unsigned  int eStrLen,
		int &retIdx,unsigned  int &retLen);
int Barcode_Inject_Prn_Find_Str_Paramete(const unsigned char* data,unsigned int dLen,
		const unsigned char *bStr,unsigned  int bStrLen,
		const unsigned char *eStr,unsigned  int eStrLen,
		int &retIdx, unsigned int &retLen, unsigned char *retStr);
int Barcode_Inject_Prn_Find_Str_Paramete(const unsigned char* cache,unsigned int cLen,
		const unsigned char* data,unsigned int dLen,
		const unsigned char *bStr,unsigned  int bStrLen,
		const unsigned char *eStr,unsigned  int eStrLen,
		int &retIdx, unsigned  int &retLen, unsigned char *retStr);
int Barcode_Inject_Prn_Find_Cmd(const unsigned char* data,unsigned int dLen,
		const unsigned char *bStr,unsigned  int bStrLen,
		const unsigned char *eStr,unsigned  int eStrLen,
		int &retIdx,unsigned  int &retLen);
int Barcode_Inject_Prn_Find_Cmd_Paramete(const unsigned char* data,unsigned int dLen,
		const unsigned char *bStr,unsigned  int bStrLen,
		const unsigned char *eStr,unsigned  int eStrLen,
		int &retIdx, unsigned int &retLen, double &retValue);
int Barcode_Inject_Prn_Find_Cmd_Paramete(const unsigned char* cache,unsigned int cLen,
		const unsigned char* data,unsigned int dLen,
		const unsigned char *bStr,unsigned  int bStrLen,
		const unsigned char *eStr,unsigned  int eStrLen,
		int &retIdx, unsigned  int &retLen, double &retValue);



#endif /* PRINT_BARCODE_INJECT_PRN_H_ */
