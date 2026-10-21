// <location> on iOS: CoreLocation. location_start asks for permission when in use the first
// time; positions and failures go to the "location" channel (src/location.esk).
#import <CoreLocation/CoreLocation.h>
#import "ui_native.h"

@interface EskiuLocation : NSObject <CLLocationManagerDelegate>
@property(nonatomic, strong) CLLocationManager *manager;
@property(nonatomic) BOOL running;
@end

static EskiuLocation *shared;

@implementation EskiuLocation
// Called when the manager is made and when the user answers: start once it is allowed.
- (void)locationManagerDidChangeAuthorization:(CLLocationManager *)manager {
    if (!self.running) return;
    CLAuthorizationStatus status = manager.authorizationStatus;
    if (status == kCLAuthorizationStatusDenied || status == kCLAuthorizationStatusRestricted) {
        ui_native_emit("location", "denied");
    } else if (status == kCLAuthorizationStatusAuthorizedWhenInUse || status == kCLAuthorizationStatusAuthorizedAlways) {
        [manager startUpdatingLocation];
    }
}
- (void)locationManager:(CLLocationManager *)manager didUpdateLocations:(NSArray<CLLocation *> *)locations {
    CLLocation *fix = locations.lastObject;
    if (!fix || !self.running) return;
    char altitude[32];
    if (fix.verticalAccuracy >= 0) snprintf(altitude, sizeof altitude, "%.2f", fix.altitude);
    else snprintf(altitude, sizeof altitude, "-");
    char payload[256];
    snprintf(payload, sizeof payload, "fix %.7f %.7f %.2f %s %.2f %.2f %lld", fix.coordinate.latitude,
             fix.coordinate.longitude, fix.horizontalAccuracy, altitude, fix.speed >= 0 ? fix.speed : -1.0,
             fix.course >= 0 ? fix.course : -1.0, (long long)(fix.timestamp.timeIntervalSince1970 * 1000));
    ui_native_emit("location", payload);
}
- (void)locationManager:(CLLocationManager *)manager didFailWithError:(NSError *)error {
    if (!self.running) return;
    // kCLErrorLocationUnknown is temporary: the manager keeps trying.
    if (error.code == kCLErrorDenied) ui_native_emit("location", "denied");
    else if (error.code != kCLErrorLocationUnknown) ui_native_emit("location", "unavailable");
}
@end

void location_start(int precise) {
    if (!shared) {
        shared = [EskiuLocation new];
        shared.manager = [CLLocationManager new];
    }
    shared.running = YES;
    shared.manager.desiredAccuracy = precise ? kCLLocationAccuracyBest : kCLLocationAccuracyHundredMeters;
    if (!CLLocationManager.locationServicesEnabled) {
        ui_native_emit("location", "unavailable");
        return;
    }
    if (shared.manager.authorizationStatus == kCLAuthorizationStatusNotDetermined) {
        shared.manager.delegate = shared;
        [shared.manager requestWhenInUseAuthorization];
        return;
    }
    // Setting the delegate reports the authorization, which starts the updates or says denied.
    if (shared.manager.delegate) [shared locationManagerDidChangeAuthorization:shared.manager];
    else shared.manager.delegate = shared;
}

void location_stop(void) {
    if (!shared) return;
    shared.running = NO;
    [shared.manager stopUpdatingLocation];
}
