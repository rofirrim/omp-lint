
void f(int z)
{
    int x, y, k;

#pragma omp task depend(out:x, k)
    {
        x = 3;
        k = 4;
    }

#pragma omp task depend(in:x, k) depend(out:y, z)
    {
        y = x + 1 + k;
        z = y + 2;
    }

#pragma omp taskwait
}
