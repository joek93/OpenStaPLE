//Funzioni definite qui dentro:
/*
- per il prodotto scalare
    + scal_prod_loc_2double(in1,in2,idx,d_re,d_im)         --> compute the scalar product locally
    + scal_prod_global(in1,in2,result)                     --> compute the global scalar product
- per l'inverter (e per altre cose)
    + combine_in1xm2_minus_in2_minus_in3(in1,in2,in3,out)  --> compute out = in1*m2-in2-in3
    + combine_in1xm2_minus_in2(in1,in2,out)                --> compute out = in1*m2-in2
    + combine_in1_minus_in2(in1,in2,out)                   --> compute out = in1-in2
    + assign_in_to_out(in,out)                             --> set     out = in
    + combine_in1xfactor_plus_in2(in1,fattore,in2,out)     --> compute out = in1*fattore+in2
 */


#ifndef FERMIONIC_UTILITIES_C_
#define FERMIONIC_UTILITIES_C_


static inline double scal_prod_loc_1double(  __restrict vec3_soa * const in_vect1,
					     __restrict vec3_soa * const in_vect2,
					     const int idx_vect
                                          ) {

  double sum  = creal(in_vect1->c0[idx_vect]) * creal(in_vect2->c0[idx_vect]) + cimag(in_vect1->c0[idx_vect]) * cimag(in_vect2->c0[idx_vect]);
         sum += creal(in_vect1->c1[idx_vect]) * creal(in_vect2->c1[idx_vect]) + cimag(in_vect1->c1[idx_vect]) * cimag(in_vect2->c1[idx_vect]);
         sum += creal(in_vect1->c2[idx_vect]) * creal(in_vect2->c2[idx_vect]) + cimag(in_vect1->c2[idx_vect]) * cimag(in_vect2->c2[idx_vect]);
  return sum;
}



static inline double l2norm2_loc( __restrict vec3_soa * const in_vect1,
				  const int idx_vect
				  ) {
  double sum = creal(in_vect1->c0[idx_vect])*creal(in_vect1->c0[idx_vect])+cimag(in_vect1->c0[idx_vect])*cimag(in_vect1->c0[idx_vect]);
  sum += creal(in_vect1->c1[idx_vect])*creal(in_vect1->c1[idx_vect])+cimag(in_vect1->c1[idx_vect])*cimag(in_vect1->c1[idx_vect]);
  sum += creal(in_vect1->c2[idx_vect])*creal(in_vect1->c2[idx_vect])+cimag(in_vect1->c2[idx_vect])*cimag(in_vect1->c2[idx_vect]);
  return sum;
}



d_complex scal_prod_global(  const __restrict vec3_soa * const in_vect1,
			     const __restrict vec3_soa * const in_vect2
			     ){
  int t;

  double resR = 0.0;
  double resI = 0.0;

#pragma acc kernels present(in_vect1) present(in_vect2)
#pragma acc loop reduction(+:resR) reduction(+:resI)
  for(t=0; t<sizeh; t++) {
  d_complex color_sum  =  conj(in_vect1->c0[t]) *  in_vect2->c0[t] ;
  color_sum +=  conj(in_vect1->c1[t]) *  in_vect2->c1[t] ;
  color_sum +=  conj(in_vect1->c2[t]) *  in_vect2->c2[t] ;


  resR+=creal(color_sum);
  resI+=cimag(color_sum);

  }
  return resR+resI*I;
}


double real_scal_prod_global(  __restrict vec3_soa * const in_vect1,
			       __restrict vec3_soa * const in_vect2
			       ){
  int t;
  double res_R_p;
  double resR = 0.0;

#pragma acc kernels present(in_vect1) present(in_vect2)
#pragma acc loop reduction(+:resR)
  for(t=0; t<sizeh; t++) {
    res_R_p=scal_prod_loc_1double(in_vect1,in_vect2,t);
    resR+=res_R_p;
  }
  return resR;
}

double l2norm2_global( __restrict  vec3_soa * const in_vect1 // constant --> is not updated
		       ){
  int t;
  double res_R_p;
  double resR = 0.0;

#pragma acc kernels present(in_vect1)
#pragma acc loop reduction(+:resR)
  for(t=0; t<sizeh; t++) {
    res_R_p=l2norm2_loc(in_vect1,t);
    resR+=res_R_p;
  }
  return resR;
}



void combine_in1xfactor_plus_in2(  __restrict vec3_soa * const in_vect1,
				   const double factor,
                                   __restrict vec3_soa * const in_vect2,
                                   __restrict vec3_soa * const out){

#pragma acc kernels present(in_vect1) present(in_vect2) present(out)
#pragma acc loop independent 
  for(int ih=0; ih<sizeh; ih++) {
    out->c0[ih]=(in_vect1->c0[ih]*factor)+in_vect2->c0[ih];
    out->c1[ih]=(in_vect1->c1[ih]*factor)+in_vect2->c1[ih];
    out->c2[ih]=(in_vect1->c2[ih]*factor)+in_vect2->c2[ih];
  }
}


void multiply_fermion_x_doublefactor(  __restrict vec3_soa * const in1,
                                       const double factor
				       ){

#pragma acc kernels present(in1)
#pragma acc loop independent
  for(int ih=0; ih<sizeh; ih++) {
    in1->c0[ih] = factor * (in1->c0[ih]);
    in1->c1[ih] = factor * (in1->c1[ih]);
    in1->c2[ih] = factor * (in1->c2[ih]);
  }
}

void combine_add_factor_x_in2_to_in1( __restrict vec3_soa * const in1,__restrict vec3_soa * const in2, double factor){
#pragma acc kernels present(in1) present(in2)
#pragma acc loop independent
  for(int ih=0; ih<sizeh; ih++) {
    in1->c0[ih] += (factor) *  (in2->c0[ih]);
    in1->c1[ih] += (factor) *  (in2->c1[ih]);
    in1->c2[ih] += (factor) *  (in2->c2[ih]);
  }
}



/*
void combine_in1xm2_minus_in2_minus_in3(   __restrict vec3_soa * const in_vect1,
                                           __restrict vec3_soa * const in_vect2,
                                           __restrict vec3_soa * const in_vect3,
					   __restrict vec3_soa * const out){ // l'ultimo argomento e' l'unico che viene modificato
#pragma acc kernels present(in_vect1) present(in_vect2) present(in_vect3) present(out)
#pragma acc loop independent 
  for(int ih=0; ih<sizeh; ih++) {
    out->c0[ih]=(in_vect1->c0[ih]*mass2)-in_vect2->c0[ih]-in_vect3->c0[ih];
    out->c1[ih]=(in_vect1->c1[ih]*mass2)-in_vect2->c1[ih]-in_vect3->c1[ih];
    out->c2[ih]=(in_vect1->c2[ih]*mass2)-in_vect2->c2[ih]-in_vect3->c2[ih];
  }
}
*/

void combine_in1xferm_mass2_minus_in2_minus_in3(   __restrict vec3_soa * const in_vect1,
						   double ferm_mass,
						   __restrict vec3_soa * const in_vect2,
						   __restrict vec3_soa * const in_vect3,
						   __restrict vec3_soa * const out){ // l'ultimo argomento e' l'unico che viene modificato
#pragma acc kernels present(in_vect1) present(in_vect2) present(in_vect3) present(out)
#pragma acc loop independent 
  for(int ih=0; ih<sizeh; ih++) {
    out->c0[ih]=(in_vect1->c0[ih]*ferm_mass)-in_vect2->c0[ih]-in_vect3->c0[ih];
    out->c1[ih]=(in_vect1->c1[ih]*ferm_mass)-in_vect2->c1[ih]-in_vect3->c1[ih];
    out->c2[ih]=(in_vect1->c2[ih]*ferm_mass)-in_vect2->c2[ih]-in_vect3->c2[ih];
  }
}

/*
void combine_before_loop(  const __restrict vec3_soa * const vect_in,
			   const __restrict vec3_soa * const vect_out,
			   __restrict vec3_soa * const vect_r,
			   __restrict vec3_soa * const vect_s,
			   __restrict vec3_soa * const vect_p){
#pragma acc kernels present(vect_in) present(vect_out) present(vect_r) present(vect_s) present(vect_p)
#pragma acc loop independent 
  for(int ih=0; ih<sizeh; ih++) {
    //s=m^2*out-s
    vect_s->c0[ih] = (vect_out->c0[ih]*mass2)-vect_s->c0[ih];
    vect_s->c1[ih] = (vect_out->c1[ih]*mass2)-vect_s->c1[ih];
    vect_s->c2[ih] = (vect_out->c2[ih]*mass2)-vect_s->c2[ih];
    //r=in-s
    vect_r->c0[ih] =  vect_in->c0[ih]-vect_s->c0[ih];
    vect_r->c1[ih] =  vect_in->c1[ih]-vect_s->c1[ih];
    vect_r->c2[ih] =  vect_in->c2[ih]-vect_s->c2[ih];
    //p=r
    vect_p->c0[ih] =  vect_r->c0[ih];
    vect_p->c1[ih] =  vect_r->c1[ih];
    vect_p->c2[ih] =  vect_r->c2[ih];
  }
}
*/

void combine_inside_loop( __restrict vec3_soa * const vect_out,
			  __restrict vec3_soa * const vect_r,
			  const __restrict vec3_soa * const vect_s,
			  const __restrict vec3_soa * const vect_p,
			  const double omega){
#pragma acc kernels present(vect_out) present(vect_r) present(vect_s) present(vect_p)
#pragma acc loop independent 
  for(int ih=0; ih<sizeh; ih++) {
    //out+=omega*p
    vect_out->c0[ih] += (vect_p->c0[ih]*omega);
    vect_out->c1[ih] += (vect_p->c1[ih]*omega);
    vect_out->c2[ih] += (vect_p->c2[ih]*omega);
    //r-=omega*s
    vect_r->c0[ih]   -= (vect_s->c0[ih]*omega);
    vect_r->c1[ih]   -= (vect_s->c1[ih]*omega);
    vect_r->c2[ih]   -= (vect_s->c2[ih]*omega);
  }
}
/*
void combine_in1xm2_minus_in2(__restrict vec3_soa * const in_vect1,
                              __restrict vec3_soa * const in_vect2){
#pragma acc kernels present(in_vect1) present(in_vect2)
#pragma acc loop independent
  for(int ih=0; ih<sizeh; ih++) {
    in_vect2->c0[ih]=(in_vect1->c0[ih]*mass2)-in_vect2->c0[ih];
    in_vect2->c1[ih]=(in_vect1->c1[ih]*mass2)-in_vect2->c1[ih];
    in_vect2->c2[ih]=(in_vect1->c2[ih]*mass2)-in_vect2->c2[ih];
  }
}
*/

void combine_in1xferm_mass_minus_in2(__restrict vec3_soa * const in_vect1,double ferm_mass2, __restrict vec3_soa * const in_vect2){
#pragma acc kernels present(in_vect1) present(in_vect2)
#pragma acc loop independent
  for(int ih=0; ih<sizeh; ih++) {
    in_vect2->c0[ih]=(in_vect1->c0[ih]*ferm_mass2)-in_vect2->c0[ih];
    in_vect2->c1[ih]=(in_vect1->c1[ih]*ferm_mass2)-in_vect2->c1[ih];
    in_vect2->c2[ih]=(in_vect1->c2[ih]*ferm_mass2)-in_vect2->c2[ih];
  }
}

void combine_in1_minus_in2( __restrict vec3_soa * const in_vect1,
                            __restrict vec3_soa * const in_vect2,
                            __restrict vec3_soa * const out){
#pragma acc kernels present(in_vect1) present(in_vect2) present(out)
#pragma acc loop independent 
  for(int ih=0; ih<sizeh; ih++) {
    out->c0[ih]=(in_vect1->c0[ih])-in_vect2->c0[ih];
    out->c1[ih]=(in_vect1->c1[ih])-in_vect2->c1[ih];
    out->c2[ih]=(in_vect1->c2[ih])-in_vect2->c2[ih];
  }
}

void assign_in_to_out(  __restrict vec3_soa * const in_vect1,
                        __restrict vec3_soa * const out){
  SETINUSE(out);
#pragma acc kernels present(in_vect1)  present(out)
#pragma acc loop independent 
  for(int ih=0; ih<sizeh; ih++) {
    out->c0[ih]=(in_vect1->c0[ih]);
    out->c1[ih]=(in_vect1->c1[ih]);
    out->c2[ih]=(in_vect1->c2[ih]);
  }

}



// Altre funzioni aggiunte per l'algebra lineare multishift "versatilizzato"
void set_vec3_soa_to_zero( __restrict vec3_soa* const fermion){
  //  printf(" puntatore incriminato (%p) \n",fermion);
  SETINUSE((fermion));
#pragma acc kernels present(fermion)
#pragma acc loop independent 
   // printf("Setting to zero memory from %p to %p ", fermion->c0 , &(fermion->c0[sizeh-1])+1);
   // printf(", %p to %p  ", fermion->c1 , &(fermion->c1[sizeh-1])+1);
   // printf(", %p to %p\n", fermion->c2 , &(fermion->c2[sizeh-1])+1);
    for(int i= 0; i < sizeh ; i++){
        fermion->c0[i]=0;
        fermion->c1[i]=0;
        fermion->c2[i]=0;
    }
}
inline void multiple_combine_in1_minus_in2x_factor_back_into_in1( __restrict vec3_soa * const out, __restrict vec3_soa * const in, const int maxiter, __restrict const int * const flag, __restrict const double * const omegas) {
  int ia, ih;
  
#pragma acc kernels present(in) present(out) copyin(omegas[0:maxiter]) copyin(flag[0:maxiter])
  {
#pragma acc cache(omegas[0:maxiter])
#pragma acc cache(flag[0:maxiter])
#pragma acc loop independent gang
    for (ia=0; ia<maxiter; ia++) {
#pragma acc loop independent gang vector(512)
      for (ih=0; ih<sizeh; ih++) {
	if (flag[ia] == 1) {
	  double factor = omegas[ia];
	  out[ia].c0[ih] -= (factor)*(in[ia].c0[ih]);
	  out[ia].c1[ih] -= (factor)*(in[ia].c1[ih]);
	  out[ia].c2[ih] -= (factor)*(in[ia].c2[ih]);
	} //if
      } //ih
    } //ia
  } //acc kernels
}
void multiple1_combine_in1_x_fact1_plus_in2_x_fact2_back_into_in1( __restrict vec3_soa * const in1, int maxiter, __restrict const int * const flag, __restrict const double * const gammas, __restrict vec3_soa * const in2, __restrict const double * const zeta_iii ) {

  int ia, ih;

  #pragma acc kernels present(in1) present(in2) copyin(gammas[0:maxiter]) copyin(flag[0:maxiter]) copyin(zeta_iii[0:maxiter])
  {

  #pragma acc cache(gammas[0:maxiter]) 
  #pragma acc cache(zeta_iii[0:maxiter])
  #pragma acc cache(flag[0:maxiter]) 

  #pragma acc loop independent gang
  for (ia=0; ia<maxiter; ia++) {
    #pragma acc loop independent gang vector(512)
    for(ih=0; ih<sizeh; ih++) {

      if (flag[ia] == 1) {

        double fact1 = gammas[ia];
        double fact2 = zeta_iii[ia];

        in1[ia].c0[ih] = fact1 * (in1[ia].c0[ih]) + fact2 * (in2->c0[ih]);
        in1[ia].c1[ih] = fact1 * (in1[ia].c1[ih]) + fact2 * (in2->c1[ih]);
        in1[ia].c2[ih] = fact1 * (in1[ia].c2[ih]) + fact2 * (in2->c2[ih]);

      } //if flag

    } //ih

  } //ia

  } //acc data

}
void combine_in1_x_fact1_minus_in2_back_into_in2( __restrict vec3_soa * const in1, double fact1, __restrict vec3_soa * const in2 ) {

  int ih;


  #pragma acc kernels present(in1) present(in2) 
  {


#pragma acc loop independent gang vector(512)
    for(ih=0; ih<sizeh; ih++) {


        in2->c0[ih] = fact1 * (in1->c0[ih]) - (in2->c0[ih]);
        in2->c1[ih] = fact1 * (in1->c1[ih]) - (in2->c1[ih]);
        in2->c2[ih] = fact1 * (in1->c2[ih]) - (in2->c2[ih]);

    } //ih
  } //acc data

}


void combine_in1_minus_in2_allxfact( __restrict vec3_soa * const in1, __restrict vec3_soa * const in2, double fact, __restrict vec3_soa * const out ) {

  int ih;


#pragma acc kernels present(in1) present(in2) present(out) 
  {


#pragma acc loop independent gang vector(512)
    for(ih=0; ih<sizeh; ih++) {


        out->c0[ih] = fact * (in1->c0[ih] - in2->c0[ih]);
        out->c1[ih] = fact * (in1->c1[ih] - in2->c1[ih]);
        out->c2[ih] = fact * (in1->c2[ih] - in2->c2[ih]);

    } //ih
  } //acc data

}






#endif


