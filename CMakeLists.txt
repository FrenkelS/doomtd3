# To use this example as a standalone project using CMake:
# mkdir Retro68
# cd Retro68
# cmake .. -DCMAKE_TOOLCHAIN_FILE=path/to/Retro68-build/toolchain/m68k-apple-macos/cmake/retro68.toolchain.cmake
# make

add_application(DOOMTD3
    d_items.c
    d_main.c
    g_game.c
    i_timer.c
    i_mac.c
    info.c
    m_random.c
    p_doors.c
    p_enemy.c
    p_floor.c
    p_inter.c
    p_lights.c
    p_map.c
    p_maputl.c
    p_mobj.c
    p_plats.c
    p_pspr.c
    p_setup.c
    p_sight.c
    p_spec.c
    p_switch.c
    p_tick.c
    p_user.c
    r_data.c
    r_draw.c
    r_plane.c
    r_things.c
    st_stuff.c
    tables.c
    v_video.c
    w_wad.c
    z_bmallo.c
    z_zone.c
    )
target_link_libraries(DOOMTD3 "-lm")

# save 200KB of code by removing unused stuff
set_target_properties(DOOMTD3 PROPERTIES COMPILE_OPTIONS "-mcpu=68000;-Ofast;-fomit-frame-pointer;-fgcse-sm;-fgcse-las;-fipa-pta;-funroll-loops;-freorder-blocks-algorithm=stc" LINK_FLAGS "-Wl,-gc-sections")
