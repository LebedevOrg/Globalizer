/////////////////////////////////////////////////////////////////////////////
//                                                                         //
//             LOBACHEVSKY STATE UNIVERSITY OF NIZHNY NOVGOROD             //
//                                                                         //
//                       Copyright (c) 2016 by UNN.                        //
//                          All Rights Reserved.                           //
//                                                                         //
//  File:      problem_manager.cpp                                         //
//                                                                         //
//  Purpose:   Source file for dynamic libraries manager class             //
//                                                                         //
//                                                                         //
//  Author(s): Sovrasov V.                                                 //
//                                                                         //
/////////////////////////////////////////////////////////////////////////////

#include "ProblemManager.h"
#include <iostream>

// ------------------------------------------------------------------------------------------------
ProblemManager::ProblemManager() : mLibHandle(NULL), mProblem(NULL),
  mCreate(NULL), mDestroy(NULL)
{

}

// ------------------------------------------------------------------------------------------------
ProblemManager::~ProblemManager()
{
  FreeProblemLibrary();
}

// ------------------------------------------------------------------------------------------------
int ProblemManager::LoadProblemLibrary(const std::string& LibPath)
{
  if (mLibHandle)
    FreeProblemLibrary();
  #ifdef WIN32
    mLibHandle = LoadLibrary(TEXT(LibPath.c_str()));
    if (!mLibHandle)
    {
        std::cerr << "Cannot load library: " << TEXT(LibPath.c_str()) << std::endl;
        return ProblemManager::ERROR_;
    }
  #else
    mLibHandle = dlopen(LibPath.c_str(), RTLD_LAZY);
    if (!mLibHandle)
    {
        std::cerr << dlerror() << std::endl;
        return ProblemManager::ERROR_;
    }
  #endif
  #ifdef WIN32
    mCreate = (create_t*) GetProcAddress(mLibHandle, "create");
    mDestroy = (destroy_t*) GetProcAddress(mLibHandle, "destroy");
    if (!mCreate || !mDestroy)
    {
      std::cerr << "Cannot load symbols: " << GetLastError() << std::endl;
      FreeLibHandler();
      return ProblemManager::ERROR_;
    }
  #else
    dlerror();
    mCreate = (create_t*) dlsym(mLibHandle, "create");
    char* dlsym_error = dlerror();
    if (dlsym_error)
    {
      mCreate = NULL;
      std::cerr << dlsym_error << std::endl;
      FreeLibHandler();
      return ProblemManager::ERROR_;
    }
    mDestroy = (destroy_t*) dlsym(mLibHandle, "destroy");
    dlsym_error = dlerror();
    if (dlsym_error)
    {
      mCreate = NULL;
      mDestroy = NULL;
      std::cerr << dlsym_error << std::endl;
      FreeLibHandler();
      return ProblemManager::ERROR_;
    }
  #endif

  mProblem = mCreate();
  if (!mProblem)
  {
    FreeLibHandler();
    mCreate = NULL;
    mDestroy = NULL;
    std::cerr << "Cannot create problem instance" << std::endl;
  }

  return ProblemManager::OK_;
}

// ------------------------------------------------------------------------------------------------
void ProblemManager::FreeLibHandler()
{
  #ifdef WIN32
    FreeLibrary(mLibHandle);
  #else
    dlclose(mLibHandle);
  #endif
  mLibHandle = NULL;
}

// ------------------------------------------------------------------------------------------------
int ProblemManager::FreeProblemLibrary()
{
  if (mProblem)
    mDestroy(mProblem);
  if (mLibHandle)
    FreeLibHandler();
  mLibHandle = NULL;
  mProblem = NULL;
  mCreate = NULL;
  mDestroy = NULL;
  return ProblemManager::OK_;
}

// ------------------------------------------------------------------------------------------------
IProblem* ProblemManager::GetProblem() const
{
  if (mProblem)
    return mProblem;
  else
    return NULL;
}
// - end of file ----------------------------------------------------------------------------------
