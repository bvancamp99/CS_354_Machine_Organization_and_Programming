/* many odd sized allocations and interspersed frees */
#include <assert.h>
#include <stdlib.h>
#include "mem.h"

int main() {
   assert(Init_Mem(4096) == 0);
   void * ptr[9];
   ptr[0] = Alloc_Mem(1);
   ptr[1] = (Alloc_Mem(5));
   ptr[2] = (Alloc_Mem(14));
   ptr[3] = (Alloc_Mem(8));
   assert(ptr[0] != NULL);
   assert(ptr[1] != NULL);
   assert(ptr[2] != NULL);
   assert(ptr[3] != NULL);
   
   assert(Free_Mem(ptr[1]) == 0);
   assert(Free_Mem(ptr[0]) == 0);
   assert(Free_Mem(ptr[3]) == 0);
   
   ptr[4] = (Alloc_Mem(1));
   ptr[5] = (Alloc_Mem(4));
   assert(ptr[4] != NULL);
   assert(ptr[5] != NULL);
   assert(Free_Mem(ptr[5]) == 0);
   
   ptr[6] = (Alloc_Mem(9));
   ptr[7] = (Alloc_Mem(33));
   assert(ptr[6] != NULL);
   assert(ptr[7] != NULL);
   
   assert(Free_Mem(ptr[4]) == 0);

   ptr[8] = (Alloc_Mem(55));
   assert(ptr[8] != NULL);

   assert(Free_Mem(ptr[2]) == 0);
   assert(Free_Mem(ptr[7]) == 0);
   assert(Free_Mem(ptr[8]) == 0);
   assert(Free_Mem(ptr[6]) == 0);

   exit(0);
}
