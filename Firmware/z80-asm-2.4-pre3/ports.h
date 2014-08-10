/* PORT INTERFACE */

#ifndef __PORT_H
#define __PORT_H

extern int init_ports(void);
extern void out_byte(unsigned char id, unsigned char data);
extern void in_byte(unsigned char id, unsigned char *data);

#endif
