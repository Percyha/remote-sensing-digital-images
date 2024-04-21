#include "progressing.h"
#include "ui_progressing.h"

Progressing::Progressing(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::Progressing)
{
    ui->setupUi(this);
    ui->label->setText("WAIT");
}

Progressing::~Progressing()
{
    delete ui;
}
