#ifndef __DEFINES_H__
#define __DEFINES_H__


/* ======================================================================== *\
**  Constants                                                               **
\* ======================================================================== */

/// Максимальная размерность
#ifdef EXAMIN_MAX_DIMENSION
#define MAX_TRIAL_DIMENSION EXAMIN_MAX_DIMENSION
#else
#define MAX_TRIAL_DIMENSION 200
#endif

// Максимальное количество глобальных минимумов
#define MAX_NUM_MIN 20
// Если константа определена то на GPU будет использоваться double иначе float
#define CUDA_VALUE_DOUBLE_PRECISION
/// Если константа определена то на СPU будет использоваться double иначе float

#define CPU_VALUE_DOUBLE_PRECISION

/**
Данный параметр определяет уровень, меньше которого константа Липшица считается нулевой.

Проблема возникновения близкой к нулю оценки константы может возникнуть, если функция принимает
одинаковые значения во многих точках (например, функция Растригина).
*/
#define _M_ZERO_LEVEL 1e-12


/* ======================================================================== *\
**  Types                                                                   **
\* ======================================================================== */
/// Используемый тип данных для вычислений на GPU
#ifdef CUDA_VALUE_DOUBLE_PRECISION
#define CUDA_VALUE double
#else
#define CUDA_VALUE float
#endif
/// Используемый тип данных для вычислений на центральном процессоре(не доделано),
#ifdef CPU_VALUE_DOUBLE_PRECISION
#define CPU_VALUE double
#else
#define CPU_VALUE float
#endif
/// Массив целых чисел
#define ints int*
/// Массив действительных чисел
#define doubles double*
/// Тип флага
#define FLAG bool

/* ======================================================================== *\
**  Defines                                                                 **
\* ======================================================================== */

/* ======================================================================== *\
**  Параметры спецификаторы и прочее,  для разных версий компиляторов       **
\* ======================================================================== */

#ifdef _GPU_CUDA_ ///Для компиляции под CUDA

#define perm 1
#define SPECIFIER __shared__
#define ARRAY_SPECIFIER __constant__
#define concatenation cuda
#define F_DEVICE __device__
#define parameter_const
#define OBJECTIV_TYPE CUDA_VALUE
#define GET_FUNCTION_PARAMETERS OBJECTIV_TYPE* x, OBJECTIV_TYPE* f
#define FUNCTION_CALCOLATION_PREF (x)

#define GKLS_VARIABLES_SPECIFIER __shared__
//Формирование констант правильной точности
#ifdef CUDA_VALUE_DOUBLE_PRECISION
#define PRECISION(x) x##0
#else
#define PRECISION(x) x##f
#endif

#else ///Для компиляции под VS


#define SPECIFIER extern
#define ARRAY_SPECIFIER extern
#define concatenation
#define F_DEVICE inline
#define OBJECTIV_TYPE CPU_VALUE
#define GET_FUNCTION_PARAMETERS tFunction* f
#define FUNCTION_CALCOLATION_PREF

#define GKLS_VARIABLES_SPECIFIER extern
//Формирование констант правильной точности
#ifdef CPU_VALUE_DOUBLE_PRECISION
#define PRECISION(x) x##0
#else
#define PRECISION(x) x##f
#endif

#endif

/* ======================================================================== *\
**  Служебные                                                               **
\* ======================================================================== */
#ifndef GLOBALIZER_MAX
#define GLOBALIZER_MAX(a,b) ((a) > (b) ? a : b)
#endif

#ifndef GLOBALIZER_MIN
#define GLOBALIZER_MIN(a,b) ((a) < (b) ? a : b)
#endif

/// Объединить два слова
#define CAT(x, y) x##y
/// Объединить четыре слова
#define CAT4(a, b, c, d) a##b##c##d
/// Объединяет два слова в обратномпорядке
#define CONCATENATION2(name, console) CAT(console,name)
/** Добавляет префикс concatenation
(см. Параметры спецификаторы и прочее,  для разных версий компиляторов  )
*/
#define CONCATENATION(name) CONCATENATION2(name, concatenation)
/// Превращает слово в комментарий
#define CAT_COM(x) //##x

/// Добавляет префикс P к имени типа, что соответствует перечислению EParameterType
#define ParType(type) P##type
/// Добавляет префикс link к имени
#define LinkParameter(name) link##name
/// Добавляет префикс com к имени
#define ComParameter(name) com##name
/// Добавляет префикс help к имени
#define HelpParameter(name) help##name
/// Добавляет префикс inc к имени
#define IncParameter(name) inc##name
/// Переводит слово bar в строку типа char*
#define make_str(bar) # bar
/// Добавляет префикс IS_ к имени
#define IsChange(name) IS_##name

/// Инициализирует параметр из класса TParameters
#define InitParameter(type, name, defVal, com, help, sizeVal)                            \
IsChange(name) = false;                                                                  \
Inc(ParType(name), ParType(type), LinkParameter(name), com,                              \
  HelpParameter(name), help, (void*)(&name), make_str(defVal),                           \
  make_str(name), &IsChange(name), IncParameter(name), make_str(name), sizeVal);

#define OWNER_NAME Owner

/// Инициализирует параметр из класса Параметров
#define InitOption(name, defVal, com, help, sizeVal)                           \
  InitializationOption((TBaseProperty<OWNER_NAME>*)(&name), make_str(name), make_str(defVal), com, help, sizeVal);

/* ======================================================================== *\
**  Директиы для объявления переменных                                      **
\* ======================================================================== */

/** простое объявление переменной
name - имя переменной
type - тип переменной
specifier - спецификатор
*/
#define VARIABLES(name, type, specifier) specifier type name
/** Объявление переменной со спецификатором GKLS_VARIABLES_SPECIFIER
(см. Параметры спецификаторы и прочее,  для разных версий компиляторов  )
*/
#define GKLS_VARIABLES(name, type) VARIABLES(name, type, GKLS_VARIABLES_SPECIFIER)
/** Объявление переменной со спецификатором SPECIFIER
(см. Параметры спецификаторы и прочее,  для разных версий компиляторов  )
*/
#define CONSTANT_VARIABLES(name, type) SPECIFIER type name
/// Объявляет функцию для инициализации значения по умолчанию
#define NEW_FUNC_DEF(name) CONCATENATION(name##_func_def())
/// Создает статический массив
#define STATIC_ARRAY(type, name, count) type name[count]
/** Создает статический массив со спецификатором ARRAY_SPECIFIER
(см. Параметры спецификаторы и прочее,  для разных версий компиляторов  )
*/
#define NEW_ARRAY(type, name, count) ARRAY_SPECIFIER STATIC_ARRAY(type, name, count)
/** Создает статический массив размером MAX_TRIAL_DIMENSION типа OBJECTIV_TYPE со спецификатором ARRAY_SPECIFIER
(см. Параметры спецификаторы и прочее,  для разных версий компиляторов  )
*/
#define NEW_ARRAY_MAX_SIZE(name) NEW_ARRAY(OBJECTIV_TYPE, name, MAX_TRIAL_DIMENSION)
/** Создает переменнут типа OBJECTIV_TYPE со спецификатором SPECIFIER
(см.Параметры спецификаторы и прочее, для разных версий компиляторов)
*/
#define FLOAT_VARIABLES(name) SPECIFIER OBJECTIV_TYPE name


/**Глобальная переменная объявляемая в файле с задачей, могут быть переопеределены в параметрах,
!!!В problems.cpp требуется повторное объявление: type name;!!!
type - тип
name - имя
val - значение по умолчанию
*/
#define NEW_VARIABLES(type, name, val) CONSTANT_VARIABLES(name, type);\
 F_DEVICE type NEW_FUNC_DEF(name)\
  {\
  name = val;\
return name;\
}\

/// Объявляет переменные для создания параметра класса TParameters
#define CreateParameter(type, name)       \
public: type name;                        \
public: FLAG IsChange(name);              \
protected: int IncParameter(name);        \
protected: EParameterType ParType(name);  \
protected: std::string LinkParameter(name);    \
protected: std::string HelpParameter(name);

/// Базовые переопределения для классов типов данные
#define BasicMethods(ClassType, Type)                                             \
  virtual void operator =(Type data) {TTypedProperty<Type, Owner>::operator=(data);}                \
  virtual void operator=(ClassType<Owner>& data) {TParameterProperty<Type, Owner>::operator=(data);}     \
  virtual void operator = (ClassType<Owner>* data) {TParameterProperty<Type, Owner>::operator=(data);}  \
  virtual std::string ToString() {return operator std::string();}                           \
  virtual void FromString(std::string val) {operator = (val);}                           \
  virtual void operator = (TBaseProperty<Owner>& data) { TParameterProperty<Type, Owner>::operator=((TParameterProperty<Type, Owner>*)(& data)); } \
  virtual void operator=(char* data) {operator = (std::string(data));}



/**Объявление Set и Get функции c именем
Get##N возвращает тип T
и Set##N параметр value типа T
*/
#define PROPERTY(T, N)     \
  T Get ## N() const;     \
  void Set ## N(T value);

/* ======================================================================== *\
**  Прочие константы                                                        **
\* ======================================================================== */

#define PI PRECISION(3.14159265359)







#endif
