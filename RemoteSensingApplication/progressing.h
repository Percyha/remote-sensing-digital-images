#ifndef PROGRESSING_H
#define PROGRESSING_H

#include <QDialog>

namespace Ui {
class Progressing;
}

class Progressing : public QDialog
{
    Q_OBJECT

public:
    explicit Progressing(QWidget *parent = nullptr);
    ~Progressing();

private:
    Ui::Progressing *ui;
};

#endif // PROGRESSING_H
