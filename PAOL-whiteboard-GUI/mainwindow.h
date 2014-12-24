#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTimer>
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
    void processWhiteboardWithCC();
    void rectifyImage();
    void findLines();

private slots:
    void displayFrame();


    void on_camera_clicked();

    void on_loadDataSet_clicked();

    void on_pause_clicked();

private:
    Ui::MainWindow *ui;
    QTimer *qTimer;

    paolMat *cam;
    paolMat *old;
    paolMat *background;
    paolMat *backgroundRefined;
    paolMat *oldBackgroundRefined;
    paolMat *rawEnhanced;
    paolMat *rectified;
    paolMat *dummyPM;

    float numDif;
    float refinedNumDif;
    int count;
    float saveNumDif;

    bool runCam;
    bool runData;
    bool pause;
};

#endif // MAINWINDOW_H
