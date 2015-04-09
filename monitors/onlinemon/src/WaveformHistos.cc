/*
 * WaveformHistos.cc
 *
 *  Created on: Jun 16, 2011
 *      Author: stanitz
 */

#include "WaveformHistos.hh"
#include "OnlineMon.hh"


WaveformHistos::WaveformHistos(SimpleStandardWaveform p, RootMonitor * mon): _n_wfs(10),_sensor(p.getName()), _id(p.getID()),n_fills(0)
{
	char out[1024], out2[1024];
	_mon=mon;
	//	std::cout << "WaveformHistos::Sensorname: " << _sensor << " "<< _id<< std::endl;
	this->InitHistos();
}

void WaveformHistos::InitHistos() {
	float minVolt = -2000;
	float maxVolt = +2000;
	//	for (int i = 0; i < _n_wfs; i++)
	//		_Waveforms.push_back(new TGraph());
	TString hName = TString::Format("h_minVoltage_%s_%d",_sensor.c_str(),_id);
	TString hTitle = TString::Format("%s %d: min Voltage",_sensor.c_str(),_id);
	h_minVoltage = new TH1F(hName,hTitle,2000,minVolt,maxVolt);
	hName = TString::Format("h_maxVoltage_%s_%d",_sensor.c_str(),_id);
	hTitle = TString::Format("%s %d: max Voltage",_sensor.c_str(),_id);
	h_maxVoltage = new TH1F(hName,hTitle,2000,minVolt,maxVolt);
	hName = TString::Format("h_deltaVoltage_%s_%d",_sensor.c_str(),_id);
	hTitle = TString::Format("%s %d: delta Voltage",_sensor.c_str(),_id);
	h_deltaVoltage = new TH1F(hName,hTitle,2000,minVolt,maxVolt);
	hName = TString::Format("h_FullIntegral_%s_%d",_sensor.c_str(),_id);
	hTitle = TString::Format("%s %d: FullIntegral",_sensor.c_str(),_id);
	h_FullIntegral = new TH1F(hName,hTitle,2000,minVolt,maxVolt);
	for (int i = 0; i < _n_wfs; i++){
		_Waveforms.push_back(new TGraph());
		_Waveforms.back()->Draw("A");
		_Waveforms.back()->GetXaxis()->SetTitle("n");
		_Waveforms.back()->GetYaxis()->SetTitle("Signal / mV");
	}
}

void WaveformHistos::Fill(const SimpleStandardWaveform & wf)
{
	float min = wf.getMin();
	float max = wf.getMax();
	float delta = max-min;
	float integral = wf.getIntegral();
	h_FullIntegral->Fill(integral);
	h_minVoltage->Fill(min);
	h_maxVoltage->Fill(max);
	h_deltaVoltage->Fill(delta);
	//	cout<<"Name: "<<wf.getName()<<" ID: "<<wf.getID()<<endl;
//	TString name = TString::Format("wf_%s_%d_%d",wf.getName().c_str(),wf.getID());
	TGraph* gr = _Waveforms[n_fills%_n_wfs];
	for (int n = 0; n < wf.getNSamples();n++)
		gr->SetPoint(n,n,wf.getData()[n]);
	n_fills++;
	if (!gr->GetN())
		return;
//	if (gr->GetXaxis())
//		gr->GetXaxis()->SetTitle("n");
//	if (gr->GetYaxis())
//		gr->GetYaxis()->SetTitle("Signal / mV");

	if (n_fills<=1){
		gr->Draw("APL");
		gr->GetXaxis()->SetTitle("n");
		gr->GetYaxis()->SetTitle("Signal / mV");
	}
}

void WaveformHistos::Reset() {
	//	for (int i = 0; i < _Waveforms.size(); i++)
	//		_Waveforms[i]->Reset();
	// we have to reset the aux array as well
	h_FullIntegral->Reset();
	h_minVoltage->Reset();
	h_maxVoltage->Reset();
	h_deltaVoltage->Reset();
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


