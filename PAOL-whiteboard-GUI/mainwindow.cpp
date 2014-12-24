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

    dummyPM = new paolMat();

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

    // Extract previous and current frames
    Mat prevFrame = old->src.clone();
    Mat curFrame = cam->src.clone();

    // Find difference pixels
    Mat allDiffs;
    paolMat::differenceMin2(allDiffs, numDif, prevFrame, curFrame, 40, 1);

    // Temporary Mat to store true differences
    Mat filteredDiffs = Mat::zeros(allDiffs.size(), allDiffs.type());

    //if there is enough of a difference between the two images
    if(numDif>.03){
        paolMat::shrinkMaskMin2(filteredDiffs, refinedNumDif, allDiffs);
        count=0;
    } else {
        refinedNumDif=0;
    }
    qDebug("%f", refinedNumDif);

    //if the images are really identical, count the number of consecultive nearly identical images
    if (numDif < .000001)
        count++;

    //if the differences are enough that we know where the lecturer is or the images have been identical
    //for two frames, and hence no lecturer present
    if(refinedNumDif>.04 || (numDif <.000001 && count==2)){

        /////////////////////////////////////////////////////////////
        // Display the current frame being processed
        paolMat::displayMat(curFrame, *ui->imDisplay1);

        // Frame counter to save processed frames
        static int frameCount = 0;
        char *frameNum = new char[3];
        sprintf(frameNum, "%03d", frameCount);
        frameCount++;

        Mat markerLocation = paolMat::findMarker(curFrame);
        Mat darkenedText = paolMat::darkenText3(curFrame, markerLocation);
        paolMat::displayMat(markerLocation, *ui->imDisplay2);

        /////////////////////////////////////////////////////////////
        //identify where motion is

        Mat diffHulls = paolMat::expandDifferencesRegion(filteredDiffs);
        Mat diffHullsFullSize = paolMat::maskMinToMaskBinary2(diffHulls);
        paolMat::displayMat(diffHullsFullSize, *ui->imDisplay4);

        ////////////////////////////////////////////////////////////////////////////////

        // Update background image (whiteboard model)

        Mat newWboardModel = paolMat::updateBack3(background->src, darkenedText, diffHullsFullSize);
        // Copy updated whiteboard model
        background->src = newWboardModel;
        paolMat::displayMat(newWboardModel, *ui->imDisplay5);

        // Rectify the model
        Mat rectified = paolMat::rectifyImage2(newWboardModel);
        paolMat::displayMat(rectified, *ui->imDisplay6);

        //////////////////////////////////////////////////

        // TODO: figure out if saves need to be made based on detected marker
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
            paolMat pm;
            Mat marker = pm.findMarker(cam->src);
            Mat darkenedText = pm.darkenText3(cam->src, marker);
            imwrite("/home/paol/shared/out/darkenedText.png", darkenedText);

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
