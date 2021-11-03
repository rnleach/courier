#include <locale.h>

#include <glib.h> // For tests

#include "../src/courier.h"

/*-------------------------------------------------------------------------------------------------
 *
 *                                      Main Test Runner
 *
 *-----------------------------------------------------------------------------------------------*/
int
main(int argc, char *argv[static 1])
{
    setlocale(LC_ALL, "");

    g_test_init(&argc, &argv, NULL);

    //
    // Run tests
    //
    return g_test_run();
}
