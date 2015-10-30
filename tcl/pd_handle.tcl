
# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

# Copyright (c) 1997-2015 Miller Puckette and others ( https://opensource.org/licenses/BSD-3-Clause ).

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

# Event handlers for the menu items. 

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

package provide pd_handle 0.1

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

package require pd_connect
package require pd_miscellaneous

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

namespace eval ::pd_handle:: {

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

namespace export newPatch
namespace export open
namespace export handle
namespace export raiseConsole
namespace export raisePrevious
namespace export raiseNext

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

# For now the default file name can not be internationalized.

variable untitledName "Untitled"
variable untitledNumber "1"

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

proc newPatch {} {

    variable untitledName
    variable untitledNumber 
    
    if {![file isdirectory $::var(directoryNew)]} { set ::var(directoryNew) $::env(HOME) }
    
    ::pd_connect::pdsend "pd menunew $untitledName-$untitledNumber [::enquote $::var(directoryNew)]"
    
    incr untitledNumber 
}

proc open {} {

    if {![file isdirectory $::var(directoryOpen)]} { set ::var(directoryOpen) $::env(HOME) }
    
    set files [tk_getOpenFile -multiple 1 -filetypes $::var(filesTypes) -initialdir $::var(directoryOpen)]

    if {$files ne ""} {
        foreach filename $files { ::pd_miscellaneous::openFile $filename }
        set ::var(directoryOpen) [file dirname $filename]
    }
}

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

proc handle {message} {

    set top [winfo toplevel $::var(windowFocused)]
    
    switch -- [winfo class $top] {
        "PdPatch"   {
            ::pd_connect::pdsend "$top $message"
        }
        "PdConsole" {
            if {$message eq "copy"} { tk_textCopy .console.text }
            if {$message eq "selectall"} { .console.text tag add sel 1.0 end }
        }
    }
}

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

proc raiseConsole {} {
    wm deiconify .console
    raise .console
    focus .console
}

proc raisePrevious {} {
    lower [lindex [wm stackorder .] end] [lindex [wm stackorder .] 0]
    focus [lindex [wm stackorder .] end]
}

proc raiseNext {} {
    set top [lindex [wm stackorder .] 0]
    raise $top
    focus $top
}

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

}

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------
