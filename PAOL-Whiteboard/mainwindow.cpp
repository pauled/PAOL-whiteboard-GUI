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

    cam=new paolMat();
    con1=new paolMat();
    con2=new paolMat();
    old=new paolMat();
    cleanImg=new paolMat();
    background=new paolMat();
    background2=new paolMat();
    test=new paolMat();
    test2=new paolMat();
    binary=new paolMat();
    back2=new paolMat();
    cam->setCameraNum(1);

    cam->takePicture();
    back2->copy(cam);

    cam->blur(1);
    cam->pDrift();
    cam->grow(30,3);
    background->copy(cam);
    background2->copy(cam);
    test->copy(cam);
    binary->copy(cam);
    con1->copy(cam);
    con2->copy(cam);
    background->updateBackground();
    background->cleanBackground(cam);
    background->darken();
    //cam->readNext(this);

    run=true;
    camInput=true;
//    cam->readNext(this);

    qTimer = new QTimer(this);
    connect(qTimer, SIGNAL(timeout()), this, SLOT(displayFrame()));
    qTimer->start(100);

}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::displayFrame() {
    if (run){
        old->copy(cam);
        if (camInput){
            cam->takePicture();
            whiteboardProcess();
        } else {
            if (cam->readNext(this)){
                whiteboardProcess();
            } else {
                run=!run;
            }
        }
    }
}

void MainWindow::whiteboardProcess(){
    if(run){
        /*
        con2->copy(cam);
        //cam->displayImage(*ui->imDisplay1);
        dif=old->difference(cam,40,1,0);
        qDebug(" dif=%f\n",dif);

        old->displayMask(*ui->imDisplay2);

        if (dif>.07){
            qDebug("start");
            con1->copy(old);
            qDebug("1");
            con1->shrinkMask2();
qDebug("2");
            con1->sweepDown();
     qDebug("3");
            con1->keepWhite();
            qDebug("4");
            con1->grow(200,8);//(200,20);
            //con1->SHRINK
            //con1->grow(200,5);
            qDebug("5");
            con1->findContoursMask();
            qDebug("6");
            //con1->contoursToMask();
            con1->sweepDown();
            qDebug("7");
            con1->keepWhite();
            qDebug("higher");
            //con1->displayMaskMin(*ui->imDisplay8);
            qDebug("8");
            //if (dif>30000){
            con2->copyMask(con1);
            qDebug("9");
            //con2->maskToMaskBinary();
            qDebug("lower");
            con2->maskToRed();
            qDebug("10");
            con2->displayImage(*ui->imDisplay9);
            qDebug("middle");

            cam->blur(1);
            cam->pDrift();
          cam->displayMask(*ui->imDisplay1);

            cam->grow(30,3);
            cam->displayImage(*ui->imDisplay3);
//cam->displayMask(*ui->imDisplay4);
            background->copy(cam);
            background->updateBackground();
            //background->cleanBackground(back2);
            background->copyMask(con1);
            //background2->copyMask(con1);
            background2->updateBack2Min(background);
            background2->displayImage(*ui->imDisplay5);
            //background->displayMaskMin(*ui->imDisplay5);
            back2->updateBack2(con2);
            qDebug("end");*/











            con2->copy(cam);
            //cam->displayImage(*ui->imDisplay1);
            dif=old->differenceMin(cam,40,1,0);
            qDebug(" dif=%f\n",dif);

            old->displayMaskMin(*ui->imDisplay2);

            if (dif>.07){
                qDebug("start");
                con1->copy(old);
                qDebug("1");
                con1->shrinkMaskMin2();
    qDebug("2");
                con1->sweepDownMin();
         qDebug("3");
                con1->keepWhiteMin();
                qDebug("4");
               con1->growMin(200,8);//(200,20);
                //con1->SHRINK
                //con1->grow(200,5);
                qDebug("5");
                con1->findContoursMaskMin();
                qDebug("6");
                //con1->contoursToMask();
                con1->sweepDownMin();
                qDebug("7");
                con1->keepWhiteMin();
                qDebug("higher");
                //con1->displayMaskMin(*ui->imDisplay8);
                qDebug("8");
                //if (dif>30000){
                con2->copyMask(con1);
                qDebug("9");
                con2->maskMinToMaskBinary();
                qDebug("lower");
                //con2->maskToRed();
                qDebug("10");
                con2->displayImage(*ui->imDisplay9);
                qDebug("middle");

                cam->blur(1);
                cam->pDrift();

                cam->displayMask(*ui->imDisplay1);

                cam->grow(30,3);
                cam->displayImage(*ui->imDisplay3);
    //cam->displayMask(*ui->imDisplay4);
                background->copy(cam);
                background->updateBackground();
                //background->cleanBackground(back2);
                background->copyMask(con1);
                //background2->copyMask(con1);
                background2->updateBack2Min(background);
                background2->displayImage(*ui->imDisplay5);
                //background->displayMaskMin(*ui->imDisplay5);
                back2->updateBack2(con2);
                qDebug("end");
        }
//        back2->displayImage(*ui->imDisplay6);
        //con1->maskToRed();
        //con1->toHSV();
        //    con1->displayMask(*ui->imDisplay8);



        //cam->displayImage(*ui->imDisplay4);
        //cam->displayMask(*ui->imDisplay5);


        //connectB->connectedComponent(edge,true);
        /*    binary->binarize(background);
    //binary->copyInDif(old);
    test->connectedComponent(binary,true);
    test2->copy(test);
    test2->colorConnected(test);
    test2->displayImage(*ui->imDisplay5);
*/
        //background->displayImage(*ui->imDisplay7);
        //background->displayMask(*ui->imDisplay8);

        //background->cleanBackground(back2);
        //background->displayImage(*ui->imDisplay3);
        //background->displayMask(*ui->imDisplay6);
        //qDebug("not here3");
        //background->darken();
        //qDebug("not here2");
        //background->displayImage(*ui->imDisplay9);
    }
}

void MainWindow::on_loadDataSet_clicked()
{
    run=false;
    cam->cameraNum=-1;
    cam->readNext(this);
    run=true;
    camInput=false;
    run=true;
}

void MainWindow::on_camera_clicked()
{
    cam->cameraNum=1;
    camInput=true;
}

void MainWindow::on_pause_clicked()
{
    run=!run;
}



void MainWindow::on_save_clicked()
{
     run=false;
     background2->saveSrc(this);
     run=true;
}
