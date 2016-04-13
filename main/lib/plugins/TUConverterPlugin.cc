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
                
                int valid = std::stoi(in_raw.GetTag("valid"));
                if(valid){
                     int id = 0;
                     RawDataEvent::data_t data = in_raw.GetBlock(id);
                     int64_t time_stamp = static_cast<int64_t>(data[0]);
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
                     unsigned int handshake_count = static_cast<unsigned int>(data[0]);
                     id++;

                     data = in_raw.GetBlock(id);
                     unsigned int cal_beam_current = static_cast<unsigned int>(data[0]);
                     id++;

                     //add data to the StandardEvent:
                     sev.SetTimestamp(time_stamp);
                     StandardTUEvent tuev(EVENT_TYPE);
                     tuev.SetValid(1);
                     tuev.SetTimeStamp(time_stamp);
                     tuev.SetCoincCount(coincidence_count);
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



