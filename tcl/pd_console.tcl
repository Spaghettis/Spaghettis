
# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

# Copyright (c) 1997-2015 Miller Puckette and others ( https://opensource.org/licenses/BSD-3-Clause ).

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

package provide pd_console 0.1

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

namespace eval ::pd_console:: {

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

variable logBuffer {}

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

# ------------------------------------------------------------------------------
# pdtk functions for 'pd' to send data to the Pd window

proc buffer_message {object_id level message} {
    variable logBuffer
    lappend logBuffer $object_id $level $message
}

proc insert_log_line {object_id level message} {
    if {$object_id eq ""} {
        .console.text.internal insert end $message log$level
    } else {
        .console.text.internal insert end $message [list log$level obj$object_id]
        .console.text.internal tag bind obj$object_id <$::var(modifierKey)-ButtonRelease-1> \
            "::pd_console::select_by_id $object_id; break"
        .console.text.internal tag bind obj$object_id <Key-Return> \
            "::pd_console::select_by_id $object_id; break"
        .console.text.internal tag bind obj$object_id <Key-KP_Enter> \
            "::pd_console::select_by_id $object_id; break"
    }
}

proc select_by_id {args} {
    if [llength $args] { # Is $args empty?
        ::pd_connect::pdsend "pd findinstance $args"
    }
}

proc logpost {object_id level message} {

    buffer_message $object_id $level $message
    if {[llength [info commands .console.text.internal]]} {
        # cancel any pending move of the scrollbar, and schedule it
        # after writing a line. This way the scrollbar is only moved once
        # when the inserting has finished, greatly speeding things up
        after cancel .console.text.internal yview end
        insert_log_line $object_id $level $message
        after idle .console.text.internal yview end
    }
}

# clear the log and the buffer
proc clear_console {} {
    variable logBuffer {}
    .console.text.internal delete 0.0 end
}

# save the contents of the pd_console::logBuffer to a file
proc save_logbuffer_to_file {} {
    variable logBuffer
    set filename [tk_getSaveFile -initialfile "pdconsole.txt" -defaultextension .txt]
    if {$filename eq ""} return; # they clicked cancel
    set f [open $filename w]
    puts $f "Tcl/Tk [info patchlevel]"
    puts $f "------------------------------------------------------------------------------"
    puts $f $logBuffer
    close $f
}


#--compute audio/DSP checkbutton-----------------------------------------------#

# set the checkbox on the "Compute Audio" menuitem and checkbox
proc pdtk_pd_dsp {value} {
    # TODO canvas_startdsp/stopdsp should really send 1 or 0, not "ON" or "OFF"
    if {$value eq "ON"} {
        set ::var(isDsp) 1
    } else {
        set ::var(isDsp) 0
    }
}

#--bindings specific to the Pd window------------------------------------------#

proc console_bindings {} {
    # these bindings are for the whole Pd window, minus the Tcl entry
    foreach window {.console.text .console.header} {
        bind $window <$::var(modifierKey)-Key-x> "tk_textCut .console.text"
        bind $window <$::var(modifierKey)-Key-c> "tk_textCopy .console.text"
        bind $window <$::var(modifierKey)-Key-v> "tk_textPaste .console.text"
    }
    # Select All doesn't seem to work unless its applied to the whole window
    bind .console <$::var(modifierKey)-Key-a> ".console.text tag add sel 1.0 end"
    # the "; break" part stops executing another binds, like from the Text class

    # these don't do anything in the Pd window, so alert the user, then break
    # so no more bindings run
    bind .console <$::var(modifierKey)-Key-s> "bell; break"
    bind .console <$::var(modifierKey)-Key-p> "bell; break"

    # ways of hiding/closing the Pd window
    if {[tk windowingsystem] eq "aqua"} {
        # on Mac OS X, you can close the Pd window, since the menubar is there
        bind .console <$::var(modifierKey)-Key-w>   "wm withdraw .console"
        wm protocol .console WM_DELETE_WINDOW "wm withdraw .console"
    } else {
        # TODO should it possible to close the Pd window and keep Pd open?
        bind .console <$::var(modifierKey)-Key-w>   "wm iconify .console"
        wm protocol .console WM_DELETE_WINDOW "::pd_connect::pdsend \"pd verifyquit\""
    }
}

#--create the window-----------------------------------------------------------#

proc set_layout {} {
    .console.text.internal tag configure log0 -foreground "#d00" -background "#ffe0e8"
    .console.text.internal tag configure log1 -foreground "#d00"
    # log2 messages are normal black on white
    .console.text.internal tag configure log3 -foreground "#484848"

    # 0-20(4-24) is a rough useful range of 'verbose' levels for impl debugging
    set start 4
    set end 25
    for {set i $start} {$i < $end} {incr i} {
        set B [expr int(($i - $start) * (40 / ($end - $start))) + 50]
        .console.text.internal tag configure log${i} -foreground grey${B}
    }
}

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

proc initialize {} {

    toplevel .console -class PdConsole
    
    set ::patch_name(.console) [_ "PureData"]
    
    wm title    .console [_ "PureData"]
    wm minsize  .console 400 75
    wm geometry .console =500x400+20+50
    
    .console configure -menu .menubar

    frame       .console.header         -background lightgrey
    checkbutton .console.header.dsp     -background lightgrey \
                                        -borderwidth 0 \
                                        -variable ::var(isDsp) \
                                        -command { ::pd_connect::pdsend "pd dsp $::var(isDsp)" }
    scrollbar   .console.scroll         -command ".console.text yview"
    text        .console.text           -borderwidth 0 \
                                        -font [getFontDefault 12] \
                                        -highlightthickness 0 \
                                        -undo 0 \
                                        -yscrollcommand ".console.scroll set"
        
    pack .console.header                -side top   -fill x     -ipady 5
    pack .console.header.dsp            -side right 
    pack .console.scroll                -side right -fill y
    pack .console.text                  -side right -fill both  -expand 1
    
    raise .console
    focus .console.text

    console_bindings

    # hack to make a good read-only text widget from http://wiki.tcl.tk/1152
    rename ::.console.text ::.console.text.internal
    proc ::.console.text {args} {
        switch -exact -- [lindex $args 0] {
            "insert" {}
            "delete" {}
            "default" { return [eval ::.console.text.internal $args] }
        }
    }
    
    # set some layout variables
    ::pd_console::set_layout

    # wait until .console.tcl.entry is visible before opening files so that
    # the loading logic can grab it and put up the busy cursor
    tkwait visibility .console.text
}

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

}

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------
