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

#include <QGraphicsScene>

#pragma once

class Family;
class FamilyMemberItem;
class QMenu;
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
