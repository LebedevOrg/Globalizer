from typing import List, Callable, Union, Optional
import numpy as np

"""
from iOpt.output_system.listeners.static_painters import StaticPainterNDListener
from iOpt.output_system.listeners.animate_painters import AnimatePainterNDListener
from iOpt.output_system.listeners.console_outputers import ConsoleOutputListener
"""

#from examples.Machine_learning.SVC._2D.Problems import SVC_2d


class PYProblem:
    """
    Инициализация объекта-задачи
    """

    def __init__(self, dimension: Optional[int] = None, numCriterions: Optional[int] = 1):
        self._functions: List[Callable] = []
        self._lower_bounds: List[float] = []
        self._upper_bounds: List[float] = []
        self._dimension: Optional[int] = dimension
        self._num_crit: Optional[int] = numCriterions

        self.number_of_discrete_variables: Optional[int]
        self.discrete_variable_values: list[list[str]] = []
        self.discrete_variable_names: list[str] = []

    def copy_from_problem(self, problem) -> None:
        """
        Копирует данные из объекта problem в текущий экземпляр PYProblem
    
        Args:
        problem: объект, у которого есть метод calculate и другие атрибуты
        """
        print("copy_from_problem start")

        # Дискретные параметры
        self.discrete_variable_values = []
        self.discrete_variable_names = []
        self.number_of_discrete_variables = 0

        if hasattr(problem, "discrete_variable_values"):
            self.discrete_variable_values = problem.discrete_variable_values
            #print(f"DEBUG: discrete_variable_values: {self.discrete_variable_values}")

        if hasattr(problem, "discrete_variable_names"):
            self.discrete_variable_names = problem.discrete_variable_names
            #print(f"DEBUG: discrete_variable_names: {self.discrete_variable_names}")

        self.number_of_discrete_variables = len(self.discrete_variable_names) if self.discrete_variable_names else 0
        #print(f"DEBUG: number_of_discrete_variables = {self.number_of_discrete_variables}")

        # Непрерывные параметры
        # Получаем количество непрерывных переменных
        if self.number_of_discrete_variables > 0:
            if hasattr(problem, 'dimension'):
                n_continuous = problem.dimension
            else:
                n_continuous = 2

            #print(f"DEBUG: n_continuous = {n_continuous}")

            # Общая размерность для решателя
            self._dimension = n_continuous + self.number_of_discrete_variables
            #print(f"DEBUG: total dimension for solver = {self._dimension}")
        else:
            if hasattr(problem, "dimension"):
                self._dimension = problem.dimension
            else:
                self._dimension = 2

        # Границы поиска для непрерывных параметров
        lower_bounds = None
        upper_bounds = None

        if hasattr(problem, 'lower_bounds'):
            lower_bounds = problem.lower_bounds
        elif hasattr(problem, '_lower_bounds'):
            lower_bounds = problem._lower_bounds

        if hasattr(problem, 'upper_bounds'):
            upper_bounds = problem.upper_bounds
        elif hasattr(problem, '_upper_bounds'):
            upper_bounds = problem._upper_bounds

        if lower_bounds and upper_bounds:
            # Обрезаем до нужного количества
            lower_bounds = lower_bounds[:n_continuous]
            upper_bounds = upper_bounds[:n_continuous]

            self._lower_bounds = lower_bounds.copy()
            self._upper_bounds = upper_bounds.copy()
            #print(f"DEBUG: bounds set: {self._lower_bounds}, {self._upper_bounds}")
        else:
            # Дефолтные границы
            self._lower_bounds = [-10.0] * self._dimension
            self._upper_bounds = [10.0] * self._dimension
            #print(f"DEBUG: using default bounds: {self._lower_bounds}, {self._upper_bounds}")

        def wrapped_calculate(x):
            """
            Обёртка для вызова метода calculate
            x - массив чисел от решателя
            """
            if self.number_of_discrete_variables > 0:
                # Разделяем непрерывные и дискретные переменные
                n_cont = len(x) - self.number_of_discrete_variables
                #print(f"DEBUG: n_cont = {n_cont}, n_disc = {self.number_of_discrete_variables}, len(x) = {len(x)}")

                float_vars = x[:n_cont]
                discrete_indices = x[n_cont:]

                # Преобразуем индексы в строковые значения
                discrete_str = []
                for i, idx in enumerate(discrete_indices):
                    idx_int = int(round(idx))

                    if self.discrete_variable_values and i < len(self.discrete_variable_values):
                        max_idx = len(self.discrete_variable_values[i]) - 1
                        idx_int = max(0, min(idx_int, max_idx))
                        discrete_str.append(self.discrete_variable_values[i][idx_int])
                    else:
                        discrete_str.append(str(idx_int))
            else:
                float_vars = list(x)
                discrete_str = []

            # Создаём объект point
            point = type('Point', (), {
                'float_variables': float_vars,
                'discrete_variables': discrete_str
            })()

            # Создаём function_value
            function_value = type('FunctionValue', (), {'value': 0.0})()

            # Вызываем calculate
            result = problem.calculate(point, function_value)

            # Возвращаем значение
            if hasattr(function_value, 'value'):
                return function_value.value
            return float(result)

        self.add_function(wrapped_calculate, name="calculate_copy")

        #self.add_function(wrapped_calculate, name="calculate_copy")

    def add_function(self, func: Callable, name: Optional[str] = None) -> None:
        """
        Добавление одной целевой функции

        Args:
            func: функция, принимающая dimension аргументов (или один аргумент-массив)
            name: название функции (опционально)
        """
        self._functions.append(func)

    def add_functions(self, functions: List[Callable]) -> None:
        """
        Добавление нескольких целевых функций
        
        Args:
            functions: список функций
        """
        for func in functions:
            self.add_function(func)

    def set_bounds(self, lower: List[float], upper: List[float]) -> None:
        """
        Установка границ для переменных
        
        Args:
            lower: список нижних границ
            upper: список верхних границ
            
        Raises:
            ValueError: если длины списков не совпадают
        """
        if len(lower) != len(upper):
            raise ValueError(f"Длины lower ({len(lower)}) и upper ({len(upper)}) не совпадают")

        if self._dimension is None:
            self._dimension = len(lower)
        elif self._dimension != len(lower):
            raise ValueError(f"Размерность задачи {self._dimension} не совпадает с длиной границ {len(lower)}")

        self._lower_bounds = lower.copy()
        self._upper_bounds = upper.copy()

    def set_lower_bounds(self, lower: List[float]) -> None:
        """Установка только нижних границ"""
        if self._upper_bounds and len(lower) != len(self._upper_bounds):
            raise ValueError(f"Длина lower ({len(lower)}) не совпадает с upper ({len(self._upper_bounds)})")
        self._lower_bounds = lower.copy()
        if self._dimension is None:
            self._dimension = len(lower)

    def set_upper_bounds(self, upper: List[float]) -> None:
        """Установка только верхних границ"""
        if self._lower_bounds and len(upper) != len(self._lower_bounds):
            raise ValueError(f"Длина upper ({len(upper)}) не совпадает с lower ({len(self._lower_bounds)})")
        self._upper_bounds = upper.copy()
        if self._dimension is None:
            self._dimension = len(upper)
