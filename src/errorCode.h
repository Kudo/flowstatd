#ifndef _ERROR_CODE_H_
#define _ERROR_CODE_H_

#define S_OK		    0
#define E_FAILED	    1
#define	E_NO_DATA	    2
#define	E_INVALID_PARAM	    3
#define	E_INVALID_COMMAND   4

const char *const errMsgList[] = {
    "Success",		    // S_OK
    "Generic error",	    // E_FAILED
    "No data",		    // E_NO_DATA
    "Invalid parameters",   // E_INVALID_PARAM
    "Invalid command",	    // E_INVALID_COMMAND
};

#define SET_JSON_RET_INFO(jsonResp, errorCode)   { \
    json_object_set_new(jsonResp, "respCode", json_integer(errorCode)); \
    json_object_set_new(jsonResp, "respMsg", json_string(errMsgList[errorCode])); \
}

#endif	// _ERROR_CODE_H_
