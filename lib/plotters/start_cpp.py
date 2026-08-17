import warnings
warnings.filterwarnings('ignore', category=UserWarning)

import sys, os

sys.path.append(os.path.dirname(sys.argv[0]))

from interface_cpp import DrawingProcess

def read_args():
    build_path = str(sys.argv[1])
    trials_file_name = str(sys.argv[2])
    problem_file_name = str(sys.argv[3])
    eps = float(sys.argv[4])
    levels = int(sys.argv[5])
    objective_grid_size = int(sys.argv[6])
    constraints_grid_size = int(sys.argv[7])
    plot_type = str(sys.argv[8])
    obj_func_type = str(sys.argv[9])
    constraints_type = str(sys.argv[10])
    str_parameters = str(sys.argv[11])
    str_parameters = str_parameters.replace("[", '')
    str_parameters = str_parameters.replace("]", '')
    params = list(map(int, str_parameters.split(",")))

    displacement_of_points = False
    if sys.argv[12].lower() == "true":
        displacement_of_points = True

    output_file_name = str(sys.argv[13])

    figure_show = False
    if sys.argv[14].lower() == "true":
        figure_show = True

    hide_trials_points = False
    if sys.argv[15].lower() == "true":
        hide_trials_points = True

    hide_no_feasible_points = False
    if sys.argv[16].lower() == "true":
        hide_no_feasible_points = True

    fill_feasible_region = False
    if sys.argv[17].lower() == "true":
        fill_feasible_region = True

    return build_path, trials_file_name, problem_file_name, eps, levels, objective_grid_size, constraints_grid_size, plot_type, obj_func_type, constraints_type, params, displacement_of_points, output_file_name, figure_show, hide_trials_points, hide_no_feasible_points, fill_feasible_region

if __name__ == "__main__":
    print("Start ", str(sys.argv[0]), "...")
    
    build_path, trials_file_name, problem_file_name, eps, levels, objective_grid_size, constraints_grid_size, plot_type, obj_func_type, constraints_type, params, displacement_of_points, output_file_name, figure_show, hide_trials_points, hide_no_feasible_points, fill_feasible_region = read_args()

    dp = DrawingProcess(build_path, trials_file_name, problem_file_name, eps)

    dp.draw_plot(output_file=output_file_name,
                 plotter_type=plot_type,
                 object_function_plotter_type=obj_func_type,
                 constraints_plotter_type=constraints_type,
                 parameters_numbers=params,
                 levels=levels,
                 grid_obj=objective_grid_size,
                 grid_c=constraints_grid_size,
                 is_points_at_bottom=displacement_of_points,
                 is_need_show_figure=figure_show,
                 is_need_hide_trials_points=hide_trials_points,
                 is_need_hide_no_feasible_points=hide_no_feasible_points,
                 is_need_fill_feasible_region=fill_feasible_region
                 )
