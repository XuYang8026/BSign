#ifndef SUPPLEMENTSIGN_H
#define SUPPLEMENTSIGN_H

#include <QDialog>

namespace Ui {
class SupplementSign;
}

class SupplementSign : public QDialog
{
    Q_OBJECT

public:
    explicit SupplementSign(QWidget *parent = nullptr);
    ~SupplementSign();

private:
    Ui::SupplementSign *ui;
};

#endif // SUPPLEMENTSIGN_H
