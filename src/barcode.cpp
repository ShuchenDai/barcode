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


bool printer_pkg_is_head(unsigned char * buf, unsigned int len);
void printer_get_cmd_type(unsigned char* buf, unsigned int len);
int findStr(const unsigned char* data,unsigned int dLen,
		const unsigned char *bStr,unsigned  int bStrLen,
		const unsigned char *eStr,unsigned  int eStrLen,
		int &retIdx,unsigned  int &retLen);
int findStrParamete(const unsigned char* data,unsigned int dLen,
		const unsigned char *bStr,unsigned  int bStrLen,
		const unsigned char *eStr,unsigned  int eStrLen,
		int &retIdx, unsigned int &retLen, unsigned char *retStr);
int findStrParamete(const unsigned char* cache,unsigned int cLen,
		const unsigned char* data,unsigned int dLen,
		const unsigned char *bStr,unsigned  int bStrLen,
		const unsigned char *eStr,unsigned  int eStrLen,
		int &retIdx, unsigned  int &retLen, unsigned char *retStr);
int findCmd(const unsigned char* data,unsigned int dLen,
		const unsigned char *bStr,unsigned  int bStrLen,
		const unsigned char *eStr,unsigned  int eStrLen,
		int &retIdx,unsigned  int &retLen);
int findCmdParamete(const unsigned char* data,unsigned int dLen,
		const unsigned char *bStr,unsigned  int bStrLen,
		const unsigned char *eStr,unsigned  int eStrLen,
		int &retIdx, unsigned int &retLen, double &retValue);
int findCmdParamete(const unsigned char* cache,unsigned int cLen,
		const unsigned char* data,unsigned int dLen,
		const unsigned char *bStr,unsigned  int bStrLen,
		const unsigned char *eStr,unsigned  int eStrLen,
		int &retIdx, unsigned  int &retLen, double &retValue);


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

char printerCMDType = PRINTER_CMD_NULL;


int main(int argv, char **args) {
	int ret;
	FILE *fin, *fout;

	const char *code128Str = "abcdefghjklm123456789";
	unsigned int code128StrLen = sizeof("abcdefghjklm123456789")-1;
	unsigned int dpiPage = 600, dpiBmp = 300;
	unsigned int w = 370;
	unsigned int h = 80;
	unsigned char *bmpBegin;
	unsigned int bmpLen;
	unsigned char *strBegin;
	unsigned int strLen;
	unsigned int bmpAndStrLen;
	unsigned int fileLen;

//	ret = Barcode_Print_Fill_Buf(code128Str, code128StrLen, buf, LEN, dpiPage, dpiBmp, w, h, &bmpBegin, bmpLen, &strBegin,  strLen, fileLen);
//	if(ret!=0) {
//		printf("%s\n", "Barcode_Print_Fill_Buf err!");
//		return 1;
//	}

//	//输出单独的位图文件
//	fout = fopen("/home/welkinm/bmp.prn", "w");
//	fwrite(buf,1, fileLen, fout);
//	fclose(fout);

	if(argv<2) return 0;

    fin = fopen(args[1], "r");
    if(fin<=0) {
    	printf("%d(%s)\n", errno, strerror(errno));
    	return 1;
    }
    fout = fopen("/home/welkinm/demoout.prn", "w");

    unsigned char pData[512];
//    bool isFirstPkg = false;		//是否是文件头
    bool isFoundDpiPage = false;	//是否己找到设置打印页面分辨率
    bool isFoundDpiBmp = false;	//是否己找到设置位图分辨率
    bool isFillBmp = false;		//是否己在打印文件中填充位图

	char printerSubCMDType = PRINTER_SUBCMD_NULL;
	bool isFoundSubCMDType = false;

#define PACKET_LOG_RESERVED_CMD_LEN 32
#define PACKET_LOG_RESERVED_CMD_LEN_D PACKET_LOG_RESERVED_CMD_LEN*2
	unsigned char interceptCmd[PACKET_LOG_RESERVED_CMD_LEN_D];
#define PRINT_DEFAULT_RESOLUTION 300

//	int retIdx;
//	unsigned int retLen;
//	ret = fread(buf, 1, LEN, fin);
//	printf("read file len: %d\n", ret);
//	if(ret<0) {
//		printf("%s\n","fread err!");
//		return 1;
//	}
//	fileLen = ret;
//	ret = findStrParament(buf, fileLen, (const unsigned char *)"@PJL ENTER LANGUAGE=", sizeof("@PJL ENTER LANGUAGE=")-1,
//			(const unsigned char *)"\n", 1, retIdx, retLen);
//	if(retIdx>=0) {
//		printf("found @PJL ENTER LANGUAGE=, idx:%d, len:%d\n", retIdx, retLen);
//	}
//	int cmdValue;
//	ret = findCmdParamete(buf, fileLen, (const unsigned char *)"\x1b*t", sizeof("\x1b*t")-1,
//			(const unsigned char *)"R", 1, cmdValue);
//	if(ret>=0) {
//		printf("found Raster Graphics Resolution, val:%d\n", cmdValue);
//	}
//	ret = findCmdParamete(buf, fileLen, (const unsigned char *)"\x1b&u", sizeof("\x1b&u")-1,
//			(const unsigned char *)"D", 1, cmdValue);
//	if(ret>=0) {
//		printf("found Unit-of-Measure, val:%d\n", cmdValue);
//	}
//	ret = findCmdParamete(buf, fileLen, (const unsigned char *)"\x1b&l", sizeof("\x1b&l")-1,
//			(const unsigned char *)"A", 1, cmdValue);
//	if(ret>=0) {
//		printf("found Unit-of-Measure, val:%d\n", cmdValue);
//	}


    unsigned int dLen;
    dLen = fread(pData, 1, 376, fin);
    unsigned char temp[128];
    int retIdx = 0;
    unsigned int retLen;
    int pkgLen = 0;
	double value;
	bool isWrite;
    while(dLen>0) {
    	isWrite = true;
    	//处理头
		if(printer_pkg_is_head(pData, dLen)) {
			printer_get_cmd_type(pData, dLen);
			if(printerCMDType==PRINTER_CMD_PCL) {
				printerSubCMDType = PRINTER_SUBCMD_PCL5;
				isFoundSubCMDType = true;
			} else {
				printerSubCMDType = PRINTER_SUBCMD_NULL;
				isFoundSubCMDType = false;
			}
//			isFirstPkg = true;
			isFoundDpiPage = false;
			isFoundDpiBmp = false;
			isFillBmp = false;
			dpiPage = PRINT_DEFAULT_RESOLUTION;
			dpiBmp = PRINT_DEFAULT_RESOLUTION;
			memset(interceptCmd+PACKET_LOG_RESERVED_CMD_LEN, 0, PACKET_LOG_RESERVED_CMD_LEN);
		}
		if(isFillBmp==false) {
			pkgLen = PACKET_LOG_RESERVED_CMD_LEN>dLen? dLen: PACKET_LOG_RESERVED_CMD_LEN;
			memcpy(interceptCmd, interceptCmd+PACKET_LOG_RESERVED_CMD_LEN, PACKET_LOG_RESERVED_CMD_LEN);
			memcpy(interceptCmd+PACKET_LOG_RESERVED_CMD_LEN, pData, pkgLen);
			if(isFoundSubCMDType==false) {
				ret = findStrParamete(interceptCmd, PACKET_LOG_RESERVED_CMD_LEN_D, pData, dLen, (const unsigned char *)"@PJL ENTER LANGUAGE=", sizeof("@PJL ENTER LANGUAGE=")-1,
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
			}
			if(isFoundSubCMDType) {
				if(printerSubCMDType == PRINTER_SUBCMD_PCL5) {
					//Page DPI
					if(isFoundDpiPage==false) {
						ret = findCmdParamete(interceptCmd, PACKET_LOG_RESERVED_CMD_LEN_D, pData, dLen, (const unsigned char *)"\x1b&u", sizeof("\x1b&u")-1,
												(const unsigned char *)"D", 1, retIdx, retLen, value);
						if(ret==0) {
							isFoundDpiPage = true;
							printf("FoundDpiPage %f \n", value);
							dpiPage = value;
						}
					}
					//Bitmap DPI
					if(isFoundDpiBmp==false) {
						ret = findCmdParamete(interceptCmd, PACKET_LOG_RESERVED_CMD_LEN_D, pData, dLen, (const unsigned char *)"\x1b*t", sizeof("\x1b*t")-1,
												(const unsigned char *)"R", 1, retIdx, retLen, value);
						if(ret==0) {
							isFoundDpiBmp = true;
							printf("FoundDpiBmp %f \n", value);
							dpiBmp = value;
						}
					}


					if(isFillBmp==false) {
						//First move point Vertical Unit
						ret = findCmdParamete(interceptCmd, PACKET_LOG_RESERVED_CMD_LEN_D, pData, dLen,
								(const unsigned char *)"\x1b*p", sizeof("\x1b*p")-1, (const unsigned char *)"Y", 1, retIdx, retLen, value);
						if(ret==0) {
							printf("get Cursor Position Vertical(Unit): idx:%d, len:%d value:%f\n", retIdx, retLen, value);
							isWrite = false;
							ret = Barcode_Print_Fill_Buf(code128Str, code128StrLen, buf, LEN, dpiPage, dpiBmp, w, h,
									&bmpBegin, bmpLen, &strBegin,  strLen, bmpAndStrLen, fileLen);
							if(ret!=0) {
								printf("%s\n", "Barcode_Print_Fill_Buf err!");
								return 1;
							}
							fwrite(pData, 1, retIdx+retLen, fout);
							fwrite(bmpBegin, 1, bmpAndStrLen, fout);
							fwrite(pData+retIdx+retLen, 1, dLen-retIdx-retLen, fout);
							isFillBmp = true;
						}
					}
					if(isFillBmp==false) {
						//First move point Vertical Decipoints
						ret = findCmdParamete(interceptCmd, PACKET_LOG_RESERVED_CMD_LEN_D, pData, dLen,
								(const unsigned char *)"\x1b&a", sizeof("\x1b&a")-1, (const unsigned char *)"V", 1, retIdx, retLen, value);
						if(ret==0) {
							printf("get Cursor Position Vertical(Decipoints): idx:%d, len:%d value:%f\n", retIdx, retLen, value);
							isWrite = false;
							ret = Barcode_Print_Fill_Buf(code128Str, code128StrLen, buf, LEN, dpiPage, dpiBmp, w, h,
									&bmpBegin, bmpLen, &strBegin,  strLen, bmpAndStrLen, fileLen);
							if(ret!=0) {
								printf("%s\n", "Barcode_Print_Fill_Buf err!");
								return 1;
							}
							fwrite(pData, 1, retIdx+retLen, fout);
							fwrite(bmpBegin, 1, bmpAndStrLen, fout);
							fwrite(pData+retIdx+retLen, 1, dLen-retIdx-retLen, fout);
							isFillBmp = true;
						}
					}
					if(isFillBmp==false) {
						//First move point Vertical Rows
						ret = findCmdParamete(interceptCmd, PACKET_LOG_RESERVED_CMD_LEN_D, pData, dLen,
								(const unsigned char *)"\x1b&a", sizeof("\x1b&a")-1, (const unsigned char *)"R", 1, retIdx, retLen, value);
						if(ret==0) {
							printf("get Cursor Position Vertical(Rows): idx:%d, len:%d value:%f\n", retIdx, retLen, value);
							isWrite = false;
							ret = Barcode_Print_Fill_Buf(code128Str, code128StrLen, buf, LEN, dpiPage, dpiBmp, w, h,
									&bmpBegin, bmpLen, &strBegin,  strLen, bmpAndStrLen, fileLen);
							if(ret!=0) {
								printf("%s\n", "Barcode_Print_Fill_Buf err!");
								return 1;
							}
							fwrite(pData, 1, retIdx+retLen, fout);
							fwrite(bmpBegin, 1, bmpAndStrLen, fout);
							fwrite(pData+retIdx+retLen, 1, dLen-retIdx-retLen, fout);
							isFillBmp = true;
						}
					}


					if(isFillBmp==false) {
						//First move point Horizontal Unit
						ret = findCmdParamete(interceptCmd, PACKET_LOG_RESERVED_CMD_LEN_D, pData, dLen,
								(const unsigned char *)"\x1b*p", sizeof("\x1b*p")-1, (const unsigned char *)"X", 1, retIdx, retLen, value);
						if(ret==0) {
							printf("get Cursor Position Horizontal(Unit): idx:%d, len:%d value:%f\n", retIdx, retLen, value);
							isWrite = false;
							ret = Barcode_Print_Fill_Buf(code128Str, code128StrLen, buf, LEN, dpiPage, dpiBmp, w, h,
									&bmpBegin, bmpLen, &strBegin,  strLen, bmpAndStrLen, fileLen);
							if(ret!=0) {
								printf("%s\n", "Barcode_Print_Fill_Buf err!");
								return 1;
							}
							fwrite(pData, 1, retIdx+retLen, fout);
							fwrite(bmpBegin, 1, bmpAndStrLen, fout);
							fwrite(pData+retIdx+retLen, 1, dLen-retIdx-retLen, fout);
							isFillBmp = true;
						}
					}
					if(isFillBmp==false) {
						//First move point Horizontal Decipoints
						ret = findCmdParamete(interceptCmd, PACKET_LOG_RESERVED_CMD_LEN_D, pData, dLen,
								(const unsigned char *)"\x1b&a", sizeof("\x1b&a")-1, (const unsigned char *)"H", 1, retIdx, retLen, value);
						if(ret==0) {
							printf("get Cursor Position Horizontal(Decipoints): idx:%d, len:%d value:%f\n", retIdx, retLen, value);
							isWrite = false;
							ret = Barcode_Print_Fill_Buf(code128Str, code128StrLen, buf, LEN, dpiPage, dpiBmp, w, h,
									&bmpBegin, bmpLen, &strBegin,  strLen, bmpAndStrLen, fileLen);
							if(ret!=0) {
								printf("%s\n", "Barcode_Print_Fill_Buf err!");
								return 1;
							}
							fwrite(pData, 1, retIdx+retLen, fout);
							fwrite(bmpBegin, 1, bmpAndStrLen, fout);
							fwrite(pData+retIdx+retLen, 1, dLen-retIdx-retLen, fout);
							isFillBmp = true;
						}
					}
					if(isFillBmp==false) {
						//First move point Horizontal Columns
						ret = findCmdParamete(interceptCmd, PACKET_LOG_RESERVED_CMD_LEN_D, pData, dLen,
								(const unsigned char *)"\x1b&a", sizeof("\x1b&a")-1, (const unsigned char *)"C", 1, retIdx, retLen, value);
						if(ret==0) {
							printf("get Cursor Position Horizontal(Columns): idx:%d, len:%d value:%f\n", retIdx, retLen, value);
							isWrite = false;
							ret = Barcode_Print_Fill_Buf(code128Str, code128StrLen, buf, LEN, dpiPage, dpiBmp, w, h,
									&bmpBegin, bmpLen, &strBegin,  strLen, bmpAndStrLen, fileLen);
							if(ret!=0) {
								printf("%s\n", "Barcode_Print_Fill_Buf err!");
								return 1;
							}
							fwrite(pData, 1, retIdx+retLen, fout);
							fwrite(bmpBegin, 1, bmpAndStrLen, fout);
							fwrite(pData+retIdx+retLen, 1, dLen-retIdx-retLen, fout);
							isFillBmp = true;
						}
					}
				}
			}
			memcpy(interceptCmd+PACKET_LOG_RESERVED_CMD_LEN, pData+dLen-pkgLen, pkgLen);
		}
		if(isWrite) {
			fwrite(pData, 1, dLen, fout);
		}
		fflush(fout);
		//////////////////////////////////////////
    	dLen = fread(pData, 1, 512, fin);
    }
//    if(len!=readLen) {
//    	printf("Read err: %d(%s)\n", errno, strerror(errno));
//    	return 1;
//    }
//    fwrite(buf2, 1, readLen, fout);//写源PRN开始部份
//    fwrite(p, 1, imgLen, fout);//写入条形码
//    fwrite(buf3, 1, strLen, fout);//写入字
//    readLen = fread(buf2, 1, 1024*1024, fin);//写源PRN结尾部份
//    while(readLen>0) {
//    	fwrite(buf2, 1, readLen, fout);
//    	readLen = fread(buf2, 1, 1024*1024, fin);
//    }
    fclose(fin);
    fclose(fout);

#if __BYTE_ORDER == __LITTLE_ENDIAN
	cout<< "Little" <<endl;
#elif __BYTE_ORDER == __BIG_ENDIAN
	count<< "Big" <<endl;
#endif
}


//打印指令开头与结尾字符串
static const unsigned char PJL_BEGIN_STR[] = {0x1B,'%','-','1','2','3','4','5', 'X','@','P','J','L'};
//static const unsigned char PJL_END_STR[] = {0X1B,'%','-','1','2','3','4','5','X'};
static const unsigned char PCL_BEGIN_STR[] = {0x1B, 'E'};
//static const unsigned char PCL_END_STR[] = {0x0A, 0x0D, 0x0C};
bool printer_pkg_is_head(unsigned char * buf, unsigned int len) {
	unsigned int idx = 0;
	//检查是不是PJL
	for(idx=0; idx<sizeof(PJL_BEGIN_STR) && idx<len; idx++)
	{
		if(buf[idx]!=PJL_BEGIN_STR[idx]) break;
	}
	if(idx==sizeof(PJL_BEGIN_STR))
	{
		return true;
	}
	//检查是不是PCL
	for(idx=0; idx<sizeof(PCL_BEGIN_STR) && idx<len; idx++)
	{
		if(buf[idx]!=PCL_BEGIN_STR[idx]) break;
	}
	if(idx==sizeof(PCL_BEGIN_STR))
	{
		return true;
	}
	return false;
}
void printer_get_cmd_type(unsigned char* buf, unsigned int len)
{
	unsigned int idx = 0;
	printerCMDType = PRINTER_CMD_NULL;
	for(idx=0; idx<sizeof(PCL_BEGIN_STR) && idx<len; idx++)
	{
		if(buf[idx]!=PCL_BEGIN_STR[idx]) break;
	}
	if(idx==sizeof(PCL_BEGIN_STR))
	{
		printerCMDType = PRINTER_CMD_PCL;
		return;
	}

	for(idx=0; idx<sizeof(PJL_BEGIN_STR) && idx<len; idx++)
	{
		if(buf[idx]!=PJL_BEGIN_STR[idx]) break;
	}
	if(idx==sizeof(PJL_BEGIN_STR))
	{
		printerCMDType = PRINTER_CMD_PJL;
		return;
	}
}

unsigned char toUpperCase(unsigned char c){
	if(c>='a'&&c<='z'){
		c = (unsigned char) (c&0x5F);
	}
	return c;
}

unsigned char toLowerCase(unsigned char c){
	if(c>='A'&&c<='Z'){
		c = (unsigned char) (c|0x20);
	}
	return c;
}

int findStr(const unsigned char* data,unsigned int dLen,
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

int findStrParamete(const unsigned char* data,unsigned int dLen,
		const unsigned char *bStr,unsigned  int bStrLen,
		const unsigned char *eStr,unsigned  int eStrLen,
		int &retIdx, unsigned int &retLen, unsigned char *retStr) {
	findStr(data, dLen, bStr,bStrLen, eStr, eStrLen, retIdx, retLen);
	if(retIdx<0) return -1;
	retIdx += bStrLen;
	retLen -= bStrLen+eStrLen;
	memcpy(retStr, data+retIdx, retLen);
	retStr[retLen] = 0;
	return 0;
}

//当要查找的字符串头出现在cache中时，retIdx为负数
int findStrParamete(const unsigned char* cache,unsigned int cLen,
		const unsigned char* data,unsigned int dLen,
		const unsigned char *bStr,unsigned  int bStrLen,
		const unsigned char *eStr,unsigned  int eStrLen,
		int &retIdx, unsigned  int &retLen, unsigned char *retStr) {
	int ret = findStrParamete(cache, cLen, bStr, bStrLen, eStr, eStrLen, retIdx, retLen, retStr);
	if(ret==0) {
//		memcpy(retStr, cache+retIdx, retLen);
//		retStr[retLen] = 0;
		if(retIdx>=(int)cLen/2) {
			retIdx = retIdx-cLen/2;
		} else {
			retIdx = retIdx-(cLen/2);
		}
	} else {
		ret = findStrParamete(data, dLen, bStr, bStrLen, eStr, eStrLen, retIdx, retLen, retStr);
//		if(ret==0) {
//			memcpy(retStr, data+retIdx, retLen);
//			retStr[retLen] = 0;
//		}
	}
	return ret;
}

int findCmd(const unsigned char* data,unsigned int dLen,
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
			else if(data[d]==eStr[e] || data[d]==toLowerCase(eStr[e])) {
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
int findCmdParamete(const unsigned char* data,unsigned int dLen,
		const unsigned char *bStr,unsigned  int bStrLen,
		const unsigned char *eStr,unsigned  int eStrLen,
		int &retIdx, unsigned int &retLen, double &retValue) {
//	int retIdx;
//	unsigned int retLen;
	findCmd(data, dLen, bStr, bStrLen, eStr, eStrLen, retIdx, retLen);
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
int findCmdParamete(const unsigned char* cache,unsigned int cLen,
		const unsigned char* data,unsigned int dLen,
		const unsigned char *bStr,unsigned  int bStrLen,
		const unsigned char *eStr,unsigned  int eStrLen,
		int &retIdx, unsigned  int &retLen, double &retValue) {
	int ret = findCmdParamete(cache, cLen, bStr, bStrLen, eStr, eStrLen, retIdx, retLen, retValue);
	if(ret==0) {
		if(retIdx>=(int)cLen/2) {
			retIdx = retIdx-cLen/2;
		} else {
			retIdx = retIdx-(cLen/2);
		}
	} else {
		ret = findCmdParamete(data, dLen, bStr, bStrLen, eStr, eStrLen, retIdx, retLen, retValue);
	}
	return ret;
}






