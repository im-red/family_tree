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

#include "familytreeview.h"

#include <QWheelEvent>

FamilyTreeView::FamilyTreeView(QWidget* parent) : QGraphicsView(parent) {
  setMouseTracking(true);
  setDragMode(QGraphicsView::ScrollHandDrag);
  setStyleSheet("QGraphicsView { border: 1px solid gray }");
}

void FamilyTreeView::wheelEvent(QWheelEvent* event) {
  if (event->modifiers() & Qt::ControlModifier) {
    if (event->angleDelta().y() > 0) {
      m_scale *= 1.1;
    } else {
      m_scale *= 0.9;
      m_scale = std::max(m_scale, 0.1);
    }

    QTransform t;
    t.scale(m_scale, m_scale);
    setTransform(t);
  } else {
    QGraphicsView::wheelEvent(event);
  }
}

void FamilyTreeView::mouseMoveEvent(QMouseEvent* event) {
  setFocus();
  QGraphicsView::mouseMoveEvent(event);
}
