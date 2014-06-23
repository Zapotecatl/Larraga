#include "mainwindow.h"
#include "ui_mainwindow.h"

#define STEPS 0

int inicio = 0;

int length = 100; //Utilizado en la visualizacion
float dens = 0.3; //utilizado en visualizacion

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);


    resolution_w = 600;
    resolution_h = 10;

    labelCity = new QLabel(this);
    labelCity->setGeometry(10,50,resolution_w,resolution_h);
    labelCity->setStyleSheet("background-color: rgb(0, 0, 0);");

    t_timer = new QTimer();
    connect(t_timer, SIGNAL(timeout()), this, SLOT(PaintCity()));
    t_timer->start(30);//60

    qi_city =  new QImage (length, 1, QImage::Format_RGB888);

}


void MainWindow::PaintCity()
{

    if (STEPS == 1)
        if (key == 80)
            inicio = 1;
        else
            inicio = 0;

     if (inicio == 1){

        //qi_city->fill(Qt::black);

        qi_city->fill(Qt::red);

        imgpaint = QPixmap::fromImage(*qi_city);
        QPainter paint(&imgpaint);

        size_s.setHeight(resolution_h);
        size_s.setWidth(resolution_w);

        RunSimulation(1);

        PrintStreetCity(&paint, p);
        //img_scaled = imgpaint.scaled(size_s, Qt::KeepAspectRatio);
        img_scaled = imgpaint.scaled(size_s, Qt::IgnoreAspectRatio);
        labelCity->setPixmap(img_scaled);

     }


     key = 0;
}

void MainWindow::PrintStreetCity(QPainter *paint, QPen &p)
{

    int i, j;

       //printf ("---Highway----\n");
       j = 0;
       for (i = 0; i < length; i++)
           if (GetValueCellStreet(i) >= 0){

               struct Color color;

               color = GetColorVehicle(GetValueCellStreet(i));
               p.setColor(QColor(color.r,  color.g,  color.b));
               //p.setColor(QColor(0,  0,  255));
               paint->setPen(p);
               paint->drawPoint(i, 0);
               j++;
           }
           else {

               p.setColor(QColor(0,0,0));
               paint->setPen(p);
               paint->drawPoint(i, 0);
           }

}

MainWindow::~MainWindow()
{
    delete ui;
}

MyLabel::MyLabel(QWidget *parent):QLabel(parent)
{
}

MyLabel::~MyLabel()
{

    if (inicio == 1)
        DestroyingAll();
}


void MainWindow::on_pushInicio_clicked()
{
    inicio = 1;
    InicilizationCity(length, dens);
}

void MainWindow::on_pushExperiment_clicked()
{

    qDebug() << "Start";
    ui->label->setText("Process...");
    mainFunction();
    qDebug() << "End";

    qDebug() << "Check the file: measures. txt";
    ui->label->setText("Check the file: measures. txt");


}

void MainWindow::keyPressEvent(QKeyEvent* event) {
    key = event->key();
}
