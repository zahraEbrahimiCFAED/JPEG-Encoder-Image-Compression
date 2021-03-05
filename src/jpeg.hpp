#include "hls_video.h"
#include <ap_fixed.h>
#include <stdint.h>
#include "hls_stream.h"


#define MAX_HEIGHT 1080//1500
#define MAX_WIDTH 1920//2000

typedef hls::stream<ap_axiu<8,1,1,1> > AXI_STREAM;//32 bit data stream
typedef hls::stream<ap_axiu<32,1,1,1> > AXI_IN_STREAM;//32 bit data stream

void write_Value(uint8_t valuewrite, AXI_STREAM &outputStream);
void write_last_Value(uint8_t valuewrite, AXI_STREAM &outputStream);
void write_first_Value(uint8_t valuewrite, AXI_STREAM &outputStream);
void write_Value_16bit(uint16_t valuewrite, AXI_STREAM &outputStream);

typedef hls::Mat<MAX_HEIGHT,   MAX_WIDTH,   HLS_8UC3> IMAGE3;
typedef hls::Mat<MAX_HEIGHT,   MAX_WIDTH,   HLS_16SC3> IMAGE3s;
typedef hls::Mat<MAX_HEIGHT,   MAX_WIDTH,   HLS_8UC1> IMAGE1;
typedef float float16;//ap_fixed<32,16>;

#ifndef STRUCTS_DEC
#define STRUCTS_DEC
struct BitCode
{
  BitCode() = default;
  BitCode(uint16_t code_, uint8_t numBits_)
  : code(code_), numBits(numBits_) {}
  uint16_t code;       //Huffman codes are limited to 16 bits
  uint8_t  numBits;    // number of valid bits
};
struct AXI_BIT_WRITER2 {

	struct BitBuffer {
		int32_t data = 0;
		uint8_t numBits = 0;
	} buffer;

	AXI_BIT_WRITER2() = default;
	AXI_STREAM stream;



	void write(const BitCode& data) {
		buffer.numBits += data.numBits;
		buffer.data <<= data.numBits;
		buffer.data |= data.code;

		while (buffer.numBits >= 8) {
			buffer.numBits -= 8;
			uint8_t oneByte = uint8_t(buffer.data >> buffer.numBits);

			write_Value(oneByte, stream);

			if (oneByte == 0xFF)
				write_Value(0, stream);
		}

	}

	void write(uint8_t data) {
		write_Value(data, stream);

	}

	template<typename T, int Size>
	void write(T (&manyBytes)[Size]) {
		for (auto c : manyBytes)
			write_Value(c, stream);
	}

	void flush() {
		this->write( BitCode(0x7F, 7));
	}

	void addMarker(uint8_t id, uint16_t length) {
		write_Value(0xFF,stream);
		write_Value(id,stream);
		write_Value(uint8_t(length >> 8),stream);
		write_Value(uint8_t(length & 0xFF),stream);
	}

	void writeStartSymbol(){
		write_first_Value(0,stream);
	}
	void writeLastSymbol(){
		write_last_Value(0,stream);
	}
	void writeEndofJPEG(){
		write_Value(0xff,stream);
		write_Value(0xd9,stream);
		writeLastSymbol();
	}
};
#endif

void readMat(IMAGE3s &matIn,uint8_t quality, AXI_STREAM &outputStream);
void mergeStream(AXI_STREAM &s1,AXI_STREAM &s2,AXI_STREAM &s3);
void writeHeader(AXI_BIT_WRITER2 &writer, int quality);
