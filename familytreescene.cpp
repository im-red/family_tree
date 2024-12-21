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

#include "familytreescene.h"

#include <QGraphicsSceneMouseEvent>

#include "arrowitem.h"
#include "family.h"
#include "familymemberitem.h"
#include "familytitleitem.h"

FamilyTreeScene::FamilyTreeScene(QMenu* itemMenu, QObject* parent) : QGraphicsScene(parent), m_itemMenu(itemMenu) {
  resetItems();
}

FamilyMemberItem* FamilyTreeScene::getItem(const QString& id) {
  if (m_idToItem.count(id) == 0) {
    return nullptr;
  }
  FamilyMemberItem* item = m_idToItem[id];
  Q_ASSERT(item);
  return item;
}

QString FamilyTreeScene::selectedMemberId() const {
  QList<QGraphicsItem*> selected = selectedItems();
  if (selected.size() != 1) {
    return "";
  }
  FamilyMemberItem* item = dynamic_cast<FamilyMemberItem*>(selected[0]);
  Q_ASSERT(item);
  return item->id();
}

void FamilyTreeScene::onMemberUpdated(const QString& id) {
  FamilyMember member = m_family->getMember(id);
  Q_ASSERT(member.isValid());
  Q_ASSERT(m_idToItem.count(id));
  if (!m_idToItem.count(id)) {
    return;
  }
  FamilyMemberItem* item = m_idToItem[id];
  Q_ASSERT(item);
  if (item == nullptr) {
    return;
  }
  item->update(member);
}

void FamilyTreeScene::onRelayouted() {
  qDebug() << "";
  Q_ASSERT(m_family);
  Q_ASSERT(m_family->isValid());

  resetItems();

  std::vector<QString> layerIds({m_family->rootId()});
  while (layerIds.size() > 0) {
    QString curParentId;
    int layoutedChildrenWidth = 0;

    std::vector<QString> nextLayerIds;
    for (const QString& id : layerIds) {
      FamilyMember member = m_family->getMember(id);
      Q_ASSERT(member.isValid());
      nextLayerIds.insert(nextLayerIds.end(), member.children.begin(), member.children.end());
      FamilyMemberItem* item = new FamilyMemberItem(this, member);
      addMemberItem(item);

      qreal totalWidth = member._subTreeWidth * (kItemWidth + kItemHSpace) - kItemHSpace;
      item->setSubTreeWidth(totalWidth);

      if (member.parentId != curParentId) {
        curParentId = member.parentId;
        layoutedChildrenWidth = 0;
      }

      qreal subTreeBeginX = [this, &member]() -> qreal {
        QString parentId = member.parentId;
        if (parentId == "") {
          return 0;
        }
        FamilyMemberItem* item = getItem(parentId);
        if (item == nullptr) {
          return 0;
        }
        return item->subTreeBeginX();
      }();

      qreal beginX = subTreeBeginX + layoutedChildrenWidth;
      item->setY(member._layer * (kItemHeight + kItemVSpace));
      item->setX(beginX + (totalWidth - item->boundingRect().width()) / 2);
      if (item->inArrow()) {
        item->inArrow()->updatePosition();
      }
      // qDebug() << member.id << member.name << item->boundingRect();
      layoutedChildrenWidth += totalWidth + kItemHSpace;
    }
    layerIds.swap(nextLayerIds);
  }

  onTitleUpdated();
}

void FamilyTreeScene::onTitleUpdated() {
  m_titleItem->setPlainText(m_family->title());
  FamilyMemberItem* rootItem = rootMemberItem();
  Q_ASSERT(rootItem);
  m_titleItem->setY(rootItem->y() - m_titleItem->boundingRect().height() - 40);
  m_titleItem->setX(rootItem->x() - (m_titleItem->boundingRect().width() - rootItem->width()) / 2);
}

void FamilyTreeScene::onTitleEditDone() {
  Q_ASSERT(m_family);
  m_family->updateTitle(m_titleItem->toPlainText());
}

void FamilyTreeScene::addMemberItem(FamilyMemberItem* item) {
  Q_ASSERT(item && item->id() != "");
  m_idToItem[item->id()] = item;
  addItem(item);

  FamilyMemberItem* parentItem = parentMemberItem(item);
  if (parentItem == nullptr) {
    return;
  }

  ArrowItem* arrow = new ArrowItem(parentItem, item);
  addItem(arrow);
  arrow->updatePosition();
}

FamilyMemberItem* FamilyTreeScene::rootMemberItem() {
  Q_ASSERT(m_family);
  QString rootId = m_family->rootId();
  return m_idToItem[rootId];
}

FamilyMemberItem* FamilyTreeScene::parentMemberItem(FamilyMemberItem* item) {
  Q_ASSERT(item);
  QString id = item->id();
  Q_ASSERT(m_family);
  QString parentId = m_family->getParentId(id);
  if (parentId == "") {
    return nullptr;
  }
  return m_idToItem[parentId];
}

std::vector<FamilyMemberItem*> FamilyTreeScene::childrenMemberItem(FamilyMemberItem* item) {
  Q_ASSERT(item);
  QString id = item->id();
  Q_ASSERT(m_family);
  FamilyMember member = m_family->getMember(id);
  Q_ASSERT(member.isValid());
  std::vector<QString> children = member.children;
  std::vector<FamilyMemberItem*> result;
  for (const QString& child : children) {
    Q_ASSERT(child != "");
    FamilyMemberItem* childItem = m_idToItem[child];
    Q_ASSERT(childItem);
    result.push_back(childItem);
  }
  return result;
}

std::vector<FamilyMemberItem*> FamilyTreeScene::siblingsMemberItem(FamilyMemberItem* item) {
  Q_ASSERT(item);
  FamilyMemberItem* parentItem = parentMemberItem(item);
  if (parentItem == nullptr) {
    return {};
  }
  return childrenMemberItem(parentItem);
}

int FamilyTreeScene::indexInSiblings(FamilyMemberItem* item) {
  Q_ASSERT(item);
  std::vector<FamilyMemberItem*> siblings = siblingsMemberItem(item);
  if (siblings.size() <= 1) {
    return 0;
  }
  return std::find(siblings.begin(), siblings.end(), item) - siblings.begin();
}

void FamilyTreeScene::resetItems() {
  m_idToItem.clear();
  clear();

  m_movingIndicator = new FamilyMemberItem(this, FamilyMember(true), nullptr, kActiveColor);
  m_movingIndicator->setOpacity(0.3);
  m_movingIndicator->setZValue(100);
  m_movingIndicator->setVisible(false);
  addItem(m_movingIndicator);

  m_movingTargetIndicator = new QGraphicsPathItem;
  m_movingTargetIndicator->setPen(kActiveColor);
  m_movingTargetIndicator->setBrush(kActiveColor);
  m_movingTargetIndicator->setPath([]() {
    QPainterPath path;
    path.addRect(0, 0, 5, kItemHeight);
    return path;
  }());
  m_movingTargetIndicator->setVisible(false);
  addItem(m_movingTargetIndicator);

  m_titleItem = new FamilyTitleItem;
  m_titleItem->setFont([this]() {
    QFont font = m_titleItem->font();
    font.setFamily("楷体");
    font.setPointSize(40);
    return font;
  }());
  m_titleItem->setTextInteractionFlags(Qt::TextEditorInteraction);
  connect(m_titleItem, &FamilyTitleItem::editDone, this, &FamilyTreeScene::onTitleEditDone);
  addItem(m_titleItem);
}

QMenu* FamilyTreeScene::itemMenu() const { return m_itemMenu; }

void FamilyTreeScene::onItemDragBegin(FamilyMemberItem* item, QGraphicsSceneMouseEvent* event) {
  m_movingTargetNewIndex = -1;
  m_movingBeginPos = event->pos();
}

void FamilyTreeScene::onItemDragMoving(FamilyMemberItem* item, QGraphicsSceneMouseEvent* event) {
  m_movingTargetNewIndex = -1;
  Q_ASSERT(item);
  Q_ASSERT(event);
  FamilyMember member = m_family->getMember(item->id());
  Q_ASSERT(member.isValid());
  m_movingIndicator->update(member);
  m_movingIndicator->setPos(event->scenePos() - m_movingBeginPos);
  m_movingIndicator->setVisible(true);

  std::vector<FamilyMemberItem*> siblings = siblingsMemberItem(item);
  if (siblings.size() <= 1) {
    return;
  }

  int x = event->scenePos().x();
  for (const FamilyMemberItem* sibling : siblings) {
    if (x >= sibling->x() && x < (sibling->x() + sibling->width())) {
      m_movingTargetIndicator->setVisible(false);
      return;
    }
  }

  m_movingTargetIndicator->setY(siblings.front()->y());

  int oldIndex = indexInSiblings(item);
  if (x <= siblings.front()->x()) {
    m_movingTargetNewIndex = 0;
    m_movingTargetIndicator->setX(siblings.front()->x() - 15);
  } else if (x > siblings.back()->x() + siblings.back()->width()) {
    m_movingTargetNewIndex = siblings.size() - 1;
    m_movingTargetIndicator->setX(siblings.back()->x() + siblings.back()->width() + 10);
  } else {
    for (int i = 0; i < siblings.size() - 1; i++) {
      if (x >= (siblings[i]->x() + siblings[i]->width()) && x < siblings[i + 1]->x()) {
        m_movingTargetNewIndex = i + 1;
        if (m_movingTargetNewIndex > oldIndex) {
          m_movingTargetNewIndex--;
        }
        m_movingTargetIndicator->setX((siblings[i]->x() + siblings[i]->width() + siblings[i + 1]->x()) / 2 - 2.5);
      }
    }
  }
  if (m_movingTargetNewIndex == oldIndex) {
    m_movingTargetNewIndex = -1;
  }
  m_movingTargetIndicator->setVisible(true);
}

void FamilyTreeScene::onItemDragDone(FamilyMemberItem* item, QGraphicsSceneMouseEvent* event) {
  qDebug() << m_movingTargetNewIndex;
  m_movingIndicator->setVisible(false);
  m_movingTargetIndicator->setVisible(false);
  if (m_movingTargetNewIndex == -1) {
    return;
  }
  Q_ASSERT(item);
  QString id = item->id();
  Q_ASSERT(m_family);
  QString parentId = m_family->getParentId(id);
  Q_ASSERT(parentId != "");
  FamilyMember parent = m_family->getMember(parentId);
  Q_ASSERT(parent.isValid());
  std::vector<QString> children = parent.children;
  Q_ASSERT(children.size() > 1);
  auto iter = std::find(children.begin(), children.end(), id);
  Q_ASSERT(iter != children.end());
  children.erase(iter);
  children.insert(children.begin() + m_movingTargetNewIndex, id);
  m_family->reorderChildren(parentId, children);
}

void FamilyTreeScene::setFamily(Family* family) {
  if (m_family) {
    disconnect(m_family, nullptr, this, nullptr);
  }
  m_family = family;
  resetItems();

  if (m_family) {
    Q_ASSERT(m_family->isValid());
    connect(m_family, &Family::titleUpdated, this, &FamilyTreeScene::onTitleUpdated);
    connect(m_family, &Family::relayouted, this, &FamilyTreeScene::onRelayouted, Qt::QueuedConnection);
    connect(m_family, &Family::memberUpdated, this, &FamilyTreeScene::onMemberUpdated);
    m_family->relayout();
  }
}
