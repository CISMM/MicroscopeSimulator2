#include <ErrorLogDialog.h>


ErrorLogDialog
::ErrorLogDialog(QWidget* parent) : QDialog(parent) {
  setupUi(this);
}


ErrorLogDialog
::~ErrorLogDialog() {
}


QPlainTextEdit*
ErrorLogDialog
::GetPlainTextEdit() {
  return gui_ErrorLogTextEdit;
}
