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

std::vector<unsigned long> NCG4RngEngine::put () const
{
  static_assert( sizeof(uint64_t) == sizeof(unsigned long)
                 || sizeof(uint32_t) == sizeof(unsigned long), "" );
  static_assert( sizeof(m_rng.state()[0]) == sizeof(uint64_t) );

  std::vector<unsigned long> v;
  assert(m_rng.state().size()==2);
  if ( sizeof(unsigned long) == sizeof(uint64_t) ) {
    v.push_back( static_cast<unsigned long>(m_rng.state()[0]) );
    v.push_back( static_cast<unsigned long>(m_rng.state()[1]) );
  } else {
    //pack 64 bit ints into double the number of 32 bit ints:
    uint64_t x = m_rng.state()[0];
    uint64_t y = m_rng.state()[1];
    uint32_t a = (uint32_t)((x & 0xFFFFFFFF00000000LL) >> 32);
    uint32_t b = (uint32_t)(x & 0xFFFFFFFFLL);
    uint32_t c = (uint32_t)((y & 0xFFFFFFFF00000000LL) >> 32);
    uint32_t d = (uint32_t)(y & 0xFFFFFFFFLL);
    v.push_back(static_cast<unsigned long>(a));
    v.push_back(static_cast<unsigned long>(b));
    v.push_back(static_cast<unsigned long>(c));
    v.push_back(static_cast<unsigned long>(d));
  }
  return v;
}

bool NCG4RngEngine::get( const std::vector<unsigned long> & v )
{
  static_assert( sizeof(uint64_t) == sizeof(unsigned long)
                 || sizeof(uint32_t) == sizeof(unsigned long), "" );
  static_assert( sizeof(m_rng.state()[0]) == sizeof(uint64_t) );

  constexpr std::size_t vsize = ( sizeof(unsigned long) == sizeof(uint64_t) ? 2 : 4 );

  if ( v.size() != vsize ) {
    std::cerr << "  -- Invalid state for NCG4RngEngine received. Engine state remains unchanged.\n";
    return false;
  }

  NC::RandXRSRImpl::state_t state;

  if ( sizeof(unsigned long) == sizeof(m_rng.state()[0]) ) {
    assert ( v.size() == 2 );
    state[0] = static_cast<uint64_t>(v[0]);
    state[1] = static_cast<uint64_t>(v[1]);
  } else {
    assert ( v.size() == 4 );
    uint32_t a = static_cast<uint32_t>( v[0] );
    uint32_t b = static_cast<uint32_t>( v[1] );
    uint32_t c = static_cast<uint32_t>( v[2] );
    uint32_t d = static_cast<uint32_t>( v[3] );
    uint64_t x = ((uint64_t)a) << 32 | b;
    uint64_t y = ((uint64_t)c) << 32 | d;
    state[0] = x;
    state[1] = y;
  }
  m_rng = NC::RandXRSRImpl( state );
  return true;
}

bool NCG4RngEngine::getState (const std::vector<unsigned long> & v)
{
  return this->get(v);
}

std::istream & NCG4RngEngine::getState ( std::istream & is )
{
  return this->get(is);
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

  return is;
  //  return getState(is);
}

