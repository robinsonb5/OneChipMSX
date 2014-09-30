/* Cut-down, read-only PS/2 handler for OSD code. */

#include "small_printf.h"

#include "ps2.h"
#include "interrupts.h"
#include "keyboard.h"

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

void ps2_ringbuffer_write(struct ps2_ringbuffer_out *r,int in)
{
	while(r->out_hw==((r->out_cpu+1)&(PS2_RINGBUFFER_SIZE-1)))
		;
//	printf("w: %d, %d\n, %d\n",r->out_hw,r->out_cpu,in);
	DisableInterrupts();
	r->outbuf[r->out_cpu]=in;
	r->out_cpu=(r->out_cpu+1) & (PS2_RINGBUFFER_SIZE-1);
//	PS2Handler();
	EnableInterrupts();
}

struct ps2_ringbuffer kbbuffer;
struct ps2_ringbuffer_out mouseoutbuffer;

static volatile int intflag;

#define PS2_TIMEOUT 5

int ps2_mousex;
int ps2_mousey;
int ps2_mousebuttons;
static int mdata[4];
static int midx=0,mctr=0;
static int mpacketsize=3; // 4 bytes for a wheel mouse.  (Chameleon firmware initializes the mouse in wheel mode.)

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
			ps2_mousebuttons=(mdata[0])&3;
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
	else
	{
		if(!(mctr--))	// Timeout
		{
			if(midx)
				mpacketsize=7-mpacketsize; // Alternate between 3 and 4 byte packets
			midx=0;
		}
	}

	if(mouse & (1<<BIT_PS2_CTS))
	{
		if(mouseoutbuffer.out_hw!=mouseoutbuffer.out_cpu)
		{
			HW_PS2(REG_PS2_MOUSE)=mouseoutbuffer.outbuf[mouseoutbuffer.out_hw];
			mouseoutbuffer.out_hw=(mouseoutbuffer.out_hw+1) & (PS2_RINGBUFFER_SIZE-1);
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
	SetIntHandler(&PS2Handler);
	ClearKeyboard();
	PS2MouseWrite(0xf4);
}

