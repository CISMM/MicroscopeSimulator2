/*=========================================================================

  Program:   Insight Segmentation & Registration Toolkit
  Module:    $RCSfile: itkFluorescenceImageSource.h,v $
  Language:  C++
  Date:      $Date: 2009/09/14 13:57:30 $
  Version:   $Revision: 1.10 $

  Copyright (c) Insight Software Consortium. All rights reserved.
  See ITKCopyright.txt or http://www.itk.org/HTML/Copyright.htm for details.

  Portions of this code are covered under the VTK copyright.
  See VTKCopyright.txt or http://www.kitware.com/VTKCopyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notices for more information.

=========================================================================*/
#ifndef __itkFluorescenceImageSource_h
#define __itkFluorescenceImageSource_h

#include "itkNumericTraits.h"
#include "itkParametricImageSource.h"
#include "itkVTKImageImport.h"

#include <vtkSmartPointer.h>

#include <FluorescenceImageSource.h>
#include <VTKImageToITKImage.h>

class vtkImageExtractComponents;
class vtkImageExport;


namespace itk
{

/** \class FluorescenceImageSource
 * \brief Generate a fluorescence image from given model object parameters.
 *
 * \ingroup DataSources
 */
template <class TOutputImage>
class FluorescenceImageSource : public ParametricImageSource<TOutputImage>
{
public:
  /** Standard class typedefs. */
  typedef FluorescenceImageSource              Self;
  typedef ParametricImageSource<TOutputImage>  Superclass;
  typedef SmartPointer<Self>                   Pointer;
  typedef SmartPointer<const Self>             ConstPointer;

  /** Typedef for the output image PixelType. */
  typedef TOutputImage                         OutputImageType;
  typedef typename OutputImageType::PixelType  PixelType;
  typedef typename OutputImageType::RegionType OutputImageRegionType;

  /** Typedef for ITK image importer */
  typedef VTKImageImport<TOutputImage> VTKImageImportType;

  itkStaticConstMacro(ImageDimension,
		      unsigned int,
		      TOutputImage::ImageDimension);

  /** Run-time type information (and related methods). */
  itkTypeMacro(FluorescenceImageSource,ParametricImageSource);

  /** Method for creation through the object factory. */
  itkNewMacro(Self);

  typedef typename Superclass::ParametersValueType ParametersValueType;
  typedef typename Superclass::ParametersType      ParametersType;

  void SetFluorescenceImageSource(::FluorescenceImageSource* source);

  /** Expects the parameters argument to contain values for ALL parameters. */
  virtual void SetParameters(const ParametersType& parameters);

  /** Gets the full parameters list. */
  virtual ParametersType GetParameters() const;

  /** Set a single parameter value. */
  virtual void SetParameter(unsigned int index, double value);

  /** Get a single parameter value. */
  virtual double GetParameter(unsigned int index) const;

  /** Gets the total number of parameters. */
  virtual unsigned int GetNumberOfParameters() const;


protected:
  ::FluorescenceImageSource* m_ImageSource;

  VTKImageToITKImage<TOutputImage>* m_VTKToITKFilter;

  int m_ExtractedComponent;
  vtkSmartPointer<vtkImageExtractComponents> m_Extractor;

  FluorescenceImageSource();
  ~FluorescenceImageSource();
  void PrintSelf(std::ostream& os, Indent indent) const;

  virtual void GenerateData();
  virtual void GenerateOutputInformation();

private:
  FluorescenceImageSource(const FluorescenceImageSource&); //purposely not implemented
  void operator=(const FluorescenceImageSource&); //purposely not implemented

};

} // end namespace itk

#ifndef ITK_MANUAL_INSTANTIATION
#include "itkFluorescenceImageSource.txx"
#endif

#endif
