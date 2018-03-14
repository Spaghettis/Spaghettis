#! /usr/bin/env bash

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

# "Spaghettis.app" automagically built.

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

# ActiveTcl ( http://wiki.tcl.tk/1875 ).

wish="/Library/Frameworks/Tk.framework/Versions/8.6/Resources/Wish.app"

if [ -e "${wish}" ]; then
    echo "Build with ActiveTcl 8.6 ..."
else
    wish="/Library/Frameworks/Tk.framework/Versions/8.5/Resources/Wish.app"
    if [ -e "${wish}" ]; then
        echo "Build with ActiveTcl 8.5 ..."
    else
        wish="/System/Library/Frameworks/Tk.framework/Versions/8.5/Resources/Wish.app"
    fi
fi

[ -e "${wish}" ] || { echo >&2 "${0##*/}: cannot find Tk framework"; exit 1; }

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

# Paths.

destination="${rep}/Application"
app="${destination}/Spaghettis.app"
plist="${rep}/resources/Info.plist"
bin="${rep}/bin"
tcl="${rep}/tcl"
help="${rep}/resources/help"
patches="${rep}/resources/patches"

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

# Do NOT overwrite previous build.

[ -e "${destination}" ] && { echo >&2 "${0##*/}: ${destination} already exist"; exit 1; }
[ -e "${app}" ] && { echo >&2 "${0##*/}: ${app} already exist"; exit 1; }

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

# Build the binaries.
# Workaround for annoying GCC 4.2.1 bug ( https://gcc.gnu.org/bugzilla/show_bug.cgi?id=33144 ).
# Avoid march=native flag on Mac OS X 10.6 system.
# Assume C++11 supported on a later OS.

echo "Build binaries ..."
cd "${rep}/src"                                                         || exit 1
if [ "$OSTYPE" != "darwin10.0" ]; then
    make -f makefile.mac MARCH="-march=native" CXXSTD="-std=c++11"      || exit 1
else
    make -f makefile.mac                                                || exit 1
fi
cd "${rep}"                                                             || exit 1

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

# Make the application bundle.

echo "Build package ..."
mkdir "${destination}"                                                  || exit 1
cp -R "${wish}" "${app}"                                                || exit 1
rm -f "${app}/Contents/Info.plist"                                      || exit 1
rm -f "${app}/Contents/PkgInfo"                                         || exit 1
rm -f "${app}/Contents/version.plist"                                   || exit 1
rm -f "${app}/Contents/MacOS/Wish Shell"                                || exit 1
rm -rf "${app}/Contents/_CodeSignature"                                 || exit 1
rm -f "${app}/Contents/CodeResources"                                   || exit 1
rm -f "${app}/Contents/Resources/Wish.sdef"                             || exit 1
cp -p "${plist}" "${app}/Contents/Info.plist"                           || exit 1
echo "APPL????" > "${app}/Contents/PkgInfo"                             || exit 1
mv "${app}/Contents/MacOS/Wish" "${app}/Contents/MacOS/Spaghettis"      || exit 1
cp -R "${bin}" "${app}/Contents/Resources/"                             || exit 1
cp -R "${tcl}" "${app}/Contents/Resources/"                             || exit 1
cp -R "${help}" "${app}/Contents/Resources/"                            || exit 1
cd "${app}/Contents/Resources/"                                         || exit 1
ln -s "tcl" "Scripts"                                                   || exit 1
cd "${rep}"                                                             || exit 1

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

# Install materials.

echo "Install patches ..."
cp -R "${patches}" "${destination}"                                     || exit 1

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

# Clean the build.

echo "Clean ..."
cd "${rep}/src"                                                         || exit 1
make -f makefile.mac clean                                              || exit 1
cd "${rep}"                                                             || exit 1

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

# Codesign with a pseudo-identity.

echo "Codesign ..."
codesign -s "-" -f "${app}"                                             || exit 1

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

# End.

echo "SUCCEEDED"

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------