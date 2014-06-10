#include "paolMat.h"

void paolMat::setCameraNum(int i){
    cam=VideoCapture(i);
    cam.set(CV_CAP_PROP_FRAME_WIDTH, 1920);
    cam.set(CV_CAP_PROP_FRAME_HEIGHT, 1080);

    //cam.set(CV_CAP_PROP_FRAME_WIDTH, 320);
    //cam.set(CV_CAP_PROP_FRAME_HEIGHT, 240);
}

void paolMat::openVideo(QWidget *fg){
    QString filename=QFileDialog::getOpenFileName(fg,fg->tr("Open Image"),".",
                                                  fg->tr("Video Files (*.avi *.mpg *.mp4)"));
    cam=VideoCapture(filename.toLatin1().data());
}

void paolMat::takePicture(){
    //qDebug("start");
    for (int i=0;i<5;i++){
        //qDebug("i=%d",i);
        cam>>src;
        //qDebug(" i=%d",i);
    }
    //qDebug("end");
}

bool paolMat::getFrame(){
    return cam.read(src);
}

bool paolMat::readNext(QWidget *fg){
    std::string readFileName;
    int lastLoaded, lastCountRead;
    int lastRead;
    int numberImagesTest = 300;
    int timeCheckOver = 300;

    if (cameraNum==-1){
        QString filename=QFileDialog::getOpenFileName(fg,fg->tr("Open First Image of Sequence"),".",
                                                      fg->tr("Image Files (*.png *.bmp *.jpg *.JPG)"));
        QStringList pieces = filename.split( "/" );
        QString imagename = pieces.value( pieces.length() - 1 );
        QString directory = pieces[0]+"";
        for (int i=0;i<pieces.length()-1;i++){
            directory+=pieces[i]+"/";
        }
        dirOut=directory.toUtf8().constData();
        std::string text = imagename.toUtf8().constData();
        sscanf(text.c_str(),"%[^0-9]%06d-%10d-%d.png",readName,&countRead,&time,&cameraNum);
    }
    //qDebug("readname=%s\n countRead=%d time=%d cameraNum=%d",readName,countRead,time,cameraNum);

    countRead++;
    char tempOut[256];
    sprintf(tempOut,"%s%s%06d-%10d-%d.png",dirOut.c_str(),readName,countRead,time,cameraNum);

    qDebug("readName:%s \n fullPath:%s\n",readName,tempOut);
    if(src.data)
        src.~Mat();
    src=imread(tempOut,CV_LOAD_IMAGE_COLOR);

    lastRead=countRead;


    if(!src.data){
        lastLoaded=time;
        lastCountRead=countRead;
        while((countRead-lastCountRead)<numberImagesTest && !src.data){
            time=lastLoaded;
            while((time-lastLoaded)<timeCheckOver && !src.data){
                time++;
                sprintf(tempOut,"%s%s%06d-%10d-%d.png",dirOut.c_str(),readName,countRead,time,cameraNum);
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

double paolMat::getFrameRate(){
    return cam.get(CV_CAP_PROP_FPS);
}

QImage paolMat::convertToQImage(){
    cvtColor(src,display,CV_BGR2RGB);
    if(src.data)
        qDebug("data cols=%d rows=%d",src.cols,src.rows);
    else
        qDebug("no data");
    if(display.data)
        qDebug("display");
    else
        qDebug("no display");
    QImage img=QImage((const unsigned char*)(display.data),display.cols,display.rows,display.step,QImage::Format_RGB888);
    return img;
}

QImage paolMat::convertMaskToQImage(){
    cvtColor(mask,display,CV_BGR2RGB);

    QImage img=QImage((const unsigned char*)(display.data),display.cols,display.rows,display.step,QImage::Format_RGB888);
    return img;
}

QImage paolMat::convertMaskMinToQImage(){
    qDebug("convert before");
    if (maskMin.data)
        qDebug("rows=%d cols=%d",maskMin.rows,maskMin.cols);
    else
        qDebug("no data");
    cvtColor(maskMin,displayMin,CV_BGR2RGB);
    qDebug(" convert after");
    QImage img=QImage((const unsigned char*)(displayMin.data),displayMin.cols,displayMin.rows,displayMin.step,QImage::Format_RGB888);
    qDebug(" convert after2");
    return img;
}

void paolMat::displayImage(QLabel &location){
    QImage img=convertToQImage();
    QPixmap temp=QPixmap::fromImage(img);
    location.setPixmap(temp);
    //location.resize(location.pixmap()->size());
}

void paolMat::displayMask(QLabel &location){
    QImage img=convertMaskToQImage();
    location.setPixmap(QPixmap::fromImage(img));
    //location.resize(location.pixmap()->size());
}

void paolMat::displayMaskMin(QLabel &location){
    qDebug("display mask min");
    QImage img=convertMaskMinToQImage();
    qDebug(" display mask min2");
    location.setPixmap(QPixmap::fromImage(img));
    qDebug(" display mask min3");
    //location.resize(location.pixmap()->size());
}

void paolMat::toChromaticity(){
    int RGB,temp;

    for (int y = 0; y < src.rows; y++)
    {
        for (int x = 0; x < src.cols; x++)
        {
            RGB=0;
            for (int c=0;c<3;c++){
                RGB+=src.at<Vec3b>(y,x)[c];
            }
            if (RGB==0)
                RGB=1;
            //qDebug(" here1 y=%d/%d x=%d/%d RGB=%d\n",y,src.rows,x,src.cols,RGB);
            for (int c=0;c<3;c++){
                temp=src.at<Vec3b>(y,x)[c]*255/RGB;
                if (temp>255)
                    temp=255;
                src.at<Vec3b>(y,x)[c]=temp;
            }
        }
    }
}

void paolMat::toHSV(){
    Mat temp;

    cvtColor(src,temp,CV_RGB2HSV);
    for (int y = 0; y < src.rows; y++)
    {
        for (int x = 0; x < src.cols; x++)
        {
            src.at<Vec3b>(y,x)[0]=temp.at<Vec3b>(y,x)[0]*255/179;
            src.at<Vec3b>(y,x)[1]=temp.at<Vec3b>(y,x)[0];
            src.at<Vec3b>(y,x)[2]=0;
        }
    }
}

void paolMat::maskToRed(){
    int temp;

    for (int y = 0; y < src.rows; y++)
    {
        for (int x = 0; x < src.cols; x++)
        {
            temp=0;
            for (int c=0;c<3;c++){
                temp+=mask.at<Vec3b>(y,x)[c];
            }
            if (temp>700){
                src.at<Vec3b>(y,x)[0]=0;
                src.at<Vec3b>(y,x)[1]=0;
            }
        }
    }
}

void paolMat::maskMinToMaskBinary(){
    bool center,right,down,corner;
    bool rightIn,downIn;//,cornerIn;

    for (int c=0;c<3;c++){
        for (int y = 0; y < mask.rows; y+=scale)
        {
            for (int x = 0; x < mask.cols; x+=scale)
            {
                if (maskMin.at<Vec3b>(y/scale,x/scale)[c]!=0)
                    center=true;
                else
                    center=false;
                //find boundaries
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

void paolMat::updateBack2(paolMat *m){
    int temp;

    for (int y = 0; y < src.rows; y++)
    {
        for (int x = 0; x < src.cols; x++)
        {
            temp=0;
            for (int c=0;c<3;c++){
                temp+=m->mask.at<Vec3b>(y,x)[c];
            }
            if (temp<700){
                for (int c=0;c<3;c++){
                    src.at<Vec3b>(y,x)[c]=m->src.at<Vec3b>(y,x)[c];
                }
            }
        }
    }
}

void paolMat::updateBack2Min(paolMat *m){
    bool center,right,down,corner;
    bool rightIn,downIn;//,cornerIn;

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
            if(!center)
                src.at<Vec3b>(y,x)=m->src.at<Vec3b>(y,x);
            if(!center && !right)
                for(int xx=x+1; xx<mask.cols && xx<x+scale;xx++)
                    src.at<Vec3b>(y,xx)=m->src.at<Vec3b>(y,xx);
            if(!center && !down)
                for(int yy=y+1; yy<mask.rows && yy<y+scale; yy++)
                    src.at<Vec3b>(yy,x)=m->src.at<Vec3b>(yy,x);
            if(!center && !right && !down && !corner)
                for(int xx=x+1; xx<mask.cols && xx<x+scale;xx++)
                    for(int yy=y+1; yy<mask.rows && yy<y+scale; yy++)
                        src.at<Vec3b>(yy,xx)=m->src.at<Vec3b>(yy,xx);

        }
    }
}

void paolMat::findContoursMask(){
    Mat src_gray;
    int thresh = 100;
    //int max_thresh = 255;
    RNG rng(12345);

    /// Convert image to gray and blur it
    cvtColor( mask, src_gray, CV_BGR2GRAY );
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


    cvtColor(drawing,src,CV_RGB2BGR);
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

    maskMin.~Mat();
    cvtColor(drawing,maskMin,CV_RGB2BGR);

    int count;

    for(int x = 0; x < maskMin.cols; x++)
      for(int y = 0; y < maskMin.rows; y++)
        {
          count=0;
          for(int c=0;c<3;c++)
              count+=maskMin.at<Vec3b>(y,x)[c];
          if(count>0)
              maskMin.at<Vec3b>(y,x)[2]=255;
      }
    drawing.~Mat();
    src_gray.~Mat();
    canny_output.~Mat();
}

void paolMat::loadImage(QWidget *fg){
    QString filename=QFileDialog::getOpenFileName(fg,fg->tr("Open Image"),".",
                                                  fg->tr("Image Files (*.png *.bmp *.jpg *.JPG)"));
    src=imread(filename.toLatin1().data());
}

void paolMat::saveSrc(QWidget *fg){
    QString filename = QFileDialog::getSaveFileName(fg,fg->tr("Save Image"), ".",
             fg->tr("All Files (*)"));

    imwrite(filename.toLatin1().data(),src);
}

paolMat::paolMat()
{
  name = "No Name";
  count = -1;
  time = -1;
  difs = -1;
  prof = Point(0,0);
  camera = Point(0,0);
  lectFound = false;
  r =0;
  g =0;
  b =0;
  scale=8;
  disp=new Shift();
  sList=new SegList();
  cameraNum=-1;
}

paolMat::paolMat(paolMat* m)
{

  src = m->src.clone();
  mask = m->mask.clone();
  count = m->count;
  time = m->time;
  name = m->name;
  difs = m->difs;
  camera.x = m->camera.x;
  camera.y = m->camera.y;
  prof.x = m->prof.x;
  prof.y = m->prof.y;
  lectFound = m->lectFound;
  r = m->r;
  g = m->g;
  b = m->b;
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
    camera.~Point();
    prof.~Point();
}

void paolMat::copy(paolMat *m)
{
    if (src.data)
        src.~Mat();
    src = m->src.clone();
    if(mask.data)
        mask.~Mat();
    mask = m->mask.clone();
    if(maskMin.data)
        maskMin.~Mat();
    maskMin = m->maskMin.clone();
    count = m->count;
    time = m->time;
    name = m->name;
    difs = m->difs;
    camera.x = m->camera.x;
    camera.y = m->camera.y;
    prof.x = m->prof.x;
    prof.y = m->prof.y;
    lectFound = m->lectFound;
    r = m->r;
    g = m->g;
    b = m->b;
    scale=m->scale;
}

void paolMat::copyNoSrc(paolMat *m)
{

  //mask = m->mask.clone();
  count = m->count;
  time = m->time;
  name = m->name;
  difs = m->difs;
  camera.x = m->camera.x;
  camera.y = m->camera.y;
  prof.x = m->prof.x;
  prof.y = m->prof.y;
  lectFound = m->lectFound;
  r = m->r;
  g = m->g;
  b = m->b;
}

void paolMat::copyMask(paolMat *m){
    if(mask.data)
        mask.~Mat();
    mask=m->mask.clone();
    if(maskMin.data)
        maskMin.~Mat();
    maskMin=m->maskMin.clone();
}

void paolMat::maskToSrc(){
    if(src.data)
        src.~Mat();
    src=mask.clone();
}

void paolMat::binarize(paolMat *in){
    int count;

    for (int y = 0; y < src.rows; y++)
    {
        for (int x = 0; x < src.cols; x++)
        {
            count=0;
            for (int c=0;c<3;c++){
                count+=in->src.at<Vec3b>(y,x)[c];
            }
            if (count!=255*3)
                for (int c=0;c<3;c++){
                    src.at<Vec3b>(y,x)[c]=0;
                }
            else
                for (int c=0;c<3;c++){
                    src.at<Vec3b>(y,x)[c]=255;
                }
        }
    }
}

void paolMat::connectedComponent(paolMat *im, bool binary){
    int up,left;
    int x,y;
    sList->~SegList();
    sList=new SegList;

    if(src.data)
        src.~Mat();
    src=Mat::zeros(im->src.size(),im->src.type());

    for(x=0;x<src.cols;x++)
        for(y=0;y<src.rows;y++){
            if ((binary &&
                    (im->src.at<Vec3b>(y,x)[0]!=0 ||
                     im->src.at<Vec3b>(y,x)[1]!=0 ||
                     im->src.at<Vec3b>(y,x)[2]!=0)) ||
            (!binary)){

                left=comparePix(im,x,y,x-1,y,binary,12);
                up=comparePix(im,x,y,x,y-1,binary,12);

                if (left!=0 and up!=0){
                    numToPix(x,y,sList->merge(left,up));
                } else if (left!=0){
                    numToPix(x,y,sList->find(left));
                } else if (up!=0){
                    numToPix(x,y,sList->find(up));
                } else {
                    numToPix(x,y,sList->newPoint());
                }
            } else {
                src.at<Vec3b>(y,x)[0]=0;
                src.at<Vec3b>(y,x)[1]=0;
                src.at<Vec3b>(y,x)[2]=0;
            }
        }

    //sList->print();
    sList->update();

    for(x=0;x<src.cols;x++)
        for(y=0;y<src.rows;y++){
            numToPix(x,y,sList->getValue(pixToNum(x,y)));
        }

    qDebug("countOut=%d",sList->getCount());
}

int paolMat::comparePix(paolMat *im, int x1, int y1, int x2, int y2, bool binary, int thresh){
    if (x2>=0 && y2>=0){
        if (binary){
            return sList->find(pixToNum(x2,y2));
        } else {
            if (pixSame(im,x1,y1,x2,y2,thresh)){
                return sList->find(pixToNum(x2,y2));
            }else
                return 0;
        }
    } else {
        return 0;
    }
}

bool paolMat::pixSame(paolMat *im, int x1, int y1, int x2, int y2, int thresh){
    int dif=0;

    for (int c=0;c<3;c++)
        dif+=abs(im->src.at<Vec3b>(y1,x1)[c]-im->src.at<Vec3b>(y2,x2)[c]);

    if (dif<=thresh)
        return true;
    else
        return false;
}

int paolMat::pixToNum(int x, int y){
    int out;

    out=src.at<Vec3b>(y,x)[0];
    out+=256*src.at<Vec3b>(y,x)[1];
    out+=256*256*src.at<Vec3b>(y,x)[2];

    return out;
}

void paolMat::numToPix(int x, int y, int num){
    int zero,one,two;

    zero=num%256;
    one=(num/256)%256;
    two=(num/256)/256;
    src.at<Vec3b>(y,x)[0]=zero;
    src.at<Vec3b>(y,x)[1]=one;
    src.at<Vec3b>(y,x)[2]=two;
}

void paolMat::colorConnected(paolMat *connected){
    int x,y,temp;
    int scale=256*256*256/connected->sList->getCount();

    for(x=0;x<src.cols;x++)
        for(y=0;y<src.rows;y++){
            temp=connected->pixToNum(x,y)*scale;
            numToPix(x,y,temp);
        }
}

void paolMat::copyInDif(paolMat *in){
    int count;

    for (int y = 0; y < src.rows; y++)
    {
        for (int x = 0; x < src.cols; x++)
        {
            count=0;
            for (int c=0;c<3;c++){
                count+=in->mask.at<Vec3b>(y,x)[c];
            }
            if (count>0)
                for (int c=0;c<3;c++){
                    src.at<Vec3b>(y,x)[c]=0;
                }

        }
    }
}

void paolMat::read(std::string fullName, std::string fileName,int countIn, int timeIn)
{
  name = fileName;
  src = imread(fullName);
  mask = Mat::zeros(src.size(), src.type());
  count=countIn;
  time=timeIn;
  //if(src.data)
  //std::cout<<"PaolMat:: Read: "<<fullName<<std::endl;
}

void paolMat::capture(CvCapture* capture, int countIn)
{
  char nametemp[256];

  int timeIn = count;
  sprintf(nametemp,"frame%06d-%10d.ppm",count,timeIn);
  name=nametemp;
  //src = imread(fullName);
  IplImage* frame = cvQueryFrame( capture );
  src = frame;
  mask = Mat::zeros(src.size(), src.type());
  count=countIn;
  time=timeIn;
  //if(src.data)
  //std::cout<<"PaolMat:: Read: "<<name<<std::endl;
}

void paolMat::write()
{
  if(!src.empty())
    {
      char temp[256];
      std::string longName = "outMedia/";
      longName.append(name);
      sprintf(temp,"%06d-%010d.png",count,time);
      longName.append(temp);
      cv::imwrite(longName, src);
      //std::cout<<longName<<std::endl;
    }else
    {
      //std::cout<<"   Tried to write a empty src"<<std::endl;
    }
}

void paolMat::write(std::string outDir)
{
  if(!src.empty())
    {
      char temp[256];
      std::string longName = outDir;
      longName.append(name);
      sprintf(temp,"%06d-%010d.png",count,time);
      longName.append(temp);
      cv::imwrite(longName, src);
      //std::cout<<longName<<std::endl;
    }else
    {
      //std::cout<<"   Tried to write a empty src"<<std::endl;
    }
}

void paolMat::write2(std::string outDir,std::string nameOut,int camNum)
{
  if(!src.empty())
    {
      char temp[256];
      std::string longName = outDir;
      longName.append(nameOut);
      sprintf(temp,"%010d-%d.png",time,camNum);
      longName.append(temp);
      cv::imwrite(longName, src);
      //std::cout<<longName<<std::endl;
    }else
    {
      //std::cout<<"   Tried to write a empty src"<<std::endl;
    }
}

void paolMat::writeByCount(std::string outDir)
{
  if(!src.empty())
    {
      char temp[256];
      std::string longName = outDir;
      longName.append(name);
      sprintf(temp,"_%06d.png",count);
      longName.append(temp);
      cv::imwrite(longName, src);
      //std::cout<<longName<<std::endl;
    }else
    {
      //std::cout<<"   Tried to write a empty src"<<std::endl;
    }
}


void paolMat::writeMask()
{
  if(!src.empty())
    {
      char temp[256];
      std::string longName = "outMedia/";
      longName.append("Mask-");
      longName.append(name);
      sprintf(temp,"%06d-%010d.png",count,time);
      longName.append(temp);
      cv::imwrite(longName, mask);
      //std::cout<<"paul's test: "<<longName<<std::endl;
    }else
    {
      //std::cout<<"   Tried to write a empty mask"<<std::endl;
    }
}

void paolMat::print()
{
  //std::cout<<"     "<<name<<" time "<<time<<" count "<<count<<std::endl;

}

void paolMat::edges()
{
  cv::Mat temp;
  temp = src.clone();
  src = Mat::zeros(src.size(),src.type());
  int total;

  for (int y = 1; y < src.rows-1; y++)
    {
      for (int x = 1; x < src.cols-1; x++)
    {
      total=0;
      for(int i = 0; i <3; i++)
        {
          total+= -2*temp.at<Vec3b>((y-1),(x-1))[i];
          total+= -1*temp.at<Vec3b>((y-1),x)[i];
          total+= -1*temp.at<Vec3b>(y,(x-1))[i];
          total+= 2*temp.at<Vec3b>((y+1),(x+1))[i];
          total+= 1*temp.at<Vec3b>((y+1),x)[i];
          total+= 1*temp.at<Vec3b>(y,(x+1))[i];

        }

      total=abs(total);
      if(total>56)
        src.at<Vec3b>(y,x)[2]=255;
      //if(total>512)
      //  src.at<Vec3b>(y,x)[1]=255;
      //if(total>768)
      //  src.at<Vec3b>(y,x)[0]=255;
    }
    }
  name="edges";
}

paolMat *paolMat::returnEdges()
{
  paolMat *out;
  out = new paolMat(this);
  out->edges();
  return out;

}

//This is a slow method for testing, not production//
void paolMat::invert()
{
  //int temp;

  for (int y = 0; y < src.rows; y++)
    {
      for (int x = 0; x < src.cols; x++)
    {
      src.at<Vec3b>(y,x)[0]=255-src.at<Vec3b>(y,x)[0];
      src.at<Vec3b>(y,x)[1]=255-src.at<Vec3b>(y,x)[1];
      src.at<Vec3b>(y,x)[2]=255-src.at<Vec3b>(y,x)[2];
    }
    }
}

void paolMat::createBackgroundImg(int kernalSize)
{
  cv::Point centerPoint(-1,-1);
  cv::blur(src, src, cv::Size(kernalSize,kernalSize), centerPoint, 1);
  name = "backgroundImg";
}

paolMat *paolMat::returnCreateBackgroundImg(int kernalSize)
{
  paolMat *img;
  img = new paolMat(this);
  img->createBackgroundImg(kernalSize);
  return img;
}

void paolMat::improveInputImg(paolMat *background)
{
  int temp;
  int thresh = 5;


  for (int y = 0; y < src.rows; y++)
    {
      for (int x = 0; x < src.cols; x++)
    {
      for (int channel = 0; channel <3; channel++)
        {
          temp=background->src.at<Vec3b>(y,x)[channel]-thresh;
          if(temp <= 0)
        temp = 1;
          temp  = (src.at<Vec3b>(y,x)[channel] * 255) / temp;
          if(temp > 255)
        src.at<Vec3b>(y,x)[channel] = 255;
          else
        src.at<Vec3b>(y,x)[channel] = temp;
        }
    }

    }

  name = "improvedImage";
}

paolMat *paolMat::returnImproveInputImg(paolMat *background)
{
  paolMat *img;
  img = new paolMat(this);
  img->improveInputImg(background);
  return img;
}

void paolMat::removeProf(paolMat *oldImg){
  //int totalDiff;
  name = "noProf";
  connected();
  lectArea();
  for(int x = 0; x < src.cols; x++)
    for(int y = 0; y < src.rows; y++)
      if(mask.at<Vec3b>(y,x)[2]==255)
    for(int i = 0; i < 3; i++)
      src.at<Vec3b>(y,x)[i] = oldImg->src.at<Vec3b>(y,x)[i];

}

paolMat *paolMat::returnRemoveProf(paolMat *oldImg)
{
  paolMat *img;
  img = new paolMat(this);
  img->removeProf(oldImg);
  return img;
}

void paolMat::createContrast(){
  int temp;
  int ave;
  for (int y = 0; y < src.rows; y++)
    {
      for (int x = 0; x < src.cols; x++)
    {
      ave = (src.at<Vec3b>(y,x)[0] + src.at<Vec3b>(y,x)[1] + src.at<Vec3b>(y,x)[2]) /3;
      if(ave <240 ||
         ((src.at<Vec3b>(y,x)[0] < 220) ||
          (src.at<Vec3b>(y,x)[1] < 220) ||
          (src.at<Vec3b>(y,x)[2] < 220)))
        {
          temp = src.at<Vec3b>(y,x)[0]-(255-src.at<Vec3b>(y,x)[0]);
          if (temp < 0)
        temp = 0;

          src.at<Vec3b>(y,x)[0] = temp;

          temp = src.at<Vec3b>(y,x)[1]-(255-src.at<Vec3b>(y,x)[1]);
          if (temp < 0)
        temp = 0;

          src.at<Vec3b>(y,x)[1] = temp;

          temp = src.at<Vec3b>(y,x)[2]-(255-src.at<Vec3b>(y,x)[2]);
          if (temp < 0)
        temp = 0;

          src.at<Vec3b>(y,x)[2] = temp;
        }else
        {
          src.at<Vec3b>(y,x)[0] = 255;
          src.at<Vec3b>(y,x)[2] = 255;
          src.at<Vec3b>(y,x)[1] = 255;
        }
    }
    }
  name = "ContrastImg";
}

paolMat *paolMat::returnCreateContrast()
{
  paolMat *img;
  img = new paolMat(this);
  img->createContrast();
  return img;
}

paolMat *paolMat::returnSharpen()
{

  paolMat *img;
  img = new paolMat(this);

  int v, temp;
  double kSharp;
  kSharp = 0.75;
  v =2;


  for (int y = v; y < (src.rows -v); y++)
    {
      for (int x = v; x < (src.cols -v); x++)
    {
      for (int channel = 0; channel <3; channel++)
        {
          temp = (int)(((double)src.at<Vec3b>(y,x)[channel] -
                ( (kSharp/4) * (double)(src.at<Vec3b>((y-v),x)[channel] +
                            src.at<Vec3b>(y,(x-v))[channel] +
                            src.at<Vec3b>(y,(x+v))[channel] +
                            src.at<Vec3b>((y+v),x)[channel] )))/
               (1.0-kSharp));

          if(temp > 255)
        img->src.at<Vec3b>(y,x)[channel] = 255;
          else if(temp < 0)
        img->src.at<Vec3b>(y,x)[channel] = 0;
          else
        img->src.at<Vec3b>(y,x)[channel] = temp;
        }

    }
    }
  img->name = "Sharp";
  return img;
}

void paolMat::sharpen(){
  paolMat *img;
  img = this->returnSharpen();
  copy(img);

}

paolMat *paolMat::returnShrink()
{
  paolMat *img;
  img = new paolMat(this);

  img->src = Scalar(255,255,255);

  int total;


  for (int y = 1; y < (src.rows -1); y++)
    for (int x = 1; x < (src.cols -1); x++)
      for (int channel = 0; channel <3; channel++)
    if(src.at<Vec3b>(y,x)[channel]<255)
      {

        total = src.at<Vec3b>((y-1),(x-1))[channel];
        total += src.at<Vec3b>((y-1),x)[channel];
        total += src.at<Vec3b>((y-1),(x+1))[channel];

        total += src.at<Vec3b>(y,(x-1))[channel];
        total += src.at<Vec3b>(y,(x+1))[channel];

        total += src.at<Vec3b>((y+1),(x-1))[channel];
        total += src.at<Vec3b>((y+1),x)[channel];
        total += src.at<Vec3b>((y+1),(x+1))[channel];

        if(total>=1530)
          img->src.at<Vec3b>(y,x)[channel] = 255;
      }
  img->name = "Shrink";
  return img;
}

void paolMat::shrink(){
  paolMat *img;
  img = returnShrink();
  copy(img);
}

float paolMat::difference(paolMat *img, int thresh, int size, int maskBottom)
{
  bool diff;
  int numDiff;
  int surroundThresh = 50;
  int dist;
  bool first;
  int cenx;
  int ceny;
  int total;
  //mask is set to a blank state
  mask = Mat::zeros(mask.size(), mask.type());

  numDiff = 0;
  first = true;
  //distance --
  dist = 0;
  //for every row
  for (int y = size; y < (src.rows-(size+1+maskBottom)); y++)
  {
      //for every column
      for (int x = size; x < (src.cols-(size+1)); x++)
      {
          diff = false;
          //for each color channel
          for(int i = 0; i < 3; i++)
          {
              //if the difference (for this pixel) between the the current img and the previous img is greater than the threshold, difference is noted; diff = true
              if(abs((double)img->src.at<Vec3b>(y,x)[i]-(double)src.at<Vec3b>(y,x)[i])>thresh)
                  diff = true;
          }
          if(diff)
          {
              //std::cout<<"First if dif size: "<<size<<std::endl;
              //mask.at<Vec3b>(y,x)[1]=255;
              // for all the pixels surrounding the current pixel
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
              //std::cout<<"Second if diff"<<std::endl;
              numDiff++;
              //calculates total difference and modifies the mask accordingly
              total = abs((double)img->src.at<Vec3b>(y,x)[0]-(double)src.at<Vec3b>(y,x)[0]) +
                      abs((double)img->src.at<Vec3b>(y,x)[1]-(double)src.at<Vec3b>(y,x)[1]) +
                      abs((double)img->src.at<Vec3b>(y,x)[2]-(double)src.at<Vec3b>(y,x)[2]);
              if(total > 512)
              {
                  mask.at<Vec3b>(y,x)[0] = 255;
              }
              if(total > 255)
              {
                  mask.at<Vec3b>(y,x)[1] = 255;
                  numDiff++;
              }
              mask.at<Vec3b>(y,x)[2]=255;
              //sets location of first differnce found
              if(first)
              {
                  first = false;
                  cenx = x;
                  ceny = y;
              }
              //std::cout<<"Difference x: "<<x<<" cenx: "<<cenx<<" y:"<<y<<" ceny: "<<ceny<<std::endl;
              //distance between pixels
              dist+=sqrt(((x-cenx)*(x-cenx))+((y-ceny)*(y-ceny)));
          }
      }
  }
  //std::cout<<"Difference dist: "<<dist<<std::endl;
  if((dist<10000))//&&(maskBottom>0))
      difs = 0;
  else
      difs = numDiff;
  return (float)numDiff/(float)(src.rows*src.cols);
}

float paolMat::differenceMin(paolMat *img, int thresh, int size, int maskBottom)
{
    int offset;
    bool diff;
    int numDiff;
    int surroundThresh = 50;
    int dist;
    bool first;
    int cenx;
    int ceny;
    int total;
    //mask is set to a blank state
    if (!maskMin.data)
        maskMin=Mat::zeros(src.rows/scale,src.cols/scale,src.type());

    if (scale>=size+1)
        offset=scale;
    else
        offset=scale*2;

  numDiff = 0;
  first = true;
  //distance --
  dist = 0;
  //for everfor( int y = 0,int yy=0; y < img->src.rows; y+=scale,yy++)


  for (int y = offset, yy=offset/scale; y < (src.rows-(offset+maskBottom)); y+=scale,yy++)
  {
      //for every column
      for (int x = offset, xx=offset/scale; x < (src.cols-offset); x+=scale,xx++)
      {
          diff = false;
          //for each color channel
          for(int i = 0; i < 3; i++)
          {
              //if the difference (for this pixel) between the the current img and the previous img is greater than the threshold, difference is noted; diff = true
              if(abs((double)img->src.at<Vec3b>(y,x)[i]-(double)src.at<Vec3b>(y,x)[i])>thresh)
                  diff = true;
          }
          if(diff)
          {
              //std::cout<<"First if dif size: "<<size<<std::endl;
              //mask.at<Vec3b>(y,x)[1]=255;
              // for all the pixels surrounding the current pixel
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
              //std::cout<<"Second if diff"<<std::endl;
              numDiff++;
              //calculates total difference and modifies the mask accordingly
              total = abs((double)img->src.at<Vec3b>(y,x)[0]-(double)src.at<Vec3b>(y,x)[0]) +
                      abs((double)img->src.at<Vec3b>(y,x)[1]-(double)src.at<Vec3b>(y,x)[1]) +
                      abs((double)img->src.at<Vec3b>(y,x)[2]-(double)src.at<Vec3b>(y,x)[2]);
              if(total > 512)
              {
                  maskMin.at<Vec3b>(yy,xx)[0] = 255;
              }
              if(total > 255)
              {
                  maskMin.at<Vec3b>(yy,xx)[1] = 255;
                  numDiff++;
              }
              maskMin.at<Vec3b>(yy,xx)[2]=255;
              //sets location of first differnce found
              if(first)
              {
                  first = false;
                  cenx = x;
                  ceny = y;
              }
              //std::cout<<"Difference x: "<<x<<" cenx: "<<cenx<<" y:"<<y<<" ceny: "<<ceny<<std::endl;
              //distance between pixels
              dist+=sqrt(((x-cenx)*(x-cenx))+((y-ceny)*(y-ceny)));
          }
      }
  }
  //std::cout<<"Difference dist: "<<dist<<std::endl;
  if((dist<10000))//&&(maskBottom>0))
      difs = 0;
  else
      difs = numDiff;
  return (float)numDiff/(float)(maskMin.rows*maskMin.cols);
}

void paolMat::differenceLect(paolMat *inImg,int thresh, int size)
{
  bool diff;
  int numDiff;
  //int surroundThresh = 50;
  int dist;
  bool first;
  int cenx;
  int ceny;
  int pixDif;

  mask = Mat::zeros(src.size(), src.type());

  numDiff = 0;
  first = true;
  dist = 0;
  for (int y = 0; y < (src.rows-size-1); y+= size)
    for (int x = 0; x < (src.cols-size-1); x+=size)
      {
    diff = false;
    pixDif=abs((double)inImg->src.at<Vec3b>(y,x)[0]-
           (double)src.at<Vec3b>(y,x)[0])+
      abs((double)inImg->src.at<Vec3b>(y,x)[1]-
          (double)src.at<Vec3b>(y,x)[1])+
      abs((double)inImg->src.at<Vec3b>(y,x)[2]-
          (double)src.at<Vec3b>(y,x)[2]);
    if (pixDif>thresh)
      {
        mask.at<Vec3b>(y,x)[2] = 255;
        diff = true;
      }
    if(pixDif > 255)
      pixDif = 255;
    mask.at<Vec3b>(y,x)[1] = 0;//pixDif;
    mask.at<Vec3b>(y,x)[0] = 0;



    if(diff)
      {

        //  for(int a = 0; a <= size; a++)
        //  for(int b = 0; b <= size; b++)
        //mask.at<Vec3b>(y+a,x+b)[1]=255;
        numDiff++;
      }
    difs = numDiff;
      }
}

void paolMat::localizeSpeaker()
{

  int left = 0;
  int right = mask.cols-1;
  int top = 0;
  int x = 0;
  int y = 0;
  int count;
  int countIgnore = 10;

  if(difs > 750)
    {
      count = countIgnore;
      while(count >=  0 && (x < mask.cols))
    {
      for(y = 0; y < mask.rows; y++)
        {
          if(mask.at<Vec3b>(y,x)[1] > 0)
        {
          count--;
          left = x;
        }
        }
      x++;
    }

      x = mask.cols-1;
      count = countIgnore;
      while(count >= 0 && (x >= 0))
    {
      for(y = 0; y < mask.rows; y++)
        {
          if(mask.at<Vec3b>(y,x)[1] > 0)
        {
          count--;
          right = x;
        }
        }
      x--;
    }

      y=0;
      count = countIgnore;
      while(count >= 0 && (y < mask.cols))
    {
      for(x = left; x <= right; x++)
        {
          if(mask.at<Vec3b>(y,x)[1] > 0)
        {
          count--;
          top = y;
        }
        }
      y++;
    }


      prof.x = (left+right) / 2;
      prof.y = top;
      //std::cout<<"Prof at X: "<<prof.x<<" Y: "<<prof.y<<std::endl;
      lectFound = true;
#ifdef _debug_
      if(prof.x < 3)
    prof.x = 3;
      if(prof.x > (mask.cols-3))
    prof.x = mask.cols-4;
      if(prof.y < 3)
    prof.y = 3;
      if(prof.y > (mask.rows-3))
    prof.y = mask.rows-4;
      char difString[10];
      sprintf(difString, "%d", difs);
      //rectangle(src, Point(left,mask.rows-1), Point(right, top), Scalar(0,0,0), 1,8);
      //rectangle(mask, Point(left,mask.rows-1), Point(right, top), Scalar(255,0,0), 1,8);
      //rectangle(src, Point(prof.x-2,prof.y-2), Point(prof.x+2, prof.y+2), Scalar(0,0,0), 1,8);
      //rectangle(mask, Point(prof.x-2,prof.y-2), Point(prof.x+2, prof.y+2), Scalar(255,0,0), 1,8);
      //putText(src, difString, Point(100,100), CV_FONT_HERSHEY_SIMPLEX, 1, Scalar(0,0,0), 5, 8);
      //putText(mask, difString, Point(100,100), CV_FONT_HERSHEY_SIMPLEX, 1, Scalar(255,0,0), 5, 8);
#endif
    }else
    {
      prof.x = -1;
      prof.y = -1;
      lectFound = false;
#ifdef _debug_
      char difString[10];
      sprintf(difString, "%d", difs);
      //putText(mask, difString, Point(100,100), CV_FONT_HERSHEY_SIMPLEX, 1, Scalar(255,0,0), 5, 8);
#endif
    }
}

void paolMat::decimateMask()
{
  int left,right,top,bottom;
  for(int x = 0; x < src.cols-1; x++)
    for(int y = 0; y < src.rows-1; y++)
      {
    if(mask.at<Vec3b>(y,x)[1]==255)
      {
        left=(x-50<0)? 0:x-50;
        right=(x+50>mask.cols-1)? mask.cols-1:x+50;
        top=(y-50<0)? 0:y-50;
        bottom=(y+50>mask.rows-1)? mask.rows-1:y+50;
        for(int xx = left; xx < right; xx++)
          for(int yy = top; yy < bottom; yy++)
        {
          if(mask.at<Vec3b>(y,x)[0]>0 || mask.at<Vec3b>(y,x)[2]>0)
            mask.at<Vec3b>(y,x)[2]=128;
        }
      }
      }
  for(int x = 0; x < src.cols-1; x++)
    for(int y = 0; y < src.rows-1; y++)
      {
    if(mask.at<Vec3b>(y,x)[2]==128)
      mask.at<Vec3b>(y,x)[1]=255;
      }
}

void paolMat::decimateMask(int thresh)
{
  paolMat *temp;
  temp = new paolMat(this);
  temp->mask = Scalar(0,0,0);


  int count;
  for(int x = 1; x < mask.cols-1; x++)
    for(int y = 1; y < mask.rows-1; y++)
      {
    count = 0;
    if(mask.at<Vec3b>(y-1,x-1)[2] > thresh)
      count++;
    if(mask.at<Vec3b>(y-1,x)[2] > thresh)
      count++;
    if(mask.at<Vec3b>(y-1,x+1)[2] > thresh)
      count++;
    if(mask.at<Vec3b>(y+1,x-1)[2] > thresh)
      count++;
    if(mask.at<Vec3b>(y+1,x)[2] > thresh)
      count++;
    if(mask.at<Vec3b>(y+1,x+1)[2] > thresh)
      count++;
    if(mask.at<Vec3b>(y,x-1)[2] > thresh)
      count++;
    if(mask.at<Vec3b>(y,x+1)[2] > thresh)
      count++;
    if(count >5)
      temp->mask.at<Vec3b>(y,x)[2] = 255;
    else
      temp->mask.at<Vec3b>(y,x)[0] = 0;
      }
  mask = temp->mask.clone();
}
void paolMat::growMask()
{
  paolMat *temp;
  temp = new paolMat(this);
  temp->mask = Scalar(0,0,0);


  int total;
  for(int x = 1; x < mask.cols-1; x++)
    for(int y = 1; y < mask.rows-1; y++)
      {
    //y-1 ------------------------------
    //y-1,x-1
    total = mask.at<Vec3b>(y,x)[2] +
      mask.at<Vec3b>(y-1,x-1)[2];
    if(total > 255)
      total = 255;
    temp->mask.at<Vec3b>(y-1,x-1)[2] = total;
    //y-1,x
    total = mask.at<Vec3b>(y,x)[2] +
      mask.at<Vec3b>(y-1,x)[2];
    if(total > 255)
      total = 255;
    temp->mask.at<Vec3b>(y-1,x)[2] = total;
    //y-1,x+1
    total = mask.at<Vec3b>(y,x)[2] +
      mask.at<Vec3b>(y-1,x+1)[2];
    if(total > 255)
      total = 255;
    temp->mask.at<Vec3b>(y-1,x+1)[2] = total;

    //y+1 ---------------------------------
    //y+1,x-1
    total = mask.at<Vec3b>(y,x)[2] +
      mask.at<Vec3b>(y+1,x-1)[2];
    if(total > 255)
      total = 255;
    temp->mask.at<Vec3b>(y+1,x-1)[2] = total;
    //y+1,x
    total = mask.at<Vec3b>(y,x)[2] +
      mask.at<Vec3b>(y+1,x)[2];
    if(total > 255)
      total = 255;
    temp->mask.at<Vec3b>(y+1,x)[2] = total;
    //y+1,x+1
    total = mask.at<Vec3b>(y,x)[2] +
      mask.at<Vec3b>(y+1,x+1)[2];
    if(total > 255)
      total = 255;
    temp->mask.at<Vec3b>(y+1,x+1)[2] = total;

    //y ---------------------------------
    //yx-1
    total = mask.at<Vec3b>(y,x)[2] +
      mask.at<Vec3b>(y,x-1)[2];
    if(total > 255)
      total = 255;
    temp->mask.at<Vec3b>(y,x-1)[2] = total;
    //y,x+1
    total = mask.at<Vec3b>(y,x)[2] +
      mask.at<Vec3b>(y,x+1)[2];
    if(total > 255)
      total = 255;
    temp->mask.at<Vec3b>(y,x+1)[2] = total;

      }
  mask = temp->mask.clone();
}

void paolMat::shrinkMask()
{
  paolMat *temp;
  temp = new paolMat(this);
  temp->mask = Scalar(0,0,0);


  int total;
  for(int x = 1; x < mask.cols-1; x++)
    for(int y = 1; y < mask.rows-1; y++)
      {
    //y-1 ------------------------------
    //y-1,x-1
    total = mask.at<Vec3b>(y,x)[2] -
      mask.at<Vec3b>(y-1,x-1)[2];
    if(total < 0)
      total = 0;
    temp->mask.at<Vec3b>(y-1,x-1)[2] = total;
    //y-1,x
    total = mask.at<Vec3b>(y,x)[2] -
      mask.at<Vec3b>(y-1,x)[2];
    if(total < 0)
      total = 0;
    temp->mask.at<Vec3b>(y-1,x)[2] = total;
    //y-1,x+1
    total = mask.at<Vec3b>(y,x)[2] -
      mask.at<Vec3b>(y-1,x+1)[2];
    if(total < 0)
      total = 0;
    temp->mask.at<Vec3b>(y-1,x+1)[2] = total;

    //y+1 ---------------------------------
    //y+1,x-1
    total = mask.at<Vec3b>(y,x)[2] -
      mask.at<Vec3b>(y+1,x-1)[2];
    if(total < 0)
      total = 0;
    temp->mask.at<Vec3b>(y+1,x-1)[2] = total;
    //y+1,x
    total = mask.at<Vec3b>(y,x)[2] -
      mask.at<Vec3b>(y+1,x)[2];
    if(total < 0)
      total = 0;
    temp->mask.at<Vec3b>(y+1,x)[2] = total;
    //y+1,x+1
    total = mask.at<Vec3b>(y,x)[2] -
      mask.at<Vec3b>(y+1,x+1)[2];
    if(total < 0)
      total = 0;
    temp->mask.at<Vec3b>(y+1,x+1)[2] = total;

    //y ---------------------------------
    //yx-1
    total = mask.at<Vec3b>(y,x)[2] -
      mask.at<Vec3b>(y,x-1)[2];
    if(total < 0)
      total = 0;
    temp->mask.at<Vec3b>(y,x-1)[2] = total;
    //y,x+1
    total = mask.at<Vec3b>(y,x)[2] -
      mask.at<Vec3b>(y,x+1)[2];
    if(total < 0)
      total = 0;
    temp->mask.at<Vec3b>(y,x+1)[2] = total;

      }
  mask = temp->mask.clone();
}

void paolMat::shrinkMask2()
{
    paolMat *temp;
    temp = new paolMat(this);
    temp->mask = Scalar(0,0,0);


    int total;
    for(int x = 1; x < mask.cols-1; x++)
        for(int y = 1; y < mask.rows-1; y++)
        {
            total=0;
            for (int xx=x-1;xx<=x+1;xx++)
                for (int yy=y-1;yy<=y+1;yy++){
                    if(mask.at<Vec3b>(yy,xx)[2]==255)
                        total++;
                }
            if(total>3)
                temp->mask.at<Vec3b>(y,x)[2]=255;

        }
    if(mask.data)
        mask.~Mat();
    mask = temp->mask.clone();
    temp->~paolMat();
}

void paolMat::shrinkMaskMin2()
{
    Mat temp = Mat::zeros(maskMin.size(),maskMin.type());

    int total;
    for(int x = 1; x < maskMin.cols-1; x++)
        for(int y = 1; y < maskMin.rows-1; y++)
        {
            total=0;
            for (int xx=x-1;xx<=x+1;xx++)
                for (int yy=y-1;yy<=y+1;yy++){
                    if(maskMin.at<Vec3b>(yy,xx)[2]==255)
                        total++;
                }
            if(total>3)
                temp.at<Vec3b>(y,x)[2]=255;

        }
    if(maskMin.data)
        maskMin.~Mat();
    maskMin = temp.clone();
    temp.~Mat();
}

void paolMat::connected(){
  paolMat *connect;
  connect = new paolMat(this);
  std::vector<int> cor;
  cor.resize(256*256*256,0);
  std::vector<int> corCount;
  corCount.resize(256*256*256,0);
  int current=1;
  cor[1]=1;
  int upCol,leftCol,col;

  mask.at<Vec3b>(0,0)[0]=0;
  mask.at<Vec3b>(0,0)[1]=0;
  mask.at<Vec3b>(0,0)[2]=0;

  for(int x = 0; x < src.cols; x++)
    for(int y = 0; y < src.rows; y++)
      {
    if (mask.at<Vec3b>(y,x)[0]>0 ||
        mask.at<Vec3b>(y,x)[1]>0 ||
        mask.at<Vec3b>(y,x)[2]>0){
      if (x==0){
        if (connect->src.at<Vec3b>(y-1,x)[0]>0 ||
        connect->src.at<Vec3b>(y-1,x)[1]>0 ||
        connect->src.at<Vec3b>(y-1,x)[2]>0){
          connect->src.at<Vec3b>(y,x)[0]=connect->src.at<Vec3b>(y-1,x)[0];
          connect->src.at<Vec3b>(y,x)[1]=connect->src.at<Vec3b>(y-1,x)[1];
          connect->src.at<Vec3b>(y,x)[2]=connect->src.at<Vec3b>(y-1,x)[2];
        } else {
          connect->src.at<Vec3b>(y,x)[0]=current%256;
          connect->src.at<Vec3b>(y,x)[1]=(current/256)%256;
          connect->src.at<Vec3b>(y,x)[2]=(current/(256*256))%256;
          current++;
          cor[current]=current;
        }
      } else if(y==0){
        if (connect->src.at<Vec3b>(y,x-1)[0]>0 ||
        connect->src.at<Vec3b>(y,x-1)[1]>0 ||
        connect->src.at<Vec3b>(y,x-1)[2]>0){
          connect->src.at<Vec3b>(y,x)[0]=connect->src.at<Vec3b>(y,x-1)[0];
          connect->src.at<Vec3b>(y,x)[1]=connect->src.at<Vec3b>(y,x-1)[1];
          connect->src.at<Vec3b>(y,x)[2]=connect->src.at<Vec3b>(y,x-1)[2];
        } else {
          connect->src.at<Vec3b>(y,x)[0]=current%256;
          connect->src.at<Vec3b>(y,x)[1]=(current/256)%256;
          connect->src.at<Vec3b>(y,x)[2]=(current/(256*256))%256;
          current++;
          cor[current]=current;
        }
      } else {
        if((connect->src.at<Vec3b>(y-1,x)[0]>0 ||
        connect->src.at<Vec3b>(y-1,x)[1]>0 ||
        connect->src.at<Vec3b>(y-1,x)[2]>0) &&
           (connect->src.at<Vec3b>(y,x-1)[0]>0 ||
        connect->src.at<Vec3b>(y,x-1)[1]>0 ||
        connect->src.at<Vec3b>(y,x-1)[2]>0)){
          upCol=connect->src.at<Vec3b>(y-1,x)[0];
          upCol+=connect->src.at<Vec3b>(y-1,x)[1]*256;
          upCol+=connect->src.at<Vec3b>(y-1,x)[2]*256*256;
          leftCol=connect->src.at<Vec3b>(y,x-1)[0];
          leftCol+=connect->src.at<Vec3b>(y,x-1)[1]*256;
          leftCol+=connect->src.at<Vec3b>(y,x-1)[2]*256*256;
          if(upCol<leftCol){
        cor[leftCol]=cor[upCol];
        connect->src.at<Vec3b>(y,x)[0]=connect->src.at<Vec3b>(y-1,x)[0];
        connect->src.at<Vec3b>(y,x)[1]=connect->src.at<Vec3b>(y-1,x)[1];
        connect->src.at<Vec3b>(y,x)[2]=connect->src.at<Vec3b>(y-1,x)[2];
          } else {
        cor[upCol]=cor[leftCol];
        connect->src.at<Vec3b>(y,x)[0]=connect->src.at<Vec3b>(y,x-1)[0];
        connect->src.at<Vec3b>(y,x)[1]=connect->src.at<Vec3b>(y,x-1)[1];
        connect->src.at<Vec3b>(y,x)[2]=connect->src.at<Vec3b>(y,x-1)[2];
          }
        } else if (connect->src.at<Vec3b>(y-1,x)[0]>0 ||
               connect->src.at<Vec3b>(y-1,x)[1]>0 ||
               connect->src.at<Vec3b>(y-1,x)[2]>0){
          connect->src.at<Vec3b>(y,x)[0]=connect->src.at<Vec3b>(y-1,x)[0];
          connect->src.at<Vec3b>(y,x)[1]=connect->src.at<Vec3b>(y-1,x)[1];
          connect->src.at<Vec3b>(y,x)[2]=connect->src.at<Vec3b>(y-1,x)[2];
        } else if (connect->src.at<Vec3b>(y,x-1)[0]>0 ||
               connect->src.at<Vec3b>(y,x-1)[1]>0 ||
               connect->src.at<Vec3b>(y,x-1)[2]>0){
          connect->src.at<Vec3b>(y,x)[0]=connect->src.at<Vec3b>(y,x-1)[0];
          connect->src.at<Vec3b>(y,x)[1]=connect->src.at<Vec3b>(y,x-1)[1];
          connect->src.at<Vec3b>(y,x)[2]=connect->src.at<Vec3b>(y,x-1)[2];
        } else {
          connect->src.at<Vec3b>(y,x)[0]=current%256;
          connect->src.at<Vec3b>(y,x)[1]=(current/256)%256;
          connect->src.at<Vec3b>(y,x)[2]=(current/(256*256))%256;
          current++;
          cor[current]=current;
        }
      }
    }else{
      connect->src.at<Vec3b>(y,x)[0]=0;
      connect->src.at<Vec3b>(y,x)[1]=0;
      connect->src.at<Vec3b>(y,x)[2]=0;

    }
      }

  for(int i=1;i<current;i++){
    cor[i]=cor[cor[i]];
  }
  for(int x = 0; x < src.cols; x++)
    for(int y = 0; y < src.rows; y++)
      {
    col=connect->src.at<Vec3b>(y,x)[0];
    col+=connect->src.at<Vec3b>(y,x)[1]*256;
    col+=connect->src.at<Vec3b>(y,x)[2]*256*256;
    col=cor[col];
    corCount[col]++;
    connect->src.at<Vec3b>(y,x)[0]=col%256;
    connect->src.at<Vec3b>(y,x)[1]=(col/256)%256;
    connect->src.at<Vec3b>(y,x)[2]=(col/(256*256))%256;
      }

  for(int i=1;i<current;i++){
    if(corCount[i]<10)
      cor[i]=0;
  }
  for(int x = 0; x < src.cols; x++)
    for(int y = 0; y < src.rows; y++)
      {
    col=connect->src.at<Vec3b>(y,x)[0];
    col+=connect->src.at<Vec3b>(y,x)[1]*256;
    col+=connect->src.at<Vec3b>(y,x)[2]*256*256;
    col=cor[col];
    connect->src.at<Vec3b>(y,x)[0]=col%256;
    connect->src.at<Vec3b>(y,x)[1]=(col/256)%256;
    connect->src.at<Vec3b>(y,x)[2]=(col/(256*256))%256;
      }
#ifdef _debug_
  connect->name="connect";
  connect->write();
#endif
  for(int i=1;i<current;i++){
    cor[i]=0;
  }
  for(int x = 0; x < src.cols; x++)
    for(int y = 0; y < src.rows; y++)
      {
    if(connect->src.at<Vec3b>(y,x)[0]>0 && abs(x-prof.x) < 100 && y > prof.y-50)
      {
        col=connect->src.at<Vec3b>(y,x)[0];
        col+=connect->src.at<Vec3b>(y,x)[1]*256;
        col+=connect->src.at<Vec3b>(y,x)[2]*256*256;
        cor[col]=255;
      }
      }
  for(int x = 0; x < src.cols; x++)
    for(int y = 0; y < src.rows; y++)
      {
    col=connect->src.at<Vec3b>(y,x)[0];
    col+=connect->src.at<Vec3b>(y,x)[1]*256;
    col+=connect->src.at<Vec3b>(y,x)[2]*256*256;
    col=cor[col];
    connect->src.at<Vec3b>(y,x)[0]=col%256;
    connect->src.at<Vec3b>(y,x)[1]=(col/256)%256;
    connect->src.at<Vec3b>(y,x)[2]=(col/(256*256))%256;
      }
#ifdef _debug_
  connect->name="connect2";
  connect->write();
#endif
  connect->src.copyTo(mask);
}


void paolMat::connected(int size){
  paolMat *connect;
  connect = new paolMat(this);
  connect->src = Mat::zeros(src.size(), src.type());
  for(int x = size+1; x < src.cols-(size+2); x+=size)
    for(int y = size+1; y < src.rows-(size+2); y+=size)
      {
    int touch = 0;
    touch += mask.at<Vec3b>(y-size,x)[1]%255;
    touch += mask.at<Vec3b>(y+size,x)[1]%255;
    touch += mask.at<Vec3b>(y,x-size)[1]%255;
    touch += mask.at<Vec3b>(y,x+size)[1]%255;
    touch += mask.at<Vec3b>(y-size,x-size)[1]%255;
    touch += mask.at<Vec3b>(y-size,x+size)[1]%255;
    touch += mask.at<Vec3b>(y+size,x-size)[1]%255;
    touch += mask.at<Vec3b>(y+size,x+size)[1]%255;
    for(int xx = x; xx < x+size; xx++)
        for(int yy = y; yy < y+size; yy++)
          if(touch >= 1)
        {
          connect->src.at<Vec3b>(yy,xx)[1] = 255;
        }
      }


  //#ifdef _debug_
  connect->name="connect2";
  connect->write();
  //#endif
  connect->src.copyTo(mask);

}

void paolMat::keepMask(int blueThresh){
  int count;
  for(int x = 0; x < mask.cols; x++)
    for(int y = 0; y < mask.rows; y++){
      if(mask.at<Vec3b>(y,x)[0] < blueThresh){
    src.at<Vec3b>(y,x)[0]=255;
    src.at<Vec3b>(y,x)[1]=255;
    src.at<Vec3b>(y,x)[2]=255;
      }
    }
}

void paolMat::lectArea(){
  paolMat *profile;
  profile = new paolMat(this);
  int x,y;
  int border=5;
  int left,right;

  for(y = 0; y < src.rows; y++){
    //from left
    for(x = 0; (x < src.cols) && (mask.at<Vec3b>(y,x)[0]==0); x++)
      profile->mask.at<Vec3b>(y,x)[2]=0;
    if(x<src.cols-1)
      {
    //std::cout<<"y: "<<y<<" x: "<<x<<" cols: "<<src.cols<<std::endl;
    //x=(x-border<0)? 0:x-border;
    for(;x<src.cols;x++)
      profile->mask.at<Vec3b>(y,x)[2]=255;
      }
    //from right
    for(x = src.cols-1; (x >= 0) && (mask.at<Vec3b>(y,x)[0]==0); x--)
      profile->mask.at<Vec3b>(y,x)[1]=0;
    if(x>1)
      {
    //std::cout<<"y: "<<y<<" x: "<<x<<" cols: "<<src.cols<<std::endl;
    //x=(x+border>=src.cols)? src.cols-1:x+border;
    for(;x>0;x--)
      profile->mask.at<Vec3b>(y,x)[1]=255;
      }
  }
  for(x = 0; x < src.cols; x++)
    {
      //from top
      for(y = 0; (y < src.rows) && (mask.at<Vec3b>(y,x)[0]==0); y++)
    profile->mask.at<Vec3b>(y,x)[0]=0;
      if(y<=src.rows)
    {
      //	  y=(y-border<0)? 0:y-border;
      for(;y<src.rows;y++)
        profile->mask.at<Vec3b>(y,x)[0]=255;
    }
    }
#ifdef _debug_
  profile->name="profile";
  profile->writeMask();
#endif
  for(x = 0; x < src.cols; x++)
    for(y = 0; y < src.rows; y++)
      {
    if(profile->mask.at<Vec3b>(y,x)[0]+
       profile->mask.at<Vec3b>(y,x)[1]+
       profile->mask.at<Vec3b>(y,x)[2] >= 510)
      {
        profile->mask.at<Vec3b>(y,x)[0] = 0;//src.at<Vec3b>(y,x)[0];
        profile->mask.at<Vec3b>(y,x)[1] = 0;//src.at<Vec3b>(y,x)[1];
        profile->mask.at<Vec3b>(y,x)[2] = 255;//src.at<Vec3b>(y,x)[2];
      }else
      {
        profile->mask.at<Vec3b>(y,x)[0] = 0;//src.at<Vec3b>(y,x)[0];
        profile->mask.at<Vec3b>(y,x)[1] = 0;//src.at<Vec3b>(y,x)[1];
        profile->mask.at<Vec3b>(y,x)[2] = 0;//src.at<Vec3b>(y,x)[2];
      }
      }
#ifdef _debug_
  profile->name="profileA";
  profile->writeMask();
#endif
  for(x = 0; x < src.cols; x++)
    {
      //from top
      for(y = 0; (y < src.rows) && (profile->mask.at<Vec3b>(y,x)[2]==0); y++)
    profile->mask.at<Vec3b>(y,x)[2]=0;
      if(y<src.rows-1)
    {
      y=(y-border<0)? 0:y-border;
      for(;y<src.rows;y++)
        profile->mask.at<Vec3b>(y,x)[2]=255;
    }
    }

  for(y = 0; y < src.rows; y++){
    //from left
    for(x = 0; (x < src.cols) && (profile->mask.at<Vec3b>(y,x)[2]==0); x++)
      profile->mask.at<Vec3b>(y,x)[2]=0;
    if(x<src.cols-1)
      {
    //std::cout<<"y: "<<y<<" x: "<<x<<" cols: "<<src.cols<<std::endl;
    left=(x-border<0)? 0:x-border;
    //from right

    for(x = src.cols-1; (x >= 0) && (profile->mask.at<Vec3b>(y,x)[2]==0); x--)
      profile->mask.at<Vec3b>(y,x)[0]=0;
    if(x>1)
      {
        //std::cout<<"y: "<<y<<" x: "<<x<<" cols: "<<src.cols<<std::endl;
        right=(x+border>=src.cols)? src.cols-1:x+border;
        for(x=left;x<right;x++)
          profile->mask.at<Vec3b>(y,x)[2]=255;
      }
      }
  }

  /*
  for(x = 0; x < src.cols; x++)
    for(y = 0; y < src.rows; y++)
      {
    if(profile->mask.at<Vec3b>(y,x)[2] == 255)
      {
        profile->mask.at<Vec3b>(y,x)[0] = 0;//src.at<Vec3b>(y,x)[0];
        profile->mask.at<Vec3b>(y,x)[1] = 0;//src.at<Vec3b>(y,x)[1];
        profile->mask.at<Vec3b>(y,x)[2] = src.at<Vec3b>(y,x)[2];
      }else
      {
        profile->mask.at<Vec3b>(y,x)[0] = src.at<Vec3b>(y,x)[0];
        profile->mask.at<Vec3b>(y,x)[1] = src.at<Vec3b>(y,x)[1];
        profile->mask.at<Vec3b>(y,x)[2] = src.at<Vec3b>(y,x)[2];
      }
      } */
#ifdef _debug_
  profile->name="profileB";
  profile->writeMask();
#endif
  profile->mask.copyTo(mask);
}

paolMat *paolMat::cropFrame(int width, int height){
  paolMat *outFrame;
  outFrame = new paolMat(this);
  cv::Point upperLeft;
  cv::Point lowerRight;
  int temp;

  upperLeft.x=camera.x-width/2;
  upperLeft.y=camera.y-height/2;
  lowerRight.x=camera.x+width/2-1;
  lowerRight.y=camera.y+height/2-1;

  if(upperLeft.x<0)
    {
      temp=0-upperLeft.x;
      upperLeft.x+=temp;
      lowerRight.x+=temp;
    }
  if(lowerRight.x>src.cols-1)
    {
      temp=src.cols-1-lowerRight.x;
      upperLeft.x+=temp;
      lowerRight.x+=temp;
    }
  if(upperLeft.y<0)
    {
      temp=0-upperLeft.y;
      upperLeft.y+=temp;
      lowerRight.y+=temp;
    }
  if(lowerRight.y>src.rows-1)
    {
      temp=src.rows-1-lowerRight.y;
      upperLeft.y+=temp;
      lowerRight.y+=temp;
    }

  outFrame->src = Mat(src, Rect(upperLeft.x, upperLeft.y, lowerRight.x-upperLeft.x, lowerRight.y - upperLeft.y));

  outFrame->name = "Cropped";
#ifdef _debug_
  outFrame->write();
#endif
  return outFrame;

  //now use upper and lower right to crop the frame
}



paolMat *paolMat::crop(int x, int y, int width, int height){
  paolMat *outFrame;
  outFrame = new paolMat(this);

  outFrame->src = Mat(src, Rect(x, y, width, height));
  outFrame->mask = Mat(mask, Rect(x, y, width, height));
  outFrame->name = "Cropped";

  return outFrame;

}


vector<int> paolMat::vertMaskHistogram()
{
  vector<int> hist;
  int vertCount;


  for(int x = 0; x < mask.cols; x++)
    {
      vertCount = 0;
      for(int y = 0; y < mask.rows; y++)
    {
      if(mask.at<Vec3b>(y,x)[2] == 255)
        vertCount ++;
    }
      hist.push_back(vertCount);
    }
  return hist;
}

vector<int> paolMat::horMaskHistogram()
{
  vector<int> hist;
  int horCount;


  for(int y = 0; y < mask.rows; y++)
    {
      horCount = 0;
      for(int x = 0; x < mask.cols; x++)
    {
      if(mask.at<Vec3b>(y,x)[2] == 255)
        horCount ++;
    }
      hist.push_back(horCount);
    }
  return hist;
}

void paolMat::decimateMaskByHistogram(int hThresh, int vThresh)
{
  vector<int> horHisto;
  vector<int> vertHisto;

  horHisto = horMaskHistogram();
  vertHisto = vertMaskHistogram();

  for(int x = 0; x < mask.cols; x++)
    for(int y = 0; y < mask.rows; y++)
      {
    if( mask.at<Vec3b>(y,x)[2] == 255 &&
        ( (horHisto[y] > hThresh) &&
          (vertHisto[x] > vThresh)))
      mask.at<Vec3b>(y,x)[1] = 255;
    else
      {
        //mask.at<Vec3b>(y,x)[1] = 0;
        //mask.at<Vec3b>(y,x)[0] = 0;
        //mask.at<Vec3b>(y,x)[2] = 0;
      }
      }
}

void paolMat::drift()
{
  int temp;
  int total;
  for(int x = 0; x < src.cols -1; x++)
    for(int y = 0; y < src.rows -1; y++)
      {
    //Clear any previous mask values
    mask.at<Vec3b>(y,x)[0] = 0;
    mask.at<Vec3b>(y,x)[1] = 0;
    mask.at<Vec3b>(y,x)[2] = 0;

    //Get difference of x+1
    temp = abs( src.at<Vec3b>(y,x)[0] - src.at<Vec3b>(y,x+1)[0] )+
      abs( src.at<Vec3b>(y,x)[1] - src.at<Vec3b>(y,x+1)[1] )+
      abs( src.at<Vec3b>(y,x)[2] - src.at<Vec3b>(y,x+1)[2] );
    total = temp;
    if(temp > 255)
      temp = 255;
    mask.at<Vec3b>(y,x)[0] = temp;

    //Get difference of y+1
    temp = abs( src.at<Vec3b>(y,x)[0] - src.at<Vec3b>(y+1,x)[0] )+
      abs( src.at<Vec3b>(y,x)[1] - src.at<Vec3b>(y+1,x)[1] )+
      abs( src.at<Vec3b>(y,x)[2] - src.at<Vec3b>(y+1,x)[2] );
    total+=temp;
    if(temp > 255)
      temp = 255;

    //Threshold for differences from previous pixel
    if(total > 50)
      total = 255;
    mask.at<Vec3b>(y,x)[1]=temp;
    mask.at<Vec3b>(y,x)[2]=total;
      }
}

void paolMat::driftWAverage()
{
  average();
  int temp;
  int total;
  for(int x = 0; x < src.cols -1; x++)
    for(int y = 0; y < src.rows -1; y++)
      {
    temp = abs( src.at<Vec3b>(y,x)[0] - src.at<Vec3b>(y,x+1)[0] )+
      abs( src.at<Vec3b>(y,x)[1] - src.at<Vec3b>(y,x+1)[1] )+
      abs( src.at<Vec3b>(y,x)[2] - src.at<Vec3b>(y,x+1)[2] );
    total = temp;
    if(temp > 255)
      temp = 255;
    if(
       (abs( src.at<Vec3b>(y,x)[0] - b) < abs(src.at<Vec3b>(y,x+1)[0] -b)) &&
       (abs( src.at<Vec3b>(y,x)[1] - g) < abs(src.at<Vec3b>(y,x+1)[1] -g)) &&
       (abs( src.at<Vec3b>(y,x)[2] - r) < abs(src.at<Vec3b>(y,x+1)[2] -r)) )
      mask.at<Vec3b>(y,x)[0] = temp;
    else
      {
        mask.at<Vec3b>(y,x)[0] = temp;
        mask.at<Vec3b>(y,x+1)[0] = temp;
      }

    temp = abs( src.at<Vec3b>(y,x)[0] - src.at<Vec3b>(y+1,x)[0] )+
      abs( src.at<Vec3b>(y,x)[1] - src.at<Vec3b>(y+1,x)[1] )+
      abs( src.at<Vec3b>(y,x)[2] - src.at<Vec3b>(y+1,x)[2] );
    total+=temp;
    if(temp > 255)
      temp = 255;
    //Threshold for differences from previous pixel
    if(total > 50)
      total = 255;

    if(
       (abs( src.at<Vec3b>(y,x)[0] - b) < abs(src.at<Vec3b>(y+1,x)[0] -b)) &&
       (abs( src.at<Vec3b>(y,x)[1] - g) < abs(src.at<Vec3b>(y+1,x)[1] -g)) &&
       (abs( src.at<Vec3b>(y,x)[2] - r) < abs(src.at<Vec3b>(y+1,x)[2] -r)) )
      mask.at<Vec3b>(y,x)[1] = temp;
    else
      {
        mask.at<Vec3b>(y,x)[1] = temp;
        mask.at<Vec3b>(y+1,x)[1] = temp;
      }
    mask.at<Vec3b>(y,x)[2]=total;
      }
}


void paolMat::sweepMask()
{
  //Sweep down then up, then left, then right, filling in gaps smaller then thresh
  //if the color of the source in gap matched the color of the source at the mask

  cv::Mat temp;
  temp = src.clone();
  temp = Mat::zeros(src.size(),src.type());
  int thresh;
  thresh = 5;


  for(int x = 0; x < src.cols -1; x++)
    {
      int y = 0;
      int last = 0;
      while(y < src.rows-1)
    {
      if(last == 0)
        {
          if(mask.at<Vec3b>(y,x)[2] > 20)
        last = 1;
        }
      else if((last > 0) && (last < thresh))
        {
          if( (
           abs(src.at<Vec3b>(y-last,x)[0] - src.at<Vec3b>(y,x)[0])+
           abs(src.at<Vec3b>(y-last,x)[1] - src.at<Vec3b>(y,x)[1])+
           abs(src.at<Vec3b>(y-last,x)[2] - src.at<Vec3b>(y,x)[2]) )< 100 )
        {
          for(int i = 0; i < last; i++)
            {
              //std::cout<<"Found something to sweep"<<std::endl;
              mask.at<Vec3b>(y-last+i,x)[0] = 255;
              mask.at<Vec3b>(y-last+i,x)[1] = 255;
              mask.at<Vec3b>(y-last+i,x)[2] = 255;
            }
          last = 0;
        }
          else
        {
          last++;
        }
        }

      else if(last > thresh)
        last = 0;

      y++;
    }
    }
}

void paolMat::difference(paolMat *img)
{
  for( int y = 0; y < img->src.rows; y++)
    for(int x = 0; x < img->src.cols; x++)
      {
      mask.at<Vec3b>(y,x)[0] = abs( src.at<Vec3b>(y,x)[0] - img->src.at<Vec3b>(y,x)[0]);
      mask.at<Vec3b>(y,x)[1] = abs( src.at<Vec3b>(y,x)[1] - img->src.at<Vec3b>(y,x)[1]);
      mask.at<Vec3b>(y,x)[2] = abs( src.at<Vec3b>(y,x)[2] - img->src.at<Vec3b>(y,x)[2]);
      }
}

void paolMat::maskDifference(paolMat *img)
{
  for( int y = 0; y < img->src.rows; y++)
    for(int x = 0; x < img->src.cols; x++)
      {
      mask.at<Vec3b>(y,x)[0] = abs( mask.at<Vec3b>(y,x)[0] - img->mask.at<Vec3b>(y,x)[0]);
      mask.at<Vec3b>(y,x)[1] = abs( mask.at<Vec3b>(y,x)[1] - img->mask.at<Vec3b>(y,x)[1]);
      mask.at<Vec3b>(y,x)[2] = abs( mask.at<Vec3b>(y,x)[2] - img->mask.at<Vec3b>(y,x)[2]);
      }
}

void paolMat::intensityMask(int thresh)
{
  int last;
  int pTotal;

  for( int y = 0; y < src.rows-1; y++)
    {
      last = 0;
      for(int x = 0; x < src.cols-1; x++)
    {
      pTotal = 0;
      //Set the mask to black

      mask.at<Vec3b>(y,x)[0] =
        (
         abs(src.at<Vec3b>(y,x)[0] - src.at<Vec3b>(y,x)[1]) +
         abs(src.at<Vec3b>(y,x)[0] - src.at<Vec3b>(y,x)[2]) );
      mask.at<Vec3b>(y,x)[1] =
        (
         abs(src.at<Vec3b>(y,x)[1] - src.at<Vec3b>(y,x)[0]) +
         abs(src.at<Vec3b>(y,x)[1] - src.at<Vec3b>(y,x)[2]) );
      mask.at<Vec3b>(y,x)[2] =
        (
         abs(src.at<Vec3b>(y,x)[2] - src.at<Vec3b>(y,x)[0]) +
         abs(src.at<Vec3b>(y,x)[2] - src.at<Vec3b>(y,x)[1]) );

      pTotal = ( mask.at<Vec3b>(y,x)[0] +
             mask.at<Vec3b>(y,x)[1] +
             mask.at<Vec3b>(y,x)[2] );
      if( pTotal < thresh )
        {
          mask.at<Vec3b>(y,x)[0] = 0;
          mask.at<Vec3b>(y,x)[0] = 0;
          mask.at<Vec3b>(y,x)[0] = 0;
        }
      if( pTotal < thresh*2)
        mask.at<Vec3b>(y,x)[0] = 255;
      if( pTotal < thresh*3)
        mask.at<Vec3b>(y,x)[1] = 150;
      if( pTotal < thresh*4)
        mask.at<Vec3b>(y,x)[1] = 255;
      if( pTotal < thresh*5)
        mask.at<Vec3b>(y,x)[2] = 150;
      if( pTotal < thresh*6)
        mask.at<Vec3b>(y,x)[2] = 255;
    }
    }
}

void paolMat::maskToWhite(int thresh)
{
  paolMat *temp;
  temp = new paolMat(this);
  int red;
  int white;
  int yellow;
  for(int y = 4; y < src.rows-5; y++)
    for(int x = 4; x < src.cols-5; x++)
      {
    red = 0;
    white = 0;
    yellow = 0;
    for(int yy = y-5; yy < y +5; yy++)
      for(int xx = x-5; xx < x+5; xx++)
        {
          if(mask.at<Vec3b>(yy,xx)[2] == 255)
        red++;
          if( (mask.at<Vec3b>(yy,xx)[1] == 255) &&
          (mask.at<Vec3b>(yy,xx)[2] == 255) )
        yellow++;
          if( (mask.at<Vec3b>(yy,xx)[1] == 255) &&
          (mask.at<Vec3b>(yy,xx)[2] == 255) &&
          (mask.at<Vec3b>(yy,xx)[3] == 255) )
        white++;
        }
    if(white > 255)
      white = 255;
    temp->mask.at<Vec3b>(y,x)[0] = white;
    if(yellow > 255)
      yellow = 255;
    temp->mask.at<Vec3b>(y,x)[1] = yellow;
    if(red > 255)
      red = 255;
    temp->mask.at<Vec3b>(y,x)[2] = red;
      }
  for(int y = 0; y < 5; y++)
    for(int x = 0; x <src.cols; x++)
      {
    temp->mask.at<Vec3b>(y,x)[0] = 255;
    temp->mask.at<Vec3b>(y,x)[1] = 255;
    temp->mask.at<Vec3b>(y,x)[2] = 255;
      }
  for(int y = src.rows-5; y < src.rows; y++)
    for(int x = 0; x <src.cols; x++)
      {
    temp->mask.at<Vec3b>(y,x)[0] = 255;
    temp->mask.at<Vec3b>(y,x)[1] = 255;
    temp->mask.at<Vec3b>(y,x)[2] = 255;
      }
  for(int x = 0; x < 5; x++)
    for(int y = 0; y <src.rows; y++)
      {
    temp->mask.at<Vec3b>(y,x)[0] = 255;
    temp->mask.at<Vec3b>(y,x)[1] = 255;
    temp->mask.at<Vec3b>(y,x)[2] = 255;
      }
  for(int x = src.cols-5; x < src.cols; x++)
    for(int y = 0; y <src.rows; y++)
      {
    temp->mask.at<Vec3b>(y,x)[0] = 255;
    temp->mask.at<Vec3b>(y,x)[1] = 255;
    temp->mask.at<Vec3b>(y,x)[2] = 255;
      }

  int r,g;
  for(int x = 0; x < src.cols; x++)
    for(int y = 0; y < src.rows; y++)
      {
    /*
    if( (abs(temp->mask.at<Vec3b>(y,x)[0] - temp->mask.at<Vec3b>(y,x)[1]) +
         abs(temp->mask.at<Vec3b>(y,x)[0] - temp->mask.at<Vec3b>(y,x)[2]) )
        < 40)
      {
        mask.at<Vec3b>(y,x)[0] = 255;
        mask.at<Vec3b>(y,x)[1] = 255;
        mask.at<Vec3b>(y,x)[2] = 255;
      }
    else
      {
    */
        mask.at<Vec3b>(y,x)[0] = 2*temp->mask.at<Vec3b>(y,x)[0];
        mask.at<Vec3b>(y,x)[1] = 2*temp->mask.at<Vec3b>(y,x)[1];
        mask.at<Vec3b>(y,x)[2] = 2*temp->mask.at<Vec3b>(y,x)[2];
        //}
      }
}

void paolMat::average()
{
  int pixels;
  pixels = src.rows * src.cols;
  for(int x = 0; x < src.cols; x++)
    for(int y = 0; y < src.rows; y++)
      {
    b += src.at<Vec3b>(y,x)[0];
    g += src.at<Vec3b>(y,x)[1];
    r += src.at<Vec3b>(y,x)[2];
      }
  b/=pixels;
  g/=pixels;
  r/=pixels;
  //std::cout<<"Image average b: "<<b<<" g: "<<g<<" r: "<<r<<std::endl;
}

void paolMat::sweepDown()
{

    vector<int> hist;
    hist = vertMaskHistogram();
    paolMat *temp;
    temp = new paolMat(this);
    temp->mask = Scalar(0,0,0);

    for(int x = 0; x < src.cols; x++)
    {
        bool hit;
        hit = false;
        for(int y = 0; y < src.rows; y++)
        {
            if(mask.at<Vec3b>(y,x)[2] == 255)
                hit = true;

            if(hit == true)
                temp->mask.at<Vec3b>(y,x)[1] = 255;
        }
    }
    for(int y = 0; y < src.rows; y++)
    {
        bool hit;
        hit = false;
        for(int x = 0; x < src.cols; x++)
        {
            if(mask.at<Vec3b>(y,x)[2] == 255)
                hit = true;
            if(hit == true)
                temp->mask.at<Vec3b>(y,x)[0] = 255;
        }
    }

    for(int y = 0; y < src.rows; y++)
    {
        bool hit;
        hit = false;
        for(int x = src.cols-1; x >-1; x--)
        {
            if(mask.at<Vec3b>(y,x)[2] == 255)
                hit = true;
            if(hit == true)
                temp->mask.at<Vec3b>(y,x)[2] = 255;
        }
    }
    mask = temp->mask.clone();
    for(int x = 0; x < mask.cols; x++)
        if(hist[x] < 3)
            for(int y = 0; y < mask.rows; y++)
            {
                mask.at<Vec3b>(y,x)[0] = 0;
                mask.at<Vec3b>(y,x)[1] = 0;
                mask.at<Vec3b>(y,x)[2] = 0;
            }
}

void paolMat::sweepDownMin()
{

    //vector<int> hist;
    //hist = vertMaskHistogram();
    Mat temp=Mat::zeros(maskMin.size(),maskMin.type());

    for(int x = 0; x < maskMin.cols; x++)
    {
        bool hit;
        hit = false;
        for(int y = 0; y < maskMin.rows; y++)
        {
            if(maskMin.at<Vec3b>(y,x)[2] == 255)
                hit = true;

            if(hit == true)
                temp.at<Vec3b>(y,x)[1] = 255;
        }
    }
    for(int y = 0; y < maskMin.rows; y++)
    {
        bool hit;
        hit = false;
        for(int x = 0; x < maskMin.cols; x++)
        {
            if(maskMin.at<Vec3b>(y,x)[2] == 255)
                hit = true;
            if(hit == true)
                temp.at<Vec3b>(y,x)[0] = 255;
        }
    }

    for(int y = 0; y < maskMin.rows; y++)
    {
        bool hit;
        hit = false;
        for(int x = maskMin.cols-1; x >-1; x--)
        {
            if(maskMin.at<Vec3b>(y,x)[2] == 255)
                hit = true;
            if(hit == true)
                temp.at<Vec3b>(y,x)[2] = 255;
        }
    }
    if(maskMin.data)
        maskMin.~Mat();
    maskMin = temp.clone();
    temp.~Mat();
    /*for(int x = 0; x < maskMin.cols; x++)
        if(hist[x] < 3)
            for(int y = 0; y < maskMin.rows; y++)
            {
                maskMin.at<Vec3b>(y,x)[0] = 0;
                maskMin.at<Vec3b>(y,x)[1] = 0;
                maskMin.at<Vec3b>(y,x)[2] = 0;
            }
            */
}

void paolMat::keepWhite(){
    for(int x = 0; x < src.cols; x++)
    {
        for(int y = 0; y < src.rows; y++)
        {
            if(!(mask.at<Vec3b>(y,x)[0] == 255 &&
                 mask.at<Vec3b>(y,x)[1] == 255 &&
                 mask.at<Vec3b>(y,x)[2] == 255)){
                for (int i=0;i<3;i++)
                    mask.at<Vec3b>(y,x)[i]=0;
            }
        }
    }
}

void paolMat::keepWhiteMin(){
    bool left,right;
    int x;
    qDebug("cols=%d row=%d",maskMin.cols,maskMin.rows);
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
    for(x=0;x<maskMin.cols;x++){
        if(maskMin.at<Vec3b>(1,x)[0] == 255)
            for (int c=0;c<3;c++)
                maskMin.at<Vec3b>(0,x)[c]=255;
        if(maskMin.at<Vec3b>(maskMin.rows-2,x)[0] == 255)
            for (int c=0;c<3;c++)
                maskMin.at<Vec3b>(maskMin.rows-1,x)[c]=255;
    }
    left=false;
    right=false;
    for(int y = 0; y < maskMin.rows; y++)
    {
        if(maskMin.at<Vec3b>(y,1)[0] == 255){
            left=true;
        }
        if(left){
            x=0;
            while(maskMin.at<Vec3b>(y,x)[0]!=255){
                for (int c=0;c<3;c++)
                    maskMin.at<Vec3b>(y,x)[c]=255;
                x++;
            }
        }
        if(maskMin.at<Vec3b>(y,maskMin.cols-2)[0] == 255){
            right=true;
        }
        if(right){
            x=maskMin.cols-1;
            while(maskMin.at<Vec3b>(y,x)[0]!=255){
                for (int c=0;c<3;c++)
                    maskMin.at<Vec3b>(y,x)[c]=255;
                x--;
            }

        }
    }

}

void paolMat::blackSrcByMask()
{
  int pTotal;
  for(int y = 0; y < mask.rows; y++)
    for(int x = 0; x < mask.cols; x++)
      {
    pTotal = (mask.at<Vec3b>(y,x)[0] +
          mask.at<Vec3b>(y,x)[1] +
          mask.at<Vec3b>(y,x)[2] );
    if(pTotal > 512)
      {
        src.at<Vec3b>(y,x)[0] = 0;
        src.at<Vec3b>(y,x)[1] = 0;
        src.at<Vec3b>(y,x)[2] = 0;
      }

      }

}


void paolMat::blur(int size)
{
  int tempR, tempB, tempG, area;
  //blur size is pixels adjacent i.e. 1 would be a 3x3 square centered on each pixel
  area = (size *2+1)*(size *2+1);

  paolMat *temp;
  temp = new paolMat(this);
  temp->src = Scalar(0,0,0);

  for(int y = size; y < mask.rows - size; y++)
    for(int x = size; x < mask.cols -size; x++)
      {
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
    temp->src.at<Vec3b>(y,x)[2] = tempR;
    temp->src.at<Vec3b>(y,x)[1] = tempG;
    temp->src.at<Vec3b>(y,x)[0] = tempB;

      }
  if(src.data)
      src.~Mat();
  src = temp->src.clone();
  temp->~paolMat();
}

void paolMat::pDrift()
{
    int temp,total;
    //int toggle,thresh;
    //int redC,greenC,blueC,red,green,blue,back,forward;
    if (mask.data){
        mask.~Mat();
    }

    mask=Mat::zeros(src.size(),src.type());
    qDebug("rows=%d cols=%d",mask.rows,mask.cols);
    for(int y = 1; y < src.rows -1; y++)
        for(int x = 1; x < src.cols -1; x++)
        {
            temp = (
                        //y,x+1
                        abs(src.at<Vec3b>(y,x)[0] - src.at<Vec3b>(y,x+1)[0])+
                    abs(src.at<Vec3b>(y,x)[1] - src.at<Vec3b>(y,x+1)[1])+
                    abs(src.at<Vec3b>(y,x)[2] - src.at<Vec3b>(y,x+1)[2])+
                    //y,x-1
                    abs(src.at<Vec3b>(y,x)[0] - src.at<Vec3b>(y,x-1)[0])+
                    abs(src.at<Vec3b>(y,x)[1] - src.at<Vec3b>(y,x-1)[1])+
                    abs(src.at<Vec3b>(y,x)[2] - src.at<Vec3b>(y,x-1)[2])
                    );

            if(temp > 255)
                temp = 255;

            mask.at<Vec3b>(y,x)[2] = temp;
            total = temp;

            temp = (
                        //y+1,x
                        abs(src.at<Vec3b>(y,x)[0] - src.at<Vec3b>(y+1,x)[0])+
                    abs(src.at<Vec3b>(y,x)[1] - src.at<Vec3b>(y+1,x)[1])+
                    abs(src.at<Vec3b>(y,x)[2] - src.at<Vec3b>(y+1,x)[2])+
                    //y-1,
                    abs(src.at<Vec3b>(y,x)[0] - src.at<Vec3b>(y-1,x)[0])+
                    abs(src.at<Vec3b>(y,x)[1] - src.at<Vec3b>(y-1,x)[1])+
                    abs(src.at<Vec3b>(y,x)[2] - src.at<Vec3b>(y-1,x)[2])
                    );
            if(temp > 255)
                temp = 255;
            total+=temp;
            if(total > 255)
                total = 255;

            mask.at<Vec3b>(y,x)[1] = temp;
            mask.at<Vec3b>(y,x)[0] = total;

        }

}

void paolMat::grow(int blueThresh, int size)
{
    int temp;
    for(int y = size; y < src.rows - size; y++)
        for(int x = size ; x < src.cols - size; x++){
            temp=mask.at<Vec3b>(y,x)[0];
            temp+=mask.at<Vec3b>(y,x)[1];
            temp+=mask.at<Vec3b>(y,x)[2];
            if (temp>255)
                temp=255;
            mask.at<Vec3b>(y,x)[0]=temp;//mask.at<Vec3b>(y,x)[2];//copy red to blue added 4/23/14
        }
    for(int y = size; y < src.rows - size; y++)
        for(int x = size ; x < src.cols - size; x++)
            if(mask.at<Vec3b>(y,x)[0] > blueThresh)
                for(int yy = y-size; yy <= y+size;yy++)
                    for(int xx = x-size; xx <= x+size; xx++)
                        mask.at<Vec3b>(yy,xx)[2] = 255;

    for(int y = 0; y < src.rows; y++)
        for(int x = 0; x < src.cols; x++)
            mask.at<Vec3b>(y,x)[0] = mask.at<Vec3b>(y,x)[2];

}

void paolMat::growMin(int blueThresh, int size)
{
    int temp;
    for(int y = size; y < maskMin.rows - size; y++)
        for(int x = size ; x < maskMin.cols - size; x++){
            temp=maskMin.at<Vec3b>(y,x)[0];
            temp+=maskMin.at<Vec3b>(y,x)[1];
            temp+=maskMin.at<Vec3b>(y,x)[2];
            if (temp>255)
                temp=255;
            maskMin.at<Vec3b>(y,x)[0]=temp;//mask.at<Vec3b>(y,x)[2];//copy red to blue added 4/23/14
        }
    for(int y = size; y < maskMin.rows - size; y++)
        for(int x = size ; x < maskMin.cols - size; x++)
            if(maskMin.at<Vec3b>(y,x)[0] > blueThresh)
                for(int yy = y-size; yy <= y+size;yy++)
                    for(int xx = x-size; xx <= x+size; xx++)
                        maskMin.at<Vec3b>(yy,xx)[2] = 255;

    for(int y = 0; y < maskMin.rows; y++)
        for(int x = 0; x < maskMin.cols; x++)
            maskMin.at<Vec3b>(y,x)[0] = maskMin.at<Vec3b>(y,x)[2];

}

void paolMat::shrink(int blueThresh, int size)
{
  paolMat *temp;
  temp = new paolMat(this);
  temp->mask = Scalar(0,0,0);

  int total,area;
  area=(2*size+1)*(2*size+1);

  for(int y = size; y < src.rows - size; y++)
    for(int x = size ; x < src.cols - size; x++)
      if(mask.at<Vec3b>(y,x)[0] > blueThresh)
    {
      total = 0;
      for(int yy = y-size; yy<=y+size; yy++)
        for(int xx = x-size; xx<=x+size; xx++)
          if(mask.at<Vec3b>(yy,xx)[0] > blueThresh)
        total++;
      if(total>=area-1)
        temp->mask.at<Vec3b>(y,x)[0] = 255;
    }
  mask = temp->mask.clone();
}

//threshedDifference, only where both masks blue > 30
void paolMat::threshedDifference(paolMat *old)
{
  int r,g,b, ave, difference;

  for(int y = 0; y < src.rows; y++)
    for(int x = 0; x < src.cols; x++)
      if(mask.at<Vec3b>(y,x)[0] > 30 && old->mask.at<Vec3b>(y,x)[0] > 30)
    {
      b = abs(old->src.at<Vec3b>(y,x)[0] - src.at<Vec3b>(y,x)[0]);
      g = abs(old->src.at<Vec3b>(y,x)[1] - src.at<Vec3b>(y,x)[1]);
      r = abs(old->src.at<Vec3b>(y,x)[2] - src.at<Vec3b>(y,x)[2]);

      if(b+g+r > 40)
        {
          mask.at<Vec3b>(y,x)[1] = 255;
          mask.at<Vec3b>(y,x)[2] = 0;
        }
      else
        {
          mask.at<Vec3b>(y,x)[1] = 0;
          mask.at<Vec3b>(y,x)[2] = 255;
        }

    }
      else
    {
      mask.at<Vec3b>(y,x)[1] = 0;
      mask.at<Vec3b>(y,x)[2] = 0;
    }
}

void paolMat::getCombine(paolMat *img)
{
  for(int y = 0; y < mask.rows; y++)
    for(int x = 0; x < mask.cols; x++)
      if(img->mask.at<Vec3b>(y,x)[0] != 0 )
    src.at<Vec3b>(y,x) = img->src.at<Vec3b>(y,x);
}

void paolMat::blackMaskByMask(paolMat *img)
{
  for(int y = 0; y < mask.rows; y++)
    for(int x = 0; x < mask.cols; x++)
      if( img->mask.at<Vec3b>(y,x)[0] +
      img->mask.at<Vec3b>(y,x)[1] +
      img->mask.at<Vec3b>(y,x)[2] != 0 )
    {
      mask.at<Vec3b>(y,x)[2] = 0;
    }
}

void paolMat::updateBackground(paolMat *alt, paolMat *img)
{
  for(int y = 0; y < mask.rows; y++)
    for(int x = 0; x < mask.cols; x++)
      {
    //Update src
    if(
       alt->mask.at<Vec3b>(y,x)[0] +
       alt->mask.at<Vec3b>(y,x)[1] +
       alt->mask.at<Vec3b>(y,x)[2] == 0 )
      {
        if(img->mask.at<Vec3b>(y,x)[2] != 0)
          src.at<Vec3b>(y,x) = img->src.at<Vec3b>(y,x);
        else if(img->mask.at<Vec3b>(y,x)[1] +
            img->mask.at<Vec3b>(y,x)[2] == 0)
          {
        src.at<Vec3b>(y,x)[0] = 255;
        src.at<Vec3b>(y,x)[1] = 255;
        src.at<Vec3b>(y,x)[2] = 255;
          }
      }

    //Update mask
    if(
       alt->mask.at<Vec3b>(y,x)[0] +
       alt->mask.at<Vec3b>(y,x)[1] +
       alt->mask.at<Vec3b>(y,x)[2] == 0 )
      {
        mask.at<Vec3b>(y,x)[0] = img->mask.at<Vec3b>(y,x)[0];
        mask.at<Vec3b>(y,x)[1] = img->mask.at<Vec3b>(y,x)[1];
        mask.at<Vec3b>(y,x)[2] = img->mask.at<Vec3b>(y,x)[2];
      }

      }
}

void paolMat::updateBackground()
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
    //blank the first row because it had bad information for whatever reason
    for(y = 0; y < mask.rows; y++){
        x=0;
        src.at<Vec3b>(y,x)[0] = 255;
        src.at<Vec3b>(y,x)[1] = 255;
        src.at<Vec3b>(y,x)[2] = 255;
    }
}

void paolMat::darken()
{
  int x,y,temp;
  for(y = 0; y < mask.rows; y++)
    for(x = 0; x < mask.cols; x++){
      for (int z=0; z<3; z++){
    temp=src.at<Vec3b>(y,x)[z]-30;
    if (temp<0)
      temp=0;
    src.at<Vec3b>(y,x)[z]=temp;
      }
    }
}

void paolMat::cleanBackground(paolMat *img)
{
  //This method goes first left to right then top to bottom a single row
  //or column at a time. It looks for a single section of none pure white
  //area (an area that contains some text, probably). When it finds a section
  //it processes that section, ignoring all others. Within that section
  //it figures out the darkest whiteboard section and uses that as true white
  //for the section. It then incresaes the difference between the text (darker
  //then white) and the background
  paolMat *result;
  result = new paolMat(this);
  //result->src = Scalar(255,255,255);
  result->mask = Scalar(0,0,0);

  int start,end;//used to store the start and end of a section of possible text
  int darkestAve;//Average (greyscale) darkest pixel in section
  int brightestAve;//Average (greyscale) lightest pixel in section (presumed whiteboard)
  int r,g,b,temp,temp2,tempOld;
  int wr,wg,wb;//darker endpoint of section (presumed darkest whiteboard)
  int rOff,gOff,bOff;//difference (offset) of darkest whiteboard from pure white
  int oRange,range,rOut,gOut,bOut;
  int startBright,endBright;
  int rangeEtoE;//range section end ot end
  int rangeTotal;//total range of section

  for(int y = 0; y < src.rows; y++)
    for(int x = 0; x < src.cols; x++)
      if( src.at<Vec3b>(y,x)[0] !=255 ||
      src.at<Vec3b>(y,x)[1] !=255 ||
      src.at<Vec3b>(y,x)[2] !=255 ){
    start = x;//set start of section at first non-white
    darkestAve = 1000;//set darkest value at something brighter then it could be
    brightestAve = 0;//set brightest value at something darger then it could ever be

    //for all pixels until next white pixel
    for(;x < src.cols && ( src.at<Vec3b>(y,x)[0] !=255 ||
                   src.at<Vec3b>(y,x)[1] !=255 ||
                   src.at<Vec3b>(y,x)[2] !=255 ); x++){
      end = x;//reset end of section
      r = img->src.at<Vec3b>(y,x)[2];
      g = img->src.at<Vec3b>(y,x)[1];
      b = img->src.at<Vec3b>(y,x)[0];
      temp = 255 * 3 -(r+g+b);//set temp to the average difference from white
      temp /= 3;

      //reset darkest and lightest as appropriate
      if(temp < darkestAve)
        darkestAve = temp;
      if(temp > brightestAve)
        brightestAve = temp;
    }
    //get brightness of ends of section
    startBright = ( img->src.at<Vec3b>(y,start)[0] +
            img->src.at<Vec3b>(y,start)[1] +
            img->src.at<Vec3b>(y,start)[2] );
    endBright = ( img->src.at<Vec3b>(y,end)[0] +
         img->src.at<Vec3b>(y,end)[1] +
         img->src.at<Vec3b>(y,end)[2] );

    rangeEtoE=abs(startBright-endBright);
    rangeTotal=brightestAve-darkestAve;

    //set whiteboard RGB to pixel at darker end of section
    if(endBright > startBright){
      wr = img->src.at<Vec3b>(y,start)[2];
      wg = img->src.at<Vec3b>(y,start)[1];
      wb = img->src.at<Vec3b>(y,start)[0];
    } else {
      wr = img->src.at<Vec3b>(y,end)[2];
      wg = img->src.at<Vec3b>(y,end)[1];
      wb = img->src.at<Vec3b>(y,end)[0];
    }
    //set offset from white to difference of darker pixel from white
    rOff = 255 - wr;
    gOff = 255 - wg;
    bOff = 255 - wb;

    //set temp to average difference between darker pixel and white
    temp = rOff+gOff+bOff;//255 * 3 - (wr+wg+wb);
    temp /=3;

    //haven't a clue anymore, but it seems to be working
    oRange = abs(temp-brightestAve);//difference between brightest and darkest white pixels
    range = oRange+temp;//brightest pixel


    for(int xx = start; xx <= end; xx++){
      //if(oRange == 0){
      if(rangeTotal < 5 || oRange==0){
        rOut = 0;
        gOut = 0;
        bOut = 0;
      }else{
        rOut = ( (255 - img->src.at<Vec3b>(y,xx)[2]) - rOff) * 3;//range/oRange;
        gOut = ( (255 - img->src.at<Vec3b>(y,xx)[1]) - gOff) * 3;//range/oRange;
        bOut = ( (255 - img->src.at<Vec3b>(y,xx)[0]) - bOff) * 3;//range/oRange;

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
      result->src.at<Vec3b>(y,xx)[0] = 255 - bOut;
      result->src.at<Vec3b>(y,xx)[1] = 255 - gOut;
      result->src.at<Vec3b>(y,xx)[2] = 255 - rOut;
    }
      }
  for(int x = 0; x < src.cols; x++)
    for(int y = 0; y < src.rows; y++)
      {
    if( (src.at<Vec3b>(y,x)[0] != 255) ||
        (src.at<Vec3b>(y,x)[1] != 255) ||
        (src.at<Vec3b>(y,x)[2] != 255) )
      {
        start = y;
        darkestAve = 1000;
        brightestAve = 0;
        for(;y < src.rows && ( (src.at<Vec3b>(y,x)[0] != 255) ||
                 (src.at<Vec3b>(y,x)[1] != 255) ||
                 (src.at<Vec3b>(y,x)[2] != 255) ); y++)
          {
        end = y;
        r = img->src.at<Vec3b>(y,x)[2];
        g = img->src.at<Vec3b>(y,x)[1];
        b = img->src.at<Vec3b>(y,x)[0];
        temp = 255*3 - (r+g+b);
        temp/=3;
        oRange = abs(temp-brightestAve);
        range=oRange+temp;

        for(int yy = start; yy <= end; yy++)
          {
            if(oRange==0)
              {
            rOut=0;
            gOut=0;
            bOut=0;
              }
            else
              {
            rOut=( (255-img->src.at<Vec3b>(yy,x)[2]) - rOff)*range/oRange;
            gOut=( (255-img->src.at<Vec3b>(yy,x)[1]) - gOff)*range/oRange;
            bOut=( (255-img->src.at<Vec3b>(yy,x)[0]) - bOff)*range/oRange;

            if(rOut>255)
              rOut=255;
            else if(rOut<0)
              rOut=0;
            if(gOut>255)
              gOut=255;
            else if(gOut<0)
              gOut=0;
            if(bOut>255)
              bOut=255;
            else if(bOut<0)
              bOut=0;
              }
            temp2 = 255*3 - (rOut+gOut+bOut);
            if(temp2 > (result->src.at<Vec3b>(yy,x)[0] +
                   result->src.at<Vec3b>(yy,x)[1] +
                   result->src.at<Vec3b>(yy,x)[2] ))
              {
            result->mask.at<Vec3b>(yy,x)[0] = 255;
            if( rOut+gOut+bOut == 0)
              result->mask.at<Vec3b>(yy,x)[2] = 255;
            else
              {
                result->src.at<Vec3b>(yy,x)[2] = 255 - rOut;
                result->src.at<Vec3b>(yy,x)[1] = 255 - gOut;
                result->src.at<Vec3b>(yy,x)[0] = 255 - bOut;
              }
              }
          }
          }

      }
      }
  result->name = "cleaned";
  //result->writeMask();
  if(src.data)
      src.~Mat();
  src = result->src.clone();
  result->~paolMat();
  //result->write();
}

void paolMat::differenceDarken(paolMat *img)
{
  int temp1, temp2;
  int tempDifs;
  tempDifs = 0;
  for(int x = 0; x < src.cols; x++)
    for(int y = 0; y < src.rows; y++)
      {
    temp1 = ( (255 - src.at<Vec3b>(y,x)[0]) +
          (255 - src.at<Vec3b>(y,x)[1]) +
          (255 - src.at<Vec3b>(y,x)[2]) );

    temp2 = ( (255 - img->src.at<Vec3b>(y,x)[0]) +
          (255 - img->src.at<Vec3b>(y,x)[1]) +
          (255 - img->src.at<Vec3b>(y,x)[2]) );
    if(temp2 > temp1)
      {
        src.at<Vec3b>(y,x)[0] = img->src.at<Vec3b>(y,x)[0];
        src.at<Vec3b>(y,x)[1] = img->src.at<Vec3b>(y,x)[1];
        src.at<Vec3b>(y,x)[2] = img->src.at<Vec3b>(y,x)[2];
        tempDifs++;
      }

      }
  difs = tempDifs;
}

void paolMat::maskGrowRed(int size)
{
  paolMat *temp;
  temp = new paolMat(this);
  temp->mask = Scalar(0,0,0);
  for(int x = size; x < mask.cols - size; x++)
    for(int y = size; y < mask.rows -size; y++)
      {
    int foo;
    foo = 0;
    foo += (mask.at<Vec3b>(y,x)[0] +
           mask.at<Vec3b>(y,x)[1] +
           mask.at<Vec3b>(y,x)[2] );
    if( foo >= 255)
      {
        temp->mask.at<Vec3b>(y,x)[1] = 255;
        for(int xx = x -size; xx < x+size; xx++)
          for(int yy = y -size; yy < y+size; yy++)
        {
          temp->mask.at<Vec3b>(yy,xx)[2] = 255;
        }
      }
      }
  mask = temp->mask.clone();
}

void paolMat::contoursToMask(){
    int count;

    for(int x = 0; x < src.cols; x++)
      for(int y = 0; y < src.rows; y++)
        {
          count=0;
          for(int c=0;c<3;c++)
              count+=src.at<Vec3b>(y,x)[c];
          if(count>0)
              mask.at<Vec3b>(y,x)[2]=255;
      }
}

void paolMat::countDiffsMasked(paolMat *img)
{
  int count;
  count = 0;
  paolMat *temp;
  temp = new paolMat(this);
  temp->mask = Scalar(0,0,0);
  for(int x = 0; x < mask.cols; x++)
    for(int y = 0; y < mask.rows; y++)
      {
    if(mask.at<Vec3b>(y,x)[2] != 255)
      if( ( abs(src.at<Vec3b>(y,x)[0] - img->src.at<Vec3b>(y,x)[0]) +
        abs(src.at<Vec3b>(y,x)[1] - img->src.at<Vec3b>(y,x)[1]) +
        abs(src.at<Vec3b>(y,x)[2] - img->src.at<Vec3b>(y,x)[2]) ) > 0)
        {
          count++;
          temp->mask.at<Vec3b>(y,x)[2] = 255;
        }
      }
  char foo[1024];
  sprintf(foo,"countedDifs-%d-",count);
  temp->name = foo;
  //temp->writeMask();
  difs = count;

}

void paolMat::finalWBUpdate(paolMat *current)
{
  int temp1, temp2;
  for(int x = 0; x < mask.cols; x++)
    for(int y = 0; y < mask.rows; y++)
      {
    if( (mask.at<Vec3b>(y,x)[2] == 255) &&
        (current->mask.at<Vec3b>(y,x)[2] == 255))
      {
        temp1 = ( (255 - src.at<Vec3b>(y,x)[0]) +
              (255 - src.at<Vec3b>(y,x)[1]) +
              (255 - src.at<Vec3b>(y,x)[2]) );

        temp2 = ( (255 - current->src.at<Vec3b>(y,x)[0]) +
              (255 - current->src.at<Vec3b>(y,x)[1]) +
              (255 - current->src.at<Vec3b>(y,x)[2]) );
        if(temp2 > temp1)
          {
        src.at<Vec3b>(y,x)[0] = current->src.at<Vec3b>(y,x)[0];
        src.at<Vec3b>(y,x)[1] = current->src.at<Vec3b>(y,x)[1];
        src.at<Vec3b>(y,x)[2] = current->src.at<Vec3b>(y,x)[2];
          }

      }
    else
      {
        src.at<Vec3b>(y,x)[0] = current->src.at<Vec3b>(y,x)[0];
        src.at<Vec3b>(y,x)[1] = current->src.at<Vec3b>(y,x)[1];
        src.at<Vec3b>(y,x)[2] = current->src.at<Vec3b>(y,x)[2];
      }

      }
  mask = current->mask.clone();
  copyNoSrc(current);
}
