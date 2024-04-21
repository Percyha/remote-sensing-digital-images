#ifndef WAIT_H
#define WAIT_H

#include <QMainWindow>

namespace Ui {
class Wait;
}

class Wait : public QMainWindow
{
    Q_OBJECT

public:
    explicit Wait(QWidget *parent = nullptr);
    ~Wait();

private:
    Ui::Wait *ui;
};

#endif // WAIT_H
