//
// Created by lydakis on 3/31/18.
//

#ifndef EXERCISE_AEO2881_IMAGEMANAGER_H
#define EXERCISE_AEO2881_IMAGEMANAGER_H

#include "opencv2/opencv_modules.hpp"
# include "opencv2/core/core.hpp"
# include "opencv2/features2d/features2d.hpp"
# include "opencv2/highgui/highgui.hpp"

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
    //Stores all the points in the region of interest
    vector<Point> region_of_interest;
    //Stores all the points in the bounding box of the roi
    vector<Point> bounding_box;

    //Callback to get click coordinates and call FIND_REGION on theses coordinates
    static void onClick(int event, int x, int y, int flags, void *param) {
        //std::cout<<x<<" "<<y<<std::endl;
        ImageManager *im_this = static_cast<ImageManager *>( param);
        //std::cout<<im_this->src.rows<<std::endl;
        //std::cout<<im_this->src.cols<<std::endl;
        if (event == EVENT_LBUTTONDOWN) {
            Point event_source(x, y);
            im_this->FIND_REGION(im_this->src, event_source, 2, 2, 2);
        }

    }

public:

    ImageManager() {

    }

    /**
     *
     * @param image_name the path of the image to load as a string
     * @param dis 1 if you want to display the image immediately, 0 otherwise
     */
    ImageManager(const std::string image_name) {
//        namedWindow(_window_name, WINDOW_AUTOSIZE);
        src = imread(image_name.c_str(), CV_LOAD_IMAGE_COLOR);
        if (!src.data) {
            std::cout << "No image read in constructor\n";
            return;
        }
    }


    /**
     * Changes the image that is being used
     * @param image_name the image name to use as a source from now on
     */
    void changeSrc(const std::string image_name) {
        src = imread(image_name.c_str());
        if (!src.data) {
            std::cout << "No image read\n";
            return;
        }
    }

    //Close the plot window
    void closeWindow() {
        destroyWindow(_window_name);
    }

    //BASIC SIMILARITY FUNCTION
    //Checks if two point rgb values are with in allowed thresholds
    //TODO probably replace it with a HSV comparison
    bool is_similar(cv::Vec3b p1, cv::Vec3b p2, int b_threshold, int r_threshold, int g_threshold) {
        //std::cout<<(int)p1.val[0]<<" "<<(int)p1.val[1]<<" "<<(int)p1.val[2]<<std::endl;
        //std::cout<<(int)p2.val[0]<<" "<<(int)p2.val[1]<<" "<<(int)p2.val[2]<<std::endl;
        return ((abs((int) p1[0] - (int) p2[0]) <= b_threshold) &&
                (abs((int) p1[1] - (int) p2[1]) <= g_threshold) &&
                (abs((int) p1[2] - (int) p2[2]) <= r_threshold));

    }

    //Boundary Check x coordinates
    static bool compareX(Point lhs, Point rhs) { return lhs.x < rhs.x; };

    //Boundary Check y coordinates
    static bool compareY(Point lhs, Point rhs) { return lhs.y < rhs.y; };

    //Check if point is in image bounds
    bool IsInBounds(int r, int c, int rows, int columns) {
        if (r < 0)return false;
        if (c < 0)return false;
        if (r >= rows)return false;
        if (c >= columns)return false;
        return true;
    }


    /**
     * Call FIND_REGION on given coordinates and save it to
     * @param x x cooodinate
     * @param y y coordinate
     */

    void
    FIND_REGION(int x, int y) {
        if (!src.data) {
            std::cout << "No image provided\n";
        }
        assert(x >= 0);
        assert(y >= 0);
        assert(x < src.cols);
        assert(y < src.rows);
        FIND_REGION(src, Point(x, y), 2, 2, 2);
    }

    /**
     * FIND THE REGION OF INTEREST and save it to bounding_box
     * @param image an image to use
     * @param point the source point that we are investating
     * @param b_threshold similarity threshold for blue color
     * @param r_threshold similarity threshold for red color
     * @param g_threshold similarity threshold for green color
     *
     */
    void
    FIND_REGION(const Mat image, const Point &point, int b_threshold = 2, int r_threshold = 2, int g_threshold = 2) {
        assert((b_threshold >= 0) && (b_threshold <= 255));
        assert((r_threshold >= 0) && (r_threshold <= 255));
        assert((g_threshold >= 0) && (g_threshold <= 255));
        //std::cout<<"Find Region "<<point.x<<" "<<point.y<<std::endl;
        Mat image_copy(image.size(), CV_8UC3, 0);
        //create point from initial given point
        Point target(point.x, point.y);
        //create a stack for floodfill
        std::stack<Point> stack_;
        stack_.push(target);

        vector<vector<bool> > visited_(image.cols, vector<bool>(image.rows, false));
        visited_[point.x][point.y] = true;

        //vector that will hold the similar points
        vector<Point> similar_points;
        //flood fill and get similar points
        std::cout << "Target Point: " << point.x << " " << point.y << std::endl;
        std::cout << image.cols << "x" << image.rows << "\n";
        while (!stack_.empty()) {
            Point cur = stack_.top();
            //std::cout<<"Current Point: "<<cur.x<<" "<<cur.y<<std::endl;
            cv::Vec3b color1 = image.at<cv::Vec3b>(Point(point.y, point.x));
            cv::Vec3b color2 = image.at<cv::Vec3b>(Point(cur.y, cur.x));
            stack_.pop();
            int row = cur.y;
            int col = cur.x;
            if (is_similar(color1, color2, b_threshold,
                           r_threshold, g_threshold)) {
                similar_points.emplace_back(cur);
                for (int dx_: dx) {
                    for (auto dy_ :dy) {
                        int new_col = col + dx_;
                        int new_row = row + dy_;
                        if (IsInBounds(new_row, new_col, image.rows, image.cols)) {
                            //std::cout<<"Adding "<<new_col<<" "<<new_row<<std::endl;
                            if (visited_[new_col][new_row] == false) {
                                stack_.push(Point(new_col, new_row));
                                visited_[new_col][new_row] = true;
                            }
                        }
                    }
                }
                visited_[col][row] = true;
            }
            //std::cout<<stack_.size()<<std::endl;
        }
        //Here we get a bounding rectangle for the points
        //We could use open cv to calculate a convex hull
        region_of_interest = similar_points;

        //get the upper left and lower right corners of the bounding box
        std::pair<vector<Point>::iterator, vector<Point>::iterator> xExtremes, yExtremes;
        xExtremes = std::minmax_element(similar_points.begin(), similar_points.end(), compareX);
        yExtremes = std::minmax_element(similar_points.begin(), similar_points.end(), compareY);
        Point upperLeft(xExtremes.first->x, yExtremes.first->y);
        Point lowerRight(xExtremes.second->x, yExtremes.second->y);
        vector<Point> bounding_box_edges;
        bounding_box_edges.emplace_back(upperLeft);
        bounding_box_edges.emplace_back(lowerRight);

        assert(bounding_box_edges.size() == 2);
//        std::cout<<"Similar "<<similar_points.size()<<"\n";
//        for(auto p: bounding_box_edges){
//			std::cout<<p.x<<" "<<p.y<<std::endl;
//		}
//		similar_points.clear();
        //find the perimeter
        FIND_PERIMETER(bounding_box_edges);
//        return bounding_box_edges;
    }

    /**
     * Get the bounding box of the points from FIND_REGION
     * Ideally we could use opencv's convex hull to be more precise
     *
     * @param region a vector of points containing the upper left and lowe right
     * of a region of interest
     * @return the four corners of the bounding box
     */
    void FIND_PERIMETER(vector<Point> region) {
        std::cout << "Find Perimeter\n";
        vector<Point> perimeter;
        Point upper_left(region[0]);
        Point lower_right(region[1]);
        Point upper_right(region[1].x, region[0].y);
        Point lower_left(region[0].x, region[1].y);
        perimeter.emplace_back(upper_left);
        perimeter.emplace_back(lower_left);
        perimeter.emplace_back(lower_right);
        perimeter.emplace_back(upper_right);

        //bounding_box = perimeter;
        bounding_box = region;

        //Display the area of interest
//        DISPLAY_PIXELS();
//        for(auto p: perimeter){
//            std::cout<<p.x<<" "<<p.y<<std::endl;
//        }
//        std::cout<<"\n";
//        return perimeter;
    }

    //Call FIND_PERIMETER with the alread stored region of interest
    void FIND_PERIMETER() {
        if (region_of_interest.size() == 0) {
            std::cout << "No data available\n";
        } else {
            FIND_PERIMETER(region_of_interest);
        }
    }

    //Start up the window to get the point of interest by clicking
    void FIND_REGION() {
//        DISPLAY_IMAGE();
    }

    //Display an alread loaded image
    void DISPLAY_IMAGE() {
        if (!src.data) {
            std::cout << "No image loaded\n";
            return;
        }
        DISPLAY_IMAGE(src);
    }

    /**
     * Displays an Image
     * @param image Image to display
     */
    void DISPLAY_IMAGE(const Mat image) {
        destroyAllWindows();
        namedWindow(_window_name, WINDOW_AUTOSIZE);
//        cv::setMouseCallback(_window_name, &ImageManager::onClick, this);
        if (!image.data) {
            std::cout << "DISP IMG (Mat)  No image read\n";
//            destroyWindow(_window_name);
            return;
        }
        try {
            std::cout << "Press any key to continue\n";
            imshow(_window_name, image);
            waitKey(0);
            std::cout << "Done\n";
//            destroyAllWindows();
//            waitKey(25);


        } catch (int e) {
            std::cout << "Could not display Image\n";
            return;
        }
    }

    /**
     * Display an image based on a path
     * @param image_name the path of the image to display
     */
    void DISPLAY_IMAGE(const string image_name) {
        std::cout<<image_name<<std::endl;
        src = imread(image_name.c_str());
        if(!src.data){
            std::cout<<"DIPLAY (string) could not load image\n";
        }
        DISPLAY_IMAGE(src);
        region_of_interest.clear();
        bounding_box.clear();
    }

    /**
     *
     * @param perimeter a vector of points containing the corners of the bounding box
     * of the area of interest
     */
    void DISPLAY_PIXELS(vector<Point> perimeter) {
        Rect roi(perimeter[0], perimeter[1]);
        if (perimeter[0].y == perimeter[1].y) {
            std::cout << "Invalid Area same X for both endpoints\n";
            return;
        }
        if (perimeter[0].y == perimeter[1].y) {
            std::cout << "Invalid Area same X for both endpoints\n";
            return;
        }
        Mat cropped = src(roi);
        for (auto p: perimeter) {
            std::cout << p.x << " " << p.y << std::endl;
        }
        std::cout << "\n";
        namedWindow("Cropped Image", WINDOW_AUTOSIZE);
        imshow("Cropped Image", cropped);
        waitKey(0);
        //destroyWindow("Cropped Image");
    }

    /**
     * Display the already calcuated area of interest, if it exists
     */
    void DISPLAY_PIXELS() {
        if (bounding_box.size() == 0) {
            std::cout << "No region selected\n";
            return;
        }
        if (!src.data) {
            std::cout << "No image selected\n";
            return;
        }
        DISPLAY_PIXELS(bounding_box);
    }

    //Save the cropped pixels
    /**
     * Save the already calcuated area of interest, if it exists
     */
    void SAVE_PIXELS(vector<Point> region, string filename = "region.jpg") {
        if (region_of_interest.size() == 0) {
            std::cout << "No region selected\n";
            return;
        }
        Rect roi(region[0], region[1]);
        Mat cropped = src(roi).clone();
        imwrite(filename, cropped);
    }

    void SAVE_PIXELS(string filename = "region.jpg") {
        if (region_of_interest.size() == 0) {
            std::cout << "No data available to save\n";
        } else {
            SAVE_PIXELS(region_of_interest, filename);
        }
    }
};

#endif //EXERCISE_AEO2881_IMAGEMANAGER_H
