//
// Created by lydakis on 3/31/18.
//

#ifndef EXERCISE_AEO2881_IMAGEMANAGER_H
#define EXERCISE_AEO2881_IMAGEMANAGER_H

#include "opencv2/opencv_modules.hpp"
# include "opencv2/core/core.hpp"
# include "opencv2/features2d/features2d.hpp"
# include "opencv2/highgui/highgui.hpp"
# include "opencv2/imgproc/imgproc.hpp"

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

//We may not want to calculate the convex hull
//for ROIS over a certain size
#define MAX_SIZE_TO_COMPUTE_CONVEX_HULL 10000
//Similarity threshold for blue
#define BT 50
//Similarity threshold for red
#define RT 50
//Similarity threshold for green
#define GT 50
using namespace cv;

/**
 * @brief Generates linearly spaced points
 *
 * @param a Start value
 * @param b End value
 * @param N Number of points
 */
template<typename T = double>
vector<T> linspace(T a, T b, size_t N) {
    T h = (b - a) / static_cast<T>(N - 1);
    vector<T> xs(N);
    typename vector<T>::iterator x;
    T val;
    for (x = xs.begin(), val = a; x != xs.end(); ++x, val += h)
        *x = val;
    return xs;
}

//Boundary Check x coordinates
static bool compareX(Point lhs, Point rhs) { return lhs.x < rhs.x; };

//Boundary Check y coordinates
static bool compareY(Point lhs, Point rhs) { return lhs.y < rhs.y; };

//Distance function for 2 points
float dist(Point p1, Point p2) {
    return sqrt((p1.x - p2.x) * (p1.x - p2.x) + (p1.y - p2.y) * (p1.y - p2.y));
}

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
    //Stores the convex hull of the region of interest
    vector<Point> convex_hull_;
    //Stores the smoothed convex hull of the region of interest
    vector<Point> smoothed_convex_hull_;
    //Store the smoothed roi for saving purposes
    Mat smoothed_roi_to_save;
    //Store the roi for saving purposes
    Mat roi_to_save;


    /**
     * Function to listen for clicks in an image, currently not used
     * @param event a click event
     * @param x coordinate of event
     * @param y coordinate of event
     * @param flags openCV flags
     * @param param parameters we might want to pass to the function
     */
    static void onClick(int event, int x, int y, int flags, void *param) {
        //std::cout<<x<<" "<<y<<std::endl;
        ImageManager *im_this = static_cast<ImageManager *>( param);
        //std::cout<<im_this->src.rows<<std::endl;
        //std::cout<<im_this->src.cols<<std::endl;
        if (event == EVENT_LBUTTONDOWN) {
            Point event_source(x, y);
            im_this->FIND_REGION(im_this->src, event_source, BT, RT, GT);
        }

    }

public:

    ImageManager() {

    }

    ~ImageManager() {}

    /**
     *
     * @param image_name the path of the image to load as a string
     * @param dis 1 if you want to display the image immediately, 0 otherwise
     */
    ImageManager(const std::string image_name) {
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

    //TODO probably replace it with a HSV comparison
    /**
     * Similarity function for two RGB points
     * @param p1 point 1
     * @param p2 point 2
     * @param b_threshold similarity threshold for blue value
     * @param r_threshold similarity threshold for red value
     * @param g_threshold similarity threshold for green value
     * @return
     */
    bool is_similar(cv::Vec3b p1, cv::Vec3b p2, int b_threshold, int r_threshold, int g_threshold) {
        return ((abs((int) p1[0] - (int) p2[0]) <= b_threshold) &&
                (abs((int) p1[1] - (int) p2[1]) <= g_threshold) &&
                (abs((int) p1[2] - (int) p2[2]) <= r_threshold));

    }

    /**
     * Check if a point is within allowed bounds of a 2D matrix
     * @param r current row
     * @param c current column
     * @param rows number of rows in the matrix
     * @param columns number of columns in the matrix
     * @return true if the point is in bounds, false otherwise
     */
    bool IsInBounds(int r, int c, int rows, int columns) {
        if (r < 0)return false;
        if (c < 0)return false;
        if (r >= rows)return false;
        if (c >= columns)return false;
        return true;
    }


    /**
     * Orientation function for the calculation of the
     * convex hull
     * @param p first point
     * @param q second point
     * @param r third point
     * @return
     */
    int orientation(Point p, Point q, Point r) {
        int val = (q.y - p.y) * (r.x - q.x) -
                  (q.x - p.x) * (r.y - q.y);
        if (val == 0) return 0;
        return (val > 0) ? 1 : 2;
    }

    /**
     * Brute force algorithm to get the
     * convex hull of a set of points
     *
     * Populates the convex_hull attribute
     *
     * @param roi the Region of interest represented
     * as a list of points
     *
     */
    void convex_hull(vector<Point> roi) {
        if (roi.size() < 3) return;
        vector<Point> hull;
        int l = 0;
        //Find left most point
        for (int i = 1; i < roi.size(); i++)
            if (roi[i].x < roi[l].x)
                l = i;
        int p = l, q;

        //Keep looking for q such that q,p,r are clockwise
        //for all other points r
        do {
            hull.push_back(roi[p]);
            q = (p + 1) % roi.size();
            for (int i = 0; i < roi.size(); i++) {
                if (orientation(roi[p], roi[i], roi[q]) == 2)
                    q = i;
            }
            p = q;
        } while (p != l);
        convex_hull_ = hull;
//        std::cout << "Convex Hull Points: " << convex_hull_.size() << "\n";
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
        if (x < 0) {
            std::cout << "X out of bounds";
            return;
        }
        if (y < 0) {
            std::cout << "X out of bounds";
            return;
        }
        if (x >= src.cols) {
            std::cout << "X out of bounds";
            return;
        }
        if (y >= src.rows) {
            std::cout << "X out of bounds";
            return;
        }
        FIND_REGION(src, Point(x, y));
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
    FIND_REGION(const Mat image, const Point &point, int b_threshold = BT, int r_threshold = RT, int g_threshold = GT) {
        if (!((b_threshold >= 0) && (b_threshold <= 255))) {
            std::cerr << "invalid b threshold\n";
            return;
        }
        if (!((r_threshold >= 0) && (r_threshold <= 255))) {
            std::cerr << "invalid r threshold\n";
            return;
        }
        if (!((r_threshold >= 0) && (r_threshold <= 255))) {
            std::cerr << "invalid g threshold\n";
            return;
        }

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
//        std::cout << "Target Point: " << point.x << " " << point.y << std::endl;
//        std::cout << image.cols << "x" << image.rows << "\n";
        while (!stack_.empty()) {
            Point cur = stack_.top();
            cv::Vec3b color1 = image.at<cv::Vec3b>(Point(point.x, point.y));
            cv::Vec3b color2 = image.at<cv::Vec3b>(Point(cur.x, cur.y));
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
                            if (visited_[new_col][new_row] == false) {
                                stack_.push(Point(new_col, new_row));
                                visited_[new_col][new_row] = true;
                            }
                        }
                    }
                }
                visited_[col][row] = true;
            }
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

        //Calculate the convex hull
        std::cout << "Region Of Interest Size: " << region_of_interest.size() << "\n";
        if (region_of_interest.size() <= MAX_SIZE_TO_COMPUTE_CONVEX_HULL) {
//            std::cout << "Finding convex hull\n";
            convex_hull(region_of_interest);
        } else {
//            std::cout << "Skipping Convex hull\n";
            convex_hull_.clear();
        }

        if (bounding_box_edges.size() != 2) {
            std::err << "Could not find region\n";
            return;
        }

        FIND_PERIMETER(bounding_box_edges);
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
//        std::cout << "Find Perimeter\n";
        vector<Point> perimeter;
        Point upper_left(region[0]);
        Point lower_right(region[1]);
        Point upper_right(region[1].x, region[0].y);
        Point lower_left(region[0].x, region[1].y);
        perimeter.emplace_back(upper_left);
        perimeter.emplace_back(lower_right);
        perimeter.emplace_back(lower_left);
        perimeter.emplace_back(upper_right);

        bounding_box = perimeter;
//        std::cout << "Found roi Perimeter\n";

    }

    //Call FIND_PERIMETER with the already stored region of interest
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

    /**
     * Display an already loaded image if available
     */
    void DISPLAY_IMAGE() {
        if (!src.data) {
            std::cerr << "No image loaded\n";
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
        if (!image.data) {
            std::cerr << "DISP IMG (Mat)  No image read\n";
            return;
        }
        try {
            std::cout << "Press any key to continue\n";
            imshow(_window_name, image);
            waitKey(0);
        } catch (int e) {
            std::cerr << "Could not display Image\n";
            return;
        }
    }

    /**
     * Display an image based on a path
     * @param image_name the path of the image to display
     */
    void DISPLAY_IMAGE(const string image_name) {
        std::cout << image_name << std::endl;
        src = imread(image_name.c_str(), 1);
        if (!src.data) {
            std::cerr << "DIPLAY (string) could not load image\n";
        }
        DISPLAY_IMAGE(src);
        //If we loaded a new image clear our variables
        region_of_interest.clear();
        bounding_box.clear();
    }

    /**
     * Display the ROI based on a simple rectangle if we don't have convex hull
     * @param perimeter a vector of points containing the corners of the bounding box
     * of the area of interest
     */
    void DISPLAY_PIXELS(vector<Point> perimeter) {
        Rect roi(perimeter[0], perimeter[1]);
        for (auto p: perimeter) {
            std::cerr << p.x << " " << p.y << std::endl;
        }
        if (perimeter[0].y == perimeter[1].y) {
            std::cerr << "Invalid Area same Y for both endpoints\n";
            return;
        }
        if (perimeter[0].x == perimeter[1].x) {
            std::cerr << "Invalid Area same X for both endpoints\n";
            return;
        }
        Mat cropped = src(roi);
        roi_to_save = cropped;
        namedWindow("Cropped Image", WINDOW_AUTOSIZE);
        imshow("Cropped Image", cropped);
        waitKey(0);
    }

    /**
     * Use the convex hull to display the ROI
     * @param perimeter a vector of points containing the convex hull of the roi
     * of the area of interest
     */
    void DISPLAY_PIXELS_CONVEX(vector<Point> hull, int i) {
        if (!src.data) {
            std::cerr << "No image selected\n";
            return;
        }

        Mat dst;
        //Create a mask
        Mat mask(src.rows, src.cols, CV_8UC3, cv::Scalar(0, 0, 0));
//        std::cout << "Created Mask\n";
        cv::Point corners[1][hull.size()];
        for (size_t i = 0; i < hull.size(); ++i) {
            corners[0][i] = hull[i];
        }
        const Point *corner_list[1] = {corners[0]};
//        std::cout << "Fill poly\n";
        int num_points = hull.size();
        int num_polygons = 1;
        int line_type = 8;
        //Create the polygon based on the convex hull points, and use it on the mask
        fillPoly(mask, corner_list, &num_points, num_polygons, cv::Scalar(255, 255, 255), line_type);
//        std::cout << "Bitwise and\n";
        //Use the mask on our image
        cv::bitwise_and(src, mask, dst);
        namedWindow("Cropped Image", WINDOW_AUTOSIZE);
//        std::cout << "Imshow\n";
        //Save the ROIs to the appropriate
        //attribute depending on which convex hull we used (smoothed or not)
        if (i == 1) {
            smoothed_roi_to_save = dst;
        } else {
            roi_to_save = dst;
        }
        imshow("Cropped Image", dst);
        waitKey(0);
//        std::cout << "Done\n";
    }

    /**
     * Display the already calcuated area of interest, if it exists
     */
    void DISPLAY_PIXELS() {
        destroyAllWindows();
        if (!src.data) {
            std::cerr << "No image selected\n";
            return;
        }
        if (smoothed_convex_hull_.size() > 0) {
//            std::cerr << "Displaying Smoothed Hull\n";
            DISPLAY_PIXELS_CONVEX(smoothed_convex_hull_, 1);
            return;
        }
        if (convex_hull_.size() > 0) {
//            std::cerr << "Displaying Convex Hull\n";
            DISPLAY_PIXELS_CONVEX(convex_hull_, 0);
            return;
        }
        if (bounding_box.size() > 0) {
            DISPLAY_PIXELS(bounding_box);
            return;
        }
        std::cerr << "Could not display pixels\n";

    }

    /**
     * Method to save the computed ROIs
     * the method will add the prefix smoothed_ if the smoothed convex
     * hull is available
     * @param filename a file name to use, defaults to region.png
     *
     */
    void SAVE_PIXELS(string filename = "region.png") {
        int cnt = 0;
        if ((!roi_to_save.data) && (!smoothed_roi_to_save.data))DISPLAY_PIXELS();
        if (roi_to_save.data) {
            std::cout << "Saving ROI to " << filename << std::endl;
            bool check = imwrite(filename, roi_to_save);
            if (!check) std::cout << "Could not save roi\n";
            else cnt++;
        }
        if (smoothed_roi_to_save.data) {
            std::cout << "Saving smooth ROI to " << "smooth_" + filename << std::endl;
            bool check = imwrite("smooth_" + filename, smoothed_roi_to_save);
            if (!check) std::cout << "Could not save smooth roi\n";
            else cnt++;
        }
        std::cout << cnt << " file(s) saved\n";
    }


    /**
     * Method to calculate knot j for points p0 and p1
     * @param t
     * @param p0 point 1
     * @param p1 point 2
     * @return
     */
    float tj(float t, Point p0, Point p1) {
        float alpha = 0.5f;
        float a = pow((p1.x - p0.x), 2.0f) + pow((p1.y - p0.y), 2.0f);
        float b = pow(a, 0.5f);
        float c = pow(b, alpha);
        return (c + t);
    }

    /**
     * Calculate the Catmull-Rom spline for points p1-4
     * @param p0 point 0
     * @param p1 point 1
     * @param p2 point 2
     * @param p3 point 3
     * @param num_points the number of points in the current part of the curve
     * @return a vector of points containing the points in the current part of the curve
     */
    vector<Point> CatmullRomSplineInterval(Point p0, Point p1, Point p2, Point p3, int num_points = 200) {
        float t0 = 0;
        float t1 = tj(t0, p0, p1);
        float t2 = tj(t1, p1, p2);
        float t3 = tj(t2, p2, p3);

        vector<Point> C;

        for (float t = t1; t < t2; t += ((t2 - t1) / num_points)) {
            Point A1;
            A1.x = (t1 - t) / (t1 - t0) * p0.x + (t - t0) / (t1 - t0) * p1.x;
            A1.y = (t1 - t) / (t1 - t0) * p0.y + (t - t0) / (t1 - t0) * p1.y;

            Point A2;
            A2.x = (t2 - t) / (t2 - t1) * p1.x + (t - t1) / (t2 - t1) * p2.x;
            A2.y = (t2 - t) / (t2 - t1) * p1.y + (t - t1) / (t2 - t1) * p2.y;

            Point A3;
            A3.x = (t3 - t) / (t3 - t2) * p2.x + (t - t2) / (t3 - t2) * p3.x;
            A3.y = (t3 - t) / (t3 - t2) * p2.y + (t - t2) / (t3 - t2) * p3.y;

            Point B1;
            B1.x = (t2 - t) / (t2 - t0) * A1.x + (t - t0) / (t2 - t0) * A2.x;
            B1.y = (t2 - t) / (t2 - t0) * A1.y + (t - t0) / (t2 - t0) * A2.y;

            Point B2;
            B2.x = (t3 - t) / (t3 - t1) * A2.x + (t - t1) / (t3 - t1) * A3.x;
            B2.y = (t3 - t) / (t3 - t1) * A2.y + (t - t1) / (t3 - t1) * A3.y;

            Point C_;
            C_.x = (t2 - t) / (t2 - t1) * B1.x + (t - t1) / (t2 - t1) * B2.x;
            C_.y = (t2 - t) / (t2 - t1) * B1.y + (t - t1) / (t2 - t1) * B2.y;

            C.emplace_back(C_);

        }

        return C;
    }

    /**
     * Method to smooth the available convex hull by calculating
     * Catmull-Rom splines of default length 100 using 4 point windows
     * from the original convex hull
     */
    void CatmullRomSpline() {
        size_t size = convex_hull_.size();
        vector<Point> c;
        for (int i = 0; i < size - 3; ++i) {
            c = CatmullRomSplineInterval(convex_hull_[i], convex_hull_[i + 1], convex_hull_[i + 2], convex_hull_[i + 3]);
            copy(c.begin(), c.end(), std::back_inserter(smoothed_convex_hull_));
        }
        //Run for the last points to create the loop
        c = CatmullRomSplineInterval(convex_hull_[size - 3], convex_hull_[size - 2], convex_hull_[size - 1],
                                 convex_hull_[0]);
        copy(c.begin(), c.end(), std::back_inserter(smoothed_convex_hull_));

        c = CatmullRomSplineInterval(convex_hull_[size - 2], convex_hull_[size - 1], convex_hull_[0], convex_hull_[1]);
        copy(c.begin(), c.end(), std::back_inserter(smoothed_convex_hull_));

        c = CatmullRomSplineInterval(convex_hull_[size - 1], convex_hull_[0], convex_hull_[1], convex_hull_[2]);
        copy(c.begin(), c.end(), std::back_inserter(smoothed_convex_hull_));
//        std::cout << "Smoothed convex hull size: " << smoothed_convex_hull_.size() << std::endl;
    }

    /**
     * RUN THE SMOOTHING FUNCTION
     */
    void FIND_SMOOTH_PERIMETER() {
//        std::cout << "Find Smooth Perimeter" << std::endl;
        if (convex_hull_.size() <= 4) {
            std::cerr << "Cannot smooth detected region\n";
            smoothed_convex_hull_ = convex_hull_;
            return;
        }
        CatmullRomSpline();
    }
};

#endif //EXERCISE_AEO2881_IMAGEMANAGER_H
