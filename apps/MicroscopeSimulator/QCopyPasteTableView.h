#ifndef _Q_COPY_PASTE_TABLE_VIEW_H_
#define _Q_COPY_PASTE_TABLE_VIEW_H_


#include <QTableView>


class QCopyPasteTableView : public QTableView {
  Q_OBJECT

 public:
  QCopyPasteTableView(QWidget* parent = 0);
  virtual ~QCopyPasteTableView();

 protected:
  void keyPressEvent(QKeyEvent* event);

  void copy();
  void paste();

};


#endif // _Q_COPY_PASTE_TABLE_VIEW_H_
