/**
 * Struck 8300 Linux userspace library.
 * Copyright (C) 2017 ESS
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdint.h>
#include <limits.h>
#include <time.h>
#include <math.h>
#include <pthread.h>
#include <sys/mman.h>
#include <errno.h>

#include "sis8300drv.h"
#include "sis8300drv_utils.h"
#include "sis8300_reg.h"

struct arg {
	char dev[32];
	pthread_t tid;

	unsigned int func;
	unsigned int acc;
	unsigned int nreads;
	unsigned int nsamples;
	unsigned int dec_offset;
	unsigned int dec_factor;
	double scale_offset;
	double scale_factor;
	double td;
	int loop;
	unsigned int daq_intr;

	sis8300drv_usr *sisuser;

	double start;
	double end;

	uint16_t *data_raw;
	unsigned int nbytes_raw;
	volatile float *data;
	unsigned int nbytes;
};

#define ROUNDUP_HEX(val) (((unsigned int)(val) + 0xF) & ~0xF)

struct Qmn {
	unsigned int int_bits_m; /** < Number of integer bits (sign bit inclusive) */
	unsigned int frac_bits_n; /** < Number of fraction bits */
	unsigned int is_signed; /** < Number is signed or unsigned */
};

struct Qmn mn_magnitude = { .int_bits_m = 1, .frac_bits_n = 15, .is_signed = 0 };
struct Qmn mn_phase =     { .int_bits_m = 3, .frac_bits_n = 13, .is_signed = 1 };

void mn_2_double(uint32_t val, struct Qmn Qmn, double *converted) {

	double pow_2_frac_bits = (double) (0x1UL << Qmn.frac_bits_n);
	double pow_2_frac_bits_int_bits = (double) (0x1UL << (Qmn.int_bits_m + Qmn.frac_bits_n));

	*converted = (double) val;

	/* check if val is signed and if it is negative */
	if (Qmn.is_signed && (*converted > (pow_2_frac_bits_int_bits / 2.0 - 1.0))) {
		*converted -= pow_2_frac_bits_int_bits;
	}

	/* convert to double */
	*converted /= pow_2_frac_bits;
}

double time_msec() {
	struct timespec ts;

	clock_gettime(CLOCK_MONOTONIC, &ts);
	return (double) ts.tv_sec * 1.0e3 + ts.tv_nsec / 1.0e6;
}

int pattern(char *data, unsigned int nbytes, int verify) {
	int i;

	for (i = 0; i < nbytes; ++i) {
		if (verify) {
			if (data[i] != i % CHAR_MAX) {
				printf("Inconsistency at i %d value %u should be %u\n", i,
						(unsigned) data[i], (unsigned) (i % CHAR_MAX));
				return -1;
			}
		} else {
			data[i] = i % CHAR_MAX;
		}
	}

	return 0;
}

void print_stats_hdr(void) {
	printf("%15s, %15s, %15s, %15s, %15s, %15s, %15s\n",
			"func", "dev", "size [MB]", "start [ms]", "end [ms]", "dur [ms]", "speed [MB/s]");
}

void print_stats(struct arg *a, const char *func) {
	char s1[16], s2[16], s3[16], s4[16], s5[16];

	sprintf(s1, "%.1f", a->nbytes_raw / 1e6);
	sprintf(s2, "%.1f", a->start - a->td);
	sprintf(s3, "%.1f", a->end - a->td);
	sprintf(s4, "%.1f", (a->end - a->start) / a->nreads);
	sprintf(s5, "%.1f", (double)a->nbytes_raw / (a->end - a->start) * a->nreads / 1.0e3);

	printf("%15s, %15s, %15s, %15s, %15s, %15s, %15s\n",
			func, a->dev, s1, s2, s3, s4, s5);
}

int func_readout(struct arg *a) {
	int ret, i;

	a->start = time_msec();
	for (i = 0; i < a->nreads; ++i) {
		ret = sis8300drv_read_ram(a->sisuser, 0, a->nbytes_raw, a->data_raw);
		if (ret) {
			printf("sis8300drv_read_ram error: %d\n", ret);
			return -1;
		}
	}
	a->end = time_msec();

	print_stats(a, __func__);
	return 0;
}

int func_dec(struct arg *a) {
	int i, j;
	unsigned nsamples_dec;

	nsamples_dec = a->nsamples - a->dec_offset;
	nsamples_dec /= a->dec_factor;

	a->start = time_msec();
	for (i = 0; i < a->nreads; ++i) {
		for (j = 0; j < nsamples_dec; j++) {
			a->data[j] = a->data_raw[j * a->dec_factor + a->dec_offset];
		}
	}
	a->end = time_msec();

	print_stats(a, __func__);
	return 0;
}

int func_sca(struct arg *a) {
	int i, j;

	a->start = time_msec();
	for (i = 0; i < a->nreads; ++i) {
		for (j = 0; j < a->nsamples; j++) {
			a->data[j] = a->data_raw[j] * a->scale_factor + a->scale_offset;
		}
	}
	a->end = time_msec();

	print_stats(a, __func__);
	return 0;
}

int func_no_decsca(struct arg *a) {
	int i, j;

	a->start = time_msec();
	for (i = 0; i < a->nreads; ++i) {
		for (j = 0; j < a->nsamples; j++) {
			a->data[j] = a->data_raw[j];
		}
	}
	a->end = time_msec();

	print_stats(a, __func__);
	return 0;
}

int func_decsca(struct arg *a) {
	int i, j;
	unsigned nsamples_dec;

	nsamples_dec = a->nsamples - a->dec_offset;
	nsamples_dec /= a->dec_factor;

	a->start = time_msec();
	for (i = 0; i < a->nreads; ++i) {
		for (j = 0; j < nsamples_dec; j++) {
			a->data[j] = a->data_raw[j * a->dec_factor + a->dec_offset] * a->scale_factor + a->scale_offset;
		}
	}
	a->end = time_msec();

	print_stats(a, __func__);
	return 0;
}

int func_smn2double(struct arg *a) {
	int i, j;

	a->start = time_msec();
	for (i = 0; i < a->nreads; ++i) {
		for (j = 0; j < a->nsamples; j++) {
			mn_2_double(a->data_raw[j], mn_phase, (double *)&a->data[j]);
			a->data[j] *= 180.0 / M_PI;
		}
	}
	a->end = time_msec();

	print_stats(a, __func__);
	return 0;
}

int func_umn2double(struct arg *a) {
	int i, j;

	a->start = time_msec();
	for (i = 0; i < a->nreads; ++i) {
		for (j = 0; j < a->nsamples; j++) {
			mn_2_double(a->data_raw[j], mn_magnitude, (double *)&a->data[j]);
		}
	}
	a->end = time_msec();

	print_stats(a, __func__);
	return 0;
}

int func_writepatt(struct arg *a) {
	int ret;

	pattern((char *)a->data_raw, a->nbytes_raw, 0);

	a->start = time_msec();
	ret = sis8300drv_write_ram(a->sisuser, 0, a->nbytes_raw, a->data_raw);
	if (ret) {
		printf("sis8300drv_write_ram error: %d\n", ret);
		return -1;
	}
	a->end = time_msec();

	print_stats(a, __func__);
	return 0;
}

void *run_func(void *ptr) {
	struct arg *a = (struct arg *) ptr;
	int ret;
	sis8300drv_dev *sisdevice;
    sis8300drv_trg_src trgsrc = trg_src_external;
    sis8300drv_trg_ext trgext = trg_ext_mlvds;
    unsigned int trgline = 4;
    unsigned int trgmask = (1 << (trgline - SIS8300DRV_NUM_FP_TRG));

	a->sisuser = malloc(sizeof(sis8300drv_usr));
	a->sisuser->file = a->dev;

	ret = sis8300drv_open_device(a->sisuser);
	if (ret) {
		printf("sis8300drv_open_device error: %d\n", ret);
		return NULL;
	}
	sisdevice = (sis8300drv_dev *)a->sisuser->device;

	/* enable all 10 channels */
	ret = sis8300drv_set_channel_mask(a->sisuser, 0x3FF);
	if (ret) {
		printf("sis8300drv_set_channel_mask error: %s (%d)\n",
				sis8300drv_strerror(ret), ret);
		return NULL;
	}

	/* internal clock */
	ret = sis8300drv_set_clock_source(a->sisuser, 0);
	if (ret) {
		printf("sis8300drv_set_clock_source error: %s (%d)\n",
				sis8300drv_strerror(ret), ret);
		return NULL;
	}

	/* divide by 3 ~ 83.3 MHz */
	ret = sis8300drv_set_clock_divider(a->sisuser, 3);
	if (ret) {
		printf("sis8300drv_set_clock_divider error: %s (%d)\n",
				sis8300drv_strerror(ret), ret);
		return NULL;
	}

	/* external trigger */
	ret = sis8300drv_set_trigger_source(a->sisuser, trgsrc);
	if (ret) {
		printf("sis8300drv_set_trigger_source error: %s (%d)\n",
				sis8300drv_strerror(ret), ret);
		return NULL;
	}

	/* backplane 0 */
	ret = sis8300drv_set_external_setup(a->sisuser, trgext, trgmask, 0);
	if (ret) {
		printf("sis8300drv_set_external_setup error: %s (%d)\n",
				sis8300drv_strerror(ret), ret);
		return NULL;
	}

	switch (a->acc) {
	case 0:
		a->data_raw = (uint16_t *) malloc(a->nbytes_raw);
	    if (a->data_raw == NULL) {
	    	printf("failed data_raw malloc(): %d - %s\n", errno, strerror(errno));
	        return NULL;
	    }
		break;
	case 1:
	    a->data_raw = (uint16_t *)mmap(NULL, a->nbytes_raw, PROT_READ, MAP_SHARED, sisdevice->handle, 0);
	    if (a->data_raw == MAP_FAILED) {
	    	printf("failed mmap(): %d - %s\n", errno, strerror(errno));
	        return NULL;
	    }
	    break;
	default:
		printf("Invalid access mode %d\n", a->func);
		return NULL;
		break;
	}

	if (a->func > 0 && a->func < 100) {
		a->data = (volatile float *) malloc(a->nbytes);
	    if (a->data == NULL) {
	    	printf("failed data malloc(): %d - %s\n", errno, strerror(errno));
	        return NULL;
	    }
	}

	do {
		if (a->loop > 0) {
		    sis8300drv_arm_device(a->sisuser);

		    if (a->daq_intr) {
		    	/* This does not poll register 0x10 - no CPU load!! */
		    	ret = sis8300drv_wait_acq_end(a->sisuser, 1000);
			    if (ret) {
			        printf("sis8300drv_wait_acq_end error: %d\n", ret);
			        return NULL;
			    }
		    } else {
		    	/* This polls register 0x10 for ~70 ms - lots of CPU load!! */
		    	ret = sis8300drv_poll_acq_end(a->sisuser);
			    if (ret) {
			        printf("sis8300drv_poll_acq_end error: %d\n", ret);
			        return NULL;
			    }
		    }
		}

		switch (a->func) {
		case 0:
			func_readout(a);
			break;
		case 1:
			func_readout(a);
			func_decsca(a);
			break;
		case 2:
			func_readout(a);
			func_dec(a);
			break;
		case 3:
			func_readout(a);
			func_sca(a);
			break;
		case 4:
			func_readout(a);
			func_no_decsca(a);
			break;
		case 5:
			func_readout(a);
			func_umn2double(a);
			break;
		case 6:
			func_readout(a);
			func_smn2double(a);
			break;

		case 99:
			func_readout(a);
			func_decsca(a);
			func_dec(a);
			func_sca(a);
			func_no_decsca(a);
			func_smn2double(a);
			func_umn2double(a);
			break;

		case 100:
			func_writepatt(a);
			break;

		default:
			printf("Invalid function %d\n", a->func);
			a->loop = 0;
			break;
		}
		a->loop--;
	} while (a->loop > 0);

	switch (a->acc) {
	case 0:
		free((void *) a->data_raw);
		a->data_raw = NULL;
		break;
	case 1:
	    munmap((void *)a->data_raw, a->nbytes_raw);
		a->data_raw = NULL;
	    break;
	default:
		printf("Invalid access mode %d\n", a->func);
		return NULL;
		break;
	}

	if (a->data) {
		free((void *) a->data);
		a->data = NULL;
	}

	sis8300drv_close_device(a->sisuser);
	free(a->sisuser);
	a->sisuser = NULL;

	return NULL;
}

int main(int argc, char **argv) {
	struct arg a[10];
	int ret, i, o;
	char c;
	unsigned int nreads, nsamples, dec_offset, dec_factor, nbytes, nbytes_raw;
	double scale_offset, scale_factor;
	unsigned int func, acc, daq_intr;
	int loop;
	long double aa[4], bb[4], loadavg;
	FILE *fp;

	memset(a, 0, sizeof(a));
	func = 0;
	acc = 0;
	nreads = 1;
	dec_offset = 0;
	dec_factor = 1;
	scale_offset = 0;
	scale_factor = 1;
	loop = 0;
	daq_intr = 0;
	nsamples = 0x100000;

	while ((c = getopt(argc, argv, "hA:B:S:N:o:f:O:F:l:i")) != -1) {
		switch (c) {
		case 'A':
			sscanf(optarg, "%u", &func);
			break;
		case 'B':
			sscanf(optarg, "%u", &acc);
			break;
		case 'S':
			sscanf(optarg, "%u", &nsamples);
			break;
		case 'N':
			sscanf(optarg, "%u", &nreads);
			break;
		case 'o':
			sscanf(optarg, "%u", &dec_offset);
			break;
		case 'f':
			sscanf(optarg, "%u", &dec_factor);
			break;
		case 'O':
			sscanf(optarg, "%lf", &scale_offset);
			break;
		case 'F':
			sscanf(optarg, "%lf", &scale_factor);
			break;
		case 'l':
			sscanf(optarg, "%u", &loop);
			break;
		case 'i':
			daq_intr = 1;
			break;
		case ':':
			printf("Option -%c requires an operand.\n", optopt);
			break;
		case '?':
		case 'h':
		default:
			printf("Usage: %s device(s) [args..]\n\n"
			"DEVICE(S)\n"
			" one or many /dev/sis8300-x\n\n"
			"ARGUMENTS\n"
			" -h                   Print this message\n"
			" -S unsigned int      Number of samples to read (default: 0x100000 ~ 1M)\n"
			" -N unsigned int      Number of reads to perform (default: 1)\n"
			" -o unsigned int      Decimation offset (default: 0)\n"
			" -f unsigned int      Decimation factor (default: 1)\n"
			" -O double            Scale offset (default: 0)\n"
			" -F double            Scale factor (default: 1)\n"
			" -l unsigned int      Loop on external trigger (default: 0)\n"
			" -i                   Use DAQ interrupt (default: 0)\n"
			" -B unsigned int      Access mode read() or mmap() (default: 0)\n"
			" -A unsigned int      Which function to perform (default: 0)\n\n"
			"FUNCTIONS\n"
			" 0                    Readout only\n"
			" 1                    Readout, decimation, scaling\n"
			" 2                    Readout, decimation\n"
			" 3                    Readout, scaling\n"
			" 4                    Readout, no decimation, no scaling\n"
			" 5                    Readout, 2'complement unsigned to double\n"
			" 6                    Readout, 2'complement signed to double\n"
			" 99                   All of the above\n"
			" 100                  Write pattern to memory\n"
			"\n", argv[0]);
			return -1;
		}
	}

	nsamples = ROUNDUP_HEX(nsamples);
	nbytes = sizeof(float) * nsamples;
	nbytes_raw = sizeof(uint16_t) * nsamples;

	printf("Parameters:\n"
			" function        %d\n"
			" access          %d\n"
			" nsamples        %d\n"
			" nbytes_raw      %d b (%.1lf MB)\n"
			" nbytes          %d b (%.1lf MB)\n"
			" nreads          %d\n"
			" dec_offset      %d\n"
			" dec_factor      %d\n"
			" scale_offset    %lf\n"
			" scale_factor    %lf\n"
			" loop            %d\n"
			" daq_intr        %d\n"
			" ndevices        %d\n"
			"\n",
			func, acc,
			nsamples,
			nbytes_raw, (double)nbytes_raw / 1e6,
			nbytes, (double)nbytes / 1e6,
			nreads,
			dec_offset, dec_factor,
			scale_offset, scale_factor,
			loop, daq_intr,
			argc - optind);

	fp = fopen("/proc/stat", "r");
	i = fscanf(fp, "%*s %Lf %Lf %Lf %Lf", &aa[0], &aa[1], &aa[2], &aa[3]);
	fclose(fp);

	print_stats_hdr();

	double td = time_msec();
	for (i = optind; i < argc; i++) {
		o = i - optind;
		sprintf(a[o].dev, "%s", argv[i]);
		a[o].func = func;
		a[o].acc = acc;
		a[o].loop = loop;
		a[o].daq_intr = daq_intr;
		a[o].nsamples = nsamples;
		a[o].nreads = nreads;
		a[o].nbytes_raw = nbytes_raw;
		a[o].nbytes = nbytes;
		a[o].dec_offset = dec_offset;
		a[o].dec_factor = dec_factor;
		a[o].scale_offset = scale_offset;
		a[o].scale_factor = scale_factor;
		a[o].td = td;

		ret = pthread_create(&a[o].tid, NULL, run_func, (void *) &a[o]);
		if (ret) {
			printf("failed pthread_create(): %d - %s\n", errno, strerror(errno));
			exit(EXIT_FAILURE);
		}
	}

	for (i = optind; i < argc; i++) {
		o = i - optind;
		pthread_join(a[o].tid, NULL);
	}

	fp = fopen("/proc/stat", "r");
	i = fscanf(fp, "%*s %Lf %Lf %Lf %Lf", &bb[0], &bb[1], &bb[2], &bb[3]);
	fclose(fp);
	loadavg = ((bb[0]+bb[1]+bb[2]) - (aa[0]+aa[1]+aa[2]))
			/ ((bb[0]+bb[1]+bb[2]+bb[3]) - (aa[0]+aa[1]+aa[2]+aa[3]));
	printf("CPU utilization was : %Lf (%.1Lf %%)\n", loadavg, loadavg*4*100.0);

	printf("\nall threads ended..\n");
	exit(EXIT_SUCCESS);
}
