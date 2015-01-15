#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "ImageScanner.h"
#include "webcamimagescanner.h"
#include "datasetimagescanner.h"
#include "clock.h"
#include <stdexcept>

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
    pause=false;

    scanner = NULL;

    // Initialize counts for processing
    stableWhiteboardCount = 0;
    saveImageCount = 0;
    capturedImageCount = 0;

    // Set timer
    qTimer = new QTimer(this);
    connect(qTimer, SIGNAL(timeout()), this, SLOT(workOnNextImage()));
    qTimer->start(100);
}

MainWindow::~MainWindow()
{
    delete ui;
    delete qTimer;
    delete scanner;
}

void MainWindow::workOnNextImage() {
    if(!pause && runCam) {
        // Try to take a picture
        bool gotPicture = takePicture();
        if(gotPicture) {
            processImage();
        }
        else {
            // We couldn't get the next picture, so stop processing
            qWarning("Stopped processing. Please choose a new file or restart the camera.");
            runCam = false;
        }
    }
}

bool MainWindow::takePicture() {
    return scanner->getNextImage(currentFrame);
}

void MainWindow::processImage() {
    displayMat(currentFrame, *ui->imDisplay1);

    //compare picture to previous picture and store differences in allDiffs
    float numDif;
    Mat allDiffs;
    WhiteboardProcessor::findAllDiffsMini(allDiffs, numDif, oldFrame, currentFrame, 40, 1);

    //if there is enough of a difference between the two images
    float refinedNumDif = 0;
    Mat filteredDiffs = Mat::zeros(currentFrame.size(), currentFrame.type());
    if(numDif>.03){
        // Find the true differences
        WhiteboardProcessor::filterNoisyDiffs(filteredDiffs, refinedNumDif, allDiffs);
        stableWhiteboardCount=0;
    }

    //if the images are really identical, count the number of consecultive nearly identical images
    if (numDif < .000001)
        stableWhiteboardCount++;

    //if the differences are enough that we know where the lecturer is or the images have been identical
    //for two frames, and hence no lecturer present
    if(refinedNumDif>.04 || (numDif <.000001 && stableWhiteboardCount==2)){
        // Find marker strokes and enhance the whiteboard (ie. make all non-marker white)
        Mat currentMarker = WhiteboardProcessor::findMarkerWithCC(currentFrame);
        Mat whiteWhiteboard = WhiteboardProcessor::whitenWhiteboard(currentFrame, currentMarker);
        Mat enhancedMarker = WhiteboardProcessor::smoothMarkerTransition(whiteWhiteboard);

        /////////////////////////////////////////////////////////////
        //identify where motion is
        Mat diffHulls = WhiteboardProcessor::expandDifferencesRegion(filteredDiffs);
        Mat diffHullsFullSize = WhiteboardProcessor::enlarge(diffHulls);

        ///////////////////////////////////////////////////////////////////////

        // Get what the whiteboard currently looks like
        Mat currentWhiteboardModel = WhiteboardProcessor::updateWhiteboardModel(oldRefinedBackground, enhancedMarker, diffHullsFullSize);
        // Get what the marker currently looks like
        Mat newMarkerModel = WhiteboardProcessor::updateWhiteboardModel(oldMarkerModel, currentMarker, diffHullsFullSize);
        //////////////////////////////////////////////////

        //figure out if saves need to be made

        // Get a percentage for how much the marker model changed
        float saveNumDif = WhiteboardProcessor::findMarkerModelDiffs(oldMarkerModel, newMarkerModel);
        if (saveNumDif>.004){
//            saveImageWithTimestamp(oldRefinedBackground);
            displayMat(oldRefinedBackground, *ui->imDisplay2);
        }
        //copy last clean whiteboard image
        oldRefinedBackground = currentWhiteboardModel.clone();
        oldMarkerModel = newMarkerModel.clone();
    }
    oldFrame = currentFrame.clone();
}

void MainWindow::on_camera_clicked()
{
    // Stop processing
    runCam=false;
    // Clear the old scanner object
    delete scanner;

    try {
        // Initialize scanner
        scanner = new WebcamImageScanner(promptWebcamNumber());

        // Initialize the current frame and whiteboard processing models
        takePicture();
        oldFrame = currentFrame.clone();
        oldRefinedBackground = Mat::zeros(oldFrame.size(), oldFrame.type());
        oldMarkerModel = Mat::zeros(oldFrame.size(), oldFrame.type());

        // Start processing whiteboard
        runCam = true;
        pause = false;
    }
    catch(std::invalid_argument e) {
        qWarning("%s", e.what());
        qWarning("Failed to initialize webcam. Please choose another webcam or data set.");
        // NOTE: Not including the below line causes a segmentation fault the next time
        // scanner gets deleted. Are there potential memory leaks with this solution?
        scanner = NULL;
    }
}

void MainWindow::on_loadDataSet_clicked()
{
    // Stop processing
    runCam=false;
    // Clear the old scanner object
    delete scanner;

    try {
        // Initialize scanner
        scanner = new DatasetImageScanner(promptFirstDataSetImage());

        // Initialize the current frame and whiteboard processing models
        takePicture();
        oldFrame = currentFrame.clone();
        oldRefinedBackground = Mat::zeros(oldFrame.size(), oldFrame.type());
        oldMarkerModel = Mat::zeros(oldFrame.size(), oldFrame.type());

        // Start processing whiteboard
        runCam = true;
        pause = false;
    }
    catch(std::invalid_argument e) {
        qWarning("%s", e.what());
        qWarning("Failed to initialize data set. Please choose another webcam or data set.");
        // NOTE: Not including the below line causes a segmentation fault the next time
        // scanner gets deleted. Are there potential memory leaks with this solution?
        scanner = NULL;
    }
}

void MainWindow::on_pause_clicked()
{
    pause=!pause;
}

// Opens a dialog box to let the user choose the first image in a data set
string MainWindow::promptFirstDataSetImage() {
    // tr is a method from QObject which facilitates localization
    QString s = QFileDialog::getOpenFileName(this, tr("Open First Image of Sequence"),".",
                                        tr("Image Files (*.png *.bmp *.jpg *.JPG)"));
    return s.toStdString();
}

// Opens an input box to let the user choose the camera. Returns -1 if the user did not press "OK".
int MainWindow::promptWebcamNumber() {
    bool okPressed;
    // tr is a method from QObject which facilitates localization
    int webcamNum = QInputDialog::getInt(this, tr("Webcam"), tr("Enter webcam device number:"), 0, 0, 100, 1, &okPressed);
    // Return the entered number if OK was pressed, otherwise return -1
    return (okPressed ? webcamNum : -1);
}

QImage MainWindow::convertMatToQImage(const Mat& mat) {
    Mat display;
    //copy mask Mat to display Mat and convert from BGR to RGB format
    cvtColor(mat,display,CV_BGR2RGB);

    //convert Mat to QImage
    QImage img=QImage((const unsigned char*)(display.data),display.cols,display.rows,display.step,QImage::Format_RGB888)
            .copy();
    return img;
}

void MainWindow::displayMat(const Mat& mat, QLabel &location) {
    //call method to convert Mat to QImage
    QImage img=convertMatToQImage(mat);
    //push image to display location "location"
    location.setPixmap(QPixmap::fromImage(img));
}

void MainWindow::saveWhiteboardImage(const Mat &frame, int& frameTime, int& deviceNum) {
    char savePath[256];
    sprintf(savePath, "/home/paol/shared/out/whiteBoard%d-%d.png", frameTime, deviceNum);
    imwrite(savePath, frame);
}
