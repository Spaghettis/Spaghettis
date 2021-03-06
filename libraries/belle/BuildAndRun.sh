#! /usr/bin/env bash

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

# Create all the PDF examples.

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

[ "${BASH_VERSION%.*}" \> "3.1" ] || { echo >&2 "${0##*/}: Bash 3.1 or higher only"; exit 1; }

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

# Script must be executed at the same level.

rep=$(pwd)

[ "${0%/*}" = "." ] || { echo >&2 "${0##*/}: Must be executed at the same level"; exit 1; }

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

# Build the example.

echo "Build Belle ..."

g++ "-std=c++11" "${rep}/Tools/Belle.cpp" -o "${rep}/Belle" "-I${rep}/Source/" || exit 1

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

# Launch it.

echo "Start Belle ..."

"./Belle" || exit 1

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------
