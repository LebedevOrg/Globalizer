import base64
import json
from pathlib import Path

import dash
import dash_bootstrap_components as dbc
import pandas as pd
import plotly.express as px
import plotly.graph_objects as go
from dash import dcc, html
from dash.dependencies import Input, Output

from components.charts import (
    calculate_data,
    create_lines_level_figure,
    create_surface_figure,
)
from components.load_data import load_and_prepare_data
from components.sidebar import create_sidebar
from pages.analytics import (
    render_multidimensional_representation,
    render_objective_function_values_scatter,
    render_parameters_dependence,
    render_parameters_importance,
    render_surface_and_level_lines,
)
from pages.archive import render_archive
from pages.solution import (
    render_iteration_characteristic,
    render_optimization_time,
    render_parameters_description,
    render_problem_description,
    render_solution_description,
)


class StaticDashboard:
    def __init__(self, data_path, mode='Release'):
        self.__mode = mode
        self.__app = self.__create_app()
        self.__load_data(data_path)
        self.__initialize_state()
        self.__register_callbacks()

    '''
    dcc.Upload(
                        id='upload-data',
                        children=html.Div([
                            'Перетащите файл или ',
                            html.A('Выберите файл')
                        ]),
                        style={
                            'width': '95%',
                            'height': '60px',
                            'lineHeight': '60px',
                            'borderWidth': '1px',
                            'borderStyle': 'dashed',
                            'borderRadius': '5px',
                            'textAlign': 'center',
                            'margin': '10px'
                        },
                        multiple=True
                )
    '''
        
    def launch(self):
        prepared_data = load_and_prepare_data(self.__data)
        self.__init_data(prepared_data)

        #del self.__data
        #self.__data = None

        self.__app.layout = html.Div([
            dcc.Location(id="url"),
            html.Div(
                id="data_from_json_file",
                style={'display': 'none'},
            ),
            self.__create_sidebar_navigator(),
            self.__create_content_page()
        ], style={"background-color": "#FBFBFB"})

        self.__app.run(debug=True)

    @staticmethod
    def __create_app():
        assets_folder = Path(__file__).resolve().parent.parent / 'assets'

        app = dash.Dash(
            __name__,
            external_stylesheets=[dbc.themes.LUX],
            title='GlobalizerDashboard',
            assets_folder=str(assets_folder)
        )
        app.config['suppress_callback_exceptions'] = True
        return app

    def __load_data(self, data_path):
        with open(data_path) as json_data:
            self.__data = json.load(json_data)

    def __initialize_state(self):
        self.__init = False

        self.__color_themes_dict = {
            "Ice": px.colors.sequential.ice,
            "Viridis": px.colors.sequential.Viridis,
            "Bluered": px.colors.sequential.Bluered,
            "Electric": px.colors.sequential.Electric,
            "Jet": px.colors.sequential.Jet,
            "Plasma": px.colors.sequential.Plasma,
            "RdBu_r": px.colors.sequential.RdBu_r,
        }
        self.__color_scale_names = list(self.__color_themes_dict)
        self.__current_color_scale = px.colors.sequential.ice
        self.__current_color = px.colors.sequential.ice[0]

        self.__figure_type_names = [
             '3D поверхность',
             'линии уровня'
        ]

        self.__calculation_type_names = [
            'интерполяция',
            'аппроксимация',
            'по точкам испытаний',
        ]

        self.__dfSDI = None
        self.__dfSDI_original = None

        self.__task_name = None
        self.__functional_count = 1
        self.__float_parameters_names = None
        self.__discrete_parameters_names = None
        self.__float_parameters_bounds = None
        self.__parameter_eps = None
        self.__parameter_r = None
        self.__parameter_iters_limit = None
        self.__parameter_start_point = None

        self.__trial_number = None
        self.__global_iter_number = None
        self.__local_iter_number = None
        self.__accuracy = None
        self.__solve_time = None
        self.__optimization_time = None

        self.__best_trial_number = None
        self.__best_value = None
        self.__best_float_point_dictionary = None
        self.__best_discrete_point_dictionary = None
        self.__importance = None

    def __register_callbacks(self):
        self.__app.callback(
            Output("page-content", "children"),
            Input("url", "pathname"),
            Input('color-theme', "value"),
            Input('elimination-of-emissions', 'on'),
            Input('data_from_json_file', 'children'),
        )(self.__render_page_content)

        self.__app.callback(
            Output('discrete_scatter_figure', 'figure'),
            Input('dsf-parameter-name', 'value')
        )(self.__update_discrete_scatter_figure)

        self.__app.callback(
            Output('continuous_scatter_figure', 'figure'),
            Input('csf-parameter-name', 'value')
        )(self.__update_continuous_scatter_figure)

        self.__app.callback(
            Output('surface_or_lines_level_figure', 'figure'),
            Input('sllf-xaxis-parameter-name', 'value'),
            Input('sllf-yaxis-parameter-name', 'value'),
            Input('sllf-figure-type', 'value'),
            Input('sllf-calculation-type', 'value'),
            Input('additional-figures', 'on'),
        )(self.__update_surface_or_lines_level_figure)

        self.__app.callback(
            Output('multidimensional_figure', 'figure'),
            Input('mdf-parameter-names', 'value')
        )(self.__update_multidimensional_figure)

        self.__app.callback(
            Output('archive_figure', "children"),
            Input('datatable-interactivity', 'derived_virtual_data'),
            Input('datatable-interactivity', 'derived_virtual_selected_rows')
        )(self.__update_archive_figure)

        self.__app.callback(
            Output('data_from_json_file', 'children'),
            Input('upload-data', 'contents', allow_optional=True)
        )(self.__load_data_from_json)

    def __init_data(self, data):
        self.__dfSDI = data['search_data']
        self.__dfSDI_original = data['search_data_original']

        self.__task_name = data['task_name']
        self.__functional_count = data['functional_count']
        self.__float_parameters_names = data['params_float']
        self.__discrete_parameters_names = data['params_discrete']
        self.__parameters_names = data['params_all']
        self.__float_parameters_bounds = data['float_parameters_bounds']

        self.__parameter_eps = data['eps']
        self.__parameter_r = data['r']
        self.__parameter_iters_limit = data['iters_limit']
        self.__parameter_start_point = data['start_point']

        self.__trial_number = data['trial_number']
        self.__global_iter_number = data['global_iter_number']
        self.__local_iter_number = data['local_iter_number']
        self.__accuracy = data['accuracy']
        self.__solve_time = data['solve_time']
        self.__optimization_time = data['iters_times']

        self.__best_trial_number = data['best_trial_number']
        self.__best_value = data['best_value']
        self.__best_float_point_dictionary = data['best_float_point_dict']
        self.__best_discrete_point_dictionary = data['best_discrete_point_dict']
        self.__importance = data['parameters_importance']

    def __create_sidebar_navigator(self):
        logo_url = self.__app.get_asset_url(
            'images/globalizer_dash_light.png'
        )
        return html.Div([create_sidebar(
            self.__color_scale_names,
            logo_url
        )])

    def __create_content_page(self):
        return html.Div(id="page-content", className="content")

    def __update_SDI(self, hide=False):
        if hide:
            up = self.__calculate_up_by_IQR()
            self.__dfSDI = self.__dfSDI[(self.__dfSDI['objective_func'] <= up)]
        else:
            del self.__dfSDI
            self.__dfSDI = self.__dfSDI_original.copy()
            self.__dfSDI.drop(self.__dfSDI[self.__dfSDI['objective_func'] >= 1.797692e+308].index, inplace=True)
            self.__dfSDI.drop(self.__dfSDI[self.__dfSDI['index'] == -2].index, inplace=True)

    def __calculate_up_by_IQR(self):
        objective_values = pd.to_numeric(
            self.__dfSDI['objective_func'],
            errors='coerce',
        ).dropna()
        
        Q1 = objective_values.quantile(0.25)
        Q3 = objective_values.quantile(0.75)
        mid = objective_values.median()
        IQR = Q3 - Q1

        return mid + 1.5 * IQR

    def __render_page_content(self, pathname, color_theme, hide, _uploaded_data):
        self.__update_SDI(hide)
        self.__current_color_scale = self.__color_themes_dict[color_theme]
        self.__current_color = self.__current_color_scale[0]
        if pathname == "/":
            return self.__render_solution_page()
        elif pathname == "/archive":
            return self.__render_archive_page()     
        elif pathname == "/analytics":
            return self.__render_analytics_page()

        # If the user tries to reach a different page, return a 404 message
        return dbc.Jumbotron(
            [
                html.H1("404: Not found", className="text-danger"),
                html.Hr(),
                html.P(f"Путь к файлу {pathname} не распознан..."),
            ]
        )

    def __render_solution_page(self):
        return [
            html.Div([
                render_parameters_description(
                    self.__parameter_eps,
                    self.__parameter_r,
                    self.__parameter_iters_limit,
                    self.__parameter_start_point
                ),
                render_problem_description(
                    self.__task_name,
                    self.__parameters_names,
                    self.__float_parameters_names,
                    self.__discrete_parameters_names,
                    self.__functional_count
                ),
            ], style={'display': 'flex', 'flexDirection': 'row'}),
            html.Div([
                render_solution_description(
                    self.__best_value,
                    self.__best_float_point_dictionary,
                    self.__best_discrete_point_dictionary,
                    self.__best_trial_number,
                    self.__accuracy,
                    self.__trial_number,
                    self.__global_iter_number,
                    self.__local_iter_number
                ),
                render_optimization_time(
                    self.__solve_time,
                    self.__dfSDI_original,
                    self.__optimization_time,
                    self.__current_color
                )
            ], style={'display': 'flex', 'flexDirection': 'row'}),
            render_iteration_characteristic(
                self.__dfSDI,
                self.__functional_count,
                self.__current_color
            ),
        ]
         
    def __render_analytics_page(self):
        return [
            html.Div(children=[
                html.P("*[c] - непрерывные параметры, [d] - дискретные параметры",
                    style={'text-align': 'right', 'color': '#212121', 'font-size': '0.8em'}),
            ], style={"background-color": "#FFFFFF", "border": "20px solid #FFFFFF"}),
            render_surface_and_level_lines(
                self.__figure_type_names, 
                self.__calculation_type_names, 
                self.__float_parameters_names
            ),
            render_multidimensional_representation(
                self.__parameters_names
            ),
            html.Div([
                render_objective_function_values_scatter(
                    self.__discrete_parameters_names,
                    self.__float_parameters_names
                ),
                render_parameters_importance(
                    self.__parameters_names,
                    self.__importance,
                    self.__current_color
                )
            ], style={'display': 'flex', 'flexDirection': 'row'}),
            html.Div([
                render_parameters_dependence(
                    self.__dfSDI,
                    self.__functional_count,
                    self.__parameters_names,
                    self.__current_color_scale
                ),
            ], style={'display': 'flex', 'flexDirection': 'row'}),
        ]

    def __render_archive_page(self):
        if self.__mode == 'Release':
            return [
                render_archive(
                    self.__dfSDI_original[['trial'] + self.__parameters_names + ["objective_func"] + ["index"]]
                )
            ]
        elif self.__mode == 'Debug':
            return [
                render_archive(
                    self.__dfSDI_original
                )
            ]    

    def __load_data_from_json(self, contents):
        if contents is None:
            return html.Div(
                id='hidden-div',
                style={'display': 'none'},
            )

        if not isinstance(contents, list):
            contents = [contents]

        _, content_string = contents[0].split(',', 1)
        decoded = base64.b64decode(content_string)
        prepared_data = load_and_prepare_data(json.loads(decoded))
        self.__init_data(prepared_data)

        return html.Div(
            id='hidden-div',
            style={'display': 'none'},
        )
    
    def __update_archive_figure(self, rows, derived_virtual_selected_rows):
        if derived_virtual_selected_rows is None:
            derived_virtual_selected_rows = []

        dff = self.__dfSDI if rows is None else pd.DataFrame(rows)

        ids = [
            row_id
            for row_id, is_new in enumerate(
                ~dff['trial'].isin(self.__dfSDI['trial'])
            )
            if is_new
        ]

        colors = ['#31B37C' if i in derived_virtual_selected_rows else
                  "red" if i in ids else
                  self.__current_color
                  for i in range(len(dff))]

        return [
            dcc.Graph(
                figure={
                    "data": [
                        {
                            "x": dff['trial'],
                            "y": dff['objective_func'],
                            "type": "bar",
                            "marker": {"color": colors},
                        }
                    ],
                    "layout": {
                        "xaxis": {"automargin": True, "title": "trial"},
                        "yaxis": {
                            "automargin": True,
                            "title": {"text": "значение<br>целевой<br>функции"}
                        },
                        "height": 250,
                        "margin": {"t": 10, "l": 10, "r": 10},
                        "paper_bgcolor": '#FFFFFF',
                        "plot_bgcolor": '#FFFFFF'
                    },
                },
            )
        ]

    def __update_discrete_scatter_figure(self, xaxis_column_name=None):
        if xaxis_column_name is None:
            xaxis_column_name = self.__discrete_parameters_names[0]

        fig = px.violin(
            self.__dfSDI.loc[self.__dfSDI['index'].values == self.__functional_count - 1],
            x=xaxis_column_name,
            y='objective_func',
            title="Разброс значений целевой функции<br>в зависимости от выбранного параметра",
            color_discrete_sequence=[self.__current_color]
        )
        fig.update_xaxes(title=xaxis_column_name)
        fig.update_yaxes(title='значение целевой функции')
        fig.update_layout(paper_bgcolor='#FFFFFF', plot_bgcolor='#FFFFFF', showlegend=False)
        return fig

    def __update_continuous_scatter_figure(self, xaxis_column_name=None):
        if xaxis_column_name is None:
            xaxis_column_name = self.__float_parameters_names[0]

        fig = px.scatter(
            self.__dfSDI.loc[self.__dfSDI['index'].values == self.__functional_count - 1],
            x=xaxis_column_name,
            y='objective_func',
            color=self.__dfSDI.loc[self.__dfSDI['index'].values == self.__functional_count - 1]['trial'][::-1],
            color_continuous_scale=list(reversed(self.__current_color_scale)),
            title="Разброс значений целевой функции<br>в зависимости от выбранного параметра",
            opacity=0.3
        )
        fig.update_xaxes(title=xaxis_column_name)
        fig.update_yaxes(title='значение целевой функции')
        fig.update_layout(paper_bgcolor='#FFFFFF', plot_bgcolor='#FFFFFF', showlegend=False,
            coloraxis_colorbar={
                'title': {
                    'text': 'номера<br>испытаний',
                }
            }
        )
        return fig

    def __update_surface_or_lines_level_figure(self, xaxis_column_name=None, yaxis_column_name=None, type='3D поверхность',
        calc='интерполяция', show_additional_figs=True):
        try:
            bounds_x, bounds_y, x, y, z, xi, yi, Z, x_unaccept, y_unaccept = calculate_data(
                xaxis_column_name, yaxis_column_name, calc,
                self.__dfSDI, self.__parameters_names, self.__float_parameters_names, self.__discrete_parameters_names,
                self.__best_discrete_point_dictionary, self.__best_float_point_dictionary, self.__parameter_eps,
                self.__functional_count, self.__float_parameters_bounds)
        except ValueError as e:
            fig = go.Figure()
            fig.add_annotation(text=f'График не построен: {e}', showarrow=False,
                               xref='paper', yref='paper', x=0.5, y=0.5)
            fig.update_xaxes(visible=False)
            fig.update_yaxes(visible=False)
            return fig
        if type == '3D поверхность':
            return create_surface_figure(self.__current_color_scale,
                self.__best_float_point_dictionary, self.__best_value, self.__discrete_parameters_names, self.__best_discrete_point_dictionary,
                xaxis_column_name, yaxis_column_name, calc, show_additional_figs,
                xi, yi, Z, x, y, z, x_unaccept, y_unaccept)
        elif type == 'линии уровня':
            return create_lines_level_figure(self.__current_color_scale, self.__current_color, self.__functional_count, self.__best_float_point_dictionary,
                self.__discrete_parameters_names, self.__best_discrete_point_dictionary,
                xaxis_column_name, yaxis_column_name, calc, show_additional_figs,
                bounds_x, bounds_y, xi, yi, Z, x, y, z, x_unaccept, y_unaccept)

    def __update_multidimensional_figure(self, xaxis_column_name=None):
        df = self.__dfSDI.copy()

        # берем только допустимые вычисленные точки
        if self.__functional_count > 1:
            df = df.loc[df['index'].values == self.__functional_count - 1]

        if xaxis_column_name is None:
            xaxis_column_name = self.__parameters_names
        xaxis_column_name = ['objective_func'] + xaxis_column_name

        dims = []
        xaxis_column_name_dict = {}
        for name in xaxis_column_name:
            if name in self.__discrete_parameters_names:
                encoded_name = f'{name}_cat'
                df[encoded_name] = df[name].astype('category').cat.codes
                dims.append(encoded_name)
                xaxis_column_name_dict[encoded_name] = name
            else:
                dims.append(name)
                xaxis_column_name_dict[name] = name

        fig = px.parallel_coordinates(
            df,
            color="objective_func",
            dimensions=dims,
            labels=xaxis_column_name_dict,
            color_continuous_scale=self.__current_color_scale
        )

        fig.update_layout(
            xaxis={'title': 'параметры', 'ticktext': xaxis_column_name},
            yaxis={'title': 'значение целевой функции'},
            paper_bgcolor='#FFFFFF',
            plot_bgcolor='#FFFFFF',
            coloraxis_colorbar_title="значения<br>целевой<br>функции"
        )

        fig.update_traces(unselected_line_opacity=0.5, selector={'type': 'parcoords'})

        del df
        return fig
