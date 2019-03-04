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
Mat GRAYSCALE_IMAGE;
Mat GRADIENT_IMAGE;
int MAXIMUM_INTENSITY = 0;
int height;
int width;
int totalPixel;
bool *visited;
vector<int> source;
vector<int> target;
queue < vector < int > > sourceQueue;

int check(int x, vector < int > cpath) {
	int size = cpath.size();
	for (int i = 0; i < size; i++)
		if (cpath[i] == x)
			return 0;
	return 1;
}
int check2(vector<int> tar, int la) {
	int x1 = la;
	for (int i = 0; i < tar.size(); i++) {
		if (tar[i] == x1)
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
	Mat grad_x, grad_y;
	Mat abs_grad_x, abs_grad_y;
	Sobel(GRAYSCALE_IMAGE, grad_x, CV_16S, 1, 0, 3, 1, 0, BORDER_DEFAULT);
	Sobel(GRAYSCALE_IMAGE, grad_y, CV_16S, 0, 1, 3, 1, 0, BORDER_DEFAULT);
	convertScaleAbs(grad_x, abs_grad_x);
	convertScaleAbs(grad_y, abs_grad_y);
	addWeighted(abs_grad_x, 0.5, abs_grad_y, 0.5, 0, GRADIENT_IMAGE);
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
			vector<int> temp;
			temp.push_back((width * y) + x);
			sourceQueue.push(temp);
		}
		else {
			target.push_back((width * y) + x);
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

void generateSourceQueue() {
	while (!sourceQueue.empty()) {
		sourceQueue.pop();
	}
	for (int i = 0; i < source.size(); i++) {
		vector<int> temp;
		temp.push_back(source[i]);
		sourceQueue.push(temp);
	}
}
int main(int argc, char * * argv) {
	if (argc != 4) {
		cout << "Usage: ../seg INPUT_IMAGE_NAME CONFIG_FILE_NAME OUTPUT_IMAGE_NAME" << endl;
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
	width = INPUT_IMAGE.cols;
	height = INPUT_IMAGE.rows;
	totalPixel = height * width;
	calculateMaximumIntensity();
	visited = new bool[totalPixel];
	bool *tempVisited = new bool[totalPixel];
	weightedMatrix.resize(totalPixel);
	vector < pair < int, int > > initialMatrix1; //initialMatrix1 is a matrix used  to identify which pixel is background or foreground
	vector < pair < int, int > > initialMatrix2; //initialMatrix2 is a matrix used  to identify which pixel is background or foreground
	int totalEdge = 0;
	int averageWeight;
	int totalWeight = 0;
	for (int id = 0; id < totalPixel; id++) {
		int i = id / width;
		int j = id % width;
		visited[id] = false;
		tempVisited[id] = false;
		initialMatrix1.push_back(make_pair(id, 0));
		initialMatrix2.push_back(make_pair(id, 0));
		generateBackground(i, j);
		if (id == 0) {
			int WEIGHT_DOWN = MAXIMUM_INTENSITY - ((GRADIENT_IMAGE.at < uchar >(i, j) + GRADIENT_IMAGE.at < uchar >(i + 1, j)) / 2);
			int WEIGHT_RIGHT = MAXIMUM_INTENSITY - ((GRADIENT_IMAGE.at < uchar >(i, j) + GRADIENT_IMAGE.at < uchar >(i, j + 1)) / 2);
			totalEdge += 2;
			totalWeight = totalWeight + WEIGHT_DOWN + WEIGHT_RIGHT;
			pair < int, int > npair1 = make_pair((i)* width + j + 1, WEIGHT_RIGHT);
			weightedMatrix[id].push_back(npair1);
			pair < int, int > npair = make_pair((i + 1) * width + j, WEIGHT_DOWN);
			weightedMatrix[id].push_back(npair);
		}
		else if (id == width - 1) {
			int WEIGHT_DOWN = MAXIMUM_INTENSITY - ((GRADIENT_IMAGE.at < uchar >(i, j) + GRADIENT_IMAGE.at < uchar >(i + 1, j)) / 2);
			int WEIGHT_LEFT = MAXIMUM_INTENSITY - ((GRADIENT_IMAGE.at < uchar >(i, j) + GRADIENT_IMAGE.at < uchar >(i, j - 1)) / 2);
			totalEdge += 2;
			totalWeight = totalWeight + WEIGHT_DOWN + WEIGHT_LEFT;
			pair < int, int > npair = make_pair((i + 1) * width + j, WEIGHT_DOWN);
			weightedMatrix[id].push_back(npair);
			pair < int, int > npair1 = make_pair((i)* width + j - 1, WEIGHT_LEFT);
			weightedMatrix[id].push_back(npair1);

		}
		else if (id == width * (height - 1)) {

			int WEIGHT_UP = MAXIMUM_INTENSITY - ((GRADIENT_IMAGE.at < uchar >(i, j) + GRADIENT_IMAGE.at < uchar >(i - 1, j)) / 2);
			int WEIGHT_RIGHT = MAXIMUM_INTENSITY - ((GRADIENT_IMAGE.at < uchar >(i, j) + GRADIENT_IMAGE.at < uchar >(i, j + 1)) / 2);
			totalEdge += 2;
			totalWeight = totalWeight + WEIGHT_UP + WEIGHT_RIGHT;
			pair < int, int > npair = make_pair((i - 1) * width + j, WEIGHT_UP);
			weightedMatrix[id].push_back(npair);
			pair < int, int > npair1 = make_pair((i)* width + j + 1, WEIGHT_RIGHT);
			weightedMatrix[id].push_back(npair1);
		}
		else if (id == totalPixel - 1) {
			int WEIGHT_UP = MAXIMUM_INTENSITY - ((GRADIENT_IMAGE.at < uchar >(i, j) + GRADIENT_IMAGE.at < uchar >(i - 1, j)) / 2);
			int WEIGHT_LEFT = MAXIMUM_INTENSITY - ((GRADIENT_IMAGE.at < uchar >(i, j) + GRADIENT_IMAGE.at < uchar >(i, j - 1)) / 2);
			totalEdge += 2;
			totalWeight = totalWeight + WEIGHT_UP + WEIGHT_LEFT;
			pair < int, int > npair1 = make_pair((i)* width + j - 1, WEIGHT_LEFT);
			weightedMatrix[id].push_back(npair1);
			pair < int, int > npair = make_pair((i - 1) * width + j, WEIGHT_UP);
			weightedMatrix[id].push_back(npair);
		}
		else if (id / width == 0) {
			int WEIGHT_DOWN = MAXIMUM_INTENSITY - ((GRADIENT_IMAGE.at < uchar >(i, j) + GRADIENT_IMAGE.at < uchar >(i + 1, j)) / 2);
			int WEIGHT_LEFT = MAXIMUM_INTENSITY - ((GRADIENT_IMAGE.at < uchar >(i, j) + GRADIENT_IMAGE.at < uchar >(i, j - 1)) / 2);
			int WEIGHT_RIGHT = MAXIMUM_INTENSITY - ((GRADIENT_IMAGE.at < uchar >(i, j) + GRADIENT_IMAGE.at < uchar >(i, j + 1)) / 2);
			totalEdge += 3;
			totalWeight = totalWeight + WEIGHT_DOWN + WEIGHT_LEFT + WEIGHT_RIGHT;
			pair < int, int > npair1 = make_pair((i)* width + j + 1, WEIGHT_RIGHT);
			weightedMatrix[id].push_back(npair1);

			pair < int, int > npair2 = make_pair((i + 1) * width + j, WEIGHT_DOWN);
			weightedMatrix[id].push_back(npair2);

			pair < int, int > npair3 = make_pair((i)* width + j - 1, WEIGHT_LEFT);
			weightedMatrix[id].push_back(npair3);

		}
		else if ((id + 1) % width == 0) {
			int WEIGHT_DOWN = MAXIMUM_INTENSITY - ((GRADIENT_IMAGE.at < uchar >(i, j) + GRADIENT_IMAGE.at < uchar >(i + 1, j)) / 2);
			int WEIGHT_UP = MAXIMUM_INTENSITY - ((GRADIENT_IMAGE.at < uchar >(i, j) + GRADIENT_IMAGE.at < uchar >(i - 1, j)) / 2);
			int WEIGHT_LEFT = MAXIMUM_INTENSITY - ((GRADIENT_IMAGE.at < uchar >(i, j) + GRADIENT_IMAGE.at < uchar >(i, j - 1)) / 2);
			totalEdge += 3;
			totalWeight = totalWeight + WEIGHT_DOWN + WEIGHT_LEFT + WEIGHT_UP;
			pair < int, int > npair = make_pair((i - 1) * width + j, WEIGHT_UP);
			weightedMatrix[id].push_back(npair);

			pair < int, int > npair2 = make_pair((i + 1) * width + j, WEIGHT_DOWN);
			weightedMatrix[id].push_back(npair2);

			pair < int, int > npair3 = make_pair((i)* width + j - 1, WEIGHT_LEFT);
			weightedMatrix[id].push_back(npair3);
		}
		else if (id % width == 0) {
			int WEIGHT_DOWN = MAXIMUM_INTENSITY - ((GRADIENT_IMAGE.at < uchar >(i, j) + GRADIENT_IMAGE.at < uchar >(i + 1, j)) / 2);
			int WEIGHT_UP = MAXIMUM_INTENSITY - ((GRADIENT_IMAGE.at < uchar >(i, j) + GRADIENT_IMAGE.at < uchar >(i - 1, j)) / 2);

			int WEIGHT_RIGHT = MAXIMUM_INTENSITY - ((GRADIENT_IMAGE.at < uchar >(i, j) + GRADIENT_IMAGE.at < uchar >(i, j + 1)) / 2);
			totalEdge += 3;
			totalWeight = totalWeight + WEIGHT_DOWN + WEIGHT_RIGHT + WEIGHT_UP;
			pair < int, int > npair = make_pair((i - 1) * width + j, WEIGHT_UP);
			weightedMatrix[id].push_back(npair);

			pair < int, int > npair1 = make_pair((i)* width + j + 1, WEIGHT_RIGHT);
			weightedMatrix[id].push_back(npair1);

			pair < int, int > npair2 = make_pair((i + 1) * width + j, WEIGHT_DOWN);
			weightedMatrix[id].push_back(npair2);

		}
		else if (id > (width * (height - 1))) {
			int WEIGHT_UP = MAXIMUM_INTENSITY - ((GRADIENT_IMAGE.at < uchar >(i, j) + GRADIENT_IMAGE.at < uchar >(i - 1, j)) / 2);
			int WEIGHT_LEFT = MAXIMUM_INTENSITY - ((GRADIENT_IMAGE.at < uchar >(i, j) + GRADIENT_IMAGE.at < uchar >(i, j - 1)) / 2);
			int WEIGHT_RIGHT = MAXIMUM_INTENSITY - ((GRADIENT_IMAGE.at < uchar >(i, j) + GRADIENT_IMAGE.at < uchar >(i, j + 1)) / 2);
			totalEdge += 3;
			totalWeight = totalWeight + WEIGHT_RIGHT + WEIGHT_LEFT + WEIGHT_UP;
			pair < int, int > npair = make_pair((i - 1) * width + j, WEIGHT_UP);
			weightedMatrix[id].push_back(npair);

			pair < int, int > npair1 = make_pair((i)* width + j + 1, WEIGHT_RIGHT);
			weightedMatrix[id].push_back(npair1);

			pair < int, int > npair3 = make_pair((i)* width + j - 1, WEIGHT_LEFT);
			weightedMatrix[id].push_back(npair3);

		}
		else {

			int WEIGHT_DOWN = MAXIMUM_INTENSITY - ((GRADIENT_IMAGE.at < uchar >(i, j) + GRADIENT_IMAGE.at < uchar >(i + 1, j)) / 2);
			int WEIGHT_UP = MAXIMUM_INTENSITY - ((GRADIENT_IMAGE.at < uchar >(i, j) + GRADIENT_IMAGE.at < uchar >(i - 1, j)) / 2);
			int WEIGHT_LEFT = MAXIMUM_INTENSITY - ((GRADIENT_IMAGE.at < uchar >(i, j) + GRADIENT_IMAGE.at < uchar >(i, j - 1)) / 2);
			int WEIGHT_RIGHT = MAXIMUM_INTENSITY - ((GRADIENT_IMAGE.at < uchar >(i, j) + GRADIENT_IMAGE.at < uchar >(i, j + 1)) / 2);
			totalEdge += 4;
			totalWeight = totalWeight + WEIGHT_RIGHT + WEIGHT_LEFT + WEIGHT_UP + WEIGHT_DOWN;
			pair < int, int > npair = make_pair((i - 1) * width + j, WEIGHT_UP);
			weightedMatrix[id].push_back(npair);

			pair < int, int > npair1 = make_pair((i)* width + j + 1, WEIGHT_RIGHT);
			weightedMatrix[id].push_back(npair1);

			pair < int, int > npair2 = make_pair((i + 1) * width + j, WEIGHT_DOWN);
			weightedMatrix[id].push_back(npair2);

			pair < int, int > npair3 = make_pair((i)* width + j - 1, WEIGHT_LEFT);
			weightedMatrix[id].push_back(npair3);
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
				weightedMatrix[i][j].second = 1000000;
			}
		}
	}
	if (readConfigFile(argv[2]) == -1)
		return -1;
	int label;

	vector < int > path;
	// min cut max flow 
	int xyz = 0;
	bool flag = true;
	while (flag == true) {
		flag = false;
		while (!sourceQueue.empty()) {
		again: vector < int > currentPath;
			currentPath = sourceQueue.front();
			tempVisited[sourceQueue.front()[0]] = true;
			sourceQueue.pop();
			int last = currentPath.back();
			if (check2(target, last) == 1) {
				path.clear();
				path = currentPath;
				flag = true;
				initialMatrix1.clear();
				initialMatrix1 = initialMatrix2;
				//generateSourceQueue();
				for (int i = 0; i < totalPixel; i++) {
					tempVisited[i] = false;
				}
				break;
			}
			for (int i = 0; i < weightedMatrix[last].size(); ++i) {
				if (!tempVisited[weightedMatrix[last][i].first] && check(weightedMatrix[last][i].first, currentPath) && weightedMatrix[last][i].second > 0 && initialMatrix1[weightedMatrix[last][i].first].second == 0) {
					vector < int > opath(currentPath);
	
					opath.push_back(weightedMatrix[last][i].first);
					initialMatrix1[weightedMatrix[last][i].first].second = 1;
					tempVisited[weightedMatrix[last][i].first] = true;
					sourceQueue.push(opath);
				}
			}
		}
		if (flag == true) {
			int smallest = 99999999;
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
	int xxx = 0;
	for (int i = 0; i < totalPixel; i++) {
		if (!visited[i]) {
			xxx++;
		}
	}
	cout << "Total visited : " << xxx << endl;
	Vec3b pixel;
	pixel[0] = 255;
	pixel[1] = 255;
	pixel[2] = 255;
	queue < int > PIXEL_COLOR;
	int u = source[0];
	PIXEL_COLOR.push(u);
	visited[u] = true;
	OUTPUT_IMAGE.at < Vec3b >(u / width, u % width) = pixel;
	int mino = 99999999;
	//performing BFS
	while (!PIXEL_COLOR.empty()) {
		int last = PIXEL_COLOR.front();
		PIXEL_COLOR.pop();
		for (int i = 0; i < weightedMatrix[last].size(); ++i) {
			if (!visited[weightedMatrix[last][i].first] && initialMatrix2[weightedMatrix[last][i].first].second == 0 && weightedMatrix[last][i].second > 0) {
				Vec3b pixel2;
				pixel2[0] = 255; // blue color
				pixel2[1] = 255; // red color
				pixel2[2] = 255; // green color
				OUTPUT_IMAGE.at < Vec3b >((weightedMatrix[last][i].first) / width, (weightedMatrix[last][i].first) % width) = pixel2;
				visited[weightedMatrix[last][i].first] = true;
				PIXEL_COLOR.push(weightedMatrix[last][i].first);
				initialMatrix2[weightedMatrix[last][i].first].second = 1;
			}
		}
	}
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