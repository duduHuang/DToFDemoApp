#include "pch.h"
#include "BaseDirectShowCamera.h"

BaseDirectShowCamera::BaseDirectShowCamera() {
	// 初始化 COM 庫
	hr = CoInitializeEx(NULL, COINIT_MULTITHREADED);
	if (hr == RPC_E_CHANGED_MODE) {
		outFile << "Error: " << "COM already initialized with a different threading model." << std::endl;
	} else if (FAILED(hr)) {
		outFile << "Error: " << "Failed to initialize COM library" << " (HRESULT: " << std::hex << hr << ")" << std::endl;
		return;
	}

	cameraData = new unsigned char* [RANGING_MODE_HEIGHT];
	for (int i = 0; i < RANGING_MODE_HEIGHT; ++i) {
		cameraData[i] = new unsigned char[RANGING_MODE_WIDTH];
		std::fill(cameraData[i], cameraData[i] + RANGING_MODE_WIDTH, 0);
	}
	grabberCallback.setBuffer(cameraData);

	CreateDirectory(tempPath, NULL);
	std::wstring logPath = std::wstring(tempPath) + L"log.txt";
	outFile.open(logPath, std::ios::out);
}

BaseDirectShowCamera::~BaseDirectShowCamera() {
	if (pControl) {
		pControl.Release();
		pControl = nullptr;
	}
	if (pMoniker) {
		pMoniker.Release();
		pMoniker = nullptr;
	}
	if (pSampleGrabberFilter) {
		pSampleGrabberFilter.Release();
		pSampleGrabberFilter = nullptr;
	}
	if (pSampleGrabber) {
		pSampleGrabber.Release();
		pSampleGrabber = nullptr;
	}
	if (pSourceFilter) {
		pSourceFilter.Release();
		pSourceFilter = nullptr;
	}
	if (pBuilder) {
		pBuilder.Release();
		pBuilder = nullptr;
	}
	if (pGraph) {
		pGraph.Release();
		pGraph = nullptr;
	}
	if (pEnum) {
		pEnum.Release();
		pEnum = nullptr;
	}
	if (pDevEnum) {
		pDevEnum.Release();
		pDevEnum = nullptr;
	}
	if (pKsControl) {
		pKsControl.Release();
		pKsControl = nullptr;
	}
	CoUninitialize();
	outFile.close();
	if (nullptr != cameraData) {
		for (int i = 0; i < RANGING_MODE_HEIGHT; ++i) {
			delete[] cameraData[i];
		}
		delete[] cameraData;
		cameraData = nullptr;
	}
}

int BaseDirectShowCamera::listDevices(std::vector<std::string>& list) {
	ICreateDevEnum* pDevEnum = NULL;
	IEnumMoniker* pEnum = NULL;
	int deviceCounter = 0;

	hr = CoCreateInstance(CLSID_SystemDeviceEnum, NULL,
		CLSCTX_INPROC_SERVER, IID_ICreateDevEnum,
		reinterpret_cast<void**>(&pDevEnum));
	if (SUCCEEDED(hr)) {
		// Create an enumerator for the video capture category.
		hr = pDevEnum->CreateClassEnumerator(
			CLSID_VideoInputDeviceCategory,
			&pEnum, 0);

		if (SUCCEEDED(hr)) {
			IMoniker* pMoniker = NULL;

			while (S_OK == pEnum->Next(1, &pMoniker, NULL)) {
				IPropertyBag* pPropBag;
				hr = pMoniker->BindToStorage(0, 0, IID_IPropertyBag, (void**)(&pPropBag));
				if (FAILED(hr)) {
					pMoniker->Release();
					continue;  // Skip this one, maybe the next one will work.
				}

				// Find the description or friendly name.
				VARIANT varName;
				VariantInit(&varName);
				hr = pPropBag->Read(L"Description", &varName, 0);
				if (FAILED(hr)) {
					hr = pPropBag->Read(L"FriendlyName", &varName, 0);
				}

				if (FAILED(hr)) {
					list.push_back("unknow camera");
				}
				else if (SUCCEEDED(hr)) {
					hr = pPropBag->Read(L"FriendlyName", &varName, 0);

					int count = 0;
					char tmp[255] = { 0 };
					while (varName.bstrVal[count] != 0x00 && count < 255) {
						tmp[count] = (char)varName.bstrVal[count];
						count++;
					}
					list.push_back(tmp);
				}
				pPropBag->Release();
				pPropBag = NULL;
				pMoniker->Release();
				pMoniker = NULL;
				deviceCounter++;
			}
			pDevEnum->Release();
			pDevEnum = NULL;
			pEnum->Release();
			pEnum = NULL;
		}
	}

	outFile << "Device counter:" << deviceCounter << std::endl;
	for (int i = 0; i < deviceCounter; ++i) {
		outFile << "Device #" << i << ": " << list.at(i) << std::endl;
	}
	return deviceCounter;
}

void BaseDirectShowCamera::openCamera(const std::string targetDevice) {
	hr = CoCreateInstance(CLSID_SystemDeviceEnum, nullptr, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&pDevEnum));
	if (FAILED(hr)) {
		outFile << "Failed to create device enumerator, HRESULT: " << hr << std::endl;
		return;
	}

	// 創建視頻捕獲類別的枚舉器
	hr = pDevEnum->CreateClassEnumerator(CLSID_VideoInputDeviceCategory, &pEnum, 0);
	if (FAILED(hr)) {
		outFile << "No video capture devices found." << std::endl;
		return;
	}

	// 列舉所有視頻設備
	while (pEnum->Next(1, &pMoniker, nullptr) == S_OK) {
		IPropertyBag* pPropBag;
		hr = pMoniker->BindToStorage(0, 0, IID_IPropertyBag, (void**)(&pPropBag));
		if (FAILED(hr)) {
			pMoniker.Release();
			continue;  // Skip this one, maybe the next one will work.
		}

		// Find the description or friendly name.
		VARIANT varName;
		VariantInit(&varName);
		hr = pPropBag->Read(L"Description", &varName, 0);
		if (FAILED(hr)) {
			hr = pPropBag->Read(L"FriendlyName", &varName, 0);
		}

		if (SUCCEEDED(hr)) {
			hr = pPropBag->Read(L"FriendlyName", &varName, 0);

			if (SUCCEEDED(hr)) {
				std::wstring wtmp(varName.bstrVal);  // BSTR to std::wstring
				std::string tmp(wtmp.begin(), wtmp.end());  // std::wstring to std::string
				if (tmp == targetDevice) {
					deviceName = targetDevice;
					outFile << "Selected: " << deviceName << std::endl;
					break;
				} else {
					outFile << "Found device: " << tmp << std::endl;
				}
			}
		}
		pMoniker.Release();
	}

	if (!pMoniker) {
		outFile << "Failed to find the target video capture device." << std::endl;
		return;
	}
}

void BaseDirectShowCamera::prepareCamera() {
	// 創建圖表管理器
	hr = CoCreateInstance(CLSID_FilterGraph, nullptr, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&pGraph));
	if (FAILED(hr)) {
		outFile << "Failed to create filter graph, HRESULT: " << std::hex << hr << std::endl;
		return;
	}

	hr = CoCreateInstance(CLSID_CaptureGraphBuilder2, nullptr, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&pBuilder));
	if (FAILED(hr)) {
		outFile << "Failed to create capture graph builder, HRESULT: " << std::hex << hr << std::endl;
		return;
	}

	// 初始化圖表
	hr = pBuilder->SetFiltergraph(pGraph);
	if (FAILED(hr)) {
		outFile << "Failed to set filter graph, HRESULT: " << std::hex << hr << std::endl;
		return;
	}

	// 加載攝像頭設備
	hr = pMoniker->BindToObject(0, 0, IID_IBaseFilter, (void**)&pSourceFilter);
	if (FAILED(hr)) {
		outFile << "Failed to bind moniker to source filter, HRESULT: " << std::hex << hr << std::endl;
		return;
	}

	hr = pGraph->AddFilter(pSourceFilter, L"Video Capture");
	if (FAILED(hr)) {
		outFile << "Failed to add source filter to graph, HRESULT: " << std::hex << hr << std::endl;
		return;
	}

	// 創建 SampleGrabber
	hr = CoCreateInstance(CLSID_SampleGrabber, nullptr, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&pSampleGrabber));
	if (FAILED(hr)) {
		outFile << "Failed to create SampleGrabber. HRESULT: " << std::hex << hr << std::endl;
		return;
	}

	// 獲取 Sample Grabber 的 IBaseFilter 接口
	hr = pSampleGrabber->QueryInterface(IID_PPV_ARGS(&pSampleGrabberFilter));
	if (FAILED(hr)) {
		outFile << "Failed to query IBaseFilter from Sample Grabber. HRESULT: " << std::hex << hr << std::endl;
		return;
	}

	hr = pGraph->AddFilter(pSampleGrabberFilter, L"Sample Grabber");
	if (FAILED(hr)) {
		outFile << "Failed to add Sample Grabber to Filter Graph. HRESULT: " << std::hex << hr << std::endl;
		return;
	}

	// 設置 SampleGrabber 的媒體類型
	ZeroMemory(&mt, sizeof(mt));
	mt.majortype = MEDIATYPE_Video;
	mt.subtype = (deviceName == dToFDevice) ? MEDIASUBTYPE_UYVY : MEDIASUBTYPE_YUY2; // 或其他格式
	hr = pSampleGrabber->SetMediaType(&mt);
	if (FAILED(hr)) {
		outFile << "Failed to set media type on SampleGrabber. HRESULT: " << std::hex << hr << std::endl;
		return;
	}

	hr = pSampleGrabber->SetCallback(&grabberCallback, 0); // 使用自定義回調來處理樣本數據

	// 創建渲染器過濾器
	/*hr = CoCreateInstance(CLSID_VideoRenderer, nullptr, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&pRenderer));
	if (FAILED(hr)) {
		outFile << "Failed to create video renderer. HRESULT: " << std::hex << hr << std::endl;
		return;
	}

	hr = pGraph->AddFilter(pRenderer, L"Video Renderer");
	if (FAILED(hr)) {
		outFile << "Failed to add video renderer to graph. HRESULT: " << std::hex << hr << std::endl;
		return;
	}*/

	// 過濾器連接
	hr = pBuilder->RenderStream(&PIN_CATEGORY_CAPTURE, &MEDIATYPE_Video, pSourceFilter, pSampleGrabberFilter, nullptr);
	if (FAILED(hr)) {
		outFile << "Failed to render preview stream: " << std::hex << hr << std::endl;
		return;
	}

	hr = pSourceFilter->QueryInterface(__uuidof(IKsControl), (void**)&pKsControl);
	if (FAILED(hr) || !pKsControl) {
		std::cerr << "IKsControl Failed: " << std::hex << hr << std::endl;
		return;
	}

	// 開始運行圖表
	hr = pGraph->QueryInterface(IID_PPV_ARGS(&pControl));
	if (FAILED(hr)) {
		outFile << "Failed to get Media Control interface: " << std::hex << hr << std::endl;
		return;
	}
}

void BaseDirectShowCamera::run() {
	hr = pControl->Run();
	if (FAILED(hr)) {
		outFile << "Failed to run the graph: " << std::hex << hr << std::endl;
		return;
	}
	outFile << "Preview running. Press Enter to exit..." << std::endl;
}

void BaseDirectShowCamera::stop() {
	// 停止圖表
	if (pControl) {
		pControl->Stop();
	}
	grabberCallback.setIsNotFull();
}

void BaseDirectShowCamera::ShowCameraData()
{
}

void BaseDirectShowCamera::sendCx3Command(uint16_t reg, uint8_t data) {
	ULONG cbReturned = 0;

	KSP_NODE ksNode = {};
	ksNode.Property.Set = CX3_XU_GUID;
	ksNode.Property.Id = 0x03; // 對應 CX3 的控制 ID，例如 wValue
	ksNode.Property.Flags = KSPROPERTY_TYPE_SET | KSPROPERTY_TYPE_TOPOLOGY;
	ksNode.NodeId = 2;         // XU 的 Node ID（參考 UVC Descriptor）
	ksNode.Reserved = 0;

	// 傳送的 payload，根據韌體定義
	UCHAR buffer[6] = { 0x00, 0x00, reg & 0xFF, (reg >> 8) & 0xFF, data & 0xFF, 0x00}; // 寫入Sensor寄存器範例

	hr = pKsControl->KsProperty(
		(PKSPROPERTY) & ksNode,
		sizeof(KSP_NODE),
		buffer,
		sizeof(buffer),
		&cbReturned);

	if (FAILED(hr)) {
		outFile << "KsProperty Failed: " << std::hex << hr << std::endl;
	} else {
		outFile << "Success. Bytes returned: " << cbReturned << std::endl;
	}
}