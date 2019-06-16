#ifndef ADDPROCESSWIDGET_H
#define ADDPROCESSWIDGET_H

#include <QDialog>

namespace Ui {
class AddProcessWidget;
}

class PCB;

class AddProcessWidget : public QDialog
{
    Q_OBJECT

public:
    explicit AddProcessWidget(QWidget* parent = nullptr);
    ~AddProcessWidget();

    QString processName;
    int size;

protected:
    void closeEvent(QCloseEvent *);

private slots:
    void on_sb_inputSize_valueChanged(int value);

private:
    Ui::AddProcessWidget* ui;

};

#endif // ADDPROCESSWIDGET_H
