#include "autoxml.h"
#include <QFile>
#include <QDomDocument>
#include <QTextStream>
#include <QDebug>
#include <QDir>
#include <QFileDialog>
#include <QMessageBox>
#include "hsvselect.h"
#include <windows.h>
#include <setupapi.h>
#include <devguid.h>

using namespace cv;
using namespace std;
#include <windows.h>
#include <dshow.h>
#include <vector>
#include <string>
#include <iostream>

#pragma comment(lib, "Strmiids.lib")


std::vector<std::wstring> EnumerateVideoCaptureDevices() {
    std::vector<std::wstring> devicePaths;

    // Initialize COM library
    HRESULT hr = CoInitialize(NULL);
    if (FAILED(hr)) {
        std::cerr << "Failed to initialize COM library" << std::endl;
        return devicePaths;
    }

    // Create the system device enumerator
    ICreateDevEnum* pCreateDevEnum = NULL;
    hr = CoCreateInstance(CLSID_SystemDeviceEnum, NULL, CLSCTX_INPROC_SERVER,
        IID_ICreateDevEnum, (void**)&pCreateDevEnum);
    if (FAILED(hr)) {
        std::cerr << "Failed to create device enumerator" << std::endl;
        CoUninitialize();
        return devicePaths;
    }

    // Create an enumerator for video capture devices
    IEnumMoniker* pEnumMoniker = NULL;
    hr = pCreateDevEnum->CreateClassEnumerator(CLSID_VideoInputDeviceCategory, &pEnumMoniker, 0);
    if (hr == S_FALSE) {
        std::cerr << "No video capture devices found" << std::endl;
        pCreateDevEnum->Release();
        CoUninitialize();
        return devicePaths;
    }
    else if (FAILED(hr)) {
        std::cerr << "Failed to create enumerator for video capture devices" << std::endl;
        pCreateDevEnum->Release();
        CoUninitialize();
        return devicePaths;
    }

    // Enumerate video capture devices
    IMoniker* pMoniker = NULL;
    while (pEnumMoniker->Next(1, &pMoniker, NULL) == S_OK) {
        IPropertyBag* pPropBag = NULL;
        hr = pMoniker->BindToStorage(0, 0, IID_IPropertyBag, (void**)&pPropBag);
        if (SUCCEEDED(hr)) {
            VARIANT var;
            VariantInit(&var);
            hr = pPropBag->Read(L"FriendlyName", &var, 0);
            if (SUCCEEDED(hr)) {
                std::wcout << L"Device Name: " << var.bstrVal << std::endl;
                VariantClear(&var);
            }

            hr = pMoniker->GetDisplayName(NULL, NULL, &var.bstrVal);
            if (SUCCEEDED(hr)) {
                std::wcout << L"Device Path: " << var.bstrVal << std::endl;
                devicePaths.push_back(var.bstrVal);
                VariantClear(&var);
            }

            pPropBag->Release();
        }
        pMoniker->Release();
    }

    // Clean up
    pEnumMoniker->Release();
    pCreateDevEnum->Release();
    CoUninitialize();

    return devicePaths;
}

AutoXML::AutoXML(QWidget *parent)
    : QMainWindow(parent)
{
    ui.setupUi(this);
    dafaultPath();
    hotkeyjietu = new QHotkey(QKeySequence("~"), true);
    imageScence = new QGraphicsScene(this);
    ui.graphicsView->setScene(imageScence);
    // 快捷键注册
    connect(hotkeyjietu, &QHotkey::activated, this, [=]() {
        on_pbsave_clicked();
        });
    connect(ui.addYolo, &QPushButton::clicked, this, &AutoXML::on_loadYolo_clicked);
    connect(ui.actionhsv, &QAction::triggered, [this]() {
        HSVSelect* hsv = HSVSelect::getInstance();
        hsv->show();
        if (image.empty())
            return;
        hsv->setImage(image,true);
        });
    connect(ui.screenScale, &QComboBox::currentTextChanged, [this]() {
        ui.addYolo->setText(ui.screenScale->currentText()+u8"设置");
        if (classMap.keys().contains(lastClass))
        {
            classMap.value(lastClass)->reset();
            lastClass = ui.screenScale->currentText();
        }
        });
    ClassAbstract* abstract1 = new DnfFullScreen;
    ui.screenScale->addItem(abstract1->getName());
    classMap.insert(abstract1->getName(), abstract1);

    ClassAbstract* abstract2 = new DnfMiniMap;
    ui.screenScale->addItem(abstract2->getName());
    classMap.insert(abstract2->getName(), abstract2);

    lastClass = ui.screenScale->currentText();

    connect(ui.pbadd, &QPushButton::clicked, this, [this]() {
        auto cameraDevicePaths = EnumerateVideoCaptureDevices();
        if (cameraDevicePaths.empty()) {
            qDebug() << "No camera devices found.";
            return -1;
        }
        for (auto&str: cameraDevicePaths)
        {
            QString qstr = QString::fromStdWString(str);
            qDebug() << qstr;
        }
        });
}

AutoXML::~AutoXML()
{
    for (auto it = classMap.begin(); it != classMap.end(); ++it)
    {
        it.value()->reset();
    }
}

void AutoXML::dafaultPath()
{
    QDir dir;
    picPath = QString("voc/pic/");
    if (!dir.exists(picPath))
    {
        dir.mkpath(picPath);
    }
    anaPath = QString("voc/xml/");
    if (!dir.exists(anaPath))
    {
        dir.mkpath(anaPath);
    }

    QString folderpath = picPath;
    QDir dir1(folderpath);
    QStringList filter;
    QFileInfoList fileInfoList = dir1.entryInfoList(filter);
    int count = fileInfoList.count() - 2;
    ui.spinBox->setValue(count);
} 

bool AutoXML::getDNFImage()
{
    Data m_data;
    m_data.picPath = picPath;
    m_data.anaPath = anaPath;
    m_data.index = ui.spinBox->value();
    m_data.imgsuf = ui.comboBox->currentText();
    m_data.name = QString::number(ui.spinBox->value());
    m_data.min = ui.sbmin->value();
    m_data.max = ui.sbmax->value();
    image = classMap.value(ui.screenScale->currentText())->detect(m_data);
    if (image.empty())
    {
        QMessageBox::critical(this, u8"错误", ui.screenScale->currentText()+u8"取图失败！！");
        return false;
    }
    return true;
}

void AutoXML::update()
{
    imageScence->clear();
    ui.graphicsView->resetTransform();
    Mat RGBimage = image.clone();

    auto qimg = QImage((unsigned char*)RGBimage.data, // 图像数据
        RGBimage.cols, RGBimage.rows, // 图像尺寸
        RGBimage.step, //bytesPerLine  ***
        QImage::Format_BGR888); //图像格式
    QPixmap tempPixmap = QPixmap::fromImage(qimg);
    imageScence->addPixmap(tempPixmap);
    ui.graphicsView->fitInView(qimg.rect(), Qt::KeepAspectRatio);
    ui.graphicsView->setSceneRect(tempPixmap.rect());
    imageScence->update();

    int k = ui.spinBox->value();
    ui.spinBox->setValue(k + 1);

}

void AutoXML::on_pushButton_clicked()
{
    QString dlgTitle = u8"选择保存图片的目录";
    QString selectedDir = QFileDialog::getExistingDirectory(this, dlgTitle, "./", QFileDialog::ShowDirsOnly);
    if (!selectedDir.isEmpty()) {
        picPath = selectedDir;
    }
}

void AutoXML::on_pbsave_clicked()
{
    if (picPath.isEmpty()) { QMessageBox::about(this, tr(u8"提示"), tr(u8"图片保存目录未设置")); return; }
    if (anaPath.isEmpty()) { QMessageBox::about(this, tr(u8"提示"), tr(u8"标签保存目录未设置")); return; }    
    //if (yolo == nullptr) {
    //    on_loadYolo_clicked();
    //}
    //if (yolo == nullptr)
    //{
    //    return;
    //}
    if (!getDNFImage()) {
        return;
    }
    update();
}

void AutoXML::on_loadYolo_clicked()
{
    //if (yolo == nullptr) {
    //    yolo = yolo::load("yolo_FP16.trt", yolo::Type::X, 0.3, 0.5);
    //    if (yolo == nullptr) {
    //        QMessageBox::critical(this, tr(u8"错误"), tr(u8"YOLO模型加载失败"));
    //        return;
    //    }
    //    else {
    //        for (auto it = classMap.begin(); it != classMap.end(); ++it) {
    //            it.value()->setYolo(yolo);
    //        }
    //        ui.addYolo->setText(u8"释放YOLO模型");
    //    }
    //}
    //else {
    //    yolo.reset();
    //    ui.addYolo->setText(u8"加载YOLO模型");
    //    if (capture.isOpened())
    //    {
    //        capture.release();
    //    }
    //}
    classMap.value(ui.screenScale->currentText())->setting();
}

