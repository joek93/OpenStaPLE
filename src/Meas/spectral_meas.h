#ifndef SPECTRAL_MEAS_H_
#define SPECTRAL_MEAS_H_

#include <stdio.h>
#include <stdlib.h>
#include "../Include/common_defines.h"
#include "../OpenAcc/struct_c_def.h"
#ifdef MULTIDEVICE
#include "../Mpi/communications.h"
#endif
#include "../Mpi/multidev.h"

typedef struct spectr_meas_param_t{

    int measEvery;
    char spmeas_outfilename[50];
} spectr_meas_params;

extern spectr_meas_params spm_par;

void spectral_measures(su3_soa * tconf_acc,
                       spectr_meas_params * tspm_par,
                       int conf_id_iter);

#endif
