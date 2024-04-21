#ifndef DIALOG_WAIT_H
#define DIALOG_WAIT_H

#include <QDialog>

namespace Ui {
class Dialog_Wait;
}

class Dialog_Wait : public QDialog
{
    Q_OBJECT

public:
    explicit Dialog_Wait(QWidget *parent = nullptr);
    ~Dialog_Wait();

private slots:
    void setwait();

private:
    Ui::Dialog_Wait *ui;
};

#endif // DIALOG_WAIT_H
