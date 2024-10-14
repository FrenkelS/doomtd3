CCOPTSS="-os -bt=none -0 -zq -s -mm -wx -zastd=c99 -zls"

wcc $CCOPTSS d_items.c  -fo=d_items.obj
wcc $CCOPTSS d_main.c   -fo=d_main.obj
wcc $CCOPTSS g_game.c   -fo=g_game.obj
wcc $CCOPTSS i_elks.c   -fo=i_elks.obj
wcc $CCOPTSS info.c     -fo=info.obj
wcc $CCOPTSS m_random.c -fo=m_random.obj
wcc $CCOPTSS p_doors.c  -fo=p_doors.obj
wcc $CCOPTSS p_enemy.c  -fo=p_enemy.obj
wcc $CCOPTSS p_floor.c  -fo=p_floor.obj
wcc $CCOPTSS p_inter.c  -fo=p_inter.obj
wcc $CCOPTSS p_lights.c -fo=p_lights.obj
wcc $CCOPTSS p_map.c    -fo=p_map.obj
wcc $CCOPTSS p_maputl.c -fo=p_maputl.obj
wcc $CCOPTSS p_mobj.c   -fo=p_mobj.obj
wcc $CCOPTSS p_plats.c  -fo=p_plats.obj
wcc $CCOPTSS p_pspr.c   -fo=p_pspr.obj
wcc $CCOPTSS p_scroll.c -fo=p_scroll.obj
wcc $CCOPTSS p_setup.c  -fo=p_setup.obj
wcc $CCOPTSS p_sight.c  -fo=p_sight.obj
wcc $CCOPTSS p_spec.c   -fo=p_spec.obj
wcc $CCOPTSS p_switch.c -fo=p_switch.obj
wcc $CCOPTSS p_tick.c   -fo=p_tick.obj
wcc $CCOPTSS p_user.c   -fo=p_user.obj
wcc $CCOPTSS r_data.c   -fo=r_data.obj
wcc $CCOPTSS r_draw.c   -fo=r_draw.obj
wcc $CCOPTSS r_things.c -fo=r_things.obj
wcc $CCOPTSS st_stuff.c -fo=st_stuff.obj
wcc $CCOPTSS tables.c   -fo=tables.obj
wcc $CCOPTSS v_video.c  -fo=v_video.obj
wcc $CCOPTSS w_wad.c    -fo=w_wad.obj
wcc $CCOPTSS z_zone.c   -fo=z_zone.obj

owcc -bos2 -s -Wl,option -Wl,start=_start -Wl,option -Wl,dosseg -Wl,option -Wl,nodefaultlibs -Wl,option -Wl,stack=0x1000 -Wl,option -Wl,heapsize=0x1000 -Wl,library -Wl,$LIBC -o doomtd3.os2 d_items.obj d_main.obj g_game.obj i_elks.obj info.obj m_random.obj p_doors.obj p_enemy.obj p_floor.obj p_inter.obj p_lights.obj p_map.obj p_maputl.obj p_mobj.obj p_plats.obj p_pspr.obj p_scroll.obj p_setup.obj p_sight.obj p_spec.obj p_switch.obj p_tick.obj p_user.obj r_data.obj r_draw.obj r_things.obj st_stuff.obj tables.obj v_video.obj w_wad.obj z_zone.obj

rm *.obj
rm *.err
