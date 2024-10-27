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

#include "familymembereditdialog.h"

#include "ui_familymembereditdialog.h"

FamilyMemberEditDialog::FamilyMemberEditDialog(QWidget* parent) : QDialog(parent), ui(new Ui::FamilyMemberEditDialog) {
  ui->setupUi(this);

  connect(ui->buttonBox, &QDialogButtonBox::accepted, this, &FamilyMemberEditDialog::onDone);
}

FamilyMemberEditDialog::~FamilyMemberEditDialog() { delete ui; }

void FamilyMemberEditDialog::show(const QString& title, const FamilyMember& member, DoneCallback cb) {
  setWindowTitle(title);

  m_doneCallback = cb;
  setMemberToUi(member);

  ui->titleEdit->setFocus();
  QDialog::show();
}

void FamilyMemberEditDialog::onDone() {
  Q_ASSERT(m_doneCallback);
  FamilyMember member = getMemberFromUi();
  m_doneCallback(member);
}

void FamilyMemberEditDialog::setMemberToUi(const FamilyMember& member) {
  m_member = member;
  ui->titleEdit->setText(member.title);
  ui->nameEdit->setText(member.name);
  ui->spouseNameEdit->setText(member.spouseName);
  ui->maleRadioButton->setChecked(member.isMale);
  ui->aliveCheckbox->setChecked(member.isAlive);
  ui->spouseAliveCheckbox->setChecked(member.isSpouseAlive);
  ui->noteEdit->setText(member.note);
}

FamilyMember FamilyMemberEditDialog::getMemberFromUi() {
  FamilyMember member = m_member;
  member.title = ui->titleEdit->text();
  member.name = ui->nameEdit->text();
  member.spouseName = ui->spouseNameEdit->text();
  member.isMale = ui->maleRadioButton->isChecked();
  member.isAlive = ui->aliveCheckbox->isChecked();
  member.isSpouseAlive = ui->spouseAliveCheckbox->isChecked();
  member.note = ui->noteEdit->toPlainText();
  return member;
}
