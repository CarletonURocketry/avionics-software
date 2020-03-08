#include <samd21j18a.h>

#include <compiler.h>
// Redefine RAMFUNC as empty
#undef RAMFUNC
#define RAMFUNC


#include <stdlib.h>
#include <stdio.h>

#define UT_PASS (printf("Pass!\n"), 0)
#define UT_FAIL (printf("Fail at %s:%d (in function %s)\n", __FILE__, __LINE__, __func__), 1)
#define ut_assert(x) do { if (!(x)) {exit(UT_FAIL);} } while (0)

