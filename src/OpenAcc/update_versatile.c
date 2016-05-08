// se metro==1 allora fa il test di metropolis
// se metro==0 allora non fa il test di metropolis --> termalizzazione
#ifndef UPDATE_VERSATILE_C_
#define UPDATE_VERSATILE_C_

#include "../DbgTools/dbgtools.h" // useful only for debug
#include "../Include/common_defines.h"
#include "../Include/debug.h"
#include "../Mpi/multidev.h"
#include "../Rand/random.h"
#include "./action.h"
#include "./alloc_vars.h"
#include "./fermionic_utilities.h"
#include "./find_min_max.h"
#include "./inverter_multishift_full.h"
#include "./io.h"
#include "./md_integrator.h"
#include "./random_assignement.h"
#include "./rettangoli.h"
#include "./stouting.h"
#include "./su3_measurements.h"
#include "./su3_utilities.h"
#include "./update_versatile.h"

#ifdef __GNUC__
#include "sys/time.h"
#endif

#ifdef MULTIDEVICE
#include <mpi.h>
#endif 

action_param act_params;

int UPDATE_SOLOACC_UNOSTEP_VERSATILE(su3_soa *tconf_acc,
        double res_metro, double res_md, int id_iter,int acc,int metro){

#ifdef STOUT_FERMIONS        
    su3_soa *tstout_conf_acc_arr = gstout_conf_acc_arr;
#endif
#ifdef NORANDOM
    printf("MIP%02d: WELCOME! NORANDOM MODE. (UPDATE_SOLOACC_UNOSTEP_VERSATILE())\n",
            devinfo.myrank);
#endif



    printf("MPI%02d: UPDATE_SOLOACC_UNOSTEP_VERSATILE_TLSM_STDFERM: starting... \n",
            devinfo.myrank);
    // DEFINIZIONE DI TUTTI I dt NECESSARI PER L'INTEGRATORE OMELYAN
    int iterazioni = id_iter+1;
    double dt_tot;
    double dt_pretrans_to_preker;
    double dt_preker_to_postker;
    double dt_postker_to_posttrans;

    if(debug_settings.save_diagnostics == 1){
        FILE *foutfile = 
            fopen(debug_settings.diagnostics_filename,"at");
        fprintf(foutfile,"\nIteration %d \t",id_iter);
        fclose(foutfile);
    }



    int mu;
    double **minmaxeig; 
    minmaxeig = (double**)malloc(NDiffFlavs*sizeof(double*));
    for(int iflav = 0 ; iflav < NDiffFlavs ; iflav++)
        minmaxeig[iflav] = (double*) malloc(2*sizeof(double));

    double p1;
    double p2;
    int accettata;
    double delta_S;
    double action_in,action_fin,action_mom_in,action_mom_fin,action_ferm_in,action_ferm_fin;

    struct timeval t0, t1,t2,t3;
    gettimeofday ( &t0, NULL );

    if(metro==1){
        // store old conf   set_su3_soa_to_su3_soa(arg1,arg2) ===>   arg2=arg1;
        set_su3_soa_to_su3_soa(tconf_acc,conf_acc_bkp);
        if(verbosity_lv > 1) printf("Backup copy of the initial gauge conf : OK \n");

    }


    //#pragma acc data copyin(delta[0:7]) // should not be needed?
    {

        gettimeofday ( &t1, NULL );

        // ESTRAZIONI RANDOM
        if(debug_settings.do_norandom_test){ // NORANDOM
            printf("NORANDOM mode, loading momenta from memory.\n");
            if(read_thmat_soa(momenta,"momenta_norndtest")){
                printf("GENERATING MOMENTA FILE FOR YOUR CONVENIENCE, RE-RUN THIS TEST\n");
                generate_Momenta_gauss(momenta);
                print_thmat_soa(momenta,"momenta_norndtest");
            }
        }
        else generate_Momenta_gauss(momenta); // NORMAL, RANDOM

        printf("MPI%02d - Momenta generated/read : OK \n", devinfo.myrank);

        //    read_thmat_soa(momenta,"momenta");
#pragma acc update device(momenta[0:8])
        if(debug_settings.do_reversibility_test)
            copy_momenta_into_old(momenta,momenta_backup);



        for(int iflav = 0 ; iflav < NDiffFlavs ; iflav++){
            for(int ips = 0 ; ips < fermions_parameters[iflav].number_of_ps ; ips++){
                int ps_index = fermions_parameters[iflav].index_of_the_first_ps + ips;

                if(debug_settings.do_norandom_test){ // NORANDOM
                    char psferm_filename[20];

                    char ps_index_str[5];
                    sprintf(ps_index_str,"%d",ps_index);
                    strcpy(psferm_filename,"fermion_norndtest");
                    strcat(psferm_filename,ps_index_str);
                    if(read_vec3_soa(&ferm_phi_acc[ps_index],psferm_filename)){
                        printf("GENERATING FERMION FILE FOR YOUR CONVENIENCE, RE-RUN THIS TEST\n");
                        generate_vec3_soa_gauss(&ferm_phi_acc[ps_index]);
                        print_vec3_soa(&ferm_phi_acc[ps_index],psferm_filename);

                    }
                }
                else{    // NORMAL, RANDOM
                    if(verbosity_lv > 3 )printf("Ferm generation (flav=%d,ps=%d,ps_index=%d) : OK \n",iflav,ips,ps_index);
                    generate_vec3_soa_gauss(&ferm_phi_acc[ps_index]);
                }
            }
        }// end for iflav
#pragma acc update device(ferm_phi_acc[0:NPS_tot])

#ifdef STOUT_FERMIONS 
        // DILATION USING STOUTED DIRAC OPERATOR
        // STOUTING...(ALREADY ON DEVICE)
        if(act_params.stout_steps > 0){
            stout_wrapper(tconf_acc,tstout_conf_acc_arr);
            gconf_as_fermionmatrix = 
                &(tstout_conf_acc_arr[8*(act_params.stout_steps-1)]);
        }
        else gconf_as_fermionmatrix = tconf_acc;
#else
        gconf_as_fermionmatrix = tconf_acc;
#endif

        // DILATION OF FIRST_INV RATIONAL APPROXIMATION
        for(int iflav = 0 ; iflav < NDiffFlavs ; iflav++){
            if(verbosity_lv > 2 ) printf("Rat approx rescale (flav=%d)\n",iflav);

            if(debug_settings.do_norandom_test){ // NORANDOM
                if(read_vec3_soa(kloc_p,"kloc_p_norndtest")){
                    generate_vec3_soa_gauss(kloc_p);
                    print_vec3_soa(kloc_p,"kloc_p_norndtest");
                    printf("GENERATED kloc_p_norndtest FOR NORANDOM TEST, RE-RUN THIS TEST\n");
                }
                if(read_vec3_soa(kloc_s,"kloc_s_norndtest")){
                    generate_vec3_soa_gauss(kloc_s);
                    print_vec3_soa(kloc_s,"kloc_s_norndtest");
                    printf("GENERATED kloc_s_norndtest FOR NORANDOM TEST, RE-RUN THIS TEST\n");
                }
            }
            else{   // NORMAL, RANDOM
                // generate gauss-randomly the fermion kloc_p that will be used in the computation of the max eigenvalue
                generate_vec3_soa_gauss(kloc_p);
                generate_vec3_soa_gauss(kloc_s);
            }
            // update the fermion kloc_p copying it from the host to the device
#pragma acc update device(kloc_p[0:1])
#pragma acc update device(kloc_s[0:1])
            // USING STOUTED GAUGE MATRIX
            //printf("    before min and max eig comp : OK \n");
            find_min_max_eigenvalue_soloopenacc(gconf_as_fermionmatrix,&(fermions_parameters[iflav]),kloc_r,kloc_h,kloc_p,kloc_s,minmaxeig[iflav]);
            if(verbosity_lv > 3 ) printf("    find min and max eig : OK \n");
            RationalApprox *approx_fi = &(fermions_parameters[iflav].approx_fi);
            RationalApprox *approx_fi_mother = &(fermions_parameters[iflav].approx_fi_mother);
            rescale_rational_approximation(approx_fi_mother,approx_fi,minmaxeig[iflav]);
            if(verbosity_lv > 4 ) printf("    rat approx rescaled : OK \n\n");

#pragma acc update device(approx_fi[0:1])
        }//end for iflav



        if(metro==1){
            /////////////// INITIAL ACTION COMPUTATION ////////////////////////////////////////////
            if(GAUGE_ACTION == 0)// Standard gauge action 
                action_in = BETA_BY_THREE*calc_plaquette_soloopenacc(tconf_acc,aux_conf_acc,local_sums);
            if(GAUGE_ACTION == 1){ //Tlsym gauge action
                action_in = C_ZERO * BETA_BY_THREE * calc_plaquette_soloopenacc(tconf_acc,aux_conf_acc,local_sums);
                action_in += C_ONE * BETA_BY_THREE * calc_rettangolo_soloopenacc(tconf_acc,aux_conf_acc,local_sums);
            }
            action_mom_in = 0.0;
            for(mu =0;mu<8;mu++)  action_mom_in += calc_momenta_action(momenta,d_local_sums,mu);
            action_ferm_in=0;
            for(int iflav = 0 ; iflav < NDiffFlavs ; iflav++){
                for(int ips = 0 ; ips < fermions_parameters[iflav].number_of_ps ; ips++){

                    int ps_index = fermions_parameters[iflav].index_of_the_first_ps + ips;
                    action_ferm_in += real_scal_prod_global(&ferm_phi_acc[ps_index],&ferm_phi_acc[ps_index]);
                }
            }// end for iflav
            ///////////////////////////////////////////////////////////////////////////////////////
            printf("MPI%02d - Initial Action Computed : OK \n", devinfo.myrank);
        }

        // FIRST INV APPROX CALC --> calculation of CHI fermion


        for(int iflav = 0 ; iflav < NDiffFlavs ; iflav++){
            for(int ips = 0 ; ips < fermions_parameters[iflav].number_of_ps ; ips++){
                if(0==devinfo.myrank)
                    printf("Calculation of chi for fermion %d, copy %d\n", iflav,ips);

                int ps_index = fermions_parameters[iflav].index_of_the_first_ps + ips;
                // USING STOUTED GAUGE MATRIX
                multishift_invert(gconf_as_fermionmatrix, &fermions_parameters[iflav], &(fermions_parameters[iflav].approx_fi), ferm_shiftmulti_acc, &(ferm_phi_acc[ps_index]), res_metro, kloc_r, kloc_h, kloc_s, kloc_p, k_p_shiftferm);
                if(0==devinfo.myrank) printf("Inversion performed for fermion %d, copy %d\n", iflav,ips);
                recombine_shifted_vec3_to_vec3(ferm_shiftmulti_acc, &(ferm_phi_acc[ps_index]), &(ferm_chi_acc[ps_index]),&(fermions_parameters[iflav].approx_fi));
                if(0==devinfo.myrank) printf("Calculated chi for fermion %d, copy %d\n", iflav,ips);


            }
        }// end for iflav
        if(verbosity_lv > 3) printf("MPI%02d: Computed the fermion CHI : OK \n", devinfo.myrank);

        // DILATION OF MOLECULAR DYNAMICS RATIONAL APPROXIMATION
        for(int iflav = 0 ; iflav < NDiffFlavs ; iflav++){
            // recovering eigenvalues from approx_fi
            RationalApprox *approx_md = &(fermions_parameters[iflav].approx_md);
            RationalApprox *approx_md_mother = &(fermions_parameters[iflav].approx_md_mother);
            rescale_rational_approximation(approx_md_mother,approx_md,minmaxeig[iflav]);
            printf("MPI%02d: Rescaled Rational Approximation for flavour %d\n", devinfo.myrank, iflav);
#pragma acc update device(approx_md[0:1])
        }//end for iflav

        // MOLECULAR DYNAMICS 
        multistep_2MN_SOLOOPENACC(ipdot_acc,tconf_acc,
#ifdef STOUT_FERMIONS
                tstout_conf_acc_arr,
#endif
                auxbis_conf_acc, // global
                aux_conf_acc,fermions_parameters,NDiffFlavs,
                ferm_chi_acc,ferm_shiftmulti_acc,kloc_r,kloc_h,kloc_s,kloc_p,
                k_p_shiftferm,momenta,local_sums,res_md);

        if(debug_settings.do_reversibility_test){

            printf("MPI%02d: PERFORMING REVERSIBILITY TEST.\n", devinfo.myrank);
            printf("MPI%02d: Inverting momenta.\n", devinfo.myrank);

            invert_momenta(momenta);

            multistep_2MN_SOLOOPENACC(ipdot_acc,tconf_acc,
#ifdef STOUT_FERMIONS
                    tstout_conf_acc_arr,
#endif
                    auxbis_conf_acc, // globale
                    aux_conf_acc,fermions_parameters,NDiffFlavs,
                    ferm_chi_acc,ferm_shiftmulti_acc,kloc_r,kloc_h,kloc_s,kloc_p,
                    k_p_shiftferm,momenta,local_sums,res_md);

#pragma acc update device(conf_acc_bkp[0:8])
            double conf_error =  calc_diff_su3_soa_norm(tconf_acc,conf_acc_bkp);
            // Note: sum instead of difference must be calculated because momenta 
            // were inverted.
            double momenta_error = calc_sum_momenta_norm(momenta,momenta_backup);

            printf("MPI%02d: Reversibility test: Conf_error: %e , momenta_error %e \n",
                    devinfo.myrank, conf_error, momenta_error);


            printf("MPI%02d: Since a reversibility test is being performed, no need to  go on. Exiting now.\n", 
                    devinfo.myrank);

            save_conf_wrapper(tconf_acc,"conf_REVTEST",0,0); // conf_id_iter =0, not using ILDG
            save_conf_wrapper(conf_acc_bkp,"conf_REVTEST_bkp",0,0);


#ifdef MULTIDEVICE
            MPI_Finalize();
#endif
            mem_free();
            exit(0);

        }



        if(verbosity_lv > 1) printf("MPI%02d - MOLECULAR DYNAMICS COMPLETED \n", 
                devinfo.myrank);


#ifdef STOUT_FERMIONS
        // STOUTING...(ALREADY ON DEVICE)
        if(act_params.stout_steps > 0){
            stout_wrapper(tconf_acc,tstout_conf_acc_arr);
            gconf_as_fermionmatrix = 
                &(tstout_conf_acc_arr[8*(act_params.stout_steps-1)]);
        }
        else gconf_as_fermionmatrix = tconf_acc;
#else
        gconf_as_fermionmatrix = tconf_acc;
#endif

        if(metro==1){
            // DILATION OF LAST_INV RATIONAL APPROX
            for(int iflav = 0 ; iflav < NDiffFlavs ; iflav++){
                // generate gauss-randomly the fermion kloc_p that will be used in the computation of the max eigenvalue
                if(debug_settings.do_norandom_test){ // NORANDOM
                    if(read_vec3_soa(kloc_p,"kloc_p_norndtest")){
                        generate_vec3_soa_gauss(kloc_p);
                        print_vec3_soa(kloc_p,"kloc_p_norndtest");
                        printf("GENERATED kloc_p_norndtest FOR NORANDOM TEST, RE-RUN THIS TEST\n");
                    }
                    if(read_vec3_soa(kloc_s,"kloc_s_norndtest")){
                        generate_vec3_soa_gauss(kloc_s);
                        print_vec3_soa(kloc_s,"kloc_s_norndtest");
                        printf("GENERATED kloc_s_norndtest FOR NORANDOM TEST, RE-RUN THIS TEST\n");
                    }
                }
                else{    // NORMAL,/ RANDOM
                    generate_vec3_soa_gauss(kloc_p);
                    generate_vec3_soa_gauss(kloc_s);
                }
                // update the fermion kloc_p copying it from the host to the device
#pragma acc update device(kloc_p[0:1])
#pragma acc update device(kloc_s[0:1])
                // USING STOUTED CONF
                find_min_max_eigenvalue_soloopenacc(gconf_as_fermionmatrix,&(fermions_parameters[iflav]),kloc_r,kloc_h,kloc_p,kloc_s,minmaxeig[iflav]);
                //#pragma acc update device(minmaxeig[0:2])
                RationalApprox *approx_li = &(fermions_parameters[iflav].approx_li);
                RationalApprox *approx_li_mother = &(fermions_parameters[iflav].approx_li_mother);
                rescale_rational_approximation(approx_li_mother,approx_li,minmaxeig[iflav]);
#pragma acc update device(approx_li[0:1])
            }

            // LAST INV APPROX CALC 
            for(int iflav = 0 ; iflav < NDiffFlavs ; iflav++){
                for(int ips = 0 ; ips < fermions_parameters[iflav].number_of_ps ; ips++){
                    int ps_index = fermions_parameters[iflav].index_of_the_first_ps + ips;
                    // USING STOUTED CONF
                    multishift_invert(gconf_as_fermionmatrix, &fermions_parameters[iflav], 
                            &(fermions_parameters[iflav].approx_li),
                            ferm_shiftmulti_acc, &(ferm_chi_acc[ps_index]), res_metro, 
                            kloc_r, kloc_h, kloc_s, kloc_p, k_p_shiftferm);
                    recombine_shifted_vec3_to_vec3(ferm_shiftmulti_acc, &(ferm_chi_acc[ps_index]), &(ferm_phi_acc[ps_index]),&(fermions_parameters[iflav].approx_li));
                }
            }
            printf(" MPI%02d - Final Action Computed : OK \n", devinfo.myrank);

            ///////////////   FINAL ACTION COMPUTATION  ////////////////////////////////////////////
            if(GAUGE_ACTION == 0) // Standard gauge action
                action_fin = BETA_BY_THREE * calc_plaquette_soloopenacc(tconf_acc,aux_conf_acc,local_sums);
            if(GAUGE_ACTION == 1){ // Tlsym gauge action
                action_fin = C_ZERO * BETA_BY_THREE * calc_plaquette_soloopenacc(tconf_acc,aux_conf_acc,local_sums);
                action_fin += C_ONE * BETA_BY_THREE * calc_rettangolo_soloopenacc(tconf_acc,aux_conf_acc,local_sums);
            }

            action_mom_fin = 0.0;
            for(mu =0;mu<8;mu++)    action_mom_fin += calc_momenta_action(momenta,d_local_sums,mu);

            action_ferm_fin=0;
            for(int iflav = 0 ; iflav < NDiffFlavs ; iflav++){
                for(int ips = 0 ; ips < fermions_parameters[iflav].number_of_ps ; ips++){
                    int ps_index = fermions_parameters[iflav].index_of_the_first_ps + ips;
                    action_ferm_fin += real_scal_prod_global(&ferm_chi_acc[ps_index],&ferm_phi_acc[ps_index]);
                }
            } // end for iflav


            ////////////////////////////////////////////////////////////////////////////////////////

            // delta_S = action_new - action_old
            delta_S  = - (-action_in+action_mom_in+action_ferm_in) + (-action_fin+action_mom_fin+action_ferm_fin);
            if(verbosity_lv > 2 && 0 == devinfo.myrank ){
                printf("MPI%02d-iterazione %i:  Gauge_ACTION  (in and out) = %.18lf , %.18lf\n",
                        devinfo.myrank,iterazioni,-action_in,-action_fin);
                printf("MPI%02d-iterazione %i:  Momen_ACTION  (in and out) = %.18lf , %.18lf\n",
                        devinfo.myrank,iterazioni,action_mom_in,action_mom_fin);
                printf("MPI%02d-iterazione %i:  Fermi_ACTION  (in and out) = %.18lf , %.18lf\n",
                        devinfo.myrank,iterazioni,action_ferm_in,action_ferm_fin);
            }       
            printf("MPI%02d-iterazione %i:  DELTA_ACTION = %.18lf. ",
                    devinfo.myrank,iterazioni,delta_S);


            if(debug_settings.do_norandom_test)  // NORANDOM
                printf("Always accept in NORANDOM MODE!!!\n");

            if(delta_S<0){
                accettata=1;
            }
            else
            {
                p1=exp(-delta_S);
                if(debug_settings.do_norandom_test) p2=0; // NORANDOM
                else{   // NORMAL, RANDOM
                    if(0==devinfo.myrank)p2=casuale();
#ifdef MULTIDEVICE
                    MPI_Bcast((void*) &p2,1,MPI_DOUBLE,0,MPI_COMM_WORLD);
                    printf("MPI%02d p2 : %f, p1 %f \n",devinfo.myrank, p2,p1);
#endif
                }
                if(p2<p1)
                {
                    accettata=1;
                }
                else
                {
                    accettata=0;
                    // configuration reject
                }
            }
        }
        gettimeofday ( &t2, NULL );
    }// end pragma acc data copyin(delta[0:7])

    gettimeofday ( &t3, NULL );

    if(metro==1){
        if(accettata==1){
            acc++;
            printf("MPI%02d,ACCEPTED   ---> [acc/iter] = [%i/%i] \n",
                    devinfo.myrank,acc,iterazioni);
            // configuration accepted   set_su3_soa_to_su3_soa(arg1,arg2) ===>   arg2=arg1;
            set_su3_soa_to_su3_soa(tconf_acc,conf_acc_bkp);
        }else{
            printf("MPI%02d,REJECTED   ---> [acc/iter] = [%i/%i] \n",
                    devinfo.myrank,acc,iterazioni);
            // configuration rejected   set_su3_soa_to_su3_soa(arg1,arg2) ===>   arg2=arg1;
            set_su3_soa_to_su3_soa(conf_acc_bkp,tconf_acc);
#pragma acc update device(tconf_acc[0:8])
            // sul device aggiorniamo la conf rimettendo quella del passo precedente
        }
    }


    if(metro==0){// accetta sempre in fase di termalizzazione
        acc++;
    }


    dt_tot = (double)(t3.tv_sec - t0.tv_sec) + ((double)(t3.tv_usec - t0.tv_usec)/1.0e6);
    dt_pretrans_to_preker = (double)(t1.tv_sec - t0.tv_sec) + ((double)(t1.tv_usec - t0.tv_usec)/1.0e6);
    dt_preker_to_postker = (double)(t2.tv_sec - t1.tv_sec) + ((double)(t2.tv_usec - t1.tv_usec)/1.0e6);
    dt_postker_to_posttrans = (double)(t3.tv_sec - t2.tv_sec) + ((double)(t3.tv_usec - t2.tv_usec)/1.0e6);

    if(metro==0){
        printf("MPI%02d-   FULL UPDATE COMPUTATION TIME NOMETRO - Tot time : %f sec \n",devinfo.myrank,dt_tot);
        printf("MPI%02d-\t\tPreTrans->Preker  : %f sec  \n",devinfo.myrank,dt_pretrans_to_preker);
        printf("MPI%02d-\t\tPreKer->PostKer   : %f sec  \n",devinfo.myrank,dt_preker_to_postker);
        printf("MPI%02d-\t\tPostKer->PostTrans: %f sec  \n",devinfo.myrank,dt_postker_to_posttrans);
    }
    if(metro==1){
        printf("MPI%02d-   FULL UPDATE COMPUTATION TIME SIMETRO - Tot time : %f sec  \n",devinfo.myrank,dt_tot);
        printf("MPI%02d-\t\tPreTrans->Preker  : %f sec  \n",devinfo.myrank,dt_pretrans_to_preker);
        printf("MPI%02d-\t\tPreKer->PostKer   : %f sec  \n",devinfo.myrank,dt_preker_to_postker);
        printf("MPI%02d-\t\tPostKer->PostTrans: %f sec  \n",devinfo.myrank,dt_postker_to_posttrans);
    }

    if(debug_settings.save_diagnostics == 1){
        FILE *foutfile = 
            fopen(debug_settings.diagnostics_filename,"at");

        if(metro==1){
            fprintf(foutfile,"GAS %.18lf GAF %.18lf \t",-action_in,-action_fin);
            fprintf(foutfile,"MAS %.18lf MAF %.18lf \t",action_mom_in,action_mom_fin);
            fprintf(foutfile,"FAS %.18lf FAF %.18lf \t",action_ferm_in,action_ferm_fin);
            fprintf(foutfile," D %.18lf",delta_S);
        }else{

            fprintf(foutfile," THERM_ITERATION ");

        }

        fclose(foutfile);
    }


    return acc;

}

#endif
