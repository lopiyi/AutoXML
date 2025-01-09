#pragma once

#include <QtWidgets/QMainWindow>
#include "ui_autoxml.h"
#include "qhotkey.h"
#include <memory>
#include <Windows.h>
#include "DnfFullScreen.h"
#include "DnfMiniMap.h"
#include <QHash>

class AutoXML : public QMainWindow
{
    Q_OBJECT

public:
    AutoXML(QWidget *parent = nullptr);
    ~AutoXML();
public:
    void dafaultPath();
    bool getDNFImage();
    void update();
    //bool caijika();
public slots:
    void on_pushButton_clicked();
    void on_pbsave_clicked();
    void on_loadYolo_clicked();
private:
    Ui::AutoXMLClass ui;
    QString picPath, anaPath;
    cv::Mat image;
    QGraphicsScene* imageScence;
    QHotkey* hotkeyjietu = nullptr;
    
    QHash<QString, ClassAbstract*> classMap;
    QString lastClass;
};
