"""Тесты интерфейса globalizer.optuna, совместимого с Optuna.

Запуск:  python integration_tests/test_globalizer_optuna.py [-v]
Нужен собранный модуль PYDGlobalizer (в _bin или GLOBALIZER_BIN).
Если установлена Optuna, дополнительно проверяется совместимость сигнатур и
то, что одна и та же целевая функция работает с обеими библиотеками.
"""

import inspect
import math
import sys
import unittest
import warnings
from pathlib import Path

import numpy as np

ROOT = Path(__file__).resolve().parent.parent
sys.path.insert(0, str(ROOT / "PYGlobalizer"))

import globalizer.optuna as goptuna            # noqa: E402
from globalizer import optuna as goptuna2      # noqa: E402

try:
    import optuna as real_optuna
except ImportError:                             # Optuna не обязательна
    real_optuna = None

goptuna.logging.set_verbosity(goptuna.logging.ERROR)
if real_optuna is not None:
    real_optuna.logging.set_verbosity(real_optuna.logging.WARNING)


def quad_objective(trial):
    x = trial.suggest_float("x", -10, 10)
    y = trial.suggest_float("y", -10, 10)
    return (x - 2.0) ** 2 + (y + 1.0) ** 2


def rastrigin_objective(trial):
    x = np.array([trial.suggest_float(f"x{i}", -5.12, 5.12) for i in range(2)]) - 0.7
    return float(10 * x.size + np.sum(x ** 2 - 10 * np.cos(2 * np.pi * x)))


class TestBasics(unittest.TestCase):

    def test_import_styles(self):
        self.assertIs(goptuna, goptuna2)
        for name in ("create_study", "Study", "Trial", "FrozenTrial", "TrialState",
                     "TrialPruned", "samplers", "pruners", "distributions", "logging"):
            self.assertTrue(hasattr(goptuna, name), name)

    def test_quadratic(self):
        study = goptuna.create_study()
        study.optimize(quad_objective, n_trials=150)
        self.assertEqual(len(study.trials), 150)
        self.assertLess(study.best_value, 1e-3)
        self.assertAlmostEqual(study.best_params["x"], 2.0, delta=0.05)
        self.assertAlmostEqual(study.best_params["y"], -1.0, delta=0.05)
        self.assertEqual(study.best_trial.state, goptuna.trial.TrialState.COMPLETE)

    def test_multimodal(self):
        study = goptuna.create_study()
        study.optimize(rastrigin_objective, n_trials=1000)
        self.assertLess(study.best_value, 0.05)       # глобальный минимум 0 в (0.7, 0.7)

    def test_deterministic(self):
        a = goptuna.create_study()
        a.optimize(quad_objective, n_trials=60)
        b = goptuna.create_study()
        b.optimize(quad_objective, n_trials=60)
        self.assertEqual([t.params for t in a.trials], [t.params for t in b.trials])

    def test_maximize(self):
        study = goptuna.create_study(direction="maximize")
        study.optimize(lambda t: -quad_objective(t), n_trials=150)
        self.assertGreater(study.best_value, -1e-3)
        self.assertEqual(study.direction, goptuna.study.StudyDirection.MAXIMIZE)

    def test_no_duplicate_points(self):
        study = goptuna.create_study()
        study.optimize(lambda t: (t.suggest_int("n", 0, 20) - 7) ** 2, n_trials=200)
        values = [t.params["n"] for t in study.trials]
        self.assertEqual(len(values), len(set(values)))   # каждое значение - один раз
        self.assertEqual(study.best_params["n"], 7)
        self.assertLess(len(values), 200)                  # перебор кончился раньше

    def test_continue_optimize(self):
        study = goptuna.create_study()
        study.optimize(quad_objective, n_trials=40)
        first = study.best_value
        study.optimize(quad_objective, n_trials=110)
        self.assertEqual(len(study.trials), 150)
        self.assertLessEqual(study.best_value, first)
        keys = [(t.params["x"], t.params["y"]) for t in study.trials]
        self.assertEqual(len(keys), len(set(keys)))


class TestSearchSpace(unittest.TestCase):

    def test_mixed_types(self):
        def objective(trial):
            lr = trial.suggest_float("lr", 1e-5, 1e-1, log=True)
            n = trial.suggest_int("n", 1, 100)
            k = trial.suggest_categorical("kind", ["a", "b", "c"])
            q = trial.suggest_float("q", 0.0, 1.0, step=0.1)
            self.assertIsInstance(n, int)
            self.assertIn(k, ("a", "b", "c"))
            self.assertAlmostEqual(q * 10, round(q * 10), places=9)
            self.assertTrue(1e-5 <= lr <= 1e-1)
            return (math.log10(lr) + 3) ** 2 + (n - 37) ** 2 / 100 + \
                {"a": 1.0, "b": 0.0, "c": 2.0}[k] + (q - 0.3) ** 2

        study = goptuna.create_study()
        study.optimize(objective, n_trials=300)
        p = study.best_params
        self.assertEqual(p["kind"], "b")
        self.assertLessEqual(abs(p["n"] - 37), 1)
        self.assertAlmostEqual(p["q"], 0.3)
        self.assertLess(abs(math.log10(p["lr"]) + 3), 0.1)

    def test_int_log_and_step(self):
        def objective(trial):
            a = trial.suggest_int("a", 1, 1024, log=True)
            b = trial.suggest_int("b", 0, 100, step=5)
            self.assertEqual(b % 5, 0)
            return (math.log2(a) - 6) ** 2 + (b - 45) ** 2

        study = goptuna.create_study()
        study.optimize(objective, n_trials=200)
        self.assertEqual(study.best_params, {"a": 64, "b": 45})

    def test_conditional_space(self):
        def objective(trial):
            model = trial.suggest_categorical("model", ["linear", "quad"])
            if model == "linear":
                a = trial.suggest_float("a", -1, 1)
                return 1.0 + abs(a)
            b = trial.suggest_float("b", -2, 2)
            return (b - 0.5) ** 2

        study = goptuna.create_study()
        study.optimize(objective, n_trials=150)
        self.assertEqual(study.best_params["model"], "quad")
        self.assertAlmostEqual(study.best_params["b"], 0.5, delta=0.02)
        self.assertNotIn("a", study.best_params)

    def test_same_param_twice(self):
        def objective(trial):
            x1 = trial.suggest_float("x", 0, 1)
            x2 = trial.suggest_float("x", 0, 1)
            self.assertEqual(x1, x2)
            return x1

        goptuna.create_study().optimize(objective, n_trials=5)

    def test_incompatible_distribution(self):
        study = goptuna.create_study()
        study.optimize(lambda t: t.suggest_float("x", 0, 1), n_trials=2)
        with self.assertRaises(ValueError):
            study.optimize(lambda t: t.suggest_categorical("x", ["a", "b"]), n_trials=1,
                           catch=())

    def test_only_categorical(self):
        def objective(trial):
            a = trial.suggest_categorical("a", [1, 2, 3, 4])
            b = trial.suggest_categorical("b", ["x", "y"])
            return (a - 3) ** 2 + (b == "x")

        study = goptuna.create_study()
        study.optimize(objective, n_trials=50)
        self.assertEqual(study.best_params, {"a": 3, "b": "y"})
        self.assertLessEqual(len(study.trials), 8)          # всего 8 сочетаний

    def test_distributions_validation(self):
        D = goptuna.distributions
        with self.assertRaises(ValueError):
            D.FloatDistribution(1, 0)
        with self.assertRaises(ValueError):
            D.FloatDistribution(0, 1, log=True)
        with self.assertRaises(ValueError):
            D.FloatDistribution(0.1, 1, log=True, step=0.1)
        with self.assertRaises(ValueError):
            D.IntDistribution(0, 10, log=True)
        self.assertEqual(D.IntDistribution(0, 10, step=3).high, 9)
        self.assertEqual(D.FloatDistribution(0, 1, step=0.3).high, 0.9)


class TestConstraints(unittest.TestCase):
    EXACT = 3.0 - 2.0 * math.sqrt(2.0)          # min (x-1)^2+(y-1)^2 при x^2+y^2 <= 1

    def test_set_constraint(self):
        def objective(trial):
            x = trial.suggest_float("x", -2, 2)
            y = trial.suggest_float("y", -2, 2)
            trial.set_constraint("circle", x * x + y * y - 1.0)
            return (x - 1) ** 2 + (y - 1) ** 2

        study = goptuna.create_study()
        study.optimize(objective, n_trials=400)
        best = study.best_trial
        self.assertLessEqual(best.constraints["circle"], 0.0)
        self.assertAlmostEqual(study.best_value, self.EXACT, delta=0.01)
        # Лучшее по значению испытание недопустимо - best_trial его пропускает.
        self.assertLess(min(t.value for t in study.trials if t.value is not None), self.EXACT)

    def test_constraints_func(self):
        def objective(trial):
            x = trial.suggest_float("x", -2, 2)
            y = trial.suggest_float("y", -2, 2)
            trial.set_user_attr("g", x * x + y * y - 1.0)
            return (x - 1) ** 2 + (y - 1) ** 2

        sampler = goptuna.samplers.GlobalizerSampler(
            constraints_func=lambda t: [t.user_attrs["g"]])
        study = goptuna.create_study(sampler=sampler)
        study.optimize(objective, n_trials=400)
        self.assertLessEqual(study.best_trial.constraints["0"], 0.0)
        self.assertAlmostEqual(study.best_value, self.EXACT, delta=0.01)

    def test_no_feasible(self):
        def objective(trial):
            x = trial.suggest_float("x", 0, 1)
            trial.set_constraint("c", 1.0 + x)
            return x

        study = goptuna.create_study()
        study.optimize(objective, n_trials=20)
        with self.assertRaises(ValueError):
            study.best_trial


class TestAskTell(unittest.TestCase):

    def test_ask_tell(self):
        study = goptuna.create_study()
        for _ in range(150):
            trial = study.ask()
            study.tell(trial, quad_objective(trial))
        self.assertLess(study.best_value, 1e-3)

    def test_tell_by_number_and_state(self):
        TS = goptuna.trial.TrialState
        study = goptuna.create_study()
        t = study.ask()
        t.suggest_float("x", 0, 1)
        study.tell(t.number, state=TS.FAIL)
        self.assertEqual(study.trials[0].state, TS.FAIL)
        with self.assertRaises(RuntimeError):
            study.tell(t, 1.0)
        study.tell(t, 1.0, skip_if_finished=True)

    def test_fixed_distributions(self):
        D = goptuna.distributions
        study = goptuna.create_study()
        for _ in range(100):
            trial = study.ask({"x": D.FloatDistribution(-3, 3), "n": D.IntDistribution(0, 5)})
            self.assertEqual(set(trial.params), {"x", "n"})
            study.tell(trial, (trial.params["x"] - 1) ** 2 + (trial.params["n"] - 4) ** 2)
        self.assertEqual(study.best_params["n"], 4)
        self.assertAlmostEqual(study.best_params["x"], 1.0, delta=0.05)

    def test_parallel_asks(self):
        study = goptuna.create_study()
        study.optimize(quad_objective, n_trials=5)
        a, b = study.ask(), study.ask()            # вторая точка - случайная
        for t in (a, b):
            study.tell(t, quad_objective(t))
        study.optimize(quad_objective, n_trials=100)
        self.assertEqual(len(study.trials), 107)
        self.assertLess(study.best_value, 1e-2)

    def test_two_studies_interleaved(self):
        s1 = goptuna.create_study()
        s2 = goptuna.create_study(direction="maximize")
        for _ in range(120):
            t1 = s1.ask()
            s1.tell(t1, quad_objective(t1))
            t2 = s2.ask()
            x = t2.suggest_float("x", 0, 3)
            s2.tell(t2, math.sin(3 * x) * x)
        self.assertLess(s1.best_value, 1e-2)
        self.assertGreater(s2.best_value, 2.6)


class TestTrialsAndCallbacks(unittest.TestCase):

    def test_enqueue_trial(self):
        study = goptuna.create_study()
        study.enqueue_trial({"x": 2.0, "y": -1.0}, user_attrs={"src": "guess"})
        study.optimize(quad_objective, n_trials=3)
        first = study.trials[0]
        self.assertEqual(first.params, {"x": 2.0, "y": -1.0})
        self.assertEqual(first.user_attrs["src"], "guess")
        self.assertEqual(study.best_value, 0.0)

    def test_add_trial(self):
        D = goptuna.distributions
        study = goptuna.create_study()
        study.add_trial(goptuna.create_trial(
            params={"x": 2.0, "y": -1.0}, value=0.0,
            distributions={"x": D.FloatDistribution(-10, 10), "y": D.FloatDistribution(-10, 10)}))
        study.optimize(quad_objective, n_trials=20)
        self.assertEqual(study.best_trial.number, 0)
        self.assertEqual(len(study.trials), 21)

    def test_failures_and_catch(self):
        TS = goptuna.trial.TrialState

        def objective(trial):
            x = trial.suggest_float("x", -1, 1)
            if x > 0.5:
                raise ArithmeticError("bad region")
            if x < -0.5:
                return float("nan")
            return x * x

        study = goptuna.create_study()
        study.optimize(objective, n_trials=80, catch=(ArithmeticError,))
        states = {t.state for t in study.trials}
        self.assertIn(TS.FAIL, states)
        self.assertLess(study.best_value, 1e-3)

        with self.assertRaises(ArithmeticError):
            goptuna.create_study().optimize(
                lambda t: 1 / 0 if t.suggest_float("x", 0, 1) >= 0 else 0, n_trials=5)

    def test_callbacks_and_stop(self):
        seen = []

        def cb(study, frozen):
            seen.append(frozen.number)
            if frozen.value is not None and frozen.value < 0.1:
                study.stop()

        study = goptuna.create_study()
        study.optimize(quad_objective, n_trials=1000, callbacks=[cb])
        self.assertLess(len(study.trials), 1000)
        self.assertEqual(seen, [t.number for t in study.trials])
        self.assertLess(study.trials[-1].value, 0.1)

    def test_timeout(self):
        import time
        study = goptuna.create_study()
        t0 = time.time()
        study.optimize(lambda t: (time.sleep(0.01), quad_objective(t))[1], timeout=0.3)
        self.assertLess(time.time() - t0, 2.0)
        self.assertGreater(len(study.trials), 3)

    def test_pruning(self):
        TS = goptuna.trial.TrialState

        def objective(trial):
            x = trial.suggest_float("x", -3, 3)
            for step in range(10):
                value = (x - 1) ** 2 + 1.0 / (step + 1)
                trial.report(value, step)
                if trial.should_prune():
                    raise goptuna.TrialPruned()
            return (x - 1) ** 2

        study = goptuna.create_study(pruner=goptuna.pruners.MedianPruner(n_startup_trials=5))
        study.optimize(objective, n_trials=150)
        states = [t.state for t in study.trials]
        self.assertIn(TS.PRUNED, states)
        self.assertLess(study.best_value, 1e-3)
        pruned = next(t for t in study.trials if t.state == TS.PRUNED)
        self.assertIsNotNone(pruned.value)                  # последнее промежуточное значение

    def test_fixed_trial(self):
        self.assertEqual(quad_objective(goptuna.trial.FixedTrial({"x": 2.0, "y": -1.0})), 0.0)

    def test_best_trial_rerun(self):
        study = goptuna.create_study()
        study.optimize(quad_objective, n_trials=30)
        self.assertEqual(quad_objective(study.best_trial), study.best_value)


@unittest.skipIf(real_optuna is None, "optuna is not installed")
class TestCompatibilityWithOptuna(unittest.TestCase):

    def _check_signature(self, ours, theirs):
        ps = list(inspect.signature(ours).parameters)
        pt = list(inspect.signature(theirs).parameters)
        self.assertEqual(ps, pt, f"{ours.__qualname__}: {ps} != {pt}")

    def test_signatures(self):
        pairs = [
            (goptuna.create_study, real_optuna.create_study),
            (goptuna.Study.optimize, real_optuna.Study.optimize),
            (goptuna.Study.ask, real_optuna.Study.ask),
            (goptuna.Study.tell, real_optuna.Study.tell),
            (goptuna.Study.enqueue_trial, real_optuna.Study.enqueue_trial),
            (goptuna.Study.get_trials, real_optuna.Study.get_trials),
            (goptuna.Study.trials_dataframe, real_optuna.Study.trials_dataframe),
            (goptuna.Trial.suggest_float, real_optuna.Trial.suggest_float),
            (goptuna.Trial.suggest_int, real_optuna.Trial.suggest_int),
            (goptuna.Trial.suggest_categorical, real_optuna.Trial.suggest_categorical),
            (goptuna.Trial.report, real_optuna.Trial.report),
            (goptuna.Trial.set_constraint, real_optuna.Trial.set_constraint),
            (goptuna.pruners.MedianPruner.__init__, real_optuna.pruners.MedianPruner.__init__),
            (goptuna.trial.create_trial, real_optuna.trial.create_trial),
            (goptuna.distributions.FloatDistribution.__init__,
             real_optuna.distributions.FloatDistribution.__init__),
            (goptuna.distributions.IntDistribution.__init__,
             real_optuna.distributions.IntDistribution.__init__),
        ]
        for ours, theirs in pairs:
            self._check_signature(ours, theirs)

    def test_same_names(self):
        for name in ("RUNNING", "COMPLETE", "PRUNED", "FAIL", "WAITING"):
            self.assertEqual(int(getattr(goptuna.trial.TrialState, name)),
                             int(getattr(real_optuna.trial.TrialState, name)))
        for attr in ("best_trial", "best_value", "best_params", "trials", "direction",
                     "optimize", "ask", "tell", "enqueue_trial", "add_trial", "stop"):
            self.assertTrue(hasattr(goptuna.Study, attr), attr)

    def test_same_objective_both_libraries(self):
        def objective(trial):
            x = trial.suggest_float("x", -5, 5)
            n = trial.suggest_int("n", 0, 10)
            c = trial.suggest_categorical("c", ["u", "v"])
            return (x - 1) ** 2 + (n - 3) ** 2 + (c == "u")

        for lib in (goptuna, real_optuna):
            kw = {"sampler": lib.samplers.TPESampler(seed=0)}
            study = lib.create_study(**kw)
            with warnings.catch_warnings():
                warnings.simplefilter("ignore")
                study.optimize(objective, n_trials=200)
            self.assertLess(study.best_value, 0.5, lib.__name__)
            self.assertEqual(study.best_params["n"], 3)
            self.assertEqual(study.best_params["c"], "v")

    def test_optuna_objects_accepted(self):
        D = real_optuna.distributions
        with warnings.catch_warnings():
            warnings.simplefilter("ignore")
            study = goptuna.create_study(sampler=real_optuna.samplers.RandomSampler(seed=1),
                                         direction=real_optuna.study.StudyDirection.MINIMIZE)
        for _ in range(60):
            t = study.ask({"x": D.FloatDistribution(-2, 2)})
            study.tell(t, (t.params["x"] - 0.5) ** 2)
        self.assertLess(study.best_value, 1e-3)


if __name__ == "__main__":
    unittest.main()
