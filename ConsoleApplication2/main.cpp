/**
  * https://docs.opencv.org/2.4/doc/tutorials/imgproc/imgtrans/sobel_derivatives/sobel_derivatives.html#explanation
  *
  *
  *
  */

#include "pch.h"
#include <opencv2/opencv.hpp>
#include<fstream>
#include <utility>
#include<queue>
#include<vector>
#define INT_MAX 2147483647
#define INT_MIN -2147483648

using namespace cv;
using namespace std;

vector<int> source;
vector<int> sink;
queue<int> sourceQueue;
Mat readInputImage(/*char* inputFile*/) {
	Mat inputImage = imread("simple.png");
	return  inputImage;
}
void readConfigFile(/*char* inputFile,*/ int width, int height) {
	ifstream configFile("config.txt");
	int entries;
	configFile >> entries;
	for (int i = 0; i < entries; i++) {
		int w, h, marker;
		configFile >> w >> h >> marker;
		if (marker == 1) {
			source.push_back((width * w) + h);
			sourceQueue.push((width * w) + h);
			cout << i << " " << (width * w) + h << endl;
		}
		else {
			sink.push_back((width * w) + h);
			cout << i << " " << (width * w) + h << endl;
		}
	}
	cout << "Source Queue Size : " << sourceQueue.size() << endl;
}
int main(int argc, char** argv) {
	/*
	if (argc != 4) {
		cout << "Invalid Argument" << endl;
		return -1;
	}
	*/
	Mat inputImage = readInputImage(/*argv[1]*/);
	if (!inputImage.data) {
		cout << "Could not load input image!!!" << endl;
		return -1;
	}
	if (inputImage.channels() != 3) {
		cout << "Image does not have 3 channels!!! " << inputImage.depth() << endl;
		return -1;
	}
	int width = inputImage.cols;
	int height = inputImage.rows;
	readConfigFile(/*argv[2],*/ width, height);
	Mat sourceImage = inputImage.clone();
	Mat grayImage;
	GaussianBlur(sourceImage, sourceImage, Size(3, 3), 0, 0, BORDER_DEFAULT);
	cvtColor(sourceImage, grayImage, COLOR_BGR2GRAY/*CV_BGR2GRAY*/);
	cout << width << endl;
	cout << height << endl;
	int scale = 1;
	int delta = 0;
	Mat grad_x, grad_y;
	Mat abs_grad_x, abs_grad_y;
	Mat grad;
	Sobel(grayImage, grad_x, CV_16S, 1, 0, 3, scale, delta, BORDER_DEFAULT);
	Sobel(grayImage, grad_y, CV_16S, 0, 1, 3, scale, delta, BORDER_DEFAULT);
	convertScaleAbs(grad_x, abs_grad_x);
	convertScaleAbs(grad_y, abs_grad_y);
	addWeighted(abs_grad_x, 0.5, abs_grad_y, 0.5, 0, grad);
	int maxIntensity = 0; // maximum intensity of pixel
	/*
	* This for loop identifies maximum intensity among pixel from GrayScale Image
	*/
	for (int i = 0; i < height; i++) {
		for (int j = 0; j < width; j++) {
			if (maxIntensity < grayImage.at<uchar>(i, j)) {
				maxIntensity = grayImage.at<uchar>(i, j);
			}
		}
	}
	ofstream OUTPUT_FILE("weight.txt");
	cout << "Max Intensity : " << maxIntensity << endl;
	cout << height * width << endl;
	int *weightedMatrix = (int *)malloc(height* width * 4 * sizeof(int));
	int count = 0;
	int total = 0;
	for (int i = 0; i < height; i++) {
		for (int j = 0; j < width; j++) {
			count++;
			int weightTop = -1;
			int weightRight = -1;
			int weightDown = -1;
			int weightLeft = -1;
			if (i - 1 != -1) {
				weightTop = maxIntensity - ((grad.at<uchar>(i, j) - grad.at<uchar>(i - 1, j)) / 2);
				count++;
				total += weightTop;
			}
			else {
				OUTPUT_FILE << " Top " << i << " " << j << endl;
			}
			if (j + 1 < width) {
				weightRight = maxIntensity - ((grad.at<uchar>(i, j) - grad.at<uchar>(i, j + 1)) / 2);
				count++;
				total += weightRight;
			}
			else {
				OUTPUT_FILE << " Right " << i << " " << j << endl;
			}
			if (i + 1 < height) {
				weightDown = maxIntensity - ((grad.at<uchar>(i, j) - grad.at<uchar>(i + 1, j)) / 2);
				count++;
				total += weightDown;
			}
			else {
				OUTPUT_FILE << " Down " << i << " " << j << endl;
			}
			if (j - 1 != -1) {
				weightLeft = maxIntensity - ((grad.at<uchar>(i, j) - grad.at<uchar>(i, j - 1)) / 2);
				count++;
				total += weightLeft;
			}
			else {
				OUTPUT_FILE << " Left " << i << " " << j << endl;
			}
			*(weightedMatrix + i * height + j * width + 0) = weightTop;
			*(weightedMatrix + i * height + j * width + 1) = weightRight;
			*(weightedMatrix + i * height + j * width + 2) = weightDown;
			*(weightedMatrix + i * height + j * width + 3) = weightLeft;
		}
	}
	cout << " Total : " << total << endl;
	OUTPUT_FILE.close();
	cout << "Count : " << count << endl;
	int averagePixel = total / count;
	cout << "averagePixel : " << averagePixel << endl;
	for (int i = 0; i < height; i++) {
		for (int j = 0; j < width; j++) {
			for (int k = 0; k < 4; k++) {
				if (*(weightedMatrix + i * height + j * width + k) < averagePixel && *(weightedMatrix + i * height + j * width + k) != -1) {
					*(weightedMatrix + i * height + j * width + k) = 1;
				}
				else if (*(weightedMatrix + i * height + j * width + k) != -1) {
					*(weightedMatrix + i * height + j * width + k) = 1000000;
				}
			}
		}
	}
	imwrite("a1.png", sourceImage);
	imwrite("a2.png", grayImage);
	imwrite("grad_x.png", grad_x);
	imwrite("grad_y.png", grad_y);
	imwrite("abs_grad_x.png", abs_grad_x);
	imwrite("abs_grad_y.png", abs_grad_y);
	imwrite("grad.png", grad);
	return 0;
}