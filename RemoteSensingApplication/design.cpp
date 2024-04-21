#include "design.h"
#include "ui_design.h"

Design::Design(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::Design)
{
    ui->setupUi(this);
}

Design::~Design()
{
    delete ui;
}
