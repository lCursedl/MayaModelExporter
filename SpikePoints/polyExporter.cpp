#include <maya/MStatus.h>
#include <maya/MString.h>
#include <maya/MObject.h>
#include <maya/MGlobal.h>
#include <maya/MItDag.h>
#include <maya/MDagPath.h>
#include <maya/MItSelectionList.h>
#include <maya/MPlug.h>

#include "polyExporter.h"
#include "polyWriter.h"

#include <fstream>
#include <ios>

MStatus
polyExporter::writer(const MFileObject& file,
                     const MString& optionString,
                     MPxFileTranslator::FileAccessMode mode) {
  const MString fileName = file.expandedFullName();

  std::ofstream newFile(fileName.asChar(), std::ios::out);
  if (!newFile) {
    MGlobal::displayError(fileName + ": could not be opened for reading");
    return MS::kFailure;
  }
  newFile.setf(std::ios::unitbuf);

  writeHeader(newFile);

  if (MPxFileTranslator::kExportAccessMode == mode) {
    if (MStatus::kFailure == exportAll(newFile)) {
      return MStatus::kFailure;
    }
  }
  else if (MPxFileTranslator::kExportActiveAccessMode == mode) {
    if (MStatus::kFailure == exportSelection(newFile)) {
      return MStatus::kFailure;
    }
  }
  else {
    return MStatus::kFailure;
  }

  writeFooter(newFile);
  newFile.flush();
  newFile.close();

  MGlobal::displayInfo("Export to " + fileName + " successful!");
  return MS::kSuccess;
}


bool
polyExporter::haveWriteMethod() const {
  return true;
}


bool
polyExporter::haveReadMethod() const {
  return false;
}


bool
polyExporter::canBeOpened() const {
  return true;
}


MStatus
polyExporter::exportAll(std::ostream& os) {
  MStatus status;

  MItDag itDag(MItDag::kDepthFirst, MFn::kMesh, &status);

  if (MStatus::kFailure == status) {
    MGlobal::displayError("MItDag::MItDag");
    return MStatus::kFailure;
  }

  for (; !itDag.isDone(); itDag.next()) {
    MDagPath dagPath;
    if (MStatus::kFailure == itDag.getPath(dagPath)) {
      MGlobal::displayError("MDagPath::getPath");
      return MStatus::kFailure;
    }

    MFnDagNode visTester(dagPath);

    if (isVisible(visTester, status) && MStatus::kSuccess == status) {
      if (MStatus::kFailure == processPolyMesh(dagPath, os)) {
        return MStatus::kFailure;
      }
    }
  }
  return MStatus::kSuccess;
}


MStatus
polyExporter::exportSelection(std::ostream& os) {
  MStatus status;

  MSelectionList selectionList;
  if (MStatus::kFailure == MGlobal::getActiveSelectionList(selectionList)) {
    MGlobal::displayError("MGlobal::getActiveSelectionList");
    return MStatus::kFailure;
  }

  MItSelectionList itSelectionList(selectionList, MFn::kMesh, &status);
  if (MStatus::kFailure == status) {
    return MStatus::kFailure;
  }

  for (itSelectionList.reset(); !itSelectionList.isDone(); itSelectionList.next()) {
    MDagPath dagPath;

    if (MStatus::kFailure == itSelectionList.getDagPath(dagPath)) {
      MGlobal::displayError("MItSelectionList::getDagPath");
      return MStatus::kFailure;
    }

    if (MStatus::kFailure == processPolyMesh(dagPath, os)) {
      return MStatus::kFailure;
    }
  }
  return MStatus::kSuccess;
}


MStatus
polyExporter::processPolyMesh(const MDagPath dagPath, std::ostream& os) {
  MStatus status;
  polyWriter* pWriter = createPolyWriter(dagPath, status);
  if (MStatus::kFailure == status) {
    delete pWriter;
    return MStatus::kFailure;
  }
  if (MStatus::kFailure == pWriter->extractGeometry()) {
    delete pWriter;
    return MStatus::kFailure;
  }
  if (MStatus::kFailure == pWriter->writeToFile(os)) {
    delete pWriter;
    return MStatus::kFailure;
  }
  delete pWriter;
  return MStatus::kSuccess;
}


bool polyExporter::isVisible(MFnDagNode& fnDag, MStatus& status) {
  if (fnDag.isIntermediateObject())
    return false;

  MPlug visPlug = fnDag.findPlug("visibility", true, &status);
  if (MStatus::kFailure == status) {
    MGlobal::displayError("MPlug::findPlug");
    return false;
  }
  else {
    bool visible;
    status = visPlug.getValue(visible);
    if (MStatus::kFailure == status) {
      MGlobal::displayError("MPlug::getValue");
    }
    return visible;
  }
}


void polyExporter::writeHeader(std::ostream& os) {
  os << "";
}


void polyExporter::writeFooter(std::ostream& os) {
  os << "";
}
