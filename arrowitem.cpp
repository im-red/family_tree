/*********************************************************************************
 * MIT License
 *
 * Copyright (c) 2024 Jia Lihong
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 ********************************************************************************/

#include "arrowitem.h"

#include "familymemberitem.h"

ArrowItem::ArrowItem(FamilyMemberItem* startItem, FamilyMemberItem* endItem, QGraphicsItem* parent)
    : QGraphicsPathItem(parent), m_startItem(startItem), m_endItem(endItem) {
  Q_ASSERT(m_startItem);
  Q_ASSERT(m_endItem);
  m_endItem->setInArrow(this);
  setFlag(QGraphicsItem::ItemIsSelectable, true);
}

void ArrowItem::updatePosition() {
  QPointF topLeft;
  QPointF bottomRight;
  if (m_startItem->x() <= m_endItem->x()) {
    topLeft = mapFromItem(m_startItem, m_startItem->width() / 2, m_startItem->height());
    bottomRight = mapFromItem(m_endItem, m_endItem->width() / 2, 0);
  } else {
    QPointF topRight = mapFromItem(m_startItem, m_startItem->width() / 2, m_startItem->height());
    QPointF bottomLeft = mapFromItem(m_endItem, m_endItem->width() / 2, 0);
    topLeft = QPointF(bottomLeft.x(), topRight.y());
    bottomRight = QPointF(topRight.x(), bottomLeft.y());
  }
  qDebug() << ">>>>>>>>>>" << topLeft << bottomRight << bottomRight.y() - topLeft.y();
  QPainterPath path;
  path.addRect(QRectF(topLeft, bottomRight));
  setPath(path);
}

void ArrowItem::mousePressEvent(QGraphicsSceneMouseEvent* event) {
  qDebug() << ">>>>>>>>>>" << m_startItem->name() << "->" << m_endItem->name() << pos() << boundingRect();
  QGraphicsPathItem::mousePressEvent(event);
}
