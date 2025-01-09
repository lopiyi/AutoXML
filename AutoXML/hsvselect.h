#pragma once

#include "ui_hsvselect.h"
//#include "showwindow.h"
#include "picwindow.h"
#include <opencv2/opencv.hpp>
class HSVSelect : public QWidget
{
	Q_OBJECT

public:
	static HSVSelect* getInstance();
	void setImage(cv::Mat& src, bool update = false);
private slots:
    void on_pushButton_clicked();
    void on_pb_lower_clicked();
    void on_pb_upper_clicked();
    void on_pb_reset_clicked();
    void on_pb_yuan_clicked();
    void on_pb_jietu_clicked();

    void on_horizontalSlider_valueChanged(int value);

    void on_spinBox_valueChanged(int arg1);

    void on_horizontalSlider_2_valueChanged(int value);

    void on_spinBox_2_valueChanged(int arg1);

    void on_horizontalSlider_3_valueChanged(int value);

    void on_spinBox_3_valueChanged(int arg1);

    void on_horizontalSlider_4_valueChanged(int value);

    void on_spinBox_4_valueChanged(int arg1);

    void on_horizontalSlider_5_valueChanged(int value);

    void on_spinBox_5_valueChanged(int arg1);

    void on_horizontalSlider_6_valueChanged(int value);

    void on_spinBox_6_valueChanged(int arg1);
private:
    void PicShuaXin();
    QImage MatToQImage(const cv::Mat& mat);
	HSVSelect(QWidget* parent = nullptr);
	~HSVSelect();
	Ui::HSVSelectClass ui;
    PicWindow* sw;
	static HSVSelect* instance;
	cv::Mat image;

    QImage img, qimghsv;
    cv::Mat mask;
    cv::Scalar lower = cv::Scalar(0, 0, 0);
    cv::Scalar upper = cv::Scalar(0, 0, 0);
};
