#include "eudaq/DataConverterPlugin.hh"
#include "eudaq/StandardEvent.hh"
#include <string>

namespace eudaq{

    static const char* EVENT_TYPE = "TU";

    class TUConverterPlugin:public DataConverterPlugin{
        public:
            TUConverterPlugin():DataConverterPlugin(EVENT_TYPE){}
            virtual unsigned GetTriggerID(const eudaq::Event & ev) const{return ev.GetEventNumber();}
            
            virtual void Initialize(const Event & bore, const Configuration & cnf){
                //std::cout << bore.GetTag("FirmwareID") << std::endl;

            }

            virtual bool GetStandardSubEvent(eudaq::StandardEvent &sev, const eudaq::Event &ev) const{
                const RawDataEvent & in_raw = dynamic_cast<const RawDataEvent &>(ev);
                int valid = std::stoi(in_raw.GetTag("valid"));
                int nblocks = in_raw.NumBlocks();

                if(valid){
                    unsigned int id = 0;
                    RawDataEvent::data_t data = in_raw.GetBlock(id);
                     
                    unsigned long time_stamp = static_cast<unsigned long>(data[0]);
                    id++;
                    
                    data = in_raw.GetBlock(id);
                    unsigned int coincidence_count = static_cast<unsigned int>(data[0]);
                    id++;

                    data = in_raw.GetBlock(id);
                    unsigned int coincidence_count_no_sin = static_cast<unsigned int>(data[0]);
                    id++;

                    data = in_raw.GetBlock(id);
                    unsigned int prescaler_count = static_cast<unsigned int>(data[0]);
                    id++;

                    data = in_raw.GetBlock(id);
                    unsigned int prescaler_count_xor_pulser_count = static_cast<unsigned int>(data[0]);
                    id++;

                    data = in_raw.GetBlock(id);
                    unsigned int accepted_prescaled_events = static_cast<unsigned int>(data[0]);
                    id++;

                    data = in_raw.GetBlock(id);
                    unsigned int accepted_pulser_events = static_cast<unsigned int>(data[0]);
                    id++;

                    data = in_raw.GetBlock(id);
                    unsigned int handshake_count = static_cast<unsigned int>(data[0]);
                    id++;


                    data = in_raw.GetBlock(id);
                    unsigned int cal_beam_current = static_cast<unsigned int>(data[0]);
                    id++;


                    #ifdef DEBUG
                        std::cout << "nblocks: " << nblocks << std::endl;
                        std::cout << "time stamp: " << time_stamp << std::endl;
                        std::cout << "coincidence_count: " << coincidence_count << std::endl;
                        std::cout << "coincidence_count_no_sin: " << coincidence_count_no_sin << std::endl;
                        std::cout << "prescaler_count: " << prescaler_count << std::endl;
                        std::cout << "prescaler_count_xor_pulser_count: " << prescaler_count_xor_pulser_count << std::endl;
                        std::cout << "accepted_prescaled_events: " << accepted_prescaled_events << std::endl;
                        std::cout << "accepted_pulser_events: " << accepted_pulser_events << std::endl;
                        std::cout << "handshake_count: " << handshake_count << std::endl;
                        std::cout << "cal_beam_current: " << cal_beam_current << std::endl;
                    #endif

                    //add data to the StandardEvent:
                    sev.SetTimestamp(time_stamp);
                    StandardTUEvent tuev(EVENT_TYPE);
                    tuev.SetValid(1);


                    //get individual scaler values
                    data = in_raw.GetBlock(id);
                    int data_size = data.size(); 
                    int n_samples = data_size/sizeof(unsigned long);
                    for(int idx=0; idx<n_samples; idx++){
                        unsigned long *sc_val = (unsigned long*)(&data[0]);
                        //std::cout << "some output" << sc_val[idx] << std::endl;
                        tuev.SetScalerValue(idx, sc_val[idx]);
                     }

                     tuev.SetTimeStamp(time_stamp);
                     tuev.SetCoincCount(coincidence_count);
                     tuev.SetPrescalerCountXorPulserCount(prescaler_count_xor_pulser_count);
                     tuev.SetAcceptedPrescaledEvents(accepted_prescaled_events);
                     tuev.SetCoincCountNoSin(coincidence_count_no_sin);
                     tuev.SetPrescalerCount(prescaler_count);
                     tuev.SetHandshakeCount(handshake_count);
                     tuev.SetBeamCurrent(cal_beam_current);
                     sev.AddTUEvent(tuev);
                }else{
                     StandardTUEvent tuev(EVENT_TYPE);
                     tuev.SetValid(0);
                     sev.AddTUEvent(tuev);
                }
                return true;
            }

        private:
            static TUConverterPlugin const m_instance;

        };

    TUConverterPlugin const TUConverterPlugin::m_instance;

}//namespace eudaq



