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

#include "familymemberitem.h"

#include <QGraphicsScene>
#include <QGraphicsSceneEvent>
#include <QMenu>

#include "familytreescene.h"

static void centerIn(QGraphicsItem* item, const QRect& rect) {
  Q_ASSERT(item);
  if (item == nullptr) {
    return;
  }
  int x = rect.x() + (rect.width() - item->boundingRect().width()) / 2;
  int y = rect.y() + (rect.height() - item->boundingRect().height()) / 2;
  item->setPos(x, y);
}

static QString verticalText(const QString& text) {
  QString result;
  std::u32string u32s = text.toStdU32String();
  for (char32_t c : u32s) {
    result += QString::fromUcs4(&c, 1);
    result += "\n";
  }
  return result.trimmed();
}

FamilyMemberItem::FamilyMemberItem(FamilyTreeScene* scene, const FamilyMember& member, QGraphicsItem* parent,
                                   const QPen& pen)
    : QGraphicsPathItem(parent),
      m_scene(scene),
      m_titleItem(new QGraphicsTextItem(this)),
      m_nameItem(new QGraphicsTextItem(this)),
      m_spouseNameItem(new QGraphicsTextItem(this)),
      m_noteItem(new QGraphicsTextItem(this)) {
  Q_ASSERT(member.isValid());
  m_id = member.id;
  m_name = member.name;
  setFlag(QGraphicsItem::ItemIsSelectable, true);

  setPen(pen);
  m_titleItem->setDefaultTextColor(pen.color());
  m_nameItem->setDefaultTextColor(pen.color());
  m_spouseNameItem->setDefaultTextColor(pen.color());
  m_noteItem->setDefaultTextColor(pen.color());

  update(member);
}

FamilyMemberItem::~FamilyMemberItem() {}

QString FamilyMemberItem::id() const { return m_id; }

void FamilyMemberItem::update(const FamilyMember& member) {
  QRect titleRect;
  QRect nameRect;
  QRect spouseNameRect;
  QRect noteRect;

  bool hasNote = member.note != "";
  bool hasSpouse = member.spouseName != "";
  int nameHeight = kItemHeight - kTitleHeight;
  if (hasNote) {
    nameHeight -= kNoteHeight;
  }

  titleRect = QRect(0, 0, kItemWidth, kTitleHeight);
  if (hasSpouse) {
    int nameWidth = kItemWidth / 2;
    nameRect = QRect(0, kTitleHeight, nameWidth, nameHeight);
    spouseNameRect = QRect(nameWidth, kTitleHeight, nameWidth, nameHeight);
  } else {
    int nameWidth = kItemWidth;
    nameRect = QRect(0, kTitleHeight, nameWidth, nameHeight);
  }
  if (hasNote) {
    noteRect = QRect(0, kTitleHeight + nameHeight, kItemWidth, kNoteHeight);
  }

  QPainterPath path;
  path.addRect(titleRect);
  path.addRect(nameRect);
  path.addRect(spouseNameRect);
  path.addRect(noteRect);
  setPath(path);

  m_titleItem->setPlainText(member.title);
  m_nameItem->setPlainText(verticalText(member.name));
  m_spouseNameItem->setPlainText(verticalText(member.spouseName));
  m_noteItem->setPlainText(member.note);

  QFont font = m_titleItem->font();
  font.setFamily("楷体");
  font.setPointSize(20);
  QFont smallFont = font;
  smallFont.setPointSize(16);
  QFont noteFont = font;
  noteFont.setPointSize(10);

  int nameSize = member.name.toUcs4().size();
  int spouseNameSize = member.spouseName.toUcs4().size();

  m_titleItem->setFont(font);
  m_nameItem->setFont((nameSize > 4 || (nameSize == 4 && member.note != "")) ? smallFont : font);
  m_spouseNameItem->setFont((spouseNameSize > 4 || (spouseNameSize == 4 && member.note != "")) ? smallFont : font);
  m_noteItem->setFont(noteFont);

  centerIn(m_titleItem, titleRect);
  centerIn(m_nameItem, nameRect);
  centerIn(m_spouseNameItem, spouseNameRect);
  centerIn(m_noteItem, noteRect);
}

qreal FamilyMemberItem::subTreeBeginX() const {
  qreal itemX = this->x();
  return itemX - (m_subTreeWidth - boundingRect().width()) / 2;
}

void FamilyMemberItem::contextMenuEvent(QGraphicsSceneContextMenuEvent* event) {
  scene()->clearSelection();
  setSelected(true);
  m_scene->itemMenu()->popup(event->screenPos());
}

void FamilyMemberItem::mouseDoubleClickEvent(QGraphicsSceneMouseEvent* event) {
  scene()->clearSelection();
  setSelected(true);
  emit m_scene->itemDoubleClicked(this);
}

void FamilyMemberItem::mousePressEvent(QGraphicsSceneMouseEvent* event) {
  m_scene->onItemDragBegin(this, event);
  QGraphicsPathItem::mousePressEvent(event);
}

void FamilyMemberItem::mouseMoveEvent(QGraphicsSceneMouseEvent* event) {
  m_scene->onItemDragMoving(this, event);
  QGraphicsPathItem::mouseMoveEvent(event);
}

void FamilyMemberItem::mouseReleaseEvent(QGraphicsSceneMouseEvent* event) {
  m_scene->onItemDragDone(this, event);
  QGraphicsPathItem::mouseReleaseEvent(event);
}

qreal FamilyMemberItem::subTreeWidth() const { return m_subTreeWidth; }

void FamilyMemberItem::setSubTreeWidth(qreal newSubTreeWidth) { m_subTreeWidth = newSubTreeWidth; }

ArrowItem* FamilyMemberItem::inArrow() const { return m_inArrow; }

void FamilyMemberItem::setInArrow(ArrowItem* newInArrow) { m_inArrow = newInArrow; }
