#include "mainwindow.h"
#include "ui_mainwindow.h"

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
    runData=false;
    pause=false;

    cam=new paolMat();
    old=new paolMat();
    modCam=new paolMat();
    background=new paolMat();
    camClean=new paolMat();
    backgroundRefined=new paolMat();
    rawEnhanced=new paolMat();

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
    //cam->displayImage(*ui->imDisplay1);

    numDif=old->differenceMin(cam,40,1);
    qDebug(" numDif=%f\n",numDif);

    if(numDif>.03){
        refinedNumDif=old->shrinkMaskMin();
        count=0;
        //old->displayMaskMin(*ui->imDisplay2);
    } else {
        refinedNumDif=0;
    }

    if (numDif < .000001)
        count++;

    qDebug(" refinedNumDif=%f  numDif=%f\n",refinedNumDif,numDif);

    if(refinedNumDif>.04 || (numDif <.000001 && count==2)){
        rawEnhanced->copy(cam);
        rawEnhanced->averageWhiteboard(20);
        rawEnhanced->enhanceText();
        //rawEnhanced->displayImage(*ui->imDisplay8);

        old->extendMaskMinToEdges();
        old->sweepDownMin();
        //old->displayMaskMin(*ui->imDisplay3);

        old->keepWhiteMaskMin();
        old->growMin(8);
        //old->displayMaskMin(*ui->imDisplay4);

        old->findContoursMaskMin();
        //old->displayMaskMin(*ui->imDisplay5);

        old->sweepDownMin();
        old->keepWhiteMaskMin();
        //old->displayMaskMin(*ui->imDisplay6);

        modCam->copy(cam);
        modCam->copyMaskMin(old);
        modCam->maskMinToMaskBinary();
        //modCam->displayMaskMin(*ui->imDisplay6);

        cam->blur(1);
        cam->pDrift();
        //cam->displayMask(*ui->imDisplay7);

        cam->grow(15,3);
        //cam->displayMask(*ui->imDisplay9);

        camClean->copy(cam);
        //camClean->nontextToWhite();
        //camClean->displayImage(*ui->imDisplay8);

        camClean->copyMaskMin(old);
        //camClean->displayMask(*ui->imDisplay9);
        //camClean->displayMaskMin(*ui->imDisplay10);
        camClean->maskMinToMaskBinary();

        background->updateBackgroundMaskMin(camClean,rawEnhanced);
        background->updateBack2(camClean,cam,rawEnhanced);
        //background->displayMask(*ui->imDisplay10);
        //background->displayImage(*ui->imDisplay11);

        backgroundRefined->copy(background);
        backgroundRefined->testMethod();
        backgroundRefined->displayImage(*ui->imDisplay10);
        //backgroundRefined->processText(background);
        //backgroundRefined->displayImage(*ui->imDisplay12);
    }
}

void MainWindow::displayFrame() {
    if(!pause && (runCam || runData)){

        old->copy(cam);

        if(runCam){
            cam->takePicture();
        }
        if(runData){
            if(!cam->readNext(this))
                runData=false;
        }

        processWhiteboard();
    }
}


void MainWindow::on_camera_clicked()
{
    runData=false;
    if(cam->src.data){
        cam->~paolMat();
        cam=new paolMat();
    }

    cam->setCameraNum(1);
    cam->takePicture();
    background->copy(cam);
    backgroundRefined->copy(cam);
    runCam=true;
    pause=false;
}

void MainWindow::on_loadDataSet_clicked()
{
    runCam=false;
    runData=false;
    if(cam->src.data){
        cam->~paolMat();
        cam=new paolMat();
    }

    cam->readNext(this);
    background->copy(cam);
    backgroundRefined->copy(cam);
    runData=true;
    pause=false;
}

void MainWindow::on_pause_clicked()
{
    pause=!pause;
}
