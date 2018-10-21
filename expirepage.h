#ifndef EXPIREPAGE_H
#define EXPIREPAGE_H

#include <QWidget>

namespace Ui {
class expirepage;
}

class expirepage : public QWidget
{
    Q_OBJECT

public:
    explicit expirepage(QWidget *parent = 0);
    ~expirepage();

private:
    Ui::expirepage *ui;
};

#endif // EXPIREPAGE_H
