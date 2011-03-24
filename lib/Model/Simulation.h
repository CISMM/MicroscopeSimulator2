#ifndef _SIMULATION_H_
#define _SIMULATION_H_

#include <string>

#include <DirtyListener.h>
#include <XMLStorable.h>

#include <ImageModelObject.h>
#include <ModelObjectList.h>

// Forward declarations
class AFMSimulation;
class FluorescenceSimulation;
class FluorescenceOptimizer;
class GradientDescentFluorescenceOptimizer;
class NelderMeadFluorescenceOptimizer;
class PointsGradientFluorescenceOptimizer;


// This is the data model for the Microscope Simulator library.
class Simulation : public DirtyListener, public XMLStorable {

  static const char* XML_ENCODING;
  static const char* SIMULATION_ELEM;
  static const char* VERSION_ELEM;
  static const char* VERSION_MAJOR_ATT;
  static const char* VERSION_MINOR_ATT;
  static const char* VERSION_REVISION_ATT;
  static const char* DESCRIPTION_ELEM;
  static const char* AFM_SIM_ELEM;
  static const char* FLUORO_SIM_ELEM;
  static const char* MODEL_OBJECT_LIST_ELEM;
  static const char* FILE_ATT;
  static const char* NAME_ATT;
  static const char* CREATED_ATT;
  static const char* MODIFIED_ATT;
  static const char* CAMERA_ELEM;
  static const char* NEW_FILE;
  static const char* ACTOR_INTERACTION_EVENT;


public:
  Simulation(DirtyListener* dirtyListener);
  virtual ~Simulation();

  void NewSimulation();

  int SaveXMLConfiguration(const std::string& fileName);
  int OpenXMLConfiguration(const std::string& fileName);

  virtual void GetXMLConfiguration(xmlNodePtr node);
  virtual void RestoreFromXML(xmlNodePtr node);

  virtual void Sully();
  virtual void SetStatusMessage(const std::string& status);

  void         SetSimulationDescription(const std::string& description);
  std::string& GetSimulationDescription();

  void         SetSimulationFileName(const std::string& name);
  std::string& GetSimulationFileName();

  void SetSimulationAlreadySaved(bool saved);
  bool GetSimulationAlreadySaved();

  void         SetSimulationCreationDate(const std::string& date);
  std::string& GetSimulationCreationDate();

  AFMSimulation*          GetAFMSimulation();
  FluorescenceSimulation* GetFluorescenceSimulation();

  void SetFluorescenceOptimizerToGradientDescent();
  void SetFluorescenceOptimizerToNelderMead();
  void SetFluorescenceOptimizerToPointsGradient();
  FluorescenceOptimizer* GetFluorescenceOptimizer();

  void SetComparisonImageModelObjectIndex(int index);
  ImageModelObject* GetComparisonImageModelObject();

  void OptimizeToFluorescence();
  void ExportFluorescenceStack(const std::string& fileName, int index, const std::string& extension,
                               bool exportRed, bool exportGreen, bool exportBlue);
  void SaveFluorescenceObjectiveFunctionValue(const std::string& fileName);

  void SetNumberOfThreads(int threads);
  int  GetNumberOfThreads();

  void AddNewModelObject(const std::string& objectTypeName);
  void ImportModelObject(const std::string& objectTypeName, const std::string& fileName);

  ModelObjectListPtr GetModelObjectList();

  vtkPolyDataAlgorithm* GetModelObjectGeometry(ModelObject* object);

  void RegenerateFluorophores();

protected:
  static const std::string DATE_FORMAT_STRING;

  std::string m_SimulationDescription;

  std::string m_SimulationFileName;

  bool m_SimulationAlreadySaved;

  std::string m_SimulationCreationDate;

  DirtyListener* m_DirtyListener;

  AFMSimulation*          m_AFMSim;
  FluorescenceSimulation* m_FluoroSim;

  GradientDescentFluorescenceOptimizer* m_GradientDescentFluoroOptimizer;
  NelderMeadFluorescenceOptimizer*      m_NelderMeadFluoroOptimizer;
  PointsGradientFluorescenceOptimizer*  m_PointsGradientFluoroOptimizer;
  FluorescenceOptimizer* m_FluoroOptimizer;

  ModelObjectList* m_ModelObjectList;

 private:
  Simulation() {};

};

#endif // _SIMULATION_H_
