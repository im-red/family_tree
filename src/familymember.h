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

#include <QJsonObject>
#include <QString>
#include <QUuid>

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
