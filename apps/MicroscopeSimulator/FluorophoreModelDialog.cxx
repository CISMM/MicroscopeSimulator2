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
    gui_AreaLabel->setText(tr("Surface area (fluorophores/micron^2)"));
    double area = surfaceProperty->GetGeometryArea();
    gui_AreaEdit->setText(QString().sprintf("%.6f", area));
    gui_DensityLabel->setText(tr("Density (fluorophores/micron^2)"));

  } else if (volumeProperty) {

    gui_GroupBox->setTitle(tr("Uniform Random Volume Labeling"));
    gui_AreaLabel->setText(tr("Volume (fluorophores/micron^3)"));
    double volume = volumeProperty->GetGeometryVolume();
    gui_AreaEdit->setText(QString().sprintf("%.6f", volume));
    gui_DensityLabel->setText(tr("Density (fluorophores / micron^3)"));
  }

  // Common settings for density-based fluorophore models.
  UniformFluorophoreProperty* uniformProperty
    = dynamic_cast<UniformFluorophoreProperty*>(property);

  int numFluorophores = 0;
  double density = 0.0;

  if (uniformProperty) {
    if (uniformProperty->GetSamplingMode() == UniformFluorophoreProperty::FIXED_NUMBER) {
      numFluorophores = uniformProperty->GetNumberOfFluorophores();
      density = NumberOfFluorophoresToDensity(numFluorophores);
    } else {
      numFluorophores = DensityToNumberOfFluorophores(uniformProperty->GetDensity());
      density = uniformProperty->GetDensity();
    }

    gui_useFixedDensity->setChecked(uniformProperty->GetSamplingMode() == UniformFluorophoreProperty::FIXED_DENSITY);
    gui_useFixedNumberOfFluorophores->setChecked(uniformProperty->GetSamplingMode() == UniformFluorophoreProperty::FIXED_NUMBER);
    gui_DensityEdit->setText(QVariant(density).toString());
    gui_NumberOfFluorophoresEdit->setText(QVariant(numFluorophores).toString());
    gui_NumberOfFluorophoresSlider->setValue(numFluorophores);

    gui_UseSingleFluorophorePerSampleRadioButton->setChecked
      (uniformProperty->GetSamplePattern() == UniformFluorophoreProperty::SINGLE_POINT);
    gui_UsePointRingClusterRadioButton->setChecked
      (uniformProperty->GetSamplePattern() == UniformFluorophoreProperty::POINT_RING);
    gui_FluorophoresAroundRingEdit->setText
      (QVariant(uniformProperty->GetNumberOfRingFluorophores()).toString());
    gui_RingRadiusEdit->setText
      (QVariant(uniformProperty->GetRingRadius()).toString());

    gui_RandomizePatternOrientationsCheckBox->setChecked
      (uniformProperty->GetRandomizePatternOrientations());
  }

}


void
FluorophoreModelDialog
::SaveSettingsToProperty() {
  UniformFluorophoreProperty* uniformProperty
    = dynamic_cast<UniformFluorophoreProperty*>(m_FluorophoreProperty);
  if (uniformProperty) {
    uniformProperty->SetDensity(GetDensity());
    uniformProperty->SetNumberOfFluorophores(GetNumberOfFluorophores());

    if (GetUseFixedNumberOfFluorophores()) {
      uniformProperty->SetSamplingModeToFixedNumber();
    } else {
      uniformProperty->SetSamplingModeToFixedDensity();
    }

    if (GetUseOneFluorophorePerSample()) {
      uniformProperty->SetSamplePatternToSinglePoint();
    } else {
      uniformProperty->SetSamplePatternToPointRing();
    }

    uniformProperty->SetNumberOfRingFluorophores(GetNumberOfFluorophoresAroundRing());
    uniformProperty->SetRingRadius(GetRingRadius());
    
    uniformProperty->SetRandomizePatternOrientations(GetRandomizePatternOrientations());
  }

  // These options apply to any kind of fluorophore property
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


bool
FluorophoreModelDialog
::GetUseOneFluorophorePerSample() {
  return gui_UseSingleFluorophorePerSampleRadioButton->isChecked();
}


int
FluorophoreModelDialog
::GetNumberOfFluorophoresAroundRing() {
  return QVariant(gui_FluorophoresAroundRingEdit->text()).toInt();
}


double
FluorophoreModelDialog
::GetRingRadius() {
  return QVariant(gui_RingRadiusEdit->text()).toDouble();
}


bool
FluorophoreModelDialog
::GetRandomizePatternOrientations() {
  return gui_RandomizePatternOrientationsCheckBox->isChecked();
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
