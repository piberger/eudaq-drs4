#ifdef ROOT_FOUND

#include "eudaq/FileNamer.hh"
#include "eudaq/FileWriter.hh"
#include "eudaq/PluginManager.hh"
#include "eudaq/Logger.hh"
#include "eudaq/FileSerializer.hh"


#include "include/SimpleStandardEvent.hh"
#include <stdlib.h>

//# include<inttypes.h>
# include "TFile.h"
# include "TTree.h"
# include "TRandom.h"
# include "TString.h"
# include "TMath.h"
# include "TH1F.h"
#include "TSystem.h"
#include "TInterpreter.h"
#include <TROOT.h>

using namespace std;


template <typename T>
inline T unpack_fh (vector <unsigned char >::iterator &src, T& data){ //unpack from host-byte-order
    data=0;
    for(unsigned int i=0;i<sizeof(T);i++){
        data+=((uint64_t)*src<<(8*i));
        src++;
    }
    return data;
}

template <typename T>
inline T unpack_fn(vector<unsigned char>::iterator &src, T& data){            //unpack from network-byte-order
    data=0;
    for(unsigned int i=0;i<sizeof(T);i++){
        data+=((uint64_t)*src<<(8*(sizeof(T)-1-i)));
        src++;
    }
    return data;
}

template <typename T>
inline T unpack_b(vector<unsigned char>::iterator &src, T& data, unsigned int nb){            //unpack number of bytes n-b-o only
    data=0;
    for(unsigned int i=0;i<nb;i++){
        data+=(uint64_t(*src)<<(8*(nb-1-i)));
        src++;
    }
    return data;
}

typedef pair<vector<unsigned char>::iterator,unsigned int> datablock_t;


namespace eudaq {
	
	class FileWriterTreeDRS4 : public FileWriter {
    public:
        FileWriterTreeDRS4(const std::string &);
        virtual void StartRun(unsigned);
        virtual void Configure();
        virtual void WriteEvent(const DetectorEvent &);
        virtual uint64_t FileBytes() const;
        float Calculate(std::vector<float> * data, int min, int max, bool _abs=false);
        float CalculatePeak(std::vector<float> * data, int min, int max);
        float CalculateMedian(std::vector<float> * data, int min, int max);
        std::pair<int, float> FindMaxAndValue(std::vector<float> * data, int min, int max);
        float avgWF(float, float, int);
        virtual ~FileWriterTreeDRS4();
    private:
        pair<int,int> pulser_range;
        pair<int,int> signal_range;
        pair<int,int> pedestal_range;
        int save_waveforms;
        void ClearVectors();
        TFile * m_tfile; // book the pointer to a file (to store the otuput)
        TTree * m_ttree; // book the tree (to store the needed event info)
        // Book variables for the Event_to_TTree conversion
        unsigned m_noe;
        short chan;
        int n_pixels;
        std::map<std::string,std::pair<float,float> > ranges;

        // Scalar Branches     
        int   f_nwfs;
        int   f_event_number;
        float f_time;
        
        int   f_pulser;
        float f_pulser_int;
        int   f_trig_time;
        
        // Vector Branches     
        // DUT
        std::vector< std::string >  * v_sensor_name;
        std::vector< std::string >  * v_type_name;
        std::vector<float>  * v_sig_peak;
        std::vector<float>  *  v_peaktime         ;
        std::vector<float>  * v_sig;
        std::vector<float>  * v_sig_int;
        std::vector<float>  * v_sig_time;
        std::vector<float>  *  v_sig_spread       ;
        std::vector<float>  * v_sig_integral9;
        std::vector<float>  * v_sig_integral27;
        std::vector<float>  * v_sig_integral54;
        std::vector<float>  *  v_sig_static       ;
        std::vector<float>  * v_ped;
        std::vector<float>  * v_ped_int;
        std::vector<float>  *  v_ped_spread       ;
        std::vector<float>  *  v_ped_median       ;
        std::vector<float>  * v_pul;
        std::vector<float>  * v_pul_int;
        std::vector<float>  *  v_pul_spread	      ;

        std::vector<bool>  	* v_is_saturated;
        
        std::vector<float> * f_wf0;
        std::vector<float> * f_wf1;
        std::vector<float> * f_wf2;
        std::vector<float> * f_wf3;
        
        // TELESCOPE
        std::vector<int> * f_plane;
        std::vector<int> * f_col;
        std::vector<int> * f_row;
        std::vector<int> * f_adc;
        std::vector<int> * f_charge;  
        
        // average waveforms of channels
        TH1F * avgWF_0;
        TH1F * avgWF_1;
        TH1F * avgWF_2;
        TH1F * avgWF_3;

};

namespace {
static RegisterFileWriter<FileWriterTreeDRS4> reg("drs4tree");      
}

FileWriterTreeDRS4::FileWriterTreeDRS4(const std::string & /*param*/)
: m_tfile(0), m_ttree(0),m_noe(0),chan(4),n_pixels(90*90+60*60)
{
    gROOT->ProcessLine("#include <vector>");
    gROOT->ProcessLine(".L loader.C+");

    f_nwfs          =  0;
    f_event_number  = -1;
    f_time          = -1.;
    f_pulser        = -1;
    f_pulser_int    =  0.;
    f_trig_time     =  0.;

    // dut
    v_sensor_name = new std::vector< std::string >;
    v_type_name   = new std::vector< std::string >;
    v_ped       = new std::vector<float>;
    v_ped_int       = new std::vector<float>;
    v_pul       = new std::vector<float>;
    v_pul_int       = new std::vector<float>;
    v_sig       = new std::vector<float>;
    v_sig_int       = new std::vector<float>;
    v_sig_time  = new std::vector<float>;

    v_sensor_name       = new std::vector< std::string >;
    v_type_name         = new std::vector< std::string >;
    v_sig_peak          = new std::vector<float>;
    v_peaktime          = new std::vector<float>;
    v_sig_spread        = new std::vector<float>;
    v_sig_integral9     = new std::vector<float>;
    v_sig_integral27    = new std::vector<float>;
    v_sig_integral54    = new std::vector<float>;
    v_sig_static        = new std::vector<float>;
    v_ped_spread        = new std::vector<float>;
    v_ped_median        = new std::vector<float>;
    v_pul_spread        = new std::vector<float>;
    v_is_saturated  = new std::vector<bool>;

    f_wf0 = new std::vector<float>;
    f_wf1 = new std::vector<float>;
    f_wf2 = new std::vector<float>;
    f_wf3 = new std::vector<float>;

    // telescope
    f_plane  = new std::vector<int>;
    f_col    = new std::vector<int>;
    f_row    = new std::vector<int>;
    f_adc    = new std::vector<int>;
    f_charge = new std::vector<int>;

    // average waveforms of channels
    avgWF_0 = new TH1F("avgWF_0","avgWF_0", 1024, 0, 1024);
    avgWF_1 = new TH1F("avgWF_1","avgWF_1", 1024, 0, 1024);
    avgWF_2 = new TH1F("avgWF_2","avgWF_2", 1024, 0, 1024);
    avgWF_3 = new TH1F("avgWF_3","avgWF_3", 1024, 0, 1024);
}

void FileWriterTreeDRS4::Configure(){
    ranges["signal"] = make_pair(25,175);
    ranges["pedestal"] = make_pair(350,450);
    ranges["pulser"] = make_pair(760,860);
    if (!this->m_config)
        return;
    m_config->SetSection("Converter.drs4tree");
    if (m_config->NKeys()==0)
        return;
    EUDAQ_INFO("Configuring FileWriterTreeDRS4" );

    ranges["pulser"] = m_config->Get("pulser_range",make_pair((int)760,(int)860));
    EUDAQ_INFO("pulser_range: "+to_string(ranges["pulser"]));

    ranges["pedestal"] = m_config->Get("pedestal_range",make_pair((int)760,(int)860));
    EUDAQ_INFO("pedestal_range: "+to_string(ranges["pedestal"]));

    ranges["signal"] = m_config->Get("signal_range",make_pair((int)760,(int)860));
    EUDAQ_INFO("signal_range: "+to_string(ranges["signal"]));

    save_waveforms = m_config->Get("save_waveforms",0);
    EUDAQ_INFO("save_waveforms: "+ to_string(save_waveforms));
    for (UInt_t i = 0; i < 4; i++)
        std::cout<<"\tch"<<i<<":"<<to_string(((save_waveforms & 1<<i) == 1<<i));
    std::cout<<std::endl;
}

void FileWriterTreeDRS4::StartRun(unsigned runnumber) {
    EUDAQ_INFO("Converting the inputfile into a DRS4 TTree " );
    std::string foutput(FileNamer(m_filepattern).Set('X', ".root").Set('R', runnumber));
    EUDAQ_INFO("Preparing the outputfile: " + foutput);

    // ---------------------------------------------------------------------
    // the following line is needed to have std::vector<float> in the tree
    // ---------------------------------------------------------------------
    gROOT->ProcessLine("#include <vector>");

    m_tfile = new TFile(foutput.c_str(), "RECREATE");
    m_ttree = new TTree("tree", "a simple Tree with simple variables");

    // Set Branch Addresses
    m_ttree->Branch("event_number"  ,&f_event_number , "event_number/I");
    m_ttree->Branch("time"          ,&f_time         , "time/F");
    m_ttree->Branch("pulser"        ,&f_pulser       , "pulser/I");
    m_ttree->Branch("pulser_int"    ,&f_pulser_int   , "pulser_int/F");
    m_ttree->Branch("trig_time"     ,&f_trig_time    , "trig_time/I");
    m_ttree->Branch("nwfs"          , &f_nwfs        , "n_waveforms/I");

    if ((save_waveforms & 1<<0) == 1<<0)
        m_ttree->Branch("wf0" , &f_wf0);
    if ((save_waveforms & 1<<1) == 1<<1)
        m_ttree->Branch("wf1" , &f_wf1);
    if ((save_waveforms & 1<<2) == 1<<2)
        m_ttree->Branch("wf2" , &f_wf2);
    if ((save_waveforms & 1<<3) == 1<<3)
        m_ttree->Branch("wf3" , &f_wf3);

    // DUT
    m_ttree->Branch("v_is_saturated" ,  &v_is_saturated);
    // DUT-2
    m_ttree->Branch("v_sensor_name" ,   &v_sensor_name);
    m_ttree->Branch("v_type_name" ,     &v_type_name);
    m_ttree->Branch("v_peaktime" ,      &v_peaktime             );
    m_ttree->Branch("sig_time"    ,     &v_sig_time);
    m_ttree->Branch("sig"         ,     &v_sig);
    m_ttree->Branch("sig_int"         , &v_sig_int);
    m_ttree->Branch("v_sig_peak" ,      &v_sig_peak             );
    m_ttree->Branch("v_sig_spread",     &v_sig_spread           );
    m_ttree->Branch("v_sig_integral9",  &v_sig_integral9        );
    m_ttree->Branch("v_sig_integral27" ,&v_sig_integral27       );
    m_ttree->Branch("v_sig_integral54" ,&v_sig_integral54       );
    m_ttree->Branch("v_sig_static" ,    &v_sig_static           );

    m_ttree->Branch("v_ped_spread" ,    &v_ped_spread           );
    m_ttree->Branch("v_ped_median" ,    &v_ped_median           );
    m_ttree->Branch("ped"         ,     &v_ped);
    m_ttree->Branch("ped_int"         , &v_ped_int);

    m_ttree->Branch("v_pul_spread" ,    &v_pul_spread           );
    m_ttree->Branch("pul"         ,     &v_pul);
    m_ttree->Branch("pul_int"         , &v_pul_int);

    // telescope
    m_ttree->Branch("plane", &f_plane);
    m_ttree->Branch("col", &f_col);
    m_ttree->Branch("row", &f_row);
    m_ttree->Branch("adc", &f_adc);
    m_ttree->Branch("charge", &f_charge);

}

void FileWriterTreeDRS4::ClearVectors(){
    v_sensor_name->clear();
    v_type_name->clear();

    v_is_saturated->clear();

    v_pul->clear();
    v_pul_int->clear();
    v_pul_spread    ->clear();

    v_ped->clear();
    v_ped_int->clear();
    v_ped_spread    ->clear();
    v_ped_median    ->clear();
    v_sig->clear();
    v_sig_int->clear();
    v_sig_time->clear();
    v_sig_peak      ->clear();
    v_peaktime      ->clear();
    v_sig_spread    ->clear();
    v_sig_integral9 ->clear();
    v_sig_integral27->clear();
    v_sig_integral54->clear();
    v_sig_static    ->clear();

    f_wf0->clear();
    f_wf1->clear();
    f_wf2->clear();
    f_wf3->clear();

    f_plane->clear();
    f_col->clear();
    f_row->clear();
    f_adc->clear();
    f_charge->clear();
}

void FileWriterTreeDRS4::WriteEvent(const DetectorEvent & ev) {
    if (ev.IsBORE()) {
        eudaq::PluginManager::Initialize(ev);
        //firstEvent =true;
        cout << "loading the first event...." << endl;
        return;
    } else if (ev.IsEORE()) {
        cout << "loading the last event...." << endl;
        return;
    }
    StandardEvent sev = eudaq::PluginManager::ConvertToStandard(ev);

    f_event_number = sev.GetEventNumber();
    f_time = sev.GetTimestamp()/384066.;
    // --------------------------------------------------------------------
    // ---------- get the number of waveforms -----------------------------
    // --------------------------------------------------------------------
    unsigned int nwfs = (unsigned int) sev.NumWaveforms();
    f_nwfs = nwfs;

    if(f_event_number <= 10){
        cout << "event number " << f_event_number << endl;
        cout << "number of waveforms " << nwfs << endl;
        if(nwfs ==0){
            cout << "----------------------------------------" << endl;
            cout << "WARNING!!! NO WAVEFORMS IN THIS EVENT!!!" << endl;
            cout << "----------------------------------------" << endl;
        }
    }
    ClearVectors();

    // --------------------------------------------------------------------
    // ---------- verbosity level and some printouts ----------------------
    // --------------------------------------------------------------------
    int verbose = 0;

    if(verbose > 3) cout << "event number " << f_event_number << endl;
    if(verbose > 3) cout << "number of waveforms " << nwfs << endl;

    // --------------------------------------------------------------------
    // ---------- get and save all info for all waveforms -----------------
    // --------------------------------------------------------------------

    std::vector<float> * data;                           
    for (unsigned int iwf = 0; iwf < nwfs;iwf++){

        const eudaq::StandardWaveform & waveform = sev.GetWaveform(iwf);
        // get the sensor name. see eventually what this actually does!
        std::string type_name;
        type_name = waveform.GetType();

        std::string sensor_name;
        sensor_name = waveform.GetSensor();

        v_type_name->push_back(type_name);
        v_sensor_name->push_back(sensor_name);
        // SimpleStandardWaveform simpWaveform(sensorname,waveform.ID());
        // simpWaveform.setNSamples(waveform.GetNSamples());
        int n_samples = waveform.GetNSamples();
        if (verbose > 3) std::cout << "number of samples in my wf " << n_samples << std::endl;
        // load the waveforms into the vector
        data = waveform.GetData();
        // calculate the signal and so on
        // float sig = CalculatePeak(data, 1075, 1150);
        std::pair<int, float> maxAndValue =waveform.getAbsMaxAndValue(0,200);
        float signal   = waveform.getSpreadInRange( signal_range.first,  signal_range.second);
        float signal_integral   = waveform.getIntegral(signal_range.first,  signal_range.second);
        int signal_time = waveform.getIndexAbsMax(signal_range.first,  signal_range.second);
        float int_9             = waveform.getIntegral( maxAndValue.first-3, maxAndValue.first+6);
        float int_27            = waveform.getIntegral( maxAndValue.first-9, maxAndValue.first+18);
        float int_54            =waveform.getIntegral( maxAndValue.first-18, maxAndValue.first+36);
        float sig_static= waveform.getIntegral( 25, 175);
        float pedestal = waveform.getSpreadInRange(pedestal_range.first,  pedestal_range.second);
        float pedestal_integral   = waveform.getIntegral(pedestal_range.first,  pedestal_range.second);
        float pedestal_median   = waveform.getMedian( 350, 500);
        float median    = waveform.getMedian(300, 800);
        float pulser   = waveform.getSpreadInRange(pulser_range.first,  pulser_range.second);
        float pulser_integral   = waveform.getIntegral(pulser_range.first,  pulser_range.second);
        float signalSpread   = waveform.getSpreadInRange(signal_range.first,  signal_range.second);
        float abs_max           = waveform.getAbsMaxInRange(0,1023);


        // float mini = waveform.getMinInRange(10,1000);
        // float minind = waveform.getIndexMin(10,1000);
        // cout << "minimum of the waveform in range 10, 1000: " << mini << " at index " << minind<< endl;

        // ------------------------------------
        // -------- SIGNAL DEFINITIONS --------
        // ------------------------------------
        //float ped         = waveform.getIntegral( 300,  700);
        //                waveform.getIndexMin(25,125);
        // save the values in the event
        v_sig     ->push_back(signal);
        v_sig_int ->push_back(signal_integral);
        v_sig_time->push_back(signal_time); // this does not make sense yet!!

        v_ped->push_back(pedestal);
        v_ped_int->push_back(pedestal_integral);
        v_ped_median    ->push_back(pedestal_median);       // pedestrial: median in [350, 500]
        v_pul->push_back(pulser);
        v_pul_int->push_back(pulser_integral);

        v_sig_peak      ->push_back(maxAndValue.second);    // signal: peak in [0,200]
        v_peaktime      ->push_back(maxAndValue.first);     // time of peak
        v_sig_spread    ->push_back(signalSpread);          // signal: spread in [25,125]
        v_sig_integral9 ->push_back(int_9);                 // signal: integral [pk-3, pk+6]
        v_sig_integral27->push_back(int_27);                // signal: integral [pk-9, pk+18]
        v_sig_integral54->push_back(int_54);                // signal: integral [pk-18, pk+36]
        v_sig_static    ->push_back(sig_static);            // signal: static integral [25, 175]
        v_ped_spread    ->push_back(pedestal);              // pedestrial: spread in [350, 450]
        v_ped_median    ->push_back(median);                // pedestrial: median in [300, 800]
        v_pul_spread    ->push_back(pulser);                // pulser: spread in [760, 860]
        v_is_saturated -> push_back(abs_max<=498);

        if(iwf == 1){ // trigger WF
            for (int j=0; j<data->size(); j++){
                if( abs(data->at(j)) > 90. ) {f_trig_time = j; break;}
            }
        }
        if(iwf == 2){ // pulser WF
            f_pulser_int = waveform.getIntegral(500, 900, true);
            f_pulser     = (f_pulser_int > 20.);
        }

        for (int j=0; j<data->size(); j++){
            if     (iwf == 0) {
                avgWF_0->SetBinContent(j+1, avgWF(avgWF_0->GetBinContent(j+1),data->at(j),f_event_number+1));
                if ((save_waveforms & 1<<0) == 1<<0)
                    f_wf0->push_back(data->at(j));
            }
            else if(iwf == 1) {
                avgWF_1->SetBinContent(j+1, avgWF(avgWF_1->GetBinContent(j+1),data->at(j),f_event_number+1));
                if ((save_waveforms & 1<<1) == 1<<1)
                    f_wf1->push_back(data->at(j));
            }
            else if(iwf == 2) {
                avgWF_2->SetBinContent(j+1, avgWF(avgWF_2->GetBinContent(j+1),data->at(j),f_event_number+1));
                if ((save_waveforms & 1<<2) == 1<<2)
                    f_wf2->push_back(data->at(j));
            }
            else if(iwf == 3) {
                avgWF_3->SetBinContent(j+1, avgWF(avgWF_3->GetBinContent(j+1),data->at(j),f_event_number+1));
                if ((save_waveforms & 1<<3) == 1<<3)
                    f_wf3->push_back(data->at(j));
            }
        }//data loop
        data->clear();
    }// end iwf waveform loop

    // --------------------------------------------------------------------
    // ---------- save all infor for the telescope ------------------------
    // --------------------------------------------------------------------

    for (size_t iplane = 0; iplane < sev.NumPlanes(); ++iplane) {
        const eudaq::StandardPlane & plane = sev.GetPlane(iplane);
        std::vector<double> cds = plane.GetPixels<double>();

        for (size_t ipix = 0; ipix < cds.size(); ++ipix) {
            f_plane->push_back(iplane);
            f_col->push_back(plane.GetX(ipix));
            f_row->push_back(plane.GetY(ipix));
            f_adc->push_back((int)plane.GetPixel(ipix));
            f_charge->push_back(42);
        }
    }
    m_ttree->Fill();
}


FileWriterTreeDRS4::~FileWriterTreeDRS4() {
    std::cout<<"Tree has " << m_ttree->GetEntries() << " entries" << std::endl;
    m_ttree->Write();
    avgWF_0->Write();
    avgWF_1->Write();
    avgWF_2->Write();
    avgWF_3->Write();

    if(m_tfile->IsOpen()) m_tfile->Close();
}

float FileWriterTreeDRS4::CalculateMedian(std::vector<float> * data, int min, int max) {

}

float FileWriterTreeDRS4::Calculate(std::vector<float> * data, int min, int max, bool _abs) {
    float integral = 0;
    int i;
    for (i = min; i <= int(max+1) && i < data->size() ;i++){
        if(!_abs) integral += data->at(i);
        else integral += abs(data->at(i));
    }
    return integral/(float)(i-(int)min);
}

float FileWriterTreeDRS4::CalculatePeak(std::vector<float> * data, int min, int max) {
    int mid = (int)( (max + min)/2 );
    int n = 1;
    while( (data->at(mid-1)/data->at(mid) -1)*(data->at(mid+1)/data->at(mid) -1) < 0 ){
        // jump up and down from the center position to find the max or min
        mid += pow(-1, n)*n;
        n+=1;
    }
    // extremal value is now at mid
    float integral = Calculate(data, mid-3, mid+6);
    return integral;
}

std::pair<int, float> FileWriterTreeDRS4::FindMaxAndValue(std::vector<float> * data, int min, int max) {
    float maxVal = -999;
    int imax = min;
    for (int i = min; i <= int(max+1) && i < data->size() ;i++){
        if (abs(data->at(i)) > maxVal){ maxVal = abs(data->at(i)); imax = i; }
    }
    maxVal = data->at(imax);
    std::pair<int, float> res = make_pair(imax, maxVal);
    return res;
}


float FileWriterTreeDRS4::avgWF(float old_avg, float new_value, int n) {
    float avg = old_avg;
    avg -= old_avg/n;
    avg += new_value/n;
    return avg;
}

uint64_t FileWriterTreeDRS4::FileBytes() const { return 0; }
    
}
#endif // ROOT_FOUND
