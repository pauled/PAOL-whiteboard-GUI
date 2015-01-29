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

    // Set up whiteboard corners
    corners.TLx = 105;
    corners.TLy = 511;
    corners.TRx = 1021;
    corners.TRy = 539;
    corners.BLx = 146;
    corners.BLy = 910;
    corners.BRx = 999;
    corners.BRy = 916;

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
            displayMat(currentFrame, *ui->imDisplay1);
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
    oldFrame = currentFrame.clone();
    return scanner->getNextImage(currentFrame);
}

void MainWindow::processImage() {
    // If this is the first time processing, initialize WB processing fields and return
    // without further processing
    if(!oldFrame.data) {
        oldRefinedBackground = Mat::zeros(currentFrame.size(), currentFrame.type());
        oldMarkerModel = Mat::zeros(currentFrame.size(), currentFrame.type());
        return;
    }

    // Get rectified versions of old and current frames
    Mat oldRectified = PAOLProcUtils::rectifyImage(oldFrame, corners);
    Mat currentRectified = PAOLProcUtils::rectifyImage(currentFrame, corners);

    //compare picture to previous picture and store differences in allDiffs
    float numDif;
    Mat allDiffs;
    PAOLProcUtils::findAllDiffsMini(allDiffs, numDif, oldRectified, currentRectified, 40, 1);

    // If there is a large enough difference, reset the stable whiteboard image count and do further processing
    if(numDif > .01) {
        // Reset stable whiteboard image count
        stableWhiteboardCount = 0;
        // Find true differences (ie. difference pixels with enough differences surrounding them)
        float refinedNumDif;
        Mat filteredDiffs;
        PAOLProcUtils::filterNoisyDiffs(filteredDiffs, refinedNumDif, allDiffs);

        // Find if there are enough true differences to update the current marker and whiteboard models
        // (ie. professor movement or lighting change detected)
        if(refinedNumDif > .01) {
            // Identify where the motion (ie. the professor) is
            Mat movement = PAOLProcUtils::expandDifferencesRegion(filteredDiffs);
            // Rescale movement info to full size
            Mat mvmtFullSize = PAOLProcUtils::enlarge(movement);

            // Find marker candidates
            Mat markerCandidates = PAOLProcUtils::findMarkerStrokeCandidates(currentRectified);
            // Find marker locations
            Mat markerLocations = PAOLProcUtils::findMarkerStrokeLocations(currentRectified);
            // Keep marker candidates intersecting with marker locations
            Mat currentMarkerWithProf = PAOLProcUtils::filterConnectedComponents(markerCandidates, markerLocations);

            // Use the movement information to erase the professor
            Mat currentMarkerModel = PAOLProcUtils::updateModel(
                        oldMarkerModel, currentMarkerWithProf, mvmtFullSize);

            // Find how much the current marker model differs from the stored one
            float markerDiffs = PAOLProcUtils::findMarkerModelDiffs(oldMarkerModel, currentMarkerModel);
            qDebug("numDif: %f", numDif);
            qDebug("refinedNumDif: %f", refinedNumDif);
            qDebug("markerDiffs: %f", markerDiffs);
            // Save and update the models if the marker content changed enough
            if(markerDiffs > .022) {
                // Save the smooth marker version of the old background image
                Mat oldRefinedBackgroundSmooth = PAOLProcUtils::smoothMarkerTransition(oldRefinedBackground);
                saveImageWithTimestamp(oldRefinedBackgroundSmooth);
                // Update marker model
                oldMarkerModel = currentMarkerModel.clone();
                // Update enhanced version of background
                Mat whiteWhiteboard = PAOLProcUtils::whitenWhiteboard(currentRectified, currentMarkerModel);
                oldRefinedBackground = PAOLProcUtils::updateModel(
                            oldRefinedBackground, whiteWhiteboard, mvmtFullSize);
                displayMat(oldRefinedBackground, *ui->imDisplay2);
            }
        }
    }
    // Otherwise, check if the frames are basically identical (ie. stable)
    else if(numDif < .000001) {
        stableWhiteboardCount++;
        // If the image has been stable for exactly three frames, the lecturer is not present, so we
        // can update the marker and whiteboard models without movement information
        if(stableWhiteboardCount == 3) {
            // Save the smooth marker version of the old background image
            Mat oldRefinedBackgroundSmooth = PAOLProcUtils::smoothMarkerTransition(oldRefinedBackground);
            saveImageWithTimestamp(oldRefinedBackgroundSmooth);

            // Update marker model
            // Find marker candidates
            Mat markerCandidates = PAOLProcUtils::findMarkerStrokeCandidates(currentRectified);
            // Find marker locations
            Mat markerLocations = PAOLProcUtils::findMarkerStrokeLocations(currentRectified);
            // Keep marker candidates intersecting with marker locations
            Mat currentMarkerModel = PAOLProcUtils::filterConnectedComponents(markerCandidates, markerLocations);

            oldMarkerModel = currentMarkerModel.clone();
            // Update enhanced version of background
            Mat whiteWhiteboard = PAOLProcUtils::whitenWhiteboard(currentRectified, currentMarkerModel);
            oldRefinedBackground = whiteWhiteboard.clone();
            displayMat(oldRefinedBackground, *ui->imDisplay2);
        }
    }
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
        scanner->setPrintDebug(true);

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
        scanner->setPrintDebug(true);

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

void MainWindow::saveImageWithTimestamp(const Mat &frame) {
    // Don't actually save the image, just display what would have been saved
    displayMat(frame, *ui->imDisplay2);
}
