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
  delete seg;
  vme->MasterUnmap(vmm);
  vme->Close();
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


u_int VX1742Interface::isRunning(){
	return vx1742->acq_status.run; //returns 1 if running
}

void VX1742Interface::startAcquisition(){
	vx1742->acq_control.run = 0;
}

void VX1742Interface::stopAcquisition(){
	vx1742->acq_control.run = 1;
}

void VX1742Interface::softwareReset(){
	vx1742->software_reset = 1;
}

void VX1742Interface::clearBuffers(){
	vx1742->software_clear = 1;
}

void VX1742Interface::setSamplingFrequency(u_int param){
	vx1742->sampling_frequency = param;
}

u_int VX1742Interface::getSamplingFrequency(){
	return vx1742->sampling_frequency;

}

void VX1742Interface::setPostTriggerSamples(u_int param){
	vx1742->post_trigger = param;
}

u_int VX1742Interface::getPostTriggerSamples(){
	return vx1742->post_trigger;
}

void VX1742Interface::setTriggerSource(u_int param){
	if(param==0){
		vx1742->trigger_src.sw_trigger = 0;
		vx1742->trigger_src.ext_trigger = 1;
	}
	if(param==1){
		vx1742->trigger_src.ext_trigger = 0;
		vx1742->trigger_src.sw_trigger = 1;
	}
}

std::string VX1742Interface::getTriggerSource(){
	std::stringstream source;
	source << "Trigger source: ";
	u_int sw = vx1742->trigger_src.sw_trigger;
	u_int ext = vx1742->trigger_src.ext_trigger; 
	if(sw==0 and ext==1){source << "external";}
	if(sw==1 and ext==0){source << "software";
	}else{source << "not defined";}
	return source.str();
}


void VX1742Interface::toggleGroups(u_int param[]){
	param[0]==1 ? vx1742->group_en_mask.group0=1 : vx1742->group_en_mask.group0=0;
	param[1]==1 ? vx1742->group_en_mask.group1=1 : vx1742->group_en_mask.group1=0;
	param[2]==1 ? vx1742->group_en_mask.group2=1 : vx1742->group_en_mask.group2=0;
	param[3]==1 ? vx1742->group_en_mask.group3=1 : vx1742->group_en_mask.group3=0;
}


void VX1742Interface::sendBusyToTRGout(){
	vx1742->front_panel_io_control.force_trg_out_mode = 0; //force it or not, it does not matter
	vx1742->front_panel_io_control.trg_out_mode = 0; //trg_out is internal signal
	vx1742->front_panel_io_control.trg_out_mode_select = 1; //select trg_out
	vx1742->front_panel_io_control.motherboad_probe = 3; //allows roc to issue busy signal
	vx1742->front_panel_io_control.busy_unlock = 0; //propagates it to trg_out
}

void VX1742Interface::setTriggerCount(){
	vx1742->acq_control.trigger_count = 1;
}


































