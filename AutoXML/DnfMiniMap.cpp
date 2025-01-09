#include "DnfMiniMap.h"
#include <QFile>
#include <QTextStream>
#include <QDomDocument>
#include "opencv2/imgproc/imgproc_c.h"
#include <QVBoxLayout>
#include <QPainter>
#include <QMouseEvent>
#include <qdebug.h>
#include <QPushButton>
#include <QEventLoop>
#include <QFormLayout>
#include <QSpinBox>
#include <QDialogButtonBox>
#include <QMessageBox>
#include <QCheckBox>
#include <QSettings>
#include <QTextCodec>

using namespace cv;
using namespace std;
//yolo::Image cvimg(const cv::Mat& image) { return yolo::Image(image.data, image.cols, image.rows); }
DnfMiniMap::DnfMiniMap()
{
    init();
}

ImageViewer::~ImageViewer()
{
}

void DnfMiniMap::init()
{
    QFile f("models/minilogo.txt");
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

    QSettings setting("ini.ini", QSettings::IniFormat);
    setting.setIniCodec(QTextCodec::codecForName("utf-8"));
    setting.beginGroup(u8"MiniLogo");
    x = setting.value("Col").toInt();
    y = setting.value("Row").toInt();
    conf = setting.value("Conf").toDouble();
    nms = setting.value("NMS").toDouble();
    caijika_use = setting.value("Caijika").toBool();
    cam_id = setting.value("CamID").toInt();
    dnfx = setting.value("DnfX").toInt();
    dnfy = setting.value("DnfY").toInt();
    setting.endGroup();

    dnf_win = FindWindowA(NULL, (LPCSTR)"地下城与勇士：创新世纪");
    pDC = ::GetDC(dnf_win);//获取屏幕DC(0为全屏，句柄则为窗口)
    memDC = ::CreateCompatibleDC(pDC);
    memBitmap = ::CreateCompatibleBitmap(pDC, 1067, 600);
    oldmemBitmap = (HBITMAP)::SelectObject(memDC, memBitmap);//将memBitmap选入内存DC;//建立和屏幕兼容的bitmap

    //x = 7;
    //y = 2;
    colMax = 1056;
    rowMin = 52;
    r_start = rowMin;
    c_start = colMax - 18 * x;
    crop_size = 32 * 0;
    resize_size = 320;
    img_ratio = qMin(1.0*resize_size/(x * 18), 1.0 * resize_size / (y * 18));
}

bool DnfMiniMap::loadModel()
{
    if (yolo == nullptr) {
        yolo = yolo::load("models/minilogo_FP16.trt", yolo::Type::X, conf, nms);
        if (yolo == nullptr) {
            QMessageBox::critical(nullptr, u8"错误", u8"minilogo_FP16.trt加载失败");
            return false;
        }
    }
    return true;
}

cv::Mat DnfMiniMap::detect(const Data& m_data)
{
    if (!loadModel())
        return cv::Mat();
    cv::Mat image1;
    if (!GetScreenBmp(0, 0, 1067, 600, image1))
        return cv::Mat();
    else {
        image = image1(cv::Range(rowMin, rowMin + y * 18), cv::Range(colMax - x * 18, colMax)).clone();
        cv::resize(image, image, cv::Size(image.size().width * img_ratio, image.size().height * img_ratio));
    }
    objs = yolo->forward(yolo::Image(image.data, image.cols, image.rows));
    
    //genBoxes(minimap(image, Scalar(92, 220, 82), Scalar(113, 244, 255)), 0);        // zhu
    //genBoxes(minimap(image, Scalar(0, 248, 60), Scalar(6, 255, 235)), 1);           // boss
    //genBoxes(minimap(image, Scalar(0, 82, 42), Scalar(5, 92, 52)), 2);              // 裂缝
    //genBoxes(minimap(image, Scalar(87, 253, 231), Scalar(101, 255, 241)), 3);       // 流雨瀑布蓝色牛头
    //genBoxes(minimap(image, Scalar(39, 243, 228), Scalar(43, 255, 240)), 4);        // 流雨瀑布黄色牛头

    //Mat mask = image.clone();
    //
    //auto qimg = QImage((unsigned char*)mask.data, // 图像数据
    //    mask.cols, mask.rows, // 图像尺寸
    //    mask.step, //bytesPerLine  ***
    //    QImage::Format_BGR888); //图像格式
    //static QVector<QPoint> points_yitong;
    //static QVector<QPoint> points_weitong;
    //points_yitong.clear();
    //points_weitong.clear();
    //ImageViewer iv(qimg, points_yitong, points_weitong);

    //QEventLoop loop;
    //QObject::connect(&iv, &ImageViewer::closed, &loop, &QEventLoop::quit);
    //iv.resize(600, 600);
    //iv.setWindowFlags(Qt::WindowStaysOnTopHint);
    //iv.showNormal();
    //loop.exec();
    //genBoxes(points_yitong, 5);                                                 // 已通关
    //genBoxes(points_weitong, 6);                                                // 未通关
    //genBoxes(minimap(image, Scalar(21, 18, 254), Scalar(33, 67, 255)), 7);        // 活动图标
    //genBoxes(minimap(image, Scalar(21, 233, 204), Scalar(21, 255, 252)), 8);        // 风暴黄色牛头
    //genBoxes(minimap(image, Scalar(143, 250, 32), Scalar(149, 255, 153)), 9);        // 未央祭坛
    //cv::resize(image, image, cv::Size(image.size().width * img_ratio, image.size().height * img_ratio));
    saveXMLAndPic(m_data); 
    //Mat RGBimage = image(Range(rowMin, rowMin + crop_size), Range(colMax - crop_size, colMax)).clone();
    
    Mat RGBimage = image.clone();
    
    for (auto& obj : objs) {
        if (obj.class_label >= m_data.min && obj.class_label <= m_data.max) {
            uint8_t b, g, r;
            tie(b, g, r) = yolo::random_color(obj.class_label);
            cv::rectangle(RGBimage, cv::Point(obj.left, obj.top), cv::Point(obj.right, obj.bottom), cv::Scalar(b, g, r), 1);
            //auto name = cocolabels[obj.class_label].toStdString();
            //auto caption = cv::format("%.2f %s", obj.confidence, name.c_str());
            //int width = cv::getTextSize(caption, 3, 1, 1, nullptr).width * 0.4;
            //cv::rectangle(RGBimage, cv::Point(obj.left - 3, obj.top - 18), cv::Point(obj.left + width, obj.top), cv::Scalar(b, g, r), -1);
            //cv::putText(RGBimage, caption, cv::Point(obj.left, obj.top - 5), 0, 0.4, cv::Scalar::all(0), 1, 6);
            //line(RGBimage, Point((obj.left + obj.right) / 2, obj.bottom - 5), Point((obj.left + obj.right) / 2, obj.bottom + 5), Scalar(b, g, r), 2);
        }
    }
	return RGBimage;
}

QString DnfMiniMap::getName()
{
	return QString(u8"小地图");
}

void DnfMiniMap::setting()
{
    QDialog dialog;
    static int currentBei = 0;
    dialog.setWindowTitle(u8"小地图设置");
    QFormLayout form(&dialog);
    QString value1 = QString(u8"小地图列数: ");
    QSpinBox* x_sb = new QSpinBox(&dialog);
    x_sb->setMinimum(1);
    x_sb->setMaximum(99);
    x_sb->setValue(x);
    QString value2 = QString(u8"小地图行数: ");
    QSpinBox* y_sb = new QSpinBox(&dialog);
    y_sb->setMinimum(1);
    y_sb->setMaximum(99);
    y_sb->setSingleStep(1);
    y_sb->setValue(y);
    QString value3 = QString(u8"置信度阈值: ");
    QDoubleSpinBox* conf_sb = new QDoubleSpinBox(&dialog);
    conf_sb->setMinimum(0.1);
    conf_sb->setMaximum(1);
    conf_sb->setSingleStep(0.05);
    conf_sb->setValue(conf);
    QString value4 = QString(u8"IOU阈值: ");
    QDoubleSpinBox* iou_sb = new QDoubleSpinBox(&dialog);
    iou_sb->setMinimum(0.1);
    iou_sb->setMaximum(1);
    iou_sb->setSingleStep(0.05);
    iou_sb->setValue(nms);
    QCheckBox* caijika_cb = new QCheckBox(u8"使用采集卡", &dialog);
    caijika_cb->setChecked(caijika_use);
    QString value5 = QString(u8"使用采集卡: ");
    QSpinBox* cam_sb = new QSpinBox(&dialog);
    cam_sb->setMinimum(0);
    cam_sb->setMaximum(9999);
    cam_sb->setValue(cam_id);
    QString value6 = QString(u8"游戏X坐标: ");
    QSpinBox* dnfx_sb = new QSpinBox(&dialog);
    dnfx_sb->setMinimum(0);
    dnfx_sb->setMaximum(9999);
    dnfx_sb->setValue(dnfx);
    QString value7 = QString(u8"游戏Y坐标: ");
    QSpinBox* dnfy_sb = new QSpinBox(&dialog);
    dnfy_sb->setMinimum(0);
    dnfy_sb->setMaximum(9999);
    dnfy_sb->setValue(dnfy);

    form.addRow(value1, x_sb);
    form.addRow(value2, y_sb);
    form.addRow(value3, conf_sb);
    form.addRow(value4, iou_sb);
    form.addRow(caijika_cb);
    form.addRow(value5, cam_sb);
    form.addRow(value6, dnfx_sb);
    form.addRow(value7, dnfy_sb);
    QDialogButtonBox buttonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel,
        Qt::Horizontal, &dialog);
    form.addRow(&buttonBox);
    QObject::connect(&buttonBox, SIGNAL(accepted()), &dialog, SLOT(accept()));
    QObject::connect(&buttonBox, SIGNAL(rejected()), &dialog, SLOT(reject()));
    if (dialog.exec() == QDialog::Rejected) {
        return;
    }
    x = x_sb->value();
    y = y_sb->value();
    conf = conf_sb->value();
    nms = iou_sb->value();
    caijika_use = caijika_cb->isChecked();
    dnfx = dnfx_sb->value();
    dnfy = dnfy_sb->value();
    QSettings setting("ini.ini", QSettings::IniFormat);
    setting.setIniCodec(QTextCodec::codecForName("utf-8"));
    setting.beginGroup(u8"MiniLogo");
    setting.setValue("Col", x);
    setting.setValue("Row", y);
    setting.setValue("Conf", conf);
    setting.setValue("NMS", nms);
    setting.setValue("Caijika", caijika_use);
    setting.setValue("CamID", cam_id);
    setting.setValue("DnfX", dnfx);
    setting.setValue("DnfY", dnfy);
    setting.endGroup();
}

void DnfMiniMap::reset()
{
    yolo.reset();
    capture.release();
}

void DnfMiniMap::saveXMLAndPic(const Data& m_data)
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
    filename.appendChild(doc.createTextNode(m_data.name + "." + m_data.imgsuf)); //为filename元素增加文本

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
    //imwrite(savePath.toLocal8Bit().toStdString(), image(Range(rowMin, rowMin + crop_size), Range(colMax - crop_size, colMax)));
    imwrite(savePath.toLocal8Bit().toStdString(), image);
}

bool DnfMiniMap::GetScreenBmp(int left, int top, int width, int height, cv::Mat& image)
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
        cv::Mat frame;
        for (int i = 0; i < 3; i++) {
            capture.read(frame);
        }
        image = frame(Range(dnfy, dnfy + 600), Range(dnfx, dnfx + 1067)).clone();
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

QVector<QPoint> DnfMiniMap::minimap(cv::Mat& src, cv::Scalar lower, cv::Scalar upper)
{
    QVector<QPoint> points;
    
    //Mat mask = src(cv::Range(rowMin, rowMin+y*18), cv::Range(colMax-x*18, colMax)).clone();
    Mat mask = src.clone();
    cvtColor(mask, mask, CV_BGR2HSV);
    inRange(mask, lower, upper, mask);
    if (cv::sum(mask)[0] / 255 < 10) {
        return points;
    }
    Mat binary;
    Mat kernel = getStructuringElement(MORPH_RECT, Size(3, 3));
    morphologyEx(mask, binary, MORPH_CLOSE, kernel);
    vector<vector<Point>> contours;
    vector<Vec4i> hierarchy;
    findContours(binary, contours, hierarchy, RETR_EXTERNAL, CHAIN_APPROX_NONE);
    for (auto& con:contours)
    {
        if (cv::contourArea(con)>5)
        {
            QPoint p;
            p.setX(con[0].x / 18 + 1);
            p.setY(con[0].y / 18 + 1);
            points << p;
        }
    }
    return points;
}

void DnfMiniMap::genBoxes(QVector<QPoint> points,int class_label)
{
    for (auto& p : points)
    {
        //objs.push_back(yolo::Box(c_start + 18 * (p.x() - 1) - (colMax - crop_size),
        //    r_start + 18 * (p.y() - 1) - rowMin,
        //    c_start + 18 * p.x() - (colMax - crop_size),
        //    r_start + 18 * p.y() - rowMin,
        //    1, class_label));
        objs.push_back(yolo::Box(18 * (p.x() - 1)* img_ratio,
            18 * (p.y() - 1) * img_ratio,
            18 * p.x() * img_ratio,
            18 * p.y() * img_ratio,
            1, class_label));
    }
}

#include <QShortcut>

ImageViewer::ImageViewer(QImage image, QVector<QPoint>& points_yitong, QVector<QPoint>& points_weitong, QWidget* parent)
    : QLabel(parent), points_yitong(points_yitong), points_weitong(points_weitong)
{
    setAttribute(Qt::WA_DeleteOnClose);
    setAttribute(Qt::WA_QuitOnClose, false);
    setWindowModality(Qt::ApplicationModal);
    setAttribute(Qt::WA_ShowModal, true);
    // 加载图片
    this->image = QPixmap::fromImage(image);
    QShortcut* shortcut = new QShortcut(QKeySequence(Qt::Key_Backspace), this);
    QObject::connect(shortcut, &QShortcut::activated, [this]() {
        if (classes.size())
        {
            if (classes.takeLast() == 0)
            {
                this->points_yitong.removeLast();
                points_ui_yitong.removeLast();
            }
            else
            {
                this->points_weitong.removeLast();
                points_ui_weitong.removeLast();
            }
            
            update();
        }
        });
    QShortcut* shortcut_quit = new QShortcut(QKeySequence(Qt::Key_Escape), this);
    QObject::connect(shortcut_quit, &QShortcut::activated, [this]() {
        close();
        });
}

void ImageViewer::paintEvent(QPaintEvent* event)
{
    QLabel::paintEvent(event);
    QPainter painter(this);
    painter.setPen(Qt::red);
    painter.setRenderHint(QPainter::SmoothPixmapTransform);
    QRect scaledRect = image.scaled(size(), Qt::KeepAspectRatio, Qt::SmoothTransformation).rect();
    scaledRect.moveCenter(rect().center());
    painter.drawPixmap(scaledRect, image);
    // 绘制圆
    for (const auto& point : points_ui_yitong) {
        painter.drawRect(QRect(leftTop.x()+ point.x()* (18 / ratio), 
            leftTop.y() + point.y() * (18 / ratio), (18 / ratio), (18 / ratio)));
    }
    painter.setPen(Qt::yellow);
    for (const auto& point : points_ui_weitong) {
        painter.drawRect(QRect(leftTop.x() + point.x() * (18 / ratio),
            leftTop.y() + point.y() * (18 / ratio), (18 / ratio), (18 / ratio)));
    }
}

void ImageViewer::mousePressEvent(QMouseEvent* event)
{
    QRect scaledRect = image.scaled(size(), Qt::KeepAspectRatio, Qt::SmoothTransformation).rect();

    ratio = image.width() / static_cast<qreal>(scaledRect.width());
    scaledRect.moveCenter(rect().center());
    leftTop = scaledRect.topLeft();
    QPointF localPos = event->pos() - scaledRect.topLeft();
    QPointF imagePos = localPos * ratio;
    if (imagePos.y()> image.height() || imagePos.y() < 0)
    {
        return;
    }
    QPoint p1 = QPoint(imagePos.x() / 18, imagePos.y() / 18) + QPoint(1, 1);
    QPointF p = QPointF((int)(localPos.x() / (18 / ratio)), (int)(localPos.y() / (18 / ratio)));
    if (event->button() == Qt::LeftButton) {
        points_yitong.append(p1);
        points_ui_yitong.append(p);
        classes.append(0);
        update();
    }
    else if (event->button() == Qt::RightButton) {
        points_weitong.append(p1);
        points_ui_weitong.append(p);
        classes.append(1);
        update();
    }
    QLabel::mousePressEvent(event);
}

void ImageViewer::resizeEvent(QResizeEvent* event)
{
    QLabel::resizeEvent(event);
    update();
}