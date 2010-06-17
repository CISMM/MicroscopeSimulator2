#include <ImageExportOptionsDialog.h>


ImageExportOptionsDialog
::ImageExportOptionsDialog(QWidget* parent) : QDialog(parent) {
  setupUi(this);
}


ImageExportOptionsDialog
::~ImageExportOptionsDialog() {

}


bool
ImageExportOptionsDialog
::IsExportRedEnabled() {
  return redChannelCheckBox->isChecked();
}


bool
ImageExportOptionsDialog
::IsExportGreenEnabled() {
  return greenChannelCheckBox->isChecked();
}


bool
ImageExportOptionsDialog
::IsExportBlueEnabled() {
  return blueChannelCheckBox->isChecked();
}


int
ImageExportOptionsDialog
::GetNumberOfCopies() {
  return QVariant(numberEdit->text()).toInt();
}
