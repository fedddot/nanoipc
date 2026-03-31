#include <cstddef>

#include "cobs.h"

int main() {
    uint8_t data[] = {0x11, 0x22, 0x00, 0x33, 0x44};
    enum: std::size_t { BUFF_LEN = 10 };
    uint8_t encoded[BUFF_LEN] = {'\0'};
    std::size_t encoded_length = 0;
    uint8_t decoded[BUFF_LEN] = {'\0'};
    std::size_t decoded_length = 0;

    const auto enc_res = cobs_encode(data, sizeof(data), encoded, BUFF_LEN, &encoded_length);
    const auto dec_res = cobs_decode(encoded, encoded_length, decoded, BUFF_LEN, &decoded_length);

    return 0;
}