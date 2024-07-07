#ifndef PAINT_H
#define PAINT_H

#include <QWidget>

class Canvas;
class QPushButton;
class QLineEdit;
class QComboBox;
class QLabel;

class Paint : public QWidget
{
    Q_OBJECT

public:
    Paint(QWidget *parent = nullptr);
    ~Paint();

private slots:
    void genBtnPressed();
    void colModeChanged(int ind);

private:
    Canvas *viewport;
    QPushButton *btnGenPoints;
    QLineEdit *leNumPoints;
    QLineEdit *leFileName;
    QComboBox *cobColModes;
    QLabel *lblColModes;
};

#endif // PAINT_H
