#pragma once

enum class ExitCode
{
  Ok,
  InvalidCli,
  SyntaxErrors,
  FailedToWriteFile,
  BytecodeParseError,
  ExecutionError
};