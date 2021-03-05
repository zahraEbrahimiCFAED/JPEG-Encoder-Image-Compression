#include "toplevel.hpp"

void cvtRGB2YC(IMAGE3 &in, IMAGE3s &out){
	int c1 = 77;//0.299
	int c2 = 150;//0.587
	int c3 = 29;//0.114
	int c4 = 43;//0.16874
	int c5 = 85;
	int c6 = 128;
	int c7 = 107;
	int c8 = 21;

	for (int y = 0; y < MAX_HEIGHT; y++) {
		for (int x = 0; x < MAX_WIDTH; x++) {
			#pragma HLS loop flatten off
			#pragma HLS pipeline II=2
			hls::Scalar<3, uint8_t> pixel;
			hls::Scalar<3, int16_t> pixelOut;
			in >> pixel;
			uint8_t b = pixel.val[0];
			uint8_t g = pixel.val[1];
			uint8_t r = pixel.val[2];
			int16_t Y  = (+c1 * r + c2 * g + c3 * b) >> 8;
			int16_t Cb = (-c4 * r - c5 * g + c6 * b) >> 8;
			int16_t Cr = (+c6 * r - c7 * g - c8 * b) >> 8;
			pixelOut.val[0] = Y - 128;
			pixelOut.val[1] = Cb;
			pixelOut.val[2] = Cr;
			out << pixelOut;
		}
	}
}

void top_level(AXI_IN_STREAM &inStream,uint8_t q, AXI_STREAM &output){
#pragma HLS INTERFACE ap_ctrl_none port=return
#pragma HLS INTERFACE axis port=inStream
#pragma HLS INTERFACE axis port=output
	IMAGE3 colorMat(MAX_HEIGHT, MAX_WIDTH);
	IMAGE3s YCrCbMat(MAX_HEIGHT, MAX_WIDTH);

#pragma HLS Dataflow
	hls::AXIvideo2Mat(inStream, colorMat);
	cvtRGB2YC(colorMat,YCrCbMat);
	readMat(YCrCbMat,q,output);
}

