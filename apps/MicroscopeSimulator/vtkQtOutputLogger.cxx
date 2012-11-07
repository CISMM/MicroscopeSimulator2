#include "vtkQtOutputLogger.h"
//------------------------------------------------------------------------------
#include <vtkObjectFactory.h>
//------------------------------------------------------------------------------
vtkStandardNewMacro(vtkQtOutputLogger);
//------------------------------------------------------------------------------
vtkQtOutputLogger::vtkQtOutputLogger()
{
  this->BufferSize = 2048;
  this->Text = NULL;
}
//------------------------------------------------------------------------------
vtkQtOutputLogger::~vtkQtOutputLogger()
{
}
//------------------------------------------------------------------------------
void vtkQtOutputLogger::PrintSelf(std::ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);
}
//------------------------------------------------------------------------------
void vtkQtOutputLogger::DisplayText(const char* text)
{
  if (this->Text)
    {
    this->Text->appendPlainText(text);
    }
}
//------------------------------------------------------------------------------
void vtkQtOutputLogger::SetTextWidget(QPlainTextEdit* text)
{
  this->Text = text;
  if (this->Text)
    {
    this->Text->setMaximumBlockCount(this->BufferSize);
    }
}
