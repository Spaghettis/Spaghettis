#! /usr/bin/env bash

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

# Use the same flags than building the application.

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

CPUFLAGS="-march=native"

if [[ $OSTYPE =~ linux-gnueabihf ]]; then
    CPUTYPE="`lscpu`"
    if [[ $CPUTYPE =~ "Cortex-A72" ]]; then
        CPUFLAGS="-mcpu=cortex-a72 -mtune=cortex-a72 -mfpu=neon-fp-armv8"
    fi
fi

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

g++ -std=c++11 main.cpp -I../libraries/belle/Source -O3 -ffast-math ${CPUFLAGS} -ldl -lpthread -lm -o tests

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------
