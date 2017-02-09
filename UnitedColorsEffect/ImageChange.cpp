#include "opencv2/highgui/highgui.hpp"
#include <opencv2/opencv.hpp>
#include <iostream>
#include <string>

const int EFFECT_SLIDER_MAX = 100;
int effectDegreeProcent;
double effectDegreeMultiplier;

int nChannels;
int nRows;
int nCols;

cv::Mat imgOriginal, imgChanged, imgLoaded;
short** convertingMatrix;

void onTrackbarChanged(int, void*);
void createConvertingMatrix();
void deleteConvertingMatrix();
void createRgbImage();
void saveChangedImage(std::string& filePath);
void openFile(std::string& filePath);
std::string getFilePath();

int main(int argc, const char** argv)
{
	std::string filePath;
	filePath = getFilePath();
	openFile(filePath);
	CV_Assert(imgLoaded.depth() == CV_8U);

	createRgbImage();
	nChannels = imgOriginal.channels();
	nRows = imgOriginal.rows;
	nCols = imgOriginal.cols * nChannels;

	createConvertingMatrix();

	cv::namedWindow("United Colors Effect", CV_WINDOW_AUTOSIZE);

	char TrackbarName[10];
	sprintf(TrackbarName, "Effect");
	cv::createTrackbar(TrackbarName, "United Colors Effect", &effectDegreeProcent, EFFECT_SLIDER_MAX, onTrackbarChanged);

	cv::imshow("United Colors Effect", imgOriginal);

	cv::waitKey(0);
	saveChangedImage(filePath);

	cvvDestroyWindow("United Colors Effect");

	deleteConvertingMatrix();

	return 0;
}



void onTrackbarChanged(int, void*)
{
	uchar* pixelOriginalImg;
	uchar* pixelChangedImg;
	effectDegreeMultiplier = (double)effectDegreeProcent / EFFECT_SLIDER_MAX;
	for (int i = 0; i < nRows; ++i)
	{
		pixelOriginalImg = imgOriginal.ptr<uchar>(i);
		pixelChangedImg = imgChanged.ptr<uchar>(i);
		for (int j = 0; j < nCols; ++j)
		{
			pixelChangedImg[j] = static_cast<int>(pixelOriginalImg[j] + effectDegreeMultiplier*convertingMatrix[i][j]);
		}
	}
	cv::imshow("United Colors Effect", imgChanged);
}



void createConvertingMatrix()
{
	convertingMatrix = new short*[nRows];
	for (int i = 0; i < nRows; i++)
		convertingMatrix[i] = new short[nCols];
	imgChanged = imgOriginal.clone();

	uchar* pixelOriginalImg;
	switch (nChannels) {
	case 3:
		for (int i = 0; i < nRows; ++i)
		{
			pixelOriginalImg = imgOriginal.ptr<uchar>(i);
			for (int j = 0; j < nCols; j += nChannels)
			{
				convertingMatrix[i][j] = -pixelOriginalImg[j]; //B
				convertingMatrix[i][j + 1] = pixelOriginalImg[j + 2] - pixelOriginalImg[j + 1]; //G
				convertingMatrix[i][j + 2] = 255 - pixelOriginalImg[j + 2]; //R
			}
		}
		break;
	case 4:
		for (int i = 0; i < nRows; ++i)
		{
			pixelOriginalImg = imgOriginal.ptr<uchar>(i);
			for (int j = 0; j < nCols; j += nChannels)
			{
				convertingMatrix[i][j] = -pixelOriginalImg[j]; //B
				convertingMatrix[i][j + 1] = pixelOriginalImg[j + 2] - pixelOriginalImg[j + 1]; //G
				convertingMatrix[i][j + 2] = 255 - pixelOriginalImg[j + 2]; //R
				convertingMatrix[i][j + 3] = 0;
			}
		}
	}
}



void deleteConvertingMatrix()
{
	for (int i = 0; i < nRows; ++i)
	{
		delete[] convertingMatrix[i];
	}
	delete[] convertingMatrix;
}


void createRgbImage()
{
	nChannels = imgLoaded.channels();

	if (nChannels == 1)
	{
		nChannels = imgOriginal.channels();
		cv::cvtColor(imgLoaded, imgOriginal, CV_GRAY2RGB);
	}
	else imgOriginal = imgLoaded;
}



void openFile(std::string& filePath)
{
	imgLoaded = cv::imread(filePath, -1);
	while (imgLoaded.empty())
	{
		std::cout << "Error : Image cannot be loaded....." << std::endl;
		filePath = getFilePath();
		imgLoaded = cv::imread(filePath, cv::IMREAD_UNCHANGED);
	}
}



void saveChangedImage(std::string& filePath)
{
	std::string fileName;
	for (int i = filePath.length() - 1; i > 0; i--)
	{
		if (filePath[i] == '.')
		{
			fileName = filePath.substr(0, i);
			break;
		}
	}
	if (nChannels == 4)
	{
		cv::imwrite(fileName + "-1.png", imgChanged);
	}
	else
	{
		cv::imwrite(fileName + "-1.jpg", imgChanged);
	}

}



std::string getFilePath()
{
	std::string filePath;
	std::cout << "Input file path:" << std::endl;
	std::cin >> filePath;
	std::cout << "Modified image will be saved in the same directory." << std::endl;
	int i = 0;
	while (i<filePath.length())
	{
		if (filePath[i] == '\\')
		{
			filePath.insert(i, "\\");
			i++;
		}
		i++;
	}
	return filePath;
}