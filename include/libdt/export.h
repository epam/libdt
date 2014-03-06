#ifndef _LIBDT_DLLDEFINES_H_
#define _LIBDT_DLLDEFINES_H_

#if defined(_WIN32) && defined(LIBDT_DLL_BUILD)
  #if defined(LIBDT_EXPORTS)
    #define  LIBDT_EXPORT __declspec(dllexport)
  #else
    #define  LIBDT_EXPORT __declspec(dllimport)
  #endif /* LIBDT_EXPORTS */
#else /* defined (_WIN32) && defined(LIBDT_DLL_BUILD) */
 #define LIBDT_EXPORT
#endif

#endif /* _LIBDT_DLLDEFINES_H_ */
