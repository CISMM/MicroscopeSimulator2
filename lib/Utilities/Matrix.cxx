#include <Matrix.h>


extern "C" {
  #include <f2c.h>
  #include <clapack.h>
}


Matrix
::Matrix(unsigned int rows, unsigned int columns) {
  m_Elements = new double[rows*columns];
  m_Rows = rows;
  m_Columns = columns;
}


Matrix
::~Matrix() {
  if (m_Elements) {
    delete[] m_Elements;
  }
}


void
Matrix
::SetElement(unsigned int row, unsigned int column, double value) {
  if (row < m_Rows && column < m_Columns) {
    m_Elements[column*m_Rows + row] = value; // This is probably wrong
  }
}


double
Matrix
::GetElement(unsigned int row, unsigned int column) {
  if (row < m_Rows && column < m_Columns) { 
    return m_Elements[column*m_Rows + row];
  }

  return 0.0;
}


double*
Matrix
::GetPointer() {
  return m_Elements;
}


void
Matrix
::LinearLeastSquaresSolve(double* x, double* b) {

  char trans = 'n';
  integer m = static_cast<integer>(m_Rows);
  integer n = static_cast<integer>(m_Columns);
  integer nrhs = 1;
  integer lda = m;
  integer ldb = static_cast<int>(m_Rows > m_Columns ? m_Rows : m_Columns);
  integer lwork = 0;
  integer info = 0;
  dgels_(&trans, &m, &n, &nrhs, m_Elements, &lda, b, &ldb, 0, &lwork, &info);
}
