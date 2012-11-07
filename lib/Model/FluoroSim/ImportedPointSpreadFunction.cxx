#pragma warning(disable : 4996)

#include <XMLHelper.h>

#include <vtkAlgorithmOutput.h>
#include <vtkImageChangeInformation.h>

#include <itkCastImageFilter.h>
#include <itkChangeInformationImageFilter.hxx>
#include <itkImageFileReader.hxx>
#include <itkBinaryFunctorImageFilter.hxx> // Needed for AddConstantToImageFilter
#include <ITKImageToVTKImage.cxx>

// WARNING: Always include the header file for this class AFTER
// including the ITK headers. Otherwise, the ITK headers will be included
// without including the implementation files, and you will have many linker
// errors.
#include <ImportedPointSpreadFunction.h>


const std::string ImportedPointSpreadFunction::FILE_NAME_ATTRIBUTE        = "FileName";
const std::string ImportedPointSpreadFunction::INTENSITY_OFFSET_ATTRIBUTE = "IntensityOffset";
const std::string ImportedPointSpreadFunction::POINT_CENTER_ELEMENT       = "PointCenter";


ImportedPointSpreadFunction
::ImportedPointSpreadFunction() : PointSpreadFunction() {
  //m_ImageReader = ImageSourceType::New();

  m_IntensityOffset = 0.0;
  m_PointCenter[0] = m_PointCenter[1] = m_PointCenter[2] = 0.0;

  // Set up parameter names
  m_ParameterNames.push_back("Summed Intensity");
  m_ParameterNames.push_back("Intensity Offset");
  m_ParameterNames.push_back("X Size (voxels)");
  m_ParameterNames.push_back("Y Size (voxels)");
  m_ParameterNames.push_back("Z Size (voxels)");
  m_ParameterNames.push_back("X Voxel Spacing (nm)");
  m_ParameterNames.push_back("Y Voxel Spacing (nm)");
  m_ParameterNames.push_back("Z Voxel Spacing (nm)");
  m_ParameterNames.push_back("X Center (nm)");
  m_ParameterNames.push_back("Y Center (nm)");
  m_ParameterNames.push_back("Z Center (nm)");

  m_FileIsValid = false;

  m_ChangeInformationFilter = ChangeInfoFilterType::New();
  m_ChangeInformationFilter->ChangeDirectionOff();
  m_ChangeInformationFilter->ChangeOriginOn();
  m_ChangeInformationFilter->ChangeRegionOff();
  m_ChangeInformationFilter->ChangeSpacingOn();
  ImageType::SpacingType spacing;
  spacing[0] = 65.0;
  spacing[1] = 65.0;
  spacing[2] = 200.0;
  m_ChangeInformationFilter->SetOutputSpacing(spacing);
  ImageType::PointType origin;
  origin[0] = 0.0;
  origin[1] = 0.0;
  origin[2] = 0.0;
  m_ChangeInformationFilter->SetOutputOrigin(origin);

  m_AddConstantFilter = AddConstantFilterType::New();
  m_AddConstantFilter->SetInput(m_ChangeInformationFilter->GetOutput());
  m_AddConstantFilter->SetConstant2(m_IntensityOffset);

  m_Statistics->SetInput(m_AddConstantFilter->GetOutput());

  m_ScaleFilter->SetInput(m_AddConstantFilter->GetOutput());

  m_ITKToVTKFilter = new ITKImageToVTKImage<ImageType>();
  m_ITKToVTKFilter->SetInput(m_ScaleFilter->GetOutput());

  m_DerivativeX->SetInput(m_ScaleFilter->GetOutput());
  m_DerivativeY->SetInput(m_ScaleFilter->GetOutput());
  m_DerivativeZ->SetInput(m_ScaleFilter->GetOutput());
}


ImportedPointSpreadFunction
::~ImportedPointSpreadFunction() {
  delete m_ITKToVTKFilter;
}


void
ImportedPointSpreadFunction
::SetFileName(const std::string& fileName) {
  m_FileName = fileName;
  this->ReadFile();
  m_ChangeInformationFilter->UpdateLargestPossibleRegion();
}


std::string
ImportedPointSpreadFunction
::GetFileName() {
  return m_FileName;
}


void
ImportedPointSpreadFunction
::SetIntensityOffset(double offset) {
  m_IntensityOffset = offset;
  m_AddConstantFilter->SetConstant2(m_IntensityOffset);
}


double
ImportedPointSpreadFunction
::GetIntensityOffset() {
  return m_IntensityOffset;
}


vtkImageData*
ImportedPointSpreadFunction
::GetOutput() {
  m_ChangeInformationFilter->UpdateLargestPossibleRegion();
  m_ITKToVTKFilter->Modified();
  return m_ITKToVTKFilter->GetOutput();
}


vtkAlgorithmOutput*
ImportedPointSpreadFunction
::GetOutputPort() {
  m_ChangeInformationFilter->UpdateLargestPossibleRegion();
  m_ITKToVTKFilter->Modified();
  return m_ITKToVTKFilter->GetOutputPort();
}


int
ImportedPointSpreadFunction
::GetNumberOfProperties() {
  return m_ParameterNames.size();
}


std::string
ImportedPointSpreadFunction
::GetParameterName(int index) {
  try {
    return m_ParameterNames.at(index);
  } catch (...) {
  }

  return std::string("Error");
}


double
ImportedPointSpreadFunction
::GetParameterValue(int index) {
  switch (index) {
  case  0: return m_SummedIntensity;
  case  1: return m_IntensityOffset;
  case  2: return m_Image->GetLargestPossibleRegion().GetSize()[0]; break;
  case  3: return m_Image->GetLargestPossibleRegion().GetSize()[1]; break;
  case  4: return m_Image->GetLargestPossibleRegion().GetSize()[2]; break;
  case  5: return m_ChangeInformationFilter->GetOutputSpacing()[0]; break;
  case  6: return m_ChangeInformationFilter->GetOutputSpacing()[1]; break;
  case  7: return m_ChangeInformationFilter->GetOutputSpacing()[2]; break;
  case  8: return m_PointCenter[0]; break;
  case  9: return m_PointCenter[1]; break;
  case 10: return m_PointCenter[2]; break;

  default: return 0.0;
  }
  
  return 0.0;
}


void
ImportedPointSpreadFunction
::SetParameterValue(int index, double value) {
  ImageType::SpacingType outputSpacing = 
    m_ChangeInformationFilter->GetOutputSpacing();
  ImageType::PointType outputOrigin =
    m_ChangeInformationFilter->GetOutputOrigin();

  switch(index) {
  case  0:
    m_SummedIntensity = value;
    break;

  case  1:
    m_IntensityOffset = value;
    m_AddConstantFilter->SetConstant2(value);
    break;

  case  2:
  case  3:
  case  4:
    break; // Not editable

  case  5:
  case  6:
  case  7:
    outputSpacing[index-5] = value;
    m_ChangeInformationFilter->SetOutputSpacing(outputSpacing);
    RecenterImage();
    break;
    
  case  8:
  case  9:
  case 10:
    m_PointCenter[index-8] = value;
    RecenterImage();
  default: break;
  }

}


bool
ImportedPointSpreadFunction
::IsFileValid() {
  return m_FileIsValid;
}


void
ImportedPointSpreadFunction
::GetXMLConfiguration(xmlNodePtr node) {
  xmlNodePtr root = xmlNewChild(node, NULL, BAD_CAST "ImportedPointSpreadFunction", NULL);

  char doubleFormat[] = "%f";
  char buf[128];

  xmlNewProp(root, BAD_CAST NAME_ATTRIBUTE.c_str(), BAD_CAST m_Name.c_str());
  xmlNewProp(root, BAD_CAST FILE_NAME_ATTRIBUTE.c_str(), BAD_CAST m_FileName.c_str());
  sprintf(buf, "%f", GetSummedIntensity());
  xmlNewProp(root, BAD_CAST SUMMED_INTENSITY_ATTRIBUTE.c_str(), BAD_CAST buf);
  sprintf(buf, "%f", GetIntensityOffset());
  xmlNewProp(root, BAD_CAST INTENSITY_OFFSET_ATTRIBUTE.c_str(), BAD_CAST buf);

  xmlNodePtr spacingNode = xmlNewChild(root, NULL, BAD_CAST SPACING_ELEMENT.c_str(), NULL);
  sprintf(buf, doubleFormat, m_ChangeInformationFilter->GetOutputSpacing()[0]);
  xmlNewProp(spacingNode, BAD_CAST X_ATTRIBUTE.c_str(), BAD_CAST buf);
  sprintf(buf, doubleFormat, m_ChangeInformationFilter->GetOutputSpacing()[1]);
  xmlNewProp(spacingNode, BAD_CAST Y_ATTRIBUTE.c_str(), BAD_CAST buf);
  sprintf(buf, doubleFormat, m_ChangeInformationFilter->GetOutputSpacing()[2]);
  xmlNewProp(spacingNode, BAD_CAST Z_ATTRIBUTE.c_str(), BAD_CAST buf);

  xmlNodePtr pointCenterNode = xmlNewChild(root, NULL, BAD_CAST POINT_CENTER_ELEMENT.c_str(), NULL);
  sprintf(buf, doubleFormat, m_PointCenter[0]);
  xmlNewProp(pointCenterNode, BAD_CAST X_ATTRIBUTE.c_str(), BAD_CAST buf);
  sprintf(buf, doubleFormat, m_PointCenter[1]);
  xmlNewProp(pointCenterNode, BAD_CAST Y_ATTRIBUTE.c_str(), BAD_CAST buf);
  sprintf(buf, doubleFormat, m_PointCenter[2]);
  xmlNewProp(pointCenterNode, BAD_CAST Z_ATTRIBUTE.c_str(), BAD_CAST buf);
}


void
ImportedPointSpreadFunction
::RestoreFromXML(xmlNodePtr node) {
  char* name = (char*) xmlGetProp(node, BAD_CAST NAME_ATTRIBUTE.c_str());
  SetName(name);

  char* fileName = (char*) xmlGetProp(node, BAD_CAST FILE_NAME_ATTRIBUTE.c_str());
  try {
    SetFileName(std::string(fileName));
  } catch (...) {
    m_FileIsValid = false;
    std::cout << "Error when reading imported PSF file '" << fileName << "'."
              << std::endl;
    return;
  }
  m_FileIsValid = true;

  char* summedIntensityStr = (char*) xmlGetProp(node, BAD_CAST SUMMED_INTENSITY_ATTRIBUTE.c_str());
  if (summedIntensityStr) {
    SetSummedIntensity(atof(summedIntensityStr));
  }

  char* intensityOffsetStr = (char*) xmlGetProp(node, BAD_CAST INTENSITY_OFFSET_ATTRIBUTE.c_str());
  if (intensityOffsetStr) {
    SetIntensityOffset(atof(intensityOffsetStr));
  }

  xmlNodePtr spacingNode = xmlGetFirstElementChildWithName(node, BAD_CAST SPACING_ELEMENT.c_str());
  if (spacingNode) {
    char* x = (char*) xmlGetProp(spacingNode, BAD_CAST X_ATTRIBUTE.c_str());
    char* y = (char*) xmlGetProp(spacingNode, BAD_CAST Y_ATTRIBUTE.c_str());
    char* z = (char*) xmlGetProp(spacingNode, BAD_CAST Z_ATTRIBUTE.c_str());
    if (x && y && z) {
      double spacing[3];
      spacing[0] = atof(x);
      spacing[1] = atof(y);
      spacing[2] = atof(z);
      m_ChangeInformationFilter->SetOutputSpacing(spacing);
    }
  }

  xmlNodePtr pointCenterNode = xmlGetFirstElementChildWithName(node, BAD_CAST POINT_CENTER_ELEMENT.c_str());
  if (pointCenterNode) {
    char* x = (char*) xmlGetProp(pointCenterNode, BAD_CAST X_ATTRIBUTE.c_str());
    char* y = (char*) xmlGetProp(pointCenterNode, BAD_CAST Y_ATTRIBUTE.c_str());
    char* z = (char*) xmlGetProp(pointCenterNode, BAD_CAST Z_ATTRIBUTE.c_str());
    if (x && y && z) {
      m_PointCenter[0] = atof(x);
      m_PointCenter[1] = atof(y);
      m_PointCenter[2] = atof(z);
    }
  }

  RecenterImage();

  // It is critical to call this to ensure that the PSF is normalized after loading
  Update();
}


void
ImportedPointSpreadFunction
::RecenterImage() {
  // Change the origin here
  const ImageType::RegionType region = m_ChangeInformationFilter->GetOutput()->GetLargestPossibleRegion();
  const ImageType::SpacingType spacing = m_ChangeInformationFilter->GetOutputSpacing();

  ImageType::PointType origin;
  for (unsigned int i = 0; i < ImageType::GetImageDimension(); i++) {
    origin[i] = -0.5 * static_cast<double>(region.GetSize(i)-1) * spacing[i] + m_PointCenter[i];
  }
  m_ChangeInformationFilter->SetOutputOrigin(origin);

  m_ChangeInformationFilter->UpdateLargestPossibleRegion();
}


template< class TPixel >
bool
ImportedPointSpreadFunction
::ReadFileTemplate( const TPixel * ) {
  typedef itk::Image< TPixel, 3 > FileImageType;
  typedef itk::ImageFileReader< FileImageType > ImageReader;

  typename ImageReader::Pointer reader = ImageReader::New();
  reader->SetFileName( m_FileName );
  try {
    reader->UpdateLargestPossibleRegion();
    typedef itk::CastImageFilter< FileImageType, ImageType > CastFilterType;
    typename CastFilterType::Pointer caster = CastFilterType::New();
    caster->SetInput( reader->GetOutput() );
    m_ChangeInformationFilter->SetInput( caster->GetOutput() );
    m_ChangeInformationFilter->UpdateLargestPossibleRegion();
    m_Image = m_ChangeInformationFilter->GetOutput();
  } catch ( itk::ExceptionObject & except ) {
    std::cerr << "Could not read file '" << m_FileName << "'" << std::endl;
    std::cerr << except << std::endl;
    return false;
  }

  // Successfully read file
  return true;
}


bool
ImportedPointSpreadFunction
::ReadFile() {
  typedef itk::ImageIOBase::IOComponentType ScalarPixelType;

  itk::ImageIOBase::Pointer imageIO =
    itk::ImageIOFactory::CreateImageIO(m_FileName.c_str(), itk::ImageIOFactory::ReadMode);

  imageIO->SetFileName(m_FileName);
  imageIO->ReadImageInformation();
  const ScalarPixelType pixelType = imageIO->GetComponentType();

  switch (pixelType) {
    case itk::ImageIOBase::UCHAR:
      return this->ReadFileTemplate(static_cast<unsigned char *>(0));
      break;

    case itk::ImageIOBase::CHAR:
      return this->ReadFileTemplate(static_cast<char *>(0));
      break;

    case itk::ImageIOBase::USHORT:
      return this->ReadFileTemplate(static_cast<unsigned short *>(0));
      break;

    case itk::ImageIOBase::SHORT:
      return this->ReadFileTemplate(static_cast<short *>(0));
      break;

    case itk::ImageIOBase::UINT:
      return this->ReadFileTemplate(static_cast<unsigned int *>(0));
      break;

    case itk::ImageIOBase::INT:
      return this->ReadFileTemplate(static_cast<int *>(0));
      break;

    case itk::ImageIOBase::ULONG:
      return this->ReadFileTemplate(static_cast<unsigned long *>(0));
      break;

    case itk::ImageIOBase::LONG:
      return this->ReadFileTemplate(static_cast<long *>(0));
      break;

    case itk::ImageIOBase::FLOAT:
      return this->ReadFileTemplate(static_cast<float *>(0));
      break;

    case itk::ImageIOBase::DOUBLE:
      return this->ReadFileTemplate(static_cast<double *>(0));
      break;

    default:
      return false;
      break;
  }

  return false;
}
