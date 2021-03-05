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


void test1(uint8_t quality){
	IplImage* src_image = new IplImage;

	const char* filename ="1.jpg";
	AXI_IN_STREAM src_stream;
	AXI_STREAM iris_in_stream;

	src_image = cvLoadImage(filename);

	IplImage2AXIvideo(src_image, src_stream);
	top_level(src_stream,quality,iris_in_stream);
	writefile(iris_in_stream,quality);

    cvReleaseImage(&src_image);

}

/*
void test3(){
	IplImage* src_image = new IplImage;

	const char* filename ="2.jpg";
	AXI_IN_STREAM src_stream;
	AXI_STREAM iris_in_stream;

	src_image = cvLoadImage(filename);

	IplImage2AXIvideo(src_image, src_stream);
	top_level(src_stream,50,iris_in_stream);
	writefile(iris_in_stream);

    cvReleaseImage(&src_image);

}
*/

void testCreateJPEG(std::string nameIn, std::string nameOut,int q) {
	IplImage* src_image = new IplImage;

	const char* filename = nameIn.c_str();
	AXI_IN_STREAM src_stream;
	AXI_STREAM iris_in_stream;

	src_image = cvLoadImage(filename);
	if (src_image->width == 1920 && src_image->height==1080){
		std::cout << "Load file\n";
		IplImage2AXIvideo(src_image, src_stream);
		top_level(src_stream, q, iris_in_stream);
		writefile(iris_in_stream,nameOut);
	}else{
		std::cout << "No correct size! Skipped\n";
	}



	cvReleaseImage(&src_image);
}

void test4(){
	std::string path = "C:\\Users\\DennisMobil\\Desktop\\TestPictures\\";
	for (int i = 1;i<=10;i++){
		std::string pathtmp1 = path + std::to_string(i)+"\\1.jpg";
		std::string pathtmp2 = path + std::to_string(i)+"\\2.jpg";
		for (int q = 10;q<=90;q+=10){
			std::string pathSave1 = path + std::to_string(i)+"\\1_q"+std::to_string(q)+".jpg";
			std::string pathSave2 = path + std::to_string(i)+"\\2_q"+std::to_string(q)+".jpg";
			testCreateJPEG(pathtmp1, pathSave1, q);
			testCreateJPEG(pathtmp2, pathSave2, q);
		}

	}
}

int main(){
	//test3();
	test1(90);
	return 0;
}
