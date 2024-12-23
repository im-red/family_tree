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

#include <QGraphicsScene>
#include <QMainWindow>
#include <QMessageBox>

#include "family.h"
#include "familymembereditdialog.h"
#include "familytreescene.h"

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow {
  Q_OBJECT

 public:
  MainWindow(QWidget* parent = nullptr);
  ~MainWindow();

  QString currentFilePath() const;
  void setCurrentFilePath(const QString& newCurrentFilePath);

 signals:
  void currentFilePathChanged();

 protected:
  void closeEvent(QCloseEvent* e) override;

 private:
  void onLoad(bool bypassPromptSave = false);
  void onSave();
  void onExport();

  void onAddChild();
  void onEdit();

  void doLoad(const QString& path, Family* family);
  void doSave(const QString& path, Family* family);

  QMessageBox::StandardButton promptSave();
  void updateWindowTitle();

 private:
  Ui::MainWindow* ui;
  FamilyMemberEditDialog* m_memberEditDialog = nullptr;
  QMenu* m_itemMenu = nullptr;
  QAction* m_addChildAction = nullptr;

  FamilyTreeScene* m_scene = nullptr;
  std::unique_ptr<Family> m_family = nullptr;

  QString m_currentFilePath;
};
