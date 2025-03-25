#include <omp.h>
#include <iostream>

using namespace std;

static long num_steps = 1000000;

#define NUM_THREADS 12
double piParallel()
{

    double step, x, pi = 0, sum = 0;
    step = 1 / (double)num_steps;
    omp_set_num_threads(NUM_THREADS);
#pragma omp parallel for reduction(+ : sum)
    for (int i = 0; i < num_steps; i++)
    {
        x = (i + 0.5) * step;
        sum += 4 / (1 + (x * x));
    }
    pi = sum * step;
    return pi;
}

double piPar_Race()
{
    int i, tthreads, id;

    double step, pi = 0.0, sum = 0.0, x;

    step = 1.0 / (double)num_steps;

#pragma omp parallel
    {
        tthreads = omp_get_num_threads();
        id = omp_get_thread_num();
        for (i = id; i < num_steps; i += tthreads)
        {
            x = (i + 0.5) * step;
            sum = sum + 4.0 / (1.0 + x * x);
        }
    }
    pi = step * sum;
    return pi;
}
pair<double, int> piPar_false()
{
    int nthreads;
    double pi, sum[NUM_THREADS];
    double step = 1.0 / (double)num_steps;
    omp_set_num_threads(NUM_THREADS);
#pragma omp parallel
    {
        int i, id, tthreads;
        double x;
        tthreads = omp_get_num_threads();
        id = omp_get_thread_num();
        if (id == 0)
            nthreads = tthreads;
        for (i = id, sum[id] = 0.0; i < num_steps; i = i + tthreads)
        {
            x = (i + 0.5) * step;
            sum[id] = sum[id] + 4.0 / (1.0 + x * x);
        }
    }
    for (int i = 0, pi = 0.0; i < nthreads; i++)
    {
        pi += step * sum[i];
    }
    return make_pair(pi, nthreads);
}
double piSequential()
{
    double step, x, pi = 0, sum = 0;
    step = 1 / (double)num_steps;
    for (int i = 0; i < num_steps; i++)
    {
        x = (i + 0.5) * step;
        sum += 4 / (1 + (x * x));
    }
    pi = sum * step;
    return pi;
}

int main()
{

    int num_of_threads = piPar_false().second;
    double SeqStart = omp_get_wtime();
    double Sequential = piSequential();
    double SeqStop = omp_get_wtime();
    cout << "Sequential: pi with " << num_steps << " steps is " << Sequential << " in " << (SeqStop - SeqStart) << " seconds" << endl;
    double Seqtime = SeqStop - SeqStart;
    cout << endl;

    double falseStart = omp_get_wtime();
    double piFalse = piPar_false().first;
    double falseStop = omp_get_wtime();

    cout << "Parallel with false sharing: pi with " << num_steps << " steps is " << piFalse << " in " << (falseStop - falseStart) << " seconds" << " using " << num_of_threads << " threads" << endl;
    cout << "Speedup: " << Seqtime / (falseStop - falseStart) << endl;
    cout << endl;

    double RaceStart = omp_get_wtime();
    double Race = piPar_Race();
    double RaceStop = omp_get_wtime();

    cout << "Parallel with race condition: pi with " << num_steps << " steps is " << Race << " in " << (RaceStop - RaceStart) << " seconds" << " using " << num_of_threads << " threads" << endl;
    cout << "Speedup: " << Seqtime / (RaceStop - RaceStart) << endl;
    cout << endl;

    double ParStart = omp_get_wtime();
    double Parallel = piParallel();
    double ParStop = omp_get_wtime();
    cout << "Parallel with no race and parallel: pi with " << num_steps << " steps is " << Parallel << " in " << (ParStop - ParStart) << " seconds" << " using " << num_of_threads << " threads" << endl;
    cout << "Speedup: " << Seqtime / (ParStop - ParStart) << endl;
    return 0;
}