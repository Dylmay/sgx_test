#include <string.h>
#include <assert.h>
#include <time.h>

#include <unistd.h>
#include <pwd.h>
#include <libgen.h>
#include <stdlib.h>

# define MAX_PATH FILENAME_MAX

# define RW_COUNT 10
# define ENC_COUNT 10

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
    printf("%s", str);
}
int rw_enclave_data(struct timespec *w_tick_rec, struct timespec *r_tick_rec, int count)
{

	if (initialize_enclave() < 0)
		return -1;

	for (int i = 0; i < count; i++) {
		struct timespec start, end;

		clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &start);

		//write random data
		ecall_rand_write(global_eid);

		//end time
		clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &end);

		w_tick_rec[i] = timespec_diff(start, end);

		//begin time
		clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &start);

		//read random data
		int ecall_ret;
		ecall_rand_read(global_eid, &ecall_ret);

		//end time
		clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &end);

		r_tick_rec[i] = timespec_diff(start, end);
	}

	return sgx_destroy_enclave(global_eid);
}

int const_dest_enclave(struct timespec *recordings, int count)
{
	for (int i = 0; i < count; i++) {
		struct timespec start, end;

		clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &start);

		if (initialize_enclave() < 0)
			return -1;

		sgx_destroy_enclave(global_eid);

		clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &end);

		recordings[i] = timespec_diff(start, end);
	}

	return 0;
}


void print_timespec_recordings(struct timespec *recordings, int length) {
	for (int i = 0; i < length; i++)
		printf("Recording %i, Time taken: %lus, %luus (%lu ms)\n",
				(i + 1),
				recordings[i].tv_sec,
				(recordings[i].tv_nsec / 1000),
				(recordings[i].tv_nsec / 1000000)
		);
}

/* Application entry */
int SGX_CDECL main(int argc, char *argv[])
{
    (void)(argc);
    (void)(argv);

    /* Changing dir to where the executable is.*/
    char absolutePath [MAX_PATH]; // @suppress("Symbol is not resolved")
    char *ptr = NULL;

    ptr = realpath(dirname(argv[0]),absolutePath);

    if( chdir(absolutePath) != 0)
    		abort();

    sgx_status_t ret = SGX_ERROR_UNEXPECTED;
    int ecall_return = 0;

    //run tests
    struct timespec w_recordings[RW_COUNT];
    struct timespec r_recordings[RW_COUNT];
    struct timespec cd_recordings[ENC_COUNT];

    ecall_return = rw_enclave_data(w_recordings, r_recordings, RW_COUNT);
    puts("-- Read/Write enclave data --");
    puts("\nWrite recordings:");
    print_timespec_recordings(w_recordings, RW_COUNT);
    puts("\nRead recordings:");
    print_timespec_recordings(r_recordings, RW_COUNT);
    puts("\n\n-- Construct/Descruct enclaves --");
    ecall_return = const_dest_enclave(cd_recordings, ENC_COUNT);
    print_timespec_recordings(cd_recordings, ENC_COUNT);

    struct timespec enc_recordings[ENC_COUNT];


    return ecall_return;
}
