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
#include "TF1.h"
#include "TBox.h"
#include "THStack.h"
#include "TStyle.h"
#include "TPad.h"
#include "TLegend.h"

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
        void writeHistogram(TH1D* h1, std::string name, bool log = false);
        void writeHistogram(TH2D* h2, std::string name);
        void writeHistogram(TH1D* h1, std::string name, float statsX, float statsY, char* drawOption = "", bool log = false);
        void writeHistogram(TH2D* h2, std::string name, float statsX, float statsY);
        void writeHistogramTest(TH1D* h2, std::string name, float statsX, float statsY, float meanClusterSizeInColumn);

        const char* analysisRevision = "v1.24 - September 20 2018";
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
        TH1D* calEffNoDead;
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
        TH1D* clusterSizeDistributionCol;
        TH1D* clusterSizeDistributionRow;

        TH1D* hitsPerEventDistribution;
        TH1D* calHitsPerEventDistribution;
        TH1D* efficiencyVsHitsPerEventDistribution;

        TH1D* calsVsHitsInOtherDoubleColumnsDistribution;
        TH1D* calHitsVsHitsInOtherDoubleColumnsDistribution;
        TH1D* efficiencyVsHitsInOtherDoubleColumnsDistribution;

        TH1D* calsVsHitsInSameDoubleColumnsDistribution;
        TH1D* calHitsVsHitsInSameDoubleColumnsDistribution;
        TH1D* efficiencyVsHitsInSameDoubleColumnsDistribution;

        TH1D* hitsPerNEventsVsTime;
        TH2D* hitsPerNEventsVsTimeDC;

        TH1D* adcDistribution;

        TH1D* calClusterFound;
        TH1D* calClusterNotFound;

        TH1D* nFramesPerDoubleColumn;

        std::map < std::vector< std::pair< int,int >  >, int > clusterShapes;

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
        int m_brokenclustercandidate;

        int m_clusterRadius;
        int m_min_adc;

        int m_eventsmax;

        bool m_includeEdges;

        int m_fiducial_area_col_min;
        int m_fiducial_area_col_max;
        int m_fiducial_area_row_min;
        int m_fiducial_area_row_max;

        mutable pxar::statistics decoding_stats;
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

        ntrig = 10;
        m_npkam = 0;
        m_npkammax = 100;
        m_nreadouterrors = 0;
        m_dumpeventsmax = 100;
        m_dumpedevents = 0;
        m_eventcounter = 0;
        m_brokenclustercandidate = 0;
        m_eventsmax = 4160 * ntrig;
        m_includeEdges = false;

        if (m_includeEdges) {
            m_fiducial_area_col_min = 0;
            m_fiducial_area_col_max = 51;
            m_fiducial_area_row_min = 0;
            m_fiducial_area_row_max = 79;
        } else {
            m_fiducial_area_col_min = 1;
            m_fiducial_area_col_max = 50;
            m_fiducial_area_row_min = 1;
            m_fiducial_area_row_max = 78;
        }

        std::cout << m_filepattern << std::endl;
        std::string foutput(FileNamer(m_filepattern).Set('X', "/").Set('R', runnumber));
        std::string foutputPrev(FileNamer(m_filepattern).Set('X', "/").Set('R', runnumber-1));
        std::string foutputNext(FileNamer(m_filepattern).Set('X', "/").Set('R', runnumber+1));
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
        clusterSizeDistributionCol = new TH1D("clusterSizeDistributionCol", "clusterSizeDistributionCol", 38, -0.5, 37.5);
        clusterSizeDistributionRow = new TH1D("clusterSizeDistributionRow", "clusterSizeDistributionRow", 38, -0.5, 37.5);

        calHitsPerEventDistribution =  new TH1D("calHitsPerEventDistribution", "calHitsPerEventDistribution", 50, 0, 50);
        hitsPerEventDistribution =  new TH1D("hitsPerEventDistribution", "hitsPerEventDistribution", 50, 0, 50);
        calHitsPerEventDistribution->Sumw2();
        hitsPerEventDistribution->Sumw2();

        calHitsVsHitsInOtherDoubleColumnsDistribution =  new TH1D("calHitsVsHitsInOtherDoubleColumnsDistribution", "calHitsVsHitsInOtherDoubleColumnsDistribution", 50, 0, 50);
        calsVsHitsInOtherDoubleColumnsDistribution =  new TH1D("calsVsHitsInOtherDoubleColumnsDistribution", "calsVsHitsInOtherDoubleColumnsDistribution", 50, 0, 50);
        calHitsVsHitsInOtherDoubleColumnsDistribution->Sumw2();
        calsVsHitsInOtherDoubleColumnsDistribution->Sumw2();

        calHitsVsHitsInSameDoubleColumnsDistribution =  new TH1D("calHitsVsHitsInSameDoubleColumnsDistribution", "calHitsVsHitsInSameDoubleColumnsDistribution", 50, 0, 50);
        calsVsHitsInSameDoubleColumnsDistribution =  new TH1D("calsVsHitsInSameDoubleColumnsDistribution", "calsVsHitsInSameDoubleColumnsDistribution", 50, 0, 50);
        calHitsVsHitsInSameDoubleColumnsDistribution->Sumw2();
        calsVsHitsInSameDoubleColumnsDistribution->Sumw2();


        int nBinsTime = floor(m_eventsmax/100)+1;
        hitsPerNEventsVsTime = new TH1D("hitsPerNEventsVsTime", "hitsPerNEventsVsTime", nBinsTime, 0, nBinsTime*100);
        hitsPerNEventsVsTimeDC = new TH2D("hitsPerNEventsVsTimeDC", "hitsPerNEventsVsTimeDC", nBinsTime, 0, nBinsTime*100, 26, 0, 26);

        adcDistribution = new TH1D("adcDistribution", "adcDistribution", 256, 0, 256.0);

        calClusterFound = new TH1D("calClusterFound", "cluster size near cal inject", 36, 0, 36.0);
        calClusterNotFound = new TH1D("calClusterNotFound", "calClusterNotFound", 36, 0, 36.0);

        nFramesPerDoubleColumn = new TH1D("nFramesPerDoubleColumn", "2x2 frames per double column", 40, 0, 40.0);

        m_clusterRadius = 4; //default: 2, use 3 or 4 to include broken clusters

        m_min_adc = -1;

        // cluster shapes
        //  list of pixel (col, row), relative to bottom left pixel of cluster  <---> #number of these clusters
        clusterShapes.clear();

        std::stringstream ss;

        ss << "<!DOCTYPE html><html><head><style>"
            "table {\n "
            "    border-collapse: collapse;\n"
            "}\n"

            "th, td {\n"
            "    text-align: left;\n"
            "    padding: 8px;\n"
            "}\n"

            "tr:nth-child(even){background-color: #f2f2f2}\n"

            "th {\n"
            "    background-color: #4CAF50;\n"
            "    color: white;\n"
            "}\n"
            "</style>\n";

        ss << "<strong>RUN " << runnumber << ":</strong><br>";
        ss << "<a href='../" << foutputPrev << "run.html'>&lt;previous<a> ";
        ss << "<a href='../" << foutputNext << "run.html'>next&gt;<a> ";

        ss << "<br><br><i>analysis rev:" << analysisRevision << "</i><br>";
        if (m_clusterRadius > 2) {
            ss << "parameters: m_clusterRadius = " << m_clusterRadius << " (broken clusters will be merged)<br><br>";
        } else {
            ss << "parameters: m_clusterRadius = " << m_clusterRadius << "<br><br>";
        }

        if (m_min_adc > -1) {
            ss << "parameters: min ADC = " << m_min_adc << " (exclude pixels with ADC below)<br><br>";
        }
        ss << "parameters: m_includeEdges = " << m_includeEdges << "<br><br>";
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
                std::cout << "event source:" << (int)m_tbmtype << " / " << (int)m_roctype << std::endl;
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
                    ss2 << outputFolder << "/" << ss.str() << ".png";
                    c1->SaveAs(ss2.str().c_str());
                    std::cout << "dump event " << m_dumpedevents << ": " << ss2.str() << std::endl;
                    m_dumpedevents++;
                }
            }

            // now CHECK EVENT STATUS

            //------------------------------------------------------------------------------------------------------------------------------
            // PKAM RESET ERROR occurred
            //------------------------------------------------------------------------------------------------------------------------------
            if (evt->hasPkamReset()) {

                std::cout << "pkam event found" << std::endl;
                //------------------------------------------------------------------------------------------------------------------------------
                // fill PKAM map
                // this map shows which pixels were under test when PKAM occurred
                //------------------------------------------------------------------------------------------------------------------------------
                pkamMap->Fill(calCol, calRow);
                for (int j = 0; j < evt->pixels.size(); j++) {
                    pkamHitmap->Fill(evt->pixels[j].column(), evt->pixels[j].row());
                    // fill background hits also for bad events
                    if (!(evt->pixels[j].column() == calCol && evt->pixels[j].row() == calRow)) {
                        bgMap->Fill(evt->pixels[j].column(), evt->pixels[j].row());
                    }
                }

                //------------------------------------------------------------------------------------------------------------------------------
                // dump first few PKAM events
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
                    ss2 << outputFolder << "/" << ss.str() << ".png";
                    c1->SaveAs(ss2.str().c_str());
                    std::cout << "dump pkam event:" << ss2.str() << std::endl;
                }
                m_npkam++;

                hitsPerNEventsVsTime->Fill(m_eventcounter, evt->pixels.size());

            //------------------------------------------------------------------------------------------------------------------------------
            // READ-OUT ERROR occurred
            //------------------------------------------------------------------------------------------------------------------------------
            //} else if (!evt->hasCalTrigger() && calCol > 0 && calRow > 0 && calCol < 250 && calRow < 250) {
            } else if (0 && calCol > 0 && calRow > 0 && calCol < 250 && calRow < 250) {
                errorsMap->Fill(calCol, calRow);
                std::cout << "read-out errors occured! skip event!!!" << (int)calCol << " " << (int)calRow << std::endl;
                m_nreadouterrors++;
            //------------------------------------------------------------------------------------------------------------------------------
            // GOOD EVENT
            //------------------------------------------------------------------------------------------------------------------------------
            } else {
                //std::cout << " good event!! " <<  (int)calCol << " " << (int)calRow << " pixels=" << evt->pixels.size()  << std::endl;
                int hitsBelow = 0;
                int hitsAbove = 0;
                bool calFound = false;
                int hitsInOtherDoubleColumns = 0;
                int hitsInSameDoubleColumn = 0;

                //------------------------------------------------------------------------------------------------------------------------------
                // loop over pixels
                //------------------------------------------------------------------------------------------------------------------------------
                for (int j = 0; j < evt->pixels.size(); j++) {

                    //------------------------------------------------------------------------------------------------------------------------------
                    // injected calibrate hit found!
                    //------------------------------------------------------------------------------------------------------------------------------
                    if (evt->pixels[j].column() == calCol && evt->pixels[j].row() == calRow) {
                        calMap->Fill(calCol, calRow);
                        calFound = true;

                    //------------------------------------------------------------------------------------------------------------------------------
                    // background hit found! (beam/X-ray/noise etc...)
                    //------------------------------------------------------------------------------------------------------------------------------
                    } else {
                        if (evt->pixels[j].value() > m_min_adc) {
                            bgMap->Fill(evt->pixels[j].column(), evt->pixels[j].row());
                            adcDistribution->Fill(evt->pixels[j].value());
                            hitsPerNEventsVsTimeDC->Fill(m_eventcounter, evt->pixels[j].column() / 2);
                        }
                    }

                    //------------------------------------------------------------------------------------------------------------------------------
                    // for debugging:
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
                    } else {
                        hitsInSameDoubleColumn++;
                    }
                }

                //------------------------------------------------------------------------------------------------------------------------------
                // for debugging:
                // efficiency vs. event length
                //------------------------------------------------------------------------------------------------------------------------------
                int backgroundHits = evt->pixels.size();
                if (calRow > 0 && calCol > 0 && calRow < 79 && calCol < 51) {
                    if (calFound) {
                        calHitsPerEventDistribution->Fill(backgroundHits-1);
                        hitsPerEventDistribution->Fill(backgroundHits-1);
                    } else {
                        hitsPerEventDistribution->Fill(backgroundHits);
                    }
                }

                //------------------------------------------------------------------------------------------------------------------------------
                // hits/event vs. time
                // remark: can detect beam loss etc.
                //------------------------------------------------------------------------------------------------------------------------------
                hitsPerNEventsVsTime->Fill(m_eventcounter, backgroundHits);

                //------------------------------------------------------------------------------------------------------------------------------
                // for debugging:
                // efficiency vs. hits in other double columns
                //------------------------------------------------------------------------------------------------------------------------------
                if (calRow > 0 && calCol > 0 && calRow < 79 && calCol < 51) {
                    if (calFound) {
                        calHitsVsHitsInOtherDoubleColumnsDistribution->Fill(hitsInOtherDoubleColumns);
                        calHitsVsHitsInSameDoubleColumnsDistribution->Fill(hitsInSameDoubleColumn);
                    }
                    calsVsHitsInOtherDoubleColumnsDistribution->Fill(hitsInOtherDoubleColumns);
                    calsVsHitsInSameDoubleColumnsDistribution->Fill(hitsInSameDoubleColumn);

                }

                //------------------------------------------------------------------------------------------------------------------------------
                // for debugging:
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
                std::vector < std::vector < std::pair<int, int> > > clusters;

                // start with all hits from the event
                for (int hitpix = 0; hitpix < nHitsTotal;hitpix++) {
                    if ((evt->pixels[hitpix].column() != calCol || evt->pixels[hitpix].row() != calRow) && evt->pixels[hitpix].value() > m_min_adc) {
                        pixels.push_back(std::make_pair(evt->pixels[hitpix].column(), evt->pixels[hitpix].row()));
                    }
                }

                int nClusters = 0;
                int newPixelsFound = 0;
                while (pixels.size() > 0 || newPixelsFound > 0) {

                    // take last pixel to initialize the clusters
                    if (cluster.size() < 1) {
                        cluster.push_back(pixels[pixels.size()-1]);
                        pixels.pop_back();
                    }

                    // loop over all pixels and check if one of them is close to already found cluster and merge pixel with this cluster
                    newPixelsFound = 0;
                    for (int i=pixels.size()-1;i>=0;i--) {
                        for (int j=0;j<cluster.size();j++) {
                            if (abs(pixels[i].first-cluster[j].first) < m_clusterRadius && abs(pixels[i].second-cluster[j].second) < m_clusterRadius) {
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
                        sort(cluster.begin(), cluster.end() );
                        clusters.push_back(cluster);
                        cluster.clear();
                    } else {
                        // remove duplicates from cluster
                        sort(cluster.begin(), cluster.end() );
                        cluster.erase(unique( cluster.begin(), cluster.end() ), cluster.end() );
                    }
                }
                if (cluster.size() > 0) {
                    sort(cluster.begin(), cluster.end() );
                    clusters.push_back(cluster);
                    cluster.clear();
                }

                //------------------------------------------------------------------------------------------------------------------------------
                // fill cluster size histograms
                //------------------------------------------------------------------------------------------------------------------------------
                for (std::vector < std::vector < std::pair<int, int> > >::iterator clusterIterator = clusters.begin(); clusterIterator != clusters.end(); clusterIterator++) {
                    long sumCol=0;
                    long sumRow=0;
                    int minRow = 82;
                    int maxRow = -1;
                    int minCol = 53;
                    int maxCol = -1;

                    // skip injected pixel, if it was alone
                    if ((*clusterIterator).size() == 1 && (*clusterIterator)[0].first == calCol &&  (*clusterIterator)[0].second == calRow) {
                        break;
                    }

                    for (unsigned int clusterPixelIndex=0;clusterPixelIndex<(*clusterIterator).size();clusterPixelIndex++) {
                        sumCol += (*clusterIterator)[clusterPixelIndex].first;
                        sumRow += (*clusterIterator)[clusterPixelIndex].second;
                        if ((*clusterIterator)[clusterPixelIndex].first > maxCol) maxCol = (*clusterIterator)[clusterPixelIndex].first;
                        if ((*clusterIterator)[clusterPixelIndex].first < minCol) minCol = (*clusterIterator)[clusterPixelIndex].first;
                        if ((*clusterIterator)[clusterPixelIndex].second > maxRow) maxRow = (*clusterIterator)[clusterPixelIndex].second;
                        if ((*clusterIterator)[clusterPixelIndex].second < minRow) minRow = (*clusterIterator)[clusterPixelIndex].second;
                    }
                    if ((*clusterIterator).size() > 0) {
                        clusterMap->Fill(sumCol / (double) (*clusterIterator).size(), sumRow / (double)(*clusterIterator).size());
                        clusterSizeDistribution->Fill((*clusterIterator).size());
                        clusterSizeDistributionCol->Fill(maxCol-minCol+1);
                        clusterSizeDistributionRow->Fill(maxRow-minRow+1);
                    }
                }

                //------------------------------------------------------------------------------------------------------------------------------
                // check if injected pixel is near/in a cluster
                //------------------------------------------------------------------------------------------------------------------------------
                bool calibrateIsNearCluster = false;
                int clusterSizeNearCalibrate = 0;

                for (std::vector < std::vector < std::pair<int, int> > >::iterator clusterIterator = clusters.begin(); clusterIterator != clusters.end(); clusterIterator++) {
                    for (unsigned int clusterPixelIndex=0;clusterPixelIndex<(*clusterIterator).size();clusterPixelIndex++) {
                        int clusterCol = (*clusterIterator)[clusterPixelIndex].first;
                        int clusterRow = (*clusterIterator)[clusterPixelIndex].second;
                        if (abs(clusterCol-calCol) < m_clusterRadius && abs(clusterRow-calRow) < m_clusterRadius) {
                            calibrateIsNearCluster = true;
                            clusterSizeNearCalibrate = (*clusterIterator).size();
                            break;
                        }
                    }
                    if (calibrateIsNearCluster) {
                        break;
                    }
                }
                if (calFound) {
                    // no hits from particles nearby, but still the injected calibrate
                    if (!calibrateIsNearCluster) {
                        clusterSizeNearCalibrate = 1;
                    }
                    calClusterFound->Fill(clusterSizeNearCalibrate);
                } else {
                    calClusterNotFound->Fill(clusterSizeNearCalibrate);
                }

                //------------------------------------------------------------------------------------------------------------------------------
                // classify the found clusters by shape
                //------------------------------------------------------------------------------------------------------------------------------

                // loop over all clusters
                for (std::vector < std::vector < std::pair<int, int> > >::iterator itCluster = clusters.begin(); itCluster != clusters.end(); itCluster++) {

                    // find "bottom left" corner (=min row, min col)
                    int minRow = 81;
                    int minCol = 52;
                    for (std::vector < std::pair<int, int> >::iterator clusterPix=(*itCluster).begin(); clusterPix != (*itCluster).end(); clusterPix++) {
                        if ((*clusterPix).first < minCol) minCol = (*clusterPix).first;
                        if ((*clusterPix).second < minRow) minRow = (*clusterPix).second;
                    }

                    // create relative cluster shape vector
                    std::vector < std::pair<int, int> > relativeClusterShape;
                    for (std::vector < std::pair<int, int> >::iterator clusterPix=(*itCluster).begin(); clusterPix != (*itCluster).end(); clusterPix++) {
                        if (!((*clusterPix).first == calCol && (*clusterPix).second == calRow)) {
                            relativeClusterShape.push_back(std::make_pair((*clusterPix).first - minCol, (*clusterPix).second - minRow));
                        }
                    }

                    // count clusters of this shape
                    if (clusterShapes.find(relativeClusterShape) != clusterShapes.end()) {
                        clusterShapes[relativeClusterShape]++;
                    } else {
                        clusterShapes[relativeClusterShape] = 1;
                    }

                }

                //------------------------------------------------------------------------------------------------------------------------------
                // search for clusters, which are exactly split by 1 not working double column
                //------------------------------------------------------------------------------------------------------------------------------

                // loop over all clusters
                for (std::vector < std::vector < std::pair<int, int> > >::iterator itCluster = clusters.begin(); itCluster != clusters.end(); itCluster++) {

                    // find "bottom left" corner (=min row, min col)
                    int maxCol = -1;
                    int minCol = 53;
                    for (std::vector < std::pair<int, int> >::iterator clusterPix=(*itCluster).begin(); clusterPix != (*itCluster).end(); clusterPix++) {
                        if ((*clusterPix).first < minCol) minCol = (*clusterPix).first;
                        if ((*clusterPix).first > maxCol) maxCol = (*clusterPix).first;
                    }

                    std::vector< int > emptyColumns;

                    // todo: this is not efficient...
                    for (int column=minCol; column<=maxCol; column++) {
                        bool found = false;
                        for (std::vector < std::pair<int, int> >::iterator clusterPix=(*itCluster).begin(); clusterPix != (*itCluster).end(); clusterPix++) {
                            if ((*clusterPix).first == column) {
                                found = true;
                                break;
                            }
                        }
                        if (!found) {
                            emptyColumns.push_back(column);
                        }
                    }

                    // min size to reduce coincidental matches
                    if (emptyColumns.size() == 2 && abs(emptyColumns[0] - emptyColumns[1]) == 1 && (*itCluster).size() > 6 && m_brokenclustercandidate < 100) {

                        std::stringstream ss;
                        ss << "brokenClusterCandidate_" << m_brokenclustercandidate << "_inj_" << (int)calCol<< "_" << (int)calRow;
                        TH2D* hitmapEvent = new TH2D(ss.str().c_str(), ss.str().c_str(), 52, 0, 52, 80, 0 ,80);
                        for (int j = 0; j < evt->pixels.size(); j++) {
                            hitmapEvent->Fill(evt->pixels[j].column(), evt->pixels[j].row(), evt->pixels[j].value());
                        }
                        std::stringstream ss2;
                        TCanvas* c1 = new TCanvas("c1","c1",500,500);
                        hitmapEvent->SetStats(kFALSE);
                        hitmapEvent->Draw("colz");

                        TBox* injectedPixel = new TBox(calCol, calRow, calCol+1, calRow+1);
                        injectedPixel->SetFillColor(kMagenta);
                        injectedPixel->Draw();
                        ss2 << outputFolder << "/" << ss.str() << ".pdf";
                        c1->SaveAs(ss2.str().c_str());
                        std::cout << "dump pkam event:" << ss2.str() << std::endl;
                        m_brokenclustercandidate++;


                    }

                }

            }


            //------------------------------------------------------------------------------------------------------------------------------
            // double column 2x2 frames
            //------------------------------------------------------------------------------------------------------------------------------
            std::vector< std::vector< bool > > pixelMap ( 52, std::vector<bool> ( 80, false ) );
            // use all bg hits of the event
            for (int hitpix = 0; hitpix < evt->pixels.size();hitpix++) {
                if ((evt->pixels[hitpix].column() != calCol || evt->pixels[hitpix].row() != calRow) && evt->pixels[hitpix].value() > m_min_adc) {
                    pixelMap[evt->pixels[hitpix].column()][evt->pixels[hitpix].row()] = true;
                }
            }
            for (int doubleColumn=0;doubleColumn<26;doubleColumn++) {
                int nFrames = 0;
                int pos = -99;
                for (int row=0;row<80;row++) {
                    bool hit = pixelMap[doubleColumn*2][row] || pixelMap[doubleColumn*2+1][row];
                    if (hit && row > pos+1) {
                        nFrames++;
                        pos = row;
                    }
                }
                nFramesPerDoubleColumn->Fill(nFrames);
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
        ss << "<div style='float:left;'><a href='" << name << ".pdf'><img src='" << name << ".png' width=350 height=350></a></div>" ;
        htmlText += ss.str();
    }


    void FileWriterEfficiency::writeHistogramTest(TH1D* h1, std::string name, float statsPosX, float statsPosY, float meanClusterSizeInColumn) {

        //std::cout << "go..." << std::endl;
        TCanvas* c1 = new TCanvas("c1","c1",500,500);
        h1->Draw("E0");

        //std::cout << "h drawn" << std::endl;
        double mean = h1->GetMean();
        double integral = h1->Integral();

        // this (red) is a poisson fit, assuming uncorrelated hits (like from xrays etc.)
        TF1* poissonf = new TF1("poissonf","TMath::Poisson(x,[0])*[1]",0,100);
        //std::cout << "tf1 created" << std::endl;
        poissonf->SetParameter(0, mean);
        poissonf->SetParameter(1, integral);
        poissonf->SetLineColor(kRed);
        poissonf->Draw("same");


        // this (green) is a poisson fit, assuming correcting for the cluster size inside the column
        double clusterSize = meanClusterSizeInColumn;
        TF1* poissonf2 = new TF1("poissonf2","TMath::Poisson(x/[2],[0])*[1]",0,100);
        poissonf2->SetParameter(0, mean / clusterSize);
        poissonf2->SetParameter(1, integral / clusterSize);
        poissonf2->SetParameter(2, clusterSize);
        poissonf2->SetLineColor(kGreen+2);
        poissonf2->Draw("same");

        //std::cout << "tf1 drawn" << std::endl;

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
        ss << "<div style='float:left;'><a href='" << name << ".pdf'><img src='" << name << ".png' width=350 height=350></a></div>" ;
        htmlText += ss.str();
    }


    void FileWriterEfficiency::writeHistogram(TH1D* h1, std::string name, bool log) {
        writeHistogram(h1, name, -1.0, -1.0, (char*)"", log);
    }

    void FileWriterEfficiency::writeHistogram(TH1D* h1, std::string name, float statsPosX, float statsPosY, char* drawOption, bool log) {
        TCanvas* c1 = new TCanvas("c1","c1",500,500);
        h1->SetStats(kTRUE);
        if (log) {
            gPad->SetLogy(1);
        }
        h1->Draw(drawOption);
        gPad->Modified(); gPad->Update();
        if (log) {
            gPad->SetLogy(1);
        }
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
        ss << "<div style='float:left;'><a href='" << name << ".pdf'><img src='" << name << ".png' width=350 height=350></a></div>" ;
        htmlText += ss.str();
    }

    // https://stackoverflow.com/questions/5056645/sorting-stdmap-using-value
    template<typename A, typename B>
    std::pair<B,A> flip_pair(const std::pair<A,B> &p)
    {
        return std::pair<B,A>(p.second, p.first);
    }

    template<typename A, typename B>
    std::multimap<B,A> flip_map(const std::map<A,B> &src)
    {
        std::multimap<B,A> dst;
        std::transform(src.begin(), src.end(), std::inserter(dst, dst.begin()),
                       flip_pair<A,B>);
        return dst;
    }

    FileWriterEfficiency::~FileWriterEfficiency() {
        //delete m_ser;

        //------------------------------------------------------------------------------------------------------------------------------
        //------------------------------------------------------------------------------------------------------------------------------
        // print list of cluster shapes to files
        //------------------------------------------------------------------------------------------------------------------------------
        //------------------------------------------------------------------------------------------------------------------------------

        std::cout << "processing cluster shapes...";
        long nclustersTotal=0;
        for (std::map < std::vector< std::pair< int,int >  >, int >::iterator clusterShape = clusterShapes.begin(); clusterShape != clusterShapes.end(); clusterShape++ ) {
            nclustersTotal += clusterShape->second;
        }

        //------------------------------------------------------------------------------------------------------------------------------
        // print list of cluster shapes to files
        //------------------------------------------------------------------------------------------------------------------------------
        //
        // this file will look like:
        //
        // (0,0) : 5.03854 %
        // (0,0) (0,1) : 9.04367 %
        // (0,0) (0,1) (0,2) : 2.28185 %
        //
        // ...

        std::multimap<int, std::vector< std::pair< int,int > > > clusterShapesSorted = flip_map(clusterShapes);

        std::ofstream outputFile;
        std::stringstream ssFileName;
        ssFileName << outputFolder << "/clusters.txt";
        outputFile.open(ssFileName.str());
        for (std::multimap < int, std::vector< std::pair< int,int >  > >::reverse_iterator clusterShape = clusterShapesSorted.rbegin(); clusterShape != clusterShapesSorted.rend(); clusterShape++ ) {
            std::stringstream ssPixels;
            float probability = (float)(clusterShape->first)/ nclustersTotal;
            for (int i=0; i<clusterShape->second.size(); i++) {
                ssPixels << "(" << (int)(clusterShape->second[i].first) << "," << (int)(clusterShape->second[i].second) << ") ";
            }
            outputFile << ssPixels.str() << ": " << 100.0*probability << " % " << std::endl;
        }
        outputFile.close();
        ssFileName.str("");
        ssFileName.clear();
        std::cout << "done, found " << clusterShapes.size() << " unique shapes!" << std::endl;

        // print long clusters
        /*
        std::cout << "---------------------------------------------"  << std::endl;
        std::cout << "looooooooong cluster shapes:" << std::endl;
        for (std::map < std::vector< std::pair< int,int >  >, int >::iterator clusterShape = clusterShapes.begin(); clusterShape != clusterShapes.end(); clusterShape++ ) {
            std::stringstream ssPixels;
            float probability = (float)(clusterShape->second)/ nclustersTotal;
            if (clusterShape->first.size() >= 15) {
                for (int i=0; i<clusterShape->first.size(); i++) {
                    ssPixels << "(" << (int)(clusterShape->first[i].first) << "," << (int)(clusterShape->first[i].second) << ") ";
                }
                std::cout << ssPixels.str() << ": " << 100.0*probability << " % " << std::endl;
            }
        }*/


        //------------------------------------------------------------------------------------------------------------------------------
        // distribution hiostograms
        //------------------------------------------------------------------------------------------------------------------------------
        calEff = new TH1D("calEffDist", "cal eff distribution", 2020, 0 ,101.0);
        calEffNoPKAM = new TH1D("calEffDistNoPKAM", "cal eff distribution (no PKAM)", 2020, 0 ,101.0);
        calEffNoDead = new TH1D("calEffDistNoDead", "cal eff distribution (no dead pixels)", 2020, 0 ,101.0);

        effHitsAbove = new TH1D("calEffDistAbove", "cal eff distribution (hits above)", 2020, 0 ,101.0);
        effHitsAboveBelow = new TH1D("calEffDistAboveBelow", "cal eff distribution (hits below and above)", 2020, 0 ,101.0);
        effHitsBelow = new TH1D("calEffDistBelow", "cal eff distribution (hits below)", 2020, 0 ,101.0);
        effHitsIsolated = new TH1D("calEffDistIsolated", "cal eff distribution (isolated)", 2020, 0 ,101.0);

        efficiencyVsHitsPerEventDistribution = new TH1D("efficiencyVsHitsPerEventDistribution", "efficiencyVsHitsPerEventDistribution", 50, 0, 50);
        efficiencyVsHitsInOtherDoubleColumnsDistribution = new TH1D("efficiencyVsHitsInOtherDoubleColumnsDistribution", "efficiency vs. #hits in other double columns", 50, 0, 50);
        efficiencyVsHitsInSameDoubleColumnsDistribution = new TH1D("efficiencyVsHitsInSameDoubleColumnsDistribution", "efficiency vs. #hits in same double column", 50, 0, 50);

        int calsTotal = 0;
        int PKAMsTotal = 0;
        int bgHitsTotal=0;
        int bgHitPixels = 0;
        int nDeadPixels = 0;

        std::vector<double> calEffNoPKAMList;

        std::vector< double > triggersDC(26, 0);
        std::vector< double > calsDC(26, 0);
        std::vector< double > bgHitsDC(26, 0);

        //------------------------------------------------------------------------------------------------------------------------------
        // aggregate efficiency data from single pixel values (previously filled TH2D's)
        //------------------------------------------------------------------------------------------------------------------------------
        for (int c=m_fiducial_area_col_min;c<=m_fiducial_area_col_max;c++) {
            for (int r=m_fiducial_area_row_min;r<=m_fiducial_area_row_max;r++) {
                int cals = calMap->GetBinContent(1 + c, 1 + r);
                int pkams = pkamMap->GetBinContent(1 + c, 1 + r);
                int errors = errorsMap->GetBinContent(1 + c, 1 + r);
                int bgHits = bgMap->GetBinContent(1 + c, 1 + r);

                if (bgHits > 0) {
                    bgHitPixels++;
                    bgHitsTotal += bgHits;
                }

                double efficiency = (double)cals  / (double) ntrig * 100.0;
                if (efficiency > 100.0) efficiency = 100.0;
                double efficiencyNoPkam = 0;

                //------------------------------------------------------------------------------------------------------------------------------
                // DISABLED: ignore dead/masked pixels
                // careful: this only works when it's impossible to have 0 hits from dynamic inefficiency alone
                // remark: at 600 MHz/cm2, it's safe to do this for ntrig=50
                //------------------------------------------------------------------------------------------------------------------------------
                if (true || (cals > 0 || bgHits > 0)) {
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
                }
                if (cals == 0 && bgHits == 0) {
                    nDeadPixels++;
                } else {
                    calEffNoDead->Fill(efficiency);
                }
                PKAMsTotal += pkams;

                //------------------------------------------------------------------------------------------------------------------------------
                // below was for debugging a special run...
                //------------------------------------------------------------------------------------------------------------------------------
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

        // this include all tested pixels, also the ones which are not used for efficiency calculation
        int nPixelsTestedTotal = 4160;
        int nGoodEvents = ntrig*nPixelsTestedTotal-PKAMsTotal;

        if (ntrig < 1) {
            // test
            nGoodEvents = m_eventcounter - PKAMsTotal;
        }

        std::stringstream ss2;
        ss2.str("");
        ss2.clear();
        ss2 << "<table>";
        ss2 << "<tr><th>property</th><th>value</th></tr>";
        ss2 << "<tr><td>ntrig per pixel</td><td>" << ntrig << "</td></tr>";
        htmlText += ss2.str();


        //------------------------------------------------------------------------------------------------------------------------------
        // hit rate [MHz/cm2]
        //------------------------------------------------------------------------------------------------------------------------------
        double meanRateUncorrected = ((bgHitsTotal) / (bgHitPixels*25*1e-9*150.0*100.0 * 1e-2 * (double)nGoodEvents));
        double rateErrorUncorrected = (sqrt(bgHitsTotal) / (bgHitPixels*25*1e-9*150.0*100.0 * 1e-2 * (double)nGoodEvents));

        //------------------------------------------------------------------------------------------------------------------------------
        // external "beam" rate [MHz/cm2] , rate corrected with (inverse) efficiency of calibrate signals
        //------------------------------------------------------------------------------------------------------------------------------
        double meanRate = meanRateUncorrected/(meanEfficiencyNoPKAM*0.01);
        double rateError = rateErrorUncorrected/(meanEfficiencyNoPKAM*0.01);

        //------------------------------------------------------------------------------------------------------------------------------
        // cluster rate (this might be buggy...)
        //------------------------------------------------------------------------------------------------------------------------------
        double clusterRate = (double)clusterMap->Integral() / (bgHitPixels*25*1e-9*150.0*100.0 * 1e-2 * (double)nGoodEvents);

        std::stringstream ss3;
        ss3 << "<tr><td>Rate:</td><td>" << meanRateUncorrected << " +/- " << rateErrorUncorrected << " MHz/cm2</td></tr>";
        ss3 << "<tr><td>Rate (corrected by cal eff.):</td><td>" <<meanRate << " +/- " << rateError << " MHz/cm2</td></tr>";
        ss3 << "<tr><td>Cluster rate:</td><td>" << clusterRate << " MHz/cm2</td></tr>";
        htmlText += ss3.str();

        //------------------------------------------------------------------------------------------------------------------------------
        // efficiency with PKAM removal
        //------------------------------------------------------------------------------------------------------------------------------
        if (calsTotal-PKAMsTotal > 0) {
            efficiencyNoPKAMError = 100.0 * sqrt( meanEfficiencyNoPKAM * 0.01 * (1-meanEfficiencyNoPKAM* 0.01) / (calsTotal-PKAMsTotal));
        }
        ss2.str("");
        ss2.clear();
        ss2 << "<tr><td>Efficiency (PKAM events & events with R/O errors removed):</td><td>" <<meanEfficiencyNoPKAM << " +/- " << efficiencyNoPKAMError << " %</td></tr>";
        htmlText += ss2.str();
        ss2.str("");
        ss2.clear();
        ss2 << "<tr><td>(cross check) efficiency =</td><td>" << std::accumulate(calEffNoPKAMList.begin(), calEffNoPKAMList.end(), 0.0)/calEffNoPKAMList.size() << " %</td></tr>";
        htmlText += ss2.str();

        //------------------------------------------------------------------------------------------------------------------------------
        // efficiency
        //------------------------------------------------------------------------------------------------------------------------------
        double meanEfficiency = calEff->GetMean();
        double efficiencyError = 100.0 * sqrt( meanEfficiency * 0.01 * (1-meanEfficiency* 0.01) / (calsTotal));
        ss2.str("");
        ss2.clear();
        ss2 << "<tr><td>Efficiency (no bad event correction):</td><td>" <<meanEfficiency << " +/- " << efficiencyError << " %</td></tr>";
        htmlText += ss2.str();

        //------------------------------------------------------------------------------------------------------------------------------
        // efficiency (dead pixels excluded)
        //------------------------------------------------------------------------------------------------------------------------------
        double meanEfficiencyNoDead = calEffNoDead->GetMean();
        double efficiencyErrorNoDead = 100.0 * sqrt( meanEfficiencyNoDead * 0.01 * (1-meanEfficiencyNoDead* 0.01) / (calsTotal));
        ss2.str("");
        ss2.clear();
        ss2 << "<tr><td>Efficiency (dead pixels excluded):</td><td>" <<meanEfficiencyNoDead << " +/- " << efficiencyErrorNoDead << " %</td></tr>";
        htmlText += ss2.str();

        ss2.str("");
        ss2.clear();
        ss2 << "<tr><td>Number of events:</td><td>" << m_eventcounter << " </td></tr>";
        ss2 << "<tr><td>good XPixelAlive events:</td><td>" << nGoodEvents << "</td></tr>";
        ss2 << "<tr><td>Dead pixels:</td><td>" << nDeadPixels << " </td></tr>";
        ss2 << "<tr><td>PKAMs:</td><td>" << m_npkam << " </td></tr>";
        htmlText += ss2.str();


        //------------------------------------------------------------------------------------------------------------------------------
        // efficiency vs. event length
        //------------------------------------------------------------------------------------------------------------------------------
        efficiencyVsHitsPerEventDistribution->Divide(calHitsPerEventDistribution, hitsPerEventDistribution, 1, 1, "b");

        //------------------------------------------------------------------------------------------------------------------------------
        // efficiency vs. hits in other double column
        //------------------------------------------------------------------------------------------------------------------------------
        efficiencyVsHitsInOtherDoubleColumnsDistribution->Divide(calHitsVsHitsInOtherDoubleColumnsDistribution, calsVsHitsInOtherDoubleColumnsDistribution, 1, 1, "b");
        efficiencyVsHitsInSameDoubleColumnsDistribution->Divide(calHitsVsHitsInSameDoubleColumnsDistribution, calsVsHitsInSameDoubleColumnsDistribution, 1, 1, "b");


        //------------------------------------------------------------------------------------------------------------------------------
        // PKAM rate [MHz/cm2]
        //------------------------------------------------------------------------------------------------------------------------------
        double meanRatePKAM = (PKAMsTotal / (4160*25*1e-9*150.0*100.0 * 1e-2 * (double)ntrig * bgHitPixels))/(meanEfficiencyNoPKAM*0.01);
        double ratePKMError = (sqrt(PKAMsTotal) / (4160*25*1e-9*150.0*100.0 * 1e-2 * (double)ntrig * bgHitPixels))/(meanEfficiencyNoPKAM*0.01);
        ss2.str("");
        ss2.clear();
        ss2 << "<tr><td>PKAM rate: </td><td>" <<meanRatePKAM << " +/- " << ratePKMError << " MHz/cm2</td></tr>";
        htmlText += ss2.str();

        //------------------------------------------------------------------------------------------------------------------------------
        // PKAM ratio [% of events]
        //------------------------------------------------------------------------------------------------------------------------------
        double PKAMratio = PKAMsTotal/(4160.0*ntrig);
        ss2.str("");
        ss2.clear();
        ss2 << "<tr><td>PKAM ratio: </td><td>" << 100.0*PKAMratio << " %</td></tr>";
        htmlText += ss2.str();

        ss2.str("");
        ss2.clear();
        ss2 << "<tr><td>Cluster size: </td><td>" << clusterSizeDistribution->GetMean() << "</td></tr>";
        htmlText += ss2.str();
        ss2.str("");
        ss2.clear();
        ss2 << "<tr><td>Cluster size (rows): </td><td>" << clusterSizeDistributionRow->GetMean() << "</td></tr>";
        htmlText += ss2.str();
        ss2.str("");
        ss2.clear();
        ss2 << "<tr><td>Cluster size (cols): </td><td>" << clusterSizeDistributionCol->GetMean() << "</td></tr>";
        ss2 << "</table>";

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
        // decoding statistics
        //------------------------------------------------------------------------------------------------------------------------------
        decoding_stats += decoder.getStatistics();
        std::stringstream ss;
        ss << "<br><p><h4>Decoding statistics:</h4>";
        ss << "<table><tr><th>General information:</td><td></th><td></td></tr>";
        ss << "<tr><td></td><td><16bit words read:        </td><td>" << decoding_stats.info_words_read() << "</td></tr>";
        ss << "<tr><td></td><td>valid events total:       </td><td>" << decoding_stats.info_events_total() << "</td></tr>";
        ss << "<tr><td></td><td>empty events:             </td><td>" << decoding_stats.info_events_empty() << "</td></tr>";
        ss << "<tr><td></td><td>valid events with pixels: </td><td>" << decoding_stats.info_events_valid() << "</td></tr>";
        ss << "<tr><td></td><td>valid pixel hits:         </td><td>" << decoding_stats.info_pixels_valid() << "</td></tr>";
        ss << "<tr><th>Event errors: \t           " << decoding_stats.errors_event() << "</th><td></td></tr>";
        ss << "<tr><td></td><td>start marker:             </td><td>" << decoding_stats.errors_event_start() << "</td></tr>";
        ss << "<tr><td></td><td>stop marker:              </td><td>" << decoding_stats.errors_event_stop() << "</td></tr>";
        ss << "<tr><td></td><td>overflow:                 </td><td>" << decoding_stats.errors_event_overflow() << "</td></tr>";
        ss << "<tr><td></td><td>invalid 5bit words:       </td><td>" << decoding_stats.errors_event_invalid_words() << "</td></tr>";
        ss << "<tr><td></td><td>invalid XOR eye diagram:  </td><td>" << decoding_stats.errors_event_invalid_xor() << "</td></tr>";
        ss << "<tr><td></td><td>frame (failed synchr.):   </td><td>" << decoding_stats.errors_event_frame() << "</td></tr>";
        ss << "<tr><td></td><td>idle data (no TBM trl):   </td><td>" << decoding_stats.errors_event_idledata() << "</td></tr>";
        ss << "<tr><td></td><td>no data (only TBM hdr):   </td><td>" << decoding_stats.errors_event_nodata() << "</td></tr>";
        ss << "<tr><th>TBM errors: \t\t           " << decoding_stats.errors_tbm() << "</th><td></td></tr>";
        ss << "<tr><td></td><td>flawed TBM headers:       </td><td>" << decoding_stats.errors_tbm_header() << "</td></tr>";
        ss << "<tr><td></td><td>flawed TBM trailers:      </td><td>" << decoding_stats.errors_tbm_trailer() << "</td></tr>";
        ss << "<tr><td></td><td>event ID mismatches:      </td><td>" << decoding_stats.errors_tbm_eventid_mismatch() << "</td></tr>";
        ss << "<tr><th>ROC errors: \t\t           " << decoding_stats.errors_roc() << "</th><td></td></tr>";
        ss << "<tr><td></td><td>missing ROC header(s):    </td><td>" << decoding_stats.errors_roc_missing() << "</td></tr>";
        ss << "<tr><td></td><td>misplaced readback start: </td><td>" << decoding_stats.errors_roc_readback() << "</td></tr>";
        ss << "<tr><th>Pixel decoding errors:\t   " << decoding_stats.errors_pixel() << "</th><td></td></tr>";
        ss << "<tr><td></td><td>pixel data incomplete:    </td><td>" << decoding_stats.errors_pixel_incomplete() << "</td></tr>";
        ss << "<tr><td></td><td>pixel address:            </td><td>" << decoding_stats.errors_pixel_address() << "</td></tr>";
        ss << "<tr><td></td><td>pulse height fill bit:    </td><td>" << decoding_stats.errors_pixel_pulseheight() << "</td></tr>";
        ss << "<tr><td></td><td>buffer corruption:        </td><td>" << decoding_stats.errors_pixel_buffer_corrupt() << "</td></tr>";
        ss << "</table>";

        htmlText += ss.str();



        //------------------------------------------------------------------------------------------------------------------------------
        // write HTML histograms
        //------------------------------------------------------------------------------------------------------------------------------
        htmlText += "<br><br><strong>Histograms:</strong><br>";
        writeHistogram(calMap, "calibrateMap");
        writeHistogram(bgMap, "bgMap");
        writeHistogram(pkamMap, "pkamMap");
        writeHistogram(pkamHitmap, "pkamHitMap");
        writeHistogram(calEff, "calEffDistr", true);
        writeHistogram(calEffNoPKAM, "calEffDistrNoPkam",true);
        writeHistogram(errorsMap, "errorsMap");

        writeHistogram(adcDistribution, "adcDistribution", 0.65, 0.65);

        writeHistogram(clusterSizeDistribution, "clusterSizeDistribution", 0.65, 0.65);
        writeHistogram(clusterSizeDistributionCol, "clusterSizeDistributionCol", 0.65, 0.65);
        writeHistogram(clusterSizeDistributionRow, "clusterSizeDistributionRow", 0.65, 0.65);

        writeHistogram(nFramesPerDoubleColumn, "nFramesPerDoubleColumn", true);

        efficiencyVsHitsPerEventDistribution->SetStats(0);
        efficiencyVsHitsPerEventDistribution->GetXaxis()->SetTitle("total hits in event");
        efficiencyVsHitsPerEventDistribution->GetYaxis()->SetTitle("efficiency");

        //double minValue = efficiencyVsHitsPerEventDistribution->GetBinContent(efficiencyVsHitsPerEventDistribution->GetMinimumBin());
        //double maxValue = efficiencyVsHitsPerEventDistribution->GetBinContent(efficiencyVsHitsPerEventDistribution->GetMaximumBin());
        //efficiencyVsHitsPerEventDistribution->GetYaxis()->SetRangeUser(minValue * 0.9-0.1, maxValue*1.01 + 0.01);

        writeHistogram(hitsPerEventDistribution, "hitsPerEventDistribution", 0.65, 0.65);
        writeHistogram(calHitsPerEventDistribution, "calHitsPerEventDistribution", 0.65, 0.65);
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

        efficiencyVsHitsInSameDoubleColumnsDistribution->SetStats(0);
        efficiencyVsHitsInSameDoubleColumnsDistribution->GetXaxis()->SetTitle("hits in other double columns");
        efficiencyVsHitsInSameDoubleColumnsDistribution->GetYaxis()->SetTitle("efficiency");

        writeHistogram(calsVsHitsInSameDoubleColumnsDistribution, "calsVsHitsInSameDoubleColumns", 0.65, 0.65);
        writeHistogram(calHitsVsHitsInSameDoubleColumnsDistribution, "calHitsVsHitsInSameDoubleColumns", 0.65, 0.65);
        writeHistogram(efficiencyVsHitsInSameDoubleColumnsDistribution, "efficiencyVsHitsInSameDoubleColumns", -10, -10, (char*)"E");


        writeHistogram(hitsPerNEventsVsTime, "hitsPerNEventsVsTime", 0.65, 0.15);
        writeHistogram(hitsPerNEventsVsTimeDC, "hitsPerNEventsVsTimeDC");

        writeHistogram(hitsAboveBelowMap, "hitsAboveBelowMap");
        writeHistogram(hitsAboveMap, "hitsAboveMap");
        writeHistogram(hitsBelowMap, "hitsBelowMap");
        writeHistogram(hitsIsolatedMap, "hitsIsolatedMap");
        writeHistogram(calsAboveBelowMap, "calsAboveBelowMap");
        writeHistogram(calsAboveMap, "calsAboveMap");
        writeHistogram(calsBelowMap, "calsBelowMap");
        writeHistogram(calsIsolatedMap, "calsIsolatedMap");

        writeHistogram(effHitsAbove, "effHitsAbove", true);
        writeHistogram(effHitsBelow, "effHitsBelow", true);
        writeHistogram(effHitsAboveBelow, "effHitsAboveBelow", true);
        writeHistogram(effHitsIsolated, "effHitsIsolated", true);

        writeHistogram(clusterMap, "clusterMap");





        for (int dc=0;dc<26;dc++){
            ss2.str("");
            ss2.clear();
            ss2 << "histNhitsDC1D_DC" << dc;
            TH1D* histNhitsDC1D = new TH1D(ss2.str().c_str(), ss2.str().c_str(), 100, 0, 100);
            int nBinsX = m_eventcounter/100;
            for (int j=0; j< nBinsX; j++) {
                histNhitsDC1D->Fill(hitsPerNEventsVsTimeDC->GetBinContent(1+j, 1+ dc));
            }
            writeHistogramTest(histNhitsDC1D, ss2.str().c_str(), 0.65, 0.65, clusterSizeDistributionRow->GetMean());
        }

        //
        if (true) {
            std::string name("calInCluster");
            THStack* hs = new THStack("hs","");

            //calClusterFound->SetFillColor(kBlue);
            //calClusterNotFound->SetFillColor(kRed);
            calClusterFound->SetLineColor(kBlue);
            calClusterNotFound->SetLineColor(kRed);
            hs->Add(calClusterFound);
            hs->Add(calClusterNotFound);
            TCanvas* c1 = new TCanvas("c1","c1",500,500);
            //hs->SetStats(kFALSE);
            gPad->SetLogy(1);
            //hs->Draw("nostackb");
            calClusterFound->Draw();
            calClusterNotFound->Draw("same");

            TLegend* l = new TLegend(0.6,0.7,0.85,0.85);
            l->AddEntry(calClusterFound, "cal found");
            l->AddEntry(calClusterNotFound, "cal not found");
            l->Draw();

            gPad->Update();
            std::stringstream ss;
            ss << outputFolder << "/" << name << ".pdf";
            c1->SaveAs(ss.str().c_str());
            ss.str("");
            ss.clear();

            ss << outputFolder << "/" << name << ".png";
            c1->SaveAs(ss.str().c_str());
            ss.str("");
            ss.clear();
            ss << "<div style='float:left;'><a href='" << name << ".pdf'><img src='" << name << ".png' width=350 height=350></a></div>" ;
            htmlText += ss.str();
        }

        //------------------------------------------------------------------------------------------------------------------------------
        // event dumps
        //------------------------------------------------------------------------------------------------------------------------------
        ss.str("");
        ss.clear();
        ss << "<div style='clear:both;'></div>";
        ss << "<h4>Events (non-empty)</h4>Including the injected calibrate signals drawn with ADC=1<br>" ;
        for (int i=0; i<m_dumpedevents; i++) {
            ss << "<div style='float:left;'><a href='nonempty_event_" << i << ".png' title='Event " << i << "'><img src='nonempty_event_" << i << ".png' width=250 height=250></a></div>";
        }
        ss << "<div style='clear:both;'></div>";
        htmlText += ss.str();

        //------------------------------------------------------------------------------------------------------------------------------
        // pkam dumps
        //------------------------------------------------------------------------------------------------------------------------------
        ss.str("");
        ss.clear();
        ss << "<h4>Events PKAM (truncated)</h4>" ;
        int nPkamsDumped = std::min(m_npkam, m_npkammax);
        for (int i=0; i<nPkamsDumped; i++) {
            ss << "<div style='float:left;'><a href='pkamEvent_" << i << ".png' title='Event " << i << "'><img src='pkamEvent_" << i << ".png' width=250 height=250></a></div>";
        }
        ss << "<div style='clear:both;'></div>";
        htmlText += ss.str();

        //------------------------------------------------------------------------------------------------------------------------------
        // clusters
        //------------------------------------------------------------------------------------------------------------------------------
        int nClustersPrinted = 0;
        int nClustersPrintMax = 100;
        int clustersPrintoutMaxSize = 32;
        ss.str("");
        ss.clear();
        ss << "<h4>Clusters</h4>Injected calibrate pulses are excluded from clustering<br>" ;
        for (std::multimap < int, std::vector< std::pair< int,int >  > >::reverse_iterator clusterShape = clusterShapesSorted.rbegin(); clusterShape != clusterShapesSorted.rend(); clusterShape++ ) {
            std::vector< std::vector< int > > clusterMap ( clustersPrintoutMaxSize, std::vector<int> ( clustersPrintoutMaxSize, 0 ) );

            float probability = (float)(clusterShape->first)/ nclustersTotal;
            bool bad_cluster = false;
            int max_col = 0;
            int max_row = 0;

            for (int i=0; i<clusterShape->second.size(); i++) {
                if (clusterShape->second[i].first < clustersPrintoutMaxSize && clusterShape->second[i].second < clustersPrintoutMaxSize) {
                    clusterMap[clusterShape->second[i].second][clusterShape->second[i].first]++;
                    if (clusterShape->second[i].second > max_row) max_row = clusterShape->second[i].second;
                    if (clusterShape->second[i].first > max_col) max_col = clusterShape->second[i].first;

                } else {
                    bad_cluster = true;
                }

            }
            if (!bad_cluster) {
                nClustersPrinted++;
                ss << "<div style='float:left;border:1px solid #ddd;margin:5px;padding:5px'>" << std::setprecision(3) << probability*100 << "%<br><table>";
                for (int i=0;i<=max_row;i++) {
                    ss << "<tr>";
                    for (int j=0;j<=max_col;j++) {
                        if (clusterMap[i][j]) {
                            ss << "<td style='width:16px;height:16px;background-color:#339;text-align:center;padding:0px;border:1px solid #888;'>x</td>";
                        } else {
                            ss << "<td style='width:16px;height:16px;background-color:white;text-align:center;padding:0px;border:1px solid #888;'></td>";
                        }
                    }
                    ss << "</tr>";
                }
                ss << "</table></div>";
            }
            if (nClustersPrinted > nClustersPrintMax) break;
            if (nClustersPrinted % 10 == 0) {
                ss << "<div style='clear:both;'></div>";

            }

        }
        ss << "<div style='clear:both;'></div>";
        htmlText += ss.str();

        //------------------------------------------------------------------------------------------------------------------------------
        // write efficiency per double column
        //------------------------------------------------------------------------------------------------------------------------------
        htmlText += ss2.str();
        htmlText += "<div style='clear:both;'>--</div><br><br>Per double column:<br>";

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
        // WRITE HTML
        //------------------------------------------------------------------------------------------------------------------------------
        htmlText += "</body></html>";
        ss.str("");
        ss.clear();
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

        ss2 << meanRate << ", " << rateError << ", " << meanEfficiencyNoPKAM << ", " << efficiencyNoPKAMError << ", " << " -1, " << effHitsAbove->GetMean() << ", " << efficiencyErrorAbove << ", " << effHitsBelow->GetMean() << ", " << efficiencyErrorBelow << ", " << effHitsAboveBelow->GetMean() << ", " << efficiencyErrorAboveBelow << ", " << effHitsIsolated->GetMean() << ", " << efficiencyErrorIsolated << ", -1, " << clusterSizeDistribution->GetMean() << ", " << clusterRate << ", " << meanRateUncorrected << ", " << clusterSizeDistributionCol->GetMean() << ", " << clusterSizeDistributionRow->GetMean() << ", " << meanEfficiency << " ";

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
