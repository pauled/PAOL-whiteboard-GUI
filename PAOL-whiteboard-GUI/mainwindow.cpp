#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "ImageScanner.h"
#include "webcamimagescanner.h"
#include "datasetimagescanner.h"
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
    // Find difference pixels
    float numDif;
    Mat allDiffs;
    paolMat::findAllDiffsMini(allDiffs, numDif, oldFrame, currentFrame, 40, 1);

    // Temporary Mat to store true differences
    Mat filteredDiffs = Mat::zeros(allDiffs.size(), allDiffs.type());

    //if there is enough of a difference between the two images
    float refinedNumDif = 0;
    if(numDif>.03){
        paolMat::filterNoisyDiffs(filteredDiffs, refinedNumDif, allDiffs);
        count=0;
    }

    //if the images are really identical, count the number of consecultive nearly identical images
    if (numDif < .000001)
        count++;

    //if the differences are enough that we know where the lecturer is or the images have been identical
    //for two frames, and hence no lecturer present
    if(refinedNumDif>.04 || (numDif <.000001 && count==2)){

        /////////////////////////////////////////////////////////////
        // Display the old and current frames being processed
        displayMat(oldFrame, *ui->imDisplay1);
        displayMat(currentFrame, *ui->imDisplay2);

        Mat markerLocation = paolMat::findMarkerWithCC(currentFrame);
//        Mat markerLocation = paolMat::findMarkerWithMarkerBorders(currentFrame);
        Mat darkenedText = paolMat::whitenWhiteboard(currentFrame, markerLocation);
        displayMat(markerLocation, *ui->imDisplay3);

        /////////////////////////////////////////////////////////////
        //identify where motion is

        Mat diffHulls = paolMat::expandDifferencesRegion(filteredDiffs);
        Mat diffHullsFullSize = paolMat::enlarge(diffHulls);
        displayMat(diffHullsFullSize, *ui->imDisplay4);

        ////////////////////////////////////////////////////////////////////////////////

        // Update background image (whiteboard model)
        Mat newWboardModel;
        if(!whiteboardModel.data) {
            // There is no previous whiteboard model, so set it to the enhanced image
            newWboardModel = darkenedText;
        }
        else {
            // Update the existing whiteboard model
            newWboardModel = paolMat::updateWhiteboardModel(whiteboardModel, darkenedText, diffHullsFullSize);
        }

        // Copy updated whiteboard model
        whiteboardModel = newWboardModel.clone();
        displayMat(newWboardModel, *ui->imDisplay5);

        //////////////////////////////////////////////////////////

        // Rectify the model
        Mat rectified = paolMat::rectifyImage(newWboardModel);
        displayMat(rectified, *ui->imDisplay6);

        // Save to disk
        char imageName[256];
        sprintf(imageName, "/home/paol/shared/out/whiteBoard%d-%d.png", lastProcessedFrameTime, deviceNum);
        imwrite(string(imageName), whiteboardModel);

        //////////////////////////////////////////////////

        // TODO: figure out if saves need to be made based on detected marker
        // Strategy:
        // Store a marker model
        // Make updated version of marker model
        // If updated version greatly differs from previous version, save whiteboard model
    }
}

void MainWindow::displayFrame() {
    if(!pause && runCam){
        // Set the previous frame
        oldFrame = currentFrame.clone();

        // Take a picture if the webcam is running
        if(runCam){
            if(!scanner->getNextImage(currentFrame, lastProcessedFrameTime, deviceNum)) {
                qWarning("Stopped processing. Please choose a new file or restart the camera.");
                runCam = false;
            }
        }

        processWhiteboard();
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

        // Initialize whiteboard frames
        scanner->getNextImage(currentFrame, lastProcessedFrameTime, deviceNum);
        whiteboardModel = Mat();
        oldFrame = Mat();

        // Start processing whiteboard
        runCam = true;
        pause = false;
    }
    catch(std::invalid_argument e) {
        qWarning(e.what());
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

        // Initialize whiteboard frames
        scanner->getNextImage(currentFrame, lastProcessedFrameTime, deviceNum);
        whiteboardModel = Mat();
        oldFrame = Mat();

        // Start processing whiteboard
        runCam = true;
        pause = false;
    }
    catch(std::invalid_argument e) {
        qWarning(e.what());
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

QImage MainWindow::convertMatToQImage(const Mat& mask) {
    Mat display;
    //copy mask Mat to display Mat and convert from BGR to RGB format
    cvtColor(mask,display,CV_BGR2RGB);

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
