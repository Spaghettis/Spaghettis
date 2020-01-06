#! /usr/bin/env wish

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

# Copyright (c) 1997-2020 Miller Puckette and others ( https://opensource.org/licenses/BSD-3-Clause ).

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

package require Tcl
package require Tk

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

# catch { console show }

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

# Withdraw the window first to avoid flashing.

catch { wm withdraw . }

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

# Note that ALL the Tcl files MUST be kept in the same directory.

set auto_path [linsert $auto_path 0 [file dirname [info script]]]

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

if {[tk windowingsystem] eq "aqua"} { package require ui_apple }

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

package require ui_array
package require ui_atom
package require ui_audio
package require ui_bind
package require ui_box
package require ui_canvas
package require ui_confirm
package require ui_console
package require ui_file
package require ui_iem
package require ui_interface
package require ui_menu
package require ui_midi
package require ui_patch
package require ui_path
package require ui_scalar
package require ui_text
package require ui_value

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

# MUST be a monospaced font.
# ( https://en.wikipedia.org/wiki/List_of_monospaced_typefaces ).

proc getDefaultFamily {} {
    
    set fonts { "DejaVu Sans Mono" \
                "Bitstream Vera Sans Mono" \
                "Inconsolata" \
                "Consolas" \
                "Droid Sans Mono" \
                "Andale Mono" \
                "Menlo" \
                "Monaco"
    }

    foreach family $fonts {
        if {[lsearch -exact -nocase [font families] $family] > -1} {
            return $family
        }
    }
    
    return "Courier"
}

proc getDefaultWeight {} {

    if {[tk windowingsystem] eq "aqua"} { 
        return "normal" 
    } else { 
        return "bold" 
    }
}

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

set var(filesExtensions)            ".pdpatch .pdhelp .pdtemplate"
set var(filesOpenPended)            {}
set var(filesRecent)                {}
set var(filesTypes)                 { {{Patch} {.pdpatch}} {{Help} {.pdhelp}} {{Template} {.pdtemplate}} }

set var(fontFamily)                 [::getDefaultFamily]
set var(fontWeight)                 [::getDefaultWeight]
set var(fontSizes)                  "6 8 9 10 11 12 14 16 18 20 24 36"

set var(isPath)                     0
set var(isDsp)                      0
set var(isEditMode)                 0
set var(isInitialized)              0
set var(isSnapToGrid)               0

set var(nil)                        "empty"

set var(searchPath)                 {}
set var(tcpHost)                    ""
set var(tcpPort)                    0

set var(templates)                  {}

set var(windowFocused)              .
set var(windowStagger)              0

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

if {[tk windowingsystem] eq "aqua"} {

set var(backgroundColorRun)         "#ffffff"
set var(backgroundColorEdit)        "#fcfcfc"

} else {

set var(backgroundColorRun)         "#ffffff"
set var(backgroundColorEdit)        "#ffffff"

}

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

source [file join [file dirname [info script]] ui_global.tcl]

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

if {[tk windowingsystem] eq "x11"} {
    source [file join [file dirname [info script]] ui_x11.tcl]
} else {
    source [file join [file dirname [info script]] ui_aqua.tcl]
}

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

proc main {argc argv} {

    tk appname spaghettis

    # Configure to UTF-8 encoding.
    
    encoding system utf-8
    fconfigure stderr -encoding utf-8
    fconfigure stdout -encoding utf-8
    
    # Don't show hidden files ( http://wiki.tcl.tk/1060 ).
    
    if {[tk windowingsystem] eq "x11"} {
        catch { tk_getOpenFile -dummy } 
        set ::tk::dialog::file::showHiddenBtn 1
        set ::tk::dialog::file::showHiddenVar 0
    }
    
    # Avoid tear-off menus.
    
    option add *tearOff 0
    
    # Handle socket connection.
    
    if {$argc == 1 && [string is int $argv]} {
        set ::var(tcpHost) "localhost"
        set ::var(tcpPort) $argv
        ::ui_interface::clientSocket $::var(tcpPort) $::var(tcpHost)
        
    } else {
        ::ui_interface::serverSocket
        set executable [file join [file dirname [info script]] "../bin/spaghettis"]
        exec -- $executable -port $::var(tcpPort) &
    }
}

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

# Notice that main is always called first.

proc initialize {} {

    # Create fonts (determine average horizontal and vertical spacing in pixels). 
    
    set measured ""
    
    foreach size $::var(fontSizes) {
        set f [::getFont $size]
        font create $f -family $::var(fontFamily) -weight $::var(fontWeight) -size [expr {-($size)}]
        set lorem [font measure $f "Bright vixens jump; dozy fowl quack."]
        set ipsum [string length   "Bright vixens jump; dozy fowl quack."]
        lappend measured $size 
        lappend measured [expr {$lorem / ($ipsum + 0.0)}]
        lappend measured [font metrics $f -linespace]
    }

    # Initialize some packages.
    
    foreach module {ui_menu ui_console ui_bind ui_file} { [format "::%s::initialize" $module] }
    
    focus .console
        
    # Respond to executable with measured fonts.
    
    ::ui_interface::pdsend "pd _font $measured"
}

proc version {s} {

    ui_console::post "$s / Wish [info patchlevel]"
}

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

# Called on macOS only.

proc openPendedFiles {} {

    set ::var(isInitialized) 1
    
    foreach filename $::var(filesOpenPended) { ::ui_file::openFile $filename }
    
    set ::var(filesOpenPended) {}
}

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

main $::argc $::argv

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------
