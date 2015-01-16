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
    return scanner->getNextImage(currentFrame);
}

void MainWindow::processImage() {

    //compare picture to previous picture and store differences in allDiffs
    float numDif;
    Mat allDiffs;
    WhiteboardProcessor::findAllDiffsMini(allDiffs, numDif, oldFrame, currentFrame, 40, 1);

    // If there is a large enough difference, reset the stable whiteboard image count and do further processing
    if(numDif > .01) {
        // Reset stable whiteboard image count
        stableWhiteboardCount = 0;
        // Find true differences (ie. difference pixels with enough differences surrounding them)
        float refinedNumDif;
        Mat filteredDiffs;
        WhiteboardProcessor::filterNoisyDiffs(filteredDiffs, refinedNumDif, allDiffs);

        // Find if there are enough true differences to update the current marker and whiteboard models
        // (ie. professor movement or lighting change detected)
        if(refinedNumDif > .01) {
            // Identify where the motion (ie. the professor) is
            Mat movement = WhiteboardProcessor::expandDifferencesRegion(filteredDiffs);
            // Rescale movement info to full size
            Mat mvmtFullSize = WhiteboardProcessor::enlarge(movement);

            // Get the marker model of the current frame
            Mat currentMarkerWithProf = WhiteboardProcessor::findMarkerWithCC(currentFrame);
            // Use the movement information to erase the professor
            Mat currentMarkerModel = WhiteboardProcessor::updateModel(
                        oldMarkerModel, currentMarkerWithProf, mvmtFullSize);

            // Find how much the current marker model differs from the stored one
            float markerDiffs = WhiteboardProcessor::findMarkerModelDiffs(oldMarkerModel, currentMarkerModel);
            // Save and update the models if the marker content changed enough
            if(markerDiffs > .004) {
                // Save the smooth marker version of the old background image
                Mat oldRefinedBackgroundSmooth = WhiteboardProcessor::smoothMarkerTransition(oldRefinedBackground);
                saveImageWithTimestamp(oldRefinedBackgroundSmooth);
                // Update marker model
                oldMarkerModel = currentMarkerModel.clone();
                // Update enhanced version of background
                Mat whiteWhiteboard = WhiteboardProcessor::whitenWhiteboard(currentFrame, currentMarkerModel);
                oldRefinedBackground = WhiteboardProcessor::updateModel(
                            oldRefinedBackground, whiteWhiteboard, mvmtFullSize);
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
            Mat oldRefinedBackgroundSmooth = WhiteboardProcessor::smoothMarkerTransition(oldRefinedBackground);
            saveImageWithTimestamp(oldRefinedBackgroundSmooth);
            // Update marker model
            Mat currentMarkerModel = WhiteboardProcessor::findMarkerWithCC(currentFrame);
            oldMarkerModel = currentMarkerModel.clone();
            // Update enhanced version of background
            Mat whiteWhiteboard = WhiteboardProcessor::whitenWhiteboard(currentFrame, currentMarkerModel);
            oldRefinedBackground = whiteWhiteboard.clone();
        }
    }
    // Update the old frame
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

void MainWindow::saveImageWithTimestamp(const Mat &frame) {
    // Don't actually save the image, just display what would have been saved
    displayMat(frame, *ui->imDisplay2);
}
