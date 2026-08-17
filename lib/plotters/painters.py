import numpy as np
import matplotlib.pyplot as plt
import os
from file_readers import ReadTrialsFile, ReadProblemFile
from plotters import Plotter2D, Plotter3D
from scipy import interpolate

class Painter:
    """
    Базовый класс рисовальщиков
    """
    def paint_objective_func(self):
        pass

    def paint_constraints(self):
        pass

    def paint_points(self):
        pass

    def paint_optimum(self):
        pass

    def save_image(self):
        pass

class StaticPainter(Painter):
    def __init__(self,
                 parameters_numbers,
                 eps,
                 dim,
                 lb,
                 rb,
                 points,
                 values,
                 sol_point,
                 sol_value,
                 x,
                 z,
                 c,
                 x_nc,
                 z_nc,
                 x_nce,
                 cc,
                 plotter_type,
                 object_function_plotter_type,
                 constraints_plotter_type,
                 levels,
                 grid_obj,
                 grid_c,
                 is_points_at_bottom,
                 is_need_hide_no_feasible_points,
                 is_need_fill_feasible_region
    ):

        self.parameters_numbers = parameters_numbers
        self.eps = eps
        self.dim = dim

        self.lb = [lb[param] for param in parameters_numbers]
        self.rb = [rb[param] for param in parameters_numbers]
        self.points = points
        self.values = values
        self.sol_point = sol_point
        self.sol_value = sol_value
        self.x = [[xi[param] for param in parameters_numbers] for xi in x]
        self.z = z
        self.c = c
        self.x_nc = x_nc
        self.z_nc = z_nc
        self.x_nce = x_nce
        self.cc = cc
        self.plotter_type = plotter_type
        self.object_function_plotter_type = object_function_plotter_type
        self.constraints_plotter_type = constraints_plotter_type
        self.is_points_at_bottom = is_points_at_bottom
        self.is_need_hide_no_feasible_points = is_need_hide_no_feasible_points
        self.hatch = None
        if plotter_type == 'lines layers' and is_need_fill_feasible_region:
            self.hatch = ' '
        self.levels = None
        if plotter_type == 'lines layers':
            self.levels = levels

        self.grid_obj = grid_obj
        self.grid_c = grid_c
        self.section_points = []
        self.section_values = []

        if self.dim > len(parameters_numbers):
            self.section_indexes = list(range(self.dim))
            for number in self.parameters_numbers:
                self.section_indexes.remove(number)

            for point, value in zip(self.points, self.values):
                is_section_of_best_point = True
                for index in self.section_indexes:
                    if abs(point[index] - self.sol_point[index]) > self.eps:
                        is_section_of_best_point = False
                        break
                if is_section_of_best_point:
                    self.section_points.append(point)
                    self.section_values.append(value)

            self.points = self.section_points
            self.values = self.section_values

            section_x_nce = []

            for x in self.x_nce:
                is_section_of_best_point = True
                for index in self.section_indexes:
                    if abs(x[index] - self.sol_point[index]) > self.eps:
                        is_section_of_best_point = False
                        break
                if is_section_of_best_point:
                    section_x_nce.append(x)

            self.x_nce = section_x_nce


        if self.dim == 1 or len(self.parameters_numbers) == 1:
            self.plotter = Plotter2D(self.parameters_numbers[0],
                                     self.lb[0],
                                     self.rb[0])
        else:
            self.plotter = Plotter3D(self.parameters_numbers,
                                     self.lb,
                                     self.rb,
                                     self.object_function_plotter_type,
                                     self.constraints_plotter_type,
                                     self.plotter_type,
                                     self.is_points_at_bottom)

    def paint_objective_func(self):
        #print(self.points, self.values)
        if len(self.parameters_numbers) == 1:
            if self.object_function_plotter_type == 'objective function':
                self.plotter.plot_by_grid(self.x, self.z, transparency=0.9)
            elif self.object_function_plotter_type == 'interpolation':
                self.plotter.plot_interpolation(self.points, self.values, points_count=self.grid_obj, transparency=0.9)
            elif self.object_function_plotter_type == 'approximation':
                self.plotter.plot_approximation(self.points, self.values, points_count=self.grid_obj, transparency=0.9)
            elif self.object_function_plotter_type == 'by points':
                self.plotter.plot_by_points(self.points, self.values, transparency=0.9)
            elif self.object_function_plotter_type == 'only points':
                pass

        else:
            if self.plotter_type == 'lines layers' or self.plotter_type == 'surface':
                if self.object_function_plotter_type == "objective function":
                    self.plotter.plot_by_grid(self.x, self.z, levels=self.levels)
                elif self.object_function_plotter_type == 'interpolation':
                    self.plotter.plot_interpolation(self.points, self.values, points_count=self.grid_obj, levels=self.levels)
                elif self.object_function_plotter_type == 'approximation':
                    self.plotter.plot_approximation(self.points, self.values, points_count=self.grid_obj, levels=self.levels)
                elif self.object_function_plotter_type == 'by points':
                    self.plotter.plot_by_points(self.points, self.values, levels=self.levels)
                elif self.object_function_plotter_type == 'only points':
                    pass

    def paint_constraints(self):
        if not self.hatch:
            if self.plotter_type != 'surface':
                if self.constraints_plotter_type == "objective function":
                    if len(self.parameters_numbers) > 1 and len(self.c) > 0:
                        for i in range(len(self.c[0])):
                            self.plotter.plot_by_grid(self.x, [cj[i] for cj in self.c], colormap='twilight', linewidths=1, levels=0, transparency=0.6)
                elif self.constraints_plotter_type == 'interpolation':
                    if len(self.parameters_numbers) > 1 and len(self.cc[0]) > 0:
                        for i in range(len(self.cc) // 2):
                            self.plotter.plot_interpolation(self.cc[2 * i], self.cc[2 * i + 1], points_count=self.grid_c,
                                                            colormap='twilight', linewidths=1, transparency=0.6, levels=0)
        else:
            if self.constraints_plotter_type == "objective function":
                if len(self.parameters_numbers) > 1 and len(self.c) > 0:
                    x1 = [xi[0] for xi in self.x]
                    x2 = [xi[1] for xi in self.x]
                    self.plotter.plot_hatch_by_grid(x1, x2, self.c)
            elif self.constraints_plotter_type == 'interpolation':
                if len(self.parameters_numbers) > 1 and len(self.cc[0]) > 0:
                    x1 = []
                    x2 = []
                    z = []
                    for i in range(len(self.cc) // 2):
                        x1.append(np.array(self.cc[2 * i])[:, self.parameters_numbers[0]])
                        x2.append(np.array(self.cc[2 * i])[:, self.parameters_numbers[1]])
                        z.append(np.array(self.cc[2 * i + 1]))
                    self.plotter.plot_hatch_by_interpolate(x1, x2, z, points_count=self.grid_c)

    def paint_points(self):
        flag1 = True
        flag2 = True
        color = 'blue'
        mrkr = 'o'

        points = self.points
        values = self.values
        diff = max(values) - min(values)

        while 1:
            if len(self.parameters_numbers) == 1:
                points = [x[self.parameters_numbers[0]] for x in points]

            if self.is_points_at_bottom:
                values = [self.sol_value - diff * 0.3] * len(values)

            self.plotter.plot_points(points, values, color, mrkr, mrkrs=2)

            if self.is_need_hide_no_feasible_points:
                break
            if len(self.x_nc) > 0 and flag1:
                points = self.x_nc
                values = [self.sol_value - diff * 0.3] * len(self.x_nc)
                color = 'slategray'
                mrkr = '.'
                flag1 = False
            elif len(self.x_nce) > 0 and flag2:
                points = self.x_nce
                values = [self.sol_value - diff * 0.3] * len(self.x_nce)
                color = 'black'
                mrkr = '.'
                flag2 = False
            else:
                break


            '''

            '''

    def paint_optimum(self):
        value = self.sol_value
        diff = max(self.values) - min(self.values)
        point = self.sol_point

        if self.is_points_at_bottom:
            value = value - diff * 0.3
        if len(self.parameters_numbers) == 1:
            point = [self.sol_point[self.parameters_numbers[0]]]

        self.plotter.plot_points([point], [value], clr='red', mrkrs=6, mrkr='*')

    def save_image(self, path, filename, is_need_show_figure):
        if not os.path.isdir(path):
            if path == "":
                plt.savefig(filename)
            else:
                os.mkdir(path)
                plt.savefig(path + filename)
        else:
            plt.savefig(path + "/" + filename)

        if is_need_show_figure:
            plt.show()
