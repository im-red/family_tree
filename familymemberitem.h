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

#include <QGraphicsPathItem>

#include "familymember.h"

#pragma once

constexpr int kItemWidth = 200;
constexpr int kItemHeight = 280;
constexpr int kTitleHeight = 80;
constexpr int kNoteHeight = 20;
constexpr int kItemVSpace = 40;
constexpr int kItemHSpace = 40;
constexpr int kArrowSize = 8;

class FamilyTreeScene;
class ArrowItem;
class FamilyMemberItem : public QGraphicsPathItem {
 public:
  explicit FamilyMemberItem(FamilyTreeScene* scene, const FamilyMember& member, QGraphicsItem* parent = nullptr);

  QString id() const;
  QString name() const { return m_name; }

  void update(const FamilyMember& member);
  qreal subTreeBeginX() const;

  qreal subTreeWidth() const;
  void setSubTreeWidth(qreal newSubTreeWidth);

  ArrowItem* inArrow() const;
  void setInArrow(ArrowItem* newInArrow);

  int width() const { return boundingRect().width(); }
  int height() const { return boundingRect().height(); }

 protected:
  void contextMenuEvent(QGraphicsSceneContextMenuEvent* event) override;
  void mouseDoubleClickEvent(QGraphicsSceneMouseEvent* event) override;
  void mousePressEvent(QGraphicsSceneMouseEvent* event) override;

 private:
  FamilyTreeScene* m_scene = nullptr;

  QString m_id;
  QString m_name;
  qreal m_subTreeWidth = 0;

  ArrowItem* m_inArrow = nullptr;

  QGraphicsTextItem* m_titleItem = nullptr;
  QGraphicsTextItem* m_nameItem = nullptr;
  QGraphicsTextItem* m_spouseNameItem = nullptr;
  QGraphicsTextItem* m_noteItem = nullptr;
};
