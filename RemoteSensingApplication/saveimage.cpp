#include "saveimage.h"
#include "ui_saveimage.h"

SaveImage::SaveImage(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::SaveImage)
{
    ui->setupUi(this);
}

SaveImage::~SaveImage()
{
    delete ui;
}
