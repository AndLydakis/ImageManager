# ImageManager

This is an exercise in basic image processing and segmentation. The user selects a point by coordinates through a CLI and
receives the area surrounding the selected point that is similar in color. Area selections is done using the flood-fill algorithm and the bounding area smoothing is done using Catmull-Rom splines. As an exercises these methods were implemented by hand instead of using OpenCV's existing methods.

## Main Executable: ./MAIN_TEST
### Available commands after starting executable:
   **DISPLAY :** Display an already loaded image, clicking on a point will trigger the FIND REGION FUNCTION

**DISPLAY \<IMAGE>**: Displays the input image, and sets it as the source image for
    subsequent functions

**FIND REGION \<X> \<Y>**: Will select the input X and Y to find similar points near X,Y and
    calculate the convex hull or a bounding box if the number of points is too high

**FIND PERIMETER**: Will attempt to find the perimeter of the selected region and will
    calculate a simple bounding rectangle

**FIND SMOOTH**: Must be run after **FIND REGION**. It will smooth the existing convex hull using Catmull-Rom splines.

**DISPLAY PIXELS**: Will display the region of the original picture that was selected based on 	FIND REGION

**SAVE PIXELS \<optional filename>**: Will save the selected area to the given filename or "region.jpg" by default

**To run a test over the sample images (press any key to proceed through the images)**

```bash
user@host:~$ ./MAIN_TEST -f TEST
```

Running Example for **test3.png** (Also see the commited images *test_original.jpg, test_smoothed.jpg*):

	./MAIN_TEST
    DISPLAY test3.png (press any key to close the image)
    FIND REGION 100 100
    DISPLAY PIXELS (press any key to close the image)
    FIND SMOOTH
    DISPLAY PIXELS (press any key to close the image)
    QUIT

 ## Implementation Details
 ### FIND_SMOOTH_PERIMETER Implementation:

After calling FIND_REGION the result is the convex hull of the points that are similar and surround our original point. To smooth this convex hull I used the Catmull-Rom spline method.
I used a standard set of 100 points for each segment, but that should ultimately be a variable
that can be changed.

This is done by sequentially calldig the **CatmullRomSplineInterval** method on a rolling window of length 4 on the points of the originally calculated convex hull.

Out of the proposed methods, I found this the easiest to implement in the
allowed time. It also has the benefit of avoiding intersections of the segments. It is also easy to avoid the need for non-zero distances between the points that is a prerequisite. However it does have the disadvantage of not producing continuous curvature as a B-spline would.

Below is the implementation and documentation of the function along with helper methods:
```cpp
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
            c = CatmullRomSplineInterval(convex_hull_[i], convex_hull_[i + 1], convex_hull_[i + 2],
                                         convex_hull_[i + 3]);
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
```

 ## TODO
- Regarding the region discovery we could preprocess the image by using HSV-based comparison methods after converting it to grayscale, blurring it to smooth out the artifacts.etc. OpenCV has methods for that but I was not sure If they were allowed so I chose not to use it. After getting a starting ROI we could also use edge detection to even further constrict it to the desired area.
- We could also speed up the convex hull creation by sampling from our ROI if it has more points than a certain threshold, instead of using every point available.
- A lot can be done to improve the functionality at this level. On a user experience level, providing a GUI and mouse interface would greatly improve the functionality of the app. I attempted to do that in the first part but I ended up wasting a lot of time on it and in the end I had issues with window management so I chose to  abandon the idea. There still are issues with windows not closing immediately but at least they are not destructive.
- As it is the class could use with some modification methods to set and get the different attributes such as window names, color thresholds etc.
- The similarity between points could also be improved using HSV values, which is usally the standard in these applications.
