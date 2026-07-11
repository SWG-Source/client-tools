/* Compat shim: old Perforce libsupp.lib references global `_tzname` array
   (decorated to `__tzname` after MSVC C-linkage underscore prepending).
   Modern UCRT only exposes `_tzname` via the `_get_tzname()` function, not
   as a global symbol. Provide our own static fallback. */
char *_tzname[2] = { (char *)"GMT", (char *)"GMT" };
long _timezone = 0;
int _daylight = 0;
