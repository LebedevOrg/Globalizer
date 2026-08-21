from typing import List

class GlobalizerProblem:

    def __init__(self):
        self.dimension = 3

        self.current_coordinate = List[float]
        self.result_value = 0.0


    def calculate(self, coordinate: List[float], discreteCoordinate: List[str], fNumber: int = 0) -> float:
        
        x1 = coordinate[0] #Плотность наполнения
        y1 = discreteCoordinate[0] #  материалов матрицы
        y2 = discreteCoordinate[1] #  материалов наполнителей
        zy1 = 0
        if y1 == 'Mindex1':
            zy1 = 1
        else:
            zy1 = 2

        zy2 = 0
        if y2 == 'Nindex1':
            zy2 = 20
        else:
            zy2 = 10

        self.result_value = x1 + zy1 + zy2

        return self.result_value

    def calculate_all_functionals(self, coordinate: List[float], discreteCoordinate: List[str]) -> List[float]:
        return [0]

    def get_dimension(self) -> int:
        return self.dimension

    def get_number_of_functions(self) -> int:
        return self.get_number_of_criterions() + self.get_number_of_constraints()

    def get_number_of_constraints(self) -> int:
        return 0

    def get_number_of_criterions(self) -> int:
        return 1

    def get_start_y(self) -> List[float]:
        #start_y = [(lb + ub) * 0.5 for lb, ub in
        #           zip(self.problem.lower_bound_of_float_variables, self.problem.upper_bound_of_float_variables)]
        return [0.5]

    def get_start_value(self) -> List[float]:
        discrete_params = self.get_discrete_params()
        u = [i[0] for i in discrete_params]
        return [self.calculate(self.get_start_y(), u ,0)]

    def get_lower_bounds(self) -> List[float]:
        res = [0]
        return res

    def get_upper_bounds(self) -> List[float]:
        res = [1]
        return res

    def get_discrete_params(self) -> List[List[str]]:
        res = [['Mindex1', 'Mindex2'],['Nindex2', 'Nindex2']]
        return res

    def set_dimension(self, dimension: int):
        pass


def get_problem_parameters_names(class_name: str)->List[str]:
    if class_name == 'Rastrigin':
        return ["Dimension"]
    if class_name == 'ECGClassificationProblem':
        return ["Dimension", "ProcRank"]
    if class_name == 'TestsProblem':
        return ["DataSet", "Method"]
    return []



def test_rastrigin():

    problem = GlobalizerProblem()
    bound = problem.get_lower_bounds()
    print(bound)
    result = problem.calculate([0.5], ['Mindex1', 'Nindex2'],0)
    sp = problem.get_start_value()
    print(result)


if __name__ == "__main__":
    #TestSVC3D()
    #TestsProblemTest()
    #test_ecg_classification_problem()
    #test_svc1d_problem()
    #test_segmentation_problem()
    test_rastrigin()
