#pragma once
#include "ClassAbstract.h"
#include <Windows.h>
#include "opencv2/imgproc/imgproc_c.h"
#include "cpm.hpp"
#include "infer.hpp"
#include "yolo.hpp"
#include <QStringList>

class DnfFullScreen:public ClassAbstract
{
public:
    DnfFullScreen();
    ~DnfFullScreen();
    cv::Mat detect(const Data& m_data) override;
    //void setYolo(std::shared_ptr<yolo::Infer> yolo) override;
    QString getName() override;
    virtual void setting()override;
    virtual void reset()override;
private:
    void init();
    void saveXMLAndPic(const Data& m_data);
    bool GetScreenBmp(int left, int top, int width, int height, cv::Mat& image);
    bool loadModel();
private:
    QStringList cocolabels;
    yolo::BoxArray objs;
    std::shared_ptr<yolo::Infer> yolo = nullptr;
    cv::Mat image;

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

