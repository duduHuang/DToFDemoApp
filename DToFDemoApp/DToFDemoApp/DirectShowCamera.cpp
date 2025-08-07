#include "pch.h"
#include "DirectShowCamera.h"

extern std::queue<std::vector<BYTE>> frameQueue;
std::queue<std::vector<float>> pointCloudQueue;

UINT ShowWindowRealTimeImage(LPVOID pParam) {
	DirectShowCamera* pThis = static_cast<DirectShowCamera*>(pParam);
	pThis->ShowCameraData();
	return 0;
}

template<typename T>
void writeProcess(char* fileName, std::queue<std::vector<T>> dataQueue) {
	std::vector<T> data;
	if (!dataQueue.empty()) {
		data = dataQueue.front();
		dataQueue.pop();
	}
	if (!data.empty()) {
		std::ofstream writeFile(fileName, std::ios::out);
		int count = 0;
		if (DP_NUMBER * 3 == data.size()) {
			for (long j = 0; j + 2 < data.size(); j += 3) {
				writeFile << data.at(j) << " " << data.at(j + 1) << " " << data.at(j + 2) << "\n";
			}
		} else {
			for (long j = 0; j < data.size(); j++) {
				writeFile << (int)data.at(j);
				count++;
				if (RANGING_MODE_WIDTH == count) {
					writeFile << "\n";
					count = 0;
				} else if (j < data.size() - 1) {
					writeFile << ",";
				}
			}
		}
		writeFile << std::endl;
		writeFile.close();
	}
}

UINT WriteFileProcess(LPVOID pParam) {
	WriteFileThreadParams* params = static_cast<WriteFileThreadParams*>(pParam);
	char fileName[MAX_PATH];
	int i = 0;
	while (true) {
		sprintf(fileName, params->filePath + "\\outFile%d.csv", i);	
		writeProcess(fileName, frameQueue);
		sprintf(fileName, params->filePath + "\\pointCloudFile%d.xyz", i++);
		writeProcess(fileName, pointCloudQueue);
		if (params->fileCount == i) {
			break;
		}
		std::cout << "write camera data...\n";
	}
	AfxMessageBox(_T("Finish..."));
	delete params;
	return 0;
}

DirectShowCamera::DirectShowCamera(): m_pThread(nullptr), m_writeFileThread(nullptr),
pLTDC(nullptr), pRTDC(nullptr), pLBDC(nullptr), pRBDC(nullptr),
subViewWidth(0), subViewHeight(0), pointCloudFileCount(0) {
	histarray = new int*[DP_NUMBER];
	for (int i = 0; i < DP_NUMBER; ++i) {
		histarray[i] = new int[HISTO_NUMBER];
		std::fill(histarray[i], histarray[i] + HISTO_NUMBER, 0);
	}
	
	peak_z = new int[DP_NUMBER];
	newarray = new int[DP_NUMBER];
	histpoints = new POINT[DP_NUMBER];
	pointCloud.resize(DP_NUMBER);

	std::fill(peak_z, peak_z + DP_NUMBER, 0);
	std::fill(newarray, newarray + DP_NUMBER, 0);

	isPreview = false;
	isSpeedUp = false;

	x = mglData(DP_NUMBER);
	y = mglData(DP_NUMBER);
	z = mglData(DP_NUMBER);
	pointCloudX = mglData(DP_NUMBER);
	pointCloudY = mglData(DP_NUMBER);
	pointCloudZ = mglData(DP_NUMBER);
}

DirectShowCamera ::~DirectShowCamera() {
	// 停止執行緒
	if (m_pThread) {
		isPreview = false;
		// 等待執行緒結束
		WaitForSingleObject(m_pThread->m_hThread, INFINITE);

		// 刪除執行緒物件
		m_pThread = nullptr;
	}
	if (m_writeFileThread) {
		WaitForSingleObject(m_writeFileThread->m_hThread, INFINITE);
		m_writeFileThread = nullptr;
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
}

void DirectShowCamera::setSubViewWH(int width, int height) {
	subViewWidth = width;
	subViewHeight = height;
	histW = width;
	histH = height;
	cloud3dW = width;
	cloud3dH = height;
}

void DirectShowCamera::setCDC(CDC* pLTDC, CDC* pRTDC, CDC* pLBDC, CDC* pRBDC) {
	this->pLTDC = pLTDC;
	this->pRTDC = pRTDC;
	this->pLBDC = pLBDC;
	this->pRBDC = pRBDC;
}

void DirectShowCamera::run() {
	BaseDirectShowCamera::run();
	// 添加消息循環，保持預覽窗口打開
	isPreview = true;
	if (deviceName == dToFDevice) {
		m_pThread = AfxBeginThread(ShowWindowRealTimeImage, this);
	}
}

void DirectShowCamera::stop() {
	isPreview = false;
	// 停止執行緒.
	if (m_pThread) {
		// 等待執行緒結束
		WaitForSingleObject(m_pThread->m_hThread, INFINITE);

		// 刪除執行緒物件
		m_pThread = nullptr;
	}
	BaseDirectShowCamera::stop();
}

POINT* DirectShowCamera::getPoints() {
	return histpoints;
}

double DirectShowCamera::getRMSE(const std::vector<std::array<float, 3>>& pointCloud) {
	double sum_x = 0, sum_y = 0, sum_z = 0;
	double sum_xx = 0, sum_yy = 0, sum_xy = 0;
	double sum_xz = 0, sum_yz = 0;

	for (const auto& p : pointCloud) {
		double x = p[0], y = p[1], z = p[2];
		sum_x += x;
		sum_y += y;
		sum_z += z;
		sum_xx += x * x;
		sum_yy += y * y;
		sum_xy += x * y;
		sum_xz += x * z;
		sum_yz += y * z;
	}

	double A[3][3] = {
		{ sum_xx, sum_xy, sum_x },
		{ sum_xy, sum_yy, sum_y },
		{ sum_x,  sum_y,  double(DP_NUMBER) }
	};
	double B[3] = { sum_xz, sum_yz, sum_z };

	for (int i = 0; i < 3; ++i) {
		// pivot row
		int maxRow = i;
		for (int k = i + 1; k < 3; ++k)
			if (std::abs(A[k][i]) > std::abs(A[maxRow][i]))
				maxRow = k;

		// 換行
		for (int k = 0; k < 3; ++k) std::swap(A[i][k], A[maxRow][k]);
		std::swap(B[i], B[maxRow]);

		// 消去
		for (int k = i + 1; k < 3; ++k) {
			double factor = A[k][i] / A[i][i];
			for (int j = i; j < 3; ++j) {
				A[k][j] -= A[i][j] * factor;
			}
			B[k] -= B[i] * factor;
		}
	}

	// 回代
	double x[3];
	for (int i = 2; i >= 0; --i) {
		x[i] = B[i];
		for (int j = i + 1; j < 3; ++j) {
			x[i] -= A[i][j] * x[j];
		}
		x[i] /= A[i][i];
	}

	float a = static_cast<float>(x[0]);
	float b = static_cast<float>(x[1]);
	float c = static_cast<float>(x[2]);

	// 計算殘差與 RMSE
	std::vector<float> distances(DP_NUMBER);
	double sum_sq_error = 0.0;

	for (size_t i = 0; i < DP_NUMBER; ++i) {
		float px = pointCloud[i][0];
		float py = pointCloud[i][1];
		float pz = pointCloud[i][2];
		float z_pred = a * px + b * py + c;
		float dz = pz - z_pred;
		distances[i] = std::abs(dz);
		sum_sq_error += dz * dz;
	}

	double rmse = std::sqrt(sum_sq_error / DP_NUMBER);

	return rmse;
}

void DirectShowCamera::ShowCameraData() {
	while (isPreview) {
		if (grabberCallback.getIsFull()) {
			ParseOneLine();
			int cnt = 0;
			for (int i = 0; i < SPOT_NUMBER; i++) {
				for (int j = 0; j < SPOT_NUMBER; j++) {
					z.a[cnt] = newarray[cnt];
					x.a[cnt] = i;
					y.a[cnt] = j;
					pointCloudX.a[i * SPOT_NUMBER + j] = pointCloud[i * SPOT_NUMBER + j][0];
					pointCloudY.a[i * SPOT_NUMBER + j] = pointCloud[i * SPOT_NUMBER + j][1];
					pointCloudZ.a[i * SPOT_NUMBER + j] = pointCloud[i * SPOT_NUMBER + j][2];
					cnt++;
				}
			}
			if (!isSpeedUp) {
				LTSubView();
				RTSubView();
				LBSubView();
			}
			RBSubView();
			grabberCallback.setIsNotFull();
			std::cout << "show camera...\n";
		}
	}
}

void DirectShowCamera::setHistIndex(const int hist, const int width, const int height) {
	histIndex = DP_NUMBER > hist ? hist : -1;
	histW = width;
	histH = height;
}

void DirectShowCamera::setRotate(const int x, const int y, const int width, const int height) {
	rotatx = x;
	rotaty = y;
	cloud3dW = width;
	cloud3dH = height;
}

void DirectShowCamera::setFilterThreshold(int threshold) {
	filterThreshold = threshold;
}

void DirectShowCamera::setMaxValue(int max) {
	maxValue = max;
}

void DirectShowCamera::LTSubView() {
	cv::Mat pic(cloud3dW, cloud3dH, CV_8UC3);
	Cloud3D(cloud3dW, cloud3dH, pic.data, rotatx, rotaty);
	subView(pLTDC, pic.data, cloud3dW, cloud3dH);
}

void DirectShowCamera::RTSubView() {
	cv::Mat pic(histW, histH, CV_8UC3);
	Histgram(histW, histH, pic.data, histIndex);
	subView(pRTDC, pic.data, histW, histH);
}

void DirectShowCamera::LBSubView() {
	cv::Mat pic(subViewWidth, subViewHeight, CV_8UC3);
	Cloud2D(subViewWidth, subViewHeight, pic.data);
	subView(pLBDC, pic.data, subViewWidth, subViewHeight);
}

void DirectShowCamera::RBSubView() {
	cv::Mat pic(subViewWidth, subViewHeight, CV_8UC3);
	Filter2D(subViewWidth, subViewHeight, pic.data, filterThreshold, 0);
	subView(pRBDC, pic.data, subViewWidth, subViewHeight);
}

void DirectShowCamera::Cloud3D(int width, int height, uchar* pic, int rx, int ry) {
	const int valzSize = 9;
	const int gap = maxValue / valzSize;
	mglGraph gr(0, width, height);
	gr.ClearFrame();

	gr.Title("point cloud", "", -1.4);
	gr.Rotate(ry, rx);
	gr.Label('x', "X", 0);
	gr.Label('y', "Y", 0);
	gr.Label('z', "Z", 1);
	gr.SetRanges(-2500, 2500, -2500, 2500, 0, maxValue);
	gr.Dots(pointCloudX, pointCloudY, pointCloudZ, "0.3");

	double valz[valzSize];
	std::ostringstream oss;
	for (int i = 1; i <= valzSize; i++) {
		valz[i] = gap * i;
		oss << valz[i];
		if (i != valzSize) {
			oss << "\\n";  // MathGL 使用 '\n' 做換行
		}
	}
	std::string valzStr = oss.str();
	gr.SetTicksVal('z', mglData(valzSize, valz), valzStr.c_str());

	double valxy[] = { -2500, -1500, -500, 0, 500, 1500, 2500 };
	gr.SetTicksVal('x', mglData(7, valxy), "-2500\n-1500\n-500\n0\n500\n1500\n2500");
	gr.SetTicksVal('y', mglData(7, valxy), "-2500\n-1500\n-500\n0\n500\n1500\n2500");

	gr.SetRange('c', 0, maxValue);
	gr.SetTicksVal('c', mglData(valzSize, valz), valzStr.c_str());
	gr.Colorbar("0123456789", 0.98, -0.3, 0.5, 1.7);

	gr.Axis();
	gr.Grid();

	cv::Mat pic1(gr.GetHeight(), gr.GetWidth(), CV_8UC3);
	pic1.data = const_cast<uchar*>(gr.GetRGB());
	cv::flip(pic1, pic1, 0);
	memcpy(pic, pic1.data, gr.GetHeight() * gr.GetWidth() * pic1.channels());
}

void DirectShowCamera::Histgram(int width, int height, uchar* pic, const int histIndex) {
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

	if (histIndex == -1) {
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
			z.a[i] = histarray[histIndex][i];
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
	const int valzSize = 9;
	const int gap = maxValue / valzSize;
	mglGraph gr(0, width, height);
	gr.ClearFrame();
	double valz[valzSize];
	std::ostringstream oss;
	for (int i = 1; i <= valzSize; i++) {
		valz[i] = gap * i;
		oss << valz[i];
		if (i != valzSize) {
			oss << "\\n";  // MathGL 使用 '\n' 做換行
		}
	}
	std::string valzStr = oss.str();

	gr.Title("2D Image", "", -1.4);
	gr.MultiPlot(17, 17, 18, 13, 14, " ");
	gr.SetRange('c', 0, maxValue);
	gr.SetTicksVal('c', mglData(valzSize, valz), valzStr.c_str());
	gr.Colorbar("0123456789", 0.95, -0.05, 0.5, 1.13);
	gr.Label('x', "X", 0);
	gr.Label('y', "Y", 0);

	gr.SetRanges(-1, 24, -1, 24, 0, maxValue);
	gr.Dots(x, y, z, "0.3");
	gr.Box();
	gr.Axis();

	cv::Mat pic1(gr.GetHeight(), gr.GetWidth(), CV_8UC3);
	pic1.data = const_cast<uchar*>(gr.GetRGB());
	cv::flip(pic1, pic1, 0);
	memcpy(pic, pic1.data, gr.GetHeight() * gr.GetWidth() * pic1.channels());
}

void DirectShowCamera::Filter2D(int width, int height, uchar* pic, int low_threadhold, int hightthreadhold) {
	const int valzSize = 9;
	const int gap = maxValue / valzSize;
	mglGraph gr(0, width, height);
	gr.ClearFrame();
	int i = 0;
	double valz[valzSize];
	std::ostringstream oss;
	for (int i = 1; i <= valzSize; i++) {
		valz[i] = gap * i;
		oss << valz[i];
		if (i != valzSize) {
			oss << "\\n";  // MathGL 使用 '\n' 做換行
		}
	}
	std::string valzStr = oss.str();

	gr.Title("Filter 2D Image", "", -1.4);
	gr.MultiPlot(17, 17, 18, 13, 14, " ");
	gr.SetRange('c', 0, maxValue);
	gr.SetTicksVal('c', mglData(valzSize, valz), valzStr.c_str());
	gr.Colorbar("0123456789", 0.95, -0.05, 0.5, 1.13);
	gr.Label('x', "X", 0);
	gr.Label('y', "Y", 0);

	for (i = 0; i < DP_NUMBER; i++) {
		if (newarray[i] >= low_threadhold)
			z.a[i] = -500;
	}

	gr.SetRanges(-1, 24, -1, 24, 0, maxValue);
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
	unsigned char* ptr = cameraData[rowIndex];

	// 將 ptr 的每個元素轉換為 CString
	for (int i = 0; i < (RANGING_MODE_WIDTH + 1); ++i) {
		CString temp;
		temp.Format(_T("%d"), (int)ptr[i]);
		strarray[i] = temp;
	}
	littleToBigFourByte(&strarray[4], DP_NUMBER, 18, 0, false, &histarray[num][0]);
}

void DirectShowCamera::fullframe_process(int rowIndex, int* ppickz) {
	CString strarray[RANGING_MODE_WIDTH + 1];
	unsigned char* ptr = cameraData[rowIndex];

	// 將 ptr 的每個元素轉換為 CString
	for (int i = 0; i < (RANGING_MODE_WIDTH + 1); ++i) {
		CString temp;
		temp.Format(_T("%d"), (int)ptr[i]);
		strarray[i] = temp;
	}

	littleToBigFourByte(&strarray[612], 4, 19, 2, false, ppickz);
}

void DirectShowCamera::ParseOneLine() {
	int scnt = 0, pickz_size = 0, pointCloudIndex = 0;
	const int ROWS_PER_BANK = 50;
	const int bank_order[] = { 0, 12, 1, 13, 4, 16, 5, 17, 8, 20, 9, 21, 2, 14, 3, 15, 6, 18, 7, 19, 10, 22, 11, 23 };

	for (int i = 0; i < RANGING_MODE_HEIGHT; i++) {
		if (!isSpeedUp) {
			if ((i % ROWS_PER_BANK) > 1) {
				ExtractBin(i, scnt);
				scnt++;
			}
		}

		if ((i % ROWS_PER_BANK) >= 2) {
			fullframe_process(i, &peak_z[pickz_size]);
			pickz_size++;
			pointCloud[pointCloudIndex] = {
				US8littleToS192(&(cameraData[i][604])),
				US8littleToS192(&(cameraData[i][608])),
				US8littleToUS192(&(cameraData[i][612]))
			};
			pointCloudIndex++;
			if (0 != pointCloudFileCount) {
				std::vector<float> flat;
				flat.reserve(DP_NUMBER * 3);
				for (const auto& p : pointCloud) {
					flat.insert(flat.end(), p.begin(), p.end());
				}
				pointCloudQueue.emplace(flat);
				pointCloudFileCount--;
			}
		}
	}

	for (int k = 0; k < SPOT_NUMBER; k++) {
		for (int k1 = 0; k1 < SPOT_NUMBER; k1++) {
			newarray[k1 + SPOT_NUMBER * k] = peak_z[(bank_order[k] * SPOT_NUMBER) + k1];
		}
	}
}

float::DirectShowCamera::US8littleToS192(uint8_t* bData) {
	if (!bData) return 0xFFFFFFFF;
	int32_t i32 = (bData[0] & 0xFF) + (bData[1] << 8) + ((bData[2] & 0x3F) << 16);

	if (i32 > (0x01 << 21) - 1) {
		return (i32 - (0x01 << 22)) / 4.0f;
	}
	else {
		return i32 / 4.0f;
	}
}

float::DirectShowCamera::US8littleToUS192(uint8_t* bData) {
	if (!bData) return 0xFFFFFFFF;

	return ((bData[0] & 0xFF) + (bData[1] << 8) + ((bData[2] & 0x3F) << 16)) / 4.0f;
}

void DirectShowCamera::writeFile(const int fileCount) {
	if (deviceName != dToFDevice) {
		return;
	}
	pointCloudFileCount = fileCount;
	CComPtr<IFileDialog> pFolderDialog;
	hr = CoCreateInstance(CLSID_FileOpenDialog, NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&pFolderDialog));
	if (SUCCEEDED(hr)) {
		DWORD options;
		pFolderDialog->GetOptions(&options);
		pFolderDialog->SetOptions(options | FOS_PICKFOLDERS); // 啟用資料夾選擇模式

		hr = pFolderDialog->Show(nullptr);
		if (SUCCEEDED(hr)) {
			CComPtr<IShellItem> pItem;
			hr = pFolderDialog->GetResult(&pItem);
			if (SUCCEEDED(hr)) {
				PWSTR pszFilePath = nullptr;
				hr = pItem->GetDisplayName(SIGDN_FILESYSPATH, &pszFilePath);
				if (SUCCEEDED(hr)) {
					CString folderPath(pszFilePath);
					AfxMessageBox(_T("選擇的資料夾是：") + folderPath);
					WriteFileThreadParams* params = new WriteFileThreadParams{
						fileCount,
						folderPath
					};
					outFile << "file count:" << fileCount << " file path:" << folderPath << std::endl;
					grabberCallback.setFileCount(fileCount);
					m_writeFileThread = AfxBeginThread(WriteFileProcess, params);
					CoTaskMemFree(pszFilePath);
				}
			}
		}
	}
}

void DirectShowCamera::setSpeedUp() {
	isSpeedUp = true;
}

void DirectShowCamera::setDefaultSpeed() {
	isSpeedUp = false;
}

void DirectShowCamera::sendCx3Command(uint16_t reg, uint8_t data) {
	BaseDirectShowCamera::sendCx3Command(reg, data);
}

ULONG DirectShowCamera::getFWVersion(uint8_t* data) {
	return BaseDirectShowCamera::getFWVersion(data);
}

void DirectShowCamera::subView(CDC* pDC, uchar* data, int width, int height) {
	CRect dcrect(0, 0, width, height);
	CDC memDC;
	CBitmap Membitmap;
	memDC.CreateCompatibleDC(pDC);
	Membitmap.CreateCompatibleBitmap(pDC, dcrect.Width(), dcrect.Height());
	CBitmap* pBmpOld = memDC.SelectObject(&Membitmap);

	BITMAPINFO bitInfo;
	bitInfo.bmiHeader.biBitCount = SPOT_NUMBER;
	bitInfo.bmiHeader.biWidth = width;
	bitInfo.bmiHeader.biHeight = height;
	bitInfo.bmiHeader.biPlanes = 1;
	bitInfo.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	bitInfo.bmiHeader.biCompression = BI_RGB;
	bitInfo.bmiHeader.biClrImportant = 0;
	bitInfo.bmiHeader.biClrUsed = 0;
	bitInfo.bmiHeader.biSizeImage = 0;
	bitInfo.bmiHeader.biXPelsPerMeter = 0;
	bitInfo.bmiHeader.biYPelsPerMeter = 0;

	::StretchDIBits(memDC.GetSafeHdc(), 0, 0,
		width, height, 0, 0,
		width, height,
		data, &bitInfo, DIB_RGB_COLORS, SRCCOPY
	);

	pDC->BitBlt(dcrect.left, dcrect.top, dcrect.Width(), dcrect.Height(), &memDC, 0, 0, SRCCOPY);
	memDC.SelectObject(pBmpOld);
	Membitmap.DeleteObject();
	memDC.DeleteDC();
}