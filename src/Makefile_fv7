# makefile for Gloom

# ARCH=i386
ARCH=x86_64
CC=gcc

# BASE_CFLAGS=-Dstricmp=strcasecmp -DQ_strncasecmp=strncasecmp
BASE_CFLAGS=-Wall -fno-strict-aliasing -DC_ONLY -DQ_strncasecmp=strncasecmp
CFLAGS=$(BASE_CFLAGS) -Os -g -fPIC -Wall -fvisibility=hidden

SHLIBEXT=so
SHLIBLDFLAGS=-lm -shared

GAME_OBJS = \
G_cmds.o \
G_combat.o \
G_main.o \
G_weapon.o \
Grapple.o \
M_cocoon.o \
M_gasser.o \
M_healer.o \
M_spike.o \
P_client.o \
P_view.o \
P_weapon.o \
mersennetwister.o \
c4.o \
depot.o \
g_account.o \
g_breakin.o \
g_chase.o \
g_class.o \
g_func.o \
g_items.o \
g_log.o \
g_map.o \
g_misc.o \
g_monster.o \
g_phys.o \
g_save.o \
g_spawn.o \
g_svcmds.o \
g_target.o \
g_trigger.o \
g_turret.o \
g_utils.o \
g_wordfilter.o \
lmine.o \
m_breeder.o \
m_detector.o \
m_drone.o \
m_guardian.o \
m_hatchling.o \
m_obstacle.o \
m_stalker.o \
m_sting.o \
m_wraith.o \
p_biotech.o \
p_commando.o \
p_engineer.o \
p_exterm.o \
p_grunt.o \
p_heavy.o \
p_hud.o \
p_mech.o \
p_menu.o \
p_shotgun.o \
q_shared.o \
turret.o

all: game

game : $(GAME_OBJS)
	$(CC) -o game$(ARCH).$(SHLIBEXT) $(SHLIBCFLAGS) $(GAME_OBJS) $(SHLIBLDFLAGS)

dist : game
	rm -f *.gz
	gzip -9 -n game$(ARCH).$(SHLIBEXT)
	mv gamex86_64.so.gz gamex86_64.`date '+%Y-%m-%d'`.so.gz

#############################################################################
# MISC
#############################################################################

clean:
	-rm -f $(GAME_OBJS) game$(ARCH).$(SHLIBEXT) gamex86_64.so.gz

# general defines
$(GAME_OBJS) : g_local.h game.h q_shared.h

# individual defines INCOMPLETE
G_cmds.o: m_player.h
G_weapon.o c4.o : c4.h
P_view.o: m_mech.h m_stalker.h m_engineer.h m_wraith.h
P_weapon.o: m_player.h m_drone.h m_mech.h m_stalker.h m_guardian.h m_stinger.h m_wraith.h
g_class.o: m_player.h m_hatchling.h m_drone.h m_mech.h m_stalker.h \
	m_hsold.h m_breeder.h m_engineer.h m_guardian.h m_exterm.h \
	m_stinger.h m_wraith.h g_class.h
M_cocoon.o: m_cocoon.h
M_healer.o: m_healer.h
M_spike.o: m_spiker.h
g_breakin.o: m_breeder.h m_engineer.h m_player.h menus.h
m_breeder.o: m_breeder.h
m_gasser.o: m_obstacle.h
m_hatchling.o: m_hatchling.h
m_obstacle.o: m_obstacle.h
m_stalker.o: m_stalker.h
m_wraith.o: m_wraith.h
p_engineer.o: m_engineer.h
p_mech.o: m_mech.h
turret.o: turret.h
