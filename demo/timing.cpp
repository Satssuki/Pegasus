/*
* Copyright (c) Ian Millington 2003-2006. All Rights Reserved.
*
* This software is distributed under licence. Use of this software
* implies agreement with all terms and conditions of the accompanying
* software licence.
*/
#include "demo/timing.hpp"

static bool qpcFlag;

#ifndef WIN32
#define TIMING_UNIX 1

#include <stdlib.h>
#include <sys/time.h>

// assume unix based OS
typedef unsigned long long LONGLONG;
#else
#define TIMING_WINDOWS 1
#include <windows.h>

#ifdef TIMING_WINDOWS
#include <intrin.h>
#include <mmsystem.h>
#endif

static double qpcFrequency;
#endif

// Internal time and clock access functions
unsigned systemTime()
{
#if TIMING_UNIX
    struct timeval tv;
    gettimeofday(&tv, 0);

    return tv.tv_sec * 1000 + tv.tv_usec / 1000;

#else
    if (qpcFlag) {
        static LONGLONG qpcMillisPerTick;
        QueryPerformanceCounter(reinterpret_cast<LARGE_INTEGER*>(&qpcMillisPerTick));
        return static_cast<unsigned>(qpcMillisPerTick * qpcFrequency);
    } else {
        return unsigned(timeGetTime());
    }
#endif
}

unsigned TimingData::getTime() { return systemTime(); }

#if TIMING_WINDOWS
unsigned long systemClock()
{
    return static_cast<unsigned long>(__rdtsc());
}
#endif

unsigned long TimingData::getClock()
{

#if TIMING_UNIX
    struct timeval tv;
    gettimeofday(&tv, 0);

    return tv.tv_sec * 1000 + tv.tv_usec / 1000;
#else
    return systemClock();
#endif
}

// Sets up the timing system and registers the performance timer.
void initTime()
{
#if TIMING_UNIX
    qpcFlag = false;
#else
    LONGLONG time;

    qpcFlag = (QueryPerformanceFrequency(reinterpret_cast<LARGE_INTEGER*>(&time)) > 0);

    // Check if we have access to the performance counter at this
    // resolution.
    if (qpcFlag)
        qpcFrequency = 1000.0 / time;
#endif
}

// Holds the global frame time that is passed around
static TimingData* timingData = nullptr;

// Retrieves the global frame info instance
TimingData& TimingData::get() { return static_cast<TimingData&>(*timingData); }

// Updates the global frame information. Should be called once per frame.
void TimingData::update()
{
    if (!timingData)
        return;

    // Advance the frame number.
    if (!timingData->isPaused) {
        timingData->frameNumber++;
    }

    // Update the timing information.
    unsigned thisTime = systemTime();
    timingData->lastFrameDuration = thisTime - timingData->lastFrameTimestamp;
    timingData->lastFrameTimestamp = thisTime;

    // Update the tick information.
    unsigned long thisClock = getClock();
    timingData->lastFrameClockTicks = thisClock - timingData->lastFrameClockstamp;
    timingData->lastFrameClockstamp = thisClock;

    // Update the RWA frame rate if we are able to.
    if (timingData->frameNumber > 1) {
        if (timingData->averageFrameDuration <= 0) {
            timingData->averageFrameDuration = static_cast<double>(timingData->lastFrameDuration);
        } else {
            // RWA over 100 frames.
            timingData->averageFrameDuration *= 0.99;
            timingData->averageFrameDuration += 0.01 * static_cast<double>(timingData->lastFrameDuration);

            // Invert to get FPS
            timingData->fps = static_cast<float>(1000.0 / timingData->averageFrameDuration);
        }
    }
}

void TimingData::init()
{
    // Set up the timing system.
    initTime();

    // Create the frame info object
    if (!timingData)
        timingData = new TimingData();

    // Set up the frame info structure.
    timingData->frameNumber = 0;

    timingData->lastFrameTimestamp = systemTime();
    timingData->lastFrameDuration = 0;

    timingData->lastFrameClockstamp = getClock();
    timingData->lastFrameClockTicks = 0;

    timingData->isPaused = false;

    timingData->averageFrameDuration = 0;
    timingData->fps = 0;
}

void TimingData::deinit()
{
    delete timingData;
    timingData = nullptr;
}
