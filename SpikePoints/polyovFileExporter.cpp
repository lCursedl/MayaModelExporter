#include <maya/MFnPlugin.h>
#include <maya/MDagPath.h>

#include "polyovFileExporter.h"
#include "polyovFileWriter.h"

#include <sstream>

void*
polyovFileExporter::creator() {
  return new polyovFileExporter();
}


MString
polyovFileExporter::defaultExtension() const {
  return MString("ovFile");
}


MStatus
initializePlugin(MObject obj) {

  MStatus status;
  MFnPlugin plugin(obj, PLUGIN_COMPANY, "4.5", "Any");

  status = plugin.registerFileTranslator("ovFile",
    "",
    polyovFileExporter::creator,
    "",
    "option1=1",
    true);
  if (!status) {
    status.perror("registerFileTranslator");
    return status;
  }

  return status;
}


MStatus
uninitializePlugin(MObject obj) {

  MStatus   status;
  MFnPlugin plugin(obj);

  status = plugin.deregisterFileTranslator("RawText");
  if (!status) {
    status.perror("deregisterFileTranslator");
    return status;
  }

  return status;
}


void
polyovFileExporter::writeHeader(std::ostream& os) {
  os << "ovFile for Overdrive Engine\n"
    << "by Gustavo Alvarez\n"
    << "Type: Model\n"
    << "Delimiter = TAB\n"
    << "~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n\n";
}


polyWriter*
polyovFileExporter::createPolyWriter(const MDagPath dagPath, MStatus& status) {
  return new polyovFileWriter(dagPath, status);
}