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
    wbProcessor = new WhiteboardProcessor(true);

    qTimer = new QTimer(this);
    connect(qTimer, SIGNAL(timeout()), this, SLOT(displayFrame()));
    qTimer->start(100);
}

MainWindow::~MainWindow()
{
    delete ui;
    delete qTimer;
    delete scanner;
    delete wbProcessor;
}

void MainWindow::displayFrame() {
    if(!pause && runCam){
        Mat currentFrame;
        int currentFrameTime;
        int deviceNum;

        // Take a picture
        bool gotImage = scanner->getNextImage(currentFrame, currentFrameTime, deviceNum);
        if(gotImage) {
            // Time how long it takes to process a frame
            Clock clock;
            Mat wboardModel = wbProcessor->processCurFrame(currentFrame);
            qDebug("Processed whiteboard in %ld ms", clock.getElapsedTime());
            // Save the new whiteboard image to file if one was produced
            if(wboardModel.data) {
                vector<Mat> debugFrames = wbProcessor->getDebugFrames();
                displayMat(debugFrames[0], *ui->imDisplay1);
                displayMat(debugFrames[1], *ui->imDisplay2);
                displayMat(debugFrames[2], *ui->imDisplay3);
                displayMat(debugFrames[3], *ui->imDisplay4);
                displayMat(debugFrames[4], *ui->imDisplay5);
                displayMat(debugFrames[5], *ui->imDisplay6);
//                saveWhiteboardImage(wboardModel, currentFrameTime, deviceNum);
            }
        }
        else {
            // We couldn't get the next picture, so stop processing
            qWarning("Stopped processing. Please choose a new file or restart the camera.");
            runCam = false;
        }
    }
}

void MainWindow::on_camera_clicked()
{
    // Stop processing
    runCam=false;
    // Reset the whiteboard processor
    wbProcessor->reset();
    // Clear the old scanner object
    delete scanner;

    try {
        // Initialize scanner
        scanner = new WebcamImageScanner(promptWebcamNumber());
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
    // Reset the whiteboard processor
    wbProcessor->reset();
    // Clear the old scanner object
    delete scanner;

    try {
        // Initialize scanner
        scanner = new DatasetImageScanner(promptFirstDataSetImage());
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
