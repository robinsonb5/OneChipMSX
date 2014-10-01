#include "osd.h"
#include "menu.h"
#include "keyboard.h"


static struct menu_entry *menu;
static int menu_visible=0;
int menu_toggle_bits;
static int menurows;
static int currentrow;


void Menu_Show()
{
	OSD_Show(menu_visible=1);
}

void Menu_Hide()
{
	OSD_Show(menu_visible=0);
}


static void DrawSlider(struct menu_entry *m)
{
	int i;
	for(i=0;i<=MENU_SLIDER_MAX(m);++i) // One extra character to leave a space before the label
	{
		OSD_Putchar(i<MENU_SLIDER_VALUE(m) ? 0x07 : 0x20);
	}
}


void Menu_Draw()
{
	struct menu_entry *m=menu;
	OSD_Clear();
	menurows=0;
	while(m->type!=MENU_ENTRY_NULL)
	{
		int i;
		char **labels;
		OSD_SetX(2);
		OSD_SetY(menurows);
		switch(m->type)
		{
			case MENU_ENTRY_CYCLE:
				i=MENU_CYCLE_VALUE(m);	// Access the first byte
				labels=(char**)m->label;
				OSD_Puts("\x16 ");
				OSD_Puts(labels[i]);
				break;
			case MENU_ENTRY_SLIDER:
				DrawSlider(m);
				OSD_Puts(m->label);
				break;
			case MENU_ENTRY_TOGGLE:
				if((menu_toggle_bits>>MENU_ACTION_TOGGLE(m->action))&1)
					OSD_Puts("\x14 ");
				else
					OSD_Puts("\x15 ");
				// Fall through
			default:
				OSD_Puts(m->label);
				break;
		}
		++menurows;
		m++;
	}
}


void Menu_Set(struct menu_entry *head)
{
	menu=head;
	Menu_Draw();
	currentrow=menurows-1;
}


int Menu_Run()
{
	int i;
	struct menu_entry *m=menu;

	if((TestKey(KEY_F11)&2)||(TestKey(KEY_F12)&2))
	{
		while(TestKey(KEY_F12))
			HandlePS2RawCodes(); // Wait for KeyUp message before opening OSD, since this disables the keyboard for the MSX core.
		while(TestKey(KEY_F11))
			HandlePS2RawCodes(); // Wait for KeyUp message before opening OSD, since this disables the keyboard for the MSX core.
		OSD_Show(menu_visible^=1);
	}

	if(!menu_visible)	// Swallow any keystrokes that occur while the OSD is hidden...
	{
		TestKey(KEY_ENTER);
		TestKey(KEY_UPARROW);
		TestKey(KEY_DOWNARROW);
		TestKey(KEY_LEFTARROW);
		TestKey(KEY_RIGHTARROW);
		return;
	}
	if((TestKey(KEY_UPARROW)&2)&&currentrow)
		--currentrow;
	if((TestKey(KEY_DOWNARROW)&2)&&(currentrow<(menurows-1)))
		++currentrow;

	// Find the currently highlighted menu item
	i=currentrow;
	while(i)
	{
		++m;
		--i;
	}

	OSD_SetX(2);
	OSD_SetY(currentrow);

	if(TestKey(KEY_LEFTARROW)&2) // Decrease slider value
	{
		switch(m->type)
		{
			case MENU_ENTRY_SLIDER:
				if((--MENU_SLIDER_VALUE(m))&0x80) // <0?
					MENU_SLIDER_VALUE(m)=0;
				DrawSlider(m);
				break;
			default:
				break;
		}
	}

	if(TestKey(KEY_RIGHTARROW)&2) // Increase slider value
	{
		switch(m->type)
		{
			case MENU_ENTRY_SLIDER:
				if((++MENU_SLIDER_VALUE(m))>MENU_SLIDER_MAX(m))
					MENU_SLIDER_VALUE(m)=MENU_SLIDER_MAX(m);
				DrawSlider(m);
				break;
			default:
				break;
		}
	}


	if(TestKey(KEY_ENTER)&2)
	{
		struct menu_entry *m=menu;
		i=currentrow;
		while(i)
		{
			++m;
			--i;
		}
		switch(m->type)
		{
			case MENU_ENTRY_SUBMENU:
				Menu_Set(MENU_ACTION_SUBMENU(m->action));
				break;
			case MENU_ENTRY_CALLBACK:
				MENU_ACTION_CALLBACK(m->action)();
				break;
			case MENU_ENTRY_TOGGLE:
				i=1<<MENU_ACTION_TOGGLE(m->action);
				menu_toggle_bits^=i;
				Menu_Draw();
				break;
			case MENU_ENTRY_CYCLE:
				i=MENU_CYCLE_VALUE(m)+1;
				if(i>=MENU_CYCLE_COUNT(m))
					i=0;
				MENU_CYCLE_VALUE(m)=i;
				Menu_Draw();
				break;
			default:
				break;

		}
	}

	for(i=0;i<OSD_ROWS-1;++i)
	{
		OSD_SetX(0);
		OSD_SetY(i);
		OSD_Putchar(i==currentrow ? (i==menurows-1 ? 17 : 16) : 32);
	}

	return(menu_visible);
}

