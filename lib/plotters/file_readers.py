def ReadTrialsFile(dir, file_for_reading):
    with open(dir + file_for_reading) as file:
        iters_count = 0
        sol_x = []
        sol_z = None
        x = []
        z = []
        x_nc = []
        z_nc = []
        x_nce = []
        cc = []

        isFirst = True
        for line in file:
            if isFirst:
                splitedline = line.split(' ')
                iters_count = int(splitedline[0])
                num_of_func = int(splitedline[1])
                isFirst = False
                for i in range(max(1, (num_of_func - 1) * 2)):
                    cc.append([])
                continue
            if iters_count == 0:
                splitedline = line.split(' | ')
                point = splitedline[0].split(' ')
                value = splitedline[1].split(' ')
                for j in range(len(point)):
                    sol_x.append(float(point[j]))
                sol_z = float(value[num_of_func - 1])
                break

            splitedline = line.split(' | ')

            value = splitedline[1].split(' ')
            point = splitedline[0].split(' ')

            xi = []
            for j in range(len(point)):
                xi.append(float(point[j]))

            if (value[0] == '|'):
                x_nce.append(xi)
            elif len(value) >= num_of_func:
                zi = float(value[num_of_func - 1])
                x.append(xi)
                z.append(zi)
            else:
                #zi = float(value[len(value) - 1])
                x_nc.append(xi)
                z_nc.append(0)  # заглушка, лишний параметр


            if (value[0] != '|'):
                i = 0
                while i != len(value) - 1:
                    cc[i * 2].append(xi)
                    cc[i * 2 + 1].append(float(value[i]))
                    i += 1

            iters_count = iters_count - 1

    print("Uncalculate point\'s count: ", len(x_nce))

    return x, z, sol_x, sol_z, x_nc, z_nc, cc, x_nce

def ReadProblemFile(dir, file_for_reading):
    with open(dir + file_for_reading) as file:
        dim = 0
        lb = []
        rb = []
        x = []
        z = []
        c = []
        is_objective_calc = False
        is_constraints_calc = False
        isFirst = True
        isSecond = True

        recalc_count = 0
        prev_val = 0

        for line in file:
            if isFirst:
                splitedline = line.split()
                dim = int(splitedline[0])
                lb_line = splitedline[1].split('_')
                rb_line = splitedline[2].split('_')
                for val in lb_line:
                    lb.append(float(val))
                for val in rb_line:
                    rb.append(float(val))
                isFirst = False
                continue
            if isSecond:
                splitedline = line.split(' ')
                is_objective_calc = bool(int(splitedline[0]))
                is_constraints_calc = bool(int(splitedline[1]))
                isSecond = False
                continue

            if is_objective_calc or is_constraints_calc:  # необязательная проверка, для надежности от пустых строк
                splitedline = line.split(' | ')

                point = splitedline[0].split(' ')

                xi = []
                for j in range(len(point)):
                    xi.append(float(point[j]))
                x.append(xi)

                if is_objective_calc:
                    values = splitedline[len(splitedline) - 1].split(' ')
                    if float(values[0]) < 1.7e308:
                        if recalc_count > 0:
                            next_val = float(values[0])
                            for i in range(recalc_count):
                                z.append((prev_val + next_val) * 0.5)
                            recalc_count = 0
                        z.append(float(values[0]))

                    else:
                        if recalc_count == 0:
                            prev_val = z[-1]
                        recalc_count += 1

                ci = []
                for j in range(1, len(splitedline) - is_objective_calc):
                    ci.append(float(splitedline[j].split(' ')[0]))
                if len(ci) > 0:
                    c.append(ci)

    return dim, lb, rb, x, z, c
