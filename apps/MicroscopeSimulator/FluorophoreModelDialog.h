#ifndef _FLUOROPHORE_MODEL_DIALOG_H_
#define _FLUOROPHORE_MODEL_DIALOG_H_

#include <ui_FluorophoreModelDialog.h>
#include <QDialog>

#include <FluorophoreModelChannels.h>

// Forward declarations
class FluorophoreModelObjectProperty;


class FluorophoreModelDialog : public QDialog, 
  private Ui_FluorophoreModelDialog {
Q_OBJECT

 public:
  FluorophoreModelDialog(FluorophoreModelObjectProperty* prop, 
                         QWidget* parent=0);
  virtual ~FluorophoreModelDialog();

  void SetProperty(FluorophoreModelObjectProperty* property);

  void SaveSettingsToProperty();

  bool                   GetEnabled();

  FluorophoreChannelType GetFluorophoreChannel();
  bool                   GetUseFixedNumberOfFluorophores();

  double                 GetDensity();
  int                    GetNumberOfFluorophores();
 
  bool   GetUseOneFluorophorePerSample();
  int    GetNumberOfFluorophoresAroundRing();
  double GetRingRadius();
  bool   GetRandomizePatternOrientations();
 
 public slots:
  virtual void on_gui_DensityEdit_textEdited(const QString& string);
  virtual void on_gui_NumberOfFluorophoresEdit_textEdited(const QString& string);
  virtual void on_gui_NumberOfFluorophoresSlider_sliderMoved(int value);
  virtual void on_gui_UseSingleFluorophorePerSampleRadioButton_clicked(bool checked);
  virtual void on_gui_UsePointRingClusterRadioButton_clicked(bool checked);

 protected:
  int DensityToNumberOfFluorophores(double density);
  double NumberOfFluorophoresToDensity(int fluorophores);

 private:
  FluorophoreModelObjectProperty* m_FluorophoreProperty;
  std::string m_DoubleFormatString;
};

#endif // _FLUOROPHORE_MODEL_DIALOG_H_
