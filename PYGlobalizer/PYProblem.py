
from typing import List, Callable, Union, Optional
import numpy as np
import numpy.typing as npt


class PYProblem:
    """
    Класс для определения задачи глобальной оптимизации для Globalizer.
    
    Attributes
    ----------
    _dimension : Optional[int]
        Размерность задачи (число переменных)
    _num_crit : int
        Число критериев (целевых функций)
    _functions : List[Callable]
        Список функций (критерии + ограничения)
    _lower_bounds : List[float]
        Нижние границы области поиска
    _upper_bounds : List[float]
        Верхние границы области поиска
    number_of_discrete_variables : int
        Число дискретных переменных
    discrete_variable_values : List[List[str]]
        Допустимые значения дискретных переменных
    discrete_variable_names : List[str]
        Имена дискретных переменных
    
    Examples
    --------
    >>> # Простая задача: минимизация сферы
    >>> problem = PYProblem(dimension=2, numCriterions=1)
    >>> problem.set_bounds([-5, -5], [5, 5])
    >>> problem.add_function(lambda x: x[0]**2 + x[1]**2)
    >>> 
    >>> # Решение
    >>> import PYGlobalizer
    >>> result = PYGlobalizer.solve(problem)
    >>> print(result["best_value"])
    """
    
    def __init__(
        self, 
        dimension: Optional[int] = None, 
        numCriterions: Optional[int] = 1
    ) -> None:
        """
        Инициализация задачи оптимизации.
        
        Parameters
        ----------
        dimension : Optional[int], default=None
            Размерность задачи. Если не задана, будет определена из границ.
        numCriterions : Optional[int], default=1
            Число целевых функций (критериев оптимизации).
        
        Raises
        ------
        ValueError
            Если dimension или numCriterions отрицательны.
        """
        if dimension is not None and dimension <= 0:
            raise ValueError(f"dimension must be positive, got {dimension}")
        if numCriterions <= 0:
            raise ValueError(f"numCriterions must be positive, got {numCriterions}")
        
        self._functions: List[Callable[[List[float]], float]] = []
        self._lower_bounds: List[float] = []
        self._upper_bounds: List[float] = []
        self._dimension: Optional[int] = dimension
        self._num_crit: int = numCriterions
        
        # Дискретные переменные
        self.number_of_discrete_variables: int = 0
        self.discrete_variable_values: List[List[str]] = []
        self.discrete_variable_names: List[str] = []
        
        # Оптимум (опционально)
        self._isSetOptimum: bool = False
        self._optimumValue: float = 0.0
        self._optimumPoint: list = []

    def copy_from_problem(self, problem) -> None:
        """
        Копирует данные из объекта problem (например, из iOpt.problem.Problem).
        
        Parameters
        ----------
        problem : object
            Объект задачи с методом calculate и атрибутами dimension, bounds.
        
        Raises
        ------
        ValueError
            Если problem не имеет необходимых атрибутов.
        TypeError
            Если типы данных некорректны.
        

        """
        print("[PYProblem] copy_from_problem: начало")
        
        # === ВАЛИДАЦИЯ ВХОДНОГО ОБЪЕКТА ===
        if not hasattr(problem, 'calculate'):
            raise ValueError("Problem object must have 'calculate' method")
        
        # === ДИСКРЕТНЫЕ ПЕРЕМЕННЫЕ ===
        self.discrete_variable_values = []
        self.discrete_variable_names = []
        self.number_of_discrete_variables = 0
        
        if hasattr(problem, "discrete_variable_values"):
            if not isinstance(problem.discrete_variable_values, list):
                raise TypeError("discrete_variable_values must be a list")
            self.discrete_variable_values = problem.discrete_variable_values.copy()
            
            # Валидация структуры
            for i, vals in enumerate(self.discrete_variable_values):
                if not isinstance(vals, list):
                    raise TypeError(
                        f"discrete_variable_values[{i}] must be a list, got {type(vals)}")
        
        if hasattr(problem, "discrete_variable_names"):
            if not isinstance(problem.discrete_variable_names, list):
                raise TypeError("discrete_variable_names must be a list")
            self.discrete_variable_names = problem.discrete_variable_names.copy()
        
        self.number_of_discrete_variables = len(self.discrete_variable_names)
        print(f"[PYProblem] Дискретных переменных: {self.number_of_discrete_variables}")
        
        # === РАЗМЕРНОСТЬ ===
        if hasattr(problem, 'dimension'):
            if not isinstance(problem.dimension, int) or problem.dimension <= 0:
                raise ValueError(
                    f"problem.dimension must be positive integer, got {problem.dimension}")
            n_continuous = problem.dimension
        else:
            n_continuous = 2
            print("[PYProblem] WARNING: dimension not found, using default=2")
        
        self._dimension = n_continuous + self.number_of_discrete_variables
        print(f"[PYProblem] Общая размерность: {self._dimension} "
              f"(непрерывных: {n_continuous}, дискретных: {self.number_of_discrete_variables})")
        
        # === ГРАНИЦЫ ПОИСКА ===
        lower_bounds = None
        upper_bounds = None
        
        # Пробуем разные варианты атрибутов
        for attr_name in ['lower_bounds', '_lower_bounds', 'lower_bound_of_float_variables']:
            if hasattr(problem, attr_name):
                lower_bounds = getattr(problem, attr_name)
                break
        
        for attr_name in ['upper_bounds', '_upper_bounds', 'upper_bound_of_float_variables']:
            if hasattr(problem, attr_name):
                upper_bounds = getattr(problem, attr_name)
                break
        
        if lower_bounds is not None and upper_bounds is not None:
            # Преобразуем в списки, если нужно
            if isinstance(lower_bounds, np.ndarray):
                lower_bounds = lower_bounds.tolist()
            if isinstance(upper_bounds, np.ndarray):
                upper_bounds = upper_bounds.tolist()
            
            # Берём только непрерывные переменные
            lower_bounds = lower_bounds[:n_continuous]
            upper_bounds = upper_bounds[:n_continuous]
            
            # Валидация
            if len(lower_bounds) != len(upper_bounds):
                raise ValueError(
                    f"Bounds size mismatch: lower={len(lower_bounds)}, upper={len(upper_bounds)}")
            
            for i, (l, u) in enumerate(zip(lower_bounds, upper_bounds)):
                if not isinstance(l, (int, float)) or not isinstance(u, (int, float)):
                    raise TypeError(f"Bounds at index {i} must be numeric")
                if l >= u:
                    raise ValueError(
                        f"Invalid bounds at index {i}: lower={l} >= upper={u}")
            
            self._lower_bounds = [float(x) for x in lower_bounds]
            self._upper_bounds = [float(x) for x in upper_bounds]
            print(f"[PYProblem] Границы: [{self._lower_bounds[0]:.2f}, {self._upper_bounds[0]:.2f}] x {len(self._lower_bounds)}")
        else:
            # Границы по умолчанию
            self._lower_bounds = [-10.0] * self._dimension
            self._upper_bounds = [10.0] * self._dimension
            print(f"[PYProblem] WARNING: Using default bounds [-10, 10]^{self._dimension}")
        
        # === СОЗДАНИЕ ОБЁРТКИ ДЛЯ ФУНКЦИИ ===
        # КРИТИЧЕСКАЯ ОПТИМИЗАЦИЯ: создаём объекты ОДИН РАЗ, переиспользуем
        class Point:
            __slots__ = ('float_variables', 'discrete_variables')
            def __init__(self):
                self.float_variables: List[float] = []
                self.discrete_variables: List[str] = []
        
        class FunctionValue:
            __slots__ = ('value',)
            def __init__(self):
                self.value: float = 0.0
        
        # Создаём ОДИН экземпляр для переиспользования
        reusable_point = Point()
        reusable_fval = FunctionValue()
        
        def wrapped_calculate(x: List[float]) -> float:
            """
            Оптимизированная обёртка для вызова problem.calculate
            Переиспользует объекты Point и FunctionValue.
            """
            if self.number_of_discrete_variables > 0:
                # Разделяем непрерывные и дискретные переменные
                n_cont = len(x) - self.number_of_discrete_variables
                float_vars = x[:n_cont]
                discrete_indices = x[n_cont:]
                
                # Преобразуем индексы в строковые значения
                discrete_str: List[str] = []
                for i, idx in enumerate(discrete_indices):
                    idx_int = int(round(idx))
                    
                    if i < len(self.discrete_variable_values):
                        max_idx = len(self.discrete_variable_values[i]) - 1
                        idx_int = max(0, min(idx_int, max_idx))
                        discrete_str.append(self.discrete_variable_values[i][idx_int])
                    else:
                        discrete_str.append(str(idx_int))
            else:
                float_vars = list(x)
                discrete_str = []
            
            # Переиспользуем объекты (НЕ создаём новые!)
            reusable_point.float_variables = float_vars
            reusable_point.discrete_variables = discrete_str
            reusable_fval.value = 0.0
            
            # Вызываем метод calculate
            try:
                problem.calculate(reusable_point, reusable_fval)
                return reusable_fval.value
            except Exception as e:
                print(f"[ERROR] Exception in wrapped_calculate: {e}")
                raise
        
        # Добавляем обёрнутую функцию
        self.add_function(wrapped_calculate, name="calculate_copy")
        print("[PYProblem] copy_from_problem: завершено успешно")

    def add_function(
        self, 
        func: Callable[[List[float]], float], 
        name: Optional[str] = None
    ) -> None:
        """
        Добавление одной целевой функции или ограничения.
        
        Parameters
        ----------
        func : Callable[[List[float]], float]
            Функция, принимающая список из dimension чисел и возвращающая float.
        name : Optional[str], default=None
            Опциональное имя функции (для отладки).
        
        Examples
        --------
        >>> problem = PYProblem(dimension=2)
        >>> problem.add_function(lambda x: x[0]**2 + x[1]**2, name="sphere")
        """
        if not callable(func):
            raise TypeError(f"func must be callable, got {type(func)}")
        self._functions.append(func)

    def add_functions(self, functions: List[Callable[[List[float]], float]]) -> None:
        """
        Добавление нескольких функций одновременно.
        
        Parameters
        ----------
        functions : List[Callable[[List[float]], float]]
            Список функций.
        """
        if not isinstance(functions, list):
            raise TypeError(f"functions must be a list, got {type(functions)}")
        for func in functions:
            self.add_function(func)
            

    def set_optimum(
        self,
        value: float,
        point: Optional[Union[List[float], "npt.NDArray[np.float64]"]] = None,
    ) -> None:
        """
        Задать известный глобальный оптимум задачи (опционально).

        Позволяет решателю оценивать точность и корректно работать критерию
        остановки по достижению оптимума.

        Parameters
        ----------
        value : float
            Значение функции в глобальном минимуме.
        point : list[float] | np.ndarray | None
            Координаты минимума. Если None — сохраняется только значение.

        Raises
        ------
        ValueError
            Если длина point не совпадает с размерностью задачи.

        Examples
        --------
        >>> problem = PYProblem(dimension=2)
        >>> problem.set_bounds([-5, -5], [5, 5])
        >>> problem.add_function(lambda x: x[0]**2 + x[1]**2)
        >>> problem.set_optimum(0.0, [0.0, 0.0])
        """
        self._isSetOptimum = True
        self._optimumValue = float(value)

        if point is not None:
            if isinstance(point, np.ndarray):
                point = point.tolist()
            if self._dimension is not None and len(point) != self._dimension:
                raise ValueError(
                    f"optimum point size ({len(point)}) != dimension ({self._dimension})"
                )
            self._optimumPoint = [float(x) for x in point]
        else:
            self._optimumPoint = []


    def set_bounds(
        self, 
        lower: Union[List[float], npt.NDArray[np.float64]], 
        upper: Union[List[float], npt.NDArray[np.float64]]
    ) -> None:
        """
        Установка границ области поиска.
        
        Parameters
        ----------
        lower : List[float] or np.ndarray
            Нижние границы для каждой переменной.
        upper : List[float] or np.ndarray
            Верхние границы для каждой переменной.
        
        Raises
        ------
        ValueError
            Если размеры не совпадают или границы некорректны.
        
        Examples
        --------
        >>> problem = PYProblem(dimension=3)
        >>> problem.set_bounds([-5, -5, -5], [5, 5, 5])
        """
        # Преобразуем в списки
        lower_list = list(lower) if isinstance(lower, np.ndarray) else lower
        upper_list = list(upper) if isinstance(upper, np.ndarray) else upper
        
        if len(lower_list) != len(upper_list):
            raise ValueError(
                f"Bounds size mismatch: lower has {len(lower_list)} elements, "
                f"upper has {len(upper_list)} elements")
        
        # Валидация поэлементно
        for i, (l, u) in enumerate(zip(lower_list, upper_list)):
            if not isinstance(l, (int, float)) or not isinstance(u, (int, float)):
                raise TypeError(f"Bounds at index {i} must be numeric")
            if l >= u:
                raise ValueError(
                    f"Invalid bounds at index {i}: lower={l} >= upper={u}")
        
        self._lower_bounds = [float(x) for x in lower_list]
        self._upper_bounds = [float(x) for x in upper_list]
        
        if self._dimension is None:
            self._dimension = len(lower_list)
        elif self._dimension != len(lower_list):
            raise ValueError(
                f"Dimension mismatch: expected {self._dimension}, "
                f"got {len(lower_list)} bounds")

    def set_lower_bounds(self, lower: Union[List[float], npt.NDArray[np.float64]]) -> None:
        """Установка только нижних границ."""
        lower_list = list(lower) if isinstance(lower, np.ndarray) else lower
        
        if self._upper_bounds and len(lower_list) != len(self._upper_bounds):
            raise ValueError(
                f"Lower bounds size ({len(lower_list)}) != "
                f"upper bounds size ({len(self._upper_bounds)})")
        
        self._lower_bounds = [float(x) for x in lower_list]
        if self._dimension is None:
            self._dimension = len(lower_list)

    def set_upper_bounds(self, upper: Union[List[float], npt.NDArray[np.float64]]) -> None:
        """Установка только верхних границ."""
        upper_list = list(upper) if isinstance(upper, np.ndarray) else upper
        
        if self._lower_bounds and len(upper_list) != len(self._lower_bounds):
            raise ValueError(
                f"Upper bounds size ({len(upper_list)}) != "
                f"lower bounds size ({len(self._lower_bounds)})")
        
        self._upper_bounds = [float(x) for x in upper_list]
        if self._dimension is None:
            self._dimension = len(upper_list)
    
    def validate(self) -> dict:
        """
        Проверка корректности задачи перед решением.
        
        Returns
        -------
        dict
            Словарь с полями:
            - "valid" (bool): True если задача корректна
            - "errors" (List[str]): Список найденных ошибок
        
        Examples
        --------
        >>> problem = PYProblem(dimension=2)
        >>> problem.set_bounds([-5, -5], [5, 5])
        >>> problem.add_function(lambda x: x[0]**2 + x[1]**2)
        >>> validation = problem.validate()
        >>> if validation["valid"]:
        >>>     print("Problem is ready to solve")
        >>> else:
        >>>     print("Errors:", validation["errors"])
        """
        errors = []
        
        if self._dimension is None or self._dimension <= 0:
            errors.append("Dimension is not set or invalid")
        
        if len(self._functions) == 0:
            errors.append("No functions added")
        
        if len(self._lower_bounds) == 0 or len(self._upper_bounds) == 0:
            errors.append("Bounds are not set")
        
        if self._dimension and len(self._lower_bounds) != self._dimension:
            errors.append(
                f"Lower bounds size ({len(self._lower_bounds)}) != dimension ({self._dimension})")
        
        if self._dimension and len(self._upper_bounds) != self._dimension:
            errors.append(
                f"Upper bounds size ({len(self._upper_bounds)}) != dimension ({self._dimension})")
        
        return {
            "valid": len(errors) == 0,
            "errors": errors
        }
    
    def __repr__(self) -> str:
        """Строковое представление объекта."""
        return (f"PYProblem(dimension={self._dimension}, "
                f"functions={len(self._functions)}, "
                f"discrete_vars={self.number_of_discrete_variables})")
