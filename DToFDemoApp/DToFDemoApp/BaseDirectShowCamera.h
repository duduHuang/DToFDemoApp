#pragma once

#include <windows.h>
#include <filesystem>
#include <fstream>
#include <cstdint>
#include <random>
#include <iostream>
#include <dshow.h>
#include <atlbase.h> // 使用 CComPtr 管理 COM 物件
#pragma comment(lib, "strmiids.lib")  // DirectShow library
#pragma comment(lib, "quartz.lib")

#include "BaseSampleGrabberCB.h"

class BaseDirectShowCamera {
private:
	TCHAR tempPath[MAX_PATH] = _T("D:\\RD_NT_DToF\\");

public:
    BaseDirectShowCamera();
    ~BaseDirectShowCamera();
    virtual int listDevices(std::vector<std::string>& list);
	virtual void openCamera(const std::string targetDevice);
	virtual void prepareCamera();
	virtual void run();
	virtual void stop();
	virtual void ShowCameraData();

protected:
    HRESULT hr;
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
	BaseSampleGrabberCB grabberCallback;
	std::ofstream outFile;
	unsigned char** cameraData;
	std::string deviceName;
	const std::string dToFDevice = "CX3-UVC";

};