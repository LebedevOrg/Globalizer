import numpy as np
import plotly.express as px
import plotly.graph_objects as go
from scipy import interpolate
from sklearn.neural_network import MLPRegressor


def create_scatter_matrix_figure(df, functional_count, parameters_names, current_color_scale):
    fig = px.scatter_matrix(
        df.loc[df['index'].values == functional_count - 1],
        dimensions=parameters_names,
        color="objective_func",
        labels={"objective_func": "значения<br>целевой<br>функции"},
        opacity=0.7,
        color_continuous_scale=current_color_scale,
        width=len(parameters_names) * 240,
        height=len(parameters_names) * 240
    )
    fig.update_traces(diagonal_visible=False)
    fig.update_layout(paper_bgcolor='#FFFFFF', plot_bgcolor='#FFFFFF')
    return fig

def create_surface_figure(current_color_scale,
                          best_float_point_dictionary, best_value, discrete_parameters_names, best_discrete_point_dictionary,
                          xaxis_column_name, yaxis_column_name, calc, show_additional_figs, xi, yi, Z, x, y, z, x_unaccept, y_unaccept):
    if calc in ['интерполяция', 'аппроксимация']:
        surface = go.Surface(x=xi, y=yi, z=Z,
            colorscale=current_color_scale,
            opacity=1,
            colorbar={'title': "значения<br>целевой<br>функции"}
        )
        fig = go.Figure(data=[surface])

        if show_additional_figs:
            fig.update_traces(contours_z={
                'show': True,
                'usecolormap': True,
                'highlightcolor': "limegreen",
                'project_z': True
            })
        bz_min = float(np.min(Z))
    elif calc == 'по точкам испытаний':
        surface = go.Mesh3d(x=x, y=y, z=z,
            showscale=True,
            intensity=z,
            colorscale=current_color_scale,
            opacity=1
        )
        fig = go.Figure(data=[surface])
        bz_min = float(np.min(z))
                        
    fig.add_scatter3d(x=x, y=y, z=z,
        mode='markers',
        name='точки испытаний',
        marker={
            'size': 2,
            'color': 'blue',
            'opacity': 0.7
        }
    )

    fig.add_scatter3d(
        x=[best_float_point_dictionary[xaxis_column_name]],
        y=[best_float_point_dictionary[yaxis_column_name]],
        z=[best_value],
        name='лучшая точка',
        mode='markers',
        marker={
            'size': 3,
            'color': 'red',
            'opacity': 1
        }
    )
    
    fig.add_trace(go.Scatter3d(
        x=x_unaccept,
        y=y_unaccept,
        z=[bz_min] * len(x_unaccept),
        mode='markers',
        marker={
            'size': 1,
            'color': 'black',
            'opacity': 0.5
        },
        name='точки с нарушенными ограничениями'
    ))

    if not discrete_parameters_names:
        fig.update_layout(
            title=f'Поверхность целевой функции<br>в сечении лучшего решения ({calc})',
            scene={
                'xaxis_title': xaxis_column_name,
                'yaxis_title': yaxis_column_name,
                'zaxis_title': 'значение целевой функции'
            },
            paper_bgcolor='#FFFFFF',
            plot_bgcolor='#FFFFFF',
            showlegend=False,
            height=590,
            template="none",
            title_x=0.1,
            title_xanchor='left'
        )
    else:
        fig.update_layout(
            title=f'Поверхность целевой функции<br>в сечении лучшего решения ({calc})\
                <br>с дискретными параметрами<br>'+str(best_discrete_point_dictionary),
            scene={
                'xaxis_title': xaxis_column_name,
                'yaxis_title': yaxis_column_name,
                'zaxis_title': 'значение целевой функции'
            },
            paper_bgcolor='#FFFFFF',
            plot_bgcolor='#FFFFFF',
            showlegend=False,
            height=590,
            template="none",
            title_x=0.1,
            title_xanchor='left'
        )
    return fig

def create_lines_level_figure(current_color_scale,current_color, functional_count, best_float_point_dictionary,
                              discrete_parameters_names, best_discrete_point_dictionary,
                              xaxis_column_name, yaxis_column_name, calc, show_additional_figs, bounds_x, bounds_y, xi, yi, Z, x, y, z, x_unaccept, y_unaccept):
    if calc in ['интерполяция', 'аппроксимация']:
        fig = go.Figure(data=[go.Contour(x=xi[0], y=yi[:, 0], z=Z,
            colorscale=current_color_scale,
            colorbar={
                'title': 'значения<br>целевой<br>функции'
            }
        )])
    elif calc == 'по точкам испытаний':
        fig = go.Figure(data=[go.Contour(x=x, y=y, z=z,
            colorscale=current_color_scale,
            colorbar={
                # 'titleside' удалён в plotly 6; title.side работает и в plotly 5
                'title': {'text': 'значения<br>целевой<br>функции', 'side': 'right'}
            }
        )])

    if functional_count > 1:
        fig.add_scatter(x=x_unaccept, y=y_unaccept,
            mode='markers',
            name='точки с нарушенными ограничениями',
            marker={
                'size': 2,
                'color': 'black',
                'opacity': 0.5
            }
        )

    fig.add_scatter(x=x, y=y,
        mode='markers',
        name='точки испытаний',
        marker={
            'size': 3,
            'color': 'blue',
            'opacity': 0.7
        }
    )
    fig.add_scatter(
        x=[best_float_point_dictionary[xaxis_column_name]],
        y=[best_float_point_dictionary[yaxis_column_name]],
        mode='markers',
        name='лучшая точка',
        marker={
            'size': 4,
            'color': 'red',
            'opacity': 1
        }
    )

    if not discrete_parameters_names:
        fig.update_layout(
            title=f'Линии уровня целевой функции<br>в сечении лучшего решения ({calc})',
            paper_bgcolor='#FFFFFF',
            plot_bgcolor='#FFFFFF',
            showlegend=True,
            height=590,
            legend={'orientation': "h"},
            xaxis_range=[bounds_x[0], bounds_x[1]], yaxis_range=[bounds_y[0], bounds_y[1]],
            xaxis_title=xaxis_column_name,
            yaxis_title=yaxis_column_name
        )
    else:
        fig.update_layout(
            title=f'Линии уровня целевой функции<br>в сечении лучшего решения ({calc})\
                <br>с дискретными параметрами<br>' + str(best_discrete_point_dictionary),
            paper_bgcolor='#FFFFFF',
            plot_bgcolor='#FFFFFF',
            showlegend=True,
            height=590,
            legend={'orientation': "h"},
            xaxis_range=[bounds_x[0], bounds_x[1]], yaxis_range=[bounds_y[0], bounds_y[1]],
            xaxis_title=xaxis_column_name,
            yaxis_title=yaxis_column_name
        )

    if show_additional_figs:
        fig.add_trace(
            go.Histogram(
                y=y,
                xaxis='x2',
                marker_color=current_color,
                name=f'гистограмма значений {yaxis_column_name}'
            )
        )
        fig.add_trace(
            go.Histogram(
                x=x,
                yaxis='y2',
                marker_color=current_color,
                name=f'гистограмма значений {xaxis_column_name}'
            )
        )
        fig.update_layout(
            xaxis_domain=[0, 0.85],
            yaxis_domain=[0, 0.85],
            xaxis2={'zeroline': False, 'domain': [0.85, 1], 'showgrid': False},
            yaxis2={'zeroline': False, 'domain': [0.85, 1], 'showgrid': False},
            bargap=0,
            hovermode='closest',
        )
    return fig

def calculate_data(xaxis_column_name, yaxis_column_name, calc,
    dfSDI, parameters_names, float_parameters_names, discrete_parameters_names,
    best_discrete_point_dictionary,best_float_point_dictionary, parameter_eps,
    functional_count, float_parameters_bounds
):
    if xaxis_column_name is None:
        xaxis_column_name = float_parameters_names[0]
    if yaxis_column_name is None:
        yaxis_column_name = float_parameters_names[1] # опечатка была? xaxis_column_name = ...

    df = dfSDI.copy()

    # берем окрестность лучшего сочетания дискретных параметров
    if discrete_parameters_names:
        for param in discrete_parameters_names:
            df = df.loc[df[param] == best_discrete_point_dictionary[param]]

    # берем окрестность лучших прочих непрерывных параметров
    for param in parameters_names:
        if (xaxis_column_name != param and yaxis_column_name != param and param not in discrete_parameters_names):
            df = df.loc[abs(df[param] - best_float_point_dictionary[param]) < parameter_eps * 10]

    # берем только допустимые точки
    if functional_count > 1:
        df = df.loc[df['index'].values == functional_count - 1]

    x = np.array(df[xaxis_column_name].values)
    y = np.array(df[yaxis_column_name].values)
    z = np.array(df['objective_func'].values)

    xi = None
    yi = None
    Z = None

    bounds_x = next(iter(float_parameters_bounds[
        (float_parameters_names).index(xaxis_column_name)
    ].values()))
    bounds_y = next(iter(float_parameters_bounds[
        (float_parameters_names).index(yaxis_column_name)
    ].values()))

    if calc in ('интерполяция', 'аппроксимация') and len(np.unique(np.array(list(zip(x, y))), axis=0)) < 3:
        # в окрестности лучшей точки (по прочим параметрам) почти нет испытаний - обычно при большой размерности
        raise ValueError(f'в окрестности лучшего решения {len(x)} точек испытаний - для режима '
                         f'«{calc}» нужно хотя бы 3; выберите режим «по точкам испытаний»')

    if calc == 'интерполяция':
        #if not self.__discrete_parameters_names:
        points = np.array(list(zip(x, y)))
        values = np.array(z)

        _, unique_indices = np.unique(points, axis=0, return_index=True)

        # Оставляем только уникальные координаты
        x_unique = points[unique_indices, 0]
        y_unique = points[unique_indices, 1]
        z_unique = values[unique_indices]

        if len(float_parameters_names) > 2:
            interp = interpolate.Rbf(x_unique, y_unique, z_unique, function='linear')
        else:
            interp = interpolate.Rbf(x_unique, y_unique, z_unique)
                    
        xi = np.linspace(bounds_x[0], bounds_x[1], 150)
        yi = np.linspace(bounds_y[0], bounds_y[1], 150)
        xi, yi = np.meshgrid(xi, yi)
        Z = interp(xi, yi)

    elif calc == 'аппроксимация':
        nn = MLPRegressor(
            activation='logistic',   # can be tanh, identity, logistic, relu
            solver='lbfgs',          # can be lbfgs, sgd , adam
            alpha=0.001,
            hidden_layer_sizes=(40,),
            max_iter=10000,
            tol=10e-6,
            random_state=10
        )

        points = [list(x), list(y)]
        points = list(map(list, zip(*points)))

        nn.fit(points, z)
        xi = np.linspace(bounds_x[0], bounds_x[1], 150)
        yi = np.linspace(bounds_y[0], bounds_y[1], 150)
        xi, yi = np.meshgrid(xi, yi)

        xy = np.c_[xi.ravel(), yi.ravel()]

        Z = nn.predict(xy)
        Z = Z.reshape(150, 150)

    del df

    '''
    x_noncomput = []
    y_noncomput = []
    '''

    x_unaccept = []
    y_unaccept = []


    # невычислимые точки
    df = dfSDI.copy()
    df = df.loc[df['index'].values == -3]

    '''
    if not df.empty:
        x_noncomput = np.array(df[xaxis_column_name].values)
        y_noncomput = np.array(df[yaxis_column_name].values)
    '''

    del df

    # недопустимые точки
    if functional_count > 1:
        df = dfSDI.copy()
        #df = df.loc[df['index'].values != self.__functional_count - 1 & df['index'].values != -3]
        indices_to_exclude = [functional_count - 1, -3]
        df = df.loc[~df['index'].isin(indices_to_exclude)]
        if not df.empty:
            x_unaccept = np.array(df[xaxis_column_name].values)
            y_unaccept = np.array(df[yaxis_column_name].values)
        del df

    return bounds_x, bounds_y, x, y, z, xi, yi, Z, x_unaccept, y_unaccept # x_noncomput, y_noncomput


