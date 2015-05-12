/*
 * barcode_inject.cpp
 *
 *  Created on: 2015年5月1日
 *      Author: welkinm
 */

#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>

#include "../EAN13/EAN13_fill.h"
#include "../Code128/Code128_fill.h"
#include "../barcode_bmp.h"
#include "barcode_inject_prn.h"
#include "barcode_print_prn.h"
#include "../barcode.h"

#define BARCODE_INJECT_PRN_RESERVED_CMD_LEN 32
#define BARCODE_INJECT_PRN_RESERVED_CMD_LEN_D BARCODE_INJECT_PRN_RESERVED_CMD_LEN*2
#define BARCODE_INJECT_PRN_DEFAULT_RESOLUTION 300
#define BARCODE_INJECT_PRN_DEFAULT_PKG_SIZE 512

#define BARCODE_INJECT_BMP_BUF_LEN 1024*100


int Barcode_Inject_Prn(const char * prnIn, const char *prnOut,
		int barcodeType, const char *barcode, unsigned int barcodeLen) {
	int ret=0;
	//打开输入输出文档
	FILE *fin, *fout;
	fin = fopen(prnIn, "r");
	if(fin<=0) return 1;
	fout = fopen(prnOut, "w");
	if(fout<=0) {fclose(fin); return 1;}

	//生成条码相关变量
	unsigned int dpiPage = BARCODE_INJECT_PRN_DEFAULT_RESOLUTION, dpiBmp = BARCODE_INJECT_PRN_DEFAULT_RESOLUTION;
	unsigned int w = 0;
	unsigned int h = 0;
	unsigned char *bmpBegin;
	unsigned int bmpLen;
	unsigned char *strBegin;
	unsigned int strLen;
	unsigned int bmpAndStrLen;
	unsigned int fileLen;

	//解析PRN文件相关变量
    unsigned char pData[BARCODE_INJECT_PRN_DEFAULT_PKG_SIZE];
//    bool isFirstPkg = false;		//是否是文件头
    bool isFoundDpiPage = false;	//是否己找到设置打印页面分辨率
    bool isFoundDpiBmp = false;	//是否己找到设置位图分辨率
    bool isFillBmp = false;		//是否己在打印文件中填充位图
    int printerCMDType = PRINTER_CMD_NULL;
	int printerSubCMDType = PRINTER_SUBCMD_NULL;
	bool isFoundSubCMDType = false;
	unsigned char interceptCmd[BARCODE_INJECT_PRN_RESERVED_CMD_LEN_D];
	//PCLXL相关
	unsigned char beginStr[16];
	unsigned char endStr[16];
	int pclxlEndian = PRINTER_PCLXL_ENDIAN_LITTLE;
	bool isFoundPclxlEndian = false;
	//查找函数使用变量
    unsigned char temp[128];
    int retIdx = 0;
    unsigned int retLen;
	double value;
	//标识当前解析包是否己写入新文件，以防重复写入
	bool alreadyWrite;
	//生成位图缓存
	BarcodeDataAutoPtr barcodeDataAutoPtr(new BarcodeData(BARCODE_INJECT_BMP_BUF_LEN));
	unsigned char *buf = barcodeDataAutoPtr.get()->data;
	//开始解析PRN文件
	unsigned int dLen = fread(pData, 1, BARCODE_INJECT_PRN_DEFAULT_PKG_SIZE, fin);
    while(dLen>0) {
    	alreadyWrite = false;
    	//处理头，查找PRN文件类型
		if(Barcode_Inject_Prn_Pkg_Is_Head(pData, dLen, printerCMDType)) {
			printf("get head\n");
			printerSubCMDType = PRINTER_SUBCMD_NULL;
			isFoundSubCMDType = false;
			isFoundDpiPage = false;
			isFoundDpiBmp = false;
			isFillBmp = false;
			dpiPage = BARCODE_INJECT_PRN_DEFAULT_RESOLUTION;
			dpiBmp = BARCODE_INJECT_PRN_DEFAULT_RESOLUTION;
			//PCLXL相关
			pclxlEndian = PRINTER_PCLXL_ENDIAN_LITTLE;
			isFoundPclxlEndian = false;
			memset(interceptCmd+BARCODE_INJECT_PRN_RESERVED_CMD_LEN, 0, BARCODE_INJECT_PRN_RESERVED_CMD_LEN);
		}
		//当还没有注入条码前，连续查找合适地方注入
		if(isFillBmp==false) {
			int pkgLen = BARCODE_INJECT_PRN_RESERVED_CMD_LEN>dLen? dLen: BARCODE_INJECT_PRN_RESERVED_CMD_LEN;
			memcpy(interceptCmd, interceptCmd+BARCODE_INJECT_PRN_RESERVED_CMD_LEN, BARCODE_INJECT_PRN_RESERVED_CMD_LEN);
			memcpy(interceptCmd+BARCODE_INJECT_PRN_RESERVED_CMD_LEN, pData, pkgLen);
			//如果文件子类型为空，需先解析子文件类型
			if(isFoundSubCMDType==false) {
//				printf("finding subCmdtype\n");
				//对于PJL，解析文件的子类型
				if(printerCMDType==PRINTER_CMD_PJL) {
					//查找PRN文件子类型
					ret = Barcode_Inject_Prn_Find_Str_Paramete(interceptCmd, BARCODE_INJECT_PRN_RESERVED_CMD_LEN_D, pData, dLen,
							(const unsigned char *)"@PJL ENTER LANGUAGE=", sizeof("@PJL ENTER LANGUAGE=")-1,
							(const unsigned char *)"\n", 1, retIdx, retLen, temp);
					if(ret==0) {
						isFoundSubCMDType = true;
						if(strcmp("PCL", (const char*)temp)==0) {
							printerSubCMDType = PRINTER_SUBCMD_PCL5;
							printf("FoundSubCMDType PCL \n");
						} else if(strcmp("PCLXL", (const char*)temp)==0) {
							printerSubCMDType = PRINTER_SUBCMD_PCLXL;
							printf("FoundSubCMDType PCLXL \n");
						} else {
							isFoundSubCMDType = false;
							printerSubCMDType = PRINTER_SUBCMD_NULL;
							printf("FoundSubCMDType NULL \n");
						}
					}
				} else if(printerCMDType==PRINTER_CMD_PCL) {
					//对于PJL，文件子类型默认都为PJL5
					printerSubCMDType = PRINTER_SUBCMD_PCL5;
					isFoundSubCMDType = true;
				}
			}
			//只有在解析到文件子类型后，才能查找条码注入点
			if(isFoundSubCMDType) {
				//对于PCL5文件处理
				if(printerSubCMDType == PRINTER_SUBCMD_PCL5) {
					//查找文档 DPI
					if(isFoundDpiPage==false) {
						ret = Barcode_Inject_Prn_Find_Cmd_Paramete(interceptCmd, BARCODE_INJECT_PRN_RESERVED_CMD_LEN_D, pData, dLen,
								(const unsigned char *)"\x1b&u", sizeof("\x1b&u")-1, (const unsigned char *)"D", 1, retIdx, retLen, value);
						if(ret==0) {
							isFoundDpiPage = true;
//							printf("FoundDpiPage %f \n", value);
							dpiPage = value;
						}
					}
					//查找位图 DPI
					if(isFoundDpiBmp==false) {
						ret = Barcode_Inject_Prn_Find_Cmd_Paramete(interceptCmd, BARCODE_INJECT_PRN_RESERVED_CMD_LEN_D, pData, dLen,
								(const unsigned char *)"\x1b*t", sizeof("\x1b*t")-1, (const unsigned char *)"R", 1, retIdx, retLen, value);
 						if(ret==0) {
							isFoundDpiBmp = true;
//							printf("FoundDpiBmp %f \n", value);
							dpiBmp = value;
						}
					}

					if(isFillBmp==false) {
						//First move point Vertical Unit
						ret = Barcode_Inject_Prn_Find_Cmd_Paramete(interceptCmd, BARCODE_INJECT_PRN_RESERVED_CMD_LEN_D, pData, dLen,
								(const unsigned char *)"\x1b*p", sizeof("\x1b*p")-1, (const unsigned char *)"Y", 1, retIdx, retLen, value);
						if(ret==0) {
//							printf("get Cursor Position Vertical(Unit): idx:%d, len:%d value:%f\n", retIdx, retLen, value);
							ret = Barcode_Print_Prn_Fill_Buf(barcode, barcodeLen, barcodeType, buf, BARCODE_INJECT_BMP_BUF_LEN, dpiPage, dpiBmp, w, h,
									&bmpBegin, bmpLen, &strBegin,  strLen, bmpAndStrLen, fileLen);
							if(ret!=0) {
								printf("%s\n", "Barcode_Print_Fill_Buf err!");
								ret = 1;
								break;
							}
							fwrite(pData, 1, retIdx+retLen, fout);
							fwrite(bmpBegin, 1, bmpAndStrLen, fout);
							fwrite(pData+retIdx+retLen, 1, dLen-retIdx-retLen, fout);
							alreadyWrite = true;
							isFillBmp = true;
						}
					}
					if(isFillBmp==false) {
						//First move point Vertical Decipoints
						ret = Barcode_Inject_Prn_Find_Cmd_Paramete(interceptCmd, BARCODE_INJECT_PRN_RESERVED_CMD_LEN_D, pData, dLen,
								(const unsigned char *)"\x1b&a", sizeof("\x1b&a")-1, (const unsigned char *)"V", 1, retIdx, retLen, value);
						if(ret==0) {
//							printf("get Cursor Position Vertical(Decipoints): idx:%d, len:%d value:%f\n", retIdx, retLen, value);
							ret = Barcode_Print_Prn_Fill_Buf(barcode, barcodeLen, barcodeType, buf, BARCODE_INJECT_BMP_BUF_LEN, dpiPage, dpiBmp, w, h,
									&bmpBegin, bmpLen, &strBegin,  strLen, bmpAndStrLen, fileLen);
							if(ret!=0) {
								printf("%s\n", "Barcode_Print_Fill_Buf err!");
								ret = 1;
								break;
							}
							fwrite(pData, 1, retIdx+retLen, fout);
							fwrite(bmpBegin, 1, bmpAndStrLen, fout);
							fwrite(pData+retIdx+retLen, 1, dLen-retIdx-retLen, fout);
							alreadyWrite = true;
							isFillBmp = true;
						}
					}
					if(isFillBmp==false) {
						//First move point Vertical Rows
						ret = Barcode_Inject_Prn_Find_Cmd_Paramete(interceptCmd, BARCODE_INJECT_PRN_RESERVED_CMD_LEN_D, pData, dLen,
								(const unsigned char *)"\x1b&a", sizeof("\x1b&a")-1, (const unsigned char *)"R", 1, retIdx, retLen, value);
						if(ret==0) {
//							printf("get Cursor Position Vertical(Rows): idx:%d, len:%d value:%f\n", retIdx, retLen, value);
							ret =Barcode_Print_Prn_Fill_Buf(barcode, barcodeLen, barcodeType, buf, BARCODE_INJECT_BMP_BUF_LEN, dpiPage, dpiBmp, w, h,
									&bmpBegin, bmpLen, &strBegin,  strLen, bmpAndStrLen, fileLen);
							if(ret!=0) {
								printf("%s\n", "Barcode_Print_Fill_Buf err!");
								ret = 1;
								break;
							}
							fwrite(pData, 1, retIdx+retLen, fout);
							fwrite(bmpBegin, 1, bmpAndStrLen, fout);
							fwrite(pData+retIdx+retLen, 1, dLen-retIdx-retLen, fout);
							alreadyWrite = true;
							isFillBmp = true;
						}
					}


					if(isFillBmp==false) {
						//First move point Horizontal Unit
						ret = Barcode_Inject_Prn_Find_Cmd_Paramete(interceptCmd, BARCODE_INJECT_PRN_RESERVED_CMD_LEN_D, pData, dLen,
								(const unsigned char *)"\x1b*p", sizeof("\x1b*p")-1, (const unsigned char *)"X", 1, retIdx, retLen, value);
						if(ret==0) {
//							printf("get Cursor Position Horizontal(Unit): idx:%d, len:%d value:%f\n", retIdx, retLen, value);
							ret = Barcode_Print_Prn_Fill_Buf(barcode, barcodeLen, barcodeType, buf, BARCODE_INJECT_BMP_BUF_LEN, dpiPage, dpiBmp, w, h,
									&bmpBegin, bmpLen, &strBegin,  strLen, bmpAndStrLen, fileLen);
							if(ret!=0) {
								printf("%s\n", "Barcode_Print_Fill_Buf err!");
								ret = 1;
								break;
							}
							fwrite(pData, 1, retIdx+retLen, fout);
							fwrite(bmpBegin, 1, bmpAndStrLen, fout);
							fwrite(pData+retIdx+retLen, 1, dLen-retIdx-retLen, fout);
							alreadyWrite = true;
							isFillBmp = true;
						}
					}
					if(isFillBmp==false) {
						//First move point Horizontal Decipoints
						ret = Barcode_Inject_Prn_Find_Cmd_Paramete(interceptCmd, BARCODE_INJECT_PRN_RESERVED_CMD_LEN_D, pData, dLen,
								(const unsigned char *)"\x1b&a", sizeof("\x1b&a")-1, (const unsigned char *)"H", 1, retIdx, retLen, value);
						if(ret==0) {
//							printf("get Cursor Position Horizontal(Decipoints): idx:%d, len:%d value:%f\n", retIdx, retLen, value);
							ret = Barcode_Print_Prn_Fill_Buf(barcode, barcodeLen, barcodeType, buf, BARCODE_INJECT_BMP_BUF_LEN, dpiPage, dpiBmp, w, h,
									&bmpBegin, bmpLen, &strBegin,  strLen, bmpAndStrLen, fileLen);
							if(ret!=0) {
								printf("%s\n", "Barcode_Print_Fill_Buf err!");
								ret = 1;
								break;
							}
							fwrite(pData, 1, retIdx+retLen, fout);
							fwrite(bmpBegin, 1, bmpAndStrLen, fout);
							fwrite(pData+retIdx+retLen, 1, dLen-retIdx-retLen, fout);
							alreadyWrite = true;
							isFillBmp = true;
						}
					}
					if(isFillBmp==false) {
						//First move point Horizontal Columns
						ret = Barcode_Inject_Prn_Find_Cmd_Paramete(interceptCmd, BARCODE_INJECT_PRN_RESERVED_CMD_LEN_D, pData, dLen,
								(const unsigned char *)"\x1b&a", sizeof("\x1b&a")-1, (const unsigned char *)"C", 1, retIdx, retLen, value);
						if(ret==0) {
//							printf("get Cursor Position Horizontal(Columns): idx:%d, len:%d value:%f\n", retIdx, retLen, value);
							ret = Barcode_Print_Prn_Fill_Buf(barcode, barcodeLen, barcodeType, buf, BARCODE_INJECT_BMP_BUF_LEN, dpiPage, dpiBmp, w, h,
									&bmpBegin, bmpLen, &strBegin,  strLen, bmpAndStrLen, fileLen);
							if(ret!=0) {
								printf("%s\n", "Barcode_Print_Fill_Buf err!");
								ret = 1;
								break;
							}
							fwrite(pData, 1, retIdx+retLen, fout);
							fwrite(bmpBegin, 1, bmpAndStrLen, fout);
							fwrite(pData+retIdx+retLen, 1, dLen-retIdx-retLen, fout);
							alreadyWrite = true;
							isFillBmp = true;
						}
					}
				} else if(printerSubCMDType == PRINTER_SUBCMD_PCLXL) { //对于PCL6文件处理
					//查找文档 endian
					if(isFoundPclxlEndian==false) {
						ret = Barcode_Inject_Prn_Find_Str_Paramete(interceptCmd, BARCODE_INJECT_PRN_RESERVED_CMD_LEN_D, pData, dLen,
													(const unsigned char *)"\n", sizeof("\n")-1,
													(const unsigned char *)" HP-PCL XL;", sizeof(" HP-PCL XL;")-1, retIdx, retLen, temp);
						if(ret==0) {
							isFoundPclxlEndian = true;
							printf("Found Pclxl Endian %s \n", temp);
							if(temp[0] == ')') {
								pclxlEndian = PRINTER_PCLXL_ENDIAN_LITTLE;
							} else if(temp[0] == '(') {
								pclxlEndian = PRINTER_PCLXL_ENDIAN_BIG;
							}
						}
					}
					//查找文档 DPI（只取X作为全文档DPI）
					if(isFoundDpiPage==false) {
						beginStr[0] = 0xd1;
						((unsigned short *)temp)[0] = 0x89f8;
//						temp[0] = 0xf8;
//						temp[1] = 0x89;
						BarCode_Inject_Prn_Mem_Write(endStr, temp, 2, pclxlEndian);
						ret = Barcode_Inject_Prn_Find_Bin_Paramete(interceptCmd, BARCODE_INJECT_PRN_RESERVED_CMD_LEN_D, pData, dLen, 32,
												(const unsigned char *)beginStr, 1,
												(const unsigned char *)endStr, 2, retIdx, retLen, temp);
						if(ret==0) {
							isFoundDpiPage = true;
							BarCode_Inject_Prn_Mem_Read(temp, (unsigned char *)&dpiPage, 2, pclxlEndian);
							dpiBmp = dpiPage;
							printf("FoundDpiPage %d \n", dpiPage);
						}
					}
					//PCLXL在BeginSession之前一定要设置DPI
					if(isFoundDpiPage && isFillBmp==false) {
						((unsigned short *)temp)[0] = 0x28f8;
//						temp[0] = 0xf8;
//						temp[1] = 0x28;
						BarCode_Inject_Prn_Mem_Write(beginStr, temp, 2, pclxlEndian);
						endStr[0] = 0x43;//begin page
						ret = Barcode_Inject_Prn_Find_Bin_Paramete(interceptCmd, BARCODE_INJECT_PRN_RESERVED_CMD_LEN_D, pData, dLen, 32,
																		(const unsigned char *)beginStr, 2,
																		(const unsigned char *)endStr, 1, retIdx, retLen, temp);
						if(ret==0) {
							printf("found first begin page\n");
							ret = Barcode_Print_Prn_Fill_Buf_XL(barcode, barcodeLen, barcodeType, buf, BARCODE_INJECT_BMP_BUF_LEN, dpiPage, dpiBmp, pclxlEndian, w, h,
									&bmpBegin, bmpLen, &strBegin,  strLen, bmpAndStrLen, fileLen);
							if(ret!=0) {
								printf("%s\n", "Barcode_Print_Fill_Buf err!");
								ret = 1;
								break;
							}
							fwrite(pData, 1, retIdx+retLen, fout);
							fwrite(bmpBegin, 1, bmpAndStrLen, fout);
							fwrite(pData+retIdx+retLen, 1, dLen-retIdx-retLen, fout);
							alreadyWrite = true;
							isFillBmp = true;
						}
					}
				}
			}
			memcpy(interceptCmd+BARCODE_INJECT_PRN_RESERVED_CMD_LEN, pData+dLen-pkgLen, pkgLen);
		}
		if(alreadyWrite==false) {
			fwrite(pData, 1, dLen, fout);
		}
//		fflush(fout);
		//////////////////////////////////////////
    	dLen = fread(pData, 1, BARCODE_INJECT_PRN_DEFAULT_PKG_SIZE, fin);
    	ret = 0;
    }

    fclose(fin);
    fclose(fout);
	return ret;
}

//打印指令开头与结尾字符串
static const unsigned char PJL_BEGIN_STR[] = {0x1B,'%','-','1','2','3','4','5', 'X','@','P','J','L'};
//static const unsigned char PJL_END_STR[] = {0X1B,'%','-','1','2','3','4','5','X'};
static const unsigned char PCL_BEGIN_STR[] = {0x1B, 'E'};
//static const unsigned char PCL_END_STR[] = {0x0A, 0x0D, 0x0C};
bool Barcode_Inject_Prn_Pkg_Is_Head(unsigned char * buf, unsigned int len, int &cmdType) {
	unsigned int idx = 0;
	//检查是不是PJL
	for(idx=0; idx<sizeof(PJL_BEGIN_STR) && idx<len; idx++)
	{
		if(buf[idx]!=PJL_BEGIN_STR[idx]) break;
	}
	if(idx==sizeof(PJL_BEGIN_STR))
	{
		cmdType = PRINTER_CMD_PJL;
		return true;
	}
	//检查是不是PCL
	for(idx=0; idx<sizeof(PCL_BEGIN_STR) && idx<len; idx++)
	{
		if(buf[idx]!=PCL_BEGIN_STR[idx]) break;
	}
	if(idx==sizeof(PCL_BEGIN_STR))
	{
		cmdType = PRINTER_CMD_PCL;
		return true;
	}
//	cmdType = PRINTER_CMD_NULL;
	return false;
}

int Barcode_Inject_Prn_Find_Str(const unsigned char* data,unsigned int dLen,
		const unsigned char *bStr,unsigned  int bStrLen,
		const unsigned char *eStr,unsigned  int eStrLen,
		int &retIdx,unsigned  int &retLen) {
	unsigned int d = 0;	//原串索引
	unsigned int b = 0;	//开始串索引
	unsigned int e = 0;	//结束串索引
	retIdx = -1;
	retLen = -1;

	for(d=0; d<=dLen; d++) {
		if(b<bStrLen) {
			//还未查找完头
			if(data[d]==bStr[b]) {
				if(retIdx==-1) {
					retIdx = d;
				}
				b++;
			} else {
				b = 0;
				e = 0;
				retIdx = -1;
			}
		} else {
			//找到头，接下来找尾，中间允许
			if(e == eStrLen)
				break;
			else if(data[d]==eStr[e]) {
				e++;
			} else if(data[d]<128) {
				if(e!=0){
					b = 0;
					e = 0;
				}
			} else {
				b = 0;
				e = 0;
			}
		}
	}
	if(b==bStrLen && e==eStrLen) {
		retLen = d - retIdx;
	} else {
		retIdx = -1;
		retLen = 0;
	}
	return 0;
}

int Barcode_Inject_Prn_Find_Str_Paramete(const unsigned char* data,unsigned int dLen,
		const unsigned char *bStr,unsigned  int bStrLen,
		const unsigned char *eStr,unsigned  int eStrLen,
		int &retIdx, unsigned int &retLen, unsigned char *retStr) {
	Barcode_Inject_Prn_Find_Str(data, dLen, bStr,bStrLen, eStr, eStrLen, retIdx, retLen);
	if(retIdx<0) return -1;
	int retIdxNew = retIdx + bStrLen;
	unsigned int retLenNew = retLen - bStrLen - eStrLen;
	memcpy(retStr, data+retIdxNew, retLenNew);
	retStr[retLenNew] = 0;
	return 0;
}

//当要查找的字符串头出现在cache中时，retIdx为负数
int Barcode_Inject_Prn_Find_Str_Paramete(const unsigned char* cache,unsigned int cLen,
		const unsigned char* data,unsigned int dLen,
		const unsigned char *bStr,unsigned  int bStrLen,
		const unsigned char *eStr,unsigned  int eStrLen,
		int &retIdx, unsigned  int &retLen, unsigned char *retStr) {
	int ret = Barcode_Inject_Prn_Find_Str_Paramete(cache, cLen, bStr, bStrLen, eStr, eStrLen, retIdx, retLen, retStr);
	if(ret==0) {
		if(retIdx>=(int)cLen/2) {
			retIdx = retIdx-cLen/2;
		} else {
			retLen -= cLen/2-retIdx;
			retIdx = 0;
		}
	} else {
		ret = Barcode_Inject_Prn_Find_Str_Paramete(data, dLen, bStr, bStrLen, eStr, eStrLen, retIdx, retLen, retStr);
	}
	return ret;
}

int Barcode_Inject_Prn_Find_Cmd(const unsigned char* data,unsigned int dLen,
		const unsigned char *bStr,unsigned  int bStrLen,
		const unsigned char *eStr,unsigned  int eStrLen,
		int &retIdx, unsigned int &retLen) {
	unsigned int d = 0;	//原串索引
	unsigned int b = 0;	//开始串索引
	unsigned int e = 0;	//结束串索引
	retIdx = -1;
	retLen = -1;

	for(d=0; d<=dLen; d++) {
		if(b<bStrLen) {
			//还未查找完头
			if(data[d]==bStr[b]) {
				if(retIdx==-1) {
					retIdx = d;
				}
				b++;
			} else {
				b = 0;
				e = 0;
				retIdx = -1;
			}
		} else {
			if(e==eStrLen)
				break;
			//找到头，接下来找尾，中间允许
			else if(data[d]==eStr[e] || data[d]==Barcode_Inject_Prn_To_Lower_Case(eStr[e])) {
				e++;
			} else if(data[d]==43 || data[d]==45 || data[d]==46 || (data[d]>=48 && data[d]<=57) || (data[d]>=96 && data[d]<=126)) {
				if(e!=0) {
					//尾字符串不允许有间格
					b = 0;
					e = 0;
				}
			} else {
				//终止符
				b = 0;
				e = 0;
			}
		}
	}
	if(b==bStrLen && e==eStrLen) {
		retLen = d - retIdx;
	} else {
		retIdx = -1;
		retLen = 0;
	}
	return 0;
}
int Barcode_Inject_Prn_Find_Cmd_Paramete(const unsigned char* data,unsigned int dLen,
		const unsigned char *bStr,unsigned  int bStrLen,
		const unsigned char *eStr,unsigned  int eStrLen,
		int &retIdx, unsigned int &retLen, double &retValue) {
	Barcode_Inject_Prn_Find_Cmd(data, dLen, bStr, bStrLen, eStr, eStrLen, retIdx, retLen);
	if(retIdx<0) {
		return -1;
	}
	unsigned char *pStr = new unsigned char[retLen];
	memcpy(pStr, data+retIdx, retLen);
	pStr[retLen-eStrLen] = 0;
	int i;
	for(i=retLen-eStrLen-1; i>0; i--) {
		if(!((pStr[i]>=43&&pStr[i]<=59))) {
			break;
		}
	}
	if(i!=0) i++;
	retValue = atof((const char *)&pStr[i]);
	delete []pStr;
	return 0;
}

//当要查找的字符串头出现在cache中时，retIdx为负数
int Barcode_Inject_Prn_Find_Cmd_Paramete(const unsigned char* cache,unsigned int cLen,
		const unsigned char* data,unsigned int dLen,
		const unsigned char *bStr,unsigned  int bStrLen,
		const unsigned char *eStr,unsigned  int eStrLen,
		int &retIdx, unsigned  int &retLen, double &retValue) {
	int ret = Barcode_Inject_Prn_Find_Cmd_Paramete(cache, cLen, bStr, bStrLen, eStr, eStrLen, retIdx, retLen, retValue);
	if(ret==0) {
		if(retIdx>=(int)cLen/2) {
			retIdx = retIdx-cLen/2;
		} else {
			retIdx = retIdx-(cLen/2);
		}
	} else {
		ret = Barcode_Inject_Prn_Find_Cmd_Paramete(data, dLen, bStr, bStrLen, eStr, eStrLen, retIdx, retLen, retValue);
	}
	return ret;
}

int Barcode_Inject_Prn_Find_Bin(const unsigned char* data,unsigned int dLen, unsigned int maxMidLen,
		const unsigned char *bStr,unsigned  int bStrLen,
		const unsigned char *eStr,unsigned  int eStrLen,
		int &retIdx,unsigned  int &retLen) {
	unsigned int d = 0;	//原串索引
	unsigned int b = 0;	//开始串索引
	unsigned int e = 0;	//结束串索引
	unsigned int max = 0;
	retIdx = -1;
	retLen = -1;

	for(d=0; d<=dLen; d++) {
		if(b<bStrLen) {
			max = 0;
			//还未查找完头
			if(data[d]==bStr[b]) {
				if(retIdx==-1) {
					retIdx = d;
				}
				b++;
			} else {
				b = 0;
				e = 0;
				retIdx = -1;
			}
		} else {
			//找到头，接下来找尾，中间允许
			if(e == eStrLen)
				break;
			else if(data[d]==eStr[e]) {
				e++;
			} else {
				max += e+1;
				if(e!=0){
					e = 0;
				}
				if(max>maxMidLen) {
					b = 0;
					e = 0;
				}
			}
		}
	}
	if(b==bStrLen && e==eStrLen) {
		retLen = d - retIdx;
	} else {
		retIdx = -1;
		retLen = 0;
	}
	return 0;
}

int Barcode_Inject_Prn_Find_Bin_Paramete(const unsigned char* data,unsigned int dLen, unsigned int maxMidLen,
		const unsigned char *bStr,unsigned  int bStrLen,
		const unsigned char *eStr,unsigned  int eStrLen,
		int &retIdx, unsigned int &retLen, unsigned char *retStr) {
	Barcode_Inject_Prn_Find_Bin(data, dLen, maxMidLen, bStr,bStrLen, eStr, eStrLen, retIdx, retLen);
	if(retIdx<0) return -1;
	int retIdxNew = retIdx + bStrLen;
	unsigned int retLenNew = retLen - bStrLen - eStrLen;
	memcpy(retStr, data+retIdxNew, retLenNew);
	return 0;
}

//当要查找的字符串头出现在cache中时，retIdx为负数
int Barcode_Inject_Prn_Find_Bin_Paramete(const unsigned char* cache,unsigned int cLen,
		const unsigned char* data,unsigned int dLen, unsigned maxMidLen,
		const unsigned char *bStr,unsigned  int bStrLen,
		const unsigned char *eStr,unsigned  int eStrLen,
		int &retIdx, unsigned  int &retLen, unsigned char *retStr) {
	int ret = Barcode_Inject_Prn_Find_Bin_Paramete(cache, cLen, maxMidLen, bStr, bStrLen, eStr, eStrLen, retIdx, retLen, retStr);
	if(ret==0) {
		if(retIdx>=(int)cLen/2) {
			retIdx = retIdx-cLen/2;
		} else {
			retLen -= cLen/2-retIdx;
			retIdx = 0;
		}
	} else {
		ret = Barcode_Inject_Prn_Find_Bin_Paramete(data, dLen, maxMidLen, bStr, bStrLen, eStr, eStrLen, retIdx, retLen, retStr);
	}
	return ret;
}

///////////////////////////////////////////////////////////////////////////////////
unsigned char Barcode_Inject_Prn_To_Upper_Case(unsigned char c){
	if(c>='a'&&c<='z'){
		c = (unsigned char) (c&0x5F);
	}
	return c;
}

unsigned char Barcode_Inject_Prn_To_Lower_Case(unsigned char c){
	if(c>='A'&&c<='Z'){
		c = (unsigned char) (c|0x20);
	}
	return c;
}


#if __BYTE_ORDER == __LITTLE_ENDIAN
void BarCode_Inject_Prn_Mem_Write(unsigned char *pRaw, unsigned char *pData, int len, int endian) {
	if(endian==PRINTER_PCLXL_ENDIAN_LITTLE) {
		for(int i=0; i<len; i++) {
			pRaw[i] = pData[i];
		}
	} else {
		for(int i=0; i<len; i++) {
			pRaw[i] = pData[len-i-1];
		}
	}
}
void BarCode_Inject_Prn_Mem_Read(unsigned char *pRaw, unsigned char *pData, int len, int endian) {
	if(endian==PRINTER_PCLXL_ENDIAN_LITTLE) {
		for(int i=0; i<len; i++) {
			pData[i] = pRaw[i];
		}
	} else {
		for(int i=0; i<len; i++) {
			pData[i] = pRaw[len-i-1];
		}
	}
}
#elif __BYTE_ORDER == __BIG_ENDIAN

void BarCode_Inject_Prn_Mem_Write(unsigned char *pRaw, unsigned char *pData, int len, int endian) {
	if(endian==PRINTER_PCLXL_ENDIAN_BIG) {
		for(int i=0; i<len; i++) {
			pRaw[i] = pData[i];
		}
	} else {
		for(int i=0; i<len; i++) {
			pRaw[i] = pData[len-i-1];
		}
	}
}
void BarCode_Inject_Prn_Mem_Read(unsigned char *pRaw, unsigned char *pData, int len, int endian) {
	if(endian==PRINTER_PCLXL_ENDIAN_BIG) {
		for(int i=0; i<len; i++) {
			pData[i] = pRaw[i];
		}
	} else {
		for(int i=0; i<len; i++) {
			pData[i] = pRaw[len-i-1];
		}
	}
}

#endif


