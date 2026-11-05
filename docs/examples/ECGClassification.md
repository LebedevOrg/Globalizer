## Описание задачи

Задача классификации в контексте машинного обучения сводится к отнесению исследуемого объекта к одному из заранее известных классов или категорий. В рамках данной работы решается задача **мультиклассовой классификации** сигналов ЭКГ, при которой каждый сигнал относится только к одному классу.
Более подробную информацию о задаче можно получить в [данном репозитории](https://github.com/OptimLLab/Globalizer_Benchmarks/tree/main/Problems/iOptProblem/ECG_Problems/Classification/README.md)

## Подготовка датасета

Для загрузки обучающего датасета необходимо запустить скрипт DataDownloader.py, расположенный в папке third_party\Problems\Problems\iOptProblem. Запуск скрипта осуществляется через терминал\командную строку по средствам ввода следующей команды:
```bash
python DataDownloader.py -s global -c ECGClassification
```

## Запуск

Запуск системы Globalizer для оптимизации параметров обучения и архитектуры нейросетевой модели осуществляется из папки _bin.
Данная папка генерируется автоматически при инициализации системы. 

Система Globalizer адаптирована для работы как на локальных машинах (персональных компьютерах), так и в инфраструктуре кластера.

### Запуск на персональном компьютере

Запуск на персональном компьютере осуществляется по средством ввода следующей команды:

- В операционной системе семейства Windows:
```bash
GlobalizerMain.exe -lib iOptProblemSimple.dll -mPyFilePath ..\third_party\Problems\Problems\iOptProblem\ECG_Problems\Classification -functionScriptName ECGClassificationProblem -functionClassName ECGClassificationProblem -MaxNumOfPoints 10 -N 2
```
- В операционной системе семейства Linux:
```bash
./GlobalizerMain -lib iOptProblemSimple.so -mPyFilePath ..\third_party\Problems\Problems\iOptProblem\ECG_Problems\Classification -functionScriptName ECGClassificationProblem -functionClassName ECGClassificationProblem -MaxNumOfPoints 10 -N 2
```
### Запуск на кластере

Запуск системы Globalizer в инфраструктуре кластера зависит от операционной системы и системы мониторинга и распределения задач.
Для примера возьмём запуск на кластере "Лобачевский" Нижегородского государственного университета.
Характеристики данного кластера:

- Процессор: 2 x Intel Xeon® Platinum 8558
- Объём оперативной памяти: 2048GB
- Видеоускорители: 8 x Nvidia RTX PRO 6000 Blackwell Server Edition, 96GB GDDR7.
- ОС: CentOS 7.2 Система управления: SLURM
- MPI: Intel MPI, OpenMPI

Пример строки запуска системы может иметь следующий вид:
```bash
srun -p p7gpuRTX6000 -w p7n1 -t 600 -N 1 -c 32 --gres=gpu:1 mpiexec -n 5 ./GlobalizerMain  -lib libiOptProblemSimple.so -functionScriptName ECGClassificationProblem -functionClassName ECGClassificationProblem -MaxNumOfPoints 3 -mPyFilePath ../third_party/Problems/Problems/iOptProblem/ECG_Problems/Classification -N 2 -np 4 -nt 4 -TypeCalculation 2 -calculationsArray 2_0
```