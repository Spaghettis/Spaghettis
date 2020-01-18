#! /usr/bin/env bash

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

# Define the best cpu flags according to the machine (e.g. RPI4 below).

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

CPUFLAGS="-march=native"

if [[ $OSTYPE =~ linux-gnueabihf ]]; then
    CPUTYPE="`lscpu`"
    if [[ $CPUTYPE =~ "Cortex-A72" ]]; then
        CPUFLAGS="-mcpu=cortex-a72 -mtune=cortex-a72 -mfpu=neon-fp-armv8"
    fi
fi

export CPUFLAGS

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

# Basic platform handler ( http://stackoverflow.com/a/18434831 ).

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

case "$OSTYPE" in
    solaris*)           echo "SOLARIS" ;;
    darwin*)            ./buildMac.sh ;;
    linux-gnueabihf)    ./buildRaspbian.sh ;;
    linux*)             ./buildLinux.sh ;;
    bsd*)               echo "BSD" ;;
    *)                  echo "unknown: $OSTYPE" ;;
esac

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------
