#include "JsonParser.h"
#include <filesystem>
namespace fs = std::filesystem;

void convertDiagragphIntoSVG() {
  for (auto &currFile : fs::directory_iterator(fs::current_path())) {
    if (currFile.is_regular_file()) {
      if (currFile.path().extension() == ".dia") {
        string command = "dot -Tsvg " + currFile.path().string() + " > " +
                         currFile.path().stem().string() + ".svg";
        system(command.c_str());
      }
    }
  }
}

int main(int argc, char *argv[]) {
  if (argc < 2) {
    fprintf(stderr, "inputer json file name\n");
    return -1;
  }

  string fileName = argv[1];
  Json::StreamWriterBuilder builder;
  JsonParser jParser(fileName);
  jParser.createBasicBlocks();
  jParser.buildCFG();
  convertDiagragphIntoSVG();
  return 0;
}
