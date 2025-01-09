#pragma once

#include <QWidget>
#include <QPixmap>
#include <QPainter>
#include <QMouseEvent>
#include <QMutex>
#include <opencv2/opencv.hpp>

class ShowWindow1 : public QWidget
{
	Q_OBJECT

public:
	ShowWindow1(QWidget* parent);
	~ShowWindow1();

	void setImage(cv::Mat& src);
	void setImage(QPixmap&src);
	void setImage(QPixmap* src);
protected:
	void paintEvent(QPaintEvent* event);
	void mousePressEvent(QMouseEvent* event);
	//void mouseReleaseEvent(QMouseEvent* event);
	void mouseMoveEvent(QMouseEvent* event);
	void wheelEvent(QWheelEvent* event);
	bool eventFilter(QObject* obj, QEvent* event) override;
private:
	void fitWindow();
	QImage MatToQImage(const cv::Mat& mat);
private:
	QTransform transform;
	QPixmap m_DisplayImage;
	QMutex m_mutex;
	QPoint m_mousePressPoint;
	QPointF mapPos1;
private:
	double m_move_x = 0;
	double m_move_y = 0;
	double m_old_move_x = 0;
	double m_old_move_y = 0;
};