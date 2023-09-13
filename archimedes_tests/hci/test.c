/*
 * Copyright (C) 2020 ETH Zurich, University of Bologna and GreenWaves Technologies
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

/*
 * Authors:  Francesco Conti <fconti@iis.ee.ethz.ch>
 *           Gianna Paulin <pauling@iis.ee.ethz.ch>
 *           Renzo Andri <andrire@iis.ee.ethz.ch>
 *           Arpan Suravi Prasad <prasadar@iis.ee.ethz.ch>
 * Main Test Program for the NEUREKA
 */

#include <stdint.h>
#include <stdio.h>

#include <pulp.h>
#include "hal_neureka.h"
#include "inc/neureka_cfg.h"
#include "inc/neureka_infeat.h"
#include "inc/neureka_weights.h"
#include "inc/neureka_scale.h"
#include "inc/neureka_scale_bias.h"
#include "inc/neureka_scale_shift.h"
#include "inc/neureka_streamin.h"
#include "inc/neureka_actual_outfeat.h"
#include "inc/neureka_golden_outfeat.h"

#define NB_CORE_ITER 1000
#define NB_NEUREKA_ITER 6

L1_DATA int array[NB_CORE_ITER*7];

int run_test() {

  uint8_t* infeat         = (uint8_t volatile*) pi_l1_malloc(0, STIM_X_SIZE);
  uint8_t* nq_scale       = (uint8_t volatile*) pi_l1_malloc(0, STIM_NQ_SIZE);
  uint8_t* nq_scale_shift = (uint8_t volatile*) pi_l1_malloc(0, STIM_NQS_SIZE);
  uint8_t* nq_scale_bias  = (uint8_t volatile*) pi_l1_malloc(0, STIM_NQB_SIZE);
  uint8_t* actual_outfeat = (uint8_t volatile*) pi_l1_malloc(0, STIM_Y_SIZE);
  uint8_t* streamin       = (uint8_t volatile*) pi_l1_malloc(0, STIM_ST_SIZE);
  uint8_t* weight         = (uint8_t volatile*) pi_l1_malloc(0, STIM_W_SIZE);
  uint8_t* golden_outfeat = neureka_golden_outfeat;

  plp_dma_wait(plp_dma_memcpy((uint32_t)neureka_infeat,         (uint32_t)infeat,         STIM_X_SIZE,    1));
  plp_dma_wait(plp_dma_memcpy((uint32_t)neureka_actual_outfeat, (uint32_t)actual_outfeat, STIM_Y_SIZE,    1));
  plp_dma_wait(plp_dma_memcpy((uint32_t)neureka_scale,          (uint32_t)nq_scale,       STIM_NQ_SIZE,   1));
  plp_dma_wait(plp_dma_memcpy((uint32_t)neureka_scale_shift,    (uint32_t)nq_scale_shift, STIM_NQS_SIZE,  1));
  plp_dma_wait(plp_dma_memcpy((uint32_t)neureka_scale_bias,     (uint32_t)nq_scale_bias,  STIM_NQB_SIZE,  1));
  plp_dma_wait(plp_dma_memcpy((uint32_t)neureka_streamin,       (uint32_t)streamin,       STIM_ST_SIZE,   1));
  plp_dma_wait(plp_dma_memcpy((uint32_t)neureka_weights,        (uint32_t)weight,         STIM_W_SIZE,    1));

  // // setup HCI
  NEUREKA_SETPRIORITY_NEUREKA();
  NEUREKA_RESET_MAXSTALL();
  NEUREKA_SET_MAXSTALL(MAX_STALL);

  // // soft-clear NEUREKA
  NEUREKA_WRITE_CMD(NEUREKA_SOFT_CLEAR, NEUREKA_SOFT_CLEAR_ALL);
  for(volatile int kk=0; kk<10; kk++);
  int job_id = -1;

  NEUREKA_BARRIER_ACQUIRE(job_id); // Acquiring a job ID
  // // CONFIGURING NEUREKA REGISTERS
  NEUREKA_WRITE_REG(NEUREKA_REG_WEIGHTS_PTR,     weight); // Weight pointer
  NEUREKA_WRITE_REG(NEUREKA_REG_INFEAT_PTR,      infeat); // Input Feature pointer
  NEUREKA_WRITE_REG(NEUREKA_REG_OUTFEAT_PTR,     actual_outfeat); // Output Feature pointer - this is the location where the accelerator will write the processed output
  NEUREKA_WRITE_REG(NEUREKA_REG_SCALE_PTR,       (nq_scale)); // scaling parameter pointer 
  NEUREKA_WRITE_REG(NEUREKA_REG_SCALE_SHIFT_PTR, nq_scale_shift); // scaling with shift 
  NEUREKA_WRITE_REG(NEUREKA_REG_SCALE_BIAS_PTR,  nq_scale_bias);// scaling with bias
  NEUREKA_WRITE_REG(NEUREKA_REG_STREAMIN_PTR,    streamin);//The data from this location could be used to initialize the accumulator buffer 
  for(int i=7; i<25; i++) {
    NEUREKA_WRITE_REG(i*4, neureka_cfg[i]); // Other configuration regsiters(mode of operation, channel size, output size etc)
  }
  NEUREKA_WRITE_CMD(NEUREKA_COMMIT_AND_TRIGGER, NEUREKA_TRIGGER_CMD); // Offloading the task to NEureka

  NEUREKA_BARRIER_ACQUIRE(job_id); // Acquiring a job ID
  // // CONFIGURING NEUREKA REGISTERS
  NEUREKA_WRITE_REG(NEUREKA_REG_WEIGHTS_PTR,     weight); // Weight pointer
  NEUREKA_WRITE_REG(NEUREKA_REG_INFEAT_PTR,      infeat); // Input Feature pointer
  NEUREKA_WRITE_REG(NEUREKA_REG_OUTFEAT_PTR,     actual_outfeat); // Output Feature pointer - this is the location where the accelerator will write the processed output
  NEUREKA_WRITE_REG(NEUREKA_REG_SCALE_PTR,       (nq_scale)); // scaling parameter pointer 
  NEUREKA_WRITE_REG(NEUREKA_REG_SCALE_SHIFT_PTR, nq_scale_shift); // scaling with shift 
  NEUREKA_WRITE_REG(NEUREKA_REG_SCALE_BIAS_PTR,  nq_scale_bias);// scaling with bias
  NEUREKA_WRITE_REG(NEUREKA_REG_STREAMIN_PTR,    streamin);//The data from this location could be used to initialize the accumulator buffer 
  for(int i=7; i<25; i++) {
    NEUREKA_WRITE_REG(i*4, neureka_cfg[i]); // Other configuration regsiters(mode of operation, channel size, output size etc)
  }
  NEUREKA_WRITE_CMD(NEUREKA_COMMIT_AND_TRIGGER, NEUREKA_TRIGGER_CMD); // Offloading the task to NEureka

  do {
    NEUREKA_BARRIER_ACQUIRE(job_id);
    NEUREKA_WRITE_CMD(NEUREKA_COMMIT_AND_TRIGGER, NEUREKA_TRIGGER_CMD);
  } while(job_id < NB_NEUREKA_ITER-1);
 
  NEUREKA_BARRIER(); // Wait for the interrupt, here core goes to sleep by performing clock gating
  NEUREKA_CG_DISABLE();

  int errors = neureka_compare_int(actual_outfeat, golden_outfeat, STIM_Y_SIZE/4);// Output generated by NEureka is comapred against the golden model 
  return errors;
}

int main() {
  if (rt_cluster_id() != 0){
    return bench_cluster_forward(0);
  }
  if(rt_cluster_id()== 0) {
    if(rt_core_id()==0){
      return run_test(); // Core0 is running NEureka Workload for NB_NEUREKA_ITER times
    }
    else
    {
      volatile int index = rt_core_id();
      for(int i=0; i<NB_CORE_ITER; i++)
      {
        array[index] = i;  // Core1-7 is writing to memory for NB_CORE_ITER times
        index = index+7;
      }
    }
    synch_barrier();
  }
}
