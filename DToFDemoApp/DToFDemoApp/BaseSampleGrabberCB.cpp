#include "pch.h"
#include "BaseSampleGrabberCB.h"

std::queue<std::vector<BYTE>> frameQueue;

STDMETHODIMP BaseSampleGrabberCB::SampleCB(double SampleTime, IMediaSample* pSample) {
	BYTE* pBuffer;
	long size;
	HRESULT hr = pSample->GetPointer(&pBuffer);
	if (FAILED(hr) || !pBuffer) {
		return hr;
	}

	size = pSample->GetSize();
	if (size != RANGING_MODE_HEIGHT * RANGING_MODE_WIDTH) {
		return E_FAIL; // 檢查緩衝區大小是否匹配
	}

	if (!isFull) {
		for (int i = 0; i < RANGING_MODE_HEIGHT; ++i) {
			memcpy(gCameraData[i], pBuffer + (i * RANGING_MODE_WIDTH), RANGING_MODE_WIDTH);
		}
		isFull = true;
	}

	if (0 != fileCount) {
		frameQueue.push(std::vector<BYTE>(pBuffer, pBuffer + size));
		fileCount--;
	}

	return S_OK;
}

void BaseSampleGrabberCB::setBuffer(unsigned char** buffer) {
	this->gCameraData = buffer;
}

void BaseSampleGrabberCB::setIsNotFull() {
	isFull = false;
}

bool BaseSampleGrabberCB::getIsFull() {
	return isFull;
}

void BaseSampleGrabberCB::setFileCount(int count) {
	this->fileCount = count;
}