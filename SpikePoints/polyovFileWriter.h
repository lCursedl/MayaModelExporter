#pragma once
#include "polyWriter.h"

struct UVSet {
  MFloatArray	uArray;
  MFloatArray	vArray;
  MString		name;
  UVSet* next;
};

class polyovFileWriter : public polyWriter
{
 public:
   polyovFileWriter(const MDagPath& dagPath, MStatus& status);
   ~polyovFileWriter() override;

   MStatus
   extractGeometry() override;
   MStatus
   writeToFile(std::ostream& os) override;
 private:
   MStatus
   outputSingleSet(std::ostream& os,
                   MString setName,
                   MIntArray faces,
                   MString textureName) override;
   
   MStatus
   outputVertexInfo(std::ostream& os);

   UVSet* fHeadUVSet;
};
