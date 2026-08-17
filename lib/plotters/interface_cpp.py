from ctypes import *
import sys, os
sys.path.append(os.path.dirname(sys.argv[0]))

from painters import StaticPainter
from file_readers import ReadTrialsFile, ReadProblemFile

class DrawingProcess:
    def __init__(self,  build_path, trials_file_name, problem_file_name, eps):
        self.path = build_path
        self.eps = eps
        self.dim, self.lb, self.rb, self.x, self.z, self.c = ReadProblemFile(build_path, problem_file_name)
        self.points, self.values, self.sol_point, self.sol_value, self.x_nc, self.z_nc, self.cc, self.x_nce = ReadTrialsFile(build_path, trials_file_name)

    def draw_plot(self,
                  plotter_type=None,
                  object_function_plotter_type="objective function",
                  constraints_plotter_type="objective function",
                  parameters_numbers=[0],
                  is_points_at_bottom=True,
                  output_file="output.png",
                  levels=25,
                  grid_obj=100,
                  grid_c=200,
                  is_need_show_figure=False,
                  is_need_hide_trials_points=False,
                  is_need_hide_no_feasible_points=False,
                  is_need_fill_feasible_region=False):
        '''
        plotter_type: 'lines layers' 'surface'
        object_function_plotter_type: 'objective function' 'approximation' 'interpolation' 'by points' 'only points'
        Comments:
        'lines layers' + 'approximation', 'lines layers' + 'by points'- не желательно
        'surface' + 'objective function' - визуализирует в объеме линии уровня вместо поверхности
        '''
        painter = StaticPainter(parameters_numbers,
                                self.eps,
                                self.dim,
                                self.lb,
                                self.rb,
                                self.points,
                                self.values,
                                self.sol_point,
                                self.sol_value,
                                self.x,
                                self.z,
                                self.c,
                                self.x_nc,
                                self.z_nc,
                                self.x_nce,
                                self.cc,
                                plotter_type,
                                object_function_plotter_type,
                                constraints_plotter_type,
                                levels,
                                grid_obj,
                                grid_c,
                                is_points_at_bottom,
                                is_need_hide_no_feasible_points,
                                is_need_fill_feasible_region)
        painter.paint_objective_func()
        painter.paint_constraints()
        if not is_need_hide_trials_points:
            painter.paint_points()
        painter.paint_optimum()
        painter.save_image(self.path, output_file, is_need_show_figure)
