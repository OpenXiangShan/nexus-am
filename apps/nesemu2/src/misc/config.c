/***************************************************************************
 *   Copyright (C) 2013 by James Holodnak                                  *
 *   jamesholodnak@gmail.com                                               *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/

#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include "misc/config.h"
#include "misc/vars.h"
#include "misc/memutil.h"
#include "system/main.h"
#include "system/system.h"
#include "version.h"

static vars_t *configvars = 0;
char configfilename[1024] = CONFIG_FILENAME;
char exepath[1024] = "";

//initialize the configuration defaults
static vars_t *config_get_defaults()
{
	vars_t *ret = vars_create();

	vars_set_int   (ret,F_CONFIG,"video.framelimit",			1);
	vars_set_int   (ret,F_CONFIG,"video.fullscreen",			0);
	vars_set_int   (ret,F_CONFIG,"video.scale",					1);
	vars_set_string(ret,F_CONFIG,"video.filter",					"none");

	vars_set_string(ret,F_CONFIG,"input.port0",					"joypad0");
	vars_set_string(ret,F_CONFIG,"input.port1",					"joypad1");
	vars_set_string(ret,F_CONFIG,"input.expansion",				"none");

	vars_set_int   (ret,F_CONFIG,"input.joypad0.a",				'j');
	vars_set_int   (ret,F_CONFIG,"input.joypad0.b",				'k');
	vars_set_int   (ret,F_CONFIG,"input.joypad0.select",	'u');
	vars_set_int   (ret,F_CONFIG,"input.joypad0.start",		'i');
	vars_set_int   (ret,F_CONFIG,"input.joypad0.up",			'w');
	vars_set_int   (ret,F_CONFIG,"input.joypad0.down",		's');
	vars_set_int   (ret,F_CONFIG,"input.joypad0.left",		'a');
	vars_set_int   (ret,F_CONFIG,"input.joypad0.right",		'd');

	vars_set_int   (ret,F_CONFIG,"input.joypad1.a",				'1');
	vars_set_int   (ret,F_CONFIG,"input.joypad1.b",				'2');
	vars_set_int   (ret,F_CONFIG,"input.joypad1.select",	'3');
	vars_set_int   (ret,F_CONFIG,"input.joypad1.start",		'4');
	vars_set_int   (ret,F_CONFIG,"input.joypad1.up",			'5');
	vars_set_int   (ret,F_CONFIG,"input.joypad1.down",		'6');
	vars_set_int   (ret,F_CONFIG,"input.joypad1.left",		'7');
	vars_set_int   (ret,F_CONFIG,"input.joypad1.right",		'8');

	return(ret);
}

int config_init()
{
	vars_t *v;

	configvars = config_get_defaults();
	if((v = vars_load(configfilename)) == 0) {
		log_printf("config_init:  unable to load file, using defaults\n");
	}
	else {
		//merge the loaded variables with the defaults
		vars_merge(configvars,v);

		//destroy the loaded vars
		vars_destroy(v);
	}

	return(0);
}

void config_kill()
{
	vars_t *v = configvars;

	if(v == 0) {
		log_printf("config_kill:  internal error!  configvars = 0.\n");
		return;
	}
	vars_save(v,configfilename);
	vars_destroy(v);
}

//gets config string variable with variables expanded
char *config_get_eval_string(char *dest,char *name)
{
	char *tmp,*p,*p2;
	char varname[64];
	int pos;

	//get string
	if((p = config_get_string(name)) == 0)
		return(0);

	//make a copy of the string
	tmp = mem_strdup(p);

	strcpy(dest,tmp);

	for(pos=0,p=tmp;*p;p++) {

		//see if we find a '%'
		if(*p == '%') {

			//clear area to hold var name
			memset(varname,0,64);

			//see if it is missing the '%'
			if((p2 = strchr(p + 1,'%')) == 0) {
				log_printf("config_get_eval_string:  missing ending '%', just copying\n");
			}

			//not missing, replace with variable data
			else {
				//skip over the '%'
				p++;

				//terminate the substring
				*p2 = 0;

				//copy substring to varname array
				strcpy(varname,p);

				//set new position in the string we parsing
				p = p2 + 1;

				if(strcmp(varname,name) == 0) {
					log_printf("config_get_eval_string:  variable cannot reference itself (var '%s')\n",varname);
				}
				else {
					char *tmp2 = (char*)mem_alloc(1024);

					p2 = var_get_eval_string(tmp2,varname);
					if(p2 == 0) {
						log_printf("config_get_eval_string:  variable '%s' referenced non-existant variable '%s', using '.'\n",name,varname);
						dest[pos++] = '.';
						dest[pos] = 0;
					}
					else {
						while(*p2) {
							dest[pos++] = *p2++;
							dest[pos] = 0;
						}
					}
					mem_free(tmp2);
				}
			}
		}

		//copy the char
		dest[pos++] = *p;
		dest[pos] = 0;
	}

	//free tmp string and return
	mem_free(tmp);
	return(dest);
}

//get config var (wraps the vars_get_*() functions)
char *config_get_string(char *name)		{	return(vars_get_string(configvars,name,""));		}
int config_get_int(char *name)			{	return(vars_get_int   (configvars,name,0));		}
int config_get_bool(char *name)			{	return(vars_get_bool  (configvars,name,0));		}
double config_get_double(char *name)	{	return(vars_get_double(configvars,name,0.0f));	}

//set config var (wraps the vars_get_*() functions)
void config_set_string(char *name,char *data)	{	vars_set_string(configvars,F_CONFIG,name,data);	}
void config_set_int(char *name,int data)			{	vars_set_int   (configvars,F_CONFIG,name,data);	}
void config_set_bool(char *name,int data)			{	vars_set_bool  (configvars,F_CONFIG,name,data);	}
void config_set_double(char *name,double data)	{	vars_set_double(configvars,F_CONFIG,name,data);	}

//set var (wraps the vars_get_*() functions)
void var_set_string(char *name,char *data)	{	vars_set_string(configvars,0,name,data);	}
void var_set_int(char *name,int data)			{	vars_set_int   (configvars,0,name,data);	}
void var_set_bool(char *name,int data)			{	vars_set_bool  (configvars,0,name,data);	}
void var_set_double(char *name,double data)	{	vars_set_double(configvars,0,name,data);	}

void var_unset(char *name)
{
	vars_delete_var(configvars,name);
}

//semi-kludge for the 'set' command
var_t *config_get_head()
{
	return(configvars->vars);
}
