/////////////////////////////////////////////////////////////////////////////
//                                                                         //
//             LOBACHEVSKY STATE UNIVERSITY OF NIZHNY NOVGOROD             //
//                                                                         //
//                       Copyright (c) 2015 by UNN.                        //
//                          All Rights Reserved.                           //
//                                                                         //
//  File:      method_factory.cpp                                          //
//                                                                         //
//  Purpose:   Source file for method factory class                        //
//                                                                         //
//  Author(s): Lebedev I.                                                  //
//                                                                         //
/////////////////////////////////////////////////////////////////////////////

#include "CalculationFactory.h"

#include "omp_calculation.h"
#include "mpi_calculation_async.h"
#include "mpi_calculation.h"
#include "cuda_calculation.h"
#include "phi_calculation.h"
#include "BlockSchemeCalculation.h"
#include "AdaptiveCalculation.h"
#include "MultievolventsCalculation.h"
#include "ParallelBlockSchemeCalculation.h"
#include "ApproximationSchemeCalculation.h"
#include "OneApi_calculation.h"

// ------------------------------------------------------------------------------------------------
TCalculation* TCalculationFactory::CreateCalculation2(TTask& _pTask, TEvolvent* evolvent)
{
  TCalculation* calculation = 0;

  //if (parameters.calculationsArray.GetIsChange())
  //{
  //  if (parameters.calculationsArray[parameters.GetProcRank()] != -1)
  //  {
  //    printf("\nprocRank = %d\n", parameters.GetProcRank());
  //    char buf[256] = { 0 };
  //    sprintf(buf, "%d", parameters.calculationsArray[parameters.GetProcRank()]);
  //    parameters.SetVal("TypeCalculation", buf);
  //    parameters.PrintParameter("TypeCalculation");

  //    if (parameters.TypeCalculation == OMP)
  //      calculation = new TOMPCalculation(_pTask);
  //    //else if (parameters.TypeCalculation == CUDA)
  //    //  calculation = new TCUDACalculation(_parameters, _pTask);
  //    //else if (parameters.TypeCalculation == PHI)
  //    //  calculation = new TPHICalculation(_parameters, _pTask);
  //    if (parameters.TypeCalculation == BlockScheme)
  //      calculation = new TBlockSchemeCalculation(_pTask);
  //  }
  //}
  if (_pTask.IsLeaf()) //Если в листе
  {
    //else if (parameters.TypeCalculation == PHI)
    //  calculation = new TPHICalculation(_parameters, _pTask);

        // Выбор между OMP иCUDA
    if ((parameters.TypeCalculation == OMP) || (parameters.TypeCalculation == MPICalculation && parameters.GetProcNum() == 1))
    {
      if (TCalculation::leafCalculation == 0)
      {
        calculation = new TOMPCalculation(_pTask);
        TCalculation::leafCalculation = calculation;
      }
      else
      {
        calculation = TCalculation::leafCalculation;
      }
    }
    else if (parameters.TypeCalculation == CUDA)
    {
      if (TCalculation::leafCalculation == 0)
      {
        calculation = new TCUDACalculation(_pTask);
        TCalculation::leafCalculation = calculation;
      }
      else
      {
        calculation = TCalculation::leafCalculation;
      }
    }
    else if (parameters.TypeCalculation == OneApi)
    {
      if (TCalculation::leafCalculation == 0)
      {
        calculation = new TOneApiCalculation(_pTask);
        TCalculation::leafCalculation = calculation;
      }
      else
      {
        calculation = TCalculation::leafCalculation;
      }
    }
    else if (parameters.TypeCalculation == MPICalculation)
    {
      if (TCalculation::leafCalculation == 0)
      {
        calculation = new TMPICalculation(_pTask);
        TCalculation::leafCalculation = calculation;
      }
      else
      {
        calculation = TCalculation::leafCalculation;
      }
    }
    else if (parameters.TypeCalculation == AsyncMPI)
    {
      if (TCalculation::leafCalculation == 0)
      {
        calculation = new TMPICalculationAsync(_pTask);
        TCalculation::leafCalculation = calculation;
      }
      else
      {
        calculation = TCalculation::leafCalculation;
      }
    }
    else
    {
      if (TCalculation::leafCalculation == 0)
      {
        if (parameters.calculationsArray.GetSize() < _pTask.GetProcLevel())
          calculation = new TCUDACalculation(_pTask);
        else
        {
          if (parameters.calculationsArray[_pTask.GetProcLevel()] == OMP)
            calculation = new TOMPCalculation(_pTask);
          else
            calculation = new TCUDACalculation(_pTask);

          TCalculation::leafCalculation = calculation;
        }
      }
      else
      {
        calculation = TCalculation::leafCalculation;
      }
    }
    //calculation = new TOMPCalculation(_pTask);

    if (_pTask.GetProcLevel() == 0) //и одновременно в корне (если только корень)
    {
      if (parameters.TypeMethod == MultievolventsMethod) // и выбраны множественные развертки
      {// то создаем вычислитель множественных разверток
        TCalculation* c = calculation;
        calculation = new TMultievolventsCalculation(_pTask, c, evolvent); // и передаем ему созданный ранее вычислитель
      }
    }
  }
  else //все остальные уровни
  {
    if (parameters.TypeCalculation == BlockScheme) // блочная многошаговая схема
      calculation = new TBlockSchemeCalculation(_pTask);
    else if (parameters.TypeCalculation == ParallelBlockScheme) // блочная многошаговая схема
      calculation = new TBlockSchemeCalculation(_pTask);
    else//Иначе
    {
      if (parameters.TypeMethod == AdaptivMethod) //Если метод адаптивный
      {
        calculation = new TAdaptiveCalculation(_pTask); // то создаем адаптивный вычислитель
      }
      else if (parameters.TypeMethod == MultievolventsMethod) // если метод с множественными развертками
      {
        if (_pTask.GetProcLevel() == 0) // то в корне создаем множествынные развертки
        {
          TCalculation* c = new TAdaptiveCalculation(_pTask); // делаем внутреним вычислителем адаптивный вычислитель
          calculation = new TMultievolventsCalculation(_pTask, c, evolvent); //создаем вычислитель множественных разверток
        }
        else // на всех остальных уровнях (кроме листа) адаптивная схема
        {
          calculation = new TAdaptiveCalculation(_pTask);
        }
      }
      else
        calculation = new TAdaptiveCalculation(_pTask);
    }
  }

  return calculation;
}


// ------------------------------------------------------------------------------------------------
TCalculation* TCalculationFactory::CreateCalculation(TTask& _pTask, TEvolvent* evolvent)
{
  TCalculation* calculation = 0;

  //if (parameters.calculationsArray.GetIsChange())
  //{
  //  if (parameters.calculationsArray[parameters.GetProcRank()] != -1)
  //  {
  //    printf("\nprocRank = %d\n", parameters.GetProcRank());
  //    char buf[256] = { 0 };
  //    sprintf(buf, "%d", parameters.calculationsArray[parameters.GetProcRank()]);
  //    parameters.SetVal("TypeCalculation", buf);
  //    parameters.PrintParameter("TypeCalculation");

  //    if (parameters.TypeCalculation == OMP)
  //      calculation = new TOMPCalculation(_pTask);
  //    //else if (parameters.TypeCalculation == CUDA)
  //    //  calculation = new TCUDACalculation(_parameters, _pTask);
  //    //else if (parameters.TypeCalculation == PHI)
  //    //  calculation = new TPHICalculation(_parameters, _pTask);
  //    if (parameters.TypeCalculation == BlockScheme)
  //      calculation = new TBlockSchemeCalculation(_pTask);
  //  }
  //}
  if (_pTask.IsLeaf()) //Если в листе
  {    
//else if (parameters.TypeCalculation == PHI)
//  calculation = new TPHICalculation(_parameters, _pTask);
    
    // Выбор между OMP иCUDA
    if ((parameters.TypeCalculation == OMP))
    {
      if (TCalculation::leafCalculation == 0)
      {
        calculation = new TOMPCalculation(_pTask);
        TCalculation::leafCalculation = calculation;
      }
      else
      {
        calculation = TCalculation::leafCalculation;
      }
    }
    if ((parameters.TypeCalculation == ApproximationScheme))
    {
      if (TCalculation::leafCalculation == 0)
      {
        calculation = new TApproximationSchemeCalculation(_pTask);
        TCalculation::leafCalculation = 0;
      }
      else
      {
        calculation = TCalculation::leafCalculation;
      }
    }
    else if (parameters.TypeCalculation == CUDA)
    {
      if (TCalculation::leafCalculation == 0)
      {
        calculation = new TCUDACalculation(_pTask);
        TCalculation::leafCalculation = calculation;
      }
      else
      {
        calculation = TCalculation::leafCalculation;
      }
    }
    else if (parameters.TypeCalculation == OneApi)
    {
      if (TCalculation::leafCalculation == 0)
      {
        calculation = new TOneApiCalculation(_pTask);
        TCalculation::leafCalculation = calculation;
      }
      else
      {
        calculation = TCalculation::leafCalculation;
      }
    }
    else if (parameters.TypeCalculation == MPICalculation)
    {
      if (TCalculation::leafCalculation == 0)
      {
        calculation = new TMPICalculation(_pTask);
        TCalculation::leafCalculation = calculation;
      }
      else
      {
        calculation = TCalculation::leafCalculation;
      }
    }
    else if (parameters.TypeCalculation == AsyncMPI)
    {
      if (TCalculation::leafCalculation == 0)
      {
        calculation = new TMPICalculationAsync(_pTask);
        TCalculation::leafCalculation = calculation;
      }
      else
      {
        calculation = TCalculation::leafCalculation;
      }
    }
    else
    {
      if (TCalculation::leafCalculation == 0)
      {
        if (parameters.calculationsArray.GetSize() < _pTask.GetProcLevel())
          calculation = new TCUDACalculation(_pTask);
        else
        {          
          if (parameters.calculationsArray[_pTask.GetProcLevel()] == OMP)
            calculation = new TOMPCalculation(_pTask);
          else
            calculation = new TCUDACalculation(_pTask);

          TCalculation::leafCalculation = calculation;
        }
      }
      else
      {
        calculation = TCalculation::leafCalculation;
      }
    }
      //calculation = new TOMPCalculation(_pTask);

    if (_pTask.GetProcLevel() == 0) //и одновременно в корне (если только корень)
    {
      if (parameters.TypeMethod == MultievolventsMethod) // и выбраны множественные развертки
      {// то создаем вычислитель множественных разверток
        TCalculation* c = calculation;
        calculation = new TMultievolventsCalculation(_pTask, c, evolvent); // и передаем ему созданный ранее вычислитель
      }
    }
  }
  else //все остальные уровни
  {
    if (parameters.TypeCalculation == BlockScheme) // блочная многошаговая схема
      calculation = new TBlockSchemeCalculation(_pTask);
    else if (parameters.TypeCalculation == ParallelBlockScheme) // блочная многошаговая схема 
      calculation = new TParallelBlockSchemeCalculation(_pTask);
    else if (parameters.TypeCalculation == ApproximationScheme) // схема с апроксимацией схема
      calculation = new TApproximationSchemeCalculation(_pTask);
    else//Иначе
    {
      if (parameters.TypeMethod == AdaptivMethod) //Если метод адаптивный
      {
        calculation = new TAdaptiveCalculation(_pTask); // то создаем адаптивный вычислитель
      }
      else if (parameters.TypeMethod == MultievolventsMethod) // если метод с множественными развертками
      {
        if (_pTask.GetProcLevel() == 0) // то в корне создаем множествынные развертки
        {
          TCalculation* c = new TAdaptiveCalculation(_pTask); // делаем внутреним вычислителем адаптивный вычислитель
          calculation = new TMultievolventsCalculation(_pTask, c, evolvent); //создаем вычислитель множественных разверток
        }
        else // на всех остальных уровнях (кроме листа) адаптивная схема
        {
          calculation = new TAdaptiveCalculation(_pTask);
        }
      }
      else
        calculation = new TAdaptiveCalculation(_pTask);
    }
  }

  return calculation;
}

TCalculation* TCalculationFactory::CreateNewCalculation(TTask& _pTask, TEvolvent* evolvent)
{
  TCalculation* calculation = 0;

  if (_pTask.IsLeaf()) //Åñëè â ëèñòå
  {    

    // Âûáîð ìåæäó OMP èCUDA
    if ((parameters.TypeCalculation == OMP))
    {
      calculation = new TOMPCalculation(_pTask);
    }
    if ((parameters.TypeCalculation == ApproximationScheme))
    {
      calculation = new TApproximationSchemeCalculation(_pTask);       
    }
    else if (parameters.TypeCalculation == CUDA)
    {      
      calculation = new TCUDACalculation(_pTask);        
    }
    else if (parameters.TypeCalculation == OneApi)
    {      
      calculation = new TOneApiCalculation(_pTask);
        
    }
    else if (parameters.TypeCalculation == MPICalculation)
    {
      calculation = new TMPICalculation(_pTask);      
    }
    else if (parameters.TypeCalculation == AsyncMPI)
    {      
      calculation = new TMPICalculationAsync(_pTask);        
    }
    else
    {
        if (parameters.calculationsArray.GetSize() < _pTask.GetProcLevel())
          calculation = new TCUDACalculation(_pTask);
        else
        {          
          if (parameters.calculationsArray[_pTask.GetProcLevel()] == OMP)
            calculation = new TOMPCalculation(_pTask);
          else
            calculation = new TCUDACalculation(_pTask);

          TCalculation::leafCalculation = calculation;
        }
     
    }

    if (_pTask.GetProcLevel() == 0) //è îäíîâðåìåííî â êîðíå (åñëè òîëüêî êîðåíü)
    {
      if (parameters.TypeMethod == MultievolventsMethod) // è âûáðàíû ìíîæåñòâåííûå ðàçâåðòêè
      {// òî ñîçäàåì âû÷èñëèòåëü ìíîæåñòâåííûõ ðàçâåðòîê
        TCalculation* c = calculation;
        calculation = new TMultievolventsCalculation(_pTask, c, evolvent); // è ïåðåäàåì åìó ñîçäàííûé ðàíåå âû÷èñëèòåëü
      }
    }
  }
  else //âñå îñòàëüíûå óðîâíè
  {
    if (parameters.TypeCalculation == BlockScheme) // áëî÷íàÿ ìíîãîøàãîâàÿ ñõåìà
      calculation = new TBlockSchemeCalculation(_pTask);
    else if (parameters.TypeCalculation == ParallelBlockScheme) // áëî÷íàÿ ìíîãîøàãîâàÿ ñõåìà 
      calculation = new TParallelBlockSchemeCalculation(_pTask);
    else if (parameters.TypeCalculation == ApproximationScheme) // ñõåìà ñ àïðîêñèìàöèåé ñõåìà
      calculation = new TApproximationSchemeCalculation(_pTask);
    else//Èíà÷å
    {
      if (parameters.TypeMethod == AdaptivMethod) //Åñëè ìåòîä àäàïòèâíûé
      {
        calculation = new TAdaptiveCalculation(_pTask); // òî ñîçäàåì àäàïòèâíûé âû÷èñëèòåëü
      }
      else if (parameters.TypeMethod == MultievolventsMethod) // åñëè ìåòîä ñ ìíîæåñòâåííûìè ðàçâåðòêàìè
      {
        if (_pTask.GetProcLevel() == 0) // òî â êîðíå ñîçäàåì ìíîæåñòâûííûå ðàçâåðòêè
        {
          TCalculation* c = new TAdaptiveCalculation(_pTask); // äåëàåì âíóòðåíèì âû÷èñëèòåëåì àäàïòèâíûé âû÷èñëèòåëü
          calculation = new TMultievolventsCalculation(_pTask, c, evolvent); //ñîçäàåì âû÷èñëèòåëü ìíîæåñòâåííûõ ðàçâåðòîê
        }
        else // íà âñåõ îñòàëüíûõ óðîâíÿõ (êðîìå ëèñòà) àäàïòèâíàÿ ñõåìà
        {
          calculation = new TAdaptiveCalculation(_pTask);
        }
      }
      else
        calculation = new TAdaptiveCalculation(_pTask);
    }
  }

  return calculation;
}
