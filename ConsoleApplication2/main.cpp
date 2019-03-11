/** Himen Sidhpura 40091993
  * Reference : https://docs.opencv.org/2.4/doc/tutorials/imgproc/imgtrans/sobel_derivatives/sobel_derivatives.html#explanation
	        https://docs.opencv.org/2.4.13.7/doc/tutorials/core/mat_the_basic_image_container/mat_the_basic_image_container.html
		https://www.geeksforgeeks.org/minimum-cut-in-a-directed-graph/
  *
  *
  *
  */
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
vector < int > source;
vector < int > sink;
queue < vector < int > > sourceQueue;
// This Method is used to check whether pixel is exist in current flow or not
// This method is used to identify whether currentPixel is sink or not
bool isPixelSink(int  pixel) {
	for (int i = 0; i < sink.size(); i++) {
		if (sink[i] == pixel)
			return true;
	}
	return false;
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
		int w;
		int h;
		int marker;
		file >> w >> h >> marker;
		if (w < 0 || w >= width || h < 0 || h >= height) {
			cout << "Invalid pixel in config file" << endl;
			return -1;
		}
		if (marker == 1) {
			source.push_back((width * h) + w);
			vector < int > temp;
			temp.push_back((width * h) + w);
			sourceQueue.push(temp);
		}
		else {
			sink.push_back((width * h) + w);
		}
	}
	return 0;
}
void generateSourceQueue() {
	while (!sourceQueue.empty()) {
		sourceQueue.pop();
	}
	for (int i = 0; i < source.size(); i++) {
		vector < int > temp;
		temp.push_back(source[i]);
		sourceQueue.push(temp);
	}
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
	while (!PIXEL_COLOR.empty()) {
		int last = PIXEL_COLOR.front();
		PIXEL_COLOR.pop();
		for (int i = 0; i < weightedMatrix[last].size(); ++i) {
			if (!visited[weightedMatrix[last][i].first] && weightedMatrix[last][i].second > 0) {
				Vec3b pixel;
				pixel[0] = 255; // blue color
				pixel[1] = 255; // red color
				pixel[2] = 255; // green color
				OUTPUT_IMAGE.at < Vec3b >((weightedMatrix[last][i].first) / width, (weightedMatrix[last][i].first) % width) = pixel;
				visited[weightedMatrix[last][i].first] = true;
				PIXEL_COLOR.push(weightedMatrix[last][i].first);
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
	width = INPUT_IMAGE.cols;
	height = INPUT_IMAGE.rows;
	totalPixel = height * width;
	calculateMaximumIntensity();
	visited = new bool[totalPixel];
	weightedMatrix.resize(totalPixel);
	Mat tempVisited = Mat::zeros(totalPixel, 1, CV_64F);
	int totalEdge = 0;
	int averageWeight;
	int totalWeight = 0;
	for (int i = 0; i < height; i++) {
	for (int j = 0; j < width; j++) {
		int index = (width * i) +j;
		visited[index] = false;
		generateBackground(i, j);
		if (index == 0) {
			totalEdge += 2;
			int WEIGHT_RIGHT = MAXIMUM_INTENSITY - ((GRADIENT_IMAGE.at < uchar >(i, j) + GRADIENT_IMAGE.at < uchar >(i, j + 1)) / 2);
			weightedMatrix[index].push_back(make_pair((i)* width + j + 1, WEIGHT_RIGHT));
			int WEIGHT_DOWN = MAXIMUM_INTENSITY - ((GRADIENT_IMAGE.at < uchar >(i, j) + GRADIENT_IMAGE.at < uchar >(i + 1, j)) / 2);
			weightedMatrix[index].push_back(make_pair((i + 1) * width + j, WEIGHT_DOWN));
			totalWeight = totalWeight + WEIGHT_DOWN + WEIGHT_RIGHT;
		}
		else if (index == width - 1) {
			totalEdge += 2;
			int WEIGHT_DOWN = MAXIMUM_INTENSITY - ((GRADIENT_IMAGE.at < uchar >(i, j) + GRADIENT_IMAGE.at < uchar >(i + 1, j)) / 2);
			weightedMatrix[index].push_back(make_pair((i + 1) * width + j, WEIGHT_DOWN));
			int WEIGHT_LEFT = MAXIMUM_INTENSITY - ((GRADIENT_IMAGE.at < uchar >(i, j) + GRADIENT_IMAGE.at < uchar >(i, j - 1)) / 2);
			weightedMatrix[index].push_back(make_pair((i)* width + j - 1, WEIGHT_LEFT));
			totalWeight = totalWeight + WEIGHT_DOWN + WEIGHT_LEFT;
		}
		else if (index == width * (height - 1)) {
			totalEdge += 2;
			int WEIGHT_UP = MAXIMUM_INTENSITY - ((GRADIENT_IMAGE.at < uchar >(i, j) + GRADIENT_IMAGE.at < uchar >(i - 1, j)) / 2);
			weightedMatrix[index].push_back(make_pair((i - 1) * width + j, WEIGHT_UP));
			int WEIGHT_RIGHT = MAXIMUM_INTENSITY - ((GRADIENT_IMAGE.at < uchar >(i, j) + GRADIENT_IMAGE.at < uchar >(i, j + 1)) / 2);
			weightedMatrix[index].push_back(make_pair((i)* width + j + 1, WEIGHT_RIGHT));
			totalWeight = totalWeight + WEIGHT_UP + WEIGHT_RIGHT;
		}
		else if (index == totalPixel - 1) {
			totalEdge += 2;
			int WEIGHT_LEFT = MAXIMUM_INTENSITY - ((GRADIENT_IMAGE.at < uchar >(i, j) + GRADIENT_IMAGE.at < uchar >(i, j - 1)) / 2);
			weightedMatrix[index].push_back(make_pair((i)* width + j - 1, WEIGHT_LEFT));
			int WEIGHT_UP = MAXIMUM_INTENSITY - ((GRADIENT_IMAGE.at < uchar >(i, j) + GRADIENT_IMAGE.at < uchar >(i - 1, j)) / 2);
			weightedMatrix[index].push_back(make_pair((i - 1) * width + j, WEIGHT_UP));
			totalWeight = totalWeight + WEIGHT_UP + WEIGHT_LEFT;
		}
		else if (index / width == 0) {
			totalEdge += 3;
			int WEIGHT_DOWN = MAXIMUM_INTENSITY - ((GRADIENT_IMAGE.at < uchar >(i, j) + GRADIENT_IMAGE.at < uchar >(i + 1, j)) / 2);
			weightedMatrix[index].push_back(make_pair((i + 1) * width + j, WEIGHT_DOWN));
			int WEIGHT_RIGHT = MAXIMUM_INTENSITY - ((GRADIENT_IMAGE.at < uchar >(i, j) + GRADIENT_IMAGE.at < uchar >(i, j + 1)) / 2);
			weightedMatrix[index].push_back(make_pair((i)* width + j + 1, WEIGHT_RIGHT));
			int WEIGHT_LEFT = MAXIMUM_INTENSITY - ((GRADIENT_IMAGE.at < uchar >(i, j) + GRADIENT_IMAGE.at < uchar >(i, j - 1)) / 2);
			weightedMatrix[index].push_back(make_pair((i)* width + j - 1, WEIGHT_LEFT));
			totalWeight = totalWeight + WEIGHT_DOWN + WEIGHT_LEFT + WEIGHT_RIGHT;
		}
		else if ((index + 1) % width == 0) {
			totalEdge += 3;
			int WEIGHT_UP = MAXIMUM_INTENSITY - ((GRADIENT_IMAGE.at < uchar >(i, j) + GRADIENT_IMAGE.at < uchar >(i - 1, j)) / 2);
			weightedMatrix[index].push_back(make_pair((i - 1) * width + j, WEIGHT_UP));
			int WEIGHT_DOWN = MAXIMUM_INTENSITY - ((GRADIENT_IMAGE.at < uchar >(i, j) + GRADIENT_IMAGE.at < uchar >(i + 1, j)) / 2);
			weightedMatrix[index].push_back(make_pair((i + 1) * width + j, WEIGHT_DOWN));
			int WEIGHT_LEFT = MAXIMUM_INTENSITY - ((GRADIENT_IMAGE.at < uchar >(i, j) + GRADIENT_IMAGE.at < uchar >(i, j - 1)) / 2);
			weightedMatrix[index].push_back(make_pair((i)* width + j - 1, WEIGHT_LEFT));
			totalWeight = totalWeight + WEIGHT_DOWN + WEIGHT_LEFT + WEIGHT_UP;
		}
		else if (index % width == 0) {
			totalEdge += 3;
			int WEIGHT_UP = MAXIMUM_INTENSITY - ((GRADIENT_IMAGE.at < uchar >(i, j) + GRADIENT_IMAGE.at < uchar >(i - 1, j)) / 2);
			weightedMatrix[index].push_back(make_pair((i - 1) * width + j, WEIGHT_UP));
			int WEIGHT_RIGHT = MAXIMUM_INTENSITY - ((GRADIENT_IMAGE.at < uchar >(i, j) + GRADIENT_IMAGE.at < uchar >(i, j + 1)) / 2);
			weightedMatrix[index].push_back(make_pair((i)* width + j + 1, WEIGHT_RIGHT));
			int WEIGHT_DOWN = MAXIMUM_INTENSITY - ((GRADIENT_IMAGE.at < uchar >(i, j) + GRADIENT_IMAGE.at < uchar >(i + 1, j)) / 2);
			weightedMatrix[index].push_back(make_pair((i + 1) * width + j, WEIGHT_DOWN));
			totalWeight = totalWeight + WEIGHT_DOWN + WEIGHT_RIGHT + WEIGHT_UP;
		}
		else if (index > (width * (height - 1))) {
			totalEdge += 3;
			int WEIGHT_UP = MAXIMUM_INTENSITY - ((GRADIENT_IMAGE.at < uchar >(i, j) + GRADIENT_IMAGE.at < uchar >(i - 1, j)) / 2);
			weightedMatrix[index].push_back(make_pair((i - 1) * width + j, WEIGHT_UP));
			int WEIGHT_RIGHT = MAXIMUM_INTENSITY - ((GRADIENT_IMAGE.at < uchar >(i, j) + GRADIENT_IMAGE.at < uchar >(i, j + 1)) / 2);
			weightedMatrix[index].push_back(make_pair((i)* width + j + 1, WEIGHT_RIGHT));
			int WEIGHT_LEFT = MAXIMUM_INTENSITY - ((GRADIENT_IMAGE.at < uchar >(i, j) + GRADIENT_IMAGE.at < uchar >(i, j - 1)) / 2);
			weightedMatrix[index].push_back(make_pair((i)* width + j - 1, WEIGHT_LEFT));
			totalWeight = totalWeight + WEIGHT_RIGHT + WEIGHT_LEFT + WEIGHT_UP;
		}
		else {
			totalEdge += 4;
			int WEIGHT_UP = MAXIMUM_INTENSITY - ((GRADIENT_IMAGE.at < uchar >(i, j) + GRADIENT_IMAGE.at < uchar >(i - 1, j)) / 2);
			weightedMatrix[index].push_back(make_pair((i - 1) * width + j, WEIGHT_UP));
			int WEIGHT_RIGHT = MAXIMUM_INTENSITY - ((GRADIENT_IMAGE.at < uchar >(i, j) + GRADIENT_IMAGE.at < uchar >(i, j + 1)) / 2);
			weightedMatrix[index].push_back(make_pair((i)* width + j + 1, WEIGHT_RIGHT));
			int WEIGHT_DOWN = MAXIMUM_INTENSITY - ((GRADIENT_IMAGE.at < uchar >(i, j) + GRADIENT_IMAGE.at < uchar >(i + 1, j)) / 2);
			weightedMatrix[index].push_back(make_pair((i + 1) * width + j, WEIGHT_DOWN));
			int WEIGHT_LEFT = MAXIMUM_INTENSITY - ((GRADIENT_IMAGE.at < uchar >(i, j) + GRADIENT_IMAGE.at < uchar >(i, j - 1)) / 2);
			weightedMatrix[index].push_back(make_pair((i)* width + j - 1, WEIGHT_LEFT));
			totalWeight = totalWeight + WEIGHT_RIGHT + WEIGHT_LEFT + WEIGHT_UP + WEIGHT_DOWN;
		}
	}
}
	averageWeight = totalWeight / totalEdge;
	for (int i = 0; i < totalPixel; i++) {
		for (int j = 0; j < weightedMatrix[i].size(); j++) {
			if (weightedMatrix[i][j].second < averageWeight) {
				weightedMatrix[i][j].second = 1;
			}
			else {
				weightedMatrix[i][j].second = 256;
			}
		}
	}
	if (readConfigFile(argv[2]) == -1)
		return -1;
	vector < int > path_flow;
		path_flow.resize(totalPixel);
bool flag = true;
	while (flag) {
		flag = false;
		path_flow.clear();
		while (!sourceQueue.empty()) {
			path_flow = sourceQueue.front();
			tempVisited.at <int >(sourceQueue.front()[0], 0) = true;
			sourceQueue.pop();
			int last = path_flow.back();
			if (isPixelSink(last)) {
				flag = true;
				generateSourceQueue();			
				tempVisited = Mat::zeros(totalPixel, 1, CV_64F);
				break;
			}
			for (int i = 0; i < weightedMatrix[last].size(); ++i) {
				int adjacentPixel = weightedMatrix[last][i].first;
				int adjacentWeight = weightedMatrix[last][i].second;
				if (tempVisited.at <int >(adjacentPixel, 0) == 0  && adjacentWeight > 0) {
					vector < int > new_flow(path_flow);
					new_flow.push_back(adjacentPixel);
					tempVisited.at <int >(adjacentPixel, 0) = 1;
					sourceQueue.push(new_flow);
				}
			}
		}
		if (flag == true) {
			int smallest = 256;
			for (int i = 0; i < path_flow.size() - 1; i++) {
				for (int j = 0; j < weightedMatrix[path_flow[i]].size(); j++) {
					if (weightedMatrix[path_flow[i]][j].first == path_flow[i + 1]) {
						if (weightedMatrix[path_flow[i]][j].second < smallest) {
							smallest = weightedMatrix[path_flow[i]][j].second;
						}
					}
				}
			}
			for (int i = 0; i < path_flow.size() - 1; i++) {
				for (int j = 0; j < weightedMatrix[path_flow[i]].size(); j++) {
					if (weightedMatrix[path_flow[i]][j].first == path_flow[i + 1]) {
						weightedMatrix[path_flow[i]][j].second -=  smallest;
					}
				}
			}
			for (int i = 0; i < path_flow.size() - 1; i++) {
				for (int j = 0; j < weightedMatrix[path_flow[i + 1]].size(); j++) {
					if (weightedMatrix[path_flow[i + 1]][j].first == path_flow[i]) {
						weightedMatrix[path_flow[i + 1]][j].second += smallest;
					}
				}
			}
		}
	}
	performBFS();
	imwrite(argv[3], OUTPUT_IMAGE);
	namedWindow("Input Image", WINDOW_AUTOSIZE);
	namedWindow("Output Image", WINDOW_AUTOSIZE);
	imshow("Input Image", INPUT_IMAGE);
	imshow("Output Image", OUTPUT_IMAGE);
	waitKey(0);
	return 0;
}
