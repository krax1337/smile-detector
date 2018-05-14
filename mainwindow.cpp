#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "pieview.h"
#include "setting_dialog.h"


#include <QMainWindow>
#include <QMessageBox>
#include <QTimer>
#include <string>
#include <QString>
#include <vector>
#include <opencv2/opencv.hpp>

using namespace cv;
using namespace std;

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    settings = new QSettings("setting_for_smile.ini",QSettings::IniFormat, this);
    if(settings->value("program/isLaunchedFirst").toBool() == false)
    {
        QMessageBox::information(this,"Информация",
                                 "Вы запустили программу в первый раз, проверьте настройки!");
        settings->setValue("program/cap_face_freq", 3000);
        settings->setValue("program/chart_freq", 5);
        settings->setValue("program/cam_choice", 0);
        settings->setValue("program/face_cascade",
                           "cpu_cascades/haarcascade_frontalface_alt.xml");
        settings->setValue("program/smile_cascade",
                           "cpu_cascades/haarcascade_smile.xml");
        setting_dialog* wnd = new setting_dialog(this);
        settings->setValue("program/isLaunchedFirst", true);

        wnd->show();
    }


    capture.open(settings->value("program/cam_choice").toInt());
    if(!capture.isOpened())
        QMessageBox::critical(this,"ERROR","ERROR: Could not open webcam");

    ui->widget->addGraph();
    ui->widget->xAxis->setLabel("Время, 1ед это установленный вами шаг");
    ui->widget->yAxis->setLabel("Улыбки");

    faceCascadeName = (settings->value("program/face_cascade").toString()).toStdString();
    smileCascadeName = (settings->value("program/smile_cascade").toString()).toStdString();

    if (!faceCascade.load(faceCascadeName))
        QMessageBox::critical(this,"ERROR","ERROR: Could not load face cascade");

    if (!smileCascade.load(smileCascadeName))
        QMessageBox::critical(this,"ERROR","ERROR: Could not load smile cascade");


    tmrTimer = new QTimer(this);
    tmrTimer2 = new QTimer(this);
    tmrTimer3 = new QTimer(this);

    setupModel();
    setupViews();

    connect(tmrTimer,SIGNAL(timeout()),this,SLOT(onlyCam()));
    connect(tmrTimer2,SIGNAL(timeout()),this,SLOT(proccesFrameUpdGui()));
    connect(tmrTimer3,SIGNAL(timeout()),this,SLOT(set_model_faces()));


}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::onlyCam()
{
    capture >> frame;

    Size raz(ui->label_original->width(),ui->label_original->height());
    cv::resize(frame,frame,raz,0,0,INTER_LINEAR);

    if(frame.empty())
        return;

    cv::cvtColor(frame, frame, CV_BGR2RGB);
    QImage qFrame((uchar*)frame.data, frame.cols, frame.rows, frame.step, QImage::Format_RGB888);

    ui->label_original->setPixmap(QPixmap::fromImage(qFrame));
}


void MainWindow::proccesFrameUpdGui()
{
    Mat frame_with_detect(frame);

    frame_with_detect = detectAndDraw_image(frame_with_detect, faceCascade, smileCascade, scale);

    Size raz_frame_with_detect(ui->label_faces->width(),ui->label_faces->height());
    cv::resize(frame_with_detect,frame_with_detect,raz_frame_with_detect,0,0,INTER_LINEAR);
    QImage qframe_with_detect((uchar*)frame_with_detect.data,
                              frame_with_detect.cols, frame_with_detect.rows,
                              frame_with_detect.step, QImage::Format_RGB888);

    ui->label_faces->setPixmap(QPixmap::fromImage(qframe_with_detect));

    double h = 1;
    if(time_line > 4)
    {
        a++;
        b++;
    }
    else
    {
        a = 0;
        b = 5;
    }
    if(faces_size_all >= faces_size_all_max)
        faces_size_all_max = faces_size_all;


    QVector<double> x(b), y(b);

    for(int i = 0; i < time_line; i++)
    {
        x[i] = i;
        y[i] = smiles[i];
    }
    time_line++;

    ui->widget->graph(0)->setData(x, y);
    ui->widget->xAxis->setRange(a, b);//Для оси Ox
    ui->widget->yAxis->setRange(-1, faces_size_all_max);//Для оси Oy
    ui->widget->replot();
}

void MainWindow::on_Start_clicked()
{
    if(!(tmrTimer->isActive()))
    {
        tmrTimer->start(value_timers);
        tmrTimer2->start(settings->value("program/cap_face_freq").toInt());
        tmrTimer3->start(settings->value("program/chart_freq").toInt()*60000);

        ui->Start->setText("Пауза");
    }
    else if(tmrTimer->isActive())
    {
        tmrTimer->stop();
        tmrTimer2->stop();
        tmrTimer3->stop();

        ui->Start->setText("Продолжить");
    }
    else
    {
        tmrTimer->start(value_timers);
        tmrTimer2->start(settings->value("program/cap_face_freq").toInt());
        tmrTimer3->start(settings->value("program/chart_freq").toInt()*60000);

        ui->Start->setText("Пауза");
    }
}





cv::Mat MainWindow::detectAndDraw_image(cv::Mat& img, cv::CascadeClassifier& cascade,
    cv::CascadeClassifier nestedCascade,
    double& scale)
{
    vector<Rect> faces;
    int face_smile = 0;
    const static Scalar colors[] =
    {
        Scalar(255,0,0),
        Scalar(255,128,0),
        Scalar(255,255,0),
        Scalar(0,255,0),
        Scalar(0,128,255),
        Scalar(0,255,255),
        Scalar(0,0,255),
        Scalar(255,0,255)
    };
    Mat gray, smallImg;

    cv::cvtColor(img, gray, COLOR_BGR2GRAY);

    double fx = 1 / scale;
    cv::resize(gray, smallImg, Size(), fx, fx, INTER_LINEAR_EXACT);
    cv::equalizeHist(smallImg, smallImg);

    cascade.detectMultiScale(smallImg, faces,
        1.1, 2, 0
        //|CASCADE_FIND_BIGGEST_OBJECT
        //|CASCADE_DO_ROUGH_SEARCH
        | CASCADE_SCALE_IMAGE,
        Size(30, 30));


    for (size_t i = 0; i < faces.size(); i++)
    {
        Rect r = faces[i];
        Mat smallImgROI;
        vector<Rect> nestedObjects;
        Point center;
        Scalar color = colors[i % 8];
        int radius;

        double aspect_ratio = (double)r.width / r.height;
        if( 0.75 < aspect_ratio && aspect_ratio < 1.3 )
        {
            center.x = cvRound((r.x + r.width*0.5)*scale);
            center.y = cvRound((r.y + r.height*0.5)*scale);
            radius = cvRound((r.width + r.height)*0.25*scale);
            circle( img, center, radius, color, 1, 8, 0 );
        }
        else
            rectangle( img, cvPoint(cvRound(r.x*scale), cvRound(r.y*scale)),
                       cvPoint(cvRound((r.x + r.width-1)*scale), cvRound((r.y + r.height-1)*scale)),
                       color, 3, 8, 0);

        smallImgROI = smallImg(r);

        nestedCascade.detectMultiScale( smallImgROI, nestedObjects,
            1.6, 0, 0
            //|CASCADE_FIND_BIGGEST_OBJECT
            //|CASCADE_DO_ROUGH_SEARCH
            //|CASCADE_DO_CANNY_PRUNING
            |CASCADE_SCALE_IMAGE,
            Size(30, 30));

        const int smile_neighbors = (int)nestedObjects.size();
        static int max_neighbors = -1;
        static int min_neighbors = -1;
        if (min_neighbors == -1) min_neighbors = smile_neighbors;
        max_neighbors = MAX(max_neighbors, smile_neighbors);

        float intensityZeroOne = ((float)smile_neighbors - min_neighbors) / (max_neighbors - min_neighbors + 1);

        CvPoint pt = cvPoint( cvRound(r.x)-30, cvRound(r.y));

        if (intensityZeroOne > 0.6)
        {
            CvFont font;
            cvInitFont(&font, CV_FONT_HERSHEY_COMPLEX, 1.0, 1.0, 0, 1, CV_AA);
            putText(img, "Smile!", pt, CV_FONT_HERSHEY_COMPLEX, 1.1,
                Scalar::all(255), 2, 8);
            face_smile++;
            smile_all++;
         }
        else
        {
            CvFont font;
            cvInitFont(&font, CV_FONT_HERSHEY_COMPLEX, 1.0, 1.0, 0, 1, CV_AA);
            putText(img, "Neutral!", pt, CV_FONT_HERSHEY_COMPLEX, 1.1,
                Scalar::all(255), 2, 8);
            other_all++;
        }
        pt = NULL;
    }

    faces_size_all = faces.size();
    smiles.push_back(face_smile);

    return img;
}


void MainWindow::on_pushButton_clicked()
{
    setting_dialog* wnd = new setting_dialog(this);
    wnd->show();
    wnd->exec();
}


void MainWindow::setupModel()
{
    model = new QStandardItemModel(2, 2, this);

    model->setHeaderData(0, Qt::Horizontal, tr("Эмоция"));
    model->setHeaderData(1, Qt::Horizontal, tr("Значение"));
    model->setData(model->index(1, 0, QModelIndex()),
                   "Другая");
    model->setData(model->index(0, 0, QModelIndex()),
                   "Улыбка");
    model->setData(model->index(0, 0, QModelIndex()),
                   QColor(Qt::GlobalColor::blue), Qt::DecorationRole);
    model->setData(model->index(1, 0, QModelIndex()),
                   QColor(Qt::GlobalColor::red), Qt::DecorationRole);
}

void MainWindow::setupViews()
{
    splitter = new QSplitter;
    table = new QTableView;

    pieChart = new PieView(this);
    splitter->addWidget(table);
    splitter->addWidget(pieChart);
    splitter->setStretchFactor(0, 0);
    splitter->setStretchFactor(1, 1);

    table->setModel(model);
    pieChart->setModel(model);

    QItemSelectionModel *selectionModel = new QItemSelectionModel(model);
    table->setSelectionModel(selectionModel);
    pieChart->setSelectionModel(selectionModel);

    QHeaderView *headerView = table->horizontalHeader();
    headerView->setStretchLastSection(true);

    ui->tab_2->deleteLater();
    ui->tab->setWindowTitle("Главная");
    ui->tabWidget->addTab(splitter,"Статистика");
}


void MainWindow::set_model_faces()
{

    double smile_perc = (smile_all * 100)/(smile_all + other_all);
    double other_perc = (other_all  * 100)/(smile_all + other_all);

    model->setData(model->index(0, 1, QModelIndex()),
                   smile_perc);
    model->setData(model->index(1, 1, QModelIndex()),
                   other_perc);

    table->setModel(model);
    pieChart->setModel(model);

    smile_all = 0;
    other_all = 0;
}
