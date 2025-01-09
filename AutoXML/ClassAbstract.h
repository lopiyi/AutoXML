#pragma once
#include <QString>
#include <opencv2/opencv.hpp>
#include "yolo.hpp"

struct Data
{
	Data() {}
	QString picPath;
	QString anaPath;
	QString name;
	int index;
	QString imgsuf;
	int min, max;
};

class ClassAbstract
{
public:
	ClassAbstract() {}
	virtual ~ClassAbstract() {}
	virtual QString getName() = 0;
	//virtual void setYolo(std::shared_ptr<yolo::Infer> yolo) = 0;
    virtual cv::Mat detect(const Data& m_data) = 0;
	virtual void setting() = 0;
	virtual void reset() = 0;
private:

};
