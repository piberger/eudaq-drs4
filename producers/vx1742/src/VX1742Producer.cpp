/* ---------------------------------------------------------------------------------
** CAEN VX1742 implementation into the EUDAQ framework
** 
**
** <VX1742Producer>.cpp
** 
** Date: April 2016
** Author: Christian Dorfer (dorfer@phys.ethz.ch)
** ---------------------------------------------------------------------------------*/

//wishlist
//software trigger
//trigger on signal
//calibration
//readout TRn
//send info in bore event!
//give channels names!


// system includes:
#include <iostream>
#include <ostream>
#include <vector>
#include <mutex>
#include <cmath>
#include <string>
#include <unistd.h>
#include <algorithm>
#include <stdlib.h>

// EUDAQ includes:
#include "eudaq/Producer.hh"
#include "eudaq/Logger.hh"
#include "eudaq/RawDataEvent.hh"
#include "eudaq/Timer.hh"
#include "eudaq/Utils.hh"
#include "eudaq/OptionParser.hh"
#include "eudaq/Configuration.hh"
#include "VX1742Producer.hh"
#include "VX1742Event.hh"


static const std::string EVENT_TYPE = "VX1742";


VX1742Producer::VX1742Producer(const std::string & name, const std::string & runcontrol, const std::string & verbosity)
:eudaq::Producer(name, runcontrol),
  m_producerName(name),
  m_event_type(EVENT_TYPE),
  m_ev(0), 
  m_run(0), 
  m_running(false){

  try{
    caen = new VX1742Interface();
    caen->openVME();
  }
  catch (...){
    EUDAQ_ERROR(std::string("Error in the VX1742Producer class constructor."));
    SetStatus(eudaq::Status::LVL_ERROR, "Error in the VX1742Producer class constructor.");}
}




void VX1742Producer::OnConfigure(const eudaq::Configuration& conf) {
  std::cout << "###Configure VX1742 board with: " << conf.Name() << "..";
  
  try{
    sampling_frequency = conf.Get("sampling_frequency", 0);
    post_trigger_samples = conf.Get("post_trigger_samples", 0);
    trigger_source = conf.Get("trigger_source", 0);
    groups[0] = conf.Get("group0", 0);
    groups[1] = conf.Get("group1", 0);
    groups[2] = conf.Get("group2", 0);
    groups[3] = conf.Get("group3", 0);
    custom_size = conf.Get("custom_size", 0);

    if(caen->isRunning())
      caen->stopAcquisition();

    caen->softwareReset();
    usleep(1000000);

    caen->setSamplingFrequency(sampling_frequency);
    caen->setPostTriggerSamples(post_trigger_samples);
    caen->setTriggerSource(trigger_source);
    caen->toggleGroups(groups);
    caen->setCustomSize(custom_size);
    caen->sendBusyToTRGout();
    caen->setTriggerCount(); //count all, not just accepted triggers


    //individual group configuration here

    //continue here...
    //#) DC offset
    //#) Calibration?

  std::cout << " [OK]" << std::endl;

  
  SetStatus(eudaq::Status::LVL_OK, "Configured VX1742 (" + conf.Name() +")");

  }catch ( ... ){
  EUDAQ_ERROR(std::string("Error in the VX1742 configuration procedure."));
   SetStatus(eudaq::Status::LVL_ERROR, "Error in the VX1742 configuration procedure.");}
}




void VX1742Producer::OnStartRun(unsigned runnumber){
  m_run = runnumber;
  m_ev = 0;
  try{
    this->SetTimeStamp();

    //create BORE
    std::cout<<"VX1742: Create " << m_event_type << "BORE EVENT for run " << m_run <<  " @time: " << m_timestamp << "." << std::endl;
    eudaq::RawDataEvent bore(eudaq::RawDataEvent::BORE(m_event_type, m_run));
    bore.SetTag("timestamp", m_timestamp);
    bore.SetTag("serial_number", caen->getSerialNumber());
    bore.SetTag("firmware_version", caen->getFirmwareVersion());
    
    
    /*
        std::string key = "VX1742_CHANNEL_DAC_"+std::to_string(ch);
	      bore.SetTag(key, m_channel_dac.at(ch));
	
        key = "VX1742_CHANNEL_RANGE_"+std::to_string(ch);
	      bore.SetTag(key, m_dynamic_range.at(ch));

        key = "VX1742_CHANNEL_GAIN_"+std::to_string(ch);
	      bore.SetTag(key, m_channel_gain.at(ch));
    }
    //set number of channels to be implemented
    //set tags for the channel numbers
    */

    caen->clearBuffers();
    caen->startAcquisition();

    //caen->printAcquisitionStatus();
    //caen->printAcquisitionControl();


    SendEvent(bore);

    SetStatus(eudaq::Status::LVL_OK, "Running");
    m_running = true;
  }
  catch (...){
  EUDAQ_ERROR(std::string("Error in the VX1742 OnStartRun procedure."));
  SetStatus(eudaq::Status::LVL_ERROR, "Error in the VX1742 OnStartRun procedure.");}
}



void VX1742Producer::ReadoutLoop() {
  while(!m_terminated){
    if(!m_running){
      sched_yield();} //if not running deprioritize thread

  while(m_running){
    try{
      std::cout << "Events stored: " << caen->getEventsStored() << ", size of next event: " << caen->getNextEventSize() << std::endl;
      usleep(500000);
      if(caen->eventReady()){
        VX1742Event vxEvent;
        caen->BlockTransferEventD64(&vxEvent);

        if(vxEvent.isValid()){
          u_int event_counter = vxEvent.EventCounter();
          u_int trigger_time_tag = vxEvent.TriggerTimeTag();
          u_int n_groups = vxEvent.Groups();
          u_int group_mask = vxEvent.GroupMask();
          u_int event_size = vxEvent.EventSize();
          








          
          //loop over group mask

          


          u_int n_channels = vxEvent.Channels(0);
          u_int samples_per_channel = vxEvent.SamplesPerChannel(0);

          std::cout << "***********************************************************************" << std::endl << std::endl;
          std::cout << "Event number: " << event_counter << std::endl;
          std::cout << "Event valid: " << vxEvent.isValid() << std::endl;
          std::cout << "Event size: " << vxEvent.EventSize() << std::endl;
          std::cout << "Number of groups: " << n_groups << std::endl;
          std::cout << "Active number of channels in group 0: " << vxEvent.Channels(0) << std::endl;
          std::cout << "Samples per channel: " << vxEvent.SamplesPerChannel(0) << std::endl;
          std::cout << "Group mask: " << vxEvent.GroupMask() << std::endl;
          std::cout << "Trigger time tag: " << vxEvent.TriggerTimeTag() << std::endl;
          std::cout << "***********************************************************************" << std::endl << std::endl; 


          //generate EUDAQ event
          u_int block_no = 0;
          eudaq::RawDataEvent ev(m_event_type, m_run, event_counter);
          ev.AddBlock(block_no, reinterpret_cast<const char*>(&event_size), sizeof(u_int));
          block_no++;
          ev.AddBlock(block_no, reinterpret_cast<const char*>(&group_mask), sizeof(u_int));
          block_no++;
          ev.AddBlock(block_no, reinterpret_cast<const char*>(&n_channels), sizeof(u_int));
          block_no++;
          ev.AddBlock(block_no, reinterpret_cast<const char*>(&samples_per_channel), sizeof(u_int));
          block_no++;
          ev.AddBlock(block_no, reinterpret_cast<const char*>(&trigger_time_tag), sizeof(u_int));
          block_no++;  
          

          if(n_groups > 0){
            for(u_int ch = 0; ch < n_channels; ch++){ //fixme: loop over groups and channels!
              uint16_t *payload = new uint16_t[samples_per_channel];
              vxEvent.getChannelData(0, ch, payload, samples_per_channel); //fixme
              ev.AddBlock(block_no, reinterpret_cast<const char*>(payload), samples_per_channel*sizeof(uint16_t));
              block_no++;
              delete payload;
            }
          }

          
          if(n_groups ==0){ //first two sucker events have no channel data for whatever reason which then fucks up the OnlineMonitor - just send zeros
            int samples = this->SamplesInCustomSize();
            for(u_int ch = 0; ch < 8; ch++){ //fixme: loop over groups and channels!
              uint16_t *payload = new uint16_t[samples];
              for(int idx = 0; idx<samples; idx++){
                payload[idx] = 0;
              }
              ev.AddBlock(block_no, reinterpret_cast<const char*>(payload), samples*sizeof(uint16_t));
              block_no++;
              //delete payload;
            }
          }
          
          SendEvent(ev);
          
        }// is valid
      }// event is ready

    }catch (...){
    EUDAQ_ERROR(std::string("Readout error................"));
    SetStatus(eudaq::Status::LVL_ERROR, "Readout error................");}

    }// while running
  }// while not terminated
}// ReadoutLoop



void VX1742Producer::OnStopRun(){
  m_running = false;
  //caen->printAcquisitionControl();
  caen->stopAcquisition();
  std::cout << "VX1742 run stopped." << std::endl;
}


void VX1742Producer::OnTerminate(){
  m_running = false;
  m_terminated = true;
  caen->closeVME();
  delete caen;
  std::cout << "VX1742 producer terminated." << std::endl; 
}


int main(int /*argc*/, const char ** argv){
  // You can use the OptionParser to get command-line arguments
  // then they will automatically be described in the help (-h) option
  eudaq::OptionParser op("VX1742 Producer", "0.0", "Run options");
  eudaq::Option<std::string> rctrl(op, "r", "runcontrol",
    "tcp://localhost:44000", "address", "The address of the RunControl.");
  eudaq::Option<std::string> level(op, "l", "log-level", "NONE", "level",
    "The minimum level for displaying log messages locally");
  eudaq::Option<std::string> name (op, "n", "name", "VX1742", "string",
    "The name of this Producer");
  eudaq::Option<std::string> verbosity(op, "v", "verbosity mode", "INFO", "string");

  try{
    // This will look through the command-line arguments and set the options
    op.Parse(argv);
    // Set the Log level for displaying messages based on command-line
    EUDAQ_LOG_LEVEL(level.Value());
    // Create a producer
    VX1742Producer *producer = new VX1742Producer(name.Value(), rctrl.Value(), verbosity.Value());
    // And set it running...
    producer->ReadoutLoop();
    // When the readout loop terminates, it is time to go
    std::cout << "VX1742: Quitting" << std::endl;

  }
  catch(...){
    // This does some basic error handling of common exceptions
    return op.HandleMainException();}

  return 0;
}


void VX1742Producer::SetTimeStamp(){
  std::chrono::high_resolution_clock::time_point epoch;
  auto now = std::chrono::high_resolution_clock::now();
  auto elapsed = now - epoch;
  m_timestamp = std::chrono::duration_cast<std::chrono::nanoseconds>(elapsed).count()/100u;
}


int VX1742Producer::SamplesInCustomSize(){
  int csizes[4] = {1024, 520, 256, 136};
  return csizes[custom_size];
}



/*//put in software trigger
        std::cout << "Event number: " << vxEvent.EventCounter() << std::endl;
        std::cout << "Event valid: " << vxEvent.isValid() << std::endl;
        std::cout << "Event size: " << vxEvent.EventSize() << std::endl;
        std::cout << "Active number of channels: " << vxEvent.Channels() << std::endl;
        std::cout << "Samples per channel: " << vxEvent.SamplesPerChannel() << std::endl;
        std::cout << "Group mask: " << vxEvent.GroupMask() << std::endl;
        std::cout << "Trigger time tag: " << vxEvent.TriggerTimeTag() << std::endl;
        std::cout << "***********************************************************************" << std::endl << std::endl;  


std::cout << "+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++" << std::endl;
          std::cout << "Number of groups: " << vxEvent.Groups() << std::endl;
          std::cout << "Number of active channels in group 0: " << vxEvent.Channels(0) << std::endl;
          std::cout << "Buffer size: " << vxEvent.rawdata_size() << std::endl;
          std::cout << "Group size in buffer: " << vxEvent.getGroupSizeInBuffer() << std::endl;
          std::cout << "Group 0 position: " << vxEvent.getGroupIndexInBuffer(0) << std::endl;
          std::cout << "Group 1 position: " << vxEvent.getGroupIndexInBuffer(1) << std::endl;
          std::cout << "Group 2 position: " << vxEvent.getGroupIndexInBuffer(2) << std::endl;
          std::cout << "Group 3 position: " << vxEvent.getGroupIndexInBuffer(3) << std::endl;
          std::cout << "Samples per channel in group 1: " << vxEvent.SamplesPerChannel(0) << std::endl;
          std::cout << "Samples per channel in group 2: " << vxEvent.SamplesPerChannel(1) << std::endl;
          std::cout << "Samples per channel in group 3: " << vxEvent.SamplesPerChannel(2) << std::endl;
          std::cout << "Samples per channel in group 4: " << vxEvent.SamplesPerChannel(3) << std::endl;
          std::cout << "+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++" << std::endl;


        */

