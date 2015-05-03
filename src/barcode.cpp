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
unsigned char buf2[LEN*10];
unsigned char buf3[1024];

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
//    printf("local mac:%s\n", mac_addr);
    close(sockfd);
    memcpy(mac,mac_addr,strlen(mac_addr));
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

int main(int argv, char **args) {
	int ret;
	FILE *fin, *fout;

	char code128Str[128] = "0";
	unsigned int code128StrLen = sizeof("0")-1;
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

	getBarcode((char *)code128Str);
	code128StrLen = strlen(code128Str);
	printf("%s: %d\n", code128Str, code128StrLen);

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
