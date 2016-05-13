/* Cut-down, read-only PS/2 handler for OSD code. */

#include "small_printf.h"

#include "ps2.h"
#include "interrupts.h"
#include "keyboard.h"
#include "host.h"

void ps2_ringbuffer_init(struct ps2_ringbuffer *r)
{
	r->in_hw=0;
	r->in_cpu=0;
}


int ps2_ringbuffer_read(struct ps2_ringbuffer *r)
{
	unsigned char result;
	if(r->in_hw==r->in_cpu)
		return(-1);	// No characters ready
	DisableInterrupts();
	result=r->inbuf[r->in_cpu];
	r->in_cpu=(r->in_cpu+1) & (PS2_RINGBUFFER_SIZE-1);
	EnableInterrupts();
	return(result);
}

int ps2_ringbuffer_count(struct ps2_ringbuffer *r)
{
	if(r->in_hw>=r->in_cpu)
		return(r->in_hw-r->in_cpu);
	return(r->in_hw+PS2_RINGBUFFER_SIZE-r->in_cpu);
}

struct ps2_ringbuffer kbbuffer;

static volatile int intflag;

#define PS2_TIMEOUT 5
#define MOUSESETTLE 1

volatile int ps2_mousex;
volatile int ps2_mousey;
//int ps2_mousebuttons;
int mdataout=-1;
static int mdata[4];
static int midx=0,mctr=0;
static int mpacketsize=3; // 4 bytes for a wheel mouse.  (Chameleon firmware initializes the mouse in wheel mode.)
static int mousesettle;

void PS2Handler()
{
	int kbd;
	int mouse;

	DisableInterrupts();
	
	kbd=HW_PS2(REG_PS2_KEYBOARD);
	mouse=HW_PS2(REG_PS2_MOUSE);

	if(kbd & (1<<BIT_PS2_RECV))
	{
		kbbuffer.inbuf[kbbuffer.in_hw]=kbd&0xff;
		kbbuffer.in_hw=(kbbuffer.in_hw+1) & (PS2_RINGBUFFER_SIZE-1);
	}


	if(mouse & (1<<BIT_PS2_RECV))
	{
		mdata[midx++]=mouse&255;
		mctr=PS2_TIMEOUT;
		if(midx==mpacketsize)	// Complete packet received?
		{
			midx=0;
 			if(!mousesettle)  // Have we figured out the packet size yet?
			{
				HW_HOST(HW_HOST_MOUSEBUTTONS)=(~mdata[0])&3;
				if(mdata[0] & (1<<5))
					ps2_mousey-=1+(mdata[2]^255);
				else
					ps2_mousey+=mdata[2];
					// Reverse X axis
				if(mdata[0] & (1<<4))
					ps2_mousex+=1+(mdata[1]^255);
				else
					ps2_mousex-=mdata[1];
			}
		}	
	}
	else
	{
		if(!(mctr--))	// Timeout
		{
			if(midx)
			{
				mousesettle=MOUSESETTLE;
				mpacketsize=7-mpacketsize; // Alternate between 3 and 4 byte packets
			}
			else if(mousesettle)
				--mousesettle;
			midx=0;
		}
	}

	intflag=0;
	GetInterrupts();	// Clear interrupt bit
	EnableInterrupts();
}


void PS2Wait()
{
	DisableInterrupts();
	intflag=1;
	EnableInterrupts();
	while(intflag)
		;
}


void PS2Init()
{
	ps2_ringbuffer_init(&kbbuffer);
	mousesettle=0;
	mpacketsize=3;
	if(HW_PS2(REG_PS2_MOUSE)&(1<<BIT_PS2_MOUSE_FOURBYTE))
		mpacketsize=4;
	if(HW_PS2(REG_PS2_MOUSE)&(1<<BIT_PS2_MOUSE_INIT))
	{
		// If we're sending an init byte we can expect a reply, which will cause a timeout and flip the packet size.
		mpacketsize=7-mpacketsize;
		while(!(HW_PS2(REG_PS2_MOUSE)&(1<<BIT_PS2_CTS)))
			;
		HW_PS2(REG_PS2_MOUSE)=0xf4;
	}
	SetIntHandler(&PS2Handler);
	ClearKeyboard();
}

