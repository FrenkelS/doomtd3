mkdir amiga

export PATH=~/amiga-gcc-build/bin:$PATH

unset CFLAGS

#export RENDER_OPTIONS="-DONE_WALL_TEXTURE -DFLAT_WALL"
#export RENDER_OPTIONS="-DVIEWWINDOWWIDTH=30 -DVIEWWINDOWHEIGHT=64  -DHORIZONTAL_RESOLUTION=HORIZONTAL_RESOLUTION_LO -DVERTICAL_RESOLUTION_DOUBLED"
#export RENDER_OPTIONS="-DVIEWWINDOWWIDTH=30 -DVIEWWINDOWHEIGHT=128 -DHORIZONTAL_RESOLUTION=HORIZONTAL_RESOLUTION_LO"
export RENDER_OPTIONS=""

export CFLAGS="-mcpu=68000 -msmall-code -mcrt=nix13 -Ofast -fomit-frame-pointer -fgcse-sm -fgcse-las -fipa-pta -mregparm -flto -fwhole-program -funroll-loops -freorder-blocks-algorithm=stc"

export GLOBOBJS="  d_items.c"
export GLOBOBJS+=" d_main.c"
export GLOBOBJS+=" g_game.c"
export GLOBOBJS+=" i_amiga.c"
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
export GLOBOBJS+=" r_things.c"
export GLOBOBJS+=" st_stuff.c"
export GLOBOBJS+=" tables.c"
export GLOBOBJS+=" v_video.c"
export GLOBOBJS+=" w_wad.c"
export GLOBOBJS+=" z_zone.c"

m68k-amigaos-gcc $GLOBOBJS $CFLAGS $RENDER_OPTIONS -o amiga/doomtd3
