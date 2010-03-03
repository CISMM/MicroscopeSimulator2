#ifndef _SIMULATION_CXX_
#define _SIMULATION_CXX_

#if defined(_WIN32) // Turn off deprecation warnings in Visual Studio
#pragma warning( disable : 4996 )
#endif

//#include <StdLibGuard.h>
#include <cstdlib>
#include <ctime>

#include <itkMultiThreader.h>
#include <itkPoint.h>

#include <Simulation.h>

#include <FluorescenceSimulation.h>
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
  m_SimulationDescription = std::string();
  m_SimulationFileName = NEW_FILE;
  m_SimulationAlreadySaved = false;
  m_DirtyListener = dirtyListener;
  m_ModelObjectList = new ModelObjectList(this);

  m_FluoroSim = new FluorescenceSimulation(this);

  // ITK will detect the number of cores on the system and set it by default.
  // Here we can override that setting if the proper environment variable is
  // set.
  char *var = getenv("MicroscopeSimulator_THREADS");
  if (var) {
    int numberOfThreads = atoi(var);
    if (numberOfThreads > 0)
      this->SetNumberOfThreads(numberOfThreads);
  }
}


Simulation
::~Simulation() {
  delete m_ModelObjectList;
}


void
Simulation
::NewSimulation() {
  // TODO - fill in

  m_SimulationFileName = std::string();
  m_SimulationDescription  = std::string();

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
  xmlNodePtr fluoroSimNode = xmlNewChild(node, NULL, BAD_CAST Simulation::FLUORO_SIM_ELEM, NULL);
  m_FluoroSim->GetXMLConfiguration(fluoroSimNode);

  xmlNodePtr molNode = xmlNewChild(node, NULL, BAD_CAST Simulation::MODEL_OBJECT_LIST_ELEM, NULL);
  m_ModelObjectList->GetXMLConfiguration(molNode);

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

  // Restore fluorescence simulation
  xmlNodePtr fluoroSimNode =
    xmlGetFirstElementChildWithName(node, BAD_CAST Simulation::FLUORO_SIM_ELEM);
  m_FluoroSim->RestoreFromXML(fluoroSimNode);

  // Restore model object list
  xmlNodePtr molNode = 
    xmlGetFirstElementChildWithName(node, BAD_CAST Simulation::MODEL_OBJECT_LIST_ELEM);
  m_ModelObjectList->RestoreFromXML(molNode);

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


FluorescenceSimulation*
Simulation
::GetFluorescenceSimulation() {
  return m_FluoroSim;
}


void
Simulation
::UpdateDisplay() {
  // TODO - visModes.visualize(displayState);
  // TODO - fluoroSim.updateView();
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


ModelObjectListPtr
Simulation
::GetModelObjectList() {
  return m_ModelObjectList;
}


#endif // _SIMULATION_CXX_
