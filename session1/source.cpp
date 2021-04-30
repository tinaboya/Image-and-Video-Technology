#include <fstream>
#include <iostream>
#include <cmath>
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
		char * buffer = new char[length];

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

//PSNR function
float psnr(float*I, float*K)
{
	int i;
	float MSE = 0, I_max = 0, PSNR;
	for (i = 0; i < 256 * 256; i++)
	{
		MSE = MSE + (I[i] - K[i])*(I[i] - K[i]);
	}
	MSE = MSE / (256 * 256);
	cout << "MSE:" << MSE << endl; //output MSE
	PSNR = 10 * log10(255*255 / MSE);
	return PSNR;
}

int main()
{
	//Define variebles
	float I[256 * 256];
	float lena_new[256 * 256];
	int x, y, i;
	double pi = 3.14, PSNR;

	//Generate image with fomula
	for (x = 0; x < 256; x++)
	{
		for (y = 0; y < 256; y++)
		{
			I[x + y * 256] = 0.5 + 0.5*cos(x*pi / 32)*cos(y*pi / 64);
		}
	}
	store(I,"GrayscaleImage"); //store the image

	//Load image lena
	float*lena = load("lena_256x256.raw");

	//Add two images together to get lena_new
	for (i = 0; i < 256 * 256; i++)
	{
		lena_new[i] = I[i] * lena[i];
	}
	store(lena_new,"Modified_lena");

	//Caculate PSNR and MSE between the original image and the new image
	PSNR = psnr(I, lena_new);

	//output PSNR
	cout << "PSNR:" << PSNR << endl;

	//make a pause for the screen to stay
	char a;
	cin >> a;

	return 0;
}
