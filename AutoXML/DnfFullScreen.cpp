#include "DnfFullScreen.h"
#include <QFile>
#include <qdebug.h>
#include <QDomDocument>
#include <QTextStream>
#include <QMessageBox>
#include <QSettings>
#include <QTextCodec>
#include <QFormLayout>
#include <QSpinBox>
#include <QDialogButtonBox>
#include <QMessageBox>
#include <QCheckBox>

using namespace cv;
using namespace std;

yolo::Image cvimg(const cv::Mat& image) { return yolo::Image(image.data, image.cols, image.rows); }

DnfFullScreen::DnfFullScreen()
{
    init();
}
DnfFullScreen::~DnfFullScreen()
{
}
QString DnfFullScreen::getName()
{
    return QString("16:9");
}

void DnfFullScreen::setting()
{
    QDialog dialog;
    static int currentBei = 0;
    dialog.setWindowTitle(u8"16:9设置");
    QFormLayout form(&dialog);
    QString value1 = QString(u8"置信度阈值: ");
    QDoubleSpinBox* conf_sb = new QDoubleSpinBox(&dialog);
    conf_sb->setMinimum(0.1);
    conf_sb->setMaximum(1);
    conf_sb->setSingleStep(0.05);
    conf_sb->setValue(conf);
    QString value2 = QString(u8"IOU阈值: ");
    QDoubleSpinBox* iou_sb = new QDoubleSpinBox(&dialog);
    iou_sb->setMinimum(0.1);
    iou_sb->setMaximum(1);
    iou_sb->setSingleStep(0.05);
    iou_sb->setValue(nms);
    QCheckBox* caijika_cb = new QCheckBox(u8"使用采集卡", &dialog);
    caijika_cb->setChecked(caijika_use);
    QString value3 = QString(u8"使用采集卡: ");
    QSpinBox* cam_sb = new QSpinBox(&dialog);
    cam_sb->setMinimum(0);
    cam_sb->setMaximum(9999);
    cam_sb->setValue(cam_id);
    QString value4 = QString(u8"游戏X坐标: ");
    QSpinBox* dnfx_sb = new QSpinBox(&dialog);
    dnfx_sb->setMinimum(0);
    dnfx_sb->setMaximum(9999);
    dnfx_sb->setValue(dnfx);
    QString value5 = QString(u8"游戏Y坐标: ");
    QSpinBox* dnfy_sb = new QSpinBox(&dialog);
    dnfy_sb->setMinimum(0);
    dnfy_sb->setMaximum(9999);
    dnfy_sb->setValue(dnfy);

    form.addRow(value1, conf_sb);
    form.addRow(value2, iou_sb);
    form.addRow(caijika_cb);
    form.addRow(value3, cam_sb);
    form.addRow(value4, dnfx_sb);
    form.addRow(value5, dnfy_sb);
    QDialogButtonBox buttonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel,
        Qt::Horizontal, &dialog);
    form.addRow(&buttonBox);
    QObject::connect(&buttonBox, SIGNAL(accepted()), &dialog, SLOT(accept()));
    QObject::connect(&buttonBox, SIGNAL(rejected()), &dialog, SLOT(reject()));
    if (dialog.exec() == QDialog::Rejected) {
        return;
    }
    conf = conf_sb->value();
    nms = iou_sb->value();
    caijika_use = caijika_cb->isChecked();
    cam_id = cam_sb->value();
    dnfx = dnfx_sb->value();
    dnfy = dnfy_sb->value();
    QSettings setting("ini.ini", QSettings::IniFormat);
    setting.setIniCodec(QTextCodec::codecForName("utf-8"));
    setting.beginGroup(u8"FullScreen");
    setting.setValue("Conf", conf);
    setting.setValue("NMS", nms);
    setting.setValue("Caijika", caijika_use);
    setting.setValue("CamID", cam_id);
    setting.setValue("DnfX", dnfx);
    setting.setValue("DnfY", dnfy);
    setting.endGroup();
    return;
}

void DnfFullScreen::reset()
{
    yolo.reset();
    yolo = nullptr;
    capture.release();
}

cv::Mat DnfFullScreen::detect(const Data& m_data)
{
    if(!loadModel())
        return cv::Mat();
    if (!GetScreenBmp(0, 0, 1067, 600, image))
        return cv::Mat();
    
    objs = yolo->forward(cvimg(image.clone()));
    saveXMLAndPic(m_data);
    Mat RGBimage = image.clone();
    for (auto& obj : objs) {
        if (obj.class_label >= m_data.min && obj.class_label <= m_data.max) {
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
    return RGBimage;
}


void DnfFullScreen::init()
{
    QFile f("models/voc_classes.txt");
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
        //qDebug() << (lineStr);
        cocolabels << lineStr.simplified();
    }
    f.close();

    dnf_win = FindWindowA(NULL, (LPCSTR)"地下城与勇士：创新世纪");
    pDC = ::GetDC(dnf_win);//获取屏幕DC(0为全屏，句柄则为窗口)
    memDC = ::CreateCompatibleDC(pDC);
    memBitmap = ::CreateCompatibleBitmap(pDC, 1067, 600);
    oldmemBitmap = (HBITMAP)::SelectObject(memDC, memBitmap);//将memBitmap选入内存DC;//建立和屏幕兼容的bitmap

    QSettings setting("ini.ini", QSettings::IniFormat);
    setting.setIniCodec(QTextCodec::codecForName("utf-8"));
    setting.beginGroup(u8"FullScreen");
    conf = setting.value("Conf").toDouble();
    nms = setting.value("NMS").toDouble();
    caijika_use = setting.value("Caijika").toBool();
    cam_id = setting.value("CamID").toInt();
    dnfx = setting.value("DnfX").toInt();
    dnfy = setting.value("DnfY").toInt();
    setting.endGroup();
}

bool DnfFullScreen::GetScreenBmp(int left, int top, int width, int height, cv::Mat& image)
{
    if (caijika_use) {
        if (!capture.isOpened())
        {
            capture.open(cam_id, cv::CAP_DSHOW);
            capture.set(cv::CAP_PROP_FOURCC, cv::VideoWriter::fourcc('Y', 'U', 'Y', '2'));
            capture.set(cv::CAP_PROP_FRAME_WIDTH, 1900);
            capture.set(cv::CAP_PROP_FRAME_HEIGHT, 1080);
            capture.set(cv::CAP_PROP_FPS, 30);
        }
        if (!capture.isOpened()) {
            QMessageBox::critical(nullptr, u8"错误", u8"采集卡打开失败！！");
            return false;
        }
        const int left = dnfx;
        const int top = dnfy;
        cv::Mat frame;
        for (int i = 0; i < 3; i++) {
            capture.read(frame);
        }
        image = frame(Range(top, top + 600), Range(left, left + 1067)).clone();
    }
    else {
        BitBlt(memDC, 0, 0, width, height, pDC, left, top, SRCCOPY);//图像宽度高度和截取位置
        BITMAP bmp;
        GetObject(memBitmap, sizeof(BITMAP), &bmp);
        image.create(cvSize(bmp.bmWidth, bmp.bmHeight), CV_MAKETYPE(CV_8U, 4));

        GetBitmapBits(memBitmap, bmp.bmHeight * bmp.bmWidth * 4, image.data);

        cvtColor(image, image, CV_BGRA2BGR);
    }
    
    return true;
}

bool DnfFullScreen::loadModel()
{
    if (yolo == nullptr) {
        yolo = yolo::load("models/yolo_FP16.trt", yolo::Type::X, conf, nms);
        if (yolo == nullptr) {
            QMessageBox::critical(nullptr, u8"错误", u8"yolo_FP16.trt加载失败");
            return false;
        }
    }
    return true;
}

void DnfFullScreen::saveXMLAndPic(const Data& m_data)
{
    QString savePath = QString("%1/%2.%3").arg(m_data.picPath).arg(m_data.name).arg(m_data.imgsuf);
    QString anaName = QString("%1/%2.xml").arg(m_data.anaPath).arg(m_data.name);
    QFile file(anaName); //相对路径、绝对路径、资源路径都可以
    if (!file.open(QFile::WriteOnly | QFile::Truncate)) //可以用QIODevice，Truncate表示清空原来的内容
        return;
    QDomDocument doc;
    QDomElement root = doc.createElement("annotation"); //创建根元素
    doc.appendChild(root); //添加根元素

    QDomElement folder = doc.createElement("folder");
    folder.appendChild(doc.createTextNode(m_data.imgsuf)); //为folder元素增加文本

    QDomElement filename = doc.createElement("filename");
    filename.appendChild(doc.createTextNode(m_data.name+"."+m_data.imgsuf)); //为filename元素增加文本

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
        if (obj.class_label >= m_data.min && obj.class_label <= m_data.max) {
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