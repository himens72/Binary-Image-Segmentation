/**
  * https://docs.opencv.org/2.4/doc/tutorials/imgproc/imgtrans/sobel_derivatives/sobel_derivatives.html#explanation
  *
  *
  *
  */
  //#include "pch.h"
#include <opencv2/opencv.hpp>
#include<fstream>
#include <utility>
#include<queue>
#include<vector>
#include <ctime>

using namespace cv;
using namespace std;
Mat INPUT_IMAGE;
Mat OUTPUT_IMAGE;
//vector < vector < pair < int, int > > > weightedMatrix;
//vector < pair < int, int > > initialMatrix1;
//vector < pair < int, int > > initialMatrix2;
Mat residualGraph;
Mat weightGraph;
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
// return true if pixel is  present Otherwise return false
bool isPixelExist(int pixel, vector < int > current_flow) {
	for (int i = 0; i < current_flow.size(); i++)
		if (current_flow[i] == pixel)
			return true;
	return false;
}
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
//	Vec3b pixel;
//	pixel[0] = 255;
//	pixel[1] = 255;
//	pixel[2] = 255;
//	queue < int > PIXEL_COLOR;
//	PIXEL_COLOR.push(source[0]);
//	visited[source[0]] = true;
//	OUTPUT_IMAGE.at < Vec3b >(source[0] / width, source[0] % width) = pixel;
//	//performing BFS
//	Mat initMat = Mat::zeros(totalPixel, 1, CV_64F);
//	while (!PIXEL_COLOR.empty()) {
//		int last = PIXEL_COLOR.front();
//		PIXEL_COLOR.pop();
///*
//		graph.at< int >(last, i)
//			weightGraph.at< int >(graph.at< int >(last, i), 0)
//*/
//		for (int i = 0; i < 4; ++i) {
//			if (graph.at< int >(last, i) != -1 && !visited[graph.at< int >(last, i)] && initMat.at < int >(graph.at< int >(last, i), 0) == 0 && weightGraph.at< int >(graph.at< int >(last, i), 0) > 0) {
//				Vec3b pixel;
//				pixel[0] = 255; // blue color
//				pixel[1] = 255; // red color
//				pixel[2] = 255; // green color
//				OUTPUT_IMAGE.at < Vec3b >(graph.at< int >(last, i) / width, graph.at< int >(last, i) % width) = pixel;
//				visited[graph.at< int >(last, i)] = true;
//				PIXEL_COLOR.push(graph.at< int >(last, i));
//				initMat.at < int >(graph.at< int >(last, i), 0) = 1;
//			}
//		}
//	}
}
void dfs(int s, bool *visited)
{
	visited[s] = true;
	for (int i = 0; i < width; i++)
		if (residualGraph.at <int >(s,i) && !visited[i])
			dfs(i, visited);
}
void minCut(int s, int t)
{
	int u, v;

	// Create a residual graph and fill the residual graph with 
	// given capacities in the original graph as residual capacities 
	// in residual graph 
	
	int *parent = new int[totalPixel];  // This array is filled by BFS and to store path 

	// Augment the flow while there is a path from source to sink 
	while (bfs(s, t, parent))
	{
		// Find minimum residual capacity of the edhes along the 
		// path filled by BFS. Or we can say find the maximum flow 
		// through the path found. 
		int path_flow = INT_MAX;
		for (v = t; v != s; v = parent[v])
		{
			u = parent[v];
			path_flow = min(path_flow, residualGraph.at < int >(u,v));
		}

		// update residual capacities of the edges and reverse edges 
		// along the path 
		for (v = t; v != s; v = parent[v])
		{
			u = parent[v];
			residualGraph.at < int >(u, v) -= path_flow;
			residualGraph.at < int >(u, v) += path_flow;
		}
	}

	// Flow is maximum now, find vertices reachable from s 
	bool *visited = new bool[totalPixel];
	memset(visited, false, sizeof(visited));
	dfs(s, visited);

	// Print all edges that are from a reachable vertex to 
	// non-reachable vertex in the original graph 
	for (int i = 0; i < height; i++)
		for (int j = 0; j < width; j++)
			if (visited[i] && !visited[j] && weightGraph.at < int >(i, j))
				cout << i << " - " << j << endl;

	return;
}
int bfs(int s, int t, int *parent)
{
	// Create a visited array and mark all vertices as not visited 
	bool *visited = new bool[totalPixel];
	memset(visited, 0, sizeof(visited));

	// Create a queue, enqueue source vertex and mark source vertex 
	// as visited 
	queue <int> q;
	q.push(s);
	visited[s] = true;
	parent[s] = -1;

	// Standard BFS Loop 
	while (!q.empty())
	{
		int u = q.front();
		q.pop();

		for (int v = 0; v < totalPixel ; v++)
		{
			if (visited[v] == false && residualGraph.at < int >(totalPixel / width, totalPixel % width) > 0)
			{
				q.push(v);
				parent[v] = u;
				visited[v] = true;
			}
		}
	}

	// If we reached sink in BFS starting from source, then return 
	// true, else false 
	return (visited[t] == true);
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
	time_t now;
	char* dt;

	generateGradientImage();
	width = INPUT_IMAGE.cols;
	height = INPUT_IMAGE.rows;
	totalPixel = height * width;
	calculateMaximumIntensity();
	visited = new bool[totalPixel];
	//weightedMatrix.resize(totalPixel);
	//initialMatrix1.resize(totalPixel);
	//initialMatrix2.resize(totalPixel);
	Mat tempVisited = Mat::zeros(totalPixel, 1, CV_64F);
	Mat initMat = Mat::zeros(totalPixel, 1, CV_64F);
	residualGraph = Mat::zeros(height+1, width, CV_64F);
	weightGraph = Mat::zeros(height+1, width, CV_64F);
	int totalEdge = 0;
	int averageWeight;
	int totalWeight = 0;
	now = time(0);
	dt = ctime(&now);
	cout << " Weight Calculation Start " << dt << endl;
	for (int index = 0; index < totalPixel; index++) {
		int i = index / width;
		int j = index % width;
		visited[index] = false;
		//initialMatrix1.push_back(make_pair(index, 0));
//		initialMatrix2.push_back(make_pair(index, 0));
		generateBackground(i, j);
		if (index == 0) {
			totalEdge += 2;
			int WEIGHT_RIGHT = MAXIMUM_INTENSITY - ((GRADIENT_IMAGE.at < uchar >(i, j) + GRADIENT_IMAGE.at < uchar >(i, j + 1)) / 2);
			int WEIGHT_DOWN = MAXIMUM_INTENSITY - ((GRADIENT_IMAGE.at < uchar >(i, j) + GRADIENT_IMAGE.at < uchar >(i + 1, j)) / 2);
			weightGraph.at < int >(i, j) = WEIGHT_RIGHT;
			weightGraph.at < int >(i, j) = WEIGHT_DOWN;
			residualGraph.at < int >(i, j) = WEIGHT_RIGHT;
			residualGraph.at < int >(i, j) = WEIGHT_DOWN;
			totalWeight = totalWeight + WEIGHT_DOWN + WEIGHT_RIGHT;
		}
		else if (index == width - 1) {
			totalEdge += 2;
			int WEIGHT_DOWN = MAXIMUM_INTENSITY - ((GRADIENT_IMAGE.at < uchar >(i, j) + GRADIENT_IMAGE.at < uchar >(i + 1, j)) / 2);
			int WEIGHT_LEFT = MAXIMUM_INTENSITY - ((GRADIENT_IMAGE.at < uchar >(i, j) + GRADIENT_IMAGE.at < uchar >(i, j - 1)) / 2);
			weightGraph.at < int >(i, j) = WEIGHT_LEFT;
			weightGraph.at < int >(i, j) = WEIGHT_DOWN;
			residualGraph.at < int >(i, j) = WEIGHT_LEFT;
			residualGraph.at < int >(i, j) = WEIGHT_DOWN;
			totalWeight = totalWeight + WEIGHT_DOWN + WEIGHT_LEFT;
		}
		else if (index == width * (height - 1)) {
			totalEdge += 2;
			int WEIGHT_UP = MAXIMUM_INTENSITY - ((GRADIENT_IMAGE.at < uchar >(i, j) + GRADIENT_IMAGE.at < uchar >(i - 1, j)) / 2);
			int WEIGHT_RIGHT = MAXIMUM_INTENSITY - ((GRADIENT_IMAGE.at < uchar >(i, j) + GRADIENT_IMAGE.at < uchar >(i, j + 1)) / 2);
			weightGraph.at < int >(i, j) = WEIGHT_UP;
			weightGraph.at < int >(i, j) = WEIGHT_RIGHT;
			residualGraph.at < int >(i, j) = WEIGHT_UP;
			residualGraph.at < int >(i, j) = WEIGHT_RIGHT;
			totalWeight = totalWeight + WEIGHT_UP + WEIGHT_RIGHT;
		}
		else if (index == totalPixel - 1) {
			totalEdge += 2;
			int WEIGHT_LEFT = MAXIMUM_INTENSITY - ((GRADIENT_IMAGE.at < uchar >(i, j) + GRADIENT_IMAGE.at < uchar >(i, j - 1)) / 2);
			int WEIGHT_UP = MAXIMUM_INTENSITY - ((GRADIENT_IMAGE.at < uchar >(i, j) + GRADIENT_IMAGE.at < uchar >(i - 1, j)) / 2);
			weightGraph.at < int >(i, j) = WEIGHT_UP;
			weightGraph.at < int >(i, j) = WEIGHT_LEFT;
			residualGraph.at < int >(i, j) = WEIGHT_UP;
			residualGraph.at < int >(i, j) = WEIGHT_LEFT;
			totalWeight = totalWeight + WEIGHT_UP + WEIGHT_LEFT;
		}
		else if (index / width == 0) {
			totalEdge += 3;
			int WEIGHT_DOWN = MAXIMUM_INTENSITY - ((GRADIENT_IMAGE.at < uchar >(i, j) + GRADIENT_IMAGE.at < uchar >(i + 1, j)) / 2);
			int WEIGHT_RIGHT = MAXIMUM_INTENSITY - ((GRADIENT_IMAGE.at < uchar >(i, j) + GRADIENT_IMAGE.at < uchar >(i, j + 1)) / 2);
			int WEIGHT_LEFT = MAXIMUM_INTENSITY - ((GRADIENT_IMAGE.at < uchar >(i, j) + GRADIENT_IMAGE.at < uchar >(i, j - 1)) / 2);
			weightGraph.at < int >(i, j) = WEIGHT_DOWN;
			weightGraph.at < int >(i, j) = WEIGHT_LEFT;
			weightGraph.at < int >(i, j) = WEIGHT_RIGHT;
			residualGraph.at < int >(i, j) = WEIGHT_DOWN;
			residualGraph.at < int >(i, j) = WEIGHT_LEFT;
			residualGraph.at < int >(i, j) = WEIGHT_RIGHT;
			totalWeight = totalWeight + WEIGHT_DOWN + WEIGHT_LEFT + WEIGHT_RIGHT;
		}
		else if ((index + 1) % width == 0) {
			totalEdge += 3;
			int WEIGHT_UP = MAXIMUM_INTENSITY - ((GRADIENT_IMAGE.at < uchar >(i, j) + GRADIENT_IMAGE.at < uchar >(i - 1, j)) / 2);
			int WEIGHT_DOWN = MAXIMUM_INTENSITY - ((GRADIENT_IMAGE.at < uchar >(i, j) + GRADIENT_IMAGE.at < uchar >(i + 1, j)) / 2);
			int WEIGHT_LEFT = MAXIMUM_INTENSITY - ((GRADIENT_IMAGE.at < uchar >(i, j) + GRADIENT_IMAGE.at < uchar >(i, j - 1)) / 2);
			weightGraph.at < int >(i, j) = WEIGHT_UP;
			weightGraph.at < int >(i, j) = WEIGHT_LEFT;
			weightGraph.at < int >(i, j) = WEIGHT_DOWN;
			residualGraph.at < int >(i, j) = WEIGHT_UP;
			residualGraph.at < int >(i, j) = WEIGHT_LEFT;
			residualGraph.at < int >(i, j) = WEIGHT_DOWN;
			totalWeight = totalWeight + WEIGHT_DOWN + WEIGHT_LEFT + WEIGHT_UP;
		}
		else if (index % width == 0) {
			totalEdge += 3;
			int WEIGHT_UP = MAXIMUM_INTENSITY - ((GRADIENT_IMAGE.at < uchar >(i, j) + GRADIENT_IMAGE.at < uchar >(i - 1, j)) / 2);
			int WEIGHT_RIGHT = MAXIMUM_INTENSITY - ((GRADIENT_IMAGE.at < uchar >(i, j) + GRADIENT_IMAGE.at < uchar >(i, j + 1)) / 2);
			int WEIGHT_DOWN = MAXIMUM_INTENSITY - ((GRADIENT_IMAGE.at < uchar >(i, j) + GRADIENT_IMAGE.at < uchar >(i + 1, j)) / 2);
			weightGraph.at < int >(i, j) = WEIGHT_UP;
			weightGraph.at < int >(i, j) = WEIGHT_RIGHT;
			weightGraph.at < int >(i, j) = WEIGHT_DOWN;
			residualGraph.at < int >(i, j) = WEIGHT_UP;
			residualGraph.at < int >(i, j) = WEIGHT_RIGHT;
			residualGraph.at < int >(i, j) = WEIGHT_DOWN;
			totalWeight = totalWeight + WEIGHT_DOWN + WEIGHT_RIGHT + WEIGHT_UP;
		}
		else if (index > (width * (height - 1))) {
			totalEdge += 3;
			int WEIGHT_UP = MAXIMUM_INTENSITY - ((GRADIENT_IMAGE.at < uchar >(i, j) + GRADIENT_IMAGE.at < uchar >(i - 1, j)) / 2);
			int WEIGHT_RIGHT = MAXIMUM_INTENSITY - ((GRADIENT_IMAGE.at < uchar >(i, j) + GRADIENT_IMAGE.at < uchar >(i, j + 1)) / 2);
			int WEIGHT_LEFT = MAXIMUM_INTENSITY - ((GRADIENT_IMAGE.at < uchar >(i, j) + GRADIENT_IMAGE.at < uchar >(i, j - 1)) / 2);
			weightGraph.at < int >(i, j) = WEIGHT_UP;
			weightGraph.at < int >(i, j) = WEIGHT_RIGHT;
			weightGraph.at < int >(i, j) = WEIGHT_LEFT;
			residualGraph.at < int >(i, j) = WEIGHT_UP;
			residualGraph.at < int >(i, j) = WEIGHT_RIGHT;
			residualGraph.at < int >(i, j) = WEIGHT_LEFT;
			totalWeight = totalWeight + WEIGHT_RIGHT + WEIGHT_LEFT + WEIGHT_UP;
		}
		else {
			totalEdge += 4;
			int WEIGHT_UP = MAXIMUM_INTENSITY - ((GRADIENT_IMAGE.at < uchar >(i, j) + GRADIENT_IMAGE.at < uchar >(i - 1, j)) / 2);
			int WEIGHT_RIGHT = MAXIMUM_INTENSITY - ((GRADIENT_IMAGE.at < uchar >(i, j) + GRADIENT_IMAGE.at < uchar >(i, j + 1)) / 2);
			int WEIGHT_DOWN = MAXIMUM_INTENSITY - ((GRADIENT_IMAGE.at < uchar >(i, j) + GRADIENT_IMAGE.at < uchar >(i + 1, j)) / 2);
			int WEIGHT_LEFT = MAXIMUM_INTENSITY - ((GRADIENT_IMAGE.at < uchar >(i, j) + GRADIENT_IMAGE.at < uchar >(i, j - 1)) / 2);
			weightGraph.at < int >(i, j) = WEIGHT_UP;
			weightGraph.at < int >(i, j) = WEIGHT_RIGHT;
			weightGraph.at < int >(i, j) = WEIGHT_LEFT;
			weightGraph.at < int >(i, j) = WEIGHT_DOWN;
			residualGraph.at < int >(i, j) = WEIGHT_DOWN;
			residualGraph.at < int >(i, j) = WEIGHT_UP;
			residualGraph.at < int >(i, j) = WEIGHT_RIGHT;
			residualGraph.at < int >(i, j) = WEIGHT_LEFT;
			totalWeight = totalWeight + WEIGHT_RIGHT + WEIGHT_LEFT + WEIGHT_UP + WEIGHT_DOWN;
		}
	}
	now = time(0);
	dt = ctime(&now);
	cout << " Weight Calculation end " << dt << endl;
	averageWeight = totalWeight / totalEdge;
	cout << "totalEdge : " << totalEdge << endl;
	cout << "totalWeight : " << totalWeight << endl;
	cout << "averageWeight : " << averageWeight << endl;
	for (int i = 0; i < height; i++) {
		for (int j = 0; j < width; j++) {
			if (weightGraph.at< int >(i,j) < averageWeight) {
				weightGraph.at< int >(i, j) = 1;
				residualGraph.at< int >(i, j) = 1;
			}
			else {
				weightGraph.at< int >(i, j) = 1000;
				residualGraph.at< int >(i, j) = 1000;
			}
		}
	}
	if (readConfigFile(argv[2]) == -1)
		return -1;
	now = time(0);
	dt = ctime(&now);
	cout << " Min Cut Start " << dt << endl;
	for (int i = 0; i < source.size(); i++) {
		for (int j = 0; j < sink.size(); j++) {
			minCut(i, j);
		}
	}
	now = time(0);
	dt = ctime(&now);
	cout << " Min Cut end " << dt << endl;
	now = time(0);
	dt = ctime(&now);
	cout << " BFS Start " << dt << endl;
	performBFS();
	now = time(0);
	dt = ctime(&now);
	cout << " BFS wnd " << dt << endl;
	imwrite(argv[3], OUTPUT_IMAGE);
	namedWindow("Input Image", WINDOW_AUTOSIZE);
	namedWindow("Output Image", WINDOW_AUTOSIZE);
	imshow("Input Image", INPUT_IMAGE);
	imshow("Output Image", OUTPUT_IMAGE);
	waitKey(0);
	return 0;
}