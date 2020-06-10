#ifndef DEVICE_INIT_C_
#define DEVICE_INIT_C_

#include "./deviceinit.h"
#include "../Mpi/multidev.h"

#ifndef __GNUC__  // assuming then PGI is used for compilation on accelerator
       // at the moment only nvidia gpus are supported.

#include "openacc.h"


// ROUTINE TO CHOOSE AND INITIALIZE THE OPENACC DEVICE
void select_init_acc_device(acc_device_t my_device_type, int dev_index)
{

  // Initialize context for this device type
  acc_init(my_device_type);

  // Get available devices of this type
  int num_devices = acc_get_num_devices(my_device_type);
  printf("MPI%02d: Number of OpenAcc exploitable devices found: %d \n",
          devinfo.myrank, num_devices);

  // Pick the device number dev_index 
  acc_set_device_num(dev_index, my_device_type);
  printf("MPI%02d: Selected device number: %d \n",
          devinfo.myrank, dev_index);

}

void shutdown_acc_device(acc_device_t my_device_type) 
{

  // Close context for this device type
  acc_shutdown(my_device_type);

}

#endif
#endif

