#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTimer>
#include <QInputDialog>
#include <QFileDialog>
#include <QLabel>
#include "PAOLProcUtils.h"
#include "ImageScanner.h"
#include "ImageProcessor.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private slots:
    void workOnNextImage();

    void on_camera_clicked();

    void on_loadDataSet_clicked();

    void on_pause_clicked();

private:
    // Objects to run the GUI
    Ui::MainWindow *ui;
    QTimer *qTimer;

    // Flags to run the GUI
    bool runCam;
    bool pause;

    // Object to read from data set or webcam
    ImageScanner* scanner;

    // Fields for general processing (mostly saving the image)
    int saveImageCount;
    char saveImagePathFormat[256];
    time_t currentImageTime;
    int capturedImageCount; // How many images were captured from the camera

    // Fields for whiteboard processing
    Mat currentFrame;
    Mat oldFrame;
    Mat oldMarkerModel;
    Mat oldRefinedBackground; // What the whiteboard from the oldFrame looks like
    int stableWhiteboardCount;

    // Methods to request user input
    string promptFirstDataSetImage();
    int promptWebcamNumber();

    // Methods to convert and display Mats to the GUI
    QImage convertMatToQImage(const Mat& mat);
    void displayMat(const Mat& mat, QLabel &location);

    // Method to save images
    void saveImageWithTimestamp(const Mat& frame);

    // Methods for processing whiteboard
    bool takePicture();
    void processImage();
};

#endif // MAINWINDOW_H
