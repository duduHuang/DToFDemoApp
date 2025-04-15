#pragma once

#include <opencv2/opencv.hpp>
#include <opencv2/highgui/highgui_c.h>
#include <windows.h>
#include <vector>
#include "qedit.h"  // For ISampleGrabber

#define RANGING_MODE_WIDTH 672
#define RANGING_MODE_HEIGHT 600

class BaseSampleGrabberCB : public ISampleGrabberCB {
private:
	unsigned char** gCameraData;
	bool isFull = false;
	int fileCount = 0;
	ULONG refCount;

public:
	STDMETHODIMP SampleCB(double SampleTime, IMediaSample* pSample) override;

	STDMETHODIMP BufferCB(double SampleTime, BYTE* pBuffer, long BufferLen) override {
		return E_NOTIMPL;  // 不用處理 BufferCB
	};

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