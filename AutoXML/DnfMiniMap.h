#pragma once
#include "ClassAbstract.h"
#include <Windows.h>
#include <QStringList>
#include <QWidget>
#include <QImage>
#include <QVector>
#include <QPoint>
#include <QPixmap>
#include <QLabel>
#include <qdialog.h>
#include "cpm.hpp"
#include "infer.hpp"
#include "yolo.hpp"

class ImageViewer : public QLabel
{
    Q_OBJECT

public:
    ImageViewer(QImage image, QVector<QPoint>& points_yitong, QVector<QPoint>& points_weitong, QWidget* parent = nullptr);
    ~ImageViewer() override;
protected:
    void paintEvent(QPaintEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    void resizeEvent(QResizeEvent* event) override;
    void closeEvent(QCloseEvent*) { emit closed(); }
signals:
    void closed();
private:
    QPixmap image;
    QPoint leftTop;
    double ratio;
    QVector<QPointF> points_ui_yitong;
    QVector<QPointF> points_ui_weitong;
    QVector<int> classes;
    QVector<QPoint>& points_yitong;
    QVector<QPoint>& points_weitong;
};

class DnfMiniMap :public ClassAbstract
{
public:
    DnfMiniMap();
    cv::Mat detect(const Data& m_data) override;
    QString getName() override;
    virtual void setting()override;
    virtual void reset()override;
private:
    void init();
    void saveXMLAndPic(const Data& m_data);
    bool GetScreenBmp(int left, int top, int width, int height, cv::Mat& image);
    QVector<QPoint> minimap(cv::Mat& src, cv::Scalar lower, cv::Scalar upper);
    void genBoxes(QVector<QPoint> points, int class_label);
    bool loadModel();
private:
    QStringList cocolabels;
    yolo::BoxArray objs;
    cv::Mat image;
    std::shared_ptr<yolo::Infer> yolo = nullptr;
    int x, y, r_start, c_start;
    int colMax;
    int rowMin;
    int crop_size;
    int resize_size;
    double img_ratio;
    HWND dnf_win;
    HDC pDC;
    HDC memDC;
    HBITMAP memBitmap;
    HBITMAP oldmemBitmap;

    double conf, nms;
    bool caijika_use;
    int cam_id, dnfx, dnfy;
    cv::VideoCapture capture;
};

