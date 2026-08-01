#include <gtest/gtest.h>

#include "Calculation.h"
#include "OMPCalculation.h"
#include "CalculationFactory.h"
#include "Task.h"
#include "Parameters.h"
#include "Trial.h"
#include "InformationForCalculation.h"
#include "CalculationProblem.h"

/**
 * \brief Тестовый класс для классов Calculation, OMPCalculation и CalculationFactory.
 *
 * \details Отвечает за создание и настройку общего тестового окружения перед каждым тестом
 * и его очистку после. Это гарантирует изоляцию тестов друг от друга.
 */
class CalculationTest : public ::testing::Test {
protected:
  /// Указатель на CalculationProblem объект задачи.
  CalculationProblem* calculationProblem;
  /// Указатель на задачу для корневого процесса (не-листовая).
  Task* nonLeafTask;
  /// Указатель на задачу для дочернего процесса (листовая).
  Task* leafTask;
  /// Хранилище для исходного состояния глобальных параметров.
  Parameters originalParameters;

  /**
   * \brief Метод настройки, вызываемый перед каждым тестом.
   * \details Инициализирует глобальные параметры, создает CalculationProblem объекты и задачи,
   * а также сбрасывает статические поля класса Calculation для обеспечения
   * чистого состояния для каждого теста.
   */
  void SetUp() override {
    int argc = 1;
    char* argv[] = { const_cast<char*>("tests"), nullptr };
    parameters.Init(argc, argv, false);

    originalParameters = parameters;
    parameters.Dimension = 2;

    calculationProblem = new CalculationProblem();
    nonLeafTask = new Task(calculationProblem, 0);
    leafTask = new Task(calculationProblem, 1);

    if (Calculation::leafCalculation) {
      delete Calculation::leafCalculation;
      Calculation::leafCalculation = nullptr;
    }
    Calculation::firstCalculation = nullptr;

    SetCountCalculation(0);
    SetStartComputingAway(true);
    GetInputCalculation().Clear();
    GetResultCalculation().Clear();
  }

  /**
   * \brief Метод очистки, вызываемый после каждого теста.
   * \details Освобождает всю выделенную в SetUp память и сбрасывает статические
   * указатели, чтобы предотвратить утечки памяти и влияние на последующие тесты.
   */
  void TearDown() override {
    delete nonLeafTask;
    delete leafTask;
    delete calculationProblem;

    GetInputCalculation().Clear();
    GetResultCalculation().Clear();

    if (Calculation::leafCalculation) {
      delete Calculation::leafCalculation;
      Calculation::leafCalculation = nullptr;
    }
    Calculation::firstCalculation = nullptr;
    parameters = originalParameters;
  }

  // --- Геттеры/сеттеры для доступа к protected-членам Calculation ---
  void SetStartComputingAway(bool value) {
    Calculation::isStartComputingAway = value;
  }

  void SetCountCalculation(int count) {
    Calculation::countCalculation = count;
  }

  InformationForCalculation& GetInputCalculation() {
    return Calculation::inputCalculation;
  }

  TResultForCalculation& GetResultCalculation() {
    return Calculation::resultCalculation;
  }

  int GetCountCalculation() {
    return Calculation::countCalculation;
  }

  bool IsStartComputingAway() {
    return Calculation::isStartComputingAway;
  }
};

// ================================================================
// --- Тесты для CalculationFactory ---
// ================================================================

/**
 * \brief Проверяет, что CreateNewCalculation корректно создает OMPCalculation для листовой задачи.
 */
TEST_F(CalculationTest, Factory_CreateNewCalculation_CreatesOMP_ForLeafTask) {
  parameters.TypeCalculation = OMP;
  Calculation* calc = CalculationFactory::CreateNewCalculation(*leafTask);

  ASSERT_NE(calc, nullptr);
  EXPECT_NE(dynamic_cast<OMPCalculation*>(calc), nullptr);

  delete calc;
}

/**
 * \brief Проверяет, что CreateNewCalculation возвращает nullptr для не-листовой задачи.
 */
TEST_F(CalculationTest, Factory_CreateNewCalculation_ReturnsNull_ForNonLeafTask) {
  parameters.TypeCalculation = OMP;
  Calculation* calc = CalculationFactory::CreateNewCalculation(*nonLeafTask);
  EXPECT_EQ(calc, nullptr);
}

/**
 * \brief Проверяет, что CreateNewCalculation всегда создаёт новый экземпляр.
 * \details В отличие от singleton-методов, два последовательных вызова должны
 * возвращать разные объекты.
 */
TEST_F(CalculationTest, Factory_CreateNewCalculation_AlwaysCreatesNewInstance) {
  parameters.TypeCalculation = OMP;

  Calculation* calc1 = CalculationFactory::CreateNewCalculation(*leafTask);
  Calculation* calc2 = CalculationFactory::CreateNewCalculation(*leafTask);

  ASSERT_NE(calc1, nullptr);
  ASSERT_NE(calc2, nullptr);
  EXPECT_NE(calc1, calc2);

  delete calc1;
  delete calc2;
}

/**
 * \brief Проверяет логику синглтона в методе CreateCalculation2.
 * \details Убеждается, что при повторном вызове для листовой задачи возвращается
 * тот же самый экземпляр вычислителя.
 */
TEST_F(CalculationTest, Factory_CreateCalculation2_SingletonLogic) {
  parameters.TypeCalculation = OMP;

  Calculation* calc1 = CalculationFactory::CreateCalculation2(*leafTask);
  ASSERT_NE(calc1, nullptr);
  EXPECT_NE(dynamic_cast<OMPCalculation*>(calc1), nullptr);

  Calculation* calc2 = CalculationFactory::CreateCalculation2(*leafTask);
  EXPECT_EQ(calc1, calc2);
}

/**
 * \brief Проверяет, что CreateCalculation возвращает nullptr для листовой задачи.
 */
TEST_F(CalculationTest, Factory_CreateCalculation_ReturnsNull_ForLeafTask) {
  parameters.TypeCalculation = OMP;
  Calculation* calc = CalculationFactory::CreateCalculation(*leafTask);
  EXPECT_EQ(calc, nullptr);
}

/**
 * \brief Проверяет singleton-логику CreateCalculation для не-листовой (корневой) задачи.
 * \details Согласно реализации фабрики, для OMP объект кэшируется в
 * Calculation::leafCalculation (не в firstCalculation). Повторный вызов
 * возвращает тот же самый экземпляр.
 */
TEST_F(CalculationTest, Factory_CreateCalculation_SingletonLogic_ForNonLeafTask) {
  parameters.TypeCalculation = OMP;

  Calculation* calc1 = CalculationFactory::CreateCalculation(*nonLeafTask);
  ASSERT_NE(calc1, nullptr);
  EXPECT_NE(dynamic_cast<OMPCalculation*>(calc1), nullptr);

  Calculation* calc2 = CalculationFactory::CreateCalculation(*nonLeafTask);
  EXPECT_EQ(calc1, calc2);
  EXPECT_EQ(calc1, Calculation::leafCalculation);
}

/**
 * \brief Проверяет, что повторный вызов CreateCalculation2 сохраняет ссылку в leafCalculation.
 */
TEST_F(CalculationTest, Factory_CreateCalculation2_StoresLeafCalculation) {
  parameters.TypeCalculation = OMP;

  Calculation* calc = CalculationFactory::CreateCalculation2(*leafTask);
  ASSERT_NE(calc, nullptr);
  EXPECT_EQ(calc, Calculation::leafCalculation);
}

// ================================================================
// --- Тесты для OMPCalculation ---
// ================================================================

/**
 * \brief Тестирует стандартный режим немедленного выполнения вычислений.
 * \details Проверяет, что Calculate сразу вычисляет значения для переданного испытания
 * и корректно заполняет поля index, FuncValues и счетчики.
 */
TEST_F(CalculationTest, OMPCalculation_Calculate_ImmediateExecution) {
  OMPCalculation calc(*leafTask);
  InformationForCalculation inputSet;
  TResultForCalculation outputSet;

  Trial trial;
  trial.y[0] = 1.0;
  trial.y[1] = 2.0;
  inputSet.trials.push_back(&trial);

  SetStartComputingAway(true);
  calc.Calculate(inputSet, outputSet);

  ASSERT_EQ(outputSet.trials.size(), 1);
  Trial* resultTrial = outputSet.trials[0];

  EXPECT_EQ(resultTrial->index, 1);
  EXPECT_DOUBLE_EQ(resultTrial->FuncValues[0], -7.0);
  EXPECT_DOUBLE_EQ(resultTrial->FuncValues[1], 13.0);
  ASSERT_EQ(outputSet.countCalcTrials.size(), 2);
  EXPECT_EQ(outputSet.countCalcTrials[0], 1);
  EXPECT_EQ(outputSet.countCalcTrials[1], 1);
}

/**
 * \brief Тестирует немедленное вычисление сразу нескольких точек (batch).
 * \details Проверяет, что Calculate корректно обрабатывает набор из нескольких
 * испытаний за один вызов в немедленном режиме.
 */
TEST_F(CalculationTest, OMPCalculation_Calculate_ImmediateExecution_MultiplePoints) {
  OMPCalculation calc(*leafTask);
  InformationForCalculation inputSet;
  TResultForCalculation outputSet;

  Trial trial1;
  trial1.y[0] = 1.0;
  trial1.y[1] = 2.0; // сумма = 3

  Trial trial2;
  trial2.y[0] = -1.0;
  trial2.y[1] = 4.0; // сумма = 3

  Trial trial3;
  trial3.y[0] = 0.5;
  trial3.y[1] = 0.5; // сумма = 1

  inputSet.trials.push_back(&trial1);
  inputSet.trials.push_back(&trial2);
  inputSet.trials.push_back(&trial3);

  SetStartComputingAway(true);
  calc.Calculate(inputSet, outputSet);

  ASSERT_EQ(outputSet.trials.size(), 3);

  EXPECT_DOUBLE_EQ(outputSet.trials[0]->FuncValues[0], 3.0 - 10.0);
  EXPECT_DOUBLE_EQ(outputSet.trials[0]->FuncValues[1], 3.0 + 10.0);
  EXPECT_EQ(outputSet.trials[0]->index, 1);

  EXPECT_DOUBLE_EQ(outputSet.trials[1]->FuncValues[0], 3.0 - 10.0);
  EXPECT_DOUBLE_EQ(outputSet.trials[1]->FuncValues[1], 3.0 + 10.0);
  EXPECT_EQ(outputSet.trials[1]->index, 1);

  EXPECT_DOUBLE_EQ(outputSet.trials[2]->FuncValues[0], 1.0 - 10.0);
  EXPECT_DOUBLE_EQ(outputSet.trials[2]->FuncValues[1], 1.0 + 10.0);
  EXPECT_EQ(outputSet.trials[2]->index, 1);
}

/**
 * \brief Проверяет, что вектор procLevel НЕ заполняется вычислителем OMP.
 * \details Согласно текущей реализации OMPCalculation::Calculate/StartCalculate,
 * поле outputSet.procLevel не изменяется. Тест фиксирует это фактическое поведение.
 * (Если в будущем procLevel начнёт заполняться — тест нужно обновить.)
 */
TEST_F(CalculationTest, OMPCalculation_Calculate_DoesNotFillProcLevel) {
  OMPCalculation calc(*leafTask);
  InformationForCalculation inputSet;
  TResultForCalculation outputSet;

  Trial trial;
  trial.y[0] = 1.0;
  trial.y[1] = 1.0;
  inputSet.trials.push_back(&trial);

  SetStartComputingAway(true);
  calc.Calculate(inputSet, outputSet);

  // procLevel не трогается реализацией — остаётся пустым.
  EXPECT_TRUE(outputSet.procLevel.empty());
  // При этом основные результаты заполнены.
  ASSERT_EQ(outputSet.trials.size(), 1);
  EXPECT_EQ(outputSet.trials[0]->index, 1);
}

/**
 * \brief Проверяет поведение CreateCalculation для не-листовой (корневой) задачи.
 * \details Не делаем предположений о singleton-кэшировании в firstCalculation.
 * Проверяем лишь, что повторные вызовы согласованы между собой: либо оба
 * возвращают nullptr, либо оба возвращают один и тот же закэшированный объект.
 */
TEST_F(CalculationTest, Factory_CreateCalculation_NonLeafTask_ConsistentBehavior) {
  parameters.TypeCalculation = OMP;

  Calculation* calc1 = CalculationFactory::CreateCalculation(*nonLeafTask);
  Calculation* calc2 = CalculationFactory::CreateCalculation(*nonLeafTask);

  // Поведение должно быть детерминированным и согласованным между вызовами.
  EXPECT_EQ(calc1, calc2);
}

/**
 * \brief Проверяет, что SetCountCalculation сохраняет переданное значение счётчика.
 * \details Проверяем только документированный контракт (сохранение значения),
 * не делая предположений о побочном изменении флага isStartComputingAway при 0.
 */
TEST_F(CalculationTest, BaseCalculation_SetCountCalculation_StoresValue) {
  OMPCalculation calc(*leafTask);

  calc.SetCountCalculation(0);
  EXPECT_EQ(GetCountCalculation(), 0);

  calc.SetCountCalculation(7);
  EXPECT_EQ(GetCountCalculation(), 7);
}

/**
 * \brief Тестирует режим отложенных вычислений.
 * \details Проверяет, что Calculate накапливает испытания при нескольких вызовах
 * и выполняет их все вместе, когда счетчик достигает нуля.
 */
TEST_F(CalculationTest, OMPCalculation_Calculate_AccumulatedExecution) {
  OMPCalculation calc(*leafTask);
  Calculation::firstCalculation = &calc;

  InformationForCalculation inputSet1, inputSet2;
  TResultForCalculation outputSet1, outputSet2;

  Trial* trial1 = new Trial();
  trial1->y[0] = 1.0;
  trial1->y[1] = 1.0;
  inputSet1.trials.push_back(trial1);

  Trial* trial2 = new Trial();
  trial2->y[0] = 2.0;
  trial2->y[1] = 2.0;
  inputSet2.trials.push_back(trial2);

  calc.SetCountCalculation(2);

  calc.Calculate(inputSet1, outputSet1);

  ASSERT_EQ(GetInputCalculation().trials.size(), 1);
  EXPECT_EQ(trial1->index, -2);

  calc.Calculate(inputSet2, outputSet2);

  ASSERT_EQ(GetResultCalculation().trials.size(), 2);
  Trial* resultTrial1 = GetResultCalculation().trials[0];
  Trial* resultTrial2 = GetResultCalculation().trials[1];

  EXPECT_EQ(resultTrial1->index, 1);
  EXPECT_DOUBLE_EQ(resultTrial1->FuncValues[0], 2.0 - 10.0);
  EXPECT_DOUBLE_EQ(resultTrial1->FuncValues[1], 2.0 + 10.0);

  EXPECT_EQ(resultTrial2->index, 1);
  EXPECT_DOUBLE_EQ(resultTrial2->FuncValues[0], 4.0 - 10.0);
  EXPECT_DOUBLE_EQ(resultTrial2->FuncValues[1], 4.0 + 10.0);

  delete trial1;
  delete trial2;
}

/**
 * \brief Проверяет, что при частичном накоплении вычисление не запускается.
 * \details При SetCountCalculation(3) и одном накопленном вызове результат
 * ещё не должен быть сформирован.
 */
TEST_F(CalculationTest, OMPCalculation_Calculate_AccumulationNotTriggeredEarly) {
  OMPCalculation calc(*leafTask);
  Calculation::firstCalculation = &calc;

  InformationForCalculation inputSet;
  TResultForCalculation outputSet;

  Trial* trial = new Trial();
  trial->y[0] = 1.0;
  trial->y[1] = 1.0;
  inputSet.trials.push_back(trial);

  calc.SetCountCalculation(3);

  calc.Calculate(inputSet, outputSet);

  // После первого из трёх ожидаемых вызовов вычисление не запускается,
  // точка помечается как ещё не вычисленная.
  EXPECT_EQ(trial->index, -2);
  ASSERT_EQ(GetInputCalculation().trials.size(), 1);
  EXPECT_TRUE(GetResultCalculation().trials.empty());

  delete trial;
}

/**
 * \brief Проверяет метод Reset() у OMPCalculation (через базовый Calculation::Reset).
 * \details Reset не должен приводить к падению и должен корректно вызываться
 * даже без предварительных вычислений.
 */
TEST_F(CalculationTest, OMPCalculation_Reset_DoesNotCrash) {
  OMPCalculation calc(*leafTask);
  EXPECT_NO_THROW(calc.Reset());
}

// ================================================================
// --- Тесты для базового класса Calculation ---
// ================================================================

/**
 * \brief Проверяет корректность работы сеттеров базового класса Calculation.
 */
TEST_F(CalculationTest, BaseCalculation_Setters) {
  OMPCalculation calc(*leafTask);

  Task* newTask = new Task(calculationProblem, 2);
  calc.SetTask(newTask);

  SearchData searchData(2);
  calc.SetSearchData(&searchData);

  calc.SetCountCalculation(5);
  EXPECT_EQ(GetCountCalculation(), 5);
  EXPECT_FALSE(IsStartComputingAway());

  delete newTask;
}

/**
 * \brief Проверяет фактическое поведение SetCountCalculation.
 * \details Согласно реализации, метод сохраняет значение счётчика и ВСЕГДА
 * устанавливает isStartComputingAway = false (в том числе при c == 0).
 */
TEST_F(CalculationTest, BaseCalculation_SetCountCalculation_AlwaysDisablesImmediate) {
  OMPCalculation calc(*leafTask);

  calc.SetCountCalculation(0);
  EXPECT_EQ(GetCountCalculation(), 0);
  EXPECT_FALSE(IsStartComputingAway());   // даже при 0 флаг сбрасывается в false

  calc.SetCountCalculation(7);
  EXPECT_EQ(GetCountCalculation(), 7);
  EXPECT_FALSE(IsStartComputingAway());
}

/**
 * \brief Проверяет, что ContinueComputing базового класса не приводит к падению.
 */
TEST_F(CalculationTest, BaseCalculation_ContinueComputing_DoesNotCrash) {
  OMPCalculation calc(*leafTask);
  EXPECT_NO_THROW(calc.ContinueComputing());
}

/**
 * \brief Проверяет, что после SetUp статические указатели сброшены.
 */
TEST_F(CalculationTest, BaseCalculation_StaticPointers_ResetAfterSetUp) {
  EXPECT_EQ(Calculation::firstCalculation, nullptr);
  EXPECT_EQ(Calculation::leafCalculation, nullptr);
  EXPECT_EQ(GetCountCalculation(), 0);
  EXPECT_TRUE(IsStartComputingAway());
}

// ================================================================
// --- Тесты для структур InformationForCalculation / TResultForCalculation ---
// ================================================================

/**
 * \brief Проверяет базовые операции InformationForCalculation:
 * Resize, GetSize, ToZero и оператор [].
 */
TEST_F(CalculationTest, InformationForCalculation_ResizeAndAccess) {
  InformationForCalculation info;

  info.Resize(3);
  EXPECT_EQ(info.GetSize(), 3);

  // После Resize все указатели должны быть обнулены.
  EXPECT_EQ(info[0], nullptr);
  EXPECT_EQ(info[1], nullptr);
  EXPECT_EQ(info[2], nullptr);

  Trial trial;
  info[1] = &trial;
  EXPECT_EQ(info[1], &trial);
}

/**
 * \brief Проверяет, что оператор [] выбрасывает исключение при выходе за границы.
 */
TEST_F(CalculationTest, InformationForCalculation_OutOfRangeThrows) {
  InformationForCalculation info;
  info.Resize(2);

  EXPECT_ANY_THROW(info[-1]);
  EXPECT_ANY_THROW(info[5]);
}

/**
 * \brief Проверяет, что Clear очищает массив trials.
 */
TEST_F(CalculationTest, InformationForCalculation_Clear) {
  InformationForCalculation info;
  Trial trial;
  info.trials.push_back(&trial);
  ASSERT_EQ(info.GetSize(), 1);

  info.Clear();
  EXPECT_EQ(info.GetSize(), 0);
}

/**
 * \brief Проверяет операции Resize и Clear у TResultForCalculation.
 */
TEST_F(CalculationTest, TResultForCalculation_ResizeAndClear) {
  TResultForCalculation result;

  result.Resize(4);
  EXPECT_EQ(result.trials.size(), 4);
  EXPECT_EQ(result.countCalcTrials.size(), 4);
  EXPECT_EQ(result.procLevel.size(), 4);
  EXPECT_TRUE(result.NeighboursAdditionalPoints.empty());
  EXPECT_TRUE(result.NeighboursAdditionalProcLevel.empty());

  result.Clear();
  EXPECT_TRUE(result.trials.empty());
  EXPECT_TRUE(result.countCalcTrials.empty());
  EXPECT_TRUE(result.procLevel.empty());
  EXPECT_TRUE(result.NeighboursAdditionalPoints.empty());
  EXPECT_TRUE(result.NeighboursAdditionalProcLevel.empty());
}