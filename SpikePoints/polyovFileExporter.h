#pragma once
#include "polyExporter.h"
#include <iosfwd>

class polyovFileExporter : public polyExporter {
 public:
  polyovFileExporter() = default;
  ~polyovFileExporter() override = default;

  static	void*
  creator();
  MString
  defaultExtension() const override;
  MStatus
  initializePlugin(MObject obj);
  MStatus
  uninitializePlugin(MObject obj);

 private:
   polyWriter*
   createPolyWriter(const MDagPath dagPath, MStatus& status) override;
   void
   writeHeader(std::ostream& os) override;
};
