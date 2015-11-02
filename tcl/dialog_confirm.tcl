
# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

# Copyright (c) 1997-2015 Miller Puckette and others ( https://opensource.org/licenses/BSD-3-Clause ).

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

# Open a window to confirm action.

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

package provide dialog_confirm 0.1

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

package require pd_connect

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

namespace eval ::dialog_confirm:: {

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

namespace export checkAction
namespace export checkClose

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

proc checkAction {top message reply default} {

    set r [tk_messageBox -message $message -type yesno -default $default -icon question -parent $top]
    
    if {$r eq "yes"} {
        ::pd_connect::pdsend $reply
    }
}

proc checkClose {top reply} {

    set message [format [_ "Save \"%s\" before closing?"] [wm title $top]]
    
    set r [tk_messageBox -message $message -type yesnocancel -default "yes" -icon question -parent $top]

    switch -- $r {
        yes { ::pd_connect::pdsend "$top menusave 1" }
        no  { ::pd_connect::pdsend $reply }
        cancel {
        
        }
    }
}

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

}

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------
