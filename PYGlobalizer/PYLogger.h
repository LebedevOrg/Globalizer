/////////////////////////////////////////////////////////////////////////////
//                                                                         //
//             LOBACHEVSKY STATE UNIVERSITY OF NIZHNY NOVGOROD             //
//                                                                         //
//  File:      PYLogger.h                                                  //
//                                                                         //
//  Purpose:   Лёгкий уровневый логгер для Python-интерфейса Globalizer.   //
//             Улучшение №6: убираем безусловный std::cout из горячего     //
//             кода. Всё пишется в stderr, чтобы stdout оставался чистым    //
//             для данных и пайплайнов.                                    //
//                                                                         //
/////////////////////////////////////////////////////////////////////////////

#pragma once
#include <iostream>
#include <string>

namespace pyglob
{

  /// Уровни логирования. Чем выше — тем подробнее.
  enum class LogLevel
  {
    Silent = 0,  ///< ничего не печатать
    Error = 1,  ///< только ошибки (по умолчанию)
    Info = 2,  ///< ход решения: создание задачи, старт/финиш солвера
    Debug = 3   ///< детальная отладка: значения в точках, тайминги
  };

  /// Глобальный текущий уровень (одна копия на процесс, инициализируется лениво).
  inline LogLevel& GlobalLogLevel()
  {
    static LogLevel level = LogLevel::Error;
    return level;
  }

  inline void SetLogLevel(LogLevel level) { GlobalLogLevel() = level; }

  /// Перегрузка для удобного проброса int из Python (0..3).
  inline void SetLogLevel(int level)
  {
    if (level < 0) level = 0;
    if (level > 3) level = 3;
    GlobalLogLevel() = static_cast<LogLevel>(level);
  }

  inline int GetLogLevel() { return static_cast<int>(GlobalLogLevel()); }

} // namespace pyglob

// Потоковые макросы: аргумент можно писать через <<,
//   PY_LOG_INFO("dim = " << dim << ", crit = " << nCrit);
// Проверка уровня — до вычисления выражения, поэтому в Silent-режиме
// оверхед нулевой.
#define PY_LOG(level, expr)                                                    \
  do {                                                                         \
    if (static_cast<int>(level) <= ::pyglob::GetLogLevel()) {                  \
      std::cerr << expr << std::endl;                                          \
    }                                                                          \
  } while (0)

#define PY_LOG_ERROR(expr) PY_LOG(::pyglob::LogLevel::Error, expr)
#define PY_LOG_INFO(expr)  PY_LOG(::pyglob::LogLevel::Info,  expr)
#define PY_LOG_DEBUG(expr) PY_LOG(::pyglob::LogLevel::Debug, expr)
// - end of file ----------------------------------------------------------------------------------
