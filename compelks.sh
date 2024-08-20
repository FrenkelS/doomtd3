mkdir GCCIA16

unset CFLAGS

export CPU=i286

export CFLAGS="-march=$CPU -mcmodel=medium -melks -Os"

export GLOBOBJS="  d_items.c"
export GLOBOBJS+=" d_main.c"
export GLOBOBJS+=" g_game.c"
export GLOBOBJS+=" i_elks.c"
export GLOBOBJS+=" info.c"
export GLOBOBJS+=" m_random.c"
export GLOBOBJS+=" p_doors.c"
export GLOBOBJS+=" p_enemy.c"
export GLOBOBJS+=" p_floor.c"
export GLOBOBJS+=" p_inter.c"
export GLOBOBJS+=" p_lights.c"
export GLOBOBJS+=" p_map.c"
export GLOBOBJS+=" p_maputl.c"
export GLOBOBJS+=" p_mobj.c"
export GLOBOBJS+=" p_plats.c"
export GLOBOBJS+=" p_pspr.c"
export GLOBOBJS+=" p_scroll.c"
export GLOBOBJS+=" p_setup.c"
export GLOBOBJS+=" p_sight.c"
export GLOBOBJS+=" p_spec.c"
export GLOBOBJS+=" p_switch.c"
export GLOBOBJS+=" p_tick.c"
export GLOBOBJS+=" p_user.c"
export GLOBOBJS+=" r_data.c"
export GLOBOBJS+=" r_draw.c"
export GLOBOBJS+=" r_plane.c"
export GLOBOBJS+=" r_things.c"
export GLOBOBJS+=" st_stuff.c"
export GLOBOBJS+=" tables.c"
export GLOBOBJS+=" v_video.c"
export GLOBOBJS+=" w_wad.c"
export GLOBOBJS+=" z_zone.c"

ia16-elf-gcc $GLOBOBJS $CFLAGS -o GCCIA16/DOOMTD3E
