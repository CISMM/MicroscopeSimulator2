#include <FluorophoreModelDialog.h>
#include <FluorophoreModelObjectProperty.h>


FluorophoreModelDialog
::FluorophoreModelDialog(FluorophoreModelObjectProperty* prop,
                         QWidget* parent) : QDialog(parent) {
  setupUi(this);

  SetProperty(prop);

  m_DoubleFormatString = "%.3f";
}


FluorophoreModelDialog
::~FluorophoreModelDialog() {
}


void
FluorophoreModelDialog
::SetProperty(FluorophoreModelObjectProperty* property) {
  m_FluorophoreProperty = property;

  // Setup the UI to reflect the property values.
  gui_EnabledCheckBox->
    setCheckState(property->GetEnabled() ? Qt::Checked : Qt::Unchecked);

  int colorChannelIndex = 0;
  switch (property->GetFluorophoreChannel()) {
  case RED_CHANNEL:
    colorChannelIndex = 0;
    break;

  case GREEN_CHANNEL:
    colorChannelIndex = 1;
    break;

  case BLUE_CHANNEL:
    colorChannelIndex = 2;
    break;

  case ALL_CHANNELS:
    colorChannelIndex = 3;
    break;
  }
  gui_ColorChannelComboBox->setCurrentIndex(colorChannelIndex);


  FluorophoreModelType type = property->GetFluorophoreModelType();

  if (type == GEOMETRY_VERTICES) {

    gui_GroupBox->setTitle(tr("Fixed Points Fluorophore Model"));
    gui_AreaLabel->setHidden(true);
    gui_AreaEdit->setHidden(true);
    gui_DensityLabel->setHidden(true);
    gui_DensityEdit->setHidden(true);
    gui_NumberOfFluorophoresLabel->setHidden(true);
    gui_NumberOfFluorophoresEdit->setHidden(true);
    gui_NumberOfFluorophoresSlider->setHidden(true);

  } else if (type == UNIFORM_RANDOM_SURFACE_SAMPLE) {

    gui_GroupBox->setTitle(tr("Uniform Random Surface Labeling"));
    gui_AreaLabel->setText(tr("Surface area (fluorophores/micron^2)"));
    double area = property->GetGeometryArea() * 1.0e-6;
    gui_AreaEdit->setText(QString().sprintf("%.6f", area));
    gui_DensityLabel->setText(tr("Density (fluorophores/micron^2)"));
    gui_DensityEdit->setText(QVariant(property->GetDensity()).toString());

    int numFluorophores = DensityToNumberOfFluorophores(property->GetDensity());
    gui_NumberOfFluorophoresSlider->setValue(numFluorophores);
    gui_NumberOfFluorophoresEdit->setText(QVariant(numFluorophores).toString());

  } else if (type == UNIFORM_RANDOM_VOLUME_SAMPLE) {

    gui_GroupBox->setTitle(tr("Uniform Random Volume Labeling"));
    gui_AreaLabel->setText(tr("Volume (fluorophores/micron^3)"));
    double volume = property->GetGeometryVolume() * 1.0e-9;
    gui_AreaEdit->setText(QString().sprintf("%.6f", volume));
    gui_DensityLabel->setText(tr("Density (fluorophores / micron^3)"));
    gui_DensityEdit->setText(QVariant(property->GetDensity()).toString());

    int numFluorophores = DensityToNumberOfFluorophores(property->GetDensity());
    gui_NumberOfFluorophoresSlider->setValue(numFluorophores);
    gui_NumberOfFluorophoresEdit->setText(QVariant(numFluorophores).toString());
  }
}


void
FluorophoreModelDialog
::SaveSettingsToProperty() {
  m_FluorophoreProperty->SetDensity(GetDensity());
  m_FluorophoreProperty->SetEnabled(GetEnabled());
  m_FluorophoreProperty->SetFluorophoreChannel(GetFluorophoreChannel());
}


double
FluorophoreModelDialog
::GetDensity() {
  return gui_DensityEdit->text().toDouble();
}


bool
FluorophoreModelDialog
::GetEnabled() {
  return gui_EnabledCheckBox->checkState() == Qt::Checked ? true : false;
}


FluorophoreChannelType
FluorophoreModelDialog
::GetFluorophoreChannel() {
  switch (gui_ColorChannelComboBox->currentIndex()) {
  case 0: return RED_CHANNEL; break;
  case 1: return GREEN_CHANNEL; break;
  case 2: return BLUE_CHANNEL; break;
  case 3: return ALL_CHANNELS; break;
  default: return ALL_CHANNELS; break;
  }
}


void
FluorophoreModelDialog
::on_gui_DensityEdit_textEdited(const QString& string) {
  double value = string.toDouble();
  int fluorophores = DensityToNumberOfFluorophores(value);
  gui_NumberOfFluorophoresEdit->setText(QString().sprintf("%d", fluorophores));
  gui_NumberOfFluorophoresSlider->setValue(fluorophores);

}


void
FluorophoreModelDialog
::on_gui_NumberOfFluorophoresEdit_textEdited(const QString& string) {
  int value = string.toInt();
  double density = NumberOfFluorophoresToDensity(value);
  gui_DensityEdit->setText(QString().sprintf(m_DoubleFormatString.c_str(),
                                             density));
  gui_NumberOfFluorophoresSlider->setValue(value);
}


void
FluorophoreModelDialog
::on_gui_NumberOfFluorophoresSlider_sliderMoved(int value) {
  gui_NumberOfFluorophoresEdit->setText(QVariant(value).toString());

  // Set new density
  gui_DensityEdit->setText(QString().sprintf(m_DoubleFormatString.c_str(),
                                             NumberOfFluorophoresToDensity(value)));
}


int
FluorophoreModelDialog
::DensityToNumberOfFluorophores(double density) {
  FluorophoreModelType type = m_FluorophoreProperty->GetFluorophoreModelType();
  if (type == UNIFORM_RANDOM_SURFACE_SAMPLE) {
    // Scale area from square nanometers to square micrometers
    double area = m_FluorophoreProperty->GetGeometryArea() * 1e-6;
    return static_cast<int>(density * area);
  } else if (type == UNIFORM_RANDOM_VOLUME_SAMPLE) {
    // Scale area from cubic nanometers to cubic micrometers
    double volume = m_FluorophoreProperty->GetGeometryVolume() * 1e-9;
    return static_cast<int>(density * volume);
  }

  return 0;
}


double
FluorophoreModelDialog
::NumberOfFluorophoresToDensity(int fluorophores) {
  FluorophoreModelType type = m_FluorophoreProperty->GetFluorophoreModelType();
  if (type == UNIFORM_RANDOM_SURFACE_SAMPLE) {
    // Scale area from square nanometers to square micrometers
    double area = m_FluorophoreProperty->GetGeometryArea() * 1e-6;
    return static_cast<double>(fluorophores) / area;
  } else if (type == UNIFORM_RANDOM_VOLUME_SAMPLE) {
    // Scale area from cubic nanometers to cubic micrometers
    double volume = m_FluorophoreProperty->GetGeometryVolume() * 1e-9;
    return static_cast<double>(fluorophores) / volume;
  }

  return 0.0;
}
