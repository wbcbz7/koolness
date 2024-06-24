#pragma once
// simple class-encapsulated librocket player implementation
// --wbcbz7 o4.lo.zol8

// define this to disable networking (always defined for dos)
//#define SYNC_PLAYER

#include "sync.h"
#include "track.h"
#include "base.h"

#include <vector>
#include <map>

class Rocket {
    public:
        Rocket(float bpm, float rpb) {rps = bpm / 60.0f * rpb;};
#ifndef SYNC_PLAYER
        Rocket(float bpm, float rpb, sync_cb &_cb) {cb = _cb; rps = bpm / 60.0f * rpb;};
#endif
        ~Rocket();
        
        // local connect
        bool open(char *prefix);
        
        // remote connect
        bool open(char *prefix, char *address);
    
        // check if remote
        bool isRemote();
    
        // remote update
        bool update(double t);
        
        // load track for use
        int loadTrack(char *name);
        
        // get track data
        double getTrack(int handle, double t);
    
        // get track data
        double getTrack(char *name, double t);
    
    private:
        bool        remote;
        float       rps;
        
        char        *addr;
        sync_device *device;
#ifndef SYNC_PLAYER
        sync_cb      cb;
#endif
        
        // track map        
        std::map<unsigned long, sync_track*> trackmap;
};
