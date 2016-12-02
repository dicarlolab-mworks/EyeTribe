//
//  EyeTribePlugin.cpp
//  EyeTribe
//
//  Created by Christopher Stawarz on 12/2/16.
//  Copyright © 2016 The MWorks Project. All rights reserved.
//

#include "EyeTribeDevice.hpp"
#include "EyeTribeActions.hpp"


BEGIN_NAMESPACE_MW


class EyeTribePlugin : public Plugin {
    void registerComponents(boost::shared_ptr<ComponentRegistry> registry) override {
        registry->registerFactory<StandardComponentFactory, EyeTribeDevice>();
        registry->registerFactory<StandardComponentFactory, EyeTribeCalibrationStartAction>();
        registry->registerFactory<StandardComponentFactory, EyeTribeCalibrationPointStartAction>();
        registry->registerFactory<StandardComponentFactory, EyeTribeCalibrationPointEndAction>();
    }
};


extern "C" Plugin* getPlugin() {
    return new EyeTribePlugin();
}


END_NAMESPACE_MW
