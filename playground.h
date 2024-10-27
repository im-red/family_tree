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

#include <QGraphicsPathItem>
#include <QGraphicsView>
#include <QString>
#include <QUuid>

#pragma once

struct FamilyMember {
  FamilyMember(bool doInit = false) {
    if (doInit) {
      id = QUuid::createUuid().toString();
    }
  }
  bool isValid() const { return id != ""; }

  QJsonObject toJson() const;
  static FamilyMember fromJson(const QJsonObject& o);

  QString id;
  QString title;
  QString name;
  QString spouseName;
  bool isMale = true;
  bool isAlive = false;
  bool isSpouseAlive = false;
  QString note;
  std::vector<QString> children;
  QString parentId;
  int indexAsChild = 0;

  void clearLayoutValue() {
    _layer = 0;
    _indexInSubTree = 0;
    _subTreeWidth = 1;
  }
  int _layer = 0;
  int _indexInSubTree = 0;
  int _subTreeWidth = 1;
};

class Family : public QObject {
  Q_OBJECT

 public:
  Family() {
    clear();
    setIsDirty(false);
  }

  bool isValid() const { return m_rootId != ""; }
  void clear();

  QString toJson() const;
  static Family* fromJson(const QString& json);

  QString rootId() const;
  void relayout();
  int updateSubTreeWidth(const QString& id);

  FamilyMember getMember(const QString& id);
  void updateMember(const FamilyMember& member);
  void addChild(const QString& parentId, const FamilyMember& child);

  bool isDirty() const;
  void setIsDirty(bool newIsDirty);

 signals:
  void cleared();
  void relayouted();
  void memberUpdated(const QString& id);
  void childAdded(const QString& parentId, const QString& childId);
  void childRemoved(const QString& parentId, const QString& childId);

  void isDirtyChanged();

 private:
  QString m_rootId;
  std::map<QString, FamilyMember> m_idToMember;

  bool m_isDirty = false;
};

class FamilyTreeScene;
class FamilyMemberItem : public QGraphicsPathItem {
 public:
  explicit FamilyMemberItem(FamilyTreeScene* scene, const FamilyMember& member, QGraphicsItem* parent = nullptr);

  QString id() const;
  void update(const FamilyMember& member);
  int subTreeBeginX() const;

  int subTreeWidth() const;
  void setSubTreeWidth(int newSubTreeWidth);

 protected:
  void contextMenuEvent(QGraphicsSceneContextMenuEvent* event) override;
  void mouseDoubleClickEvent(QGraphicsSceneMouseEvent* event) override;

 private:
  FamilyTreeScene* m_scene = nullptr;

  QString m_id;
  int m_subTreeWidth = 0;

  QGraphicsTextItem* m_titleItem = nullptr;
  QGraphicsTextItem* m_nameItem = nullptr;
  QGraphicsTextItem* m_spouseNameItem = nullptr;
  QGraphicsTextItem* m_noteItem = nullptr;
};

class FamilyTreeView : public QGraphicsView {
 public:
  explicit FamilyTreeView(QWidget* parent = nullptr);

 protected:
  void wheelEvent(QWheelEvent* event) override;
  void mouseMoveEvent(QMouseEvent* event) override;

 private:
  qreal m_scale = 1.0;
};

class FamilyTreeScene : public QGraphicsScene {
  Q_OBJECT

 public:
  FamilyTreeScene(QMenu* itemMenu, QObject* parent = nullptr);

  void setFamily(Family* family);

  FamilyMemberItem* getItem(const QString& id);
  QString selectedMemberId() const;

  QMenu* itemMenu() const;

 signals:
  void itemDoubleClicked(FamilyMemberItem* item);

 private:
  void onMemberUpdated(const QString& id);
  void onRelayouted();

  void addMemberItem(FamilyMemberItem* item);

 private:
  QMenu* m_itemMenu = nullptr;
  Family* m_family = nullptr;
  std::map<QString, FamilyMemberItem*> m_idToItem;
};
