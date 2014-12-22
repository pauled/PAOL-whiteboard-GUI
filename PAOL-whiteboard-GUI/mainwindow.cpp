#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    ui->imDisplay1->setScaledContents(true);
    ui->imDisplay2->setScaledContents(true);
    ui->imDisplay3->setScaledContents(true);
    ui->imDisplay4->setScaledContents(true);
    ui->imDisplay5->setScaledContents(true);
    ui->imDisplay6->setScaledContents(true);
    ui->imDisplay7->setScaledContents(true);
    ui->imDisplay8->setScaledContents(true);
    ui->imDisplay9->setScaledContents(true);
    ui->imDisplay10->setScaledContents(true);
    ui->imDisplay11->setScaledContents(true);
    ui->imDisplay12->setScaledContents(true);

    runCam=false;
    runData=false;
    pause=false;

    cam=new paolMat();
    old=new paolMat();
    background=new paolMat();
    backgroundRefined=new paolMat();
    oldBackgroundRefined=new paolMat();
    rawEnhanced=new paolMat();
    rectified=new paolMat();

    count=0;

    qTimer = new QTimer(this);
    connect(qTimer, SIGNAL(timeout()), this, SLOT(displayFrame()));
    qTimer->start(100);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::processWhiteboard(){
    //take picture
//    cam->displayImage(*ui->imDisplay1);

    //compare picture to previous picture and store differences in old->maskMin
    numDif=old->differenceMin(cam,40,1);
//qDebug(" numDif=%f\n",numDif);

    //if there is enough of a difference between the two images
    if(numDif>.03){
        //set up a new % that represents difference
        refinedNumDif=old->shrinkMaskMin();
        count=0;
    } else {
        refinedNumDif=0;
    }

    //if the images are really identical, count the number of consecultive nearly identical images
    if (numDif < .000001)
        count++;

//qDebug(" refinedNumDif=%f  numDif=%f\n",refinedNumDif,numDif);

    //if the differences are enough that we know where the lecturer is or the images have been identical
    //for two frames, and hence no lecturer present
    if(refinedNumDif>.04 || (numDif <.000001 && count==2)){

        /////////////////////////////////////////////////////////////
        //copy the input image and process it to highlight the text
        rawEnhanced->copy(cam);
        rawEnhanced->displayImage(*ui->imDisplay1);

//        // Frame counter to save processed frames
//        static int frameCount = 0;
//        char *frameNum = new char[3];
//        sprintf(frameNum, "%03d", frameCount);
//        frameCount++;

        // Get the connected components found by the DoG edge detector
        paolMat dog;
        dog.copy(cam);
        dog.dogEdges(13, 17, 1);
        dog.adjustLevels(0, 4, 1);
        dog.binarizeMask(10);
//        imwrite(string("/home/paol/shared/out/") + frameNum + "comps.png", dog.mask);

        // Actually get the components
        int **components;
        components = new int*[dog.src.rows];
        for(int i =0; i < dog.src.rows; i++)
            components[i] = new int[dog.src.cols];
        dog.getConnectedComponents(components);

        // Keep components that intersect with pDrift filter
        paolMat pDrift;
        pDrift.copy(cam);
        pDrift.pDrift();
        pDrift.binarizeMask(10);
//        imwrite(string("/home/paol/shared/out/") + frameNum + "drift.png", pDrift.mask);
        pDrift.addComponentsFromMask(components);
//        imwrite(string("/home/paol/shared/out/") + frameNum + "keptComps.png", pDrift.mask);

        // Whiten the whiteboard (note: pDrift.mask represents marker location)
        rawEnhanced->darkenText2(pDrift.mask);
        rawEnhanced->displayImage(*ui->imDisplay2);

        /////////////////////////////////////////////////////////////
        //identify where motion is

        //extend the area of differences and sweep differences for more solid area
        old->extendMaskMinToEdges();
        old->sweepDownMin();
        //keep only the solid area and grow that region
        old->keepWhiteMaskMin();
        old->growMin(8);
        //draw a convex hull around area of differences
        old->findContoursMaskMin();
        //fill in area surrounded by convex hull
        old->sweepDownMin();
        old->keepWhiteMaskMin();
        ///////////////////////////////////////////////////////////////////////

        //process to identify text location

        //smooth image
        cam->blur(1);
        //find edge information and store total edge information in 0 (blue) color channel of mask
        cam->pDrift();
        //grow the area around where the edges are found (if edge in channel 0 grow in channel 2)
        cam->grow(15,3);
        ////////////////////////////////////////////////////////////////////////////////

        //process to update background image

        //copy movement information into rawEnhanced and then expand to full mask
        rawEnhanced->copyMaskMin(old);
        rawEnhanced->maskMinToMaskBinary();
        rawEnhanced->displayMask(*ui->imDisplay4);

        //update the background image with new information
        background->updateBack2(rawEnhanced,cam);
        background->displayImage(*ui->imDisplay5);

        //copy text location information into mask
//        backgroundRefined->copyMask(background);
        rectified->rectifyImage(background);
//        rectified->displayImage(*ui->imDisplay6);

        //////////////////////////////////////////////////

        //figure out if saves need to be made

        //count the number of differences in the refined text area between refined images
        saveNumDif = oldBackgroundRefined->countDifsMask(backgroundRefined);
        qDebug("save dif=%f",saveNumDif);
        //NOTE: once the image has been cut down to just white board saveNumDif should be a way
        //of determining what to save, write now crap from wall background causes it to be useless

        //oldBackgroundRefined->displayMask(*ui->imDisplay12);
        //copy last clean whiteboard image
        oldBackgroundRefined->copy(backgroundRefined);
    }
}

void MainWindow::rectifyImage(){
    rectified->rectifyImage(cam);
    rectified->displayImage(*ui->imDisplay1);
}

void MainWindow::findLines(){
    old->findBoard(cam);
    old->displayImage(*ui->imDisplay1);
}

void MainWindow::displayFrame() {
    if(!pause && (runCam || runData)){

        old->copy(cam);

        if(runCam){
            cam->takePicture();
        }
        if(runData){
            if(!cam->readNext(this))
                runData=false;
        }
        //rectified->copy(cam);
        //cam->rectifyImage(rectified);

        bool testIndivFrames = false;
        if(testIndivFrames) {
            paolMat dog;
            dog.copy(cam);
            dog.dogEdges(17, 17, 1);
            dog.adjustLevels(0, 4, 1);
            dog.binarizeMask(10);

            imwrite("/home/paol/shared/out/dog.png", dog.mask);
            qDebug("Wrote frame");
            pause = !pause;
        }
        else
            processWhiteboard();

        //rectifyImage();
        //findLines();
    }
}


void MainWindow::on_camera_clicked()
{
    runData=false;
    if(cam->src.data){
        cam->~paolMat();
        cam=new paolMat();
    }

    cam->setCameraNum(1);
    cam->takePicture();
    background->copy(cam);
    backgroundRefined->copyClean(cam);
    oldBackgroundRefined->copyClean(cam);
    runCam=true;
    pause=false;
}

void MainWindow::on_loadDataSet_clicked()
{
    runCam=false;
    runData=false;
    if(cam->src.data){
        cam->~paolMat();
        cam=new paolMat();
    }

    cam->readNext(this);
    background->copy(cam);
    backgroundRefined->copyClean(cam);
    oldBackgroundRefined->copyClean(cam);
    runData=true;
    pause=false;
}

void MainWindow::on_pause_clicked()
{
    pause=!pause;
}
