#include <vector>
#include <sys/types.h>
#include <sys/stat.h>
#include <numeric>
#include <algorithm>

#include "dictionaries.h"
#include "constants.h"
#include "../api/api.h"
//#include "../api/datatypes.h"
//#include "../decoder/datapipe.h"
#include "datasource_evt.h"

#include "eudaq/FileNamer.hh"
#include "eudaq/FileWriter.hh"
#include "eudaq/FileSerializer.hh"
#include "eudaq/RawDataEvent.hh"

#include "TH1D.h"
#include "TH2D.h"
#include "TCanvas.h"
#include "TPaveStats.h"
#include "TList.h"
#include "TF1.h"
#include "TGraph.h"

//#include "eudaq/Logger.hh"

using namespace pxar;

namespace eudaq {


    class FileWriterHitmap : public FileWriter {
    public:
        FileWriterHitmap(const std::string &);
        virtual void StartRun(unsigned);
        virtual void WriteEvent(const DetectorEvent &);
        virtual uint64_t FileBytes() const;
        virtual ~FileWriterHitmap();
        static std::vector<uint16_t> TransformRawData(const std::vector<unsigned char> & block) {

            // Transform data of form char* to vector<int16_t>
            std::vector<uint16_t> rawData;

            int size = block.size();
            if(size < 2) { return rawData; }

            int i = 0;
            while(i < size-1) {
                uint16_t temp = ((uint16_t)block.data()[i+1] << 8) | block.data()[i];
                rawData.push_back(temp);
                i+=2;
            }
            return rawData;
        }
        void writeHistogram(TH1D* h1, std::string name);
        void writeHistogram(TH2D* h2, std::string name);
        void writeHistogram(TH1D* h1, std::string name, float statsX, float statsY, char* drawOption = "");
        void writeHistogram(TH2D* h2, std::string name, float statsX, float statsY);

        const char* analysisRevision = "v1.15 /H";
    private:

        uint8_t m_roctype, m_tbmtype;
        size_t m_nplanes;

        evtSource src;
        passthroughSplitter splitter;
        dtbEventDecoder decoder;
        dataSink<pxar::Event*> Eventpump;
        pxar::Event* evt ;

        FileSerializer * m_ser;

        std::string outputFolder;
        TH2D* pkamMap;
        TH2D* pkamHitmap;
        TH2D* bgMap;
        TH2D* bgMapNotHot;
        TH2D* totalADCMap;
        TH2D* meanADCMap;
        TH2D* totalChargeMap;
        TH2D* meanChargeMap;
        TH2D* errorsMap;
        TH2D* clusterMap;
        TH1D* adcDistribution;
        TH1D* pixelChargeDistribution;
        TH1D* clusterChargeDistribution;

        TH1D* clusterSizeDistribution;

        TH1D* hitsPerNEventsVsTime;
        TH1D* chargePerNEventsVsTime;

        int ntrig;
        bool eventSourceCreated;
        std::string htmlText;
        int m_runnumber;
        int m_npkam;
        int m_npkammax;
        int m_nreadouterrors;
        int m_dumpeventsmax;
        int m_dumpedevents;
        int m_eventcounter;

        std::vector< std::pair < double, double > > phCalibrationTable;
        bool phCalibrationOK;
        std::vector< int > lowVcalValues;

        std::vector< std::pair< std::pair<int,int>, double> > chargesList;

    };

    namespace {
        static RegisterFileWriter<FileWriterHitmap> reg("hitmap");
    }

    FileWriterHitmap::FileWriterHitmap(const std::string & /*param*/) : m_ser(0) {


    }

    void FileWriterHitmap::StartRun(unsigned runnumber) {
        //delete m_ser;
        //m_ser = new FileSerializer(FileNamer(m_filepattern).Set('X', ".raw").Set('R', runnumber));
        std::cout << "start run" << runnumber << std::endl;
        m_runnumber = runnumber;
        eventSourceCreated = false;

        std::cout << m_filepattern << std::endl;
        std::string foutput(FileNamer(m_filepattern).Set('X', "/").Set('R', runnumber));
        std::cout << "writing files to " << foutput << std::endl;
        outputFolder = foutput;
        mkdir(outputFolder.c_str(), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);


        phCalibrationOK = false;
        std::ifstream phCalibrationFile("phCalibration_C0.dat");
        if (phCalibrationFile) {
            std::cout << "PH calibration found...\n";
            std::string line;
            std::string lowVcalValuesLine;
            std::getline(phCalibrationFile, line);
            std::getline(phCalibrationFile, lowVcalValuesLine);
            std::stringstream ss(lowVcalValuesLine);
            std::cout << "VCALS:" << lowVcalValuesLine << std::endl;
            std::getline(ss, line, ':');
            while (std::getline(ss, line, ' ')) {
                line.erase(remove_if(line.begin(), line.end(), ::isspace), line.end());
                if (line.size() > 0) {
                    int lowVcal = stoi(line);
                    lowVcalValues.push_back(lowVcal);
                }
            }
            std::getline(phCalibrationFile, line);
            std::getline(phCalibrationFile, line);

            std::cout << "pixels..." << std::endl;
            for (int iPix=0;iPix<4160;iPix++) {
                std::string line;
                std::getline(phCalibrationFile, line);
                std::vector< float > vcals;
                std::vector< float > ADCs;
                std::stringstream ss(line);
                std::cout << "LINE:" << line << std::endl;
                std::string adcString;
                float gain = 0;
                float offset = 0;
                int ADC;

                int vcalIndex=0;
                while (vcalIndex<lowVcalValues.size()){
                    std::getline(ss, adcString, ' ');
                    adcString.erase(remove_if(adcString.begin(), adcString.end(), ::isspace), adcString.end());
                    if (adcString.size() > 0) {
                        ADC = stoi(adcString);
                        if (ADC > 0 && ADC < 254) {
                            vcals.push_back(lowVcalValues[vcalIndex]);
                            ADCs.push_back(ADC);
                            std::cout << "(" << lowVcalValues[vcalIndex] << " | " << ADC << "), ";
                        }
                        vcalIndex++;
                    }

                }

                std::cout << "..." << std::endl;
                if (vcals.size() > 0) {
                    TGraph *tg = new TGraph(vcals.size(), &vcals[0], &ADCs[0]);
                    TF1 *linearFit = new TF1("pol1", "pol1", 0, 256);
                    linearFit->SetParLimits(0, -255, 255);
                    linearFit->SetParLimits(1, 0, 100);
                    linearFit->SetParameter(1, 0.33);

                    tg->Fit(linearFit, "QBSR");
                    gain = linearFit->GetParameter(1);
                    offset = linearFit->GetParameter(0);
                }

                phCalibrationTable.push_back(std::make_pair(offset, gain));
                std::cout << " pix " << iPix << " : " << offset << " + VCAL * " << gain << std::endl;

            }
            phCalibrationOK = true;

            phCalibrationFile.close();
        } else {
            std::cout << "NO PH calibration found!!!!! \n";
        }

        bgMap = new TH2D("bgMap", "bgMap", 52, 0, 52, 80, 0 ,80);
        bgMapNotHot = new TH2D("bgMapNotHot", "bgMapNotHot", 52, 0, 52, 80, 0 ,80);
        pkamMap = new TH2D("pkamMap", "pkamMap", 52, 0, 52, 80, 0 ,80);
        errorsMap = new TH2D("errorsMap", "errorsMap", 52, 0, 52, 80, 0 ,80);

        totalADCMap = new TH2D("totalADCMap", "totalADCMap", 52, 0, 52, 80, 0 ,80);
        meanADCMap = new TH2D("meanADCMap", "meanADCMap", 52, 0, 52, 80, 0 ,80);

        totalChargeMap = new TH2D("totalChargeMap", "totalChargeMap", 52, 0, 52, 80, 0 ,80);
        meanChargeMap = new TH2D("meanChargeMap", "meanChargeMap", 52, 0, 52, 80, 0 ,80);

        clusterMap = new TH2D("clusterMap", "clusterMap", 104, 0, 52, 160, 0 ,80);
        clusterSizeDistribution = new TH1D("clusterSizeDistribution", "clusterSizeDistribution", 38, 0, 38);

        hitsPerNEventsVsTime = new TH1D("hitsPerNEventsVsTime", "hitsPerNEventsVsTime", 21000, 0, 210000);
        chargePerNEventsVsTime = new TH1D("chargePerNEventsVsTime", "chargePerNEventsVsTime", 21000, 0, 210000);

        pixelChargeDistribution = new TH1D("pixelChargeDistribution", "pixelChargeDistribution", 1000, -500, 2500);
        clusterChargeDistribution = new TH1D("clusterChargeDistribution", "clusterChargeDistribution", 5000, -500, 4500);
        adcDistribution = new TH1D("adcDistribution", "adcDistribution", 256, 0, 256.0);


        ntrig = 10;
        m_npkam = 0;
        m_npkammax = 10;
        m_nreadouterrors = 0;
        m_dumpeventsmax = 10;
        m_dumpedevents = 0;
        m_eventcounter = 0;

        std::stringstream ss;
        ss << "<strong>RUN " << runnumber << ":</strong><br><i>analysis rev:" << analysisRevision << "</i><br><br>";
        htmlText += ss.str();
    }

    void FileWriterHitmap::WriteEvent(const DetectorEvent & ev) {

        if (ev.IsBORE()) {
            std::cout << "loading the first event...." << std::endl;

            DeviceDictionary* devDict;
            std::string roctype = ev.GetTag("ROCTYPE", "proc600");
            std::string tbmtype = ev.GetTag("TBMTYPE", "tbmemulator");

            // Get the number of planes:
            m_nplanes = ev.GetTag("PLANES", 1);

            m_roctype = ROC_PROC600; //devDict->getInstance()->getDevCode(roctype);
            m_tbmtype = devDict->getInstance()->getDevCode(tbmtype);

            if (!eventSourceCreated) {
                src = evtSource(0, m_nplanes, 0, m_tbmtype, m_roctype);
                src >> splitter >> decoder >> Eventpump;
            }
            std::cout << "ROC: " << roctype << " TBM:" << tbmtype << std::endl;
            return;
        } else if (ev.IsEORE()) {
            std::cout << "loading the last event...." << std::endl;
            return;
        }

        //std::cout << "write event"<< std::endl;
        for (size_t i=0;i<ev.NumEvents();++i)
        {
            const RawDataEvent * raw = dynamic_cast<const RawDataEvent *>(ev.GetEvent(i));

            //std::cout << "#blocks=" << raw->NumBlocks() << std::endl;
            int calCol = raw->GetBlock(1)[0];
            int calRow = raw->GetBlock(2)[0];

            // Transform from EUDAQ data, add it to the datasource:
            src.AddData(TransformRawData(raw->GetBlock(0)));
            // ...and pull it out at the other end
            evt = Eventpump.Get();

            // event has pkam?
            if (evt->hasAutoReset()) {
                std::cout << "AUTO RESET!!! #################################################" << std::endl;
            }
            if (evt->hasSyncError()) {
                std::cout << "SYNC ERR!!! #################################################" << std::endl;
            }

            // dump first few events
            if (m_dumpedevents < m_dumpeventsmax) {
                if (evt->pixels.size() > 0) {
                    std::stringstream ss;
                    ss << "nonempty_event_" << m_dumpedevents;
                    TH2D *hitmapEvent = new TH2D(ss.str().c_str(), ss.str().c_str(), 52, 0, 52, 80, 0, 80);
                    for (int j = 0; j < evt->pixels.size(); j++) {
                        if (evt->pixels[j].column() == calCol && evt->pixels[j].row() == calRow) {
                            hitmapEvent->Fill(evt->pixels[j].column(), evt->pixels[j].row(), 1);
                        } else {
                            hitmapEvent->Fill(evt->pixels[j].column(), evt->pixels[j].row(), evt->pixels[j].value());
                        }
                    }
                    std::stringstream ss2;
                    TCanvas *c1 = new TCanvas("c1", "c1", 500, 500);
                    hitmapEvent->SetStats(kFALSE);
                    hitmapEvent->Draw("colz");
                    ss2 << outputFolder << "/" << ss.str() << ".pdf";
                    c1->SaveAs(ss2.str().c_str());
                    std::cout << "dump event " << m_dumpedevents << ": " << ss2.str() << std::endl;
                    m_dumpedevents++;
                }
            }


            //------------------------------------------------------------------------------------------------------------------------------
            // PKAM RESET ERROR occurred
            //------------------------------------------------------------------------------------------------------------------------------
            if (evt->hasPkamReset()) {

                //------------------------------------------------------------------------------------------------------------------------------
                // fill PKAM map
                //------------------------------------------------------------------------------------------------------------------------------
                pkamMap->Fill(calCol, calRow);
                for (int j = 0; j < evt->pixels.size(); j++) {
                    pkamHitmap->Fill(evt->pixels[j].column(), evt->pixels[j].row());
                }

                //------------------------------------------------------------------------------------------------------------------------------
                // dump PKAM events
                //------------------------------------------------------------------------------------------------------------------------------
                if (m_npkam < m_npkammax) {
                    std::stringstream ss;
                    ss << "pkamEvent_" << m_npkam;
                    TH2D* pkamHitmapEvent = new TH2D(ss.str().c_str(), ss.str().c_str(), 52, 0, 52, 80, 0 ,80);
                    for (int j = 0; j < evt->pixels.size(); j++) {
                        pkamHitmapEvent->Fill(evt->pixels[j].column(), evt->pixels[j].row(), evt->pixels[j].value());
                    }
                    std::stringstream ss2;
                    TCanvas* c1 = new TCanvas("c1","c1",500,500);
                    pkamHitmapEvent->SetStats(kFALSE);
                    pkamHitmapEvent->Draw("colz");
                    ss2 << outputFolder << "/" << ss.str() << ".pdf";
                    c1->SaveAs(ss2.str().c_str());
                    std::cout << "dump pkam event:" << ss2.str() << std::endl;
                }
                m_npkam++;

                hitsPerNEventsVsTime->Fill(m_eventcounter, evt->pixels.size());

                //------------------------------------------------------------------------------------------------------------------------------
                // READ-OUT ERROR occurred
                //------------------------------------------------------------------------------------------------------------------------------
            } else {
                int backgroundHits=0;
                //------------------------------------------------------------------------------------------------------------------------------
                // loop over pixels
                //------------------------------------------------------------------------------------------------------------------------------
                for (int j = 0; j < evt->pixels.size(); j++) {
                    bgMap->Fill(evt->pixels[j].column(), evt->pixels[j].row());
                    adcDistribution->Fill(evt->pixels[j].value());
                    totalADCMap->Fill(evt->pixels[j].column(), evt->pixels[j].row(),evt->pixels[j].value());

                    if (phCalibrationOK) {
                        std::pair< double, double> phCalibration = phCalibrationTable[evt->pixels[j].column() * 80 + evt->pixels[j].row()];
                        double charge = 0;
                        if (phCalibration.second > 0) {
                            charge = (evt->pixels[j].value() - phCalibration.first) / phCalibration.second;
                        }
                        if (phCalibration.second > 0.05) {
                            chargesList.push_back(
                                    std::make_pair(std::make_pair(evt->pixels[j].column(), evt->pixels[j].row()), charge));
                        }
                    }
                    backgroundHits++;
                }

                //------------------------------------------------------------------------------------------------------------------------------
                // hits/event vs. time
                //------------------------------------------------------------------------------------------------------------------------------
                hitsPerNEventsVsTime->Fill(m_eventcounter, backgroundHits);

                //------------------------------------------------------------------------------------------------------------------------------
                // clustering
                //------------------------------------------------------------------------------------------------------------------------------
                int nHitsTotal = evt->pixels.size();
                std::vector < std::pair<int, int> > pixels;
                std::vector < std::pair<int, int> > pixelsNew;
                std::vector < std::pair<int, int> > cluster;

                for (int hitpix = 0; hitpix < nHitsTotal;hitpix++) {
                    pixels.push_back(std::make_pair(evt->pixels[hitpix].column(), evt->pixels[hitpix].row()));
                }

                int nClusters = 0;
                int clusterRadius = 2; //default: 2
                while (pixels.size() > 0) {
                    // take last pixel to initialize the clusters
                    if (cluster.size() < 1) {
                        cluster.push_back(pixels[pixels.size()-1]);
                        pixels.pop_back();
                    }

                    int newPixelsFound = 0;
                    for (int i=pixels.size()-1;i>=0;i--) {
                        for (int j=0;j<cluster.size();j++) {
                            if (abs(pixels[i].first-cluster[j].first) < clusterRadius && abs(pixels[i].second-cluster[j].second) < clusterRadius) {
                                // add to cluster
                                cluster.push_back(pixels[i]);
                                // remove from list
                                pixels.erase(pixels.begin()+i);
                                newPixelsFound++;
                                break;
                            }
                        }
                    }

                    if (newPixelsFound < 1) {
                        // complete cluster found!
                        nClusters++;
                        long sumCol=0;
                        long sumRow=0;
                        for (unsigned int clusterPixelIndex=0;clusterPixelIndex<cluster.size();clusterPixelIndex++) {
                            sumCol += cluster[clusterPixelIndex].first;
                            sumRow += cluster[clusterPixelIndex].second;
                        }
                        if (cluster.size() > 0) {
                            clusterMap->Fill(sumCol / (double) cluster.size(), sumRow / (double) cluster.size());
                            clusterSizeDistribution->Fill(cluster.size());
                        }
                        cluster.clear();
                    } else {
                        // remove duplicates from cluster
                        sort(cluster.begin(), cluster.end() );
                        cluster.erase(unique( cluster.begin(), cluster.end() ), cluster.end() );
                    }
                }


            }

            ntrig = raw->GetBlock(5)[0];
            m_eventcounter++;
        }

    }
    void FileWriterHitmap::writeHistogram(TH2D* h1, std::string name) {
        writeHistogram(h1, name, -1.0, -1.0);
    }

    void FileWriterHitmap::writeHistogram(TH2D* h1, std::string name, float statsPosX, float statsPosY) {
        TCanvas* c1 = new TCanvas("c1","c1",500,500);
        h1->SetStats(kFALSE);
        h1->Draw("colz");
        gPad->Update();
        if (statsPosX > -1) {
            TPaveStats *s = (TPaveStats *) gPad->GetPrimitive("stats");
            s->SetX1NDC(statsPosX);
            s->SetY1NDC(statsPosY);
        }
        std::stringstream ss;
        ss << outputFolder << "/" << name << ".pdf";
        c1->SaveAs(ss.str().c_str());
        ss.str("");
        ss.clear();
        ss << outputFolder << "/" << name << ".png";
        c1->SaveAs(ss.str().c_str());
        ss.str("");
        ss.clear();
        ss << outputFolder << "/" << name << ".root";
        c1->SaveAs(ss.str().c_str());
        ss.str("");
        ss.clear();
        c1->Clear();

        ss.str("");
        ss.clear();
        ss << "<div style='float:left;'><a href='" << name << ".pdf'><img src='" << name << ".png'></a></div><br>" ;
        htmlText += ss.str();
    }

    void FileWriterHitmap::writeHistogram(TH1D* h1, std::string name) {
        writeHistogram(h1, name, -1.0, -1.0);
    }

    void FileWriterHitmap::writeHistogram(TH1D* h1, std::string name, float statsPosX, float statsPosY, char* drawOption) {
        TCanvas* c1 = new TCanvas("c1","c1",500,500);
        h1->SetStats(kTRUE);

        h1->Draw(drawOption);
        gPad->Modified(); gPad->Update();
        float statsX = 0.15;
        float statsY = 0.65;

        if (statsPosX > -1) {
            statsX = statsPosX;
            statsY = statsPosY;
        }
        TPaveStats *st =
                (TPaveStats*)h1->GetListOfFunctions()->FindObject("stats");
        st->SetX1NDC(statsX);
        st->SetY1NDC(statsY+0.2);
        st->SetX2NDC(statsX+0.2);
        st->SetY2NDC(statsY);

        if (statsPosX < -5) {
            h1->SetStats(kFALSE);
        }
        std::stringstream ss;
        ss << outputFolder << "/" << name << ".pdf";
        c1->SaveAs(ss.str().c_str());
        ss.str("");
        ss.clear();
        ss << outputFolder << "/" << name << ".png";
        c1->SaveAs(ss.str().c_str());
        ss.str("");
        ss.clear();
        ss << outputFolder << "/" << name << ".root";
        c1->SaveAs(ss.str().c_str());
        ss.str("");
        ss.clear();
        c1->Clear();

        ss.str("");
        ss.clear();
        ss << "<a href='" << name << ".pdf'><img src='" << name << ".png'></a><br>" ;
        htmlText += ss.str();
    }

    FileWriterHitmap::~FileWriterHitmap() {
        //delete m_ser;

        int calsTotal = 0;
        int PKAMsTotal = 0;
        int bgHitsTotal=0;
        int bgHitPixels = 0;

        std::vector<double> calEffNoPKAMList;

        std::vector< double > triggersDC(26, 0);
        std::vector< double > calsDC(26, 0);
        std::vector< double >  bgHitsDC(26, 0);

        //------------------------------------------------------------------------------------------------------------------------------
        // aggregate data from single pixel values
        //------------------------------------------------------------------------------------------------------------------------------
        int nPixels=0;

        std::vector< int > hitsPerPixel;
        for (int c=1;c<51;c++) {
            for (int r=1;r<79;r++) {
                bgHitsTotal += bgMap->GetBinContent(1 + c, 1 + r);
                nPixels ++;
                hitsPerPixel.push_back(bgMap->GetBinContent(1 + c, 1 + r));
            }
        }
        std::sort(hitsPerPixel.begin(), hitsPerPixel.end());

        double meanHitsPerPixel = hitsPerPixel[(int)(hitsPerPixel.size()*3/4)];
        if (meanHitsPerPixel < 1) {
            meanHitsPerPixel = hitsPerPixel[(int)(hitsPerPixel.size()*9/10)];
        }
        if (meanHitsPerPixel < 1) {
            meanHitsPerPixel = hitsPerPixel[(int)(hitsPerPixel.size()*98/100)];
        }

        int nPixelsNotHot=0;
        int nHitsNotHot=0;
        int nPixelsHot=0;
        double hitsTolerance = sqrt(meanHitsPerPixel) * 5 + 1;


        TH2D* hotPixelMap = new TH2D("hotPixelMap", "hotPixelMap", 52, 0, 52, 80, 0 ,80);

        for (int c=1;c<51;c++) {
            for (int r=1;r<79;r++) {
                int nHits = bgMap->GetBinContent(1 + c, 1 + r);
                if (nHits > meanHitsPerPixel + hitsTolerance) {
                    nPixelsHot++;
                    hotPixelMap->SetBinContent( 1+c, 1+r, 1);
                    totalChargeMap->SetBinContent(1+c, 1+r, 0);
                } else {
                    nPixelsNotHot++;
                    nHitsNotHot += nHits;
                    bgMapNotHot->SetBinContent(1 + c, 1 + r, nHits);
                }
            }
        }

        for (unsigned int i=0;i<chargesList.size();i++) {
            if (hotPixelMap->GetBinContent( 1+ chargesList[i].first.first, 1+chargesList[i].first.second) < 1) {
                totalChargeMap->Fill(chargesList[i].first.first, chargesList[i].first.second, chargesList[i].second);
                pixelChargeDistribution->Fill(chargesList[i].second);
            }
        }

        for (int c=1;c<51;c++) {
            for (int r=1;r<79;r++) {
                if (bgMapNotHot->GetBinContent(1 + c, 1 + r) > 0) {
                    meanChargeMap->SetBinContent(1 + c, 1 + r,totalChargeMap->GetBinContent(1 + c, 1 + r) / bgMapNotHot->GetBinContent(1 + c, 1 + r));
                }
            }
        }

        //------------------------------------------------------------------------------------------------------------------------------
        // rate
        //------------------------------------------------------------------------------------------------------------------------------
        double meanRate = ((nHitsNotHot) / (nPixelsNotHot*25*1e-9*150.0*100.0 * 1e-2 * (double)m_eventcounter));
        double rateError =(sqrt(nHitsNotHot) / (nPixelsNotHot*25*1e-9*150.0*100.0 * 1e-2 * (double)m_eventcounter));
        //------------------------------------------------------------------------------------------------------------------------------
        // cluster rate
        //------------------------------------------------------------------------------------------------------------------------------
        double clusterRate = 0;
        for (unsigned int clusterSize =1;clusterSize<36;clusterSize++) {
            double clusterEfficiency = 1.0;
            clusterRate += clusterSizeDistribution->GetBinContent(1 + clusterSize) / (bgHitPixels*25*1e-9*150.0*100.0 * 1e-2 * (double)m_eventcounter);
        }

        std::stringstream ss;
        std::stringstream ss3;
        std::stringstream ss2;
        ss3 << "Events: " << m_eventcounter << "<br>";
        ss3 << "Hits: " << bgHitsTotal << "<br>";
        ss3 << "Rate: " << meanRate << " +/- " << rateError << " MHz/cm2<br>Cluster rate: " << clusterRate << " MHz/cm2<br>Hot pixels:" << nPixelsHot;
        htmlText += ss3.str();

        //------------------------------------------------------------------------------------------------------------------------------
        // write HTML page HEADER
        //------------------------------------------------------------------------------------------------------------------------------
        ss2.str("");
        ss2.clear();
        ss2 << "<br>{" << m_runnumber << ", " << meanRate << "," << rateError << ", " << bgHitsTotal << ", ";


        //------------------------------------------------------------------------------------------------------------------------------
        // write HTML histograms
        //------------------------------------------------------------------------------------------------------------------------------
        htmlText += "<br><br><strong>Histograms:</strong><br>";
        writeHistogram(bgMap, "bgMap");
        writeHistogram(bgMapNotHot, "bgMapNotHot");
        writeHistogram(totalADCMap, "totalADCMap");
        writeHistogram(adcDistribution, "adcDistribution");
        writeHistogram(totalChargeMap, "totalChargeMap");
        writeHistogram(meanChargeMap, "meanChargeMap");
        pixelChargeDistribution->GetXaxis()->SetRangeUser(0, 300);
        writeHistogram(pixelChargeDistribution, "pixelChargeDistribution", 0.65, 0.65);
        writeHistogram(hotPixelMap, "hotPixelMap");


        writeHistogram(clusterMap, "clusterMap");
        writeHistogram(clusterSizeDistribution, "clusterSizeDistribution", 0.65, 0.65);

        writeHistogram(hitsPerNEventsVsTime, "hitsPerNEventsVsTime", 0.65, 0.15);

        ss << outputFolder << "/run.html";
        std::ofstream out(ss.str());
        out << htmlText;
        htmlText = "";
        out.close();


        //------------------------------------------------------------------------------------------------------------------------------
        // CSV
        //------------------------------------------------------------------------------------------------------------------------------
        ss.str("");
        ss.clear();
        ss << outputFolder << "/results.csv";
        std::ofstream out2(ss.str());

        ss2.str("");
        ss2.clear();
        ss2 <<  meanRate << ", " << rateError;
        out2 << ss2.str();
        out2.close();
    }

    uint64_t FileWriterHitmap::FileBytes() const { return 0; }


}
