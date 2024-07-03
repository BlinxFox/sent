#pragma once

#include "sent/BaseSent.h"

typedef void(*MLX90372Callback)(uint16_t);

class MLX90372:ISentFrameHandler{
    public:
        MLX90372(BaseSent &sent)
            : _sent(&sent)
            , _callback(nullptr)
        { }

        void begin(MLX90372Callback callback){
            _callback = callback;
            _sent->registerHandler(this);
        }

        void onFrame(BaseSent *sender, SentFrame &frame) override {
            (void) sender;

            uint16_t value = (frame[1] << 8) | (frame[2] << 4) | frame[3];

            if(_callback){
                (*_callback)(value);
            }
        }

    private:
        BaseSent *_sent;
        MLX90372Callback _callback;
};
