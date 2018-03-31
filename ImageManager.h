//
// Created by lydakis on 3/31/18.
//

#ifndef EXERCISE_AEO2881_IMAGEMANAGER_H
#define EXERCISE_AEO2881_IMAGEMANAGER_H

#include "opencv2/opencv_modules.hpp"
# include "opencv2/core/core.hpp"
# include "opencv2/features2d/features2d.hpp"
# include "opencv2/highgui/highgui.hpp"
# include "opencv2/nonfree/features2d.hpp"

#include <iostream>
#include <fstream>
#include <algorithm>
#include <stdexcept>
#include <map>
#include <list>
#include <sstream>
#include <iterator>
#include <utility>
#include <climits>
#include <valarray>
#include "cxxopts.hpp"

using namespace cv;

class ImageManager {
private:
//    int dx[] = {1, -1, 0, 0, 1, -1, -1, 1};
//    int dy[] = {0, 0, 1, -1, 1, -1, 1, -1};
    vector<int> dx = {1, -1, 0, 0};
    vector<int> dy = {0, 0, 1, -1};
    const string _window_name = "Image Manager";
    Mat src;
	
	static void onClick(int event, int x, int y, int flags, void* param)
	{	
		std::cout<<x<<" "<<y<<std::endl;
		ImageManager *im_this = static_cast<ImageManager*>( param);
		 if  ( event == EVENT_LBUTTONDOWN )
		 {
			 Point event_source(x,y);
			 im_this->FIND_REGION(im_this->src, event_source, 2, 2, 2);
		 }
		 
	}
	
public:
    ImageManager(const std::string image_name, int dis = 1) {
		namedWindow(_window_name, WINDOW_AUTOSIZE);
		src = imread(image_name.c_str(), IMREAD_COLOR);
		cv::setMouseCallback(_window_name, &ImageManager::onClick, this);
		if(dis==1){
			DISPLAY_IMAGE(src);
		}
    }


    //BASIC SIMILARITY FUNCTION
    //TODO probably replace it with a HSV comparison
    bool is_similar(const Vec3b &p1, const Vec3b &p2, int b_threshold, int r_threshold, int g_threshold) {
        return (abs(p1[0] - p2[0] <= b_threshold) &&
                abs(p1[1] - p2[1] <= g_threshold) &&
                abs(p1[2] - p2[2] <= r_threshold));

    }

    static bool compareX(Point lhs, Point rhs) { return lhs.x < rhs.x; };

    static bool compareY(Point lhs, Point rhs) { return lhs.y < rhs.y; };

    bool IsInBounds(int r, int c, int rows, int columns) {
        if (r < 0)return false;
        if (c < 0)return false;
        if (r >= rows)return false;
        if (c >= columns)return false;
        return true;
    }

    vector<Point>
    FIND_REGION(const Mat image, const Point &point, int b_threshold = 2, int r_threshold = 2, int g_threshold = 2) {
        assert((b_threshold >= 0) && (b_threshold <= 255));
        assert((r_threshold >= 0) && (r_threshold <= 255));
        assert((g_threshold >= 0) && (g_threshold <= 255));
        std::cout<<"Find Region "<<point.x<<" "<<point.y<<std::endl;
        Mat image_copy(image.size(), CV_8UC3, 0);
        //create point from initial given point
        Point target(point.x, point.y);
        //create a stack for floodfill
        std::stack<Point> stack;
        stack.push(target);
        //vector that will hold the similar points
        vector<Point> similar_points;
        //flood fill and get similar points
        while (!stack.empty()) {
            Point cur = stack.top();
            if (is_similar(image.at<cv::Vec3b>(point.y, point.x), image.at<cv::Vec3b>(cur.y, cur.x), b_threshold,
                           r_threshold, g_threshold)) {
                similar_points.emplace_back(cur);
                int row = cur.x;
                int col = cur.y;
                for (int dx_: dx) {
                    for (auto dy_ :dy) {
                        if (IsInBounds(row + dy_, col + dx_, image.rows, image.cols)) {

                        }
                    }
                }
            }
            stack.pop();
        }
        std::pair <vector<Point>::iterator, vector<Point>::iterator> xExtremes, yExtremes;
        xExtremes = std::minmax_element(similar_points.begin(), similar_points.end(), compareX);
        yExtremes = std::minmax_element(similar_points.begin(), similar_points.end(), compareY);
        Point upperLeft(xExtremes.first->x, yExtremes.first->y);
        Point lowerRight(xExtremes.second->x, yExtremes.second->y);
        vector<Point> bounding_box_edges;
        bounding_box_edges.emplace_back(upperLeft);
        bounding_box_edges.emplace_back(lowerRight);
        return bounding_box_edges;

    }

    Mat FIND_PIXELS(const Mat) {}

    vector<Point> FIND_PERIMNETER(vector<Point> region) {
        vector<Point> perimeter;
		Point upper_left(region[0]);
		Point lower_right(region[1]);
		Point upper_right(region[1].x, region[0].y);
		Point lower_left(region[0].x, region[1].y);
		perimeter.emplace_back(upper_left);
		perimeter.emplace_back(lower_left);
		perimeter.emplace_back(lower_right);
		perimeter.emplace_back(upper_right);
        return perimeter;
    }

    void DISPLAY_IMAGE(const Mat image) {
        try {
            
            std::cout << "Press 'c' to continue\n";
            while(true){
				imshow(_window_name, image);
				int k = waitKey(0);
				if ( k==27 ) break;
			}
        } catch (int e) {
            std::cout << "Could not display Image\n";
            return;
        }
    }

    void DISPLAY_PIXELS(const Mat image) {

    }

    void SAVE_PIXELS(const Mat &image) {

    }
};

#endif //EXERCISE_AEO2881_IMAGEMANAGER_H
