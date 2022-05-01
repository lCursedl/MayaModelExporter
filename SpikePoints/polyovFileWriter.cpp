#include <maya/MIOStream.h>
#include <maya/MGlobal.h>
#include <maya/MIntArray.h>
#include <maya/MFnSet.h>
#include <maya/MItDependencyGraph.h>
#include <maya/MDagPath.h>
#include <maya/MFnMesh.h>
#include <maya/MPlug.h>
#include <maya/MIOStream.h>
#include <time.h>

#include <maya/MItMeshPolygon.h>

#include "polyovFileWriter.h"

#define DELIMITER "\t"
#define SHAPE_DIVIDER "*******************************************************************************\n"
#define HEADER_LINE "===============================================================================\n"
#define LINE "-------------------------------------------------------------------------------\n"

polyovFileWriter::polyovFileWriter(const MDagPath& dagPath, MStatus& status) :
  polyWriter(dagPath, status),
  fHeadUVSet(NULL) {}

polyovFileWriter::~polyovFileWriter() {
  if (NULL != fHeadUVSet) delete fHeadUVSet;
}

MStatus
polyovFileWriter::extractGeometry() {

  if (MStatus::kFailure == polyWriter::extractGeometry()) {
    return MStatus::kFailure;
  }

  MStringArray uvSetNames;
  if (MStatus::kFailure == fMesh->getUVSetNames(uvSetNames)) {
    MGlobal::displayError("MFnMesh::getUVSetNames");
    return MStatus::kFailure;
  }

  unsigned int uvSetCount = uvSetNames.length();
  unsigned int i;

  UVSet* currUVSet = NULL;

  for (i = 0; i < uvSetCount; ++i) {
    if (0 == i) {
      currUVSet = new UVSet;
      fHeadUVSet = currUVSet;
    }
    else {
      currUVSet->next = new UVSet;
      currUVSet = currUVSet->next;
    }

    currUVSet->name = uvSetNames[i];
    currUVSet->next = NULL;

    if (MStatus::kFailure == fMesh->getUVs(currUVSet->uArray, currUVSet->vArray, &currUVSet->name)) {
      return MStatus::kFailure;
    }
  }

  return MStatus::kSuccess;
}


MStatus
polyovFileWriter::writeToFile(ostream& os) {

  MGlobal::displayInfo("Exporting " + fMesh->partialPathName());

  os << SHAPE_DIVIDER;
  os << "Shape:  " << fMesh->partialPathName() << "\n";
  os << SHAPE_DIVIDER;
  os << "\n";

  if (MStatus::kFailure == outputVertexInfo(os)) {
    return MStatus::kFailure;
  }

  return MStatus::kSuccess;
}

MStatus
polyovFileWriter::outputVertexInfo(ostream& os) {
  unsigned int faceCount = fMesh->numPolygons();
  unsigned i, j, indexCount;

  MStatus status;
  MIntArray indexArray;

  //output the header
  os << "Faces: " << faceCount << "\n";
  os << HEADER_LINE;
  os << "Format:  Vertex [x, y, z] | Normal [x, y, z] | Binormal [x, y, z] ";
  os << " | Tangent[x, y, z] | UV[x, y]\n";
  os << LINE;

  MIntArray normalIndexArray;
  int uvID;
  MVector binormalV, tangentV;

  for (i = 0; i < faceCount; ++i) {

    indexCount = fMesh->polygonVertexCount(i, &status);
    if (MStatus::kFailure == status) {
      MGlobal::displayError("MFnMesh::polygonVertexCount");
      return MStatus::kFailure;
    }

    status = fMesh->getPolygonVertices(i, indexArray);
    if (MStatus::kFailure == status) {
      MGlobal::displayError("MFnMesh::getPolygonVertices");
      return MStatus::kFailure;
    }

    status = fMesh->getFaceNormalIds(i, normalIndexArray);
    if (MStatus::kFailure == status) {
      MGlobal::displayError("MFnMesh::getFaceNormalIds");
      return MStatus::kFailure;
    }

    for (j = 0; j < indexCount; ++j) {
      
      status = fMesh->getFaceVertexBinormal(i, j, binormalV, MSpace::kWorld);

      if (MStatus::kFailure == status) {
        MGlobal::displayError("MFnMesh::getFaceVertexBinormal");
        return MStatus::kFailure;
      }

      status = fMesh->getFaceVertexTangent(i, j, tangentV, MSpace::kWorld);

      if (MStatus::kFailure == status) {
        MGlobal::displayError("MFnMesh::getFaceVertexTangent");
        return MStatus::kFailure;
      }

      //output the face, face vertex index, vertex index, normal index, color index
      //for the current vertex on the current face
      os << fVertexArray[indexArray[j]].x << ","
                << fVertexArray[indexArray[j]].y << ","
                << fVertexArray[indexArray[j]].z
                << DELIMITER
         << fNormalArray[normalIndexArray[j]].x << ","
                << fNormalArray[normalIndexArray[j]].y << ","
                << fNormalArray[normalIndexArray[j]].z
                << DELIMITER
         << binormalV.x << ","
                << binormalV.y << ","
                << binormalV.z
                << DELIMITER
         << tangentV.x << ","
                << tangentV.y << ","
                << tangentV.z;

      //output each uv set index for the current vertex on the current face
      for (UVSet* currUVSet = fHeadUVSet; currUVSet != NULL; currUVSet = currUVSet->next) {
        status = fMesh->getPolygonUVid(i, j, uvID, &currUVSet->name);
        if (MStatus::kFailure == status) {
          MGlobal::displayError("MFnMesh::getPolygonUVid");
          return MStatus::kFailure;
        }
        os << DELIMITER
        << currUVSet->uArray[uvID] << "," << currUVSet->vArray[uvID] << "\n";
      }
    }
  }
  os << "\n";
  return MStatus::kSuccess;
}

MStatus
polyovFileWriter::outputSingleSet(ostream& os,
                               MString setName,
                               MIntArray faces,
                               MString textureName) {
  unsigned int i;
  unsigned int faceCount = faces.length();

  os << "Set:  " << setName << "\n";
  os << HEADER_LINE;
  os << "Faces:  ";
  for (i = 0; i < faceCount; ++i) {
    os << faces[i] << " ";
  }
  os << "\n";
  if (textureName == "") {
    textureName = "none";
  }
  os << "Texture File: " << textureName << "\n";
  os << "\n\n";
  return MStatus::kSuccess;
}
