//
//  EyeTribeDevice.cpp
//  EyeTribe
//
//  Created by Christopher Stawarz on 12/2/16.
//  Copyright © 2016 The MWorks Project. All rights reserved.
//

#include "EyeTribeDevice.hpp"

#define EYE_TRIBE_TRACKER_SERVER "Eye Tribe Tracker Server"


BEGIN_NAMESPACE_MW


const std::string EyeTribeDevice::GAZE_X("gaze_x");
const std::string EyeTribeDevice::GAZE_Y("gaze_y");
const std::string EyeTribeDevice::PORT("port");


void EyeTribeDevice::describeComponent(ComponentInfo &info) {
    IODevice::describeComponent(info);
    
    info.setSignature("iodevice/eyetribe");
    
    info.addParameter(GAZE_X);
    info.addParameter(GAZE_Y);
    info.addParameter(PORT, "6555");
}


EyeTribeDevice::EyeTribeDevice(const ParameterValueMap &parameters) :
    IODevice(parameters),
    gazeX(parameters[GAZE_X]),
    gazeY(parameters[GAZE_Y]),
    port(parameters[PORT]),
    trackerServerPID(-1),
    running(false)
{
    gazeApi.add_listener(static_cast<gtl::ITrackerStateListener &>(*this));
}


EyeTribeDevice::~EyeTribeDevice() {
    gazeApi.remove_listener(static_cast<gtl::ITrackerStateListener &>(*this));
    gazeApi.disconnect();
    
    // Give server time to shut things down
    Clock::instance()->sleepMS(1000);
    
    reapTrackerServer();
}


bool EyeTribeDevice::initialize() {
    lock_guard lock(mutex);
    
    if (!spawnTrackerServer()) {
        return false;
    }
    
    if (!gazeApi.connect(port)) {
        merror(M_IODEVICE_MESSAGE_DOMAIN, "Cannot connect to %s", EYE_TRIBE_TRACKER_SERVER);
        return false;
    }
    
    auto display = StimulusDisplay::getCurrentStimulusDisplay();
    
    GLint width, height;
    {
        OpenGLContextLock ctxLock = display->setCurrent(0);
        display->getCurrentViewportSize(width, height);
    }
    displaySizePixels = { double(width), double(height) };
    
    GLdouble xMin, xMax, yMin, yMax;
    display->getDisplayBounds(xMin, xMax, yMin, yMax);
    displayRectDegrees = { { xMin, yMin }, xMax - xMin, yMax - yMin };
    
    return true;
}


bool EyeTribeDevice::startDeviceIO() {
    lock_guard lock(mutex);
    
    if (!running) {
        gazeApi.add_listener(static_cast<gtl::ICalibrationProcessHandler &>(*this));
        gazeApi.add_listener(static_cast<gtl::IGazeListener &>(*this));
        running = true;
    }
    
    return true;
}


bool EyeTribeDevice::stopDeviceIO() {
    lock_guard lock(mutex);
    
    if (running) {
        gazeApi.remove_listener(static_cast<gtl::IGazeListener &>(*this));
        gazeApi.remove_listener(static_cast<gtl::ICalibrationProcessHandler &>(*this));
        running = false;
    }
    
    return true;
}


void EyeTribeDevice::startCalibration(int numPoints) {
    lock_guard lock(mutex);
    
    // Abort prior incomplete calibration, if any
    gazeApi.calibration_abort();
    
    if (!gazeApi.calibration_start(numPoints)) {
        merror(M_IODEVICE_MESSAGE_DOMAIN, "Cannot start Eye Tribe calibration");
    }
}


void EyeTribeDevice::startCalibrationPoint(double xDeg, double yDeg) {
    lock_guard lock(mutex);
    
    int x = std::round(displaySizePixels.width *
                       ((xDeg - displayRectDegrees.origin.x) / displayRectDegrees.width));
    int y = std::round(displaySizePixels.height *
                       (1.0 - (yDeg - displayRectDegrees.origin.y) / displayRectDegrees.height));
    
    if (!gazeApi.calibration_point_start(x, y)) {
        merror(M_IODEVICE_MESSAGE_DOMAIN, "Cannot start Eye Tribe calibration point");
    }
}


void EyeTribeDevice::endCalibrationPoint() {
    lock_guard lock(mutex);
    gazeApi.calibration_point_end();
}


bool EyeTribeDevice::spawnTrackerServer() {
    const char * const argv[] = { "EyeTribe", 0 };
    
    int status = posix_spawn(&trackerServerPID,
                             "/Applications/EyeTribe/EyeTribe",
                             nullptr,
                             nullptr,
                             const_cast<char * const *>(argv),
                             *_NSGetEnviron());
    
    if (status != 0) {
        merror(M_IODEVICE_MESSAGE_DOMAIN, "Cannot launch %s: %s", EYE_TRIBE_TRACKER_SERVER, std::strerror(status));
        return false;
    }
    
    // Give server some time to start up
    Clock::instance()->sleepMS(1000);
    
    //
    // Verify that the server has not exited
    //
    
    pid_t pid = waitpid(trackerServerPID, &status, WNOHANG | WUNTRACED);
    
    if (-1 == pid) {
        merror(M_IODEVICE_MESSAGE_DOMAIN,
               "Error while checking status of %s: %s",
               EYE_TRIBE_TRACKER_SERVER,
               std::strerror(errno));
    } else if (pid != 0) {
        trackerServerPID = -1;
        const char *msg;
        
        if (WSTOPSIG(status)) {
            msg = "was stopped by a signal";
        } else if (WIFSIGNALED(status)) {
            msg = "terminated due to a signal";
        } else {
            msg = "exited prematurely";
        }
        
        merror(M_IODEVICE_MESSAGE_DOMAIN, "%s %s", EYE_TRIBE_TRACKER_SERVER, msg);
        return false;
    }
    
    return true;
}


void EyeTribeDevice::reapTrackerServer() {
    if (trackerServerPID <= 0) {
        return;
    }
    
    int status = kill(trackerServerPID, SIGTERM);
    if (-1 == status) {
        merror(M_IODEVICE_MESSAGE_DOMAIN, "Cannot terminate %s: %s", EYE_TRIBE_TRACKER_SERVER, std::strerror(status));
        return;
    }
    
    if (-1 == waitpid(trackerServerPID, &status, WUNTRACED)) {
        merror(M_IODEVICE_MESSAGE_DOMAIN,
               "Error while waiting for %s to exit: %s",
               EYE_TRIBE_TRACKER_SERVER,
               std::strerror(errno));
        return;
    }
    
    if (!WIFSIGNALED(status) ||
        WTERMSIG(status) != SIGTERM)
    {
        mwarning(M_IODEVICE_MESSAGE_DOMAIN, "%s terminated abnormally", EYE_TRIBE_TRACKER_SERVER);
    }
}


void EyeTribeDevice::on_tracker_connection_changed(int trackerState) {
    // Do NOT acquire lock here, as doing so is unnecessary and will lead to deadlock
    
    const char *error = nullptr;
    
    switch (trackerState) {
        case gtl::ServerState::TRACKER_CONNECTED:
            break;
            
        case gtl::ServerState::TRACKER_NOT_CONNECTED:
            error = "is not connected";
            break;
            
        case gtl::ServerState::TRACKER_CONNECTED_BADFW:
            error = "has incorrect or unsupported firmware";
            break;
            
        case gtl::ServerState::TRACKER_CONNECTED_NOUSB3:
            error = "is not connected via USB 3";
            break;
            
        case gtl::ServerState::TRACKER_CONNECTED_NOSTREAM:
            error = "is not streaming data";
            break;
            
        default:
            error = "returned an unexpected status code";
            break;
    }
    
    if (error) {
        merror(M_IODEVICE_MESSAGE_DOMAIN, "Eye Tribe tracker %s", error);
    }
}


void EyeTribeDevice::on_calibration_result(bool isCalibrated, const gtl::CalibResult &calibResult) {
    lock_guard lock(mutex);
    
    if (!isCalibrated) {
        merror(M_IODEVICE_MESSAGE_DOMAIN, "Eye Tribe calibration failed");
    } else {
        mprintf(M_IODEVICE_MESSAGE_DOMAIN,
                "Eye Tribe calibration succeeded; average error is %g degrees",
                calibResult.deg);
    }
}


void EyeTribeDevice::on_gaze_data(const gtl::GazeData &gazeData) {
    lock_guard lock(mutex);
    
    if (gazeData.state & gtl::GazeData::GD_STATE_TRACKING_GAZE) {
        auto &coords = gazeData.avg;
        
        double xDeg = (displayRectDegrees.origin.x +
                       displayRectDegrees.width * (double(coords.x) / displaySizePixels.width));
        double yDeg = (displayRectDegrees.origin.y +
                       displayRectDegrees.height * (1.0 - double(coords.y) / displaySizePixels.height));
        
        gazeX->setValue(xDeg);
        gazeY->setValue(yDeg);
    }
}


END_NAMESPACE_MW



























