# ImageManager (REFACTORED) README


#### This is the readme for the refactored version that does not use the mouse to get the desired coordinates. It is more stable than the last commit
## Main Executable: ./MAIN_TEST <-i  path to image (optional)>
### Available commands after starting executable:
    DISPLAY :Display an already loaded image, clicking on a point will trigger the FIND REGION FUNCTION

	DISPLAY <IMAGE>: Displays the selected image and sets that image as the source image
    
    FIND REGION X Y: Find the region similar to point(X, Y)
    
    FIND PERIMETER: Will attempt to find the perimeter of the selected region, must be called after FIND REGION
    
    DISPLAY PIXELS: Will display the region of the original picture that was selected based on FIND REGION
    
    SAVE PIXELS <optional filename>: Will save the selected area to the given filename or "region.jpg" by default

    run ./MAIN_TEST -f TEST to run a test over the sample images (I could not find the test X Y coordinates in the shared folder. If possible change the desired x,y values for each image in main.cpp, make the project and run the test to check)
    

Unfortunately a lot of the functionality is not ready for user in a client environment.
A lot of memory issues and window management problems exist. The ROI selection is at a good level, but there is an issue when calculating the bounding box.

Running Example ():

	./MAIN_TEST
	DISPLAY test1.png
    FIND REGION 13 13
    DISPLAY PIXELS
    DISPLAY test2.png
    FIND REGION 145 145
    DISPLAY PIXELS
    QUIT
