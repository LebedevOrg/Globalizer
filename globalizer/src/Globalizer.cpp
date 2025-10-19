﻿#include "Globalizer.h"

// ------------------------------------------------------------------------------------------------
void GlobalizerInitialization(int argc, char* argv[], bool isMPIInit, 
  bool isPrintParameters, std::string mLogFileName, int processCount,
  int processNumber, bool isPrintToFile,
  std::string* errorsName, int* errorsCode, int errorsCount)
{
  if (isPrintParameters)
  {
    std::cout << "\n\n" << std::endl;
    for (int i = 1; i < argc; i++)
    {
      std::cout << argv[i] << " ";
    }
    std::cout << "\n\n" << std::endl;
  }

  if (isMPIInit)
    MPI_Init(&argc, &argv);

  // Инициализация параметров
  parameters.Init(argc, argv, true);

  if (mLogFileName == "")
    mLogFileName = parameters.logFileNamePrefix;

  if (processCount == -1)
    processCount = parameters.GetProcNum();

  if (processNumber == -1)
    processNumber = parameters.GetProcRank();

  // Инициализация системы вывода и печати ошибок
  OutputMessage::Init(isMPIInit, mLogFileName, processCount,
    processNumber, isPrintToFile, errorsName, errorsCode, errorsCount);

  if (!parameters.IsStart())
  {
    print << "Need command-line arguments!";
    _ERROR_(-1);
  }

  if (isPrintParameters)
  {
    if (parameters.GetProcRank() == 0 && !parameters.disablePrintParameters)
    {
      parameters.PrintParameters();

      unsigned long size = 256;
      char* CompName = 0;
#ifdef WIN32
      LPWSTR buffer = new wchar_t[size];
      for (unsigned long i = 0; i < size; i++)
        buffer[i] = 0;
      GetComputerNameW(buffer, &size);
      CompName = new char[size + 1];
      for (unsigned long i = 0; i < size; i++)
        CompName[i] = (char)buffer[i];
      CompName[size] = 0;
      size++;
#else
      char* hostname = new char[size];
      for (unsigned long i = 0; i < size; i++)
        hostname[i] = 0;
      gethostname(hostname, 256);
      size = (unsigned long)strlen(hostname);
      CompName = new char[size + 1];
      for (unsigned long i = 0; i < size; i++)
        CompName[i] = (char)hostname[i];
      CompName[size] = 0;
      size++;
#endif

      print << CompName << "\tProcRank = " << parameters.GetProcRank() << "\tProcNum = " << parameters.GetProcNum() << "\n";
    }
  }
}
