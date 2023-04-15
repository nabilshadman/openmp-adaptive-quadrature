#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <omp.h>
#include "function.h"

#define MAXQUEUE 10000

struct Interval
{
    double left;    // left boundary
    double right;   // right boundary
    double tol;     // tolerance
    double f_left;  // function value at left boundary
    double f_mid;   // function value at midpoint
    double f_right; // function value at right boundary
};

struct Queue
{
    struct Interval entry[MAXQUEUE]; // array of queue entries
    int top;                         // index of last entry
};

// add an interval to the queue
void enqueue(struct Interval interval, struct Queue *queue_p)
{

    if (queue_p->top == MAXQUEUE - 1)
    {
        printf("Maximum queue size exceeded - exiting\n");
        exit(1);
    }
    queue_p->top++;
    queue_p->entry[queue_p->top].left = interval.left;
    queue_p->entry[queue_p->top].right = interval.right;
    queue_p->entry[queue_p->top].tol = interval.tol;
    queue_p->entry[queue_p->top].f_left = interval.f_left;
    queue_p->entry[queue_p->top].f_mid = interval.f_mid;
    queue_p->entry[queue_p->top].f_right = interval.f_right;
}

// extract last interval from queue
struct Interval dequeue(struct Queue *queue_p)
{

    if (queue_p->top == -1)
    {
        printf("Attempt to extract from empty queue - exiting\n");
        exit(1);
    }

    struct Interval interval;

    interval.left = queue_p->entry[queue_p->top].left;
    interval.right = queue_p->entry[queue_p->top].right;
    interval.tol = queue_p->entry[queue_p->top].tol;
    interval.f_left = queue_p->entry[queue_p->top].f_left;
    interval.f_mid = queue_p->entry[queue_p->top].f_mid;
    interval.f_right = queue_p->entry[queue_p->top].f_right;
    queue_p->top--;

    return interval;
}

// initialise queue
void init(struct Queue *queue_p)
{
    queue_p->top = -1;
}

// return whether queue is empty
int isempty(struct Queue *queue_p)
{
    return (queue_p->top == -1);
}

// get current number of queue entries
int size(struct Queue *queue_p)
{
    return (queue_p->top + 1);
}

double simpson(double (*func)(double), struct Queue *queue_p)
{
    double quad = 0.0;

    // keep working until queue is empty and there are no active intervals

    // track active intervals in a shared variable
    int active_intervals = 0;

    // create locks to use for synchronisation
    omp_lock_t lock_queue_p;
    omp_lock_t lock_active_intervals;
    omp_lock_t lock_quad;

    // initialise locks
    omp_init_lock(&lock_queue_p);
    omp_init_lock(&lock_active_intervals);
    omp_init_lock(&lock_quad);

    // enclose do while loop in parallel region and
    // allow all threads to enqueue and dequeue intervals
    #pragma omp parallel default(none) \
    shared(queue_p, active_intervals, quad, func, lock_queue_p, lock_active_intervals, lock_quad)
    {
        do
        {
            // already have function values at left and right boundaries and midpoint
            // now evaluate function at one-qurter and three-quarter points
            struct Interval interval;

            // initialise interval with arbitrary values to check occurrence of dequeue later
            interval.left = 5000.0;
            interval.right = 5000.0;
            interval.tol = 5000.0;
            interval.f_left = 5000.0;
            interval.f_mid = 5000.0;
            interval.f_right = 5000.0;

            // synchronise accesses to queue with lock
            omp_set_lock(&lock_queue_p);
            if (!isempty(queue_p))
            {
                interval = dequeue(queue_p);
            }
            omp_unset_lock(&lock_queue_p);

            // check if dequeue occurred above or else continue to next iteration
            if (interval.left == 5000.0 && interval.right == 5000.0)
            {
                continue;
            }

            // increment active intervals
            // synchronise accesses to active intervals counter with lock
            omp_set_lock(&lock_active_intervals);
            active_intervals++;
            omp_unset_lock(&lock_active_intervals);

            double h = interval.right - interval.left;
            double c = (interval.left + interval.right) / 2.0;
            double d = (interval.left + c) / 2.0;
            double e = (c + interval.right) / 2.0;
            double fd = func(d);
            double fe = func(e);

            // calculate integral estimates using 3 and 5 points respectively
            double q1 = h / 6.0 * (interval.f_left + 4.0 * interval.f_mid + interval.f_right);
            double q2 = h / 12.0 * (interval.f_left + 4.0 * fd + 2.0 * interval.f_mid + 4.0 * fe + interval.f_right);

            if ((fabs(q2 - q1) < interval.tol) || ((interval.right - interval.left) < 1.0e-12))
            {
                // tolerance is met, add to total
                // synchronise accesses to quadrature with lock
                omp_set_lock(&lock_quad);
                quad += q2 + (q2 - q1) / 15.0;
                omp_unset_lock(&lock_quad);

                // decrement active intervals by 1
                // synchronise accesses to active intervals with lock
                omp_set_lock(&lock_active_intervals);
                active_intervals--;
                omp_unset_lock(&lock_active_intervals);
            }
            else
            {
                // tolerance is not met, split interval in two and add both halves to queue
                struct Interval i1, i2;

                i1.left = interval.left;
                i1.right = c;
                i1.tol = interval.tol;
                i1.f_left = interval.f_left;
                i1.f_mid = fd;
                i1.f_right = interval.f_mid;

                i2.left = c;
                i2.right = interval.right;
                i2.tol = interval.tol;
                i2.f_left = interval.f_mid;
                i2.f_mid = fe;
                i2.f_right = interval.f_right;

                // synchronise accesses to queue with lock
                omp_set_lock(&lock_queue_p);
                enqueue(i1, queue_p);
                enqueue(i2, queue_p);
                omp_unset_lock(&lock_queue_p);
            }
        } while (!isempty(queue_p) && active_intervals > 0);
    }

    // destroy locks
    omp_destroy_lock(&lock_queue_p);
    omp_destroy_lock(&lock_active_intervals);
    omp_destroy_lock(&lock_quad);

    return quad;
}

int main(void)
{

    struct Queue queue;
    struct Interval whole;

    // initialise queue
    init(&queue);

    double start = omp_get_wtime();

    // add initial interval to the queue
    whole.left = 0.0;
    whole.right = 10.0;
    whole.tol = 1e-06;
    whole.f_left = func1(whole.left);
    whole.f_right = func1(whole.right);
    whole.f_mid = func1((whole.left + whole.right) / 2.0);

    enqueue(whole, &queue);

    // call queue-based quadrature routine
    double quad = simpson(func1, &queue);
    double time = omp_get_wtime() - start;
    printf("Result = %e\n", quad);
    printf("Time(s) = %f\n", time);
}
