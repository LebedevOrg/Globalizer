"""Сравнение: Globalizer (три интерфейса), scipy.optimize и Optuna на тестовых задачах.

Методы
  Globalizer   GLZ-native  - собственный интерфейс PYDGlobalizer.solve (PYProblem);
               GLZ-scipy   - globalizer.optimize.differential_evolution (стиль scipy);
               GLZ-optuna  - globalizer.optuna (стиль Optuna);
  scipy        SciPy-DE, SciPy-DA (dual_annealing), SciPy-DIRECT, SciPy-SHGO;
  Optuna       Optuna-TPE, Optuna-CMA (CmaEsSampler, нужен пакет cmaes), Optuna-Random.

Задачи с известным глобальным минимумом f*: без ограничений (Branin,
Six-hump camel, Rastrigin, Ackley, Rosenbrock, Hartmann 3D/6D, Shekel),
с нелинейными ограничениями (круг, G08) и частично целочисленные.

Как сравниваются методы
  * Каждая задача оборачивается счётчиком вычислений. Вычисление - это одна
    точка, в которой вызвана целевая функция или (для методов, которые
    сначала проверяют ограничения) хотя бы одно ограничение.
  * Метрика - лучшее ДОПУСТИМОЕ значение среди первых B вычислений
    («anytime»-сравнение при равном бюджете). Каждый метод запускается один
    раз с наибольшим бюджетом; результаты для меньших бюджетов - по началу
    той же истории (у всех методов здесь последовательность точек не
    зависит от лимита: у SciPy-DE отключён polish).
  * Ошибка = f_best - f*; задача решена, если ошибка <= tol
    (tol = 1e-3 * max(1, |f*|)).
  * Стохастические методы (DE, DA, TPE, CMA-ES, Random) запускаются с
    несколькими зёрнами: в ячейке медиана ошибки и доля решённых запусков.
    Globalizer, DIRECT и SHGO детерминированы - один запуск.

Запуск
  python examples/compare_optimizers.py                     # бюджеты 100 300 1000, 5 зёрен
  python examples/compare_optimizers.py --quick             # 100 300, 3 зерна
  python examples/compare_optimizers.py --budgets 200 --seeds 3 --problems branin g08
  python examples/compare_optimizers.py --out results_dir   # куда писать report.md и results.csv
Нужны scipy и optuna (CMA-ES - если установлен cmaes).
"""

import argparse
import csv
import importlib.util
import math
import os
import statistics
import sys
import tempfile
import time
import warnings
from pathlib import Path

import numpy as np

ROOT = Path(__file__).resolve().parent.parent
for _d in (ROOT / "PYGlobalizer", ROOT / "_bin"):
    if _d.exists() and str(_d) not in sys.path:
        sys.path.insert(0, str(_d))

import PYDGlobalizer                          # noqa: E402
from PYProblem import PYProblem               # noqa: E402
import globalizer.optimize as gopt            # noqa: E402
import globalizer.optuna as goptuna           # noqa: E402

try:
    import optuna                             # noqa: E402
    import scipy.optimize as sopt             # noqa: E402
except ImportError as _exc:
    print(f"[SKIP] the comparison needs scipy and optuna: {_exc} "
          "(pip install scipy optuna cmaes)")
    sys.exit(77)

optuna.logging.set_verbosity(optuna.logging.ERROR)
goptuna.logging.set_verbosity(goptuna.logging.ERROR)
warnings.filterwarnings("ignore")


# =============================================================================
#  Задачи
# =============================================================================
class Problem:
    def __init__(self, name, title, bounds, f, f_star, constraints=(), integrality=None):
        self.name = name
        self.title = title
        self.bounds = [(float(lo), float(hi)) for lo, hi in bounds]
        self.dim = len(self.bounds)
        self.f = f
        self.f_star = float(f_star)
        self.constraints = list(constraints)          # g(x) <= 0
        self.integrality = list(integrality) if integrality else [False] * self.dim
        self.tol = 1e-3 * max(1.0, abs(self.f_star))

    @property
    def kind(self):
        if self.constraints:
            return "constrained"
        if any(self.integrality):
            return "mixed"
        return "box"


def branin(x):
    b, c, t = 5.1 / (4 * math.pi ** 2), 5 / math.pi, 1 / (8 * math.pi)
    return (x[1] - b * x[0] ** 2 + c * x[0] - 6) ** 2 + 10 * (1 - t) * math.cos(x[0]) + 10


def camel(x):
    a, b = x
    return (4 - 2.1 * a * a + a ** 4 / 3) * a * a + a * b + (-4 + 4 * b * b) * b * b


def make_rastrigin(shift):
    shift = np.asarray(shift, dtype=float)

    def f(x):
        z = np.asarray(x, dtype=float) - shift
        return float(10 * z.size + np.sum(z * z - 10 * np.cos(2 * np.pi * z)))
    return f


def make_ackley(shift):
    shift = np.asarray(shift, dtype=float)

    def f(x):
        z = np.asarray(x, dtype=float) - shift
        n = z.size
        return float(-20 * np.exp(-0.2 * np.sqrt(np.sum(z * z) / n))
                     - np.exp(np.sum(np.cos(2 * np.pi * z)) / n) + 20 + np.e)
    return f


def rosenbrock(x):
    x = np.asarray(x, dtype=float)
    return float(np.sum(100 * (x[1:] - x[:-1] ** 2) ** 2 + (1 - x[:-1]) ** 2))


_H3_A = np.array([[3, 10, 30], [0.1, 10, 35], [3, 10, 30], [0.1, 10, 35]])
_H3_P = 1e-4 * np.array([[3689, 1170, 2673], [4699, 4387, 7470], [1091, 8732, 5547],
                         [381, 5743, 8828]])
_H6_A = np.array([[10, 3, 17, 3.5, 1.7, 8], [0.05, 10, 17, 0.1, 8, 14],
                  [3, 3.5, 1.7, 10, 17, 8], [17, 8, 0.05, 10, 0.1, 14]])
_H6_P = 1e-4 * np.array([[1312, 1696, 5569, 124, 8283, 5886], [2329, 4135, 8307, 3736, 1004, 9991],
                         [2348, 1451, 3522, 2883, 3047, 6650], [4047, 8828, 8732, 5743, 1091, 381]])
_H_ALPHA = np.array([1.0, 1.2, 3.0, 3.2])


def hartmann3(x):
    x = np.asarray(x, dtype=float)
    return float(-np.sum(_H_ALPHA * np.exp(-np.sum(_H3_A * (x - _H3_P) ** 2, axis=1))))


def hartmann6(x):
    x = np.asarray(x, dtype=float)
    return float(-np.sum(_H_ALPHA * np.exp(-np.sum(_H6_A * (x - _H6_P) ** 2, axis=1))))


_SH_C = 0.1 * np.array([1, 2, 2, 4, 4, 6, 3, 7, 5, 5])
_SH_A = np.array([[4, 4, 4, 4], [1, 1, 1, 1], [8, 8, 8, 8], [6, 6, 6, 6], [3, 7, 3, 7],
                  [2, 9, 2, 9], [5, 5, 3, 3], [8, 1, 8, 1], [6, 2, 6, 2], [7, 3.6, 7, 3.6]])


def shekel10(x):
    x = np.asarray(x, dtype=float)
    return float(-np.sum(1.0 / (np.sum((x - _SH_A) ** 2, axis=1) + _SH_C)))


def g08(x):
    x1, x2 = x
    return -(math.sin(2 * math.pi * x1) ** 3 * math.sin(2 * math.pi * x2)) / (x1 ** 3 * (x1 + x2))


def mixed_quadratic(x):
    return (x[0] - 1.3) ** 2 + (x[1] + 2.6) ** 2 + (x[2] - 0.4) ** 2


_mixed_ras = make_rastrigin([0.7, -0.3])


def mixed_rastrigin(x):
    return _mixed_ras(x[:2]) + (x[2] - 2) ** 2 + (x[3] + 1) ** 2


PROBLEMS = [
    Problem("branin", "Branin 2D", [(-5, 10), (0, 15)], branin, 0.397887357729739),
    Problem("camel", "Six-hump camel 2D", [(-3, 3), (-2, 2)], camel, -1.031628453489877),
    Problem("rastrigin2", "Rastrigin 2D (shifted)", [(-5.12, 5.12)] * 2,
            make_rastrigin([0.7, -0.3]), 0.0),
    Problem("ackley3", "Ackley 3D (shifted)", [(-5, 5)] * 3, make_ackley([0.5, -1.2, 0.8]), 0.0),
    Problem("rosenbrock3", "Rosenbrock 3D", [(-2, 2)] * 3, rosenbrock, 0.0),
    Problem("hartmann3", "Hartmann 3D", [(0, 1)] * 3, hartmann3, -3.86278214782076),
    Problem("shekel4", "Shekel 4D (m=10)", [(0, 10)] * 4, shekel10, -10.536409816692),
    Problem("hartmann6", "Hartmann 6D", [(0, 1)] * 6, hartmann6, -3.32236801141551),
    Problem("circle", "Constrained circle 2D", [(-2, 2)] * 2,
            lambda x: (x[0] - 1) ** 2 + (x[1] - 1) ** 2, 3 - 2 * math.sqrt(2),
            constraints=[lambda x: x[0] ** 2 + x[1] ** 2 - 1.0]),
    Problem("g08", "G08 (2 constraints)", [(0.1, 10), (0, 10)], g08, -0.0958250414180359,
            constraints=[lambda x: x[0] ** 2 - x[1] + 1, lambda x: 1 - x[0] + (x[1] - 4) ** 2]),
    Problem("mixed3", "Mixed-integer quadratic 3D", [(-5, 5)] * 3, mixed_quadratic, 0.25,
            integrality=[True, False, True]),
    Problem("mixed_ras4", "Rastrigin 2D + 2 integers", [(-5.12, 5.12)] * 2 + [(-5, 5)] * 2,
            mixed_rastrigin, 0.0, integrality=[False, False, True, True]),
]


# =============================================================================
#  Счётчик вычислений
# =============================================================================
class BudgetExhausted(Exception):
    """Жёсткий лимит вычислений/времени запуска исчерпан."""


class Recorder:
    """История вычислений: значение для допустимых точек, inf - для недопустимых.

    Подряд идущие вызовы в одной точке (ограничения, затем критерий) считаются
    одним вычислением.
    """

    def __init__(self, problem, max_evals=None, time_limit=None):
        self.p = problem
        self.values = []
        self._last_x = None
        self.max_evals = max_evals
        self.deadline = None if time_limit is None else time.perf_counter() + time_limit
        self.cut = False

    def _feasible(self, x):
        return all(g(x) <= 0.0 for g in self.p.constraints)

    def record(self, x, value):
        x = np.array(x, dtype=float)
        if self._last_x is not None and self._last_x.shape == x.shape and \
                np.array_equal(self._last_x, x):
            self.values[-1] = min(self.values[-1], value)
        else:
            # Метод, не соблюдающий свой лимит (например, SHGO с ограничениями),
            # прерывается: результат считается по уже накопленной истории.
            if (self.max_evals is not None and len(self.values) >= self.max_evals) or                     (self.deadline is not None and time.perf_counter() > self.deadline):
                self.cut = True
                raise BudgetExhausted()
            self.values.append(value)
            self._last_x = x

    def objective(self, x):
        x = np.asarray(x, dtype=float)
        f = float(self.p.f(x))
        self.record(x, f if self._feasible(x) and math.isfinite(f) else math.inf)
        return f

    def constraint(self, j):
        def g(x):
            x = np.asarray(x, dtype=float)
            v = float(self.p.constraints[j](x))
            if v > 0.0:
                self.record(x, math.inf)
            return v
        return g


# =============================================================================
#  Методы
# =============================================================================
def _scipy_constraints(problem, rec):
    return [sopt.NonlinearConstraint(rec.constraint(j), -np.inf, 0.0)
            for j in range(len(problem.constraints))]


def run_glz_native(problem, rec, budget, seed):
    cont = [i for i in range(problem.dim) if not problem.integrality[i]]
    disc = [i for i in range(problem.dim) if problem.integrality[i]]

    def to_x(y):
        x = np.empty(problem.dim)
        x[cont] = y[:len(cont)]
        x[disc] = np.rint(y[len(cont):])
        return x

    pr = PYProblem(dimension=len(cont), numCriterions=1)
    pr.set_bounds([problem.bounds[i][0] for i in cont], [problem.bounds[i][1] for i in cont])
    if disc:
        pr.set_discrete_variables(
            [[str(v) for v in range(int(problem.bounds[i][0]), int(problem.bounds[i][1]) + 1)]
             for i in disc], [f"x{i}" for i in disc])
    for j in range(len(problem.constraints)):            # ограничения - первыми
        g = rec.constraint(j)
        pr.add_function(lambda y, g=g: g(to_x(y)))
    pr.add_function(lambda y: rec.objective(to_x(y)))    # критерий - последним

    prm = PYDGlobalizer.SolverParameters()
    prm.max_iterations = budget
    prm.r = 4.0
    prm.epsilon = 0.01
    prm.local_refine = True
    if hasattr(prm, "local_refine_mode"):
        prm.local_refine_mode = "updated"
    prm.verbose = 0
    prm.print_result = False
    prm.log_prefix = os.path.join(tempfile.gettempdir(), "globalizer_compare_log")
    PYDGlobalizer.solve(pr, prm)


def run_glz_scipy(problem, rec, budget, seed):
    gopt.differential_evolution(
        rec.objective, problem.bounds,
        constraints=_scipy_constraints(problem, rec) if problem.constraints else (),
        integrality=problem.integrality if any(problem.integrality) else None,
        globalizer_options={"maxfev": budget})


def _optuna_objective(problem, rec):
    def objective(trial):
        x = np.array([trial.suggest_int(f"x{i}", int(lo), int(hi)) if problem.integrality[i]
                      else trial.suggest_float(f"x{i}", lo, hi)
                      for i, (lo, hi) in enumerate(problem.bounds)], dtype=float)
        for j, g in enumerate(problem.constraints):
            trial.set_constraint(f"g{j}", float(g(x)))
        return rec.objective(x)
    return objective


def run_glz_optuna(problem, rec, budget, seed):
    study = goptuna.create_study()
    study.optimize(_optuna_objective(problem, rec), n_trials=budget)


def run_scipy_de(problem, rec, budget, seed):
    popsize = 15
    maxiter = max(1, math.ceil(budget / (popsize * problem.dim)))
    sopt.differential_evolution(
        rec.objective, problem.bounds, maxiter=maxiter, popsize=popsize, polish=False,
        rng=np.random.default_rng(seed),
        constraints=_scipy_constraints(problem, rec) if problem.constraints else (),
        integrality=problem.integrality if any(problem.integrality) else None)


def run_scipy_da(problem, rec, budget, seed):
    sopt.dual_annealing(rec.objective, problem.bounds, maxfun=budget,
                        rng=np.random.default_rng(seed))


def run_scipy_direct(problem, rec, budget, seed):
    sopt.direct(rec.objective, problem.bounds, maxfun=budget, maxiter=100000)


def run_scipy_shgo(problem, rec, budget, seed):
    cons = [{"type": "ineq", "fun": (lambda x, g=rec.constraint(j): -g(x))}
            for j in range(len(problem.constraints))]
    sopt.shgo(rec.objective, problem.bounds, constraints=cons or None,
              n=min(128, max(16, budget // 8)), iters=100, sampling_method="sobol",
              options={"maxfev": budget})


def _optuna_run(sampler):
    def run(problem, rec, budget, seed):
        study = optuna.create_study(sampler=sampler(seed))
        study.optimize(_optuna_objective(problem, rec), n_trials=budget)
    return run


HAS_CMAES = importlib.util.find_spec("cmaes") is not None

# name, функция, стохастический, допустимые виды задач, описание
METHODS = [
    ("GLZ-native", run_glz_native, False, {"box", "constrained", "mixed"},
     "PYDGlobalizer.solve: r=4, eps=0.01, max_iterations=B, локальный поиск при улучшении минимума"),
    ("GLZ-scipy", run_glz_scipy, False, {"box", "constrained", "mixed"},
     "globalizer.optimize.differential_evolution(maxfev=B), параметры по умолчанию"),
    ("GLZ-optuna", run_glz_optuna, False, {"box", "constrained", "mixed"},
     "globalizer.optuna: create_study() по умолчанию (GlobalizerSampler), n_trials=B"),
    ("SciPy-DE", run_scipy_de, True, {"box", "constrained", "mixed"},
     "scipy differential_evolution: popsize=15, maxiter=B/(15*n), polish=False"),
    ("SciPy-DA", run_scipy_da, True, {"box"},
     "scipy dual_annealing(maxfun=B) - только задачи без ограничений"),
    ("SciPy-DIRECT", run_scipy_direct, False, {"box"},
     "scipy direct(maxfun=B) - только задачи без ограничений"),
    ("SciPy-SHGO", run_scipy_shgo, False, {"box", "constrained"},
     "scipy shgo: sobol, n=min(128, B/8), maxfev=B"),
    ("Optuna-TPE", _optuna_run(lambda s: optuna.samplers.TPESampler(seed=s)), True,
     {"box", "constrained", "mixed"}, "optuna TPESampler(seed), ограничения - trial.set_constraint"),
    ("Optuna-CMA", _optuna_run(lambda s: optuna.samplers.CmaEsSampler(seed=s)), True,
     {"box", "mixed"}, "optuna CmaEsSampler(seed) - без ограничений (нужен пакет cmaes)"),
    ("Optuna-Random", _optuna_run(lambda s: optuna.samplers.RandomSampler(seed=s)), True,
     {"box", "constrained", "mixed"}, "optuna RandomSampler(seed) - базовый уровень"),
]
if not HAS_CMAES:
    METHODS = [m for m in METHODS if m[0] != "Optuna-CMA"]


# =============================================================================
#  Прогон и отчёт
# =============================================================================
def run_all(problems, methods, budgets, n_seeds, progress=True, time_limit=120.0):
    b_max = max(budgets)
    rows = []
    for p in problems:
        for name, func, stochastic, kinds, _ in methods:
            if p.kind not in kinds:
                continue
            for seed in (range(n_seeds) if stochastic else [0]):
                rec = Recorder(p, max_evals=2 * b_max + 100, time_limit=time_limit)
                t0 = time.perf_counter()
                error = ""
                try:
                    func(p, rec, b_max, seed)
                except BudgetExhausted:
                    pass
                except Exception as exc:                 # noqa: BLE001 - фиксируем в отчёте
                    if not rec.cut:
                        error = f"{type(exc).__name__}: {exc}"
                elapsed = time.perf_counter() - t0
                vals = np.array(rec.values[:b_max] if rec.values else [math.inf])
                best_curve = np.minimum.accumulate(vals)
                hit = np.nonzero(best_curve - p.f_star <= p.tol)[0]
                to_target = int(hit[0]) + 1 if hit.size else None
                for b in budgets:
                    best = float(best_curve[min(b, len(best_curve)) - 1])
                    err = best - p.f_star
                    rows.append({
                        "problem": p.name, "method": name, "seed": seed, "budget": b,
                        "best": best, "error": err, "solved": bool(err <= p.tol),
                        "evals_to_target": to_target, "evals_used": len(rec.values),
                        "time_s": elapsed, "error_msg": error})
                if progress:
                    best = float(best_curve[-1])
                    print(f"  {p.name:12s} {name:14s} seed={seed}  f_best={best:.6g}  "
                          f"evals={len(rec.values):5d}  {elapsed:6.2f}s"
                          + ("  [cut by hard limit]" if rec.cut else "")
                          + (f"  [{error}]" if error else ""), flush=True)
    return rows


def _fmt_err(e):
    if not math.isfinite(e):
        return "inf"
    if e <= 0:
        return "0"
    return f"{e:.1e}"


def _cell(cell_rows, stochastic):
    errs = [r["error"] for r in cell_rows]
    solved = sum(r["solved"] for r in cell_rows)
    med = statistics.median(errs)
    if stochastic:
        return f"{_fmt_err(med)} ({solved}/{len(cell_rows)})"
    return f"{_fmt_err(med)} {'✓' if solved else '✗'}"


def make_report(rows, problems, methods, budgets, n_seeds):
    names = [m[0] for m in methods]
    stoch = {m[0]: m[2] for m in methods}
    by = {}
    for r in rows:
        by.setdefault((r["problem"], r["method"], r["budget"]), []).append(r)

    out = ["# Globalizer vs scipy.optimize vs Optuna", ""]
    out.append(f"Бюджеты: {', '.join(map(str, budgets))} вычислений; зёрен у стохастических "
               f"методов: {n_seeds}. Ячейка: ошибка f_best - f* (медиана по зёрнам) и "
               "решено ли (✓/✗ или число решённых запусков); tol = 1e-3·max(1, |f*|). "
               "«—» - метод не применим к задаче.")
    out.append("")
    for b in budgets:
        out.append(f"## Бюджет {b} вычислений")
        out.append("")
        out.append("| Задача | " + " | ".join(names) + " |")
        out.append("|---|" + "---|" * len(names))
        solved_share = {n: [] for n in names}
        ranks = {n: [] for n in names}
        for p in problems:
            cells, meds = [], {}
            for n in names:
                rs = by.get((p.name, n, b))
                if not rs:
                    cells.append("—")
                    continue
                cells.append(_cell(rs, stoch[n]))
                meds[n] = statistics.median(r["error"] for r in rs)
                solved_share[n].append(sum(r["solved"] for r in rs) / len(rs))
            order = sorted(meds, key=lambda n: meds[n])
            for n in meds:                               # ранг с учётом равенств
                ranks[n].append(1 + sum(meds[m] < meds[n] - 1e-12 for m in order))
            out.append(f"| {p.title} | " + " | ".join(cells) + " |")
        out.append("| **Доля решённых** | " + " | ".join(
            f"**{100 * np.mean(solved_share[n]):.0f}%**" if solved_share[n] else "—"
            for n in names) + " |")
        out.append("| **Средний ранг** | " + " | ".join(
            f"{np.mean(ranks[n]):.2f}" if ranks[n] else "—" for n in names) + " |")
        out.append("")

    b_max = max(budgets)
    out.append(f"## Вычислений до достижения tol (бюджет {b_max})")
    out.append("")
    out.append("Медиана по запускам, достигшим tol; в скобках - сколько запусков достигли.")
    out.append("")
    out.append("| Задача | " + " | ".join(names) + " |")
    out.append("|---|" + "---|" * len(names))
    for p in problems:
        cells = []
        for n in names:
            rs = by.get((p.name, n, b_max))
            if not rs:
                cells.append("—")
                continue
            hits = [r["evals_to_target"] for r in rs if r["evals_to_target"] is not None]
            if not hits:
                cells.append("✗")
            else:
                med = int(statistics.median(hits))
                cells.append(f"{med}" + (f" ({len(hits)}/{len(rs)})" if stoch[n] else ""))
        out.append(f"| {p.title} | " + " | ".join(cells) + " |")
    out.append("")

    out.append(f"## Время одного запуска, с (медиана, бюджет {b_max})")
    out.append("")
    out.append("| Задача | " + " | ".join(names) + " |")
    out.append("|---|" + "---|" * len(names))
    for p in problems:
        cells = []
        for n in names:
            rs = by.get((p.name, n, b_max))
            cells.append(f"{statistics.median(r['time_s'] for r in rs):.2f}" if rs else "—")
        out.append(f"| {p.title} | " + " | ".join(cells) + " |")
    out.append("")

    errors = sorted({(r["problem"], r["method"], r["error_msg"]) for r in rows if r["error_msg"]})
    if errors:
        out.append("## Ошибки запусков")
        out.append("")
        for prob, meth, msg in errors:
            out.append(f"- {prob} / {meth}: {msg}")
        out.append("")

    out.append("## Задачи")
    out.append("")
    out.append("| Задача | Размерность | Вид | f* |")
    out.append("|---|---|---|---|")
    for p in problems:
        out.append(f"| {p.title} | {p.dim} | {p.kind} | {p.f_star:.6g} |")
    out.append("")
    out.append("## Настройки методов")
    out.append("")
    for name, _, stochastic, kinds, desc in methods:
        out.append(f"- **{name}**{' (стохастический)' if stochastic else ''}: {desc}.")
    out.append("")
    cut = sorted({(r["problem"], r["method"]) for r in rows if r["evals_used"] > 2 * b_max})
    out.append("## Как читать результаты")
    out.append("")
    out.append("- Все методы получают одну и ту же функцию со счётчиком; учитываются первые B "
               "вычислений одного запуска. Метод, который тратит вычисления в конце "
               "(финальное локальное уточнение), при бюджете B его не получает: так у "
               "GLZ-scipy (локальный поиск после глобального, `maxfev` ограничивает только "
               "глобальную фазу) и поэтому у SciPy-DE отключён `polish`.")
    out.append("- Точка, в которой метод проверил ограничение и не стал вычислять критерий, "
               "тоже считается вычислением (так считают и Globalizer, и SHGO/DE).")
    out.append("- Методы, не соблюдающие свой лимит, прерываются после 2·B+100 вычислений "
               "или 120 с; результат считается по накопленной истории"
               + (": " + ", ".join(f"{p}/{m}" for p, m in cut) if cut else "") + ".")
    out.append("")
    return "\n".join(out)


def main(argv=None):
    ap = argparse.ArgumentParser(description=__doc__.split("\n")[0])
    ap.add_argument("--budgets", type=int, nargs="+", default=None)
    ap.add_argument("--seeds", type=int, default=None)
    ap.add_argument("--problems", nargs="+", default=None,
                    help="имена задач: " + ", ".join(p.name for p in PROBLEMS))
    ap.add_argument("--methods", nargs="+", default=None,
                    help="имена методов: " + ", ".join(m[0] for m in METHODS))
    ap.add_argument("--quick", action="store_true", help="бюджеты 100 300, 3 зерна")
    ap.add_argument("--out", default="comparison_results",
                    help="папка для report.md и results.csv (по умолчанию ./comparison_results)")
    ap.add_argument("--quiet", action="store_true")
    args = ap.parse_args(argv)

    budgets = sorted(args.budgets or ([100, 300] if args.quick else [100, 300, 1000]))
    n_seeds = args.seeds or (3 if args.quick else 5)
    problems = [p for p in PROBLEMS if not args.problems or p.name in args.problems]
    methods = [m for m in METHODS if not args.methods or m[0] in args.methods]
    if not problems or not methods:
        ap.error("no problems or methods selected")
    if not HAS_CMAES:
        print("[info] cmaes is not installed - Optuna-CMA is skipped (pip install cmaes)")

    t0 = time.time()
    rows = run_all(problems, methods, budgets, n_seeds, progress=not args.quiet)
    report = make_report(rows, problems, methods, budgets, n_seeds)

    out_dir = Path(args.out)
    out_dir.mkdir(parents=True, exist_ok=True)
    (out_dir / "report.md").write_text(report, encoding="utf-8")
    with open(out_dir / "results.csv", "w", newline="", encoding="utf-8") as fh:
        writer = csv.DictWriter(fh, fieldnames=list(rows[0]))
        writer.writeheader()
        writer.writerows(rows)

    try:
        sys.stdout.reconfigure(encoding="utf-8", errors="replace")
    except AttributeError:
        pass
    print()
    print(report)
    print(f"Total time {time.time() - t0:.0f} s. Report: {(out_dir / 'report.md').resolve()}")
    print(f"Raw results: {(out_dir / 'results.csv').resolve()}")


if __name__ == "__main__":
    main()
