#ifndef _SIMULATION_CXX_
#define _SIMULATION_CXX_

#if defined(_WIN32) // Turn off deprecation warnings in Visual Studio
#pragma warning( disable : 4996 )
#endif

#include <cstdlib>
#include <ctime>

#include <itkMultiThreader.h>
#include <itkPoint.h>

#include <Simulation.h>

#include <AFMSimulation.h>

#include <FluorescenceSimulation.h>
#include <GradientDescentFluorescenceOptimizer.h>
#include <NelderMeadFluorescenceOptimizer.h>
#include <PointsGradientFluorescenceOptimizer.h>

#include <ImageModelObject.h>
#include <ModelObjectList.h>
#include <XMLHelper.h>


const char* Simulation::XML_ENCODING     = "ISO-8859-1";
const char* Simulation::SIMULATION_ELEM  = "SimulatedExperiments";
const char* Simulation::DESCRIPTION_ELEM = "Description";
const char* Simulation::AFM_SIM_ELEM     = "AFMSimulation";
const char* Simulation::FLUORO_SIM_ELEM  = "FluorescenceSimulation";
const char* Simulation::MODEL_OBJECT_LIST_ELEM = "ModelObjectList";
const char* Simulation::FILE_ATT         = "file";
const char* Simulation::NAME_ATT         = "name";
const char* Simulation::CREATED_ATT      = "created";
const char* Simulation::MODIFIED_ATT     = "modified";
const char* Simulation::CAMERA_ELEM      = "Camera";
const char* Simulation::NEW_FILE         = "New Simulation";
const char* Simulation::ACTOR_INTERACTION_EVENT = "InteractionEvent";


Simulation
::Simulation(DirtyListener* dirtyListener) {
  m_DirtyListener = dirtyListener;
  m_ModelObjectList = new ModelObjectList(this);

  m_AFMSim    = new AFMSimulation(this);
  m_FluoroSim = new FluorescenceSimulation(this);

  m_GradientDescentFluoroOptimizer = 
    new GradientDescentFluorescenceOptimizer(dirtyListener);
  m_GradientDescentFluoroOptimizer->SetFluorescenceSimulation(m_FluoroSim);
  m_GradientDescentFluoroOptimizer->SetModelObjectList(m_ModelObjectList);

  m_NelderMeadFluoroOptimizer = 
    new NelderMeadFluorescenceOptimizer(dirtyListener);
  m_NelderMeadFluoroOptimizer->SetFluorescenceSimulation(m_FluoroSim);
  m_NelderMeadFluoroOptimizer->SetModelObjectList(m_ModelObjectList);

  m_PointsGradientFluoroOptimizer = 
    new PointsGradientFluorescenceOptimizer(dirtyListener);
  m_PointsGradientFluoroOptimizer->SetFluorescenceSimulation(m_FluoroSim);
  m_PointsGradientFluoroOptimizer->SetModelObjectList(m_ModelObjectList);

  m_FluoroOptimizer = m_NelderMeadFluoroOptimizer;

  // ITK will detect the number of cores on the system and set it by default.
  // Here we can override that setting if the proper environment variable is
  // set.
  char *var = getenv("MicroscopeSimulator_THREADS");
  if (var) {
    int numberOfThreads = atoi(var);
    if (numberOfThreads > 0)
      this->SetNumberOfThreads(numberOfThreads);
  }

  NewSimulation();
}


Simulation
::~Simulation() {
  delete m_ModelObjectList;
  delete m_AFMSim;
  delete m_FluoroSim;
  delete m_GradientDescentFluoroOptimizer;
  delete m_NelderMeadFluoroOptimizer;
  delete m_PointsGradientFluoroOptimizer;
}


void
Simulation
::NewSimulation() {
  m_SimulationDescription  = std::string();
  m_SimulationFileName = NEW_FILE;
  m_SimulationAlreadySaved = false;

  m_ModelObjectList->DeleteAll();

  m_AFMSim->NewSimulation();
  m_FluoroSim->NewSimulation();
}


int
Simulation
::SaveXMLConfiguration(const std::string& fileName) {
  int rc;
  xmlDocPtr doc;
  xmlNodePtr node;
  
  /* Create a new XML DOM tree, to which the XML document will be written. */
  doc = xmlNewDoc(BAD_CAST XML_DEFAULT_VERSION);
  if (doc == NULL) {
    std::cout << "Simulation: Error creating the XML document tree" << std::endl;
    return -1;
  }

  // Create a new XML node, to which the XML document will be appended.
  node = xmlNewDocNode(doc, NULL, BAD_CAST Simulation::SIMULATION_ELEM, NULL);
  if (node == NULL) {
    std::cout << "Simulation: Error creating the XML node" << std::endl;
    return -1;
  }

  // Set the file name here while we have it.
  xmlNewProp(node, BAD_CAST Simulation::FILE_ATT, BAD_CAST fileName.c_str());

  GetXMLConfiguration(node);

  /* Make ELEMENT the root node of the tree */
  xmlDocSetRootElement(doc, node);

  rc = xmlSaveFileEnc(fileName.c_str(), doc, Simulation::XML_ENCODING);

  xmlFreeDoc(doc);

  return rc;
}


int
Simulation
::OpenXMLConfiguration(const std::string& fileName) {
  xmlDocPtr doc = xmlReadFile(fileName.c_str(), Simulation::XML_ENCODING,
                              XML_PARSE_RECOVER);
  if (!doc) {
    return -1;
  }
  
  xmlNodePtr rootNode = xmlDocGetRootElement(doc);
  
  // Restore the simulation from the XML tree.
  RestoreFromXML(rootNode);
  
  xmlFreeDoc(doc);

  return 0;
}


void
Simulation
::GetXMLConfiguration(xmlNodePtr node) {

  // Set SimulatedExperiments attributes
  time_t rawTime;
  time(&rawTime);
  xmlNewProp(node, BAD_CAST Simulation::MODIFIED_ATT, BAD_CAST ctime(&rawTime));

  if (GetSimulationAlreadySaved()) {
    xmlNewProp(node, BAD_CAST Simulation::CREATED_ATT,
      BAD_CAST GetSimulationCreationDate().c_str());
  } else {
    xmlNewProp(node, BAD_CAST Simulation::CREATED_ATT, BAD_CAST ctime(&rawTime));
    SetSimulationCreationDate(std::string(ctime(&rawTime)));
  }


  // Write the description if non-empty.
  if (GetSimulationDescription() != "") {
    xmlNewChild(node, NULL, BAD_CAST Simulation::DESCRIPTION_ELEM,
      BAD_CAST GetSimulationDescription().c_str());
  }

  // Get XML configuration from the simulators here. Each child class will
  // append nodes to the Simulation node.
  xmlNodePtr afmSimNode = xmlNewChild(node, NULL, BAD_CAST AFM_SIM_ELEM, NULL);
  m_AFMSim->GetXMLConfiguration(afmSimNode);

  xmlNodePtr fluoroSimNode = xmlNewChild(node, NULL, BAD_CAST FLUORO_SIM_ELEM, NULL);
  m_FluoroSim->GetXMLConfiguration(fluoroSimNode);

  xmlNodePtr gradientDescentOptimizerNode = 
    xmlNewChild(fluoroSimNode, NULL, BAD_CAST GradientDescentFluorescenceOptimizer::OPTIMIZER_ELEM, NULL);
  m_GradientDescentFluoroOptimizer->GetXMLConfiguration(gradientDescentOptimizerNode);

  xmlNodePtr nelderMeadOptimizerNode =
    xmlNewChild(fluoroSimNode, NULL, BAD_CAST NelderMeadFluorescenceOptimizer::OPTIMIZER_ELEM, NULL);
  m_NelderMeadFluoroOptimizer->GetXMLConfiguration(nelderMeadOptimizerNode);

  xmlNodePtr pointsGradientOptimizerNode =
    xmlNewChild(fluoroSimNode, NULL, BAD_CAST PointsGradientFluorescenceOptimizer::OPTIMIZER_ELEM, NULL);
  m_PointsGradientFluoroOptimizer->GetXMLConfiguration(pointsGradientOptimizerNode);

  xmlNodePtr molNode = xmlNewChild(node, NULL, BAD_CAST MODEL_OBJECT_LIST_ELEM, NULL);
  m_ModelObjectList->GetXMLConfiguration(molNode);

  ModelObject* comparisonImageModelObject = 
    m_FluoroOptimizer->GetComparisonImageModelObject();
  std::string comparisonImageName = "None";
  if (comparisonImageModelObject) {
    comparisonImageName = comparisonImageModelObject->GetName();
  }

  xmlNodePtr fluorescenceComparisonImageNode = xmlNewChild
    (fluoroSimNode, NULL, BAD_CAST "FluorescenceComparisonImageModelObject",
     NULL);

  xmlNewProp(fluorescenceComparisonImageNode, BAD_CAST "name",
             BAD_CAST comparisonImageName.c_str());
}


void
Simulation
::RestoreFromXML(xmlNodePtr node) {

  // Get simulation file creation time
  xmlChar* creationTime = xmlGetProp(node, BAD_CAST "created");
  if (creationTime) {
    SetSimulationCreationDate(std::string((char*) creationTime));
  }

  xmlNodePtr descriptionNode =
    xmlGetFirstElementChildWithName(node, BAD_CAST "Description");
  char* description = xmlGetElementText(descriptionNode);
  if (description) {
    SetSimulationDescription(std::string(description));
  }

  // Restore AFM simulation
  xmlNodePtr afmSimNode =
    xmlGetFirstElementChildWithName(node, BAD_CAST AFM_SIM_ELEM);
  if (afmSimNode) {
    m_AFMSim->RestoreFromXML(afmSimNode);
  }

  // Restore fluorescence simulation
  xmlNodePtr fluoroSimNode =
    xmlGetFirstElementChildWithName(node, BAD_CAST FLUORO_SIM_ELEM);
  if (fluoroSimNode) {
    m_FluoroSim->RestoreFromXML(fluoroSimNode);
  }

  xmlNodePtr gradientDescentOptimizerNode =
    xmlGetFirstElementChildWithName(fluoroSimNode, BAD_CAST GradientDescentFluorescenceOptimizer::OPTIMIZER_ELEM);
  if (gradientDescentOptimizerNode) {
    m_GradientDescentFluoroOptimizer->RestoreFromXML(gradientDescentOptimizerNode);
  }

  xmlNodePtr nelderMeadOptimizerNode =
    xmlGetFirstElementChildWithName(fluoroSimNode, BAD_CAST NelderMeadFluorescenceOptimizer::OPTIMIZER_ELEM);
  if (nelderMeadOptimizerNode) {
    m_NelderMeadFluoroOptimizer->RestoreFromXML(nelderMeadOptimizerNode);
  }

  xmlNodePtr pointsGradientOptimizerNode =
    xmlGetFirstElementChildWithName(fluoroSimNode, BAD_CAST PointsGradientFluorescenceOptimizer::OPTIMIZER_ELEM);
  if (pointsGradientOptimizerNode) {
    m_PointsGradientFluoroOptimizer->RestoreFromXML(pointsGradientOptimizerNode);
  }

  // Restore model object list
  xmlNodePtr molNode = 
    xmlGetFirstElementChildWithName(node, BAD_CAST MODEL_OBJECT_LIST_ELEM);
  if (molNode) {
    m_ModelObjectList->RestoreFromXML(molNode);
  }

  // Restoring the fluorescence comparison image must be done AFTER the
  // model object list is restored.
  xmlNodePtr fluorescenceComparisonImageNode =
    xmlGetFirstElementChildWithName(fluoroSimNode, BAD_CAST "FluorescenceComparisonImageModelObject");
  if (fluorescenceComparisonImageNode) {
    std::string modelObjectName((char*) xmlGetProp(fluorescenceComparisonImageNode, BAD_CAST "name"));
    ModelObject* comparisonModelObject = m_ModelObjectList->GetModelObjectByName(modelObjectName);
    m_FluoroOptimizer->SetComparisonImageModelObject(comparisonModelObject);
  }                                 

}


void
Simulation
::Sully() {
  if (m_DirtyListener)
    m_DirtyListener->Sully();
}


void
Simulation
::SetStatusMessage(const std::string& status) {
  if (m_DirtyListener)
    m_DirtyListener->SetStatusMessage(status);
}


void
Simulation
::SetSimulationDescription(const std::string& description) {
  m_SimulationDescription = description;
  Sully();
}


std::string&
Simulation
::GetSimulationDescription() {
  return m_SimulationDescription;
}


void
Simulation
::SetSimulationFileName(const std::string& name) {
  m_SimulationFileName = name;
  Sully();
}


std::string&
Simulation
::GetSimulationFileName() {
  return m_SimulationFileName;
}


void
Simulation
::SetSimulationAlreadySaved(bool saved) {
  m_SimulationAlreadySaved = saved;
}


bool
Simulation
::GetSimulationAlreadySaved() {
  return m_SimulationAlreadySaved;
}


void
Simulation
::SetSimulationCreationDate(const std::string& date) {
  m_SimulationCreationDate = date;
}


std::string&
Simulation
::GetSimulationCreationDate() {
  return m_SimulationCreationDate;
}


AFMSimulation*
Simulation
::GetAFMSimulation() {
  return m_AFMSim;
}


FluorescenceSimulation*
Simulation
::GetFluorescenceSimulation() {
  return m_FluoroSim;
}


void
Simulation
::SetFluorescenceOptimizerToGradientDescent() {
  m_FluoroOptimizer = m_GradientDescentFluoroOptimizer;
}


void
Simulation
::SetFluorescenceOptimizerToNelderMead() {
  m_FluoroOptimizer = m_NelderMeadFluoroOptimizer;
}


void
Simulation
::SetFluorescenceOptimizerToPointsGradient() {
  m_FluoroOptimizer = m_PointsGradientFluoroOptimizer;
}


FluorescenceOptimizer*
Simulation
::GetFluorescenceOptimizer() {
  return m_FluoroOptimizer;
}


void
Simulation
::SetComparisonImageModelObjectIndex(int index) {
  if (!m_ModelObjectList)
    return;

  //m_FluoroOptimizer->SetComparisonImageModelObjectIndex(index);
  ImageModelObjectPtr comparisonObject = NULL;
  if (index >= 0) {
    comparisonObject = dynamic_cast<ImageModelObject*>
      (m_ModelObjectList->GetModelObjectAtIndex
       (index, ImageModelObject::OBJECT_TYPE_NAME));
  }
  m_FluoroSim->SetComparisonImageModelObject(comparisonObject);
  m_GradientDescentFluoroOptimizer->SetComparisonImageModelObject(comparisonObject);
  m_NelderMeadFluoroOptimizer->SetComparisonImageModelObject(comparisonObject);
  m_PointsGradientFluoroOptimizer->SetComparisonImageModelObject(comparisonObject);
}


ImageModelObject*
Simulation
::GetComparisonImageModelObject() {
  return m_FluoroOptimizer->GetComparisonImageModelObject();
}


void
Simulation
::OptimizeToFluorescence() {
  m_FluoroOptimizer->Optimize();
}


void
Simulation
::SetNumberOfThreads(int threads) {
  itk::MultiThreader::SetGlobalDefaultNumberOfThreads(threads);
  itk::MultiThreader::SetGlobalMaximumNumberOfThreads(threads);
}


int
Simulation
::GetNumberOfThreads() {
  return itk::MultiThreader::GetGlobalMaximumNumberOfThreads();
}


void
Simulation
::AddNewModelObject(const std::string& objectTypeName) {
  m_ModelObjectList->AddModelObject(objectTypeName);
}


void
Simulation
::ImportModelObject(const std::string& objectTypeName, const std::string& fileName) {
  m_ModelObjectList->ImportModelObject(objectTypeName, fileName);
}


ModelObjectListPtr
Simulation
::GetModelObjectList() {
  return m_ModelObjectList;
}


#endif // _SIMULATION_CXX_
