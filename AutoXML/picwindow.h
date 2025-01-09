#ifndef PICWINDOW_H
#define PICWINDOW_H

#include <QWidget>
#include <QFileDialog>
#include<opencv2/opencv.hpp>
#include "opencv2/imgproc/imgproc_c.h"
#include "QGraphicsScene"
#include <QGraphicsView>
#include "qimagewidget.h"
#include <QPushButton>
//using namespace cv;
using namespace std;
namespace Ui {
class PicWindow;
}

class PicWindow : public QGraphicsView
{
    Q_OBJECT

public:
    explicit PicWindow(QWidget *parent = nullptr);
    PicWindow(QImage image);
//    void OpenImageFile(QString fileName);
    ~PicWindow();
    void setImage(QImage image,bool update=false);
private slots:


private:
    QGraphicsScene* qgraphicsScene = nullptr;//要用QGraphicsView就必须要有QGraphicsScene搭配着用
    QImageWidget* m_pImage = nullptr;
    QPixmap ConvertPixmap;
};

#endif // PICWINDOW_H
