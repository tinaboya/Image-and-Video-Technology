#define _USE_MATH_DEFINES
#include <fstream>
#include <iostream>
#include <cmath>
#include <stdlib.h>
#include <random>

using namespace std;

// Store function
int store(float* I,string name)
{
	int size = 256 * 256;
	ofstream outfile(name + ".raw", ofstream::binary);
	outfile.write((const char*)I, size * sizeof(float));
	outfile.close();
	return outfile.bad();
}

// Load function
float* load(const char* filename)
{
	std::ifstream is(filename, std::ifstream::binary);
	if (is) {
		// get length of file:
		is.seekg(0, is.end);
		int length = is.tellg();
		is.seekg(0, is.beg);
		char* buffer = new char[length];

		std::cout << "Reading " << length << " characters... ";
		// read data as a block:
		is.read(buffer, length);

		if (is)
			std::cout << "all characters read successfully.";
		else
			std::cout << "error: only " << is.gcount() << " could be read";
		is.close();
		// ...buffer contains the entire file...
		return (float*)buffer;
	}
	return 0;
}

// Matrix transpose
float* transpose(float* a)
{
	int i, j;
	float* result = new float[256*256];
	for (i = 0; i<256; i++)
	{
		for (j = 0; j<256; j++)
		{
			result[256 *i + j] = a[256 *j + i];
		}
	}
	return result;
}


// Matrix multiplication
float* multiplication(float* a, float* b)
{
	int i, j, k;
	float* result = new float[256*256];
	for (i = 0; i<256; i++)
	{
		for (j = 0; j<256; j++)
		{
			result[256*i+j] = 0;
			for (k = 0; k<256; k++)
			{
				result[256*i + j] += a[256 * i + k]*b[k*256 + j];
			}
		}
	}
	return result;
}

// Transform function
float* transform(float* a, float* b) // (image, basis)
{
	int i, j, k;
	float* result = new float[256*256];
	float* c = new float[256 * 256];
	c = multiplication(a, b);
	result = multiplication(transpose(c), b);
	return result;
}

// Threshold function
float* threshold(float* a,double thres)
{
	int i;
	float* result = new float[256 * 256];
	for (i = 0; i < 256 * 256; i++)
	{
		if (abs(a[i]) < thres) {
			result[i] = 0;
		}
		else result[i] = a[i];
	}
	return result;
}

//PSNR function
float psnr(float* I, float* K, int l)
{
	int i;
	float MSE = 0, I_max = 0, PSNR;
	for (i = 0; i < l * l; i++)
	{
		MSE = MSE + (I[i] - K[i]) * (I[i] - K[i]);
	}
	MSE = MSE / (l * l);
	//cout << "MSE:" << MSE << endl; //output MSE
	PSNR = 10 * log10((l-1) * (l-1) / MSE);
	return PSNR;
}

int main()
{
	int i, j, PSNR1, PSNR10, PSNR100;
	float DCT[256*256], a[256];
	float* result_i = new float[256 * 256];
	float* result_t = new float[256 * 256];
	float* result_th1 = new float[256 * 256];
	float* result_th10 = new float[256 * 256];
	float* result_th100 = new float[256 * 256];
	float* result_reconstruct1 = new float[256 * 256];
	float* result_reconstruct10 = new float[256 * 256];
	float* result_reconstruct100 = new float[256 * 256];

	//Generate DCT matrix
	for (i = 0; i < 256; i++)
	{
		a[i] = sqrt(2 / 256.);
	}
	a[0] = sqrt(1 / 256.);

	for (i = 0; i < 256.; i++)
	{
		for (j = 0; j < 256.; j++)
		{
			DCT[i*256+j] = float(a[i]) * cosf((j + 0.5f) * i * float(M_PI) / 256.f);
		}
	}

	result_i = multiplication(DCT,transpose(DCT)); 	//Check if DCT*DCT(transpose) = identity matrix
	//store(result_i,"IdentityMatrix");

	float* lena = load("lena_256x256.raw"); //load image
	result_t = transform(lena, DCT); //perform DCT on image
	//store(result_t,"DCT coefficients from the image of Lena");

	result_th1 = threshold(result_t,1); //threshold is 1
	result_th10 = threshold(result_t, 10); //threshold is 10
	result_th100 = threshold(result_t, 100); //threshold is 100

	//perform IDCT on thresholded DCT image
	result_reconstruct1 = multiplication(transpose(multiplication(result_th1, transpose(DCT))), transpose(DCT));
	//store(result_reconstruct1,"result_reconstruct");
	result_reconstruct10 = multiplication(transpose(multiplication(result_th10, transpose(DCT))), transpose(DCT));
	//store(result_reconstruct10,"result_reconstruct10");
	result_reconstruct100 = multiplication(transpose(multiplication(result_th100, transpose(DCT))), transpose(DCT));
	//store(result_reconstruct100,"result_reconstruct100");

    //Calculate PSNR
	PSNR1 = psnr(result_reconstruct1, lena, 256);
	PSNR10 = psnr(result_reconstruct10, lena, 256);
	PSNR100 = psnr(result_reconstruct100, lena, 256);

	//cout << PSNR1 << endl;
	//cout << PSNR10 << endl;
	//cout << PSNR100 << endl;
	return 0;
}
