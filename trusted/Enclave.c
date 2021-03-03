#include <stdarg.h>
#include <stdio.h>      /* vsnprintf */
#include <string.h>

#include <sgx_trts.h>
#include <sgx_tcrypto.h>

#include "Enclave.h"
#include "Enclave_t.h"  /* print_string */

#define DATA_SIZE 1000
#define MAX_DATA_MALLOC 10000
#define BUFFER_LEN 2048

#define IV_PTR(buffer) (buffer + SGX_AESGCM_MAC_SIZE)
#define MSG_PTR(buffer) (buffer + SGX_AESGCM_MAC_SIZE + SGX_AESGCM_IV_SIZE)
#define MAC_PTR(buffer) ((sgx_aes_gcm_128bit_tag_t *) (buffer))

char ENCLAVE_DATA[DATA_SIZE];

void *DATA_PTR = 0x0;

sgx_aes_gcm_128bit_key_t AES_KEY;

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

static inline int rand()
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

void ecall_data_in(char* data, size_t len)
{
	if (len < DATA_SIZE)
		memcpy(ENCLAVE_DATA, data, len);
}

void ecall_data_out(char *data, size_t len)
{
	if (len < DATA_SIZE)
		memcpy(data, ENCLAVE_DATA, len);
}


void ecall_data_in_malloc(char *data, size_t len)
{
	if (len < MAX_DATA_MALLOC) {
		if (&DATA_PTR != NULL)
			free(DATA_PTR);

		DATA_PTR = calloc(len, sizeof(char));
		memcpy(DATA_PTR, data, len);
	}
}

void ecall_data_out_malloc(char *data, size_t len)
{
	if (len < MAX_DATA_MALLOC)
		memcpy(data, DATA_PTR, len);
}

void ecall_free()
{
	free(DATA_PTR);
	DATA_PTR = NULL;
}


void ecall_aesgcm_init()
{
	sgx_read_rand(AES_KEY, SGX_AESGCM_KEY_SIZE);
}

void print_hex(char *data, size_t data_len)
{
	for (size_t i = 0; i < data_len; i++)
		printf("0x%x ", data[i]);
}

void ecall_aesgcm_enc(char *text_in, size_t len_in, char *enc_out, size_t len_out)
{
	uint8_t *buffer = calloc(len_out, sizeof(uint8_t));
	//sets iv
	sgx_read_rand(IV_PTR(buffer), SGX_AESGCM_IV_SIZE);

	sgx_status_t res;
	res = sgx_rijndael128GCM_encrypt(&AES_KEY,
			text_in, len_in, //data
			MSG_PTR(buffer), //message
			IV_PTR(buffer), SGX_AESGCM_IV_SIZE, //iv
			NULL, 0, //aad
			MAC_PTR(buffer) //MAC
	);

	if (res != 0)
		printf("Error encrypting: 0x%x", res);

	memcpy(enc_out, buffer, len_out);
	free(buffer);
}

void ecall_aesgcm_dec(char *enc_in, size_t len_in, char *text_out, size_t len_out)
{
	uint8_t *buffer = calloc(len_out, sizeof(uint8_t));

	sgx_status_t res = {0};
	res = sgx_rijndael128GCM_decrypt(&AES_KEY, //key
			MSG_PTR(enc_in), len_out, //data
			buffer, //decrypted message
			IV_PTR(enc_in), SGX_AESGCM_IV_SIZE, //iv
			NULL, 0, //aad
			MAC_PTR(enc_in) //MAC
	);

	if (res != 0)
		printf("Error decrypting: 0x%x", res);

	memcpy(text_out, buffer, len_out);
	free(buffer);
}

