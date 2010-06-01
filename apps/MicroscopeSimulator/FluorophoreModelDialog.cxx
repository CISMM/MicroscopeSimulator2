#include <FluorophoreModelDialog.h>
#include <GeometryVerticesFluorophoreProperty.h>
#include <SurfaceUniformFluorophoreProperty.h>
#include <VolumeUniformFluorophoreProperty.h>


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

  GeometryVerticesFluorophoreProperty* geometryProperty
    = dynamic_cast<GeometryVerticesFluorophoreProperty*>(property);
  SurfaceUniformFluorophoreProperty* surfaceProperty
    = dynamic_cast<SurfaceUniformFluorophoreProperty*>(property);
  VolumeUniformFluorophoreProperty* volumeProperty
    = dynamic_cast<VolumeUniformFluorophoreProperty*>(property);

  int numFluorophores = 0;
  double density = 0.0;
  if (geometryProperty) {

    gui_GroupBox->setTitle(tr("Fixed Points Fluorophore Model"));
    gui_AreaLabel->setHidden(true);
    gui_AreaEdit->setHidden(true);
    gui_DensityLabel->setHidden(true);
    gui_DensityEdit->setHidden(true);
    gui_NumberOfFluorophoresLabel->setHidden(true);
    gui_NumberOfFluorophoresEdit->setHidden(true);
    gui_NumberOfFluorophoresSlider->setHidden(true);

  } else if (surfaceProperty) {

    gui_GroupBox->setTitle(tr("Uniform Random Surface Labeling"));

    gui_useFixedDensity->setChecked(surfaceProperty->GetUseFixedDensity());
    gui_useFixedNumberOfFluorophores->setChecked(surfaceProperty->GetUseFixedNumberOfFluorophores());

    gui_AreaLabel->setText(tr("Surface area (fluorophores/micron^2)"));
    double area = surfaceProperty->GetGeometryArea();
    gui_AreaEdit->setText(QString().sprintf("%.6f", area));
    gui_DensityLabel->setText(tr("Density (fluorophores/micron^2)"));

    if (surfaceProperty->GetUseFixedNumberOfFluorophores()) {
      numFluorophores = surfaceProperty->GetNumberOfFluorophores();
      density = NumberOfFluorophoresToDensity(numFluorophores);
    } else {
      numFluorophores = DensityToNumberOfFluorophores(surfaceProperty->GetDensity());
      density = surfaceProperty->GetDensity();
    }

  } else if (volumeProperty) {

    gui_GroupBox->setTitle(tr("Uniform Random Volume Labeling"));

    gui_useFixedDensity->setChecked(volumeProperty->GetUseFixedDensity());
    gui_useFixedNumberOfFluorophores->setChecked(volumeProperty->GetUseFixedNumberOfFluorophores());

    gui_AreaLabel->setText(tr("Volume (fluorophores/micron^3)"));
    double volume = volumeProperty->GetGeometryVolume();
    gui_AreaEdit->setText(QString().sprintf("%.6f", volume));
    gui_DensityLabel->setText(tr("Density (fluorophores / micron^3)"));

    if (volumeProperty->GetUseFixedNumberOfFluorophores()) {
      numFluorophores = volumeProperty->GetNumberOfFluorophores();
      density = NumberOfFluorophoresToDensity(numFluorophores);
    } else {
      numFluorophores = DensityToNumberOfFluorophores(volumeProperty->GetDensity());
      density = volumeProperty->GetDensity();
    }

    gui_DensityEdit->setText(QVariant(volumeProperty->GetDensity()).toString());

    int numFluorophores = DensityToNumberOfFluorophores(volumeProperty->GetDensity());
    gui_NumberOfFluorophoresSlider->setValue(numFluorophores);
    gui_NumberOfFluorophoresEdit->setText(QVariant(numFluorophores).toString());

  }

  gui_DensityEdit->setText(QVariant(density).toString());
  gui_NumberOfFluorophoresEdit->setText(QVariant(numFluorophores).toString());
  gui_NumberOfFluorophoresSlider->setValue(numFluorophores);

}


void
FluorophoreModelDialog
::SaveSettingsToProperty() {
  SurfaceUniformFluorophoreProperty* surfaceProperty
    = dynamic_cast<SurfaceUniformFluorophoreProperty*>(m_FluorophoreProperty);
  VolumeUniformFluorophoreProperty* volumeProperty
    = dynamic_cast<VolumeUniformFluorophoreProperty*>(m_FluorophoreProperty);  

  if (surfaceProperty) {
    surfaceProperty->SetDensity(GetDensity());
    surfaceProperty->SetNumberOfFluorophores(GetNumberOfFluorophores());
    if (GetUseFixedNumberOfFluorophores()) {
      surfaceProperty->UseFixedNumberOfFluorophores();
    } else {
      surfaceProperty->UseFixedDensity();
    }
  }

  if (volumeProperty) {
    volumeProperty->SetDensity(GetDensity());
    volumeProperty->SetNumberOfFluorophores(GetNumberOfFluorophores());
    if (GetUseFixedNumberOfFluorophores()) {
      volumeProperty->UseFixedNumberOfFluorophores();
    } else {
      volumeProperty->UseFixedDensity();
    }
  }

  m_FluorophoreProperty->SetEnabled(GetEnabled());
  m_FluorophoreProperty->SetFluorophoreChannel(GetFluorophoreChannel());
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


bool
FluorophoreModelDialog
::GetUseFixedNumberOfFluorophores() {
  return gui_useFixedNumberOfFluorophores->isChecked();
}


double
FluorophoreModelDialog
::GetDensity() {
  return gui_DensityEdit->text().toDouble();
}


int
FluorophoreModelDialog
::GetNumberOfFluorophores() {
  return gui_NumberOfFluorophoresEdit->text().toInt();
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
  SurfaceUniformFluorophoreProperty* surfaceProperty =
    dynamic_cast<SurfaceUniformFluorophoreProperty*>(m_FluorophoreProperty);
  VolumeUniformFluorophoreProperty* volumeProperty =
    dynamic_cast<VolumeUniformFluorophoreProperty*>(m_FluorophoreProperty);

  if (surfaceProperty) {
    // Scale area from square nanometers to square micrometers
    double area = surfaceProperty->GetGeometryArea();
    return static_cast<int>(density * area + 0.5);
  } else if (volumeProperty) {
    // Scale area from cubic nanometers to cubic micrometers
    double volume = volumeProperty->GetGeometryVolume();
    return static_cast<int>(density * volume + 0.5);
  }

  return 0;
}


double
FluorophoreModelDialog
::NumberOfFluorophoresToDensity(int fluorophores) {
  SurfaceUniformFluorophoreProperty* surfaceProperty =
    dynamic_cast<SurfaceUniformFluorophoreProperty*>(m_FluorophoreProperty);
  VolumeUniformFluorophoreProperty* volumeProperty =
    dynamic_cast<VolumeUniformFluorophoreProperty*>(m_FluorophoreProperty);

  if (surfaceProperty) {
    // Scale area from square nanometers to square micrometers
    double area = surfaceProperty->GetGeometryArea();
    return static_cast<double>(fluorophores) / area;
  } else if (volumeProperty) {
    // Scale area from cubic nanometers to cubic micrometers
    double volume = volumeProperty->GetGeometryVolume();
    return static_cast<double>(fluorophores) / volume;
  }

  return 0.0;
}
