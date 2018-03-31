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

class ExerciseManager {
private:
//    int dx[] = {1, -1, 0, 0, 1, -1, -1, 1};
//    int dy[] = {0, 0, 1, -1, 1, -1, 1, -1};
    vector<int> dx = {1, -1, 0, 0};
    vector<int> dy = {0, 0, 1, -1};
    char *_window_name;

public:
    ExerciseManager(char *window_name) {
        _window_name = window_name;
    }


    //BASIC SIMILARITY FUNCTION
    //TODO probably replace it with a HSV comparison
    bool is_similar(const Vec3b &p1, const Vec3b &p2, int b_threshold, int r_threshold, int g_threshold) {
        return (abs(p1[0] - p2[0] <= b_threshold) &&
                abs(p1[1] - p2[1] <= g_threshold) &&
                abs(p1[2] - p2[2] <= r_threshold));

    }

    bool IsInBounds(int r, int c, int rows, int columns) {
        if (r < 0)return false;
        if (c < 0)return false;
        if (r >= rows)return false;
        if (c >= columns)return false;
        return true;
    }

    Mat
    FIND_REGION(const Mat image, const Point &point, int b_threshold = 10, int r_threshold = 10, int g_threshold = 10) {
        assert((b_threshold >= 0) && (b_threshold <= 255));
        assert((r_threshold >= 0) && (r_threshold <= 255));
        assert((g_threshold >= 0) && (g_threshold <= 255));
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

    }

    Mat FIND_PIXELS(const Mat) {}

    void DISPLAY_IMAGE(const Mat image) {
        try {
            namedWindow(_window_name, WINDOW_AUTOSIZE);
            imshow(_window_name, image);
            std::cout << "Press any key to continue\n";
            waitKey(0);
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
