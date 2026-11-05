#include <iostream>
#include <vector>
#include <cmath>
#include <iomanip>
#include <fstream>
#include <string>
#include <functional>
#include <limits>

#include "json.hpp"

// ПРИМЕЧАНИЕ: макрос _GLOBALIZER_BENCHMARKS задаётся в CMake (add_definitions),
// поэтому здесь его повторно НЕ определяем — иначе предупреждение C4005.

#include "Globalizer.h"
#include "Solver.h"
#include "ProblemInterface.h"
#include "Parameters.h"
#include "Trial.h"
#ifdef _GLOBALIZER_BENCHMARKS
#include "IGlobalOptimizationProblem.h"
#include "GlobalOptimizationProblemManager.h"
#endif // _GLOBALIZER_BENCHMARKS

using json = nlohmann::json;

// ==============================================================================
// Вспомогательные функции
// ==============================================================================

void PrintPoint(const double* p, int dim)
{
    std::cout << "[";
    for (int i = 0; i < dim; i++)
    {
        std::cout << std::fixed << std::setprecision(6) << p[i] << (i == dim - 1 ? "" : ", ");
    }
    std::cout << "]";
}

double CalculateDistance(const double* p1, const std::vector<double>& p2, int dim)
{
    double sum = 0.0;
    for (int i = 0; i < dim; i++)
    {
        double diff = p1[i] - p2[i];
        sum += diff * diff;
    }
    return std::sqrt(sum);
}

// ==============================================================================
// Фабрика задач
// ==============================================================================
IProblem* CreateProblem(const json& config, int dimension)
{
    std::string name = config["name"].get<std::string>();
    std::string type = config.value("type", "Benchmark");

    if (type == "Simple")
    {
        if (name == "Hill")
        {
            return new ProblemFromFunctionPointers(
                dimension, std::vector<double>(dimension, 0.0), std::vector<double>(dimension, 1.0),
                std::vector<std::function<double(const double*)>>(1, [dimension](const double* y) {
                    double sum = 0.0;
                    for (int j = 0; j < dimension; j++) sum += (y[j] - 0.5) * (y[j] - 0.5);
                    return sum;
                    }), true, 0.0, std::vector<double>(dimension, 0.5)
                        );
        }
        else if (name.find("Rastrigin") != std::string::npos)
        {
            return new ProblemFromFunctionPointers(
                dimension, std::vector<double>(dimension, -2.2), std::vector<double>(dimension, 1.8),
                std::vector<std::function<double(const double*)>>(1, [dimension](const double* y) {
                    double pi = 3.14159265358979323846;
                    double sum = 0.0;
                    for (int j = 0; j < dimension; j++) sum += y[j] * y[j] - 10.0 * cos(2.0 * pi * y[j]) + 10.0;
                    return sum;
                    }), true, 0.0, std::vector<double>(dimension, 0.0)
                        );
        }
        else if (name == "StronginC3_Lambda")
        {
            return new ProblemFromFunctionPointers(
                dimension, { 0.0, -1.0 }, { 4.0, 3.0 },
                std::vector<std::function<double(const double*)>>({
                  [](const double* y) { return 0.01 * ((y[0] - 2.2) * (y[0] - 2.2) + (y[1] - 1.2) * (y[1] - 1.2) - 2.25); },
                  [](const double* y) { return 100.0 * (1.0 - ((y[0] - 2.0) / 1.2) * ((y[0] - 2.0) / 1.2) - (y[1] / 2.0) * (y[1] / 2.0)); },
                  [](const double* y) { return 10.0 * (y[1] - 1.5 - 1.5 * sin(6.283 * (y[0] - 1.75))); },
                  [](const double* y) {
                    double t1 = pow(0.5 * y[0] - 0.5, 4.0);
                    double t2 = pow(y[1] - 1.0, 4.0);
                    return -((1.5 * y[0] * y[0] * exp(1.0 - y[0] * y[0] - 20.25 * (y[0] - y[1]) * (y[0] - y[1]))) + t1 * t2 * exp(2.0 - t1 - t2));
                  }
                    }), true, -1.489444, { 0.941176, 0.941176 }
                  );
        }
    }
    return nullptr;
}

// ==============================================================================
// Запуск теста
// ==============================================================================
bool RunSingleTest(const json& config)
{
    std::string name = config["name"].get<std::string>();
    int dimension = config["dimension"].get<int>();

    std::cout << "------------------------------------------------" << std::endl;
    std::cout << "[RUN] Testing: " << name << " (Dimension: " << dimension << ")" << std::endl;

    IProblem* manualProblem = CreateProblem(config, dimension);
    if (!manualProblem) { std::cout << "      Result: SKIP" << std::endl; return true; }

    char progName[] = "tester";
    char* dummyArgv[] = { progName, nullptr };

    try
    {
        parameters.Init(1, dummyArgv, false);
        parameters.Dimension = dimension;
        parameters.r = config["r"].get<double>();
        parameters.Epsilon = config.value("epsilon", 0.01);

        parameters.m = 10;

        parameters.IsPrintResultToConsole = false;
        parameters.DisablePrintParameters = true;

        std::string multipliers = "1.0 1.0 1.0 1.0 1.0 1.0 1.0 1.0 1.0 1.0 "
            "1.0 1.0 1.0 1.0 1.0 1.0 1.0 1.0 1.0 1.0";
        parameters.SetVal("functionSignMultiplier", multipliers);
    }
    catch (...) { std::cout << "      Result: FAIL (Params)" << std::endl; return false; }

    manualProblem->Initialize();

    Solver* solver = new Solver(manualProblem);

    bool passed = true;
    try
    {
        if (solver->Solve() != SYSTEM_OK)
        {
            std::cout << "      Result: FAIL (Solver Error)" << std::endl;
            delete solver; return false;
        }
    }
    catch (...)
    {
        std::cout << "      Result: FAIL (Exception)" << std::endl;
        delete solver; return false;
    }

    SolutionResult* result = solver->GetSolutionResult();
    if (result && result->BestTrial)
    {
        int totalFuncs = manualProblem->GetNumberOfFunctions();
        double actualValue = result->BestTrial->FuncValues[totalFuncs - 1];
        double expectedValue = config["optimum"].get<double>();
        double tolerance = config["tolerance"].get<double>();

        std::cout << "      Value:    Expected " << std::fixed << std::setprecision(6) << expectedValue
            << ", Actual " << actualValue
            << " (Diff: " << std::scientific << std::abs(actualValue - expectedValue) << ")" << std::endl;

        if (std::abs(actualValue - expectedValue) > tolerance) passed = false;

        if (config.contains("point"))
        {
            std::vector<double> expectedPoint = config["point"].get<std::vector<double>>();
            double dist = CalculateDistance(result->BestTrial->y, expectedPoint, dimension);

            std::cout << "      Point:    Expected "; PrintPoint(expectedPoint.data(), dimension);
            std::cout << "\n                Actual   "; PrintPoint(result->BestTrial->y, dimension);
            std::cout << " (Dist: " << std::fixed << std::setprecision(6) << dist << ")" << std::endl;

            if (dist > 0.5) passed = false;
        }

        if (config.contains("expected_iterations"))
        {
            int expectedIters = config["expected_iterations"].get<int>();
            std::cout << "      Trials:   " << result->TrialCount << " calculations (iOpt: " << expectedIters << ")" << std::endl;
        }
        else
        {
            std::cout << "      Trials:   " << result->TrialCount << " calculations" << std::endl;
        }
    }
    else passed = false;

    std::cout << "      Status:   " << (passed ? "PASS" : "FAIL") << std::endl;

    delete solver;

    return passed;
}

int main(int argc, char* argv[])
{
    std::string jsonPath = "tests.json";
    if (argc > 1) jsonPath = argv[1];

    std::ifstream f(jsonPath);
    if (!f.is_open()) { std::cerr << "Cannot open config file!" << std::endl; return 1; }

    json testsData = json::parse(f);
    int passed = 0;
    std::cout << "================================================" << std::endl;
    std::cout << "   GLOBALIZER INTEGRATION TESTS STARTING" << std::endl;
    std::cout << "================================================" << std::endl;

    for (const auto& testConfig : testsData)
    {
        if (RunSingleTest(testConfig)) passed++;
    }

    std::cout << "================================================" << std::endl;
    std::cout << "Summary: " << passed << "/" << testsData.size() << " tests passed." << std::endl;
    std::cout << "================================================" << std::endl;

    return (passed == (int)testsData.size()) ? 0 : 1;
}
