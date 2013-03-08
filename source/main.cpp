// ===================================================================
// $Id: main.cpp,v 1.1 2007/02/14 12:03:52 bittner Exp $

// Qt headers
#include <QApplication>

// Local headers
#include "Scene.h"
#include "AppEnvironment.h"
#include "BasicRenderer.h"
#include "GvsPreprocessor.h"
#include "ObjParser.h"

// If true assumes that the environment file is in the same directory as the
// the executable. Otherwise uses current directory
#define ENVIRONMENT_FILE_IN_EXE_PATH false

int
main(int argc, char **argv)
{
  // Open debug stream: use debug for various info/warning/error messages
  // Do not use it for statistics! Create other streams for that.
  // Prefix your messages with Info:/Warning:/Error:
  Debug.open("debug.log");
  Debug<<"Info: Application started\n";
  
  // setup environment
  Environment *environment = new AppEnvironment;
  // register all basic options
  environment->RegisterOptions();
  //  environment->PrintUsage(cout);

  if (!environment->Parse(argc, argv, ENVIRONMENT_FILE_IN_EXE_PATH)) {
    cerr<<"Error parsing environment file!";
    exit(1);
  }
  
  Environment::SetSingleton(environment);

  string filename;
  Environment::GetSingleton()->GetStringValue("Scene.filename", filename);
  Debug<<"Info: scene filename = "<<filename.c_str()<<endl;
  
  InitTiming();

  // Parse the scene
  Scene *scene = new Scene;
  scene->Parse(filename);

  bool exportAll = false;

  if (exportAll) {
	Parser *parser;
	// Currently only .obj is supported
	parser = new ObjParser;
	parser->ExportFile("all.obj", *scene, 0);
	delete parser;
  }

  // Here is the scene preprocessing, for example, construction
  // of data structures needed by the application
  scene->Preprocess();

  // Computes the result using a data structure
  GvsPreprocessor *preprocessor = new GvsPreprocessor(scene);
  preprocessor->ComputeVisibility();
  
  // now do the Qt stuff
  if (1) {
	QApplication app(argc, argv);
	BasicRenderer renderer(scene);
	renderer.show();
	return app.exec(); // endless loop started
  }
  return 0;
}

// End of file main.cpp
