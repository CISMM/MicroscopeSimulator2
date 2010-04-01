#ifndef _IMPORTED_POINT_SPREAD_FUNCTION_H_
#define _IMPORTED_POINT_SPREAD_FUNCTION_H_

#include <vector>

#define ITK_MANUAL_INSTANTIATION
#include <itkChangeInformationImageFilter.h>
#include <itkImageFileReader.h>
#include <ITKImageToVTKImage.h>
#undef ITK_MANUAL_INSTANTIATION

#include <vtkSmartPointer.h>

#include <PointSpreadFunction.h>


class ImportedPointSpreadFunction : public PointSpreadFunction {

 public:
  static const std::string FILE_NAME_ATTRIBUTE;
  static const std::string POINT_CENTER_ELEMENT;


  ImportedPointSpreadFunction();
  virtual ~ImportedPointSpreadFunction();

  virtual void SetFileName(const std::string& fileName);
  virtual std::string GetFileName();

  void GetSize(int& ix, int& iy, int& iz);

  virtual vtkImageData* GetOutput();
  virtual vtkAlgorithmOutput* GetOutputPort();

  virtual int GetNumberOfProperties();
  std::string GetParameterName(int index);
  double      GetParameterValue(int index);
  virtual void SetParameterValue(int index, double value);

  virtual void GetXMLConfiguration(xmlNodePtr node);
  virtual void RestoreFromXML(xmlNodePtr node);

  typedef float                                     PixelType;
  typedef itk::Image<PixelType, 3>                  ImageType;
  typedef itk::ImageFileReader<ImageType>           ImageSourceType;
  typedef itk::ChangeInformationImageFilter<ImageType> ChangeInfoFilterType;
  
 protected:
  std::vector<std::string> m_ParameterNames;

  std::string m_FileName;
  double      m_PointCenter[3];

  ImageSourceType::Pointer       m_ImageReader;

  ITKImageToVTKImage<ImageType>* m_ITKToVTKFilter;

  ChangeInfoFilterType::Pointer  m_ChangeInformationFilter;

  void RecenterImage();
};


#endif // _POINT_SPREAD_FUNCTION_H_
