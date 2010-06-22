#ifndef _IMAGE_EXPORT_OPTIONS_DIALOG_H_
#define _IMAGE_EXPORT_OPTIONS_DIALOG_H_

#include <ui_ImageExportOptionsDialog.h>
#include <QDialog>


class ImageExportOptionsDialog : public QDialog,
  private Ui_ImageExportOptionsDialog {
Q_OBJECT

 public:
  ImageExportOptionsDialog(QWidget* parent=0);
  virtual ~ImageExportOptionsDialog();

  bool IsExportRedEnabled();
  bool IsExportGreenEnabled();
  bool IsExportBlueEnabled();

  int GetNumberOfCopies();

};

#endif // _IMAGE_EXPORT_OPTIONS_DIALOG_H_
