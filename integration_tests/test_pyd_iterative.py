"""Тесты пошаговой работы PYDGlobalizer.IterativeSolver (аналог IterationMain.cpp).

Запуск:  python integration_tests/test_pyd_iterative.py [-v]
Нужен собранный модуль PYDGlobalizer (в _bin или GLOBALIZER_BIN).
"""

import os
import sys
import tempfile
import unittest
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent
_bin = Path(os.environ.get("GLOBALIZER_BIN", ROOT / "_bin"))
sys.path[:0] = [str(_bin), str(ROOT / "PYGlobalizer")]

import PYDGlobalizer as g                  # noqa: E402
from PYProblem import PYProblem            # noqa: E402

LOG_PREFIX = os.path.join(tempfile.gettempdir(), "globalizer_test_log")


def make_params(**kw):
    p = g.SolverParameters()
    p.verbose = 0
    p.print_result = False
    p.log_prefix = LOG_PREFIX
    for k, v in kw.items():
        setattr(p, k, v)
    return p


def sphere_problem():
    problem = PYProblem(dimension=2)
    problem.set_bounds([-1.0, -1.0], [1.0, 1.0])
    problem.add_function(lambda x: (x[0] - 0.3) ** 2 + (x[1] + 0.2) ** 2)
    return problem


class TestIterativeSolver(unittest.TestCase):
    def test_same_result_as_solve(self):
        """Пошаговый прогон до конца даёт то же, что и solve()."""
        params = make_params(max_iterations=5000)
        full = g.solve(sphere_problem(), params)
        solver = g.IterativeSolver(sphere_problem(), params)
        steps = 0
        while not solver.do_iteration():
            steps += 1
        res = solver.result()
        self.assertTrue(res["finished"])
        self.assertTrue(res["feasible"])
        self.assertEqual(res["best_value"], full["best_value"])
        self.assertEqual(res["best_point"], full["best_point"])
        self.assertEqual(res["iterations"], full["iterations"])

    def test_data_changes_between_iterations(self):
        """Как в IterationMain.cpp: данные функции меняются между итерациями."""
        data = {"value": 100.0}
        seen = []

        def f(x):
            seen.append(data["value"])
            return x[0] * x[0] * data["value"]

        problem = PYProblem(dimension=1)
        problem.set_bounds([0.0], [10.0])
        problem.add_function(f)
        solver = g.IterativeSolver(problem, make_params(stop_condition="iterations",
                                                        max_iterations=300))
        for it in range(1000):
            data["value"] = it + 1.0
            if solver.do_iteration():
                break
        self.assertTrue(solver.finished)
        self.assertLessEqual(solver.iterations, 301)        # лимит max_iterations
        self.assertGreater(len(set(seen)), 250)             # функция видела новые данные
        res = solver.result()
        self.assertLess(res["best_point"][0], 0.05)

    def test_stop_condition_iterations_ignores_accuracy(self):
        acc = g.IterativeSolver(sphere_problem(), make_params(max_iterations=3000))
        acc.run()
        it_only = g.IterativeSolver(sphere_problem(),
                                    make_params(max_iterations=3000, stop_condition="iterations"))
        it_only.run()
        # Итог завершённого решателя доступен и после запуска следующего.
        self.assertFalse(acc.is_active)
        self.assertLess(acc.iterations, 3000)               # остановка по точности
        self.assertTrue(acc.result()["finished"])
        self.assertGreaterEqual(it_only.iterations, 3000)   # только по лимиту

    def test_run_and_result(self):
        solver = g.IterativeSolver(sphere_problem(), make_params(max_iterations=5000))
        self.assertEqual(solver.run(10), 10)
        self.assertFalse(solver.finished)
        mid = solver.result()
        self.assertFalse(mid["finished"])
        self.assertIsNotNone(mid["best_value"])
        rest = solver.run()
        self.assertGreater(rest, 0)
        self.assertTrue(solver.finished)
        self.assertEqual(solver.run(), 0)
        self.assertTrue(solver.do_iteration())              # после конца - сразу True
        self.assertLessEqual(solver.result()["best_value"], mid["best_value"])

    def test_iterator_protocol(self):
        states = list(g.IterativeSolver(sphere_problem(), make_params(max_iterations=5000)))
        self.assertTrue(states[-1]["finished"])
        values = [s["best_value"] for s in states if s["best_value"] is not None]
        self.assertEqual(values, sorted(values, reverse=True))   # лучшее не ухудшается
        self.assertLess(values[-1], 1e-3)

    def test_constrained_feasible(self):
        problem = PYProblem(dimension=2)
        problem.set_bounds([-2.0, -2.0], [2.0, 2.0])
        problem.add_function(lambda x: x[0] ** 2 + x[1] ** 2 - 1.0)      # ограничение
        problem.add_function(lambda x: (x[0] - 1) ** 2 + (x[1] - 1) ** 2)  # критерий
        solver = g.IterativeSolver(problem, make_params(max_iterations=5000))
        solver.run()
        res = solver.result()
        self.assertTrue(res["feasible"])
        x = res["best_point"]
        self.assertLessEqual(x[0] ** 2 + x[1] ** 2, 1.0)
        self.assertAlmostEqual(res["best_value"], 3 - 2 * 2 ** 0.5, delta=0.01)

    def test_invalidated_by_solve(self):
        solver = g.IterativeSolver(sphere_problem(), make_params(max_iterations=5000))
        solver.run(5)
        self.assertTrue(solver.is_active)
        g.solve(sphere_problem(), make_params())
        self.assertFalse(solver.is_active)
        with self.assertRaises(RuntimeError):
            solver.do_iteration()
        with self.assertRaises(RuntimeError):
            solver.result()

    def test_invalidated_by_new_solver(self):
        first = g.IterativeSolver(sphere_problem(), make_params())
        second = g.IterativeSolver(sphere_problem(), make_params())
        with self.assertRaises(RuntimeError):
            first.do_iteration()
        self.assertFalse(second.do_iteration())

    def test_bad_parameters(self):
        with self.assertRaises(ValueError):
            g.IterativeSolver(sphere_problem(), make_params(stop_condition="never"))
        with self.assertRaises(ValueError):
            g.IterativeSolver(sphere_problem(), make_params(max_iterations=1))
        with self.assertRaises(ValueError):
            g.IterativeSolver(object(), make_params())


if __name__ == "__main__":
    unittest.main()
