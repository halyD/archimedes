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
 * Main Test Program for the NEUREKA RV Cluster coexecution
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

#define NB_CORE_ITER 100
#define NB_NEUREKA_ITER 6

L1_DATA int array[NB_CORE_ITER*7];

int run_test() {

  uint8_t* infeat         = neureka_infeat;
  uint8_t* weight         = neureka_weights;
  uint8_t* nq_scale       = neureka_scale+4;
  uint8_t* nq_scale_shift = neureka_scale_shift;
  uint8_t* nq_scale_bias  = neureka_scale_bias;
  uint8_t* golden_outfeat = neureka_golden_outfeat;
  uint8_t* actual_outfeat = neureka_actual_outfeat;
  uint8_t* streamin       = neureka_streamin;

  // enable clock
  NEUREKA_CG_ENABLE();

  // setup HCI
  NEUREKA_SETPRIORITY_CORE();
  // NEUREKA_SETPRIORITY_NEUREKA();
  NEUREKA_RESET_MAXSTALL();
  NEUREKA_SET_MAXSTALL(MAX_STALL);

  // soft-clear NEUREKA
  NEUREKA_WRITE_CMD(NEUREKA_SOFT_CLEAR, NEUREKA_SOFT_CLEAR_ALL);
  for(volatile int kk=0; kk<10; kk++);
  int job_id = -1;
 
  NEUREKA_BARRIER_ACQUIRE(job_id);
  NEUREKA_WRITE_REG(NEUREKA_REG_WEIGHTS_PTR,     weight);
  NEUREKA_WRITE_REG(NEUREKA_REG_INFEAT_PTR,      infeat);
  NEUREKA_WRITE_REG(NEUREKA_REG_OUTFEAT_PTR,     actual_outfeat);
  NEUREKA_WRITE_REG(NEUREKA_REG_SCALE_PTR,       nq_scale);
  NEUREKA_WRITE_REG(NEUREKA_REG_SCALE_SHIFT_PTR, nq_scale_shift);
  NEUREKA_WRITE_REG(NEUREKA_REG_SCALE_BIAS_PTR,  nq_scale_bias);
  NEUREKA_WRITE_REG(NEUREKA_REG_STREAMIN_PTR,    streamin);
  for(int i=7; i<25; i++) {
    NEUREKA_WRITE_REG(i*4, neureka_cfg[i]);
  }
  NEUREKA_WRITE_CMD(NEUREKA_COMMIT_AND_TRIGGER, NEUREKA_TRIGGER_CMD);

  NEUREKA_BARRIER_ACQUIRE(job_id);
  NEUREKA_WRITE_REG(NEUREKA_REG_WEIGHTS_PTR,     weight);
  NEUREKA_WRITE_REG(NEUREKA_REG_INFEAT_PTR,      infeat);
  NEUREKA_WRITE_REG(NEUREKA_REG_OUTFEAT_PTR,     actual_outfeat);
  NEUREKA_WRITE_REG(NEUREKA_REG_SCALE_PTR,       nq_scale);
  NEUREKA_WRITE_REG(NEUREKA_REG_SCALE_SHIFT_PTR, nq_scale_shift);
  NEUREKA_WRITE_REG(NEUREKA_REG_SCALE_BIAS_PTR,  nq_scale_bias);
  NEUREKA_WRITE_REG(NEUREKA_REG_STREAMIN_PTR,    streamin);
  for(int i=7; i<25; i++) {
    NEUREKA_WRITE_REG(i*4, neureka_cfg[i]);
  }
  NEUREKA_WRITE_CMD(NEUREKA_COMMIT_AND_TRIGGER, NEUREKA_TRIGGER_CMD);

  do {
    NEUREKA_BARRIER_ACQUIRE(job_id);
    NEUREKA_WRITE_CMD(NEUREKA_COMMIT_AND_TRIGGER, NEUREKA_TRIGGER_CMD);
  } while(job_id < NB_NEUREKA_ITER-1);
  NEUREKA_BARRIER();
  NEUREKA_CG_DISABLE();
 
  int errors = neureka_compare_int(actual_outfeat, golden_outfeat, STIM_Y_SIZE/4);
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
