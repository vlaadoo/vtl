#ifndef _VTL_APP_RESULT_H
#define _VTL_APP_RESULT_H

#ifdef __cplusplus
extern "C"
{
#endif



typedef enum _VTL_AppResult 
{
    VTL_res_kOk = 0,
    
    
    VTL_res_video_fs_r_kMissingFileErr = 1,
    VTL_res_video_fs_r_kFileIsBusyErr,

    VTL_res_video_fs_w_kFileIsBusyErr = 10,

    // Общие ошибки
    VTL_res_kUnsupportedFormat = 20,
    VTL_res_kParseError = 21,
    VTL_res_kMemoryError = 22,
    VTL_res_kArgumentError = 23,
    VTL_res_kIOError = 24,
    VTL_res_kUnknownError = 25,

    // Новые коды для субтитров и универсального API
    VTL_res_kNullArgument = 100,
    VTL_res_kAllocError = 101,
    VTL_res_kEndOfFile = 102,
    VTL_res_kInvalidIndex = 103
} VTL_AppResult;




#ifdef __cplusplus
}
#endif


#endif