/*
    Quake II Glooom, a total conversion mod for Quake II
    Copyright (C) 1999-2007  Gloom Developers

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

void OpenClassMenuWrapper (edict_t *ent);
void CTFChaseCam(edict_t *ent);
void Back(edict_t *ent);
void SpawnClass(edict_t *ent);
void Activate_Upgrade (edict_t *ent);

//  build menus

pmenu_t breeder_menu[] = {
	{ "",	0, NULL },
	{ "",	0, NULL },
	{ "", 0, lay_egg },
	{ "", 0, lay_obstacle },
	{ "", 0, lay_spiker },
	{ "", 0, lay_healer },
	{ "", 0, lay_gasser },
	{ NULL,	0, NULL }
};

pmenu_t engineer_menu[] = {
	{ "", 0, NULL },
	{ "", 0, NULL },
	{ "", 0, create_teleporter},
	{ "", 0, create_turret},
	{ "", 0, create_mgturret},
	{ "", 0, create_detector},
	{ "", 0, create_tripwire},
	{ "", 0, create_depot},
	{ NULL,	0, NULL }
};

//  team menus

pmenu_t alien_menu[] = {	
	{ "", 0, NULL },
	{ "", 0, NULL },
	{ "", 0, SpawnClass },
	{ "", 0, SpawnClass },
	{ "", 0, SpawnClass },
	{ "", 0, SpawnClass },
	{ "", 0, SpawnClass },
	{ "", 0, SpawnClass },
	{ "", 0, SpawnClass },
	{ "", 0, SpawnClass },
	{ "", 0, NULL },
	{ "", 0, NULL },
	{ "", 0, NULL },
	{ "", 0, NULL },
	{ "", 0, NULL },
	{ "", 0, Back },
	{ NULL,	0, NULL }
};
			
pmenu_t human_menu[] = {	
	{ "", 0, NULL },
	{ "", 0, NULL },
	{ "", 0, SpawnClass },
	{ "", 0, SpawnClass },
	{ "", 0, SpawnClass },
	{ "", 0, SpawnClass },
	{ "", 0, SpawnClass },
	{ "", 0, SpawnClass },
	{ "", 0, SpawnClass },
	{ "", 0, SpawnClass },
	{ "", 0, NULL },
	{ "", 0, NULL },
	{ "", 0, NULL },
	{ "", 0, NULL },
	{ "", 0, NULL },
	{ "", 0, Back },
 	{ NULL,	0, NULL }
};

pmenu_t upgrade_menu_exterm[] = 
{
		{ "*Exterminator Upgrades", PMENU_ALIGN_CENTER, NULL  },
		{ "", 0, NULL },
		{ "", 0, NULL },
		{ "2 - Charon Boosters", 0, Activate_Upgrade },
		{ "8 - Suit Supercharger", 0, Activate_Upgrade },
		{ "2 - Advanced Coolant", 0, Activate_Upgrade },
		{ "1 - Shrapnel", 0, Activate_Upgrade },
		{ "", 0, NULL },
		{ "", 0, NULL },
		{ "", 0, NULL },
		{ "", 0, NULL },
		{ "", 0, NULL },
		{ "", 0, NULL },
		{ "", 0, NULL },
		{ "", 0, NULL },
		{ "Close", 0, PMenu_Close },
 		{ NULL,	0, NULL }
};

pmenu_t upgrade_menu_mech[] = 
{
		{ "*Mech Upgrades", PMENU_ALIGN_CENTER, NULL  },
		{ "", 0, NULL },
		{ "", 0, NULL },
		{ "1 - Depot Interface", 0, Activate_Upgrade },
		{ "6 - Doomsday Device", 0, Activate_Upgrade },
		{ "2 - Rocket Science", 0, Activate_Upgrade },
		{ "", 0, NULL },
		{ "", 0, NULL },
		{ "", 0, NULL },
		{ "", 0, NULL },
		{ "", 0, NULL },
		{ "", 0, NULL },
		{ "", 0, NULL },
		{ "", 0, NULL },
		{ "", 0, NULL },
		{ "Close", 0, PMenu_Close },
 		{ NULL,	0, NULL }
};

pmenu_t upgrade_menu_guardian[] = 
{
		{ "*Guardian Upgrades", PMENU_ALIGN_CENTER, NULL  },
		{ "", 0, NULL },
		{ "", 0, NULL },
		{ "5 - Cellulose Pigment", 0, Activate_Upgrade },
		{ "1 - Brachii Evolvement", 0, Activate_Upgrade },
		{ "", 0, NULL },
		{ "", 0, NULL },
		{ "", 0, NULL },
		{ "", 0, NULL },
		{ "", 0, NULL },
		{ "", 0, NULL },
		{ "", 0, NULL },
		{ "", 0, NULL },
		{ "", 0, NULL },
		{ "", 0, NULL },
		{ "Close", 0, PMenu_Close },
 		{ NULL,	0, NULL }
};

/*Savvy edit added in my own upgrade - explosive mag shots*/
pmenu_t upgrade_menu_commando[] = 
{
		{ "*Commando Upgrades", PMENU_ALIGN_CENTER, NULL  },
		{ "", 0, NULL },
		{ "", 0, NULL },
		{ "1 - Cyborg Ligaments", 0, Activate_Upgrade },
		{ "2 - Shrapnel", 0, Activate_Upgrade },
		{ "1 - D3ATH Shells", 0, Activate_Upgrade },
		{ "", 0, NULL },
		{ "", 0, NULL },
		{ "", 0, NULL },
		{ "", 0, NULL },
		{ "", 0, NULL },
		{ "", 0, NULL },
		{ "", 0, NULL },
		{ "", 0, NULL },
		{ "", 0, NULL },
		{ "Close", 0, PMenu_Close },
 		{ NULL,	0, NULL }
};


pmenu_t upgrade_menu_ht[] = 
{
		{ "*HT Upgrades", PMENU_ALIGN_CENTER, NULL  },
		{ "", 0, NULL },
		{ "", 0, NULL },
		{ "1 - Stealth Plating", 0, Activate_Upgrade },
		{ "2 - Shrapnel", 0, Activate_Upgrade },
		{ "", 0, NULL },
		{ "", 0, NULL },
		{ "", 0, NULL },
		{ "", 0, NULL },
		{ "", 0, NULL },
		{ "", 0, NULL },
		{ "", 0, NULL },
		{ "", 0, NULL },
		{ "", 0, NULL },
		{ "", 0, NULL },
		{ "Close", 0, PMenu_Close },
 		{ NULL,	0, NULL }
};

/*Savvy edit enhanced hatchling damage upgrade added*/
pmenu_t upgrade_menu_hatch[] = 
{
		{ "*Hatchling Upgrades", PMENU_ALIGN_CENTER, NULL  },
		{ "", 0, NULL },
		{ "", 0, NULL },
		{ "1 - Translucent Membrane", 0, Activate_Upgrade },
		{ "1 - Transgenetic Infiltration", 0, Activate_Upgrade },
		{ "1 - Martyrdom", 0, Activate_Upgrade },
		{ "", 0, NULL },
		{ "", 0, NULL },
		{ "", 0, NULL },
		{ "", 0, NULL },
		{ "", 0, NULL },
		{ "", 0, NULL },
		{ "", 0, NULL },
		{ "", 0, NULL },
		{ "", 0, NULL },
		{ "Close", 0, PMenu_Close },
 		{ NULL,	0, NULL }
};

pmenu_t upgrade_menu_drone[] = 
{
		{ "*Drone Upgrades", PMENU_ALIGN_CENTER, NULL  },
		{ "", 0, NULL },
		{ "", 0, NULL },
		{ "1 - Mucus Glands", 0, Activate_Upgrade },
		{ "", 0, NULL },
		{ "", 0, NULL },
		{ "", 0, NULL },
		{ "", 0, NULL },
		{ "", 0, NULL },
		{ "", 0, NULL },
		{ "", 0, NULL },
		{ "", 0, NULL },
		{ "", 0, NULL },
		{ "", 0, NULL },
		{ "", 0, NULL },
		{ "Close", 0, PMenu_Close },
 		{ NULL,	0, NULL }
};

pmenu_t upgrade_menu_kami[] = 
{
		{ "*Kamikaze Upgrades", PMENU_ALIGN_CENTER, NULL  },
		{ "", 0, NULL },
		{ "", 0, NULL },
		{ "1 - Spike Pouch", 0, Activate_Upgrade },
		{ "", 0, NULL },
		{ "", 0, NULL },
		{ "", 0, NULL },
		{ "", 0, NULL },
		{ "", 0, NULL },
		{ "", 0, NULL },
		{ "", 0, NULL },
		{ "", 0, NULL },
		{ "", 0, NULL },
		{ "", 0, NULL },
		{ "", 0, NULL },
		{ "Close", 0, PMenu_Close },
 		{ NULL,	0, NULL }
};

pmenu_t upgrade_menu_grunt[] = 
{
		{ "*Grunt Upgrades", PMENU_ALIGN_CENTER, NULL  },
		{ "", 0, NULL },
		{ "", 0, NULL },
		{ "1 - Stimpacks", 0, Activate_Upgrade },
		{ "1 - Smartgun", 0, Activate_Upgrade },
		{ "", 0, NULL },
		{ "", 0, NULL },
		{ "", 0, NULL },
		{ "", 0, NULL },
		{ "", 0, NULL },
		{ "", 0, NULL },
		{ "", 0, NULL },
		{ "", 0, NULL },
		{ "", 0, NULL },
		{ "", 0, NULL },
		{ "", 0, NULL },
		{ "Close", 0, PMenu_Close },
 		{ NULL,	0, NULL }
};

pmenu_t upgrade_menu_engi[] = 
{
		{ "*Engineer Upgrades", PMENU_ALIGN_CENTER, NULL  },
		{ "", 0, NULL },
		{ "", 0, NULL },
		{ "1 - Phobos Reactor", 0, Activate_Upgrade },
		{ "", 0, NULL },
		{ "", 0, NULL },
		{ "", 0, NULL },
		{ "", 0, NULL },
		{ "", 0, NULL },
		{ "", 0, NULL },
		{ "", 0, NULL },
		{ "", 0, NULL },
		{ "", 0, NULL },
		{ "", 0, NULL },
		{ "", 0, NULL },
		{ "Close", 0, PMenu_Close },
 		{ NULL,	0, NULL }
};

pmenu_t upgrade_menu_wraith[] = 
{
		{ "*Wraith Upgrades", PMENU_ALIGN_CENTER, NULL  },
		{ "", 0, NULL },
		{ "", 0, NULL },
		{ "1 - Venom Glands", 0, Activate_Upgrade },
		{ "", 0, NULL },
		{ "", 0, NULL },
		{ "", 0, NULL },
		{ "", 0, NULL },
		{ "", 0, NULL },
		{ "", 0, NULL },
		{ "", 0, NULL },
		{ "", 0, NULL },
		{ "", 0, NULL },
		{ "", 0, NULL },
		{ "", 0, NULL },
		{ "Close", 0, PMenu_Close },
 		{ NULL,	0, NULL }
};

pmenu_t upgrade_menu_bio[] = {
		{ "*Biotech Upgrades", PMENU_ALIGN_CENTER, NULL  },
		{ "", 0, NULL },
		{ "", 0, NULL },
		{ "1 - Medtech Kit", 0, Activate_Upgrade },
		{ "1 - Battletech Pack", 0, Activate_Upgrade },
		{ "1 - Flare Pack", 0, Activate_Upgrade },
		{ "", 0, NULL },
		{ "", 0, NULL },
		{ "", 0, NULL },
		{ "", 0, NULL },
		{ "", 0, NULL },
		{ "", 0, NULL },
		{ "", 0, NULL },
		{ "", 0, NULL },
		{ "", 0, NULL },
		{ "Close", 0, PMenu_Close },
 		{ NULL,	0, NULL }
};

pmenu_t upgrade_menu_stalker[] = 
{
		{ "*Stalker Upgrades", PMENU_ALIGN_CENTER, NULL  },
		{ "", 0, NULL },
		{ "", 0, NULL },
		{ "1 - Spike Sac", 0, Activate_Upgrade },
		{ "1 - Anabolic Synthesis", 0, Activate_Upgrade },
		{ "", 0, NULL },
		{ "", 0, NULL },
		{ "", 0, NULL },
		{ "", 0, NULL },
		{ "", 0, NULL },
		{ "", 0, NULL },
		{ "", 0, NULL },
		{ "", 0, NULL },
		{ "", 0, NULL },
		{ "", 0, NULL },
		{ "Close", 0, PMenu_Close },
 		{ NULL,	0, NULL }
};

/*Savvy i added in stinger upgrades*/
pmenu_t upgrade_menu_stinger[] = 
{
		{ "*Stinger Upgrades", PMENU_ALIGN_CENTER, NULL  },
		{ "", 0, NULL },
		{ "", 0, NULL },
		{ "1 - Inferno", 0, Activate_Upgrade },
		{ "", 0, NULL },
		{ "", 0, NULL },
		{ "", 0, NULL },
		{ "", 0, NULL },
		{ "", 0, NULL },
		{ "", 0, NULL },
		{ "", 0, NULL },
		{ "", 0, NULL },
		{ "", 0, NULL },
		{ "", 0, NULL },
		{ "", 0, NULL },
		{ "Close", 0, PMenu_Close },
 		{ NULL,	0, NULL }
};

/*Savvy i added in shock trooper upgrades*/
pmenu_t upgrade_menu_shock_trooper[] = 
{
		{ "*Shock Trooper Upgrades", PMENU_ALIGN_CENTER, NULL  },
		{ "", 0, NULL },
		{ "", 0, NULL },
		{ "1 - Explosive Shell Pack", 0, Activate_Upgrade },
		{ "", 0, NULL },
		{ "", 0, NULL },
		{ "", 0, NULL },
		{ "", 0, NULL },
		{ "", 0, NULL },
		{ "", 0, NULL },
		{ "", 0, NULL },
		{ "", 0, NULL },
		{ "", 0, NULL },
		{ "", 0, NULL },
		{ "", 0, NULL },
		{ "Close", 0, PMenu_Close },
 		{ NULL,	0, NULL }
};

pmenu_t team_menu[] = {
	{ "", 0, NULL },
	{ "", 0, NULL },
	{ "", 0, OpenClassMenuWrapper },
	{ "", 0, NULL },   // dynamic data
	{ "", 0, OpenClassMenuWrapper },
	{ "", 0, NULL },   // dynamic data
	{ "", 0, CTFChaseCam },
	{ "", 0, NULL },
	{ "", 0, NULL },
	{ "", 0, NULL },
	{ "", 0, NULL },
	{ "", 0, NULL },	//motd
	{ "", 0, NULL },	//motd
	{ "", 0, NULL },	//motd
	{ "", 0, NULL },
	{ "", 0, NULL },
	{ "*"GLOOMVERSION,			PMENU_ALIGN_RIGHT, NULL },
	{ NULL,	0, NULL }
};
