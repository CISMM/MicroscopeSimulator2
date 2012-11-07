#ifndef __vtkQtOutputLogger_h
#define __vtkQtOutputLogger_h

#include <vtkOutputWindow.h>
#include <QPlainTextEdit>

class vtkQtOutputLogger : public vtkOutputWindow
{
 public:
  static vtkQtOutputLogger* New();
  vtkTypeMacro(vtkQtOutputLogger, vtkOutputWindow);
  virtual void PrintSelf(std::ostream& os, vtkIndent indent);

  vtkSetMacro(BufferSize,int);
  vtkGetMacro(BufferSize,int);

  void SetTextWidget(QPlainTextEdit* text);

 protected:
  vtkQtOutputLogger();
  ~vtkQtOutputLogger();

  virtual void DisplayText(const char*);

  QPlainTextEdit* Text;
  int BufferSize;

 private:
  vtkQtOutputLogger(const vtkQtOutputLogger &);
  void operator=(const vtkQtOutputLogger &);
};
#endif
