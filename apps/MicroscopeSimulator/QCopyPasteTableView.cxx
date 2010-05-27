#include <QCopyPasteTableView.h>

#include <QApplication>
#include <QClipboard>
#include <QKeyEvent>

#include <iostream>


QCopyPasteTableView
::QCopyPasteTableView(QWidget* parent) 
  : QTableView(parent) {
}


QCopyPasteTableView
::~QCopyPasteTableView() {
}


void
QCopyPasteTableView
::keyPressEvent(QKeyEvent* event) {
  Qt::KeyboardModifiers modifiers = event->modifiers();
  if (modifiers == Qt::ControlModifier) {
    Qt::Key key = (Qt::Key) event->key();
    if (key == Qt::Key_C) {
      copy();
    } else if (key == Qt::Key_V) {
      paste();
    }
  }
}


void
QCopyPasteTableView
::copy() {
  QItemSelectionModel* selection = selectionModel();
  QModelIndexList indices = selection->selectedIndexes();
  QAbstractItemModel *model = this->model();
  QStringList valueStrings;
  for (int i = 0; i < indices.length(); i++) {
    QVariant variant = model->data(indices[i], Qt::DisplayRole);
    valueStrings.append(variant.toString());
  }

  QClipboard* clipboard = QApplication::clipboard();
  clipboard->setText(valueStrings.join("\n"));
}


void
QCopyPasteTableView
::paste() {
  // Split strings pasted in. Assumes new rows are separated by newlines.
  QClipboard* clipboard = QApplication::clipboard();
  QStringList valueStrings = clipboard->text().split(QRegExp("\\s+"));

  QModelIndex startIndex = currentIndex();
  int numModelValues = model()->rowCount() - startIndex.row();
  int numStringValues = valueStrings.length();

  QAbstractItemModel *model = this->model();

  int count = (numModelValues < numStringValues) ? numModelValues : numStringValues;
  for (int i = 0; i < count; i++) {
    QVariant variant(valueStrings[i]);
    QModelIndex index = model->index(i+startIndex.row(), 1);
    model->setData(index, variant);
  }


}
