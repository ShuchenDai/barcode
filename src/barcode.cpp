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
#include "Print/barcode_print_prn.h"
#include "Print/barcode_inject_prn.h"
#include "barcode.h"
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


char printerCMDType = PRINTER_CMD_NULL;


int main(int argv, char **args) {
	int ret;
	FILE *fin, *fout;

	const char *code128Str = "abcdefghjklm123456789";
	unsigned int code128StrLen = sizeof("abcdefghjklm123456789")-1;
	unsigned int dpiPage = 600, dpiBmp = 300;
	unsigned int w = 370;
	unsigned int h = 80;

//	ret = Barcode_Print_Fill_Buf(code128Str, code128StrLen, buf, LEN, dpiPage, dpiBmp, w, h, &bmpBegin, bmpLen, &strBegin,  strLen, fileLen);
//	if(ret!=0) {
//		printf("%s\n", "Barcode_Print_Fill_Buf err!");
//		return 1;
//	}

//	//输出单独的位图文件
//	fout = fopen("/home/welkinm/bmp.prn", "w");
//	fwrite(buf,1, fileLen, fout);
//	fclose(fout);

	ret = Barcode_Inject_Prn("/home/welkinm/basereport_2010_001.pcl", "/home/welkinm/barcode.prn",
			BARCODE_TYPE_CODE128, code128Str, code128StrLen);
	if(ret!=0) {
		printf("%s\n", "Barcode_Print_Fill_Buf err!");
		return 1;
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
//    fclose(fin);
//    fclose(fout);

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






