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
    void whiteboardProcess();

private slots:
    void on_loadDataSet_clicked();

    void on_camera_clicked();

    void on_pause_clicked();

    void displayFrame();

    void on_save_clicked();

private:
    Ui::MainWindow *ui;
    paolMat *cam;
    paolMat *old;
    paolMat *test;
    paolMat *test2;
    paolMat *con1,*con2;
    paolMat *binary;
    paolMat *cleanImg;
    paolMat *background;
    paolMat *background2;
    paolMat *back2;
    QTimer *qTimer;
    bool run;
    bool camInput;
    float dif;
};

#endif // MAINWINDOW_H
