#if defined(_MSC_VER)
#pragma warning ( disable : 4786 )
#endif

#ifdef __BORLANDC__
#define ITK_LEAN_AND_MEAN
#endif

#include "itkPluginFilterWatcher.h"
#include "itkPluginUtilities.h"

// vtkITK includes
#include <vtkITKArchetypeImageSeriesScalarReader.h>

// added for taking VTK and VTP as input and output
#include <vtkPolyDataReader.h>
#include <vtksys/SystemTools.hxx>

// STD includes
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>

int main( int argc, char * argv[] )
{
  if( argc < 3 )
    {
    std::cerr << "Both output and baseline txt files are required!" << std::endl;
    return EXIT_FAILURE;
    }

  std::string resultTxtPath = argv[1];
  std::string baselineTxtPath = argv[2];
  std::ifstream resultTxt(resultTxtPath.c_str());
  std::ifstream baselineTxt(baselineTxtPath.c_str());

  std::string resultLine;
  std::string baselineLine;
  std::string resultMeasure;
  std::string baselineMeasure;
  int c = 1;
  while (std::getline(resultTxt, resultLine) && std::getline(baselineTxt, baselineLine))
   {
      if (c > 1)
        {
          std::string delimiter = "fiber_labels_0_8_1027.vt";
          resultMeasure = resultLine.substr(resultLine.find(delimiter));
          baselineMeasure = baselineLine.substr(baselineLine.find(delimiter));
        }
      else
        {
          resultMeasure = resultLine;
          baselineMeasure = baselineLine;
        }

      if (resultMeasure.compare(baselineMeasure) != 0)
        {
          std::cerr << "Measurements are not the same!" << std::endl;
          return EXIT_FAILURE;
        }

      c++;
   }

  return EXIT_SUCCESS;
}





