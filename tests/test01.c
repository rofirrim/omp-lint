int f(void)
{
    int x, y, z;
#pragma omp task shared(x) firstprivate(y) private(z)
    {
        x = 1;
        y = 2;
        z = 3;
    }
#pragma omp taskwait

    return x + y + z;
}
