
/* Copyright (c) 1997-2018 Miller Puckette and others. */

/* < https://opensource.org/licenses/BSD-3-Clause > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

#include "m_spaghettis.h"

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

/* Belle. */

#include "libs/belle/belle.cpp"

/* Core. */

#include "core/m_symbols.c"
#include "core/m_environment.c"
#include "core/m_autorelease.c"
#include "core/m_instance.c"
#include "core/m_stack.c"
#include "core/m_bind.c"
#include "core/m_message.c"
#include "core/m_pd.c"
#include "core/m_method.c"
#include "core/m_class.c"
#include "core/m_object.c"
#include "core/m_inlet.c"
#include "core/m_outlet.c"
#include "core/m_atom.c"
#include "core/m_buffer.c"
#include "core/m_parse.c"
#include "core/m_file.c"
#include "core/m_eval.c"
#include "core/m_global.c"
#include "core/m_clipboard.c"
#include "core/m_encapsulate.c"
#include "core/m_deencapsulate.c"
#include "core/m_snippet.c"
#include "core/m_snap.c"
#include "core/m_setup.c"
#include "core/m_dollar.c"
#include "core/m_error.c"
#include "core/m_point.c"
#include "core/m_rectangle.c"
#include "core/m_bounds.c"
#include "core/m_iterator.c"
#include "core/m_slots.c"
#include "core/m_pathlist.c"
#include "core/m_heapstring.c"
#include "core/m_register.c"
#include "core/m_utils.c"
#include "core/m_symbol.c"
#include "core/m_color.c"
#include "core/m_math.c"
#include "core/m_string.c"

/* Undo. */

#include "undo/m_undosnippet.c"
#include "undo/m_undoaction.c"
#include "undo/m_undomanager.c"
#include "undo/m_undocollapse.c"
#include "undo/m_undoseparator.c"
#include "undo/m_undoadd.c"
#include "undo/m_undoremove.c"
#include "undo/m_undocut.c"
#include "undo/m_undopaste.c"
#include "undo/m_undoduplicate.c"
#include "undo/m_undosnap.c"
#include "undo/m_undoencapsulate.c"
#include "undo/m_undodeencapsulate.c"
#include "undo/m_undoconnect.c"
#include "undo/m_undodisconnect.c"
#include "undo/m_undocreate.c"
#include "undo/m_undodelete.c"
#include "undo/m_undomotion.c"
#include "undo/m_undoproperties.c"
#include "undo/m_undoresizebox.c"
#include "undo/m_undoresizegraph.c"
#include "undo/m_undofront.c"
#include "undo/m_undoback.c"
#include "undo/m_undotyping.c"


/* System. */

#include "system/s_entry.c"
#include "system/s_main.c"
#include "system/s_scheduler.c"
#include "system/s_priority.c"
#include "system/s_handlers.c"
#include "system/s_clock.c"
#include "system/s_time.c"
#include "system/s_atomic_mac.c"
#include "system/s_atomic_posix.c"
#include "system/s_receiver.c"
#include "system/s_monitor.c"
#include "system/s_defer.c"
#include "system/s_gui.c"
#include "system/s_interface.c"
#include "system/s_midi.c"
#include "system/s_inmidi.c"
#include "system/s_outmidi.c"
#include "system/s_audio.c"
#include "system/s_file.c"
#include "system/s_recentfiles.c"
#include "system/s_searchpath.c"
#include "system/s_path.c"
#include "system/s_loader.c"
#include "system/s_properties.c"
#include "system/s_preferences.c"
#include "system/s_memory.c"
#include "system/s_leak.c"
#include "system/s_font.c"
#include "system/s_post.c"
#include "system/s_ringbuffer.c"
#include "system/s_logger.c"
#include "system/s_utf8.c"
#include "system/s_MT.c"
#include "system/s_MT_32.c"
#include "system/s_MT_64.c"
#include "system/s_devicesproperties.c"
#include "system/s_deviceslist.c"
#include "system/s_midi_API.c"
#include "system/s_audio_API.c"

/* Graphics. */

#include "graphics/g_stub.c"
#include "graphics/g_proxy.c"

#include "graphics/patch/g_object.c"
#include "graphics/patch/g_typeset.c"
#include "graphics/patch/g_box.c"
#include "graphics/patch/g_traverser.c"
#include "graphics/patch/g_cord.c"
#include "graphics/patch/g_editor.c"
#include "graphics/patch/g_glist.c"
#include "graphics/patch/g_draw.c"
#include "graphics/patch/g_select.c"
#include "graphics/patch/g_edit.c"
#include "graphics/patch/g_arrange.c"
#include "graphics/patch/g_interface.c"
#include "graphics/patch/g_file.c"
#include "graphics/patch/g_make.c"
#include "graphics/patch/g_geometry.c"
#include "graphics/patch/g_behavior.c"
#include "graphics/patch/g_serialize.c"
#include "graphics/patch/g_canvas.c"
#include "graphics/patch/g_message.c"
#include "graphics/patch/g_text.c"
#include "graphics/patch/g_gatom.c"
#include "graphics/patch/g_garray.c"
#include "graphics/patch/g_vinlet.c"
#include "graphics/patch/g_voutlet.c"

#include "graphics/iem/g_iem.c"
#include "graphics/iem/g_bang.c"
#include "graphics/iem/g_toggle.c"
#include "graphics/iem/g_radio.c"
#include "graphics/iem/g_slider.c"
#include "graphics/iem/g_dial.c"
#include "graphics/iem/g_vu.c"
#include "graphics/iem/g_panel.c"
#include "graphics/iem/g_menubutton.c"

#include "graphics/scalars/g_word.c"
#include "graphics/scalars/g_field.c"
#include "graphics/scalars/g_array.c"
#include "graphics/scalars/g_gpointer.c"
#include "graphics/scalars/g_template.c"
#include "graphics/scalars/g_struct.c"
#include "graphics/scalars/g_constructor.c"
#include "graphics/scalars/g_scalar.c"
#include "graphics/scalars/g_paint.c"
#include "graphics/scalars/g_pointer.c"
#include "graphics/scalars/g_get.c"
#include "graphics/scalars/g_set.c"
#include "graphics/scalars/g_element.c"
#include "graphics/scalars/g_getsize.c"
#include "graphics/scalars/g_setsize.c"
#include "graphics/scalars/g_append.c"
#include "graphics/scalars/g_fields.c"
#include "graphics/scalars/g_drawpolygon.c"
#include "graphics/scalars/g_drawtext.c"
#include "graphics/scalars/g_drawcircle.c"
#include "graphics/scalars/g_plot.c"
#include "graphics/scalars/g_draw.c"
#include "graphics/scalars/g_scalars.c"

/* Control. */

#include "control/x_atomoutlet.c"

#include "control/atom/x_int.c"
#include "control/atom/x_float.c"
#include "control/atom/x_symbol.c"
#include "control/atom/x_bang.c"

#include "control/list/x_list.c"
#include "control/list/x_listinlet.c"
#include "control/list/x_listappend.c"
#include "control/list/x_listprepend.c"
#include "control/list/x_listsplit.c"
#include "control/list/x_listtrim.c"
#include "control/list/x_listlength.c"
#include "control/list/x_liststore.c"
#include "control/list/x_listiterate.c"
#include "control/list/x_listgroup.c"
#include "control/list/x_listfromsymbol.c"
#include "control/list/x_listtosymbol.c"

#include "control/text/x_text.c"
#include "control/text/x_textbuffer.c"
#include "control/text/x_textclient.c"
#include "control/text/x_textget.c"
#include "control/text/x_textset.c"
#include "control/text/x_textinsert.c"
#include "control/text/x_textdelete.c"
#include "control/text/x_textsize.c"
#include "control/text/x_textlist.c"
#include "control/text/x_textsearch.c"
#include "control/text/x_textsequence.c"
#include "control/text/x_qlist.c"
#include "control/text/x_textfile.c"

#include "control/array/x_array.c"
#include "control/array/x_arrayclient.c"
#include "control/array/x_arrayrange.c"
#include "control/array/x_arraysize.c"
#include "control/array/x_arraysum.c"
#include "control/array/x_arrayget.c"
#include "control/array/x_arrayset.c"
#include "control/array/x_arrayquantile.c"
#include "control/array/x_arrayrandom.c"
#include "control/array/x_arraymax.c"
#include "control/array/x_arraymin.c"

#include "control/tab/x_tabwrite.c"
#include "control/tab/x_tabread.c"
#include "control/tab/x_tabread4.c"
#include "control/tab/x_tabreceive.c"

#include "control/math/x_acoustic.c"
#include "control/math/x_math.c"
#include "control/math/x_atan2.c"
#include "control/math/x_binop1.c"
#include "control/math/x_binop2.c"
#include "control/math/x_binop3.c"
#include "control/math/x_random.c"
#include "control/math/x_clip.c"
#include "control/math/x_expr.c"
#include "control/math/x_functions.c"

#include "control/time/x_metro.c"
#include "control/time/x_delay.c"
#include "control/time/x_line.c"
#include "control/time/x_timer.c"
#include "control/time/x_pipe.c"

#include "control/flow/x_send.c"
#include "control/flow/x_receive.c"
#include "control/flow/x_select.c"
#include "control/flow/x_route.c"
#include "control/flow/x_pack.c"
#include "control/flow/x_unpack.c"
#include "control/flow/x_trigger.c"
#include "control/flow/x_prepend.c"
#include "control/flow/x_spigot.c"
#include "control/flow/x_moses.c"
#include "control/flow/x_until.c"
#include "control/flow/x_uzi.c"
#include "control/flow/x_swap.c"
#include "control/flow/x_change.c"
#include "control/flow/x_value.c"
#include "control/flow/x_counter.c"

#include "control/miscellaneous/x_print.c"
#include "control/miscellaneous/x_key.c"
#include "control/miscellaneous/x_keyup.c"
#include "control/miscellaneous/x_keyname.c"
#include "control/miscellaneous/x_openpanel.c"
#include "control/miscellaneous/x_savepanel.c"
#include "control/miscellaneous/x_makefilename.c"
#include "control/miscellaneous/x_loadbang.c"
#include "control/miscellaneous/x_closebang.c"
#include "control/miscellaneous/x_samplerate.c"
#include "control/miscellaneous/x_blocksize.c"
#include "control/miscellaneous/x_dspstatus.c"
#include "control/miscellaneous/x_arguments.c"
#include "control/miscellaneous/x_title.c"
#include "control/miscellaneous/x_freeze.c"
#include "control/miscellaneous/x_namecanvas.c"
#include "control/miscellaneous/x_serial.c"
#include "control/miscellaneous/x_realtime.c"
#include "control/miscellaneous/x_netsend.c"
#include "control/miscellaneous/x_netreceive.c"
#include "control/miscellaneous/x_oscparse.c"
#include "control/miscellaneous/x_oscformat.c"
#include "control/miscellaneous/x_oscbundle.c"
#include "control/miscellaneous/x_oscstream.c"
#include "control/miscellaneous/x_timestamp.c"

#include "control/midi/x_makenote.c"
#include "control/midi/x_stripnote.c"
#include "control/midi/x_bag.c"
#include "control/midi/x_poly.c"
#include "control/midi/x_midiin.c"
#include "control/midi/x_midiout.c"
#include "control/midi/x_notein.c"
#include "control/midi/x_noteout.c"
#include "control/midi/x_ctlin.c"
#include "control/midi/x_ctlout.c"
#include "control/midi/x_pgmin.c"
#include "control/midi/x_pgmout.c"
#include "control/midi/x_bendin.c"
#include "control/midi/x_bendout.c"
#include "control/midi/x_touchin.c"
#include "control/midi/x_touchout.c"
#include "control/midi/x_polytouchin.c"
#include "control/midi/x_polytouchout.c"
#include "control/midi/x_sysexin.c"
#include "control/midi/x_midirealtimein.c"

#include "control/mica/x_mica.c"
#include "control/mica/x_micaset.c"
#include "control/mica/x_micaget.c"
#include "control/mica/x_micainfo.c"
#include "control/mica/x_micasequence.c"
#include "control/mica/x_micamap.c"
#include "control/mica/x_micaindex.c"
#include "control/mica/x_micaitem.c"
#include "control/mica/x_micainterval.c"
#include "control/mica/x_micaspell.c"

#include "control/tralala/x_tralala.c"

/* DSP. */

#include "dsp/graph/d_dsp.c"
#include "dsp/graph/d_signal.c"
#include "dsp/graph/d_ugen.c"
#include "dsp/graph/d_canvas.c"
#include "dsp/graph/d_vinlet.c"
#include "dsp/graph/d_voutlet.c"
#include "dsp/graph/d_adc.c"
#include "dsp/graph/d_dac.c"
#include "dsp/graph/d_resample.c"
#include "dsp/graph/d_block.c"
#include "dsp/graph/d_blockinfo.c"
#include "dsp/graph/d_vperform.c"
#include "dsp/graph/d_perform.c"
#include "dsp/graph/d_functions.c"

#include "dsp/global/d_throw.c"
#include "dsp/global/d_catch.c"
#include "dsp/global/d_send.c"
#include "dsp/global/d_receive.c"

#include "dsp/osc/d_osc.c"
#include "dsp/osc/d_phasor.c"
#include "dsp/osc/d_cos.c"

#include "dsp/convert/d_sig.c"
#include "dsp/convert/d_line.c"
#include "dsp/convert/d_vline.c"
#include "dsp/convert/d_snapshot.c"
#include "dsp/convert/d_env.c"
#include "dsp/convert/d_threshold.c"
#include "dsp/convert/d_samphold.c"

#include "dsp/delay/d_delwrite.c"
#include "dsp/delay/d_delread.c"
#include "dsp/delay/d_vd.c"

#include "dsp/tab/d_tabwrite.c"
#include "dsp/tab/d_tabread.c"
#include "dsp/tab/d_tabread4.c"
#include "dsp/tab/d_tabplay.c"
#include "dsp/tab/d_tabosc4.c"
#include "dsp/tab/d_tabsend.c"
#include "dsp/tab/d_tabreceive.c"

#include "dsp/math/d_add.c"
#include "dsp/math/d_subtract.c"
#include "dsp/math/d_multiply.c"
#include "dsp/math/d_divide.c"
#include "dsp/math/d_greater.c"
#include "dsp/math/d_less.c"
#include "dsp/math/d_max.c"
#include "dsp/math/d_min.c"
#include "dsp/math/d_clip.c"
#include "dsp/math/d_abs.c"
#include "dsp/math/d_wrap.c"
#include "dsp/math/d_sqrt.c"
#include "dsp/math/d_rsqrt.c"
#include "dsp/math/d_pow.c"
#include "dsp/math/d_exp.c"
#include "dsp/math/d_log.c"
#include "dsp/math/d_mtof.c"
#include "dsp/math/d_ftom.c"
#include "dsp/math/d_dbtorms.c"
#include "dsp/math/d_rmstodb.c"
#include "dsp/math/d_dbtopow.c"
#include "dsp/math/d_powtodb.c"

#include "dsp/miscellaneous/d_print.c"
#include "dsp/miscellaneous/d_bang.c"
#include "dsp/miscellaneous/d_noise.c"
#include "dsp/miscellaneous/d_lrshift.c"

#include "dsp/soundfile/d_soundfile.c"
#include "dsp/soundfile/d_subchunk.c"
#include "dsp/soundfile/d_codec.c"
#include "dsp/soundfile/d_soundfiler.c"
#include "dsp/soundfile/d_soundinfo.c"
#include "dsp/soundfile/d_sfthread.c"
#include "dsp/soundfile/d_readsf.c"
#include "dsp/soundfile/d_writesf.c"

#include "dsp/filters/d_vcf.c"
#include "dsp/filters/d_hip.c"
#include "dsp/filters/d_lop.c"
#include "dsp/filters/d_bp.c"
#include "dsp/filters/d_biquad.c"
#include "dsp/filters/d_rpole.c"
#include "dsp/filters/d_rzero.c"
#include "dsp/filters/d_rzeroreverse.c"
#include "dsp/filters/d_cpole.c"
#include "dsp/filters/d_czero.c"
#include "dsp/filters/d_czeroreverse.c"

#include "dsp/fft/d_rfft.c"
#include "dsp/fft/d_rifft.c"
#include "dsp/fft/d_fft.c"
#include "dsp/fft/d_ifft.c"
#include "dsp/fft/d_framp.c"
#include "dsp/fft/d_mag.c"
#include "dsp/fft/d_rmag.c"

/* FFT library. */

#include "dsp/d_fftOOURA.c"
#include "dsp/fftsg.c"

/* Audio / MIDI backends. */

#if PD_WITH_DUMMY

    #include "system/s_midi_dummy.c"
    #include "system/s_audio_dummy.c"

#else

#if PD_APPLE
    #include "system/s_midi_pm.c"
    #include "system/s_audio_pa.c"
    #include "libs/pa_mac_hostapis.c"
#endif

#if PD_LINUX
    #include "system/s_midi_alsa.c"
    #include "system/s_audio_jack.c"
#endif

#endif // PD_WITH_DUMMY

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
