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

#pragma once

#include <QObject>

#include "familymember.h"

class Family : public QObject {
  Q_OBJECT

 public:
  Family() {
    clear();
    setIsDirty(false);
  }

  bool isValid() const { return m_rootId != ""; }
  int size() const { return m_idToMember.size(); }
  void clear();

  QString toJson() const;
  static Family* fromJson(const QString& json);

  QString title() const;
  QString rootId() const;
  void relayout();
  int updateSubTreeWidth(const QString& id);

  FamilyMember getMember(const QString& id);
  QString getParentId(const QString& id);

  void updateTitle(const QString& title);
  void updateMember(const FamilyMember& member);
  void reorderChildren(const QString& parentId, const std::vector<QString>& children);
  void addChild(const QString& parentId, const FamilyMember& child);

  bool isDirty() const;
  void setIsDirty(bool newIsDirty);

 signals:
  void titleUpdated();
  void relayouted();
  void memberUpdated(const QString& id);

  void isDirtyChanged();

 private:
  QString m_rootId;
  QString m_title;
  std::map<QString, FamilyMember> m_idToMember;

  bool m_isDirty = false;
};
