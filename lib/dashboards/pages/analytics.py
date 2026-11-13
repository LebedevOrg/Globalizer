import dash_daq as daq
from dash import dcc, html

from components.charts import create_scatter_matrix_figure


def render_multidimensional_representation(parameters_names):
        return html.Div(children=[
            html.H2('Многомерная визуализация', style={'textAlign': 'left'}),
            html.Div(children=[
                html.Div(children=[
                    html.Br(),
                    html.Label('Параметры для визуализации'),
                    dcc.Dropdown(
                        parameters_names,
                        parameters_names,
                        id='mdf-parameter-names',
                        multi=True
                    ),
                ], style={'width': '15%'}),
                html.Div(children=[
                    dcc.Graph(
                        id='multidimensional_figure',
                        config={'displayModeBar': True},
                    )
                ], style={'width': '85%'}),
            ], style={'display': 'flex', 'flexDirection': 'row', 'height': '90%'})
        ], style={"background-color": "#FFFFFF", "border": "20px solid #FFFFFF"})

def render_surface_and_level_lines(figure_type_names, calculation_type_names, float_parameters_names):
    if len(float_parameters_names) < 2:
        # поверхность и линии уровня строятся по двум непрерывным параметрам
        return html.Div(children=[
            html.H2('Визуализация в окрестности лучшего решения', style={'textAlign': 'left'}),
            html.P('Нужны хотя бы два непрерывных параметра, у задачи их: '
                   f'{len(float_parameters_names)}.'),
        ], style={"background-color": "#FFFFFF", "border": "20px solid #FFFFFF", })
    return html.Div(children=[
        html.H2('Визуализация в окрестности лучшего решения', style={'textAlign': 'left'}),
        html.Div([
            html.Div(children=[
                html.Br(),
                html.Label('Тип графика'),
                dcc.Dropdown(
                    figure_type_names,
                    figure_type_names[0],
                    id='sllf-figure-type',
                ),
                html.Label('Режим построения целевой функции'),
                dcc.Dropdown(
                    calculation_type_names,
                    calculation_type_names[0],
                    id='sllf-calculation-type',
                ),
                html.Label('Параметр на X-оси'),
                dcc.Dropdown(
                    float_parameters_names,
                    float_parameters_names[0],
                    id='sllf-xaxis-parameter-name',
                ),
                html.Label('Параметр на Y-оси'),
                dcc.Dropdown(
                    float_parameters_names,
                    float_parameters_names[1],
                    id='sllf-yaxis-parameter-name',
                ),
                html.Label('Показать доп.графики'),
                html.Td(
                    daq.BooleanSwitch(id='additional-figures', on=True, color="#3E59A5")
                )
            ], style={'width': '20%'}),
            html.Div(children=[
                dcc.Graph(
                    id='surface_or_lines_level_figure',
                    config={'displayModeBar': True},
                )
            ], style={'width': '75%'}),
        ], style={'display': 'flex', 'flexDirection': 'row', 'width': '90%', 'height': '600px'}),
    ], style={"background-color": "#FFFFFF", "border": "20px solid #FFFFFF", })

def render_objective_function_values_scatter(discrete_parameters_names, float_parameters_names):
    if not discrete_parameters_names:
        return html.Div(children=[
                html.H2('Распределение значений целевой функции', style={'textAlign': 'left'}),
                dcc.Tab(label='Непрерывные параметры', children=[
                    html.Div([
                        html.Div(children=[
                            html.Br(),
                            html.Label('Параметр на X-оси'),
                            dcc.Dropdown(
                                float_parameters_names,
                                float_parameters_names[0],
                                id='csf-parameter-name',
                                style={'width': '60%'}
                            ),
                            dcc.Graph(
                                id='continuous_scatter_figure',
                                config={'displayModeBar': True},
                            )
                        ], style={'width': '100%'}),
                    ], style={'display': 'flex', 'flexDirection': 'row', 'width': '90%', 'height': '70%'}),
                ])
            ], style={'width': '60%', "background-color": "#FFFFFF", "border": "20px solid #FFFFFF", })
    else:
        return html.Div(children=[
            html.H2('Распределение значений целевой функции', style={'textAlign': 'left'}),
            dcc.Tabs([
                dcc.Tab(label='Непрерывные параметры', children=[
                    html.Div([
                        html.Div(children=[
                            html.Br(),
                            html.Label('Параметр на X-оси'),
                            dcc.Dropdown(
                                float_parameters_names,
                                float_parameters_names[0],
                                id='csf-parameter-name',
                                style={'width': '60%'}
                            ),
                            dcc.Graph(
                                id='continuous_scatter_figure',
                                config={'displayModeBar': True},
                            )
                        ], style={'width': '100%'}),
                    ], style={'display': 'flex', 'flexDirection': 'row', 'width': '90%', 'height': '70%'}),
                ]),
                dcc.Tab(label='Дискретные параметры', children=[
                    html.Div([
                        html.Div(children=[
                            html.Br(),
                            html.Label('Параметр на X-оси'),
                            dcc.Dropdown(
                                discrete_parameters_names,
                                discrete_parameters_names[0],
                                id='dsf-parameter-name',
                                style={'width': '60%'}
                            ),
                            dcc.Graph(
                                id='discrete_scatter_figure',
                                config={'displayModeBar': True},
                            )
                        ], style={'width': '100%'}),
                    ], style={'display': 'flex', 'flexDirection': 'row', 'width': '90%', 'height': '70%'}),
                ])
            ], style={'width': '90%'})
        ], style={'width': '60%', "background-color": "#FFFFFF", "border": "20px solid #FFFFFF", })
    
def render_parameters_dependence(df, functional_count, parameters_names, current_color_scale):
    return html.Div([
        html.H2('Матрица графиков рассеяния', style={'textAlign': 'left'}),
        html.Div(children=[
            html.Div(children=[
                dcc.Graph(
                    figure=create_scatter_matrix_figure(df, functional_count, parameters_names, current_color_scale),
                    config={'displayModeBar': True},
                )
            ], style={'width': '100%'}),
        ], style={'maxWidth': '100%', 'maxHeight': '600px', "overflow": "scroll"}),
        html.P('** Показывает связь между разными парами переменных, цвет опредяет группы точек, соответствующих большим и меньшим значениям целевой функции', style={'text-align': 'right', 'color': '#212121', 'font-size': '0.8em'}),    
    ], style={'width': '100%', "background-color": "#FFFFFF", "border": "20px solid #FFFFFF", 'height': '750px'})

def render_parameters_importance(parameters_names, importance, current_color):
        return html.Div([
            html.H2('Значимость параметров', style={'textAlign': 'left'}),
            html.Div([
                dcc.Graph(
                    figure={
                        'data': [
                            {'x': parameters_names,
                             'y': importance,
                             'orientation': 'v',
                             'type': 'bar',
                             'text': importance,
                             'textposition': 'outside',
                             'textfont': "black",
                             'text_auto': '.2s',
                             'marker': {
                                 'color': current_color,
                                 'line': {'color': current_color, 'width': '1'}
                             }
                             },
                        ],
                        'layout': {
                            'xaxis': {
                                'anchor': 'y',
                                'tickfont': {'size': '10'},
                                'tickangle': -90
                            },
                            'yaxis': {
                                'range': [0, 1],
                                'anchor': 'x',
                                'title': {'text': 'вклад'},
                                'tickfont': {'size': '10'}
                            },
                            'title': "Значимость по вкладу</br></br>в разброс значений целевой функции",
                            'paper_bgcolor': '#FFFFFF',
                            'plot_bgcolor': '#FFFFFF',
                            'margin': {
                                't': 130,
                                'b': 200,
                                'r': 100,
                            }
                        }
                    },
                    config={
                        'scrollZoom': True,  # True, False
                        'showTips': True,  # True, False
                        'displayModeBar': True,  # True, False, 'hover'
                    },
                )
            ])
        ], style={'width': '40%', "background-color": "#FFFFFF", "border": "20px solid #FFFFFF"})
