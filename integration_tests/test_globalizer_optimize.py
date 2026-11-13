"""Тесты интерфейса globalizer.optimize, совместимого с scipy.optimize.

Запуск:  python integration_tests/test_globalizer_optimize.py [-v]
Нужен собранный модуль PYDGlobalizer (в _bin или GLOBALIZER_BIN).
Если установлен scipy, дополнительно проверяется совместимость сигнатур и
совпадение решений с scipy.
"""

import inspect
import os
import sys
import tempfile
import unittest
from pathlib import Path

import numpy as np

ROOT = Path(__file__).resolve().parent.parent
sys.path.insert(0, str(ROOT / "PYGlobalizer"))

import globalizer.optimize as gopt          # noqa: E402
from globalizer import optimize as gopt2    # noqa: E402

try:
    import scipy.optimize as sopt
except ImportError:                          # scipy не обязателен
    sopt = None

GLOBAL_FUNCTIONS = ["differential_evolution", "dual_annealing", "shgo", "direct",
                    "basinhopping", "minimize"]


def rastrigin(x, a=10.0):
    x = np.asarray(x)
    return a * x.size + np.sum(x ** 2 - a * np.cos(2 * np.pi * x))


def shifted_sphere(x):
    return float(np.sum((np.asarray(x) - 0.3) ** 2))


def toward_one(x):
    return (x[0] - 1) ** 2 + (x[1] - 1) ** 2


RASTRIGIN_BOUNDS = [(-5.12, 5.12)] * 2
CIRCLE_OPT = 3.0 - 2.0 * np.sqrt(2.0)     # min toward_one при x0^2 + x1^2 <= 1


def call(name, func, bounds, **kw):
    """Единый вызов любой функции модуля (у basinhopping/minimize своя сигнатура)."""
    f = getattr(gopt, name)
    if name == "basinhopping":
        mk = {"bounds": bounds}
        for key in ("args", "constraints"):
            if key in kw:
                mk[key] = kw.pop(key)
        return f(func, np.mean(np.asarray(bounds, float), axis=1), minimizer_kwargs=mk, **kw)
    if name == "minimize":
        return f(func, np.mean(np.asarray(bounds, float), axis=1), bounds=bounds, **kw)
    return f(func, bounds, **kw)


class TestImportStyles(unittest.TestCase):
    def test_import_styles(self):
        self.assertIs(gopt, gopt2)
        for name in GLOBAL_FUNCTIONS + ["OptimizeResult", "Bounds", "NonlinearConstraint",
                                        "LinearConstraint", "OptimizeWarning"]:
            self.assertTrue(hasattr(gopt, name), name)


@unittest.skipIf(sopt is None, "scipy is not installed")
class TestScipySignatures(unittest.TestCase):
    """Любой вызов, корректный для scipy, должен быть корректен и здесь."""

    def check(self, ours, theirs):
        p_ours = inspect.signature(ours).parameters
        for name, p in inspect.signature(theirs).parameters.items():
            self.assertIn(name, p_ours, f"{ours.__name__}: no parameter '{name}'")
            q = p_ours[name]
            if p.kind in (p.POSITIONAL_ONLY, p.POSITIONAL_OR_KEYWORD):
                self.assertIn(q.kind, (q.POSITIONAL_ONLY, q.POSITIONAL_OR_KEYWORD),
                              f"{ours.__name__}: '{name}' must be positional")
        pos_theirs = [n for n, p in inspect.signature(theirs).parameters.items()
                      if p.kind == p.POSITIONAL_OR_KEYWORD]
        pos_ours = [n for n, p in p_ours.items() if p.kind == p.POSITIONAL_OR_KEYWORD]
        self.assertEqual(pos_ours[:len(pos_theirs)], pos_theirs,
                         f"{ours.__name__}: positional order differs from scipy")

    def test_functions(self):
        for name in GLOBAL_FUNCTIONS:
            with self.subTest(name=name):
                self.check(getattr(gopt, name), getattr(sopt, name))

    def test_classes(self):
        for name in ("Bounds", "NonlinearConstraint", "LinearConstraint"):
            with self.subTest(name=name):
                self.check(getattr(gopt, name), getattr(sopt, name))


class TestGlobalFunctions(unittest.TestCase):
    def test_result_fields(self):
        for name in GLOBAL_FUNCTIONS:
            with self.subTest(name=name):
                res = call(name, shifted_sphere, [(-1, 1)] * 2)
                self.assertIsInstance(res, gopt.OptimizeResult)
                for key in ("x", "fun", "success", "status", "message", "nfev", "nit"):
                    self.assertIn(key, res)
                self.assertEqual(res.x.shape, (2,))
                self.assertTrue(res.success)
                self.assertEqual(res.fun, res["fun"])
                self.assertAlmostEqual(res.fun, shifted_sphere(res.x))
                np.testing.assert_allclose(res.x, [0.3, 0.3], atol=1e-3)

    def test_rastrigin_global_minimum(self):
        for name in GLOBAL_FUNCTIONS:
            with self.subTest(name=name):
                res = call(name, rastrigin, RASTRIGIN_BOUNDS)
                self.assertTrue(res.success)
                self.assertLess(res.fun, 1e-3)
                np.testing.assert_allclose(res.x, [0, 0], atol=1e-2)

    def test_args(self):
        res = gopt.differential_evolution(rastrigin, RASTRIGIN_BOUNDS, args=(5.0,))
        self.assertLess(res.fun, 1e-3)
        res = gopt.minimize(lambda x, c: np.sum((x - c) ** 2), [0.0], args=0.25,
                            bounds=[(-1, 1)])          # args не кортеж - как в scipy
        np.testing.assert_allclose(res.x, [0.25], atol=1e-3)

    def test_bounds_object(self):
        res = gopt.direct(shifted_sphere, gopt.Bounds([-1, -1], [1, 1]))
        np.testing.assert_allclose(res.x, [0.3, 0.3], atol=1e-3)

    def test_x0_fixed_variable_and_vectorized(self):
        res = gopt.differential_evolution(lambda x: (x[0] - 0.3) ** 2 + x[1],
                                          [(-1, 1), (2, 2)], x0=[0.3, 2.0])
        np.testing.assert_allclose(res.x, [0.3, 2.0])
        self.assertEqual(res.fun, 2.0)
        res = gopt.differential_evolution(lambda x: np.sum(x ** 2, axis=0), [(-1, 1)] * 2,
                                          vectorized=True)
        self.assertLess(res.fun, 1e-6)

    def test_workers(self):
        res = gopt.differential_evolution(shifted_sphere, [(-1, 1)] * 2, workers=2)
        np.testing.assert_allclose(res.x, [0.3, 0.3], atol=1e-3)


class TestConstraints(unittest.TestCase):
    def check_circle(self, res, tol=2e-3):
        self.assertTrue(res.success, res.message)
        self.assertAlmostEqual(res.fun, CIRCLE_OPT, delta=tol)
        self.assertLessEqual(res.x[0] ** 2 + res.x[1] ** 2, 1.0 + 1e-12)
        self.assertEqual(res.maxcv, 0.0)

    def test_nonlinear_constraint(self):
        con = gopt.NonlinearConstraint(lambda x: x[0] ** 2 + x[1] ** 2, -np.inf, 1.0)
        self.check_circle(gopt.differential_evolution(toward_one, [(-2, 2)] * 2,
                                                      constraints=con))

    def test_dict_constraint(self):
        con = {"type": "ineq", "fun": lambda x, r: r - x[0] ** 2 - x[1] ** 2, "args": (1.0,)}
        for name in ("shgo", "minimize", "basinhopping"):
            with self.subTest(name=name):
                self.check_circle(call(name, toward_one, [(-2, 2)] * 2, constraints=[con]))

    def test_vector_constraint_and_linear(self):
        # x0 + x1 <= 0.5 и x0 >= 0.3 (векторное ограничение из двух компонент)
        lin = gopt.LinearConstraint([[1, 1], [1, 0]], [-np.inf, 0.3], [0.5, np.inf])
        res = gopt.differential_evolution(toward_one, [(-2, 2)] * 2, constraints=lin,
                                          globalizer_options={"eps": 1e-3})
        self.assertTrue(res.success)
        self.assertLessEqual(res.x[0] + res.x[1], 0.5 + 1e-12)
        self.assertGreaterEqual(res.x[0], 0.3)
        # (0.25, 0.25) недоступна из-за x0 >= 0.3: минимум в вершине (0.3, 0.2)
        self.assertAlmostEqual(res.fun, 1.13, delta=5e-3)

    def test_bounds_as_constraint(self):
        res = gopt.differential_evolution(toward_one, [(-2, 2)] * 2,
                                          constraints=gopt.Bounds([-2, -2], [0.5, 0.5]))
        np.testing.assert_allclose(res.x, [0.5, 0.5], atol=1e-2)

    def test_equality_is_approximate(self):
        con = {"type": "eq", "fun": lambda x: x[0] + 2 * x[1] - 1}
        res = gopt.minimize(toward_one, [0, 0], bounds=[(-2, 2)] * 2, constraints=con,
                            globalizer_options={"eps": 1e-3})
        self.assertTrue(res.success)
        self.assertLessEqual(res.maxcv, 1e-2 + 1e-12)
        self.assertAlmostEqual(res.fun, 0.8, delta=0.05)

    def test_infeasible(self):
        con = {"type": "ineq", "fun": lambda x: -1.0 - x[0] ** 2}
        res = gopt.shgo(toward_one, [(-2, 2)] * 2, constraints=con)
        self.assertFalse(res.success)
        self.assertEqual(res.status, 3)
        self.assertGreater(res.maxcv, 0.0)

    @unittest.skipIf(sopt is None, "scipy is not installed")
    def test_scipy_constraint_objects(self):
        con = sopt.NonlinearConstraint(lambda x: x[0] ** 2 + x[1] ** 2, -np.inf, 1.0)
        self.check_circle(gopt.differential_evolution(toward_one, sopt.Bounds([-2, -2], [2, 2]),
                                                      constraints=con))


class TestIntegrality(unittest.TestCase):
    @staticmethod
    def g(x):
        return (x[0] - 1.3) ** 2 + (x[1] + 2.6) ** 2 + (x[2] - 0.4) ** 2

    def test_mixed(self):
        res = gopt.differential_evolution(self.g, [(-5, 5)] * 3, integrality=[True, False, True])
        np.testing.assert_allclose(res.x, [1.0, -2.6, 0.0], atol=1e-3)
        self.assertEqual(res.x[0], round(res.x[0]))

    def test_all_integer(self):
        res = gopt.differential_evolution(self.g, [(-5, 5)] * 3, integrality=True)
        np.testing.assert_array_equal(res.x, [1.0, -3.0, 0.0])

    def test_no_integer_values(self):
        with self.assertRaises(ValueError):
            gopt.differential_evolution(self.g, [(0.2, 0.8)] * 3, integrality=[True, False, False])


class TestCallbacksAndErrors(unittest.TestCase):
    def test_legacy_callback_stop(self):
        calls = []

        def cb(xk, convergence):
            calls.append(xk)
            return len(calls) >= 3
        res = gopt.differential_evolution(shifted_sphere, [(-1, 1)] * 3, callback=cb)
        self.assertEqual(len(calls), 3)
        self.assertEqual(res.status, 2)
        self.assertFalse(res.success)

    def test_intermediate_result_stop_iteration(self):
        seen = []

        def cb(intermediate_result):
            seen.append(intermediate_result.fun)
            if intermediate_result.fun < 0.5:
                raise StopIteration
        res = gopt.minimize(shifted_sphere, [0.9, 0.9], bounds=[(-1, 1)] * 2, callback=cb)
        self.assertEqual(res.status, 2)
        self.assertLess(res.fun, 0.5)
        self.assertEqual(seen[-1], res.fun)

    def test_dual_annealing_callback(self):
        args = []
        gopt.dual_annealing(shifted_sphere, [(-1, 1)], callback=lambda x, f, c: args.append(c))
        self.assertTrue(args)

    def test_exception_propagates(self):
        def bad(x):
            if x[0] > 0.5:
                raise ZeroDivisionError("boom")
            return x[0] ** 2
        with self.assertRaises(ZeroDivisionError):
            gopt.direct(bad, [(-1, 1)])

    def test_direct_f_min(self):
        res = gopt.direct(shifted_sphere, [(-1, 1)] * 2, f_min=0.0, f_min_rtol=1e-2,
                          globalizer_options={"local_search": False})
        self.assertTrue(res.success)
        self.assertLessEqual(res.fun, 1e-2)

    def test_bad_arguments(self):
        for bounds in ([(-1, None)], [(1, -1)], None, [(-np.inf, 1)]):
            with self.subTest(bounds=bounds), self.assertRaises(ValueError):
                gopt.differential_evolution(shifted_sphere, bounds)
        with self.assertRaises(ValueError):
            gopt.minimize(shifted_sphere, [0.0])             # без bounds
        with self.assertRaises(ValueError):
            gopt.basinhopping(shifted_sphere, [0.0])         # без bounds
        with self.assertRaises(TypeError):
            gopt.direct(shifted_sphere, [(-1, 1)], globalizer_options={"unknown": 1})


class TestOptions(unittest.TestCase):
    def tearDown(self):
        gopt.set_options()

    def test_set_options(self):
        self.assertEqual(gopt.set_options(r=3.0, eps=0.005), {"r": 3.0, "eps": 0.005})
        self.assertEqual(gopt.get_options(), {"r": 3.0, "eps": 0.005})
        res = gopt.dual_annealing(shifted_sphere, [(-1, 1)] * 2)
        self.assertTrue(res.success)
        gopt.set_options()
        self.assertEqual(gopt.get_options(), {})
        with self.assertRaises(TypeError):
            gopt.set_options(popsize=10)

    def test_maxfev(self):
        res = gopt.differential_evolution(rastrigin, [(-5.12, 5.12)] * 4,
                                          globalizer_options={"maxfev": 200, "eps": 1e-4,
                                                              "local_search": False})
        self.assertEqual(res.status, 1)
        self.assertLessEqual(res.nfev, 200)

    def test_no_files_in_cwd(self):
        with tempfile.TemporaryDirectory() as tmp:
            old = os.getcwd()
            os.chdir(tmp)
            try:
                gopt.differential_evolution(shifted_sphere, [(-1, 1)] * 2)
                with self.assertRaises(ZeroDivisionError):
                    gopt.direct(lambda x: 1 / 0, [(-1, 1)])
                self.assertEqual(os.listdir(tmp), [])
            finally:
                os.chdir(old)


@unittest.skipIf(sopt is None, "scipy is not installed")
class TestSameAnswerAsScipy(unittest.TestCase):
    """Один и тот же код со scipy и с Globalizer даёт тот же минимум."""

    def test_same_minimum(self):
        con = sopt.NonlinearConstraint(lambda x: x[0] ** 2 + x[1] ** 2, -np.inf, 1.0)
        cases = [
            ("differential_evolution", (rastrigin, RASTRIGIN_BOUNDS), {"seed": 1}),
            ("differential_evolution", (toward_one, [(-2, 2)] * 2), {"constraints": con, "seed": 1}),
            ("dual_annealing", (rastrigin, RASTRIGIN_BOUNDS), {"seed": 1}),
            ("shgo", (rastrigin, RASTRIGIN_BOUNDS), {}),
            ("direct", (rastrigin, RASTRIGIN_BOUNDS), {}),
        ]
        for name, args, kw in cases:
            with self.subTest(name=name):
                ours = getattr(gopt, name)(*args, **kw)
                theirs = getattr(sopt, name)(*args, **kw)
                self.assertAlmostEqual(ours.fun, theirs.fun, delta=2e-3)
                np.testing.assert_allclose(ours.x, theirs.x, atol=2e-2)


if __name__ == "__main__":
    unittest.main()
