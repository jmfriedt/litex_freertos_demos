#include <stddef.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>

#include <libbase/console.h>

#include "litex_compat.h"

#define UART_HW_HANDLE          ( LITEX_UART0 )
#define UART_BAUD_RATE (19200)         // transmission speed
#define PROMPT_TASK_PRIORITY    ( tskIDLE_PRIORITY + 1 )
#define PROMPT_TASK_STACK_WORDS ( configMINIMAL_STACK_SIZE + 128 )
#define INPUT_BUFFER_SIZE       ( 48 )

void ToggleLED(void);
static char *readstr(void);
static char *get_token(char **str);
static void HandleCommand(char *str);
static void HandleMainCommand(char *str);
static int ParseUnsigned(const char *text, uint32_t *value);

int main( void ) {
  char *command;
  litex_gpio_port_set(0);                              // clear GPIO.out port
  litex_uart_setup(UART_HW_HANDLE, UART_BAUD_RATE, 0); // default baud rate, no interrupts
  litex_uart_printf(UART_HW_HANDLE, "\n<<< LiteX running >>>\n\n");
  litex_uart_puts(UART_HW_HANDLE, "Commands: help, led, pwm en 0|1, pwm p <value>, pwm d <value>, spi cs 0|1, spi w <value>\n\n");
  while (1)
  { 
    litex_uart_puts(UART_HW_HANDLE, "prompt> ");
    command = readstr();    // blocking
    HandleCommand(command);
  }
  return -1;
}

void ToggleLED(void) {
  litex_gpio_pin_toggle(0);
}

static void HandleMainCommand(char *str)
{
	char *subcommand = get_token(&str);
	char *argument = get_token(&str);
	uint32_t value;

#ifndef CSR_MAIN_BASE
	(void)subcommand;
	(void)argument;
	litex_uart_puts(UART_HW_HANDLE, "Main is not present in this SoC build.\n");
#else
	if (strcmp(subcommand, "en") == 0) {
		if (!ParseUnsigned(argument, &value) || (value > 1u)) {
			litex_uart_puts(UART_HW_HANDLE, "Usage: main en 0|1\n");
			return;
		}

// 	pwm_enable_write(value);
		litex_uart_printf(UART_HW_HANDLE, "Main enable = %lu\n", value);
	} else if (strcmp(subcommand, "p") == 0) {
		if (!ParseUnsigned(argument, &value)) {
			litex_uart_puts(UART_HW_HANDLE, "Usage: pwm p <value>\n");
			return;
		}

//		pwm_period_write(value);
		litex_uart_printf(UART_HW_HANDLE, "PWM period = %lu\n", value);
	} else if (strcmp(subcommand, "d") == 0) {
		if (!ParseUnsigned(argument, &value)) {
			litex_uart_puts(UART_HW_HANDLE, "Usage: pwm d <value>\n");
			return;
		}

//		pwm_width_write(value);
		litex_uart_printf(UART_HW_HANDLE, "PWM duty/width = %lu\n", value);
	} else {
		litex_uart_puts(UART_HW_HANDLE,
				"Usage: pwm en 0|1, pwm p <value>, pwm d <value>\n");
	}
#endif
}

static void HandleCommand(char *str)
{
	char *command = get_token(&str);

	if (strcmp(command, "help") == 0) {
		litex_uart_puts(UART_HW_HANDLE,
				"help          - show commands\n"
				"led           - toggle LED 0\n"
				"main en 0|1    - disable or enable PWM\n"
				"main p <value> - set PWM period\n"
				"main d <value> - set PWM duty/width\n"
			       );
	} else if (strcmp(command, "led") == 0) {
		ToggleLED();
		litex_uart_puts(UART_HW_HANDLE, "LED toggled.\n");
	} else if (strcmp(command, "main") == 0) {
		HandleMainCommand(str);
	} else if (command[0] == '\0') {
		litex_uart_puts(UART_HW_HANDLE, "Type 'help' for commands.\n");
	} else {
		litex_uart_puts(UART_HW_HANDLE, "Unknown command. Type 'help'.\n");
	}
}

static char *readstr(void)
{
	char c[2];
	static char s[INPUT_BUFFER_SIZE];
	static int ptr = 0;

	while(1) {
		c[0] = getchar();
		c[1] = 0;
		switch (c[0]) {
		case 0x7f:
		case 0x08:
			if (ptr > 0) {
				ptr--;
				fputs("\x08 \x08", stdout);
			}
			break;
		case 0x07:
			break;
		case '\r':
		case '\n':
			s[ptr] = 0x00;
			fputs("\n", stdout);
			ptr = 0;
			return s;
		default:
			if (ptr >= ((int)sizeof(s) - 1))
				break;
			fputs(c, stdout);
			s[ptr] = c[0];
			ptr++;
			break;
		}
	}
	return NULL;
}

static char *get_token(char **str)
{
	char *c, *d;

	c = (char *)strchr(*str, ' ');
	if (c == NULL) {
		d = *str;
		*str = *str + strlen(*str);
		return d;
	}
	*c = 0;
	d = *str;
	*str = c + 1;
	return d;
}

static int ParseUnsigned(const char *text, uint32_t *value)
{
	char *end;
	unsigned long parsed;

	if ((text == NULL) || (*text == '\0') || (*text == '-'))
		return 0;

	parsed = strtoul(text, &end, 0);
	if ((end == text) || (*end != '\0'))
		return 0;

	*value = (uint32_t)parsed;
	return 1;
}
