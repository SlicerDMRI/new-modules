#if defined(_MSC_VER)
#pragma warning ( disable : 4786 )
#endif

#ifdef __BORLANDC__
#define ITK_LEAN_AND_MEAN
#endif

#include "FiberEndPointFromLabelMapCLP.h"

#include "itkPluginFilterWatcher.h"
#include "itkPluginUtilities.h"

// vtkITK includes
#include <vtkITKArchetypeImageSeriesScalarReader.h>

// VTK includes
#include <vtkCellArray.h>
#include <vtkFloatArray.h>
#include <vtkInformation.h>
#include <vtkImageCast.h>
#include <vtkImageData.h>
#include <vtkMath.h>
#include <vtkNew.h>
#include <vtkPointData.h>
#include <vtkPolyData.h>
#include <vtkPolyDataWriter.h>
#include <vtkSmartPointer.h>
#include <vtkStreamingDemandDrivenPipeline.h>
#include <vtkTimerLog.h>
#include <vtkTransform.h>
#include <vtkTransformPolyDataFilter.h>
#include <vtkXMLPolyDataReader.h>
#include <vtkXMLPolyDataWriter.h>
#include <vtkVersion.h>
//#include <vtkPolyDataTensorToColor.h>
#include <vtkGlobFileNames.h>
#include <vtkStringArray.h>
#include <vtkAssignAttribute.h>
#include <vtkPolyDataReader.h>

 // VTKsys includes
#include <vtksys/SystemTools.hxx>

// ITK includes
#include <itkFloatingPointExceptions.h>

// STD includes
#include <iostream>
#include <algorithm>
#include <string>
#include <set>

int computeFiberStats(vtkPolyData *poly,
                       vtkNew<vtkITKArchetypeImageSeriesScalarReader> &readerLabel_A,
                       std::string &id);

void printTable(std::ofstream &ofs, bool printHeader,
                std::map< std::string, std::map<std::string, double> > &output);

int addClusters();

std::map< std::string, std::map<std::string, double> > OutTable;
std::map< std::string, std::string> ClusterNames;
std::map< std::string, std::map<std::string, double> > Clusters;
std::string SEPARATOR;

#define INVALID_NUMBER_PRINT std::string("NAN")

int main( int argc, char * argv[] )
{

  PARSE_ARGS;

  try
  {
    // output file
    std::ofstream ofs(outputFile.c_str());
    if (ofs.fail())
    {
      std::cerr << "Output file doesn't exist: " <<  outputFile << std::endl;
      return EXIT_FAILURE;
    }

    // Load input label map
    vtkNew<vtkITKArchetypeImageSeriesScalarReader> readerLabel_A;
    readerLabel_A->SetArchetype(InputLabel_A.c_str());
    readerLabel_A->SetUseOrientationFromFile(1);
    readerLabel_A->SetUseNativeOriginOn();
    readerLabel_A->SetOutputScalarTypeToNative();
    readerLabel_A->SetDesiredCoordinateOrientationToNative();
    readerLabel_A->SetSingleFile(1);
    readerLabel_A->Update();

    // Load all vtp or vtk files, each of them represents one tract
    if (InputDirectory.size() == 0)
    {
      std::cerr << "Input directory doesn't exist: " << std::endl;
      return EXIT_FAILURE;
    }

    vtkNew<vtkGlobFileNames> gfnVTK;
    gfnVTK->SetDirectory(InputDirectory.c_str());
    gfnVTK->AddFileNames("*.vtk");
    vtkStringArray *fileNamesVTK = gfnVTK->GetFileNames();

    vtkNew<vtkGlobFileNames> gfnVTP;
    gfnVTP->SetDirectory(InputDirectory.c_str());
    gfnVTP->AddFileNames("*.vtp");
    vtkStringArray *fileNamesVTP = gfnVTP->GetFileNames();

    // Calculating the percentage of fibers touching certain regisons for each tract
    for (vtkIdType i = 0; i < fileNamesVTP->GetNumberOfValues(); i++)
    {
      vtkNew<vtkXMLPolyDataReader> readerVTP;
      vtkStdString fileName = fileNamesVTP->GetValue(i);
      readerVTP->SetFileName(fileName.c_str());
      readerVTP->Update();
      std::string id = fileName;

      computeFiberStats(readerVTP->GetOutput(), readerLabel_A, id);
    }

    for (vtkIdType i = 0; i < fileNamesVTK->GetNumberOfValues(); i++)
    {
      vtkNew<vtkPolyDataReader> readerVTK;
      vtkStdString fileName = fileNamesVTK->GetValue(i);
      readerVTK->SetFileName(fileName.c_str());
      readerVTK->Update();
      std::string id = fileName;

      computeFiberStats(readerVTK->GetOutput(), readerLabel_A, id);
    }

    // Output the results
    SEPARATOR = "\t";
    printTable(ofs, true, OutTable);
    printTable(ofs, false, Clusters);

    ofs.flush();
    ofs.close();
  }
  catch ( ... )
  {
    std::cerr << "Default exception";
    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}

int computeFiberStats(vtkPolyData *input,
                      vtkNew<vtkITKArchetypeImageSeriesScalarReader> &readerLabel_A,
                      std::string &id)
{
  std::cout << "Input tract: " << id << std::endl;
  // Number of fibers
  int npolys = input->GetNumberOfCells();
  std::map< std::string, std::map<std::string, double> >::iterator it = OutTable.find(id);
  if (it == OutTable.end())
  {
    OutTable[id] = std::map<std::string, double>();
    it = OutTable.find(id);
  }
  it->second[std::string("Num_Fibers")] = npolys;

  // Read Label volume inputs
  vtkNew<vtkImageCast> imageCastLabel_A;
  imageCastLabel_A->SetOutputScalarTypeToShort();
  #if (VTK_MAJOR_VERSION <= 5)
    imageCastLabel_A->SetInput(readerLabel_A->GetOutput() );
  #else
    imageCastLabel_A->SetInputConnection(readerLabel_A->GetOutputPort() );
  #endif
    imageCastLabel_A->Update();

  // Set up matrices to put fibers into ijk space of volume
  // This assumes fibers are in RAS space of volume (i.e. RAS==world)
  vtkNew<vtkMatrix4x4> Label_A_RASToIJK;
  Label_A_RASToIJK->DeepCopy(readerLabel_A->GetRasToIjkMatrix());

  // The volume we're probing knows its spacing so take this out of the matrix
  double sp[3];
  imageCastLabel_A->GetOutput()->GetSpacing(sp);
  vtkNew<vtkTransform> trans;
  trans->Identity();
  trans->PreMultiply();
  trans->SetMatrix(Label_A_RASToIJK.GetPointer());

  int inExt[6];
  #if (VTK_MAJOR_VERSION <= 5)
    imageCastLabel_A->GetOutput()->GetWholeExtent(inExt);
  #else
    imageCastLabel_A->GetOutputInformation(0)->Get(
      vtkStreamingDemandDrivenPipeline::WHOLE_EXTENT(), inExt);
  #endif

  int *labelDims = imageCastLabel_A->GetOutput()->GetDimensions();

  // Get the total number of labels in the label map  
  int pt[3];
  int maxLabel = 0;
  std::set<short> uniqueLabel;
  for (int z = 0; z < labelDims[2]; z++)
  {
    for (int y = 0; y < labelDims[1]; y++)
    {
      for (int x = 0; x < labelDims[0]; x++)
      {
        pt[0] = x;
        pt[1] = y;
        pt[2] = z;

        short *inPtr = (short *) imageCastLabel_A->GetOutput()->GetScalarPointer(pt);
        uniqueLabel.insert(*inPtr);
        if ( *inPtr > maxLabel)
        {
            maxLabel =  *inPtr;
        }
      }     
    }
  }
  std::cout << " - Label map: number of labels (regions) " << uniqueLabel.size() 
            << " , maximum label: " << maxLabel << std::endl;

  // Initialize the label-point array
  double labelStartPointCount[maxLabel+1];
  double labelEndPointCount[maxLabel+1];
  for (int c=0; c < maxLabel + 1; c++)
  {
     labelStartPointCount[c] = 0;
     labelEndPointCount[c] = 0;
  }

  vtkPoints *inPts =input->GetPoints();
  vtkIdType numPts = inPts->GetNumberOfPoints();
  vtkCellArray *inLines = input->GetLines();
  vtkIdType numLines = inLines->GetNumberOfCells();
  vtkIdType npts=0, *pts=NULL;

  std::cout << " - Input tract: number of fibers " << numLines << std::endl;

  if ( !inPts || numPts  < 1 || !inLines || numLines < 1 )
  {
    std::cerr << "No fiber in this track file. " << std::endl;
    return 0;
  }

  // for each fiber (line)
  vtkIdType j;
  double p[3];
  vtkIdType inCellId;
  for (inCellId=0, inLines->InitTraversal();
       inLines->GetNextCell(npts,pts); inCellId++)
  {
    if (npts < 2)
    {
      std::cerr << "Less than two points in this fiber " << inCellId << std::endl;
      continue; //skip this polyline
    }

    double pIJK[3];
    int pt[3];
    short *inPtr;

    // First point of the fiber
    j = 0;
    inPts->GetPoint(pts[j],p);
    trans->TransformPoint(p,pIJK);
    pt[0] = (int) floor(pIJK[0]);
    pt[1] = (int) floor(pIJK[1]);
    pt[2] = (int) floor(pIJK[2]);
    if (pt[0] < 0 || pt[1] < 0 || pt[2] < 0 ||
        pt[0] >= labelDims[0] || pt[1] >= labelDims[1] || pt[2] >= labelDims[2])
    {
      continue;
    }
    
    // Label (region) touched by the start point
    inPtr = (short *) imageCastLabel_A->GetOutput()->GetScalarPointer(pt);
    labelStartPointCount[*inPtr] ++;
   
    // Last point of the fiber
    j = npts - 1;
    inPts->GetPoint(pts[j],p);
    trans->TransformPoint(p,pIJK);
    pt[0] = (int) floor(pIJK[0]);
    pt[1] = (int) floor(pIJK[1]);
    pt[2] = (int) floor(pIJK[2]);
    if (pt[0] < 0 || pt[1] < 0 || pt[2] < 0 ||
        pt[0] >= labelDims[0] || pt[1] >= labelDims[1] || pt[2] >= labelDims[2])
    {
      continue;
    }

    // Label (region) touched by the end point
    inPtr = (short *) imageCastLabel_A->GetOutput()->GetScalarPointer(pt);
    labelEndPointCount[*inPtr] ++;

    // NOTE: there could be fibers in which the endpoints in the same region, so
    // there may be percentage over 100.
  }

  // Compute the percentange for each label (region)
  double tempSum = 0;
  for (int c=0; c < maxLabel + 1; c++)
  {
    char labelName[5];
    sprintf(labelName, "%04d", c);
    
    if (uniqueLabel.find(c) != uniqueLabel.end())  
      it->second[std::string(labelName)] = (labelStartPointCount[c] + labelEndPointCount[c]) / npolys / 2 * 100;
    //it->second[std::string(labelName) + "(start)"] = labelStartPointCount[c] / npolys * 100;
    //it->second[std::string(labelName) + "(end)"] = labelEndPointCount[c] / npolys * 100;

    tempSum += (labelStartPointCount[c] + labelEndPointCount[c]) / npolys / 2 * 100;
  }
  std::cout << " - Sum of percentage for testing (should be 100): " << tempSum << std::endl;

  return 1;
}

std::map<std::string, std::string> getMeasureNames()
{
  std::map<std::string, std::string> names;
  std::map< std::string, std::map<std::string, double> >::iterator it;
  std::map<std::string, double>::iterator it1;

  for(it = OutTable.begin(); it != OutTable.end(); it++)
    {
    for (it1 = it->second.begin(); it1 != it->second.end(); it1++)
      {
      names[it1->first] = it1->first;
      }
    }
  return names;
}


bool isInCluster(const std::string &id, const std::string &clusterName)
{
  std::string s = id;
  std::string delimiter = ":";
  size_t pos = 0;
  std::string token;
  while ((pos = s.find(delimiter)) != std::string::npos)
    {
    token = s.substr(0, pos);
    if (clusterName == token)
      {
      return true;
      }
    s.erase(0, pos + delimiter.length());
    }
  return false;
}

void printTable(std::ofstream &ofs, bool printHeader,
                std::map< std::string, std::map<std::string, double> > &output)
{
  std::map<std::string, std::string> names = getMeasureNames();

  std::map< std::string, std::map<std::string, double> >::iterator it;
  std::map<std::string, double>::iterator it1;
  std::map<std::string, std::string>::iterator it2;

  if (printHeader)
    {
    //std::cout << "Name";
    ofs << "Name";

    it2 = names.find(std::string("Num_Points"));
    if (it2 != names.end())
      {
      //std::cout << " " << SEPARATOR << " " << it2->second;
      ofs << " " << SEPARATOR << " " << it2->second;
      }
    it2 = names.find(std::string("Num_Fibers"));
    if (it2 != names.end())
      {
      //std::cout << " " << SEPARATOR << " " << it2->second;
      ofs << " " << SEPARATOR << " " << it2->second;
      }

    for (it2 = names.begin(); it2 != names.end(); it2++)
      {
      if (it2->first != std::string("Num_Points") &&
          it2->first != std::string("Num_Fibers"))
        {
        //std::cout << " " << SEPARATOR << " " << it2->second;
        ofs << " " << SEPARATOR << " " << it2->second;
        }
      }
    //std::cout << std::endl;
    ofs << std::endl;
    }

  for(it = output.begin(); it != output.end(); it++)
    {
    //std::cout << it->first;
    ofs << it->first;

    it2 = names.find(std::string("Num_Points"));
    if (it2 != names.end())
      {
      //std::cout << " " << SEPARATOR << " ";
      ofs << " " << SEPARATOR << " ";
      it1 = it->second.find(std::string("Num_Points"));
      if (it1 != it->second.end())
        {
        if (vtkMath::IsNan(it1->second))
          {
          //std::cout << INVALID_NUMBER_PRINT;
          ofs << INVALID_NUMBER_PRINT;
          }
        else
          {
          //std::cout << std::fixed << it1->second;
          ofs << std::fixed << it1->second;
          }
        }
      }
    it2 = names.find(std::string("Num_Fibers"));
    if (it2 != names.end())
      {
      //std::cout << " " << SEPARATOR << " ";
      ofs << " " << SEPARATOR << " ";
      it1 = it->second.find(std::string("Num_Fibers"));
      if (it1 != it->second.end())
        {
        if (vtkMath::IsNan(it1->second))
          {
          //std::cout << INVALID_NUMBER_PRINT;
          ofs << INVALID_NUMBER_PRINT;
          }
        else
          {
          //std::cout << std::fixed << it1->second;
          ofs << std::fixed << it1->second;
          }
        }
      }

    for (it2 = names.begin(); it2 != names.end(); it2++)
      {
      if (it2->first != std::string("Num_Points") &&
          it2->first != std::string("Num_Fibers"))
        {
        //std::cout << " " << SEPARATOR << " ";
        ofs << " " << SEPARATOR << " ";

        it1 = it->second.find(it2->second);
        if (it1 != it->second.end())
          {
          if (vtkMath::IsNan(it1->second))
            {
            //std::cout << INVALID_NUMBER_PRINT;
            ofs << INVALID_NUMBER_PRINT;
            }
          else
            {
            //std::cout << std::fixed << it1->second;
            ofs << std::fixed << it1->second;
            }
          }
        }
      }
    //std::cout << std::endl;
    ofs << std::endl;
    }
}

int addClusters()
{
  std::map< std::string, std::map<std::string, double> >::iterator itOutput;
  std::map<std::string, double>::iterator itValues;
  std::map<std::string, double>::iterator itClusterValues;
  std::map<std::string, std::string>::iterator itClusterNames;
  std::map<std::string, std::string>::iterator itNames;

  std::map<std::string, std::string> names = getMeasureNames();

  for (itClusterNames = ClusterNames.begin(); itClusterNames!= ClusterNames.end(); itClusterNames++)
    {
    Clusters[itClusterNames->second] = std::map<std::string, double>();
    std::map< std::string, std::map<std::string, double> >::iterator itCluster = Clusters.find(itClusterNames->second);

    int npoints = 0;
    int npointsCluster = 0;
    for(itOutput = OutTable.begin(); itOutput != OutTable.end(); itOutput++)
      {
      if (isInCluster(itOutput->first, itClusterNames->first))
        {
        itValues = itOutput->second.find(std::string("Num_Points"));
        if (itValues != itOutput->second.end())
          {
          npoints = itValues->second;
          }
        npointsCluster += npoints;

        for (itNames = names.begin(); itNames != names.end(); itNames++)
          {
          itValues = itOutput->second.find(itNames->second);
          if (itValues == itOutput->second.end())
            {
            std::cerr << "Fibers contain different number of scalars" << std::endl;
            return 0;
            }

          itClusterValues = itCluster->second.find(itNames->second);
          if (itClusterValues == itCluster->second.end())
            {
            itCluster->second[itNames->second] = 0;
            itClusterValues = itCluster->second.find(itNames->second);
            }
          double clusterValue = itClusterValues->second;
          if (itValues != itOutput->second.end() && itNames->second != std::string("Num_Points") &&
              itNames->second != std::string("Num_Fibers") )
            {
            if (!vtkMath::IsNan(itValues->second))
              {
              clusterValue += npoints * itValues->second;
              }
            }
          else
            {
            clusterValue += itValues->second;
            }

          itCluster->second[itNames->second] = clusterValue;
          } ////for (itNames = names.begin(); itNames != names.end(); itNames++)
        } // if (isInCluster(itOutput->first, itClusterNames->first)
      } // for(itOutput = OutTable.begin(); itOutput != OutTable.end(); itOutput++)

      // second pass divide by npoints
      for (itNames = names.begin(); itNames != names.end(); itNames++)
        {
        itClusterValues = itCluster->second.find(itNames->second);
        if (itClusterValues != itCluster->second.end() && itNames->second != std::string("Num_Points") &&
              itNames->second != std::string("Num_Fibers") && npointsCluster )
          {
          double clusterValue = itClusterValues->second;
          itCluster->second[itNames->second] = clusterValue/npointsCluster;
          }
        }
    } //  for (itClusterNames = ClusterNames.begin(); itClusterNames!= ClusterNames.end(); itClusterNames++)
  return 1;
}
