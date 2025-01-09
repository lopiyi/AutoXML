#include "showwindow.h"
#include <qdebug.h>
#include <QStyleOption>

#include <QAction>


using namespace cv;
using namespace std;

ShowWindow1::ShowWindow1(QWidget* parent)
	: QWidget(parent)
{
	installEventFilter(this);
	
}

ShowWindow1::~ShowWindow1()
{}

void ShowWindow1::setImage(cv::Mat& src)
{
	static bool first = true;
	QMutexLocker lock(&m_mutex);
	if (src.empty())
	{
		return;
	}
    m_DisplayImage = QPixmap::fromImage(MatToQImage(src));
	if (first)
	{
		fitWindow();
		first = false;
		return;
	}
	update();
}

void ShowWindow1::setImage(QPixmap&src)
{
	static bool first = true;
	QMutexLocker lock(&m_mutex);
	if (src.isNull())
	{
		return;
	}
	m_DisplayImage = src;
	if (first)
	{
		fitWindow();
		first = false;
		return;
	}
	update();
}

void ShowWindow1::setImage(QPixmap* src)
{
	static bool first = true;
	QMutexLocker lock(&m_mutex);
	if (src->isNull())
	{
		return;
	}
	m_DisplayImage = *src;
	if (first)
	{
		fitWindow();
		first = false;
		return;
	}
	update();
}

void ShowWindow1::paintEvent(QPaintEvent* event)
{
	if (m_DisplayImage.isNull())
	{
		return;
	}
	QPainter  p(this);
	p.setRenderHint(QPainter::SmoothPixmapTransform);
	QTransform tran = transform;
	p.setTransform(tran.translate(m_move_x, m_move_y));
	p.drawPixmap(0, 0, m_DisplayImage);
	//p.setPen(QColor("green")); //设置画笔记颜色
	//p.drawRect(0, 0, width() - 1, height() - 1); //绘制边框
	//QStyleOption opt;
	//opt.initFrom(this);
	//style()->drawPrimitive(QStyle::PE_Widget, &opt, &p, this);//绘制样式
	//p.drawRect(rect());
}

void ShowWindow1::mousePressEvent(QMouseEvent* event)
{
	if (event->button() == Qt::LeftButton)
	{
		m_mousePressPoint = event->pos();
		m_old_move_x = m_move_x;
		m_old_move_y = m_move_y;
		mapPos1 = transform.inverted().map(m_mousePressPoint);
	}
	return QWidget::mouseMoveEvent(event);
}

void ShowWindow1::mouseMoveEvent(QMouseEvent* event)
{
	if (event->buttons() == Qt::LeftButton)
	{
		auto m_mouseMovePoint = event->pos();
		QPointF mapPos2 = transform.inverted().map(event->pos());
		auto bias = mapPos2 - mapPos1;
		m_move_x = bias.x() + m_old_move_x;
		m_move_y = bias.y() + m_old_move_y;
		update();
	}
}

void ShowWindow1::wheelEvent(QWheelEvent* event)
{
	//得到滚轮转动的数值
	QPoint numDegrees = event->angleDelta() / 8;
	double ss = numDegrees.y() > 0 ? 1.2 : 1 / 1.2;
	mapPos1 = transform.inverted().map(event->position());
	transform.scale(ss, ss);
	QPointF mapPos2 = transform.inverted().map(event->position());
	auto bias = mapPos2 - mapPos1;
	transform.translate(bias.x(), bias.y());
	update();
}

bool ShowWindow1::eventFilter(QObject* obj, QEvent* event)
{
	if (event->type() == QEvent::MouseButtonDblClick)//当为双击事件时
	{
		static bool full = false;
		full = !full;
		if (full) //此处为双击一次全屏，再双击一次退出
		{
			setWindowFlags(Qt::Dialog);
			showFullScreen();//全屏显示
		}
		else
		{
			setWindowFlags(Qt::SubWindow);
			showNormal();//退出全屏
		};
		fitWindow();
	}
	return QObject::eventFilter(obj, event);
}

void ShowWindow1::fitWindow()
{
	m_move_x = 0;
	m_move_y = 0;
	m_old_move_x = 0;
	m_old_move_y = 0;
	//得到控件大小
	auto m_nw = this->width();
	auto m_nh = this->height();
	//得到图片大小
	auto m_niw = m_DisplayImage.width();
	auto m_nih = m_DisplayImage.height();

	//计算控件和图片的比例
	auto m_dScaleW = (double)m_nw / (double)m_niw;
	auto m_dScaleH = (double)m_nh / (double)m_nih;
	//取小比例，保证图片完全显示在控件内
	auto m_scale = qMin(m_dScaleW, m_dScaleH);
	//放大或者缩小图片

	m_niw *= m_scale;
	m_nih *= m_scale;
	//调整图片在控件出现的位置
	auto m_adjust_x = -(m_niw - m_nw) / 2;
	auto m_adjust_y = -(m_nih - m_nh) / 2;
	QTransform transform_tem;
	transform_tem.translate(m_adjust_x, m_adjust_y).scale(m_scale, m_scale);
	transform = transform_tem;
	update();
}

QImage ShowWindow1::MatToQImage(const cv::Mat& mat)
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

