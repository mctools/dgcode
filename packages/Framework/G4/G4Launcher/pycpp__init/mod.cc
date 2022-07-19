#include "Core/Python.hh"
#include "G4Launcher/Launcher.hh"
#include "G4Interfaces/GeoConstructBase.hh"
#include "G4Interfaces/ParticleGenBase.hh"
#include "G4Interfaces/StepFilterBase.hh"
#include "G4Interfaces/PhysListProviderBase.hh"
#include "G4RunManager.hh"
#include "G4UserSteppingAction.hh"
#include "G4UserEventAction.hh"
#include "G4Version.hh"

namespace G4Launcher_py {
  std::string Launcher_rndEvtMsgMode(G4Launcher::Launcher* l)
  { return l->rndEvtMsgMode(); }

  G4ThreeVector pytuple2g4vect(const py::tuple&t)
  {
    assert(py::len(t)==3);
    float x = py::extract<float>(t[0]);
    float y = py::extract<float>(t[1]);
    float z = py::extract<float>(t[2]);
    return G4ThreeVector(x,y,z);
  }

  void Launcher_setParticleGun1(G4Launcher::Launcher* l,int pdgcode, double eKin,
                                const py::tuple& pos,const py::tuple& momdir)
  {
    l->setParticleGun(pdgcode,eKin,pytuple2g4vect(pos),pytuple2g4vect(momdir));
  }
  void Launcher_setParticleGun2(G4Launcher::Launcher* l,const char* particleName, double eKin,
                                const py::tuple& pos,const py::tuple& momdir)
  {
    l->setParticleGun(particleName,eKin,pytuple2g4vect(pos),pytuple2g4vect(momdir));
  }

  void Launcher_setOutput_1arg(G4Launcher::Launcher* l,const char* filename) { l->setOutput(filename); }
  void Launcher_setVis_0args(G4Launcher::Launcher* l) { l->setVis(); }
  void Launcher_startSession_0args(G4Launcher::Launcher* l) { l->startSession(); }

  class HookWrapper : public G4Launcher::Launcher::HookFct {
    public:
    HookWrapper(py::object o) : m_o(o) {}
    virtual ~HookWrapper() {}
    virtual void operator()() { m_o(); }
  private:
    py::object m_o;
  };

  void HookFct_addPrePreInitHook(G4Launcher::Launcher*l,py::object f) { l->addPrePreInitHook(new HookWrapper(f)); }
  void HookFct_addPreInitHook(G4Launcher::Launcher*l,py::object f) { l->addPreInitHook(new HookWrapper(f)); }
  void HookFct_addPostInitHook(G4Launcher::Launcher*l,py::object f) { l->addPostInitHook(new HookWrapper(f)); }
  void HookFct_addPostSimHook(G4Launcher::Launcher*l,py::object f) { l->addPostSimHook(new HookWrapper(f)); }
  void HookFct_addPostMPHook(G4Launcher::Launcher*l,py::object f) { l->addPostMPHook(new HookWrapper(f)); }
  int g4version() { return G4VERSION_NUMBER; }
  const char* g4versionstr() { return G4Version.c_str(); }

  class LauncherExtraPars : public Utils::ParametersBase {
  public:
    //To be called in the constructor of derived classes, defining which parameters are available:
    void pyaddPD(const std::string&n, double defval) { addParameterDouble(n,defval); }
    void pyaddPI(const std::string&n, int defval) { addParameterInt(n,defval); }
    void pyaddPB(const std::string&n, bool defval) { addParameterBoolean(n,defval); }
    void pyaddPS(const std::string&n, const std::string& defval) { addParameterString(n,defval); }
  };
}

PYTHON_MODULE
{
  py::import("G4Interfaces");
  py::import("Utils.ParametersBase");

  //We could expose G4RunManager, G4UserSteppingAction and G4UserEventAction as
  //well (at least register_ptr so it can be passed around on the python side)
  //... update: can't find register_ptr anymore, so here is a very basic
  //declaration of at least the stepping and event actions:
  py::class_<G4UserSteppingAction,boost::noncopyable>("G4UserSteppingAction",py::no_init)
    ;
  py::class_<G4UserEventAction,boost::noncopyable>("G4UserEventAction",py::no_init)
    ;

  py::def("g4version",G4Launcher_py::g4version);
  py::def("g4versionstr",G4Launcher_py::g4versionstr);

  py::class_<G4Launcher_py::LauncherExtraPars,boost::noncopyable,py::bases<Utils::ParametersBase> >("LauncherExtraPars")
    .def("addParameterBoolean",&G4Launcher_py::LauncherExtraPars::pyaddPB)
    .def("addParameterInt",&G4Launcher_py::LauncherExtraPars::pyaddPI)
    .def("addParameterString",&G4Launcher_py::LauncherExtraPars::pyaddPS)
    .def("addParameterDouble",&G4Launcher_py::LauncherExtraPars::pyaddPD)
    ;

  py::class_<G4Launcher::Launcher,boost::noncopyable>("Launcher")
    .def(py::init<G4Interfaces::GeoConstructBase*>())
    .def(py::init<G4Interfaces::GeoConstructBase*,G4Interfaces::ParticleGenBase*>())
    .def(py::init<G4Interfaces::GeoConstructBase*,G4Interfaces::ParticleGenBase*,G4Interfaces::StepFilterBase*>())
    .def("setMultiProcessing",&G4Launcher::Launcher::setMultiProcessing)
    .def("setGeo",&G4Launcher::Launcher::setGeo)
    .def("getRunManager",&G4Launcher::Launcher::getRunManager,py::return_ptr())
    .def("setVis",&G4Launcher::Launcher::setVis)
    .def("setVis",&G4Launcher_py::Launcher_setVis_0args)
    .def("startSession",&G4Launcher::Launcher::startSession)
    .def("startSession",&G4Launcher_py::Launcher_startSession_0args)
    .def("startSimulation",&G4Launcher::Launcher::startSimulation)
    .def("setGen",&G4Launcher::Launcher::setGen)
    .def("setFilter",&G4Launcher::Launcher::setFilter)
    .def("setKillFilter",&G4Launcher::Launcher::setKillFilter)
    .def("setRndEvtMsgMode",&G4Launcher::Launcher::setRndEvtMsgMode)
    .def("rndEvtMsgMode",&G4Launcher_py::Launcher_rndEvtMsgMode)
    .def("setPhysicsList",&G4Launcher::Launcher::setPhysicsList)
    .def("setPhysicsListProvider",&G4Launcher::Launcher::setPhysicsListProvider)
    .def("hasPhysicsListProvider",&G4Launcher::Launcher::hasPhysicsListProvider)
    .def("setParticleGun",&G4Launcher_py::Launcher_setParticleGun1)
    .def("setParticleGun",&G4Launcher_py::Launcher_setParticleGun2)
    .def("setOutput",&G4Launcher::Launcher::setOutput)
    .def("setOutput",&G4Launcher_py::Launcher_setOutput_1arg)
    .def("closeOutput",&G4Launcher::Launcher::closeOutput)
    .def("noRandomSetup",&G4Launcher::Launcher::noRandomSetup)
    .def("setSeed",&G4Launcher::Launcher::setSeed)
    .def("setUserSteppingAction",&G4Launcher::Launcher::setUserSteppingAction)
    .def("setUserEventAction",&G4Launcher::Launcher::setUserEventAction)
    .def("cmd",&G4Launcher::Launcher::cmd)
    .def("cmd_preinit",&G4Launcher::Launcher::cmd_preinit)
    .def("cmd_postinit",&G4Launcher::Launcher::cmd_postinit)
    .def("init",&G4Launcher::Launcher::init)
    .def("initVis",&G4Launcher::Launcher::initVis)
    .def("getMultiProcessing",&G4Launcher::Launcher::getMultiProcessing)
    .def("GetNoRandomSetup",&G4Launcher::Launcher::GetNoRandomSetup)
    .def("getSeed",&G4Launcher::Launcher::getSeed)
    .def("getOutputFile",&G4Launcher::Launcher::getOutputFile)
    .def("getOutputMode",&G4Launcher::Launcher::getOutputMode)
    .def("getVis",&G4Launcher::Launcher::getVis)
    .def("getGeo",&G4Launcher::Launcher::getGeo,py::return_ptr())
    .def("getGen",&G4Launcher::Launcher::getGen,py::return_ptr())
    .def("getFilter",&G4Launcher::Launcher::getFilter,py::return_ptr())
    .def("getPhysicsList",&G4Launcher::Launcher::getPhysicsList)
    .def("getPrintPrefix",&G4Launcher::Launcher::getPrintPrefix)
    .def("allowMultipleSettings",&G4Launcher::Launcher::allowMultipleSettings)
    .def("addHist",&G4Launcher::Launcher::addHist)
    .def("setUserData",&G4Launcher::Launcher::setUserData)
    .def("allowFPE",&G4Launcher::Launcher::allowFPE)
    .def("dumpGDML",&G4Launcher::Launcher::dumpGDML)
    .def("preInitDone",&G4Launcher::Launcher::preInitDone)
    .def("rmIsInit",&G4Launcher::Launcher::rmIsInit)
    .def("addPrePreInitHook",&G4Launcher_py::HookFct_addPrePreInitHook)
    .def("addPreInitHook",&G4Launcher_py::HookFct_addPreInitHook)
    .def("addPostInitHook",&G4Launcher_py::HookFct_addPostInitHook)
    .def("addPostSimHook",&G4Launcher_py::HookFct_addPostSimHook)
    .def("addPostMPHook",&G4Launcher_py::HookFct_addPostMPHook)
    .def("prepreinit_hook",&G4Launcher_py::HookFct_addPrePreInitHook)
    .def("preinit_hook",&G4Launcher_py::HookFct_addPreInitHook)
    .def("postinit_hook",&G4Launcher_py::HookFct_addPostInitHook)
    .def("postsim_hook",&G4Launcher_py::HookFct_addPostSimHook)
    .def("postmp_hook",&G4Launcher_py::HookFct_addPostMPHook)
    .def("addPreGenHook",&G4Launcher::Launcher::addPreGenHook)
    .def("addPostGenHook",&G4Launcher::Launcher::addPostGenHook)
    .def("_shutdown",&G4Launcher::Launcher::shutdown)
    ;

  py::def("getTheLauncher",&G4Launcher::Launcher::getTheLauncher,py::return_ptr());
}
