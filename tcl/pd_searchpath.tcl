
# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

# Copyright (c) 1997-2015 Miller Puckette and others ( https://opensource.org/licenses/BSD-3-Clause ).

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

package provide pd_searchpath 0.1

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

namespace eval pd_searchpath {

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

variable lastIndex 0

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

proc get_curidx { mytoplevel } {
    set idx [$mytoplevel.listbox.box index active]
    if {$idx < 0 || \
            $idx == [$mytoplevel.listbox.box index end]} {
        return [expr {[$mytoplevel.listbox.box index end] + 1}]
    }
    return [expr $idx]
}

proc insert_item { mytoplevel idx name } {
    if {$name != ""} {
        $mytoplevel.listbox.box insert $idx $name
        set activeIdx [expr {[$mytoplevel.listbox.box index active] + 1}]
        $mytoplevel.listbox.box see $activeIdx
        $mytoplevel.listbox.box activate $activeIdx
        $mytoplevel.listbox.box selection clear 0 end
        $mytoplevel.listbox.box selection set active
        focus $mytoplevel.listbox.box
    }
}

proc add_item { mytoplevel add_method } {
    set dir [$add_method]
    insert_item $mytoplevel [expr {[get_curidx $mytoplevel] + 1}] $dir
}

proc edit_item { mytoplevel edit_method } {
    set idx [expr {[get_curidx $mytoplevel]}]
    set initialValue [$mytoplevel.listbox.box get $idx]
    if {$initialValue != ""} {
        set dir [$edit_method $initialValue]

        if {$dir != ""} {
            $mytoplevel.listbox.box delete $idx
            insert_item $mytoplevel $idx $dir
        }
        $mytoplevel.listbox.box activate $idx
        $mytoplevel.listbox.box selection clear 0 end
        $mytoplevel.listbox.box selection set active
        focus $mytoplevel.listbox.box
    }
}

proc delete_item { mytoplevel } {
    set cursel [$mytoplevel.listbox.box curselection]
    foreach idx $cursel {
        $mytoplevel.listbox.box delete $idx
    }
}

# Double-clicking on the listbox should edit the current item,
# or add a new one if there is no current
proc dbl_click { mytoplevel edit_method add_method x y } {
    if { $x == "" || $y == "" } {
        return
    }

    set curBB [$mytoplevel.listbox.box bbox @$x,$y]

    # listbox bbox returns an array of 4 items in the order:
    # left, top, width, height
    set height [lindex $curBB 3]
    set top [lindex $curBB 1]
    if { $height == "" || $top == "" } {
        # If for some reason we didn't get valid bbox info,
        # we want to default to adding a new item
        set height 0
        set top 0
        set y 1
    }

    set bottom [expr {$height + $top}]

    if {$y > $bottom} {
        add_item $mytoplevel $add_method
    } else {
        edit_item $mytoplevel $edit_method
    }
}

proc click { mytoplevel x y } {
    # record the index of the current element being
    # clicked on
    variable lastIndex 
    
    set lastIndex [$mytoplevel.listbox.box index @$x,$y]

    focus $mytoplevel.listbox.box
}

# For drag-and-drop reordering, recall the last-clicked index
# and move it to the position of the item currently under the mouse
proc release { mytoplevel x y } {
    variable lastIndex 
    set curIdx [$mytoplevel.listbox.box index @$x,$y]

    if { $curIdx != $lastIndex  } {
        # clear any current selection
        $mytoplevel.listbox.box selection clear 0 end

        set oldIdx $lastIndex 
        set newIdx [expr {$curIdx+1}]
        set selIdx $curIdx

        if { $curIdx < $lastIndex  } {
            set oldIdx [expr {$lastIndex  + 1}]
            set newIdx $curIdx
            set selIdx $newIdx
        }

        $mytoplevel.listbox.box insert $newIdx [$mytoplevel.listbox.box get $lastIndex ]
        $mytoplevel.listbox.box delete $oldIdx
        $mytoplevel.listbox.box activate $newIdx
        $mytoplevel.listbox.box selection set $selIdx
    }
}

# Make a pd_searchpath widget in a given window and set of data.
#
# id - the parent window for the pd_searchpath
# listdata - array of data to populate the pd_searchpath
# add_method - method to be called when we add a new item
# edit_method - method to be called when we edit an existing item
proc make { mytoplevel listdata add_method edit_method } {
    frame $mytoplevel.listbox
    listbox $mytoplevel.listbox.box \
        -selectmode browse -activestyle dotbox \
        -yscrollcommand [list "$mytoplevel.listbox.scrollbar" set]

    # Create a scrollbar and keep it in sync with the current
    # listbox view
    pack $mytoplevel.listbox.box [scrollbar "$mytoplevel.listbox.scrollbar" \
                              -command [list $mytoplevel.listbox.box yview]] \
        -side left -fill y -anchor w 

    # Populate the listbox widget
    foreach item $listdata {
        $mytoplevel.listbox.box insert end $item
    }

    # Standard listbox key/mouse bindings
    event add <<Delete>> <Delete>
    if { [tk windowingsystem] eq "aqua" } { event add <<Delete>> <BackSpace> }

    bind $mytoplevel.listbox.box <ButtonPress> "::pd_searchpath::click $mytoplevel %x %y"
    bind $mytoplevel.listbox.box <Double-1> "::pd_searchpath::dbl_click $mytoplevel $edit_method $add_method %x %y"
    bind $mytoplevel.listbox.box <ButtonRelease> "::pd_searchpath::release $mytoplevel %x %y"
    bind $mytoplevel.listbox.box <Return> "::pd_searchpath::edit_item $mytoplevel $edit_method"
    bind $mytoplevel.listbox.box <<Delete>> "::pd_searchpath::delete_item $mytoplevel"

    # <Configure> is called when the user modifies the window
    # We use it to capture resize events, to make sure the
    # currently selected item in the listbox is always visible
    bind $mytoplevel <Configure> "$mytoplevel.listbox.box see active"

    # The listbox should expand to fill its containing window
    # the "-fill" option specifies which direction (x, y or both) to fill, while
    # the "-expand" option (false by default) specifies whether the widget
    # should fill
    pack $mytoplevel.listbox.box -side left -fill both -expand 1
    pack $mytoplevel.listbox -side top -pady 2m -padx 2m -fill both -expand 1

    # All widget interactions can be performed without buttons, but
    # we still need a "New..." button since the currently visible window
    # might be full (even though the user can still expand it)
    frame $mytoplevel.actions 
    pack $mytoplevel.actions -side top -padx 2m -fill x 
    button $mytoplevel.actions.add_path -text {New...} \
        -command "::pd_searchpath::add_item $mytoplevel $add_method"
    button $mytoplevel.actions.edit_path -text {Edit...} \
        -command "::pd_searchpath::edit_item $mytoplevel $edit_method"
    button $mytoplevel.actions.delete_path -text {Delete} \
        -command "::pd_searchpath::delete_item $mytoplevel"

    pack $mytoplevel.actions.delete_path -side right -pady 2m
    pack $mytoplevel.actions.edit_path -side right -pady 2m
    pack $mytoplevel.actions.add_path -side right -pady 2m

    $mytoplevel.listbox.box activate end
    $mytoplevel.listbox.box selection set end
    focus $mytoplevel.listbox.box
}

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

}

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

