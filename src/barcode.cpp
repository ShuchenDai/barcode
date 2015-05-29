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

#include <unistd.h>
#include <sys/time.h>

#include <sys/socket.h>
#include <sys/ioctl.h>
#include <netinet/in.h>
#include <net/if.h>
#include <string.h>
#include <stdio.h>


#define LEN 2024*1024+100
unsigned char buf[LEN];
//unsigned char buf2[LEN*10];
//unsigned char buf3[1024];

int get_mac(char* mac)
{
    int sockfd;
    struct ifreq tmp;
    char mac_addr[30];
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if( sockfd < 0)
    {
        perror("create socket fail\n");
        return 1;
    }
    memset(&tmp,0,sizeof(struct ifreq));
    strncpy(tmp.ifr_name,"eth0",sizeof(tmp.ifr_name)-1);
    if( (ioctl(sockfd,SIOCGIFHWADDR,&tmp)) < 0 )
    {
        printf("mac ioctl error\n");
        return 1;
    }
    sprintf(mac_addr, "%02x%02x%02x%02x%02x%02x",
            (unsigned char)tmp.ifr_hwaddr.sa_data[0],
            (unsigned char)tmp.ifr_hwaddr.sa_data[1],
            (unsigned char)tmp.ifr_hwaddr.sa_data[2],
            (unsigned char)tmp.ifr_hwaddr.sa_data[3],
            (unsigned char)tmp.ifr_hwaddr.sa_data[4],
            (unsigned char)tmp.ifr_hwaddr.sa_data[5]
            );
    close(sockfd);
    int macLen = strlen(mac_addr);
    memcpy(mac,mac_addr, macLen);
    mac[macLen] = 0;
    return 0;
}

int getBarcode(char *barcode) {
	if(get_mac(barcode)!=0) return 1;
	timeval t;
	char nowStr[42];
	gettimeofday(&t, NULL);
	sprintf(nowStr, "%lu%lu", t.tv_sec, t.tv_usec/100000);
	strcat(barcode, nowStr);
	return 0;
}



///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void usage(char *arg) {
	printf("usb-prn-barcode - command line tool for generate a bitmap file with a barcode\n");
	printf("Usage: %s -c barcode -o outputFile [OPTIONS]\n", arg);
	printf("\t-c <barcode> A barcode string\n");
	printf("\t-o <file> A output file name\n");
	printf("Options:\n");
	printf("\t-t <type> The type of barcode, default is Code128\n");
	printf("\t-i <dpi> The dpi of bitmap, default is 600\n");
	printf("Type:\n");
	printf("\tCode128 EAN13");
	printf("\n");
}
int main(int argc, char **argv) {
	int opt;
	char barcode[128] = {0};
	unsigned int barcodeLen = 0;
	char foutStr[128] = {0};
	char barcodeTypeStr[32] = "Code128";
	int dpiBmp = 600;

	int ret;
	Barcode_Type barcodeType = BARCODE_TYPE_CODE128;
	FILE *fout;
	unsigned int w, h, fileLen, fileLenRet;

	while((opt = getopt (argc, argv, "c:o:t:i:h")) != EOF) {
		switch (opt) {
			case 'c':
				strcpy(barcode, optarg);
				barcodeLen = strlen(barcode);
				break;
			case 'o':
				strcpy(foutStr, optarg);
				break;
			case 't':
				strcpy(barcodeTypeStr, optarg);
				break;
			case 'i':
				dpiBmp = atoi(optarg);
				break;
			case 'h':
				usage(argv[0]);
				return 0;
				break;
			default:
				usage(argv[0]);
				return 1;
		}
	}
	if(barcodeLen==0 || foutStr[0]==0) {
		usage(argv[0]);
		return 1;
	}
	if(strcmp(barcodeTypeStr, "Code128")==0) {
		barcodeType = BARCODE_TYPE_CODE128;
	} else if(strcmp(barcodeTypeStr, "EAN13")==0) {
		barcodeType = BARCODE_TYPE_EAN13;
	} else {
		fprintf(stderr, "Unsupport barcode type: %s\n", barcodeTypeStr);
		usage(argv[0]);
		return 1;
	}

	switch(barcodeType) {
	case BARCODE_TYPE_CODE128:
		ret = Code128B_Auto_Fill_Buf(barcode, barcodeLen, buf, LEN, dpiBmp, w, h, fileLen, false);
		break;
	case BARCODE_TYPE_EAN13:
		ret = EAN13_Fill_Buf(barcode, buf, LEN, dpiBmp, w, h, fileLen, false);
		break;
	}
	if(ret!=0) {
		fprintf(stderr, "Generate bitmap err![%d]\n", ret);
		return 1;
	}

	fout = fopen(foutStr, "w");
	if(!fout) {
		fprintf(stderr, "Fail on open %s [%d:%s]\n", foutStr, errno, strerror(errno));
		return 1;
	}
	fileLenRet = fwrite(buf, 1, fileLen, fout);
	if(fileLenRet!=fileLen) {
		fprintf(stderr, "Fail on open %s [%d:%s]\n", foutStr, errno, strerror(errno));
		return 1;
	}
	fclose(fout);
}

//int main(int argv, char **args) {
//	int ret;
//	FILE *fin, *fout;
//
//	char code128Str[128] = "69139876487923";
//	unsigned int code128StrLen = sizeof("69139876487923")-1;
//	unsigned int dpiPage = 600, dpiBmp = 600;
//	unsigned int w = 370;
//	unsigned int h = 80;
//	unsigned char *bmpBegin, *strBegin;
//	unsigned int bmpLen, strLen, bmpAndStrLen, fileLen;
//
////	memset(code128Str, 0, 128);
//	getBarcode((char *)code128Str);
//	code128StrLen = strlen(code128Str);
//	ret = Barcode_Print_Prn_Fill_Buf_XL(code128Str, code128StrLen, BARCODE_TYPE_CODE128,
//			buf, LEN, dpiPage, dpiBmp, PRINTER_PCLXL_ENDIAN_LITTLE, w, h,
//			&bmpBegin, bmpLen, &strBegin,  strLen, bmpAndStrLen, fileLen);
//	if(ret!=0) {
//		printf("%s\n", "Barcode_Print_Prn_Fill_Buf_XL err!");
//		return 1;
//	}
//
//	//输出单独的位图文件
//	fout = fopen("/home/welkinm/pclxl.prn", "w");
//	fwrite(buf,1, fileLen, fout);
//	fclose(fout);
//
//	getBarcode((char *)code128Str);
//	code128StrLen = strlen(code128Str);
//	printf("barcode : %s: %d\n", code128Str, code128StrLen);
//
//	ret = Barcode_Inject_Prn("/home/welkinm/pcl6.prn", "/home/welkinm/pcl6_1.prn",
//			BARCODE_TYPE_CODE128, code128Str, code128StrLen);
//	if(ret!=0) {
//		printf("%s\n", "Barcode_Print_Fill_Buf err!");
//		return 1;
//	}
//
//#if __BYTE_ORDER == __LITTLE_ENDIAN
//	cout<< "Little" <<endl;
//#elif __BYTE_ORDER == __BIG_ENDIAN
//	count<< "Big" <<endl;
//#endif
//}
