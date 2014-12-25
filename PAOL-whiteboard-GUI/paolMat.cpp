#include "paolMat.h"
#include "uf.h"
#include <map>
#include <typeinfo>
#include <iostream>
#include <stdexcept>
#include <set>

paolMat::~paolMat()
{
    if(cam.isOpened())
        cam.release();
}

bool paolMat::initWebcam(int i) {
    // Reject negative input values
    if(i < 0) {
        qWarning("setCameraNum2: Attempted to open with an invalid webcam number %d.", i);
        return false;
    }

    // Initialize the camera and check if it is valid
    cam = VideoCapture(i);
    if(!cam.isOpened()) {
        qWarning("setCameraNum2: Failed to open /dev/video%d.", i);
        return false;
    }

    //set parameters for camera
    cam.set(CV_CAP_PROP_FRAME_WIDTH, 1920);
    cam.set(CV_CAP_PROP_FRAME_HEIGHT, 1080);

    return true;
}

bool paolMat::takePicture2(Mat& destination) {
    Mat temp;
    //grab 5 consecutive images to clear camera buffer
    for (int i = 0; i < 5;i++) {
        cam >> temp;
    }
    if(temp.data) {
        qDebug("Successfully took a webcam picture.");
        destination = temp.clone();
        return true;
    }
    else {
        qWarning("Failed to take a webcam picture.");
        return false;
    }
}

bool paolMat::initDataSetReadProps(QString firstImageLoc) {
    // Handle case where the given path is empty
    if(firstImageLoc.length() == 0) {
        qWarning("initDataSetReadProps: The given file path was empty.");
        return false;
    }

    // Convert QString to std::string
    string locAsString = firstImageLoc.toStdString();
    // Get the data set directory by cutting off whatever is after the last "/"
    dataSetDir = locAsString.substr(0, locAsString.find_last_of("/"));

    // Parse file name and get time and index of the next frame to read (ie. of the given image)
    // Set format of the first image location and use it to scan for next frame index and time
    string scanFormat = dataSetDir + "/cameraIn%06d-%10d-%d.png";
    int scanResult = sscanf(locAsString.c_str(), scanFormat.c_str(), &nextFrameIndex, &nextFrameTime, &datasetCamNum);

    // sscanf should have found three arguments
    if(scanResult == 3) {
        return true;
    }
    else {
        qWarning("initDataSetReadProps: The wrong number of items were found in the scanned string.");
        return false;
    }
}

// Read the next frame and save it to the destination
// Return true if the read was a success, false otherwise
bool paolMat::readNext2(Mat& destination) {
    // Attempt to read the next file using the stored time and index of the next frame
    char nextFrameLoc[256];
    Mat temp;

    // Search for next image up to TIME_SKIP_LIMIT times
    for(int searchCount = 0; searchCount < TIME_SKIP_LIMIT; searchCount++) {
        // Set the index and time to look for
        int tempFrameIndex = nextFrameIndex + searchCount;
        int tempFrameTime = nextFrameTime + searchCount;

        // Attempt to read the file with the temp frame index and time
        sprintf(nextFrameLoc, "%s/cameraIn%06d-%10d-%d.png", dataSetDir.c_str(), tempFrameIndex, tempFrameTime, datasetCamNum);
        temp = imread(nextFrameLoc, CV_LOAD_IMAGE_COLOR);

        // Update destination, next frame index and time if read was successful
        if(temp.data) {
            qDebug("Successfully read %s", nextFrameLoc);
            destination = temp.clone();
            nextFrameIndex = tempFrameIndex + 1;
            nextFrameTime = tempFrameTime + 1;
            return true;
        }
    }
    qDebug("Failed to read the next file.");
    return false;
}

// Process differences between shrunken versions of the two given frames
void paolMat::differenceMin2(Mat& diffLocations, float& numDiff, const Mat& oldImg, const Mat& newImg, int thresh, int size) {
    int offset;
    bool diff;
    int surroundThresh = 50;

    //maskMin is set to a blank state (1/(scale*scale)) the size of src
    diffLocations=Mat::zeros(oldImg.rows/SCALE,oldImg.cols/SCALE,oldImg.type());

    if (SCALE>=size+1)
        offset=SCALE;
    else
        offset=SCALE*2;

  numDiff = 0;

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
              numDiff++;
              diffLocations.at<Vec3b>(yy,xx)[2]=255;
          }
      }
  }
  //return the percent of maskMin that are differences
  numDiff = (float)numDiff/(float)(diffLocations.rows*diffLocations.cols);
}

// Filter out pixels that are not surrounded by enough difference pixels
void paolMat::shrinkMaskMin2(Mat& filteredDiffs, float& numDiff, const Mat& origDiffs) {
    filteredDiffs = Mat::zeros(origDiffs.size(), origDiffs.type());
    int total;
    numDiff = 0;
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
                numDiff++;
            }
        }
    //return the percentage of the maskMin that are valid differences
    numDiff = (float)numDiff/(float)(origDiffs.rows*origDiffs.cols);
}

Mat paolMat::extendMaskMinToEdges2(const Mat& orig) {
    Mat ret = orig.clone();
    int x;

    //extend bottom and top edge where differences overlap
    for(x=0;x<ret.cols;x++){
        if(ret.at<Vec3b>(1,x)[1] == 255)
            ret.at<Vec3b>(0,x)[1]=255;
        if(ret.at<Vec3b>(ret.rows-2,x)[1] == 255)
            ret.at<Vec3b>(ret.rows-1,x)[1]=255;
    }

    //extend right and left edge wher differences overlap
    for(int y = 0; y < ret.rows; y++)
    {
        if(ret.at<Vec3b>(y,1)[1] == 255)
            ret.at<Vec3b>(y,0)[1] = 255;

        if(ret.at<Vec3b>(y,ret.cols-2)[1] == 255)
            ret.at<Vec3b>(y,ret.cols-1)[1] = 255;
    }
    return ret;
}

Mat paolMat::sweepDownMin2(const Mat& orig){
    bool left,right,top;
    //create a Mat the size of orig to store results
    Mat ret=Mat::zeros(orig.size(),orig.type());

    //from left to right
    for(int x = 0; x < orig.cols; x++)
    {
        //from top to bottom
        top = false;
        for(int y = 0; y < orig.rows; y++){
            if(orig.at<Vec3b>(y,x)[1] == 255)
                top = true;

            if(top == true)
                ret.at<Vec3b>(y,x)[0] = 255;
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
                ret.at<Vec3b>(y,x)[1] = 255;
        }

        //sweep from the right
        right = false;
        for(int x = orig.cols-1; x >-1; x--)
        {
            if(orig.at<Vec3b>(y,x)[1] == 255)
                right = true;

            if(right == true)
                ret.at<Vec3b>(y,x)[2] = 255;
        }
    }

    return ret;
}

Mat paolMat::keepWhiteMaskMin2(const Mat& orig){
    Mat ret = Mat::zeros(orig.size(), orig.type());
    int x;

    //keep only the white pixels
    for(x = 0; x < orig.cols; x++)
    {
        for(int y = 0; y < orig.rows; y++)
        {
            if(orig.at<Vec3b>(y,x)[0] == 255 &&
                 orig.at<Vec3b>(y,x)[1] == 255 &&
                 orig.at<Vec3b>(y,x)[2] == 255) {
                for (int i=0;i<3;i++)
                    ret.at<Vec3b>(y,x)[i]=255;
            }
        }
    }
    return ret;
}

Mat paolMat::growMin2(const Mat& orig, int size)
{
    Mat ret = orig.clone();
    int startx,endx,starty,endy;

    //for every pixel in the image
    for(int y = 0; y < ret.rows; y++)
        for(int x = 0; x < ret.cols; x++)

            //if the pixel is turned on
            if(ret.at<Vec3b>(y,x)[0] == 255){
                startx=x-size;
                if (startx<0)
                    startx=0;

                starty=y-size;
                if (starty<0)
                    starty=0;

                endx=x+size;
                if (endx>=ret.cols)
                    endx=ret.cols-1;

                endy=y+size;
                if (endy>=ret.rows)
                    endy=ret.rows-1;

                //grow the region around that pixel
                for(int yy = starty; yy <= endy;yy++)
                    for(int xx = startx; xx <= endx; xx++)
                        ret.at<Vec3b>(yy,xx)[1] = 255;
            }
    return ret;
}

Mat paolMat::growMin3(const Mat& orig, int size) {
    Mat ret = orig.clone();
    Mat element = getStructuringElement(MORPH_RECT, Size(2*size+1,2*size+1));
    dilate(orig, ret, element);
    return ret;
}

Mat paolMat::findContoursMaskMin2(const Mat& orig) {
    Mat ret = orig.clone();
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
              ret.at<Vec3b>(y,x)[1]=255;
      }

    return ret;
}

Mat paolMat::maskMinToMaskBinary2(const Mat& orig) {
    Mat ret = Mat::zeros(Size(orig.cols*SCALE, orig.rows*SCALE), orig.type());
    bool center,right,down,corner;
    bool rightIn,downIn;//,cornerIn;

    //for every color channel
    for (int c=0;c<3;c++){
        //go through the mask scalexscale box
        for (int y = 0; y < ret.rows; y+=SCALE)
        {
            for (int x = 0; x < ret.cols; x+=SCALE)
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
                    ret.at<Vec3b>(y,x)[c]=255;
                if(center || right)
                    for(int xx=x+1; xx<ret.cols && xx<x+SCALE;xx++)
                        ret.at<Vec3b>(y,xx)[c]=255;
                if(center || down)
                    for(int yy=y+1; yy<ret.rows && yy<y+SCALE; yy++)
                        ret.at<Vec3b>(yy,x)[c]=255;
                if(center || right || down || corner)
                    for(int xx=x+1; xx<ret.cols && xx<x+SCALE;xx++)
                        for(int yy=y+1; yy<ret.rows && yy<y+SCALE; yy++)
                            ret.at<Vec3b>(yy,xx)[c]=255;

            }
        }
    }
    return ret;
}

// The result does not quite scale the original perfectly, but it should suffice...
Mat paolMat::maskMinToMaskBinary3(const Mat& orig, int scale) {
    Mat ret;
    cv::resize(orig, ret, Size(orig.rows*scale, orig.cols*scale), 0, 0, INTER_NEAREST);
    return ret;
}

Mat paolMat::blur2(const Mat& orig, int size) {
    Mat ret;
    cv::blur(orig, ret, Size(2*size+1, 2*size+1));
    return ret;
}

Mat paolMat::thresholdOnBlue(const Mat& orig, int blueThresh, int size) {
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

// Given the current frame, previous frame, and foreground (lecturer) location,
// return the current frame with the foreground pixels filled in with the previous
Mat paolMat::updateBack3(const Mat& oldWboardModel, const Mat& newInfo, const Mat& mvmtInfo) {
    // Throw exception if one of the given arguments has no image data
    // Mostly useful to protect against updating a nonexistent whiteboard model
    if(!oldWboardModel.data || !newInfo.data || !mvmtInfo.data) {
        throw std::invalid_argument("updateBack3: Attempted to update a whiteboard model with missing data.");
    }

    Mat ret = oldWboardModel.clone();

    //for every pixel in the image
    for (int y = 0; y < ret.rows; y++) {
        for (int x = 0; x < ret.cols; x++) {
            //if there was no movement at that pixel
            if (mvmtInfo.at<Vec3b>(y,x)[0] == 0) {
                //update the whiteboard model at that pixel
                for (int c=0;c<3;c++){
                    ret.at<Vec3b>(y,x)[c]=newInfo.at<Vec3b>(y,x)[c];
                }
            }
        }
    }

    return ret;
}

Mat paolMat::averageWhiteboard2(const Mat& orig, int size) {
    Mat ret = Mat::zeros(orig.size(), orig.type());
    int x,y,xx,yy;
    int count,color,thresh;
    vector <int> pix;
    vector <int> ave;

    //go through the image by squares of radius size
    for (x=0;x<orig.cols;x+=size)
        for (y=0;y<orig.rows;y+=size){
            pix.clear();
            ave.clear();

            //within each square create a vector pix that hold all brightness values
            //for the pixels
            for(xx=x; xx<x+size && xx<orig.cols; xx++)
                for (yy=y; yy<y+size && yy<orig.rows; yy++){
                    color=0;
                    for (int c=0;c<3;c++)
                        color+=orig.at<Vec3b>(yy,xx)[c];
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
            for(xx=x; xx<x+size && xx<orig.cols; xx++)
                for (yy=y; yy<y+size && yy<orig.rows; yy++){
                    color=0;
                    for (int c=0;c<3;c++)
                        color+=orig.at<Vec3b>(yy,xx)[c];
                    color/=3;

                    if(color>=thresh){
                        count++;
                        for (int c=0;c<3;c++)
                            ave[c]+=orig.at<Vec3b>(yy,xx)[c];
                    }
                }
            //figure out the average brightness of each channel for the brightest pixels
            for (int c=0;c<3;c++)
                ave[c]/=count;

            //set the pixels in the mask to the average brightness of the image, square by square
            for(xx=x; xx<x+size && xx<orig.cols; xx++)
                for (yy=y; yy<y+size && yy<orig.rows; yy++){
                    for (int c=0;c<3;c++)
                        ret.at<Vec3b>(yy,xx)[c]=ave[c];
                }
        }
    return ret;
}

//Use the average image to turn the whiteboard of the image white and to darken the text
Mat paolMat::enhanceText2(const Mat& orig){
    Mat ret = Mat::zeros(orig.size(), orig.type());
    Mat avg = averageWhiteboard2(orig, 10);

    //for every pixel in the image and for every color channel
    for(int x = 0; x < orig.cols; x++)
        for(int y = 0; y < orig.rows; y++){
            for(int c=0;c<3;c++){
                int dif;
                //if the pixel is not 0 (just put in so that we don't divide by 0)
                if (orig.at<Vec3b>(y,x)[c]>0){
                    //take the brightness of the pixel and divide it by what white is in
                    //that location (average from mask)
                    dif=255*orig.at<Vec3b>(y,x)[c]/avg.at<Vec3b>(y,x)[c];
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
                ret.at<Vec3b>(y,x)[c]=dif;
            }
        }
    return ret;
}

/////////////////// Code that just finds marker with Mats

Mat paolMat::dogEdges2(const Mat& orig, int kerSize, int rad1, int rad2) {
    Mat g1, g2;
    GaussianBlur(orig, g1, Size(kerSize, kerSize), rad1);
    GaussianBlur(orig, g2, Size(kerSize, kerSize), rad2);
    return g1-g2;
}

Mat paolMat::adjustLevels2(const Mat& orig, int lo, int hi, double gamma) {
    return 255/pow(hi-lo, 1/gamma)*(orig-lo)^(1/gamma);
}

Mat paolMat::binarizeMask2(const Mat& orig, int threshold) {
    Mat ret = Mat::zeros(orig.size(), orig.type());
    for(int i = 0; i < orig.rows; i++) {
        for(int j = 0; j < orig.cols; j++) {
            if(orig.at<Vec3b>(i,j)[0] > threshold &&
                    orig.at<Vec3b>(i,j)[1] > threshold &&
                    orig.at<Vec3b>(i,j)[2] > threshold) {
                ret.at<Vec3b>(i,j)[0] = 255;
                ret.at<Vec3b>(i,j)[1] = 255;
                ret.at<Vec3b>(i,j)[2] = 255;
            } else {
                ret.at<Vec3b>(i,j)[0] = 0;
                ret.at<Vec3b>(i,j)[1] = 0;
                ret.at<Vec3b>(i,j)[2] = 0;
            }
        }
    }
    return ret;
}

int** paolMat::getConnectedComponents2(const Mat& orig) {
    int** a = new int*[orig.rows];
    for(int i = 0; i < orig.rows; i++) {
        a[i] = new int[orig.cols];
    }

    // The disjoint set structure that keeps track of component classes
    UF compClasses;
    // Counter for the regions in the image
    int regCounter = 1;
    for(int i = 0; i < orig.rows; i++) {
        for(int j = 0; j < orig.cols; j++) {
            // Set component class if mask is white at current pixel
            if(orig.at<Vec3b>(i, j)[0] == 255) {
                // Check surrounding pixels
                if(i-1 < 0) {
                    // On top boundary, so just check left
                    if(j-1 < 0) {
                        // This is the TL pixel, so set as new class
                        a[i][j] = regCounter;
                        compClasses.addClass(regCounter);
                        regCounter++;
                    }
                    else if(a[i][j-1] == -1) {
                        // No left neighbor, so set pixel as new class
                        a[i][j] = regCounter;
                        compClasses.addClass(regCounter);
                        regCounter++;
                    }
                    else {
                        // Assign pixel class to the same as left neighbor
                        a[i][j] = a[i][j-1];
                    }
                }
                else {
                    if(j-1 < 0) {
                        // On left boundary, so just check top
                        if(a[i-1][j] == -1) {
                            // No top neighbor, so set pixel as new class
                            a[i][j] = regCounter;
                            compClasses.addClass(regCounter);
                            regCounter++;
                        }
                        else {
                            // Assign pixel class to same as top neighbor
                            a[i][j] = a[i-1][j];
                        }
                    }
                    else {
                        // Normal case (get top and left neighbor and reassign classes if necessary)
                        int topClass = a[i-1][j];
                        int leftClass = a[i][j-1];
                        if(topClass == -1 && leftClass == -1) {
                            // No neighbor exists, so set pixel as new class
                            a[i][j] = regCounter;
                            compClasses.addClass(regCounter);
                            regCounter++;
                        }
                        else if(topClass == -1 && leftClass != -1) {
                            // Only left neighbor exists, so copy its class
                            a[i][j] = leftClass;
                        }
                        else if(topClass != -1 && leftClass == -1) {
                            // Only top neighbor exists, so copy its class
                            a[i][j] = topClass;
                        }
                        else {
                            // Both neighbors exist
                            int minNeighbor = std::min(a[i-1][j], a[i][j-1]);
                            int maxNeighbor = std::max(a[i-1][j], a[i][j-1]);
                            a[i][j] = minNeighbor;
                            // If we have differing neighbor values, merge them
                            if(minNeighbor != maxNeighbor) {
                                compClasses.merge(minNeighbor, maxNeighbor);
                            }
                        }
                    }
                }
            }
            else {
                a[i][j] = -1;
            }
        }
    }
    for(int i=0; i < orig.rows; i++) {
        for(int j=0; j < orig.cols; j++) {
            a[i][j] = compClasses.find(a[i][j]);
        }
    }

    return a;
}

Mat paolMat::pDrift2(const Mat& orig) {
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

Mat paolMat::addComponentsFromMask2(const Mat& compsImg, const Mat& edgeImg) {
    Mat ret = Mat::zeros(compsImg.size(), compsImg.type());
    int** components = getConnectedComponents2(compsImg);

    std::set<int> componentsToKeep;
    // Go through mask and keep track of components that intersect with the edge detector
    for(int i=0; i < edgeImg.rows; i++) {
        for(int j=0; j < edgeImg.cols; j++) {
            if(edgeImg.at<Vec3b>(i,j)[2] == 255 && components[i][j] > 0) {
                componentsToKeep.insert(components[i][j]);
            }
        }
    }
    // Turn off components that did not intersect with the edge detector
    for(int i=0; i < edgeImg.rows; i++) {
        for(int j=0; j < edgeImg.cols; j++) {
            if(componentsToKeep.find(components[i][j]) != componentsToKeep.end()) {
                // Component should be kept
                ret.at<Vec3b>(i,j)[0] = 255;
                ret.at<Vec3b>(i,j)[1] = 255;
                ret.at<Vec3b>(i,j)[2] = 255;
            }
        }
    }

    // Free memory used by components array
    for(int i = 0; i < compsImg.rows; i++) {
        delete [] components[i];
    }
    delete [] components;

    return ret;
}

// New method to find marker using DoG
Mat paolMat::findMarker(const Mat& orig) {
    Mat markerCandidates = dogEdges2(orig, 13, 17, 1);
    markerCandidates = adjustLevels2(markerCandidates, 0, 4, 1);
    markerCandidates = binarizeMask2(markerCandidates, 10);
    Mat markerLocations = pDrift2(orig);
    markerLocations = binarizeMask2(markerLocations, 10);
    return addComponentsFromMask2(markerCandidates, markerLocations);
}

Mat paolMat::fillMarkerBorders(const Mat& grownEdges) {
    //run a morphological closure (grow then shrink)
    //this will fill in spaces in text caused by only looking at edges
    int dilation_type = MORPH_RECT;
    int dilation_size = 1;
    Mat element = getStructuringElement( dilation_type,
                                         Size( 2*dilation_size + 1, 2*dilation_size+1 ),
                                         Point( dilation_size, dilation_size ) );
    Mat temp;
    dilate(grownEdges, temp, element);
    erode(temp, temp, element);
    return temp;
}

// Old, faster method to find marker
Mat paolMat::findMarker2(const Mat &orig) {
    Mat temp = blur2(orig, 1);
    temp = pDrift2(temp);
    temp = thresholdOnBlue(temp, 15, 3);
    temp = fillMarkerBorders(temp);
    return temp;
}

Mat paolMat::darkenText3(const Mat& orig, const Mat& marker) {
    Mat ret = orig.clone();
    //for every pixel
    for(int y = 0; y < orig.rows; y++)
        for(int x = 0; x < orig.cols; x++){
            //if there isn't and edge (text) in that location turn the pixel white
            if (marker.at<Vec3b>(y,x)[1]<50){
                ret.at<Vec3b>(y,x)[0]=255;
                ret.at<Vec3b>(y,x)[1]=255;
                ret.at<Vec3b>(y,x)[2]=255;
            }
        }
    return ret;
}

// Covers the difference pixels by first dilating the difference, then drawing
// a convex hull around the dilation
Mat paolMat::expandDifferencesRegion(const Mat& differences) {
    // Dilate the difference pixels
    Mat grownDiffs = extendMaskMinToEdges2(differences);
    grownDiffs = sweepDownMin2(grownDiffs);
    grownDiffs = keepWhiteMaskMin2(grownDiffs);
    grownDiffs = growMin2(grownDiffs, 8);

    // Draw hull around the dilation
    Mat diffHulls = findContoursMaskMin2(grownDiffs);
    diffHulls = sweepDownMin2(diffHulls);
    diffHulls = keepWhiteMaskMin2(diffHulls);

    return diffHulls;
}

Mat paolMat::rectifyImage2(const Mat& orig){
    Mat ret = Mat::zeros(orig.size(), orig.type());
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
                    xInput < orig.cols &&
                    yInput >= 0 &&
                    yInput < orig.rows){
                ret.at<Vec3b>(y,x)[0] = orig.at<Vec3b>(yInput,xInput)[0];
                ret.at<Vec3b>(y,x)[1] = orig.at<Vec3b>(yInput,xInput)[1];
                ret.at<Vec3b>(y,x)[2] = orig.at<Vec3b>(yInput,xInput)[2];
            } else {
                ret.at<Vec3b>(y,x)[0]=0;
                ret.at<Vec3b>(y,x)[1]=0;
                ret.at<Vec3b>(y,x)[2]=0;
            }
        }
    return ret;
}

Mat paolMat::findBoard2(Mat& orig) {
    // Find edges with Canny detector
    Mat cannyEdges;
    Canny(orig, cannyEdges, 50, 200, 3);

    // detect lines
    vector<Vec2f> lines;
    HoughLines(cannyEdges, lines, 1, CV_PI/180, 250, 0, 0 );

    // draw lines
    Mat ret = orig.clone();
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
