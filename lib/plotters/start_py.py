from interface_cpp import DrawingProcess

import argparse
from datetime import datetime

def parse_args():
    ap = argparse.ArgumentParser()

    ap.add_argument(
        "SourceFilesPath",
        type=str,
        help="The path of the files to read problem info and trials points."
    )
    ap.add_argument(
        "ProblemFileName",
        type=str,
        help="The file with problem info."
    )
    ap.add_argument(
        "TrialsFileName",
        type=str,
        help="The file with trials points."
    )
    ap.add_argument(
        "-pfn",
        "--PlotFileName",
        type=str,
        default=datetime.now().strftime("plotter_%Y_%d_%m_%H_%M_%S.png"),
        help="The name of the file to save the image."
    )
    ap.add_argument(
        "-ft",
        "--FigureType",
        type=str,
        default="LevelLayers",
        choices=[
            "LevelLayers",
            "Surface"
        ],
        help="The type of visualization of the target function."
    )
    ap.add_argument(
        "-ct",
        "--CalcsType",
        type=str,
        default="Interpolation",
        choices=[
            "ObjectiveFunction",
            "Approximation",
            "Interpolation",
            "ByPoints",
            "OnlyPoints"
        ],
        help="The type of value calculations for visualizing the objective function."
    )
    ap.add_argument(
        "-ctc",
        "--CalcsTypeC",
        type=str,
        default="Interpolation",
        choices=[
            "ObjectiveFunction",
            "Approximation",
            "Interpolation",
            "ByPoints",
            "OnlyPoints"
        ],
        help="The type of value calculations for visualizing the objective function."
    )
    ap.add_argument(
        "-eps",
        "--Epsilon",
        type=float,
        default=0.01,
        help="Accuracy of solve."
    )
    ap.add_argument(
        "-x1",
        "--x1",
        type=int,
        default=0,
        help="First parameter number for visualization."
    )
    ap.add_argument(
        "-x2",
        "--x2",
        type=int,
        default=1,
        help="Second parameter number for visualization."
    )

    ap.add_argument(
        "-lvls",
        "--Levels",
        type=int,
        default=25,
        help="The number of level lines for visualizing in mode LevelLayers."
    )

    ap.add_argument(
        "-grido",
        "--ObjectiveGridSize",
        type=int,
        default=100,
        help="The number of grid points for visualizing the objective function."
    )

    ap.add_argument(
        "-gridc",
        "--ConstraintsGridSize",
        type=int,
        default=200,
        help="The number of grid points for visualizing the constraints."
    )

    ap.add_argument(
        "-b",
        "--Bounds",
        type=str,
        default="",
        help="Visualization area boundaries\
        (in format \" {lower_x1}_{lower_x2} {upper_x1}_{upper_x2}\" or \" {lower_x1}_{upper_x1}\"). \
        Warning: begining space nedded for lower_x1 < 0."
    )
    ap.add_argument(
        "-ShowFigure",
        "--ShowFigure",
        action="store_true",
        help="A flag indicating the need to open the resulting drawing in an interactive window on the screen."
    )
    ap.add_argument(
        "-HideTrialsPoints",
        "--HideTrialsPoints",
        action="store_true",
        help="A flag indicating the need to hide trial points."
    )
    ap.add_argument(
        "-PointsBelowGraph",
        "--PointsBelowGraph",
        action="store_true",
        help="A flag indicating the need to shift the trial points in the figure below the graph."
    )
    ap.add_argument(
        "-FillFeasibleRegion",
        "--FillFeasibleRegion",
        action="store_true",
        help="A flag indicating the need to fill feasible region with color."
    )
    ap.add_argument(
        "-1D",
        "--1D",
        action="store_true",
        help="A flag indicating the need to paint 1-D figure by x1."
    )

    return ap

def read_args():
    ap = parse_args()
    args = vars(ap.parse_args())

    if (args["SourceFilesPath"] is None) or (args["TrialsFileName"] is None) or (args["ProblemFileName"] is None):
        raise ValueError("The required arguments SourceFilesPath, TrialsFileName, and ProblemFileName are missing.")

    else:
        path = str(args["SourceFilesPath"])
        if not path.endswith("/"):
            path += "/"
        trials_file_name = str(args["TrialsFileName"])
        problem_file_name = str(args["ProblemFileName"])
        output_file_name = str(args["PlotFileName"])

        eps = float(args["Epsilon"])

        types_names_map = {
            "LevelLayers": "lines layers",
            "Surface": "surface",
            "ObjectiveFunction": "objective function",
            "Approximation": "approximation",
            "Interpolation": "interpolation",
            "ByPoints": "by points",
            "OnlyPoints": "only points"
        }

        plot_type = str(args["FigureType"])
        plot_type = types_names_map.get(plot_type)

        obj_func_type = str(args["CalcsType"])
        obj_func_type = types_names_map.get(obj_func_type)

        levels = int(args["Levels"])

        grid_obj = int(args["ObjectiveGridSize"])
        grid_c = int(args["ConstraintsGridSize"])

        bounds = str(args["Bounds"])

        displacement_of_points = bool(args["PointsBelowGraph"])
        figure_show = bool(args["ShowFigure"])
        hide_trials_points = bool(args["HideTrialsPoints"])
        fill_feasible_region = bool(args["FillFeasibleRegion"])

        is_1D = bool(args["1D"])

        if is_1D:
            params = list({int(args["x1"])})
        else:
            params = list({int(args["x1"]), int(args["x2"])})


    return (
        path,
        trials_file_name,
        problem_file_name,
        output_file_name,
        eps,
        plot_type,
        obj_func_type,
        params,
        levels,
        grid_obj,
        grid_c,
        displacement_of_points,
        figure_show,
        hide_trials_points,
        fill_feasible_region,
        bounds
    )

if __name__ == "__main__":
    (path, trials_file_name, problem_file_name, output_file_name,
     eps, plot_type, obj_func_type, params, levels, grid_obj, grid_c,
     displacement_of_points, figure_show, hide_trials_points, fill_feasible_region, bounds) = read_args()

    print(f"""
    Plotter startup parameters:
    SourceFilesPath: {path}
    TrialsFileName: {trials_file_name}
    ProblemFileName: {problem_file_name}
    OutputFileName: {output_file_name}
    Eps: {eps}
    PlotType: {plot_type}
    Levels: {levels}
    ObjectiveGridSize: {grid_obj}
    ConstraintsGridSize: {grid_c}
    ObjFuncType: {obj_func_type}
    Params: {params}
    DisplacementOfPoints: {displacement_of_points}
    FigureShow: {figure_show}
    HideTrialsPoints: {hide_trials_points}
    FillFeasibleRegion: {fill_feasible_region}
    Bounds: {bounds}
    """)

    filename = path + problem_file_name

    if bounds:
        lines = ""
        with open(filename, "r") as file:
            lines = file.readlines()

        old_problem_init_info = lines[0]
        new_problem_init_info = old_problem_init_info.split(' ')[0] + ' ' + bounds + '\n'

        lines[0] = new_problem_init_info
        with open(filename, "w") as file:
            file.writelines(lines)

    dp = DrawingProcess(path, trials_file_name, problem_file_name, eps)

    if bounds:
        lines[0] = old_problem_init_info
        with open(filename, "w") as file:
            file.writelines(lines)

    dp.draw_plot(plotter_type=plot_type,
                 object_function_plotter_type=obj_func_type,
                 parameters_numbers=params,
                 is_points_at_bottom=displacement_of_points,
                 output_file=output_file_name,
                 levels=levels,
                 grid_obj=grid_obj,
                 grid_c=grid_c,
                 is_need_show_figure=figure_show,
                 is_need_hide_trials_points=hide_trials_points,
                 is_need_fill_feasible_region=fill_feasible_region
                 )
    
    print(f"Picture was saved in {path + output_file_name}.")
