#include "hsvselect.h"
#include <QFileDialog>
#include <QTextCodec>
HSVSelect* HSVSelect::instance = nullptr;


HSVSelect::HSVSelect(QWidget *parent)
	: QWidget(parent)
{
	ui.setupUi(this);
	setAttribute(Qt::WA_DeleteOnClose);
	setAttribute(Qt::WA_QuitOnClose, false);
	sw = new PicWindow(this);
    QVBoxLayout* layout = new QVBoxLayout(this); // 创建布局并关联到窗口

    layout->addWidget(sw,1); // 添加按钮到布局
    layout->addWidget(ui.frame,0);
    layout->addWidget(ui.frame_8,0);

    setLayout(layout); // 设置窗口的布局为QVBoxLayout

}

HSVSelect::~HSVSelect()
{
	instance = nullptr;
}

HSVSelect* HSVSelect::getInstance()
{
	if (instance == nullptr) {
		instance = new HSVSelect(nullptr);
	}
	return instance;
}

void HSVSelect::setImage(cv::Mat& src, bool update)
{
	image = src.clone();
	sw->setImage(MatToQImage(image),update);
}

void HSVSelect::on_pushButton_clicked()
{
    QString fileName = QFileDialog::getOpenFileName(this, "open Image", "./voc", "Image File(*.bmp *.jpg *.jpeg *.png)");
    if (fileName.size()) {
        QTextCodec* code = QTextCodec::codecForName("gb18030");
        std::string name = code->fromUnicode(fileName).data();
        image = cv::imread(name);

        if (image.data)
        {
            sw->setImage(MatToQImage(image),true);
        }
    }
}

void HSVSelect::on_pb_lower_clicked()
{
    QClipboard* clip = QApplication::clipboard();
    clip->setText(QString("%1, %2, %3").arg(ui.spinBox->value()).arg(ui.spinBox_3->value()).arg(ui.spinBox_5->value()));
}

void HSVSelect::on_pb_upper_clicked()
{
    QClipboard* clip = QApplication::clipboard();
    clip->setText(QString("%1, %2, %3").arg(ui.spinBox_2->value()).arg(ui.spinBox_4->value()).arg(ui.spinBox_6->value()));
}

void HSVSelect::on_pb_reset_clicked()
{
    ui.spinBox->setValue(0);
    ui.spinBox_3->setValue(0);
    ui.spinBox_5->setValue(0);
    ui.spinBox_2->setValue(255);
    ui.spinBox_4->setValue(255);
    ui.spinBox_6->setValue(255);
}

void HSVSelect::on_pb_yuan_clicked()
{
    if (image.empty())return;
    sw->setImage(MatToQImage(image));
}

void HSVSelect::on_pb_jietu_clicked()
{
    if (mask.empty())return;
    auto r = selectROI(u8"手动截图", mask, false);
    cv::destroyAllWindows();
    if (r.width > 0) {
        auto id = mask(r);
        imwrite("jietu.bmp", id);
    }
}


void HSVSelect::on_horizontalSlider_valueChanged(int value)
{
    if (ui.spinBox->value() == value)return;
    ui.spinBox->setValue(value);
    PicShuaXin();
}


void HSVSelect::on_spinBox_valueChanged(int arg1)
{
    if (ui.horizontalSlider->value() == arg1)return;
    ui.horizontalSlider->setValue(arg1);
    PicShuaXin();
}


void HSVSelect::on_horizontalSlider_2_valueChanged(int value)
{
    if (ui.spinBox_2->value() == value)return;
    ui.spinBox_2->setValue(value);
    PicShuaXin();
}


void HSVSelect::on_spinBox_2_valueChanged(int arg1)
{
    if (ui.horizontalSlider_2->value() == arg1)return;
    ui.horizontalSlider_2->setValue(arg1);
    PicShuaXin();
}


void HSVSelect::on_horizontalSlider_3_valueChanged(int value)
{
    if (ui.spinBox_3->value() == value)return;
    ui.spinBox_3->setValue(value);
    PicShuaXin();
}


void HSVSelect::on_spinBox_3_valueChanged(int arg1)
{
    if (ui.horizontalSlider_3->value() == arg1)return;
    ui.horizontalSlider_3->setValue(arg1);
    PicShuaXin();
}


void HSVSelect::on_horizontalSlider_4_valueChanged(int value)
{
    if (ui.spinBox_4->value() == value)return;
    ui.spinBox_4->setValue(value);
    PicShuaXin();
}


void HSVSelect::on_spinBox_4_valueChanged(int arg1)
{
    if (ui.horizontalSlider_4->value() == arg1)return;
    ui.horizontalSlider_4->setValue(arg1);
    PicShuaXin();
}


void HSVSelect::on_horizontalSlider_5_valueChanged(int value)
{
    if (ui.spinBox_5->value() == value)return;
    ui.spinBox_5->setValue(value);
    PicShuaXin();
}


void HSVSelect::on_spinBox_5_valueChanged(int arg1)
{
    if (ui.horizontalSlider_5->value() == arg1)return;
    ui.horizontalSlider_5->setValue(arg1);
    PicShuaXin();
}


void HSVSelect::on_horizontalSlider_6_valueChanged(int value)
{
    if (ui.spinBox_6->value() == value)return;
    ui.spinBox_6->setValue(value);
    PicShuaXin();
}


void HSVSelect::on_spinBox_6_valueChanged(int arg1)
{
    if (ui.horizontalSlider_6->value() == arg1)return;
    ui.horizontalSlider_6->setValue(arg1);
    PicShuaXin();
}

void HSVSelect::PicShuaXin() {
    lower = cv::Scalar(ui.horizontalSlider->value(), ui.horizontalSlider_3->value(), ui.horizontalSlider_5->value());
    upper = cv::Scalar(ui.horizontalSlider_2->value(), ui.horizontalSlider_4->value(), ui.horizontalSlider_6->value());
    if (image.empty())return;
    if (ui.comboBox->currentText() == "RGB")
    {
        mask = image.clone();
    }
    else if (ui.comboBox->currentText() == "HSV")
    {
        cvtColor(image, mask, CV_BGR2HSV);
    }
    cv::inRange(mask, lower, upper, mask);
    sw->setImage(MatToQImage(mask));
}

QImage HSVSelect::MatToQImage(const cv::Mat& mat)
{
    QImage image;
    switch (mat.type())
    {
    case CV_8UC1:
        // QImage构造：数据，宽度，高度，每行多少字节，存储结构
        image = QImage((const unsigned char*)mat.data, mat.cols, mat.rows, mat.step, QImage::Format_Grayscale8);
        break;
    case CV_8UC3:
        image = QImage((const unsigned char*)mat.data, mat.cols, mat.rows, mat.step, QImage::Format_RGB888);
        image = image.rgbSwapped(); // BRG转为RGB
        // Qt5.14增加了Format_BGR888
        // image = QImage((const unsigned char*)mat.data, mat.cols, mat.rows, mat.cols * 3, QImage::Format_BGR888);
        break;
    case CV_8UC4:
        image = QImage((const unsigned char*)mat.data, mat.cols, mat.rows, mat.step, QImage::Format_ARGB32);
        break;
    case CV_16UC4:
        image = QImage((const unsigned char*)mat.data, mat.cols, mat.rows, mat.step, QImage::Format_RGBA64);
        image = image.rgbSwapped(); // BRG转为RGB
        break;
    }
    return image;
}