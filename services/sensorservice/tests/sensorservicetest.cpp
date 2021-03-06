/*
 * Copyright (C) 2010 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <android/sensor.h>
#include <gui/Sensor.h>
#include <gui/SensorManager.h>
#include <gui/SensorEventQueue.h>
#include <utils/Looper.h>

using namespace android;

int receiver(int fd, int events, void* data)
{
    sp<SensorEventQueue> q((SensorEventQueue*)data);
    ssize_t n;
    ASensorEvent buffer[8];

    static nsecs_t oldTimeStamp = 0;

    while ((n = q->read(buffer, 8)) > 0) {
        for (int i=0 ; i<n ; i++) {
            if (buffer[i].type == Sensor::TYPE_GYROSCOPE) {
                printf("time=%lld, value=<%5.1f,%5.1f,%5.1f>\n",
                        buffer[i].timestamp,
                        buffer[i].acceleration.x,
                        buffer[i].acceleration.y,
                        buffer[i].acceleration.z);
            }

            if (oldTimeStamp) {
                float t = float(buffer[i].timestamp - oldTimeStamp) / s2ns(1);
                printf("%f ms (%f Hz)\n", t*1000, 1.0/t);
            }
            oldTimeStamp = buffer[i].timestamp;

        }
    }
    if (n<0 && n != -EAGAIN) {
        printf("error reading events (%s)\n", strerror(-n));
    }
    return 1;
}


int main(int argc, char** argv)
{
    SensorManager& mgr(SensorManager::getInstance());

    Sensor const* const* list;
    ssize_t count = mgr.getSensorList(&list);
    printf("numSensors=%d\n", int(count));

    sp<SensorEventQueue> q = mgr.createEventQueue();
    printf("queue=%p\n", q.get());

    Sensor const* accelerometer = mgr.getDefaultSensor(Sensor::TYPE_GYROSCOPE);
    printf("accelerometer=%p (%s)\n",
            accelerometer, accelerometer->getName().string());
    q->enableSensor(accelerometer);

    q->setEventRate(accelerometer, ms2ns(10));

    sp<Looper> loop = new Looper(false);
    loop->addFd(q->getFd(), 0, ALOOPER_EVENT_INPUT, receiver, q.get());

    do {
        //printf("about to poll...\n");
        int32_t ret = loop->pollOnce(-1);
        switch (ret) {
            case ALOOPER_POLL_WAKE:
                //("ALOOPER_POLL_WAKE\n");
                break;
            case ALOOPER_POLL_CALLBACK:
                //("ALOOPER_POLL_CALLBACK\n");
                break;
            case ALOOPER_POLL_TIMEOUT:
                printf("ALOOPER_POLL_TIMEOUT\n");
                break;
            case ALOOPER_POLL_ERROR:
                printf("ALOOPER_POLL_TIMEOUT\n");
                break;
            default:
                printf("ugh? poll returned %d\n", ret);
                break;
        }
    } while (1);


    return 0;
}
