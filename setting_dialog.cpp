#include "setting_dialog.h"
#include "mainwindow.h"
#include "ui_setting_dialog.h"
#include <QApplication>
#include <QProcess>

setting_dialog::setting_dialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::setting_dialog)
{
    ui->setupUi(this);
    settings = new QSettings("setting_for_smile.ini",QSettings::IniFormat, this);


    ui->lineEdit->setText(settings->value("program/face_cascade").toString());
    ui->lineEdit_2->setText(settings->value("program/smile_cascade").toString());
    ui->cap_face_freq->setValue(settings->value("program/cap_face_freq").toInt());
    ui->chart_freq->setValue(settings->value("program/chart_freq").toInt());
    ui->cam_freq->setValue(settings->value("program/cam_choice").toInt());


}

setting_dialog::~setting_dialog()
{
    delete ui;
}

void setting_dialog::on_lineEdit_textChanged(const QString &arg1)
{

}

void setting_dialog::on_pushButton_clicked()
{

    settings->setValue("program/face_cascade",ui->lineEdit->text());
    settings->setValue("program/smile_cascade",ui->lineEdit_2->text());
    settings->setValue("program/cap_face_freq", ui->cap_face_freq->value());
    settings->setValue("program/chart_freq", ui->chart_freq->value());
    settings->setValue("program/cam_choice", ui->cam_freq->value());

    QMessageBox::information(this,"Информация",
                             "Настройки успешно изменены!\nПрограмма будет перезапущена!");

    qApp->quit();
    QProcess::startDetached(qApp->arguments()[0], qApp->arguments());
}

void setting_dialog::on_pushButton_2_clicked()
{
    settings->setValue("program/cap_face_freq", 3000);
    settings->setValue("program/chart_freq", 5);
    settings->setValue("program/cam_choice", 0);
    settings->setValue("program/face_cascade",
                       "cpu_cascades/haarcascade_frontalface_alt.xml");
    settings->setValue("program/smile_cascade",
                       "cpu_cascades/haarcascade_smile.xml");

    ui->lineEdit->setText(settings->value("program/face_cascade").toString());
    ui->lineEdit_2->setText(settings->value("program/smile_cascade").toString());
    ui->cap_face_freq->setValue(settings->value("program/cap_face_freq").toInt());
    ui->chart_freq->setValue(settings->value("program/chart_freq").toInt());
    ui->cam_freq->setValue(settings->value("program/cam_choice").toInt());

    QMessageBox::information(this,"Информация",
                             "Настройки сброшены по умолчанию!\nПрограмма будет перезапущена!");

    qApp->quit();
    QProcess::startDetached(qApp->arguments()[0], qApp->arguments());
}
