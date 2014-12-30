#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTimer>
#include <QInputDialog>
#include "paolMat.h"
#include "ImageScanner.h"
#include "webcamimagescanner.h"

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
    void displayFrame();

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

    // Object to process whiteboard
    paolMat* wbProcessor;

    // Methods to request user input
    string promptFirstDataSetImage();
    int promptWebcamNumber();

    // Methods to convert and display Mats to the GUI
    QImage convertMatToQImage(const Mat& mat);
    void displayMat(const Mat& mat, QLabel &location);

    // Method to save images
    void saveWhiteboardImage(const Mat& frame, int &frameTime, int &deviceNum);
};

#endif // MAINWINDOW_H
