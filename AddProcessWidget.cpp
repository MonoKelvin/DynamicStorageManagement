#include "AddProcessWidget.h"
#include "ui_addprocesswidget.h"

#include <QSpinBox>

AddProcessWidget::AddProcessWidget(QWidget* parent) :
    QDialog(parent),
    ui(new Ui::AddProcessWidget)
{
    ui->setupUi(this);

    connect(ui->le_inputName, &QLineEdit::textChanged, [ = ](const QString & str)
    {
        processName = str;
        if (processName.isEmpty()) {
            ui->btn_done->setDisabled(true);
        } else {
            ui->btn_done->setEnabled(true);
        }
    });

    connect(ui->btn_done, &QPushButton::clicked, [ = ] {
        this->accept();
    });

    ui->le_inputName->clear();
    ui->sb_inputSize->setMinimum(64);
    ui->sb_inputSize->setMaximum(8192);
    ui->sb_inputSize->setValue(1024);
    ui->btn_done->setDisabled(true);
}

AddProcessWidget::~AddProcessWidget()
{
    delete ui;
}

void AddProcessWidget::closeEvent(QCloseEvent *)
{
    processName.clear();
    size = -1;
}

void AddProcessWidget::on_sb_inputSize_valueChanged(int value)
{
    size = value;
}
