/*
 * WaveformHistos.cc
 *
 *  Created on: Jun 16, 2011
 *      Author: stanitz
 */

#include "WaveformHistos.hh"
#include "OnlineMon.hh"
#include <math.h>

WaveformHistos::WaveformHistos(SimpleStandardWaveform p, RootMonitor * mon): _n_wfs(10),_sensor(p.getName()), _id(p.getID()),n_fills(0),_n_samples(1024)
{
    char out[1024], out2[1024];
    _mon=mon;
    min_wf = 1e9;
    max_wf = -1e9;

    // signal_integral_range = make_pair(500.,800.);
    signal_integral_range   = make_pair(   25, 125.);
    pedestal_integral_range = make_pair(  350, 450.);// should be the same length as signal
    //	std::cout << "WaveformHistos::Sensorname: " << _sensor << " "<< _id<< std::endl;
    this->InitHistos();
}


void WaveformHistos::InitIntegralHistos(){
//    std::cout<<"WaveformHistos::InitIntegralHistos"<<std::endl;
    float minVolt = -500; //-1000;
    float maxVolt = 500; //+1000;
    int nbins =  1000; //2000;

    TString hName = TString::Format("h_FullIntegral_%s_%d",_sensor.c_str(),_id);
    TString hTitle = TString::Format("%s %d: FullIntegral; full integral/mV; number of entries",_sensor.c_str(),_id);
    histos["FullIntegral"] = new TH1F(hName,hTitle,nbins,minVolt,maxVolt);
    SetMaxRangeX((string)hName,-50,50);

    hName = TString::Format("h_PulserIntegral_%s_%d",_sensor.c_str(),_id);
    hTitle = TString::Format("%s %d: PulserIntegral; pulser integral/mV (%5.1f - %5.1f); number of entries",
            _sensor.c_str(),_id,pulser_integral_range.first,pulser_integral_range.second);
    histos["PulserIntegral"] = new TH1F(hName,hTitle,nbins,minVolt,maxVolt);
    SetMaxRangeX((string)hName,-50,50);

    hName = TString::Format("h_SignalIntegral_%s_%d",_sensor.c_str(),_id);
    hTitle = TString::Format("%s %d: SignalIntegral Range (%5.1f - %5.1f); signal integral/mV; number of entries",
            _sensor.c_str(),_id,signal_integral_range.first,signal_integral_range.second);
    histos["SignalIntegral"] = new TH1F(hName,hTitle,nbins,minVolt,maxVolt);
    SetMaxRangeX((string)hName,-50,500);

    hName = TString::Format("h_PedestalIntegral_%s_%d",_sensor.c_str(),_id);
    hTitle = TString::Format("%s %d: PedestalIntegral Range (%5.1f - %5.1f); pedestal integral/mV; number of entries",
            _sensor.c_str(),_id,pedestal_integral_range.first,pedestal_integral_range.second);
    histos["PedestalIntegral"] = new TH1F(hName,hTitle,nbins,minVolt,maxVolt);
    SetMaxRangeX((string)hName,-30, 30);

    hName = TString::Format("h_DeltaIntegral_%s_%d",_sensor.c_str(),_id);
    hTitle = TString::Format("%s %d: DeltaIntegral; signal integral/mV; number of entries",_sensor.c_str(),_id);
    histos["DeltaIntegral"] = new TH1F(hName,hTitle,nbins,minVolt,maxVolt);
    //SetMaxRangeX((string)hName,-5,200);

    hName = TString::Format("h_SignalMinusPedestal_%s_%d",_sensor.c_str(),_id);
    hTitle = TString::Format("%s %d: SignalMinusPedestal; (signalSpread-pedestalSpread)/mV; number of entries",_sensor.c_str(),_id);
    histos["SignalMinusPedestal"] = new TH1F(hName,hTitle,nbins,minVolt,maxVolt);
}

void WaveformHistos::InitHistos() {
//    std::cout<<"WaveformHistos::InitHistos"<<std::endl;
    float minVolt = -500; //-1000;
    float maxVolt = 500; //+1000;
    int nbins =  1000; //2000;
    //	for (int i = 0; i < _n_wfs; i++)
    //		_Waveforms.push_back(new TGraph());
    TString hName = TString::Format("h_minVoltage_%s_%d",_sensor.c_str(),_id);
    TString hTitle = TString::Format("%s %d: min Voltage total Range; min Volt /mV; number of entries",_sensor.c_str(),_id);
    histos["MinVoltage"] = new TH1F(hName,hTitle,nbins,minVolt,maxVolt);
    SetMaxRangeX((string)hName,-50,50);

    hName = TString::Format("h_maxVoltage_%s_%d",_sensor.c_str(),_id);
    hTitle = TString::Format("%s %d: max Voltage total Range; max Volt/mV; number of entries",_sensor.c_str(),_id);
    histos["MaxVoltage"] = new TH1F(hName,hTitle,nbins,minVolt,maxVolt);
    SetMaxRangeX((string)hName,0,500);

    hName = TString::Format("h_deltaVoltage_%s_%d",_sensor.c_str(),_id);
    hTitle = TString::Format("%s %d: delta Voltage; delta Volt/mV; number of entries",_sensor.c_str(),_id);
    histos["DeltaVoltage"] = new TH1F(hName,hTitle,nbins,minVolt,maxVolt);
    SetMaxRangeX((string)hName,0,600);

//    std::cout<<"maxSignalVoltage"<<std::endl;
    hName = TString::Format("h_maxSignalVoltage_%s_%d",_sensor.c_str(),_id);
    hTitle = TString::Format("%s %d: max Voltage in signal Range (%5.1f - %5.1f); max Volt/mV; number of entries",
            _sensor.c_str(),_id,signal_integral_range.first,signal_integral_range.second);
    histos["MaxSignalVoltage"] = new TH1F(hName,hTitle,nbins,minVolt,maxVolt);
    SetMaxRangeX((string)hName,0,500);

//    std::cout<<"minSignalVoltage"<<std::endl;
    hName = TString::Format("h_minSignalVoltage_%s_%d",_sensor.c_str(),_id);
    hTitle = TString::Format("%s %d: min Voltage in signal Range (%5.1f - %5.1f); min Volt/mV; number of entries",
            _sensor.c_str(),_id,
            signal_integral_range.first,signal_integral_range.second);
    histos["MinSignalVoltage"] = new TH1F(hName,hTitle,nbins,minVolt,maxVolt);
    SetMaxRangeX((string)hName,0,500);


//    std::cout<<"PulserMaximum"<<std::endl;
    hName = TString::Format("h_maxPulser_%s_%d",_sensor.c_str(),_id);
    hTitle = TString::Format("%s %d: max Voltage in pulser Range (%5.1f - %5.1f); max Volt/mV; number of entries",
            _sensor.c_str(),_id,pulser_integral_range.first,pulser_integral_range.second);
    histos["PulserMaximum"] = new TH1F(hName,hTitle,nbins,minVolt,maxVolt);
    SetMaxRangeX((string)hName,0,500);

//    std::cout<<"PulserMinimum"<<std::endl;
    hName = TString::Format("h_minPulser_%s_%d",_sensor.c_str(),_id);
    hTitle = TString::Format("%s %d: min Voltage in pulser Range (%5.1f - %5.1f); min Volt/mV; number of entries",
            _sensor.c_str(),_id,pulser_integral_range.first,pulser_integral_range.second);
    histos["PulserMinimum"] = new TH1F(hName,hTitle,nbins,minVolt,maxVolt);
    SetMaxRangeX((string)hName,0,500);

    InitIntegralHistos();
    InitProfiles();
}

void WaveformHistos::InitProfiles(){
//    std::cout<<"WaveformHistos::InitProfiles"<<std::endl;
    float minVolt = -500; //-1000;
    float maxVolt = 500; //+1000;
    int nbins =  1000; //2000;
    TString hName = TString::Format("h_ProfileDelta_%s_%d",_sensor.c_str(),_id);
    TString hTitle = TString::Format("%s %d: Profile Delta; event number / 5000events; signal/mV",_sensor.c_str(),_id);
    profiles["DeltaVoltage"] = new TProfile(hName,hTitle,1,0,1000);
    profiles["DeltaVoltage"]->SetStats(false);

    hName = TString::Format("h_ProfileFullIntegral_%s_%d",_sensor.c_str(),_id);
    hTitle = TString::Format("%s %d: Profile FullIntegral; event number / 5000events; signal/mV",_sensor.c_str(),_id);
    profiles["FullIntegral"] = new TProfile(hName,hTitle,1,0,1000);
    profiles["FullIntegral"]->SetStats(false);

    hName = TString::Format("h_ProfileSignalIntegral_%s_%d",_sensor.c_str(),_id);
    hTitle = TString::Format("%s %d: Profile SignalIntegral (%5.1f - %5.1f); event number / 5000events; signal/mV",
            _sensor.c_str(),_id,
            signal_integral_range.first,
            signal_integral_range.second);
    profiles["SignalIntegral"] = new TProfile(hName,hTitle,1,0,1000);
    profiles["SignalIntegral"]->SetStats(false);

    hName = TString::Format("h_ProfilePulserIntegral_%s_%d",_sensor.c_str(),_id);
    hTitle = TString::Format("%s %d: Profile PulserIntegral (%5.1f - %5.1f); event number / 5000events; signal/mV",
            _sensor.c_str(),_id,
            pulser_integral_range.first,
            pulser_integral_range.second);
    profiles["PulserIntegral"] = new TProfile(hName,hTitle,1,0,1000);
    profiles["PulserIntegral"]->SetStats(false);

    hName = TString::Format("h_ProfilePedestalIntegral_%s_%d",_sensor.c_str(),_id);
    hTitle = TString::Format("%s %d: Profile PedestalIntegral (%5.1f - %5.1f); event number / 5000events; signal/mV",
            _sensor.c_str(),_id,
            pedestal_integral_range.first,
            pedestal_integral_range.second);
    profiles["PedestalIntegral"] = new TProfile(hName,hTitle,1,0,1000);
    profiles["PedestalIntegral"]->SetStats(false);

    hName = TString::Format("h_ProfileDeltaIntegral_%s_%d",_sensor.c_str(),_id);
    hTitle = TString::Format("%s %d: Profile SignalIntegral-PedestalIntegral; event number / 5000events; signal/mV",_sensor.c_str(),_id);
    profiles["DeltaIntegral"] = new TProfile(hName,hTitle,1,0,1000);
    profiles["DeltaIntegral"]->SetStats(false);

    hName = TString::Format("h_wf_stack_%s_%d",_sensor.c_str(),_id);
    hTitle = TString::Format("%s %d: Waveform Stack;time; signal/mV",_sensor.c_str(),_id);
    h_wf_stack = new THStack(hName,hTitle);

    for (int i = 0; i < _n_wfs; i++){
        hName = TString::Format("Waveform_%d_%d",_id,i);
        hTitle = TString::Format("Waveform ID %d - %d",_id,i);
        _Waveforms.push_back(new TH1F(hName,hTitle,_n_samples,0,_n_samples));
        //		FixRangeY(string)
        ////		_Waveforms.back()->SetPoint(0,0,0);
        //		_Waveforms.back()->SetPoint(1,1,1);
        //		_Waveforms.back()->Draw("APL");
        if (_Waveforms.back()->GetXaxis())
            _Waveforms.back()->GetXaxis()->SetTitle("n");
        if (_Waveforms.back()->GetYaxis())
            _Waveforms.back()->GetYaxis()->SetTitle("Signal / mV");
        h_wf_stack->Add(_Waveforms.back());
    }

    for (std::map<std::string, TH1*>::iterator it = profiles.begin();it!=profiles.end();it++){
        it->second->SetMinimum(0);
    }
}

void WaveformHistos::Reinitialize_Waveforms() {
    //	if (h_wf_stack){
    //		delete h_wf_stack;
    //	}
    cout<<"WaveformHistos::Reinitialize_Waveforms of "<<_sensor.c_str()<<"_"<<_id<<" with "<<getNSamples()<<" Samples."<<endl;
    //	TString hName = TString::Format("h_wf_stack_%s_%d",_sensor.c_str(),_id);
    //	TString hTitle = TString::Format("%s %d: Waveform Stack;time; signal/mV",_sensor.c_str(),_id);
    ////	h_wf_stack = new THStack(hName,hTitle);
    for (int i = 0; i < _n_wfs; i++){
        //		hName = TString::Format("Waveform_%d_%d",_id,i);
        //		hTitle = TString::Format("Waveform ID %d - %d",_id,i);
        TH1F* histo = _Waveforms.at(i);
        histo->SetBins(_n_samples,0,_n_samples);
        //		_Waveforms.push_back(new TH1F(hName,hTitle,_n_samples,0,_n_samples));
        //		FixRangeY(string)
        ////		_Waveforms.back()->SetPoint(0,0,0);
        //		_Waveforms.back()->SetPoint(1,1,1);
        //		_Waveforms.back()->Draw("APL");
        //		if (_Waveforms.back()->GetXaxis())
        //			_Waveforms.back()->GetXaxis()->SetTitle("n");
        //		if (_Waveforms.back()->GetYaxis())
        //			_Waveforms.back()->GetYaxis()->SetTitle("Signal / mV");
        //		h_wf_stack->Add(_Waveforms.back());
    }



}

void WaveformHistos::Fill(const SimpleStandardWaveform & wf)
{
//    std::cout<<"WaveformHistos::Fill"<<std::endl;
    if (wf.getNSamples() > this->getNSamples()){
        _n_samples = wf.getNSamples();
        Reinitialize_Waveforms();
    }
    bool isPulserEvent = wf.isPulserEvent();
    this->FillEvent(wf, isPulserEvent);
    // if (isPulserEvent)
    //     this->FillPulserEvent(wf);
    // else
    //     this->FillSignalEvent(wf);
}
void WaveformHistos::FillEvent(const SimpleStandardWaveform & wf, bool isPulserEvent){
//    std::cout<<"WaveformHistos::FillSignalEvent "<<std::endl;
    int event_no = wf.getEvent();
    ULong64_t timestamp = wf.getTimestamp();
    int sign = wf.getSign(); //why is this here? it's never properly assigned

    float min      = wf.getMin();
    float max      = wf.getMax();
    float delta    = fabs(max-min);
    float integral = wf.getIntegral();
    float signalSpread   = wf.maxSpreadInRegion(signal_integral_range.first  ,signal_integral_range.second);
    float pedestalSpread = wf.maxSpreadInRegion(pedestal_integral_range.first,pedestal_integral_range.second);

    float signal_integral   = wf.getIntegral(signal_integral_range.first  ,signal_integral_range.second);
    float signal_maximum    = wf.getMaximum (signal_integral_range.first  ,signal_integral_range.second);
    float signal_minimum    = wf.getMinimum (signal_integral_range.first  ,signal_integral_range.second);
    float pedestal_integral = wf.getIntegral(pedestal_integral_range.first,pedestal_integral_range.second);

    if(!isPulserEvent){
        histos["FullIntegral"]     -> Fill(sign*integral);
        histos["SignalIntegral"]   -> Fill(signalSpread);
        histos["PedestalIntegral"] -> Fill(pedestalSpread);
        histos["DeltaIntegral"]    -> Fill((signal_integral - pedestal_integral)*sign);
        histos["SignalMinusPedestal"]    -> Fill(signalSpread-pedestalSpread);
        histos["MinVoltage"]       -> Fill(max);//sign<0?sign*max:sign*min);
        histos["MaxVoltage"]       -> Fill(min);//sign<0?sign*min:sign*max);
        histos["MinSignalVoltage"] -> Fill(signal_minimum);
        histos["MaxSignalVoltage"] -> Fill(signal_maximum);
        histos["DeltaVoltage"]     -> Fill(delta);
        for (std::map<std::string, TH1*>::iterator it = profiles.begin();it!=profiles.end();it++){
            if (it->second->GetXaxis()->GetXmax() < event_no){
                int bins = (event_no+5000)/5000;
                int max = (bins)*5000;
                it->second->SetBins(bins,0,max);
                //			cout<<it->first<<": Extend Profile "<<bins<<" "<<max<<endl;
            }
            if (it->first == "DeltaVoltage")
                it->second->Fill(event_no,delta);
            else if (it->first == "FullIntegral")
                it->second->Fill(event_no,sign*integral);
            else if (it->first == "SignalIntegral")
                it->second->Fill(event_no,sign*signal_integral);
            else if (it->first == "PedestalIntegral")
                it->second->Fill(event_no,sign*pedestal_integral);
            else if (it->first == "DeltaIntegral")
                it->second->Fill(event_no,sign*(signal_integral-pedestal_integral));
            if (event_no % 5000 == 0 && event_no >20000){
                TF1* fit = new TF1("expoFit", "pol0(0)+expo(1)",0,event_no+5000);
                it->second->Fit(fit,"Q");
                delete fit;
            }
        }
    }
    else { // PULSER EVENTS
        histos["Pulser_FullIntegral"]     -> Fill(sign*integral);
        histos["Pulser_SignalIntegral"]   -> Fill(signalSpread);
        histos["Pulser_PedestalIntegral"] -> Fill(pedestalSpread);
        histos["Pulser_DeltaIntegral"]    -> Fill((signal_integral - pedestal_integral)*sign);
        histos["Pulser_SignalMinusPedestal"]    -> Fill(signalSpread-pedestalSpread);
        histos["Pulser_MinVoltage"]       -> Fill(max);//sign<0?sign*max:sign*min);
        histos["Pulser_MaxVoltage"]       -> Fill(min);//sign<0?sign*min:sign*max);
        histos["Pulser_MinSignalVoltage"] -> Fill(signal_minimum);
        histos["Pulser_MaxSignalVoltage"] -> Fill(signal_maximum);
        histos["Pulser_DeltaVoltage"]     -> Fill(delta);
        for (std::map<std::string, TH1*>::iterator it = profiles.begin();it!=profiles.end();it++){
            if (it->second->GetXaxis()->GetXmax() < event_no){
                int bins = (event_no+5000)/5000;
                int max = (bins)*5000;
                it->second->SetBins(bins,0,max);
                //			cout<<it->first<<": Extend Profile "<<bins<<" "<<max<<endl;
            }
            if (it->first == "Pulser_DeltaVoltage")
                it->second->Fill(event_no,delta);
            else if (it->first == "Pulser_FullIntegral")
                it->second->Fill(event_no,sign*integral);
            else if (it->first == "Pulser_SignalIntegral")
                it->second->Fill(event_no,sign*signal_integral);
            else if (it->first == "Pulser_PedestalIntegral")
                it->second->Fill(event_no,sign*pedestal_integral);
            else if (it->first == "Pulser_DeltaIntegral")
                it->second->Fill(event_no,sign*(signal_integral-pedestal_integral));
            if (event_no % 5000 == 0 && event_no >20000){
                TF1* fit = new TF1("expoFit", "pol0(0)+expo(1)",0,event_no+5000);
                it->second->Fit(fit,"Q");
                delete fit;
            }
        }
    }
    UpdateRanges();
    TH1F* gr = _Waveforms[n_fills%_n_wfs];
    for (int n = 0; n < wf.getNSamples();n++)
        gr->SetBinContent(n+1,wf.getData()[n]);
    for (int i = 0; i < _n_wfs; i++)
        _Waveforms[(n_fills-i)%_n_wfs]->SetLineColor(kAzure+i);
    gr->SetEntries(event_no);
    n_fills++;

    if (n_fills<=1){
        //		gr->Draw("APL");
        if (gr->GetXaxis())
            gr->GetXaxis()->SetTitle("n");
        if (gr->GetYaxis())
            gr->GetYaxis()->SetTitle("Signal / mV");
    }
}


void WaveformHistos::FillPulserEvent(const SimpleStandardWaveform & wf){
//    std::cout<<"WaveformHistos::FillPulserEvent "<<std::endl;
    int event_no = wf.getEvent();
    ULong64_t timestamp = wf.getTimestamp();
    int sign = wf.getSign(); //why is this here? it's never properly assigned
//    std::cout<<" Ignoring Pulser Event "<<std::endl;
    float pulser_integral = wf.getIntegral(pulser_integral_range.first,pulser_integral_range.second);
    float pulser_maximum = wf.getMaximum(pulser_integral_range.first,pulser_integral_range.second);
    float pulser_minimum = wf.getMinimum(pulser_integral_range.first,pulser_integral_range.second);
//    std::cout<<"Fill1"<<std::endl;
    histos["PulserIntegral"] -> Fill(sign* pulser_integral);
//    std::cout<<"Fill2"<<std::endl;
    histos["PulserMaximum"] -> Fill(sign* pulser_maximum);
    histos["PulserMinimum"] -> Fill(sign* pulser_minimum);
    return;
}
void WaveformHistos::FillSignalEvent(const SimpleStandardWaveform & wf){
//    std::cout<<"WaveformHistos::FillSignalEvent "<<std::endl;
    int event_no = wf.getEvent();
    ULong64_t timestamp = wf.getTimestamp();
    int sign = wf.getSign(); //why is this here? it's never properly assigned

    float min      = wf.getMin();
    float max      = wf.getMax();
    float delta    = fabs(max-min);
    float integral = wf.getIntegral();
    float signalSpread   = wf.maxSpreadInRegion(signal_integral_range.first  ,signal_integral_range.second);
    float pedestalSpread = wf.maxSpreadInRegion(pedestal_integral_range.first,pedestal_integral_range.second);

    float signal_integral   = wf.getIntegral(signal_integral_range.first  ,signal_integral_range.second);
    float signal_maximum    = wf.getMaximum (signal_integral_range.first  ,signal_integral_range.second);
    float signal_minimum    = wf.getMinimum (signal_integral_range.first  ,signal_integral_range.second);
    float pedestal_integral = wf.getIntegral(pedestal_integral_range.first,pedestal_integral_range.second);

//     cout << "first and second: " << signal_integral_range.first << " " << signal_integral_range.second << endl;
//     cout << "this is the signal   integral: " << signal_integral << endl;
//     cout << "this is the pedestal integral: " << pedestal_integral << endl;
//     cout << "this is the delta integral: " << signal_integral - pedestal_integral << endl;

    histos["FullIntegral"]     -> Fill(sign*integral);
    // histos["SignalIntegral"]   -> Fill(sign*signal_integral);
    histos["SignalIntegral"]   -> Fill(signalSpread);
    // histos["PedestalIntegral"] -> Fill(sign*pedestal_integral);
    histos["PedestalIntegral"] -> Fill(pedestalSpread);
//    std::cout<<"Fill1"<<std::endl;
    histos["DeltaIntegral"]    -> Fill((signal_integral - pedestal_integral)*sign);
    histos["SignalMinusPedestal"]    -> Fill(signalSpread-pedestalSpread);
    histos["MinVoltage"]       -> Fill(max);//sign<0?sign*max:sign*min);
    histos["MaxVoltage"]       -> Fill(min);//sign<0?sign*min:sign*max);
//    std::cout<<"Fill2"<<std::endl;
    histos["MinSignalVoltage"] -> Fill(signal_minimum);
    histos["MaxSignalVoltage"] -> Fill(signal_maximum);
//    std::cout<<"Fill3"<<std::endl;
    histos["DeltaVoltage"]     -> Fill(delta);
//    std::cout<<"FillProfiles"<<std::endl;
    for (std::map<std::string, TH1*>::iterator it = profiles.begin();it!=profiles.end();it++){
        if (it->second->GetXaxis()->GetXmax() < event_no){
            int bins = (event_no+5000)/5000;
            int max = (bins)*5000;
            it->second->SetBins(bins,0,max);
            //			cout<<it->first<<": Extend Profile "<<bins<<" "<<max<<endl;
        }
        if (it->first == "DeltaVoltage")
            it->second->Fill(event_no,delta);
        else if (it->first == "FullIntegral")
            it->second->Fill(event_no,sign*integral);
        else if (it->first == "SignalIntegral")
            it->second->Fill(event_no,sign*signal_integral);
        else if (it->first == "PedestalIntegral")
            it->second->Fill(event_no,sign*pedestal_integral);
        else if (it->first == "DeltaIntegral")
            it->second->Fill(event_no,sign*(signal_integral-pedestal_integral));
        if (event_no % 5000 == 0 && event_no >20000){
            TF1* fit = new TF1("expoFit", "pol0(0)+expo(1)",0,event_no+5000);
            it->second->Fit(fit,"Q");
            delete fit;
        }
    }
    UpdateRanges();
    //	cout<<"Name: "<<wf.getName()<<" ID: "<<wf.getID()<<endl;
    //	TString name = TString::Format("wf_%s_%d_%d",wf.getName().c_str(),wf.getID());
    TH1F* gr = _Waveforms[n_fills%_n_wfs];
    for (int n = 0; n < wf.getNSamples();n++)
        gr->SetBinContent(n+1,wf.getData()[n]);
    for (int i = 0; i < _n_wfs; i++)
        _Waveforms[(n_fills-i)%_n_wfs]->SetLineColor(kAzure+i);
    gr->SetEntries(event_no);
    n_fills++;
    //	if (!gr->GetN())
    //		return;
    //	if (gr->GetXaxis())
    //		gr->GetXaxis()->SetTitle("n");
    //	if (gr->GetYaxis())
    //		gr->GetYaxis()->SetTitle("Signal / mV");

    if (n_fills<=1){
        //		gr->Draw("APL");
        if (gr->GetXaxis())
            gr->GetXaxis()->SetTitle("n");
        if (gr->GetYaxis())
            gr->GetYaxis()->SetTitle("Signal / mV");
    }
}

void WaveformHistos::Reset() {
    for (int i = 0; i < _Waveforms.size(); i++)
        _Waveforms[i]->Reset();
    // we have to reset the aux array as well
    std::map<std::string, TH1*>::iterator it;
    for (it = histos.begin(); it != histos.end();it++)
        it->second->Reset();
    for (it = profiles.begin(); it != profiles.end();it++)
        it->second->Reset();

    n_fills = 0;
}

void WaveformHistos::Calculate(const int currentEventNum)
{
    _wait = true;
    //	cout<<"WaveformHistos::Calculate"<<currentEventNum<<endl;
    _wait = false;
}


void WaveformHistos::Write()
{
    //	_Waveform->Write();
}

int WaveformHistos::SetHistoAxisLabelx(TH1* histo,string xlabel)
{
    if (histo!=NULL)
    {
        histo->GetXaxis()->SetTitle(xlabel.c_str());
    }
    return 0;
}

void WaveformHistos::SetPedestalIntegralRange(float min, float max) {
    pedestal_integral_range = make_pair(min,max);
    TString hTitle = TString::Format("%s %d: Profile PedestalIntegral (%5.1f - %5.1f)",
            _sensor.c_str(),_id,min,max);
    profiles["PedestalIntegral"]->SetTitle(hTitle);
}

void WaveformHistos::SetSignalIntegralRange(float min, float max) {
    signal_integral_range = make_pair(min,max);
    TString hTitle = TString::Format("%s %d: Profile PedestalIntegral (%5.1f - %5.1f)",
            _sensor.c_str(),_id,min,max);
    profiles["SignalIntegral"]->SetTitle(hTitle);
}

TProfile* WaveformHistos::getProfile(std::string key) const {
//    std::cout<<"WaveformHistos::getProfile "<<key<<std::endl;
    std::map<std::string, TH1*>::const_iterator it = profiles.find(key);
    if ( it == profiles.end()) return 0;
    return (TProfile*)profiles.at(key);
}

TH1F* WaveformHistos::getHisto(std::string key) const {
//    std::cout<<"WaveformHistos::getHisto"<<key<<std::endl;
    std::map<std::string, TH1*>::const_iterator it = profiles.find(key);
    if ( it == profiles.end()) return 0;
    return (TH1F*)histos.at(key);
}

int WaveformHistos::SetHistoAxisLabels(TH1* histo,string xlabel, string ylabel)
{
    SetHistoAxisLabelx(histo,xlabel);
    SetHistoAxisLabely(histo,ylabel);
    return 0;
}



int WaveformHistos::SetHistoAxisLabely(TH1* histo,string ylabel)
{
    if (histo!=NULL)
    {
        histo->GetYaxis()->SetTitle(ylabel.c_str());
    }
    return 0;
}

void WaveformHistos::SetMaxRangeX(std::string name, float min, float max) {
    if (min<max)
        rangesX[name] = make_pair(min,max);
    else
        std::cout<<"Cannot SetMaxRangeY: "<<name<< " to "<<min<<max<<std::endl;
}

void WaveformHistos::SetMaxRangeY(std::string name, float min, float max) {
    if (min<max)
        rangesY[name] = make_pair(min,max);
    else
        std::cout<<"Cannot SetMaxRangeY: "<<name<< " to "<<min<<max<<std::endl;
}


void WaveformHistos::UpdateRanges() {
    std::map<std::string, TH1*>::iterator it;
    for (it = histos.begin(); it != histos.end();it++)
        UpdateRange(it->second);

    float min = _Waveforms[0]->GetBinContent(_Waveforms[0]->GetMinimumBin());
    float max = _Waveforms[0]->GetBinContent(_Waveforms[0]->GetMaximumBin());
    bool changed = false;
    if (min < min_wf){
        this->min_wf = min;
        changed = true;
    }
    else
        min = min_wf;
    if (max > max_wf){
        this->max_wf = max;
        changed = true;
    }
    else
        max = max_wf;
    if (changed){
        float delta = max - min;
        min = min - .05 * delta;
        max = max + .05 * delta;
        //	cout<<"Range: "<<min<<" "<<max<<endl;
        _Waveforms[0]->GetYaxis()->SetRangeUser(min,max);
    }

}

void WaveformHistos::UpdateRange(TH1* histo) {
    //	cout<<"Update range for "<<histo->GetName()<<endl;
    int binLow = histo->FindFirstBinAbove();
    int binHigh = histo->FindLastBinAbove();
    int delta = binHigh-binLow;
    binLow -= int(.05*delta+1);
    binHigh += int(.05*delta+1);
    binLow = binLow<1?1:binLow;
    binHigh = histo->GetNbinsX()<binHigh?histo->GetNbinsX():binHigh;
    std::string name = histo->GetName();
    if (rangesX.find(name) != rangesX.end()){
        pair<float, float> range = rangesX.find(name)->second;
        //		std::cout<<"  Max range of "<<histo->GetName()<<" "<<range.first<<"-"<<range.second<<std::endl;
        //		std::cout<<"  entries in "<<histo->GetXaxis()->GetBinLowEdge(binLow)<<"-"<<histo->GetXaxis()->GetBinUpEdge(binHigh)<<std::endl;
        if (binLow < histo->GetXaxis()->FindBin(range.first))
            binLow = histo->GetXaxis()->FindBin(range.first);
        if (binHigh > histo->GetXaxis()->FindBin(range.second))
            binHigh = histo->GetXaxis()->FindBin(range.second);
        //		std::cout<<"  ==> new range in "<<histo->GetXaxis()->GetBinLowEdge(binLow)<<"-"<<histo->GetXaxis()->GetBinUpEdge(binHigh)<<std::endl;
    }
    histo->GetXaxis()->SetRange(binLow,binHigh);
}
