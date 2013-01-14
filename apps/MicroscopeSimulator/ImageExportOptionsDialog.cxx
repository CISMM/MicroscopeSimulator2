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


bool
ImageExportOptionsDialog
::IsRegenerateFluorophoresEnabled() {
  return regenerateFluorophoresButton->isChecked();
}


bool
ImageExportOptionsDialog
::IsRandomizeObjectPositionsEnabled() {
  return randomizeObjectPositions->isChecked();
}


bool
ImageExportOptionsDialog
::IsRandomizeStagePositionEnabled() {
  return randomizeStagePosition->isChecked();
}


double
ImageExportOptionsDialog
::GetObjectRandomPositionRangeX() {
  return QVariant(randomPositionRangeXEdit->text()).toDouble();
}


double
ImageExportOptionsDialog
::GetObjectRandomPositionRangeY() {
  return QVariant(randomPositionRangeYEdit->text()).toDouble();
}


double
ImageExportOptionsDialog
::GetObjectRandomPositionRangeZ() {
  return QVariant(randomPositionRangeZEdit->text()).toDouble();
}
