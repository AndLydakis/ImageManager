# ImageManager README

## Main Executable: ./MAIN_TEST -i < path to image >
### Available commands after starting executable:
    DISPLAY :Display an already loaded image, clicking on a point will trigger the FIND REGION FUNCTION

	DISPLAY <IMAGE>: Displays the selected image, clicking on a point will trigger the FIND REGION FUNCTION
    
    FIND REGION: Same functionality as DISPLAY IMAGE
    
    FIND PERIMETER: Will attempt to find the perimeter of the selected region, must be called after FIND REGION
    
    DISPLAY PIXELS: Will display the region of the original picture that was selected based on FIND REGION
    
    SAVE PIXELS <optional filename>: Will save the selected area to the given filename or "region.jpg" by default

    run ./MAIN_TEST -f TEST to run a test over the sample images (at this point it segfaults)
    

Unfortunately a lot of the functionality is not ready for user in a client environment.
A lot of memory issues and window management problems exist. The ROI selection is at a good level, but there is an issue when calculating the bounding box.

Running Example ():

	./MAIN_TEST -i test1.png
    click somwhere, the region of interest should appear
    close ROI window
    click somewhere else, the region of interest should appear
    press 'q' to exit the window loop
    use the command line to input one of the above commands