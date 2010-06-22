#ifndef _Q_FOCAL_PLANE_POSITIONS_TABLE_MODEL_H_
#define _Q_FOCAL_PLANE_POSITIONS_TABLE_MODEL_H_

#include <QAbstractTableModel>

// Forward declarations
class FluorescenceSimulation;


class QFocalPlanePositionsTableModel : public QAbstractTableModel {
Q_OBJECT

 public:
  QFocalPlanePositionsTableModel(QObject* parent = 0);
  virtual ~QFocalPlanePositionsTableModel();

  void SetFluorescenceSimulation(FluorescenceSimulation* simulation);
  FluorescenceSimulation* GetFluorescenceSimulation();
  
  bool setData(const QModelIndex& index, const QVariant& value, int role = Qt::EditRole);
  
  QVariant data(const QModelIndex& index, int role) const;
  
  Qt::ItemFlags flags(const QModelIndex& index) const;
  
  QVariant headerData(int section, Qt::Orientation orientation,
                      int role = Qt::DisplayRole) const;
  
  int rowCount(const QModelIndex& parent = QModelIndex()) const;
  int columnCount(const QModelIndex& parent = QModelIndex()) const;
  
  void Refresh();

 protected:
  FluorescenceSimulation* m_Simulation;

};

#endif // _Q_FOCAL_PLANE_POSITIONS_TABLE_MODEL_H_
