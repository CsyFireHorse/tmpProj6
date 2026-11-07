#include "pci_zecm.h"

void wwww(){
    ECAT_HANDLE hHandle;
    EcatOpen(&hHandle, "zecm0", 0);
    EcatBusRun(hHandle, "pse_servo3.xml");
    
}