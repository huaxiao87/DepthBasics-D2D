//Disclaimer:
//This is based on preliminary software and/or hardware, subject to change.

#include <iostream>
#include <sstream>

#include <Windows.h>
#include <Kinect.h>

#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>

inline void CHECKERROR(HRESULT n) {
	if (!SUCCEEDED(n)) {
		std::stringstream ss;
		ss << "ERROR " << std::hex << n << std::endl;
		std::cin.ignore();
		std::cin.get();
		throw std::runtime_error(ss.str().c_str());
	}
}

// Safe release for interfaces
template<class Interface>
inline void SAFERELEASE(Interface *& pInterfaceToRelease) {
	if (pInterfaceToRelease != nullptr) {
		pInterfaceToRelease->Release();
		pInterfaceToRelease = nullptr;
	}
}

IColorFrameReader* colorFrameReader = nullptr; // color reader

void processIncomingData() {
	IColorFrame *data = nullptr;
	IFrameDescription *frameDesc = nullptr;
	HRESULT hr = -1;
	RGBQUAD *colorBuffer = nullptr;

	hr = colorFrameReader->AcquireLatestFrame(&data);
	if (SUCCEEDED(hr)) hr = data->get_FrameDescription(&frameDesc);
	if (SUCCEEDED(hr)) {
		int height = 0, width = 0;
		if (SUCCEEDED(frameDesc->get_Height(&height)) &&
			SUCCEEDED(frameDesc->get_Width(&width))) {
			colorBuffer = new RGBQUAD[height * width];
			hr = data->CopyConvertedFrameDataToArray(height * width * sizeof(RGBQUAD),
				reinterpret_cast<BYTE*>(colorBuffer), ColorImageFormat_Bgra);
			if (SUCCEEDED(hr)) {
				cv::Mat img1(height, width, CV_8UC4,
					reinterpret_cast<void*>(colorBuffer));
				cv::imshow("Color Only", img1);
			}
		}
	}
	if (colorBuffer != nullptr) {
		delete[] colorBuffer;
		colorBuffer = nullptr;
	}
	SAFERELEASE(data);
}

int main(int argc, char** argv) {
	HRESULT hr;
	IKinectSensor* kinectSensor = nullptr;     // kinect sensor

											   // initialize Kinect Sensor
	hr = GetDefaultKinectSensor(&kinectSensor);
	if (FAILED(hr) || !kinectSensor) {
		std::cout << "ERROR hr=" << hr << "; sensor=" << kinectSensor << std::endl;
		return -1;
	}
	CHECKERROR(kinectSensor->Open());

	// initialize color frame reader
	IColorFrameSource* colorFrameSource = nullptr; // color source
	CHECKERROR(kinectSensor->get_ColorFrameSource(&colorFrameSource));
	CHECKERROR(colorFrameSource->OpenReader(&colorFrameReader));
	SAFERELEASE(colorFrameSource);

	while (colorFrameReader) {
		processIncomingData();
		int key = cv::waitKey(10);
		if (key == 'q') {
			break;
		}
	}

	// de-initialize Kinect Sensor
	CHECKERROR(kinectSensor->Close());
	SAFERELEASE(kinectSensor);
	return 0;
}