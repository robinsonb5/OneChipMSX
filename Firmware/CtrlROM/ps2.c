#include "small_printf.h"

#include "ps2.h"
#include "interrupts.h"
#include "keyboard.h"

void ps2_ringbuffer_init(struct ps2_ringbuffer *r)
{
	r->in_hw=0;
	r->in_cpu=0;
	r->out_hw=0;
	r->out_cpu=0;
}

void ps2_ringbuffer_write(struct ps2_ringbuffer *r,int in)
{
	while(r->out_hw==((r->out_cpu+1)&(PS2_RINGBUFFER_SIZE-1)))
		;
	DisableInterrupts();
	r->outbuf[r->out_cpu]=in;
	r->out_cpu=(r->out_cpu+1) & (PS2_RINGBUFFER_SIZE-1);
	PS2Handler();
	EnableInterrupts();
}


int ps2_ringbuffer_read(struct ps2_ringbuffer *r)
{
	unsigned char result;
	if(r->in_hw==r->in_cpu)
		return(-1);	// No characters ready
	result=r->inbuf[r->in_cpu];
	r->in_cpu=(r->in_cpu+1) & (PS2_RINGBUFFER_SIZE-1);
	return(result);
}

int ps2_ringbuffer_count(struct ps2_ringbuffer *r)
{
	if(r->in_hw>=r->in_cpu)
		return(r->in_hw-r->in_cpu);
	return(r->in_hw+PS2_RINGBUFFER_SIZE-r->in_cpu);
}

struct ps2_ringbuffer kbbuffer;
//struct ps2_ringbuffer mousebuffer;


void PS2Handler()
{
	int kbd;
	int mouse;

	DisableInterrupts();
	GetInterrupts();	// Clear interrupt bit
	
	kbd=HW_PS2(REG_PS2_KEYBOARD);

	if(kbd & (1<<BIT_PS2_RECV))
	{
		kbbuffer.inbuf[kbbuffer.in_hw]=kbd&0xff;
		kbbuffer.in_hw=(kbbuffer.in_hw+1) & (PS2_RINGBUFFER_SIZE-1);
	}
	if(kbd & (1<<BIT_PS2_CTS))
	{
		if(kbbuffer.out_hw!=kbbuffer.out_cpu)
		{
			HW_PS2(REG_PS2_KEYBOARD)=kbbuffer.outbuf[kbbuffer.out_hw];
			kbbuffer.out_hw=(kbbuffer.out_hw+1) & (PS2_RINGBUFFER_SIZE-1);
		}
	}
#if 0
	mouse=HW_PS2(REG_PS2_MOUSE);
	if(mouse & (1<<BIT_PS2_RECV))
	{
		mousebuffer.inbuf[mousebuffer.in_hw]=mouse&0xff;
		mousebuffer.in_hw=(mousebuffer.in_hw+1) & (PS2_RINGBUFFER_SIZE-1);
	}
	if(mouse & (1<<BIT_PS2_CTS))
	{
		if(mousebuffer.out_hw!=mousebuffer.out_cpu)
		{
			HW_PS2(REG_PS2_MOUSE)=mousebuffer.outbuf[mousebuffer.out_hw];
			mousebuffer.out_hw=(mousebuffer.out_hw+1) & (PS2_RINGBUFFER_SIZE-1);
		}
	}
#endif
	EnableInterrupts();
}

void PS2Init()
{
	ps2_ringbuffer_init(&kbbuffer);
//	ps2_ringbuffer_init(&mousebuffer);
	SetIntHandler(&PS2Handler);
	ClearKeyboard();
}

