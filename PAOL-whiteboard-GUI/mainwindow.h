#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTimer>
#include <QInputDialog>
#include "paolMat.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();
    void processWhiteboard();

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
    bool runData;
    bool pause;

    // Object to read from data set or webcam
    paolMat *cam;

    // Count for storing how many consecutive frames are similar
    int count;

    // Objects to store the frames to process, as well as what the whiteboard looks like
    Mat currentFrame;
    Mat oldFrame;
    Mat whiteboardModel;

    // Methods to request user input
    QString promptFirstDataSetImage();
    int promptWebcamNumber();

    // Methods to convert and display Mats to the GUI
    QImage convertMatToQImage(const Mat& mask);
    void displayMat(const Mat& mat, QLabel &location);
};

#endif // MAINWINDOW_H
