/*********************************************************************************
 * MIT License
 *
 * Copyright (c) 2023 Jia Lihong
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 ********************************************************************************/

#include "playground.h"

#include <QFont>
#include <QGraphicsSceneEvent>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QMenu>
#include <QUuid>
#include <QWheelEvent>

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
  QList<uint32_t> ucs4 = text.toUcs4();
  for (uint32_t u : ucs4) {
    result += QString::fromUcs4(&u, 1);
    result += "\n";
  }
  return result.trimmed();
}

FamilyMemberItem::FamilyMemberItem(FamilyTreeScene* scene, const FamilyMember& member, QGraphicsItem* parent)
    : QGraphicsPathItem(parent),
      m_scene(scene),
      m_titleItem(new QGraphicsTextItem(this)),
      m_nameItem(new QGraphicsTextItem(this)),
      m_spouseNameItem(new QGraphicsTextItem(this)),
      m_noteItem(new QGraphicsTextItem(this)) {
  Q_ASSERT(member.isValid());
  m_id = member.id;
  setFlag(QGraphicsItem::ItemIsSelectable, true);

  QFont font = m_titleItem->font();
  font.setPointSize(20);
  m_titleItem->setFont(font);
  m_nameItem->setFont(font);
  m_spouseNameItem->setFont(font);

  update(member);
}

QString FamilyMemberItem::id() const { return m_id; }

constexpr int kItemWidth = 200;
constexpr int kItemHeight = 280;
constexpr int kTitleHeight = 80;
constexpr int kNoteHeight = 20;
constexpr int kItemVSpace = 40;
constexpr int kItemHSpace = 40;

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

  centerIn(m_titleItem, titleRect);
  centerIn(m_nameItem, nameRect);
  centerIn(m_spouseNameItem, spouseNameRect);
  centerIn(m_noteItem, noteRect);
}

int FamilyMemberItem::subTreeBeginX() const {
  int itemX = this->x();
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

int FamilyMemberItem::subTreeWidth() const { return m_subTreeWidth; }

void FamilyMemberItem::setSubTreeWidth(int newSubTreeWidth) { m_subTreeWidth = newSubTreeWidth; }

FamilyTreeView::FamilyTreeView(QWidget* parent) : QGraphicsView(parent) {
  setMouseTracking(true);
  setDragMode(QGraphicsView::ScrollHandDrag);
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

QString Family::toJson() const {
  Q_ASSERT(isValid());
  if (!isValid()) {
    return "";
  }

  QJsonObject o;
  o["rootId"] = m_rootId;
  o["members"] = [this]() -> QJsonArray {
    QJsonArray a;
    for (const auto& pair : m_idToMember) {
      a.push_back(pair.second.toJson());
    }
    return a;
  }();

  return QJsonDocument(o).toJson();
}

Family* Family::fromJson(const QString& json) {
  QJsonDocument d = QJsonDocument::fromJson(json.toUtf8());
  if (!d.isObject()) {
    qDebug() << "not a valid json" << json;
    return nullptr;
  }

  Family* result = new Family;
  QJsonObject o = d.object();
  result->m_rootId = o["rootId"].toString();
  result->m_idToMember = [&o]() -> std::map<QString, FamilyMember> {
    std::map<QString, FamilyMember> result;
    QJsonValue v = o["members"];
    Q_ASSERT(v.isArray());
    if (!v.isArray()) {
      return result;
    }
    QJsonArray a = v.toArray();
    for (const QJsonValue& mv : a) {
      Q_ASSERT(mv.isObject());
      if (!mv.isObject()) {
        continue;
      }
      QJsonObject mo = mv.toObject();
      FamilyMember member = FamilyMember::fromJson(mo);
      Q_ASSERT(member.isValid());
      if (!member.isValid()) {
        continue;
      }
      result[member.id] = member;
    }
    return result;
  }();
  result->setIsDirty(false);
  return result;
}

void Family::addChild(const QString& parentId, const FamilyMember& child) {
  Q_ASSERT(child.isValid());
  Q_ASSERT(m_idToMember.count(parentId));
  Q_ASSERT(!m_idToMember.count(child.id));

  m_idToMember[child.id] = child;
  m_idToMember[child.id].parentId = parentId;
  m_idToMember[child.id].indexAsChild = m_idToMember[parentId].children.size();

  m_idToMember[parentId].children.push_back(child.id);

  relayout();
  setIsDirty(true);
}

bool Family::isDirty() const { return m_isDirty; }

void Family::setIsDirty(bool newIsDirty) {
  if (m_isDirty == newIsDirty) return;
  m_isDirty = newIsDirty;
  emit isDirtyChanged();
}

QString Family::rootId() const { return m_rootId; }

void Family::relayout() {
  Q_ASSERT(isValid());
  if (!isValid()) {
    return;
  }
  for (auto& pair : m_idToMember) {
    pair.second.clearLayoutValue();
  }
  int layer = 0;
  std::vector<QString> layerIds({m_rootId});
  while (layerIds.size() > 0) {
    std::vector<QString> nextLayerIds;
    for (const QString& id : layerIds) {
      Q_ASSERT(m_idToMember.count(id));
      if (!m_idToMember.count(id)) {
        qDebug() << "no id:" << id;
        continue;
      }
      FamilyMember& member = m_idToMember[id];
      Q_ASSERT(member.isValid());
      if (!member.isValid()) {
        qDebug() << "member is not valid:" << id;
        continue;
      }
      Q_ASSERT(member._layer == 0);
      member._layer = layer;
      // qDebug() << member.id << member.name << "layer:" << layer;
      nextLayerIds.insert(nextLayerIds.end(), member.children.begin(), member.children.end());
      // qDebug() << "==========";
    }
    layerIds.swap(nextLayerIds);
    layer++;
  }
  updateSubTreeWidth(m_rootId);
  emit relayouted();
}

int Family::updateSubTreeWidth(const QString& id) {
  Q_ASSERT(m_idToMember.count(id));
  FamilyMember& member = m_idToMember[id];
  Q_ASSERT(member.isValid());
  if (member.children.empty()) {
    member._subTreeWidth = 1;
    return 1;
  }
  int result = 0;
  for (const QString& childId : member.children) {
    result += updateSubTreeWidth(childId);
  }
  member._subTreeWidth = result;
  qDebug() << member.name << "subTreeWidth:" << result;
  return result;
}

FamilyMember Family::getMember(const QString& id) {
  if (m_idToMember.count(id)) {
    return m_idToMember[id];
  }
  return FamilyMember();
}

void Family::updateMember(const FamilyMember& member) {
  Q_ASSERT(member.isValid());
  m_idToMember[member.id] = member;
  emit memberUpdated(member.id);
  setIsDirty(true);
}

void Family::clear() {
  m_idToMember.clear();
  m_rootId = QUuid::createUuid().toString();
  FamilyMember member;
  member.id = m_rootId;
  m_idToMember[member.id] = member;
  emit cleared();
  setIsDirty(true);
}

QJsonObject FamilyMember::toJson() const {
  QJsonObject o;
  o["id"] = id;
  o["title"] = title;
  o["name"] = name;
  o["spouseName"] = spouseName;
  o["note"] = note;
  o["isMale"] = isMale;
  o["isAlive"] = isAlive;
  o["isSpouseAlive"] = isSpouseAlive;
  o["children"] = [this]() -> QJsonArray {
    QJsonArray a;
    for (const QString& child : children) {
      a.append(child);
    }
    return a;
  }();
  o["parentId"] = parentId;
  o["indexAsChild"] = indexAsChild;
  return o;
}

FamilyMember FamilyMember::fromJson(const QJsonObject& o) {
  FamilyMember result;
  result.id = o["id"].toString();
  result.title = o["title"].toString();
  result.name = o["name"].toString();
  result.spouseName = o["spouseName"].toString();
  result.note = o["note"].toString();
  result.isMale = o["isMale"].toBool();
  result.isAlive = o["isAlive"].toBool();
  result.isSpouseAlive = o["isSpouseAlive"].toBool();
  if (o["children"].isArray()) {
    QJsonArray a = o["children"].toArray();
    for (const auto& v : a) {
      result.children.push_back(v.toString());
    }
  }
  result.parentId = o["parentId"].toString();
  result.indexAsChild = o["indexAsChild"].toInt();
  return result;
}

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

      int totalWidth = member._subTreeWidth * (kItemWidth + kItemHSpace) - kItemHSpace;
      item->setSubTreeWidth(totalWidth);

      if (member.parentId != curParentId) {
        curParentId = member.parentId;
        layoutedChildrenWidth = 0;
      }

      int subTreeBeginX = [this, &member]() {
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

      int beginX = subTreeBeginX + layoutedChildrenWidth;
      item->setY(member._layer * (kItemHeight + kItemVSpace));
      item->setX(beginX + (totalWidth - item->boundingRect().width()) / 2);
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
