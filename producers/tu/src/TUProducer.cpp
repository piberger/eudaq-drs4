/* ---------------------------------------------------------------------------------
** OSU Trigger Logic Unit EUDAQ Implementation
** 
**
** <TUProducer>.cpp
** 
** Date: March 2016
** Author: Christian Dorfer (dorfer@phys.ethz.ch)
** ---------------------------------------------------------------------------------*/

//TU includes
#include "TUProducer.hh"
#include "trigger_controll.h"

//EUDAQ includes
#include "eudaq/Utils.hh"
#include "eudaq/Logger.hh"
#include "eudaq/OptionParser.hh"
#include "eudaq/RawDataEvent.hh"
#include "eudaq/Configuration.hh"

//System Includes
#include <iostream>
#include <string>
#include <cstdint>

static const std::string EVENT_TYPE = "TU";

TUProducer::TUProducer(const std::string &name, const std::string &runcontrol, const std::string &verbosity):eudaq::Producer(name, runcontrol),
	event_type(EVENT_TYPE),
	m_run(0), m_ev(0),
	done(false), TUStarted(false), TUJustStopped(false),
	tc(NULL), stream(NULL){

    try{
    	std::fill_n(trigger_counts, 10, 0);
    	std::fill_n(prev_trigger_counts, 10, 0);
		std::fill_n(input_frequencies, 10, 0);
    	std::fill_n(trigger_counts_multiplicity, 10, 0);
    	std::fill_n(time_stamps, 2, 0);
    	std::fill_n(beam_current, 2, 0);
    	avg.assign(20,0);
    	tc = new trigger_controll();
    	stream = new Triger_Logic_tpc_Stream();
    }catch (...){
    EUDAQ_ERROR(std::string("Error in the TUProducer class constructor."));
    SetStatus(eudaq::Status::LVL_ERROR, "Error in the TUProducer class constructor.");}
}


void TUProducer::MainLoop(){
	//variables for trigger-fallover check:
	const unsigned int bit_28 = 268435456; //28bit counter
	const unsigned int limit = bit_28/2;

	do{
	  	if(!tc){
			eudaq::mSleep(500);
			continue;}

		if(TUJustStopped){
			tc->enable(false);
			eudaq::mSleep(1000);
		}
			    	
		if(TUStarted || TUJustStopped){
			eudaq::mSleep(500); //only read out every 1/2 second

			Readout_Data *rd;
			rd = stream->timer_handler();
			if (rd){

				/******************************************** start get all the data ********************************************/
				coincidence_count_no_sin = rd->coincidence_count_no_sin;
				coincidence_count = rd->coincidence_count;
				prescaler_count = rd->prescaler_count;
				prescaler_count_xor_pulser_count = rd->prescaler_count_xor_pulser_count;
				accepted_prescaled_events = rd->prescaler_xor_pulser_and_prescaler_delayed_count;
				accepted_pulser_events = rd->pulser_delay_and_xor_pulser_count;
				handshake_count = rd->handshake_count; //equivalent to event count
				beam_current[0] = beam_current[1]; //save old beam current monitor count for frequency calculations
				beam_current[1] = rd->beam_curent; //save new
				time_stamps[0] = time_stamps[1]; //save old timestamp for frequency calculations
				time_stamps[1] = rd->time_stamp; //save new
				cal_beam_current = SlidingWindow(0.01*((beam_current[1]-beam_current[0])/(time_stamps[1] - time_stamps[0])));

				for(int idx=0; idx<10; idx++){
					//check if there was a fallover
					unsigned int new_tc = rd->trigger_counts[idx]; //data from readout
					unsigned int old_tc = trigger_counts[idx]%bit_28; //data from previous readout	

					//check for fallover
					if (old_tc > limit && new_tc < limit){
						trigger_counts_multiplicity[idx]++;
					}

					prev_trigger_counts[idx] = trigger_counts[idx];
					trigger_counts[idx] = trigger_counts_multiplicity[idx]*bit_28 + new_tc;
					input_frequencies[idx] = 1000*(trigger_counts[idx] - prev_trigger_counts[idx])/(time_stamps[1] - time_stamps[0]);
				}
				/******************************************** end get all the data ********************************************/

				//check if eventnumber has changed since last readout:
				if(handshake_count != prev_handshake_count){
				
                        std::cout << "************************************************************************************" << std::endl;
                        std::cout << "time stamp: " << time_stamps[1] << std::endl;
                        std::cout << "coincidence_count: " << coincidence_count << std::endl;
                        std::cout << "coincidence_count_no_sin: " << coincidence_count_no_sin << std::endl;
                        std::cout << "prescaler_count: " << prescaler_count << std::endl;
                        std::cout << "prescaler_count_xor_pulser_count: " << prescaler_count_xor_pulser_count << std::endl;
                        std::cout << "accepted_prescaled_events: " << accepted_prescaled_events << std::endl;
                        std::cout << "accepted_pulser_events: " << accepted_pulser_events << std::endl;
                        std::cout << "handshake_count: " << handshake_count << std::endl;
                        std::cout << "cal_beam_current: " << cal_beam_current << std::endl;
                        std::cout << "************************************************************************************" << std::endl;
					
					
					//send fake events for events we are missing out between readout cycles
					if(handshake_count > m_ev_prev){
						int tmp =0;
						for (unsigned int id = m_ev_prev; id < handshake_count; id++){
							eudaq::RawDataEvent f_ev(event_type, m_run, id);
							f_ev.SetTag("valid", std::to_string(0));
							SendEvent(f_ev);
							tmp++;
							//std::cout << "----- fake event " << id << " sent." << std::endl;
						}
					}

					uint64_t ts = time_stamps[1];
					m_ev = handshake_count;
					
					//real data event
					eudaq::RawDataEvent ev(event_type, m_run, m_ev); //generate a raw event
					ev.SetTag("valid", std::to_string(1));

        			unsigned int block_no = 0;
        			ev.AddBlock(block_no, static_cast<const uint64_t*>(&ts), sizeof(uint64_t)); //timestamp
        			block_no++;
        			ev.AddBlock(block_no, static_cast<const uint32_t*>(&coincidence_count), sizeof(uint32_t));
        			block_no++;
        			ev.AddBlock(block_no, static_cast<const uint32_t*>(&coincidence_count_no_sin), sizeof(uint32_t));
        			block_no++;
        			ev.AddBlock(block_no, static_cast<const uint32_t*>(&prescaler_count), sizeof(uint32_t));
        			block_no++;
        			ev.AddBlock(block_no, static_cast<const uint32_t*>(&prescaler_count_xor_pulser_count), sizeof(uint32_t));
        			block_no++;
        			ev.AddBlock(block_no, static_cast<const uint32_t*>(&accepted_prescaled_events), sizeof(uint32_t));
        			block_no++;
        			ev.AddBlock(block_no, static_cast<const uint32_t*>(&accepted_pulser_events), sizeof(uint32_t));
        			block_no++;
        			ev.AddBlock(block_no, static_cast<const uint32_t*>(&handshake_count), sizeof(uint32_t));
        			block_no++;
        			ev.AddBlock(block_no, static_cast<const float*>(&cal_beam_current), sizeof(float));
        			block_no++;

        			//also send individual event scalers
        			unsigned long *cnts = trigger_counts;
        			ev.AddBlock(block_no, reinterpret_cast<const char*>(cnts), 10*sizeof(uint64_t));

        			SendEvent(ev);

        			m_ev_prev = m_ev+1;
					prev_handshake_count = handshake_count;
					std::cout << "Event number TUProducer: " << handshake_count << std::endl;
				
				}//end if (prev event count)

			}//end if(rd)

		}//end if(TUStarted)


		if(TUJustStopped){
		    SendEvent(eudaq::RawDataEvent::EORE(event_type, m_run, m_ev));
			OnStatus();
			TUJustStopped = false;
		}
	}while (!done);
}




void TUProducer::OnStartRun(unsigned run_nr) {
	try{
		SetStatus(eudaq::Status::LVL_OK, "Wait");
		eudaq::mSleep(5000);
		m_run = run_nr;
		m_ev = 0;

		std::cout << "--> Starting TU Run." << std::endl;

		eudaq::RawDataEvent ev(eudaq::RawDataEvent::BORE(event_type, m_run));
		ev.SetTag("FirmwareID", "not given"); //FIXME!
		ev.SetTag("TriggerMask", "0x" + eudaq::to_hex(trg_mask));
		ev.SetTimeStampToNow();
		SendEvent(ev);

		std::cout << "Sent BORE event." << std::endl;

		OnReset();
		TUStarted = true;
		tc->enable(true);

        EUDAQ_INFO("Triggers are now accepted");
		SetStatus(eudaq::Status::LVL_OK, "Started");
	}catch(const std::exception & e){
		printf("Caught exception: %s\n", e.what());
		SetStatus(eudaq::Status::LVL_ERROR, "Start Error");
	}catch(...){
		printf("Unknown exception\n");
		SetStatus(eudaq::Status::LVL_ERROR, "Start Error");
	}
}



void TUProducer::OnStopRun(){
	std::cout << "--> Stopping TU Run." << std::endl;	
	try{
		tc->enable(false);
		TUStarted = false;
		TUJustStopped = true;
		eudaq::mSleep(2000);
		OnReset();
		SetStatus(eudaq::Status::LVL_OK, "Stopped");

	}catch(const std::exception & e){
		printf("Caught exception: %s\n", e.what());
		SetStatus(eudaq::Status::LVL_ERROR, "Stop Error");
	} catch (...) {
		printf("Unknown exception\n");
		SetStatus(eudaq::Status::LVL_ERROR, "Stop Error");
		}
}



void TUProducer::OnTerminate(){
		try{
		std::cout << "--> Terminate (press enter)" << std::endl;
		tc->enable(false);
		tc->reset_counts();
		done = true;
		delete tc, stream; //clean up
		eudaq::mSleep(1000);
		}catch(const std::exception & e){
			printf("Caught exception: %s\n", e.what());
			SetStatus(eudaq::Status::LVL_ERROR, "Terminate Error");
		}catch(...){
			printf("Unknown exception\n");
			SetStatus(eudaq::Status::LVL_ERROR, "Terminate Error");
		}
}



void TUProducer::OnReset(){
		try{
			std::cout << "--> Resetting TU." << std::endl;
			tc->reset_counts();
			std::fill_n(trigger_counts, 10, 0);
			std::fill_n(prev_trigger_counts, 10, 0);
			std::fill_n(input_frequencies, 10, 0);
    		std::fill_n(trigger_counts_multiplicity, 10, 0);
    		std::fill_n(time_stamps, 2, 0);
    		std::fill_n(beam_current, 2, 0);
    		avg.assign(20,0);
			coincidence_count_no_sin = 0;
			coincidence_count = 0;
			cal_beam_current = 0;
			prescaler_count = 0;
			prescaler_count_xor_pulser_count = 0;
			accepted_pulser_events = 0;
			handshake_count = 0;
			prev_handshake_count = 99999;
			m_ev_prev = 0;
			SetStatus(eudaq::Status::LVL_OK);
		}catch(const std::exception & e){
			printf("Caught exception: %s\n", e.what());
			SetStatus(eudaq::Status::LVL_ERROR, "Reset Error");
		}catch(...){
			printf("Unknown exception\n");
			SetStatus(eudaq::Status::LVL_ERROR, "Reset Error");
		}
}



void TUProducer::OnStatus(){
	if(TUStarted && handshake_count > 0)
		m_status.SetTag("COINC_COUNT", std::to_string(handshake_count));
	else 
		m_status.SetTag("TRIG", std::to_string(0));

	if (tc){
		m_status.SetTag("TIMESTAMP", std::to_string(time_stamps[1]/1000.0));
		m_status.SetTag("LASTTIME", std::to_string((time_stamps[0])/1000.0));

		if(TUStarted)
			m_status.SetTag("STATUS", "OK");
		else
			m_status.SetTag("STATUS", "NOT RUNNING");

		for (int i = 0; i < 10; ++i) {
			m_status.SetTag("SCALER" + std::to_string(i), std::to_string(input_frequencies[i]));
		}

		if(cal_beam_current > 0)
			m_status.SetTag("BEAM_CURR", std::to_string(cal_beam_current));
		else
			m_status.SetTag("BEAM_CURR", std::to_string(0));
	}
}


void TUProducer::OnConfigure(const eudaq::Configuration& conf) {

	try {

		SetStatus(eudaq::Status::LVL_OK, "Wait");

		//open stream to TU
		std::string ip_adr = conf.Get("ip_adr", "192.168.1.120");
		const char *ip = ip_adr.c_str();

		stream->set_ip_adr(ip);
   		std::cout << "Opening connection to TU @ " << ip << std::endl;
   		stream->open();


   		std::cout << "Configuring (" << conf.Name() << ").." << std::endl;			
  		//enabling/disabling and getting&setting delays for scintillator and planes 1-8 (same order in array)
  		std::cout << "--> Setting delays for scintillator, planes 1-8 and pad." << std::endl;
		tc->set_scintillator_delay(conf.Get("scintillator_delay", 100));
  		tc->set_plane_1_delay(conf.Get("plane1del", 100));
  		std::cout << "Debug: Plane 1 delay: " << conf.Get("plane_1", 100) << std::endl;
  		tc->set_plane_2_delay(conf.Get("plane2del", 100));
  		tc->set_plane_3_delay(conf.Get("plane3del", 100));
  		tc->set_plane_4_delay(conf.Get("plane4del", 100));
  		tc->set_plane_5_delay(conf.Get("plane5del", 100));
  		tc->set_plane_6_delay(conf.Get("plane6del", 100));
  		tc->set_plane_7_delay(conf.Get("plane7del", 100));
  		tc->set_plane_8_delay(conf.Get("plane8del", 100));
  		tc->set_pad_delay(conf.Get("pad_delay", 100));
  		tc->set_delays();

  		//generate and set a trigger mask
  		trg_mask = conf.Get("pad", 0);
  		for (unsigned int idx=8; idx>0; idx--){
  			std::string sname = "plane" + std::to_string(idx);
  			int tmp = conf.Get(sname, 0);
  			trg_mask = (trg_mask<<1)+tmp;
  		}

  		trg_mask = (trg_mask<<1)+conf.Get("scintillator", 0);
  		std::cout << "Debug: Trigger mask: " << trg_mask << std::endl;
  		tc->set_coincidence_enable(trg_mask);


  		std::cout << "--> Setting prescaler and delay." << std::endl;
		int scal = conf.Get("prescaler", 1);
		int predel = conf.Get("prescaler_delay", 5); //must be >4
		tc->set_prescaler(scal);
		tc->set_prescaler_delay(predel);


		std::cout << "--> Setting pulser frequency, width and delay." << std::endl;
		double freq = conf.Get("pulser_freq", 0);
		int width = conf.Get("pulser_width", 0);
		int puldel = conf.Get("pulser_delay", 5); //must be > 4

		tc->set_Pulser_width(freq, width);
		tc->set_pulser_delay(puldel);


		std::cout << "--> Setting coincidence pulse and edge width." << std::endl;
		int copwidth = conf.Get("coincidence_pulse_width", 10);
		int coewidth = conf.Get("coincidence_edge_width", 10);
		tc->set_coincidence_pulse_width(copwidth);
		tc->set_coincidence_edge_width(coewidth);
		tc->send_coincidence_edge_width();
		tc->send_coincidence_pulse_width();

		int hs_del = conf.Get("handshake_delay", 0);
		tc->set_handshake_delay(hs_del);
		tc->send_handshake_delay();

		int hs_mask = conf.Get("handshake_mask", 0);
		tc->set_handshake_mask(hs_mask);
		tc->send_handshake_mask();

		int trigdel1 = conf.Get("trig_1_delay", 100);
		int trigdel2 = conf.Get("trig_2_delay", 100);
		int trigdel12 = (trigdel1<<12) | trigdel2;
		tc->set_trigger_12_delay(trigdel12);
				

		int trigdel3 = conf.Get("trig_3_delay", 100);
		tc->set_trigger_3_delay(trigdel3);
		tc->set_delays();

		//set current UNIX timestamp
   		tc->set_time();
		eudaq::mSleep(1000);
		SetStatus(eudaq::Status::LVL_OK, "Configured (" + conf.Name() + ")");
		
		std::cout << "--> Readback of values" << std::endl;
		std::cout << "############################################################" << std::endl;
		std::cout << "Scintillator delay [ns]: " << tc->get_scintillator_delay()*2.5 << std::endl;
		std::cout << "Plane 1 delay [ns]: " << tc->get_plane_1_delay()*2.5 << std::endl;
		std::cout << "Plane 2 delay [ns]: " << tc->get_plane_2_delay()*2.5 << std::endl;
		std::cout << "Plane 3 delay [ns]: " << tc->get_plane_3_delay()*2.5 << std::endl;
		std::cout << "Plane 4 delay [ns]: " << tc->get_plane_4_delay()*2.5 << std::endl;
		std::cout << "Plane 5 delay [ns]: " << tc->get_plane_5_delay()*2.5 << std::endl;
		std::cout << "Plane 6 delay [ns]: " << tc->get_plane_6_delay()*2.5 << std::endl;
		std::cout << "Plane 7 delay [ns]: " << tc->get_plane_7_delay()*2.5 << std::endl;
		std::cout << "Plane 8 delay [ns]: " << tc->get_plane_8_delay()*2.5 << std::endl;
		std::cout << "Pad delay [ns]: " << tc->get_pad_delay()*2.5 << std::endl << std::endl;

		std::cout << "Handshake delay [ns]: " << tc->get_handshake_delay()*2.5 << std::endl;
		std::cout << "Handshake mask: " << tc->get_handshake_mask() << std::endl << std::endl;

		std::cout << "Coincidence pulse width [ns]: " << tc->get_coincidence_pulse_width() << std::endl;
		std::cout << "Coincidence edge width [ns]: " << tc->get_coincidence_edge_width() << std::endl << std::endl;

		std::cout << "--> ##### Configuring TU with settings file (" << conf.Name() << ") done. #####" << std::endl;

		SetStatus(eudaq::Status::LVL_OK, "Configured (" + conf.Name() + ")");
	}catch (...){
		printf("Configuration Error\n");
		SetStatus(eudaq::Status::LVL_ERROR, "Configuration Error");
	}
}




float TUProducer::SlidingWindow(float val){
	avg.pop_back();
	avg.push_front(val);
	
	int len = 0;
	float temp = 0;
	for (std::deque<float>::iterator itr=avg.begin(); itr!=avg.end(); ++itr){
		if (*itr != 0){			
			temp += *itr;
			len++;
		}
	}
	return (1.0*temp/len);
}




int main(int /*argc*/, const char ** argv) {
	//FIXME: get rid of useless options here and implement new ones
	eudaq::OptionParser op("EUDAQ TU Producer", "1.0", "The Producer for the Ohio State trigger unit.");
	eudaq::Option<std::string> rctrl(op, "r", "runcontrol", "tcp://localhost:44000", "address", "The address of the RunControl application");
	eudaq::Option<std::string> level(op, "l", "log-level", "NONE", "level", "The minimum level for displaying log messages locally");
	eudaq::Option<std::string> name (op, "n", "name", "TU", "string", "The name of this Producer");
	eudaq::Option<std::string> verbosity(op, "v", "verbosity mode", "INFO", "string");
	try {
		op.Parse(argv);
		EUDAQ_LOG_LEVEL(level.Value());
		//FIXME option parser missing
		TUProducer producer(name.Value(), rctrl.Value(), verbosity.Value());
		producer.MainLoop();
		std::cout << "Quitting" << std::endl;
		eudaq::mSleep(300);
	} catch (...) {
		return op.HandleMainException();
	}
	return 0;
}