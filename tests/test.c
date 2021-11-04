#include <locale.h>
#include <pthread.h>

#include <glib.h> // For tests

#include "../src/courier.h"

/*-------------------------------------------------------------------------------------------------
 *
 *                                  Constructor / Destructor
 *
 *-----------------------------------------------------------------------------------------------*/
static void
test_create_destroy(void)
{
    Courier cr = courier_new();
    courier_destroy(&cr);
}

/*-------------------------------------------------------------------------------------------------
 *
 *                                     Single Threaded
 *
 *-----------------------------------------------------------------------------------------------*/
static void
test_single_threaded(void)
{
    Courier cr = courier_new();
    courier_open(&cr);

    for (size_t i = 1; i <= COURIER_QUEUE_SIZE; ++i) {
        courier_send(&cr, (void *)i);
    }

    courier_close(&cr);

    courier_wait_until_ready(&cr);

    void *ptr = 0;
    size_t count = 0;
    size_t prev_i = 0;
    while ((ptr = courier_receive(&cr))) {
        size_t i = (size_t)ptr;

        count += 1;
        g_assert_cmpint(i, ==, count);
        g_assert_cmpint(prev_i, <, i);
        prev_i = i;
    }

    g_assert_cmpint(count, ==, COURIER_QUEUE_SIZE);

    courier_destroy(&cr);
}

/*-------------------------------------------------------------------------------------------------
 *
 *                                Single Producer Multiple Consumer
 *
 *-----------------------------------------------------------------------------------------------*/
static void *
producer(void *arg)
{
    Courier *cr = arg;

    courier_open(cr);

    for (size_t i = 1; i <= 1000000; ++i) {
        courier_send(cr, (void *)i);
    }

    courier_close(cr);

    return 0;
}

static void *
multiple_consumer_from_single_producer(void *arg)
{
    Courier *cr = arg;

    size_t max_val = 0;
    size_t count = 0;

    courier_wait_until_ready(cr);

    void *ptr = 0;
    size_t prev_val = 0;
    while ((ptr = courier_receive(cr))) {
        size_t val = (size_t)ptr;
        max_val = val > max_val ? val : max_val;
        count += 1;

        g_assert_cmpint(count, <=, 1000000);
        g_assert_cmpint(max_val, <=, 1000000);
        g_assert_cmpint(prev_val, <, val);

        prev_val = val;
    }

    fprintf(stdout, "     %zu recieved, max value received %zu\n", count, max_val);

    g_assert_cmpint(count, <=, 1000000);
    g_assert_cmpint(max_val, <=, 1000000);

    return 0;
}

static void
test_single_producer_multiple_consumer(void)
{
    Courier cr = courier_new();

    pthread_t prod_thread = {0};
    pthread_create(&prod_thread, 0, producer, &cr);

    pthread_t consumers[4] = {0};
    for (unsigned int i = 0; i < sizeof(consumers) / sizeof(consumers[0]); ++i) {
        pthread_create(&consumers[i], 0, multiple_consumer_from_single_producer, &cr);
    }

    pthread_join(prod_thread, 0);
    for (unsigned int i = 0; i < sizeof(consumers) / sizeof(consumers[0]); ++i) {
        pthread_join(consumers[i], 0);
    }

    courier_destroy(&cr);
}

/*-------------------------------------------------------------------------------------------------
 *
 *                                Multiple Producer Single Consumer
 *
 *-----------------------------------------------------------------------------------------------*/
// Reuse producer test function from above.

static void *
single_consumer(void *arg)
{
    Courier *cr = arg;

    size_t max_val = 0;
    size_t count = 0;

    courier_wait_until_ready(cr);

    void *ptr = 0;
    size_t prev_val = 0;
    size_t num_down_counts = 0;
    while ((ptr = courier_receive(cr))) {
        size_t val = (size_t)ptr;
        max_val = val > max_val ? val : max_val;
        count += 1;

        g_assert_cmpint(count, <=, 4 * 1000000);
        g_assert_cmpint(max_val, <=, 1000000);

        if (val < prev_val) {
            num_down_counts += 1;
        }

        prev_val = val;
    }

    fprintf(stdout, "     %zu recieved, max value received %zu\n", count, max_val);
    fprintf(stdout,
            "     %zu downcounts observed. That's OK, order is not guaranteed in this case.\n",
            num_down_counts);

    g_assert_cmpint(count, <=, 4 * 1000000);
    g_assert_cmpint(max_val, <=, 1000000);

    return 0;
}

static void
test_multiple_producer_single_consumer(void)
{
    Courier cr = courier_new();

    pthread_t producers[4] = {0};
    for (unsigned int i = 0; i < sizeof(producers) / sizeof(producers[0]); ++i) {
        pthread_create(&producers[i], 0, producer, &cr);
    }

    pthread_t consumer_thread = {0};
    pthread_create(&consumer_thread, 0, single_consumer, &cr);

    pthread_join(consumer_thread, 0);
    for (unsigned int i = 0; i < sizeof(producers) / sizeof(producers[0]); ++i) {
        pthread_join(producers[i], 0);
    }

    courier_destroy(&cr);
}

/*-------------------------------------------------------------------------------------------------
 *
 *                               Multiple Producer Multiple Consumer
 *
 *-----------------------------------------------------------------------------------------------*/
// Reuse producer test function from above.

static void *
multiple_consumer(void *arg)
{
    Courier *cr = arg;

    size_t max_val = 0;
    size_t count = 0;

    courier_wait_until_ready(cr);

    void *ptr = 0;
    while ((ptr = courier_receive(cr))) {
        size_t val = (size_t)ptr;
        max_val = val > max_val ? val : max_val;
        count += 1;

        g_assert_cmpint(count, <=, 4 * 1000000);
        g_assert_cmpint(max_val, <=, 1000000);
    }

    fprintf(stdout, "     %zu recieved, max value received %zu\n", count, max_val);

    g_assert_cmpint(count, <=, 4 * 1000000);
    g_assert_cmpint(max_val, <=, 1000000);

    return 0;
}

static void
test_multiple_producer_multiple_consumer(void)
{
    Courier cr = courier_new();

    pthread_t producers[4] = {0};
    for (unsigned int i = 0; i < sizeof(producers) / sizeof(producers[0]); ++i) {
        pthread_create(&producers[i], 0, producer, &cr);
    }

    pthread_t consumers[4] = {0};
    for (unsigned int i = 0; i < sizeof(consumers) / sizeof(consumers[0]); ++i) {
        pthread_create(&consumers[i], 0, multiple_consumer, &cr);
    }

    for (unsigned int i = 0; i < sizeof(producers) / sizeof(producers[0]); ++i) {
        pthread_join(producers[i], 0);
    }

    for (unsigned int i = 0; i < sizeof(consumers) / sizeof(consumers[0]); ++i) {
        pthread_join(consumers[i], 0);
    }

    courier_destroy(&cr);
}

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

    g_test_add_func("/courier/create_destroy", test_create_destroy);
    g_test_add_func("/courier/single_threaded", test_single_threaded);
    g_test_add_func("/courier/single_producer_multiple_consumer",
                    test_single_producer_multiple_consumer);
    g_test_add_func("/courier/multiple_producer_single_consumer",
                    test_multiple_producer_single_consumer);
    g_test_add_func("/courier/multiple_producer_multiple_consumer",
                    test_multiple_producer_multiple_consumer);

    //
    // Run tests
    //
    return g_test_run();
}
