/*
 * barcode.h
 *
 *  Created on: 2015年5月1日
 *      Author: welkinm
 */

#ifndef BARCODE_H_
#define BARCODE_H_

#include <memory>

//打印指令类型
enum Barcode_Type {
	BARCODE_TYPE_EAN13 = 1,
	BARCODE_TYPE_CODE128 = 10
};


class BarcodeData {
public:
	unsigned int len;
	unsigned char *data;

	BarcodeData(unsigned int length){
		len=length;
		data = new unsigned char[len];
	}
	virtual ~BarcodeData() {
		len = 0;
		if (data) {
			delete []data;
			data=NULL;
		}
	}
};
typedef std::shared_ptr<BarcodeData> BarcodeDataSharePtr;
typedef std::auto_ptr<BarcodeData> BarcodeDataAutoPtr;

#endif /* BARCODE_H_ */
