/* --------------------------------------------------------------------------------------------
** CAEN VX1742 implementation into the EUDAQ framework
** 
**
** <VX1742Interface>.hh
** 
** Date: April 2016
** Remarks: This is not a stand-alone driver, which was specifically written for our needs.
** Author: Christian Dorfer (dorfer@phys.ethz.ch)
** -------------------------------------------------------------------------------------------- */


#ifndef VX1742INTERFACE_HH
#define VX1742INTERFACE_HH

// VME includes
#include "RCDVme/RCDVme.h"
#include "RCDVme/RCDCmemSegment.h"
#include "VX1742DEFS.hh"

class VX1742Event;


class VX1742Interface{

  private:
    RCD::VME *vme;
    RCD::VMEMasterMap *vmm;
    u_long virtual_address;
    RCD::CMEMSegment* seg;
    vx1742_regs_t *vx1742;

  public:
	VX1742Interface();
	~VX1742Interface(); //if mapping not yet destroyed do it
	u_int openVME(); //open vme and create master mapping
	u_int closeVME(); //close and destroy mapping

	std::string getSerialNumber();
	std::string getFirmwareVersion();
	std::string getDRS4FirmwareVersion();
	u_int isRunning();
	void startAcquisition();
	void stopAcquisition();
	void softwareReset();
	void clearBuffers();
	void printAcquisitionStatus();
	void printAcquisitionControl();

	void setSamplingFrequency(u_int param);
	u_int getSamplingFrequency();

	void setPostTriggerSamples(u_int param);
	u_int getPostTriggerSamples();

	void setCustomSize(u_int param);
	u_int getCustomSize();

	void setTriggerSource(u_int param);
	std::string getTriggerSource();

	void toggleGroups(u_int param[]);

	void sendBusyToTRGout();

	void setTriggerCount();
	void setMaxBLTEvents(u_int param);
	bool eventReady();
	u_int getEventsStored();
	u_int getNextEventSize();

	void SetMaxBLTEvents(u_int param);
	u_int BlockTransferEventD64(VX1742Event *vxEvent);



};


#endif