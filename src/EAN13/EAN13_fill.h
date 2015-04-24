/*
 * EAN13_writer.h
 *
 *  Created on: 2015年4月22日
 *      Author: welkinm
 */

#ifndef EAN13_EAN13_FILL_H_
#define EAN13_EAN13_FILL_H_


const unsigned char EAN13_LEFT_HAND_ENCODING_ODD[10]={13,25,19,61,35,49,47,59,55,11};
//const unsigned char EAN13_LEFT_HAND_ENCODING_ODD[10] = {
//	0b0001101,	//0
//	0b0011001,	//1
//	0b0010011,	//2
//	0b0111101,	//3
//	0b0100011,	//4
//	0b0110001,	//5
//	0b0101111,	//6
//	0b0111011,	//7
//	0b0110111,	//8
//	0b0001011	//9
//};
const unsigned char EAN13_LEFT_HAND_ENCODING_EVEN[10]={39,51,27,33,29,57,5,17,9,23};
//const unsigned char EAN13_LEFT_HAND_ENCODING_EVEN[10] = {
//	0b0100111,	//0
//	0b0110011,	//1
//	0b0011011,	//2
//	0b0100001,	//3
//	0b0011101,	//4
//	0b0111001,	//5
//	0b0000101,	//6
//	0b0010001,	//7
//	0b0001001,	//8
//	0b0010111	//9
//};
const unsigned char EAN13_RIGHT_HAND_ENCODING[10]={114,102,108,66,92,78,80,68,72,116};
//const unsigned char EAN13_RIGHT_HAND_ENCODING[10] = {
//	0b1110010,	//0
//	0b1100110,	//1
//	0b1101100,	//2
//	0b1000010,	//3
//	0b1011100,	//4
//	0b1001110,	//5
//	0b1010000,	//6
//	0b1000100,	//7
//	0b1001000,	//8
//	0b1110100	//9
//};
const unsigned char EAN13_LEFT_HAND_ENCODING_RULES[10]={63,52,50,49,44,38,35,42,41,37};
//const unsigned char EAN13_LEFT_HAND_ENCODING_RULES[10] = {
//				//    0    1    2    3    4    5
//	0b111111,	//0  odd  odd  odd  odd  odd  odd
//	0b110100,	//1  odd  odd  even odd  even even
//	0b110010,	//2  odd  odd  even even odd  even
//	0b110001,	//3  odd  odd  even even even odd
//	0b101100,	//4  odd  even odd  odd  even even
//	0b100110, 	//5  odd  even even odd  odd  even
//	0b100011,	//6  odd  even even even odd  odd
//	0b101010,	//7  odd  even odd  even odd  even
//	0b101001,	//8  odd  even odd  even even odd
//	0b100101,	//9  odd  even even odd  even odd
//};

char EAN13_Get_Check_Sum(const char *barcode);
int EAN13_Fill_Buf(char *barcode, unsigned char *buf, unsigned int len, unsigned int& w, unsigned int& h, unsigned int& bmpLen, bool isColorExchange);



#endif /* EAN13_EAN13_FILL_H_ */
