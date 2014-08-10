#include <stdio.h>
#include "z80-cpu.h"
#include "mini-display.h"
#include "keyboard.h"
#include "asm_interface.h"
#include "memory.h"
#include "hardware/port_buffer.h"

#ifdef  OLD_STYLE
static  FILE  *ports;
#endif

static int fd_out;
static int fd_in;


int
init_ports(void)
{
#ifdef  OLD_STYLE
   char  buffer[256];
#endif
   fd_out=fileno(stdout);
   fd_in=fileno(stdin);
#ifdef  OLD_STYLE
   ports= fopen(Z80_PORTS,"r+b");   /* read_writable_binary file */
   if (ports)
      if (256 != fread(buffer,1,256,ports))
      {  fclose(ports), ports= (FILE*)0;  }
   return !ports;
#else
   return init_port_buffer();
#endif
}


void
out_byte(unsigned char id, unsigned char data)
{
#ifndef  OLD_STYLE
   int i;
#endif
   if (cpu_pin[busrq]) acknowledge_bus_request();
   if (!cpu_is_in_disassemble) ADDRESS = (ADDRESS&~255) | id;
   DATA= data;
   set_cpu_pin(wr,1);
   set_cpu_pin(iorq,1);
   if (id == fd_out)
      display_in_line(data);
#ifdef  OLD_STYLE
   else if (ports)
   {  int i=fseek(ports,(long)id,SEEK_SET);
      if (i || 1 != fwrite(&data,1,1,ports))
         error(i,"hardware malfunction","port write:");
      fflush(ports);
      if (i=bank_port_index(id))
         switch_bank(data,i);
   }
#else
   else if (i=bank_port_index(id))
      switch_bank(data,i);
#endif
   if (!cpu_is_in_disassemble)
   {  wait_tics(TICS_MEMO);
      set_cpu_pin(wait,1);
      wait_tics(TICS_WAIT);
      set_cpu_pin(wait,0);
   }
   set_cpu_pin(iorq,0);
   set_cpu_pin(wr,0);
}

void
in_byte(unsigned char id, unsigned char *data)
{
   if (cpu_pin[busrq]) acknowledge_bus_request();
   if (!cpu_is_in_disassemble) ADDRESS = (ADDRESS&~255) | id;
   set_cpu_pin(rd,1);
   set_cpu_pin(iorq,1);
#ifdef  OLD_STYLE
   if (ports)
   {  int  i;  fflush(ports);
      i=fseek(ports,(long)id,SEEK_SET);
      if (i || 1 != fread(data,1,1,ports))
         error(i,"hardware malfunction","port read:");
      DATA= *data;
   }
#endif
   if (!cpu_is_in_disassemble)
   {  wait_tics(TICS_MEMO);
      set_cpu_pin(wait,1);
      wait_tics(TICS_WAIT);
      set_cpu_pin(wait,0);
   }
   if (id == fd_in)
   {  keystrobe(data);
      DATA= *data;
   }
   *data= DATA;
   set_cpu_pin(iorq,0);
   set_cpu_pin(rd,0);
}
