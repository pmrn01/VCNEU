#include <QPainter>
#include <QMouseEvent>
#include <QImageReader>
#include <QDebug>
#include <algorithm>
#include "canvas.h"

Canvas::Canvas(QWidget *parent)
    : QFrame(parent)
{
    setFrameStyle(QFrame::Box);
    setMouseTracking(true);

    op = NONE;
    threshold = 128;
    buffer = nullptr;
}

Canvas::~Canvas()
{
    delete buffer;
}

QSize Canvas::minimumSizeHint() const
{
    return QSize(200, 200);
}

QSize Canvas::sizeHint() const
{
    return QSize(640, 480);
}

void Canvas::setData(int thresh, QString f)
{
    threshold = thresh;
    file = f;

    doOperation();
}

void Canvas::setMode(int m)
{
    op = (Canvas::OperationMode)m;
}

void Canvas::filter(const QImage &img, QImage &buf, int x, int y)
{
    int w = img.width(), h = img.height();
    QColor col = img.pixelColor(x, y);
    int r = col.red(), g = col.green(), b = col.blue();

    switch (op) {
    case NONE: {
        buf.setPixelColor(x, y, col);
    } break;
    case CHANNEL: {
        // Remove blue channel
        col.setBlue(0);
        buf.setPixelColor(x, y, col);
    } break;
    case GREY: {
        // Convert to grayscale using the Y component of YIQ
        int gray = qGray(col.rgb());
        col.setRgb(gray, gray, gray);
        buf.setPixelColor(x, y, col);
    } break;
    case BINARY: {
        // Binarize the image
        int gray = qGray(col.rgb());
        if (gray > threshold) {
            buf.setPixelColor(x, y, Qt::white);
        } else {
            buf.setPixelColor(x, y, Qt::black);
        }
    } break;
    case BLUR: {
        // Apply Gaussian blur
        int kernel[3][3] = {{1, 2, 1}, {2, 4, 2}, {1, 2, 1}};
        int sumR = 0, sumG = 0, sumB = 0, sumKernel = 16;
        for (int ky = -1; ky <= 1; ++ky) {
            for (int kx = -1; kx <= 1; ++kx) {
                QColor neighbor = img.pixelColor(x + kx, y + ky);
                int weight = kernel[ky + 1][kx + 1];
                sumR += neighbor.red() * weight;
                sumG += neighbor.green() * weight;
                sumB += neighbor.blue() * weight;
            }
        }
        buf.setPixel(x, y, qRgb(sumR / sumKernel, sumG / sumKernel, sumB / sumKernel));
    } break;
    case EDGE: {
        // Apply Laplace edge detection
        int kernel[3][3] = {{0, 1, 0}, {1, -4, 1}, {0, 1, 0}};
        int sumR = 0, sumG = 0, sumB = 0;
        for (int ky = -1; ky <= 1; ++ky) {
            for (int kx = -1; kx <= 1; ++kx) {
                QColor neighbor = img.pixelColor(x + kx, y + ky);
                int weight = kernel[ky + 1][kx + 1];
                sumR += neighbor.red() * weight;
                sumG += neighbor.green() * weight;
                sumB += neighbor.blue() * weight;
            }
        }
        buf.setPixel(x, y, qRgb(std::clamp(sumR, 0, 255), std::clamp(sumG, 0, 255), std::clamp(sumB, 0, 255)));
    } break;
    case HISTO: {
        // Convert to grayscale and update histogram
        int gray = qGray(col.rgb());
        histogram[gray]++;
        col.setRgb(gray, gray, gray);
        buf.setPixelColor(x, y, col);
    } break;
    default:
        break;
    }
}

void Canvas::doOperation()
{
    QImageReader reader(file);
    QImage img;
    bool ok = reader.read(&img);

    if (!ok) {
        qDebug() << "Error loading file" << file;
        return;
    }

    int w = img.width(), h = img.height();
    qDebug() << "w =" << w << "h =" << h;

    delete buffer;
    buffer = new QImage(w, h, QImage::Format_RGB32);

    if (op == HISTO) {
        std::memset(histogram, 0, sizeof(histogram));
    }

    for (int y = 0; y < h; y++) {
        for (int x = 0; x < w; x++) {
            filter(img, *buffer, x, y);
        }
    }

    if (op == HISTO) {
        // Histogram equalization for contrast stretching
        int minGray = 255, maxGray = 0;
        for (int i = 0; i < 256; ++i) {
            if (histogram[i] > 0) {
                if (i < minGray) minGray = i;
                if (i > maxGray) maxGray = i;
            }
        }

        for (int y = 0; y < h; ++y) {
            for (int x = 0; x < w; ++x) {
                int gray = qGray(buffer->pixel(x, y));
                int stretchedGray = 255 * (gray - minGray) / (maxGray - minGray);
                buffer->setPixel(x, y, qRgb(stretchedGray, stretchedGray, stretchedGray));
            }
        }
    }
}

void Canvas::paintEvent(QPaintEvent *event)
{
    QFrame::paintEvent(event);

    QPainter painter(this);
    int cw = width(), ch = height();

    // white background
    painter.fillRect(QRect(1, 1, cw - 2, ch - 2), Qt::white);
    painter.setPen(Qt::gray);

    if (buffer) {
        int s = (cw < ch ? cw : ch) - 2;
        QImage out(buffer->scaled(s, s,
                                  Qt::KeepAspectRatioByExpanding,
                                  Qt::SmoothTransformation));
        QRect rect(1, 1, out.width(), out.height());
        painter.drawImage(rect, out);
    }
}

void Canvas::resizeEvent(QResizeEvent *event)
{
    QFrame::resizeEvent(event);
}

void Canvas::mouseReleaseEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        //QPoint currPos = event->pos();
        //update();
    }
}
