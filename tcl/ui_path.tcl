
# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

# Copyright (c) 1997-2018 Miller Puckette and others ( https://opensource.org/licenses/BSD-3-Clause ).

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

# Manage the search path.

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

package provide ui_path 1.0

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

namespace eval ::ui_path:: {

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

proc show {} { 
    
    if {[winfo exists .path]} { ::bringToFront .path } else { ::ui_path::_create }
    
    ::ui_path::_updateInfo
}

proc hide {} {

    ::ui_path::closed
}

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

proc _create {} {

    toplevel .path -class PdTool
    wm title .path [_ "Path"]
    wm group .path .
        
    wm minsize  .path {*}[::styleMinimumSize]
    wm geometry .path [format "=500x300%s" [::rightNextTo .console]]
    
    ttk::frame      .path.f             {*}[::styleFrame]
    ttk::labelframe .path.f.paths       {*}[::styleLabelFrame] \
                                            -text [_ [::ifAqua "Folders" "Directories"]]
    
    pack            .path.f             {*}[::packMain]
    pack            .path.f.paths       {*}[::packCategory]
    
    listbox         .path.f.paths.list  -selectmode extended \
                                            -activestyle none \
                                            -borderwidth 0 \
                                            -background white
                                        
    label           .path.f.paths.info  -text [_ "Double-click to add a path"] \
                                            -anchor center \
                                            -foreground DarkGrey \
                                            -background white
                                            
    pack            .path.f.paths.list  -side top -fill both -expand 1

    foreach item $::var(searchPath) { .path.f.paths.list insert end $item }
    
    bind .path.f.paths.list <BackSpace>             "::ui_path::_deleteItems"
    bind .path.f.paths.list <Double-Button-1>       "::ui_path::_addItem"
    bind .path.f.paths.list <Triple-Button-1>       { ::ui_interface::pdsend "pd _dummy" }
    bind .path.f.paths.list <Quadruple-Button-1>    { ::ui_interface::pdsend "pd _dummy" }
    
    wm protocol .path WM_DELETE_WINDOW { ::ui_path::closed }
}

# The parameter provided is not used. 

proc closed {{top {}}} {

    ::removeFromScreen .path
        
    set ::var(isPath) 0
}

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

proc _addItem {} {

    .path.f.paths.list selection clear 0 end
    
    set item [tk_chooseDirectory -title [_ "Add a Directory"]]
    
    if {$item ne ""} { .path.f.paths.list insert end $item; .path.f.paths.list selection set end }
    
    ::ui_path::_apply
    ::ui_path::_updateInfo
}

proc _deleteItems {} {

    set i 0
    
    foreach item [.path.f.paths.list curselection] { .path.f.paths.list delete [expr {$item - $i}]; incr i }
    
    ::ui_path::_apply
    ::ui_path::_updateInfo
}

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

proc _apply {} {

    set ::var(searchPath) {}
    
    foreach path [.path.f.paths.list get 0 end] { lappend ::var(searchPath) [::encoded $path] }

    ::ui_interface::pdsend "pd _path $::var(searchPath)"
    ::ui_interface::pdsend "pd _savepreferences"
}

# Show info message if the search path is empty.

proc _updateInfo {} {
    
    if {[.path.f.paths.list size] == 0} {
    
        place .path.f.paths.info -relwidth .8 -relheight .3 -relx .1 -rely .35
    
    } else {
    
        place forget .path.f.paths.info
    }
}

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

}

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

