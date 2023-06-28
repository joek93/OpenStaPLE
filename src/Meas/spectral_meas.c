#ifndef SPECTRAL_MEAS_C_
#define SPECTRAL_MEAS_C_

#include "./spectral_meas.h"
#include "../Include/common_defines.h"
#include "../Include/montecarlo_parameters.h"

spectr_meas_params spm_par;

void spectral_measures(su3_soa * tconf_acc,
                       spectr_meas_params * tspm_par,
        int conf_id_iter)
{

  // FILE checks
  // find the actual file size
  FILE *foutfile;
  int fsize;


    
  if(devinfo.myrank_world == 0 ){
    foutfile = fopen(tspm_par->spmeas_outfilename,"ab");
    if(foutfile){
        fseek(foutfile, 0L, SEEK_END);
        fsize = ftell(foutfile);
        fseek(foutfile, 0L, SEEK_SET);
        fsize -= ftell(foutfile);

    }else {
        printf("File %s can't be opened for writing. Exiting.\n", tspm_par->spmeas_outfilename);
        exit(1);
    }
    fclose(foutfile);// found file size


    // proceed by saving measurement
    foutfile = fopen(tspm_par->spmeas_outfilename,"ab");
    if(!foutfile) {
        printf("File %s can't be opened for writing. Exiting.\n", tspm_par->spmeas_outfilename);
        exit(1);
    }
    int icopy = mc_params.measures_done_spectr;
    fprintf(foutfile,"%d\t%d\t",conf_id_iter,icopy);

    fprintf(foutfile,"%.18lf\t%.18lf\t",0.0, 0.0);
    
  }

  mc_params.measures_done_spectr++; 
}


#endif
