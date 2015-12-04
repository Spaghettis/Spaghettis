
# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

# Copyright (c) 1997-2015 Miller Puckette and others ( https://opensource.org/licenses/BSD-3-Clause ).

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

# Scalar properties.

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

package provide pd_data 1.0

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

namespace eval ::pd_data:: {

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

proc show {top content} {

    ::pd_data::_create $top $content
}

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

proc _create {top content} {

    toplevel $top -class PdText
    wm title $top [_ "Data"]
    wm group $top .
    
    wm minsize  $top 50 50
    wm geometry $top [format "=600x400%s" [::rightNextTo $::var(windowFocused)]]

    text $top.text  -borderwidth 0 \
                    -highlightthickness 0
    
    pack $top.text  -side left -fill both -expand 1

    $top.text insert end $content
    
    wm protocol $top WM_DELETE_WINDOW   "::pd_data::closed $top"
        
    focus $top.text
}

proc closed {top} {

    ::pd_data::_apply $top
    ::cancel $top
}

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

proc _apply {top} {

    for {set i 1} {[$top.text compare $i.end < end]} {incr i 1} {
        set line [$top.text get $i.0 $i.end]
        if {$line != ""} {
            ::pd_connect::pdsend "$top data $line"
        }
    }
    
    ::pd_connect::pdsend "$top end"
}

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

}

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------
