#include "picwindow.h"
#include "qimagewidget.h"

PicWindow::PicWindow(QWidget *parent) :
    QGraphicsView(parent)
{
    this->setAttribute(Qt::WA_QuitOnClose, false);

    qgraphicsScene = new QGraphicsScene;//要用QGraphicsView就必须要有QGraphicsScene搭配着用

    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_pImage = new QImageWidget(&ConvertPixmap);
    qgraphicsScene->addItem(m_pImage);
    setScene(qgraphicsScene);//Sets the current scene to scene. If scene is already being viewed, this function does nothing.
}
PicWindow::PicWindow(QImage image){

}
PicWindow::~PicWindow()
{
    
}

//void PicWindow::OpenImageFile(QString fileName)
//{
////    QString fileName = QFileDialog::getOpenFileName(this, "open Image", "", "Image File(*.bmp *.jpg *.jpeg *.png)");
//    QTextCodec* code = QTextCodec::codecForName("gb18030");
//    std::string name = code->fromUnicode(fileName).data();
//    auto m_srcImage = cv::imread(name);

//    if (m_srcImage.data)
//    {
//        cvtColor(m_srcImage, m_srcImage, cv::COLOR_BGR2RGB);//BGR转化为RGB
//                QImage::Format format = QImage::Format_RGB888;
//        switch (m_srcImage.type())
//        {
//        case CV_8UC1:
//            format = QImage::Format_Indexed8;
//            break;
//        case CV_8UC3:
//            format = QImage::Format_RGB888;
//            break;
//        case CV_8UC4:
//            format = QImage::Format_ARGB32;
//            break;
//        }
//        QImage img = QImage((const uchar*)m_srcImage.data, m_srcImage.cols, m_srcImage.rows,
//            m_srcImage.cols * m_srcImage.channels(), format);
//        cv::Mat imgHsv;
//        cv::cvtColor(m_srcImage, imgHsv, cv::COLOR_RGB2HSV);
//        QImage qimghsv = QImage((const uchar*)imgHsv.data, imgHsv.cols, imgHsv.rows,
//            imgHsv.cols * imgHsv.channels(), format);
//        recvShowPicSignal(img);
//        recvShowPicSignalHSV(qimghsv);
//    }
//}

void PicWindow::setImage(QImage image, bool update)
{
    
    //qgraphicsScene->clear();
    //QPixmap ConvertPixmap = QPixmap::fromImage(image);//The QPixmap class is an off-screen image representation that can be used as a paint device
    //QImageWidget* m_pImage = new QImageWidget(&ConvertPixmap);//实例化类ImageWidget的对象m_Image，该类继承自QGraphicsItem，是自己写的类
    //int nwith = this->width();//获取界面控件Graphics View的宽度
    //int nheight = this->height();//获取界面控件Graphics View的高度
    //m_pImage->setQGraphicsViewWH(nwith, nheight);//将界面控件Graphics View的width和height传进类m_Image中
    //qgraphicsScene->addItem(m_pImage);//将QGraphicsItem类对象放进QGraphicsScene中
    //this->setSceneRect(QRectF(-(nwith / 2), -(nheight / 2), nwith, nheight));//使视窗的大小固定在原始大小，不会随图片的放大而放大（默认状态下图片放大的时候视窗两边会自动出现滚动条，并且视窗内的视野会变大），防止图片放大后重新缩小的时候视窗太大而不方便观察图片
    //qDebug() << nwith<< nheight;
    ConvertPixmap = QPixmap::fromImage(image);//The QPixmap class is an off-screen image representation that can be used as a paint device
    m_pImage->ini(&ConvertPixmap);
    if (update)
    {
        int nwith = this->width();//获取界面控件Graphics View的宽度
        int nheight = this->height();//获取界面控件Graphics View的高度
        m_pImage->setQGraphicsViewWH(nwith, nheight);//将界面控件Graphics View的width和height传进类m_Image中
        this->setSceneRect(QRectF(-(nwith / 2), -(nheight / 2), nwith, nheight));
        m_pImage->ResetItemPos();
    }
    
    
    qgraphicsScene->update();
}
