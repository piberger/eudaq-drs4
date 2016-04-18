/* ---------------------------------------------------------------------------------
** CAEN VX1742 implementation into the EUDAQ framework
** 
**
** <VX1742Interface>.cpp
** 
** Date: April 2016
** Author: Christian Dorfer (dorfer@phys.ethz.ch)
** ---------------------------------------------------------------------------------*/


#include <iostream>
#include <sstream>
#include "VX1742Interface.hh"

using namespace RCD;


VX1742Interface::VX1742Interface(){
	vme = 0;
	vmm = 0;
	seg = 0;
	vx1742=0;
	virtual_address = 0;
}


VX1742Interface::~VX1742Interface(){
	//unmap, delete pointers, etc

}


u_int VX1742Interface::openVME(){
  try{
    std::cout << "###Initialize VX1742 connection.. ";  
    vme = VME::Open();
    if((*vme)()){
      std::cout << std::endl << "ERROR opening VME object." << std::endl;
      vme->ErrorPrint((*vme)());
      std::exit(-1);
    }
    std::cout << "[OK]" << std::endl;

    std::cout << "###Create VME master mapping.. "; 
    vmm = vme->MasterMap(vmebus_address, window_size, address_modifier, options);
    if((*vmm)()) {
      std::cout << "ERROR opening VME master mapping." << std::endl;
      vme->ErrorPrint((*vmm)());
      std::exit(-1);
    }

    virtual_address = 0;
    vmm->VirtualAddress(&virtual_address);
    if((*vmm)() || virtual_address == 0) {
      std::cout << "ERROR returning virtual address." << std::endl;
      vme->ErrorPrint((*vmm)());
      std::exit(-1);
    }

    vx1742 = (vx1742_regs_t *) virtual_address;
    std::cout << "[OK]" << std::endl;
     
    //printf("checksum              is at 0x%08x\n", (size_t)&vx1742->flash_data  - (size_t)&vx1742->output_buffer);
    std::cout << "###Create new contiguous memory segment.. ";
    seg = new CMEMSegment("caenProducer", buffer_size);
    std::cout << "[OK]" << std::endl;
    

    //connection is initialized at this point, print some information:
    std::cout << "Board serial number: " << this->getSerialNumber() << std::endl;
    std::cout << this->getFirmwareVersion() << std::endl;


  }catch(...){
  	std::cout << "Problem occured in VX1742Interface::OpenVME()" << std::endl;
  }

}


u_int VX1742Interface::closeVME(){

}





std::string VX1742Interface::getSerialNumber(){
  return std::to_string(vx1742->configuration_rom.sernum0);	
}


std::string VX1742Interface::getFirmwareVersion(){
	std::stringstream firmware;
	u_int value, value1, value2;
    value = ((vx1742->mother_roc_firmware)&0xff);
    value1 = (((vx1742->mother_roc_firmware)>>8)&0xff);
    value2 = ((vx1742->mother_roc_firmware)>>16);
    u_int day = (value2&0xf) + 10*((value2>>4)&0xf);
    u_int month = ((value2>>8)&0xf);
    u_int year = ((value2>>12)&0xf);
    firmware << "VX1742 firmware version: " << value1 << "." << value << " from " << day << "." << month << "." << year;
    return firmware.str();
}