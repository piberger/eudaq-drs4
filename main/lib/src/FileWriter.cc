#include "eudaq/FileWriter.hh"
#include "eudaq/FileNamer.hh"
#include "eudaq/Exception.hh"

namespace eudaq {

  namespace {
    typedef std::map<std::string, FileWriterFactory::factoryfunc> map_t;

    static map_t & FileWriterMap() {
      static map_t m;
      return m;
    }
  }

  void FileWriterFactory::do_register(const std::string & name, FileWriterFactory::factoryfunc func) {
    //std::cout << "DEBUG: Registering FileWriter: " << name << std::endl;
    FileWriterMap()[name] = func;
  }

  FileWriter * FileWriterFactory::Create(const std::string & name,Configuration *config, const std::string & params) {
    map_t::const_iterator it = FileWriterMap().find(name == "" ? "native" : name);
    if (it == FileWriterMap().end()) EUDAQ_THROW("Unknown file writer: " + name);
    FileWriter *fw = (it->second)(params);
    fw->SetConfig(config);
    fw->Configure();
    return fw;
  }

  std::vector<std::string> FileWriterFactory::GetTypes() {
    std::vector<std::string> result;
    for (map_t::const_iterator it = FileWriterMap().begin(); it != FileWriterMap().end(); ++it) {
      result.push_back(it->first);
    }
    return result;
  }

  FileWriter::FileWriter(Configuration* config) : m_filepattern(FileNamer::default_pattern),m_config(config) {}
  FileWriter::FileWriter() : m_filepattern(FileNamer::default_pattern),m_config(nullptr) {}
  void FileWriter::Configure() {};
  // Used to get the max event number to evaluate: DA
  long FileWriter::GetMaxEventNumber() {return 0; };
}
