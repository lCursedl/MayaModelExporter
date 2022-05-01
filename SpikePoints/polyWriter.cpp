#include <maya/MIOStream.h>
#include <maya/MGlobal.h>
#include <maya/MFnSet.h>
#include <maya/MDagPath.h>
#include <maya/MItDependencyGraph.h>
#include <maya/MPlug.h>

#include <maya/MItMeshPolygon.h>

#include "polyWriter.h"

polyWriter::polyWriter(MDagPath dagPath, MStatus& status) {
  fDagPath = new MDagPath(dagPath);
  fMesh = new MFnMesh(*fDagPath, &status);
}

polyWriter::~polyWriter(){
  if (NULL != fDagPath) delete fDagPath;
  if (NULL != fMesh) delete fMesh;
}

MStatus polyWriter::extractGeometry() {

  if (MStatus::kFailure == fMesh->getPoints(fVertexArray, MSpace::kWorld)) {
    MGlobal::displayError("MFnMesh::getPoints");
    return MStatus::kFailure;
  }

  if (MStatus::kFailure == fMesh->getNormals(fNormalArray, MSpace::kWorld)) {
    MGlobal::displayError("MFnMesh::getNormals");
    return MStatus::kFailure;
  }
  if (MStatus::kFailure == fMesh->getCurrentUVSetName(fCurrentUVSetName)) {
    MGlobal::displayError("MFnMesh::getCurrentUVSetName");
    return MStatus::kFailure;
  }

  fDagPath->extendToShape();
  
  int instanceNum = 0;
  if (fDagPath->isInstanced()) {
    instanceNum = fDagPath->instanceNumber();
  }
  
  if (!fMesh->getConnectedSetsAndMembers(instanceNum, fPolygonSets, fPolygonComponents, true)) {
    MGlobal::displayError("MFnMesh::getConnectedSetsAndMembers");
    return MStatus::kFailure;
  }

  return MStatus::kSuccess;
}

void polyWriter::outputTabs(ostream& os, unsigned int tabCount) {
  unsigned int i;
  for (i = 0; i < tabCount; ++i) {
    os << "\t";
  }
}

MObject polyWriter::findShader(const MObject& setNode) {

  MFnDependencyNode fnNode(setNode);
  MPlug shaderPlug = fnNode.findPlug("surfaceShader", true);

  if (!shaderPlug.isNull()) {
    MPlugArray connectedPlugs;

    MStatus status;
    shaderPlug.connectedTo(connectedPlugs, true, false, &status);
    if (MStatus::kFailure == status) {
      MGlobal::displayError("MPlug::connectedTo");
      return MObject::kNullObj;
    }

    if (1 != connectedPlugs.length()) {
      MGlobal::displayError("Error getting shader for: " + fMesh->partialPathName());
    }
    else {
      return connectedPlugs[0].node();
    }
  }

  return MObject::kNullObj;
}

MStatus polyWriter::outputSets(ostream& os) {
  MStatus status;

  unsigned int setCount = fPolygonSets.length();
  if (setCount > 1) {
    setCount--;
  }

  MIntArray faces;

  unsigned int i;
  for (i = 0; i < setCount; ++i) {
    MObject set = fPolygonSets[i];
    MObject comp = fPolygonComponents[i];
    MFnSet fnSet(set, &status);
    if (MS::kFailure == status) {
      MGlobal::displayError("MFnSet::MFnSet");
      continue;
    }

    MItMeshPolygon itMeshPolygon(*fDagPath, comp, &status);

    if ((MS::kFailure == status)) {
      MGlobal::displayError("MItMeshPolygon::MItMeshPolygon");
      continue;
    }

    faces.setLength(itMeshPolygon.count());

    unsigned int j = 0;
    for (itMeshPolygon.reset(); !itMeshPolygon.isDone(); itMeshPolygon.next()) {
      faces[++j] = itMeshPolygon.index();
    }

    MObject shaderNode = findShader(set);
    if (MObject::kNullObj == shaderNode) {
      continue;
    }

    MPlug colorPlug = MFnDependencyNode(shaderNode).findPlug("color", true, &status);
    if (MS::kFailure == status) {
      MGlobal::displayError("MFnDependencyNode::findPlug");
      continue;
    }

    MItDependencyGraph itDG(colorPlug, MFn::kFileTexture,
      MItDependencyGraph::kUpstream,
      MItDependencyGraph::kBreadthFirst,
      MItDependencyGraph::kNodeLevel,
      &status);

    if (MS::kFailure == status) {
      MGlobal::displayError("MItDependencyGraph::MItDependencyGraph");
      continue;
    }

    itDG.disablePruningOnFilter();

    MString textureName("");
    if (itDG.isDone()) {
      if (MStatus::kFailure == outputSingleSet(os,
                                               MString(fnSet.name()),
                                               faces,
                                               textureName)) {
        return MStatus::kFailure;
      }
    }
    else {
      MObject textureNode = itDG.currentItem();
      MPlug filenamePlug = MFnDependencyNode(textureNode).findPlug("fileTextureName", true);
      filenamePlug.getValue(textureName);
      if (MStatus::kFailure == outputSingleSet(os,
                                               MString(fnSet.name()),
                                               faces,
                                               textureName)) {
        return MStatus::kFailure;
      }

    }
  }
  return MStatus::kSuccess;
}
