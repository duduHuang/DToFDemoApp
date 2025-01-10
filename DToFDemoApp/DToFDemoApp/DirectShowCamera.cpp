#include "pch.h"
#include "DirectShowCamera.h"

bool gIsFull = false;
unsigned char** gCameraData;

STDMETHODIMP MySampleGrabberCallback::SampleCB(double SampleTime, IMediaSample* pSample) {
	BYTE* pBuffer;
	long size;
	HRESULT hr = pSample->GetPointer(&pBuffer);
	if (FAILED(hr) || !pBuffer) {
		return hr;
	}

	size = pSample->GetSize();
	if (size != RANGING_MODE_HEIGHT * RANGING_MODE_WIDTH) {
		return E_FAIL; // �ˬd�w�İϤj�p�O�_�ǰt
	}

	if (!gIsFull) {
		for (int i = 0; i < RANGING_MODE_HEIGHT; ++i) {
			memcpy(gCameraData[i], pBuffer + (i * RANGING_MODE_WIDTH), RANGING_MODE_WIDTH);
		}
		gIsFull = true;
	}

	return S_OK;
}

UINT ShowWindowRealTimeImage(LPVOID pParam) {
	DirectShowCamera* pThis = reinterpret_cast<DirectShowCamera*>(pParam);
	pThis->ShowCameraData();
	return 0;
}

DirectShowCamera::DirectShowCamera(): m_pThread(nullptr),
pLTDC(nullptr), pRTDC(nullptr), pLBDC(nullptr), pRBDC(nullptr) {
	// ��l�� COM �w
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
	gCameraData = cameraData;

	histarray = new int*[DP_NUMBER];
	for (int i = 0; i < DP_NUMBER; ++i) {
		histarray[i] = new int[HISTO_NUMBER];
		std::fill(histarray[i], histarray[i] + HISTO_NUMBER, 0);
	}
	
	peak_z = new int[DP_NUMBER];
	newarray = new int[DP_NUMBER];
	histpoints = new POINT[DP_NUMBER];

	std::fill(peak_z, peak_z + DP_NUMBER, 0);
	std::fill(newarray, newarray + DP_NUMBER, 0);
	std::fill(histpoints, histpoints + DP_NUMBER, POINT{ 0, 0 });
	isPreview = false;

	CreateDirectory(tempPath, NULL);
	std::wstring logPath = std::wstring(tempPath) + L"log.txt";
	outFile.open(logPath, std::ios::out);

	x = mglData(DP_NUMBER);
	y = mglData(DP_NUMBER);
	z = mglData(DP_NUMBER);
}

DirectShowCamera ::~DirectShowCamera() {
	// ��������
	if (m_pThread) {
		// �q�����������
		m_bThreadStop = true;

		// ���ݰ��������
		WaitForSingleObject(m_pThread->m_hThread, INFINITE);

		// �R�����������
		m_pThread = nullptr;
	}
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
	CoUninitialize();
	if (nullptr != cameraData) {
		for (int i = 0; i < RANGING_MODE_HEIGHT; ++i) {
			delete[] cameraData[i];
		}
		delete[] cameraData;
		cameraData = nullptr;
	}
	if (nullptr != histarray) {
		for (int i = 0; i < DP_NUMBER; ++i) {
			delete[] histarray[i];
		}
		delete[] histarray;
		histarray = nullptr;
	}
	if (nullptr != peak_z) {
		delete[] peak_z;
		peak_z = nullptr;
	}
	if (nullptr != newarray) {
		delete[] newarray;
		newarray = nullptr;
	}
	if (nullptr != histpoints) {
		delete[] histpoints;
		histpoints = nullptr;
	}
	if (pLTDC) {
		pLTDC->DeleteDC();
		pLTDC = nullptr;
	}
	if (pRTDC) {
		pRTDC->DeleteDC();
		pRTDC = nullptr;
	}
	if (pLBDC) {
		pLBDC->DeleteDC();
		pLBDC = nullptr;
	}
	if (pRBDC) {
		pRBDC->DeleteDC();
		pRBDC = nullptr;
	}
	outFile.close();
}

int DirectShowCamera::listDevices(std::vector<std::string>& list) {
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

void DirectShowCamera::setSubViewWH(int width, int height) {
	subViewWidth = width;
	subViewHeight = height;
}

void DirectShowCamera::setCDC(CDC* pLTDC, CDC* pRTDC, CDC* pLBDC, CDC* pRBDC) {
	this->pLTDC = pLTDC;
	this->pRTDC = pRTDC;
	this->pLBDC = pLBDC;
	this->pRBDC = pRBDC;
}

void DirectShowCamera::openCamera(const char* targetDevice) {
	hr = CoCreateInstance(CLSID_SystemDeviceEnum, nullptr, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&pDevEnum));
	if (FAILED(hr)) {
		outFile << "Failed to create device enumerator, HRESULT: " << hr << std::endl;
		return;
	}

	// �Ыص��W�������O���T�|��
	hr = pDevEnum->CreateClassEnumerator(CLSID_VideoInputDeviceCategory, &pEnum, 0);
	if (FAILED(hr)) {
		outFile << "No video capture devices found." << std::endl;
		return;
	}

	// �C�|�Ҧ����W�]��
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

			int count = 0;
			char tmp[255] = { 0 };
			while (varName.bstrVal[count] != 0x00 && count < 255) {
				tmp[count] = (char)varName.bstrVal[count];
				count++;
			}
			if (0 == strcmp(tmp, targetDevice)) {
				break;
			}
		}
		pMoniker.Release();
	}

	if (!pMoniker) {
		outFile << "Failed to find the target video capture device." << std::endl;
		return;
	}
}

void DirectShowCamera::prepareCamera() {
	// �ЫعϪ�޲z��
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

	// ��l�ƹϪ�
	hr = pBuilder->SetFiltergraph(pGraph);
	if (FAILED(hr)) {
		outFile << "Failed to set filter graph, HRESULT: " << std::hex << hr << std::endl;
		return;
	}

	// �[���ṳ�Y�]��
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

	// �Ы� SampleGrabber
	hr = CoCreateInstance(CLSID_SampleGrabber, nullptr, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&pSampleGrabber));
	if (FAILED(hr)) {
		outFile << "Failed to create SampleGrabber. HRESULT: " << std::hex << hr << std::endl;
		return;
	}

	// ��� Sample Grabber �� IBaseFilter ���f
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

	// �]�m SampleGrabber ���C������
	ZeroMemory(&mt, sizeof(mt));
	mt.majortype = MEDIATYPE_Video;
	mt.subtype = MEDIASUBTYPE_UYVY; // �Ψ�L�榡
	hr = pSampleGrabber->SetMediaType(&mt);
	if (FAILED(hr)) {
		outFile << "Failed to set media type on SampleGrabber. HRESULT: " << std::hex << hr << std::endl;
		return;
	}

	hr = pSampleGrabber->SetCallback(&grabberCallback, 0); // �ϥΦ۩w�q�^�ըӳB�z�˥��ƾ�

	// �Ыش�V���L�o��
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

	// �L�o���s��
	hr = pBuilder->RenderStream(&PIN_CATEGORY_CAPTURE, &MEDIATYPE_Video, pSourceFilter, pSampleGrabberFilter, nullptr);
	if (FAILED(hr)) {
		outFile << "Failed to render preview stream: " << std::hex << hr << std::endl;
		return;
	}

	// �}�l�B��Ϫ�
	hr = pGraph->QueryInterface(IID_PPV_ARGS(&pControl));
	if (FAILED(hr)) {
		outFile << "Failed to get Media Control interface: " << std::hex << hr << std::endl;
		return;
	}
}

void DirectShowCamera::run() {
	hr = pControl->Run();
	if (FAILED(hr)) {
		outFile << "Failed to run the graph: " << std::hex << hr << std::endl;
		return;
	}
	outFile << "Preview running. Press Enter to exit..." << std::endl;
	// �K�[�����`���A�O���w�����f���}
	isPreview = true;
	m_pThread = AfxBeginThread(ShowWindowRealTimeImage, this);
}

void DirectShowCamera::stop() {
	// ����Ϫ�
	if (pControl) {
		pControl->Stop();
	}
	// ��������
	if (m_pThread) {
		// �q�����������
		m_bThreadStop = true;

		// ���ݰ��������
		WaitForSingleObject(m_pThread->m_hThread, INFINITE);

		// �R�����������
		m_pThread = nullptr;
	}
	isPreview = false;
	gIsFull = false;
}

void DirectShowCamera::ShowCameraData() {
	while (isPreview) {
		if (gIsFull) {
			ParseOneLine();
			int cnt = 0;
			for (int i = 0; i < SPOT_NUMBER; i++) {
				for (int j = 0; j < SPOT_NUMBER; j++) {
					z.a[cnt] = newarray[cnt];
					x.a[cnt] = i;
					y.a[cnt] = j;
					cnt++;
				}
			}
			LTSubView();
			RTSubView();
			LBSubView();
			RBSubView();
			gIsFull = false;
			std::cout << "show camera...\n";
			//outFile << "show camera...\n";
		}
	}
}

void DirectShowCamera::LTSubView() {
	cv::Mat pic(subViewWidth, subViewHeight, CV_8UC3);
	Cloud3D(subViewWidth, subViewHeight, pic.data, rotatx, rotaty);
	subView(pLTDC, pic.data);
}

void DirectShowCamera::RTSubView() {
	cv::Mat pic(subViewWidth, subViewHeight, CV_8UC3);
	Histgram(subViewWidth, subViewHeight, pic.data, histindex);
	subView(pRTDC, pic.data);
}

void DirectShowCamera::LBSubView() {
	cv::Mat pic(subViewWidth, subViewHeight, CV_8UC3);
	Cloud2D(subViewWidth, subViewHeight, pic.data);
	subView(pLBDC, pic.data);
}

void DirectShowCamera::RBSubView() {
	cv::Mat pic(subViewWidth, subViewHeight, CV_8UC3);
	Filter2D(subViewWidth, subViewHeight, pic.data, 1000, 0);
	subView(pRBDC, pic.data);
}

void DirectShowCamera::Cloud3D(int width, int height, uchar* pic, int rx, int ry) {
	mglGraph gr(0, width, height);
	gr.ClearFrame();

	gr.Title("3D Image", "", -1.4);
	gr.Rotate(ry, rx);
	gr.Label('x', "X", 0);
	gr.Label('y', "Y", 0);
	gr.Label('z', "Z", 1);
	gr.SetRanges(0, 25, 0, 25, 0, 2250);
	gr.Dots(x, y, z, "0.3");

	double valz[] = { 250, 500, 750, 1000, 1250, 1500, 1750, 2000, 2250 };
	gr.SetTicksVal('z', mglData(9, valz), "250\n500\n750\n1000\n1250\n1500\n1750\n2000\n2250");

	double valx[] = { 0, 5, 10, 15 , 20 ,25 };
	gr.SetTicksVal('x', mglData(6, valx), "0\n5\n10\n15\n20");
	double valy[] = { 0, 5, 10, 15 , 20 ,25 };
	gr.SetTicksVal('y', mglData(6, valy), "0\n5\n10\n15\n20\n25");

	gr.SetRange('c', 0, 2250);
	gr.SetTicksVal('c', mglData(9, valz), "250\n500\n750\n1000\n1250\n1500\n1750\n2000\n2250");
	gr.Colorbar("0123456789", 0.98, -0.3, 0.5, 1.7);

	gr.Axis();
	gr.Grid();

	cv::Mat pic1(gr.GetHeight(), gr.GetWidth(), CV_8UC3);
	pic1.data = const_cast<uchar*>(gr.GetRGB());
	cv::flip(pic1, pic1, 0);
	memcpy(pic, pic1.data, gr.GetHeight() * gr.GetWidth() * pic1.channels());
}

void DirectShowCamera::Histgram(int width, int height, uchar* pic, const int histindex) {
	mglGraph gr(0, width, height);
	gr.ClearFrame();
	int i, n = 45;
	mglData x(n), z(n);

	gr.Title("Histogram", "", -1.4);
	gr.MultiPlot(16, 16, 17, 15, 14, " ");
	gr.SetRanges(0, 45, -1200, 22500);
	gr.Box();
	gr.SetColor(0, 255, 255, 0);
	gr.SetColor(1, 0, 255, 255);

	if (histindex == -1) {
		for (int j = 0; j < DP_NUMBER; j++) {
			for (i = 0; i < n; i++) {
				x.a[i] = i;
				z.a[i] = histarray[j][i];
			}
			gr.Plot(x, z);
		}
	} else {
		for (i = 0; i < n; i++) {
			x.a[i] = i;
			z.a[i] = histarray[histindex][i];
		}
		gr.Plot(x, z);
	}

	double valz[] = { -2500, 0, 2500, 5000, 7500, 10000, 12500, 15000, 17500, 20000 };
	gr.SetTicksVal('y', mglData(10, valz), "\n0\n2500\n5000\n7500\n10000\n12500\n15000\n17500\n20000");
	gr.SetFontSize(3);
	gr.Label('x', "bin", 0);
	gr.Puts(mglPoint(-0.1, 1), "value", "a", 3);
	gr.Axis();

	cv::Mat pic1(gr.GetHeight(), gr.GetWidth(), CV_8UC3);
	pic1.data = const_cast<uchar*>(gr.GetRGB());
	cv::flip(pic1, pic1, 0);
	memcpy(pic, pic1.data, gr.GetHeight() * gr.GetWidth() * pic1.channels());
}

void DirectShowCamera::Cloud2D(int width, int height, uchar* pic) {
	mglGraph gr(0, width, height);
	gr.ClearFrame();
	double valz[] = { 250, 500, 750, 1000, 1250, 1500, 1750, 2000, 2250 };

	gr.Title("2D Image", "", -1.4);
	gr.MultiPlot(17, 17, 18, 13, 14, " ");
	gr.SetRange('c', 0, 2250);
	gr.SetTicksVal('c', mglData(9, valz), "250\n500\n750\n1000\n1250\n1500\n1750\n2000\n2250");
	gr.Colorbar("0123456789", 0.95, -0.05, 0.5, 1.13);
	gr.Label('x', "X", 0);
	gr.Label('y', "Y", 0);

	gr.SetRanges(-1, 24, -1, 24, 0, 2550);
	gr.Dots(x, y, z, "0.3");
	gr.Box();
	gr.Axis();

	cv::Mat pic1(gr.GetHeight(), gr.GetWidth(), CV_8UC3);
	pic1.data = const_cast<uchar*>(gr.GetRGB());
	cv::flip(pic1, pic1, 0);
	memcpy(pic, pic1.data, gr.GetHeight() * gr.GetWidth() * pic1.channels());
}

void DirectShowCamera::Filter2D(int width, int height, uchar* pic, int low_threadhold, int hightthreadhold) {
	mglGraph gr(0, width, height);
	gr.ClearFrame();
	int i = 0;
	double valz[] = { 250, 500, 750, 1000, 1250, 1500, 1750, 2000, 2250 };

	gr.Title("Filter 2D Image", "", -1.4);
	gr.MultiPlot(17, 17, 18, 13, 14, " ");
	gr.SetRange('c', 0, 2250);
	gr.SetTicksVal('c', mglData(9, valz), "250\n500\n750\n1000\n1250\n1500\n1750\n2000\n2250");
	gr.Colorbar("0123456789", 0.95, -0.05, 0.5, 1.13);
	gr.Label('x', "X", 0);
	gr.Label('y', "Y", 0);

	for (i = 0; i < DP_NUMBER; i++) {
		if (newarray[i] >= low_threadhold)
			z.a[i] = -500;
	}

	gr.SetRanges(-1, 24, -1, 24, 0, 2550);
	gr.Dots(x, y, z, "0.3");
	gr.Box();
	gr.Axis();

	cv::Mat pic1(gr.GetHeight(), gr.GetWidth(), CV_8UC3);
	pic1.data = const_cast<uchar*>(gr.GetRGB());
	cv::flip(pic1, pic1, 0);
	memcpy(pic, pic1.data, gr.GetHeight() * gr.GetWidth() * pic1.channels());
}

void DirectShowCamera::littleToBigFourByte(CString* bData, int _size, int exponent, int fraction, bool _signed, int* pint) {
	int denominator = 1 << fraction;
	int _mask, big;
	CString* parray = new CString[_size];

	if (_signed) {
		_mask = (1 << (exponent + fraction + 1)) - 1;
	}
	else {
		_mask = (1 << (exponent + fraction)) - 1;
	}

	for (int i = 0; i < _size; i += 4) {
		if ((i + 3) < _size) {
			big = _wtoi(bData[i]) + (_wtoi(bData[i + 1]) << 8) + (_wtoi(bData[i + 2]) << 16) + (_wtoi(bData[i + 3]) << SPOT_NUMBER) & _mask;
			*pint = big / denominator;
			pint++;
		}
	}

	if (parray != NULL) {
		delete[] parray;
		parray = NULL;
	}
}

void DirectShowCamera::ExtractBin(int rowIndex, int num) {
	CString strarray[RANGING_MODE_WIDTH + 1];
	unsigned char* ptr = gCameraData[rowIndex];

	// �N ptr ���C�Ӥ����ഫ�� CString
	for (int i = 0; i < (RANGING_MODE_WIDTH + 1); ++i) {
		CString temp;
		temp.Format(_T("%d"), (int)ptr[i]);
		strarray[i] = temp;
	}
	littleToBigFourByte(&strarray[4], DP_NUMBER, 18, 0, false, &histarray[num][0]);
}

void DirectShowCamera::fullframe_process(int rowIndex, int* ppickz) {
	CString strarray[RANGING_MODE_WIDTH + 1];
	unsigned char* ptr = gCameraData[rowIndex];

	// �N ptr ���C�Ӥ����ഫ�� CString
	for (int i = 0; i < (RANGING_MODE_WIDTH + 1); ++i) {
		CString temp;
		temp.Format(_T("%d"), (int)ptr[i]);
		strarray[i] = temp;
	}

	littleToBigFourByte(&strarray[612], 4, 19, 2, false, ppickz);
}

void DirectShowCamera::ParseOneLine() {
	int scnt = 0, pickz_size = 0;
	const int ROWS_PER_BANK = 50;
	const int bank_order[] = { 0, 12, 1, 13, 4, 16, 5, 17, 8, 20, 9, 21, 2, 14, 3, 15, 6, 18, 7, 19, 10, 22, 11, 23 };

	for (int i = 0; i < RANGING_MODE_HEIGHT; i++) {
		if ((i % ROWS_PER_BANK) > 1) {
			ExtractBin(i, scnt);
			scnt++;
		}

		if ((i % ROWS_PER_BANK) >= 2) {
			fullframe_process(i, &peak_z[pickz_size]);
			pickz_size++;
		}
	}

	for (int k = 0; k < SPOT_NUMBER; k++) {
		for (int k1 = 0; k1 < SPOT_NUMBER; k1++) {
			newarray[k1 + SPOT_NUMBER * k] = peak_z[(bank_order[k] * SPOT_NUMBER) + k1];
		}
	}
}

void DirectShowCamera::writeFile(const char* filePath, const unsigned char* data, const int fileCount) {
	
}

void DirectShowCamera::subView(CDC* pDC, uchar* data) {
	CRect dcrect(0, 0, subViewWidth, subViewHeight);
	CDC memDC;
	CBitmap Membitmap;
	memDC.CreateCompatibleDC(pDC);
	Membitmap.CreateCompatibleBitmap(pDC, dcrect.Width(), dcrect.Height());
	CBitmap* pBmpOld = memDC.SelectObject(&Membitmap);

	BITMAPINFO bitInfo;
	bitInfo.bmiHeader.biBitCount = SPOT_NUMBER;
	bitInfo.bmiHeader.biWidth = subViewWidth;
	bitInfo.bmiHeader.biHeight = subViewHeight;
	bitInfo.bmiHeader.biPlanes = 1;
	bitInfo.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	bitInfo.bmiHeader.biCompression = BI_RGB;
	bitInfo.bmiHeader.biClrImportant = 0;
	bitInfo.bmiHeader.biClrUsed = 0;
	bitInfo.bmiHeader.biSizeImage = 0;
	bitInfo.bmiHeader.biXPelsPerMeter = 0;
	bitInfo.bmiHeader.biYPelsPerMeter = 0;

	::StretchDIBits(memDC.GetSafeHdc(), 0, 0,
		subViewWidth, subViewHeight, 0, 0,
		subViewWidth, subViewHeight,
		data, &bitInfo, DIB_RGB_COLORS, SRCCOPY
	);

	pDC->BitBlt(dcrect.left, dcrect.top, dcrect.Width(), dcrect.Height(), &memDC, 0, 0, SRCCOPY);
	memDC.SelectObject(pBmpOld);
	Membitmap.DeleteObject();
	memDC.DeleteDC();
}