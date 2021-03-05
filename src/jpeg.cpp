#include "jpeg.hpp"


const uint8_t DefaultQuantLuminance[8*8] =
    { 16, 11, 10, 16, 24, 40, 51, 61,
      12, 12, 14, 19, 26, 58, 60, 55,
      14, 13, 16, 24, 40, 57, 69, 56,
      14, 17, 22, 29, 51, 87, 80, 62,
      18, 22, 37, 56, 68,109,103, 77,
      24, 35, 55, 64, 81,104,113, 92,
      49, 64, 78, 87,103,121,120,101,
      72, 92, 95, 98,112,100,103, 99 };
const uint8_t DefaultQuantChrominance[8*8] =
    { 17, 18, 24, 47, 99, 99, 99, 99,
      18, 21, 26, 66, 99, 99, 99, 99,
      24, 26, 56, 99, 99, 99, 99, 99,
      47, 66, 99, 99, 99, 99, 99, 99,
      99, 99, 99, 99, 99, 99, 99, 99,
      99, 99, 99, 99, 99, 99, 99, 99,
      99, 99, 99, 99, 99, 99, 99, 99,
      99, 99, 99, 99, 99, 99, 99, 99 };
const uint8_t ZigZagInv[8*8] =
    {  0, 1, 8,16, 9, 2, 3,10,   // ZigZag[] =  0, 1, 5, 6,14,15,27,28,
      17,24,32,25,18,11, 4, 5,   //             2, 4, 7,13,16,26,29,42,
      12,19,26,33,40,48,41,34,   //             3, 8,12,17,25,30,41,43,
      27,20,13, 6, 7,14,21,28,   //             9,11,18,24,31,40,44,53,
      35,42,49,56,57,50,43,36,   //            10,19,23,32,39,45,52,54,
      29,22,15,23,30,37,44,51,   //            20,22,33,38,46,51,55,60,
      58,59,52,45,38,31,39,46,   //            21,34,37,47,50,56,59,61,
      53,60,61,54,47,55,62,63 }; //            35,36,48,49,57,58,62,63


const uint8_t DcLuminanceCodesPerBitsize[16]   = { 0,1,5,1,1,1,1,1,1,0,0,0,0,0,0,0 };
const uint8_t DcLuminanceValues         [12]   = { 0,1,2,3,4,5,6,7,8,9,10,11 };
const uint8_t AcLuminanceCodesPerBitsize[16]   = { 0,2,1,3,3,2,4,3,5,5,4,4,0,0,1,125 };
const uint8_t AcLuminanceValues        [162]   =
    { 0x01,0x02,0x03,0x00,0x04,0x11,0x05,0x12,0x21,0x31,0x41,0x06,0x13,0x51,0x61,0x07,0x22,0x71,0x14,0x32,0x81,0x91,0xA1,0x08, // 16*10+2 symbols because
      0x23,0x42,0xB1,0xC1,0x15,0x52,0xD1,0xF0,0x24,0x33,0x62,0x72,0x82,0x09,0x0A,0x16,0x17,0x18,0x19,0x1A,0x25,0x26,0x27,0x28, // upper 4 bits can be 0..F
      0x29,0x2A,0x34,0x35,0x36,0x37,0x38,0x39,0x3A,0x43,0x44,0x45,0x46,0x47,0x48,0x49,0x4A,0x53,0x54,0x55,0x56,0x57,0x58,0x59, // while lower 4 bits can be 1..A
      0x5A,0x63,0x64,0x65,0x66,0x67,0x68,0x69,0x6A,0x73,0x74,0x75,0x76,0x77,0x78,0x79,0x7A,0x83,0x84,0x85,0x86,0x87,0x88,0x89, // plus two special codes 0x00 and 0xF0
      0x8A,0x92,0x93,0x94,0x95,0x96,0x97,0x98,0x99,0x9A,0xA2,0xA3,0xA4,0xA5,0xA6,0xA7,0xA8,0xA9,0xAA,0xB2,0xB3,0xB4,0xB5,0xB6, // order of these symbols was determined empirically by JPEG committee
      0xB7,0xB8,0xB9,0xBA,0xC2,0xC3,0xC4,0xC5,0xC6,0xC7,0xC8,0xC9,0xCA,0xD2,0xD3,0xD4,0xD5,0xD6,0xD7,0xD8,0xD9,0xDA,0xE1,0xE2,
      0xE3,0xE4,0xE5,0xE6,0xE7,0xE8,0xE9,0xEA,0xF1,0xF2,0xF3,0xF4,0xF5,0xF6,0xF7,0xF8,0xF9,0xFA };
// Huffman definitions for second DC/AC tables (chrominance / Cb and Cr channels)
const uint8_t DcChrominanceCodesPerBitsize[16] = { 0,3,1,1,1,1,1,1,1,1,1,0,0,0,0,0 };   // sum = 12
const uint8_t DcChrominanceValues         [12] = { 0,1,2,3,4,5,6,7,8,9,10,11 };         // => 12 codes (identical to DcLuminanceValues)
const uint8_t AcChrominanceCodesPerBitsize[16] = { 0,2,1,2,4,4,3,4,7,5,4,4,0,1,2,119 }; // sum = 162
const uint8_t AcChrominanceValues        [162] =                                        // => 162 codes
    { 0x00,0x01,0x02,0x03,0x11,0x04,0x05,0x21,0x31,0x06,0x12,0x41,0x51,0x07,0x61,0x71,0x13,0x22,0x32,0x81,0x08,0x14,0x42,0x91, // same number of symbol, just different order
      0xA1,0xB1,0xC1,0x09,0x23,0x33,0x52,0xF0,0x15,0x62,0x72,0xD1,0x0A,0x16,0x24,0x34,0xE1,0x25,0xF1,0x17,0x18,0x19,0x1A,0x26, // (which is more efficient for AC coding)
      0x27,0x28,0x29,0x2A,0x35,0x36,0x37,0x38,0x39,0x3A,0x43,0x44,0x45,0x46,0x47,0x48,0x49,0x4A,0x53,0x54,0x55,0x56,0x57,0x58,
      0x59,0x5A,0x63,0x64,0x65,0x66,0x67,0x68,0x69,0x6A,0x73,0x74,0x75,0x76,0x77,0x78,0x79,0x7A,0x82,0x83,0x84,0x85,0x86,0x87,
      0x88,0x89,0x8A,0x92,0x93,0x94,0x95,0x96,0x97,0x98,0x99,0x9A,0xA2,0xA3,0xA4,0xA5,0xA6,0xA7,0xA8,0xA9,0xAA,0xB2,0xB3,0xB4,
      0xB5,0xB6,0xB7,0xB8,0xB9,0xBA,0xC2,0xC3,0xC4,0xC5,0xC6,0xC7,0xC8,0xC9,0xCA,0xD2,0xD3,0xD4,0xD5,0xD6,0xD7,0xD8,0xD9,0xDA,
      0xE2,0xE3,0xE4,0xE5,0xE6,0xE7,0xE8,0xE9,0xEA,0xF2,0xF3,0xF4,0xF5,0xF6,0xF7,0xF8,0xF9,0xFA };
const int16_t CodeWordLimit = 2048; // +/-2^11, maximum value after DCT

template <typename Number, typename Limit>
Number clamp(Number value, Limit minValue, Limit maxValue)
{
  if (value <= minValue) return minValue;
  if (value >= maxValue) return maxValue;
  return value;
}

BitCode huffmanLuminanceDC[256];
BitCode huffmanLuminanceAC[256];
BitCode huffmanChrominanceDC[256];
BitCode huffmanChrominanceAC[256];

float16 scaledLuminance[8 * 8];
float16 scaledChrominance[8 * 8];
uint8_t quantLuminance  [8*8];
uint8_t quantChrominance[8*8];
BitCode codewordsArray[2 * CodeWordLimit];

/**
 * Huffman Tabels can either be saved as constant or calculated
 * Here is the calculation
 */ 
void generateHuffmanTable(const uint8_t numCodes[16], const uint8_t* val,
		BitCode res[256]) {
	auto huffmanCode = 0;
	for (auto numBits = 1; numBits <= 16; numBits++) {
		for (auto i = 0; i < numCodes[numBits - 1]; i++)
			res[*val++] = BitCode(huffmanCode++, numBits);

		huffmanCode <<= 1;
	}
}
void write_first_Value(uint8_t valuewrite, AXI_STREAM &outputStream) {
	ap_axiu<8, 1, 1, 1> value;
	value.data = valuewrite;
	value.user = 1;
	value.last = 0;
	value.keep = -1;
	outputStream << value;
}
void write_last_Value(uint8_t valuewrite, AXI_STREAM &outputStream) {
	ap_axiu<8, 1, 1, 1> value;
	value.data = valuewrite;
	value.user = 0;
	value.last = 1;
	value.keep = -1;
	outputStream << value;
}
void write_Value(uint8_t valuewrite, AXI_STREAM &outputStream) {
	ap_axiu<8, 1, 1, 1> value;
	value.data = valuewrite;
	value.user = 0;
	value.last = 0;
	value.keep = -1;
	outputStream << value;

}
void write_Value_16bit(uint16_t valuewrite, AXI_STREAM &outputStream) {
	uint8_t msb = (uint8_t) (valuewrite >> 8);
	uint8_t lsb = (uint8_t) (valuewrite);

	ap_axiu<8, 1, 1, 1> value;
	value.data = msb;
	value.user = 0;
	value.last = 0;
	value.keep = -1;
	outputStream << value;

	ap_axiu<8, 1, 1, 1> value2;
	value2.data = lsb;
	value2.user = 0;
	value2.last = 0;
	value2.keep = -1;
	outputStream << value2;

}

void mergeStream(AXI_STREAM &s1, AXI_STREAM &s2, AXI_STREAM &s3) {
	bool sof1 = 0;
	bool sof2 = 0;
	ap_axiu<8, 1, 1, 1> axi1;
	ap_axiu<8, 1, 1, 1> axi2;
	bool firstPartDone = false;
	while (!sof1) {
		#pragma HLS pipeline II=1
		#pragma HLS loop_tripcount avg=0 max=0
		s1 >> axi1;
		sof1 = axi1.user.to_int();
	}

	//write_first_Value((uint8_t) (axi1.data), s3);
	bool first = 1;
	while (!axi1.last.to_bool()) {
		s1 >> axi1;
		if (first){
			write_first_Value((uint8_t) axi1.data, s3);
			first=false;
		}
		else{
			if (!axi1.last.to_bool()){
				write_Value((uint8_t) axi1.data, s3);
			}else{
				firstPartDone = true;
			}
		}

	}

	while (!sof2) {
#pragma HLS pipeline II=1
#pragma HLS loop_tripcount avg=0 max=0
		s2 >> axi2;
		sof2 = axi2.user.to_int(); //kann verworfen werden da fuellwert
	}

	if (firstPartDone) {
		while (!axi2.last.to_bool()) {
			s2 >> axi2;
			uint8_t tmp = (uint8_t) axi2.data.to_int();

			write_Value(tmp, s3);
		}
		write_last_Value((uint8_t) axi2.data, s3);
	}
}

void DCT(float16 block[8 * 8], uint8_t stride) {
	const float16 SqrtHalfSqrt = 1.306562965; 	//     sqrt((2 + sqrt(2)) / 2) 	= cos(pi * 1 / 8) * sqrt(2)
	const float16 InvSqrt = 0.707106781; 		// 1 / sqrt(2)                	= cos(pi * 2 / 8)
	const float16 HalfSqrtSqrt = 0.382683432; 	//     sqrt(2 - sqrt(2)) / 2  	= cos(pi * 3 / 8)
	const float16 InvSqrtSqrt = 0.541196100; 	// 1 / sqrt(2 - sqrt(2))      	= cos(pi * 3 / 8) * sqrt(2)

	// modify in-place
	float16& block0 = block[0];
	float16& block1 = block[1 * stride];
	float16& block2 = block[2 * stride];
	float16& block3 = block[3 * stride];
	float16& block4 = block[4 * stride];
	float16& block5 = block[5 * stride];
	float16& block6 = block[6 * stride];
	float16& block7 = block[7 * stride];

	float16 add07 = block0 + block7;
	float16 sub07 = block0 - block7;
	float16 add16 = block1 + block6;
	float16 sub16 = block1 - block6;
	float16 add25 = block2 + block5;
	float16 sub25 = block2 - block5;
	float16 add34 = block3 + block4;
	float16 sub34 = block3 - block4;

	float16 add0347 = add07 + add34;
	float16 sub07_34 = add07 - add34;
	float16 add1256 = add16 + add25;
	float16 sub16_25 = add16 - add25;

	block0 = add0347 + add1256;
	block4 = add0347 - add1256;

	float16 z1 = (sub16_25 + sub07_34) * InvSqrt;
	block2 = sub07_34 + z1;
	block6 = sub07_34 - z1;

	float16 sub23_45 = sub25 + sub34;
	float16 sub12_56 = sub16 + sub25;
	float16 sub01_67 = sub16 + sub07;

	float16 z5 = (sub23_45 - sub01_67) * HalfSqrtSqrt;
	float16 z2 = sub23_45 * InvSqrtSqrt + z5;
	float16 z3 = sub12_56 * InvSqrt;
	float16 z4 = sub01_67 * SqrtHalfSqrt + z5;
	float16 z6 = sub07 + z3;
	float16 z7 = sub07 - z3;
	block1 = z6 + z4;
	block7 = z6 - z4;
	block5 = z7 + z2;
	block3 = z7 - z2;
}

void Quant(float16* block, const float16 scaled[8 * 8], int16_t *quant,
		int &posNonZero, int &DC) {
#pragma HLS inline off

	//This Loop is theoretical part of the dct, but scaled[] is prepared and optimized for quantization
	quantLoop: for (auto i = 0; i < 8 * 8; i++)
		block[i] = block[i] * scaled[i];

	QuantToIntLoop: for (auto i = 0; i < 8 * 8; i++) {
		float16 value = block[ZigZagInv[i]];
		if (value > 0)
			value += (float16) 0.5;
		else
			value -= (float16) 0.5;
		quant[i] = int(value);

		if (quant[i] != 0)
			posNonZero = i;
	}
	DC = quant[0];

}

void huffmann(int16_t *quant, int posNonZero, int DC, int lastDC,
		AXI_BIT_WRITER2& writer, const BitCode huffmanDC[256],
		const BitCode huffmanAC[256]) {
#pragma HLS inline off
	auto diff = DC - lastDC;
	if (diff == 0)
		writer.write(huffmanDC[0x00]);
	else {
		auto bits = codewordsArray[diff + CodeWordLimit];
		writer.write(huffmanDC[bits.numBits]);
		writer.write(bits);
	}

	auto offset = 0;
	for (auto i = 1; i <= posNonZero; i++) {

		while (quant[i] == 0) {
			offset += 0x10;
			if (offset > 0xF0) {
				writer.write(huffmanAC[0xf0]);
				offset = 0;
			}
			i++;
		}

		auto encoded = codewordsArray[quant[i] + CodeWordLimit];
		writer.write(huffmanAC[offset + encoded.numBits]);
		writer.write(encoded);
		offset = 0;
	}

	if (posNonZero < 8 * 8 - 1) // = 63
		writer.write(huffmanAC[0x00]);
}

int16_t encodeBlock(AXI_BIT_WRITER2& w, float16 block[8][8],
		const float16 scaled[8 * 8], int16_t lastDC, const BitCode huffDC[256],
		const BitCode huffAC[256]) {

	float16 *block64 = (float16*) block;

	DCT_row: for (int offset = 0; offset < 8; offset++)
		DCT(block64 + offset * 8, 1);
	DCT_col: for (int offset = 0; offset < 8; offset++)
		DCT(block64 + offset * 1, 8);

	int16_t quantized[8 * 8];
	int posNonZero = 0;
	int DC;
	Quant(block64, scaled, quantized, posNonZero, DC);

	huffmann(quantized, posNonZero, DC, lastDC, w, huffDC, huffAC);

	return DC;
}
/**
 * Prepare the header of a JPEG file with constants and
 * other definitions to
 */
void writeHeader(AXI_BIT_WRITER2 &writer, uint8_t quality_) {

	writer.writeStartSymbol();

	uint16_t height = MAX_HEIGHT;
	uint16_t width = MAX_WIDTH;

	const uint8_t HeaderJfif[2 + 2 + 16] = { 0xFF, 0xD8, 0xFF, 0xE0, 0, 16, 'J',
			'F', 'I', 'F', 0, 1, 1, 0, 0, 1, 0, 1, 0, 0 };
	writer.write(HeaderJfif);

	auto quality = clamp<uint8_t>(quality_, 1, 100);

	quality = quality < 50 ? 5000 / quality : 200 - quality * 2;
	for (auto i = 0; i < 8 * 8; i++) {
		uint8_t lum = (DefaultQuantLuminance[ZigZagInv[i]] * quality + 50) / 100;
		uint8_t chr = (DefaultQuantChrominance[ZigZagInv[i]] * quality + 50) / 100;

		quantLuminance[i] = clamp(lum, 1, 255);
		quantChrominance[i] = clamp(chr, 1, 255);
	}
	writer.addMarker(0xdb, 132);
	writer.write(0);
	writer.write(quantLuminance);
	writer.write(1);
	writer.write(quantChrominance);
	writer.addMarker(0xC0, 2 + 6 + 3 * 3);
	writer.write(0x08);
	writer.write(height >> 8);
	writer.write((uint8_t)height);
	writer.write(width >> 8);
	writer.write((uint8_t)width);
	writer.write(3);
	for (auto id = 1; id <= 3; id++) {
		writer.write(id);
		writer.write(0x11);
		writer.write(id == 1 ? 0 : 1);
	}

	writer.addMarker(0xC4, (2 + 208 + 208));
	writer.write(0);
	writer.write(DcLuminanceCodesPerBitsize);
	writer.write(DcLuminanceValues);
	writer.write(0x10);
	writer.write(AcLuminanceCodesPerBitsize);
	for (auto i =0; i<162;i++){
		writer.write(AcLuminanceValues[i]);
	}
	//writer.write(AcLuminanceValues);

	generateHuffmanTable(DcLuminanceCodesPerBitsize, DcLuminanceValues,huffmanLuminanceDC);
	generateHuffmanTable(AcLuminanceCodesPerBitsize, AcLuminanceValues,huffmanLuminanceAC);



	writer.write(1);
	writer.write(DcChrominanceCodesPerBitsize);
	writer.write(DcChrominanceValues);
	writer.write(0x11);
	writer.write(AcChrominanceCodesPerBitsize);
	for (auto i =0; i<162;i++){
		writer.write(AcChrominanceValues[i]);
	}

	generateHuffmanTable(DcChrominanceCodesPerBitsize, DcChrominanceValues,huffmanChrominanceDC);
	generateHuffmanTable(AcChrominanceCodesPerBitsize, AcChrominanceValues,huffmanChrominanceAC);

	writer.addMarker(0xDA, 2 + 1 + 2 * 3 + 3);
	writer.write(3);

	for (auto id = 1; id <= 3; id++) {
		writer.write(id);
		writer.write(id == 1 ? 0 : 0x11);
	}
	static const uint8_t Spectral[3] = { 0, 0x3f, 0 };
	writer.write(Spectral);

	static const float AanScaleFactors[8] = { 1, 1.387039845f, 1.306562965f,
			1.175875602f, 1, 0.785694958f, 0.541196100f, 0.275899379f };

	prepQuant:
	for (auto i = 0; i < 8 * 8; i++) {
		auto row = ZigZagInv[i] / 8;
		auto column = ZigZagInv[i] % 8;
		auto factor = 1 / (AanScaleFactors[row] * AanScaleFactors[column] * 8);
		scaledLuminance[ZigZagInv[i]] = factor / quantLuminance[i];
		scaledChrominance[ZigZagInv[i]] = factor / quantChrominance[i];
	}


	uint8_t numBits = 1;
	int32_t mask = 1;
	BitCode* codewords = &codewordsArray[CodeWordLimit];
	for (int16_t value = 1; value < CodeWordLimit; value++) {
		if (value > mask) {
			numBits++;
			mask = (mask << 1) | 1;
		}
		codewords[-value] = BitCode(mask - value, numBits);
		codewords[+value] = BitCode(value, numBits);
	}

	writer.writeLastSymbol();
}

void Encode_color(IMAGE3s &matIn,AXI_BIT_WRITER2 &datawriter) {
	datawriter.writeStartSymbol();
	int16_t linebuffer1[8][MAX_WIDTH];
	int16_t linebuffer2[8][MAX_WIDTH];
	int16_t linebuffer3[8][MAX_WIDTH];
	float16 window1[8][8];
	float16 window2[8][8];
	float16 window3[8][8];
	int line = 0;
	int16_t lastYDC = 0, lastCbDC = 0, lastCrDC = 0;
	for (int y = 0; y < MAX_HEIGHT; y++) {
		for (int x = 0; x < MAX_WIDTH; x++) {
			#pragma HLS loop flatten off
			#pragma HLS pipeline II=2
			hls::Scalar<3, int16_t> pixel;
			matIn >> pixel;

			linebuffer1[line][x] = pixel.val[0];	//y
			linebuffer2[line][x] = pixel.val[1];	//cr
			linebuffer3[line][x] = pixel.val[2];	//cb
		}
		if (line == 7) {
			line = 0;
			for (int step = 0; step < MAX_WIDTH; step += 8) {
				for (int i = 0; i < 8; i++) {
					for (int j = 0; j < 8; j++) {
						window1[j][i] = linebuffer1[j][i + step];	//y
						window2[j][i] = linebuffer2[j][i + step];	//cr
						window3[j][i] = linebuffer3[j][i + step];	//cb
					}
				}
				lastYDC = encodeBlock(datawriter, window1, scaledLuminance,
						lastYDC, huffmanLuminanceDC, huffmanLuminanceAC);
				lastCbDC = encodeBlock(datawriter, window2, scaledChrominance,
						lastCbDC, huffmanChrominanceDC, huffmanChrominanceAC);
				lastCrDC = encodeBlock(datawriter, window3, scaledChrominance,
						lastCrDC, huffmanChrominanceDC, huffmanChrominanceAC);
			}
		} else {
			line++;
		}
	}
	datawriter.flush();
	datawriter.writeEndofJPEG();
}

void readMat(IMAGE3s &matIn,uint8_t quality, AXI_STREAM &outputStream){
#pragma HLS Dataflow
	AXI_BIT_WRITER2 headerwriter,datawriter;
	writeHeader(headerwriter,quality);
	Encode_color(matIn,datawriter);
	mergeStream(headerwriter.stream,datawriter.stream,outputStream);
}


/*
 * -----------Grayscale-----------------
 */

void writeHeaderGray(AXI_BIT_WRITER2 &writer, uint8_t quality_) {

	auto quality = clamp<uint8_t>(quality_, 1, 100);
	quality = quality < 50 ? 5000 / quality : 200 - quality * 2;

	for (auto i = 0; i < 64; i++) {
		uint8_t lum = (DefaultQuantLuminance[ZigZagInv[i]] * quality + 50)
				/ 100;
		uint8_t chr = (DefaultQuantChrominance[ZigZagInv[i]] * quality + 50)
				/ 100;
		quantLuminance[i] = clamp(lum, 1, 255);
		quantChrominance[i] = clamp(chr, 1, 255);
	}

	generateHuffmanTable(DcLuminanceCodesPerBitsize, DcLuminanceValues,huffmanLuminanceDC);
	generateHuffmanTable(AcLuminanceCodesPerBitsize, AcLuminanceValues,huffmanLuminanceAC);
	generateHuffmanTable(DcChrominanceCodesPerBitsize, DcChrominanceValues,huffmanChrominanceDC);
	generateHuffmanTable(AcChrominanceCodesPerBitsize, AcChrominanceValues,huffmanChrominanceAC);


	writer.writeStartSymbol();

	uint16_t height = MAX_HEIGHT;
	uint16_t width = MAX_WIDTH;

	writer.write(0xff);
	writer.write(0xD8);

	writer.write(0xff);
	writer.write(0xDB);

	writer.write(0x00);
	writer.write(0x84);
	writer.write(0x00);

	writer.write(quantLuminance);
	writer.write(1);
	writer.write(quantChrominance);



	writer.addMarker(0xC4, (2 + 208 + 208));
	writer.write(0);
	writer.write(DcLuminanceCodesPerBitsize);
	writer.write(DcLuminanceValues);
	writer.write(0x10);
	writer.write(AcLuminanceCodesPerBitsize);
	for (auto i =0; i<162;i++){
		writer.write(AcLuminanceValues[i]);
	}
	//writer.write(AcLuminanceValues);



	writer.write(1);
	writer.write(DcChrominanceCodesPerBitsize);
	writer.write(DcChrominanceValues);
	writer.write(0x11);
	writer.write(AcChrominanceCodesPerBitsize);
	for (auto i =0; i<162;i++){
		writer.write(AcChrominanceValues[i]);
	}


	writer.addMarker(0xC0, 8 + 3 * 1);

	writer.write(0x08);//precision

	writer.write(height >> 8);
	writer.write((uint8_t)height);

	writer.write(width >> 8);
	writer.write((uint8_t)width);

	writer.write(1);//# of components

	writer.write(0x01);
	writer.write(0x11);
	writer.write(0x00);

	writer.addMarker(0xDA,8);//Start of scan(data)

	writer.write(0x01);//# of components
	writer.write(0x01);
	writer.write(0x00);

	writer.write(0x0);//spectral
	writer.write(0x3f);
	writer.write(0x0);


	static const float AanScaleFactors[8] = { 1, 1.387039845f, 1.306562965f,
			1.175875602f, 1, 0.785694958f, 0.541196100f, 0.275899379f };

	prepQuant:
	for (auto i = 0; i < 8 * 8; i++) {
		auto row = ZigZagInv[i] / 8;
		auto column = ZigZagInv[i] % 8;
		auto factor = 1 / (AanScaleFactors[row] * AanScaleFactors[column] * 8);
		scaledLuminance[ZigZagInv[i]] = factor / quantLuminance[i];
		scaledChrominance[ZigZagInv[i]] = factor / quantChrominance[i];
	}


	uint8_t numBits = 1;
	int32_t mask = 1;
	BitCode* codewords = &codewordsArray[CodeWordLimit];
	for (int16_t value = 1; value < CodeWordLimit; value++) {
		if (value > mask) {
			numBits++;
			mask = (mask << 1) | 1;
		}
		codewords[-value] = BitCode(mask - value, numBits);
		codewords[+value] = BitCode(value, numBits);
	}

	writer.writeLastSymbol();
}

void Encode_gray(IMAGE1 &matIn,AXI_BIT_WRITER2 &datawriter) {
	datawriter.writeStartSymbol();
	int16_t linebuffer1[8][MAX_WIDTH];
	float16 window1[8][8];

	int line = 0;
	int16_t lastYDC = 0, lastCbDC = 0, lastCrDC = 0;
	for (int y = 0; y < MAX_HEIGHT; y++) {
		for (int x = 0; x < MAX_WIDTH; x++) {
			#pragma HLS loop flatten off
			#pragma HLS pipeline II=2
			hls::Scalar<1, uint8_t> pixel;
			matIn >> pixel;
			linebuffer1[line][x] = pixel.val[0];	//y

		}
		if (line == 7) {
			line = 0;
			for (int step = 0; step < MAX_WIDTH; step += 8) {
				for (int i = 0; i < 8; i++) {
					for (int j = 0; j < 8; j++) {
						window1[j][i] = linebuffer1[j][i + step];	//y

					}
				}
				lastYDC = encodeBlock(datawriter, window1, scaledLuminance,
						lastYDC, huffmanLuminanceDC, huffmanLuminanceAC);

			}
		} else {
			line++;
		}
	}
	datawriter.flush();
	datawriter.writeEndofJPEG();
}

void readMatGrey(IMAGE1 &matIn,uint8_t quality, AXI_STREAM &outputStream){
#pragma HLS Dataflow
	AXI_BIT_WRITER2 headerwriter,datawriter;
	writeHeaderGray(headerwriter,quality);
	Encode_gray(matIn,datawriter);
	mergeStream(headerwriter.stream,datawriter.stream,outputStream);
}


