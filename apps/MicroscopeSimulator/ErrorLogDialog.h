#ifndef _ERROR_LOG_DIALOG_H_
#define _ERROR_LOG_DIALOG_H_

#include <ui_ErrorLogDialog.h>
#include <QDialog.h>


class ErrorLogDialog : public QDialog, private Ui_ErrorLogDialog {
Q_OBJECT

 public:
  ErrorLogDialog(QWidget* parent=0);
  virtual ~ErrorLogDialog();

  QPlainTextEdit* GetPlainTextEdit();

};

#endif // _ERROR_LOG_DIALOG_H_
