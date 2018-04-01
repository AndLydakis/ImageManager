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
    vector<Point> region_of_interest;
    vector<Point> bounding_box;

    //Callback to get click coordinates and call FIND_REGION on theses coordinates
	static void onClick(int event, int x, int y, int flags, void* param)
	{	
		//std::cout<<x<<" "<<y<<std::endl;
		ImageManager *im_this = static_cast<ImageManager*>( param);
		 if  ( event == EVENT_LBUTTONDOWN )
		 {
			 Point event_source(x,y);
			 im_this->FIND_REGION(im_this->src, event_source, 2, 2, 2);
		 }
		 
	}
	
public:
    //Initialize with an image
    ImageManager(const std::string image_name, int dis = 1) {
		namedWindow(_window_name, WINDOW_AUTOSIZE);
		src = imread(image_name.c_str());
		if(!src.data){
			std::cout<<"No image read/n";
			return;
		}
		cv::setMouseCallback(_window_name, &ImageManager::onClick, this);
		if(dis==1){
			DISPLAY_IMAGE(src);
		}
    }

    //Change the image used
	void changeSrc(const std::string image_name){
		src = imread(image_name.c_str());
		if(!src.data){
			std::cout<<"No image read/n";
			return;
		}
	}

    //BASIC SIMILARITY FUNCTION
    //TODO probably replace it with a HSV comparison
    bool is_similar(Vec3b p1, Vec3b p2, int b_threshold, int r_threshold, int g_threshold) {
		std::cout<<p1.val[0]<<" "<<p1.val[1]<<" "<<p1.val[2]<<std::endl;
		std::cout<<p2.val[0]<<" "<<p2.val[1]<<" "<<p2.val[2]<<std::endl;
        return (abs(p1[0] - p2[0] <= b_threshold) &&
                abs(p1[1] - p2[1] <= g_threshold) &&
                abs(p1[2] - p2[2] <= r_threshold));

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
     *
     * @param image an image to use
     * @param point the source point that we are investating
     * @param b_threshold similarity threshold for blue color
     * @param r_threshold similarity threshold for red color
     * @param g_threshold similarity threshold for green color
     * @return a vector of points similar in color to the source point
     */
    vector<Point>
    FIND_REGION(const Mat image, const Point &point, int b_threshold = 2, int r_threshold = 2, int g_threshold = 2) {
        assert((b_threshold >= 0) && (b_threshold <= 255));
        assert((r_threshold >= 0) && (r_threshold <= 255));
        assert((g_threshold >= 0) && (g_threshold <= 255));
        //std::cout<<"Find Region "<<point.x<<" "<<point.y<<std::endl;
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
            std::cout<<"Current Point: "<<cur.x<<" "<<cur.y<<std::endl;
            std::cout<<"Target Point: "<<point.x<<" "<<point.y<<std::endl;
            Vec3b color1 = image.at<Vec3b>(Point(point.y, point.x));
            Vec3b color2 = image.at<Vec3b>(Point(cur.y, cur.x));
            if (is_similar(color1, color2, b_threshold,
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
        //Here we get a bounding rectangle for the points
        //We could use open cv to calculate a convex hull
        region_of_interest = similar_points;
        std::pair <vector<Point>::iterator, vector<Point>::iterator> xExtremes, yExtremes;
        xExtremes = std::minmax_element(similar_points.begin(), similar_points.end(), compareX);
        yExtremes = std::minmax_element(similar_points.begin(), similar_points.end(), compareY);
        Point upperLeft(xExtremes.first->x, yExtremes.first->y);
        Point lowerRight(xExtremes.second->x, yExtremes.second->y);
        vector<Point> bounding_box_edges;
        bounding_box_edges.emplace_back(upperLeft);
        bounding_box_edges.emplace_back(lowerRight);
        std::cout << bounding_box_edges.size()<<std::endl;
        
        for(auto p: bounding_box_edges){
			std::cout<<p.x<<" "<<p.y<<std::endl;
		}
		FIND_PERIMETER(bounding_box_edges);
        return bounding_box_edges;

    }

    /**
     *
     * @param region a vector of points containing the upper left and lowe right
     * of a region of interest
     * @return the four corners of the bounding box
     */
    vector<Point> FIND_PERIMETER(vector<Point> region) {
		std::cout<<"Find Perimeter\n";
        vector<Point> perimeter;
		Point upper_left(region[0]);
		Point lower_right(region[1]);
		Point upper_right(region[1].x, region[0].y);
		Point lower_left(region[0].x, region[1].y);
		perimeter.emplace_back(upper_left);
		perimeter.emplace_back(lower_left);
		perimeter.emplace_back(lower_right);
		perimeter.emplace_back(upper_right);
		for(auto p: perimeter){
			std::cout<<p.x<<" "<<p.y<<std::endl;
		}
		std::cout<<"\n";
        return perimeter;
    }
	
	void FIND_PERIMETER(){
		if(region_of_interest.size()==0){
			std::cout<<"No data available\n";
		}else{
			FIND_PERIMETER(region_of_interest);
		}
	}
	void FIND_REGION(){
		DISPLAY_IMAGE();
	}
	//Display an alread loaded image
	void DISPLAY_IMAGE(){
		if(!src.data){
			std::cout<<"NO image loaded\n";
		}
		DISPLAY_IMAGE(src);
	}
    /**
     * Displays an Image
     * @param image Image to display
     */
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

    /**
     * Display an image based on a path
     * @param image_name the path of the image to display
     */
    void DISPLAY_IMAGE(const string image_name) {
        src = imread(image_name.c_str());
        region_of_interest.clear();
        bounding_box.clear();
        if(!src.data){
            std::cout<<"No image read/n";
            return;
        }
        try {
            std::cout << "Press 'c' to continue\n";
            while(true){
                imshow(_window_name, src);
                int k = waitKey(0);
                if ( k==27 ) break;
            }
        } catch (int e) {
            std::cout << "Could not display Image\n";
            return;
        }
    }



    void DISPLAY_PIXELS(vector<Point> region) {
		Rect roi(region[0], region[1]);
		Mat cropped = src(roi).clone();
		imshow(_window_name, cropped);
		waitKey(0);
    }

    void DISPLAY_PIXELS() {
        if(region_of_interest.size() == 0){
            std::cout<<"No region selected\n";
            return;
        }
        if(!src.data){
            std::cout<<"No image selected\n";
            return;
        }
        DISPLAY_PIXELS(region_of_interest);
    }
	
	//Save the cropped pixels
    void SAVE_PIXELS(vector<Point> region) {
		if(region_of_interest.size() == 0){
			std::cout<<"No region selected\n";
			return;
		}
		Rect roi(region[0], region[1]);
		Mat cropped = src(roi).clone();
		imwrite( "region.jpg", cropped);
    }
    
    void SAVE_PIXELS(){
		if(region_of_interest.size()==0){
			std::cout<<"No data available to save\n";
		}else{
			SAVE_PIXELS(region_of_interest);
		}
	}
};

#endif //EXERCISE_AEO2881_IMAGEMANAGER_H
