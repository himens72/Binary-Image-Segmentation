/**
  * https://docs.opencv.org/2.4/doc/tutorials/imgproc/imgtrans/sobel_derivatives/sobel_derivatives.html#explanation
  *
  *
  *
  */
#include "pch.h"
#include <opencv2/opencv.hpp>
#include <fstream>
#include <utility>
#include <queue>
#include <vector>


using namespace cv;
using namespace std;
int width;
int height;
vector < vector < pair < int, int > > > graphMatrix;
int *source;// = (int *)malloc(entries * 2 * sizeof(int));
int *sink;
int sourceCount = 0;
int sinkCount = 0;
int entries;
queue < vector < int > > sourceQueue;
// checking is the pixel already present in path or not
int check(int x, vector < int > cpath) {
	int size = cpath.size();
	for (int i = 0; i < size; i++)
		if (cpath[i] == x)
			return 0;
	return 1;
}
int check2(int la) {
	int x1 = la;
	for (int i = 0; i < sinkCount; i++) {
		if ((width * sink[1]) + sink[0] == x1)
			return 1;
	}
	return 0;
}
Mat readInputImage(/*char* inputFile*/) {
	Mat inputImage = imread("simple.png");
	return  inputImage;
}
void readConfigFile(/*char* inputFile*/) {
	ifstream configFile("config.txt");
	configFile >> entries;
	source = (int *)malloc(entries * 2 * sizeof(int));
	sink = (int *)malloc(entries * 2 * sizeof(int));

	for (int i = 0; i < entries; i++) {
		int w, h, marker;
		configFile >> h >> w >> marker;
		if (marker == 1) {
			*(source + sourceCount * entries + 0) = h;
			*(source + sourceCount * entries + 1) = w;
			//cout << "Source Width" << *(source + sourceCount * entries + 1) << " Height " << *(source + sourceCount * entries + 0) << endl;
			sourceCount++;
			vector <int> temp;
			temp.push_back((width * w) + h);
			sourceQueue.push(temp);
		}
		else {
			*(sink + sinkCount * entries + 0) = h;
			*(sink + sinkCount * entries + 1) = w;
			cout << "Sink Width" << *(sink + sinkCount * entries + 1) << " Height " << *(sink + sinkCount * entries + 0) << endl;
			sinkCount++;
		}
	}
}
int main(int argc, char * * argv) {
	/*
	if (argc != 4) {
		cout << "Usage: ../seg input_image initialization_file output_mask" << endl;
		return -1;
	}
	*/
	
	Mat inputImage = readInputImage(/*argv[1]*/);
	if (!inputImage.data || inputImage.channels() != 3) {
		cout << "Unable to Load Image or  3 channels doesn't exist!!!" << endl;
		return -1;
	}
	readConfigFile(/*argv[2]*/);
	width = inputImage.cols;
	height = inputImage.rows;
	// the output image
	Mat sourceImage = inputImage.clone();
	Mat outputImage;
	Mat grayImage;
	GaussianBlur(sourceImage, sourceImage, Size(3, 3), 0, 0, BORDER_DEFAULT);
	cvtColor(sourceImage, grayImage, COLOR_BGR2GRAY/*CV_BGR2GRAY*/);
	//cout << width << endl;
	cout << height << endl;
	int scale = 1;
	int delta = 0;
	Mat grad_x, grad_y;
	Mat abs_grad_x, abs_grad_y;
	Mat gradientImage;
	Sobel(grayImage, grad_x, CV_16S, 1, 0, 3, scale, delta, BORDER_DEFAULT);
	Sobel(grayImage, grad_y, CV_16S, 0, 1, 3, scale, delta, BORDER_DEFAULT);
	convertScaleAbs(grad_x, abs_grad_x);
	convertScaleAbs(grad_y, abs_grad_y);
	addWeighted(abs_grad_x, 0.5, abs_grad_y, 0.5, 0, gradientImage);
	int maxIntensity = 0; // maximum intensity of pixel
	// getting max energy of pixel in image
	int maxIntensity = 0;
	for (int i = 0; i < height; i++) {
		for (int j = 0; j < width; j++) {
			if (maxIntensity < grayImage.at<uchar>(i, j)) {
				maxIntensity = grayImage.at<uchar>(i, j);
			}
		}
	}
	graphMatrix.resize(height * width);
	//calculate weights by taking pixel and pixel toward its left,right,down,up
	for (int id = 0; id < height * width; id++) {
		int i = id / width;
		int j = id % width;
		if (id == 0) {
			int weightssd = maxIntensity - ((gradientImage.at < uchar >(i, j) + gradientImage.at < uchar >(i + 1, j)) / 2);
			int weightssr = maxIntensity - ((gradientImage.at < uchar >(i, j) + gradientImage.at < uchar >(i, j + 1)) / 2);
			pair < int, int > npair1 = make_pair((i)* width + j + 1, weightssr);
			graphMatrix[id].push_back(npair1);
			pair < int, int > npair = make_pair((i + 1) * width + j, weightssd);
			graphMatrix[id].push_back(npair);
		}
		else if (id == width - 1) {
			int weightssd = maxIntensity - ((gradientImage.at < uchar >(i, j) + gradientImage.at < uchar >(i + 1, j)) / 2);
			int weightssl = maxIntensity - ((gradientImage.at < uchar >(i, j) + gradientImage.at < uchar >(i, j - 1)) / 2);
			pair < int, int > npair = make_pair((i + 1) * width + j, weightssd);
			graphMatrix[id].push_back(npair);
			pair < int, int > npair1 = make_pair((i)* width + j - 1, weightssl);
			graphMatrix[id].push_back(npair1);
		}
		else if (id == width * (height - 1)) {
			int weightssu = maxIntensity - ((gradientImage.at < uchar >(i, j) + gradientImage.at < uchar >(i - 1, j)) / 2);
			int weightssr = maxIntensity - ((gradientImage.at < uchar >(i, j) + gradientImage.at < uchar >(i, j + 1)) / 2);
			pair < int, int > npair = make_pair((i - 1) * width + j, weightssu);
			graphMatrix[id].push_back(npair);
			pair < int, int > npair1 = make_pair((i)* width + j + 1, weightssr);
			graphMatrix[id].push_back(npair1);
		}
		else if (id == height * width - 1) {
			int weightssu = maxIntensity - ((gradientImage.at < uchar >(i, j) + gradientImage.at < uchar >(i - 1, j)) / 2);
			int weightssl = maxIntensity - ((gradientImage.at < uchar >(i, j) + gradientImage.at < uchar >(i, j - 1)) / 2);
			pair < int, int > npair1 = make_pair((i)* width + j - 1, weightssl);
			graphMatrix[id].push_back(npair1);
			pair < int, int > npair = make_pair((i - 1) * width + j, weightssu);
			graphMatrix[id].push_back(npair);
		}
		else if (id / width == 0) {
			int weightssd = maxIntensity - ((gradientImage.at < uchar >(i, j) + gradientImage.at < uchar >(i + 1, j)) / 2);
			int weightssl = maxIntensity - ((gradientImage.at < uchar >(i, j) + gradientImage.at < uchar >(i, j - 1)) / 2);
			int weightssr = maxIntensity - ((gradientImage.at < uchar >(i, j) + gradientImage.at < uchar >(i, j + 1)) / 2);
			pair < int, int > npair1 = make_pair((i)* width + j + 1, weightssr);
			graphMatrix[id].push_back(npair1);
			pair < int, int > npair2 = make_pair((i + 1) * width + j, weightssd);
			graphMatrix[id].push_back(npair2);
			pair < int, int > npair3 = make_pair((i)* width + j - 1, weightssl);
			graphMatrix[id].push_back(npair3);
		}
		else if ((id + 1) % width == 0) {
			int weightssd = maxIntensity - ((gradientImage.at < uchar >(i, j) + gradientImage.at < uchar >(i + 1, j)) / 2);
			int weightssu = maxIntensity - ((gradientImage.at < uchar >(i, j) + gradientImage.at < uchar >(i - 1, j)) / 2);
			int weightssl = maxIntensity - ((gradientImage.at < uchar >(i, j) + gradientImage.at < uchar >(i, j - 1)) / 2);
			pair < int, int > npair = make_pair((i - 1) * width + j, weightssu);
			graphMatrix[id].push_back(npair);
			pair < int, int > npair2 = make_pair((i + 1) * width + j, weightssd);
			graphMatrix[id].push_back(npair2);
			pair < int, int > npair3 = make_pair((i)* width + j - 1, weightssl);
			graphMatrix[id].push_back(npair3);
		}
		else if (id % width == 0) {
			int weightssd = maxIntensity - ((gradientImage.at < uchar >(i, j) + gradientImage.at < uchar >(i + 1, j)) / 2);
			int weightssu = maxIntensity - ((gradientImage.at < uchar >(i, j) + gradientImage.at < uchar >(i - 1, j)) / 2);
			int weightssr = maxIntensity - ((gradientImage.at < uchar >(i, j) + gradientImage.at < uchar >(i, j + 1)) / 2);
			pair < int, int > npair = make_pair((i - 1) * width + j, weightssu);
			graphMatrix[id].push_back(npair);
			pair < int, int > npair1 = make_pair((i)* width + j + 1, weightssr);
			graphMatrix[id].push_back(npair1);
			pair < int, int > npair2 = make_pair((i + 1) * width + j, weightssd);
			graphMatrix[id].push_back(npair2);
		}
		else if (id > (width * (height - 1))) {
			int weightssu = maxIntensity - ((gradientImage.at < uchar >(i, j) + gradientImage.at < uchar >(i - 1, j)) / 2);
			int weightssl = maxIntensity - ((gradientImage.at < uchar >(i, j) + gradientImage.at < uchar >(i, j - 1)) / 2);
			int weightssr = maxIntensity - ((gradientImage.at < uchar >(i, j) + gradientImage.at < uchar >(i, j + 1)) / 2);
			pair < int, int > npair = make_pair((i - 1) * width + j, weightssu);
			graphMatrix[id].push_back(npair);
			pair < int, int > npair1 = make_pair((i)* width + j + 1, weightssr);
			graphMatrix[id].push_back(npair1);
			pair < int, int > npair3 = make_pair((i)* width + j - 1, weightssl);
			graphMatrix[id].push_back(npair3);
		}
		else {
			int weightssd = maxIntensity - ((gradientImage.at < uchar >(i, j) + gradientImage.at < uchar >(i + 1, j)) / 2);
			int weightssu = maxIntensity - ((gradientImage.at < uchar >(i, j) + gradientImage.at < uchar >(i - 1, j)) / 2);
			int weightssl = maxIntensity - ((gradientImage.at < uchar >(i, j) + gradientImage.at < uchar >(i, j - 1)) / 2);
			int weightssr = maxIntensity - ((gradientImage.at < uchar >(i, j) + gradientImage.at < uchar >(i, j + 1)) / 2);
			pair < int, int > npair = make_pair((i - 1) * width + j, weightssu);
			graphMatrix[id].push_back(npair);
			pair < int, int > npair1 = make_pair((i)* width + j + 1, weightssr);
			graphMatrix[id].push_back(npair1);
			pair < int, int > npair2 = make_pair((i + 1) * width + j, weightssd);
			graphMatrix[id].push_back(npair2);
			pair < int, int > npair3 = make_pair((i)* width + j - 1, weightssl);
			graphMatrix[id].push_back(npair3);
		}
	}

	//calculating total number of pixels
	int ncont = 0;
	int totl = 0;
	for (int i = 0; i < height * width; i++) {
		for (int j = 0; j < graphMatrix[i].size(); j++) {
			totl = totl + graphMatrix[i][j].second;
			ncont++;
		}
	}
	int avr = totl / ncont;
	for (int i = 0; i < height * width; i++) {
		for (int j = 0; j < graphMatrix[i].size(); j++) {
			if (graphMatrix[i][j].second < avr) {
				graphMatrix[i][j].second = 1;
			}
			else {
				graphMatrix[i][j].second = 1000000;
			}
		}
	}
	vector < pair < int, int > > comb2;
	for (int i = 0; i < height * width; i++) {
		pair < int, int > npair = make_pair(i, 0);
		comb2.push_back(npair);
	}
	vector < pair < int, int > > comb;
	for (int i = 0; i < height * width; i++) {
		pair < int, int > npair = make_pair(i, 0);
		comb.push_back(npair);
	}
	int label;
	vector < int > path;
	
	vector<int>::iterator it;
	// min cut max flow 
	ofstream OUTPUT_FILE("weight.txt");
	bool v;
	v = true;
	while (v == true) {
		v = false;
		while (!sourceQueue.empty()) {
		again: vector < int > cpath;
			cpath = sourceQueue.front();
			OUTPUT_FILE << "  " << cpath.size() << endl;
			sourceQueue.pop();
			int last = cpath.back();
			if (check2(last) == 1) {
				path.clear();
				path = cpath;
				v = true;
				comb.clear();
				for (int i = 0; i < height * width; i++) {
					pair < int, int > npair = make_pair(i, 0);
					comb.push_back(npair);
				}
				while (!sourceQueue.empty()) {
					sourceQueue.pop();
				}
				for (int i = 0; i < sourceCount; i++) {
					vector<int> path0;
					path0.push_back(source[i]);
					sourceQueue.push(path0);
				}
				break;
			}
			for (int i = 0; i < graphMatrix[last].size(); ++i) {
				if (check(graphMatrix[last][i].first, cpath) && graphMatrix[last][i].second > 0 && comb[graphMatrix[last][i].first].second == 0) {
					vector < int > opath(cpath);
					opath.push_back(graphMatrix[last][i].first);
					comb[graphMatrix[last][i].first].second = 1;
					sourceQueue.push(opath);
				}
			}
		}
		if (v == true) {
			int smallest = 99999999;
			for (int i = 0; i < path.size() - 1; i++) {
				for (int j = 0; j < graphMatrix[path[i]].size(); j++) {
					if (graphMatrix[path[i]][j].first == path[i + 1]) {
						if (graphMatrix[path[i]][j].second < smallest) {
							smallest = graphMatrix[path[i]][j].second;
						}
					}
				}
			}
			for (int i = 0; i < path.size() - 1; i++) {
				for (int j = 0; j < graphMatrix[path[i]].size(); j++) {
					if (graphMatrix[path[i]][j].first == path[i + 1]) {
						graphMatrix[path[i]][j].second = graphMatrix[path[i]][j].second - smallest;

					}
				}

			}
			for (int i = 0; i < path.size() - 1; i++) {
				for (int j = 0; j < graphMatrix[path[i + 1]].size(); j++) {
					if (graphMatrix[path[i + 1]][j].first == path[i]) {
						graphMatrix[path[i + 1]][j].second = graphMatrix[path[i + 1]][j].second + smallest;
					}
				}
			}
		}
	}
	for (int i = 0; i < height; i++) {
		for (int j = 0; j < width; j++) {
			Vec3b pixel;
			pixel[0] = 255;
			pixel[1] = 255;
			pixel[2] = 255;
			outputImage.at<Vec3b>(i, j) = pixel;
		}
	}
	vector < pair < int, int > > comb1;
	for (int i = 0; i < height * width; i++) {
		pair < int, int > npair = make_pair(i, 0);
		comb1.push_back(npair);
	}
	Vec3b pixel2;
	pixel2[0] = 0;
	pixel2[1] = 0;
	pixel2[2] = 0;

	queue < int > qcolor;
	int u = source[0];
	qcolor.push(u);
	int x1, x2;
	x1 = u / width;
	x2 = u % width;
	outputImage.at < Vec3b >(x1, x2) = pixel2;
	int mino = 99999999;

	//performing BFS
	while (!qcolor.empty()) {
		int last = qcolor.front();
		qcolor.pop();
		for (int i = 0; i < graphMatrix[last].size(); ++i) {
			if (comb1[graphMatrix[last][i].first].second == 0 && graphMatrix[last][i].second > 0) {
				Vec3b pixel2;
				pixel2[0] = 255;
				pixel2[1] = 255;
				pixel2[2] = 255;
				int x3, x4;
				x3 = (graphMatrix[last][i].first) / width;
				x4 = (graphMatrix[last][i].first) % width;
				outputImage.at < Vec3b >(x3, x4) = pixel2;
				qcolor.push(graphMatrix[last][i].first);
				comb1[graphMatrix[last][i].first].second = 1;
			}
		}
	}


	// write it on disk
	imwrite(argv[3], outputImage);

	// also display them both
	OUTPUT_FILE.close();
	namedWindow("Original image", WINDOW_AUTOSIZE);
	namedWindow("Show Marked Pixels", WINDOW_AUTOSIZE);
	imshow("Original image", inputImage);
	imshow("Show Marked Pixels", outputImage);
	waitKey(0);
	return 0;
}