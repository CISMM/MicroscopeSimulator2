#ifndef _Q_FLUORESCENCE_OPTIMIZER_SETTINGS_TABLE_MODEL_H_
#define _Q_FLUORESCENCE_OPTIMIZER_SETTINGS_TABLE_MODEL_H_

#include <QAbstractTableModel>

// Forward declarations
class FluorescenceOptimizer;


class QFluorescenceOptimizerSettingsTableModel : public QAbstractTableModel {
 Q_OBJECT

 public:
  QFluorescenceOptimizerSettingsTableModel(QObject* parent = 0);
  ~QFluorescenceOptimizerSettingsTableModel();

  void SetFluorescenceOptimizer(FluorescenceOptimizer* optimizer);
  FluorescenceOptimizer* GetFluorescenceOptimizer();

  bool setData(const QModelIndex& index, const QVariant& value, int role = Qt::EditRole);

  QVariant data(const QModelIndex& index, int role) const;

  Qt::ItemFlags flags(const QModelIndex& index) const;

  QVariant headerData(int section, Qt::Orientation orientation,
                      int role = Qt::DisplayRole) const;

  int rowCount(const QModelIndex& parent = QModelIndex()) const;
  int columnCount(const QModelIndex& parent = QModelIndex()) const;

  void Refresh();

 protected:
  FluorescenceOptimizer* m_FluorescenceOptimizer;

};


#endif // _Q_FLUORESCENCE_OPTIMIZER_SETTINGS_TABLE_MODEL_H_
