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
  for (char c = stream.peek(); !stream.fail() && !stream.eof();
       c = stream.peek()) {
    if (c == '"') {
      if (!inStr)
        inStr = true;
      else {
        stream.get();
        curr += c;
        break;
      }
    }
    if (!inStr && (c == ' ' || c == '\n' || c == '\r' || stream.fail())) {
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

std::string BytecodeParser::buildDepStr() {
  std::string depStr = "";
  for (char c = stream.peek(); c != ' ' && !stream.fail() && !stream.eof();
       c = stream.peek()) {
    depStr += c;
    stream.get();
  }

  return depStr;
}

std::vector<InstrDependent>
BytecodeParser::buildDependents(std::string depStr) {

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

    int idNum = std::atoi(id.c_str());
    deps.emplace_back(&instructions[idNum],
                      index != "" ? std::make_optional(std::atoi(index.c_str()))
                                  : std::nullopt);
  }

  return deps;
}

void BytecodeParser::buildSingleInstruction() {
  Instruction instr((int)instructions.size());

  // Parse depCount
  std::string curr = "";
  for (char c = stream.peek();
       c != ' ' && c != '\n' && c != '\r' && !stream.fail();
       c = stream.peek()) {
    stream.get();
    curr += c;
  }

  // Skip ' '
  if (stream.peek() == ' ')
    stream.get();

  instr.depCount = std::atoi(curr.c_str());

  depStrs.push_back(buildDepStr());
  stream.get(); // Consume ' '

  // Parse type
  curr = "";
  for (char c = stream.peek();
       c != ' ' && c != '\n' && c != '\r' && !stream.fail();
       c = stream.peek()) {
    stream.get();
    curr += c;
  }

  instr.type = (InstructionType)std::atoi(curr.c_str());

  // Parse args
  while (stream.peek() != '\n' && stream.peek() != '\r' && !stream.fail() &&
         !stream.eof()) {
    Value arg = buildArg();
    instr.bytecodeArgs.push_back(arg);
  }

  if (cliArgs.verbose)
    log(LOCATION, "Built instruction: {}", instr.toString());

  instructions.push_back(instr);
  return;
}

void BytecodeParser::buildInstructions() {
  while (!stream.fail()) {
    buildSingleInstruction();

    stream.get(); // Consume '\n'
  }

  for (int i = 0; i < instructions.size(); i++) {
    instructions[i].dependents = buildDependents(depStrs[i]);
  }

  if (cliArgs.verbose)
    log(LOCATION, "Parsed {} instructions", instructions.size());
}

BytecodeParser::BytecodeParser(const CliArgs &cliArgs,
                               std::vector<Instruction> &instructions,
                               std::istream &stream)
    : cliArgs(cliArgs), instructions(instructions), stream(stream) {}