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
                std::cout << bore.GetTag("FirmwareID") << std::endl;

            }
            virtual bool GetStandardSubEvent(eudaq::StandardEvent &sev, const eudaq::Event &ev) const{
                const RawDataEvent & in_raw = dynamic_cast<const RawDataEvent &>(ev);
                std::cout << "plugin sucker ." << std::endl;
                int valid = std::stoi(in_raw.GetTag("valid"));
                int nblocks = in_raw.NumBlocks();
                std::cout << "nblocks: " << nblocks << std::endl;


                if(valid){
                    std::cout << "plugin sucker .." << std::endl;
                    unsigned int id = 0;
                    RawDataEvent::data_t data = in_raw.GetBlock(id);
                     
                    unsigned long time_stamp = static_cast<unsigned long>(data[0]);
                    id++;
                    std::cout << "time stamp: " << time_stamp << std::endl;
                    
                    data = in_raw.GetBlock(id);
                    unsigned int coincidence_count = static_cast<unsigned int>(data[0]);
                    id++;
                    std::cout << "coincidence_count: " << coincidence_count << std::endl;

                    data = in_raw.GetBlock(id);
                    unsigned int coincidence_count_no_sin = static_cast<unsigned int>(data[0]);
                    id++;
                    std::cout << "coincidence_count_no_sin: " << coincidence_count_no_sin << std::endl;

                    data = in_raw.GetBlock(id);
                    unsigned int prescaler_count = static_cast<unsigned int>(data[0]);
                    id++;
                    std::cout << "prescaler_count: " << prescaler_count << std::endl;

                    data = in_raw.GetBlock(id);
                    unsigned int prescaler_count_xor_pulser_count = static_cast<unsigned int>(data[0]);
                    id++;
                    std::cout << "prescaler_count_xor_pulser_count: " << prescaler_count_xor_pulser_count << std::endl;

                    data = in_raw.GetBlock(id);
                    unsigned int accepted_prescaled_events = static_cast<unsigned int>(data[0]);
                    id++;
                    std::cout << "accepted_prescaled_events: " << accepted_prescaled_events << std::endl;

                    data = in_raw.GetBlock(id);
                    unsigned int accepted_pulser_events = static_cast<unsigned int>(data[0]);
                    id++;
                    std::cout << "accepted_pulser_events: " << accepted_pulser_events << std::endl;

                    data = in_raw.GetBlock(id);
                    unsigned int handshake_count = static_cast<unsigned int>(data[0]);
                    id++;
                    std::cout << "handshake_count: " << handshake_count << std::endl;

                    data = in_raw.GetBlock(id);
                    unsigned int cal_beam_current = static_cast<unsigned int>(data[0]);
                    id++;
                    std::cout << "cal_beam_current: " << cal_beam_current << std::endl;

                    //add data to the StandardEvent:
                    sev.SetTimestamp(time_stamp);
                    StandardTUEvent tuev(EVENT_TYPE);
                    tuev.SetValid(1);

                    std::cout << "I get here." << std::endl;
                    //get individual scaler values
                    for(int idx=0; idx<10; idx++){
                        //data = in_raw.GetBlock(id);
                        std::cout << "I get here.." << std::endl;


                        unsigned long sc_val = 0;
                        std::cout << "scaler value " << idx << ": " << sc_val << std::endl;
                        tuev.SetScalerValue(idx, sc_val);
                        //id++;
                     }






                     tuev.SetTimeStamp(time_stamp);
                     tuev.SetCoincCount(coincidence_count);
                     tuev.SetPrescalerCountXorPulserCount(prescaler_count_xor_pulser_count);
                     tuev.SetAcceptedPrescaledEvents(accepted_prescaled_events);
                     tuev.SetCoincCountNoSin(coincidence_count_no_sin);
                     tuev.SetPrescalerCount(prescaler_count);
                     tuev.SetHandshakeCount(handshake_count);
                     tuev.SetBeamCurrent(cal_beam_current);
                     std::cout << "plugin sucker .." << std::endl;
                     sev.AddTUEvent(tuev);
                }else{
                     StandardTUEvent tuev(EVENT_TYPE);
                     tuev.SetValid(0);
                     std::cout << "plugin sucker .." << std::endl;
                     sev.AddTUEvent(tuev);
                }
                std::cout << "plugin sucker ..." << std::endl;
                return true;
            }

        private:
            static TUConverterPlugin const m_instance;

        };

    TUConverterPlugin const TUConverterPlugin::m_instance;

}//namespace eudaq



