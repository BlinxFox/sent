
#include "crc4.hpp"

// Connect data pin of sensor to D8 of the Arduino Uno/Nano/Mini

// Time for one tick (in µs)
constexpr uint8_t tick_time = 3;

constexpr auto serial_baud = 230400;

/************************************************************/

constexpr uint8_t tick_syn = 56;
constexpr uint8_t tick_offest = 12;

constexpr uint16_t cycl_tick = F_CPU / 1000000 * tick_time; // CPU cycles per tick

// Allow +/- 20% difference
constexpr uint16_t cycl_syn_min = cycl_tick * tick_syn * 4 / 5;
constexpr uint16_t cycl_syn_max = cycl_tick * tick_syn * 6 / 5;

constexpr auto LOOKUP_SIZE = 256;
constexpr auto LOOKUP_DIV = 4;

uint16_t cycl_syn;
uint16_t cycl_offset;
int8_t cycl_lookup[LOOKUP_SIZE];

constexpr uint8_t BUFFER_SIZE = 32;

volatile uint16_t buffer[BUFFER_SIZE];
volatile uint8_t buffer_write_pointer;
volatile uint8_t buffer_read_pointer;

uint16_t last_icr = 0;
volatile bool error = false;
ISR(TIMER1_CAPT_vect) {
  uint16_t value = ICR1;
  buffer[buffer_write_pointer] = value - last_icr;
  last_icr = value;

  buffer_write_pointer++;
  if (buffer_write_pointer >= BUFFER_SIZE) {
    buffer_write_pointer = 0;
  }
  if (buffer_write_pointer == buffer_read_pointer) {
    error = true;
  }
}

bool buffer_available() {
  return buffer_write_pointer != buffer_read_pointer;
}

uint16_t buffer_read() {
  while (!buffer_available()) {
    // wait for data
  }
  uint16_t ret = buffer[buffer_read_pointer];
  auto new_p = buffer_read_pointer + 1;
  if (new_p >= BUFFER_SIZE) {
    new_p = 0;
  }

  uint8_t oldSREG = SREG;
  cli();
  buffer_read_pointer = new_p;
  SREG = oldSREG;

  return ret;
}

uint16_t get_syn_cycl() {
  while (true) {
    uint16_t dx = buffer_read();
    if (dx >= cycl_syn_min && dx <= cycl_syn_max) {
      return dx;
    }
  }
}

void wait_for_syn() {
  while (true) {
    uint16_t dx = buffer_read();
    auto t = dx > cycl_syn ? dx - cycl_syn : cycl_syn - dx;
    if ( t < 10) {
      return;
    }
  }
}

void setup() {
  Serial.begin(serial_baud);
  Serial.print("cycl_tick = ");
  Serial.println(cycl_tick);

  TCCR1A = 0;
  TCCR1B = 1;
  TCCR1C = 0;

  TIMSK1 = (1 << ICIE1);

  cycl_syn = get_syn_cycl();
  cycl_offset = cycl_syn * (tick_offest * 2 - 1) / tick_syn / 2; // cycl_tick * 11.5

  for (uint16_t c = 0; c < LOOKUP_SIZE; c++) {
    int8_t value = c * tick_syn * LOOKUP_DIV / cycl_syn;
    if ( value >= 16) {
      value = -1;
    }
    cycl_lookup[c] = value;
  }

  auto cycl_syn1 = get_syn_cycl();
  auto dx = cycl_syn > cycl_syn1 ? cycl_syn - cycl_syn1 : cycl_syn1 - cycl_syn;

  Serial.print("cycl_syn = ");
  Serial.println(cycl_syn);

  Serial.print("cycl_syn ok? ");
  Serial.println(dx < 10 ? "yes" : "no");

  Serial.print("cycl_offset = ");
  Serial.println(cycl_offset);

  if (LOOKUP_SIZE < (cycl_syn * 17 / tick_syn / LOOKUP_DIV)) {
    Serial.println("Lookup table would exceed expected size!");
    while (true) {
      ;
    }
  }

  Serial.println("Lookup table:");
  for (int x = 0; x < 16; x++) {
    Serial.print(x * 16, HEX);
    Serial.print(": ");
    for (int y = 0; y < 16; y++) {
      Serial.print(cycl_lookup[x * 16 + y], HEX);
      Serial.print(" ");
    }
    Serial.println("");
  }
  Serial.println("************************");
  Serial.flush();
  error = false;
}

Crc4Sent crc;

constexpr char toHex[] = "0123456789ABCDEF";
void loop() {
  constexpr auto FRAME_SIZE = 9; // 8 ( +1 for padding pulse)
  char frame[FRAME_SIZE];
  bool crcOk = false;

  wait_for_syn();
  crc.start();
  for (int c = 0; c < FRAME_SIZE; c++) {
    auto dx = buffer_read();
    dx = (dx - cycl_offset) / LOOKUP_DIV;
    if (dx > LOOKUP_SIZE || cycl_lookup[dx] < 0) {
      frame[c] = '!';
    } else {
      frame[c] = toHex[cycl_lookup[dx]];
      if(c>0 && c<7){
        crc.update(cycl_lookup[dx]);
      }
      if(c==7){
        crcOk = crc.finish() == cycl_lookup[dx];
      }
    }
  }
  Serial.write(frame, 8);
  if(!crcOk){
    Serial.print(" CRC error!");
  }
  Serial.println("");
  if (error) {
    Serial.println("Buffer overflow");
    while (true) {
      ;
    }
  }
}
