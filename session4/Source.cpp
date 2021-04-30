#define _USE_MATH_DEFINES
#include <fstream>
#include <iostream>
#include <cmath>
#include <stdlib.h>
#include <random>

using namespace std;

// Store function
int store(float* I, int w,string name)
{
	int size = w * w;
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

// Matrix multiplication
float* multiplication(float* a, float* b,int length)
{
	int i, j, k;
	float* result = new float[length * length];
	for (i = 0; i < length; i++)
	{
		for (j = 0; j < length; j++)
		{
			result[length * i + j] = 0;
			for (k = 0; k < length; k++)
			{
				result[length * i + j] += a[length * i + k] * b[k * length + j];
			}
		}
	}
	return result;
}

// Matrix transpose
float* transpose(float* a,int length)
{
	int i, j;
	float* result = new float[length * length];
	for (i = 0; i < length; i++)
	{
		for (j = 0; j < length; j++)
		{
			result[length * i + j] = a[length * j + i];
		}
	}
	return result;
}

// Transform function
float* transform(float* a, float* b,int length) // (image, basis)
{
	int i, j, k;
	float* result = new float[length * length];
	float* c = new float[length * length];
	c = multiplication(a, b,length);
	result = multiplication(transpose(c,length), b,length);
	return result;
}

// Generate DCT matrix
float* DCTgenerator(int l)
{
	int p, q;
	float* DCT = new float[l*l];
	float* a = new float[l*l];
	for (p = 0; p < l; p++)
	{
		a[p] = sqrt(2./l);
	}
	a[0] = sqrt(1./l);

	for (p = 0; p < l; p++)
	{
		for (q = 0; q < l; q++)
		{
			DCT[p * l + q] = float(a[p]) * cosf((q + 0.5f) * p * float(M_PI) / l);
		}
	}
	return DCT;
}

// Approximate function
float* approximate(float* image, float* Q) //void, for outputing image for each step
{
	int i, j, m,n;
	float* DCT = new float[8 * 8];
	float* image_8x8 = new float[8 * 8];
	float* image_8x8_DCT = new float[8 * 8];
	float* image_8x8_Q = new float[8 * 8];
	float* image_8x8_IQ = new float[8 * 8];
	float* image_8x8_IDCT = new float[8 * 8];
	float* image_DCT = new float[256 * 256];
	float* image_Q = new float[256 * 256];
	float* image_IQ = new float[256 * 256];
	float* image_IDCT = new float[256 * 256];

	DCT = DCTgenerator(8);

	for (i = 0;i < 32;i++)
	{
		for (j = 0;j < 32;j++)
		{

			for (m = 0;m < 8;m++)
			{
				for (n = 0;n < 8;n++)
				{
					image_8x8[8*m + n] = image[256 * ((8 * i) + m) + (8 * j+n)];
				}
			}

			//DCT
			image_8x8_DCT = transform(image_8x8,DCT,8);
			for (m = 0;m < 8;m++)
			{
				for (n = 0;n < 8;n++)
				{
					image_DCT[256 * ((8 * i) + m) + (8 * j + n)] = image_8x8_DCT[8 * m + n]; //DCT image
				}
			}
			//Quantization
			for (m = 0;m < 8;m++)
			{
				for (n = 0;n < 8;n++)
				{
					image_8x8_Q[8 * m + n] = round(image_8x8_DCT[8 * m + n]/Q[8*m + n]);
					image_Q[256 * ((8 * i) + m) + (8 * j + n)] = image_8x8_Q[8 * m + n]; // Quantized image
				}
			}

			//Inverse Quantization
			for (m = 0;m < 8;m++)
			{
				for (n = 0;n < 8;n++)
				{
					image_8x8_IQ[8 * m + n] = image_8x8_Q[8 * m + n]*Q[8 * m + n];
					image_IQ[256 * ((8 * i) + m) + (8 * j + n)] = image_8x8_IQ[8 * m + n]; // Inverse Quantized image
				}
			}

			//IDCT
			image_8x8_IDCT = multiplication(transpose(multiplication(image_8x8_IQ, transpose(DCT,8),8),8), transpose(DCT,8),8);
			for (m = 0;m < 8;m++)
			{
				for (n = 0;n < 8;n++)
				{
					image_IDCT[256 * ((8 * i) + m) + (8 * j + n)] = image_8x8_IDCT[8 * m + n]; // IDCT image
				}
			}
		}
	}

	//store(image_DCT,256,"Image_DCT");
	//store(image_Q, 256,"Image_Q");
	//store(image_IQ, 256, "image_IQ");
	//store(image_IDCT, 256,"image_IDCT");
	return image_IDCT;
}

// clip function
float* clip(float* img, int l)
{
	int i;
	float* clipped_img = new float[l];
	for (i = 0; i < l; i++)
	{
		if (img[i] >= 255)
		{
			clipped_img[i] = 255; //max 255
		}
		else if (img[i] <= 0)
		{
			clipped_img[i] = 0; //min 0
		}
		else
		{
			clipped_img[i] = round(img[i]);
		}
	}
	return clipped_img;
}

//Encode function
float* encode(float* image, float* Q)
{
	int i, j, m, n;
	float* DCT = new float[8 * 8];
	float* image_8x8 = new float[8 * 8];
	float* image_8x8_DCT = new float[8 * 8];
	float* image_8x8_Q = new float[8 * 8];
	float* image_DCT = new float[256 * 256];
	float* image_Q = new float[256 * 256];

	DCT = DCTgenerator(8);

	for (i = 0;i < 32;i++)
	{
		for (j = 0;j < 32;j++)
		{

			for (m = 0;m < 8;m++)
			{
				for (n = 0;n < 8;n++)
				{
					image_8x8[8 * m + n] = image[256 * ((8 * i) + m) + (8 * j + n)];
				}
			}

			//DCT
			image_8x8_DCT = transform(image_8x8, DCT, 8);
			for (m = 0;m < 8;m++)
			{
				for (n = 0;n < 8;n++)
				{
					image_DCT[256 * ((8 * i) + m) + (8 * j + n)] = image_8x8_DCT[8 * m + n]; //DCT image
				}
			}

			//Quantization
			for (m = 0;m < 8;m++)
			{
				for (n = 0;n < 8;n++)
				{
					image_8x8_Q[8 * m + n] = round(image_8x8_DCT[8 * m + n] / Q[8 * m + n]);
					image_Q[256 * ((8 * i) + m) + (8 * j + n)] = image_8x8_Q[8 * m + n]; // Quantized image
				}
			}
		}
	}
	return image_Q; //return encoded image
}

//Decode function
float* decode(float* image, float* Q) //(encoded_image, Q)
{
	int i, j, m, n;
	float* DCT = new float[8 * 8];
	float* image_8x8 = new float[8 * 8];
	float* image_8x8_IQ = new float[8 * 8];
	float* image_8x8_IDCT = new float[8 * 8];
	float* image_IQ = new float[256 * 256];
	float* image_IDCT = new float[256 * 256];

	DCT = DCTgenerator(8);

	for (i = 0;i < 32;i++)
	{
		for (j = 0;j < 32;j++)
		{

			for (m = 0;m < 8;m++)
			{
				for (n = 0;n < 8;n++)
				{
					image_8x8[8 * m + n] = image[256 * ((8 * i) + m) + (8 * j + n)];
				}
			}
			//Inverse Quantization
			for (m = 0;m < 8;m++)
			{
				for (n = 0;n < 8;n++)
				{
					image_8x8_IQ[8 * m + n] = image_8x8[8 * m + n] * Q[8 * m + n];
					image_IQ[256 * ((8 * i) + m) + (8 * j + n)] = image_8x8_IQ[8 * m + n]; // Inverse Quantized image
				}
			}

			//IDCT
			image_8x8_IDCT = multiplication(transpose(multiplication(image_8x8_IQ, transpose(DCT, 8), 8), 8), transpose(DCT, 8), 8);
			for (m = 0;m < 8;m++)
			{
				for (n = 0;n < 8;n++)
				{
					image_IDCT[256 * ((8 * i) + m) + (8 * j + n)] = image_8x8_IDCT[8 * m + n]; // IDCT image
				}
			}
		}
	}
	return image_IDCT; //return decoded image
}

int main() {
	int i,l = 256;
	int* clipped_img = new int[l * l];
	float* image_IDCT = new float[l * l];
	float* encoded_image = new float[l * l];
	float* decoded_image = new float[l * l];
	float* clipped_image = new float[l * l];

	float Q[8 * 8] = {16,11,10,16,24,40,51,61,12,12,14,19,26,58,60,55,14,13,16,24,40,57,69,56,14,17,22,29,51,87,80,62,18,22,37,56,68,109,103,77,24,35,55,64,81,104,113,92,49,64,78,87,103,121,120,101,72,92,95,98,112,100,103,99};
	store(Q,8,"8x8_Q");

	float* lena = load("lena_256x256.raw"); //load image
	image_IDCT = approximate(lena, Q);
	clipped_image = clip(image_IDCT,256*256);
	store(clipped_image,256,"image_IDCT_clipped");

	encoded_image = encode(lena, Q);
	store(encoded_image,256,"encoded_image");

	decoded_image = decode(encoded_image, Q);
	store(decoded_image, 256,"decoded_image");

	return 0;
}
