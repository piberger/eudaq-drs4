// system includes:
#include <iostream>
#include <ostream>
#include <vector>
#include <mutex>
#include <cmath>
#include <string>
#include <unistd.h>
#include <algorithm>


// EUDAQ includes:
#include "eudaq/Producer.hh"
#include "eudaq/Logger.hh"
#include "eudaq/RawDataEvent.hh"
#include "eudaq/Timer.hh"
#include "eudaq/Utils.hh"
#include "eudaq/OptionParser.hh"
#include "eudaq/Configuration.hh"

#include "vme/Concurrent/VP717.h"
#include "V1730Producer.hh"
#include "vme/CAEN/V1730_ADC/caen_v1730.h"


static const std::string EVENT_TYPE = "V1730";


V1730Producer::V1730Producer(const std::string & name, const std::string & runcontrol, const std::string & verbosity)  
  : eudaq::Producer(name, runcontrol),
  m_producerName(name),
  m_event_type(EVENT_TYPE),
  m_ev(0), 
  m_run(0), 
  m_running(false){

  if (V1730_handle)
    delete V1730_handle;
  
  try{
  //initialize a pointer to the caen_v1730 class:
  vp717_interface = new VP717();
  interface = vp717_interface;
  vme_addr_t V1730_handleBaseAddress=0x32100000;
  V1730_handle = new caen_v1730(interface,V1730_handleBaseAddress);
  
  //get serial number and firmware revision and initialize class variables:
  m_serialno = V1730_handle->getSerialNumber();
  caen_v1730::firmware_revision fw = V1730_handle->caen_v1730::getROC_firmware();
  m_firmware = fw.firmware_major + 0.1*fw.firmware_minor;

  std::cout << "V1730: VME connection to CAEN " << V1730_handle->getBoardModel() << " established." << std::endl;
  std::cout << "Firmware version: " << m_firmware << std::endl << std::endl;

  m_terminated = false;
  }
  catch (...){
    EUDAQ_ERROR(std::string("Error in the V1730Producer class constructor."));
    SetStatus(eudaq::Status::LVL_ERROR, "Error in the V1730Producer class constructor.");}
}





void V1730Producer::OnStartRun(unsigned runnumber){
  m_run = runnumber;
  try{
    //set time stamp:
    this->SetTimeStamp();

    //create event:
    std::cout<<"V1730: Create " << m_event_type << " EVENT for run " << m_run <<  " @time: " << m_timestamp << "." << std::endl;
    eudaq::RawDataEvent bore(eudaq::RawDataEvent::BORE(m_event_type, m_run));
    bore.SetTag("V1730_timestamp", std::to_string(m_timestamp));
    bore.SetTag("V1730_serial_no", std::to_string(m_serialno));
    bore.SetTag("V1730_firmware_v", std::to_string(m_firmware));
    //set number of channels to be implemented
    //set tags for the channel numbers

    //buffer in event??

    //set digitizer online (allow it to accept triggers)
    V1730_handle->Start();

    V1730_handle->printAcquisitionStatus(V1730_handle->getAcquisitionStatus());

    SendEvent(bore);

    SetStatus(eudaq::Status::LVL_OK, "Running");
    m_running = true;
  }
  catch (...){
  EUDAQ_ERROR(std::string("Error in the V1730 OnStartRun procedure."));
  SetStatus(eudaq::Status::LVL_ERROR, "Error in the V1730 OnStartRun procedure.");}
}


void V1730Producer::OnStopRun(){
  // Break the readout loop
  m_running = false;
  V1730_handle->Stop();
  std::cout << "V1730 run stopped." << std::endl;
}



void V1730Producer::OnTerminate(){
  m_running = false;
  m_terminated = true;
  V1730_handle->Stop();
  V1730_handle->SoftwareReset(); //resets registers
  V1730_handle->SoftwareClear(); //clears buffers
  std::cout << "V1730 run terminated, registers reset and buffers cleared." << std::endl;
  delete vp717_interface, interface, V1730_handle;
}



void V1730Producer::ReadoutLoop() {
  while(!m_terminated){
    if(!m_running){
      sched_yield();} //if not running deprioritize thread


  while(m_running){
    try{
      //std::cout << "Readout loop running.." << std::endl;
      //V1730_handle->SoftwareTrigger();
      //sleep(1);

      if(V1730_handle->isEventReady()){
        //CAEN event:
        v1730_event event;
        //generate a raw event
        eudaq::RawDataEvent ev(m_event_type, m_run, m_ev);

        V1730_handle->ReadEvent_D32(event);

        unsigned int block_no = 0;
        size_t arraylen = event.SamplesPerChannel();
        uint16_t *payload = new uint16_t[arraylen];
        unsigned int chan = 0;
        event.getChannelData(chan, payload, arraylen);
        ev.AddBlock(block_no, reinterpret_cast<const char*>(payload), arraylen);
        SendEvent(ev);



        

        //V1730_handle->clearBuffer();
        m_ev++;
        std::cout << "Event counter (loop): " << m_ev << std::endl;
        

        //print WF data
        for(int i = 0; i<arraylen; i++){
          std::cout << payload[i] << ", ";}
          std::cout << std::endl;    

        delete payload; 


      std::cout << "Event valid: "          << event.isValid()       << std::endl;
      std::cout << "Event size(32b words): "<< event.EventSize()     << std::endl;
      std::cout << "Channel mask: "         << event.ChannelMask()   << std::endl;
      std::cout << "Event Counter: "        << event.EventCounter()       << std::endl;
      std::cout << "Samples per channel: "  << event.SamplesPerChannel()  << std::endl;
      std::cout << "Channels: "             << event.Channels()       << std::endl;  


      std::cout << std::endl << std::endl; 
            
      }//end if
      

    }  
    catch (...){
    EUDAQ_ERROR(std::string("Readout error................"));
    SetStatus(eudaq::Status::LVL_ERROR, "Readout error................");}



    }//running
  
  }//terminated
}




void V1730Producer::OnConfigure(const eudaq::Configuration& conf) {
  std::cout << "###Configure V1730 board:" << std::endl;  
  m_config = conf;

  try{
    if(V1730_handle->isRunning()){
      V1730_handle->Stop();}
    
    //set all configuration registers to their default values
    V1730_handle->SoftwareReset();
    sleep(1);

    //enable external trigger
    caen_v1730::trigger_mask t_mask = V1730_handle->getTriggerSourceMask();
    t_mask.external = 1;
    t_mask.software = 0;
    V1730_handle->setTriggerSourceMask(t_mask);
    V1730_handle->printTriggerMask(V1730_handle->getTriggerSourceMask());

    //enable channel 1
    //caen_v1730::channel_enable_mask c_enable_mask = V1730_handle->getChannelEnableMask();
    caen_v1730::channel_enable_mask c_enable_mask = caen_v1730::channel_enable_mask(0);
    c_enable_mask.ch0 = 1;
    V1730_handle->setChannelEnableMask(c_enable_mask);
    V1730_handle->printChannelEnableMask(V1730_handle->getChannelEnableMask());

    //set channel voltage range
    for(int ch = 0; ch < V1730_handle->GROUPS; ch++){
      V1730_handle->setChannel_Gain(ch, caen_v1730::GAIN_4); //set input range to +/-0.5V
      std::cout << "Calibrating channel " << ch << "..";
      V1730_handle->doChannel_Calibration(ch);
      caen_v1730::channel_status status = V1730_handle->getChannel_Status(ch);
      if(status.calibration_status){
        std::cout << " [OK]" << std::endl;
        //std::cout << " [OK] - Temp: " << V1730_handle->getChannel_Temperature(ch) << std::endl;
      }
    }

    //all samples are pre-triggered
    V1730_handle->setPostTriggerSamples(0);

  std::cout << "V1730: Configured! Ready to take data." << std::endl;

  }catch ( ... ){
  EUDAQ_ERROR(std::string("Error in the V1730 configuration procedure."));
   SetStatus(eudaq::Status::LVL_ERROR, "Error in the V1730 configuration procedure.");}
}




int main(int /*argc*/, const char ** argv){
  // You can use the OptionParser to get command-line arguments
  // then they will automatically be described in the help (-h) option
  eudaq::OptionParser op("V1730 Producer", "0.0", "Run options");
  eudaq::Option<std::string> rctrl(op, "r", "runcontrol",
    "tcp://localhost:44000", "address", "The address of the RunControl.");
  eudaq::Option<std::string> level(op, "l", "log-level", "NONE", "level",
    "The minimum level for displaying log messages locally");
  eudaq::Option<std::string> name (op, "n", "name", "V1730", "string",
    "The name of this Producer");
  eudaq::Option<std::string> verbosity(op, "v", "verbosity mode", "INFO", "string");

  try{
    // This will look through the command-line arguments and set the options
    op.Parse(argv);
    // Set the Log level for displaying messages based on command-line
    EUDAQ_LOG_LEVEL(level.Value());
    // Create a producer
    V1730Producer *producer = new V1730Producer(name.Value(), rctrl.Value(), verbosity.Value());
    // And set it running...
    producer->ReadoutLoop();
    // When the readout loop terminates, it is time to go
    std::cout << "V1730: Quitting" << std::endl;

  }
  catch(...){
    // This does some basic error handling of common exceptions
    return op.HandleMainException();}

  return 0;
}


void V1730Producer::SetTimeStamp(){
  std::chrono::high_resolution_clock::time_point epoch;
  auto now = std::chrono::high_resolution_clock::now();
  auto elapsed = now - epoch;
  m_timestamp = std::chrono::duration_cast<std::chrono::nanoseconds>(elapsed).count()/100u;
}




