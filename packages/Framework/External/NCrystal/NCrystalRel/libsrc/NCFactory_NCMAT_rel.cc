////////////////////////////////////////////////////////////////////////////////
//                                                                            //
//  This file is part of NCrystal (see https://mctools.github.io/ncrystal/)   //
//                                                                            //
//  Copyright 2015-2022 NCrystal developers                                   //
//                                                                            //
//  Licensed under the Apache License, Version 2.0 (the "License");           //
//  you may not use this file except in compliance with the License.          //
//  You may obtain a copy of the License at                                   //
//                                                                            //
//      http://www.apache.org/licenses/LICENSE-2.0                            //
//                                                                            //
//  Unless required by applicable law or agreed to in writing, software       //
//  distributed under the License is distributed on an "AS IS" BASIS,         //
//  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.  //
//  See the License for the specific language governing permissions and       //
//  limitations under the License.                                            //
//                                                                            //
////////////////////////////////////////////////////////////////////////////////

#include "NCrystalRel/NCFactImpl.hh"
#include "NCrystalRel/NCDataSources.hh"
#include "NCrystalRel/NCLoadNCMAT.hh"
namespace NC = NCrystalRel;

namespace NCrystalRel {

  class NCMATFactory final : public FactImpl::InfoFactory {
  public:
    const char * name() const noexcept final { return "stdncmat"; }

    Priority query( const FactImpl::InfoRequest& cfg ) const final
    {
      return cfg.getDataType()=="ncmat" ? Priority{100} : Priority{Priority::Unable};
    }

    shared_obj<const Info> produce( const FactImpl::InfoRequest& cfg ) const final
    {
      return makeSO<const Info>( loadNCMAT(cfg) );
    }
  };

}

//Finally, a function which can be used to enable the above factory. Note that
//this function is forward declared elsewhere or might be dynamically invoked
//(hence the C-mangling), and its name should not be changed just here:

extern "C" void ncrystalrel_register_stdncmat_factory()
{
  NC::FactImpl::registerFactory( std::make_unique<NC::NCMATFactory>(),
                                 NC::FactImpl::RegPolicy::IGNORE_IF_EXISTS );
  NC::DataSources::addRecognisedFileExtensions("ncmat");
}
