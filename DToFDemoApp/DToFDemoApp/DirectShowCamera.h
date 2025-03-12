#ifndef __H_DIRECTSHOWCAMERA__
#define __H_DIRECTSHOWCAMERA__

#include <opencv2/opencv.hpp>
#include <opencv2/highgui/highgui_c.h>
#include <windows.h>
#include <filesystem>
#include <fstream>
#include <cstdint>
#include <random>
#include <vector>
#include <iostream>
#include <dshow.h>
#include <atlbase.h> // 使用 CComPtr 管理 COM 物件
#include "qedit.h"  // For ISampleGrabber
#pragma comment(lib, "strmiids.lib")  // DirectShow library
#pragma comment(lib, "quartz.lib")

#include <mgl2/mgl.h>
#undef  _CRT_STDIO_ISO_WIDE_SPECIFIERS 
#undef MGL_STATIC_DEFINE

#include <mgl2/type.h>

#define DP_NUMBER 576
#define SPOT_NUMBER 24
#define HISTO_NUMBER 144
#define RANGING_MODE_WIDTH 672
#define RANGING_MODE_HEIGHT 600
#define INDEX_START_X 92
#define INDEX_START_Y 644

struct WriteFileThreadParams {
	int fileCount;
	CString filePath;
};

class MySampleGrabberCallback : public ISampleGrabberCB {
private:
	unsigned char** gCameraData;
	bool isFull = false;
	int fileCount = 0;
	ULONG refCount;
public:
	STDMETHODIMP SampleCB(double SampleTime, IMediaSample* pSample) override;

	STDMETHODIMP BufferCB(double SampleTime, BYTE* pBuffer, long BufferLen) override {
		return E_NOTIMPL;  // 不用處理 BufferCB
	}

	STDMETHOD(QueryInterface)(REFIID riid, void** ppvObject) override {
		if (riid == IID_IUnknown) {
			*ppvObject = static_cast<IUnknown*>(this);
			AddRef();
			return S_OK;
		}
		*ppvObject = nullptr;
		return E_NOINTERFACE;
	};

	STDMETHOD_(ULONG, AddRef)() override {
		return InterlockedIncrement(&refCount);
	};

	STDMETHOD_(ULONG, Release)() override {
		ULONG count = InterlockedDecrement(&refCount);
		if (count == 0) {
			delete this;
		}
		return count;
	};

	void setBuffer(unsigned char** buffer);
	void setIsNotFull();
	bool getIsFull();
	void setFileCount(int count);
};

class DirectShowCamera {
public:
	DirectShowCamera ();
	~DirectShowCamera ();
	int listDevices(std::vector<std::string>& list);
	void setSubViewWH(int width, int height);
	void setCDC(CDC* pLTDC, CDC* pRTDC, CDC* pLBDC, CDC* pRBDC);
	void openCamera(const char* targetDevice);
	void prepareCamera();
	void run();
	void stop();
	POINT* getPoints();
	void ShowCameraData();
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

	volatile bool isPreview;

private:
	void subView(CDC* pDC, uchar* data, int width, int height);

	HRESULT hr;
	int subViewWidth, subViewHeight;
	CDC* pLTDC, * pRTDC, * pLBDC, * pRBDC;
	CWinThread* m_pThread, * m_writeFileThread;
	CComPtr<ICreateDevEnum> pDevEnum;
	CComPtr<IEnumMoniker> pEnum;
	CComPtr<IGraphBuilder> pGraph;
	CComPtr<ICaptureGraphBuilder2> pBuilder;
	CComPtr<IBaseFilter> pSourceFilter;
	CComPtr<ISampleGrabber> pSampleGrabber;
	CComPtr<IBaseFilter> pSampleGrabberFilter;
	CComPtr<IMoniker> pMoniker;
	CComPtr<IBaseFilter> pRenderer;
	CComPtr<IMediaControl> pControl;
	AM_MEDIA_TYPE mt;
	MySampleGrabberCallback grabberCallback;
	TCHAR tempPath[MAX_PATH] = _T("D:\\RD_NT_DToF\\");
	std::ofstream outFile;
	unsigned char** cameraData;
	int** histarray;
	int* peak_z;
	int* newarray;
	POINT* histpoints;
	HCURSOR defaultcursor;
	mglData x, y, z;
	int rotatx = 40, rotaty = 50, histIndex = -1, histW = 0, histH = 0, cloud3dW = 0, cloud3dH = 0, filterThreshold = 1000, maxValue = 2250;
	char* deviceName;
	const char* dToFDevice = "CX3-UVC";
};

#endif // !__H_DIRECTSHOWCAMERA__