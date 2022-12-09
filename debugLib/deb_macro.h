#pragma once 

#ifdef DEBUG_MODE
  
  #define SAFE_PRINTF(...) fprintf(stderr, __VA_ARGS__)

  #define PRINT_LINE {                                \
    if (DEBUG_MODE)                                   \
      SAFE_PRINTF("[%s:%d]\n", __func__, __LINE__);   \
  }
  #define print_debug(code)   \
  {                           \
    if (DEBUG_MODE)           \
      {code;}                 \
  }
  
  #define IS_VALID(param) {             \
  if (DEBUG_MODE && !param)             \
  {                                     \
    printf("Invalid " #param "ptr ");   \
    return -1;                          \
  }                                     \
}

#endif
