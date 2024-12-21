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

#include "familymember.h"

#include <QJsonArray>

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
