
# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

# Copyright (c) 1997-2018 Miller Puckette and others ( https://opensource.org/licenses/BSD-3-Clause ).

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

# Global handy tutti frutti.

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

proc getFont {size} { 

    if {[lsearch -exact $::var(fontSizes) $size] > -1} { 
        return [format "::var(font%s)" $size] 
        
    } else {
        set next [lindex $::var(fontSizes) end]
        foreach f $::var(fontSizes) { if {$f > $size} { set next $f; break } }
        return [format "::var(font%s)" $next]
    }
}

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

proc getTitle {top} { 
    
    if {[winfo class $top] eq "PdPatch"} { return [::ui_patch::getTitle $top] }
    
    return [wm title $top]
}

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

proc nextEntry {w} {

    set next [tk_focusNext $w]
    
    focus $next
    
    if {[string match "*Entry" [winfo class $next]]} { $next selection range 0 end }
}

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

proc rightNextTo {top} {

    set offset [expr {[incr ::var(windowStagger) 50] + 50}]
    
    set x [expr {[winfo rootx $top] + $offset}]
    set y [expr {[winfo rooty $top] + $offset}]
    
    after 1000 { set ::var(windowStagger) 0 }
    
    return [format "+%d+%d" $x $y]
}

proc bringToFront {top} {

    wm deiconify $top; raise $top; focus $top
}

proc removeFromScreen {top} {

    wm withdraw $top; focus [lindex [wm stackorder .] end]
}

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

# Escaping and quoting.

proc encoded   {x} { return @[string map {" " "@_" "$" "@d" ";" "@s" "," "@c" "@" "@@"} $x] }
proc escaped   {x} { return [string map {"," "\\," ";" "\\;" "$" "\\$" " " "\\ "} $x] }
proc sanitized {x} { return [string map {" " "_" ";" "" "," "" "{" "" "}" "" "\\" ""} [lindex $x 0]] }

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

proc parseNil {x} { if {$x eq $::var(nil)} { return "" } else { return $x } }
proc withNil  {x} { if {$x eq ""} { return $::var(nil) } else { return $x } }

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

proc hashToDollar {x} { return [string map {"#" "$"} $x] }
proc dollarToHash {x} { return [string map {"$" "#"} $x] }    

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

proc ifInteger {new old} {

    if {[string is integer -strict $new]} { return $new } else { return $old }
}

proc ifNumber  {new old} {

    if {[string is double -strict $new]}  { return $new } else { return $old }
}

proc ifNotNumber {new old} {

    if {[string is double -strict $new]}  { return $old } else { return $new }
}

proc ifNotZero {new old} {

    if {$new != 0.0} { return $new } else { return $old }
}

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

proc ifAqua {a b} {

    if {[tk windowingsystem] eq "aqua"} { return $a } else { return $b }
}

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

# Get namespace for dialog windows.

proc getNamespace {top} {

    switch -- [wm title $top] {
        "Array"         { return "::ui_array"   }
        "Atom"          { return "::ui_atom"    }
        "Audio"         { return "::ui_audio"   }
        "Bang"          { return "::ui_iem"     }
        "Dial"          { return "::ui_iem"     }
        "Element"       { return "::ui_scalar"  }
        "Menu Button"   { return "::ui_iem"     }
        "MIDI"          { return "::ui_midi"    }
        "Patch"         { return "::ui_canvas"  }
        "Panel"         { return "::ui_iem"     }
        "Path"          { return "::ui_path"    }
        "Radio Button"  { return "::ui_iem"     }
        "Slider"        { return "::ui_iem"     }
        "Scalar"        { return "::ui_scalar"  }
        "Text"          { return "::ui_text"    }
        "Toggle"        { return "::ui_iem"     }
        "Value"         { return "::ui_value"   }
        "VU"            { return "::ui_iem"     }
    }
    
    return ""
}

proc cancel {w {signoff 1}} {

    set top [winfo toplevel $w]
    set class [winfo class $top]
    
    if {$class eq "PdDialog"} {
        set command [format "%s::released" [getNamespace $top]]; $command $top
    }
    
    if {$class eq "PdDialog" || $class eq "PdText"} {
    
        destroy $top
        
        if {$signoff == 1} { ::ui_interface::pdsend "$top _signoff" }
    }
}

proc remove {w} {
    
    ::cancel $w 0
}

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

# For future msgcat ( https://www.gnu.org/software/gettext/manual/html_node/Tcl.html ).

proc _ {s} { return $s }

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

# Could use ::msgcat::max later.

proc measure {words} {

    set m 0
    
    foreach e $words {
        set m [::tcl::mathfunc::max $m [string length $e]]
    }
    
    return $m
}

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

# Note that the name parameter must be fully qualified.

proc createMenuByIndex {top values name args} {

    upvar $name v
    
    ttk::menubutton $top            {*}[::styleMenuButton] \
                                        -text [lindex $values $v] \
                                        -takefocus 0 \
                                        {*}$args
    
    menu $top.menu
    $top configure                  -menu $top.menu
    
    set i 0
    
    foreach e $values {
        $top.menu add radiobutton   -label "$e" \
                                    -variable $name \
                                    -value $i \
                                    -command [list $top configure -text [lindex $values $i]]
        incr i
    }
}

proc createMenuByValue {top values name args} {

    upvar $name v
    
    ttk::menubutton $top            {*}[::styleMenuButton] \
                                        -text $v \
                                        -takefocus 0 \
                                        {*}$args
    
    menu $top.menu
    $top configure                  -menu $top.menu
    
    foreach e $values {
        $top.menu add radiobutton   -label "$e" \
                                    -variable $name \
                                    -value $e \
                                    -command [list $top configure -text "$e"]
    }
}

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

proc colorToInteger {color} {

    set hex [string replace $color 0 0 "0x"]
    return  [expr {$hex}]
}

proc integerToColor {integer} {

    return [format "#%06x" $integer]
}

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

proc chooseColor {label initial title} {
    
    set color [tk_chooseColor   -initialcolor [::integerToColor $initial] \
                                -title $title \
                                -parent [winfo toplevel $label]]

    if {$color ne ""} {
        $label configure -background $color
        return [::colorToInteger $color]
    }
    
    return $initial
}

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------
