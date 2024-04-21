#include "dialog_wait.h"
#include "ui_dialog_wait.h"

Dialog_Wait::Dialog_Wait(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::Dialog_Wait)
{
    ui->setupUi(this);
}

Dialog_Wait::~Dialog_Wait()
{
    delete ui;
}

void Dialog_Wait::setwait()
{
    ui->label->setText("Wait...");
}
