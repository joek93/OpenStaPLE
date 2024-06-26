#ifndef REP_INFO_C
#define REP_INFO_C

#include "./rep_info.h"

rep_info *rep;
#ifdef PAR_TEMP
rep_utils *r_utils;

int get_index_of_pbc_replica(){
  // finds index corresponding to label=0
  int ridx_lab0;
  for(ridx_lab0=0; 0!=rep->label[ridx_lab0]; ++ridx_lab0){}

  return ridx_lab0;
}

void setup_replica_utils(char save_conf_name[100]){
  //TODO: in principle, these can be allocated on world master only (see src/OpenAcc/HPT_utilities.c)
  r_utils->swap_number=0;
  r_utils->all_swap_vector=malloc(sizeof(int)*(rep->replicas_total_number-1));
  r_utils->acceptance_vector=malloc(sizeof(int)*(rep->replicas_total_number-1));
  r_utils->acceptance_vector_old=malloc(sizeof(int)*(rep->replicas_total_number-1));
    
  for(int lab=0;lab<rep->replicas_total_number-1;lab++){
    r_utils->acceptance_vector[lab]=0;
    r_utils->acceptance_vector_old[lab]=0;
    r_utils->all_swap_vector[lab]=0;
  }

  //defect_info def;
  strcpy(r_utils->aux_name_file,save_conf_name);
}

void free_replica_utils(){
  free(r_utils->all_swap_vector);
  free(r_utils->acceptance_vector);
  free(r_utils->acceptance_vector_old);

  // freeing rep_info vectors
  free(rep->cr_vec);
  free(rep->label);
}

void label_print(rep_info * hpt_params, FILE *file, int step_number){

  int i;
  fprintf(file,"%d    ",step_number);    
  for(i=0;i<hpt_params->replicas_total_number;i++){
    fprintf(file,"%d    ", hpt_params->label[i]);
  }
  fprintf(file,"\n");
}


#endif // PAR_TEMP

#endif
