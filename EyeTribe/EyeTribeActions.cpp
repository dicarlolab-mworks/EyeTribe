//
//  EyeTribeActions.cpp
//  EyeTribe
//
//  Created by Christopher Stawarz on 12/9/16.
//  Copyright © 2016 The MWorks Project. All rights reserved.
//

#include "EyeTribeActions.hpp"


BEGIN_NAMESPACE_MW


const std::string EyeTribeAction::DEVICE("device");


void EyeTribeAction::describeComponent(ComponentInfo &info) {
    Action::describeComponent(info);
    info.addParameter(DEVICE);
}


EyeTribeAction::EyeTribeAction(const ParameterValueMap &parameters) :
    Action(parameters),
    device(parameters[DEVICE].getRegistry()->getObject<EyeTribeDevice>(parameters[DEVICE].str()))
{
    if (!device) {
        throw SimpleException(M_IODEVICE_MESSAGE_DOMAIN,
                              "Device is not an Eye Tribe tracker",
                              parameters[DEVICE].str());
    }
}


const std::string EyeTribeCalibrationStartAction::NUM_POINTS("num_points");


void EyeTribeCalibrationStartAction::describeComponent(ComponentInfo &info) {
    EyeTribeAction::describeComponent(info);
    
    info.setSignature("action/eyetribe_calibration_start");
    
    info.addParameter(NUM_POINTS);
}


EyeTribeCalibrationStartAction::EyeTribeCalibrationStartAction(const ParameterValueMap &parameters) :
    EyeTribeAction(parameters),
    numPoints(parameters[NUM_POINTS])
{ }


bool EyeTribeCalibrationStartAction::execute() {
    device->startCalibration(numPoints->getValue().getInteger());
    return true;
}


const std::string EyeTribeCalibrationPointStartAction::X("x");
const std::string EyeTribeCalibrationPointStartAction::Y("y");


void EyeTribeCalibrationPointStartAction::describeComponent(ComponentInfo &info) {
    EyeTribeAction::describeComponent(info);
    
    info.setSignature("action/eyetribe_calibration_point_start");
    
    info.addParameter(X);
    info.addParameter(Y);
}


EyeTribeCalibrationPointStartAction::EyeTribeCalibrationPointStartAction(const ParameterValueMap &parameters) :
    EyeTribeAction(parameters),
    x(parameters[X]),
    y(parameters[Y])
{ }


bool EyeTribeCalibrationPointStartAction::execute() {
    device->startCalibrationPoint(x->getValue().getFloat(), y->getValue().getFloat());
    return true;
}


void EyeTribeCalibrationPointEndAction::describeComponent(ComponentInfo &info) {
    EyeTribeAction::describeComponent(info);
    
    info.setSignature("action/eyetribe_calibration_point_end");
}


bool EyeTribeCalibrationPointEndAction::execute() {
    device->endCalibrationPoint();
    return true;
}


END_NAMESPACE_MW



























