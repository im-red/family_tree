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

#include "family.h"

#include <QJsonArray>
#include <QJsonDocument>

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

QString Family::getParentId(const QString& id) { return getMember(id).parentId; }

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
