/************************************
*
*   Config.h
*   
*
*   By Patrick Wyatt - 5/19/2010
*
***/


#ifdef CONFIG_H
#error "Header included more than once"
#endif
#define CONFIG_H


void ConfigInitialize ();
void ConfigDestroy ();

void ConfigMonitorFile (const wchar filename[]);

