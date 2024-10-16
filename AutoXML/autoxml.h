#pragma once

#include <QtWidgets/QMainWindow>
#include "ui_autoxml.h"
#include "qhotkey.h"
#include <memory>
#include <opencv2/opencv.hpp>
#include "opencv2/imgproc/imgproc_c.h"
#include "cpm.hpp"
#include "infer.hpp"
#include "yolo.hpp"
#include <Windows.h>

class AutoXML : public QMainWindow
{
    Q_OBJECT

public:
    AutoXML(QWidget *parent = nullptr);
    ~AutoXML();
public:
    void saveXMLAndPic();
    void loadClasses();
    void dafaultPath();
    bool GetScreenBmp(int left, int top, int width, int height, cv::Mat& image);
    void getDNFImage();
    void update();
public slots:
    void on_pushButton_clicked();
    void on_pbsave_clicked();
    void on_loadYolo_clicked();
private:
    Ui::AutoXMLClass ui;
    QString picPath, anaPath;
    QStringList cocolabels;
    std::shared_ptr<yolo::Infer> yolo = nullptr;
    cv::Mat image;
    yolo::BoxArray objs;
    QGraphicsScene* imageScence;

    HWND dnf_win = FindWindowA(NULL, (LPCSTR)"���³�����ʿ����������");
    QHotkey* hotkeyjietu = nullptr;

    const HDC pDC = ::GetDC(dnf_win);//��ȡ��ĻDC(0Ϊȫ���������Ϊ����)
    const HDC memDC = ::CreateCompatibleDC(pDC);
    HBITMAP memBitmap = ::CreateCompatibleBitmap(pDC, 1067, 600);
    HBITMAP oldmemBitmap = (HBITMAP)::SelectObject(memDC, memBitmap);//��memBitmapѡ���ڴ�DC;//��������Ļ���ݵ�bitmap
};
