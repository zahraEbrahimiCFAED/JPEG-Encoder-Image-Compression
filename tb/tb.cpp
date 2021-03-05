#include "toplevel.hpp"
#include <ap_fixed.h>
#include <hls_opencv.h>
#include <iostream>
#include <fstream>

using namespace std;

void writefile(AXI_STREAM &stream,uint8_t i){
	remove("example.jpg");
	ap_axiu<8,1,1,1> data;
	ofstream myfile;
	std::string filename = "example_"+std::to_string(i)+".jpg";
	myfile.open (filename.c_str(), ios::out | ios::app | ios::binary);

	do{
		stream >> data;
		if (!data.last.to_bool())myfile << (uint8_t)data.data;
	}while(data.last.to_bool()==0);

	myfile.close();

}

void writefile(AXI_STREAM &stream,std::string nameOut){
	ap_axiu<8,1,1,1> data;
	ofstream myfile;
	std::string filename = nameOut;
	myfile.open (filename.c_str(), ios::out | ios::app | ios::binary);
	do{
		stream >> data;
		if (!data.last.to_bool())myfile << (uint8_t)data.data;
	}while(data.last.to_bool()==0);

	myfile.close();

}

void writefile(AXI_STREAM &stream){
	remove("example.jpg");
	ap_axiu<8,1,1,1> data;
	ofstream myfile;
	std::string filename = "example.jpg";
	myfile.open (filename.c_str(), ios::out | ios::app | ios::binary);

	do{
		stream >> data;
		if (!data.last.to_bool())myfile << (uint8_t)data.data;
	}while(data.last.to_bool()==0);

	myfile.close();

}

void testOneFileGrey(uint8_t quality,std::string inputFile,std::string outputFile){
	IplImage* src_image = new IplImage;

	const char* filename =inputFile.c_str();
	AXI_IN_STREAM src_stream;
	AXI_STREAM out_stream;

	src_image = cvLoadImage(filename);
	if (src_image->width == 1920 && src_image->height==1080){
		IplImage2AXIvideo(src_image, src_stream);
		top_level_gr(src_stream,quality,out_stream);
		writefile(out_stream,outputFile);

	}else{
		std::cout << "Please select valid file size!\nSkipped\n";
	}
    cvReleaseImage(&src_image);
}

void testOneFileColor(uint8_t quality,std::string inputFile,std::string outputFile){
	IplImage* src_image = new IplImage;

	const char* filename =inputFile.c_str();
	AXI_IN_STREAM src_stream;
	AXI_STREAM out_stream;

	src_image = cvLoadImage(filename);
	if (src_image->width == 1920 && src_image->height==1080){
		IplImage2AXIvideo(src_image, src_stream);
		top_level(src_stream,quality,out_stream);
		writefile(out_stream,outputFile);

	}else{
		std::cout << "Please select valid file size!\nSkipped\n";
	}
    cvReleaseImage(&src_image);
}

int main(){
	for (int q=90;q>40;q-=10){
		testOneFileColor(q,"files//1.jpg","result_"+std::to_string(q)+".jpg");
		testOneFileGrey(q,"files//1.jpg","result_gr_"+std::to_string(q)+".jpg");
	}
	return 0;
}
