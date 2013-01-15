#ifndef _SIMULATION_CXX_
#define _SIMULATION_CXX_

#if defined(_WIN32) // Turn off deprecation warnings in Visual Studio
#pragma warning( disable : 4996 )
#define snprintf _snprintf
#endif

#include <cstdlib>
#include <ctime>

#include <itkMultiThreader.h>
#include <itkPoint.h>

#include <vtkMath.h>

#include "Simulation.h"

#include "AFMSimulation.h"

#include "FluorescenceSimulation.h"
#include "GradientDescentFluorescenceOptimizer.h"
#include "NelderMeadFluorescenceOptimizer.h"
#include "PointsGradientFluorescenceOptimizer.h"

#include "ImageModelObject.h"
#include "ModelObjectList.h"
#include "XMLHelper.h"
#include "Version.h"
#include "Visualization.h"

#include "ImageWriter.h"

// TODO - Simulation.cxx shouldn't have references to VTK header files
#include <vtkImageExtractComponents.h>


const char* Simulation::XML_ENCODING         = "ISO-8859-1";
const char* Simulation::SIMULATION_ELEM      = "SimulatedExperiments";
const char* Simulation::VERSION_ELEM         = "Version";
const char* Simulation::VERSION_MAJOR_ATT    = "major";
const char* Simulation::VERSION_MINOR_ATT    = "minor";
const char* Simulation::VERSION_REVISION_ATT = "revision";
const char* Simulation::DESCRIPTION_ELEM     = "Description";
const char* Simulation::AFM_SIM_ELEM         = "AFMSimulation";
const char* Simulation::FLUORO_SIM_ELEM      = "FluorescenceSimulation";
const char* Simulation::MODEL_OBJECT_LIST_ELEM = "ModelObjectList";
const char* Simulation::FILE_ATT             = "file";
const char* Simulation::NAME_ATT             = "name";
const char* Simulation::CREATED_ATT          = "created";
const char* Simulation::MODIFIED_ATT         = "modified";
const char* Simulation::CAMERA_ELEM          = "Camera";
const char* Simulation::NEW_FILE             = "New Simulation";
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

  // Set the comparison imaage to NULL in the fluorescence optimizer
  if (m_FluoroOptimizer && m_FluoroOptimizer->GetComparisonImageModelObject() != NULL)
    m_FluoroOptimizer->SetComparisonImageModelObject(NULL);

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

  // Write the program version
  xmlNodePtr versionNode =
    xmlNewChild(node, NULL, BAD_CAST Simulation::VERSION_ELEM, NULL);
  char buf[128];
  sprintf(buf, "%d", MicroscopeSimulator_MAJOR_NUMBER);
  xmlNewProp(versionNode, BAD_CAST Simulation::VERSION_MAJOR_ATT,
             BAD_CAST buf);
  sprintf(buf, "%d", MicroscopeSimulator_MINOR_NUMBER);
  xmlNewProp(versionNode, BAD_CAST Simulation::VERSION_MINOR_ATT,
             BAD_CAST buf);
  sprintf(buf, "%d", MicroscopeSimulator_REVISION_NUMBER);
  xmlNewProp(versionNode, BAD_CAST Simulation::VERSION_REVISION_ATT,
             BAD_CAST buf);

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
  // Save the position of the focal plane
  unsigned int focalPlaneIndex = m_FluoroSim->GetFocalPlaneIndex();

  // Run the optimization
  m_FluoroOptimizer->Optimize();

  // Reset the focal plane position
  m_FluoroSim->SetFocalPlaneIndex(focalPlaneIndex);
}


void
Simulation
::ExportFluorescenceStack(const std::string& fileName, const std::string& extension,
                          bool exportRed, bool exportGreen, bool exportBlue,
                          bool regenerateFluorophores, bool randomizeObjectPositions,
                          bool randomizeStagePosition,
                          double xRange, double yRange, double zRange,
                          int numberOfCopies) {

  int originalIndex = this->GetFluorescenceSimulation()->GetFocalPlaneIndex();

  // Save the original object positions
  std::vector< double > originalPositions;
  for (unsigned int i = 0; i < this->GetModelObjectList()->GetSize(); i++) {
    ModelObject* mo = this->GetModelObjectList()->GetModelObjectAtIndex(i);
    if (mo->GetProperty(ModelObject::X_POSITION_PROP)) {
      originalPositions.push_back(mo->GetProperty(ModelObject::X_POSITION_PROP)->GetDoubleValue());
      originalPositions.push_back(mo->GetProperty(ModelObject::Y_POSITION_PROP)->GetDoubleValue());
      originalPositions.push_back(mo->GetProperty(ModelObject::Z_POSITION_PROP)->GetDoubleValue());
    }
  }

  for (int i = 0; i < numberOfCopies; i++) {
    if (regenerateFluorophores) {
      this->RegenerateFluorophores();
    }

    if (randomizeObjectPositions) {
      // Randomize each object's position.
      unsigned int mIndex = 0;
      for (unsigned int mi = 0; mi < this->GetModelObjectList()->GetSize(); mi++) {
        ModelObject* mo = this->GetModelObjectList()->GetModelObjectAtIndex(mi);
        if (!mo->GetProperty(ModelObject::X_POSITION_PROP))
          continue;

        double newX = originalPositions[mIndex++] + xRange * (vtkMath::Random() - 0.5);
        double newY = originalPositions[mIndex++] + yRange * (vtkMath::Random() - 0.5);
        double newZ = originalPositions[mIndex++] + zRange * (vtkMath::Random() - 0.5);

        mo->GetProperty(ModelObject::X_POSITION_PROP)->SetDoubleValue(newX);
        mo->GetProperty(ModelObject::Y_POSITION_PROP)->SetDoubleValue(newY);
        mo->GetProperty(ModelObject::Z_POSITION_PROP)->SetDoubleValue(newZ);
      }
    }

    if (randomizeStagePosition) {
      // Randomize all object's positions by the same random offset.
      double offsetX = xRange * (vtkMath::Random() - 0.5);
      double offsetY = yRange * (vtkMath::Random() - 0.5);
      double offsetZ = zRange * (vtkMath::Random() - 0.5);

      for (unsigned int mi = 0; mi < this->GetModelObjectList()->GetSize(); mi++) {
        ModelObject* mo = this->GetModelObjectList()->GetModelObjectAtIndex(mi);
        if (!mo->GetProperty(ModelObject::X_POSITION_PROP))
          continue;
        double x = mo->GetProperty(ModelObject::X_POSITION_PROP)->GetDoubleValue() + offsetX;
        double y = mo->GetProperty(ModelObject::Y_POSITION_PROP)->GetDoubleValue() + offsetY;
        double z = mo->GetProperty(ModelObject::Z_POSITION_PROP)->GetDoubleValue() + offsetZ;
        mo->GetProperty(ModelObject::X_POSITION_PROP)->SetDoubleValue(x);
        mo->GetProperty(ModelObject::Y_POSITION_PROP)->SetDoubleValue(y);
        mo->GetProperty(ModelObject::Z_POSITION_PROP)->SetDoubleValue(z);
      }
    }

    FluorescenceImageSource * imageSource = this->GetFluorescenceSimulation()->GetFluorescenceImageSource();
    if ( !imageSource ) {
      std::cerr << "No FluorescenceImageSource set in Simulation::ExportFluorescenceStack" << std::endl;
      return;
    }

    vtkImageData* rawStack = imageSource->GenerateFluorescenceStackImage();
    vtkSmartPointer<vtkImageExtractComponents> extractor = vtkSmartPointer<vtkImageExtractComponents>::New();
    extractor->SetInputData(rawStack);

    char fileNameBuffer[2048];
    if (exportRed) {
      extractor->SetComponents(0);
      snprintf( fileNameBuffer, sizeof(fileNameBuffer), "%s%04d_R.%s",
                fileName.c_str(), i, extension.c_str());

      try {
        ImageWriter writer;
        writer.SetFileName(fileNameBuffer);
        writer.SetInputConnection(extractor->GetOutputPort());
        writer.WriteUShortImage();
      } catch (itk::ExceptionObject e) {
        std::cout << "Error on writing the red channel" << std::endl;
        std::cout << e.GetDescription() << std::endl;
      }
    }

    if (exportGreen) {
      extractor->SetComponents(1);
      snprintf( fileNameBuffer, sizeof(fileNameBuffer), "%s%04d_G.%s",
                fileName.c_str(), i, extension.c_str());

      try {
        ImageWriter writer;
        writer.SetFileName(fileNameBuffer);
        writer.SetInputConnection(extractor->GetOutputPort());
        writer.WriteUShortImage();
      } catch (itk::ExceptionObject e) {
        std::cout << "Error on writing the green channel" << std::endl;
        std::cout << e.GetDescription() << std::endl;
      }
    }

    if (exportBlue) {
      extractor->SetComponents(2);
      snprintf( fileNameBuffer, sizeof(fileNameBuffer), "%s%04d_B.%s",
                fileName.c_str(), i, extension.c_str());

      try {
        ImageWriter writer;
        writer.SetFileName(fileNameBuffer);
        writer.SetInputConnection(extractor->GetOutputPort());
        writer.WriteUShortImage();
      } catch (itk::ExceptionObject e) {
        std::cout << "Error on writing the blue channel" << std::endl;
        std::cout << e.GetDescription() << std::endl;
      }
    }

    rawStack->Delete();
  }

  // Restore the original object positions
  unsigned int index = 0;
  for (unsigned int i = 0; i < this->GetModelObjectList()->GetSize(); i++) {
    ModelObject* mo = this->GetModelObjectList()->GetModelObjectAtIndex(i);
    if (mo->GetProperty(ModelObject::X_POSITION_PROP)) {
      mo->GetProperty(ModelObject::X_POSITION_PROP)->SetDoubleValue(originalPositions[index++]);
      mo->GetProperty(ModelObject::Y_POSITION_PROP)->SetDoubleValue(originalPositions[index++]);
      mo->GetProperty(ModelObject::Z_POSITION_PROP)->SetDoubleValue(originalPositions[index++]);
    }
  }

  // Reset to original focal plane depth
  this->GetFluorescenceSimulation()->SetFocalPlaneIndex(originalIndex);
}


void
Simulation
::SaveFluorescenceObjectiveFunctionValue(const std::string& fileName) {
  double value = this->GetFluorescenceOptimizer()->GetObjectiveFunctionValue();
  std::ofstream file(fileName.c_str());

  file.setf(ios::scientific, ios::floatfield);
  file.precision(10);
  file << value << std::endl;
  file.close();
}


void
Simulation
::SetNumberOfThreads(unsigned int threads) {
  itk::MultiThreader::SetGlobalDefaultNumberOfThreads(threads);
  itk::MultiThreader::SetGlobalMaximumNumberOfThreads(threads);
}


unsigned int
Simulation
::GetNumberOfThreads() {
  return itk::MultiThreader::GetGlobalMaximumNumberOfThreads();
}


void
Simulation
::AddNewModelObject(const std::string& objectTypeName) {
  ModelObjectPtr mop = m_ModelObjectList->AddModelObject(objectTypeName);

  if (m_FluoroSim) {
    double pixelSize = 0.5 *m_FluoroSim->GetPixelSize();
    double xCenter = pixelSize * static_cast<double>(m_FluoroSim->GetImageWidth());
    double yCenter = pixelSize * static_cast<double>(m_FluoroSim->GetImageHeight());
    double zCenter = 0.0;

    ModelObjectProperty* xProp =  mop->GetProperty(ModelObject::X_POSITION_PROP);
    if (xProp) xProp->SetDoubleValue(xCenter);
    ModelObjectProperty* yProp =  mop->GetProperty(ModelObject::Y_POSITION_PROP);
    if (yProp) yProp->SetDoubleValue(yCenter);
    ModelObjectProperty* zProp =  mop->GetProperty(ModelObject::Z_POSITION_PROP);
    if (zProp) zProp->SetDoubleValue(zCenter);
  }
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


void
Simulation
::RegenerateFluorophores() {
  // Loop over model objects
  for (int i = 0; i < static_cast<int>(m_ModelObjectList->GetSize()); i++) {
    ModelObject* mo = m_ModelObjectList->GetModelObjectAtIndex(i);
    mo->RegenerateFluorophores();
  }
}


#endif // _SIMULATION_CXX_
