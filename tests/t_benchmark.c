
/* < https://opensource.org/licenses/BSD-3-Clause > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

#define TEST_BENCHMARK_LOOP     1000000

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

#if 0
void test60__cosine() {
#endif

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

static volatile double benchmark_dummy;

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

TTT_BEGIN_ALLOW (BenchmarkCosine, 60, "Benchmark - Cosine")

    int i;
    
    t_rand48 seed; PD_RAND48_INIT (seed);

    cos_tilde_initialize();
    
    /* Claude Heiland-Allen's polynomial approximation. */
    
    PD_MEMORY_BARRIER; benchmark_dummy = 0; ttt_timeTrigger();
    
        for (i = 0; i < TEST_BENCHMARK_LOOP; i++) {
        //
        benchmark_dummy += cosf9 ((float)(PD_RAND48_DOUBLE (seed)));
        //
        }
    
    double t1 = ttt_timeTrigger();
    
    /* Miller Puckette's LUT. */
    
    PD_MEMORY_BARRIER; benchmark_dummy = 0; ttt_timeTrigger();
    
        for (i = 0; i < TEST_BENCHMARK_LOOP; i++) {
        //
        benchmark_dummy += dsp_getCosineAtLUT ((double)(PD_RAND48_DOUBLE (seed) * COSINE_TABLE_SIZE));
        //
        }
    
    double t2 = ttt_timeTrigger();
    
    /* Standard function. */
    
    PD_MEMORY_BARRIER; benchmark_dummy = 0; ttt_timeTrigger();
    
        for (i = 0; i < TEST_BENCHMARK_LOOP; i++) {
        //
        benchmark_dummy += cosf ((float)(PD_RAND48_DOUBLE (seed) * PD_TWO_PI));
        //
        }
    
    double t3 = ttt_timeTrigger();
    
    //ttt_stdout (TTT_COLOR_BLUE, "COS: %f", t1);
    //ttt_stdout (TTT_COLOR_BLUE, "LUT: %f", t2);
    //ttt_stdout (TTT_COLOR_BLUE, "STD: %f", t3);
    
    /* Pure Data's LUT is the best? */
    
    PD_ASSERT (t2 < t3);
    PD_ASSERT (t2 < t1);
    
    cos_tilde_release();

TTT_END

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

#if 0
}
#endif

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

#define TEST_BENCHMARK_SQRT     (1 << 16)

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

#if 0
void test61__rsqrt() {
#endif

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

TTT_BEGIN (BenchmarkRsqrt, 61, "Benchmark - rsqrt")

    int i;
    
    t_rand48 seed; PD_RAND48_INIT (seed);

    rsqrt_tilde_initialize();
    
    /* LUT. */
    
    PD_MEMORY_BARRIER; benchmark_dummy = 0; ttt_timeTrigger();
    
        for (i = 0; i < TEST_BENCHMARK_LOOP; i++) {
        //
        float f[4];
        
        f[0] = (float)(PD_RAND48_DOUBLE (seed) * TEST_BENCHMARK_SQRT);
        f[1] = (float)(PD_RAND48_DOUBLE (seed) * TEST_BENCHMARK_SQRT);
        f[2] = (float)(PD_RAND48_DOUBLE (seed) * TEST_BENCHMARK_SQRT);
        f[3] = (float)(PD_RAND48_DOUBLE (seed) * TEST_BENCHMARK_SQRT);
        
        benchmark_dummy += rsqrt_fastLUT (f[0]);
        benchmark_dummy += rsqrt_fastLUT (f[1]);
        benchmark_dummy += rsqrt_fastLUT (f[2]);
        benchmark_dummy += rsqrt_fastLUT (f[3]);
        //
        }
    
    double t1 = ttt_timeTrigger();
    
    /* Standard function. */
    
    PD_MEMORY_BARRIER; benchmark_dummy = 0; ttt_timeTrigger();
    
        for (i = 0; i < TEST_BENCHMARK_LOOP; i++) {
        //
        float f[4];
        
        f[0] = (float)(PD_RAND48_DOUBLE (seed) * TEST_BENCHMARK_SQRT);
        f[1] = (float)(PD_RAND48_DOUBLE (seed) * TEST_BENCHMARK_SQRT);
        f[2] = (float)(PD_RAND48_DOUBLE (seed) * TEST_BENCHMARK_SQRT);
        f[3] = (float)(PD_RAND48_DOUBLE (seed) * TEST_BENCHMARK_SQRT);
        
        benchmark_dummy += rsqrt_fastSTD (f[0]);
        benchmark_dummy += rsqrt_fastSTD (f[1]);
        benchmark_dummy += rsqrt_fastSTD (f[2]);
        benchmark_dummy += rsqrt_fastSTD (f[3]);
        //
        }
    
    double t2 = ttt_timeTrigger();
    
    //ttt_stdout (TTT_COLOR_BLUE, "LUT: %f", t1);
    //ttt_stdout (TTT_COLOR_BLUE, "STD: %f", t2);

TTT_END

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

#if 0
}
#endif

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

#if 0
void test62__sqrt() {
#endif

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

TTT_BEGIN (BenchmarkSqrt, 62, "Benchmark - sqrt")

    int i;
    
    t_rand48 seed; PD_RAND48_INIT (seed);

    rsqrt_tilde_initialize();
    
    /* LUT. */
    
    PD_MEMORY_BARRIER; benchmark_dummy = 0; ttt_timeTrigger();
    
        for (i = 0; i < TEST_BENCHMARK_LOOP; i++) {
        //
        float f[4];
        
        f[0] = (float)(PD_RAND48_DOUBLE (seed) * TEST_BENCHMARK_SQRT);
        f[1] = (float)(PD_RAND48_DOUBLE (seed) * TEST_BENCHMARK_SQRT);
        f[2] = (float)(PD_RAND48_DOUBLE (seed) * TEST_BENCHMARK_SQRT);
        f[3] = (float)(PD_RAND48_DOUBLE (seed) * TEST_BENCHMARK_SQRT);
        
        benchmark_dummy += sqrt_fastLUT (f[0]);
        benchmark_dummy += sqrt_fastLUT (f[1]);
        benchmark_dummy += sqrt_fastLUT (f[2]);
        benchmark_dummy += sqrt_fastLUT (f[3]);
        //
        }
    
    double t1 = ttt_timeTrigger();
    
    /* Standard function. */
    
    PD_MEMORY_BARRIER; benchmark_dummy = 0; ttt_timeTrigger();
    
        for (i = 0; i < TEST_BENCHMARK_LOOP; i++) {
        //
        float f[4];
        
        f[0] = (float)(PD_RAND48_DOUBLE (seed) * TEST_BENCHMARK_SQRT);
        f[1] = (float)(PD_RAND48_DOUBLE (seed) * TEST_BENCHMARK_SQRT);
        f[2] = (float)(PD_RAND48_DOUBLE (seed) * TEST_BENCHMARK_SQRT);
        f[3] = (float)(PD_RAND48_DOUBLE (seed) * TEST_BENCHMARK_SQRT);
        
        benchmark_dummy += sqrt_fastSTD (f[0]);
        benchmark_dummy += sqrt_fastSTD (f[1]);
        benchmark_dummy += sqrt_fastSTD (f[2]);
        benchmark_dummy += sqrt_fastSTD (f[3]);
        //
        }
    
    double t2 = ttt_timeTrigger();
    
    //ttt_stdout (TTT_COLOR_BLUE, "LUT: %f", t1);
    //ttt_stdout (TTT_COLOR_BLUE, "STD: %f", t2);

TTT_END

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

#if 0
}
#endif

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
