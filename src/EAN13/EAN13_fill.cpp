/*
 * EAN13_writer.cpp
 *
 *  Created on: 2015年4月22日
 *      Author: welkinm
 */


#include "../barcode_bmp.h"
#include <stdio.h>
#include "EAN13_fill.h"

char EAN13_Get_Check_Sum(const char *barcode) {
	unsigned int ret = 0;
	//将最右边一个数位作为“奇数”位，从右向左为每个字符指定奇数/偶数位。
	//对所有奇数位上的数值求和，将结构乘以3。
	ret = (barcode[11]+barcode[9]+barcode[7]+barcode[5]+barcode[3]+barcode[1]-0x30*6)*3;
	//加上对所有偶数位上的数值求和。
	ret += barcode[10]+barcode[8]+barcode[6]+barcode[4]+barcode[2]+barcode[0]-0x30*6;
	//校验位的数字加上用第4步计算的总和数应该能够被10整除
	ret = ret%10;
	if(ret)
		return (unsigned char) (10-ret);
	else
		return ret;
}

int EAN13_Fill_Buf(char *barcode, unsigned char *buf, unsigned int len,
		unsigned int& w, unsigned int& h, unsigned int& bmpLen, bool isColorExchange) {
	//设置BMP文件头信息
	BarCode_BMPHead_Type head;
	int ret = BarCode_BMP_Build_Head(head, w, h, 1, BARCODE_BMP_COLOR_TABLE_1);
	if(ret!=0) return 1;
	//计算X轴基本间隙长度，整个条码一共有 3 + 7*6 + 5 + 7*6 + 3 = 95, 算上空白区 14 * 95 * 12 = 121
	if(w<121) return 2;
	int thickness = w/121;		//基本条的宽度
	//写入BMP头信息
	ret = BarCode_BMP_Mem_Write_Head(head, buf, len);
	if(ret!=0) return 1;
	bmpLen = head.fh.bfSize;
	//背景色与线条色
	BarCode_BMPColor_Type bgRGB;
	BarCode_BMPColor_Type barRGB;
	if(isColorExchange) {
		bgRGB.idx.i = 1;
		barRGB.idx.i = 0;
	} else {
		bgRGB.idx.i = 0;
		barRGB.idx.i = 1;
	}
	ret = BarCode_BMP_Mem_Wrire_BK_Color(head, buf, bgRGB);
	//复制条码值
	int i, j;
	char EAN13_str[13];
	for(i=0; i<12; i++) EAN13_str[i] = barcode[i];
	//计算较验符
	EAN13_str[12] = EAN13_Get_Check_Sum(EAN13_str)+0x30;
	barcode[12] = EAN13_str[12];
	//查找左边编码规则
	unsigned char rule = EAN13_LEFT_HAND_ENCODING_RULES[EAN13_str[0]-0x30];
	//计算Y轴长条（边界条）和短条（数据条）的高度与开始Y坐标
	int yLongBarHeight = h;
	int yShortBarHeight = yLongBarHeight*0.86;
	int yLongBarBegin = 0;
	int yShortBarBegin = 0;
	//定义起始的条X坐标
	int xBegin = thickness * 14;
	//当前条X坐标的索引
	int xIdx = xBegin;
	//当前数字的编码
	unsigned char code;
	//画起始符
	BarCode_BMP_Mem_Fill_Rect(head, buf, xIdx, yLongBarBegin, thickness, yLongBarHeight, barRGB);
	xIdx += thickness<<1;
	BarCode_BMP_Mem_Fill_Rect(head, buf, xIdx, yLongBarBegin, thickness, yLongBarHeight, barRGB);
	xIdx += thickness;
	//画左边区域
	for(i=1; i<7; i++) {
		code = (rule>>(6-i)&1)? EAN13_LEFT_HAND_ENCODING_ODD[EAN13_str[i]-0x30]: EAN13_LEFT_HAND_ENCODING_EVEN[EAN13_str[i]-0x30];
		for(j=0; j<7; j++) {
			if(code>>(6-j)&1) BarCode_BMP_Mem_Fill_Rect(head, buf, xIdx, yShortBarBegin, thickness, yShortBarHeight, barRGB);
			xIdx += thickness;
		}
	}
	//画中间分隔符
	xIdx += thickness;
	BarCode_BMP_Mem_Fill_Rect(head, buf, xIdx, yLongBarBegin, thickness, yLongBarHeight, barRGB);
	xIdx += thickness<<1;
	BarCode_BMP_Mem_Fill_Rect(head, buf, xIdx, yLongBarBegin, thickness, yLongBarHeight, barRGB);
	xIdx += thickness<<1;
	//画右侧
	for(i=0; i<6; i++) {
		code = EAN13_RIGHT_HAND_ENCODING[EAN13_str[i+7]-0x30];
		for(j=0; j<7; j++) {
			if(code>>(6-j)&1) BarCode_BMP_Mem_Fill_Rect(head, buf, xIdx, yShortBarBegin, thickness, yShortBarHeight, barRGB);
			xIdx += thickness;
		}
	}
	//画终止符
	BarCode_BMP_Mem_Fill_Rect(head, buf, xIdx, yLongBarBegin, thickness, yLongBarHeight, barRGB);
	xIdx += thickness<<1;
	BarCode_BMP_Mem_Fill_Rect(head, buf, xIdx, yLongBarBegin, thickness, yLongBarHeight, barRGB);
	return 0;
}



