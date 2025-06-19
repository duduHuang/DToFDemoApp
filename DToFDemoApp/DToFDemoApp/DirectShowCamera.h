#pragma once

#include <mgl2/mgl.h>
#undef  _CRT_STDIO_ISO_WIDE_SPECIFIERS 
#undef MGL_STATIC_DEFINE

#include <mgl2/type.h>

#include "BaseDirectShowCamera.h"

#define DP_NUMBER 576
#define SPOT_NUMBER 24
#define HISTO_NUMBER 144

struct WriteFileThreadParams {
	int fileCount;
	CString filePath;
};

class DirectShowCamera : public BaseDirectShowCamera {
public:
	DirectShowCamera ();
	~DirectShowCamera ();
	void setSubViewWH(int width, int height);
	void setCDC(CDC* pLTDC, CDC* pRTDC, CDC* pLBDC, CDC* pRBDC);
	void run() override;
	void stop() override;
	POINT* getPoints();
	void ShowCameraData() override;
	void LTSubView();
	void RTSubView();
	void LBSubView();
	void RBSubView();
	void setHistIndex(const int hist, const int width, const int height);
	void setRotate(const int x, const int y, const int width, const int height);
	void setFilterThreshold(int threshold);
	void setMaxValue(int max);
	void Cloud3D(int width, int height, uchar* pic, int rx, int ry);
	void Histgram(int width, int height, uchar* pic, const int histIndex);
	void Cloud2D(int width, int height, uchar* pic);
	void Filter2D(int width, int height, uchar* pic, int lowThresdhold, int hightThresdhold);

	void littleToBigFourByte(CString* bData, int _size, int exponent, int fraction, bool _signed, int* pint);
	void ExtractBin(int rowIndex, int num);
	void fullframe_process(int rowIndex, int* ppickz);
	void ParseOneLine();

	void writeFile(const int fileCount);
	void setSpeedUp();
	void setDefaultSpeed();

	volatile bool isPreview;

private:
	void subView(CDC* pDC, uchar* data, int width, int height);
	float US8littleToS192(uint8_t* bData);
	float US8littleToUS192(uint8_t* bData);

	int subViewWidth, subViewHeight;
	CDC* pLTDC, * pRTDC, * pLBDC, * pRBDC;
	CWinThread* m_pThread, * m_writeFileThread;
	int** histarray;
	int* peak_z;
	int* newarray;
	std::vector<std::array<float, 3>> pointCloud;
	int pointCloudFileCount;
	POINT* histpoints;
	HCURSOR defaultcursor;
	mglData x, y, z, pointCloudX, pointCloudY, pointCloudZ;
	bool isSpeedUp;
	int rotatx = 40, rotaty = 50, histIndex = -1, histW = 0, histH = 0, cloud3dW = 0, cloud3dH = 0, filterThreshold = 1000, maxValue = 2250;
};