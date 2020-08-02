#include "uart.h"
#include "util.h"
#include "iso_reg.h"
#include "uart_reg.h"

volatile        UARTREG         *UartReg;

void init_uart(void)
{
        unsigned int reg_val;
        unsigned int urMuxPad_val = 0x00555000;

        //1195, set ur0,ur1 pin mux, pin : 12~23
        reg_val = REG32(ISO_muxpad0) & ~(0x00fff000);
        REG32(ISO_muxpad0) = reg_val | urMuxPad_val;

#ifdef ENABLE_UART_FUNC
        // init uart0
        REG32(U0LCR) = 0x90;
#ifdef FPGA
        //REG32(U0RBR_THR_DLL) = 0x1b;  // 115200 baud (for FPGA 50MHz) 50*1000*1000/115200/16
        REG32(U0RBR_THR_DLL) = 0x12;    // 115200 baud (for FPGA 33MHz) 33*1000*1000/115200/16
#else
        REG32(U0RBR_THR_DLL) = 0xe;             // 115200 baud (for ASIC 27MHz)
#endif
        REG32(U0IER_DLH) = 0;
        REG32(U0LCR) = 0x3;                             // 8-N-1
        REG32(U0IIR_FCR) = 0xb7;
        REG32(U0IER_DLH) = 0;

#if 0
        //init uart1
        REG32(U1LCR) = 0x90;
#ifdef FPGA
        REG32(U1RBR_THR_DLL) = 0x12;    // 115200 baud (for FPGA 33MHz) 33*1000*1000/115200/16
#else
        REG32(U1RBR_THR_DLL) = 0xf;             // 115200 baud (for ASIC 27MHz)
#endif
        REG32(U1IER_DLH) = 0;
        REG32(U1LCR) = 0x3;                     // 8-N-1
        REG32(U1IIR_FCR) = 0xb7;
        REG32(U1IER_DLH) = 0;
#endif
#endif

        UartReg = (UARTREG *) UARTREG_BASE_ADDRESS;

}

void set_focus_uart(int currentUartPort)
{
        UartReg = (currentUartPort == 0) ? (UARTREG *)UART0_REG_BASE : (UARTREG *)UART1_REG_BASE;
}

void serial_write(const unsigned char  *p_param)
{
        while((UartReg->UartLsr.Value & UARTINFO_TRANSMITTER_READY_MASK) != UARTINFO_TRANSMITTER_READY_MASK);

        UartReg->UartRbrTheDll.Value = *p_param;
}

void prints(const char *ch)
{
#ifdef ENABLE_UART_FUNC

        unsigned char ch8;
        unsigned char tmpBuf[2048]={0};

        memset(tmpBuf, 0x00, 2048);


        while (*ch != '\0') {
                ch8 = *ch;
                if (*ch++ == '\n') {
                        ch8 = 0x0d;
                        serial_write((UINT8 *)&ch8);
                        ch8 = 0x0a;
                }
                serial_write((UINT8 *) &ch8);
        }
#endif
}

void print_val(UINT32 val, UINT32 len)
{
#ifdef ENABLE_UART_FUNC
        unsigned char ch;

        len--;
        do {
                ch = (val >> (len << 2)) & 0xf;
                ch += (ch < 0xa ? 0x30 : 0x37);
                serial_write(&ch);
        } while(len--);
#endif
}

void print_hex(unsigned int value)
{
#ifdef ENABLE_UART_FUNC
        print_val(value, 8);
#endif
}

#ifdef ENABLE_UART_FUNC
#define print_uart prints
#endif

void hexdump(const char *str, const void *buf, unsigned int length)
{
	unsigned int i;
	char *ptr = (char *)buf;
	
	if ((buf == NULL) || (length == 0)) {
		prints("NULL\n");
		return;
	}

	prints(str == NULL ? __FUNCTION__ : str);
	prints(" (0x");
	print_hex((UINT32)buf);
	prints(")\n");

	for (i = 0; i < length; i++) {
		print_val((unsigned int)(ptr[i]), 2);

		if ((i & 0xf) == 0xf)
			prints("\n");
		else
			prints(" ");
	}
	prints("\n");
}

void puts(const char *s)
{
        while(*s) {
                serial_write(s);
                ++s;
        }
}

void putc(const char c) {
        serial_write(&c);
}