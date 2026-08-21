#ifndef __GLOBALIZER_API_H__
#define __GLOBALIZER_API_H__

#if defined(_WIN32) || defined(_WIN64)
#ifdef GLOBALIZER_EXPORTS
#define GLOBALIZER_API __declspec(dllexport)
#elif defined(GLOBALIZER_STATIC)
#define GLOBALIZER_API
#else
#define GLOBALIZER_API __declspec(dllimport)
#endif
#else
#define GLOBALIZER_API __attribute__((visibility("default")))
#endif

#endif // __GLOBALIZER_API_H__