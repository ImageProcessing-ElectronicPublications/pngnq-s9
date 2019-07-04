/* error.h
 * Error handling for pngnq 
 */

/* Error codes */
#define PNGNQ_ERR_NONE 0
#define PNGNQ_ERR_ 0

#define PNGNQ_ERROR(...) (fprintf(stderr,\
    "pngnq - Error in %s near line %d :\n",__FILE__,__LINE__));\
    fprintf(stderr, __VA_ARGS__);\
    fflush(stderr);

#define PNGNQ_WARNING(...)                                \
    do {                                                  \
        fprintf(stderr, "pngnq - Warning: " __VA_ARGS__); \
        fflush(stderr);                                   \
    } while (0)

#define PNGNQ_MESSAGE(...) {if(verbose) {fprintf(stderr,__VA_ARGS__);fflush(stderr);}}
