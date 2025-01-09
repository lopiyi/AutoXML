#ifndef QIMAGEWIDGET_H
#define QIMAGEWIDGET_H

#pragma once
#include <QWidget>
#include <QtGui>
#include <QPixmap>
#include <QPainter>
#include <QRectF>
#include <QMouseEvent>
#include <QPointF>
#include <QDragEnterEvent>
#include <QGraphicsSceneWheelEvent>
#include <QGraphicsItem>
enum Enum_ZoomState {
    NO_STATE,
    RESET,
    ZOOM_IN,
    ZOOM_OUT
};
class QImageWidget :
    public QGraphicsItem
{
public:
    QImageWidget(QPixmap* pixmap);
    QRectF  boundingRect() const;
    void    paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget);
    void    wheelEvent(QGraphicsSceneWheelEvent* event);
    void    ResetItemPos();
    void    mousePressEvent(QGraphicsSceneMouseEvent* event);
    void    mouseMoveEvent(QGraphicsSceneMouseEvent* event);
    void    mouseReleaseEvent(QGraphicsSceneMouseEvent* event);
    qreal   getScaleValue() const;
    void    setQGraphicsViewWH(int nwidth, int nheight);

    void    ini(QPixmap* pixmap);
private:
    qreal       m_scaleValue;
    qreal       m_scaleDafault;
    QPixmap     m_pix;
    int         m_zoomState;
    bool        m_isMove;
    QPointF     m_startPos;
};

#endif // QIMAGEWIDGET_H
