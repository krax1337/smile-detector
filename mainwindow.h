#ifndef MAINWINDOW_H
#define MAINWINDOW_H



#include <QMainWindow>
#include <QMessageBox>
#include <QTimer>
#include <string>
#include <QString>
#include <QDebug>
#include <QSettings>
#include <vector>
#include <opencv2/core.hpp>
#include <opencv2/opencv.hpp>



QT_BEGIN_NAMESPACE
class QAbstractItemModel;
class QAbstractItemView;
class QItemSelectionModel;
class QSplitter;
class  QTableView;
QT_END_NAMESPACE





namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT


public:

    cv::Mat detectAndDraw_image(cv::Mat& img, cv::CascadeClassifier& cascade,
        cv::CascadeClassifier nestedCascade,
        double& scale);

    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

public slots:
    void proccesFrameUpdGui();
    void onlyCam();
    void set_model_faces();


private slots:
    void on_Start_clicked();
    void on_pushButton_clicked();



private:
    void setupModel();
    void setupViews();
    QSplitter *splitter;
    QTableView *table;
    QAbstractItemModel *model;
    QAbstractItemView *pieChart;
    QItemSelectionModel *selectionModel;

    QSettings* settings;

    int value_timers = 20;
    Ui::MainWindow *ui;
    cv::VideoCapture capture;
    cv::Mat frame;


    int a = 0;
    int b = 0;
    int time_line = 0;
    int faces_size_all = 0;
    int time_lin = 1;
    int faces_size_all_max = 5;

    QImage qFrame;
    double scale = 1;

    cv::CascadeClassifier faceCascade,smileCascade;

    std::string faceCascadeName;
    std::string smileCascadeName;

    QVector<int> smiles;

    long long smile_all = 0;
    long long other_all = 0;

    QTimer* tmrTimer;
    QTimer* tmrTimer3;
    QTimer* tmrTimer2;
};

#endif // MAINWINDOW_H
