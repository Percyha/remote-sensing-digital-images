#include "wait.h"
#include "ui_wait.h"

Wait::Wait(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::Wait)
{
    ui->setupUi(this);
}

Wait::~Wait()
{
    delete ui;
}
