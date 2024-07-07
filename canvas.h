#ifndef CANVAS_H
#define CANVAS_H

#include <QFrame>
#include <QImage>

class Canvas : public QFrame
{
    Q_OBJECT

public:
    Canvas(QWidget *parent = nullptr);
    ~Canvas();

    QSize minimumSizeHint() const override;
    QSize sizeHint() const override;

    void setData(int thresh, QString f);
    void setMode(int m);
    enum OperationMode { NONE, CHANNEL, GREY, BINARY, BLUR, EDGE, HISTO };
protected:
    void paintEvent(QPaintEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;

private:
    void filter(const QImage &img, QImage &buf, int x, int y);
    void doOperation();
    void calculateHistogram(const QImage &img);
    void showHistogram();
    QImage *buffer;
    QString file;
    int threshold;
    int histogram[256];
    void drawHistogram(QPainter &painter);


    OperationMode op;
};

#endif // CANVAS_H
