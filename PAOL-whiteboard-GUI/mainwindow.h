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
    paolMat *modCam;
    paolMat *background;
    paolMat *backgroundRefined;
    paolMat *camClean;
    paolMat *rawEnhanced;

    float numDif;
    float refinedNumDif;
    int count;

    bool runCam;
    bool runData;
    bool pause;
};

#endif // MAINWINDOW_H
