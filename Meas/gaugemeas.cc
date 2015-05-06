// plaquette measurement 
void Conf::calc_plaq(REAL &pls, REAL &plt) const
  {
  #ifdef DEBUG_MODE
  cout << "DEBUG: inside Conf::calc_plaq ..."<<endl;
  #endif

  Su3 aux;
  long int r, index_mu, index_nu;
  int mu, nu;
  double pls_loc, plt_loc;

	  

  for(r=0; r<size; r++)
     {
     d_vector1[r]=0.0;
     d_vector2[r]=0.0;
     for(mu=0; mu<3; mu++) 
        {
        index_mu=mu*size; 
        for(nu=mu+1; nu<4; nu++)
           {
           //            (3)
           //          +<---+
           // nu       |    ^ 
           // ^    (4) V    | (2)
           // |        +--->+
           // |        r (1)
           // +---> mu
 
           index_nu=nu*size;

           aux=  u_save[index_mu + r];          // 1
           aux*= u_save[index_nu + nnp[r][mu]]; // 2
           aux*=~u_save[index_mu + nnp[r][nu]]; // 3
           aux*=~u_save[index_nu + r];

           if(nu==3)
             {
             d_vector1[r]-=(double)aux.retr();  // temporal
             }
           else                                 // MINUS SIGN DUE TO STAGGERED PHASES
             {
             d_vector2[r]-=(double)aux.retr();  // spatial
             }
          
	   }
	}   
     }
  
  global_sum(d_vector1, size);
  global_sum(d_vector2, size);

  plt_loc=d_vector1[0]*inv_size_by_three*one_by_three;
  pls_loc=d_vector2[0]*inv_size_by_three*one_by_three;   // 3 plaquettes for site and 3 colors

  plt=plt_loc;
  pls=pls_loc;

  #ifdef DEBUG_MODE
  cout << "\tterminated Conf::calc_plaq"<<endl;
  #endif
  }

// plaquette measurement 
void Conf::calc_plaq_uwork(REAL &pls, REAL &plt) const
  {
  #ifdef DEBUG_MODE
  cout << "DEBUG: inside Conf::calc_plaq ..."<<endl;
  #endif

  Su3 aux;
  long int r, index_mu, index_nu;
  int mu, nu;
  double pls_loc, plt_loc;

	  

  for(r=0; r<size; r++)
     {
     d_vector1[r]=0.0;
     d_vector2[r]=0.0;
     for(mu=0; mu<3; mu++) 
        {
        index_mu=mu*size; 
        for(nu=mu+1; nu<4; nu++)
           {
           //            (3)
           //          +<---+
           // nu       |    ^ 
           // ^    (4) V    | (2)
           // |        +--->+
           // |        r (1)
           // +---> mu
 
           index_nu=nu*size;

           aux=  u_work[index_mu + r];          // 1
           aux*= u_work[index_nu + nnp[r][mu]]; // 2
           aux*=~u_work[index_mu + nnp[r][nu]]; // 3
           aux*=~u_work[index_nu + r];

           if(nu==3)
             {
             d_vector1[r]-=(double)aux.retr();  // temporal
             }
           else                                 // MINUS SIGN DUE TO STAGGERED PHASES
             {
             d_vector2[r]-=(double)aux.retr();  // spatial
             }
          
	   }
	}   
     }
  
  global_sum(d_vector1, size);
  global_sum(d_vector2, size);

  plt_loc=d_vector1[0]*inv_size_by_three*one_by_three;
  pls_loc=d_vector2[0]*inv_size_by_three*one_by_three;   // 3 plaquettes for site and 3 colors

  plt=plt_loc;
  pls=pls_loc;

  #ifdef DEBUG_MODE
  cout << "\tterminated Conf::calc_plaq"<<endl;
  #endif
  }

/*
// Calculate all the plaquettes 6 planes x 2 component (real and imag)
void Conf::calc_all_plaq(REAL &pxyr,REAL &pxyi,REAL &pxzr,REAL &pxzi,REAL &pxtr,REAL &pxti,REAL &pyzr,REAL &pyzi,REAL &pytr,REAL &pyti,REAL &pztr,REAL &pzti) const
  {
  #ifdef DEBUG_MODE
  cout << "DEBUG: inside Conf::calc_plaq ..."<<endl;
  #endif

  Su3 aux;
  long int r, index_mu, index_nu;
  int mu, nu;
  double pls_loc, plt_loc;
           
           complex<REAL> *auxE;
	   complex<REAL> *auxB;
	   auxE= new complex<REAL>[no_ps];
	   auxB= new complex<REAL>[no_ps];
	   REAL arg;
	   REAL *flux;
	   flux= new REAL[no_ps];
	   complex<REAL> *flux_exp;
	   flux_exp= new complex<REAL>[no_ps];
	   int quark;

	   ofstream out_file;
	   out_file.open(QUOTEME(FLUX_FILE), ios::out);
  
  


  for(r=0; r<size; r++)
     {
       d_vector_xy_r[r]=0.0;
       d_vector_xy_i[r]=0.0;
       d_vector_xz_r[r]=0.0;
       d_vector_xz_i[r]=0.0;
       d_vector_xt_r[r]=0.0;
       d_vector_xt_i[r]=0.0;
       d_vector_yz_r[r]=0.0;
       d_vector_yz_i[r]=0.0;
       d_vector_yt_r[r]=0.0;
       d_vector_yt_i[r]=0.0;
       d_vector_zt_r[r]=0.0;
       d_vector_zt_i[r]=0.0;

     for(mu=0; mu<3; mu++) 
        {
        index_mu=mu*size; 
        for(nu=mu+1; nu<4; nu++)
           {
           //            (3)
           //          +<---+
           // nu       |    ^ 
           // ^    (4) V    | (2)
           // |        +--->+
           // |        r (1)
           // +---> mu
 
           index_nu=nu*size;

           aux=  u_save[index_mu + r];          // 1
           aux*= u_save[index_nu + nnp[r][mu]]; // 2
           aux*=~u_save[index_mu + nnp[r][nu]]; // 3
           aux*=~u_save[index_nu + r];

           if(mu==0)
             {
	       if(nu==1){
		 d_vector_xy_r[r]-=(double)aux.retr(); 
		 d_vector_xy_i[r]-=(double)aux.imtr(); 
	       }
	       if(nu==2){
		 d_vector_xz_r[r]-=(double)aux.retr(); 
		 d_vector_xz_i[r]-=(double)aux.imtr(); 
	       }
	       if(nu==3){
		 d_vector_xt_r[r]-=(double)aux.retr(); 
		 d_vector_xt_i[r]-=(double)aux.imtr(); 
	       }
             }
           if(mu==1)
             {
	       if(nu==2){
		 d_vector_yz_r[r]-=(double)aux.retr(); 
		 d_vector_yz_i[r]-=(double)aux.imtr(); 
	       }
	       if(nu==3){
		 d_vector_yt_r[r]-=(double)aux.retr(); 
		 d_vector_yt_i[r]-=(double)aux.imtr(); 
	       }
	     }
           if(mu==2)
             {
	       if(nu==3){
		 d_vector_zt_r[r]-=(double)aux.retr(); 
		 d_vector_zt_i[r]-=(double)aux.imtr(); 
	       }
	     }
           }
        }
     }
  global_sum(d_vector_xy_r, size);
  global_sum(d_vector_xy_i, size);
  global_sum(d_vector_xz_r, size);
  global_sum(d_vector_xz_i, size);
  global_sum(d_vector_xt_r, size);
  global_sum(d_vector_xt_i, size);
  global_sum(d_vector_yz_r, size);
  global_sum(d_vector_yz_i, size);
  global_sum(d_vector_yt_r, size);
  global_sum(d_vector_yt_i, size);
  global_sum(d_vector_zt_r, size);
  global_sum(d_vector_zt_i, size);

  pxyr=d_vector_xy_r[0]*inv_size_by_three;
  pxyi=d_vector_xy_i[0]*inv_size_by_three;
  pxzr=d_vector_xz_r[0]*inv_size_by_three;
  pxzi=d_vector_xz_i[0]*inv_size_by_three;
  pxtr=d_vector_xt_r[0]*inv_size_by_three;
  pxti=d_vector_xt_i[0]*inv_size_by_three;
  pyzr=d_vector_yz_r[0]*inv_size_by_three;
  pyzi=d_vector_yz_i[0]*inv_size_by_three;
  pytr=d_vector_yt_r[0]*inv_size_by_three;
  pyti=d_vector_yt_i[0]*inv_size_by_three;
  pztr=d_vector_zt_r[0]*inv_size_by_three;
  pzti=d_vector_zt_i[0]*inv_size_by_three;

  #ifdef DEBUG_MODE
  cout << "\tterminated Conf::calc_plaq"<<endl;
  #endif
  }
*/


// polyakov loop measurement
void Conf::calc_poly(REAL &re, REAL &im) const 
  {
  #ifdef DEBUG_MODE
  cout << "DEBUG: inside Conf::calc_poly ..."<<endl;
  #endif

  long int r, r2, t;
  double re_loc, im_loc;
  Su3 aux;

  for(r=0; r<vol3; r++)   // r<vol3 iff t=0
     {
     aux.one();
     r2=r;
     for(t=0; t<nt; t++)
        {
        aux*=u_save[r2+size3];
        r2=nnp[r2][3];
        }

     d_vector1[r]=-(double)aux.retr();    // MINUS SIGN DUE TO ANTIPERIODIC TEMPORAL B.C. FOR STAGGERED PHASES
     d_vector2[r]=-(double)aux.imtr();
     }
  
  global_sum(d_vector1, vol3);
  global_sum(d_vector2, vol3);

  re_loc=d_vector1[0]*inv_vol3*one_by_three;
  im_loc=d_vector2[0]*inv_vol3*one_by_three;   // three colors

  re=re_loc;
  im=im_loc;

  #ifdef DEBUG_MODE
  cout << "\tterminated Conf::calc_poly"<<endl;
  #endif
  }





// a single step of the cooling algorithm                                                                                                                                                                                                                                                
void Conf::cooling(){
  //costruire le nuove u_work che minimizzano localmente l'azione
  int r; //r dovrà andare da 0 a no_links-1                                                                                                                                                                                                                                              
  Su3 aux_staple;
  Su3 aux_link;
  Su3 aux_U_prime;
  Su3 aux_G;
  REAL v1=0.0,v2=0.0;
  aux_U_prime.one();
  int index1,index2;           //index1 ed index2 dovranno andare assumere iterativamente i valori 0, 1 e 2.
  double plaq=0.0;
  double dplaq=0.0;
  //  long int conteggio=0;
  for(int links=0;links<size;links++){
    for(int mu=0;mu<4;mu++){
      r=links+mu*size;
      //     conteggio++;
      //     if((conteggio%1000)==0) cerr << "Link " << conteggio << endl;
      aux_staple=(gauge_conf->get_staple(links,mu));
      aux_link = (gauge_conf->u_work[r]);
      //     cout << "Staple :" << endl << aux_staple << endl ;
      for(index1=0;index1<3;index1++){
      //      index1=((int)(casuale()*3.0));
      //      if(index1==3) index1=2;
      aux_U_prime.one();
      aux_U_prime *= aux_link;
      aux_U_prime *= (aux_staple);
      //      cout << "Prima della cura: " << endl << aux_U_prime << endl; 
      aux_U_prime.sub_su2unitarize(index1,r);
      //      cout << "Dopo la cura: " << endl << aux_U_prime << endl;
      aux_U_prime = ~aux_U_prime;
      aux_U_prime *= aux_link;
      //       cout << "Staple : " << endl << aux_staple << "Link vecchio :" << endl << aux_link << "Link nuovo :" << endl << aux_U_prime << endl;
      //       cout << "Link vecchio * Staple :" << endl << (aux_link)*(aux_staple) << endl << "Link nuovo * Staple : " << endl << (aux_U_prime)*(aux_staple) << endl;
      //       v1 = -(aux_link * (aux_staple)).retr();
      //     v1 = (aux_link * (~(aux_staple))).retr();
      //       v2 = -(aux_U_prime * (aux_staple)).retr();
      //     v2 = (aux_U_prime * (~(aux_staple))).retr();
      //      cerr << v1 << "  --->  " << v2 << endl << endl;
      //    cout << "New link : " << endl << aux_U_prime << endl;
      aux_link = aux_U_prime;
      
      //  dplaq = (aux_link * ((aux_staple))).retr();
      //      plaq -= dplaq;
      }
      (gauge_conf->u_work[r])=aux_U_prime;
    }
  }
  //gauge_conf->save();
  //  plaq*=inv_size_by_three*one_by_three*0.125;
  //  cerr << "Plaq --> " << plaq;
}



//maximal trace Su3 projector: non_unitary_smeared_link is projected to Su3 into
//variable projected_link taking as a starting point the previous unsmeared link old_link

void su3_project_maximal_trace(Su3 &projected_link, Su3 non_unitary_smeared_link, Su3 old_link){
  
  Su3 aux_link;
  Su3 aux_staple;
  Su3 aux_U_prime;
  double traccia_old;
  double traccia_new;
    
  int index1;           //index1 ed index2 dovranno andare assumere iterativamente i valori 0, 1 e 2.

  aux_link = old_link;

  for(int step=0;step<4;step++){

    //    traccia_old = (aux_link*non_unitary_smeared_link).retr();
    
    for(index1=0;index1<3;index1++){
      aux_U_prime = aux_link * non_unitary_smeared_link;
      aux_U_prime.sub_su2unitarize_persmearing(index1,0);    //il secondo argomento è il sito, ma non viene usato...
      aux_link = (~aux_U_prime)*(aux_link);
    }  
    
    //    traccia_new=(aux_link*non_unitary_smeared_link).retr();
    
    //    if(abs(traccia_new-traccia_old)<(1.0e-6)){
    //      break;
    //    }
    
  }


  projected_link = aux_link;


  
}





///////////////////////// era dentro a calc_all_plaq ///////////////
////////////Controllo dei flussi sulle placchette///////////////////

	   /*  if(nu==3 && mu==2){
	     for(quark=0; quark<no_ps;quark++){
	       arg=((double)(2.0 * pi * (-3.0 * quark + 1.0 )))*((double)ez_quantum)/(((double)nz)*((double)nt));
	       flux_exp[quark]=complex<REAL>(cos(arg),-sin(arg));
	       auxE[quark]=  b_field[quark][index_mu+r];
	       auxE[quark]*= b_field[quark][index_nu + nnp[r][mu]];
	       auxE[quark]*=conj(b_field[quark][index_mu + nnp[r][nu]]);
	       auxE[quark]*=conj(b_field[quark][index_nu + r]);
	       auxE[quark]-=flux_exp[quark];
	       flux[quark]=norm(auxE[quark]);
	       out_file << "zt"<<quark<<": "<<flux[quark] << " ";
	     }
	   }
	      if(nu==3 && mu==1){
	     for(quark=0; quark<no_ps;quark++){
	       arg=((double)(2.0 * pi * (-3.0 * quark + 1.0 )))*((double)ey_quantum)/(((double)ny)*((double)nt));
	       flux_exp[quark]=complex<REAL>(cos(arg),-sin(arg));
	       auxE[quark]=  b_field[quark][index_mu+r];
	       auxE[quark]*= b_field[quark][index_nu + nnp[r][mu]];
	       auxE[quark]*=conj(b_field[quark][index_mu + nnp[r][nu]]);
	       auxE[quark]*=conj(b_field[quark][index_nu + r]);
	       auxE[quark]-=flux_exp[quark];
	       flux[quark]=norm(auxE[quark]);
	       out_file << "yt"<<quark<<": "<<flux[quark] << " ";
	     }
	   }
	   if(nu==3 && mu==0){
	     for(quark=0; quark<no_ps;quark++){
	       arg=((double)(2.0 * pi * (-3.0 * quark + 1.0 )))*((double)ex_quantum)/(((double)nx)*((double)nt));
	       flux_exp[quark]=complex<REAL>(cos(arg),-sin(arg));
	       auxE[quark]=  b_field[quark][index_mu+r];
	       auxE[quark]*= b_field[quark][index_nu + nnp[r][mu]];
	       auxE[quark]*=conj(b_field[quark][index_mu + nnp[r][nu]]);
	       auxE[quark]*=conj(b_field[quark][index_nu + r]);
	       auxE[quark]-=flux_exp[quark];
	       flux[quark]=norm(auxE[quark]);
	       out_file << "xt"<<quark<<": "<<flux[quark] << " ";
	     }
	   }
	   if(nu==1 && mu==0){
	     for(quark=0; quark<no_ps;quark++){
	       arg=-((double)(2.0 * pi * (-3.0 * quark + 1.0 )))*((double)bz_quantum)/(((double)nx)*((double)ny));
	       flux_exp[quark]=complex<REAL>(cos(arg),sin(arg));
	       auxB[quark]=  b_field[quark][index_mu+r];
	       auxB[quark]*= b_field[quark][index_nu + nnp[r][mu]];
	       auxB[quark]*=conj(b_field[quark][index_mu + nnp[r][nu]]);
	       auxB[quark]*=conj(b_field[quark][index_nu + r]);
	       auxB[quark]-=flux_exp[quark];
	       flux[quark]=norm(auxB[quark]);
	       out_file << "xy"<<quark<<": "<<flux[quark] << " ";
	       
	     }
	   }
	   if(nu==2 && mu==0){
	     for(quark=0; quark<no_ps;quark++){
	       arg=-((double)(2.0 * pi * (-3.0 * quark + 1.0 )))*((double)by_quantum)/(((double)nz)*((double)nx));
	       flux_exp[quark]=complex<REAL>(cos(arg),sin(arg));
	       auxB[quark]=  b_field[quark][index_nu+r];
	       auxB[quark]*= b_field[quark][index_mu + nnp[r][nu]];
	       auxB[quark]*=conj(b_field[quark][index_nu + nnp[r][mu]]);
	       auxB[quark]*=conj(b_field[quark][index_mu + r]);
	       auxB[quark]-=flux_exp[quark];
	       flux[quark]=norm(auxB[quark]);
	       out_file << "zx"<<quark<<": "<<flux[quark] << " ";
	       
	     }
	   }
	   if(nu==2 && mu==1){
	     for(quark=0; quark<no_ps;quark++){
	       arg=-((double)(2.0 * pi * (-3.0 * quark + 1.0 )))*((double)bx_quantum)/(((double)ny)*((double)nz));
	       flux_exp[quark]=complex<REAL>(cos(arg),sin(arg));
	       auxB[quark]=  b_field[quark][index_mu+r];
	       auxB[quark]*= b_field[quark][index_nu + nnp[r][mu]];
	       auxB[quark]*=conj(b_field[quark][index_mu + nnp[r][nu]]);
	       auxB[quark]*=conj(b_field[quark][index_nu + r]);
	       auxB[quark]-=flux_exp[quark];
	       flux[quark]=norm(auxB[quark]);
	       out_file << "yz"<<quark<<": "<<flux[quark] << " ";
	       if (flux[quark]>0.1) {
		 int x,y,z,t;
		 coord(r,x,y,z,t);
		 out_file <<"("<<x<<","<<y<<","<<z<<","<<t<<")";
	       }
	     }
	     }*/
     // out_file << "\n";
