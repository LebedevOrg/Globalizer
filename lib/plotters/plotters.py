import matplotlib.pyplot as plt
import numpy as np
from sklearn.neural_network import MLPRegressor
from scipy import interpolate

class Plotter:
    """
    Базовый класс плоттеров
    """
    def plot_by_grid(self):
        """
        Отрисовка целевой функции с построением сетки в заданном сечении
        """
        pass
    def plot_approximation(self):
        """
        Отрисовка целевой функции по поисковым испытаниям с использованием аппроксимации
        """
        pass
    def plot_interpolation(self):
        """
        Отрисовка целевой функции по поисковым испытаниям с использованием интерполяции
        """
        pass
    def plot_by_points(self):
        """
        Отрисовка целевой функции по поисковым испытаниям с натягиванием графика на точки
        """
        pass

    def plot_points(self):
        """
        Отрисовка точек поисковых испытаний
        """
        pass

    #@property
    def figure_style_settings_setup(self):
        plt.style.use("bmh") #"fivethirtyeight" / "bmh"
        plt.rcParams['contour.negative_linestyle'] = 'solid'
        plt.rcParams["figure.figsize"] = (8, 6)


    def plot_line(self, x, z, linecolor, linewidth, transparency):
        plt.plot(x, z, color=linecolor, linewidth=linewidth, alpha=transparency)

    def plot_contour(self, x1, x2, z, colormap, linewidths, levels):
        self.ax.contour(x1, x2, z, cmap=colormap, linewidths=linewidths, levels=levels)
    def plot_hatch_by_grid(self, x1, x2, zc):
        total_points = len(x1)
        # assert total_points == rows * cols, f"Ошибка: Ожидалось {rows * cols} точек, а найдено {total_points}."
        rows = int(np.sqrt(total_points))
        cols = int(np.sqrt(total_points))

        z = []
        for i in range(len(zc[0])):
            z.append(np.array([zcj[i] for zcj in zc]))

        xgrid = np.zeros((rows, cols))
        ygrid = np.zeros((rows, cols))

        for i in range(rows):
            for j in range(cols):
                idx = i * cols + j
                xgrid[i, j] = x1[idx]
                ygrid[i, j] = x2[idx]

        zgrid = []
        for k in range(len(z)):
            zgrid.append(np.zeros((rows, cols)))
            for i in range(rows):
                for j in range(cols):
                    idx = i * cols + j
                    zgrid[k][i, j] = z[k][idx]

        for k in range(len(z)):
            self.ax.contour(xgrid, ygrid, zgrid[k], colors=["#393a398f"], linewidths=1, levels=[0])

        mask = (zgrid[0] <= 0)
        for k in range(len(zgrid) - 1):
            mask &= (zgrid[k + 1] <= 0)
        self.ax.contourf(xgrid, ygrid, mask.astype(float), levels=[0.5, 1.0], alpha=0.6, colors=["#00ff483e"])

    def plot_hatch_by_interpolate(self, x1, x2, zc, points_count=200):
        interp = []
        #try:
        for i in range(len(zc)):
            interp.append(interpolate.Rbf(x1[i],
                                          x2[i],
                                          zc[i]))
        #except Exception as err:
        #    print(f"\033[33m\nWARNING: the graph is plotted without displaying the constraints!\n\n\
        #           The trials number is too large to plot using Rbf-interpolation.\n\n\
        #           Possible solutions:\n\
        #          - Reduce the number of points to plot.\n\
        #          Original error text of scipy.interpolate.Rbf:\n\
        #          {err}\n\n\033[0m")
        #   return

        xgrid = np.linspace(self.leftBounds[0], self.rightBounds[0], points_count)
        ygrid = np.linspace(self.leftBounds[1], self.rightBounds[1], points_count)
        xgrid, ygrid = np.meshgrid(xgrid, ygrid)

        zgrid = []
        for i in range(len(zc)):
            zgrid.append(interp[i](xgrid, ygrid))

        for k in range(len(zgrid)):
            self.ax.contour(xgrid, ygrid, zgrid[k], colors=["#393a398f"], linewidths=1, levels=[0])

        mask = (zgrid[0] <= 0)
        for k in range(len(zgrid) - 1):
            mask &= (zgrid[k + 1] <= 0)

        self.ax.contourf(xgrid, ygrid, mask.astype(float), levels=[0.5, 1.0], alpha=0.6, colors=["#00ff483e"])

    def plot_contourf(self, x1, x2, z, colormap, levels):
        c = self.ax.contourf(x1, x2, z, cmap=colormap, levels=levels)

    def plot_surface(self, x1, x2, z, colormap, transparency):
        self.ax.plot_surface(x1, x2, z, cmap=colormap, alpha=transparency)

    def plot_line_pulling_on_points(self, x, z, linecolor, linewidth, transparency):
        self.ax.plot(x, z, color=linecolor, linewidth=linewidth, alpha=transparency)

    def plot_contour_pulling_on_points(self, x1, x2, z, colormap, linewidths, levels, transparency):
        self.ax.tricontour(x1, x2, z, cmap=colormap, linewidths=linewidths, levels=levels, alpha=transparency)

    def plot_surface_pulling_on_points(self, x1, x2, z, colormap, transparency):
        self.ax.plot_trisurf(x1, x2, z, cmap=colormap, alpha=transparency)

class Plotter2D(Plotter):
    """
    Плоттер для построения графика зависимости целевой функции от заданного параметра для задач различной размерности
    """
    def __init__(self, parameter_number, left_bound, right_bound):
        self.index = parameter_number
        self.leftBound = left_bound
        self.rightBound = right_bound

        self.figure_style_settings_setup()

        self.fig, self.ax = plt.subplots(1, 1)
        if self.leftBound != None and self.rightBound != None:
            self.ax.set_xlim([self.leftBound, self.rightBound])
        self.ax.tick_params(axis='both', labelsize=8)
        self.ax.set_facecolor('white')

    def plot_by_grid(self, x, z, linecolor='black', linewidth=1, transparency=0.7):
        self.plot_line(x, z, linecolor, linewidth, transparency)

    def plot_approximation(self, points, values, points_count=100, linecolor='black', linewidth=1, transparency=0.7):
        nn = MLPRegressor(activation='logistic',    # tanh, identity, logistic, relu
                          solver='lbfgs',           # lbfgs, sgd , adam
                          alpha=0.001,
                          hidden_layer_sizes=(50,),
                          max_iter=5000,
                          tol=10e-8,
                          random_state=None)

        nn.fit(np.array(points), np.array(values))
        x = np.linspace(self.leftBound, self.rightBound, points_count)
        z = nn.predict(x[:, np.newaxis])
        self.plot_line(x, z, linecolor, linewidth, transparency)

    def plot_interpolation(self, points, values, points_count=100, linecolor='black', linewidth=1, transparency=0.7):
        continous_points = np.array(points)[:, 0]
        x = np.linspace(min(continous_points), max(continous_points), points_count)
        z = interpolate.interp1d(continous_points.flatten(), np.array(values).flatten(), kind=3)(x)
        self.plot_line(x, z, linecolor, linewidth, transparency)

    def plot_by_points(self, points, values, linecolor='black', linewidth=1, transparency=0.7):
        x, z = zip(*sorted(zip(np.array(points).flatten(), values)))
        self.plot_line_pulling_on_points(x, z, linecolor, linewidth, transparency)

    def plot_points(self, points, values, clr='blue', mrkr='o', mrkrs=4):
        self.ax.scatter(points, values, color=clr, marker=mrkr, s=mrkrs)

class Plotter3D(Plotter):
    """
    Плоттер для построения графика для задач различной размерности
    """
    def __init__(self, parameters_numbers, left_bounds, right_bounds, objective_function_plotter_type,
                 constraints_plotter_type, plotter_type, is_points_at_bottom):
        self.indexes = parameters_numbers
        self.leftBounds = left_bounds
        self.rightBounds = right_bounds
        self.objective_function_type = objective_function_plotter_type
        self.constraints_type = constraints_plotter_type

        self.plotterType = plotter_type
        self.is_points_at_bottom = is_points_at_bottom

        self.figure_style_settings_setup()

        self.fig = plt.Figure()

        if self.plotterType == 'surface':
            self.ax = plt.subplot(projection='3d')
        elif self.plotterType == 'lines layers':
            self.ax = plt.subplot()

        self.ax.set_xlim([self.leftBounds[0], self.rightBounds[0]])
        self.ax.set_ylim([self.leftBounds[1], self.rightBounds[1]])
        self.ax.tick_params(axis='both', labelsize=8)
        self.ax.set_facecolor('white')

    def plot_by_grid(self, x, z, colormap=plt.cm.viridis, linewidths=1, levels=25, transparency=0.6):
        x1 = [xi[0] for xi in x]
        x2 = [xi[1] for xi in x]

        total_points = len(x1)
        #assert total_points == rows * cols, f"Ошибка: Ожидалось {rows * cols} точек, а найдено {total_points}."
        rows = int(np.sqrt(total_points))
        cols = int(np.sqrt(total_points))
        xgrid = np.zeros((rows, cols))
        ygrid = np.zeros((rows, cols))
        zgrid = np.zeros((rows, cols))

        for i in range(rows):
            for j in range(cols):
                idx = i * cols + j
                xgrid[i, j] = x1[idx]
                ygrid[i, j] = x2[idx]
                zgrid[i, j] = z[idx]

        if self.plotterType == 'lines layers':
            self.plot_contour(xgrid, ygrid, zgrid, colormap=colormap, linewidths=linewidths, levels=levels)
        elif self.plotterType == 'surface':
            self.plot_surface(xgrid, ygrid, zgrid, colormap=colormap, transparency=transparency)

    def plot_approximation(self, points, values, points_count=100, colormap=plt.cm.viridis, transparency=0.6, linewidths=1, levels=25):
        nn = MLPRegressor(activation='logistic',  # can be tanh, identity, logistic, relu
                          solver='lbfgs',  # can be lbfgs, sgd , adam
                          alpha=0.001,
                          hidden_layer_sizes=(40,),
                          max_iter=10000,
                          tol=10e-6,
                          random_state=10)

        nn.fit(np.array(points)[:, self.indexes], values)
        x1 = np.linspace(self.leftBounds[0], self.rightBounds[0], points_count)
        x2 = np.linspace(self.leftBounds[1], self.rightBounds[1], points_count)
        x1, x2 = np.meshgrid(x1, x2)

        # np.c - cлияние осей X и Y в точки
        # ravel - развернуть (к одномерному массиву)
        xy = np.c_[x1.ravel(), x2.ravel()]

        # делаем предсказание значений
        z = nn.predict(xy)
        z = z.reshape(points_count, points_count)

        if self.plotterType == 'lines layers':
            self.plot_contour(x1, x2, z, colormap=colormap, linewidths=linewidths, levels=levels, zorder=1)
        elif self.plotterType == 'surface':
            self.plot_surface(x1, x2, z, colormap=colormap, transparency=transparency)

    def plot_interpolation(self, points, values, points_count=100,
                           colormap=plt.cm.viridis, transparency=0.6, linewidths=1, levels=25):
        try:
            interp = interpolate.Rbf(np.array(points)[:, self.indexes[0]], np.array(points)[:, self.indexes[1]], values)
        except Exception as err:
            print(f"\033[33m\nWARNING: the graph is plotted without displaying the objective function!\n\n\
The trials number is too large to plot a 3D graph using Rbf-interpolation.\n\n\
Possible solutions:\n\
- Reduce the number of points to plot.\n\
- Use the mode \'ByPoints\' to plot surface.\n\
Original error text of scipy.interpolate.Rbf:\n\
{err}\n\n\033[0m")
            return

        x1 = np.linspace(self.leftBounds[0], self.rightBounds[0], points_count)
        x2 = np.linspace(self.leftBounds[1], self.rightBounds[1], points_count)
        x1, x2 = np.meshgrid(x1, x2)
        z = interp(x1, x2)

        if self.plotterType == 'lines layers':
            self.plot_contour(x1, x2, z, colormap=colormap, linewidths=linewidths, levels=levels)
        elif self.plotterType == 'surface':
            self.plot_surface(x1, x2, z, colormap=colormap, transparency=transparency)

    def plot_by_points(self, points, values, colormap=plt.cm.viridis, transparency=0.9, linewidths=1, levels=25):
        if self.plotterType == 'lines layers':
            self.plot_contour_pulling_on_points(np.array(points)[:, self.indexes[0]], np.array(points)[:, self.indexes[1]], values,
                                                colormap, linewidths, levels, transparency)
        elif self.plotterType == 'surface':
            self.plot_surface_pulling_on_points(np.array(points)[:, self.indexes[0]], np.array(points)[:, self.indexes[1]], values,
                                                colormap, transparency)

    def plot_points(self, points, values, clr='blue', mrkr='o', mrkrs=3):
        if self.plotterType == 'lines layers':
            self.ax.scatter(np.array(points)[:, self.indexes[0]], np.array(points)[:, self.indexes[1]], color=clr, marker=mrkr, s=mrkrs, zorder=2)
        elif self.plotterType == 'surface':
            if self.objective_function_type == 'by points':
                self.ax.scatter(np.array(points)[:, self.indexes[0]], np.array(points)[:, self.indexes[1]], values,
                                s=mrkrs, color=clr, marker=mrkr, alpha=0.2)
            else:
                self.ax.scatter(np.array(points)[:, self.indexes[0]], np.array(points)[:, self.indexes[1]], values,
                                s=mrkrs, color=clr, marker=mrkr, alpha=1.0)
