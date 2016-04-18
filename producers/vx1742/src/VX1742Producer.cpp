/* ---------------------------------------------------------------------------------
** CAEN VX1742 implementation into the EUDAQ framework
** 
**
** <VX1742Producer>.cpp
** 
** Date: April 2016
** Author: Christian Dorfer (dorfer@phys.ethz.ch)
** ---------------------------------------------------------------------------------*/

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
#include "VX1742Producer.hh"


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
  std::cout << "###Configure VX1742 board with: " << m_config.Name() << std::endl;  
  
  try{
    sampling_frequency = conf.Get("sampling_frequency", 0);
    post_trigger_samples = conf.Get("post_trigger_samples", 0);
    trigger_source = conf.Get("trigger_source", 0);
    groups[0] = conf.Get("group1", 1);
    groups[1] = conf.Get("group2", 1);
    groups[2] = conf.Get("group3", 1);
    groups[3] = conf.Get("group4", 1);

    if(caen->isRunning())
      caen->stopAcquisition();

    caen->softwareReset();
    usleep(1000000);

    caen->setSamplingFrequency(sampling_frequency);
    caen->setPostTriggerSamples(post_trigger_samples);
    caen->setTriggerSource(trigger_source);
    caen->toggleGroups(groups);

    //send busy signal via the TRG OUT Limo connnector
    caen->sendBusyToTRGout();
    caen->setTriggerCount(); //count all, not just accepted triggers


    //individual group configuration

    //continue here...



 


/*



    //set channel settings: threshold
    for(int gr = 0; gr < VX1742_handle->GROUPS; gr++){
      VX1742_handle->setGroup_Thres(gr, m_trigger_threshold);
      //DC offset?
      //calibration?
      //VX1742_handle->printGroupStatus(VX1742_handle->getGroup_Status(0));
      float dac = VX1742_handle->getGroup_DAC(0); //FIXME: hard-coded
      float range = 2; //FIXME: hard coded +/-1V
      float gain = 1; //FIXME: does not exis
    
      m_channel_dac[gr] = dac;
      m_dynamic_range[gr] = range;
      m_channel_gain[gr] = gain;

      //std::cout << "Range group " << gr << ": " << range << " dac: " << dac << ", gain: " << gain << std::endl;   
    */
    

  std::cout << "VX1742: Configured! Ready to take data." << std::endl << std::endl;
  
  SetStatus(eudaq::Status::LVL_OK, "Configured VX1742 (" + m_config.Name() +")");

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
    for (unsigned ch = 0; ch <  VX1742_handle->GROUPS; ch++){
        std::string key = "VX1742_CHANNEL_DAC_"+std::to_string(ch);
	      //std::cout << key << " - " << m_channel_dac.at(ch);
	      bore.SetTag(key, m_channel_dac.at(ch));
	
        key = "VX1742_CHANNEL_RANGE_"+std::to_string(ch);
	      //std::cout << key << " - " << m_dynamic_range.at(ch);
	      bore.SetTag(key, m_dynamic_range.at(ch));

        key = "VX1742_CHANNEL_GAIN_"+std::to_string(ch);
	      //std::cout << key << " - " << m_channel_gain.at(ch);
	      bore.SetTag(key, m_channel_gain.at(ch));
    }
    */
    //set number of channels to be implemented
    //set tags for the channel numbers


    caen->clearBuffers();
    caen->startAcquisition();

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
      
  std::cout << "Function prototype" << std::endl;
      //end if
      
    }catch (...){
    EUDAQ_ERROR(std::string("Readout error................"));
    SetStatus(eudaq::Status::LVL_ERROR, "Readout error................");}

    }//running
  }}






void VX1742Producer::OnStopRun(){
  m_running = false;
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


/*//put in software trigger
      //VX1742_handle->SoftwareTrigger();
      //sleep(1);
      /*
      if(VX1742_handle->isEventReady()){ //check acquisition status register
        //set time stamp:
        //uint32_t events_stored = VX1742_handle->getEventsStored();
        this->SetTimeStamp();
        //sleep(1);
        
        bool event_valid = true;
        //VX1742_event event; //CAEN event:


        //VX1742_handle->ReadEvent_D32(event);
        //if(!event.isValid()){
        //  event_valid = false;
       // }

        //uint32_t event_counter = event.EventCounter();

        
        std::cout << "Events stored:            " << events_stored << std::endl;
        std::cout << "Event valid:              " << event.isValid() << std::endl;
        std::cout << "Event size (32bit words): " << event.EventSize() << std::endl;
        std::cout << "BoardID:                  " << event.BoardID() << std::endl;
        std::cout << "Group mask:               " << event.GroupMask() << std::endl;
        //std::cout << "\rEvent Counter:          " << event_counter << "/" << m_ev  << std::flush;
        std::cout << "Event Counter:            " << event_counter << "/" << m_ev  <<  std::endl;
        std::cout << "Samples per channel:      " << event.SamplesPerChannel()  << std::endl;
        std::cout << "Channels:                 " << event.Channels()           << std::endl;
        std::cout << "TimeStamp:                " << event.TriggerTimeTag()     << std:: endl;
        std::cout << std::endl << std::endl; 
        

  //if(m_ev>1){
    //uint16_t *ar1=new uint16_t(1024);
    size_t dum=event.getChannelData(0,1,ar1,1024);

    for(int x=0;x<1024;x++){
      std::cout << "Channel Data (i="<<x<<"):        "<< ar1[x] << std::endl;
      }
  //}
  std::cout<<"m_event_type: "<<m_event_type<<"\tm_run: "<<m_run<<"\tevent_counter: "<<event_counter<<std::endl;
        eudaq::RawDataEvent ev(m_event_type, m_run, event_counter); //generate a raw event
        std::cout << "0" << std::endl;  
        ev.SetTimeStampToNow(); // Let's think whether this information can help us...        
  std::cout << "1" << std::endl;  
        //ev.SetTag("TriggerTimeTag", event.TriggerTimeTag());

        std::cout << "2" << std::endl;  

        unsigned int block_no = 0;
        ev.AddBlock(block_no, reinterpret_cast<const char*>(&event_valid), sizeof(bool)); //valid bit
        block_no++;
        std::cout << "3" << std::endl;  
        ev.AddBlock(block_no, reinterpret_cast<const char*>(&m_timestamp), sizeof(m_timestamp)); //timestamp
        block_no++;
        std::cout << "4" << std::endl;  
        ev.AddBlock(block_no, reinterpret_cast<const char*>(&m_ev), sizeof(uint32_t)); //event counter
        block_no++;

        //get information regarding the number and size of the events
        //uint32_t n_channels = event.Channels();
        //std::cout << "Number of Channels: " << n_channels << std::endl;
        //size_t n_samples = event.SamplesPerChannel();

        //read out all channels
        for(uint32_t ch = 0; ch < n_channels; ch++){
          uint16_t *payload = new uint16_t[n_samples];
          //event.getChannelData(ch, payload, n_samples);
          //std::cout << "Size of readout - n_samples: " << n_samples << ", payload [byte]: " << n_samples*sizeof(uint16_t) << std::endl;
          ev.AddBlock(block_no, reinterpret_cast<const char*>(payload), n_samples*sizeof(uint16_t));
          block_no++;
          delete payload;
        }
        SendEvent(ev);
  m_ev++;

        //std::cout << "Event counter (loop): " << m_ev << std::endl;      
        */

