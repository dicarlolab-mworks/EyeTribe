//
//  EyeTribeActions.hpp
//  EyeTribe
//
//  Created by Christopher Stawarz on 12/9/16.
//  Copyright © 2016 The MWorks Project. All rights reserved.
//

#ifndef EyeTribeActions_hpp
#define EyeTribeActions_hpp

#include "EyeTribeDevice.hpp"


BEGIN_NAMESPACE_MW


class EyeTribeAction : public Action {
    
public:
    static const std::string DEVICE;
    
    static void describeComponent(ComponentInfo &info);
    
    explicit EyeTribeAction(const ParameterValueMap &parameters);
    
protected:
    const boost::shared_ptr<EyeTribeDevice> device;
    
};


class EyeTribeCalibrationStartAction : public EyeTribeAction {
    
public:
    static const std::string NUM_POINTS;
    
    static void describeComponent(ComponentInfo &info);
    
    explicit EyeTribeCalibrationStartAction(const ParameterValueMap &parameters);
    
    bool execute() override;
    
private:
    const VariablePtr numPoints;
    
};


class EyeTribeCalibrationPointStartAction : public EyeTribeAction {
    
public:
    static const std::string X;
    static const std::string Y;
    
    static void describeComponent(ComponentInfo &info);
    
    explicit EyeTribeCalibrationPointStartAction(const ParameterValueMap &parameters);
    
    bool execute() override;
    
private:
    const VariablePtr x;
    const VariablePtr y;
    
};


class EyeTribeCalibrationPointEndAction : public EyeTribeAction {
    
public:
    static void describeComponent(ComponentInfo &info);
    
    using EyeTribeAction::EyeTribeAction;
    
    bool execute() override;
    
};


END_NAMESPACE_MW


#endif /* EyeTribeActions_hpp */




























