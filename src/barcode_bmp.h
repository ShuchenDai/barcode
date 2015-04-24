/*
 * barcode_bmp.h
 *
 *  Created on: 2015年4月22日
 *      Author: welkinm
 */

#ifndef BARCODE_BMP_H_
#define BARCODE_BMP_H_


#if __BYTE_ORDER == __LITTLE_ENDIAN
	#define BARCODE_BMP_TYPE (unsigned short) 0X4D42
#elif __BYTE_ORDER == __BIG_ENDIAN
	#define BARCODE_BMP_TYPE (unsigned short) 0X424D
#endif


#define BARCODE_BMP_FILE_HEADER_LENGTH 14
#define BARCODE_BMP_INFORMATION_HEADER_LENGTH 40

//排序为 蓝 绿 红
typedef struct BARCODE_BMPRGBQUAD_TYPEDEF {
	unsigned char b;			//蓝，范围 0-255
	unsigned char g;			//绿，范围 0-255
	unsigned char	r;			//红，范围 0-255
	unsigned char a;			//透明度
} BarCode_BMPRGBQuad_Type;
extern BarCode_BMPRGBQuad_Type BARCODE_BMP_COLOR_TABLE_1[2];

//颜色索引
typedef struct BARCODE_BMCOLOR_IDX_TYPEDEF {
	unsigned short i;
	unsigned short reserved;
} BarCode_BMPColor_Idx_Type;
//颜色
typedef union BARCODE_BMPCOLOR_TYPEDEF {
	BarCode_BMPRGBQuad_Type rgb;
	BarCode_BMPColor_Idx_Type idx;
} BarCode_BMPColor_Type;


//位图文件头 bitmap file header(14 byte)
//BMP文件头数据结构含有BMP文件的类型、文件大小和位图起始位置等信息
typedef struct BARCODE_BMPFH_TYPEDEF {
	unsigned short bfType;				//00-01h, 位图文件类型，必需为 "BM"
	unsigned int	 bfSize;			//02-05h, 位图文件大小，以字节为单位
	unsigned short bfReserved1;		//06-07h, 保留，必需为0
	unsigned short bfReserved2;		//08-09h, 保留，必需为0
	unsigned int	 bfOffBits;			//0a-0dh, 位图数据的起始位置，以相对于位图文件头的偏移量表示，以字节为单位
} BarCode_BMPFH_Type;

//位图信息头 bitmap information header(40 byte)
//BMP位图信息头数据用于说明位图的尺寸等信息
typedef struct BARCODE_BMPIH_TYPEDEF {
	unsigned int	 biSize;			//0e-11h, 本结构所占用字节数   
	unsigned int	 biWidth;			//12-15h, 位图的宽度，以像素为单位
	unsigned int	 biHeight;			//16-19h, 位图的高度，以像素为单位
	unsigned short biPlanes;			//1a-1bh, 目标设备的级别，必须为1 
	unsigned short biBitCount;			//1c-1dh, 每个像素所需的位数，必须是1(双色),4(16色)，8(256色)或24(真彩色)之一  
	unsigned int	 biCompression;		//1e-21h, 位图压缩类型，必须是 0(不压缩),1(BI_RLE8压缩类型)或2(BI_RLE4压缩类型)之一 
	unsigned int	 biSizeImage;		//22-25h, 位图数据的大小，以字节为单位 
	unsigned int	 biXPelsPerMeter;	//26-29h, 位图水平分辨率，每米像素数 
	unsigned int	 biYPelsPerMeter;	//2a-2dh, 位图垂直分辨率，每米像素数
	unsigned int	 biClrUsed;			//2e-31h, 位图实际使用的颜色表中的颜色数
	unsigned int	 biClrImportant;	//32-35h, 位图显示过程中重要的颜色数
} BarCode_BMPIH_Type;

typedef struct BARCODE_BMPHEAD_TYPEDEF {
	BarCode_BMPFH_Type fh;
	BarCode_BMPIH_Type ih;
	BarCode_BMPRGBQuad_Type *ct;			//颜色表
	unsigned short ctCount;			//颜色表项数
	unsigned int	widthBypes;			//位图每行的实际存储所需的字节数，以字节为单位
} BarCode_BMPHead_Type;

//颜色表 bitmap color table(不定长)
//颜色表用于说明位图中的颜色，它有若干个表项，每一个表项是一个RGBQUAD类型的结构，定义一种颜色。
//颜色表中 BMRGBQUAD 结构数据的个数由 BMIH biBitCount来确定:  当biBitCount=1,4,8时，分别有2,16,256个表项;  当biBitCount=24时，没有颜色表项。
//排序为 蓝 绿 红 0

//位图数据：位图数据记录了位图的每一个像素值，记录顺序是在扫描行内是从左到右,扫描行之间是从下到上。
//位图的一个像素值所占的字节数:  当biBitCount=1时，8个像素占1个字节; 当biBitCount=4时，2个像素占1个字节; 
//							   当biBitCount=8时，1个像素占1个字节; 当biBitCount=24时,1个像素占3个字节;




/*
 * 根据位图的像素宽度和每行像素数，计算每行实际存储所需的字节数
 */
unsigned int BarCode_BMP_Get_Width_Bypes(unsigned int pixelsCount, unsigned short bitCount);


/*
 * 根据位图的像素宽和像素高、每像素位数
 * 如果每像素位数为1，4，8，需提供颜色表，颜色表的长度满足: len(colorTable) ＝ pow(2,bitCount)
 */
int BarCode_BMP_Build_Head(BarCode_BMPHead_Type &head,
		unsigned int w, unsigned int h, unsigned short bitCount, BarCode_BMPRGBQuad_Type *colorTable);

/*
 * 将BMP相关描述信息写入指定buf
 */
int BarCode_BMP_Mem_Write_Head(BarCode_BMPHead_Type &head, unsigned char *buf, unsigned int bufLen);

/*
 * 填充背景色
 */
int BarCode_BMP_Mem_Wrire_BK_Color(BarCode_BMPHead_Type &head, unsigned char *buf, BarCode_BMPColor_Type &bgColor);

/*
 * 方便初始化位图
 */
int BarCode_BMP_Mem_Write_Default(BarCode_BMPHead_Type &head, unsigned char *buf, unsigned int bufLen,
		unsigned int& w, unsigned int& h, unsigned short bitCount, BarCode_BMPRGBQuad_Type *colorTable,
		BarCode_BMPColor_Type &bgColor);

/*
 * 画一个像素
 */
int BarCode_BMP_Mem_Wrire_Pixel(BarCode_BMPHead_Type &head, unsigned char *buf,
		unsigned int x, unsigned int y, BarCode_BMPColor_Type &color);

/*
 * 画一个矩形
 */
//int BarCode_BMP_Mem_Fill_Rect(BarCode_BMPHead_Type &head, unsigned char *buf, unsigned int x, unsigned int y,
//		unsigned int w, unsigned int h, BarCode_BMPColor_Type &color);
int BarCode_BMP_Mem_Fill_Rect(BarCode_BMPHead_Type &head, unsigned char *buf, unsigned int x, unsigned int y,
		unsigned int w, unsigned int h, BarCode_BMPColor_Type &color);





//
////位图数据：位图数据记录了位图的每一个像素值，记录顺序是在扫描行内是从左到右,扫描行之间是从下到上。
////位图的一个像素值所占的字节数:  当biBitCount=1时，8个像素占1个字节; 当biBitCount=4时，2个像素占1个字节; 
////							   当biBitCount=8时，1个像素占1个字节; 当biBitCount=24时,1个像素占3个字节;
//
//int BarCode_BMP_Build_Normal_Head(BarCode_BMPHead_Type *head, unsigned int& w, unsigned int h);
//int BarCode_BMP_Mem_Write_Head(BarCode_BMPHead_Type *head, unsigned char *buf, unsigned int len);
//int BarCode_BMP_Mem_Set_Default(BarCode_BMPHead_Type *head, unsigned char *buf, unsigned int bufLen,
//		unsigned int& w, unsigned int& h, unsigned int& bmpLen,
//		BarCode_BMRGBQuad_Type &bgRGB, BarCode_BMRGBQuad_Type &barRGB, bool isColorExchange);
//
//int BarCode_BMP_Mem_Wrire_BK_Color(BarCode_BMPHead_Type &head, unsigned char *buf, BarCode_BMRGBQuad_Type &rgb);
//int BarCode_BMP_Mem_Wrire_Pixel(BarCode_BMPHead_Type &head, unsigned char *buf, unsigned int x, unsigned int y, BarCode_BMRGBQuad_Type &rgb);
//int BarCode_BMP_Mem_Fill_Rect(BarCode_BMPHead_Type &head, unsigned char *buf, unsigned int x, unsigned int y,
//		unsigned int w, unsigned int h, BarCode_BMRGBQuad_Type &rgb);
//
//unsigned int BarCode_BMP_Get_4_Align_Width(unsigned int width);


#endif /* BARCODE_BMP_H_ */
