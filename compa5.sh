mkdir amiga

export PATH=~/amiga-gcc-build/bin:$PATH

unset CFLAGS

#export RENDER_OPTIONS="-DONE_WALL_TEXTURE -DFLAT_WALL"
export RENDER_OPTIONS=""

m68k-amigaos-gcc -c i_amiga.c  $RENDER_OPTIONS -march=68000 -Ofast -fomit-frame-pointer -fgcse-sm -fgcse-las -fipa-pta -flto -fwhole-program -funroll-loops -freorder-blocks-algorithm=simple
m68k-amigaos-gcc -c p_map.c    $RENDER_OPTIONS -march=68000 -Ofast -fomit-frame-pointer -fgcse-sm -fgcse-las -fipa-pta -flto -fwhole-program -funroll-loops -freorder-blocks-algorithm=simple
m68k-amigaos-gcc -c p_maputl.c $RENDER_OPTIONS -march=68000 -Ofast -fomit-frame-pointer -fgcse-sm -fgcse-las -fipa-pta -flto -fwhole-program -funroll-loops -freorder-blocks-algorithm=simple
m68k-amigaos-gcc -c p_sight.c  $RENDER_OPTIONS -march=68000 -Ofast -fomit-frame-pointer -fgcse-sm -fgcse-las -fipa-pta -flto -fwhole-program -funroll-loops -freorder-blocks-algorithm=simple
m68k-amigaos-gcc -c r_draw.c   $RENDER_OPTIONS -march=68000 -Ofast -fomit-frame-pointer -fgcse-sm -fgcse-las -fipa-pta -flto -fwhole-program -funroll-loops -freorder-blocks-algorithm=simple
m68k-amigaos-gcc -c r_plane.c  $RENDER_OPTIONS -march=68000 -Ofast -fomit-frame-pointer -fgcse-sm -fgcse-las -fipa-pta -flto -fwhole-program -funroll-loops -freorder-blocks-algorithm=simple
m68k-amigaos-gcc -c tables.c   $RENDER_OPTIONS -march=68000 -Ofast -fomit-frame-pointer -fgcse-sm -fgcse-las -fipa-pta -flto -fwhole-program -funroll-loops -freorder-blocks-algorithm=simple
m68k-amigaos-gcc -c w_wad.c    $RENDER_OPTIONS -march=68000 -Ofast -fomit-frame-pointer -fgcse-sm -fgcse-las -fipa-pta -flto -fwhole-program -funroll-loops -freorder-blocks-algorithm=simple
m68k-amigaos-gcc -c z_zone.c   $RENDER_OPTIONS -march=68000 -Ofast -fomit-frame-pointer -fgcse-sm -fgcse-las -fipa-pta -flto -fwhole-program -funroll-loops -freorder-blocks-algorithm=simple

export CFLAGS="-march=68000 -mcrt=nix13 -Os -fomit-frame-pointer -fgcse-sm -fgcse-las -fipa-pta -flto -fwhole-program -funroll-loops -freorder-blocks-algorithm=stc"
#export CFLAGS="$CFLAGS -Wno-attributes -Wpedantic"
#export CFLAGS="$CFLAGS -Wall -Wextra"

export GLOBOBJS="  d_items.c"
export GLOBOBJS+=" d_main.c"
export GLOBOBJS+=" g_game.c"
export GLOBOBJS+=" i_timer.c"
#export GLOBOBJS+=" i_amiga.c"
export GLOBOBJS+=" i_amiga.o"
export GLOBOBJS+=" info.c"
export GLOBOBJS+=" m_random.c"
export GLOBOBJS+=" p_doors.c"
export GLOBOBJS+=" p_enemy.c"
export GLOBOBJS+=" p_floor.c"
export GLOBOBJS+=" p_inter.c"
export GLOBOBJS+=" p_lights.c"
#export GLOBOBJS+=" p_map.c"
export GLOBOBJS+=" p_map.o"
#export GLOBOBJS+=" p_maputl.c"
export GLOBOBJS+=" p_maputl.o"
export GLOBOBJS+=" p_mobj.c"
export GLOBOBJS+=" p_plats.c"
export GLOBOBJS+=" p_pspr.c"
export GLOBOBJS+=" p_setup.c"
#export GLOBOBJS+=" p_sight.c"
export GLOBOBJS+=" p_sight.o"
export GLOBOBJS+=" p_spec.c"
export GLOBOBJS+=" p_switch.c"
export GLOBOBJS+=" p_tick.c"
export GLOBOBJS+=" p_user.c"
export GLOBOBJS+=" r_data.c"
#export GLOBOBJS+=" r_draw.c"
export GLOBOBJS+=" r_draw.o"
#export GLOBOBJS+=" r_plane.c"
export GLOBOBJS+=" r_plane.o"
export GLOBOBJS+=" r_things.c"
export GLOBOBJS+=" st_stuff.c"
#export GLOBOBJS+=" tables.c"
export GLOBOBJS+=" tables.o"
export GLOBOBJS+=" v_video.c"
#export GLOBOBJS+=" w_wad.c"
export GLOBOBJS+=" w_wad.o"
export GLOBOBJS+=" z_bmallo.c"
#export GLOBOBJS+=" z_zone.c"
export GLOBOBJS+=" z_zone.o"

m68k-amigaos-gcc $GLOBOBJS $CFLAGS $RENDER_OPTIONS -o amiga/doomtd3

rm i_amiga.o
rm p_map.o
rm p_maputl.o
rm p_sight.o
rm r_draw.o
rm r_plane.o
rm tables.o
rm w_wad.o
rm z_zone.o
