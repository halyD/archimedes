# ArchiMEDES 

Architecture for Minimum Energy DNNs at Edge and domain-specific processing(ArchiMEDES) is a heterogeneous cluster template designed for AR/VR applications. The AR/VR applications are still evolving. They require DNN acceleration as well as general-purpose DSP computing while constrained by the power and latency budget offered by the AR/VR application. ArchiMEDES focuses on the concurrent and cooperative execution of DNN and DSP tasks by adding Specialization and Flexibility. 
1. Specialization is added by employing hardware acceleration of commonly used DNN inference workloads via a dedicated Neural Engine called NEureka(The name is derived from the famous Archimedes quote, "Eureka! - I found it"). NEureka targets acceleration of 1x1, 3x3 and 3x3 depthwise convolutions - the most commonly used kernels for the State-of-the-Art DNNs. NEureka can seamlessly support 2-8bit weights along with 8-bit activations, providing an opportunity to reduce the execution cycles and/or memory footprints in lower precision in some kernel executions. 
2. NEureka is integrated with RISC-V DSP cluster derived from  [`PULP`](https://github.com/pulp-platform/pulp) open-source design. The RISC-V cores and NEureka share a tightly coupled data memory(TCDM). The RISC-V cores add flexibility to execute general-purpose workloads. Additionally, the custom extension enables faster execution of DSP workloads.
3. A heterogeneous approach is used to share TCDM with RISC-V cores and NEureka through HCI efficiently interconnect in [`HCI` repository](https://github.com/pulp-platform/hci.git). Additionally, a tunable knob, MAX_STALL could be exploited to prioritise the memory access from either NEureka or RISC-V, which could help to achieve better performance in latency-sensitive applications.

This repository is built on top of [`pulp_cluster`](https://github.com/pulp-platform/pulp_cluster.git)

This repository contains the structure of the cluster subsystem
used in PULP chips. For more details on the internal architecture, see the
README.md in the [`pulp` repository](https://github.com/pulp-platform/pulp).

![pulp_cluster schematic](doc/PULP_CLUSTER.png)
The [`doc`](doc/) folder contains the draw.io schematic shown above, as well as
the raw source to allow for updates, outlining the most important components in
pulp_cluster, as well as the communication interfaces connecting these. While
not a complete overview of all signals, this is meant as a slightly more
detailed overview that can assist in development. Please be aware that the
schematic may not be 100% accurate.

## Get Started 
To get started with ArchiMEDES we have provided a few tests in the archimedes_tests folder. Every folder has the following structure 
- `test.c` - main test application
- `Makefile` - To enable the test run 
- `inc` - optional include files needed for the application 
- `hal_neureka.h` - hardware abstraction layer for NEureka 

The following tests are given as a reference. 
- `Conv3x3/Wbit8` - A kernel demonstrating 3x3 convolution with Weight precision=8 bits
- `Conv3x3/Wbit2` - A kernel demonstrating 3x3 convolution with Weight precision=2 bits. Compared to the previous test, this test takes fewer cycles and has a lower weight footprint due to the bit-serial nature of the accelerator in this mode of operation 
- `DW3x3` - Showing a depthwise layer kernel execution on NEureka
- `Conv1x1` - Mapping a pointwise layer kernel on NEureka  
- `hci`- A very simple synthetic workload is provided to show the impact of HCI in the NEureka and RISC-V cluster cores co-execution. MAX_STALL parameter can be swept to see the stalls observed by NEureka
- `streamin` - NEureka uses 32-bit data to initialize the accumulation buffers in this test. On top of it, NEureka performs convolution and generates 32-bit output data. 


## Simulation

To perform a simulation in ArchiMEDES follow the following steps:

1. Ensure the PULP RV32 toolchain is in your `PATH`. Please refer to [PULP
   RISCV GCC toolchain](https://github.com/pulp-platform/pulp-riscv-gcc) to use
   a pre-built release.

2. We need the RV64 toolchain to compile DPI libraries. Export it to a `RISCV` env
   variable. Please refer to [RISC-V GNU
   toolchain](https://github.com/riscv-collab/riscv-gnu-toolchain/) to use a
   pre-built release.

3. Compile the hw:
   ```
   make checkout
   make scripts/compile.tcl
   make build
   ```
4. Download the SW stack:
	```
	make pulp-runtime
	```
5. Source the environment:
   ```
   source env/env.sh
   ```

6. To run these tests. Choose any test among the `archimedes_tests`, for example, `DW3x3` tests. 
   Move to the respective folder. 

   ```
   cd archimedes_tests/DW3x3
   make clean all run
   ```

   To use the GUI, add `gui=1` to the previous command.

7. To run applications on RISC-V cluster cores, download the bare-metal tests and pick any applications from `parallel_bare_tests` and `mchan_tests`. The applications could be downloaded by performing 
   ```
   make regression-tests 
   ```
## Publications
If you are using ArchiMEDES, you can cite us at
```
@inproceedings{prasad2023specialization,
  title={Specialization meets Flexibility: a Heterogeneous Architecture for High-Efficiency, High-flexibility AR/VR Processing},
  author={Prasad, Arpan Suravi and Benini, Luca and Conti, Francesco},
  booktitle={2023 60th ACM/IEEE Design Automation Conference (DAC)},
  pages={1--6},
  year={2023},
  organization={IEEE}
}
```
