#ifndef SETTING_DIALOG_H
#define SETTING_DIALOG_H

#include <QDialog>
#include <QSettings>

namespace Ui {
class setting_dialog;
}

class setting_dialog : public QDialog
{
    Q_OBJECT

public:
    explicit setting_dialog(QWidget *parent = 0);
    ~setting_dialog();

private slots:
    void on_lineEdit_textChanged(const QString &arg1);

    void on_pushButton_clicked();

    void on_pushButton_2_clicked();

private:
    Ui::setting_dialog *ui;

    QSettings* settings;
};

#endif // SETTING_DIALOG_H
