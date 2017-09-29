
Created on: Aug 8, 2017

Author: hinko.kocevar@esss.se

     
## What

Some notes on performance measurements made with SIS8300L/L2/KU boards.

## Goals

* Measure time and speed of data transfer over PCIe
* Measure time and speed data conversion on the CPU
* Measure CPU utilization

## Setup

* 9U crate
* Intel(R) Core(TM) i7-3555LE CPU @ 2.50GHz
* 8GB memory
* up to 6x Struck SIS8300 (L, L2 and KU)

## Tools

Used CLI tool in _vendor/ess/tools/threads_ that spawns required amount of threads.
Each thread deals with individual AMC.

	$ ./bin/linux-x86_64/sis8300drv_threads -h
	Usage: ./bin/linux-x86_64/sis8300drv_threads device(s) [args..]
	
	DEVICE(S)
	 one or many /dev/sis8300-x
	
	ARGUMENTS
	 -h                   Print this message
	 -S unsigned int      Number of samples to read (default: 0x100000 ~ 1M)
	 -N unsigned int      Number of reads to perform (default: 1)
	 -o unsigned int      Decimation offset (default: 0)
	 -f unsigned int      Decimation factor (default: 1)
	 -O double            Scale offset (default: 0)
	 -F double            Scale factor (default: 1)
	 -B unsigned int      Access mode read() or mmap() (default: 0)
	 -A unsigned int      Which function to perform (default: 0)
	
	FUNCTIONS
	 0                    Readout only
	 1                    Readout, decimation, scaling
	 2                    Readout, decimation
	 3                    Readout, scaling
	 4                    Readout, no decimation, no scaling
	 5                    Readout, 2'complement unsigned to double
	 6                    Readout, 2'complement signed to double
	 7                    Readout, 2'complement unsigned to float
	 8                    Readout, 2'complement signed to float
	 99                   All of the above
	 100                  Write pattern to memory

## Tests

### PCIe transfer speed

Findings:
* SIS8300L/L2 transfer speed is around 600 MB/s
* SIS8300KU transfer speed is around 1200 MB/s
* Data transfer from individual AMC are made in parallel (DMA)
* By increasing number of AMCs transfer speeds are lowering (per AMC)
* Note the effect of SIS8300KU on SIS8300L/L2

Parameters:
* Average over 30 readouts
* Readout 30000000 samples (60 Mb)
* Sample is 16-bit
* Data is only stored in local buffer
* No data conversion or manilpulation

Example command:

	$ ./bin/linux-x86_64/sis8300drv_threads <devs> -S $((100*300000)) -N 30
	Parameters:
	 function        0
	 access          0
	 nsamples        30000000
	 nbytes_raw      60000000 b (60.0 MB)
	 nbytes          120000000 b (120.0 MB)
	 nreads          30
	 dec_offset      0
	 dec_factor      1
	 scale_offset    0.000000
	 scale_factor    1.000000
	 ndevices        6

#### 1x AMC SIS8300L/L2

	           func,             dev,       size [MB],      start [ms],        end [ms],        dur [ms],    speed [MB/s]
	   func_readout,  /dev/sis8300-6,            60.0,             0.3,          2895.6,            96.5,           621.7

#### 1x AMC SIS8300KU

	           func,             dev,       size [MB],      start [ms],        end [ms],        dur [ms],    speed [MB/s]
	   func_readout,  /dev/sis8300-3,            60.0,             0.3,          1441.3,            48.0,          1249.2

#### 5x AMC SIS8300L/L2

	           func,             dev,       size [MB],      start [ms],        end [ms],        dur [ms],    speed [MB/s]
	   func_readout,  /dev/sis8300-6,            60.0,             0.7,          3086.8,           102.9,           583.3
	   func_readout,  /dev/sis8300-8,            60.0,             0.8,          3101.8,           103.4,           580.5
	   func_readout,  /dev/sis8300-7,            60.0,             0.5,          3101.8,           103.4,           580.4
	   func_readout,  /dev/sis8300-9,            60.0,             0.8,          3114.0,           103.8,           578.2
	   func_readout,  /dev/sis8300-4,            60.0,             0.9,          3128.7,           104.3,           575.5

#### 6x AMC SIS8300L/L2/KU

	           func,             dev,       size [MB],      start [ms],        end [ms],        dur [ms],    speed [MB/s]
	   func_readout,  /dev/sis8300-3,            60.0,             1.4,          1756.9,            58.5,          1025.3
	   func_readout,  /dev/sis8300-9,            60.0,             1.4,          3507.7,           116.9,           513.4
	   func_readout,  /dev/sis8300-6,            60.0,             1.4,          3511.7,           117.0,           512.8
	   func_readout,  /dev/sis8300-7,            60.0,             1.1,          3521.7,           117.4,           511.3
	   func_readout,  /dev/sis8300-4,            60.0,             1.4,          3534.4,           117.8,           509.5
	   func_readout,  /dev/sis8300-8,            60.0,             1.5,          3545.5,           118.1,           507.9


### PCIe transfer time vs. data size

Findings:
* Linearly scales with the amount of bytes to transfer
* To simulate floating point transfers multiply results below by 2 (float is 4 bytes)
* SIS8300KU data transfer would be ~2 times faster

Parameters:
* Average over 30 readouts
* Sample is 16-bit
* Data is only stored in local buffer
* No data conversion or manilpulation

#### 30000 samples / channel, 10 channels

	$ ./bin/linux-x86_64/sis8300drv_threads /dev/sis8300-6 /dev/sis8300-7 /dev/sis8300-4 /dev/sis8300-8 /dev/sis8300-9 -S $((10*30000)) -N 30
	Parameters:
	 function        0
	 access          0
	 nsamples        300000
	 nbytes_raw      600000 b (0.6 MB)
	 nbytes          1200000 b (1.2 MB)
	 nreads          30
	 dec_offset      0
	 dec_factor      1
	 scale_offset    0.000000
	 scale_factor    1.000000
	 ndevices        5
	
	           func,             dev,       size [MB],      start [ms],        end [ms],        dur [ms],    speed [MB/s]
	   func_readout,  /dev/sis8300-7,             0.6,             0.5,            38.0,             1.2,           480.5
	   func_readout,  /dev/sis8300-4,             0.6,             0.6,            39.1,             1.3,           467.8
	   func_readout,  /dev/sis8300-8,             0.6,             0.7,            39.1,             1.3,           468.8
	   func_readout,  /dev/sis8300-9,             0.6,             0.6,            39.7,             1.3,           459.9
	   func_readout,  /dev/sis8300-6,             0.6,             0.6,            39.8,             1.3,           459.9

#### 30000 samples / channel, 28 channels

	$ ./bin/linux-x86_64/sis8300drv_threads /dev/sis8300-6 /dev/sis8300-7 /dev/sis8300-4 /dev/sis8300-8 /dev/sis8300-9 -S $((28*30000)) -N 30
	Parameters:
	 function        0
	 access          0
	 nsamples        840000
	 nbytes_raw      1680000 b (1.7 MB)
	 nbytes          3360000 b (3.4 MB)
	 nreads          30
	 dec_offset      0
	 dec_factor      1
	 scale_offset    0.000000
	 scale_factor    1.000000
	 ndevices        5
	
	           func,             dev,       size [MB],      start [ms],        end [ms],        dur [ms],    speed [MB/s]
	   func_readout,  /dev/sis8300-6,             1.7,             1.1,           105.6,             3.5,           482.4
	   func_readout,  /dev/sis8300-8,             1.7,             1.2,           105.8,             3.5,           481.7
	   func_readout,  /dev/sis8300-9,             1.7,             1.3,           105.8,             3.5,           482.0
	   func_readout,  /dev/sis8300-4,             1.7,             1.2,           105.9,             3.5,           481.0
	   func_readout,  /dev/sis8300-7,             1.7,             1.2,           106.1,             3.5,           480.6

#### 300000 samples / channel, 10 channels

	$ ./bin/linux-x86_64/sis8300drv_threads /dev/sis8300-6 /dev/sis8300-7 /dev/sis8300-4 /dev/sis8300-8 /dev/sis8300-9 -S $((100*30000)) -N 30
	Parameters:
	 function        0
	 access          0
	 nsamples        3000000
	 nbytes_raw      6000000 b (6.0 MB)
	 nbytes          12000000 b (12.0 MB)
	 nreads          30
	 dec_offset      0
	 dec_factor      1
	 scale_offset    0.000000
	 scale_factor    1.000000
	 ndevices        5
	
	           func,             dev,       size [MB],      start [ms],        end [ms],        dur [ms],    speed [MB/s]
	   func_readout,  /dev/sis8300-6,             6.0,             1.6,           319.7,            10.6,           565.9
	   func_readout,  /dev/sis8300-7,             6.0,             1.7,           320.0,            10.6,           565.6
	   func_readout,  /dev/sis8300-9,             6.0,             1.8,           325.0,            10.8,           556.8
	   func_readout,  /dev/sis8300-4,             6.0,             1.7,           325.5,            10.8,           555.8
	   func_readout,  /dev/sis8300-8,             6.0,             1.7,           331.9,            11.0,           545.1

### Data conversion

Findings:
* Time taken depends on what operations are performed
* Time taken **does not** depend on what are the scaling factor / offset
* Time taken **does** depend on what decimation factor is used

Parameters:
* Average over 30 readouts
* Sample is 16-bit
* Data size is 300000 samples x 10 channels
* Data is only stored in local buffer
* Possible data conversion to floating point performed

#### Readout

	$ ./bin/linux-x86_64/sis8300drv_threads /dev/sis8300-6 /dev/sis8300-7 /dev/sis8300-4 /dev/sis8300-8 /dev/sis8300-9 -S $((100*30000)) -N 30 -A 99
	Parameters:
	 function        1
	 access          0
	 nsamples        3000000
	 nbytes_raw      6000000 b (6.0 MB)
	 nbytes          12000000 b (12.0 MB)
	 nreads          30
	 dec_offset      0
	 dec_factor      1
	 scale_offset    0.000000
	 scale_factor    1.000000
	 ndevices        5
	
	           func,             dev,       size [MB],      start [ms],        end [ms],        dur [ms],    speed [MB/s]
	   func_readout,  /dev/sis8300-4,             6.0,             1.1,           317.6,            10.5,           568.7
	   func_readout,  /dev/sis8300-6,             6.0,             1.1,           325.0,            10.8,           555.7
	   func_readout,  /dev/sis8300-8,             6.0,             1.2,           333.2,            11.1,           542.1
	   func_readout,  /dev/sis8300-7,             6.0,             1.0,           333.4,            11.1,           541.6
	   func_readout,  /dev/sis8300-9,             6.0,             1.1,           343.0,            11.4,           526.5

#### Cast (convert) to float only

	 func_no_decsca,  /dev/sis8300-6,             6.0,           328.7,           506.8,             5.9,          1011.0
	 func_no_decsca,  /dev/sis8300-7,             6.0,           331.0,           509.9,             6.0,          1005.8
	 func_no_decsca,  /dev/sis8300-8,             6.0,           324.1,           512.3,             6.3,           956.3
	 func_no_decsca,  /dev/sis8300-4,             6.0,           340.4,           563.4,             7.4,           806.9
	 func_no_decsca,  /dev/sis8300-9,             6.0,           331.5,           569.7,             7.9,           755.6

#### Decimation and scaling

	    func_decsca,  /dev/sis8300-7,             6.0,           333.5,           827.0,            16.5,           364.7
	    func_decsca,  /dev/sis8300-8,             6.0,           333.3,           939.8,            20.2,           296.8
	    func_decsca,  /dev/sis8300-4,             6.0,           317.7,           947.3,            21.0,           285.9
	    func_decsca,  /dev/sis8300-6,             6.0,           325.0,           950.0,            20.8,           288.0
	    func_decsca,  /dev/sis8300-9,             6.0,           343.0,           960.2,            20.6,           291.6

#### Decimation only (factor == 1)

	       func_dec,  /dev/sis8300-9,             6.0,           321.3,           529.4,             6.9,           865.2
	       func_dec,  /dev/sis8300-8,             6.0,           319.1,           547.8,             7.6,           787.3
	       func_dec,  /dev/sis8300-7,             6.0,           323.0,           556.6,             7.8,           770.4
	       func_dec,  /dev/sis8300-6,             6.0,           338.9,           587.2,             8.3,           724.8
	       func_dec,  /dev/sis8300-4,             6.0,           328.7,           594.0,             8.8,           678.5

#### Decimation only (factor == 10)

	       func_dec,  /dev/sis8300-4,             6.0,           310.3,           351.6,             1.4,          4365.5
	       func_dec,  /dev/sis8300-7,             6.0,           316.8,           364.4,             1.6,          3777.4
	       func_dec,  /dev/sis8300-6,             6.0,           319.6,           367.7,             1.6,          3739.1
	       func_dec,  /dev/sis8300-9,             6.0,           326.8,           377.0,             1.7,          3582.1
	       func_dec,  /dev/sis8300-8,             6.0,           336.8,           379.1,             1.4,          4254.9

#### Scaling only

	       func_sca,  /dev/sis8300-9,             6.0,           322.7,           859.8,            17.9,           335.1
	       func_sca,  /dev/sis8300-4,             6.0,           334.4,           907.6,            19.1,           314.1
	       func_sca,  /dev/sis8300-7,             6.0,           328.3,           946.4,            20.6,           291.2
	       func_sca,  /dev/sis8300-6,             6.0,           343.1,           963.4,            20.7,           290.2
	       func_sca,  /dev/sis8300-8,             6.0,           328.7,           973.2,            21.5,           279.3

#### 2'complement unsigned --> double

	func_umn2double,  /dev/sis8300-6,             6.0,           318.9,           860.7,            18.1,           332.2
	func_umn2double,  /dev/sis8300-8,             6.0,           328.4,           923.7,            19.8,           302.3
	func_umn2double,  /dev/sis8300-4,             6.0,           340.1,           944.7,            20.2,           297.7
	func_umn2double,  /dev/sis8300-9,             6.0,           340.1,           963.2,            20.8,           288.9
	func_umn2double,  /dev/sis8300-7,             6.0,           330.1,           995.9,            22.2,           270.4


#### 2'complement signed --> double

	func_smn2double,  /dev/sis8300-4,             6.0,           326.7,           859.3,            17.8,           338.0
	func_smn2double,  /dev/sis8300-9,             6.0,           332.3,           929.4,            19.9,           301.5
	func_smn2double,  /dev/sis8300-8,             6.0,           336.4,           983.3,            21.6,           278.2
	func_smn2double,  /dev/sis8300-7,             6.0,           333.8,           995.9,            22.1,           271.9
	func_smn2double,  /dev/sis8300-6,             6.0,           333.5,          1023.3,            23.0,           261.0

### CPU utilization

Findings:
* **No** difference in CPU utilization if data size was 30000 or 300000 samples
* perf reports that most of the time spent was for **copying data to userspace**
* We can see lots of ioread32() calls; reading FPGA register 0x10 to detect end of DAQ
* End of DAQ is detected using DAQ interrupt CPU utlization drops to ~4%

Parameters:
* Single readout
* External trigger at 14 Hz
* Sample is 16-bit
* Data is stored in local buffer
* Possible data conversion to floating point performed
* Measured CPU utilization with *top* with refresh rate at 0.5 s
* Measured bottlenecks with *perf record -F 99 cmd*

### Readout only 30000 samples x 10 channels

#### 1 AMC (polling DAQ)

	$ perf record -F 99 ./bin/linux-x86_64/sis8300drv_threads /dev/sis8300-9 -S 300000 -l 100
	Parameters:
	 function        0
	 access          0
	 nsamples        300000
	 nbytes_raw      600000 b (0.6 MB)
	 nbytes          1200000 b (1.2 MB)
	 nreads          1
	 dec_offset      0
	 dec_factor      1
	 scale_offset    0.000000
	 scale_factor    1.000000
	 loop            100
	 ndevices        1

perf reports:

	  15.82%  sis8300drv_thre  [kernel.kallsyms]  [k] native_write_msr_safe
	   6.93%  sis8300drv_thre  [kernel.kallsyms]  [k] do_sync_read
	   6.86%  sis8300drv_thre  [kernel.kallsyms]  [k] perf_pmu_sched_task
	   6.26%  sis8300drv_thre  [kernel.kallsyms]  [k] copy_user_enhanced_fast_string
	   6.08%  sis8300drv_thre  [kernel.kallsyms]  [k] ioread32

~17% CPU time to readout data from a single AMC.

#### 1 AMC (DAQ interrupt)

	$ perf record -F 99 ./bin/linux-x86_64/sis8300drv_threads /dev/sis8300-9 -S 300000 -l 100 -i
	Parameters:
	 function        0
	 access          0
	 nsamples        300000
	 nbytes_raw      600000 b (0.6 MB)
	 nbytes          1200000 b (1.2 MB)
	 nreads          1
	 dec_offset      0
	 dec_factor      1
	 scale_offset    0.000000
	 scale_factor    1.000000
	 loop            100
	 daq_intr        1
	 ndevices        1

perf reports:

	  45.91%  sis8300drv_thre  [kernel.kallsyms]  [k] copy_user_enhanced_fast_string
	  25.17%  sis8300drv_thre  [kernel.kallsyms]  [k] kmem_cache_alloc_node
	  12.48%  sis8300drv_thre  [sis8300drv]       [k] sis8300_ioctl
	  11.17%  sis8300drv_thre  [kernel.kallsyms]  [k] cpuacct_charge
	   3.12%  sis8300drv_thre  [kernel.kallsyms]  [k] flush_tlb_mm_range

~4% CPU time to readout data from a single AMC.

#### 5 AMCs (polling DAQ)

	$ perf record -F 99 ./bin/linux-x86_64/sis8300drv_threads /dev/sis8300-9 /dev/sis8300-4 /dev/sis8300-6 /dev/sis8300-7 /dev/sis8300-8 -S 3000000 -l 100
	Parameters:
	 function        0
	 access          0
	 nsamples        3000000
	 nbytes_raw      6000000 b (6.0 MB)
	 nbytes          12000000 b (12.0 MB)
	 nreads          1
	 dec_offset      0
	 dec_factor      1
	 scale_offset    0.000000
	 scale_factor    1.000000
	 loop            100
	 ndevices        5

perf reports:

	  41.04%  sis8300drv_thre  [kernel.kallsyms]  [k] copy_user_enhanced_fast_string
	  14.21%  sis8300drv_thre  [kernel.kallsyms]  [k] ioread32

~100% CPU time to readout data from five AMCs.

#### 5 AMCs (DAQ interrupt)

	$ perf record -F 99 ./bin/linux-x86_64/sis8300drv_threads /dev/sis8300-9 /dev/sis8300-4 /dev/sis8300-6 /dev/sis8300-7 /dev/sis8300-8 -S 3000000 -l 100 -i
	Parameters:
	 function        0
	 access          0
	 nsamples        3000000
	 nbytes_raw      6000000 b (6.0 MB)
	 nbytes          12000000 b (12.0 MB)
	 nreads          1
	 dec_offset      0
	 dec_factor      1
	 scale_offset    0.000000
	 scale_factor    1.000000
	 loop            100
	 daq_intr        1
	 ndevices        5

perf reports:

	  78.40%  sis8300drv_thre  [kernel.kallsyms]  [k] copy_user_enhanced_fast_string
	   6.38%  sis8300drv_thre  ld-2.17.so         [.] do_lookup_x
	   6.17%  sis8300drv_thre  [kernel.kallsyms]  [k] __find_get_page
	   1.75%  sis8300drv_thre  [kernel.kallsyms]  [k] tg_load_down

~44% CPU time to readout data from five AMCs.




### Readout only 300000 samples x 10 channels

~17% CPU time to readout data from a single AMC.

~100% CPU time to readout data from five AMCs.

### Readout only 600000 samples x 10 channels

~17% CPU time to readout data from a single AMC.

~100% CPU time to readout data from five AMCs.

### Readout only 900000 samples x 10 channels

~17% CPU time to readout data from a single AMC.

~100% CPU time to readout data from five AMCs.


### Conversion 30000 samples x 10 channels

	$ perf record -F 99 ./bin/linux-x86_64/sis8300drv_threads /dev/sis8300-7 -S 300000 -l 100 -A 5
	Parameters:
	 function        5
	 access          0
	 nsamples        300000
	 nbytes_raw      600000 b (0.6 MB)
	 nbytes          1200000 b (1.2 MB)
	 nreads          1
	 dec_offset      0
	 dec_factor      1
	 scale_offset    0.000000
	 scale_factor    1.000000
	 loop            100
	 ndevices        1

perf reports

	  37.22%  sis8300drv_thre  sis8300drv_threads  [.] func_umn2double
	  10.68%  sis8300drv_thre  [kernel.kallsyms]   [k] ioread32
	   8.12%  sis8300drv_thre  ld-2.17.so          [.] do_lookup_x
	   6.45%  sis8300drv_thre  [kernel.kallsyms]   [k] __audit_syscall_exit


~23% CPU time to readout data from a single AMC.


	$ ./bin/linux-x86_64/sis8300drv_threads /dev/sis8300-7 /dev/sis8300-4 /dev/sis8300-6 /dev/sis8300-8 /dev/sis8300-9 -S 300000 -l 100 -A5
	Parameters:
	 function        5
	 access          0
	 nsamples        300000
	 nbytes_raw      600000 b (0.6 MB)
	 nbytes          1200000 b (1.2 MB)
	 nreads          1
	 dec_offset      0
	 dec_factor      1
	 scale_offset    0.000000
	 scale_factor    1.000000
	 loop            100
	 ndevices        5


perf reports

	  37.79%  sis8300drv_thre  sis8300drv_threads  [.] func_umn2double
	  14.92%  sis8300drv_thre  [kernel.kallsyms]   [k] ioread32
	   3.26%  sis8300drv_thre  [kernel.kallsyms]   [k] native_write_msr_safe
	   2.71%  sis8300drv_thre  [kernel.kallsyms]   [k] copy_user_enhanced_fast_string

~111% CPU time to readout data from five AMCs.


### Conversion 300000 samples x 10 channels

~62% CPU time to readout data from a single AMC.

~250% CPU time to readout data from five AMCs.

### Conversion 3000 samples x 10 channels

~15% CPU time to readout data from a single AMC.

~90% CPU time to readout data from five AMCs.

### Conversion 900000 samples x 10 channels

~17% CPU time to readout data from a single AMC.

~100% CPU time to readout data from five AMCs.


### 2'complement conversion to double function

	       │     void mn_2_double(uint32_t val, struct Qmn Qmn, double *converted) {                                                                               ▒
	       │                                                                                                                                                       ▒
	       │             double pow_2_frac_bits = (double) (0x1UL << Qmn.frac_bits_n);                                                                             ▒
	       │             double pow_2_frac_bits_int_bits = (double) (0x1UL << (Qmn.int_bits_m + Qmn.frac_bits_n));                                                 ▒
	       │                                                                                                                                                       ▒
	       │             *converted = (double) val;                                                                                                                ▒
	  2.37 │178:┌─→movzwl (%rcx,%rax,2),%r8d                                                                                                                       ▒
	 20.76 │    │  cvtsi2 %r8d,%xmm0                                                                                                                               ▒
	       │    │        if (Qmn.is_signed && (*converted > (pow_2_frac_bits_int_bits / 2.0 - 1.0))) {                                                             ▒
	       │    │                *converted -= pow_2_frac_bits_int_bits;                                                                                           ▒
	       │    │        }                                                                                                                                         ▒
	       │    │                                                                                                                                                  ▒
	       │    │        /* convert to double */                                                                                                                   ▒
	       │    │        *converted /= pow_2_frac_bits;                                                                                                            ▒
	 54.71 │    │  divsd  %xmm1,%xmm0                                                                                                                              ▒
	  1.85 │    │  movsd  %xmm0,(%rdx,%rax,4)                                                                                                                      ▒
	  0.07 │    │  add    $0x1,%rax                                                                                                                                ▒
	       │    │func_umn2double():                                                                                                                                ▒
	       │    │int func_umn2double(struct arg *a) {                                                                                                              ▒
	       │    │        int i, j;                                                                                                                                 ▒
	       │    │                                                                                                                                                  ▒
	       │    │        a->start = time_msec();                                                                                                                   ▒
	       │    │        for (i = 0; i < a->nreads; ++i) {                                                                                                         ▒
	       │    │                for (j = 0; j < a->nsamples; j++) {                                                                                               ▒
	 20.24 │    │  cmp    %eax,%esi                                                                                                                                ▒
	       │    └──ja     178                                                                                                                                      ▒
	       │                                                                                                                                                       ▒
