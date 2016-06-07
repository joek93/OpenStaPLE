#include <stdio.h>
#include <stdlib.h>

#ifndef __GNUC__
#include "openacc.h"
#endif

#include "../Include/common_defines.h"
#include "../Include/setting_file_parser.h"
#include "../Include/fermion_parameters.h"
#include "../OpenAcc/alloc_vars.h"
#include "../OpenAcc/inverter_multishift_full.h"
#include "../OpenAcc/io.h"
#include "../OpenAcc/random_assignement.h"
#include "../Rand/random.h"
#include "../RationalApprox/rationalapprox.h"
#include "./dbgtools.h"
#include "../OpenAcc/action.h"
#include "../Include/montecarlo_parameters.h"
#include "../Include/debug.h"
#include "../OpenAcc/deviceinit.h" 
#include <sys/time.h>
#include "../OpenAcc/md_integrator.h"

#include "../Mpi/multidev.h"
#ifdef MULTIDEVICE
#include <mpi.h>
#endif

//double casuale(void);


int id_iter;
int verbosity_lv;

void init_fake_rational_approx(RationalApprox* rational_approx,
        double a,double b, int order){
    // a0=0, an=a, bn=b  
    printf("Initializing fake rational approximation, constant term = 0, all terms \
            %f/(x+%f)",a,b);

    rational_approx->exponent_den = 0;
    rational_approx->exponent_num = 0;
    rational_approx->approx_order = order;
    rational_approx->lambda_min = 0.1 ; 
    rational_approx->lambda_max = 1.0 ;
    rational_approx->gmp_remez_precision = 0;
    rational_approx->error = 1.0;
    rational_approx->RA_a0 = 0; 
    int i;
    for(i = 0; i < order; i++){
        rational_approx->RA_a[i]= a;
        rational_approx->RA_b[i]= b;
    }
}



int main(int argc, char* argv[]){

    const char confname[50] = "test_conf";
    const char fermionname[50] = "test_fermion";
    char myconfname             [50];
    char myfermionname          [50];


    act_params.stout_steps = 0;
    NPS_tot = 1;


    // INITIALIZATION
#ifdef MULTIDEVICE
    pre_init_multidev1D(&devinfo);
#endif
    fflush(stdout);
    printf("DEODOE test\n");
    // INIT FERM PARAMS AND READ RATIONAL APPROX COEFFS
    printf("WELCOME! \n");
    // READ input file.
    set_global_vars_and_fermions_from_input_file(argv[1]);

    initrand((unsigned int) mc_params.seed+devinfo.myrank);
    verbosity_lv = debug_settings.input_vbl;
    // INIT FERM PARAMS AND READ RATIONAL APPROX COEFFS
    if(init_ferm_params(fermions_parameters))
        printf("Ignoring issues in init_ferm_params,\
                this is a multishift inverter test.\n");

    if(NPS_tot == 0) 
    {
        printf("ERROR: For technical reasons, at least a fermion in the input file is needed\n");
        exit(1);
    }
    else {
        printf("N fermions found: %d\n", NPS_tot);
    }
    //

#ifdef MULTIDEVICE
    init_multidev1D(&devinfo);
#else
    devinfo.myrank = 0;
    devinfo.nranks = 1;
#endif

    mem_alloc();
    printf("Allocazione della memoria : OK \n");
    compute_nnp_and_nnm_openacc();
    printf("nn computation : OK \n");
    init_all_u1_phases(backfield_parameters,fermions_parameters);
    printf("u1_backfield initialization : OK \n");
    sprintf(myconfname             ,"%s_MPI%02d",confname             ,devinfo.myrank);
    sprintf(myfermionname          ,"%s_MPI%02d",fermionname          ,devinfo.myrank);


#ifndef __GNUC__
    //////  OPENACC CONTEXT INITIALIZATION    //////////////////////////////////////////////////////
    // NVIDIA GPUs
    acc_device_t my_device_type = acc_device_nvidia;
    // AMD GPUs
    // acc_device_t my_device_type = acc_device_radeon;
    // Intel XeonPhi
    //acc_device_t my_device_type = acc_device_xeonphi;
    // Select device ID
    printf("MPI%02d: Selecting device.\n");
#ifdef MULTIDEVICE
    select_init_acc_device(my_device_type, devinfo.myrank%devinfo.proc_per_node);
#else
    select_init_acc_device(my_device_type, devinfo.single_dev_choice);
#endif
    printf("Device Selected : OK \n");
#endif
    int conf_id_iter = 0;
    if(!read_conf_wrapper(conf_acc,mc_params.save_conf_name,
                &conf_id_iter,debug_settings.use_ildg)){
        // READS ALSO THE conf_id_iter
        printf("MPI%02d - Stored Gauge Conf \"%s\" Read : OK \n",
                devinfo.myrank, mc_params.save_conf_name);

    }
    else{
        generate_Conf_cold(conf_acc,mc_params.eps_gen);
        printf("MPI%02d - Cold Gauge Conf Generated : OK \n",
                devinfo.myrank);
        save_conf_wrapper(conf_acc,mc_params.save_conf_name,0,debug_settings.use_ildg);
        if(debug_settings.use_ildg)
            printf("MPI%02d: You're using ILDG format.\n", devinfo.myrank);
        conf_id_iter=0;
    }


    // init fermion
    //generate_vec3_soa_gauss(ferm_chi_acc);
    //print_vec3_soa(ferm_chi_acc,"ferm_chi_acc");
    if(!read_vec3_soa_wrapper(ferm_chi_acc,fermionname )){
        printf("MPI%02d - Fermion READ : OK \n",devinfo.myrank);
    }else{
        printf("MPI%02d - Fermion READ : FAIL, generating\n",devinfo.myrank);
        generate_vec3_soa_gauss(ferm_chi_acc);
        printf("MPI%02d - Fermion GENERATED : OK\n",devinfo.myrank);
        printf("MPI%02d - Writing file %s.\n",devinfo.myrank, fermionname);
        print_vec3_soa_wrapper(ferm_chi_acc,fermionname);
    }


#ifdef MULTIDEVICE
    communicate_fermion_borders_hostonly(ferm_chi_acc);
#endif

    //    print_vec3_soa(ferm_chi_acc,myfermionname);



    // fake rational approx

    RationalApprox fakeRationalApprox;
    double minshift = fermions_parameters->ferm_mass;
    init_fake_rational_approx(&fakeRationalApprox, 1, minshift*minshift, 15);
    if(0==devinfo.myrank){
        printf("Using fermions_parameters->ferm_mass ^2 as shiftn");
    }



    //  
    //#pragma acc data  copyin(conf_acc[0:8]) copyin(ferm_chi_acc[0:1])\
    create(ferm_phi_acc[0:1])  copyin(u1_back_phases[0:8*NDiffFlavs]) \
        create(kloc_s[0:1])
#pragma acc data  copy(conf_acc[0:8]) copy(ferm_chi_acc[0:1])\
        copy(ferm_phi_acc[0:1])  copy(u1_back_phases[0:8*NDiffFlavs]) \
        create(kloc_r[0:1]) create(kloc_h[0:1]) create(kloc_s[0:1]) create(kloc_p[0:1]) \
        create(ferm_shiftmulti_acc[max_ps*MAX_APPROX_ORDER] \
                create(k_p_shiftferm[max_ps*MAX_APPROX_ORDER] 
                    {
                    struct timeval t0,t1,t2,t3,t4,t5;
                    int r;
                    if(0 == devinfo.myrank){
                    printf("Multishift Inversion, %d times, with residue %e, shift %e\n",
                        mc_params.ntraj,md_parameters.residue_metro, minshift*minshift );
                    printf("max_cg_iterations: %d\n", md_parameters.max_cg_iterations);

                    }

                    gettimeofday(&t0,NULL);
                    multishift_invert_iterations = 0;
                    for(r=0; r<mc_params.ntraj; r++)
                    multishift_invert(conf_acc,&fermions_parameters[0],
                        &fakeRationalApprox,
                        ferm_shiftmulti_acc,
                        ferm_chi_acc,
                        md_parameters.residue_metro,
                        kloc_r,
                        kloc_h,
                        kloc_s,
                        kloc_p,
                        k_p_shiftferm,
                        md_parameters.max_cg_iterations);
                gettimeofday(&t1,NULL);

                for(r=0; r<fakeRationalApprox.approx_order; r++){

                    char fermionname_shift[50];
                    sprintf(fermionname_shift,"fermion_shift_%d.dat",r);

                    // shift fermio names
                    printf("Writing file %s.\n", fermionname_shift);

#pragma acc update host(ferm_phi_acc[0:1]) // update on host the right fermion
                    print_vec3_soa_wrapper(&ferm_shiftmulti_acc[r],fermionname_shift);
                }
                printf("MPI%02d: End of data region!\n", devinfo.myrank);

                double dt_cgm = (double)(t1.tv_sec - t0.tv_sec) + 
                    ((double)(t1.tv_usec - t0.tv_usec)/1.0e6);
                printf("Time for 1 step of multishift inversion   : %e\n",
                        dt_cgm/multishift_invert_iterations);
                    }
#ifndef __GNUC__
    shutdown_acc_device(my_device_type);
#endif
    printf("MPI%02d: Before MPI_Finalize()...\n",devinfo.myrank);
#ifdef MULTIDEVICE
    MPI_Finalize();
#endif 

    mem_free();

    printf("MPI%02d: Test completed.\n",devinfo.myrank);

    return 0; 


}
