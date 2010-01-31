#ifndef _Q_PSF_LIST_MODEL_H_
#define _Q_PSF_LIST_MODEL_H_

#include <QAbstractListModel>

// Forward declarations
class PointSpreadFunctionList;


class QPSFListModel : public QAbstractListModel {
Q_OBJECT

 public:
  QPSFListModel(QObject* parent=0);
  virtual ~QPSFListModel();
  
  void SetPSFList(PointSpreadFunctionList* list);
  PointSpreadFunctionList* GetPSFList();

  void SetHasNone(bool hasNone);

  bool setData(const QModelIndex& index, const QVariant& value, int role = Qt::EditRole);

  QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const;

  Qt::ItemFlags flags(const QModelIndex& index) const;

  int rowCount(const QModelIndex& parent = QModelIndex()) const;

  void Refresh();

 protected:
  PointSpreadFunctionList* m_PSFList;
  bool m_HasNonePSF;

};

#endif // _Q_PSF_LIST_MODEL_H_
