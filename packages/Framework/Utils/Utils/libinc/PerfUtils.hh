
//Performance gathering utilities, based on file written by Thomas Kittelmann
//for ATLAS during 2011.

#include <sys/time.h>
#include <fcntl.h>
#include <ctime>
#include <cassert>
#include <cstring>
#include <cstdlib>
#include <climits>
#include <limits>
#include <sstream>
#include <string>
#include <map>
#include <cmath>//fabs
#include <cstdlib>//for getenv
#include <dlfcn.h>

#ifdef __linux
#include <malloc.h>
#endif

#include <stdint.h>
#include <unistd.h>

namespace PerfUtils {

  //base functions for collecting data:
  void get_vmem_rss_kb(double&vmem,double&rss,bool vmemonly=false);//~4microsecs
  double get_malloc_kb();//~0.2microsecs (stdcmalloc) or ~4microsecs (tcmalloc)
  double get_vmem_kb();//~4microsecs
  double get_rss_kb();//~4microsecs
  double get_cpu_ms();//~0.3microsecs
  double get_wall_ms();
  void setUTCTimeString(std::string&s,double offset_ms=0);

  struct Meas {
    double vmem;
    double malloc;
    double cpu;
    double wall;
    bool hasWallTime() const { return wall>-950; }
    void capture(bool cpufirst=true)
    {
      m_unused=false;

      if (cpufirst) {
        cpu=get_cpu_ms();
        if (hasWallTime())
          wall=get_wall_ms();
      }

      vmem=get_vmem_kb();
      malloc = m_mallocDisabled ? 0.0 : get_malloc_kb();
      if (!cpufirst) {
        if (hasWallTime())
          wall=get_wall_ms();
        cpu=get_cpu_ms();
      }
      cpu-=m_offset_cpu;
      vmem-=m_offset_vmem;
      malloc-=m_offset_malloc;
      wall-=m_offset_wall;
    }
    Meas(bool enable_walltime=false) : vmem(-999), malloc(-999), cpu(-999),wall(enable_walltime?-900:-999),
                                       m_offset_vmem(0),m_offset_malloc(0),m_offset_cpu(0),m_offset_wall(0),
                                       m_unused(true),m_mallocDisabled(false)
    {}
    //Double counting protection:
    void incrementOffsets(double pvmem,double pmalloc,double pcpu,double pwall=0)
    {
      m_offset_vmem+=pvmem;
      if (!m_mallocDisabled) m_offset_malloc+=pmalloc;
      m_offset_cpu+=pcpu;
      m_offset_wall+=pwall;
    }
    bool unused() const { return m_unused; }
    void captureIfUnused() { if (unused()) capture(); }
    double last_cpu_raw_ms() { return cpu+m_offset_cpu; }//for internal overhead monitoring
    bool mallocDisabled() const { return m_mallocDisabled; }
    void disableMalloc() { m_mallocDisabled=true; m_offset_malloc=malloc=0; }
  private:
    double m_offset_vmem;
    double m_offset_malloc;
    double m_offset_cpu;
    double m_offset_wall;
    bool m_unused;
    bool m_mallocDisabled;
  };

  class CompDataBasic {
  public:
    //Simply calculates a running mean of each variable (intended for
    //steps typically occuring just once in a job)
    CompDataBasic();
    virtual ~CompDataBasic(){}
    virtual void addPointStart(const Meas&);
    virtual void addPointStop(Meas&, unsigned val_nevts=0);
    unsigned nEntries() const;
    double meanDeltaCPU() const;
    double meanDeltaVMEM() const;
    double meanDeltaMalloc() const;

    //for sorting:
    double sortVal(unsigned i) const { return fabs(getVal(i)); }
    double sortValCPU() const { return fabs(m_sum_dcpu); }
    double sortValVMEM() const { return fabs(m_sum_dmem); }
    double sortValMalloc() const { return fabs(m_sum_dmal); }
    double sortValMemory() const { return fabs(m_sum_dmal)+fabs(m_sum_dmem); }
    //
    static const unsigned nvals=3;
    static const unsigned ival_cpu=0;
    static const unsigned ival_vmem=1;
    static const unsigned ival_malloc=2;
    float getVal(unsigned i) const
    {
      if (i==ival_cpu) return m_sum_dcpu;
      else if (i==ival_vmem) return m_sum_dmem;
      else return m_sum_dmal;
    }
    //To be used by the post-processing and double-dso protection:
    void setEntries(unsigned n) {
      m_nentries=n;
    }
    void add(const CompDataBasic& other) {
      //Does deliberately add just 1 to nentries (used this way in post-proc.)
      ++m_nentries;
      m_sum_dcpu+=other.m_sum_dcpu;
      m_sum_dmem+=other.m_sum_dmem;
      m_sum_dmal+=other.m_sum_dmal;
    }
  protected:
    unsigned m_nentries;
    float m_tmp_dcpu;float m_sum_dcpu;
    float m_tmp_dmem;float m_sum_dmem;
    float m_tmp_dmal;float m_sum_dmal;
  };

  class CompDataExtended : public CompDataBasic {
  public:
    //Extends CompDataBasic by also noticing which event had the
    //maximal contribution for each variable (intended for the "evt" step).
    CompDataExtended();
    void addPointStart(const Meas&);
    void addPointStop(Meas&, unsigned val_nevts);
    double maxDeltaCPU() const;
    unsigned maxDeltaCPU_iEntry() const;
    double maxDeltaVMEM() const;
    unsigned maxDeltaVMEM_iEntry() const;
    double maxDeltaMalloc() const;
    unsigned maxDeltaMalloc_iEntry() const;
  private:
    float m_max_dcpu;unsigned m_max_dcpu_ientry;
    float m_max_dmem;unsigned m_max_dmem_ientry;
    float m_max_dmal;unsigned m_max_dmal_ientry;
  };

  struct CompDataStdSteps {
    CompDataExtended data_evt;//evt (after first event)
    CompDataBasic data[4];//ini/1st/fin/cbk (not dso since compNames there are libs)

    //Helper method for proper classification:
    static const int index_other=-2;
    static const int index_evt=-1;
    static const int index_1st=0;
    static const int index_ini=1;
    static const int index_fin=2;
    static const int index_cbk=3;
    static int index(const std::string& stepName);//index in data array (if >=0)
  };

  //Maps for standard steps:
  typedef std::pair<std::string,std::string> OtherKey;
  class FastStrCmp {
    //Fast comparisons without hash calculations.
  public:
    bool operator()(const std::string&a,const std::string&b) const {
      //by size, middle char and finally entire string
      if (a.size()!=b.size()) return a.size()<b.size();
      if (!a.empty()&&a[a.size()-1]!=b[a.size()-1])
        return a[a.size()-1]<b[a.size()-1];
      return a<b;
    }
    bool operator()(const OtherKey&a,const OtherKey&b) const {
      if (a.first.size()!=b.first.size()) return a.first.size()<b.first.size();
      if (a.second.size()!=b.second.size()) return a.second.size()<b.second.size();
      return a<b;
    }
  };
  typedef std::map<std::string,CompDataStdSteps,FastStrCmp> MapStdSteps;
  //Maps for custom steps (not in the same map for memory reasons):
  typedef std::map<OtherKey,CompDataBasic,FastStrCmp> MapOtherSteps;

  // To make sorting and presenting two different types of data storages easier we add a wrapper:
  class StepWrapper {
  public:
    virtual ~StepWrapper(){}
    virtual const std::string& stepName() const = 0;
    virtual const CompDataBasic* getData(const std::string&compName) const = 0;
    virtual void iterateReset() = 0;
    virtual bool getNext(const std::string*& compName,const CompDataBasic*&) = 0;
    virtual unsigned nComps() const =0;
    virtual bool extended() const { return false; }
    const CompDataBasic* getExampleData()//for formatting columns
    {
      iterateReset();
      const std::string* dummy;const CompDataBasic* data;
      bool ok= getNext(dummy,data);
      iterateReset();
      return ok ? data : 0;
    }
  };
  /////////////
  class StepWrapperStd : public StepWrapper {
  public:
    StepWrapperStd(const char* stepName_,const MapStdSteps*m)
      : m_map(m),
        m_it(m_map->begin()),
        m_itE(m_map->end()),
        m_stepName(stepName_),
        m_idx(CompDataStdSteps::index(stepName_))
    {}
    const std::string& stepName() const { return m_stepName; }

    const CompDataBasic* getData(const std::string&
#ifndef NDEBUG
				 compName
#endif
				 ) const
    {
#ifndef NDEBUG
      MapStdSteps::const_iterator it = m_map->find(compName);
#endif
      assert(it!=m_map->end());
      assert(false);//TKESS: must be buggy to return 0...
      return 0;
    }
    virtual void iterateReset() { m_it=m_map->begin(); }
    virtual bool getNext(const std::string*& compName,const CompDataBasic*&data) {
      if (m_it==m_itE) return false;
      compName=&(m_it->first);
      if (m_idx==CompDataStdSteps::index_evt) data=&(m_it->second.data_evt);
      else data=&(m_it->second.data[m_idx]);
      ++m_it;
      return true;
    }
    virtual unsigned nComps() const { return m_map->size(); }
    virtual bool extended() const { return (m_idx==CompDataStdSteps::index_evt); }
  private:
    const MapStdSteps* m_map;
    MapStdSteps::const_iterator m_it;
    MapStdSteps::const_iterator m_itE;
    const std::string m_stepName;
    int m_idx;
  };
  /////////////
  class StepWrapperOther : public StepWrapper {
  public:
    StepWrapperOther(const std::string&stepName_,const MapOtherSteps*m)
      : m_map(m), m_itE(m->end()), m_stepName(stepName_)
    {
      iterateReset();
    }
    const std::string& stepName() const { return m_stepName; }
    const CompDataBasic* getData(const std::string&compName) const
    {
      MapOtherSteps::const_iterator it= m_map->find(OtherKey(compName,m_stepName));
      assert(it!=m_itE);
      return &(it->second);
    }
    void iterateReset() { m_it=m_map->begin(); }
    bool getNext(const std::string*& compName,const CompDataBasic*&data)
    {
      if (m_it==m_itE) return false;
      while(m_it!=m_itE&&m_it->first.second!=m_stepName) { ++m_it; }
      if (m_it==m_itE) return false;
      compName=&(m_it->first.first);
      data=&(m_it->second);
      ++m_it;
      return true;
    }
    virtual unsigned nComps() const { return m_map->size(); }
  private:
    const MapOtherSteps* m_map;
    MapOtherSteps::const_iterator m_it;
    MapOtherSteps::const_iterator m_itE;
    const std::string m_stepName;
  };
  inline unsigned vmpeak()
  {
    FILE* file = fopen("/proc/self/status", "r");
    if (!file)
      return 0;
    unsigned result = 0;
    char line[256];
    while (fgets(line, 256, file) != NULL){
      if (strncmp(line, "VmPeak:", 7) == 0) {
        result=atoi(&(line[7]));
        break;
      }
    }
    fclose(file);
    return result;
  }
  inline std::string get_field(const char* filename,const std::string& fieldname,char space='_')
  {
    //nb: not hyper-optimised
    FILE* file = fopen(filename, "r");
    if (!file)
      return "unknown";
    std::string result;
    result.reserve(128);
    result="unknown";
    char line[256];
    while (fgets(line, 256, file) != NULL){
      if (strncmp(line, fieldname.c_str(), fieldname.size()) == 0) {
        size_t pos = std::string(line).find( ':', fieldname.size() );
        if (pos==std::string::npos) {
          fclose(file);
          return result;
        }
        result.clear();
        bool ignore_nextspace=true;
        ++pos;
        for (;pos<256;++pos) {
          if (line[pos]=='\n')
            break;
          if (line[pos]==' '||line[pos]=='\t') {
            if (ignore_nextspace)
              continue;
            ignore_nextspace=true;
            result+=space;
          } else {
            result+=line[pos];
            ignore_nextspace=false;
          }
        }
        break;
      }
    }
    fclose(file);
    //make sure there are no trailing "spaces":
    while (!result.empty()&&result.at(result.size()-1)==space)
      result.resize(result.size()-1);
    return result;
  }
  inline int64_t clock_nooverflow();
  inline std::string cpu_model()
  {
    std::stringstream s;
    s<<get_field("/proc/cpuinfo","model name",'_')<<"/"<<get_field("/proc/cpuinfo","cache size",'_');
    return s.str();
  }
  inline int bogomips()
  {
    std::string bm=get_field("/proc/cpuinfo","bogomips",' ');
    if (bm=="unknown")
      return 0;
    return int(atof(bm.c_str())+0.5);
  }
  inline unsigned long system_boot_time_seconds()
  {
    FILE* file = fopen("/proc/stat", "r");
    if (!file)
      return 0;
    char line[32];
    while (fgets(line, 32, file) != NULL){
      if (strncmp(line, "btime ", 6) == 0) {
        unsigned long l;
        sscanf(&(line[6]),"%lu",&l);
        fclose(file);
        return l;
      }
    }
    fclose(file);
    return 0;
  }

  inline double get_absolute_wall_ms()
  {
    timeval  tv;
    int err=gettimeofday(&tv, 0);
    if (err!=0)
      return 0.0;
    uint64_t res_usec=uint64_t(tv.tv_sec)*1000000 + uint64_t(tv.tv_usec);
    return 0.001*res_usec;
  }

  inline double secs_per_jiffy()
  {
#ifdef __linux
    //tick will almost certainly be one of 100, 250, 300 or 1000
    //depending on the kernel configuration (USER_HZ).
    long tick=sysconf(_SC_CLK_TCK);
    if (tick>=1&&tick<1000000)
      return 1.0/tick;
#endif
    //Fall-back to getting it the hard way:
    clock_t t0=clock(); clock_t t1=t0;  while (t1==t0) t1=clock();
    return double(t1-t0)/CLOCKS_PER_SEC;
  }

  inline double jobstart_jiffy2unix_ms(const std::string& s) {
    const double d=strtod(&(s[0]),0); if (d<=0.) return 0;
    const unsigned long b=system_boot_time_seconds(); if (!b) return  0;
    const double spj = secs_per_jiffy(); if (!spj) return 0;
    return 1000.0*(d*spj+b);
  }

  inline const char * symb2lib(const char*symbol,const char * failstr = "unknown") {
    void * addr = dlsym(RTLD_DEFAULT,symbol);
    if (!addr) return failstr;
    Dl_info di;
    if (!dladdr(addr, &di)) return failstr;
    if (!di.dli_fname) return failstr;
    return di.dli_fname;
  }
}

//////////////////////
//  IMPLEMENTATION  //
//////////////////////

inline void PerfUtils::get_vmem_rss_kb(double&vmem,double&rss,bool vmemonly) {
  vmem=rss=0.0;
#ifndef __linux
  vmemonly=false;//avoid compile warning
  return;
#else
  //It would be a lot faster without this open call...
  int fd = open("/proc/self/statm", O_RDONLY);
  if (fd<0) return;
  static char data[32];
  int l = read(fd, data, sizeof(data));
  close(fd);
  if (l<4) return;//Failure or suspiciously short
  static float pg_size_in_kb = sysconf(_SC_PAGESIZE)/1024.0;
  //Assume that the first field is vmem and the second is rss.
  vmem=atoi(data)*pg_size_in_kb;
  if (vmemonly)
    return;
  for (unsigned i=0;i+1<sizeof(data);++i)
    if (data[i]==' ') {
      rss=atoi(&(data[i+1]))*pg_size_in_kb;
      return;
    }
#endif
}
inline double PerfUtils::get_vmem_kb() { double v,r;get_vmem_rss_kb(v,r,true); return v; }
inline double PerfUtils::get_rss_kb() { double v,r;get_vmem_rss_kb(v,r); return r; }
inline double PerfUtils::get_malloc_kb() {
#ifndef __linux
  return 0.0;
#else
  //~0.2microsec/call under libc malloc, 4microsec/call under tcmalloc (mallinfo() call dominates).
  //NB: Bug in http://sourceware.org/bugzilla/show_bug.cgi?id=4349
  //makes mallinfo quite slow for fragmented processes on libc<=2.5 (like on SLC5)
  struct mallinfo mi=mallinfo();
  //mallinfo fields are always "int" which is pretty annoying. As long
  //as the true amount of mallocs between two calls does not change
  //more than 2GB, we can work around this with the following ugly
  //code, repeated for each of the two fields we are looking at:
  int64_t uordblks_raw = mi.uordblks;
  if (sizeof(mi.uordblks)==sizeof(int32_t)) {
    //assert(typeid(mi.uordblks)==typeid(int32_t));
    const int64_t half_range =std::numeric_limits<int32_t>::max();
    static int64_t last_uordblks=mi.uordblks;
    static int64_t offset_uordblks=0;
    if (uordblks_raw-last_uordblks>half_range) {
      //underflow detected
      offset_uordblks-=2*half_range;
    } else if (last_uordblks-uordblks_raw>half_range) {
      //overflow detected
      offset_uordblks+=2*half_range;
    }
    last_uordblks=uordblks_raw;
    uordblks_raw+=offset_uordblks;
  }
  //exact same code for hblkhd (a bit of code duplication...):
  int64_t hblkhd_raw = mi.hblkhd;
  if (sizeof(mi.hblkhd)==sizeof(int32_t)) {
    //assert(typeid(mi.hblkhd)==typeid(int32_t));
    const int64_t half_range =std::numeric_limits<int32_t>::max();
    static int64_t last_hblkhd=mi.hblkhd;
    static int64_t offset_hblkhd=0;
    if (hblkhd_raw-last_hblkhd>half_range) {
      //underflow detected
      offset_hblkhd-=2*half_range;
    } else if (last_hblkhd-hblkhd_raw>half_range) {
      //overflow detected
      offset_hblkhd+=2*half_range;
    }
    last_hblkhd=hblkhd_raw;
    hblkhd_raw+=offset_hblkhd;
  }
  return (uordblks_raw+hblkhd_raw)/1024.0;
#endif
}

inline int64_t PerfUtils::clock_nooverflow() {
  //In gnu, clock_t is a long and CLOCKS_PER_SECOND 1000000 so clock()
  //will overflow in 32bit builds after a few thousands of seconds. To
  //avoid this, we have the following method instead which notices when
  //overflow occurs and corrects for it (it won't notice if it doesn't
  //get called for >4000ms, but this should be ok for almost all of our
  //use cases):
  assert(std::numeric_limits<clock_t>::is_integer);
  if (sizeof(clock_t)>=sizeof(int64_t))
    return clock();//64bit builds shouldn't have overflow issues.

  //not so clean with statics i guess:
  static clock_t last=clock();
  static int64_t offset=0;
  clock_t c=clock();
  if (c<last)
    offset+=int64_t(std::numeric_limits<unsigned>::max())-int64_t(std::numeric_limits<unsigned>::min());
  last=c;
  return offset+c;
}

inline double PerfUtils::get_cpu_ms() { return clock_nooverflow()*(1000.0/CLOCKS_PER_SEC); }
inline PerfUtils::CompDataBasic::CompDataBasic()
  : m_nentries(0),
    m_tmp_dcpu(0.),m_sum_dcpu(0.),
    m_tmp_dmem(0.),m_sum_dmem(0.),
    m_tmp_dmal(0.),m_sum_dmal(0.)
{
}
inline void PerfUtils::CompDataBasic::addPointStart(const Meas&me)
{
  m_tmp_dcpu=me.cpu;
  m_tmp_dmem=me.vmem;
  m_tmp_dmal=me.malloc;
}
inline void PerfUtils::CompDataBasic::addPointStop(Meas&me,unsigned)
{
  float dcpu(me.cpu-m_tmp_dcpu);m_sum_dcpu+=dcpu;
  float dmem(me.vmem-m_tmp_dmem);m_sum_dmem+=dmem;
  float dmal(me.malloc-m_tmp_dmal);m_sum_dmal+=dmal;
  me.incrementOffsets(dmem,dmal,dcpu);//register that we dished these out already.
  ++m_nentries;
}
inline unsigned PerfUtils::CompDataBasic::nEntries() const { return m_nentries; }
inline double PerfUtils::CompDataBasic::meanDeltaCPU() const { return m_nentries ? m_sum_dcpu/m_nentries : 0.0; }
inline double PerfUtils::CompDataBasic::meanDeltaVMEM() const { return m_nentries ? m_sum_dmem/m_nentries : 0.0; }
inline double PerfUtils::CompDataBasic::meanDeltaMalloc() const { return m_nentries ? m_sum_dmal/m_nentries : 0.0; }
inline PerfUtils::CompDataExtended::CompDataExtended()
  : CompDataBasic(),
    m_max_dcpu(0.),m_max_dcpu_ientry(0),
    m_max_dmem(0.),m_max_dmem_ientry(0),
    m_max_dmal(0.),m_max_dmal_ientry(0)
{}
inline void PerfUtils::CompDataExtended::addPointStart(const Meas&me)
{
  m_tmp_dcpu=me.cpu;
  m_tmp_dmem=me.vmem;
  m_tmp_dmal=me.malloc;
}
inline void PerfUtils::CompDataExtended::addPointStop(Meas&me,unsigned val_nevt)
{
  float dcpu(me.cpu-m_tmp_dcpu);m_sum_dcpu+=dcpu;
  float dmem(me.vmem-m_tmp_dmem);m_sum_dmem+=dmem;
  float dmal(me.malloc-m_tmp_dmal);m_sum_dmal+=dmal;
  me.incrementOffsets(dmem,dmal,dcpu);//register that we dished these out already.
  if (dcpu>m_max_dcpu) { m_max_dcpu=dcpu; m_max_dcpu_ientry=val_nevt; }
  if (dmem>m_max_dmem) { m_max_dmem=dmem; m_max_dmem_ientry=val_nevt; }
  if (dmal>m_max_dmal) { m_max_dmal=dmal; m_max_dmal_ientry=val_nevt; }
  ++m_nentries;
}
inline double PerfUtils::CompDataExtended::maxDeltaCPU() const { return m_max_dcpu; }
inline unsigned PerfUtils::CompDataExtended::maxDeltaCPU_iEntry() const { return m_max_dcpu_ientry; }
inline double PerfUtils::CompDataExtended::maxDeltaVMEM() const { return m_max_dmem; }
inline unsigned PerfUtils::CompDataExtended::maxDeltaVMEM_iEntry() const { return m_max_dmem_ientry; }
inline double PerfUtils::CompDataExtended::maxDeltaMalloc() const { return m_max_dmal; }
inline unsigned PerfUtils::CompDataExtended::maxDeltaMalloc_iEntry() const { return m_max_dmal_ientry; }
inline int PerfUtils::CompDataStdSteps::index(const std::string& stepName)
{
  static std::string last="";
  static int lastval=index_other;
  if (stepName==last)
    return lastval;
  if (stepName=="evt") lastval=index_evt;
  else if (stepName=="ini") lastval=index_ini;
  else if (stepName=="fin") lastval=index_fin;
  else if (stepName=="cbk") lastval=index_cbk;
  else if (stepName=="1st") lastval=index_1st;
  else lastval=index_other;
  last=stepName;
  return lastval;
}

inline void PerfUtils::setUTCTimeString(std::string&ss,double offset_ms)
{
  //Encoded in ISO 8601 UTC time format. It can be decoded in python
  //(in a "naive" timezone unaware datetime object) with:
  //datetime.strptime(input,'%Y-%m-%dT%H:%M:%S+0000')
  time_t t = time(NULL);//unixtime in seconds
  if (offset_ms)
    t+=static_cast<time_t>(0.5+0.001*offset_ms);
  tm *ptm = gmtime(&t);
  ss.clear();
  ss.resize(26);
  size_t r = strftime(&(ss[0]),25,"%Y-%m-%dT%H:%M:%S+0000",ptm);
  if (r>0&&r<25) ss.resize(r);
  else ss="0000-00-00T00:00:00+0000";
}

inline double PerfUtils::get_wall_ms()
{
  //don't care about timezone since we will just be doing differences
  //(ignore daylight savings issues...). Like clock() it returns 0.0
  //on first call in a process.
  double t=get_absolute_wall_ms();
  static double offset = -1;
  if (offset==-1)
    offset=t;
  return t-offset;
}
