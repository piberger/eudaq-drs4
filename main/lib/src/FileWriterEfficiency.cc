#include <vector>
#include <sys/types.h>
#include <sys/stat.h>
#include <numeric>

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

//#include "eudaq/Logger.hh"

using namespace pxar;

namespace eudaq {


    class FileWriterEfficiency : public FileWriter {
    public:
        FileWriterEfficiency(const std::string &);
        virtual void StartRun(unsigned);
        virtual void WriteEvent(const DetectorEvent &);
        virtual uint64_t FileBytes() const;
        virtual ~FileWriterEfficiency();
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

        const char* analysisRevision = "v1.15";
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
        TH2D* calMap;
        TH2D* bgMap;
        TH2D* pkamMap;
        TH2D* pkamHitmap;
        TH1D* calEff;
        TH1D* calEffNoPKAM;
        TH2D* errorsMap;
        TH2D* hitsAboveMap;
        TH2D* hitsAboveBelowMap;
        TH2D* hitsBelowMap;
        TH2D* hitsIsolatedMap;
        TH2D* clusterMap;

        TH2D* calsAboveMap;
        TH2D* calsAboveBelowMap;
        TH2D* calsBelowMap;
        TH2D* calsIsolatedMap;

        TH1D* effHitsAbove;
        TH1D* effHitsBelow;
        TH1D* effHitsAboveBelow;
        TH1D* effHitsIsolated;

        TH1D* clusterSizeDistribution;

        TH1D* calsVsHitsPerEventDistribution;
        TH1D* calHitsVsHitsPerEventDistribution;
        TH1D* efficiencyVsHitsPerEventDistribution;

        TH1D* calsVsHitsInOtherDoubleColumnsDistribution;
        TH1D* calHitsVsHitsInOtherDoubleColumnsDistribution;
        TH1D* efficiencyVsHitsInOtherDoubleColumnsDistribution;

        TH1D* hitsPerNEventsVsTime;

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

    };

    namespace {
        static RegisterFileWriter<FileWriterEfficiency> reg("efficiency");
    }

    FileWriterEfficiency::FileWriterEfficiency(const std::string & /*param*/) : m_ser(0) {


    }

    void FileWriterEfficiency::StartRun(unsigned runnumber) {
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

        calMap = new TH2D("calMap", "calMap", 52, 0, 52, 80, 0 ,80);
        bgMap = new TH2D("bgMap", "bgMap", 52, 0, 52, 80, 0 ,80);
        pkamMap = new TH2D("pkamMap", "pkamMap", 52, 0, 52, 80, 0 ,80);
        errorsMap = new TH2D("errorsMap", "errorsMap", 52, 0, 52, 80, 0 ,80);
        pkamHitmap = new TH2D("pkamhitMap", "pkamHitMap", 52, 0, 52, 80, 0 ,80);

        hitsAboveMap = new TH2D("hitsAboveMap", "hitsAboveMap", 52, 0, 52, 80, 0 ,80);
        hitsBelowMap = new TH2D("hitsBelowMap", "hitsBelowMap", 52, 0, 52, 80, 0 ,80);
        hitsAboveBelowMap = new TH2D("hitsAboveBelowMap", "hitsAboveBelowMap", 52, 0, 52, 80, 0 ,80);
        hitsIsolatedMap = new TH2D("hitsIsolatedMap", "hitsIsolatedMap", 52, 0, 52, 80, 0 ,80);

        calsAboveMap = new TH2D("calsAboveMap", "calsAboveMap", 52, 0, 52, 80, 0 ,80);
        calsBelowMap = new TH2D("calsBelowMap", "calsBelowMap", 52, 0, 52, 80, 0 ,80);
        calsAboveBelowMap = new TH2D("calsAboveBelowMap", "calsAboveBelowMap", 52, 0, 52, 80, 0 ,80);
        calsIsolatedMap = new TH2D("calsIsolatedMap", "calsIsolatedMap", 52, 0, 52, 80, 0 ,80);

        clusterMap = new TH2D("clusterMap", "clusterMap", 104, 0, 52, 160, 0 ,80);
        clusterSizeDistribution = new TH1D("clusterSizeDistribution", "clusterSizeDistribution", 38, -0.5, 37.5);

        calHitsVsHitsPerEventDistribution =  new TH1D("calHitsVsHitsPerEventDistribution", "calHitsVsHitsPerEventDistribution", 50, 0, 50);
        calsVsHitsPerEventDistribution =  new TH1D("calsVsHitsPerEventDistribution", "calsVsHitsPerEventDistribution", 50, 0, 50);
        calHitsVsHitsPerEventDistribution->Sumw2();
        calsVsHitsPerEventDistribution->Sumw2();

        calHitsVsHitsInOtherDoubleColumnsDistribution =  new TH1D("calHitsVsHitsInOtherDoubleColumnsDistribution", "calHitsVsHitsInOtherDoubleColumnsDistribution", 50, 0, 50);
        calsVsHitsInOtherDoubleColumnsDistribution =  new TH1D("calsVsHitsInOtherDoubleColumnsDistribution", "calsVsHitsInOtherDoubleColumnsDistribution", 50, 0, 50);
        calHitsVsHitsInOtherDoubleColumnsDistribution->Sumw2();
        calsVsHitsInOtherDoubleColumnsDistribution->Sumw2();

        hitsPerNEventsVsTime = new TH1D("hitsPerNEventsVsTime", "hitsPerNEventsVsTime", 21000, 0, 210000);

        ntrig = 10;
        m_npkam = 0;
        m_npkammax = 100;
        m_nreadouterrors = 0;
        m_dumpeventsmax = 100;
        m_dumpedevents = 0;
        m_eventcounter = 0;

        std::stringstream ss;
        ss << "<strong>RUN " << runnumber << ":</strong><br><i>analysis rev:" << analysisRevision << "</i><br><br>";
        htmlText += ss.str();
    }

    void FileWriterEfficiency::WriteEvent(const DetectorEvent & ev) {

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

                std::cout << "pkam event found" << std::endl;
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
            } else if (!evt->hasCalTrigger() && calCol > -1 && calRow > -1 && calCol < 250 && calRow < 250) {
                errorsMap->Fill(calCol, calRow);
                std::cout << "read-out errors occured! skip event!!!" << (int)calCol << " " << (int)calRow << std::endl;
                m_nreadouterrors++;
            //------------------------------------------------------------------------------------------------------------------------------
            // GOOD EVENT
            //------------------------------------------------------------------------------------------------------------------------------
            } else {

                int hitsBelow = 0;
                int hitsAbove = 0;
                bool calFound = false;
                int hitsInOtherDoubleColumns = 0;

                //------------------------------------------------------------------------------------------------------------------------------
                // loop over pixels
                //------------------------------------------------------------------------------------------------------------------------------
                for (int j = 0; j < evt->pixels.size(); j++) {
                    if (evt->pixels[j].column() == calCol && evt->pixels[j].row() == calRow) {
                        calMap->Fill(calCol, calRow);
                        calFound = true;
                    } else {
                        bgMap->Fill(evt->pixels[j].column(), evt->pixels[j].row());
                    }

                    //------------------------------------------------------------------------------------------------------------------------------
                    // check for hits in same double column
                    //------------------------------------------------------------------------------------------------------------------------------
                    if ((int)(evt->pixels[j].column() / 2) == (int)(calCol / 2) && evt->pixels[j].row() > calRow) {
                        hitsAbove++;
                    }
                    if ((int)(evt->pixels[j].column() / 2) == (int)(calCol / 2) && evt->pixels[j].row() < calRow) {
                        hitsBelow++;
                    }

                    if (((int)(evt->pixels[j].column() / 2) != (int)(calCol / 2))) {
                        hitsInOtherDoubleColumns++;
                    }
                }

                //------------------------------------------------------------------------------------------------------------------------------
                // efficiency vs. event length
                //------------------------------------------------------------------------------------------------------------------------------
                int backgroundHits = evt->pixels.size() + 1;
                if (calRow > 0 && calCol > 0 && calRow < 79 && calCol < 51) {
                    if (calFound) {
                        backgroundHits--;
                        calHitsVsHitsPerEventDistribution->Fill(backgroundHits);
                    }
                    calsVsHitsPerEventDistribution->Fill(backgroundHits);
                }

                //------------------------------------------------------------------------------------------------------------------------------
                // hits/event vs. time
                //------------------------------------------------------------------------------------------------------------------------------
                hitsPerNEventsVsTime->Fill(m_eventcounter, backgroundHits);

                //------------------------------------------------------------------------------------------------------------------------------
                // efficiency vs. hits in other double columns
                //------------------------------------------------------------------------------------------------------------------------------
                if (calRow > 0 && calCol > 0 && calRow < 79 && calCol < 51) {
                    if (calFound) {
                        calHitsVsHitsInOtherDoubleColumnsDistribution->Fill(hitsInOtherDoubleColumns);
                    }
                    calsVsHitsInOtherDoubleColumnsDistribution->Fill(hitsInOtherDoubleColumns);
                }


                //------------------------------------------------------------------------------------------------------------------------------
                // other hits in double column analysis
                //------------------------------------------------------------------------------------------------------------------------------
                if (calFound) {
                    if (hitsAbove < 1 && hitsBelow < 1) {
                        hitsIsolatedMap->Fill(calCol, calRow);
                    } else if (hitsAbove < 1 && hitsBelow > 0) {
                        hitsBelowMap->Fill(calCol, calRow);
                    } else if (hitsAbove > 0 && hitsBelow < 1) {
                        hitsAboveMap->Fill(calCol, calRow);
                    } else {
                        hitsAboveBelowMap->Fill(calCol, calRow);
                    }
                }
                if (hitsAbove < 1 && hitsBelow < 1) {
                    calsIsolatedMap->Fill(calCol, calRow);
                } else if (hitsAbove < 1 && hitsBelow > 0) {
                    calsBelowMap->Fill(calCol, calRow);
                } else if (hitsAbove > 0 && hitsBelow < 1) {
                    calsAboveMap->Fill(calCol, calRow);
                } else {
                    calsAboveBelowMap->Fill(calCol, calRow);
                }

                //------------------------------------------------------------------------------------------------------------------------------
                // clustering
                //------------------------------------------------------------------------------------------------------------------------------
                int nHitsTotal = evt->pixels.size();
                std::vector < std::pair<int, int> > pixels;
                std::vector < std::pair<int, int> > pixelsNew;
                std::vector < std::pair<int, int> > cluster;

                for (int hitpix = 0; hitpix < nHitsTotal;hitpix++) {
                    if (evt->pixels[hitpix].column() != calCol || evt->pixels[hitpix].row() != calRow) {
                        pixels.push_back(std::make_pair(evt->pixels[hitpix].column(), evt->pixels[hitpix].row()));
                    }
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
    void FileWriterEfficiency::writeHistogram(TH2D* h1, std::string name) {
        writeHistogram(h1, name, -1.0, -1.0);
    }

    void FileWriterEfficiency::writeHistogram(TH2D* h1, std::string name, float statsPosX, float statsPosY) {
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

    void FileWriterEfficiency::writeHistogram(TH1D* h1, std::string name) {
        writeHistogram(h1, name, -1.0, -1.0);
    }

    void FileWriterEfficiency::writeHistogram(TH1D* h1, std::string name, float statsPosX, float statsPosY, char* drawOption) {
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

    FileWriterEfficiency::~FileWriterEfficiency() {
        //delete m_ser;

        calEff = new TH1D("calEffDist", "cal eff distribution", 2020, 0 ,101.0);
        calEffNoPKAM = new TH1D("calEffDistNoPKAM", "cal eff distribution (no PKAM)", 2020, 0 ,101.0);

        effHitsAbove = new TH1D("calEffDistAbove", "cal eff distribution (hits above)", 2020, 0 ,101.0);
        effHitsAboveBelow = new TH1D("calEffDistAboveBelow", "cal eff distribution (hits below and above)", 2020, 0 ,101.0);
        effHitsBelow = new TH1D("calEffDistBelow", "cal eff distribution (hits below)", 2020, 0 ,101.0);
        effHitsIsolated = new TH1D("calEffDistIsolated", "cal eff distribution (isolated)", 2020, 0 ,101.0);

        efficiencyVsHitsPerEventDistribution = new TH1D("efficiencyVsHitsPerEventDistribution", "efficiencyVsHitsPerEventDistribution", 50, 0, 50);
        efficiencyVsHitsInOtherDoubleColumnsDistribution = new TH1D("efficiencyVsHitsInOtherDoubleColumnsDistribution", "efficiencyVsHitsInOtherDoubleColumnsDistribution", 50, 0, 50);

        int calsTotal = 0;
        int PKAMsTotal = 0;
        int bgHitsTotal=0;
        int bgHitPixels = 0;

        std::vector<double> calEffNoPKAMList;

        std::vector< double > triggersDC(26, 0);
        std::vector< double > calsDC(26, 0);
        std::vector< double >  bgHitsDC(26, 0);

        //------------------------------------------------------------------------------------------------------------------------------
        // aggregate efficnecy data from single pixel values
        //------------------------------------------------------------------------------------------------------------------------------
        for (int c=1;c<51;c++) {
            for (int r=1;r<79;r++) {
                int cals = calMap->GetBinContent(1 + c, 1 + r);
                int pkams = pkamMap->GetBinContent(1 + c, 1 + r);
                int errors = errorsMap->GetBinContent(1 + c, 1 + r);
                int bgHits = bgMap->GetBinContent(1 + c, 1 + r);

                if (cals > 0 || bgHits > 0) {
                    bgHitPixels++;
                    bgHitsTotal += bgHits;
                }

                double efficiency = (double)cals  / (double) ntrig * 100.0;
                if (efficiency > 100.0) efficiency = 100.0;
                double efficiencyNoPkam = 0;
                if (ntrig - pkams - errors > 0) {
                    efficiencyNoPkam = (double)cals / (double) (ntrig - pkams - errors) * 100.0;
                    if (efficiencyNoPkam > 100.0) efficiencyNoPkam = 100.0;
                    calEffNoPKAM->Fill(efficiencyNoPkam);
                    calEffNoPKAMList.push_back(efficiencyNoPkam);

                    triggersDC[c/2] += ntrig - pkams;
                    calsDC[c/2] += cals;
                    bgHitsDC[c/2] += bgHits;
                }
                calEff->Fill(efficiency);

                calsTotal += cals;
                PKAMsTotal += pkams;

                int calsBelow = calsBelowMap->GetBinContent(1 + c, 1 + r);
                int calsAbove = calsAboveMap->GetBinContent(1 + c, 1 + r);
                int calsAboveBelow = calsAboveBelowMap->GetBinContent(1 + c, 1 + r);
                int calsIsolated = calsIsolatedMap->GetBinContent(1 + c, 1 + r);
                int hitsBelow = hitsBelowMap->GetBinContent(1 + c, 1 + r);
                int hitsAbove = hitsAboveMap->GetBinContent(1 + c, 1 + r);
                int hitsAboveBelow = hitsAboveBelowMap->GetBinContent(1 + c, 1 + r);
                int hitsIsolated = hitsIsolatedMap->GetBinContent(1 + c, 1 + r);

                if (calsBelow > 0) {
                    double efficiencyBelow = (double) hitsBelow / (double) calsBelow * 100.0;
                    if (efficiencyBelow > 100.0) efficiencyBelow = 100.0;
                    effHitsBelow->Fill(efficiencyBelow);
                }
                if (calsAboveBelow > 0) {
                    double efficiencyAboveBelow = (double) hitsAboveBelow / (double) calsAboveBelow * 100.0;
                    if (efficiencyAboveBelow > 100.0) efficiencyAboveBelow = 100.0;
                    effHitsAboveBelow->Fill(efficiencyAboveBelow);
                }
                if (calsAbove > 0) {
                    double efficiencyAbove = (double) hitsAbove / (double) calsAbove * 100.0;
                    if (efficiencyAbove > 100.0) efficiencyAbove = 100.0;
                    effHitsAbove->Fill(efficiencyAbove);
                }
                if (calsIsolated > 0) {
                    double efficiencyIsolated = (double) hitsIsolated / (double) calsIsolated * 100.0;
                    if (efficiencyIsolated > 100.0) efficiencyIsolated = 100.0;
                    effHitsIsolated->Fill(efficiencyIsolated);
                }

            }
        }

        double efficiencyNoPKAMError=0;
        double meanEfficiencyNoPKAM = calEffNoPKAM->GetMean();

        int nPixelsTestedTotal = 4160;
        int nGoodEvents = ntrig*nPixelsTestedTotal-PKAMsTotal;

        //------------------------------------------------------------------------------------------------------------------------------
        // rate
        //------------------------------------------------------------------------------------------------------------------------------
        double meanRate = ((bgHitsTotal) / (bgHitPixels*25*1e-9*150.0*100.0 * 1e-2 * (double)nGoodEvents))/(meanEfficiencyNoPKAM*0.01);
        double rateError = (sqrt(bgHitsTotal) / (bgHitPixels*25*1e-9*150.0*100.0 * 1e-2 * (double)nGoodEvents))/(meanEfficiencyNoPKAM*0.01);

        //------------------------------------------------------------------------------------------------------------------------------
        // cluster rate
        //------------------------------------------------------------------------------------------------------------------------------
        double clusterRate = 0;
        for (unsigned int clusterSize =1;clusterSize<36;clusterSize++) {
            double singlePixelInefficiency = (1.0 - meanEfficiencyNoPKAM*0.01);
            double clusterInefficiency = pow(singlePixelInefficiency, (double)clusterSize);
            double clusterEfficiency = 1.0 - clusterInefficiency;
            if (clusterEfficiency > 1.0) {
                clusterEfficiency = 1.0;
            }
            if (clusterEfficiency > 0) {
                clusterRate += clusterSizeDistribution->GetBinContent(1 + clusterSize) / (bgHitPixels*25*1e-9*150.0*100.0 * 1e-2 * (double)nGoodEvents) / clusterEfficiency;
            }
        }

        std::stringstream ss3;
        ss3 << "Rate: " <<meanRate << " +/- " << rateError << " MHz/cm2<br>Cluster rate: " << clusterRate << " MHz/cm2<br>";
        htmlText += ss3.str();

        //------------------------------------------------------------------------------------------------------------------------------
        // efficiency with pkam removal
        //------------------------------------------------------------------------------------------------------------------------------
        if (calsTotal-PKAMsTotal > 0) {
            efficiencyNoPKAMError = 100.0 * sqrt( meanEfficiencyNoPKAM * 0.01 * (1-meanEfficiencyNoPKAM* 0.01) / (calsTotal-PKAMsTotal));
        }
        std::stringstream ss2;
        ss2 << "Efficiency (PKAM events & events with R/O errors removed): " <<meanEfficiencyNoPKAM << " +/- " << efficiencyNoPKAMError << " %<br>";
        htmlText += ss2.str();
        ss2.str("");
        ss2.clear();
        ss2 << "#=" << std::accumulate(calEffNoPKAMList.begin(), calEffNoPKAMList.end(), 0.0)/calEffNoPKAMList.size() << "<br>";
        htmlText += ss2.str();

        //------------------------------------------------------------------------------------------------------------------------------
        // efficiency
        //------------------------------------------------------------------------------------------------------------------------------
        double meanEfficiency = calEff->GetMean();
        double efficiencyError = 100.0 * sqrt( meanEfficiency * 0.01 * (1-meanEfficiency* 0.01) / (calsTotal));
        ss2.str("");
        ss2.clear();
        ss2 << "Efficiency: " <<meanEfficiency << " +/- " << efficiencyError << " %<br>";
        htmlText += ss2.str();


        //------------------------------------------------------------------------------------------------------------------------------
        // efficiency vs. event length
        //------------------------------------------------------------------------------------------------------------------------------
        efficiencyVsHitsPerEventDistribution->Divide(calHitsVsHitsPerEventDistribution, calsVsHitsPerEventDistribution, 1, 1, "b");

        //------------------------------------------------------------------------------------------------------------------------------
        // efficiency vs. hits in other double column
        //------------------------------------------------------------------------------------------------------------------------------
        efficiencyVsHitsInOtherDoubleColumnsDistribution->Divide(calHitsVsHitsInOtherDoubleColumnsDistribution, calsVsHitsInOtherDoubleColumnsDistribution, 1, 1, "b");


        //------------------------------------------------------------------------------------------------------------------------------
        // PKAM rate
        //------------------------------------------------------------------------------------------------------------------------------
        double meanRatePKAM = (PKAMsTotal / (4160*25*1e-9*150.0*100.0 * 1e-2 * (double)ntrig * bgHitPixels))/(meanEfficiencyNoPKAM*0.01);
        double ratePKMError = (sqrt(PKAMsTotal) / (4160*25*1e-9*150.0*100.0 * 1e-2 * (double)ntrig * bgHitPixels))/(meanEfficiencyNoPKAM*0.01);
        ss2.str("");
        ss2.clear();
        ss2 << "PKAM rate: " <<meanRatePKAM << " +/- " << ratePKMError << " MHz/cm2<br>";
        htmlText += ss2.str();


        //------------------------------------------------------------------------------------------------------------------------------
        // PKAM ratio
        //------------------------------------------------------------------------------------------------------------------------------
        double PKAMratio = PKAMsTotal/(4160.0*ntrig);
        ss2.str("");
        ss2.clear();
        ss2 << "PKAM ratio: " << 100.0*PKAMratio << " %<br>";
        htmlText += ss2.str();


        //------------------------------------------------------------------------------------------------------------------------------
        // write HTML page HEADER
        //------------------------------------------------------------------------------------------------------------------------------
        ss2.str("");
        ss2.clear();
        ss2 << "<br>{" << m_runnumber << ", " << meanRate << "," << rateError << ", " << meanEfficiencyNoPKAM << ", " << efficiencyNoPKAMError << ", " << meanEfficiency << ", " << efficiencyError << ", " << calsTotal << ", " << bgHitsTotal << ", " << bgHitPixels << ", " << PKAMsTotal << ", " << effHitsAbove->GetMean() << ", " << effHitsBelow->GetMean() << ", " << effHitsAboveBelow->GetMean() << ", " << effHitsIsolated->GetMean() << "}<br>";
        htmlText += ss2.str();

        ss2.str("");
        ss2.clear();
        ss2 << "<br>[" <<  meanRate << "," << rateError << ", " << meanEfficiencyNoPKAM << ", " << efficiencyNoPKAMError << ", " << " -1, " << effHitsAbove->GetMean() << ", " << effHitsBelow->GetMean() << ", " << effHitsAboveBelow->GetMean() << ", " << effHitsIsolated->GetMean() << "]";


        //------------------------------------------------------------------------------------------------------------------------------
        // write efficiency per double column
        //------------------------------------------------------------------------------------------------------------------------------
        htmlText += ss2.str();
        htmlText += "<br><br>Per double column:<br>";

        int activePixelsDC = 152;
        for (int dc=1;dc<25;dc++) {
            ss2.str("");
            ss2.clear();
            double dcEfficiency = 100.0 * calsDC[dc] / (double)triggersDC[dc];
            double dcEfficiencyError = 100.0 * sqrt( dcEfficiency * 0.01 * (1-dcEfficiency* 0.01) / (double)triggersDC[dc]);
            double dcRate = (bgHitsDC[dc] / (activePixelsDC *25*1e-9*150.0*100.0 * 1e-2 * (double)(ntrig*4160-PKAMsTotal)))/(dcEfficiency*0.01);
            double dcrateError = (sqrt(bgHitsDC[dc]) / (activePixelsDC*25*1e-9*150.0*100.0 * 1e-2 * (double)(ntrig*4160-PKAMsTotal)))/(dcEfficiency*0.01);
            ss2 << "<br>[" <<  dcRate << "," << dcrateError << ", " << dcEfficiency << ", " << dcEfficiencyError << ", " << " -1" << "],";
            htmlText += ss2.str();
        }


        //------------------------------------------------------------------------------------------------------------------------------
        // write HTML histograms
        //------------------------------------------------------------------------------------------------------------------------------
        htmlText += "<br><br><strong>Histograms:</strong><br>";
        writeHistogram(calMap, "calibrateMap");
        writeHistogram(bgMap, "bgMap");
        writeHistogram(pkamMap, "pkamMap");
        writeHistogram(pkamHitmap, "pkamHitMap");
        writeHistogram(calEff, "calEffDistr");
        writeHistogram(calEffNoPKAM, "calEffDistrNoPkam");
        writeHistogram(errorsMap, "errorsMap");
        
        writeHistogram(hitsAboveBelowMap, "hitsAboveBelowMap");
        writeHistogram(hitsAboveMap, "hitsAboveMap");
        writeHistogram(hitsBelowMap, "hitsBelowMap");
        writeHistogram(hitsIsolatedMap, "hitsIsolatedMap");
        writeHistogram(calsAboveBelowMap, "calsAboveBelowMap");
        writeHistogram(calsAboveMap, "calsAboveMap");
        writeHistogram(calsBelowMap, "calsBelowMap");
        writeHistogram(calsIsolatedMap, "calsIsolatedMap");

        writeHistogram(effHitsAbove, "effHitsAbove");
        writeHistogram(effHitsBelow, "effHitsBelow");
        writeHistogram(effHitsAboveBelow, "effHitsAboveBelow");
        writeHistogram(effHitsIsolated, "effHitsIsolated");

        writeHistogram(clusterMap, "clusterMap");
        writeHistogram(clusterSizeDistribution, "clusterSizeDistribution", 0.65, 0.65);


        efficiencyVsHitsPerEventDistribution->SetStats(0);
        efficiencyVsHitsPerEventDistribution->GetXaxis()->SetTitle("total hits in event");
        efficiencyVsHitsPerEventDistribution->GetYaxis()->SetTitle("efficiency");

        //double minValue = efficiencyVsHitsPerEventDistribution->GetBinContent(efficiencyVsHitsPerEventDistribution->GetMinimumBin());
        //double maxValue = efficiencyVsHitsPerEventDistribution->GetBinContent(efficiencyVsHitsPerEventDistribution->GetMaximumBin());
        //efficiencyVsHitsPerEventDistribution->GetYaxis()->SetRangeUser(minValue * 0.9-0.1, maxValue*1.01 + 0.01);

        writeHistogram(calsVsHitsPerEventDistribution, "calsVsHitsPerEventDistribution", 0.65, 0.65);
        writeHistogram(calHitsVsHitsPerEventDistribution, "calHitsVsHitsPerEventDistribution", 0.65, 0.65);
        writeHistogram(efficiencyVsHitsPerEventDistribution, "efficiencyVsHitsPerEventDistribution", -10, -10, (char*)"E");

        //minValue = efficiencyVsHitsInOtherDoubleColumnsDistribution->GetBinContent(efficiencyVsHitsInOtherDoubleColumnsDistribution->GetMinimumBin());
        //maxValue = efficiencyVsHitsInOtherDoubleColumnsDistribution->GetBinContent(efficiencyVsHitsInOtherDoubleColumnsDistribution->GetMaximumBin());
        //efficiencyVsHitsInOtherDoubleColumnsDistribution->GetYaxis()->SetRangeUser(minValue * 0.9-0.1, maxValue*1.01 + 0.01);

        efficiencyVsHitsInOtherDoubleColumnsDistribution->SetStats(0);
        efficiencyVsHitsInOtherDoubleColumnsDistribution->GetXaxis()->SetTitle("hits in other double columns");
        efficiencyVsHitsInOtherDoubleColumnsDistribution->GetYaxis()->SetTitle("efficiency");

        writeHistogram(calsVsHitsInOtherDoubleColumnsDistribution, "calsVsHitsInOtherDoubleColumns", 0.65, 0.65);
        writeHistogram(calHitsVsHitsInOtherDoubleColumnsDistribution, "calHitsVsHitsInOtherDoubleColumns", 0.65, 0.65);
        writeHistogram(efficiencyVsHitsInOtherDoubleColumnsDistribution, "efficiencyVsHitsInOtherDoubleColumns", -10, -10, (char*)"E");


        writeHistogram(hitsPerNEventsVsTime, "hitsPerNEventsVsTime", 0.65, 0.15);

        std::stringstream ss;
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

        double efficiencyErrorAbove = 100.0 * sqrt( effHitsAbove->GetMean() * 0.01 * (1-effHitsAbove->GetMean()* 0.01) / (calsAboveMap->GetEntries()+1e-8));
        double efficiencyErrorBelow = 100.0 * sqrt( effHitsBelow->GetMean() * 0.01 * (1-effHitsBelow->GetMean()* 0.01) / (calsBelowMap->GetEntries()+1e-8));
        double efficiencyErrorAboveBelow = 100.0 * sqrt( effHitsAboveBelow->GetMean() * 0.01 * (1-effHitsAboveBelow->GetMean()* 0.01) / (calsAboveBelowMap->GetEntries()+1e-8));
        double efficiencyErrorIsolated = 100.0 * sqrt( effHitsIsolated->GetMean() * 0.01 * (1-effHitsIsolated->GetMean()* 0.01) / (calsIsolatedMap->GetEntries()+1e-8));

        ss2 << meanRate << "," << rateError << ", " << meanEfficiencyNoPKAM << ", " << efficiencyNoPKAMError << ", " << " -1, " << effHitsAbove->GetMean() << ", " << efficiencyErrorAbove << ", " << effHitsBelow->GetMean() << ", " << efficiencyErrorBelow << ", " << effHitsAboveBelow->GetMean() << ", " << efficiencyErrorAboveBelow << ", " << effHitsIsolated->GetMean() << ", " << efficiencyErrorIsolated << ", -1, " << clusterSizeDistribution->GetMean() << ", " << clusterRate;

        out2 << ss2.str();
        out2.close();

        //------------------------------------------------------------------------------------------------------------------------------
        // CSV efficiency per double column
        //------------------------------------------------------------------------------------------------------------------------------
        ss.str("");
        ss.clear();
        ss << outputFolder << "/results_dc.csv";
        std::ofstream out3(ss.str());

        ss2.str("");
        ss2.clear();
        activePixelsDC = 152;
        double meanEfficiency14_25 = 0;
        double meanEfficiencyError14_25 = 0;
        double meanRate14_25 = 0;

        for (int dc=1;dc<25;dc++) {
            ss2.str("");
            ss2.clear();
            double dcEfficiency = 100.0 * calsDC[dc] / (double)triggersDC[dc];
            double dcEfficiencyError = 100.0 * sqrt( dcEfficiency * 0.01 * (1-dcEfficiency* 0.01) / (double)triggersDC[dc]);
            double dcRate = (bgHitsDC[dc] / (activePixelsDC *25*1e-9*150.0*100.0 * 1e-2 * (double)(ntrig*4160-PKAMsTotal)))/(dcEfficiency*0.01);
            double dcrateError = (sqrt(bgHitsDC[dc]) / (activePixelsDC*25*1e-9*150.0*100.0 * 1e-2 * (double)(ntrig*4160-PKAMsTotal)))/(dcEfficiency*0.01);
            ss2 << dcRate << "," << dcrateError << ", " << dcEfficiency << ", " << dcEfficiencyError << ", " << " -1, " << effHitsAbove->GetMean() << ", " << effHitsBelow->GetMean() << ", " << effHitsAboveBelow->GetMean() << ", " << effHitsIsolated->GetMean() << "\n";
            out3 << ss2.str();
            if (dc > 13 && dc < 24) {
                meanEfficiency14_25 += dcEfficiency;
                meanRate14_25 += dcRate;
                meanEfficiencyError14_25 += dcEfficiencyError;
            }
        }
        out3.close();

        meanEfficiency14_25 /= 10.0;
        meanEfficiencyError14_25 /= 10.0;
        meanRate14_25 /= 10.0;
        ss.str("");
        ss.clear();
        ss << outputFolder << "/results_dc14_24.csv";
        std::ofstream out4(ss.str());
        ss2.str("");
        ss2.clear();
        ss2 << meanRate14_25 << ", 0, " << meanEfficiency14_25 << ", " << meanEfficiencyError14_25 << ", " << " -1, 0, 0, 0, 0\n";
        out4 << ss2.str();
        out4.close();
    }

    uint64_t FileWriterEfficiency::FileBytes() const { return 0; }


}
