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

#include "mainwindow.h"

#include <QCloseEvent>
#include <QFileDialog>
#include <QMessageBox>

#include "familytreescene.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent),
      ui(new Ui::MainWindow),
      m_memberEditDialog(new FamilyMemberEditDialog),
      m_itemMenu(new QMenu(this)),
      m_addChildAction(new QAction(this)),
      m_scene(new FamilyTreeScene(m_itemMenu, this)) {
  ui->setupUi(this);
  ui->graphicsView->setScene(m_scene);
  ui->graphicsView->setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform);

  m_addChildAction->setText("Add child");
  m_itemMenu->addAction(m_addChildAction);

  connect(m_addChildAction, &QAction::triggered, this, &MainWindow::onAddChild);
  connect(ui->actionLoad, &QAction::triggered, this, &MainWindow::onLoad);
  connect(ui->actionSave, &QAction::triggered, this, &MainWindow::onSave);
  connect(m_scene, &FamilyTreeScene::itemDoubleClicked, this, &MainWindow::onEdit);

  connect(this, &MainWindow::currentFilePathChanged, this, &MainWindow::updateWindowTitle);

  doLoad("", new Family);
}

MainWindow::~MainWindow() { delete ui; }

void MainWindow::onLoad(bool bypassPromptSave) {
  qDebug() << bypassPromptSave;
  Q_ASSERT(m_family);
  Q_ASSERT(m_family->isValid());
  if (!bypassPromptSave && m_family && m_family->isDirty()) {
    QMessageBox::StandardButton button = promptSave();
    if (button == QMessageBox::Save) {
      onSave();
    } else if (button == QMessageBox::Discard) {
      onLoad(true);
    }
    return;
  }
  QString path = QFileDialog::getOpenFileName(this, tr("Load File"), "", tr("*.json"));
  qDebug() << path;
  if (path == "") {
    return;
  }
  QFile file(path);
  bool ret = file.open(QFile::ReadOnly);
  Q_ASSERT(ret);
  if (!ret) {
    return;
  }
  QString json = file.readAll();
  Family* family = Family::fromJson(json);
  if (!family || !family->isValid()) {
    qDebug().noquote() << "not valid:" << json;
    return;
  }

  doLoad(path, family);
}

void MainWindow::onSave() {
  qDebug() << "";
  Q_ASSERT(m_family && m_family->isValid());
  QString path = m_currentFilePath;
  if (path == "") {
    path = QFileDialog::getSaveFileName(this, tr("Save File"), "", tr("*.json"));
  }
  if (path == "") {
    return;
  }
  doSave(path, m_family.get());
}

void MainWindow::onAddChild() {
  qDebug() << "";
  QString parentId = m_scene->selectedMemberId();
  Q_ASSERT(parentId != "");
  m_memberEditDialog->show("Add", FamilyMember(true),
                           [this, parentId](const FamilyMember& member) { m_family->addChild(parentId, member); });
}

void MainWindow::onEdit() {
  qDebug() << "";
  QString id = m_scene->selectedMemberId();
  Q_ASSERT(id != "");
  FamilyMember member = m_family->getMember(id);
  Q_ASSERT(member.isValid());
  m_memberEditDialog->show("Edit", member, [this](const FamilyMember& member) { m_family->updateMember(member); });
}

void MainWindow::doLoad(const QString& path, Family* family) {
  qDebug() << "path:" << path;
  Q_ASSERT(family);
  Q_ASSERT(family->isValid());

  m_scene->setFamily(family);
  m_family.reset(family);
  setCurrentFilePath(path);
  connect(m_family.get(), &Family::isDirtyChanged, this, &MainWindow::updateWindowTitle);
  updateWindowTitle();
}

void MainWindow::doSave(const QString& path, Family* family) {
  qDebug() << "path:" << path;
  Q_ASSERT(family);
  Q_ASSERT(family->isValid());
  Q_ASSERT(path != "");

  QFile file(path);
  bool ret = file.open(QFile::WriteOnly | QFile::Truncate);
  Q_ASSERT(ret);
  if (!ret) {
    return;
  }
  QString json = m_family->toJson();
  file.write(json.toUtf8());
  file.close();

  qDebug().noquote() << json;

  setCurrentFilePath(path);
  family->setIsDirty(false);
}

QMessageBox::StandardButton MainWindow::promptSave() {
  QMessageBox::StandardButton result =
      QMessageBox::warning(this, "Save?", "File is modified, save changes?",
                           QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel, QMessageBox::Save);
  qDebug() << result;
  return result;
}

void MainWindow::updateWindowTitle() {
  QString title = QString("FamilyTree - %1").arg(m_currentFilePath);
  Q_ASSERT(m_family);
  if (m_family->isDirty()) {
    title += " *";
  }
  qDebug() << "title:" << title;
  setWindowTitle(title);
}

QString MainWindow::currentFilePath() const { return m_currentFilePath; }

void MainWindow::setCurrentFilePath(const QString& newCurrentFilePath) {
  if (m_currentFilePath == newCurrentFilePath) return;
  m_currentFilePath = newCurrentFilePath;
  emit currentFilePathChanged();
}

void MainWindow::closeEvent(QCloseEvent* e) {
  qDebug() << e;
  Q_ASSERT(m_family);
  if (!m_family->isDirty()) {
    QMainWindow::closeEvent(e);
    return;
  }
  e->ignore();
  QMessageBox::StandardButton button = promptSave();
  if (button == QMessageBox::Save) {
    onSave();
    QApplication::quit();
  } else if (button == QMessageBox::Discard) {
    QApplication::quit();
  }
}
