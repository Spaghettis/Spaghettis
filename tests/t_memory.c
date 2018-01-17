
/* < https://opensource.org/licenses/BSD-3-Clause > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

#if PD_MALLOC_ALIGNED

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

#define TEST_MEMORY_LOOP    10000

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

#if 0
void test50__aligned() {
#endif

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

TTT_BEGIN (MemoryAligned, 50, "Memory - Aligned")
    
    int i, k = 0;
    
    t_sample *t[TEST_MEMORY_LOOP] = { 0 };
    
    for (i = 0; i < TEST_MEMORY_LOOP; i++) {
        t[i] = (t_sample *)PD_MEMORY_GET (sizeof (t_sample) * (i % 11));
    }
    
    for (i = 0; i < TEST_MEMORY_LOOP; i++) {
        k += PD_IS_ALIGNED_16(t[i]);            /* Aligned 16-bytes. */
        PD_MEMORY_FREE (t[i]);
    }
    
    TTT_EXPECT (k == TEST_MEMORY_LOOP);
    
TTT_END

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

#if 0
}
#endif

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

#endif // PD_MALLOC_ALIGNED

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

#if 0
void test51__cast() {
#endif

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

TTT_BEGIN (MemoryCast, 51, "Memory - Cast")
    
    t_rawcast64 z;
    
    z.z_d = DSP_UNITBIT + 0.5;
    TTT_EXPECT ((z.z_i[PD_RAWCAST64_LSB] == 0x80000000));
    TTT_EXPECT ((z.z_i[PD_RAWCAST64_MSB] == DSP_UNITBIT_MSB));
    
    z.z_d = COSINE_UNITBIT;
    TTT_EXPECT ((z.z_i[PD_RAWCAST64_MSB] == COSINE_UNITBIT_MSB));
    
TTT_END

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

#if 0
}
#endif

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
