void f(int z)
{
    int x, y;

#pragma omp task private(x) depend(out:x)
    {
        x = 3;
    }

#pragma omp task private(x, y) depend(in:x) depend(out:y)
    {
        y = x + 1;
    }

#pragma omp taskwait
}
