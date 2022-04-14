#pragma once
#include <maya/MFnMesh.h>
#include <maya/MPointArray.h>
#include <maya/MFloatVectorArray.h>
#include <maya/MFloatArray.h>

#include <iosfwd>

class polyWriter
{
 public:
  polyWriter(MDagPath dagPath, MStatus& status);
  virtual ~polyWriter();
  virtual MStatus extractGeometry();
  virtual MStatus writeToFile(std::ostream& os) = 0;

 protected:
  MObject
  findShader(const MObject& setNode);

  virtual MStatus
  outputSets(std::ostream& os);

  virtual MStatus
  outputSingleSet(std::ostream& os,
                  MString setName,
                  MIntArray faces,
                  MString textureName) = 0;
  static void
  outputTabs(std::ostream& os, unsigned int tabCount);

  MString				fCurrentUVSetName;

  MPointArray			fVertexArray;
  MColorArray			fColorArray;
  MFloatVectorArray	fNormalArray;
  MFloatVectorArray	fTangentArray;
  MFloatVectorArray	fBinormalArray;

  MFnMesh* fMesh;
  MDagPath* fDagPath;
  MObjectArray		fPolygonSets;
  MObjectArray		fPolygonComponents;
};
