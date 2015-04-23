#ifdef DEBUG
#ifndef debug
#define debug(a) printf("%s",a)
#endif
#else
#define debug(a) 
#endif

extern int hextoint(char a);
extern int errored;
