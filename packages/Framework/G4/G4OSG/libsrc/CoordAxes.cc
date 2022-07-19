#include "G4OSG/CoordAxes.hh"
#include "G4OSG/Viewer.hh"
#include "G4SystemOfUnits.hh"

#include "Core/FindData.hh"//NB: Is this our only dependency on other packages?
#include "Core/String.hh"//NB: Is this our only dependency on other packages?
#include <osgDB/ReadFile>
#include <string>
#include <stdexcept>

G4OSG::CoordAxes::CoordAxes(Viewer*viewer,osg::Group* sceneHook)
  : m_viewer(viewer),
    m_sceneHook(sceneHook),
    m_trf(new osg::MatrixTransform),
    m_pos(0,0,0),
    m_scalemod(0),
    m_scale_base_nanometer(1000000000),
    m_visible(false)
{
  assert(viewer);
  assert(sceneHook);

  //
  std::string origin_override(Viewer::envStr("G4OSG_COORDAXISORIGIN"));
  if (!origin_override.empty()) {
    std::vector<std::string> parts;
    Core::split(parts,origin_override,",;");
    if (parts.size()!=3)
      throw std::runtime_error("Invalid length of vector in G4OSG_COORDAXISORIGIN env var");
    for(unsigned i = 0; i <parts.size();++i)
      m_pos[i] = std::stod(parts.at(i));
  }

  //import axes model and make sure normals are normalised when it is scaled:
  osg::ref_ptr<osg::Node> model_axes = osgDB::readNodeFile(Core::findData("G4OSG","axes.osgt"));
  model_axes->getOrCreateStateSet()->setMode(GL_NORMALIZE,osg::StateAttribute::ON);
  m_trf->addChild( model_axes.get() );
  updateTrf();

}

G4OSG::CoordAxes::~CoordAxes()
{
}

void G4OSG::CoordAxes::setPos(const osg::Vec3d&pos,bool makeVisible)
{
  m_viewer->safeStopThreading();
  if (m_pos!=pos) {
    m_pos=pos;
    updateTrf();
  }
  //make visible when pos is changed (makes sense from UI POV):
  if (makeVisible&&!m_visible) {
    m_visible=true;
    m_sceneHook->addChild( m_trf.get() );
  }
  m_viewer->safeStartThreading();
}

double G4OSG::CoordAxes::getScale() const
{
  double fact(m_scalemod?(m_scalemod==1?0.2:0.5):1.0);
  return (fact * CLHEP::nanometer) * m_scale_base_nanometer;
}

void G4OSG::CoordAxes::scaleUp(bool makeVisible)
{
  //We make sure we scale up and down between nice numbers (.. -> 1 cm -> 2cm -> 5cm -> 10cm -> 20cm -> 50cm -> 1meter -> ...)
  if (m_scalemod == 0 && m_scale_base_nanometer >= 1000000000000000000)
    return;//max 1 gigameter
  m_scalemod = (m_scalemod+1)%3;
  if (m_scalemod==1)
    m_scale_base_nanometer *= 10;
  scaleChanged(makeVisible);
}

void G4OSG::CoordAxes::scaleDown(bool makeVisible)
{
  if (m_scalemod == 0 && m_scale_base_nanometer == 1)
    return;//min 1 nanometer
  m_scalemod = (m_scalemod-1+3)%3;
  if (m_scalemod==0)
    m_scale_base_nanometer /= 10;
  scaleChanged(makeVisible);
}

void G4OSG::CoordAxes::overrideScale(double val,bool makeVisible)
{
  m_scale_base_nanometer = val*1e6;//mm to nanometer
  m_scalemod = 0;
  scaleChanged(makeVisible);
}


void G4OSG::CoordAxes::scaleChanged(bool makeVisible)
{
  bool vis(makeVisible||m_visible);
  if (vis)
    m_viewer->safeStopThreading();
  updateTrf();
  if (makeVisible&&!m_visible) {
    m_visible=true;
    m_sceneHook->addChild( m_trf.get() );
  }
  if (vis)
    m_viewer->safeStartThreading();
}

void G4OSG::CoordAxes::toggleVisible()
{
  m_viewer->safeStopThreading();
  m_visible=!m_visible;
  if (m_visible)
    m_sceneHook->addChild( m_trf.get() );
  else
    m_sceneHook->removeChild( m_trf.get() );
  m_viewer->safeStartThreading();
}

void G4OSG::CoordAxes::updateTrf()
{
  //The tip of the arrows in the model axes.osgt actually extend to 1.2, so we
  //scale it down accordingly:
  double scale = getScale();
  m_trf->setMatrix( osg::Matrix::scale(scale/1.2,scale/1.2,scale/1.2)
                    *osg::Matrix::translate(m_pos));
}
