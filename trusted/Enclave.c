#include <stdarg.h>
#include <stdio.h>      /* vsnprintf */

#include <sgx_trts.h>

#include "Enclave.h"
#include "Enclave_t.h"  /* print_string */

#define DATA_SIZE 100-000

int ENCLAVE_DATA[DATA_SIZE];

/* 
 * printf: 
 *   Invokes OCALL to display the enclave buffer to the terminal.
 */
void printf(const char *fmt, ...)
{
    char buf[BUFSIZ] = {'\0'};
    va_list ap;
    va_start(ap, fmt);
    vsnprintf(buf, BUFSIZ, fmt, ap);
    va_end(ap);
    ocall_enclave_str(buf);
}

int rand()
{
	int val = -1;

	sgx_read_rand((unsigned char *) &val, 8);

	return val;
}

int ecall_enclave_print()
{
  printf("IN ENCLAVE\n");
  return 0;
}

int ecall_rand_read()
{
	return ENCLAVE_DATA[rand() % DATA_SIZE];
}

void ecall_rand_write()
{
	for (int i = 0; i < DATA_SIZE; i++) {
		int rn = rand() % DATA_SIZE;
		ENCLAVE_DATA[rn] = rn;
	}
}
