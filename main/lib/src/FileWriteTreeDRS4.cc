#ifdef ROOT_FOUND

#include "eudaq/FileNamer.hh"
#include "eudaq/FileWriter.hh"
#include "eudaq/PluginManager.hh"
#include "eudaq/Logger.hh"
#include "eudaq/FileSerializer.hh"


#include "include/SimpleStandardEvent.hh"
#include <stdlib.h>
#include <cmath>

//# include<inttypes.h>
#include "TFile.h"
#include "TTree.h"
#include "TRandom.h"
#include "TString.h"
#include "TMath.h"
#include "TH1F.h"
#include "TSystem.h"
#include "TInterpreter.h"
#include <TROOT.h>
#include "TF1.h"
#include "TGraph.h"
#include "TLinearFitter.h"

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
        std::pair<int, float> FindMaxAndValue(std::vector<float> * data, int min, int max);
        float avgWF(float, float, int);
        virtual ~FileWriterTreeDRS4();
    private:
        long max_event_number;
        int save_waveforms;
        void ClearVectors();
        void ResizeVectors(unsigned n_channels);
        TFile * m_tfile; // book the pointer to a file (to store the otuput)
        TTree * m_ttree; // book the tree (to store the needed event info)
        // Book variables for the Event_to_TTree conversion
        unsigned m_noe;
        short chan;
        int n_pixels;
        std::map<std::string,std::pair<float,float> *> ranges;
        std::vector<int> polarities;

        vector<int> * v_polarities;

        // Scalar Branches     
        int   f_nwfs;
        int   f_event_number;
        int f_pulser_events;
        int f_signal_events;
        float f_time;
        
        int   f_pulser;
        float f_pulser_int;
        int   f_trig_time;
        

        // Vector Branches     
        // DUT
        std::vector< std::string >  * v_sensor_name;
        std::vector< std::string >  * v_type_name;
        std::vector<float>  *  v_sig_peak;
        std::vector<float>  *  v_peaktime         ;
        std::vector<float>  *  v_sig_int;
        std::vector<float>  *  v_sig_time;
        std::vector<float>  *  v_sig_spread       ;
        std::vector<float>  *  v_sig_integral1;
        std::vector<float>  *  v_sig_integral2;
        std::vector<float>  *  v_sig_integral3;
        std::vector<float>  *  v_ped_int;
        std::vector<float>  *  v_ped_spread       ;
        std::vector<float>  *  v_ped_median       ;
        std::vector<float>  *  v_pul_int;
        std::vector<float>  *  v_pul_spread	      ;

        std::vector<bool>  	*  v_is_saturated;
        std::vector<float>  *  v_median;
        
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
        TH1F * avgWF_0_pul;
        TH1F * avgWF_0_sig;
        TH1F * avgWF_1;
        TH1F * avgWF_2;
        TH1F * avgWF_3;
        TH1F * avgWF_3_pul;
        TH1F * avgWF_3_sig;
        TF1* f_pol1;
        TLinearFitter* fitter;
        std::vector<Double_t> v_x;
        std::vector<Double_t> v_y;
        std::vector<float>* chi2;
        std::vector<float>* par0;
        std::vector<float>* par1;
        std::vector<float>* sigma;
        std::vector<float>* kurtosis;
        std::vector<float>* skewness;
        float max_par0;
        float max_par1;

};

namespace {
static RegisterFileWriter<FileWriterTreeDRS4> reg("drs4tree");      
}

FileWriterTreeDRS4::FileWriterTreeDRS4(const std::string & /*param*/)
: m_tfile(0), m_ttree(0),m_noe(0),chan(4),n_pixels(90*90+60*60)
{
    max_par0 = 0;
    max_par1 = 0;
    gROOT->ProcessLine("#include <vector>");
    gROOT->ProcessLine(".L loader.C+");

    //Polarities of signals, default is positive signals
    polarities.resize(4,1);

    //how many events will be analyzed, 0 = all events
    max_event_number = 0;

    f_nwfs          =  0;
    f_event_number  = -1;
    f_pulser_events = 0;
    f_signal_events = 0;
    f_time          = -1.;
    f_pulser        = -1;
    f_pulser_int    =  0.;
    f_trig_time     =  0.;

    // dut
    v_sensor_name 	= new std::vector< std::string >;
    v_type_name   	= new std::vector< std::string >;
    v_ped_int       = new std::vector<float>;
    v_pul_int       = new std::vector<float>;
    v_sig_int       = new std::vector<float>;
    v_sig_time  	= new std::vector<float>;

    v_sensor_name       = new std::vector< std::string >;
    v_type_name         = new std::vector< std::string >;
    v_sig_peak          = new std::vector<float>;
    v_peaktime          = new std::vector<float>;
    v_sig_spread        = new std::vector<float>;
    v_sig_integral1     = new std::vector<float>;
    v_sig_integral2     = new std::vector<float>;
    v_sig_integral3     = new std::vector<float>;
    v_ped_spread        = new std::vector<float>;
    v_ped_median        = new std::vector<float>;
    v_pul_spread        = new std::vector<float>;
    
    v_is_saturated  	= new std::vector<bool>;
    v_median       		= new std::vector<float>;

    f_wf0 = new std::vector<float>;
    f_wf1 = new std::vector<float>;
    f_wf2 = new std::vector<float>;
    f_wf3 = new std::vector<float>;

    //linear pedestal fit
    chi2 = new std::vector<float>;
    par0 = new std::vector<float>;
    par1 = new std::vector<float>;
    sigma = new std::vector<float>;

    skewness = new std::vector<float>;
    kurtosis= new std::vector<float>;

    // telescope
    f_plane  = new std::vector<int>;
    f_col    = new std::vector<int>;
    f_row    = new std::vector<int>;
    f_adc    = new std::vector<int>;
    f_charge = new std::vector<int>;

    // average waveforms of channels
    avgWF_0 = new TH1F("avgWF_0","avgWF_0", 1024, 0, 1024);
    avgWF_0_pul = new TH1F("avgWF_0_pul","avgWF_0_pul", 1024, 0, 1024);
    avgWF_0_sig = new TH1F("avgWF_0_sig","avgWF_0_sig", 1024, 0, 1024);
    avgWF_1 = new TH1F("avgWF_1","avgWF_1", 1024, 0, 1024);
    avgWF_2 = new TH1F("avgWF_2","avgWF_2", 1024, 0, 1024);
    avgWF_3 = new TH1F("avgWF_3","avgWF_3", 1024, 0, 1024);
    avgWF_3_pul = new TH1F("avgWF_3_pul","avgWF_3_pul", 1024, 0, 1024);
    avgWF_3_sig = new TH1F("avgWF_3_sig","avgWF_3_sig", 1024, 0, 1024);

    fitter = new TLinearFitter();
    f_pol1 = new TF1("f1", "pol1", 0,1024);
    fitter->SetFormula(f_pol1);
    fitter->StoreData(false);
    for(auto i=0;i<1024;i++)
        v_x.push_back(i);

}

void FileWriterTreeDRS4::Configure(){
    ranges["signal"] =  new pair<float,float>(25,175);
    ranges["pedestal"] = new pair<float,float>(350,450);
    ranges["pulser"] = new pair<float,float>(760,860);
    ranges["pedestalFit"] = new pair<float,float>(250,650);
    if (!this->m_config){
    	std::cout<<"Configure: abortion [!this->m_config is True]"<<endl;
        return;
    }
    m_config->SetSection("Converter.drs4tree");
    if (m_config->NSections()==0){
    	std::cout<<"Configure: abortion [m_config->NSections()==0 is True]"<<endl;
        return;
    }
    EUDAQ_INFO("Configuring FileWriterTreeDRS4" );

    ranges["pulser"] = new pair<float,float>(m_config->Get("pulser_range",make_pair((float)770,(float)860)));
    ranges["pedestal"] =  new pair<float,float>(m_config->Get("pedestal_range",make_pair((float)350,(float)450)));
    ranges["pedestalFit"] =  new pair<float,float>(m_config->Get("pedestalFit_range",make_pair((float)250,(float)650)));
    ranges["signal"] =  new pair<float,float>(m_config->Get("signal_range",make_pair((float)25,(float)175)));
    ranges["PeakIntegral1"] =  new pair<float,float>(m_config->Get("PeakIntegral1_range",make_pair((int)3,(int)9)));
    ranges["PeakIntegral2"] =  new pair<float,float>(m_config->Get("PeakIntegral2_range",make_pair((int)9,(int)18)));
    ranges["PeakIntegral3"] =  new pair<float,float>(m_config->Get("PeakIntegral3_range",make_pair((int)18,(int)36)));
    std::cout<<"  - Ranges: "<<ranges.size()<<std::endl;
    for (auto& it: ranges)
        cout<<"     * range_"<<it.first<<" "<<to_string(*(it.second))<<endl;
    save_waveforms = (int)m_config->Get("save_waveforms",9);
    std::cout<<"  - save_waveforms: "+ to_string(save_waveforms)<<std::endl;
    
    for (int i = 0; i < 4; i++){
        std::cout<<"\t\t* ch"<<i<<":"<<to_string(((save_waveforms & 1<<i) == 1<<i));
    	std::cout<<std::endl;
    }

    polarities = m_config->Get("polarities",polarities);
    cout<<"POLARITIES: ";
    for (auto& i:polarities)
        cout<<" "<<i;
    cout<<endl;
    v_polarities = &polarities;

    max_event_number = m_config->Get("max_event_number",0);
}

void FileWriterTreeDRS4::StartRun(unsigned runnumber) {
    EUDAQ_INFO("Converting the inputfile into a DRS4 TTree " );
    std::string foutput(FileNamer(m_filepattern).Set('X', ".root").Set('R', runnumber));
    EUDAQ_INFO("Preparing the outputfile: " + foutput);

    // ---------------------------------------------------------------------
    // the following line is needed to have std::vector<float> in the tree
    // ---------------------------------------------------------------------
    gROOT->ProcessLine("#include <vector>");
    gROOT->ProcessLine("#include <pair>");
    
    m_tfile = new TFile(foutput.c_str(), "RECREATE");
    m_ttree = new TTree("tree", "a simple Tree with simple variables");

    // Set Branch Addresses
    m_ttree->Branch("event_number"  ,&f_event_number , "event_number/I");
    m_ttree->Branch("time"          ,&f_time         , "time/F");
    m_ttree->Branch("pulser"        ,&f_pulser       , "pulser/I");
    m_ttree->Branch("pulser_int"    ,&f_pulser_int   , "pulser_int/F");
    m_ttree->Branch("trig_time"     ,&f_trig_time    , "trig_time/I");
    m_ttree->Branch("nwfs"          ,&f_nwfs        , "n_waveforms/I");
    m_ttree->Branch("chi2",&chi2);
    m_ttree->Branch("par0",&par0);
    m_ttree->Branch("par1",&par1);
    m_ttree->Branch("sigma",&sigma);
    m_ttree->Branch("kurtosis",&kurtosis);
    m_ttree->Branch("skewness",&skewness);



    //settings
//    std::cout<<"Ranges: "<<std::endl;
    for (auto& it: ranges){
        m_ttree->Branch((TString)"range_" + (TString)it.first,"pair<float,float>",it.second);
//        m_ttree->Branch((TString)"range_" + (TString)it.first, &it.second);//, (TString)"range_" + (TString)it.first + "/pair<float,float>");
    }
    if ((save_waveforms & 1<<0) == 1<<0)
        m_ttree->Branch("wf0" , &f_wf0);
    if ((save_waveforms & 1<<1) == 1<<1)
        m_ttree->Branch("wf1" , &f_wf1);
    if ((save_waveforms & 1<<2) == 1<<2)
        m_ttree->Branch("wf2", &f_wf2);
    if ((save_waveforms & 1 << 3) == 1 << 3)
        m_ttree->Branch("wf3", &f_wf3);

    // DUT
    m_ttree->Branch("polarities",&v_polarities);
    m_ttree->Branch("is_saturated", &v_is_saturated);
    m_ttree->Branch("median", 		&v_median);
    // DUT-2
    m_ttree->Branch("sensor_name", 	&v_sensor_name);
    m_ttree->Branch("type_name", 	&v_type_name);
    m_ttree->Branch("peaktime", 	&v_peaktime);
    m_ttree->Branch("sig_time", 	&v_sig_time);
    m_ttree->Branch("sig_int", 		&v_sig_int);
    m_ttree->Branch("sig_peak", 	&v_sig_peak);
    m_ttree->Branch("sig_spread", 	&v_sig_spread);
    m_ttree->Branch("sig_integral1", &v_sig_integral1);
    m_ttree->Branch("sig_integral2", &v_sig_integral2);
    m_ttree->Branch("sig_integral3", &v_sig_integral3);

    m_ttree->Branch("ped_spread", 	&v_ped_spread);
    m_ttree->Branch("ped_median", 	&v_ped_median);
    m_ttree->Branch("ped_int", 		&v_ped_int);

    m_ttree->Branch("pul_spread", &v_pul_spread);
    m_ttree->Branch("pul_int", 		&v_pul_int);

    // telescope
    m_ttree->Branch("plane", 	&f_plane);
    m_ttree->Branch("col", 		&f_col);
    m_ttree->Branch("row", 		&f_row);
    m_ttree->Branch("adc", 		&f_adc);
    m_ttree->Branch("charge", 	&f_charge);
}

void FileWriterTreeDRS4::ClearVectors(){
    v_sensor_name	->clear();
    v_type_name		->clear();

    v_is_saturated	->clear();
    v_median		->clear();
    
    v_pul_int		->clear();
    v_pul_spread    ->clear();

    v_ped_int		->clear();
    v_ped_spread    ->clear();
    v_ped_median    ->clear();
    
    v_sig_int		->clear();
    v_sig_time		->clear();
    v_sig_peak      ->clear();
    v_peaktime      ->clear();
    v_sig_spread    ->clear();
    v_sig_integral1 ->clear();
    v_sig_integral2->clear();
    v_sig_integral3->clear();

    f_wf0			->clear();
    f_wf1			->clear();
    f_wf2			->clear();
    f_wf3			->clear();

    f_plane			->clear();
    f_col			->clear();
    f_row			->clear();
    f_adc			->clear();
    f_charge		->clear();

    par0->clear();
    par1->clear();
    chi2->clear();
    sigma->clear();
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
    if(max_event_number >0 &&f_event_number> max_event_number)
        return;
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

    //use different order of wfs in order to 'know' if its a pulser event or not.
    vector<int> wf_order = {2,1,0,3};
    ResizeVectors(sev.GetNWaveforms());
    for (auto iwf:wf_order){//unsigned int iwf = 0; iwf < nwfs;iwf++){

        const eudaq::StandardWaveform & waveform = sev.GetWaveform(iwf);
        // get the sensor name. see eventually what this actually does!
        std::string type_name;
        type_name = waveform.GetType();

        std::string sensor_name;
        sensor_name = waveform.GetSensor();

        // SimpleStandardWaveform simpWaveform(sensorname,waveform.ID());
        // simpWaveform.setNSamples(waveform.GetNSamples());
        int n_samples = waveform.GetNSamples();
        if (verbose > 3) std::cout << "number of samples in my wf " << n_samples << std::endl;
        // load the waveforms into the vector
        data = waveform.GetData();
        v_y.resize(1024);
        int n_start =ranges["pedestalFit"]->first;
        int n_end = ranges["pedestalFit"]->second;
        for (unsigned i = 0; i < 1024; i++){
            v_y.at(i) =  data->at(i);
        }
//        cout<<"\n"<<n_start<<"-"<<n_end<<"\n";

        fitter->ClearPoints();
        fitter->AssignData(n_end-n_start, 1, &v_x[n_start],&v_y[n_start]);
        fitter->Eval();
//        cout<<fitter->GetParameter(0)<<"/"<<fitter->GetParameter(1)<<" "<<v_y.size()<<" "<<fitter->GetNpoints()<<" "<<fitter->GetNumberFreeParameters() <<std::endl;//<<fitter->GetNDF()<<std::endl;
        chi2->at(iwf) = f_pol1->GetChisquare()/f_pol1->GetNDF();
        par0->at(iwf) = f_pol1->Eval(((float)n_end-(float)n_end)/2.);//f_pol1->GetParameter(0);
        par1->at(iwf) = f_pol1->GetParameter(1);
        int do_print = 0;
        if (iwf==0 && (std::abs( par0->at(iwf) ) > max_par0)){
            do_print = 1;
            max_par0 = std::abs(par0->at(iwf));
        }
        if (iwf==0 && (std::abs(par1->at(iwf))>max_par1) ){
            do_print = 2;
            max_par1 = std::abs(par1->at(iwf));
        }
        if (do_print >0 || f_event_number == 6137){
            std::cout<<setw(7)<<f_event_number<<" "<<iwf<<" "<<do_print<<" "<<std::setw(8)<<max_par0<<" "<<std::setw(8)<<max_par1<<" ";
            std::cout<<std::setw(8)<<par0->at(iwf)<<" + "<<std::setw(10)<<par1->at(iwf)<<" * x\t"<<setw(8)<<chi2->at(iwf)<<"\t";
            std::cout<<setw(8)<<sigma->at(iwf)<<"/"<<setw(8)<<kurtosis->at(iwf)<<"/"<<setw(8)<<skewness->at(iwf)<<std::endl;
        }
        float sigma = 0;
        float skewness = 0;
        float kurtosis = 0;
//        float sigma = 0;
        for (unsigned i = n_start; i< n_end && i<v_y.size();i++){
            float delta = data->at(i)-f_pol1->Eval(i);
            sigma += TMath::Power(delta,2);
            kurtosis += TMath::Power(delta,3);
            skewness += TMath::Power(delta,4);
        }
        sigma = TMath::Sqrt(sigma);
        kurtosis = cbrt(kurtosis);
        skewness = TMath::Power(skewness,1./4.);
        this->sigma->at(iwf) = sigma;
        this->kurtosis->at(iwf) = kurtosis;
        this->skewness->at(iwf) = skewness;

        // calculate the signal and so on
        // float sig = CalculatePeak(data, 1075, 1150);
        int pol = polarities.at(iwf);
        std::pair<int, float> maxAndValue =waveform.getAbsMaxAndValue(ranges["signal"]->first,  ranges["signal"]->second);
        float signal   			= waveform.getSpreadInRange( ranges["signal"]->first,  ranges["signal"]->second);
        float signal_integral   = pol*waveform.getIntegral(ranges["signal"]->first,  ranges["signal"]->second);
        int signal_time 		= waveform.getIndexAbsMax(ranges["signal"]->first,  ranges["signal"]->second);
//        fitter->
//        if (iwf==0)
//            cout<<"WF"<<iwf<<": "<<maxAndValue.first<<"/"<<maxAndValue.second<<"/"<<data->at(maxAndValue.first)<<"/"<<signal_time<<endl;
//        cout<<f_event_number<<":\n  ";
////        cout<<"[";
////        for (float i: *data)
////                cout<<" "<<i;
////        cout<<"]"<<endl;
//        cout<<"   abs max: "<<data->at(waveform.getIndexAbsMax(0,1023))<<"/"<<waveform.getIndexAbsMax(0,1023)<<endl;
//        cout<<"       min: "<<*std::min_element(data->begin(),data->end())<<"/"<<std::min_element(data->begin(),data->end())-data->begin()<<endl;
//        cout<<"       max: "<<*std::max_element(data->begin(),data->end())<<"/"<<std::max_element(data->begin(),data->end())-data->begin()<<endl;
        pair<float,float>* r = ranges["PeakIntegral1"];
        float int_9             = pol*waveform.getIntegral( maxAndValue.first-r->first, maxAndValue.first+r->second);
        r = ranges["PeakIntegral2"];
        float int_27            = pol*waveform.getIntegral( maxAndValue.first-r->first, maxAndValue.first+r->second);
        r = ranges["PeakIntegral3"];
        float int_54            = pol*waveform.getIntegral( maxAndValue.first-r->first, maxAndValue.first+r->second);

        float sig_static = pol*waveform.getIntegral( ranges["signal"]->first,  ranges["signal"]->second);
        float signalSpread      = waveform.getSpreadInRange(ranges["signal"]->first,  ranges["signal"]->second);

        float pedestal = waveform.getSpreadInRange(ranges["pedestal"]->first,  ranges["pedestal"]->second);
        float pedestal_integral   = pol*waveform.getIntegral( ranges["pedestal"]->first,  ranges["pedestal"]->second);
        float pedestal_median   = pol*waveform.getMedian( ranges["pedestal"]->first,  ranges["pedestal"]->second);

        float median    = pol*waveform.getMedian(200, 700);
        float median2            = pol*waveform.getMedian(0, 1023);

        float pulser   = waveform.getSpreadInRange( ranges["pulser"]->first,   ranges["pulser"]->second);
        float pulser_integral   = pol*waveform.getIntegral( ranges["pulser"]->first,   ranges["pulser"]->second);

        float abs_max           = waveform.getAbsMaxInRange(0,1023);

        // float mini = waveform.getMinInRange(10,1000);
        // float minind = waveform.getIndexMin(10,1000);
        // cout << "minimum of the waveform in range 10, 1000: " << mini << " at index " << minind<< endl;
//        cout<< "Signal Range: "<< ranges["signal"]->first << " / " << ranges["signal"]->second << endl;
//        cout<< "Pedestal Range: "<< ranges["pedestal"]->first << " / " << ranges["pedestal"]->second << endl;

        // ------------------------------------
        // ---------- LOAD VALUES  ------------
        // ------------------------------------
        // save the values in the event
        v_type_name		->at(iwf) = (type_name);				// Type Name
        v_sensor_name	->at(iwf) = (sensor_name);			// Sensor Name
        
        v_sig_int 		->at(iwf) = (signal_integral);		// Signal: Integral over Signalrange
        v_sig_spread    ->at(iwf) = (signalSpread);      	// Signal: Spread in Signalrange
        v_sig_peak      ->at(iwf) = (pol*maxAndValue.second); 	// Signal: Value of peak (no pedestal correction)
        v_sig_integral1 ->at(iwf) = (int_9);                 // Signal: Integral around peak with range set in config file
        v_sig_integral2	->at(iwf) = (int_27);                // Signal: Integral around peak with range set in config file
        v_sig_integral3	->at(iwf) = (int_54); 				// Signal: Integral around peak with range set in config file
        
        v_sig_time		->at(iwf) = (signal_time); 			// Peakposition: Calc by waveform.getIndexAbsMax()
        v_peaktime      ->at(iwf) = (maxAndValue.first);  	// Peakposition: Calc by waveform.getAbsMaxAndValue() ( the same?? )
        
        v_ped_int		->at(iwf) = (pedestal_integral);		// Pedestal: Integral over Pedestalrange
        v_ped_spread    ->at(iwf) = (pedestal);              // Pedestal: Spread in Pedestalrange
        v_ped_median    ->at(iwf) = (pedestal_median);       // Pedestal: Median in Pedestalrange
                                                            
        v_pul_int		->at(iwf) = (pulser_integral);       // Pulser: Integral over Pulserrange
        v_pul_spread    ->at(iwf) = (pulser);                // Pulser: Spread in Pulserrange
           
        v_is_saturated 	->at(iwf) = (abs_max>498);			// indicator if saturation is reached in sampling region (1-1024)
        v_median		->at(iwf) = (median);				// Median over large sampling region
                       
        
        if(iwf == 1){ // trigger WF
            for (int j=0; j<data->size(); j++){
                if( abs(data->at(j)) > 90. ) {f_trig_time = j; break;}
            }
        }
        if(iwf == 2){ // pulser WF
            f_pulser_int = waveform.getIntegral(700, 950, true);
            f_pulser     = (f_pulser_int > 80.);
            if (f_pulser)
                f_pulser_events++;
            else
                f_signal_events++;
        }
        for (int j=0; j<data->size(); j++){
            if     (iwf == 0) {
                if (f_pulser)
                    avgWF_0_pul->SetBinContent(j+1, avgWF(avgWF_0_pul->GetBinContent(j+1),data->at(j),f_pulser_events));
                else
                    avgWF_0_sig->SetBinContent(j+1, avgWF(avgWF_0_sig->GetBinContent(j+1),data->at(j),f_signal_events));
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
                if (f_pulser)
                    avgWF_3_pul->SetBinContent(j+1, avgWF(avgWF_3_pul->GetBinContent(j+1),data->at(j),f_pulser_events));
                else
                    avgWF_3_sig->SetBinContent(j+1, avgWF(avgWF_3_sig->GetBinContent(j+1),data->at(j),f_signal_events));
                avgWF_3->SetBinContent(j+1, avgWF(avgWF_3->GetBinContent(j+1),data->at(j),f_event_number+1));
                if ((save_waveforms & 1<<3) == 1<<3)
                    f_wf3->push_back(data->at(j));
            }
        }//data loop
        data->clear();
    }// end iwf waveform loop

    // --------------------------------------------------------------------
    // ---------- save all info for the telescope ------------------------
    // --------------------------------------------------------------------

    for (size_t iplane = 0; iplane < sev.NumPlanes(); ++iplane) {
        const eudaq::StandardPlane & plane = sev.GetPlane(iplane);
        std::vector<double> cds = plane.GetPixels<double>();

        for (size_t ipix = 0; ipix < cds.size(); ++ipix) {
            f_plane		->push_back(iplane);
            f_col		->push_back(plane.GetX(ipix));
            f_row		->push_back(plane.GetY(ipix));
            f_adc		->push_back((int)plane.GetPixel(ipix));
            f_charge	->push_back(42);						// <-------------------------------------------------------------- !
        }
    }
    m_ttree->Fill();
}


FileWriterTreeDRS4::~FileWriterTreeDRS4() {
    std::cout<<"Tree has " << m_ttree->GetEntries() << " entries" << std::endl;
    std::cout<<"Max parameter: :"<<max_par0<<"/"<<max_par1<<std::endl;
    m_ttree->Write();
    avgWF_0->Write();
    avgWF_0_pul->Write();
    avgWF_0_sig->Write();
    avgWF_1->Write();
    avgWF_2->Write();
    avgWF_3->Write();
    avgWF_3_sig->Write();
    avgWF_3_pul->Write();

    if(m_tfile->IsOpen()) m_tfile->Close();
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

inline void FileWriterTreeDRS4::ResizeVectors(unsigned n_channels) {
    v_type_name->resize(n_channels);
    v_sensor_name->resize(n_channels);

    v_sig_int->resize(n_channels);
    v_sig_spread->resize(n_channels);
    v_sig_peak->resize(n_channels);
    v_sig_integral1->resize(n_channels);
    v_sig_integral2->resize(n_channels);
    v_sig_integral3->resize(n_channels);

    v_sig_time->resize(n_channels);
    v_peaktime->resize(n_channels);

    v_ped_int->resize(n_channels);
    v_ped_spread->resize(n_channels);
    v_ped_median->resize(n_channels);

    v_pul_int->resize(n_channels);
    v_pul_spread->resize(n_channels);

    v_is_saturated->resize(n_channels);
    v_median->resize(n_channels);

    par0->resize(n_channels);
    par1->resize(n_channels);
    chi2->resize(n_channels);
    sigma->resize(n_channels);
    skewness->resize(n_channels);
    kurtosis->resize(n_channels);
}

}
#endif // ROOT_FOUND
