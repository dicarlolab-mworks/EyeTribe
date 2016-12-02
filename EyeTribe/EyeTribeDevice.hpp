//
//  EyeTribeDevice.hpp
//  EyeTribe
//
//  Created by Christopher Stawarz on 12/2/16.
//  Copyright © 2016 The MWorks Project. All rights reserved.
//

#ifndef EyeTribeDevice_hpp
#define EyeTribeDevice_hpp

#include "gazeapi.h"


BEGIN_NAMESPACE_MW


class EyeTribeDevice : public IODevice,
                       public gtl::ITrackerStateListener,
                       public gtl::ICalibrationProcessHandler,
                       public gtl::IGazeListener
{
    
public:
    static const std::string GAZE_X;
    static const std::string GAZE_Y;
    static const std::string PORT;
    
    static void describeComponent(ComponentInfo &info);
    
    explicit EyeTribeDevice(const ParameterValueMap &parameters);
    ~EyeTribeDevice();
    
    bool initialize() override;
    bool startDeviceIO() override;
    bool stopDeviceIO() override;
    
    void startCalibration(int numPoints);
    void startCalibrationPoint(double xDeg, double yDeg);
    void endCalibrationPoint();
    
private:
    bool spawnTrackerServer();
    void reapTrackerServer();
    
    // ITrackerStateListener methods
    void on_tracker_connection_changed(int trackerState) override;
    void on_screen_state_changed(const gtl::Screen &screen) override { }
    
    // ICalibrationProcessHandler methods
    void on_calibration_started() override { }
    void on_calibration_progress(double progress) override { }
    void on_calibration_processing() override { }
    void on_calibration_result(bool isCalibrated, const gtl::CalibResult &calibResult) override;
    
    // IGazeListener methods
    void on_gaze_data(const gtl::GazeData &gazeData) override;
    
    const VariablePtr gazeX;
    const VariablePtr gazeY;
    const int port;
    
    pid_t trackerServerPID;
    gtl::GazeApi gazeApi;
    
    struct {
        double width;
        double height;
    } displaySizePixels;
    
    struct {
        struct {
            double x;
            double y;
        } origin;
        double width;
        double height;
    } displayRectDegrees;
    
    using lock_guard = std::lock_guard<std::mutex>;
    lock_guard::mutex_type mutex;
    
    bool running;
    
};


END_NAMESPACE_MW


#endif /* EyeTribeDevice_hpp */



























