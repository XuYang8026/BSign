#ifndef LOADINGWAIT_H
#define LOADINGWAIT_H

#include <QWidget>
#include <QDialog>

namespace Ui {
class LoadingWait;
}

class LoadingWait : public QWidget
{
    Q_OBJECT

public:
    explicit LoadingWait(QWidget *parent = nullptr);
    ~LoadingWait();
    QString content;

private:
    Ui::LoadingWait *ui;
};

#endif // LOADINGWAIT_H
