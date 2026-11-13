/////////////////////////////////////////////////////////////////////////////
//                                                                         //
//             LOBACHEVSKY STATE UNIVERSITY OF NIZHNY NOVGOROD             //
//                                                                         //
//  File:      test_hybrid_calculation.cpp                                 //
//  Purpose:   Модульные и интеграционные тесты гибридного вычислителя     //
//                                                                         //
/////////////////////////////////////////////////////////////////////////////

#include <gtest/gtest.h>
#include <cmath>
#include <vector>
#include <functional>
#include <algorithm>

#include "Parameters.h"
#include "ProblemFromFunctionPointers.h"
#include "Trial.h"
#include "TrialFactory.h"

#include "HybridLocalTask.h"
#include "HybridLocalCalculation.h"
#include "CalculationFactory.h"

#include "test_env.h"

using namespace test_env;

class HybridLocalTaskTest : public ::testing::Test
{
protected:
    IProblem* problem = nullptr;
    Task* task = nullptr;

    void SetUp() override
    {
        ResetParametersToMethodDefaults(2);
        problem = MakeParaboloid(2);
        problem->Initialize();
        task = new Task(problem, 0);
    }

    void TearDown() override
    {
        delete task;
        delete problem;
        ResetParametersToMethodDefaults(2);
    }
};

/**
 * \brief Корректное вычисление при неизменных глобальных переменных.
 */
TEST_F(HybridLocalTaskTest, evaluates_correctly_when_global_vars_unchanged)
{
    // x[0] - глобальная (фиксируем), x[1] - локальная
    std::vector<int> localIndices = { 1 };
    HybridLocalTask hybridTask(task, localIndices);

    // Фиксируем глобальную переменную x[0] = 0.5
    double fixedPoint[2] = { 0.5, 0.0 };
    hybridTask.SetFixedPoint(fixedPoint);

    // Запрашиваем точку, где x[0] совпадает с зафиксированным (0.5), а x[1] = 0.5
    double queryPoint[2] = { 0.5, 0.5 };
    double result = hybridTask.CalculateFuncs(queryPoint, 0);

    // Ожидаем конечное значение
    EXPECT_NEAR(result, 0.53, 1e-9);
    EXPECT_LT(result, 1e100);
}

/**
 * \brief Возврат штрафа при попытке изменить глобальную переменную.
 */
TEST_F(HybridLocalTaskTest, returns_huge_val_when_global_vars_changed)
{
    std::vector<int> localIndices = { 1 }; // x[0] глобальная
    HybridLocalTask hybridTask(task, localIndices);

    double fixedPoint[2] = { 0.5, 0.0 };
    hybridTask.SetFixedPoint(fixedPoint);

    double invalidQueryPoint[2] = { 0.9, 0.5 };
    double result = hybridTask.CalculateFuncs(invalidQueryPoint, 0);

    EXPECT_DOUBLE_EQ(result, HUGE_VAL);
}

/**
 * \brief Корректная делегация вычислений для задач с ограничениями.
 */
TEST_F(HybridLocalTaskTest, delegates_constraints_correctly)
{
    problem = test_env::MakeConstrainedProblem();
    problem->Initialize();
    delete task;
    task = new Task(problem, 0);

    std::vector<int> localIndices = { 1 }; // y0 глобальная, y1 локальная
    double fixedPoint[2] = { 0.2, 0.0 };
    HybridLocalTask hybridTask(task, localIndices);
    hybridTask.SetFixedPoint(fixedPoint);

    // Проверяем точку, где глобальная переменная не тронута
    double queryPoint[2] = { 0.2, 0.5 };

    // Проверяем ограничение (fNumber=0). Ожидаем: 0.1 - 0.2 = -0.1
    double gVal = hybridTask.CalculateFuncs(queryPoint, 0);
    EXPECT_NEAR(gVal, -0.1, 1e-9);

    // Проверяем критерий (fNumber=1). Ожидаем: 0.2^2 + 0.5^2 = 0.29
    double fVal = hybridTask.CalculateFuncs(queryPoint, 1);
    EXPECT_NEAR(fVal, 0.29, 1e-9);
}

class HybridLocalCalculationTest : public ::testing::Test
{
protected:
    IProblem* problem = nullptr;
    Task* task = nullptr;

    void SetUp() override
    {
        ResetParametersToMethodDefaults(2);
        problem = MakeParaboloid(2);
        problem->Initialize();
        task = new Task(problem, 0);
    }

    void TearDown() override
    {
        delete task;
        delete problem;
        ResetParametersToMethodDefaults(2);
    }
};

/**
 * \brief Если индексы не заданы, локальная оптимизация не выполняется.
 */
TEST_F(HybridLocalCalculationTest, does_not_optimize_when_indices_are_empty)
{
    parameters.LocalVarIndices.SetSize(0);
    parameters.TypeCalculation = HybridLocalCalc;

    HybridLocalCalculation calc(*task);

    Trial* t = TrialFactory::CreateTrial();
    t->index = 0;
    t->y[0] = 1.0; t->y[1] = 1.0;
    t->FuncValues[0] = MaxDouble;

    InformationForCalculation in; in.trials.push_back(t);
    TResultForCalculation out;
    calc.Calculate(in, out);

    // Координаты НЕ должны были измениться
    EXPECT_DOUBLE_EQ(out.trials[0]->y[0], 1.0);
    EXPECT_DOUBLE_EQ(out.trials[0]->y[1], 1.0);

    // Но значение функции должно быть вычислено
    EXPECT_LT(out.trials[0]->FuncValues[0], MaxDouble);
    EXPECT_EQ(out.countCalcTrials[0], 1);

    delete t;
}

/**
 * \brief Если все индексы невалидны, локальная оптимизация не выполняется.
 */
TEST_F(HybridLocalCalculationTest, does_not_optimize_when_all_indices_are_invalid)
{
    parameters.LocalVarIndices.SetSize(1);
    parameters.LocalVarIndices[0] = 99;
    parameters.TypeCalculation = HybridLocalCalc;

    HybridLocalCalculation calc(*task);

    Trial* t = TrialFactory::CreateTrial();
    t->index = 0;
    t->y[0] = 1.0; t->y[1] = 1.0;
    t->FuncValues[0] = MaxDouble;

    InformationForCalculation in; in.trials.push_back(t);
    TResultForCalculation out;
    calc.Calculate(in, out);

    // Координаты НЕ должны были измениться
    EXPECT_DOUBLE_EQ(out.trials[0]->y[0], 1.0);
    EXPECT_DOUBLE_EQ(out.trials[0]->y[1], 1.0);

    // Но значение функции должно быть вычислено
    EXPECT_LT(out.trials[0]->FuncValues[0], MaxDouble);
    EXPECT_EQ(out.countCalcTrials[0], 1);

    delete t;
}

/**
 * \brief Реальное улучшение точки и корректный подсчет вычислений.
 *
 * Проверяет, что LocalMethod действительно
 * сдвигает локальную переменную к оптимуму, глобальная остается на месте,
 * а счетчик вычислений (countCalcTrials) увеличивается, предотвращая краши решателя.
 */
TEST_F(HybridLocalCalculationTest, improves_point_and_updates_counters)
{
    parameters.LocalVarIndices.SetSize(1);
    parameters.LocalVarIndices[0] = 1;
    parameters.TypeCalculation = HybridLocalCalc;

    parameters.Epsilon = 0.5;
    parameters.LocalIteration = 100;

    HybridLocalCalculation calc(*task);
    Trial* t = TrialFactory::CreateTrial();
    t->index = 0;
    t->y[0] = 0.3; // Глобальная (уже в оптимуме)
    t->y[1] = 0.9; // Локальная (далеко от оптимума -0.2)
    t->FuncValues[0] = MaxDouble;

    InformationForCalculation in;
    in.trials.push_back(t);
    TResultForCalculation out;

    calc.Calculate(in, out);

    // 1. Глобальная переменная не должна была измениться
    EXPECT_NEAR(out.trials[0]->y[0], 0.3, 1e-6) << "Global variable y[0] changed significantly!";

    // 2. Локальная переменная должна сдвинуться к оптимуму (-0.2)
    EXPECT_LT(out.trials[0]->y[1], 0.9) << "Local method failed to move the point!";

    // 3. Значение функции должно улучшиться. 
    EXPECT_LT(out.trials[0]->FuncValues[0], 1.21) << "Value did not improve!";

    // 4. Счётчик должен быть строго > 0 (критически важно для Method::CalculateFunctionals)
    EXPECT_GT(out.countCalcTrials[0], 0) << "Trials counter is zero!";

    delete t;
}

/**
 * \brief Корректная работа со дискретными переменными.
 */
TEST_F(HybridLocalCalculationTest, handles_discrete_variables_gracefully)
{
    IProblem* oldP = problem;
    Task* oldT = task;

    ResetParametersToMethodDefaults(3);

    // y0, y1 непрерывные, y2 дискретная (3 значения)
    problem = new ProblemFromFunctionPointers(3, 1,
        std::vector<double>(3, -2), std::vector<double>(3, 2), std::vector<int>{3},
        std::vector<std::function<double(const double*)>>(1, [](const double* y) {
            return (y[0] - 0.3) * (y[0] - 0.3) + (y[1] + 0.2) * (y[1] + 0.2) + std::abs(y[2]);
            }));
    problem->Initialize();
    task = new Task(problem, 0);

    parameters.LocalVarIndices.SetSize(1);
    parameters.LocalVarIndices[0] = 0; // Пробуем оптимизировать непрерывную y[0]
    parameters.TypeCalculation = HybridLocalCalc;

    HybridLocalCalculation calc(*task);

    Trial* t = TrialFactory::CreateTrial();
    t->index = 0; t->y[0] = 1.0; t->y[1] = -0.2; t->y[2] = 0.0;
    t->FuncValues[0] = MaxDouble;

    InformationForCalculation in;
    in.trials.push_back(t);
    TResultForCalculation out;

    EXPECT_NO_THROW(calc.Calculate(in, out));

    if (!out.trials.empty() && out.trials[0]) {
        EXPECT_LT(out.trials[0]->FuncValues[0], MaxDouble);
    }

    delete t; delete task; delete problem;
    task = oldT; problem = oldP;
    ResetParametersToMethodDefaults(2);
}

/**
 * \brief Корректное создание вычислителя через фабрику.
 */
TEST(HybridCalculationFactoryTest, creates_hybrid_calculation)
{
    ResetParametersToMethodDefaults(4);
    parameters.TypeCalculation = HybridLocalCalc;
    parameters.LocalVarIndices.SetSize(2);
    parameters.LocalVarIndices[0] = 2;
    parameters.LocalVarIndices[1] = 3;

    IProblem* prob = MakeParaboloid(4);
    prob->Initialize();
    Task* t = new Task(prob, 0);

    Calculation* calc = CalculationFactory::CreateCalculation(*t, nullptr);

    ASSERT_NE(calc, nullptr);
    EXPECT_NE(dynamic_cast<HybridLocalCalculation*>(calc), nullptr);

    delete calc;
    delete t;
    delete prob;
    ResetParametersToMethodDefaults(4);
}