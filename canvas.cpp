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
    std::fill(histogram, histogram + 256, 0);
    buffer = nullptr;
}

Canvas::~Canvas()
{
    delete buffer;
}

void Canvas::calculateHistogram(const QImage &img) {
    // Gehe durch jedes Pixel im Bild und erhöhe das Histogramm entsprechend
    for (int y = 0; y < img.height(); ++y) {
        for (int x = 0; x < img.width(); ++x) {
            QColor color(img.pixel(x, y));
            int gray = qGray(color.rgb()); // Konvertiere zur Graustufe
            histogram[gray]++;
        }
    }
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

void Canvas::showHistogram() {
    qDebug() << "Histogram:";
    for (int i = 0; i < 256; ++i) {
        qDebug() << i << ": " << histogram[i];
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

        // Calculate histogram and find min/max gray values
        int minGray = 255, maxGray = 0;
        for (int y = 0; y < h; ++y) {
            for (int x = 0; x < w; ++x) {
                QColor col = img.pixelColor(x, y);
                int gray = qGray(col.rgb());
                histogram[gray]++;
                if (gray < minGray) minGray = gray;
                if (gray > maxGray) maxGray = gray;
            }
        }

        // Histogram equalization for contrast stretching
        for (int y = 0; y < h; ++y) {
            for (int x = 0; x < w; ++x) {
                QColor col = img.pixelColor(x, y);
                int gray = qGray(col.rgb());

                // Stretch gray level to full range [0, 255]
                int stretchedGray = 255 * (gray - minGray) / (maxGray - minGray);

                buffer->setPixelColor(x, y, qRgb(stretchedGray, stretchedGray, stretchedGray));
            }
        }
    } else {
        // Handle other operations (NONE, CHANNEL, GREY, BINARY, BLUR, EDGE)
        for (int y = 0; y < h; y++) {
            for (int x = 0; x < w; x++) {
                filter(img, *buffer, x, y);
            }
        }
    }
}


void Canvas::drawHistogram(QPainter &painter) {
    // Prüfen, ob der aktuelle Modus HISTO ist
    if (op != HISTO) {
        return;
    }

    // Festlegen der Parameter für das Histogramm-Diagramm
    int barWidth = 2;
    int maxValue = *std::max_element(std::begin(histogram), std::end(histogram));
    int maxHeight = 100; // Höhe des Histogramms
    int maxWidth = 256 * barWidth; // Breite des Histogramms

    // Skalierungsfaktor für die Höhe der Säulen
    qreal scaleFactor = static_cast<qreal>(maxHeight) / maxValue;

    // Position des Histogramms rechts unten vom Bild
    int histogramX = width() - maxWidth - 20; // Abstand vom rechten Rand
    int histogramY = height() - maxHeight - 20; // Abstand vom unteren Rand

    // Zeichnen des Hintergrunds
    QRect histogramRect(histogramX, histogramY, maxWidth, maxHeight);
    painter.fillRect(histogramRect, Qt::white);
    painter.setPen(Qt::black);

    // Zeichnen der Säulen des Histogramms
    for (int i = 0; i < 256; ++i) {
        int barHeight = static_cast<int>(histogram[i] * scaleFactor);
        painter.fillRect(i * barWidth + histogramRect.left(), maxHeight - barHeight + histogramRect.top(),
                         barWidth, barHeight, Qt::black);
    }



    // Zeichnen des Rahmens (optional)
    painter.drawRect(histogramRect);
}



void Canvas::paintEvent(QPaintEvent *event) {
    QFrame::paintEvent(event);

    QPainter painter(this);
    int cw = width(), ch = height();

    // Zeichnen des Bildes wie bisher
    if (buffer) {
        int s = (cw < ch ? cw : ch) - 2;
        QImage out(buffer->scaled(s, s,
                                  Qt::KeepAspectRatioByExpanding,
                                  Qt::SmoothTransformation));
        QRect rect(1, 1, out.width(), out.height());
        painter.drawImage(rect, out);
    }

    // Zeichnen des Histogramms
    drawHistogram(painter);
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
