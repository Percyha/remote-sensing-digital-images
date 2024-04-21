#ifndef DESIGN_H
#define DESIGN_H

#include <QMainWindow>

namespace Ui {
class Design;
}

class Design : public QMainWindow
{
    Q_OBJECT

public:
    explicit Design(QWidget *parent = nullptr);
    ~Design();

private:
    Ui::Design *ui;
};

#endif // DESIGN_H
