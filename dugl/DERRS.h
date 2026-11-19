#ifndef DERRS_H_INCLUDED
#define DERRS_H_INCLUDED

#define DG_ERRS_SYSTEM_INIT_FAIL                1
#define DG_ERSS_DWORKERS_INIT_FAIL              2
#define DG_ERSS_EVENT_MUTEX_INIT_FAIL           3
#define DG_ERSS_NO_MEM                          4
#define DG_ERSS_FAIL_CREATE_SYSTEM_RGB_SURF     5
#define DG_ERSS_FAIL_QUERY_DISPLAY_MODE         6
#define DG_ERSS_WINDOW_CREATION_FAIL            7
#define DG_ERSS_WINDOW_SURFACE_CREATION_FAIL    8
#define DG_ERSS_INVALID_DGSURF_FORMAT           9
#define DG_ERSS_FILE_NOT_FOUND                  10
#define DG_ERSS_INVALID_BMFONT_DESC_FORMAT      11

#ifdef __cplusplus
extern "C" {
#endif

extern int              dgLastErrID;

// Get Last DUGL Errors ID, 0 if none
int DgGetLastErr();

#ifdef __cplusplus
}
#endif


#endif // DERRS_H_INCLUDED
