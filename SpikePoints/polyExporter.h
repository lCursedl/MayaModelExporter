#pragma once
#include <maya/MPxFileTranslator.h>
#include <iosfwd>

class polyWriter;

class polyExporter : public MPxFileTranslator
{
 public:
  polyExporter() = default;
  ~polyExporter() override = default;

  MStatus
  writer(const MFileObject& file,
         const MString& optionString,
         MPxFileTranslator::FileAccessMode mode) override;
  bool
  haveWriteMethod() const override;
  bool
  haveReadMethod() const override;
  bool
  canBeOpened() const override;
  MString
  defaultExtension() const override = 0;
 
 protected:
   virtual	bool
   isVisible(MFnDagNode& fnDag, MStatus& status);
   virtual	MStatus
   exportAll(std::ostream& os);
   virtual	MStatus
   exportSelection(std::ostream& os);
   virtual void	
   writeHeader(std::ostream& os);
   virtual void
   writeFooter(std::ostream& os);
   virtual MStatus
   processPolyMesh(const MDagPath dagPath, std::ostream& os);
   virtual polyWriter*
   createPolyWriter(const MDagPath dagPath, MStatus& status) = 0;
};
