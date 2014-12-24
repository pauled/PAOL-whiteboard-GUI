#include "paolMat.h"
#include "uf.h"
#include <map>
#include <typeinfo>
#include <iostream>
#include <stdexcept>
#include <set>

paolMat::paolMat()
{
    cameraNum=-1;
//    scale=8;
}

paolMat::paolMat(paolMat* m)
{
    if(m->src.data)
        src = m->src.clone();
    if(m->mask.data)
        mask = m->mask.clone();
    if(m->maskMin.data)
        maskMin = m->maskMin.clone();

    cameraNum=m->cameraNum;
    sprintf(readName,"%s",m->readName);
    countRead=m->countRead;
    time=m->time;
    dirOut=m->dirOut;
//    scale=m->scale;
}

paolMat::~paolMat()
{
    if(src.data)
        src.~Mat();
    if(mask.data)
        mask.~Mat();
    if(display.data)
        display.~Mat();
    if(displayMin.data)
        displayMin.~Mat();
    if(maskMin.data)
        maskMin.~Mat();
    if(cam.isOpened())
        cam.release();
}

void paolMat::copy(paolMat *m){
    //clear out existing images and copy in new
    if(src.data)
        src.~Mat();
    if(mask.data)
        mask.~Mat();
    if(maskMin.data)
        maskMin.~Mat();

    if(m->src.data)
        src = m->src.clone();
    if(m->mask.data)
        mask = m->mask.clone();
    if(m->maskMin.data)
        maskMin = m->maskMin.clone();

    cameraNum=m->cameraNum;
    sprintf(readName,"%s",m->readName);
    countRead=m->countRead;
    time=m->time;
    dirOut=m->dirOut;
//    scale=m->scale;
}

void paolMat::copyClean(paolMat *m){
    //clear out existing images and make blank
    if(src.data)
        src.~Mat();
    if(mask.data)
        mask.~Mat();
    if(maskMin.data)
        maskMin.~Mat();

    if(m->src.data){
        src = Mat::zeros(m->src.size(),m->src.type());
        mask = Mat::zeros(m->src.size(),m->src.type());
        maskMin = Mat::zeros(m->src.size(),m->src.type());
    }

    cameraNum=m->cameraNum;
    sprintf(readName,"%s",m->readName);
    countRead=m->countRead;
    time=m->time;
    dirOut=m->dirOut;
//    scale=m->scale;
}

void paolMat::copyMaskMin(paolMat *m){
    if(maskMin.data)
        maskMin.~Mat();

    if(m->maskMin.data)
        maskMin = m->maskMin.clone();
}

void paolMat::copyMask(paolMat *m){
    if(mask.data)
        mask.~Mat();

    if(m->mask.data)
        mask = m->mask.clone();
}

void paolMat::setCameraNum(int i){
    cam=VideoCapture(i);

    //set parameters for camera
    cam.set(CV_CAP_PROP_FRAME_WIDTH, 1920);
    cam.set(CV_CAP_PROP_FRAME_HEIGHT, 1080);

    //cam.set(CV_CAP_PROP_FRAME_WIDTH, 320);
    //cam.set(CV_CAP_PROP_FRAME_HEIGHT, 240);
}

void paolMat::takePicture(){
    //grab 5 consecutive images to clear camera buffer
    for (int i=0;i<5;i++){
        cam>>src;
    }
}

bool paolMat::readNext(QWidget *fg){
    std::string readFileName;
    int lastLoaded, lastCountRead;
    int lastRead;
    int numberImagesTest = 300;
    int timeCheckOver = 300;

    //if no image has yet been loaded open a dialog to let the user select the first image
    if (cameraNum==-1){
        //open dialog window to find first image, return file name and path
        QString filename=QFileDialog::getOpenFileName(fg,fg->tr("Open First Image of Sequence"),".",
                                                      fg->tr("Image Files (*.png *.bmp *.jpg *.JPG)"));
        //split the filename into image name and directory path
        QStringList pieces = filename.split( "/" );
        QString imagename = pieces.value( pieces.length() - 1 );
        QString directory = pieces[0]+"";
        for (int i=0;i<pieces.length()-1;i++){
            directory+=pieces[i]+"/";
        }
        //convert filename and directory to std strings
        dirOut=directory.toUtf8().constData();
        std::string text = imagename.toUtf8().constData();
        //read in image name and information
        sscanf(text.c_str(),"%[^0-9]%06d-%10d-%d.png",readName,&countRead,&time,&cameraNum);
    }

    //increase image number and create new image name
    countRead++;
    char tempOut[256];
    sprintf(tempOut,"%s%s%06d-%10d-%d.png",dirOut.c_str(),readName,countRead,time,cameraNum);

    qDebug("readName:%s \n fullPath:%s\n",readName,tempOut);
    //clear src mat and try to read image name
    if(src.data)
        src.~Mat();
    src=imread(tempOut,CV_LOAD_IMAGE_COLOR);

    lastRead=countRead;

    //if image read failed
    if(!src.data){
        //store the time and number of last image read
        lastLoaded=time;
        lastCountRead=countRead;
        //while the count is less then the maximum count between images and no image has been loaded
        while((countRead-lastCountRead)<numberImagesTest && !src.data){
            //reset time to that of last loaded
            time=lastLoaded;
            //while the time is less then the maximum time between images and no image as been loaded
            while((time-lastLoaded)<timeCheckOver && !src.data){
                //increment time
                time++;
                //create new filename
                sprintf(tempOut,"%s%s%06d-%10d-%d.png",dirOut.c_str(),readName,countRead,time,cameraNum);
                //try to load filename
                src=imread(tempOut,CV_LOAD_IMAGE_COLOR);
                lastRead=countRead;
            }
            countRead++;
        }
    }
    //qDebug("rows=%d cols=%d",(int)src.rows,(int)src.cols);

    countRead=lastRead;
    if(!src.data){
        return false;
    } else {
        return true;
    }
}

QImage paolMat::convertToQImage(){
    //copy src Mat to display Mat and convert from BGR to RGB format
    cvtColor(src,display,CV_BGR2RGB);

    //convert Mat to QImage
    QImage img=QImage((const unsigned char*)(display.data),display.cols,display.rows,display.step,QImage::Format_RGB888);

    return img;
}

QImage paolMat::convertMaskToQImage(){
    //copy mask Mat to display Mat and convert from BGR to RGB format
    cvtColor(mask,display,CV_BGR2RGB);

    //convert Mat to QImage
    QImage img=QImage((const unsigned char*)(display.data),display.cols,display.rows,display.step,QImage::Format_RGB888);

    return img;
}

QImage paolMat::convertMaskMinToQImage(){
    //copy maskMin Mat to displayMin Mat and convert from BGR to RGB format
    cvtColor(maskMin,displayMin,CV_BGR2RGB);

    //convert Mat to QImage
    QImage img=QImage((const unsigned char*)(displayMin.data),displayMin.cols,displayMin.rows,displayMin.step,QImage::Format_RGB888);

    return img;
}

QImage paolMat::convertMatToQImage(const Mat& mask) {
    Mat display;
    //copy mask Mat to display Mat and convert from BGR to RGB format
    cvtColor(mask,display,CV_BGR2RGB);

    //convert Mat to QImage
    QImage img=QImage((const unsigned char*)(display.data),display.cols,display.rows,display.step,QImage::Format_RGB888)
            .copy();
    return img;
}

void paolMat::displayMat(const Mat& mat, QLabel &location){
    //call method to convert Mat to QImage
    QImage img=convertMatToQImage(mat);
    //push image to display location "location"
    location.setPixmap(QPixmap::fromImage(img));
}

void paolMat::displayImage(QLabel &location){
    //call method to convert Mat to QImage
    QImage img=convertToQImage();

    // Frame counter to save processed frames
    static int frameCount = 0;
    char *name = new char[31];
    sprintf(name, "/home/paol/shared/out/%03dbs.png", frameCount);
    frameCount++;
    img.save(name);

    //push image to display location "location"
    location.setPixmap(QPixmap::fromImage(img));
}

void paolMat::displayMask(QLabel &location){
    //call method to convert Mat to QImage
    QImage img=convertMaskToQImage();
    //push image to display location "location"
    location.setPixmap(QPixmap::fromImage(img));
}

void paolMat::displayMaskMin(QLabel &location){
    //call method to convert Mat to QImage
    QImage img=convertMaskMinToQImage();
    //push image to display location "location"
    location.setPixmap(QPixmap::fromImage(img));
}

float paolMat::differenceMin(paolMat *img, int thresh, int size){
    int offset;
    bool diff;
    int numDiff;
    int surroundThresh = 50;

    //maskMin is set to a blank state (1/(scale*scale)) the size of src
    if (!maskMin.data)
        maskMin=Mat::zeros(src.rows/scale,src.cols/scale,src.type());

    if (scale>=size+1)
        offset=scale;
    else
        offset=scale*2;

  numDiff = 0;


  for (int y = offset, yy=offset/scale; y < (src.rows-offset); y+=scale,yy++)
  {
      //for every column
      for (int x = offset, xx=offset/scale; x < (src.cols-offset); x+=scale,xx++)
      {
          diff = false;
          //for each color channel
          for(int i = 0; i < 3; i++)
          {
              //if the difference (for this pixel) between the the current img and the previous img is greater than the threshold, difference is noted; diff = true
              if(abs((double)img->src.at<Vec3b>(y,x)[i]-(double)src.at<Vec3b>(y,x)[i]) > thresh)
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
                          if(abs(((double)(img->src.at<Vec3b>(yy,xx)[ii]))-(((double)(img->src.at<Vec3b>((yy+1),xx)[ii])))>surroundThresh))
                              diff = false;
                          if(abs(((double)(img->src.at<Vec3b>(yy,xx)[ii]))-(((double)(img->src.at<Vec3b>(yy,(xx+1))[ii])))>surroundThresh))
                              diff = false;
                      }
                  }
              }
          }
          if(diff)
          {
              numDiff++;
              maskMin.at<Vec3b>(yy,xx)[2]=255;
          }
      }
  }
  //return the percent of maskMin that are differences
  return (float)numDiff/(float)(maskMin.rows*maskMin.cols);
}

// Process differences between shrunken versions of the two given frames
void paolMat::differenceMin2(Mat& diffLocations, float& numDiff, const Mat& oldImg, const Mat& newImg, int thresh, int size) {
    int offset;
    bool diff;
    int surroundThresh = 50;

    //maskMin is set to a blank state (1/(scale*scale)) the size of src
    diffLocations=Mat::zeros(oldImg.rows/scale,oldImg.cols/scale,oldImg.type());

    if (scale>=size+1)
        offset=scale;
    else
        offset=scale*2;

  numDiff = 0;

  for (int y = offset, yy=offset/scale; y < (oldImg.rows-offset); y+=scale,yy++)
  {
      //for every column
      for (int x = offset, xx=offset/scale; x < (oldImg.cols-offset); x+=scale,xx++)
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

float paolMat::shrinkMaskMin()
{
    int total;
    int numDiff=0;
    //for every 3x3 group of pixels in maskMin
    for(int x = 1; x < maskMin.cols-1; x++)
        for(int y = 1; y < maskMin.rows-1; y++)
        {
            //count the number of differences in the 3x3
            total=0;
            for (int xx=x-1;xx<=x+1;xx++)
                for (int yy=y-1;yy<=y+1;yy++){
                    if(maskMin.at<Vec3b>(yy,xx)[2]==255)
                        total++;
                }
            //if the number of differences is greater then 3 mark it as different and count it
            if(total>3){
                maskMin.at<Vec3b>(y,x)[1]=255;
                numDiff++;
            }
        }
    //return the percentage of the maskMin that are valid differences
    return (float)numDiff/(float)(maskMin.rows*maskMin.cols);
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

//extend differences in maskMin to edges since differences start 1 pixel in
void paolMat::extendMaskMinToEdges(){
    int x;

    //extend bottom and top edge where differences overlap
    for(x=0;x<maskMin.cols;x++){
        if(maskMin.at<Vec3b>(1,x)[1] == 255)
            maskMin.at<Vec3b>(0,x)[1]=255;
        if(maskMin.at<Vec3b>(maskMin.rows-2,x)[1] == 255)
            maskMin.at<Vec3b>(maskMin.rows-1,x)[1]=255;
    }

    //extend right and left edge wher differences overlap
    for(int y = 0; y < maskMin.rows; y++)
    {
        if(maskMin.at<Vec3b>(y,1)[1] == 255)
            maskMin.at<Vec3b>(y,0)[1] = 255;

        if(maskMin.at<Vec3b>(y,maskMin.cols-2)[1] == 255)
            maskMin.at<Vec3b>(y,maskMin.cols-1)[1] = 255;
    }
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

//sweep mask from left, right, and top to extend difference region
void paolMat::sweepDownMin(){
    bool left,right,top;
    //create a temporary Mat the size of maskMin to store results
    Mat temp=Mat::zeros(maskMin.size(),maskMin.type());

    //from left to right
    for(int x = 0; x < maskMin.cols; x++)
    {
        //from top to bottom
        top = false;
        for(int y = 0; y < maskMin.rows; y++){
            if(maskMin.at<Vec3b>(y,x)[1] == 255)
                top = true;

            if(top == true)
                temp.at<Vec3b>(y,x)[0] = 255;
        }
    }

    //from top to bottom
    for(int y = 0; y < maskMin.rows; y++){

        //sweep from the left
        left = false;
        for(int x = 0; x < maskMin.cols; x++)
        {
            if(maskMin.at<Vec3b>(y,x)[1] == 255)
                left = true;

            if(left == true)
                temp.at<Vec3b>(y,x)[1] = 255;
        }

        //sweep from the right
        right = false;
        for(int x = maskMin.cols-1; x >-1; x--)
        {
            if(maskMin.at<Vec3b>(y,x)[1] == 255)
                right = true;

            if(right == true)
                temp.at<Vec3b>(y,x)[2] = 255;
        }
    }

    if(maskMin.data)
        maskMin.~Mat();
    maskMin = temp.clone();
    temp.~Mat();
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

void paolMat::keepWhiteMaskMin(){
    int x;

    //keep only the white pixels
    for(x = 0; x < maskMin.cols; x++)
    {
        for(int y = 0; y < maskMin.rows; y++)
        {
            if(!(maskMin.at<Vec3b>(y,x)[0] == 255 &&
                 maskMin.at<Vec3b>(y,x)[1] == 255 &&
                 maskMin.at<Vec3b>(y,x)[2] == 255)){
                for (int i=0;i<3;i++)
                    maskMin.at<Vec3b>(y,x)[i]=0;
            }
        }
    }
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

//grow any turned on pixels by size
void paolMat::growMin(int size)
{
    int startx,endx,starty,endy;

    //for every pixel in the image
    for(int y = 0; y < maskMin.rows; y++)
        for(int x = 0; x < maskMin.cols; x++)

            //if the pixel is turned on
            if(maskMin.at<Vec3b>(y,x)[0] == 255){
                startx=x-size;
                if (startx<0)
                    startx=0;

                starty=y-size;
                if (starty<0)
                    starty=0;

                endx=x+size;
                if (endx>=maskMin.cols)
                    endx=maskMin.cols-1;

                endy=y+size;
                if (endy>=maskMin.rows)
                    endy=maskMin.rows-1;

                //grow the region around that pixel
                for(int yy = starty; yy <= endy;yy++)
                    for(int xx = startx; xx <= endx; xx++)
                        maskMin.at<Vec3b>(yy,xx)[1] = 255;
            }
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

void paolMat::findContoursMaskMin(){
    //method combines findCountoursMask and contoursToMask methods

    Mat src_gray;
    int thresh = 100;
    //int max_thresh = 255;
    RNG rng(12345);

    /// Convert image to gray and blur it
    cvtColor( maskMin, src_gray, CV_BGR2GRAY );
    cv::blur( src_gray, src_gray, cv::Size(3,3) );
    // createTrackbar( " Canny thresh:", "Source", &thresh, max_thresh, thresh_callback );

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
    for(int x = 0; x < maskMin.cols; x++)
      for(int y = 0; y < maskMin.rows; y++)
        {
          count=0;
          for(int c=0;c<3;c++)
              count+=drawing.at<Vec3b>(y,x)[c];
          if(count>0)
              maskMin.at<Vec3b>(y,x)[1]=255;
      }
    drawing.~Mat();
    src_gray.~Mat();
    canny_output.~Mat();
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

//method to expand maskMin up to the mask
void paolMat::maskMinToMaskBinary(){
    bool center,right,down,corner;
    bool rightIn,downIn;//,cornerIn;

    if(mask.data)
        mask.~Mat();
    mask=Mat::zeros(src.size(),src.type());

    //for every color channel
    for (int c=0;c<3;c++){
        //go through the mask scalexscale box
        for (int y = 0; y < mask.rows; y+=scale)
        {
            for (int x = 0; x < mask.cols; x+=scale)
            {
                //set the location on if the corresponding location in maskMin is on
                if (maskMin.at<Vec3b>(y/scale,x/scale)[c]!=0)
                    center=true;
                else
                    center=false;

                //repeat for other corners of box
                if((x+scale)/scale<maskMin.cols){
                    rightIn=true;
                    if(maskMin.at<Vec3b>(y/scale,(x+scale)/scale)[c]!=0)
                        right=true;
                    else
                        right=false;
                } else
                    rightIn=false;

                if((y+scale)/scale<maskMin.rows){
                    downIn=true;
                    if(maskMin.at<Vec3b>((y+scale)/scale,x/scale)[c]!=0)
                        down=true;
                    else
                        down=false;
                } else
                    downIn=false;

                if(downIn && rightIn){
                    //cornerIn=true;
                    if(maskMin.at<Vec3b>((y+scale)/scale,(x+scale)/scale)[c]!=0)
                        corner=true;
                    else
                        corner=false;
                } //else
                    //cornerIn=false;

                //fill in mask based on which corners are turned on based on maskMin
                if(center)
                    mask.at<Vec3b>(y,x)[c]=255;
                if(center || right)
                    for(int xx=x+1; xx<mask.cols && xx<x+scale;xx++)
                        mask.at<Vec3b>(y,xx)[c]=255;
                if(center || down)
                    for(int yy=y+1; yy<mask.rows && yy<y+scale; yy++)
                        mask.at<Vec3b>(yy,x)[c]=255;
                if(center || right || down || corner)
                    for(int xx=x+1; xx<mask.cols && xx<x+scale;xx++)
                        for(int yy=y+1; yy<mask.rows && yy<y+scale; yy++)
                            mask.at<Vec3b>(yy,xx)[c]=255;

            }
        }
    }
}

Mat paolMat::maskMinToMaskBinary2(const Mat& orig) {
    Mat ret = Mat::zeros(Size(orig.cols*scale, orig.rows*scale), orig.type());
    bool center,right,down,corner;
    bool rightIn,downIn;//,cornerIn;

    //for every color channel
    for (int c=0;c<3;c++){
        //go through the mask scalexscale box
        for (int y = 0; y < ret.rows; y+=scale)
        {
            for (int x = 0; x < ret.cols; x+=scale)
            {
                //set the location on if the corresponding location in maskMin is on
                if (orig.at<Vec3b>(y/scale,x/scale)[c]!=0)
                    center=true;
                else
                    center=false;

                //repeat for other corners of box
                if((x+scale)/scale<orig.cols){
                    rightIn=true;
                    if(orig.at<Vec3b>(y/scale,(x+scale)/scale)[c]!=0)
                        right=true;
                    else
                        right=false;
                } else
                    rightIn=false;

                if((y+scale)/scale<orig.rows){
                    downIn=true;
                    if(orig.at<Vec3b>((y+scale)/scale,x/scale)[c]!=0)
                        down=true;
                    else
                        down=false;
                } else
                    downIn=false;

                if(downIn && rightIn){
                    //cornerIn=true;
                    if(orig.at<Vec3b>((y+scale)/scale,(x+scale)/scale)[c]!=0)
                        corner=true;
                    else
                        corner=false;
                } //else
                    //cornerIn=false;

                //fill in mask based on which corners are turned on based on maskMin
                if(center)
                    ret.at<Vec3b>(y,x)[c]=255;
                if(center || right)
                    for(int xx=x+1; xx<ret.cols && xx<x+scale;xx++)
                        ret.at<Vec3b>(y,xx)[c]=255;
                if(center || down)
                    for(int yy=y+1; yy<ret.rows && yy<y+scale; yy++)
                        ret.at<Vec3b>(yy,x)[c]=255;
                if(center || right || down || corner)
                    for(int xx=x+1; xx<ret.cols && xx<x+scale;xx++)
                        for(int yy=y+1; yy<ret.rows && yy<y+scale; yy++)
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

//blur goes through the entire image and makes each pixel the average color of the pixels in a box of radious size around it
void paolMat::blur(int size)
{
    int tempR, tempB, tempG, area;
    //blur size is pixels adjacent i.e. 1 would be a 3x3 square centered on each pixel
    area = (size *2+1)*(size *2+1);

    paolMat *temp;
    temp = new paolMat(this);
    temp->src = Scalar(0,0,0);

    //for all possible locations in the image
    for(int y = size; y < mask.rows - size; y++)
        for(int x = size; x < mask.cols -size; x++)
        {
            //average the pixel color of the surrounding box
            tempB = 0;
            tempG = 0;
            tempR = 0;
            for(int yy = y-size; yy <= y+size; yy++)
                for(int xx = x-size; xx <= x+size; xx++)
                {
                    tempR+=src.at<Vec3b>(yy,xx)[2];
                    tempG+=src.at<Vec3b>(yy,xx)[1];
                    tempB+=src.at<Vec3b>(yy,xx)[0];
                }
            tempR /=area;
            tempG /=area;
            tempB /=area;
            //set the pixel color
            temp->src.at<Vec3b>(y,x)[2] = tempR;
            temp->src.at<Vec3b>(y,x)[1] = tempG;
            temp->src.at<Vec3b>(y,x)[0] = tempB;

        }
    if(src.data)
        src.~Mat();
    src = temp->src.clone();
    temp->~paolMat();
}

Mat paolMat::blur2(const Mat& orig, int size) {
    Mat ret;
    cv::blur(orig, ret, Size(2*size+1, 2*size+1));
    return ret;
}

//this method seeks to identify where possible content is in the src image
void paolMat::pDrift()
{
    int temp,total;
    //clean mask
    if (mask.data){
        mask.~Mat();
    }
    mask=Mat::zeros(src.size(),src.type());

    //for every pixel in image (excludeing edges where perocess would break
    for(int y = 0; y < src.rows -1; y++)
        for(int x = 0; x < src.cols -1; x++)
        {
            //look for edges in the vertical direction using a variation on a Sobel filter
            //[1 -1]
            temp = (
                        //y,x+1
                        abs(src.at<Vec3b>(y,x)[0] - src.at<Vec3b>(y,x+1)[0])+
                    abs(src.at<Vec3b>(y,x)[1] - src.at<Vec3b>(y,x+1)[1])+
                    abs(src.at<Vec3b>(y,x)[2] - src.at<Vec3b>(y,x+1)[2])
                    );

            if(temp > 255)
                temp = 255;
            //write the vertical edge information to the red color channel
            mask.at<Vec3b>(y,x)[2] = temp;
            total = temp;

            //run the same filters in the vertical direction to look for edges in the
            //horizontal direction.
            temp = (
                        //y+1,x
                        abs(src.at<Vec3b>(y,x)[0] - src.at<Vec3b>(y+1,x)[0])+
                    abs(src.at<Vec3b>(y,x)[1] - src.at<Vec3b>(y+1,x)[1])+
                    abs(src.at<Vec3b>(y,x)[2] - src.at<Vec3b>(y+1,x)[2])
                    );
            if(temp > 255)
                temp = 255;
            total+=temp;
            if(total > 255)
                total = 255;

            //write the horizontal edge information to the green color channel
            mask.at<Vec3b>(y,x)[1] = temp;
            //write the addition of the horizontal and vertical edges found to the blue color channel
            mask.at<Vec3b>(y,x)[0] = total;
        }
}

//grow the areas highlighted in the 0 color channel of the mask
// //translation-if the difference in blue is enough then turn on the surrounding red pixel
void paolMat::grow(int blueThresh, int size)
{
    //for every pixel
    for(int y = size; y < src.rows - size; y++)
        for(int x = size ; x < src.cols - size; x++)
            //if the value in the mask is greater then a theshold
            if(mask.at<Vec3b>(y,x)[0] > blueThresh){
                //brighten edge
                mask.at<Vec3b>(y,x)[0]=255;
                //turn all the pixels in the square of size 2*size+1 around it on in the 2 color channel
                for(int yy = y-size; yy <= y+size;yy++)
                    for(int xx = x-size; xx <= x+size; xx++)
                        mask.at<Vec3b>(yy,xx)[2] = 255;
            } else {
                mask.at<Vec3b>(y,x)[0]=0;
            }
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

void paolMat::nontextToWhite()
{
    int x,y;
    for(y = 0; y < mask.rows; y++)
        for(x = 0; x < mask.cols; x++){
            if(mask.at<Vec3b>(y,x)[2] < 255){
                src.at<Vec3b>(y,x)[0] = 255;
                src.at<Vec3b>(y,x)[1] = 255;
                src.at<Vec3b>(y,x)[2] = 255;
            }
        }
    //blank the first row because it had bad information
    for(y = 0; y < mask.rows; y++){
        x=0;
        src.at<Vec3b>(y,x)[0] = 255;
        src.at<Vec3b>(y,x)[1] = 255;
        src.at<Vec3b>(y,x)[2] = 255;
    }
}

//updates pixels inthe background image where there is no foreground in maskMin
void paolMat::updateBackgroundMaskMin(paolMat *m, paolMat *foreground){
    bool center,right,down,corner;
    bool rightIn,downIn;//,cornerIn;
    int count=0;

    //go through the entire image
    for (int y = 0; y < src.rows; y+=scale)
    {
        for (int x = 0; x < src.cols; x+=scale)
        {
            //check the maskMin to see where changes are and upscale them to size of src
            if (m->maskMin.at<Vec3b>(y/scale,x/scale)[0]!=0)
                center=true;
            else
                center=false;

            //find boundaries
            if((x+scale)/scale<maskMin.cols){
                rightIn=true;
                if(m->maskMin.at<Vec3b>(y/scale,(x+scale)/scale)[0]!=0)
                    right=true;
                else
                    right=false;
            } else
                rightIn=false;

            if((y+scale)/scale<maskMin.rows){
                downIn=true;
                if(m->maskMin.at<Vec3b>((y+scale)/scale,x/scale)[0]!=0)
                    down=true;
                else
                    down=false;
            } else
                downIn=false;

            if(downIn && rightIn){
                //cornerIn=true;
                if(m->maskMin.at<Vec3b>((y+scale)/scale,(x+scale)/scale)[0]!=0)
                    corner=true;
                else
                    corner=false;
            } //else
            //cornerIn=false;

            //if there is nothing in the mask (no change hense no foreground) update the pixel
            for(int c=0;c<3;c++){
                if(!center){
                    src.at<Vec3b>(y,x)[c]=foreground->src.at<Vec3b>(y,x)[c];
                    count++;
                }
                if(!center && !right){
                    for(int xx=x+1; xx<mask.cols && xx<x+scale;xx++){
                        src.at<Vec3b>(y,xx)[c]=foreground->src.at<Vec3b>(y,xx)[c];
                        count++;
                    }
                }
                if(!center && !down){
                    for(int yy=y+1; yy<mask.rows && yy<y+scale; yy++){
                        src.at<Vec3b>(yy,x)[c]=foreground->src.at<Vec3b>(yy,x)[c];
                        count++;
                    }
                }

                if(!center && !right && !down && !corner)
                    for(int xx=x+1; xx<mask.cols && xx<x+scale;xx++)
                        for(int yy=y+1; yy<mask.rows && yy<y+scale; yy++){
                            src.at<Vec3b>(yy,xx)[c]=foreground->src.at<Vec3b>(yy,xx)[c];
                            count++;
                        }
            }
        }
    }
    //qDebug("count=%d",count);
}

//this method updates the whiteboard image, removing the professor
//the foreground image contains the foreground information in src and the
//  information on movement in its mask
//the edgeInfo image contains the current mask data related to edges
void paolMat::updateBack2(paolMat *foreground,paolMat *edgeInfo){
    //if no mask exists (the first time) create a blank mask
    if(!mask.data)
        mask=Mat::zeros(src.size(),src.type());

    //for every pixel in the image
    for (int y = 0; y < src.rows; y++)
    {
        for (int x = 0; x < src.cols; x++)
        {
            //if there was no movement at that pixel
            if (foreground->mask.at<Vec3b>(y,x)[0]==0){
                //update what the whiteboard and edge information look like at that pixel
                for (int c=0;c<3;c++){
                    src.at<Vec3b>(y,x)[c]=foreground->src.at<Vec3b>(y,x)[c];
                    mask.at<Vec3b>(y,x)[c]=edgeInfo->mask.at<Vec3b>(y,x)[c];
                }
            }
        }
    }
}

// Given the current frame, previous frame, and foreground (lecturer) location,
// return the current frame with the foreground pixels filled in with the previous
Mat paolMat::updateBack3(const Mat& oldWboardModel, const Mat& newInfo, const Mat& mvmtInfo) {
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

//fix to work on a separate image so that it doesn't kill background
void paolMat::processText(paolMat *m){
    int start,end;
    int dif;
    int r,g,b;
    int rAve,gAve,bAve;
    int count;
    int rOut,gOut,bOut;

    for(int y = 0; y < src.rows; y++)
        for(int x = 0; x < src.cols; x++)
            if( m->src.at<Vec3b>(y,x)[0] !=255 ||
                    m->src.at<Vec3b>(y,x)[1] !=255 ||
                    m->src.at<Vec3b>(y,x)[2] !=255 ){
                start = x;//set start of section at first non-white
                r=0;
                g=0;
                b=0;
                count=0;

                //for all pixels until next white pixel
                for(;x < src.cols && ( m->src.at<Vec3b>(y,x)[0] !=255 ||
                                       m->src.at<Vec3b>(y,x)[1] !=255 ||
                                       m->src.at<Vec3b>(y,x)[2] !=255 ); x++){
                    end = x;//reset end of section
                    r += m->src.at<Vec3b>(y,x)[2];
                    g += m->src.at<Vec3b>(y,x)[1];
                    b += m->src.at<Vec3b>(y,x)[0];
                    count++;
                }

                rAve=r/count;
                gAve=g/count;
                bAve=b/count;

                for(int xx = start; xx <= end; xx++){
                    dif=abs(m->src.at<Vec3b>(y,xx)[2]-rAve);
                    dif+=abs(m->src.at<Vec3b>(y,xx)[1]-gAve);
                    dif+=abs(m->src.at<Vec3b>(y,xx)[0]-bAve);

                    if(dif<75){
                        rOut = 255;
                        gOut = 255;
                        bOut = 255;
                    }else{
                        rOut = rAve + (m->src.at<Vec3b>(y,xx)[2]-rAve)*2;
                        gOut = gAve + (m->src.at<Vec3b>(y,xx)[1]-gAve)*2;
                        bOut = bAve + (m->src.at<Vec3b>(y,xx)[0]-bAve)*2;

                        if(rOut > 255)
                            rOut = 255;
                        else if(rOut < 0)
                            rOut = 0;

                        if(gOut > 255)
                            gOut = 255;
                        else if(gOut < 0)
                            gOut = 0;

                        if(bOut > 255)
                            bOut = 255;
                        else if(bOut < 0)
                            bOut = 0;
                    }
                    src.at<Vec3b>(y,xx)[0] = bOut;
                    src.at<Vec3b>(y,xx)[1] = gOut;
                    src.at<Vec3b>(y,xx)[2] = rOut;
                }
            } else {
                src.at<Vec3b>(y,x)[0] = 255;
                src.at<Vec3b>(y,x)[1] = 255;
                src.at<Vec3b>(y,x)[2] = 255;
            }
}

//method for darkening text and setting whiteboard to white
void paolMat::darkenText(){
    //int temp;
    Mat tempOut;

    //for every pixel
    for(int y = 0; y < src.rows; y++)
        for(int x = 0; x < src.cols; x++){
            //write edge information from blue into green channel and zero out red
            mask.at<Vec3b>(y,x)[1]=mask.at<Vec3b>(y,x)[0];
            if (mask.at<Vec3b>(y,x)[1]>15)
                mask.at<Vec3b>(y,x)[1]=255;
            mask.at<Vec3b>(y,x)[2]=0;
        }

    //run a morphological closure (grow then shrink)
    //this will fill in spaces in text caused by only looking at edges
    int dilation_type = MORPH_RECT;
    int dilation_size = 1;
    Mat element = getStructuringElement( dilation_type,
                                           Size( 2*dilation_size + 1, 2*dilation_size+1 ),
                                           Point( dilation_size, dilation_size ) );

    dilate(mask, tempOut, element);
    erode(tempOut, tempOut, element);

    //for every pixel
    for(int y = 0; y < src.rows; y++)
        for(int x = 0; x < src.cols; x++){
            //code to make it look pretty on the mask
            if (mask.at<Vec3b>(y,x)[1]!=255 && tempOut.at<Vec3b>(y,x)[1]>50){
                mask.at<Vec3b>(y,x)[2]=255;
            }

            //if there isn't and edge (text) in that location turn the pixel white
            if (tempOut.at<Vec3b>(y,x)[1]<50){
                src.at<Vec3b>(y,x)[0]=255;
                src.at<Vec3b>(y,x)[1]=255;
                src.at<Vec3b>(y,x)[2]=255;
            }
        }
}

//method to determine group truth of what it whiteboard in the image
//averages the brightest 25% of pixels in each square of size size
void paolMat::averageWhiteboard(int size){
    int x,y,xx,yy;
    int count,color,thresh;
    vector <int> pix;
    vector <int> ave;

    //clear the mask and create a new one
    if(mask.data)
        mask.~Mat();
    mask=Mat::zeros(src.size(),src.type());

    //go through the image by squares of radius size
    for (x=0;x<src.cols;x+=size)
        for (y=0;y<src.rows;y+=size){
            pix.clear();
            ave.clear();

            //within each square create a vector pix that hold all brightness values
            //for the pixels
            for(xx=x; xx<x+size && xx<src.cols; xx++)
                for (yy=y; yy<y+size && yy<src.rows; yy++){
                    color=0;
                    for (int c=0;c<3;c++)
                        color+=src.at<Vec3b>(yy,xx)[c];
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
            for(xx=x; xx<x+size && xx<src.cols; xx++)
                for (yy=y; yy<y+size && yy<src.rows; yy++){
                    color=0;
                    for (int c=0;c<3;c++)
                        color+=src.at<Vec3b>(yy,xx)[c];
                    color/=3;

                    if(color>=thresh){
                        count++;
                        for (int c=0;c<3;c++)
                            ave[c]+=src.at<Vec3b>(yy,xx)[c];
                    }
                }
            //figure out the average brightness of each channel for the brightest pixels
            for (int c=0;c<3;c++)
                ave[c]/=count;

            //set the pixels in the mask to the average brightness of the image, square by square
            for(xx=x; xx<x+size && xx<src.cols; xx++)
                for (yy=y; yy<y+size && yy<src.rows; yy++){
                    for (int c=0;c<3;c++)
                        mask.at<Vec3b>(yy,xx)[c]=ave[c];
                }
        }
}

//Use the average image to turn the whiteboard of the image white and to darken the text
void paolMat::enhanceText(){
    int dif;

    //for every pixel in the image and for every color channel
    for(int x = 0; x < src.cols; x++)
        for(int y = 0; y < src.rows; y++){
            for(int c=0;c<3;c++){

                //if the pixel is not 0 (just put in so that we don't divide by 0)
                if (mask.at<Vec3b>(y,x)[c]>0){
                    //take the brightness of the pixel and divide it by what white is in
                    //that location (average from mask)
                    dif=255*src.at<Vec3b>(y,x)[c]/mask.at<Vec3b>(y,x)[c];
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
                src.at<Vec3b>(y,x)[c]=dif;
            }
        }
}

// Get the edges produced by the difference of Gaussians (rad1 should be larger than rad2)
void paolMat::dogEdges(int kerSize, int rad1, int rad2) {
    Mat g1, g2;
    GaussianBlur(src, g1, Size(kerSize, kerSize), rad1);
    GaussianBlur(src, g2, Size(kerSize, kerSize), rad2);
    mask = g1 - g2;
}

// Adjusts the image's color levels with the given black/white thresholds and gamma value
void paolMat::adjustLevels(int lo, int hi, double gamma) {
    mask = 255/pow(hi-lo, 1/gamma)*(mask-lo)^(1/gamma);
}

// Turn mask black and white based on threshold
void paolMat::binarizeMask(int threshold) {
    for(int i = 0; i < mask.rows; i++) {
        for(int j = 0; j < mask.cols; j++) {
            if(mask.at<Vec3b>(i,j)[0] > threshold &&
                    mask.at<Vec3b>(i,j)[1] > threshold &&
                    mask.at<Vec3b>(i,j)[2] > threshold) {
                mask.at<Vec3b>(i,j)[0] = 255;
                mask.at<Vec3b>(i,j)[1] = 255;
                mask.at<Vec3b>(i,j)[2] = 255;
            } else {
                mask.at<Vec3b>(i,j)[0] = 0;
                mask.at<Vec3b>(i,j)[1] = 0;
                mask.at<Vec3b>(i,j)[2] = 0;
            }
        }
    }
}

// Turn src black and white based on threshold
void paolMat::binarizeSrc(int threshold) {
    for(int i = 0; i < src.rows; i++) {
        for(int j = 0; j < src.cols; j++) {
            if(src.at<Vec3b>(i,j)[0] > threshold &&
                    src.at<Vec3b>(i,j)[1] > threshold &&
                    src.at<Vec3b>(i,j)[2] > threshold) {
                src.at<Vec3b>(i,j)[0] = 255;
                src.at<Vec3b>(i,j)[1] = 255;
                src.at<Vec3b>(i,j)[2] = 255;
            } else {
                src.at<Vec3b>(i,j)[0] = 0;
                src.at<Vec3b>(i,j)[1] = 0;
                src.at<Vec3b>(i,j)[2] = 0;
            }
        }
    }
}

// Converts the image to its negative
void paolMat::invert() {
    for(int i = 0; i < src.rows; i++) {
        for(int j = 0; j < src.cols; j++) {
            src.at<Vec3b>(i, j)[0] = 255 - src.at<Vec3b>(i, j)[0];
            src.at<Vec3b>(i, j)[1] = 255 - src.at<Vec3b>(i, j)[1];
            src.at<Vec3b>(i, j)[2] = 255 - src.at<Vec3b>(i, j)[2];
        }
    }
}

// Populates a 2D array with the connected components
int** paolMat::getConnectedComponents() {
    int** a = new int*[src.rows];
    for(int i = 0; i < src.rows; i++) {
        a[i] = new int[src.cols];
    }
    // Initialize a by filling it with -1's
    for(int i = 0; i < mask.rows; i++) {
        for(int j = 0; j < mask.cols; j++) {
            a[i][j] = -1;
        }
    }
    // The disjoint set structure that keeps track of component classes
    UF compClasses;
    // Counter for the regions in the image
    int regCounter = 1;
    for(int i = 0; i < mask.rows; i++) {
        for(int j = 0; j < mask.cols; j++) {
            // Set component class if mask is white at current pixel
            if(mask.at<Vec3b>(i, j)[0] == 255) {
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
        }
    }
    for(int i=0; i < mask.rows; i++) {
        for(int j=0; j < mask.cols; j++) {
            a[i][j] = compClasses.find(a[i][j]);
            if(a[i][j] != -1) {
                mask.at<Vec3b>(i,j)[0] = a[i][j]/255;
                mask.at<Vec3b>(i,j)[1] = a[i][j]%255;
                mask.at<Vec3b>(i,j)[3] = 0;
            }
        }
    }

    // TEMP: Write to test file; breaks regular processing for some reason
//    vector<int> compression_params;
//    compression_params.push_back(CV_IMWRITE_PNG_COMPRESSION);
//    compression_params.push_back(9);
//    try {
//        imwrite("/home/paol/test.png", mask, compression_params);
//    }
//    catch (std::runtime_error& ex) {
//        qDebug("Exception converting image to PNG format: %s\n", ex.what());
//    }
    return a;
}

// Given an array of the connected components, adds components that have pixels
// in the edge detector (ie. pixels that have value 255 in the blue channel in
// the mask)
void paolMat::addComponentsFromMask(int **components) {
    std::set<int> componentsToKeep;
    // Go through mask and keep track of components that intersect with the edge detector
    for(int i=0; i < mask.rows; i++) {
        for(int j=0; j < mask.cols; j++) {
            if(mask.at<Vec3b>(i,j)[2] == 255 && components[i][j] > 0) {
                componentsToKeep.insert(components[i][j]);
            }
        }
    }
    // Turn off components that did not intersect with the edge detector
    for(int i=0; i < mask.rows; i++) {
        for(int j=0; j < mask.cols; j++) {
            if(componentsToKeep.find(components[i][j]) != componentsToKeep.end()) {
                // Component should be kept
                mask.at<Vec3b>(i,j)[0] = 255;
                mask.at<Vec3b>(i,j)[1] = 255;
                mask.at<Vec3b>(i,j)[2] = 255;
            }
            else {
                // Component should be discarded
                mask.at<Vec3b>(i,j)[0] = 0;
                mask.at<Vec3b>(i,j)[1] = 0;
                mask.at<Vec3b>(i,j)[2] = 0;
            }
        }
    }
}

// Blur the source by a Gaussian with the given radius
void paolMat::blurSrc(int blurRad) {
    GaussianBlur(src, src, Size(blurRad,blurRad), 0);
}

// Run the Laplace edge detector on the source
void paolMat::laplaceEdges() {
    Laplacian(src, src, -1);
}

// Whitens the whiteboard, given the location of the marker as an argument
void paolMat::darkenText2(Mat marker){
    //for every pixel
    for(int y = 0; y < src.rows; y++)
        for(int x = 0; x < src.cols; x++){
            //if there isn't and edge (text) in that location turn the pixel white
            if (marker.at<Vec3b>(y,x)[1]<50){
                src.at<Vec3b>(y,x)[0]=255;
                src.at<Vec3b>(y,x)[1]=255;
                src.at<Vec3b>(y,x)[2]=255;
            }
        }
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
    // Initialize a by filling it with -1's
//    for(int i = 0; i < orig.rows; i++) {
//        for(int j = 0; j < orig.cols; j++) {
//            a[i][j] = -1;
//        }
//    }
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

//gives the percentage of differences in text in the image
// it does this by counting the number of times text (0-blue color channel)
// is the not the same between images and where it appears in one image is not in an area surrounding text
// in the other
float paolMat::countDifsMask(paolMat *newIm){
    int difs=0;
    int sizeBuffer=40;//area around edge of whiteboard that is ignored when looking for difference
    //there is often a lot of noise at the edges

    for(int x = sizeBuffer; x < src.cols-sizeBuffer; x++)
        for(int y = sizeBuffer; y < src.rows-sizeBuffer; y++){
            if((mask.at<Vec3b>(y,x)[0]!=0 and newIm->mask.at<Vec3b>(y,x)[2]!=255) ||
                (newIm->mask.at<Vec3b>(y,x)[0]!=0 and mask.at<Vec3b>(y,x)[2]!=255)){
                difs++;
                mask.at<Vec3b>(y,x)[0]=255;
                mask.at<Vec3b>(y,x)[1]=255;
                mask.at<Vec3b>(y,x)[2]=255;
            } else {
                mask.at<Vec3b>(y,x)[0]=0;
                mask.at<Vec3b>(y,x)[1]=0;
                mask.at<Vec3b>(y,x)[2]=0;
            }
            //src.at<Vec3b>(y,x)[2]=255;
        }

    return (double)difs/((double)(mask.cols*mask.rows));
}

void paolMat::rectifyImage(paolMat *m){
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
    /*int rows,cols;
    if (RTx-LTx>RBx-LBx)
        cols=RTx-LTx;
    else
        cols=RBx-LBx;
    if (RBy-RTy>LBy-LTy)
        rows=RBy-RTy;
    else
        rows=LBy-LTy;
*/
    if(src.data)
        src.~Mat();
    //src=Mat::zeros(rows,cols,m->src.type());
    src=Mat::zeros(m->src.size(),m->src.type());

    for(int x = 0; x < src.cols; x++)
        for(int y = 0; y < src.rows; y++){
            widthP=(double)x/(double)src.cols;
            heightP=(double)y/(double)src.rows;
            LPx=LTx+(LBx-LTx)*heightP;
            LPy=LTy+(LBy-LTy)*heightP;
            RPx=RTx+(RBx-RTx)*heightP;
            RPy=RTy+(RBy-RTy)*heightP;

            xInput=(int)(LPx+(RPx-LPx)*widthP);
            yInput=(int)(LPy+(RPy-LPy)*widthP);

            if (xInput>=0 &&
                    xInput<m->src.cols &&
                    yInput>=0 &&
                    yInput<m->src.rows){
                src.at<Vec3b>(y,x)[0]=m->src.at<Vec3b>(yInput,xInput)[0];
                src.at<Vec3b>(y,x)[1]=m->src.at<Vec3b>(yInput,xInput)[1];
                src.at<Vec3b>(y,x)[2]=m->src.at<Vec3b>(yInput,xInput)[2];
            } else {
                src.at<Vec3b>(y,x)[0]=0;
                src.at<Vec3b>(y,x)[1]=0;
                src.at<Vec3b>(y,x)[2]=0;
            }
        }
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

void paolMat::findBoard(paolMat *m){
    Canny(m->src, mask, 50, 200, 3);
    cvtColor(mask, src, CV_GRAY2BGR);

    /*vector<Vec4i> lines;
    HoughLinesP( mask, lines, 1, CV_PI/180, 80, 30, 10 );
    for( size_t i = 0; i < lines.size(); i++ )
    {
        line( src, Point(lines[i][0], lines[i][1]),
                Point(lines[i][2], lines[i][3]), Scalar(0,0,255), 3, 8 );
    }*/


    vector<Vec2f> lines;
    // detect lines
    HoughLines(mask, lines, 1, CV_PI/180, 250, 0, 0 );
    src.~Mat();
    src=m->src.clone();
    // draw lines
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
        line( src, pt1, pt2, Scalar(0,0,255), 3, CV_AA);
    }
}
