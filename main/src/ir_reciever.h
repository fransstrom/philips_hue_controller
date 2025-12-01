#ifndef IR_RECIEVER_H
#define IR_RECIEVER_H
#include "driver/rmt_rx.h"
#include "soc/gpio_num.h"

void ir_receiver_init(gpio_num_t gpio_pin);
bool rmt_rx_done_callback(rmt_channel_handle_t channel,
                          const rmt_rx_done_event_data_t *edata,
                          void *user_data);

void ir_rx_task(void *arg);

uint32_t decode_nec(rmt_symbol_word_t *symbols, size_t num_symbols);

void handleCode(uint32_t nec_code);
int testBuild();

#endif // !IR_REVIEVER_H
