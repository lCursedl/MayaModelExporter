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

  /*if (MStatus::kFailure == outputFaces(os)) {
    return MStatus::kFailure;
  }*/

  /*if (MStatus::kFailure == outputVertices(os)) {
    return MStatus::kFailure;
  }*/

  if (MStatus::kFailure == outputVertexInfo(os)) {
    return MStatus::kFailure;
  }

  /*if (MStatus::kFailure == outputNormals(os)) {
    return MStatus::kFailure;
  }*/

  /*if (MStatus::kFailure == outputTangents(os)) {
    return MStatus::kFailure;
  }*/

  /*if (MStatus::kFailure == outputBinormals(os)) {
    return MStatus::kFailure;
  }*/

  /*if (MStatus::kFailure == outputColors(os)) {
    return MStatus::kFailure;
  }*/

  /*if (MStatus::kFailure == outputUVs(os)) {
    return MStatus::kFailure;
  }*/

  /*if (MStatus::kFailure == outputSets(os)) {
    return MStatus::kFailure;
  }
  os << "\n\n";*/

  return MStatus::kSuccess;
}


MStatus
polyovFileWriter::outputFaces(ostream& os) {

  //MIntArray triangleCounts;
  //MIntArray triangleVerts;

  //if (MStatus::kFailure == fMesh->getTriangles(triangleCounts, triangleVerts)) {
  //  return MStatus::kFailure;
  //}

  //unsigned int tVerts = triangleVerts.length();
  //unsigned int tCounts = triangleCounts.length();
  //MIntArray indexArray;

  //os << "Indices:  " << tVerts << "\n";
  //os << LINE;

  //for (unsigned int i = 0; i < tVerts; i+=3) {
  //  os << triangleVerts[i] << ", "
  //     << triangleVerts[i+1] << ", "
  //     << triangleVerts[i+2] << "\n";
  //}

  ///*for (unsigned int i = 0; i < tVerts; ++i) {
  //  os << triangleCounts[i] << "\n";
  //}*/

  unsigned int faceCount = fMesh->numPolygons();
  if (0 == faceCount) {
    return MStatus::kFailure;
  }

  MStatus status;
  MIntArray indexArray;

  os << "Faces:  " << faceCount << "\n";
  os << HEADER_LINE;
  os << "Format:  Index|Vertex Indices\n";
  os << LINE;

  unsigned int i;
  for (i = 0; i < faceCount; i++) {
    os << i << DELIMITER;

    unsigned int indexCount = fMesh->polygonVertexCount(i, &status);
    if (MStatus::kFailure == status) {
      MGlobal::displayError("MFnMesh::polygonVertexCount");
      return MStatus::kFailure;
    }

    status = fMesh->getPolygonVertices(i, indexArray);
    if (MStatus::kFailure == status) {
      MGlobal::displayError("MFnMesh::getPolygonVertices");
      return MStatus::kFailure;
    }

    unsigned int j;
    for (j = 0; j < indexCount; j++) {
      os << indexArray[j] << " ";
    }

    os << "\n";
  }

  os << "\n\n";

  return MStatus::kSuccess;
}


MStatus
polyovFileWriter::outputVertices(ostream& os) {

  unsigned int vertexCount = fVertexArray.length();
  unsigned i;

  if (0 == vertexCount) {
    return MStatus::kFailure;
  }

  os << "Vertices:  " << vertexCount << "\n";
  os << HEADER_LINE;
  os << "Format:  Vertex|(x, y, z)\n";
  os << LINE;
  for (i = 0; i < vertexCount; i++) {
    os << i << DELIMITER << "("
      << fVertexArray[i].x << ", "
      << fVertexArray[i].y << ", "
      << fVertexArray[i].z << ")\n";
  }
  os << "\n\n";

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
  os << "Format:  Vertex [x, y, z] | Normal [x, y, z]| UV [x, y]" << DELIMITER;
  os << LINE;

  MIntArray normalIndexArray;
  int uvID;

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
      //status = fMesh->getFaceVertexColorIndex(i, j, colorIndex);

      //output the face, face vertex index, vertex index, normal index, color index
      //for the current vertex on the current face
      os << "[" << fVertexArray[indexArray[j]].x << ","
                << fVertexArray[indexArray[j]].y << ","
                << fVertexArray[indexArray[j]].z << "]"
        << DELIMITER << DELIMITER << DELIMITER
        << fNormalArray[normalIndexArray[j]];

      //output each uv set index for the current vertex on the current face
      for (UVSet* currUVSet = fHeadUVSet; currUVSet != NULL; currUVSet = currUVSet->next) {
        status = fMesh->getPolygonUVid(i, j, uvID, &currUVSet->name);
        if (MStatus::kFailure == status) {
          MGlobal::displayError("MFnMesh::getPolygonUVid");
          return MStatus::kFailure;
        }
        os << DELIMITER << DELIMITER << "(" << currUVSet->uArray[uvID] << ", " << currUVSet->vArray[uvID] << ")\n";
      }
    }
  }
  os << "\n";
  return MStatus::kSuccess;
}


MStatus polyovFileWriter::outputNormals(ostream& os) {
  unsigned int normalCount = fNormalArray.length();
  if (0 == normalCount) {
    return MStatus::kFailure;
  }

  os << "Normals:  " << normalCount << "\n";
  os << HEADER_LINE;
  os << "Format:  Index|[x, y, z]\n";
  os << LINE;

  unsigned int i;
  for (i = 0; i < normalCount; ++i) {
    os << i << DELIMITER << "["
      << fNormalArray[i].x << ", "
      << fNormalArray[i].y << ", "
      << fNormalArray[i].z << "]\n";
  }
  os << "\n\n";

  return MStatus::kSuccess;
}

MStatus polyovFileWriter::outputTangents(ostream& os) {
  unsigned int tangentCount = fTangentArray.length();
  if (0 == tangentCount) {
    return MStatus::kFailure;
  }

  os << "Tangents:  " << tangentCount << "\n";
  os << HEADER_LINE;
  os << "Format:  Index|[x, y, z]\n";
  os << LINE;

  unsigned int i;
  for (i = 0; i < tangentCount; i++) {
    os << i << DELIMITER << "["
      << fTangentArray[i].x << ", "
      << fTangentArray[i].y << ", "
      << fTangentArray[i].z << "]\n";
  }
  os << "\n\n";

  return MStatus::kSuccess;
}

MStatus polyovFileWriter::outputBinormals(ostream& os) {
  unsigned int binormalCount = fBinormalArray.length();
  if (0 == binormalCount) {
    return MStatus::kFailure;
  }

  os << "Binormals:  " << binormalCount << "\n";
  os << HEADER_LINE;
  os << "Format:  Index|[x, y, z]\n";
  os << LINE;

  unsigned int i;
  for (i = 0; i < binormalCount; ++i) {
    os << i << DELIMITER << "["
      << fBinormalArray[i].x << ", "
      << fBinormalArray[i].y << ", "
      << fBinormalArray[i].z << "]\n";
  }
  os << "\n\n";

  return MStatus::kSuccess;
}

MStatus polyovFileWriter::outputColors(ostream& os) {
  unsigned int colorCount = fColorArray.length();
  if (0 == colorCount) {
    return MStatus::kFailure;
  }

  os << "Colors:  " << colorCount << "\n";
  os << HEADER_LINE;
  os << "Format:  Index|R G B A\n";
  os << LINE;

  unsigned int i;
  for (i = 0; i < colorCount; ++i) {
    os << i << DELIMITER
      << fColorArray[i].r << " "
      << fColorArray[i].g << " "
      << fColorArray[i].b << " "
      << fColorArray[i].a << "\n";
  }
  os << "\n\n";

  return MStatus::kSuccess;
}


MStatus polyovFileWriter::outputUVs(ostream& os) {
  UVSet* currUVSet;
  unsigned int i, uvCount;
  for (currUVSet = fHeadUVSet; currUVSet != NULL; currUVSet = currUVSet->next) {
    if (currUVSet->name == fCurrentUVSetName) {
      os << "Current ";
    }

    os << "UV Set:  " << currUVSet->name << "\n";
    uvCount = currUVSet->uArray.length();
    os << "UV Count:  " << uvCount << "\n";
    os << HEADER_LINE;
    os << "Format:  Index|(u, v)\n";
    os << LINE;
    for (i = 0; i < uvCount; i++) {
      os << i << DELIMITER << "(" << currUVSet->uArray[i] << ", " << currUVSet->vArray[i] << ")\n";
    }
    os << "\n";
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
