#ifndef LOADINGWAIT_H
#define LOADINGWAIT_H

#include <QWidget>
#include <QDialog>
#include <QThread>

namespace Ui {
class LoadingWait;
}

class LoadingWait : public QDialog
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
