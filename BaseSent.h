#pragma once

#include <Arduino.h>

#include "crc4.hpp"

const uint8_t BUFFER_SIZE{32};
const uint16_t LOOKUP_SIZE{256};
const uint8_t LOOKUP_DIV{4};


class SentBuffer{
    public:
        SentBuffer()
            : _buffer_write_pointer(0)
            , _buffer_read_pointer(0)
            , _error(false)
        {}

        bool available(){
            return _buffer_write_pointer != _buffer_read_pointer;
        }

        uint16_t read() {
            while (!available()) {
                // wait for data
            }
            uint16_t ret = _buffer[_buffer_read_pointer];
            auto new_p = _buffer_read_pointer + 1;
            if (new_p >= BUFFER_SIZE) {
                new_p = 0;
            }

            _buffer_read_pointer = new_p;

            return ret;
        }

        void write(uint16_t value){
            _buffer[_buffer_write_pointer] = value;
            _buffer_write_pointer++;
            if (_buffer_write_pointer >= BUFFER_SIZE) {
                _buffer_write_pointer = 0;
            }
            if (_buffer_write_pointer == _buffer_read_pointer) {
                _error = true;
            }
        }

        bool isError(){
            return _error;
        }

        void resetError(){
            _error = false;
        }

    private:
        uint16_t _buffer[BUFFER_SIZE];
        uint8_t _buffer_write_pointer;
        uint8_t _buffer_read_pointer;

        bool _error;
};

using SentFrame = uint8_t[8];
typedef void(*SentCallback)(SentFrame&);

enum class SentError:uint8_t{
    SyncError,
    CrcError,
    NibbleError,
    OverflowError,
    ConfigurationError
};

class BaseSent{
    public:
        BaseSent(uint8_t tick_time, bool padding, SentBuffer &buffer)
            : _state(State::sync)
            , _tick_time(tick_time)
            , _lastValue(0)
            , _buffer(buffer)
            , _frameIdx(0)
            , _padding(padding)
        {}

        virtual void begin(SentCallback callback){
            _callback = callback;
        }

        void update(){
            if (!_buffer.available())
                return;

            if (_buffer.isError()){
                onError(SentError::OverflowError);
                _buffer.resetError();
                _lastValue = _buffer.read();
                _state = State::sync;
            }

            while(_buffer.available()){
                uint16_t tmp = _buffer.read();
                uint16_t dx = tmp - _lastValue;
                _lastValue = tmp;

                switch(_state){
                    case State::sync:
                        if((_cycl_syn_min <= dx) && (dx <= _cycl_syn_max)){
                            _state = State::data;
                            _frameIdx = 0;
                            _crc.start();
                        } else {
                            onError(SentError::SyncError);
                        }
                        break;
                    case State::data:
                        dx = (dx - _cycl_offset) / LOOKUP_DIV;
                        if (dx > LOOKUP_SIZE || _cycl_lookup[dx] < 0) {
                            onError(SentError::NibbleError);
                            _state = State::sync;
                        } else {
                            auto nibble = _cycl_lookup[dx];
                            _frame[_frameIdx] = nibble;
                            if(_frameIdx>0 && _frameIdx<7){
                                _crc.update4(nibble);
                            }
                            if(_frameIdx == 7){
                                auto crcOk = _crc.finish() == nibble;
                                if(!crcOk) {
                                    onError(SentError::CrcError);
                                    _state = State::sync;
                                }
                            }
                            _frameIdx++;
                            if(_frameIdx >= 8){
                                (*_callback)(_frame);
                                if(_padding){
                                    _state = State::padding;
                                } else {
                                    _state = State::sync;
                                }
                            }
                        }
                        break;
                    case State::padding:
                        _state = State::sync;
                        break;
                }
            }
        }

        virtual void onError(SentError error){}

    protected:
        void updateLut(uint64_t f_timer){
            _cycl_tick = f_timer / 1000000 * _tick_time; // CPU cycles per tick

            // Allow +/- 20% difference
            _cycl_syn_min = _cycl_tick * _tick_syn * 4 / 5;
            _cycl_syn_max = _cycl_tick * _tick_syn * 6 / 5;

            _cycl_syn = _cycl_tick * _tick_syn;
            _cycl_offset = _cycl_syn * (_tick_offest * 2 - 1) / _tick_syn / 2;

            for (uint16_t c = 0; c < LOOKUP_SIZE; c++) {
                int8_t value = c * _tick_syn * LOOKUP_DIV / _cycl_syn;
                if ( value >= 16) {
                    value = -1;
                }
                _cycl_lookup[c] = value;
            }
        }

    private:
        enum class State{
            sync,
            data,
            padding
        };
        State _state;

        const uint8_t _tick_syn{56};
        const uint8_t _tick_offest{12};

        uint8_t _tick_time;

        int8_t _cycl_lookup[LOOKUP_SIZE];

        uint16_t _lastValue;
        SentBuffer& _buffer;
        uint8_t _frameIdx;
        bool _padding;
        SentFrame _frame;
        SentCallback _callback;
        Crc4Sent _crc;
        
        uint16_t _cycl_tick;
        uint16_t _cycl_syn_min;
        uint16_t _cycl_syn_max;
        uint16_t _cycl_syn;
        uint16_t _cycl_offset;

};
