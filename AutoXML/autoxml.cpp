#include "autoxml.h"
#include <QFile>
#include <QDomDocument>
#include <QTextStream>
#include <QDebug>
#include <QDir>
#include <QFileDialog>
#include <QMessageBox>

using namespace cv;
using namespace std;

yolo::Image cvimg(const cv::Mat& image) { return yolo::Image(image.data, image.cols, image.rows); }

AutoXML::AutoXML(QWidget *parent)
    : QMainWindow(parent)
{
    ui.setupUi(this);
    loadClasses();
    dafaultPath();
    hotkeyjietu = new QHotkey(QKeySequence("delete"), true);
    imageScence = new QGraphicsScene(this);
    ui.graphicsView->setScene(imageScence);
    // 快捷键注册
    connect(hotkeyjietu, &QHotkey::activated, this, [=]() {
        on_pbsave_clicked();
        });
    connect(ui.addYolo, &QPushButton::clicked, this, &AutoXML::on_loadYolo_clicked);
}

AutoXML::~AutoXML()
{}

void AutoXML::saveXMLAndPic()
{
    QString name = QString("%1.%2").arg(ui.spinBox->text()).arg(ui.comboBox->currentText());
    QString savePath = QString("%1/%2").arg(picPath).arg(name);
    QString anaName = QString("%1/%2.xml").arg(anaPath).arg(ui.spinBox->text());
    QFile file(anaName); //相对路径、绝对路径、资源路径都可以
    if (!file.open(QFile::WriteOnly | QFile::Truncate)) //可以用QIODevice，Truncate表示清空原来的内容
        return;
    QDomDocument doc;
    QDomElement root = doc.createElement("annotation"); //创建根元素
    doc.appendChild(root); //添加根元素

    QDomElement folder = doc.createElement("folder");
    folder.appendChild(doc.createTextNode(ui.comboBox->currentText())); //为folder元素增加文本

    QDomElement filename = doc.createElement("filename");
    filename.appendChild(doc.createTextNode(name)); //为filename元素增加文本

    QDomElement path = doc.createElement("path");
    path.appendChild(doc.createTextNode(savePath)); //为folder元素增加文本

    root.appendChild(folder); //添加根元素
    root.appendChild(filename); //添加根元素
    root.appendChild(path); //添加根元素

    QDomElement source = doc.createElement("source");
    QDomElement database = doc.createElement("database");
    database.appendChild(doc.createTextNode("Unknown"));
    source.appendChild(database);
    root.appendChild(source); //添加根元素

    QDomElement size = doc.createElement("size");
    QDomElement width = doc.createElement("width");
    width.appendChild(doc.createTextNode(QString("%1").arg(image.cols)));
    size.appendChild(width);
    QDomElement height = doc.createElement("height");
    height.appendChild(doc.createTextNode(QString("%1").arg(image.rows)));
    size.appendChild(height);
    QDomElement depth = doc.createElement("depth");
    depth.appendChild(doc.createTextNode("3"));
    size.appendChild(depth);
    root.appendChild(size); //添加根元素

    QDomElement segmented = doc.createElement("segmented");
    segmented.appendChild(doc.createTextNode("0"));
    root.appendChild(segmented); //添加根元素

    for (auto& obj : objs) {
        if (obj.class_label >= ui.sbmin->value() && obj.class_label <= ui.sbmax->value()) {
            auto yoloname = cocolabels[obj.class_label];
            QDomElement object = doc.createElement("object");
            QDomElement objname = doc.createElement("name");
            objname.appendChild(doc.createTextNode(yoloname));
            object.appendChild(objname);

            QDomElement pose = doc.createElement("pose");
            pose.appendChild(doc.createTextNode("Unspecified"));
            object.appendChild(pose);

            QDomElement truncated = doc.createElement("truncated");
            truncated.appendChild(doc.createTextNode("0"));
            object.appendChild(truncated);

            QDomElement difficult = doc.createElement("difficult");
            difficult.appendChild(doc.createTextNode("0"));
            object.appendChild(difficult);

            QDomElement bndbox = doc.createElement("bndbox");
            QDomElement xmin = doc.createElement("xmin");
            xmin.appendChild(doc.createTextNode(QString("%1").arg(qRound(obj.left))));
            bndbox.appendChild(xmin);
            QDomElement ymin = doc.createElement("ymin");
            ymin.appendChild(doc.createTextNode(QString("%1").arg(qRound(obj.top))));
            bndbox.appendChild(ymin);
            QDomElement xmax = doc.createElement("xmax");
            xmax.appendChild(doc.createTextNode(QString("%1").arg(qRound(obj.right))));
            bndbox.appendChild(xmax);
            QDomElement ymax = doc.createElement("ymax");
            ymax.appendChild(doc.createTextNode(QString("%1").arg(qRound(obj.bottom))));
            bndbox.appendChild(ymax);
            object.appendChild(bndbox);
            root.appendChild(object); //添加根元素
        }
    }
    QTextStream out_stream(&file); //输出到文件
    doc.save(out_stream, 8); //缩进8格
    file.close();
    imwrite(savePath.toLocal8Bit().toStdString(), image);
}

void AutoXML::loadClasses()
{
    QFile f("voc_classes.txt");
    if (!f.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        qDebug() << (u8"打开文件失败");
    }
    QTextStream txtInput(&f);
    QString lineStr;
    cocolabels.clear();
    while (!txtInput.atEnd())
    {
        lineStr = txtInput.readLine();
        qDebug() << (lineStr);
        cocolabels << lineStr.simplified();
    }
    f.close();
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

bool AutoXML::GetScreenBmp(int left, int top, int width, int height, cv::Mat& image)
{
    BitBlt(memDC, 0, 0, width, height, pDC, left, top, SRCCOPY);//图像宽度高度和截取位置
    BITMAP bmp;
    GetObject(memBitmap, sizeof(BITMAP), &bmp);
    image.create(cvSize(bmp.bmWidth, bmp.bmHeight), CV_MAKETYPE(CV_8U, 4));

    GetBitmapBits(memBitmap, bmp.bmHeight * bmp.bmWidth * 4, image.data);

    cvtColor(image, image, CV_BGRA2BGR);

    return true;
}

void AutoXML::getDNFImage()
{
    switch (ui.screenScale->currentIndex()) {
    case 0:
        GetScreenBmp(0, 0, 1067, 600, image);
        break;
    case 1:
        GetScreenBmp(0, 0, 800, 600, image);
        break;
    case 2:
        GetScreenBmp(0, 0, 960, 600, image);
        break;
    default:
        GetScreenBmp(0, 0, 1067, 600, image);
        break;
    }
}

void AutoXML::update()
{
    ui.graphicsView->resetTransform();
    Mat RGBimage = image.clone();
    for (auto& obj : objs) {
        if (obj.class_label >= ui.sbmin->value() && obj.class_label <= ui.sbmax->value()) {
            uint8_t b, g, r;
            tie(b, g, r) = yolo::random_color(obj.class_label);
            cv::rectangle(RGBimage, cv::Point(obj.left, obj.top), cv::Point(obj.right, obj.bottom), cv::Scalar(b, g, r), 1);
            auto name = cocolabels[obj.class_label].toStdString();
            auto caption = cv::format("%.2f %s", obj.confidence, name.c_str());
            int width = cv::getTextSize(caption, 3, 1, 1, nullptr).width * 0.4;
            cv::rectangle(RGBimage, cv::Point(obj.left - 3, obj.top - 18), cv::Point(obj.left + width, obj.top), cv::Scalar(b, g, r), -1);
            cv::putText(RGBimage, caption, cv::Point(obj.left, obj.top - 5), 0, 0.4, cv::Scalar::all(0), 1, 6);
            line(RGBimage, Point((obj.left + obj.right) / 2, obj.bottom - 5), Point((obj.left + obj.right) / 2, obj.bottom + 5), Scalar(b, g, r), 2);
        }
    }
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
    imageScence->clear();
    
    if (yolo == nullptr) {
        on_loadYolo_clicked();
    }
    if (yolo == nullptr)
    {
        return;
    }
    getDNFImage();
    objs = yolo->forward(cvimg(image.clone()));
    saveXMLAndPic();
    update();
}

void AutoXML::on_loadYolo_clicked()
{
    if (yolo == nullptr) {
        yolo = yolo::load("yolo_FP16.trt", yolo::Type::X, 0.3, 0.5);
        if (yolo == nullptr) {
            QMessageBox::critical(this, tr(u8"错误"), tr(u8"YOLO模型加载失败"));
            return;
        }
        else {
            ui.addYolo->setText(u8"释放YOLO模型");
        }
    }
    else {
        yolo.reset();
        ui.addYolo->setText(u8"加载YOLO模型");
    }

}

