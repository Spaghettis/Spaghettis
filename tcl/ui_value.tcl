
# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

# Copyright (c) 1997-2018 Miller Puckette and others ( https://opensource.org/licenses/BSD-3-Clause ).

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

# Value window.

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

package provide ui_value 1.0

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

namespace eval ::ui_value:: {

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

variable  valueType
variable  valueValue

array set valueType  {}
array set valueValue {}

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

proc show {top name type value} {
    
    ::ui_value::_create $top $name $type $value
}

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

proc _create {top name type value} {
    
    variable valueType
    variable valueValue
    
    toplevel $top -class PdDialog
    wm title $top [_ "Value"]
    wm group $top .
    
    wm resizable $top 0 0
    wm minsize   $top {*}[::styleMinimumSizeScalar]
    wm geometry  $top [::rightNextTo $::var(windowFocused)]

    set valueType($top)         $type
    set valueValue($top)        [::hashToDollar $value]
    set valueValue(${top}.old)  [::hashToDollar $value]
    
    ttk::frame      $top.f                      {*}[::styleFrame]
    ttk::labelframe $top.f.properties           {*}[::styleLabelFrame]  -text [_ [string totitle $name]]
    
    pack $top.f                                 {*}[::packMain]
    pack $top.f.properties                      {*}[::packCategory]
    
    if {$type eq "floatatom"} {
    
    ttk::entry $top.f.properties.value          {*}[::styleEntryNumber] \
                                                    -textvariable ::ui_value::valueValue($top) \
                                                    -width $::width(large)
    
    }
    
    if {$type eq "symbolatom"} {
    
    ttk::entry $top.f.properties.value          {*}[::styleEntry] \
                                                    -textvariable ::ui_value::valueValue($top) \
                                                    -width $::width(large)
    
    }
    
    grid $top.f.properties.value                -row 0 -column 0 -sticky ew
    
    grid columnconfigure $top.f.properties      0 -weight 1
    
    focus $top.f.properties.value
    
    after idle "$top.f.properties.value selection range 0 end"

    wm protocol $top WM_DELETE_WINDOW  "::ui_value::closed $top"
}

proc closed {top} {
    
    ::ui_value::_apply $top
    ::cancel $top
}

proc released {top} {

    variable valueType
    variable valueValue

    unset valueType($top)
    unset valueValue($top)
    
    unset valueValue(${top}.old)
}

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

proc _apply {top} {

    variable valueType
    variable valueValue
    
    ::ui_value::_forceValue $top
    
    if {$valueType($top) eq "floatatom"} {
        ::ui_interface::pdsend "$top _valuedialog $valueValue($top)"
    }
    
    if {$valueType($top) eq "symbolatom"} {
        ::ui_interface::pdsend "$top _valuedialog [::sanitized [::dollarToHash $valueValue($top)]]"
    }
}

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

proc _forceValue {top} {

    variable valueType
    variable valueValue
    
    if {$valueType($top) eq "floatatom"} {
        set valueValue($top) [::ifNumber    $valueValue($top)   $valueValue(${top}.old)]
    }
    
    if {$valueType($top) eq "symbolatom"} {
        set valueValue($top) [::ifNotNumber $valueValue($top)   $valueValue(${top}.old)]
    }
}

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

}

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------
