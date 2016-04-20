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
	std::cout << "reserved: " << vx1742->acq_control.reserved << std::endl;
	std::cout << "buffer mode: " << vx1742->acq_control.buffer_mode<< std::endl;
	std::cout << "reserved 1: " << vx1742->acq_control.reserved_1 << std::endl;
	std::cout << "trigger count: " << vx1742->acq_control.trigger_count << std::endl;
	std::cout << "run: " << vx1742->acq_control.run << std::endl;
	std::cout << "reserved 0: " << vx1742->acq_control.reserved_0 << std::endl;

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

void VX1742Interface::setMaxBLTEvents(u_int param){
	vx1742->blt_event_number = param;
}

u_int VX1742Interface::getEventsStored(){
	return vx1742->events_stored;
}

u_int VX1742Interface::getNextEventSize(){
	return vx1742->event_size;
}


//nEvents needs to be smaller than 255
u_int VX1742Interface::BlockTransferD64(u_int VX1742Event , u_int nEvents){

	u_int eventsize = this->getNextEventSize();
	u_int vme_buffer_address = (size_t)&vx1742->output_buffer;
	std::cout << "Buffer address: " << vme_buffer_address << std::endl;
	vme->RunBlockTransfer(*seg, 0x00000000, vme_buffer_address, eventsize, VME_DMA_D64R, 0xffffffff, false);
}




/*
readout collection:
return(vme->RunBlockTransfer(*seg,0x00000000,g_base+addr,size,dir,0xffffffff,false));
u_int RunBlockTransfer(const CMEMSegment&, const u_int, const u_int, const u_int, const u_int, const u_int msiz = 0xffffffff, const bool incr = false);
//(nEvents < 255) ? this->BlockTransferD64(nEvents) : std::cout << "Number of events for BLT to big " << std::endl; //FIXME: raise error
uint32_t* data = (uint32_t*)seg->VirtualAddress();
*/



/*
===============
blist.number_of_items = 1;
blist.list_of_items[0].vmebus_address       = VME_BLOCK_TRANSFER_ADDRESS;
blist.list_of_items[0].system_iobus_address = cmem_desc_in_paddr;  
blist.list_of_items[0].size_requested       = 0x400;              
blist.list_of_items[0].control_word         = VME_DMA_D64R;        //Type of transfer: D64 read

ret = VME_BlockTransfer(&blist, time_out);
===============

VME_DMA_D64R für MBLT64 read und VME_DMA_D64W für MBLT64 write.







int caen_v1742::BlockTransfer_D64_many_events(v1742_event *events, int events_to_read) {

	
	size_t eventsize=(number_enabled_groups(GroupEnableMask)*blksize())/4+4;	//in multiples of 4 bytes (32 bits)
	uint32_t* buffer = new uint32_t[eventsize*events_to_read+2];


	if ( (ret=vme->read64(ba, reinterpret_cast <uint64_t*>(buffer), (eventsize*events_to_read/2)+1, false)) < 0 ){
		throw llbad_caen_v1742_vme_error("BlockTransfer_D64_many_events::read error",ret);
	}
	#if __BYTE_ORDER == __LITTLE_ENDIAN
		for(size_t i=0; i<ret; i++){
			uint32_t buf=buffer[2*i];
			buffer[2*i]=buffer[2*i+1];
			buffer[2*i+1]=buf;
		}
	#endif
	//each 64 bit transfer covers two 32 bit words
	ret*=2;
	
	#ifdef v1742_DEBUG
		cout << "ret=" <<ret << "; ret/eventsize=" << ret/eventsize;
		if( ret%eventsize != 0 ) {
			cout << "; ret%eventsize= " << ret%eventsize <<"!=0";
		}
		cout << endl;
	
		
		for(int i=eventsize*(ret/eventsize);i<ret;i++){
			cout << dec <<i << ":" << hex << buffer[i] << dec <<endl;
		}
	#endif

	ret/=eventsize;

	v1742_event::header header;

	for(int i=0; i< events_to_read; i++) {
		if(i>=ret){
			events[i].invalidate();
		} else {
			header.size.raw=buffer[i*eventsize];
			if (header.size.A != 0xA) {
				throw llbad_caen_v1742("Event does not start with 0xA",header.size.raw);
			}
			header.pattern.raw=buffer[i*eventsize+1];
			header.evnt_cnt.raw=buffer[i*eventsize+2];
			header.trigger_time=buffer[i*eventsize+3];
	#ifdef v1742_DEBUG	
			printf(" header_size:    0x%08X\n", header.size.raw);
			printf(" header_pattern: 0x%08X\n", header.pattern.raw);
			printf(" event_counter:  0x%08X\n", header.evnt_cnt.raw);
			printf(" trigger_time:   0x%08X\n\n", header.trigger_time);

			printf(" EventSize: %d x32bit = 4x32bit Header + %d byte data\n", header.size.eventSize, (header.size.eventSize-4)*4);
	#endif
		

			events[i].setData(&header, buffer+i*eventsize+4, header.size.eventSize-4);// 4 long word header is not part of buffer
		}
	}
	delete [] buffer;
	return ret;
}



*/





















