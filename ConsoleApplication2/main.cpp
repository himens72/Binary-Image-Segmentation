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


using namespace cv;
using namespace std;
Mat INPUT_IMAGE;
Mat OUTPUT_IMAGE;
vector < vector < pair < int, int > > > weightedMatrix;
vector < pair < int, int > > initialMatrix1;
vector < pair < int, int > > initialMatrix2;
Mat GRAYSCALE_IMAGE;
Mat GRADIENT_IMAGE;
int MAXIMUM_INTENSITY = 0;
int height;
int width;
int totalPixel;
bool *visited;
vector < int > source;
vector < int > sink;
queue < vector < int > > sourceQueue;

int check(int x, vector < int > cpath) {
	int size = cpath.size();
	for (int i = 0; i < size; i++)
		if (cpath[i] == x)
			return 0;
	return 1;
}
int check2(int  index) {
	for (int i = 0; i < sink.size(); i++) {
		if (sink[i] == index)
			return 1;
	}
	return 0;
}
//This Method is used to read Input Image

void readInputImage(char* inputFile) {
	INPUT_IMAGE = imread(inputFile);
	OUTPUT_IMAGE = INPUT_IMAGE.clone();
}
// This Method is used to generate Gradient Image 
void generateGradientImage() {
	cvtColor(INPUT_IMAGE, GRAYSCALE_IMAGE, COLOR_BGR2GRAY);
	Mat sobel_x, sobel_y;
	Mat absolute_x, absolute_y;
	Sobel(GRAYSCALE_IMAGE, sobel_x, CV_16S, 1, 0, 3, 1, 0, BORDER_DEFAULT);
	Sobel(GRAYSCALE_IMAGE, sobel_y, CV_16S, 0, 1, 3, 1, 0, BORDER_DEFAULT);
	convertScaleAbs(sobel_x, absolute_x);
	convertScaleAbs(sobel_y, absolute_y);
	addWeighted(absolute_x, 0.5, absolute_y, 0.5, 0, GRADIENT_IMAGE);
}
void calculateMaximumIntensity() {
	for (int i = 0; i < height; i++) {
		for (int j = 0; j < width; j++) {
			int currentIntensity = GRAYSCALE_IMAGE.at < uchar >(i, j);
			if (MAXIMUM_INTENSITY < currentIntensity) {
				MAXIMUM_INTENSITY = currentIntensity;
			}
		}
	}
}
int readConfigFile(char* configFile) {
	ifstream file(configFile);
	if (!file) {
		cout << "Unable to load config file" << endl;
		return -1;
	}
	int data;
	file >> data;
	for (int i = 0; i < data; ++i) {
		int x;
		int y;
		int marker;
		file >> x >> y >> marker;
		if (x < 0 || x >= width || y < 0 || y >= height) {
			cout << "Invalid pixel in config file" << endl;
			return -1;
		}
		if (marker == 1) {
			source.push_back((width * y) + x);
			vector<int> path0;
			path0.push_back((width * y) + x);
			sourceQueue.push(path0);
		}
		else {
			sink.push_back((width * y) + x);
		}
	}
	return 0;
}
// This method is used to generate background in Output Image
void generateBackground(int i, int j) {
	Vec3b pixel;
	pixel[0] = 0;
	pixel[1] = 0;
	pixel[2] = 0;
	OUTPUT_IMAGE.at<Vec3b>(i, j) = pixel;
}
void performBFS() {
	Vec3b pixel;
	pixel[0] = 255;
	pixel[1] = 255;
	pixel[2] = 255;
	queue < int > PIXEL_COLOR;
	PIXEL_COLOR.push(source[0]);
	visited[source[0]] = true;
	OUTPUT_IMAGE.at < Vec3b >(source[0] / width, source[0] % width) = pixel;
	//performing BFS
	while (!PIXEL_COLOR.empty()) {
		int last = PIXEL_COLOR.front();
		PIXEL_COLOR.pop();
		for (int i = 0; i < weightedMatrix[last].size(); ++i) {
			if (!visited[weightedMatrix[last][i].first] && initialMatrix2[weightedMatrix[last][i].first].second == 0 && weightedMatrix[last][i].second > 0) {
				Vec3b pixel;
				pixel[0] = 255; // blue color
				pixel[1] = 255; // red color
				pixel[2] = 255; // green color
				OUTPUT_IMAGE.at < Vec3b >((weightedMatrix[last][i].first) / width, (weightedMatrix[last][i].first) % width) = pixel;
				visited[weightedMatrix[last][i].first] = true;
				PIXEL_COLOR.push(weightedMatrix[last][i].first);
				initialMatrix2[weightedMatrix[last][i].first].second = 1;
			}
		}
	}
}
int main(int argc, char * * argv) {
	if (argc != 4) {
		cout << "Usage: ../seg input_image initialization_file output_mask" << endl;
		return -1;
	}
	readInputImage(argv[1]);
	if (!INPUT_IMAGE.data) {
		cout << "Could not load input image!!!" << endl;
		return -1;
	}
	if (INPUT_IMAGE.channels() != 3) {
		cout << "Image does not have 3 channels!!! " << INPUT_IMAGE.depth() << endl;
		return -1;
	}
	generateGradientImage();
	//calculate the pixel energies
	width = INPUT_IMAGE.cols;
	height = INPUT_IMAGE.rows;
	totalPixel = height * width;
	calculateMaximumIntensity();
	visited = new bool[totalPixel];
	bool *tempVisited = new bool[totalPixel];
	weightedMatrix.resize(totalPixel);
	initialMatrix1.resize(totalPixel);
	initialMatrix2.resize(totalPixel);
	int totalEdge = 0;
	int averageWeight;
	int totalWeight = 0;
	cout << "Weight Modified" << endl;
	for (int id = 0; id < totalPixel; id++) {
		int i = id / width;
		int j = id % width;
		visited[id] = false;
		tempVisited[id] = false;
		initialMatrix1.push_back(make_pair(id, 0));
		initialMatrix2.push_back(make_pair(id, 0));
		generateBackground(i, j);
		if (id == 0) {
			totalEdge += 2;
			int WEIGHT_RIGHT = MAXIMUM_INTENSITY - ((GRADIENT_IMAGE.at < uchar >(i, j) + GRADIENT_IMAGE.at < uchar >(i, j + 1)) / 2);
			weightedMatrix[id].push_back(make_pair((i)* width + j + 1, WEIGHT_RIGHT));
			int WEIGHT_DOWN = MAXIMUM_INTENSITY - ((GRADIENT_IMAGE.at < uchar >(i, j) + GRADIENT_IMAGE.at < uchar >(i + 1, j)) / 2);
			weightedMatrix[id].push_back(make_pair((i + 1) * width + j, WEIGHT_DOWN));
			totalWeight = totalWeight + WEIGHT_DOWN + WEIGHT_RIGHT;
		}
		else if (id == width - 1) {
			totalEdge += 2;
			int WEIGHT_DOWN = MAXIMUM_INTENSITY - ((GRADIENT_IMAGE.at < uchar >(i, j) + GRADIENT_IMAGE.at < uchar >(i + 1, j)) / 2);
			weightedMatrix[id].push_back(make_pair((i + 1) * width + j, WEIGHT_DOWN));
			int WEIGHT_LEFT = MAXIMUM_INTENSITY - ((GRADIENT_IMAGE.at < uchar >(i, j) + GRADIENT_IMAGE.at < uchar >(i, j - 1)) / 2);
			weightedMatrix[id].push_back(make_pair((i)* width + j - 1, WEIGHT_LEFT));
			totalWeight = totalWeight + WEIGHT_DOWN + WEIGHT_LEFT;
		}
		else if (id == width * (height - 1)) {
			totalEdge += 2;
			int WEIGHT_UP = MAXIMUM_INTENSITY - ((GRADIENT_IMAGE.at < uchar >(i, j) + GRADIENT_IMAGE.at < uchar >(i - 1, j)) / 2);
			weightedMatrix[id].push_back(make_pair((i - 1) * width + j, WEIGHT_UP));
			int WEIGHT_RIGHT = MAXIMUM_INTENSITY - ((GRADIENT_IMAGE.at < uchar >(i, j) + GRADIENT_IMAGE.at < uchar >(i, j + 1)) / 2);
			weightedMatrix[id].push_back(make_pair((i)* width + j + 1, WEIGHT_RIGHT));
			totalWeight = totalWeight + WEIGHT_UP + WEIGHT_RIGHT;
		}
		else if (id == totalPixel - 1) {
			totalEdge += 2;
			int WEIGHT_LEFT = MAXIMUM_INTENSITY - ((GRADIENT_IMAGE.at < uchar >(i, j) + GRADIENT_IMAGE.at < uchar >(i, j - 1)) / 2);
			weightedMatrix[id].push_back(make_pair((i)* width + j - 1, WEIGHT_LEFT));
			int WEIGHT_UP = MAXIMUM_INTENSITY - ((GRADIENT_IMAGE.at < uchar >(i, j) + GRADIENT_IMAGE.at < uchar >(i - 1, j)) / 2);
			weightedMatrix[id].push_back(make_pair((i - 1) * width + j, WEIGHT_UP));
			totalWeight = totalWeight + WEIGHT_UP + WEIGHT_LEFT;
		}
		else if (id / width == 0) {
			int WEIGHT_RIGHT = MAXIMUM_INTENSITY - ((GRADIENT_IMAGE.at < uchar >(i, j) + GRADIENT_IMAGE.at < uchar >(i, j + 1)) / 2);
			totalEdge += 3;
			int WEIGHT_DOWN = MAXIMUM_INTENSITY - ((GRADIENT_IMAGE.at < uchar >(i, j) + GRADIENT_IMAGE.at < uchar >(i + 1, j)) / 2);
			weightedMatrix[id].push_back(make_pair((i + 1) * width + j, WEIGHT_DOWN));
			int WEIGHT_LEFT = MAXIMUM_INTENSITY - ((GRADIENT_IMAGE.at < uchar >(i, j) + GRADIENT_IMAGE.at < uchar >(i, j - 1)) / 2);
			weightedMatrix[id].push_back(make_pair((i)* width + j + 1, WEIGHT_RIGHT));
			weightedMatrix[id].push_back(make_pair((i)* width + j - 1, WEIGHT_LEFT));
			totalWeight = totalWeight + WEIGHT_DOWN + WEIGHT_LEFT + WEIGHT_RIGHT;
		}
		else if ((id + 1) % width == 0) {
			totalEdge += 3;
			int WEIGHT_UP = MAXIMUM_INTENSITY - ((GRADIENT_IMAGE.at < uchar >(i, j) + GRADIENT_IMAGE.at < uchar >(i - 1, j)) / 2);
			weightedMatrix[id].push_back(make_pair((i - 1) * width + j, WEIGHT_UP));
			int WEIGHT_DOWN = MAXIMUM_INTENSITY - ((GRADIENT_IMAGE.at < uchar >(i, j) + GRADIENT_IMAGE.at < uchar >(i + 1, j)) / 2);
			weightedMatrix[id].push_back(make_pair((i + 1) * width + j, WEIGHT_DOWN));
			int WEIGHT_LEFT = MAXIMUM_INTENSITY - ((GRADIENT_IMAGE.at < uchar >(i, j) + GRADIENT_IMAGE.at < uchar >(i, j - 1)) / 2);
			weightedMatrix[id].push_back(make_pair((i)* width + j - 1, WEIGHT_LEFT));
			totalWeight = totalWeight + WEIGHT_DOWN + WEIGHT_LEFT + WEIGHT_UP;
		}
		else if (id % width == 0) {
			totalEdge += 3;
			int WEIGHT_UP = MAXIMUM_INTENSITY - ((GRADIENT_IMAGE.at < uchar >(i, j) + GRADIENT_IMAGE.at < uchar >(i - 1, j)) / 2);
			weightedMatrix[id].push_back(make_pair((i - 1) * width + j, WEIGHT_UP));
			int WEIGHT_RIGHT = MAXIMUM_INTENSITY - ((GRADIENT_IMAGE.at < uchar >(i, j) + GRADIENT_IMAGE.at < uchar >(i, j + 1)) / 2);
			weightedMatrix[id].push_back(make_pair((i)* width + j + 1, WEIGHT_RIGHT));
			int WEIGHT_DOWN = MAXIMUM_INTENSITY - ((GRADIENT_IMAGE.at < uchar >(i, j) + GRADIENT_IMAGE.at < uchar >(i + 1, j)) / 2);
			weightedMatrix[id].push_back(make_pair((i + 1) * width + j, WEIGHT_DOWN));
			totalWeight = totalWeight + WEIGHT_DOWN + WEIGHT_RIGHT + WEIGHT_UP;
		}
		else if (id > (width * (height - 1))) {
			totalEdge += 3;
			int WEIGHT_UP = MAXIMUM_INTENSITY - ((GRADIENT_IMAGE.at < uchar >(i, j) + GRADIENT_IMAGE.at < uchar >(i - 1, j)) / 2);
			weightedMatrix[id].push_back(make_pair((i - 1) * width + j, WEIGHT_UP));
			int WEIGHT_RIGHT = MAXIMUM_INTENSITY - ((GRADIENT_IMAGE.at < uchar >(i, j) + GRADIENT_IMAGE.at < uchar >(i, j + 1)) / 2);
			weightedMatrix[id].push_back(make_pair((i)* width + j + 1, WEIGHT_RIGHT));
			int WEIGHT_LEFT = MAXIMUM_INTENSITY - ((GRADIENT_IMAGE.at < uchar >(i, j) + GRADIENT_IMAGE.at < uchar >(i, j - 1)) / 2);
			weightedMatrix[id].push_back(make_pair((i)* width + j - 1, WEIGHT_LEFT));
			totalWeight = totalWeight + WEIGHT_RIGHT + WEIGHT_LEFT + WEIGHT_UP;
		}
		else {
			totalEdge += 4;
			int WEIGHT_UP = MAXIMUM_INTENSITY - ((GRADIENT_IMAGE.at < uchar >(i, j) + GRADIENT_IMAGE.at < uchar >(i - 1, j)) / 2);
			weightedMatrix[id].push_back(make_pair((i - 1) * width + j, WEIGHT_UP));
			int WEIGHT_RIGHT = MAXIMUM_INTENSITY - ((GRADIENT_IMAGE.at < uchar >(i, j) + GRADIENT_IMAGE.at < uchar >(i, j + 1)) / 2);
			weightedMatrix[id].push_back(make_pair((i)* width + j + 1, WEIGHT_RIGHT));
			int WEIGHT_DOWN = MAXIMUM_INTENSITY - ((GRADIENT_IMAGE.at < uchar >(i, j) + GRADIENT_IMAGE.at < uchar >(i + 1, j)) / 2);
			weightedMatrix[id].push_back(make_pair((i + 1) * width + j, WEIGHT_DOWN));
			int WEIGHT_LEFT = MAXIMUM_INTENSITY - ((GRADIENT_IMAGE.at < uchar >(i, j) + GRADIENT_IMAGE.at < uchar >(i, j - 1)) / 2);
			weightedMatrix[id].push_back(make_pair((i)* width + j - 1, WEIGHT_LEFT));
			totalWeight = totalWeight + WEIGHT_RIGHT + WEIGHT_LEFT + WEIGHT_UP + WEIGHT_DOWN;
		}
	}
	averageWeight = totalWeight / totalEdge;
	cout << "totalEdge : " << totalEdge << endl;
	cout << "totalWeight : " << totalWeight << endl;
	cout << "averageWeight : " << averageWeight << endl;
	for (int i = 0; i < totalPixel; i++) {
		for (int j = 0; j < weightedMatrix[i].size(); j++) {
			if (weightedMatrix[i][j].second < averageWeight) {
				weightedMatrix[i][j].second = 1;
			}
			else {
				weightedMatrix[i][j].second = 1000;
			}
		}
	}
	if (readConfigFile(argv[2]) == -1)
		return -1;
	vector < int > path;
	// min cut max flow 
	bool flag = true;
	while (flag == true) {
		flag = false;
		while (!sourceQueue.empty()) {
		again: vector < int > cpath;
			cpath = sourceQueue.front();
			tempVisited[sourceQueue.front()[0]] = true;
			sourceQueue.pop();
			int last = cpath.back();
			if (check2(last) == 1) {
				path.clear();
				path = cpath;
				flag = true;
				initialMatrix1.clear();
				initialMatrix1 = initialMatrix2;
				while (!sourceQueue.empty()) {
					sourceQueue.pop();
				}
				for (int i = 0; i < source.size(); i++) {
					vector<int> path0;
					path0.push_back(source[i]);
					sourceQueue.push(path0);
				}
				for (int i = 0; i < totalPixel; i++) {
					tempVisited[i] = false;
				}
				break;
			}
			for (int i = 0; i < weightedMatrix[last].size(); ++i) {
				if (!tempVisited[weightedMatrix[last][i].first] && check(weightedMatrix[last][i].first, cpath) && weightedMatrix[last][i].second > 0 && initialMatrix1[weightedMatrix[last][i].first].second == 0) {
					vector < int > opath(cpath);
					opath.push_back(weightedMatrix[last][i].first);
					initialMatrix1[weightedMatrix[last][i].first].second = 1;
					tempVisited[weightedMatrix[last][i].first] = true;
					sourceQueue.push(opath);
				}
			}
		}
		if (flag == true) {
			int smallest = 1001;
			for (int i = 0; i < path.size() - 1; i++) {
				for (int j = 0; j < weightedMatrix[path[i]].size(); j++) {
					if (weightedMatrix[path[i]][j].first == path[i + 1]) {
						if (weightedMatrix[path[i]][j].second < smallest) {
							smallest = weightedMatrix[path[i]][j].second;
						}
					}
				}
			}
			for (int i = 0; i < path.size() - 1; i++) {
				for (int j = 0; j < weightedMatrix[path[i]].size(); j++) {
					if (weightedMatrix[path[i]][j].first == path[i + 1]) {
						weightedMatrix[path[i]][j].second = weightedMatrix[path[i]][j].second - smallest;
					}
				}
			}
			for (int i = 0; i < path.size() - 1; i++) {
				for (int j = 0; j < weightedMatrix[path[i + 1]].size(); j++) {
					if (weightedMatrix[path[i + 1]][j].first == path[i]) {
						weightedMatrix[path[i + 1]][j].second = weightedMatrix[path[i + 1]][j].second + smallest;
					}
				}
			}
		}
	}
	performBFS();
	// write it on disk
	imwrite(argv[3], OUTPUT_IMAGE);
	// also display them both
	namedWindow("Input Image", WINDOW_AUTOSIZE);
	namedWindow("Output Image", WINDOW_AUTOSIZE);
	imshow("Input Image", INPUT_IMAGE);
	imshow("Output Image", OUTPUT_IMAGE);
	waitKey(0);
	return 0;
}
