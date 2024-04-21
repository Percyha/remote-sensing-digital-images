#ifndef SAVEIMAGE_H
#define SAVEIMAGE_H

#include <QMainWindow>

namespace Ui {
class SaveImage;
}

class SaveImage : public QMainWindow
{
    Q_OBJECT

public:
    explicit SaveImage(QWidget *parent = nullptr);
    ~SaveImage();

private:
    Ui::SaveImage *ui;
};

#endif // SAVEIMAGE_H
