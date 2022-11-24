#include "GraphImpl.h"
#include <filesystem>
#include <fstream>
#include <iostream>
#include <jsoncpp/json/config.h>
#include <jsoncpp/json/json.h>
#include <jsoncpp/json/value.h>
#include <jsoncpp/json/writer.h>
#include <string>
namespace fs = std::filesystem;
using namespace std;

class JsonParser {
private:
  ifstream m_inputFile;
  string m_inputFileName;
  Json::Reader m_reader;
  Json::Value m_jsonData;
  Json::StreamWriterBuilder builder;
  vector<Json::Value> m_functionBlocks;

public:
  JsonParser(string inputFileName) : m_inputFileName(inputFileName) {
    m_inputFile = ifstream{inputFileName};
    m_reader.parse(m_inputFile, m_jsonData);
  }

  void createBasicBlocks() {
    Json::Value functions = m_jsonData["functions"];

    Json::Value blocks;
    blocks = Json::objectValue;

    for (int index = 0; index < functions.size(); ++index) {
      int BLOCK_COUNT = 100;
      string currBlockName = "block_" + to_string(BLOCK_COUNT++);
      Json::Value parent;
      parent["blocks"] = Json::arrayValue;
      Json::Value instructions = functions[index]["instrs"];
      if (instructions.size() == 0)
        continue;

      blocks = Json::objectValue;
      blocks[currBlockName] = Json::objectValue;
      blocks[currBlockName]["instrs"] = Json::arrayValue;
      Json::Value blockName = Json::stringValue;
      blockName = functions[index]["name"].asString();
      blocks[currBlockName]["name"] = blockName;

      for (int j = 0; j < instructions.size(); ++j) {
        if (!isTerminator(instructions[j]["op"]) && !isLabel(instructions[j])) {
          blocks[currBlockName]["instrs"].append(instructions[j]);
        } else {
          if (isLabel(instructions[j])) {
            if (blocks[currBlockName]["instrs"].size() != 0) {
              parent["blocks"].append(blocks);
              currBlockName = "block_" + to_string(BLOCK_COUNT++);
            }

            blocks = Json::objectValue;
            blocks[currBlockName] = Json::objectValue;
            blocks[currBlockName]["instrs"] = Json::arrayValue;
            blocks[currBlockName]["instrs"].append(instructions[j]);
            blockName = instructions[j]["label"].asString();
            blocks[currBlockName]["name"] = blockName;
          } else {

            blocks[currBlockName]["instrs"].append(instructions[j]);
            parent["blocks"].append(blocks);
            currBlockName = "block_" + to_string(BLOCK_COUNT++);
            blockName = currBlockName;
            blocks = Json::objectValue;
            blocks[currBlockName]["instrs"] = Json::arrayValue;
            blocks[currBlockName]["name"] = blockName;
            if (instructions[j]["op"].asString() == "jmp") {
              ++j;
              while (!isLabel(instructions[j])) {
                ++j;
              }
              --j;
            }

            if ((j + 1) < instructions.size() &&
                !isLabel(instructions[j + 1])) {
              blocks[currBlockName]["name"] = blockName;
            }
          }
        }
      }

      if (blocks[currBlockName]["instrs"].size() != 0) {
        parent["blocks"].append(blocks);
        m_functionBlocks.push_back(parent);
      } else {
        m_functionBlocks.push_back(parent);
      }
      BLOCK_COUNT = 0;
    }
  }

  void buildCFG() {
    for (auto itr = m_functionBlocks.begin(); itr != m_functionBlocks.end();
         ++itr) {
      fs::path inputFilePath(m_inputFileName.c_str());
      string fileName = inputFilePath.stem().c_str() +
                        to_string(itr - m_functionBlocks.begin()) + ".dia";
      Graph g((*itr)["blocks"].size(), getBasicBlocks((*itr)["blocks"]),
              fileName);
      createCFG(g, (*itr)["blocks"]);
      g.createDotFile();
    }
  }

  void createCFG(Graph &g, Json::Value basicBlocks) {
    for (int i = 0; i < basicBlocks.size(); ++i) {
      Json::Value currBlock = basicBlocks[i];
      vector<string> x = basicBlocks[i].getMemberNames();
      if (x.size() != 1) {
        return;
      }

      string blockName = x[0];
      if (hasBranchInst(currBlock[blockName])) {
        vector<string> branchLabels = getBranchLabels(currBlock[blockName]);
        g.addEdge(currBlock[blockName]["name"].asString(), branchLabels[0],
                  "true");
        g.addEdge(currBlock[blockName]["name"].asString(), branchLabels[1],
                  "false");
      } else if (hasJmpInst(currBlock[blockName])) {
        string jmpLabel = getJmpLabel(currBlock[blockName]);
        g.addEdge(currBlock[blockName]["name"].asString(), jmpLabel);
      } else if (!hasReturnInst(currBlock[blockName])) {
        if (i + 1 < basicBlocks.size()) {
          vector<string> y = basicBlocks[i + 1].getMemberNames();
          if (y.size() != 1)
            return;
          string nextBlockName = y[0];
          g.addEdge(currBlock[blockName]["name"].asString(),
                    basicBlocks[i + 1][nextBlockName]["name"].asString());
        }
      }
    }
  }
  static bool isLabel(Json::Value inputJsonValue);
  static string getJmpLabel(Json::Value inputJsonValue);
  static vector<string> getBasicBlocks(Json::Value basicBlocks);
  static bool hasReturnInst(Json::Value inputJsonValue);
  static bool hasJmpInst(Json::Value inputJsonValue);
  static vector<string> getBranchLabels(Json::Value inputJsonValue);
  static bool hasBranchInst(Json::Value inputJsonValue);
  static bool isTerminator(Json::Value inputJsonValue);
};

bool JsonParser::isLabel(Json::Value inputJsonValue) {
  return inputJsonValue.isMember("label");
}

string JsonParser::getJmpLabel(Json::Value inputJsonValue) {
  if (!inputJsonValue.isMember("instrs"))
    return "";

  Json::Value instructions = inputJsonValue["instrs"];
  for (int i = 0; i < instructions.size(); ++i) {
    if (instructions[i]["op"].asString() == "jmp") {
      return instructions[i]["labels"][0].asString();
    }
  }
  return "";
}

bool JsonParser::hasJmpInst(Json::Value inputJsonValue) {
  if (!inputJsonValue.isMember("instrs"))
    return "";

  Json::Value instructions = inputJsonValue["instrs"];
  for (int i = 0; i < instructions.size(); ++i) {
    if (instructions[i]["op"].asString() == "jmp")
      return true;
  }
  return false;
}

vector<string> JsonParser::getBranchLabels(Json::Value inputJsonValue) {
  vector<string> result;
  if (!inputJsonValue.isMember("instrs"))
    return result;

  Json::Value instructions = inputJsonValue["instrs"];
  for (int i = 0; i < instructions.size(); ++i) {
    if (instructions[i]["op"].asString() == "br") {
      result.push_back(instructions[i]["labels"][0].asString());
      result.push_back(instructions[i]["labels"][1].asString());
      return result;
    }
  }
  return result;
}

bool JsonParser::hasReturnInst(Json::Value inputJsonValue) {
  if (!inputJsonValue.isMember("instrs"))
    return false;

  Json::Value instructions = inputJsonValue["instrs"];
  for (int i = 0; i < instructions.size(); ++i) {
    if (instructions[i]["op"].asString() == "ret")
      return true;
  }
  return false;
}

bool JsonParser::hasBranchInst(Json::Value inputJsonValue) {
  if (!inputJsonValue.isMember("instrs"))
    return false;

  Json::Value instructions = inputJsonValue["instrs"];
  for (int i = 0; i < instructions.size(); ++i) {
    if (instructions[i]["op"].asString() == "br")
      return true;
  }
  return false;
}

vector<string> JsonParser::getBasicBlocks(Json::Value basicBlocks) {
  vector<string> blockNames;
  for (int i = 0; i < basicBlocks.size(); ++i) {
    vector<string> x = basicBlocks[i].getMemberNames();
    for (int j = 0; j < x.size(); ++j) {
      blockNames.push_back(basicBlocks[i][x[j]]["name"].asString());
    }
  }
  return blockNames;
}

bool JsonParser::isTerminator(Json::Value inputJsonValue) {
  return (inputJsonValue.asString() == "br" ||
          inputJsonValue.asString() == "ret" ||
          inputJsonValue.asString() == "jmp");
}
