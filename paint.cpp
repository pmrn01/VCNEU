#include <QPushButton>
#include <QLineEdit>
#include <QComboBox>
#include <QLabel>
#include <QGridLayout>
#include "paint.h"
#include "canvas.h"

Paint::Paint(QWidget *parent)
    : QWidget(parent)
{
    setWindowTitle("Image Operations");

    viewport = new Canvas();

    btnGenPoints = new QPushButton("&Apply Operation");
    leNumPoints = new QLineEdit("128");
    leFileName = new QLineEdit("../../lena.png");

    cobColModes = new QComboBox();
    cobColModes->addItem(tr("None"), Canvas::NONE);
    cobColModes->addItem(tr("Blue to 0"), Canvas::CHANNEL);
    cobColModes->addItem(tr("Grayscale"), Canvas::GREY);
    cobColModes->addItem(tr("Binarization"), Canvas::BINARY);
    cobColModes->addItem(tr("Blur"), Canvas::BLUR);
    cobColModes->addItem(tr("Edge Detect"), Canvas::EDGE);
    cobColModes->addItem(tr("Histogram"), Canvas::HISTO);
    cobColModes->setCurrentIndex(0);

    lblColModes = new QLabel("Choose filter mode:");
    lblColModes->setBuddy(cobColModes);

    QGridLayout *mainLayout = new QGridLayout;

    mainLayout->addWidget(viewport, 0, 0, 1, 3);
    mainLayout->addWidget(lblColModes, 1, 1, Qt::AlignRight);
    mainLayout->addWidget(cobColModes, 1, 2);
    mainLayout->addWidget(leNumPoints, 2, 1, Qt::AlignRight);
    mainLayout->addWidget(btnGenPoints, 2, 2);
    mainLayout->addWidget(leFileName, 3, 1, 1, 2);

    setLayout(mainLayout);

    connect(btnGenPoints, SIGNAL(clicked()), this, SLOT(genBtnPressed()));
    connect(cobColModes, SIGNAL(activated(int)), this, SLOT(colModeChanged(int)));
}

Paint::~Paint()
{
}

void Paint::genBtnPressed()
{
    int num = leNumPoints->text().toInt();
    viewport->setData(num >= 0 ? num : 0, leFileName->text());
    update();
}

void Paint::colModeChanged(int ind)
{
    int val = cobColModes->itemData(ind >= 0 ? ind : 0).toInt();
    viewport->setMode(val);
    update();
}
