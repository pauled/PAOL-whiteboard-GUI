#include "WhiteboardProcessor.h"
#include "uf.h"
#include <stdexcept>

///////////////////////////////////////////////////////////////////
///
///   Definitions for constructing and restoring original state
///   of the whiteboard processor
///
///////////////////////////////////////////////////////////////////

// Constructor
WhiteboardProcessor::WhiteboardProcessor() {
    reset();
    debugProcessing = false;
}

WhiteboardProcessor::WhiteboardProcessor(bool debugFlag) {
    reset();
    debugProcessing = debugFlag;
}

// Return the whiteboard processor to its original state (ie. no processed frames or whiteboard model)
void WhiteboardProcessor::reset() {
    stableImageCount = 0;
    oldFrame = Mat();
    whiteboardModel = Mat();
}

///////////////////////////////////////////////////////////////////
///
///   Method to process the given frame and update whiteboard model
///
///////////////////////////////////////////////////////////////////

// Processes the given frame and updates the whiteboard model.
Mat WhiteboardProcessor::processCurFrame(const Mat& currentFrame) {
    // Clear out the debugging frames
    debugFrames.clear();
    // Stores the images that we eventually want to copy to frameOutput
    vector<Mat> toSave;

    // Initialize oldFrame if we have not processed a frame before
    if(!oldFrame.data) {
        // Duplicate the current frame
        oldFrame = currentFrame.clone();
        return Mat();
    }

    // Find difference pixels
    float numDif;
    Mat allDiffs;
    WhiteboardProcessor::findAllDiffsMini(allDiffs, numDif, oldFrame, currentFrame, 40, 1);

    // Temporary Mat to store true differences
    Mat filteredDiffs = Mat::zeros(allDiffs.size(), allDiffs.type());

    //if there is enough of a difference between the two images
    float refinedNumDif = 0;
    if(numDif>.03){
        WhiteboardProcessor::filterNoisyDiffs(filteredDiffs, refinedNumDif, allDiffs);
        stableImageCount=0;
    }

    //if the images are really identical, count the number of consecultive nearly identical images
    if (numDif < .000001)
        stableImageCount++;

    //if the differences are enough that we know where the lecturer is or the images have been identical
    //for two frames, and hence no lecturer present
    if(refinedNumDif>.04 || (numDif <.000001 && stableImageCount==2)){
        // Store the old and current frames for debugging
        toSave.push_back(oldFrame);
        toSave.push_back(currentFrame);

        /////////////////////////////////////////////////////////////
        // Display the old and current frames being processed
        Mat markerLocation = WhiteboardProcessor::findMarkerWithCC(currentFrame);
        imwrite("/home/paol/shared/out/markerLocation.png", markerLocation);
        Mat whiteWhiteboard = whitenWhiteboard(currentFrame, markerLocation);
        imwrite("/home/paol/shared/out/whiteWhiteboard.png", whiteWhiteboard);
        Mat darkenedText = smoothMarkerTransition(whiteWhiteboard);
        imwrite("/home/paol/shared/out/smoothMarker.png", darkenedText);
        toSave.push_back(darkenedText);

        /////////////////////////////////////////////////////////////
        //identify where motion is

        Mat diffHulls = WhiteboardProcessor::expandDifferencesRegion(filteredDiffs);
        Mat diffHullsFullSize = WhiteboardProcessor::enlarge(diffHulls);
        toSave.push_back(diffHullsFullSize);

        ////////////////////////////////////////////////////////////////////////////////

        // Update background image (whiteboard model)
        if(!whiteboardModel.data) {
            // There is no previous whiteboard model, so set it to the enhanced image
            whiteboardModel = darkenedText;
        }
        else {
            // Update the existing whiteboard model
            whiteboardModel = WhiteboardProcessor::updateWhiteboardModel(whiteboardModel, darkenedText, diffHullsFullSize);
        }
        toSave.push_back(whiteboardModel);

        // Update old frame
        oldFrame = currentFrame.clone();

        //////////////////////////////////////////////////////////

        // Rectify the model
        Mat rectified = WhiteboardProcessor::rectifyImage(whiteboardModel);
        toSave.push_back(rectified);

        // If debugging whiteboard processing, push images to save to debugFrames
        if(debugProcessing) {
            for(unsigned int i = 0; i < toSave.size(); i++) {
                debugFrames.push_back(toSave[i].clone());
            }
        }

        // Return a copy of the current whiteboard model
        return whiteboardModel.clone();

        //////////////////////////////////////////////////

        // TODO: figure out if saves need to be made based on detected marker
        // Strategy:
        // Store a marker model
        // Make updated version of marker model
        // If updated version greatly differs from previous version, save whiteboard model
    }

    // The frame was not processed, so return an empty Mat
    return Mat();
}

///////////////////////////////////////////////////////////////////
///
///   Methods to find and process differences (ie. find the lecturer)
///
///////////////////////////////////////////////////////////////////

// Shrink the two given images, then find the significantly different pixels between the shrunken images
// For a pixel to be considered significantly different, one of the channels must differ by more than
// the threshold between the two images
// Arguments:
//    diffLocations: Where to store the differences from the shrunken images
//    percentDiff: Where to store the percentage of pixels that differ
//    oldImg: The previous (older) whiteboard image
//    newImg: The current (newer) whiteboard image
//    thresh: How much a channel needs to differ between a pixel to count the pixel as a difference
//    size: Size of window to use to filter out differences near edges
void WhiteboardProcessor::findAllDiffsMini(Mat& diffLocations, float& percentDiff, const Mat& oldImg, const Mat& newImg, int thresh, int size) {
    int offset;
    bool diff;
    int surroundThresh = 50;

    //maskMin is set to a blank state (1/(scale*scale)) the size of src
    diffLocations=Mat::zeros(oldImg.rows/SCALE,oldImg.cols/SCALE,oldImg.type());

    if (SCALE>=size+1)
        offset=SCALE;
    else
        offset=SCALE*2;

  percentDiff = 0;

  for (int y = offset, yy=offset/SCALE; y < (oldImg.rows-offset); y+=SCALE,yy++)
  {
      //for every column
      for (int x = offset, xx=offset/SCALE; x < (oldImg.cols-offset); x+=SCALE,xx++)
      {
          diff = false;
          //for each color channel
          for(int i = 0; i < 3; i++)
          {
              //if the difference (for this pixel) between the the current img and the previous img is greater than the threshold, difference is noted; diff = true
              if(abs((double)newImg.at<Vec3b>(y,x)[i]-(double)oldImg.at<Vec3b>(y,x)[i]) > thresh)
                  diff = true;
          }
          if(diff)
          {
              for(int yy = y-size; yy <= y+size; yy++)
              {
                  for(int xx = x-size; xx <= x+size; xx++)
                  {
                      //for each color channel
                      for(int ii = 0; ii < 3; ii++)
                      {
                          //ignore all differneces found at the edges; sometimes pixels get lost in tranmission
                          if(abs(((double)(newImg.at<Vec3b>(yy,xx)[ii]))-(((double)(newImg.at<Vec3b>((yy+1),xx)[ii])))>surroundThresh))
                              diff = false;
                          if(abs(((double)(newImg.at<Vec3b>(yy,xx)[ii]))-(((double)(newImg.at<Vec3b>(yy,(xx+1))[ii])))>surroundThresh))
                              diff = false;
                      }
                  }
              }
          }
          if(diff)
          {
              percentDiff++;
              diffLocations.at<Vec3b>(yy,xx)[2]=255;
          }
      }
  }
  //return the percent of maskMin that are differences
  percentDiff = (float)percentDiff/(float)(diffLocations.rows*diffLocations.cols);
}

// Filter out pixels that are not surrounded by enough difference pixels
// Arguments:
//    filteredDiffs: Where to store the pixels that are surrounded by enough difference pixels
//    percentDiff: Where to store the percentage of white pixels in filteredDiffs
//    origDiffs: The unfiltered difference pixels (gotten from findAllDiffsMini)
void WhiteboardProcessor::filterNoisyDiffs(Mat& filteredDiffs, float& percentDiff, const Mat& origDiffs) {
    filteredDiffs = Mat::zeros(origDiffs.size(), origDiffs.type());
    int total;
    percentDiff = 0;
    //for every 3x3 group of pixels in maskMin
    for(int x = 1; x < origDiffs.cols-1; x++)
        for(int y = 1; y < origDiffs.rows-1; y++)
        {
            //count the number of differences in the 3x3
            total=0;
            for (int xx=x-1;xx<=x+1;xx++)
                for (int yy=y-1;yy<=y+1;yy++){
                    if(origDiffs.at<Vec3b>(yy,xx)[2]==255)
                        total++;
                }
            //if the number of differences is greater then 3 mark it as different and count it
            if(total>3){
                filteredDiffs.at<Vec3b>(y,x)[1]=255;
                percentDiff++;
            }
        }
    //return the percentage of the maskMin that are valid differences
    percentDiff = (float)percentDiff/(float)(origDiffs.rows*origDiffs.cols);
}

// Given a binary image, fill the image border by replicating the adjacent pixel
Mat WhiteboardProcessor::replicateToImageBorder(const Mat& orig) {
    Mat origWithBorderPixels = orig.clone();
    int x;

    //extend bottom and top edge where differences overlap
    for(x=0;x<origWithBorderPixels.cols;x++){
        if(origWithBorderPixels.at<Vec3b>(1,x)[1] == 255)
            origWithBorderPixels.at<Vec3b>(0,x)[1]=255;
        if(origWithBorderPixels.at<Vec3b>(origWithBorderPixels.rows-2,x)[1] == 255)
            origWithBorderPixels.at<Vec3b>(origWithBorderPixels.rows-1,x)[1]=255;
    }

    //extend right and left edge wher differences overlap
    for(int y = 0; y < origWithBorderPixels.rows; y++)
    {
        if(origWithBorderPixels.at<Vec3b>(y,1)[1] == 255)
            origWithBorderPixels.at<Vec3b>(y,0)[1] = 255;

        if(origWithBorderPixels.at<Vec3b>(y,origWithBorderPixels.cols-2)[1] == 255)
            origWithBorderPixels.at<Vec3b>(y,origWithBorderPixels.cols-1)[1] = 255;
    }
    return origWithBorderPixels;
}

// Sweep the given image (whatever that means), returns a binary image
Mat WhiteboardProcessor::sweepDown(const Mat& orig){
    bool left,right,top;
    //create a Mat the size of orig to store results
    Mat sweeped=Mat::zeros(orig.size(),orig.type());

    //from left to right
    for(int x = 0; x < orig.cols; x++)
    {
        //from top to bottom
        top = false;
        for(int y = 0; y < orig.rows; y++){
            if(orig.at<Vec3b>(y,x)[1] == 255)
                top = true;

            if(top == true)
                sweeped.at<Vec3b>(y,x)[0] = 255;
        }
    }

    //from top to bottom
    for(int y = 0; y < orig.rows; y++){

        //sweep from the left
        left = false;
        for(int x = 0; x < orig.cols; x++)
        {
            if(orig.at<Vec3b>(y,x)[1] == 255)
                left = true;

            if(left == true)
                sweeped.at<Vec3b>(y,x)[1] = 255;
        }

        //sweep from the right
        right = false;
        for(int x = orig.cols-1; x >-1; x--)
        {
            if(orig.at<Vec3b>(y,x)[1] == 255)
                right = true;

            if(right == true)
                sweeped.at<Vec3b>(y,x)[2] = 255;
        }
    }

    return sweeped;
}

// Color the region around the white pixels of the given image with green. This
// is helpful for debugging.
// Arguments:
//    content: A binary image with white pixels to place the border around
//    borderSize: The thickness of the border
Mat WhiteboardProcessor::borderContentWithGreen(const Mat& content, int borderSize)
{
    Mat ret = content.clone();
    int startx,endx,starty,endy;

    //for every pixel in the image
    for(int y = 0; y < ret.rows; y++)
        for(int x = 0; x < ret.cols; x++)

            //if the pixel is turned on
            if(ret.at<Vec3b>(y,x)[0] == 255){
                startx=x-borderSize;
                if (startx<0)
                    startx=0;

                starty=y-borderSize;
                if (starty<0)
                    starty=0;

                endx=x+borderSize;
                if (endx>=ret.cols)
                    endx=ret.cols-1;

                endy=y+borderSize;
                if (endy>=ret.rows)
                    endy=ret.rows-1;

                //grow the region around that pixel
                for(int yy = starty; yy <= endy;yy++)
                    for(int xx = startx; xx <= endx; xx++)
                        ret.at<Vec3b>(yy,xx)[1] = 255;
            }
    return ret;
}

// Dilate the given image by the given size
Mat WhiteboardProcessor::grow(const Mat& orig, int size) {
    Mat ret = orig.clone();
    Mat element = getStructuringElement(MORPH_RECT, Size(2*size+1,2*size+1));
    dilate(orig, ret, element);
    return ret;
}

// Get the contours of the given image
Mat WhiteboardProcessor::getImageContours(const Mat& orig) {
    Mat contourImage = orig.clone();
    Mat src_gray;
    int thresh = 100;
    //int max_thresh = 255;
    RNG rng(12345);

    /// Convert image to gray and blur it
    cvtColor( orig, src_gray, CV_BGR2GRAY );
    cv::blur( src_gray, src_gray, cv::Size(3,3) );

    Mat canny_output;
    vector<vector<Point> > contours;
    vector<Vec4i> hierarchy;

    /// Detect edges using canny
    Canny( src_gray, canny_output, thresh, thresh*2, 3 );
    /// Find contours
    findContours( canny_output, contours, hierarchy, CV_RETR_TREE, CV_CHAIN_APPROX_SIMPLE, Point(0, 0) );

    vector<vector<Point> >hull( contours.size() );
    for(unsigned int i = 0; i < contours.size(); i++ )
    {  convexHull( Mat(contours[i]), hull[i], false ); }

    /// Draw contours + hull results
    Mat drawing = Mat::zeros( canny_output.size(), CV_8UC3 );
    for(unsigned int i = 0; i< contours.size(); i++ )
    {
        Scalar color = Scalar( rng.uniform(0, 255), rng.uniform(0,255), rng.uniform(0,255) );
        drawContours( drawing, contours, i, color, 1, 8, vector<Vec4i>(), 0, Point() );
        drawContours( drawing, hull, i, color, 1, 8, vector<Vec4i>(), 0, Point() );
    }

    int count;
    //add the contours to maskmin
    for(int x = 0; x < orig.cols; x++)
      for(int y = 0; y < orig.rows; y++)
        {
          count=0;
          for(int c=0;c<3;c++)
              count+=drawing.at<Vec3b>(y,x)[c];
          if(count>0)
              contourImage.at<Vec3b>(y,x)[1]=255;
      }

    return contourImage;
}

// Enlarge the given image by a factor of WhiteboardProcessor::SCALE
Mat WhiteboardProcessor::enlarge(const Mat& orig) {
    Mat enlarged = Mat::zeros(Size(orig.cols*SCALE, orig.rows*SCALE), orig.type());
    bool center,right,down,corner;
    bool rightIn,downIn;//,cornerIn;

    //for every color channel
    for (int c=0;c<3;c++){
        //go through the mask scalexscale box
        for (int y = 0; y < enlarged.rows; y+=SCALE)
        {
            for (int x = 0; x < enlarged.cols; x+=SCALE)
            {
                //set the location on if the corresponding location in maskMin is on
                if (orig.at<Vec3b>(y/SCALE,x/SCALE)[c]!=0)
                    center=true;
                else
                    center=false;

                //repeat for other corners of box
                if((x+SCALE)/SCALE<orig.cols){
                    rightIn=true;
                    if(orig.at<Vec3b>(y/SCALE,(x+SCALE)/SCALE)[c]!=0)
                        right=true;
                    else
                        right=false;
                } else
                    rightIn=false;

                if((y+SCALE)/SCALE<orig.rows){
                    downIn=true;
                    if(orig.at<Vec3b>((y+SCALE)/SCALE,x/SCALE)[c]!=0)
                        down=true;
                    else
                        down=false;
                } else
                    downIn=false;

                if(downIn && rightIn){
                    //cornerIn=true;
                    if(orig.at<Vec3b>((y+SCALE)/SCALE,(x+SCALE)/SCALE)[c]!=0)
                        corner=true;
                    else
                        corner=false;
                } //else
                    //cornerIn=false;

                //fill in mask based on which corners are turned on based on maskMin
                if(center)
                    enlarged.at<Vec3b>(y,x)[c]=255;
                if(center || right)
                    for(int xx=x+1; xx<enlarged.cols && xx<x+SCALE;xx++)
                        enlarged.at<Vec3b>(y,xx)[c]=255;
                if(center || down)
                    for(int yy=y+1; yy<enlarged.rows && yy<y+SCALE; yy++)
                        enlarged.at<Vec3b>(yy,x)[c]=255;
                if(center || right || down || corner)
                    for(int xx=x+1; xx<enlarged.cols && xx<x+SCALE;xx++)
                        for(int yy=y+1; yy<enlarged.rows && yy<y+SCALE; yy++)
                            enlarged.at<Vec3b>(yy,xx)[c]=255;

            }
        }
    }
    return enlarged;
}

// Covers the (filtered) difference pixels by first dilating the difference,
// then drawing a convex hull around the dilation
Mat WhiteboardProcessor::expandDifferencesRegion(const Mat& differences) {
    // Dilate the difference pixels
    Mat grownDiffs = replicateToImageBorder(differences);
    grownDiffs = sweepDown(grownDiffs);
    grownDiffs = binarize(grownDiffs, 255);
    grownDiffs = borderContentWithGreen(grownDiffs, 8);

    // Draw hull around the dilation
    Mat diffHulls = getImageContours(grownDiffs);
    diffHulls = sweepDown(diffHulls);
    diffHulls = binarize(diffHulls, 255);

    return diffHulls;
}

///////////////////////////////////////////////////////////////////
///
///   Methods to find the marker strokes
///
///////////////////////////////////////////////////////////////////

// Determine the pixels in orig where all the values of all three
// channels are greater than the threshold
Mat WhiteboardProcessor::binarize(const Mat& orig, int threshold) {
    Mat binarized = Mat::zeros(orig.size(), orig.type());
    for(int i = 0; i < orig.rows; i++) {
        for(int j = 0; j < orig.cols; j++) {
            if(orig.at<Vec3b>(i,j)[0] >= threshold &&
                    orig.at<Vec3b>(i,j)[1] >= threshold &&
                    orig.at<Vec3b>(i,j)[2] >= threshold) {
                binarized.at<Vec3b>(i,j)[0] = 255;
                binarized.at<Vec3b>(i,j)[1] = 255;
                binarized.at<Vec3b>(i,j)[2] = 255;
            } else {
                binarized.at<Vec3b>(i,j)[0] = 0;
                binarized.at<Vec3b>(i,j)[1] = 0;
                binarized.at<Vec3b>(i,j)[2] = 0;
            }
        }
    }
    return binarized;
}

// Determine the pixels in orig where the value of the blue channel
// is greater than the threshold. Ignores pixels on the image border
// Arguments:
//    orig: The image to threshold
//    threshold: The minimum value of the blue channel
//    size: The width of the image border to ignore
Mat WhiteboardProcessor::thresholdOnBlueChannel(const Mat& orig, int blueThresh, int size) {
    Mat ret = Mat::zeros(orig.size(), orig.type());
    //for every pixel
    for(int y = size; y < orig.rows - size; y++) {
        for(int x = size ; x < orig.cols - size; x++) {
            //if the blue channel in orig is greater than the theshold
            if(orig.at<Vec3b>(y,x)[0] > blueThresh) {
                //brighten edge
                ret.at<Vec3b>(y,x)[0]=255;
                ret.at<Vec3b>(y,x)[1]=255;
                ret.at<Vec3b>(y,x)[2]=255;
            }
        }
    }
    return ret;
}

// Run a specialized edge filter on the original image. The derivative in the horizontal
// direction is stored in the red channel (for vertical edges), the derivative in the
// vertical direction is stored in the green channel (for horizontal edges), and the
// sum is stored in the blue channel
Mat WhiteboardProcessor::pDrift(const Mat& orig) {
    int temp,total;
    Mat ret = Mat::zeros(orig.size(), orig.type());

    //for every pixel in image (excludeing edges where perocess would break
    for(int y = 0; y < orig.rows -1; y++)
        for(int x = 0; x < orig.cols -1; x++)
        {
            //look for edges in the vertical direction using a variation on a Sobel filter
            //[1 -1]
            temp = (
                        //y,x+1
                        abs(orig.at<Vec3b>(y,x)[0] - orig.at<Vec3b>(y,x+1)[0])+
                    abs(orig.at<Vec3b>(y,x)[1] - orig.at<Vec3b>(y,x+1)[1])+
                    abs(orig.at<Vec3b>(y,x)[2] - orig.at<Vec3b>(y,x+1)[2])
                    );

            if(temp > 255)
                temp = 255;
            //write the vertical edge information to the red color channel
            ret.at<Vec3b>(y,x)[2] = temp;
            total = temp;

            //run the same filters in the vertical direction to look for edges in the
            //horizontal direction.
            temp = (
                        //y+1,x
                        abs(orig.at<Vec3b>(y,x)[0] - orig.at<Vec3b>(y+1,x)[0])+
                    abs(orig.at<Vec3b>(y,x)[1] - orig.at<Vec3b>(y+1,x)[1])+
                    abs(orig.at<Vec3b>(y,x)[2] - orig.at<Vec3b>(y+1,x)[2])
                    );
            if(temp > 255)
                temp = 255;
            total+=temp;
            if(total > 255)
                total = 255;

            //write the horizontal edge information to the green color channel
            ret.at<Vec3b>(y,x)[1] = temp;
            //write the addition of the horizontal and vertical edges found to the blue color channel
            ret.at<Vec3b>(y,x)[0] = total;
        }
    return ret;
}

// Given approximate borders (enclosures) of the marker strokes, fill in the enclosed regions
Mat WhiteboardProcessor::fillMarkerBorders(const Mat& markerBorders) {
    //run a morphological closure (grow then shrink)
    //this will fill in spaces in text caused by only looking at edges
    int dilation_type = MORPH_RECT;
    int dilation_size = 1;
    Mat element = getStructuringElement( dilation_type,
                                         Size( 2*dilation_size + 1, 2*dilation_size+1 ),
                                         Point( dilation_size, dilation_size ) );
    Mat filledMarker;
    dilate(markerBorders, filledMarker, element);
    erode(filledMarker, filledMarker, element);
    return filledMarker;
}

// Locates marker strokes by approximating the borders for the marker strokes, then
// filling in the enclosed regions. This method is faster than the connected
// components method, but not as accurate.
Mat WhiteboardProcessor::findMarkerWithMarkerBorders(const Mat &whiteboardImage) {
    Mat temp = boxBlur(whiteboardImage, 1);
    temp = pDrift(temp);
    temp = thresholdOnBlueChannel(temp, 15, 3);
    temp = fillMarkerBorders(temp);
    return temp;
}

// Runs the Difference of Gaussians (DoG) edge detector on the given image.
// TODO: Implement the DoG detector by first subtracting the Gaussian kernels,
//       then filter the result with orig. I can't figure out how to do this.
// Arguments:
//    orig: The image to locate the edges of
//    kerSize: The kernel size for the edge detector. It must be odd, otherwise
//             the filter fails.
//    sig1: The sigma of the first Gaussian blur. Should be larger than sig2.
//    sig2: The sigma of the second Gaussian blur.
Mat WhiteboardProcessor::getDoGEdges(const Mat& orig, int kerSize, float sig1, float sig2) {
    Mat g1, g2;
    GaussianBlur(orig, g1, Size(kerSize, kerSize), sig1);
    GaussianBlur(orig, g2, Size(kerSize, kerSize), sig2);
    return g1-g2;
}

// Change the brightness levels of the given image.
// Arguments:
//    orig: The image to adjust the brightness levels of
//    lo: The low threshold of the input brightnesses
//    hi: The high threshold of the input brightnesses
//    gamma: The curvature of the level remapping
Mat WhiteboardProcessor::adjustLevels(const Mat& orig, int lo, int hi, double gamma) {
    return 255/pow(hi-lo, 1/gamma)*(orig-lo)^(1/gamma);
}

// Given a binary image, locate the connected components in the image
// WARNING: The returned array must be manually destroyed after use.
int** WhiteboardProcessor::getConnectedComponents(const Mat& components) {
    int** componentLabels = new int*[components.rows];
    for(int i = 0; i < components.rows; i++) {
        componentLabels[i] = new int[components.cols];
    }

    // The disjoint set structure that keeps track of component classes
    UF compClasses(DEFAULT_NUM_CC);
    // Counter for the components in the image
    int regCounter = 1;
    for(int i = 0; i < components.rows; i++) {
        for(int j = 0; j < components.cols; j++) {
            // Set component class if mask is white at current pixel
            if(components.at<Vec3b>(i, j)[0] == 255) {
                // Check surrounding pixels
                if(i-1 < 0) {
                    // On top boundary, so just check left
                    if(j-1 < 0) {
                        // This is the TL pixel, so set as new class
                        componentLabels[i][j] = regCounter;
                        regCounter++;
                    }
                    else if(componentLabels[i][j-1] == -1) {
                        // No left neighbor, so set pixel as new class
                        componentLabels[i][j] = regCounter;
                        regCounter++;
                    }
                    else {
                        // Assign pixel class to the same as left neighbor
                        componentLabels[i][j] = componentLabels[i][j-1];
                    }
                }
                else {
                    if(j-1 < 0) {
                        // On left boundary, so just check top
                        if(componentLabels[i-1][j] == -1) {
                            // No top neighbor, so set pixel as new class
                            componentLabels[i][j] = regCounter;
                            regCounter++;
                        }
                        else {
                            // Assign pixel class to same as top neighbor
                            componentLabels[i][j] = componentLabels[i-1][j];
                        }
                    }
                    else {
                        // Normal case (get top and left neighbor and reassign classes if necessary)
                        int topClass = componentLabels[i-1][j];
                        int leftClass = componentLabels[i][j-1];
                        if(topClass == -1 && leftClass == -1) {
                            // No neighbor exists, so set pixel as new class
                            componentLabels[i][j] = regCounter;
                            regCounter++;
                        }
                        else if(topClass == -1 && leftClass != -1) {
                            // Only left neighbor exists, so copy its class
                            componentLabels[i][j] = leftClass;
                        }
                        else if(topClass != -1 && leftClass == -1) {
                            // Only top neighbor exists, so copy its class
                            componentLabels[i][j] = topClass;
                        }
                        else {
                            // Both neighbors exist
                            int minNeighbor = std::min(componentLabels[i-1][j], componentLabels[i][j-1]);
                            int maxNeighbor = std::max(componentLabels[i-1][j], componentLabels[i][j-1]);
                            componentLabels[i][j] = minNeighbor;
                            // If we have differing neighbor values, merge them
                            if(minNeighbor != maxNeighbor) {
                                compClasses.merge(minNeighbor, maxNeighbor);
                            }
                        }
                    }
                }
            }
            else {
                // The pixel is black, so do not give a component label
                componentLabels[i][j] = -1;
            }
        }
    }
    // Unify the labels such that every pixel in a component has the same label
    for(int i=0; i < components.rows; i++) {
        for(int j=0; j < components.cols; j++) {
            componentLabels[i][j] = compClasses.find(componentLabels[i][j]);
        }
    }
    return componentLabels;
}

// Given the connected components and an additional binary image, keep the components
// that overlap with a pixel from the other binary image.
// Arguments:
//    compsImg: The connected components (labels not yet assigned)
//    keepCompLocs: The pixels that a component must overlap with to be kept
Mat WhiteboardProcessor::filterConnectedComponents(const Mat& compsImg, const Mat& keepCompLocs) {
    // Get the component labels
    int** components = getConnectedComponents(compsImg);

    // Initialize lookup table of components to keep (initially keep no components)
    vector<bool> componentsToKeep(DEFAULT_NUM_CC, false);
    for(int i=0; i < keepCompLocs.rows; i++) {
        for(int j=0; j < keepCompLocs.cols; j++) {
            // If there is a component at the pixel and the pixel from keepCompLocs is
            // white, then keep the component
            if(keepCompLocs.at<Vec3b>(i,j)[2] == 255 && components[i][j] > 0) {
                unsigned int compLabel = components[i][j];
                // Resize the lookup table if it cannot contain the found comp label
                if(compLabel >= componentsToKeep.size())
                    componentsToKeep.resize(2*compLabel, false);
                componentsToKeep[compLabel] = true;
            }
        }
    }

    // Add the components that intersected with the edge detector
    Mat filteredComps = Mat::zeros(compsImg.size(), compsImg.type());
    for(int i=0; i < keepCompLocs.rows; i++) {
        for(int j=0; j < keepCompLocs.cols; j++) {
            // If the pixel is part of a kept component, add it to the output
            int compLabel = components[i][j];
            if(compLabel > 0 && componentsToKeep[compLabel]) {
                filteredComps.at<Vec3b>(i,j)[0] = 255;
                filteredComps.at<Vec3b>(i,j)[1] = 255;
                filteredComps.at<Vec3b>(i,j)[2] = 255;
            }
        }
    }

    // Free memory used by components array
    for(int i = 0; i < compsImg.rows; i++) {
        delete [] components[i];
    }
    delete [] components;

    return filteredComps;
}

// Locate the marker strokes with the DoG and connected components approach.
// First, find the DoG edges and threshold them. Then, use pDrift to determine
// which connected components from the threholded DoG edges should be kept.
Mat WhiteboardProcessor::findMarkerWithCC(const Mat& orig) {
    imwrite("/home/paol/shared/out/orig.png", orig);
    Mat markerCandidates = getDoGEdges(orig, 13, 17, 1);
    markerCandidates = adjustLevels(markerCandidates, 0, 4, 1);
    markerCandidates = binarize(markerCandidates, 10);
    imwrite("/home/paol/shared/out/DoG.png", markerCandidates);
    Mat markerLocations = pDrift(orig);
    markerLocations = binarize(markerLocations, 10);
    imwrite("/home/paol/shared/out/pDrift.png", markerLocations);
    return filterConnectedComponents(markerCandidates, markerLocations);
}

///////////////////////////////////////////////////////////////////
///
///   Methods to enhance the whiteboard
///
///////////////////////////////////////////////////////////////////

// Do a box blurring on the original image
// Arguments:
//    orig: The image to blur
//    size: The radius of the box blur kernel
Mat WhiteboardProcessor::boxBlur(const Mat& orig, int size) {
    Mat ret;
    cv::blur(orig, ret, Size(2*size+1, 2*size+1));
    return ret;
}

// Approximate the whiteboard color at each pixel by splitting the image
// into cells, then finding the average value of the brightest 25% of
// pixels in the cell.
// Arguments:
//    whiteboardImg: The unmodified whiteboard image to extract the whiteboard colors of
//    size: The size of the cells
Mat WhiteboardProcessor::getAvgWhiteboardColor(const Mat& whiteboardImg, int size) {
    Mat ret = Mat::zeros(whiteboardImg.size(), whiteboardImg.type());
    int x,y,xx,yy;
    int count,color,thresh;
    vector <int> pix;
    vector <int> ave;

    //go through the image by squares of radius size
    for (x=0;x<whiteboardImg.cols;x+=size)
        for (y=0;y<whiteboardImg.rows;y+=size){
            pix.clear();
            ave.clear();

            //within each square create a vector pix that hold all brightness values
            //for the pixels
            for(xx=x; xx<x+size && xx<whiteboardImg.cols; xx++)
                for (yy=y; yy<y+size && yy<whiteboardImg.rows; yy++){
                    color=0;
                    for (int c=0;c<3;c++)
                        color+=whiteboardImg.at<Vec3b>(yy,xx)[c];
                    color/=3;
                    pix.push_back(color);
                }

            //clear the average pixel values
            for (int c=0;c<3;c++)
                ave.push_back(0);

            //sort the vector of brightness pixels (low to high)
            sort(pix.begin(),pix.end());
            //figure out what the brightness theshold is for the brightest 25%
            thresh=pix[pix.size()*3/4];
            count=0;

            //for all the pixels in the square add the color components to the averge vector
            //if the brightness is over the theshold
            for(xx=x; xx<x+size && xx<whiteboardImg.cols; xx++)
                for (yy=y; yy<y+size && yy<whiteboardImg.rows; yy++){
                    color=0;
                    for (int c=0;c<3;c++)
                        color+=whiteboardImg.at<Vec3b>(yy,xx)[c];
                    color/=3;

                    if(color>=thresh){
                        count++;
                        for (int c=0;c<3;c++)
                            ave[c]+=whiteboardImg.at<Vec3b>(yy,xx)[c];
                    }
                }
            //figure out the average brightness of each channel for the brightest pixels
            for (int c=0;c<3;c++)
                ave[c]/=count;

            //set the pixels in the mask to the average brightness of the image, square by square
            for(xx=x; xx<x+size && xx<whiteboardImg.cols; xx++)
                for (yy=y; yy<y+size && yy<whiteboardImg.rows; yy++){
                    for (int c=0;c<3;c++)
                        ret.at<Vec3b>(yy,xx)[c]=ave[c];
                }
        }
    return ret;
}

// Given a whiteboard image, darken the text by scaling the difference from the average whiteboard color
Mat WhiteboardProcessor::raiseMarkerContrast(const Mat& whiteboardImg){
    Mat hiContrastImg = Mat::zeros(whiteboardImg.size(), whiteboardImg.type());
    Mat avg = getAvgWhiteboardColor(whiteboardImg, 10);

    //for every pixel in the image and for every color channel
    for(int x = 0; x < whiteboardImg.cols; x++)
        for(int y = 0; y < whiteboardImg.rows; y++){
            for(int c=0;c<3;c++){
                int dif;
                //if the pixel is not 0 (just put in so that we don't divide by 0)
                if (whiteboardImg.at<Vec3b>(y,x)[c]>0){
                    //take the brightness of the pixel and divide it by what white is in
                    //that location (average from mask)
                    dif=255*whiteboardImg.at<Vec3b>(y,x)[c]/avg.at<Vec3b>(y,x)[c];
                    //if it's brighter then white turn it white
                    if (dif>255)
                        dif=255;
                } else {
                    //if the average pixel color is 0 turn it white
                    dif=255;
                }

                //double the distance each color is from white to make text darker
                dif=255-(255-dif)*2;
                if (dif<0)
                    dif=0;
                hiContrastImg.at<Vec3b>(y,x)[c]=dif;
            }
        }
    return hiContrastImg;
}

// Given a whiteboard image and the marker locations, make the non-marker pixels white
// Arguments:
//    whiteboardImg: The whiteboard image to modify
//    markerPixels: A binary image where the marker pixel locations are white
Mat WhiteboardProcessor::whitenWhiteboard(const Mat& whiteboardImg, const Mat& markerPixels) {
    Mat ret = whiteboardImg.clone();
    //for every pixel
    for(int y = 0; y < whiteboardImg.rows; y++)
        for(int x = 0; x < whiteboardImg.cols; x++){
            //if there isn't and edge (text) in that location turn the pixel white
            if (markerPixels.at<Vec3b>(y,x)[1]<50){
                ret.at<Vec3b>(y,x)[0]=255;
                ret.at<Vec3b>(y,x)[1]=255;
                ret.at<Vec3b>(y,x)[2]=255;
            }
        }
    return ret;
}

// Skew the given image so the whiteboard region is rectangular
// TODO: Modify this method so it takes in a structure of corner coordinates
Mat WhiteboardProcessor::rectifyImage(const Mat& whiteboardImg){
    Mat ret = Mat::zeros(whiteboardImg.size(), whiteboardImg.type());
    double widthP,heightP;
    double LTx,LTy,LBx,LBy,RTx,RTy,RBx,RBy;//L left R right T top B bottom
    LTx=354;
    LTy=236;
    LBx=444;
    LBy=706;
    RTx=1915;
    RTy=260;
    RBx=1825;
    RBy=727;
    int xInput,yInput;
    double LPx,LPy,RPx,RPy;//end points of line between edges on which point is found

    for(int x = 0; x < ret.cols; x++)
        for(int y = 0; y < ret.rows; y++){
            widthP=(double)x/(double)ret.cols;
            heightP=(double)y/(double)ret.rows;
            LPx=LTx+(LBx-LTx)*heightP;
            LPy=LTy+(LBy-LTy)*heightP;
            RPx=RTx+(RBx-RTx)*heightP;
            RPy=RTy+(RBy-RTy)*heightP;

            xInput=(int)(LPx+(RPx-LPx)*widthP);
            yInput=(int)(LPy+(RPy-LPy)*widthP);

            if (xInput >= 0 &&
                    xInput < whiteboardImg.cols &&
                    yInput >= 0 &&
                    yInput < whiteboardImg.rows){
                ret.at<Vec3b>(y,x)[0] = whiteboardImg.at<Vec3b>(yInput,xInput)[0];
                ret.at<Vec3b>(y,x)[1] = whiteboardImg.at<Vec3b>(yInput,xInput)[1];
                ret.at<Vec3b>(y,x)[2] = whiteboardImg.at<Vec3b>(yInput,xInput)[2];
            } else {
                ret.at<Vec3b>(y,x)[0]=0;
                ret.at<Vec3b>(y,x)[1]=0;
                ret.at<Vec3b>(y,x)[2]=0;
            }
        }
    return ret;
}

// Given a whiteboard image, draw red lines through the long straight lines of
// the image.
Mat WhiteboardProcessor::findWhiteboardBorders(Mat& whiteboardImg) {
    // Find edges with Canny detector
    Mat cannyEdges;
    Canny(whiteboardImg, cannyEdges, 50, 200, 3);

    // detect lines
    vector<Vec2f> lines;
    HoughLines(cannyEdges, lines, 1, CV_PI/180, 250, 0, 0 );

    // draw lines
    Mat ret = whiteboardImg.clone();
    for( size_t i = 0; i < lines.size(); i++ )
    {
        float rho = lines[i][0], theta = lines[i][1];
        Point pt1, pt2;
        double a = cos(theta), b = sin(theta);
        double x0 = a*rho, y0 = b*rho;
        pt1.x = cvRound(x0 + 2000*(-b));
        pt1.y = cvRound(y0 + 2000*(a));
        pt2.x = cvRound(x0 - 2000*(-b));
        pt2.y = cvRound(y0 - 2000*(a));
        line( ret, pt1, pt2, Scalar(0,0,255), 3, CV_AA);
    }
    return ret;
}

// Make marker-whiteboard transition smoother
Mat WhiteboardProcessor::smoothMarkerTransition(const Mat& whiteWhiteboardImage) {
    Mat blurred;
    GaussianBlur(whiteWhiteboardImage, blurred, Size(5,5), .8);
    // Overlay blurred version with pixels from whiteWhiteboardImage
    for(int i = 0; i < whiteWhiteboardImage.rows; i++) {
        for(int j = 0; j < whiteWhiteboardImage.cols; j++) {
            // If whiteWhiteboardImage is not white at this pixel, it is marker, so fill in marker color
            if(whiteWhiteboardImage.at<Vec3b>(i,j)[0] != 255
                    || whiteWhiteboardImage.at<Vec3b>(i,j)[1] != 255
                    || whiteWhiteboardImage.at<Vec3b>(i,j)[2] != 255) {
                blurred.at<Vec3b>(i, j)[0] = whiteWhiteboardImage.at<Vec3b>(i,j)[0];
                blurred.at<Vec3b>(i, j)[1] = whiteWhiteboardImage.at<Vec3b>(i,j)[1];
                blurred.at<Vec3b>(i, j)[2] = whiteWhiteboardImage.at<Vec3b>(i,j)[2];
            }
        }
    }
    return blurred;
}

///////////////////////////////////////////////////////////////////
///
///   Update the background (whiteboard) model
///
///////////////////////////////////////////////////////////////////

// Arguments:
//    oldWboardModel: The previous image of the whiteboard
//    newInfo: The enhanced version of the current whiteboard image, which includes the professor
//    mvmtInfo: A binary image indicating where there was significant movement (ie. where the
//              professor might be)
Mat WhiteboardProcessor::updateWhiteboardModel(const Mat& oldWboardModel, const Mat& newInfo, const Mat& mvmtInfo) {
    // Throw exception if one of the given arguments has no image data
    // Mostly useful to protect against updating a nonexistent whiteboard model
    if(!oldWboardModel.data || !newInfo.data || !mvmtInfo.data) {
        throw std::invalid_argument("updateBack3: Attempted to update a whiteboard model with missing data.");
    }

    Mat updatedModel = oldWboardModel.clone();

    //for every pixel in the image
    for (int y = 0; y < updatedModel.rows; y++) {
        for (int x = 0; x < updatedModel.cols; x++) {
            //if there was no movement at that pixel
            if (mvmtInfo.at<Vec3b>(y,x)[0] == 0) {
                //update the whiteboard model at that pixel
                for (int c=0;c<3;c++){
                    updatedModel.at<Vec3b>(y,x)[c]=newInfo.at<Vec3b>(y,x)[c];
                }
            }
        }
    }

    return updatedModel;
}

///////////////////////////////////////////////////////////////////
///
///   Method to get debugging frames
///
///////////////////////////////////////////////////////////////////
vector<Mat> WhiteboardProcessor::getDebugFrames() {
    return debugFrames;
}
