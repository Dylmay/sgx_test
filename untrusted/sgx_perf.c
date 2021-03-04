#include <string.h>
#include <assert.h>
#include <time.h>

#include <unistd.h>
#include <pwd.h>
#include <libgen.h>
#include <stdlib.h>

# define MAX_PATH FILENAME_MAX

# define SGX_AESGCM_MAC_SIZE 16
# define SGX_AESGCM_IV_SIZE 12

# define DEF_COUNT 10000
# define DATA_LEN 1000
# define ENC_MSG_SIZE(count) (count + SGX_AESGCM_MAC_SIZE + SGX_AESGCM_IV_SIZE)

#include <sgx_urts.h>
#include "sgx_perf.h"

#include "Enclave_u.h"

/* Global EID shared by multiple threads */
sgx_enclave_id_t global_eid = 0;

typedef struct _sgx_errlist_t {
    sgx_status_t err;
    const char *msg;
    const char *sug; /* Suggestion */
} sgx_errlist_t;

/* Error code returned by sgx_create_enclave */
static sgx_errlist_t sgx_errlist[] = {
    {
        SGX_ERROR_UNEXPECTED,
        "Unexpected error occurred.",
        NULL
    },
    {
        SGX_ERROR_INVALID_PARAMETER,
        "Invalid parameter.",
        NULL
    },
    {
        SGX_ERROR_OUT_OF_MEMORY,
        "Out of memory.",
        NULL
    },
    {
        SGX_ERROR_ENCLAVE_LOST,
        "Power transition occurred.",
        "Please refer to the sample \"PowerTransition\" for details."
    },
    {
        SGX_ERROR_INVALID_ENCLAVE,
        "Invalid enclave image.",
        NULL
    },
    {
        SGX_ERROR_INVALID_ENCLAVE_ID,
        "Invalid enclave identification.",
        NULL
    },
    {
        SGX_ERROR_INVALID_SIGNATURE,
        "Invalid enclave signature.",
        NULL
    },
    {
        SGX_ERROR_OUT_OF_EPC,
        "Out of EPC memory.",
        NULL
    },
    {
        SGX_ERROR_NO_DEVICE,
        "Invalid Intel(R) SGX device.",
        "Please make sure Intel(R) SGX module is enabled in the BIOS, and install Intel(R) SGX driver afterwards."
    },
    {
        SGX_ERROR_MEMORY_MAP_CONFLICT,
        "Memory map conflicted.",
        NULL
    },
    {
        SGX_ERROR_INVALID_METADATA,
        "Invalid enclave metadata.",
        NULL
    },
    {
        SGX_ERROR_DEVICE_BUSY,
        "Intel(R) SGX device was busy.",
        NULL
    },
    {
        SGX_ERROR_INVALID_VERSION,
        "Enclave version was invalid.",
        NULL
    },
    {
        SGX_ERROR_INVALID_ATTRIBUTE,
        "Enclave was not authorized.",
        NULL
    },
    {
        SGX_ERROR_ENCLAVE_FILE_ACCESS,
        "Can't open enclave file.",
        NULL
    },
};

void rand_arr(char* arr, size_t len)
{
	srand(time(NULL));
	for (size_t i = 0; i < len; i++)
		arr[i] = rand() % sizeof(char);
}

/* Check error conditions for loading enclave */
void print_error_message(sgx_status_t ret)
{
    size_t idx = 0;
    size_t ttl = sizeof sgx_errlist/sizeof sgx_errlist[0];

    for (idx = 0; idx < ttl; idx++) {
        if(ret == sgx_errlist[idx].err) {
            if(NULL != sgx_errlist[idx].sug)
                printf("Info: %s\n", sgx_errlist[idx].sug);
            printf("Error: %s\n", sgx_errlist[idx].msg);
            break;
        }
    }
    
    if (idx == ttl)
        printf("Error code is 0x%X. Please refer to the \"Intel SGX SDK Developer Reference\" for more details.\n", ret);
}

double measure_ticks(clock_t start, clock_t end)
{
  return (double) (end - start);
}

struct timespec timespec_diff(struct timespec start, struct timespec end)
{
	struct timespec temp;

	if ((end.tv_nsec - start.tv_nsec) < 0) {
		temp.tv_sec = end.tv_sec - start.tv_sec -1;
		temp.tv_nsec = (end.tv_nsec - start.tv_nsec) + 1000000000;
	} else {
		temp.tv_sec = end.tv_sec - start.tv_sec;
		temp.tv_nsec = end.tv_nsec - start.tv_nsec;
	}

	return temp;
}

/* Initialize the enclave:
 *   Call sgx_create_enclave to initialize an enclave instance
 */
int initialize_enclave(void)
{
    sgx_status_t ret = SGX_ERROR_UNEXPECTED;

    /* Call sgx_create_enclave to initialize an enclave instance */
    /* Debug Support: set 2nd parameter to 1 */
    ret = sgx_create_enclave(ENCLAVE_FILENAME, SGX_DEBUG_FLAG, NULL, NULL, &global_eid, NULL);

    if (ret != SGX_SUCCESS) {
        print_error_message(ret);
        return -1;
    }

    return 0;
}

/* OCall functions */
void ocall_enclave_str(const char *str)
{
    /* Proxy/Bridge will check the length and null-terminate 
     * the input string to prevent buffer overflow. 
     */
    printf("%s", str); fflush(stdout);
}

int rw_enclave_data(struct timespec *w_rec, struct timespec *r_rec, size_t count)
{
	if (initialize_enclave() < 0) return -1;


	for (size_t i = 0; i < count; i++) {
		struct timespec start, end;
		int ecall_ret;

		//write random data
		clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &start);
		ecall_rand_write(global_eid);
		clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &end);
		w_rec[i] = timespec_diff(start, end);

		//read random data
		clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &start);
		ecall_rand_read(global_eid, &ecall_ret);
		clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &end);
		r_rec[i] = timespec_diff(start, end);
	}

	return sgx_destroy_enclave(global_eid);
}

int ecall_test(struct timespec *e_rec, size_t count)
{
	if (initialize_enclave() < 0) return -1;
	printf("#ecall: constructed enclave\n"); fflush(stdout);

	for (size_t i = 0 ; i < count; i++) {
		struct timespec start, end;

		//call empty ecall
		printf("#ecall: iter %lu; calling enclave\n", i+1); fflush(stdout);
		clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &start);
		ecall_empty(global_eid);
		clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &end);
		printf("#ecall: iter %lu; called enclave, recording\n", i+1); fflush(stdout);
		e_rec[i] = timespec_diff(start, end);

	}

	return sgx_destroy_enclave(global_eid);
}

int const_dest_enclave(struct timespec *const_rec, struct timespec *dest_rec, size_t count)
{
	for (size_t i = 0; i < count; i++) {
		struct timespec start, end;

		//construct enclave
		printf("#const_dest: iter %lu; constructing enclave\n", i+1); fflush(stdout);
		clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &start);
		if (initialize_enclave() < 0) return -1;
		clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &end);
		printf("#const_dest: iter %lu; constructed enclave, recording\n", i+1); fflush(stdout);
		const_rec[i] = timespec_diff(start, end);

		//destruct enclave
		printf("#const_dest: iter %lu; destructing enclave\n", i+1); fflush(stdout);
		clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &start);
		if (sgx_destroy_enclave(global_eid) < 0) return -1;
		clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &end);
		printf("#const_dest: iter %lu; destructed enclave, recording\n", i+1); fflush(stdout);
		dest_rec[i] = timespec_diff(start, end);
	}

	return 0;
}

int io_enclave(struct timespec *i_rec, struct timespec *o_rec, size_t count, size_t data_len)
{
	if (initialize_enclave() < 0) return -1;
	printf("#io: initialized enclave\n"); fflush(stdout);

	//io data
	char *data = (char*) calloc(data_len, sizeof(char));
	rand_arr(data, data_len);
	printf("#io: initialized test data\n"); fflush(stdout);

	for (size_t i = 0; i < count; i++) {
		struct timespec start, end;

		// record input times
		printf("#io: iter %lu; sending data\n", i+1); fflush(stdout);
		clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &start);
		ecall_data_in_malloc(global_eid, data, data_len);
		clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &end);
		printf("#io: iter %lu; sent data, recording\n", i+1); fflush(stdout);
		i_rec[i] = timespec_diff(start, end);

		// record output times
		printf("#io: iter %lu; retrieving data\n", i+1); fflush(stdout);
		clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &start);
		ecall_data_out_malloc(global_eid, data, data_len);
		clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &end);
		printf("#io: iter %lu; retrieved data, recording\n", i+1); fflush(stdout);
		o_rec[i] = timespec_diff(start, end);
	}

	free(data);
	ecall_free(global_eid);

	return sgx_destroy_enclave(global_eid);
}

void print_hex(char *data, size_t data_len)
{
	for (size_t i = 0; i < data_len; i++)
		printf("0x%x ", data[i]);
}

int enc_dec_data(struct timespec *e_rec, struct  timespec *d_rec, size_t count, size_t data_len)
{
	if (initialize_enclave() < 0) return -1;
	printf("#enc_dec: initialized enclave\n"); fflush(stdout);

	char *data = (char*) calloc(data_len, sizeof(char));
	char *enc_buffer = (char*) calloc(ENC_MSG_SIZE(data_len), sizeof(char));
	char *dec_buffer = (char*) calloc(data_len, sizeof(char));
	printf("#enc_dec: initialized test data\n"); fflush(stdout);

	rand_arr(data, data_len);
	ecall_aesgcm_init(global_eid);

	for (size_t i = 0; i < count; i++) {
		struct timespec start, end;

		//record encryption times
		printf("#enc_dec: iter %lu; encrypting data\n", i+1); fflush(stdout);
		clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &start);
		ecall_aesgcm_enc(global_eid, data, data_len, enc_buffer, ENC_MSG_SIZE(data_len));
		clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &end);
		printf("#enc_dec: iter %lu; data encrypted, recording\n", i+1); fflush(stdout);
		e_rec[i] = timespec_diff(start, end);

		//record decryption times
		printf("#enc_dec: iter %lu; decrypting data\n", i+1); fflush(stdout);
		clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &start);
		ecall_aesgcm_dec(global_eid, enc_buffer, ENC_MSG_SIZE(data_len), dec_buffer, data_len);
		clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &end);
		printf("#enc_dec: iter %lu; data decrypted, recording\n", i+1); fflush(stdout);
		d_rec[i] = timespec_diff(start, end);
	}

	free(data);
	free(enc_buffer);
	free(dec_buffer);
	return sgx_destroy_enclave(global_eid);
}


void print_timespec_recordings(struct timespec *recordings, size_t length)
{
	for (int i = 0; i < length; i++)
		printf("Recording %i, Time taken: %lus, %luus (%lu ms)\n",
				(i + 1),
				recordings[i].tv_sec,
				(recordings[i].tv_nsec / 1000),
				(recordings[i].tv_nsec / 1000000)
		);
}

void print_nanosec_recordings(struct timespec *recordings, size_t length)
{
	for (int i = 0; i < length; i++)
		printf("Recording %i; %lu (s); %lu (ns)\n", i+1 , recordings[i].tv_sec, recordings[i].tv_nsec);
}

static inline void assert_exit(int retval)
{
	if (retval != 0) {
		printf("Error: bad enclave return value 0x%x\n", retval);
		exit(-1);
	}
}

/* Application entry */
int SGX_CDECL main(int argc, char *argv[])
{
    (void)(argc);
    (void)(argv);

    /* Changing dir to where the executable is.*/
    char absolutePath [MAX_PATH]; // @suppress("Symbol is not resolved")
    char *ptr = NULL;
    size_t rec_count;
    size_t data_len;


    ptr = realpath(dirname(argv[0]),absolutePath);

    if( chdir(absolutePath) != 0)
    		abort();

    if (argc < 2) {
    	puts("At least two arguments required");
    	puts("Help:");
    	puts("\t./sgx_perf {test iterations} {data len}");
    	puts("\ttest iterations = number of times to repeat a test");
    	puts("\tdata len (optional) = if present, run the data tests with the given length, else, run the non-data length dependant tests");
    	return 0;
    }

    //check for user-supplied iteration count
    if (argc >= 2)
    	rec_count = strtol(argv[1], NULL, 10);
    else
    	rec_count = DEF_COUNT;

    //check for user-supplied data length, if none is passed only run const_dest tests
    if (argc == 3)
    	data_len = strtol(argv[2], NULL, 10);
    else
    	data_len = 0;

    if (rec_count <= 0 || data_len < 0) {
    	puts("Error: expected number larger than 0 for recording count and a non-negative data length");
    	return -1;
    }

    printf("| Count:    %lu\t|\n", rec_count);
    printf("| Data len: %lu\t|\n", data_len);

    struct timespec *rec_one = (struct timespec *) malloc(rec_count * sizeof(struct timespec));
    struct timespec *rec_two = (struct timespec *) malloc(rec_count * sizeof(struct timespec));

    if (rec_one == NULL || rec_two == NULL) {
    	puts("Error: unable to allocate the memory required for value recording");
    	return -1;
    }

    // input/output enclave test
    if (data_len) {
		puts("_______________________________________________________________");
    	printf("#main: beginning io test\n"); fflush(stdout);
		assert_exit(io_enclave(rec_one, rec_two, rec_count, data_len));
    	printf("#main: ending io test. Printing results\n"); fflush(stdout);
		// print results
		puts("\n-- Input recordings --");
		print_nanosec_recordings(rec_one, rec_count);
		puts("\n-- Output recordings --");
		print_nanosec_recordings(rec_two, rec_count);

		puts("\n_______________________________________________________________");
		// encryption/decryption enclave test
    	printf("#main: beginning enc/dec test.\n"); fflush(stdout);
		assert_exit(enc_dec_data(rec_one, rec_two, rec_count, data_len));
    	printf("#main: ending enc/dec test. Printing results\n"); fflush(stdout);
		puts("\n-- Encryption recordings --");
		print_nanosec_recordings(rec_one, rec_count);
		puts("\n-- Decryption recordings --");
		print_nanosec_recordings(rec_two, rec_count);
    } else {
		 // construct/destruct enclave test
    	printf("#main: beginning const/dest test. Printing results\n"); fflush(stdout);
		assert_exit(const_dest_enclave(rec_one, rec_two, rec_count));
    	printf("#main: ending const/dest test. Printing results\n"); fflush(stdout);
		// print results
		puts("\n-- Construct recordings --");
		print_nanosec_recordings(rec_one, rec_count);
		puts("\n-- Destruct recordings --");
		print_nanosec_recordings(rec_two, rec_count);

		puts("\n_______________________________________________________________");
		//ecall enclave test
    	printf("#main: beginning empty ecall test. Printing results\n"); fflush(stdout);
		assert_exit(ecall_test(rec_one, rec_count));
    	printf("#main: ending empty ecall test. Printing results\n"); fflush(stdout);
		// print results
		puts("\n-- Ecall recordings --");
		print_nanosec_recordings(rec_one, rec_count);
    }

    free(rec_one);
    free(rec_two);

    return 0;
}
