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

#include "arrowitem.h"
#include "family.h"
#include "familymemberitem.h"

FamilyTreeScene::FamilyTreeScene(QMenu* itemMenu, QObject* parent) : QGraphicsScene(parent), m_itemMenu(itemMenu) {}

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

  m_idToItem.clear();
  clear();

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
}

void FamilyTreeScene::addMemberItem(FamilyMemberItem* item) {
  Q_ASSERT(item && item->id() != "");
  m_idToItem[item->id()] = item;
  addItem(item);
  Q_ASSERT(m_family);
  QString parentId = m_family->getParentId(item->id());
  if (parentId == "") {
    return;
  }
  FamilyMemberItem* parentItem = m_idToItem[parentId];
  Q_ASSERT(parentItem);
  ArrowItem* arrow = new ArrowItem(parentItem, item);
  addItem(arrow);
  arrow->updatePosition();
}

QMenu* FamilyTreeScene::itemMenu() const { return m_itemMenu; }

void FamilyTreeScene::setFamily(Family* family) {
  if (m_family) {
    disconnect(m_family, nullptr, this, nullptr);
  }
  m_family = family;
  m_idToItem.clear();
  clear();

  if (m_family) {
    Q_ASSERT(m_family->isValid());
    connect(m_family, &Family::relayouted, this, &FamilyTreeScene::onRelayouted);
    connect(m_family, &Family::memberUpdated, this, &FamilyTreeScene::onMemberUpdated);
    m_family->relayout();
  }
}
