#ifndef EUDAQ_INCLUDED_TUEvent
#define EUDAQ_INCLUDED_TUEvent

#include <vector>
#include "eudaq/Event.hh"
#include "eudaq/Platform.hh"

namespace eudaq {

  class DLLEXPORT TUEvent : public Event {
    EUDAQ_DECLARE_EVENT(TUEvent);
    public:
    typedef std::vector<uint64_t> vector_t;
    virtual void Serialize(Serializer &) const;
    explicit TUEvent(unsigned run, unsigned event, uint64_t timestamp,
        const vector_t & extratimes = vector_t()) :
      Event(run, event, timestamp),
      m_extratimes(extratimes) {}
    explicit TUEvent(Deserializer &);
    virtual void Print(std::ostream &) const;

    /// Return "TUEvent" as type.
    virtual std::string GetType() const {return "TUEvent";}

    static TUEvent BORE(unsigned run) {
      return TUEvent(run);
    }
    static TUEvent EORE(unsigned run, unsigned event) {
      return TUEvent(run, event);
    }
    private:
    TUEvent(unsigned run, unsigned event = 0)
      : Event(run, event, NOTIMESTAMP, event ? Event::FLAG_EORE : Event::FLAG_BORE)
      {}
    vector_t m_extratimes;
  };

}


#endif // EUDAQ_INCLUDED_TUEvent
