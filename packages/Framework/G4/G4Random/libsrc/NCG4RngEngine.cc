#include "NCG4RngEngine.hh"
#include "NCrystalRel/internal_NCString.hh"

NCG4RngEngine::~NCG4RngEngine() = default;

void NCG4RngEngine::setSeed( long seed, int )
{
  long tmp[2] = { seed, 0 };
  int dummy{0};
  setSeeds(&tmp[0],dummy);
}

void NCG4RngEngine::setSeeds( const long * seeds, int )
{
  // seeds is 0 terminated array, second parameter ignored.
  std::vector<long> v;
  v.reserve(2);
  while ( seeds && *seeds && v.size() < 2 )
    v.push_back( *seeds++ );
  if ( v.empty() )
    v.push_back( 1234567890 );
  const uint32_t val1 = static_cast<uint32_t>( v.front() );
  const uint32_t val2 = static_cast<uint32_t>( v.back() );
  const uint64_t seedval = static_cast<uint64_t>( val1 << 16 ) | static_cast<uint64_t>(val2);
  dgcode_set64BitSeed( seedval );
}

void NCG4RngEngine::saveStatus( const char filename[] ) const
{
  std::ofstream fh( filename, std::ios::out ) ;
  if ( !fh.bad() )
    put(fh);
}

void NCG4RngEngine::restoreStatus( const char filename[] )
{
  std::ifstream inFile( filename, std::ios::in);
  if (!checkFile ( inFile, filename, name(), "restoreStatus" )) {
    std::cerr << "  -- Engine state remains unchanged\n";
    return;
  }
  get(inFile);
}

void NCG4RngEngine::showStatus() const
{
  assert(m_rng.state().size()==2);
  std::cout << std::endl;
  std::cout << "--------- NCrystalXoroshiroEngine engine status ---------" << std::endl;
  std::cout << " Current state = { "<<m_rng.state()[0]<<", "<<m_rng.state()[1]<<"}\n";
  std::cout << "----------------------------------------" << std::endl;
}

std::ostream & NCG4RngEngine::put( std::ostream& os ) const
{
  assert(m_rng.state().size()==2);
  os << "NCrystalXoroshiroEngine-begin\n"
     << m_rng.state()[0] << "\n"
     << m_rng.state()[1] << "\n"
     << "NCrystalXoroshiroEngine-end\n";
  return os;
}

std::istream& NCG4RngEngine::get ( std::istream& is )
{
  auto endBad = [&is]() -> std::istream&
  {
    is.clear(std::ios::badbit | is.rdstate());
    std::cerr << "\nInput stream mispositioned or"
              << "\nNCG4RngEngine state description missing or"
              << "\nwrong engine type found." << std::endl;
    return is;
  };
  auto strtouint64t = []( const std::string& ss ) -> std::pair<bool,uint64_t>
  {
    try {
      return { true, static_cast<uint64_t>( std::stoull(ss) ) };
    } catch ( std::invalid_argument& ) {
      return { false, 0 };
    } catch ( std::out_of_range& ) {
      return { false, 0 };
    }
  };

  std::string tmp;
  NC::RandXRSRImpl::state_t state;
  is >> tmp;
  if ( tmp != "NCrystalXoroshiroEngine-begin" )
    return endBad();
  is >> tmp;
  bool ok;
  std::tie(ok,state[0]) = strtouint64t( tmp );
  if (!ok)
    return endBad();
  is >> tmp;
  std::tie(ok,state[1]) = strtouint64t( tmp );
  if (!ok)
    return endBad();
  is >> tmp;
  if ( tmp != "NCrystalXoroshiroEngine-end" )
    return endBad();
  m_rng = NC::RandXRSRImpl( state );

  return getState(is);
}

