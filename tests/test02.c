void f(int z)
{
    int x, y;

#pragma omp task depend(out:x)
    {
        x = 3;
    }

#pragma omp task depend(in:x) depend(out:y)
    {
        y = x + 1;
    }

#pragma omp taskwait
}
