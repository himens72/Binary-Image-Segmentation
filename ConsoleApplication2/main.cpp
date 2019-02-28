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
int *source;// = (int *)malloc(entries * 2 * sizeof(int));
int *sink;
int *weightedMatrix;
int sourceCount = 0;
int sinkCount = 0;
int entries;
//vector<int> source;
//vector<int> sink;
queue<vector<pair<int, int>>> sourceQueue;
//int check(int height, int width, int weight, vector < pair<int, int>> cpath) {
int check(int heightIndex, int widthIndex, vector < pair<int, int>> cpath) {
	int size = cpath.size();
	for (int i = 0; i < size; i++) {
		//cout << " Weight  : " << weight << " Current Weight" << *(weightedMatrix + cpath[i].first * height + cpath[i].second * width + j) << endl;
		if (heightIndex == cpath[i].first && widthIndex == cpath[i].second)
			return 0;
	}
	return 1;
}
int check2(int srcWidth, int srcHeight) {
	for (int i = 0; i < sinkCount; i++) {
		if (*(sink + i * entries + 1) == srcWidth && *(sink + i * entries + 0) == srcHeight)
			return 1;
	}
	return 0;
}

void generateSourceQueue() {
	for (int i = 0; i < sourceCount; i++) {
		vector<pair<int, int>> temp;
		temp.push_back(make_pair(*(source + sourceCount * entries + 0), *(source + sourceCount * entries + 1)));
		sourceQueue.push(temp);
	}
}
Mat readInputImage(/*char* inputFile*/) {
	Mat inputImage = imread("simple.png");
	return  inputImage;
}
void readConfigFile(/*char* inputFile,*/ int width, int height) {
	ifstream configFile("config.txt");
	configFile >> entries;
	source = (int *)malloc(entries * 2 * sizeof(int));
	sink = (int *)malloc(entries * 2 * sizeof(int));

	for (int i = 0; i < entries; i++) {
		int w, h, marker;
		configFile >> h >> w >> marker;
		if (marker == 1) {
			*(source + sourceCount * entries + 0) = h - 1;
			*(source + sourceCount * entries + 1) = w - 1;
			//cout << "Source Width" << *(source + sourceCount * entries + 1) << " Height " << *(source + sourceCount * entries + 0) << endl;
			sourceCount++;
			vector<pair<int, int>> temp;
			temp.push_back(make_pair(h, w));
			sourceQueue.push(temp);
		}
		else {
			*(sink + sinkCount * entries + 0) = h - 1;
			*(sink + sinkCount * entries + 1) = w - 1;
			cout << "Sink Width" << *(sink + sinkCount * entries + 1) << " Height " << *(sink + sinkCount * entries + 0) << endl;
			sinkCount++;
		}
	}
	/*for (int i = 0; i < entries; i++) {
		int w, h, marker;
		configFile >> w >> h >> marker;
		if (marker == 1) {
			source.push_back((width * w) + h);
			sourceQueue.push((width * w) + h);
			//cout << i << " " << (width * w) + h << endl;
		}
		else {
			sink.push_back((width * w) + h);
			cout << i << " " << (width * w) + h << endl;
		}
	}*/
	cout << "Source Queue Size : " << sourceQueue.size() << endl;
}
int main(int argc, char** argv) {
	/*
	if (argc != 4) {
		//cout << "Invalid Argument" << endl;
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
	//cout << width << endl;
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
	OUTPUT_FILE << "Maximum Intensity : " << maxIntensity << endl;
	//cout << height * width << endl;
	weightedMatrix = (int *)malloc(height* width * 4 * sizeof(int));
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
				weightTop = maxIntensity - ((grad.at<uchar>(i, j) + grad.at<uchar>(i - 1, j)) / 2);
				count++;
				total += weightTop;
			}
			else {
				//OUTPUT_FILE << " Top " << i << " " << j << endl;
			}
			if (j + 1 < width) {
				weightRight = maxIntensity - ((grad.at<uchar>(i, j) + grad.at<uchar>(i, j + 1)) / 2);
				count++;
				total += weightRight;
			}
			else {
				//OUTPUT_FILE << " Right " << i << " " << j << endl;
			}
			if (i + 1 < height) {
				weightDown = maxIntensity - ((grad.at<uchar>(i, j) + grad.at<uchar>(i + 1, j)) / 2);
				count++;
				total += weightDown;
			}
			else {
				//OUTPUT_FILE << " Down " << i << " " << j << endl;
			}
			if (j - 1 != -1) {
				weightLeft = maxIntensity - ((grad.at<uchar>(i, j) + grad.at<uchar>(i, j - 1)) / 2);
				count++;
				total += weightLeft;
			}
			else {
				//	OUTPUT_FILE << " Left " << i << " " << j << endl;
			}
			OUTPUT_FILE << "Weight  Top  " << weightTop << " Right " << weightRight << " Down " << weightDown << " Left " << weightLeft << endl;
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
	int *tempMatrix = (int *)malloc(height* width * sizeof(int));
	for (int i = 0; i < height; i++) {
		for (int j = 0; j < width; j++) {
					*(tempMatrix + i * height + j) = 0;
		}
	}
	//generateSourceQueue();
	bool flag = true;
	vector <pair<int, int>> path;
	while (flag) {
		flag = false;
		while (!sourceQueue.empty()) {
			//cout << "dasdsadasd " << endl;
			again: vector <pair<int, int>> cpath;
			cpath = sourceQueue.front();
			//cout << " Cpath Size " << cpath.size() << " Queue Size " << sourceQueue.size() << endl;
			sourceQueue.pop();
			//cout << " Cpath Size " << cpath.size() << " Source Queue" << sourceQueue.size() << endl;
			pair<int, int> last = cpath.back();
			int srcHeight = last.first;
			int srcWidth = last.second;
			if (check2(srcWidth, srcHeight) == 1) {
				flag = true;
				path.clear();
				path = cpath;
				for (int i = 0; i < height; i++) {
					for (int j = 0; j < width; j++) {		
						*(tempMatrix + i * height + j ) = 0;			
					}
				}
				while (!sourceQueue.empty()) {
					sourceQueue.pop();
				}
				generateSourceQueue();
				break;
			}
			if (*(weightedMatrix + last.first * height + last.second * width + 0) > -1) {
				int tempHeight = last.first - 1;
				int tempWidth = last.second;
				if (check(tempHeight, tempWidth, cpath) && *(weightedMatrix + last.first * height + last.second * width + 0) > 0 && *(tempMatrix + tempHeight * height + tempWidth) == 0) {
					//cout << "->last Height " << tempHeight << "last Width " << tempWidth << endl;
					vector < pair<int, int> > opath(cpath);
					//opath.push_back(adlis[last][i].first);
					opath.push_back(make_pair(tempHeight, tempWidth));
					*(tempMatrix + tempHeight * height + tempWidth) = 1;
					sourceQueue.push(opath);
					//cout << "source queue size : " << sourceQueue.size() << endl;
				}
			} 
			if (*(weightedMatrix + last.first * height + last.second * width + 1) > -1) {
				int tempHeight = last.first;
				int tempWidth = last.second + 1;
				if (check(tempHeight, tempWidth, cpath) && *(weightedMatrix + last.first * height + last.second * width + 1) > 0 && *(tempMatrix + tempHeight * height + tempWidth) == 0) {
					//cout << "->last Height " << tempHeight << "last Width " << tempWidth << endl;
					vector < pair<int, int> > opath(cpath);
					//opath.push_back(adlis[last][i].first);
					opath.push_back(make_pair(tempHeight, tempWidth));
					*(tempMatrix + tempHeight * height + tempWidth) = 1;
					sourceQueue.push(opath);
					//cout << "source queue size : " << sourceQueue.size() << endl;
				}
			} 
			if (*(weightedMatrix + last.first * height + last.second * width + 2) > -1) {
				int tempHeight = last.first +1 ;
				int tempWidth = last.second;
				if (check(tempHeight, tempWidth, cpath) && *(weightedMatrix + last.first * height + last.second * width + 2) > 0 && *(tempMatrix + tempHeight * height + tempWidth) == 0) {
					//cout << "->last Height " << tempHeight << "last Width " << tempWidth << endl;
					vector < pair<int, int> > opath(cpath);
					//opath.push_back(adlis[last][i].first);
					opath.push_back(make_pair(tempHeight, tempWidth));
					*(tempMatrix + tempHeight * height + tempWidth) = 1;
					sourceQueue.push(opath);
					///cout << "source queue size : " << sourceQueue.size() << endl;
				}
			}
			if (*(weightedMatrix + last.first * height + last.second * width + 3) > -1) {
				int tempHeight = last.first;
				int tempWidth = last.second - 1;
				if (check(tempHeight, tempWidth, cpath) && *(weightedMatrix + last.first * height + last.second * width + 3) > 0 && *(tempMatrix + tempHeight * height + tempWidth) == 0) {
					//cout << "->last Height " << tempHeight << "last Width " << tempWidth << endl;
					vector < pair<int, int> > opath(cpath);
					//opath.push_back(adlis[last][i].first);
					opath.push_back(make_pair(tempHeight, tempWidth));
					*(tempMatrix + tempHeight * height + tempWidth) = 1;
					sourceQueue.push(opath);
					//cout << "source queue size : " << sourceQueue.size() << endl;
				}
			}
		}
		if (flag) {
			int smallest = 99999999;
			for (int i = 0; i < path.size() - 1; i++) {
				int tempWidth = 0;
				int tempHeight = 0;
				if (*(weightedMatrix + path[i].first * height + path[i].second * width + 0) > -1) {
					tempHeight = path[i].first - 1;
					tempWidth = path[i].second;
					if (tempHeight == path[i + 1].first && tempWidth == path[i + 1].second && *(weightedMatrix + path[i + 1].first * height + path[i + 1].second * width + 0) < smallest) {
						smallest = *(weightedMatrix + path[i].first * height + path[i].second * width + 0);
					}

				}
				if (*(weightedMatrix + path[i].first * height + path[i].second * width + 1) > -1) {
					tempHeight = path[i].first;
					tempWidth = path[i].second + 1;
					if (tempHeight == path[i + 1].first && tempWidth == path[i + 1].second && *(weightedMatrix + path[i + 1].first * height + path[i + 1].second * width + 1) < smallest) {
						smallest = *(weightedMatrix + path[i].first * height + path[i].second * width + 1);
					}
				}
				if (*(weightedMatrix + path[i].first * height + path[i].second * width + 2) > -1) {
					tempHeight = path[i].first + 1;
					tempWidth = path[i].second;
					if (tempHeight == path[i + 1].first && tempWidth == path[i + 1].second && *(weightedMatrix + path[i + 1].first * height + path[i + 1].second * width + 2) < smallest) {
						smallest = *(weightedMatrix + path[i].first * height + path[i].second * width + 2);
					}
				}
				if (*(weightedMatrix + path[i].first * height + path[i].second * width + 3) > -1) {
					tempHeight = path[i].first;
					tempWidth = path[i].second - 1;
					if (tempHeight == path[i + 1].first && tempWidth == path[i + 1].second && *(weightedMatrix + path[i + 1].first * height + path[i + 1].second * width + 3) < smallest) {
						smallest = *(weightedMatrix + path[i].first * height + path[i].second * width + 3);
					}
				}
			}
			for (int i = 0; i < path.size() - 1; i++) {
				int tempWidth = 0;
				int tempHeight = 0;
				if (*(weightedMatrix + path[i].first * height + path[i].second * width + 0) > -1) {
					tempHeight = path[i].first - 1;
					tempWidth = path[i].second;
					if (tempHeight == path[i + 1].first && tempWidth == path[i + 1].second) {
						*(weightedMatrix + path[i].first * height + path[i].second * width + 0) -= smallest;
					}
				}
				if (*(weightedMatrix + path[i].first * height + path[i].second * width + 1) > -1) {
					tempHeight = path[i].first;
					tempWidth = path[i].second + 1;
					if (tempHeight == path[i + 1].first && tempWidth == path[i + 1].second) {
						*(weightedMatrix + path[i].first * height + path[i].second * width + 1) -= smallest;
					}
				}
				if (*(weightedMatrix + path[i].first * height + path[i].second * width + 2) > -1) {
					tempHeight = path[i].first +1;
					tempWidth = path[i].second;
					if (tempHeight == path[i + 1].first && tempWidth == path[i + 1].second) {
						*(weightedMatrix + path[i].first * height + path[i].second * width + 2) -= smallest;
					}
				}
				if (*(weightedMatrix + path[i].first * height + path[i].second * width + 3) > -1) {
					tempHeight = path[i].first ;
					tempWidth = path[i].second - 1;
					if (tempHeight == path[i + 1].first && tempWidth == path[i + 1].second) {
						*(weightedMatrix + path[i].first * height + path[i].second * width + 3) -= smallest;
					}
				}
			}
			for (int i = 0; i < path.size() - 1; i++) {
				int tempWidth = 0;
				int tempHeight = 0;
				if (*(weightedMatrix + path[i].first * height + path[i].second * width + 0) > -1) {
					tempHeight = path[i].first - 1;
					tempWidth = path[i].second;
					if (tempHeight == path[i].first && tempWidth == path[i].second) {
						*(weightedMatrix + path[i + 1].first * height + path[i + 1].second * width + 0) += smallest;
					}
				}
				if (*(weightedMatrix + path[i].first * height + path[i].second * width + 1) > -1) {
					tempHeight = path[i].first;
					tempWidth = path[i].second + 1;
					if (tempHeight == path[i].first && tempWidth == path[i].second) {
						*(weightedMatrix + path[i + 1].first * height + path[i + 1].second * width + 1) += smallest;
					}
				}
				if (*(weightedMatrix + path[i].first * height + path[i].second * width + 2) > -1) {
					tempHeight = path[i].first + 1;
					tempWidth = path[i].second;
					if (tempHeight == path[i].first && tempWidth == path[i].second) {
						*(weightedMatrix + path[i + 1].first * height + path[i + 1].second * width + 2) += smallest;
					}
				}
				if (*(weightedMatrix + path[i].first * height + path[i].second * width + 3) > -1) {
					tempHeight = path[i].first;
					tempWidth = path[i].second - 1;
					if (tempHeight == path[i].first && tempWidth == path[i].second) {
						*(weightedMatrix + path[i + 1].first * height + path[i + 1].second * width + 3) += smallest;
					}
				}
			}

		}
	}

	Mat out_image = inputImage.clone();

	for (int i = 0; i < height; i++) {
		for (int j = 0; j < width; j++) {
			Vec3b pixel;
			pixel[0] = 0;
			pixel[1] = 0;
			pixel[2] = 0;
			out_image.at<Vec3b>(i, j) = pixel;
		}
	}
	int *tempMatrix1 = (int *)malloc(height* width * sizeof(int));
	for (int i = 0; i < height; i++) {
		for (int j = 0; j < width; j++) {
				*(tempMatrix1 + i * height + j ) = 0;

		}
	}
	Vec3b pixel2;
	pixel2[0] = 0;
	pixel2[1] = 0;
	pixel2[2] = 255;

	queue <pair<int, int> > qcolor;
	pair<int, int> u;
	u.first = *(source + 0 * entries + 0);
	u.second = *(source + 0 * entries + 1);
	qcolor.push(u);
	out_image.at < Vec3b >(u.first, u.second) = pixel2;
	int mino = 99999999;

	//performing BFS
	while (!qcolor.empty()) {
		//cout << "Qcolor Size : " << qcolor.size() << endl;
		pair<int, int> last = qcolor.front();
		qcolor.pop();
		int tempWidth = 0;
		int tempHeight = 0;
		if (*(weightedMatrix + last.first * height + last.second * width + 0) > -1) {
			tempHeight = last.first - 1;
			tempWidth = last.second;
			if (*(tempMatrix1 + tempHeight * height + tempWidth) == 0 && *(weightedMatrix + last.first * height + last.second * width + 0) > 0) {
				Vec3b pixel2;
				pixel2[0] = 255;
				pixel2[1] = 255;
				pixel2[2] = 225;
				int x3, x4;
				x3 = tempHeight;
				x4 = tempWidth;
				out_image.at < Vec3b >(x3, x4) = pixel2;
				qcolor.push(make_pair(tempHeight, tempWidth));
				*(tempMatrix1 + tempHeight * height + tempWidth) = 1;

			}
		}
		if (*(weightedMatrix + last.first * height + last.second * width + 1) > -1) {
			tempHeight = last.first;
			tempWidth = last.second + 1;
			if (*(tempMatrix1 + tempHeight * height + tempWidth) == 0 && *(weightedMatrix + last.first * height + last.second * width + 1) > 0) {
				Vec3b pixel2;
				pixel2[0] = 255;
				pixel2[1] = 255;
				pixel2[2] = 225;
				int x3, x4;
				x3 = tempHeight;
				x4 = tempWidth;
				out_image.at < Vec3b >(x3, x4) = pixel2;
				qcolor.push(make_pair(tempHeight, tempWidth));
				*(tempMatrix1 + tempHeight * height + tempWidth) = 1;

			}

		}
		if (*(weightedMatrix + last.first * height + last.second * width + 2) > -1) {
			tempHeight = last.first + 1;
			tempWidth = last.second;
			if (*(tempMatrix1 + tempHeight * height + tempWidth) == 0 && *(weightedMatrix + last.first * height + last.second * width + 2) > 0) {
				Vec3b pixel2;
				pixel2[0] = 255;
				pixel2[1] = 255;
				pixel2[2] = 225;
				int x3, x4;
				x3 = tempHeight;
				x4 = tempWidth;
				out_image.at < Vec3b >(x3, x4) = pixel2;
				qcolor.push(make_pair(tempHeight, tempWidth));
				*(tempMatrix1 + tempHeight * height + tempWidth) = 1;

			}
		}
		if (*(weightedMatrix + last.first * height + last.second * width + 3) > -1) {
			tempHeight = last.first;
			tempWidth = last.second - 1;
			if (*(tempMatrix1 + tempHeight * height + tempWidth) == 0 && *(weightedMatrix + last.first * height + last.second * width + 3) > 0) {
				Vec3b pixel2;
				pixel2[0] = 255;
				pixel2[1] = 255;
				pixel2[2] = 225;
				int x3, x4;
				x3 = tempHeight;
				x4 = tempWidth;
				out_image.at < Vec3b >(x3, x4) = pixel2;
				qcolor.push(make_pair(tempHeight, tempWidth));
				*(tempMatrix1 + tempHeight * height + tempWidth) = 1;

			}
		}

	}
	imwrite("adasdsadsdas.png", out_image);

	imwrite("a1.png", sourceImage);
	imwrite("a2.png", grayImage);
	imwrite("grad_x.png", grad_x);
	imwrite("grad_y.png", grad_y);
	imwrite("abs_grad_x.png", abs_grad_x);
	imwrite("abs_grad_y.png", abs_grad_y);
	imwrite("grad.png", grad);
	return 0;
}