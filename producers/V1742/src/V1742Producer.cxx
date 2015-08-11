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
#include "V1742Producer.hh"
#include "vme/CAEN/V1742_ADC/caen_v1742.h"


static const std::string EVENT_TYPE = "V1742";


V1742Producer::V1742Producer(const std::string & name, const std::string & runcontrol, const std::string & verbosity)  
  : eudaq::Producer(name, runcontrol),
  m_producerName(name),
  m_event_type(EVENT_TYPE),
  m_ev(0), 
  m_run(0), 
  m_running(false){

  if (V1742_handle)
    delete V1742_handle;
  
  try{
  std::cout << "###Initialize V1742 board:" << std::endl;  
  //initialize a pointer to the caen_v1742 class:
  vp717_interface = new VP717();
  interface = vp717_interface;
  vme_addr_t V1742_handleBaseAddress=0x32100000;
  V1742_handle = new caen_v1742(interface, V1742_handleBaseAddress);
  std::cout << "Handle to digitizer: " << V1742_handle << std::endl;
  
  //get serial number 
  m_serialno = V1742_handle->getSerialNumber();
  std::cout << "Serial number: " << m_serialno << std::endl;

  //get firmware revision
  caen_v1742::firmware_revision fw = V1742_handle->getROC_firmware();
  V1742_handle->printFirmware(fw);

  //get board model
  std::string board_model = V1742_handle->getBoardModel(); 
  std::cout << "VME connection to CAEN " << V1742_handle->getBoardModel() << " established." << std::endl;

  m_terminated = false;
  }
  catch (...){
    EUDAQ_ERROR(std::string("Error in the V1742Producer class constructor."));
    SetStatus(eudaq::Status::LVL_ERROR, "Error in the V1742Producer class constructor.");}
}



void V1742Producer::OnStartRun(unsigned runnumber){
  m_run = runnumber;
  m_ev = 0;
  try{
    //set time stamp:
    this->SetTimeStamp();

    //create event:
    std::cout<<"V1742: Create " << m_event_type << " EVENT for run " << m_run <<  " @time: " << m_timestamp << "." << std::endl;
    eudaq::RawDataEvent bore(eudaq::RawDataEvent::BORE(m_event_type, m_run));
    bore.SetTag("V1742_timestamp", m_timestamp);
    bore.SetTag("V1742_serial_no", m_serialno);
    bore.SetTag("V1742_firmware_v", m_firmware);
    
    for (unsigned ch = 0; ch <  V1742_handle->GROUPS; ch++){
        std::string key = "V1742_CHANNEL_DAC_"+std::to_string(ch);
	std::cout << key << " - " << m_channel_dac.at(ch);
	bore.SetTag(key, m_channel_dac.at(ch));
	
        key = "V1742_CHANNEL_RANGE_"+std::to_string(ch);
	std::cout << key << " - " << m_dynamic_range.at(ch);
	bore.SetTag(key, m_dynamic_range.at(ch));

        key = "V1742_CHANNEL_GAIN_"+std::to_string(ch);
	std::cout << key << " - " << m_channel_gain.at(ch);
	bore.SetTag(key, m_channel_gain.at(ch));
    }
    //set number of channels to be implemented
    //set tags for the channel numbers

    //buffer in event??

    //set digitizer online (allow it to accept triggers)
    V1742_handle->Start();

    V1742_handle->printAcquisitionStatus(V1742_handle->getAcquisitionStatus());

    SendEvent(bore);

    SetStatus(eudaq::Status::LVL_OK, "Running");
    m_running = true;
  }
  catch (...){
  EUDAQ_ERROR(std::string("Error in the V1742 OnStartRun procedure."));
  SetStatus(eudaq::Status::LVL_ERROR, "Error in the V1742 OnStartRun procedure.");}
}


void V1742Producer::OnStopRun(){
  // Break the readout loop
  m_running = false;
  V1742_handle->Stop();
  std::cout << "V1742 run stopped." << std::endl;
}


void V1742Producer::OnTerminate(){
  m_running = false;
  m_terminated = true;
  V1742_handle->Stop();
  V1742_handle->SoftwareReset(); //resets registers
  V1742_handle->SoftwareClear(); //clears buffers
  std::cout << "V1742 run terminated, registers reset and buffers cleared." << std::endl;
  delete vp717_interface, interface, V1742_handle;
}



void V1742Producer::ReadoutLoop() {
  while(!m_terminated){
    if(!m_running){
      sched_yield();} //if not running deprioritize thread


  while(m_running){
    try{
      if(V1742_handle->isEventReady()){ //check acquisition status register
        //set time stamp:
        this->SetTimeStamp();
        
        bool event_valid = true;
        v1742_event event; //CAEN event:


        V1742_handle->ReadEvent_D32(event);
        if(!event.isValid()){
          event_valid = false;
        }

        uint32_t event_counter = event.EventCounter();
        //std::cout << "#######" << std::endl;
        //std::cout << "Event valid:           " << event.isValid()           << std::endl;
        //std::cout << "Event size(32b words): " << event.EventSize()          << std::endl;
        //std::cout << "Channel mask:          " << event.ChannelMask()        << std::endl;
        std::cout << "\rEvent Counter:         " << event_counter << "/" << m_ev  << std::flush;
        //std::cout << "Samples per channel:   " << event.SamplesPerChannel()  << std::endl;
        //std::cout << "Channels:              " << event.Channels()           << std::endl;
        //std::cout << "TimeStamp:             " << event.TriggerTimeTag()     << std:: endl;
        //std::cout << std::endl << std::endl; 

        eudaq::RawDataEvent ev(m_event_type, m_run, event_counter); //generate a raw event
        ev.SetTimeStampToNow(); // Let's think weather this information can help us...
        ev.SetTag("TriggerTimeTag", event.TriggerTimeTag());

        unsigned int block_no = 0;
        ev.AddBlock(block_no, reinterpret_cast<const char*>(&event_valid), sizeof(bool)); //valid bit
        block_no++;

        ev.AddBlock(block_no, reinterpret_cast<const char*>(&m_timestamp), sizeof(m_timestamp)); //timestamp
        block_no++;


        ev.AddBlock(block_no, reinterpret_cast<const char*>(&m_ev), sizeof(uint32_t)); //event counter
        block_no++;


        //get information regarding the number and size of the events
        uint32_t n_channels = event.Channels();
        //std::cout << "Number of Channels: " << n_channels << std::endl;
        size_t n_samples = event.SamplesPerChannel();

        //read out all channels
        for(uint32_t ch = 0; ch < n_channels; ch++){
          uint16_t *payload = new uint16_t[n_samples];
          event.getChannelData(ch, payload, n_samples);
          //std::cout << "Size of readout - n_samples: " << n_samples << ", payload [byte]: " << n_samples*sizeof(uint16_t) << std::endl;
          ev.AddBlock(block_no, reinterpret_cast<const char*>(payload), n_samples*sizeof(uint16_t));
          block_no++;
          delete payload;
        }
        
        SendEvent(ev);
	      m_ev++;

        //std::cout << "Event counter (loop): " << m_ev << std::endl;      
      }//end if
      
    }  
    catch (...){
    EUDAQ_ERROR(std::string("Readout error................"));
    SetStatus(eudaq::Status::LVL_ERROR, "Readout error................");}

    }//running
  
  }//terminated
}




void V1742Producer::OnConfigure(const eudaq::Configuration& conf) {
  std::cout << "###Configure V1742 board:" << std::endl;  
  m_config = conf;

  m_trigger_source = m_config.Get("trigger_source", 1); //default 1 for external trigger
  m_active_channels = m_config.Get("active_channels", 1); 
  m_post_trigger_samples = m_config.Get("post_trigger_samples", 0); //default0 - 1 sample = 8.5ns
  m_trigger_threshold = m_config.Get("trigger_threshold", 1); //default 1
  


  try{
    if(V1742_handle->isRunning()){
      V1742_handle->Stop();}
    
    //set all configuration registers to their default values
    V1742_handle->SoftwareReset();
    sleep(1);

    //set trigger source
    caen_v1742::trigger_source_mask t_mask = V1742_handle->getTriggerSourceMask();
    if(m_trigger_source == 1){
      t_mask.ext_trigger = 1;
      t_mask.sw_trigger = 0;
    }
    else if(m_trigger_source == 0){
      t_mask.ext_trigger = 0;
      t_mask.sw_trigger = 1;
    }
    V1742_handle->setTriggerSourceMask(t_mask);
    V1742_handle->printTriggerMask(V1742_handle->getTriggerSourceMask());


    //enable channels/groups
    caen_v1742::group_enable_mask c_enable_mask = V1742_handle->getGroupEnableMask();
    if(m_active_channels == 1){
      c_enable_mask.group0 = 1;
    }
    if(m_active_channels == 2){
      c_enable_mask.group0 = 1;
      c_enable_mask.group1 = 1;
    }
    V1742_handle->setGroupEnableMask(c_enable_mask);
    V1742_handle->printGroupEnableMask(V1742_handle->getGroupEnableMask());


    //set post trigger samples:
    V1742_handle->setPostTrigger(m_post_trigger_samples);
    std::cout << "Post trigger samples set to: " << m_post_trigger_samples << std::endl;

    
    //send busy signal via the TRG OUT Limo connnector; print it?
    caen_v1742::front_panel_io_control_reg ctr_reg = V1742_handle->getFrontPanelIOControl();
    ctr_reg.force_trg_out_mode = 0; //force it or not, does not matter
    ctr_reg.trg_out_mode = 0; //trg_out is internal signal
    ctr_reg.trg_out_mode_select = 1; //select trg_out
    ctr_reg.motherboad_probe = 3; //allows roc to issue busy signal
    ctr_reg.busy_unlock = 0; //propagates it to trg_out
    V1742_handle->setFrontPanelIOControl(ctr_reg);


    caen_v1742::acq_control acq_ctrl = V1742_handle->getACQControl();
    acq_ctrl.trigger_count = 1; //cout ALL triggers
    acq_ctrl.buffer_mode = 1; //always keep one buffer free
    V1742_handle->setACQControl(acq_ctrl);

    //FIXME: set sampling frequency
    //FIXME: read out DRS4 temperature

    //set channel settings: threshold
    for(int gr = 0; gr < V1742_handle->GROUPS; gr++){
      V1742_handle->setGroup_Thres(gr, m_trigger_threshold);
      //DC offset?
      //calibration?
      V1742_handle->printGroupStatus(V1742_handle->getGroup_Status(0));
      float dac = V1742_handle->getGroup_DAC(0); //FIXME: hard-coded
      float range = 2; //FIXME: hard coded +/-1V
      float gain = 1; //FIXME: does not exis
    
      m_channel_dac[gr] = dac;
      m_dynamic_range[gr] = range;
      m_channel_gain[gr] = gain;

      std::cout << "Range group " << gr << ": " << range << " dac: " << dac << ", gain: " << gain << std::endl;   
    }

  std::cout << "V1742: Configured! Ready to take data." << std::endl;
  
  SetStatus(eudaq::Status::LVL_OK, "Configured V1742");

  }catch ( ... ){
  EUDAQ_ERROR(std::string("Error in the V1742 configuration procedure."));
   SetStatus(eudaq::Status::LVL_ERROR, "Error in the V1742 configuration procedure.");}
}





int main(int /*argc*/, const char ** argv){
  // You can use the OptionParser to get command-line arguments
  // then they will automatically be described in the help (-h) option
  eudaq::OptionParser op("V1742 Producer", "0.0", "Run options");
  eudaq::Option<std::string> rctrl(op, "r", "runcontrol",
    "tcp://localhost:44000", "address", "The address of the RunControl.");
  eudaq::Option<std::string> level(op, "l", "log-level", "NONE", "level",
    "The minimum level for displaying log messages locally");
  eudaq::Option<std::string> name (op, "n", "name", "V1742", "string",
    "The name of this Producer");
  eudaq::Option<std::string> verbosity(op, "v", "verbosity mode", "INFO", "string");

  try{
    // This will look through the command-line arguments and set the options
    op.Parse(argv);
    // Set the Log level for displaying messages based on command-line
    EUDAQ_LOG_LEVEL(level.Value());
    // Create a producer
    V1742Producer *producer = new V1742Producer(name.Value(), rctrl.Value(), verbosity.Value());
    // And set it running...
    producer->ReadoutLoop();
    // When the readout loop terminates, it is time to go
    std::cout << "V1742: Quitting" << std::endl;

  }
  catch(...){
    // This does some basic error handling of common exceptions
    return op.HandleMainException();}

  return 0;
}


void V1742Producer::SetTimeStamp(){
  std::chrono::high_resolution_clock::time_point epoch;
  auto now = std::chrono::high_resolution_clock::now();
  auto elapsed = now - epoch;
  m_timestamp = std::chrono::duration_cast<std::chrono::nanoseconds>(elapsed).count()/100u;
}




