#ifdef ROOT_FOUND

#include "eudaq/FileNamer.hh"
#include "eudaq/FileWriter.hh"
#include "eudaq/PluginManager.hh"
#include "eudaq/Logger.hh"
#include "eudaq/FileSerializer.hh"


#include "include/SimpleStandardEvent.hh"
#include <stdlib.h>
#include <stdio.h>
#include <cmath>
#include <time.h>
#include <numeric>
#include <string>
#include <functional>
#include "TStopwatch.h"

//# include<inttypes.h>
#include "TFile.h"
#include "TTree.h"
#include "TRandom.h"
#include "TString.h"
#include "TMath.h"
#include "TH1F.h"
#include "TSystem.h"
#include "TInterpreter.h"
#include "TROOT.h"
#include "TF1.h"
#include "TGraph.h"
#include "TCanvas.h"
#include "TLinearFitter.h"
#include "TSpectrum.h"
#include "TPolyMarker.h"
#include "TVirtualFFT.h"

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
        unsigned runnumber;
        TH1F* histo;
        long max_event_number;
        int save_waveforms;
        void ClearVectors();
        void ResizeVectors(unsigned n_channels);
        int IsPulserEvent(const StandardWaveform *wf);
        void FillSignalRange(int iwf,const StandardWaveform *wf, int pol);
        void FillPulserRange(int iwf,const StandardWaveform *wf, int pol);
        void FillPedestalRange(int iwf,const StandardWaveform *wf, int pol);
        void FillTotalRange(int iwf,const StandardWaveform *wf, int pol);
        void UpdateWaveforms(int iwf, const StandardWaveform *wf);
        void DoLinearFitting(int iwf);
        void DoSpectrumFitting(int iwf);
        void DoFFTAnalysis(int iwf);
        // clocks for checking execution time
        TStopwatch w_linear_fitting;
        TStopwatch w_spectrum;
        TStopwatch w_fft;
        TStopwatch w_total;
        TFile * m_tfile; // book the pointer to a file (to store the otuput)
        TTree * m_ttree; // book the tree (to store the needed event info)
        int verbose;
        std::vector<float> * data;

        std::vector<float> v1;
        std::vector<float> v_yy;
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

        float spectrum_sigma;
        float spectrum_threshold;
        int spectrum_deconIterations;
        int spectrum_averageWindow;
        bool spectrum_markov;
        bool spectrum_background_removal;

        int spectrum_waveforms;
        int fft_waveforms;
        int linear_fitting_waveforms;

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
        std::vector<float>  *  v_ped_integral1      ;
        std::vector<float>  *  v_ped_integral2      ;
        std::vector<float>  *  v_ped_integral3      ;
        std::vector<float>  *  v_ped_median       ;
        std::vector<float>  *  v_ped_median1       ;
        std::vector<float>  *  v_ped_median2       ;
        std::vector<float>  *  v_pul_int;
        std::vector<float>  *  v_pul_spread	      ;

        std::vector<bool>  	*  v_is_saturated;
        std::vector<float>  *  v_median;
        std::vector<float>  *  v_average;
        std::vector<bool>   *  v_has_spikes;

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
        TSpectrum *spec;
        TVirtualFFT *fft_own;
        Int_t n_samples;
        Double_t *re_full;
        Double_t *im_full;
        Double_t *in;

        std::vector<Double_t> v_x;
        std::vector<Double_t> v_y;
        std::vector<float>* chi2;
        std::vector<float>* par0;
        std::vector<float>* par1;
        std::vector<float>* sigma;
        std::vector<float>* kurtosis;
        std::vector<float>* skewness;
        std::vector<std::vector<float>*> peaks_x;
        std::vector<std::vector<float>*> peaks_y;
        std::vector<std::vector<int>*> peaks_no;
        std::vector<float>* npeaks;
        std::vector<float>* fft_mean;
        std::vector<float>* fft_mean_freq;
        std::vector<float>* fft_max;
        std::vector<float>* fft_max_freq;
        std::vector<float>* fft_min;
        std::vector<float>* fft_min_freq;
        std::vector< std::vector<float>* > fft_modes;
        TCanvas *c1;

};

namespace {
static RegisterFileWriter<FileWriterTreeDRS4> reg("drs4tree");      
}

FileWriterTreeDRS4::FileWriterTreeDRS4(const std::string & /*param*/)
: m_tfile(0), m_ttree(0),m_noe(0),chan(4),n_pixels(90*90+60*60), histo(0),spec(0),fft_own(0),runnumber(0)
{
    gROOT->ProcessLine("#include <vector>");
    gROOT->ProcessLine(".L loader.C+");
    gROOT->ProcessLine( "gErrorIgnoreLevel = 5001;");
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
    v_ped_integral1		= new std::vector<float>;
    v_ped_integral2		= new std::vector<float>;
    v_ped_integral3		= new std::vector<float>;
    v_ped_median        = new std::vector<float>;
    v_ped_median1       = new std::vector<float>;
    v_ped_median2       = new std::vector<float>;
    v_pul_spread        = new std::vector<float>;

    v_is_saturated  	= new std::vector<bool>;
    v_has_spikes        = new std::vector<bool>;
    v_median       		= new std::vector<float>;
    v_average           = new std::vector<float>;

    f_wf0 = new std::vector<float>;
    f_wf1 = new std::vector<float>;
    f_wf2 = new std::vector<float>;
    f_wf3 = new std::vector<float>;

    //linear pedestal fit
    chi2 = new std::vector<float>;
    par0 = new std::vector<float>;
    par1 = new std::vector<float>;
    sigma = new std::vector<float>;

    peaks_x.resize(4, new std::vector<float>);
    peaks_y.resize(4, new std::vector<float>);
    peaks_no.resize(4, new std::vector<int>);
    fft_modes.resize(4, new std::vector<float>);

    npeaks = new std::vector<float>;
    fft_mean = new std::vector<float>;
    fft_mean_freq = new std::vector<float>;
    fft_max = new std::vector<float>;
    fft_max_freq = new std::vector<float>;
    fft_min = new std::vector<float>;
    fft_min_freq = new std::vector<float>;

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

    spec = new TSpectrum(20,3);
    fft_own = 0;
    if(!fft_own){
        int n = 1024;
        n_samples = n+1;
        cout<<"Creating a new VirtualFFT with "<<n_samples<<" Samples"<<endl;
        re_full = new Double_t[n];
        im_full = new Double_t[n];
        in = new Double_t[n];
        fft_own = TVirtualFFT::FFT(1, &n_samples, "R2C");
    }
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


    spectrum_sigma = m_config->Get("spectrum_sigma",(float)10.);
    spectrum_threshold = m_config->Get("spectrum_threshold",(float)1.);
    spectrum_deconIterations = m_config->Get("spectrum_deconIterations",10);
    spectrum_averageWindow = m_config->Get("spectrum_averageWindow",5);
    spec->SetDeconIterations(spectrum_deconIterations);
    spec->SetAverageWindow(spectrum_averageWindow);
    spectrum_markov = m_config->Get("spectrum_markov",true);
    spectrum_background_removal= m_config->Get("spectrum_background_removal",true);
    spectrum_waveforms = (int)m_config->Get("spectrum_waveforms",9);
    fft_waveforms = (int)m_config->Get("fft_waveforms",9);
    linear_fitting_waveforms = (int)m_config->Get("linear_fitting_waveforms",9);

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
    this->runnumber = runnumber;
    EUDAQ_INFO("Converting the inputfile into a DRS4 TTree " );
    std::string foutput(FileNamer(m_filepattern).Set('X', ".root").Set('R', runnumber));
    EUDAQ_INFO("Preparing the outputfile: " + foutput);

    c1 = new TCanvas();
    c1->Draw();

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

    // linearfitting
    m_ttree->Branch("chi2",&chi2);
    m_ttree->Branch("par0",&par0);
    m_ttree->Branch("par1",&par1);
    m_ttree->Branch("sigma",&sigma);
    m_ttree->Branch("kurtosis",&kurtosis);
    m_ttree->Branch("skewness",&skewness);
    //    m_ttree->Branch("histo",&histo);

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
    m_ttree->Branch("has_spikes", &v_has_spikes);
    m_ttree->Branch("median", 		&v_median);
    m_ttree->Branch("average",       &v_average);
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
    m_ttree->Branch("ped_integral1", &v_ped_integral1);
    m_ttree->Branch("ped_integral2", &v_ped_integral2);
    m_ttree->Branch("ped_integral3", &v_ped_integral3);
    m_ttree->Branch("ped_median", 	&v_ped_median);
    m_ttree->Branch("ped_median1", 	&v_ped_median1);
    m_ttree->Branch("ped_median2", 	&v_ped_median2);
    m_ttree->Branch("ped_int", 		&v_ped_int);

    m_ttree->Branch("pul_spread", &v_pul_spread);
    m_ttree->Branch("pul_int", 		&v_pul_int);

    /// for tspectrum
    m_ttree->Branch("npeaks",      &npeaks);
    m_ttree->Branch("fft_mean", &fft_mean);
    m_ttree->Branch("fft_mean_freq", &fft_mean_freq);
    m_ttree->Branch("fft_max", &fft_max);
    m_ttree->Branch("fft_max_freq", &fft_max_freq);
    m_ttree->Branch("fft_min", &fft_min);
    m_ttree->Branch("fft_min_freq", &fft_min_freq);

    for (int i=0; i < 4; i++){
        TString name = TString::Format("peaks%d_x",i);
        m_ttree->Branch(name,&peaks_x.at(i));
        name = TString::Format("peaks%d_y",i);
        m_ttree->Branch(name,&peaks_y.at(i));
        name = TString::Format("peaks%d_no",i);
        m_ttree->Branch(name,&peaks_no.at(i));
        name = TString::Format("fft_modes%d",i);
        m_ttree->Branch(name,&fft_modes.at(i));
    }

    // telescope
    m_ttree->Branch("plane", 	&f_plane);
    m_ttree->Branch("col", 		&f_col);
    m_ttree->Branch("row", 		&f_row);
    m_ttree->Branch("adc", 		&f_adc);
    m_ttree->Branch("charge", 	&f_charge);
    verbose = 0;
}

void FileWriterTreeDRS4::ClearVectors(){
    v_sensor_name	->clear();
    v_type_name		->clear();

    v_is_saturated	->clear();
    v_has_spikes->clear();
    v_median		->clear();
    v_average       ->clear();

    v_pul_int		->clear();
    v_pul_spread    ->clear();

    v_ped_int		->clear();
    v_ped_spread    ->clear();
    v_ped_integral1 ->clear();
    v_ped_integral2 ->clear();
    v_ped_integral3 ->clear();
    v_ped_median    ->clear();
    v_ped_median1   ->clear();
    v_ped_median2   ->clear();

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
    for (auto peak: peaks_x)
        peak->clear();
    for (auto peak: peaks_y)
        peak->clear();
    for (auto peak: peaks_no)
        peak->clear();
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
    w_total.Start(false);
    StandardEvent sev = eudaq::PluginManager::ConvertToStandard(ev);

    f_event_number = sev.GetEventNumber();
    f_time = sev.GetTimestamp()/384066.;
    // --------------------------------------------------------------------
    // ---------- get the number of waveforms -----------------------------
    // --------------------------------------------------------------------
    unsigned int nwfs = (unsigned int) sev.NumWaveforms();
    f_nwfs = nwfs;

    if(f_event_number <= 10 && verbose > 0){
        cout << "event number " << f_event_number << endl;
        cout << "number of waveforms " << nwfs << endl;
        if(nwfs ==0){
            cout << "----------------------------------------" << endl;
            cout << "WARNING!!! NO WAVEFORMS IN THIS EVENT!!!" << endl;
            cout << "----------------------------------------" << endl;
        }
    }
    if(verbose > 3)
        cout<<"ClearVectors"<<endl;
    ClearVectors();

    // --------------------------------------------------------------------
    // ---------- verbosity level and some printouts ----------------------
    // --------------------------------------------------------------------

    if(verbose > 3) cout << "event number " << f_event_number << endl;
    if(verbose > 3) cout << "number of waveforms " << nwfs << endl;

    // --------------------------------------------------------------------
    // ---------- get and save all info for all waveforms -----------------
    // --------------------------------------------------------------------


    //use different order of wfs in order to 'know' if its a pulser event or not.
    vector<int> wf_order = {2,1,0,3};
    ResizeVectors(sev.GetNWaveforms());
    for (auto iwf:wf_order){//unsigned int iwf = 0; iwf < nwfs;iwf++){
        int pol = polarities.at(iwf);

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
        if (verbose > 3)
            cout<<"DoSpectrumFitting "<<iwf<<endl;
        this->DoSpectrumFitting(iwf);
        if (verbose > 3)
            cout<<"DoLinearFitting "<<iwf<<endl;
        this->DoLinearFitting(iwf);

        if (verbose > 3)
            cout<<"DoFFT "<<iwf<<endl;
        this->DoFFTAnalysis(iwf);

        if (verbose > 3)
            cout<<"get Values "<<iwf<<endl;

        // calculate the signal and so on
        // float sig = CalculatePeak(data, 1075, 1150);
        FillSignalRange(iwf, &waveform,pol);
        FillPedestalRange(iwf, &waveform,pol);
        FillPulserRange(iwf, &waveform,pol);
        FillTotalRange(iwf, &waveform,pol);

        // ------------------------------------
        // ---------- LOAD VALUES  ------------
        // ------------------------------------
        // save the values in the event
        v_type_name		->at(iwf) = (type_name);				// Type Name
        v_sensor_name	->at(iwf) = (sensor_name);			// Sensor Name

        if (verbose > 3)
            cout<<"get trigger wf "<<iwf<<endl;
        if(iwf == 2){ // trigger WF
            for (int j=0; j<data->size(); j++){
                if( abs(data->at(j)) > 90. ) {f_trig_time = j; break;}
            }
        }

        if (verbose > 3)
            cout<<"get pulser wf "<<iwf<<endl;
        if(iwf == 1){ // pulser WF
            f_pulser = this->IsPulserEvent(&waveform);
            if (f_pulser)
                f_pulser_events++;
            else
                f_signal_events++;
        }
        if (verbose > 3)
            cout<<"fill wf "<<iwf<<endl;
        UpdateWaveforms(iwf, &waveform);

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
    if (f_event_number %1000 == 0)
        cout<<runnumber<<" "<<std::setw(7)<<f_event_number<<"\tSpectrum: "<<w_spectrum.RealTime()/w_spectrum.Counter()<<"\t"
        <<"LinearFitting: "<<w_linear_fitting.RealTime()/w_linear_fitting.Counter()<<"\t"<<
        w_spectrum.Counter()<<"/"<<w_linear_fitting.Counter()<<"\t"<<flush;
    w_total.Stop();
}


FileWriterTreeDRS4::~FileWriterTreeDRS4() {
    std::cout<<"\n****************************************************"<<std::endl;
    std::cout<<"Summary of RUN "<<runnumber<<std::endl;
    std::cout<<"Tree has " << m_ttree->GetEntries() << " entries" << std::endl;
    cout<<f_event_number<<"\tSpectrum: "<<w_spectrum.RealTime()/w_spectrum.Counter()<<"\t"
    <<"LinearFitting: "<<w_linear_fitting.RealTime()/w_linear_fitting.Counter()<<"\t"<<
    w_spectrum.Counter()<<"/"<<w_linear_fitting.Counter()<<endl;
    cout<<f_event_number<<"\tSpectrum: "<<w_spectrum.RealTime()<<"\t"
    <<"LinearFitting: "<<w_linear_fitting.RealTime()<<endl;
    cout<<"\nSpectrum:"<<endl;
    w_spectrum.Print();
    cout<<"\nLinearFit:"<<endl;
    w_linear_fitting.Print();
    cout<<"\nFFT:"<<endl;
    w_fft.Print();

    cout<<"\n Total time: "<<w_total.RealTime()<<endl;
    std::cout<<"****************************************************\n"<<std::endl;
    w_total.Print();
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
    v_ped_integral1->resize(n_channels);
    v_ped_integral2->resize(n_channels);
    v_ped_integral3->resize(n_channels);
    v_ped_median->resize(n_channels);
    v_ped_median1->resize(n_channels);
    v_ped_median2->resize(n_channels);

    v_pul_int->resize(n_channels);
    v_pul_spread->resize(n_channels);

    v_is_saturated->resize(n_channels);
    v_has_spikes->resize(n_channels,false);
    v_median->resize(n_channels);
    v_average->resize(n_channels);

    par0->resize(n_channels);
    par1->resize(n_channels);
    chi2->resize(n_channels);
    sigma->resize(n_channels);
    skewness->resize(n_channels);
    kurtosis->resize(n_channels);
    npeaks->resize(n_channels,0);
    fft_mean->resize(n_channels,0);
    fft_min->resize(n_channels,0);
    fft_max->resize(n_channels,0);

    fft_mean_freq->resize(n_channels,0);
    fft_min_freq->resize(n_channels,0);
    fft_max_freq->resize(n_channels,0);
    for (auto p: peaks_x)
        p->clear();
    for (auto p: peaks_y)
            p->clear();
    for (auto p: peaks_no)
            p->clear();
    for (auto p: fft_modes)
            p->clear();
}


void FileWriterTreeDRS4::DoFFTAnalysis(int iwf){
    bool b_fft = (fft_waveforms & 1<<iwf) == 1<<iwf;
    fft_mean->at(iwf) = 0;
    fft_max->at(iwf) = 0;
    fft_min->at(iwf) = 1e9;
    fft_mean_freq->at(iwf) = -1;
    fft_max_freq->at(iwf) = -1;
    fft_min_freq->at(iwf) = -1;
    if (!b_fft)
        return;
    w_fft.Start(false);
    int n = data->size();
    float sample_rate = 2e6;
    if(fft_own->GetN()[0] != n+1){
        n_samples = n+1;
        cout<<"RECreating a new VirtualFFT with "<<n_samples<<" Samples, before "<<fft_own->GetN()<<endl;
        delete fft_own;
        delete in;
        delete re_full;
        delete im_full;
        fft_own = 0;
        in = new Double_t[n];
        re_full = new Double_t[n];
        im_full = new Double_t[n];
        fft_own = TVirtualFFT::FFT(1, &n_samples, "R2C");
    }
    for (int j = 0; j < n; ++j) {
        in[j] = data->at(j);
    }
    fft_own->SetPoints(in);
    fft_own->Transform();
    fft_own->GetPointsComplex(re_full,im_full);
    float finalVal = 0;
    float max = -1;
    float min = 1e10;
    float freq;
    float mean_freq;
    float min_freq = -1;
    float max_freq  = -1;
    float value;
    for (int j = 0; j < (n/2+1); ++j) {
        freq = j * sample_rate/n;
        value =  TMath::Sqrt(re_full[j]*re_full[j] + im_full[j]*im_full[j]);
        if (value>max){
            max = value;
            max_freq = freq;
        }
        if (value<min) {
            min = value;
            min_freq = freq;
        }
        finalVal+= value;
        mean_freq += freq * value;
        if (j < 10 || j == n/2)
            fft_modes.at(iwf)->push_back(value);
    }
    mean_freq /= finalVal;
    finalVal/= ((n/2) + 1);
    fft_mean->at(iwf) = finalVal;
    fft_max->at(iwf) = max;
    fft_min->at(iwf) = min;
    fft_mean_freq->at(iwf) = mean_freq;
    fft_max_freq->at(iwf) = max_freq;
    fft_min_freq->at(iwf) = min_freq;
    w_fft.Stop();
    if (verbose>0 && f_event_number < 1000)
        cout<<runnumber<<" "<<std::setw(3)<<f_event_number<<" "<<iwf<<" "<<finalVal<<" "<<max<<" "<<min<<endl;
}

void FileWriterTreeDRS4::DoSpectrumFitting(int iwf){
    bool b_spectrum = (spectrum_waveforms & 1<<iwf) == 1<<iwf;
    peaks_x.at(iwf)->clear();
    peaks_y.at(iwf)->clear();
    peaks_no.at(iwf)->clear();
    npeaks->at(iwf) = 0;
    if (!b_spectrum)
        return;

    w_spectrum.Start(false);
    v_yy.resize(v_y.size());
    int peaks = spec->SearchHighRes(&v1[0],&(v_yy[0]),v1.size(),spectrum_sigma,spectrum_threshold,
            spectrum_background_removal, spectrum_deconIterations,spectrum_markov, spectrum_averageWindow);
    npeaks->at(iwf) = peaks;
//        std::cout <<iwf<<": "<<peaks<<"   ";
    for(UInt_t i=0; i< peaks; i++){
//            std::cout<<i<<": ";
        float xval = spec->GetPositionX()[i];
//            std::cout<<xval<<"/";
        int bin = (int)(xval+.5);
        int min_bin = bin-5>=0?bin-5:0;
        int max_bin = bin+5<v_y.size()?bin+5:v_y.size()-1;
        float max = *std::max_element(&v1.at(min_bin),&v1.at(min_bin));
//            std::cout<<max<<"   ";
        peaks_x.at(iwf)->push_back(xval);
        peaks_y.at(iwf)->push_back(max);
        peaks_no.at(iwf)->push_back(i);
    }
//        std::cout<<peaks_x.at(iwf)->size()<<"/"<<peaks_y.at(iwf)->size()<<"/"<<peaks_no.at(iwf)->size()<<std::endl;
    w_spectrum.Stop();
}

void FileWriterTreeDRS4::DoLinearFitting(int iwf){
    bool b_linear_fit = (linear_fitting_waveforms & 1<<iwf) == 1<<iwf;
    bool b_spectrum = (spectrum_waveforms & 1<<iwf) == 1<<iwf;
    bool b_fft = (fft_waveforms & 1<<iwf) == 1<<iwf;
//    cout<<"iwf: "<<iwf<<"lin "<<linear_fitting_waveforms<<" "<< 1<<iwf<<" "<<(linear_fitting_waveforms & 1<<iwf)<<" "<<b_linear_fit<<"\t";
//    cout<<"iwf: "<<iwf<<"spec "<<spectrum_waveforms<<" "<< 1<<iwf<<" "<<(spectrum_waveforms & 1<<iwf)<<" "<<b_spectrum<<"\t"<<endl;;
    if(b_spectrum || b_linear_fit || b_fft){
        v_y.resize(data->size());
        v1.resize(data->size());
        int pol = polarities.at(iwf);
        for (unsigned i = 0; i < data->size(); i++){
            v_y.at(i) =  pol*data->at(i);
            v1.at(i) =  pol*data->at(i);
        }
    }
    if (b_linear_fit){
        w_linear_fitting.Start(false);
        int n_start = ranges["pedestalFit"]->first;
        int n_end = ranges["pedestalFit"]->second;

        fitter->ClearPoints();
        fitter->AssignData(n_end-n_start, 1, &v_x[n_start],&v_y[n_start]);
        fitter->Eval();
        //        cout<<fitter->GetParameter(0)<<"/"<<fitter->GetParameter(1)<<" "<<v_y.size()<<" "<<fitter->GetNpoints()<<" "<<fitter->GetNumberFreeParameters() <<std::endl;//<<fitter->GetNDF()<<std::endl;
        chi2->at(iwf) = f_pol1->GetChisquare()/f_pol1->GetNDF();
        par0->at(iwf) = f_pol1->Eval(((float)n_end-(float)n_end)/2.);//f_pol1->GetParameter(0);
        par1->at(iwf) = f_pol1->GetParameter(1);
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
        w_linear_fitting.Stop();
    }
}


void FileWriterTreeDRS4::FillSignalRange(int iwf, const StandardWaveform *wf, int pol){
    std::pair<int, float> maxAndValue =wf->getAbsMaxAndValue(ranges["signal"]->first,  ranges["signal"]->second);
    float signal            = wf->getSpreadInRange( ranges["signal"]->first,  ranges["signal"]->second);
    float signal_integral   = pol*wf->getIntegral(ranges["signal"]->first,  ranges["signal"]->second);
    int signal_time         = wf->getIndexAbsMax(ranges["signal"]->first,  ranges["signal"]->second);

    pair<float,float>* r = ranges["PeakIntegral1"];
    float int_9             = pol*wf->getIntegral( maxAndValue.first-r->first, maxAndValue.first+r->second);
    r = ranges["PeakIntegral2"];
    float int_27            = pol*wf->getIntegral( maxAndValue.first-r->first, maxAndValue.first+r->second);
    r = ranges["PeakIntegral3"];
    float int_54            = pol*wf->getIntegral( maxAndValue.first-r->first, maxAndValue.first+r->second);

    if (verbose > 3)
        cout<<"get Values2 "<<iwf<<endl;
    float sig_static = pol*wf->getIntegral( ranges["signal"]->first,  ranges["signal"]->second);
    float signalSpread      = wf->getSpreadInRange(ranges["signal"]->first,  ranges["signal"]->second);

    v_sig_int       ->at(iwf) = (signal_integral);      // Signal: Integral over Signalrange
    v_sig_spread    ->at(iwf) = (signalSpread);         // Signal: Spread in Signalrange
    v_sig_peak      ->at(iwf) = (pol*maxAndValue.second);   // Signal: Value of peak (no pedestal correction)
    v_sig_integral1 ->at(iwf) = (int_9);                 // Signal: Integral around peak with range set in config file
    v_sig_integral2 ->at(iwf) = (int_27);                // Signal: Integral around peak with range set in config file
    v_sig_integral3 ->at(iwf) = (int_54);               // Signal: Integral around peak with range set in config file

    v_sig_time      ->at(iwf) = (signal_time);          // Peakposition: Calc by wf->getIndexAbsMax()
    v_peaktime      ->at(iwf) = (maxAndValue.first);    // Peakposition: Calc by wf->getAbsMaxAndValue() ( the same?? )

}

void FileWriterTreeDRS4::FillPedestalRange(int iwf, const StandardWaveform *wf, int pol){
	
    float pedestal = wf->getSpreadInRange(ranges["pedestal"]->first,  ranges["pedestal"]->second);
    float pedestal_integral   = pol*wf->getIntegral( ranges["pedestal"]->first,  ranges["pedestal"]->second);
    float pedestal_median   = pol*wf->getMedian( ranges["pedestal"]->first,  ranges["pedestal"]->second);
    int halfped = (ranges["pedestal"]->second - ranges["pedestal"]->first)/2;
    float pedestal_median1   = pol*wf->getMedian( ranges["pedestal"]->first,  ranges["pedestal"]->first + halfped);
    float pedestal_median2   = pol*wf->getMedian( ranges["pedestal"]->second - halfped,  ranges["pedestal"]->second);

    pair<float,float>* r = ranges["PeakIntegral1"];
    float int_9             = pol*wf->getIntegral(ranges["pedestal"]->first + halfped - r->first, ranges["pedestal"]->first + halfped + r->second);
    r = ranges["PeakIntegral2"];
    float int_27            = pol*wf->getIntegral(ranges["pedestal"]->first + halfped - r->first, ranges["pedestal"]->first + halfped + r->second);
    r = ranges["PeakIntegral3"];
    float int_54            = pol*wf->getIntegral(ranges["pedestal"]->first + halfped - r->first, ranges["pedestal"]->first + halfped + r->second);

    v_ped_int       ->at(iwf) = (pedestal_integral);        // Pedestal: Integral over Pedestalrange
    v_ped_spread    ->at(iwf) = (pedestal);              // Pedestal: Spread in Pedestalrange
    v_ped_integral1 ->at(iwf) = (int_9);                 // Signal: Integral around peak with range set in config file
    v_ped_integral2 ->at(iwf) = (int_27);                // Signal: Integral around peak with range set in config file
    v_ped_integral3 ->at(iwf) = (int_54);               // Signal: Integral around peak with range set in config file
    v_ped_median    ->at(iwf) = (pedestal_median);       // Pedestal: Median in Pedestalrange
    v_ped_median1    ->at(iwf) = (pedestal_median1);       // Pedestal: Median in first half of Pedestalrange
    v_ped_median2    ->at(iwf) = (pedestal_median2);       // Pedestal: Median in second half of Pedestalrange
}

void FileWriterTreeDRS4::FillPulserRange(int iwf, const StandardWaveform *wf, int pol){


    float pulser   = wf->getSpreadInRange( ranges["pulser"]->first,   ranges["pulser"]->second);
    float pulser_integral   = pol*wf->getIntegral( ranges["pulser"]->first,   ranges["pulser"]->second);

    v_pul_int       ->at(iwf) = (pulser_integral);       // Pulser: Integral over Pulserrange
    v_pul_spread    ->at(iwf) = (pulser);                // Pulser: Spread in Pulserrange
}

void FileWriterTreeDRS4::FillTotalRange(int iwf, const StandardWaveform *wf, int pol){

    float median2            = pol*wf->getMedian(0, 1023);
    float average   = pol*wf->getIntegral(0,1023);
    float abs_max           = wf->getAbsMaxInRange(0,1023);

    v_is_saturated  ->at(iwf) = (abs_max>498);          // indicator if saturation is reached in sampling region (1-1024)
    v_median        ->at(iwf) = (median2);               // Median over whole sampling region
    v_average       ->at(iwf) = average;
    float threshold = 20.0;
    bool flag =    ( abs(wf->GetData()->at(0) - wf->GetData()->at(2)) > threshold);
    flag = flag || ( abs(wf->GetData()->at(1) - wf->GetData()->at(2)) > threshold);
    flag = flag || ( abs(wf->GetData()->at(1) - wf->GetData()->at(1)) > threshold);
    v_has_spikes->at(iwf) = flag;
}



void FileWriterTreeDRS4::UpdateWaveforms(int iwf, const StandardWaveform *wf){
    data = wf->GetData();
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
}

int FileWriterTreeDRS4::IsPulserEvent(const StandardWaveform *wf){
    f_pulser_int = wf->getIntegral(740, 860, true);
    return f_pulser_int > 80.;
} //end IsPulserEvent

} //end namespace eudaq
#endif // ROOT_FOUND
