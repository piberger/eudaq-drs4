/* -----------------------------------------------------------------------------------
** CAEN VX1742 implementation into the EUDAQ framework
** 
**
** <VX1742Interface>.cpp
** 
** Date: April 2016
** Info: This is not a stand-alone driver. It was specifically written for our needs.
** Author: Christian Dorfer (dorfer@phys.ethz.ch)
** -----------------------------------------------------------------------------------*/


#include <iostream>
#include <sstream>
#include "VX1742Interface.hh"
#include "VX1742Event.hh"

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
    std::cout << std::endl << "###Initialize VX1742 connection.. ";  
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
    std::cout << std::endl << "***********************************************************************" << std::endl;
    std::cout << "Board serial number: " << this->getSerialNumber() << std::endl;
    std::cout << this->getFirmwareVersion() << std::endl;
    std::cout << this->getDRS4FirmwareVersion();
    std::cout << "***********************************************************************" << std::endl << std::endl;

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


std::string VX1742Interface::getDRS4FirmwareVersion(){
	std::stringstream firmware;
	u_int tmp, value, value1, value2;
	for(u_int idx=0; idx<VX1742_GROUPS; idx++){
		tmp = vx1742->group_n_conf[idx].daugther_board_fw;
		value = (tmp&0xff);
  		value1 = ((tmp>>8)&0xff);
  		value2 = (tmp>>16);
  		u_int day = (value2&0xf) + 10*((value2>>4)&0xf);
  		u_int month = ((value2>>8)&0xf);
  		u_int year = ((value2>>12)&0xf);
  		firmware << "DRS4 firmware version group " << idx << ": " << value1 << "." << value << " from " << day << "." << month << "." << year << std::endl;
	}
	return firmware.str();
}


u_int VX1742Interface::isRunning(){
	return vx1742->acq_status.run; //returns 1 if running
}

void VX1742Interface::startAcquisition(){
	vx1742->acq_control.run = 1;
}

void VX1742Interface::stopAcquisition(){
	vx1742->acq_control.run = 0;
}

void VX1742Interface::printAcquisitionStatus(){
	printf("Board %s\n", vx1742->acq_status.run ? "running" : "not running");
	printf("Event %s\n", vx1742->acq_status.event_ready ? "ready" : "not ready");
	printf("Event %s\n", vx1742->acq_status.event_full ? "full" : "free");
	printf("Clock source %s\n", vx1742->acq_status.clock_source ? "external" : "internal");
	printf("PLL %s\n", vx1742->acq_status.PLL_bypass ? "bypassed" : "no bypass");
	printf("PLL %s\n", vx1742->acq_status.PLL_status ? "locked" : "loss of lock");
	printf("Board %s\n", vx1742->acq_status.ready ? "ready" : "not ready");
}

void VX1742Interface::printAcquisitionControl(){
	std::cout << "reserved 0: " << vx1742->acq_control.reserved_0 << std::endl;
	std::cout << "buffer mode: " << vx1742->acq_control.buffer_mode<< std::endl;
	std::cout << "reserved 1: " << vx1742->acq_control.reserved_1 << std::endl;
	std::cout << "trigger count: " << vx1742->acq_control.trigger_count << std::endl;
	std::cout << "run: " << vx1742->acq_control.run << std::endl;
	std::cout << "reserved" << vx1742->acq_control.reserved << std::endl;

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


void VX1742Interface::setCustomSize(u_int param){
	vx1742->custom_size = param;
}

u_int VX1742Interface::getCustomSize(){
	return vx1742->custom_size;
}

void VX1742Interface::setTriggerCount(){
	vx1742->acq_control.trigger_count = 1;
}

void VX1742Interface::setMaxBLTEvents(u_int param){
	vx1742->blt_event_number = param;
}

bool VX1742Interface::eventReady(){
	return vx1742->acq_status.event_ready;
}


u_int VX1742Interface::getEventsStored(){
	return vx1742->events_stored;
}

u_int VX1742Interface::getNextEventSize(){
	return vx1742->event_size;
}


//nEvents needs to be smaller than 255
u_int VX1742Interface::BlockTransferEventD64(VX1742Event *vxEvent){
	u_int eventsize = this->getNextEventSize();
	this->setMaxBLTEvents(1); //raise BE if more than 1 event is read out

	if(eventsize > 0){
		vme->RunBlockTransfer(*seg, 0x0, vmebus_address, eventsize, VME_DMA_D64R, buffer_size, true);
		u_int* data = (u_int*)seg->VirtualAddress();
    	//size = (u_int)seg->Size()/sizeof(u_int);
    	
    	#ifdef DEBUG
    		std::printf("CMEM segment, virt = %p, phys = 0x%016lx, size = %d\n", (void*)data, seg->PhysicalAddress(), eventsize);
    		std::cout <<"Dump data:" << std::endl;
    		for(u_int i=0; i<eventsize; i++){
        		std::printf("%6d: 0x%08x\n",i , data[i]);}
        #endif
        
		//write data to event class
		VX1742Event::header head;
		
		u_int offset = 0;
		head.size.raw = data[offset];
		
		//some checks on the data
		if(head.size.raw == 0)
			std::printf("First word of VX1742 event not found!\n");
		if(head.size.eventSize != eventsize)
			std::printf("Read event contains %d words instead of the expected %d!\n", head.size.eventSize, eventsize);
		if(head.size.A != 0xA)
			std::printf("Event does not start with 0xA!");

		head.pattern.raw= data[(++offset)];
		head.evnt_cnt.raw= data[(++offset)];
		head.trigger_time= data[(++offset)];

		#ifdef DEBUG
			printf("******************************************************\n");
			printf("RAW header:         0x%08X\n", head.size.raw);
			printf("0xA:                0x%01X\n", head.size.A);
			printf("Event size:         %d\n", head.size.eventSize);
			printf("------------------------------------------------------\n");
			printf("RAW header_pattern: 0x%08X\n", head.pattern.raw);
			printf("Group mask:         %d\n", head.pattern.group_mask);
			printf("Pattern:            %d\n", head.pattern.pattern);
			printf("Board ID:           %d\n", head.pattern.board_id);
			printf("------------------------------------------------------\n");
			printf("RAW event_counter:  0x%08X\n", head.evnt_cnt.raw);
			printf("Event count:        %d\n", head.evnt_cnt.event_counter);
			printf("------------------------------------------------------\n");
			printf("Trigger time:       %d\n", head.trigger_time);
			printf("******************************************************\n\n");
		#endif

		vxEvent->setData(&head, data, (head.size.eventSize-4));
	}

	return 0;
}