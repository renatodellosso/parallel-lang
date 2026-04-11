#include "bytecodeParser.hpp"
#include "../logging.hpp"
#include "../utils.hpp"

#define LOCATION "BytecodeParser"

Value BytecodeParser::buildArg() {
  std::string curr = "";

  // Consume spaces
  while (stream.peek() == ' ') {
    stream.get();
  }

  bool inStr = false;
  bool endInstr = false;
  for (char c = stream.peek(); true; c = stream.peek()) {
    if (c == '"') {
      if (!inStr)
        inStr = true;
      else {
        stream.get();
        curr += c;
        break;
      }
    }
    if (!inStr && (c == ' ' || c == '\n' || c == '\r' || c == ';')) {
      break;
    }

    stream.get();
    curr += c;
  }

  Value arg;
  if (curr.at(0) == '"' && curr.at(curr.length() - 1) == '"') {
    // String
    arg.type = ValueType::String;
    arg.val = curr.substr(1, curr.length() - 2); // Remove ""
  } else if (curr == "true") {
    // bool (true)
    arg.type = ValueType::Bool;
    arg.val = true;
  } else if (curr == "false") {
    // bool (true)
    arg.type = ValueType::Bool;
    arg.val = false;
  } else if (isInteger(curr)) {
    // int
    arg.type = ValueType::Integer;
    arg.val = std::atoi(curr.c_str());
  } else {
    arg.type = ValueType::Identifier;
    arg.val = curr;
  }

  return arg;
}

std::vector<InstrDependent> BytecodeParser::buildDependents() {
  // Build overall string
  std::string depStr = "";
  for (char c = stream.peek(); c != ' '; c = stream.peek()) {
    depStr += c;
    stream.get();
  }

  // Split string into dependents
  std::vector<std::string> rawDeps;
  std::string curr = "";
  for (auto c : depStr) {
    if (c != ',')
      curr += c;
    else {
      rawDeps.push_back(curr);
      curr = "";
    }
  }
  // Push last string
  if (curr != "")
    rawDeps.push_back(curr);

  // Convert strings to Dependents
  std::vector<InstrDependent> deps;
  for (auto str : rawDeps) {
    std::string id = "", index = "", curr = "";
    for (auto c : str) {
      if (c != '.')
        curr += c;
      else {
        id = curr;
        curr = "";
      }
    }
    // Check if dep has arg index
    if (id != "" && curr != "")
      index = curr;
    else if (id == "")
      id = curr;

    deps.emplace_back(std::atoi(id.c_str()),
                      index != "" ? std::make_optional(std::atoi(index.c_str()))
                                  : std::nullopt);
  }

  return deps;
}

void BytecodeParser::buildSingleInstruction() {
  Instruction instr((int)instructions.size());

  // Parse depCount
  std::string curr = "";
  for (char c = stream.peek(); c != ' ' && c != ';' && c != '\n' && c != '\r';
       c = stream.peek()) {
    stream.get();
    curr += c;
  }

  // Skip ' '
  if (stream.peek() == ' ')
    stream.get();

  instr.depCount = std::atoi(curr.c_str());

  instr.dependents = buildDependents();
  stream.get(); // Consume ' '

  // Parse type
  curr = "";
  for (char c = stream.peek(); c != ' ' && c != ';' && c != '\n' && c != '\r';
       c = stream.peek()) {
    stream.get();
    curr += c;
  }

  instr.type = (InstructionType)std::atoi(curr.c_str());

  // Parse args
  while (stream.peek() != ';' && stream.peek() != '\n' &&
         stream.peek() != '\r' && !stream.eof()) {
    Value arg = buildArg();
    instr.bytecodeArgs.push_back(arg);
  }

  if (stream.peek() == ';') {
    instr.endsLine = true;
    stream.get();
  }

  instructions.push_back(instr);
  return;
}

void BytecodeParser::buildInstructions() {
  while (!stream.eof()) {
    buildSingleInstruction();

    stream.get(); // Consume '\n'
  }

  if (cliArgs.verbose)
    log(LOCATION, "Parsed {} instructions", instructions.size());
}

BytecodeParser::BytecodeParser(const CliArgs &cliArgs,
                               std::vector<Instruction> &instructions,
                               std::istream &stream)
    : cliArgs(cliArgs), instructions(instructions), stream(stream) {}