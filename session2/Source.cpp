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

// MSE caculation function
float mse(float* I)
{
	float MSE = 0, mean = 0;
	int i;
	for (i = 0; i < 256 * 256; i++)
	{
		mean = mean + I[i];
	}

	mean = mean / (256 * 256);

	for (i = 0; i < 256 * 256; i++)
	{
		MSE = MSE + (I[i] - mean) * (I[i] - mean);
	}
	//Output the MSE value
	MSE = MSE / (256 * 256);
	//cout << "MSE:" << MSE << endl;
	char a;
	cin >> a;
return 0;
}

//PSNR function
float psnr(float* I, float* K)
{
	int i;
	float MSE = 0, I_max = 0, PSNR;
	for (i = 0; i < 256 * 256; i++)
	{
		MSE = MSE + (I[i] - K[i]) * (I[i] - K[i]);
	}
	MSE = MSE / (256 * 256);
	//cout << "MSE:" << MSE << endl; //output MSE
	PSNR = 10 * log10(255 * 255 / MSE);
	return PSNR;
}

//blur function(not considering border)
float* blur(float* a, float* G, int n) // a is the rectangular region, G is the kernel
{
	float* result = new float[n * n];
	for (int i = 0; i < (n * n);i++)
	{
		result[i] = 0;
	}

	for (int i = 1;i < n-1;i++)
	{
		for (int j = 1;j < n-1;j++)
		{
			for (int p = -1;p < 2;p++)
			{
				for (int q = -1;q < 2;q++)
				{
					result[n * i + j] += a[n*(i+p)+(j+q)]* G[(p + 1) * 3 + (q + 1)];
				}
			}
		}
	}
	return result;
}

//blur function(considering the border)
//use the extending strategy
float* blur_wb(float* a, float* G, int n)
{
	float* result = new float[n * n];

	for (int i = 0; i < (n * n);i++)
	{
		result[i] = 0;
	}

	for (int i = 0;i < n ;i++)
	{
		for (int j = 0;j < n ;j++)
		{
			for (int p = -1;p < 2;p++)
			{
				for (int q = -1;q < 2;q++)
				{

					if (i == 0 || i == n - 1 || j == 0 || j == n - 1)
					{
						int u = p, v = q;
						if ((i + p) == -1) u = p + 1;
						if ((j + q) == -1) v = q + 1;
						if ((i + p) == n) u = p - 1;
						if ((j + q) == n) v = q - 1;
						result[n * i + j] += a[n * (i + u) + (j + v)] * G[(p + 1) * 3 + (q + 1)];
					}
					else result[n * i + j] += a[n * (i + p) + (j + q)] * G[(p + 1) * 3 + (q + 1)];
				}
			}
		}
	}
	return result;
}

int main()
{
	float* I_ND = new float[256 * 256];
	float* I_GD = new float[256 * 256];
	float* I_GD_new = new float[256 * 256];
	double PSNR_ND, PSNR_GD, PSNR_blured, PSNR_blured_wb;
	int i=0;

	// Normal distribution image
	for (i = 0; i < 256 * 256; i++)
	{
		I_ND[i] = rand() % 100 / (double)101 - 0.5;
	}

	//store(I_ND,"Uniform random white noise");
	//mse(I_ND);

	// Gaussian distribution image
	std::default_random_engine generator;
	std::normal_distribution<double> distribution(0.0, 12.75); //0.2875
	for (i = 0; i < 256 * 256; i++)
	{
		I_GD[i] = distribution(generator);
	}

	//store(I_GD,"Gaussian random white noise");
	//mse(I_GD);

	//load lena
	float*lena = load("lena_256x256.raw");

	//add gaussian random noise realization
	for (i = 0; i < 256 * 256; i++)
	{
		I_GD_new[i] = lena[i] + I_GD[i];
	}
	//store(I_GD_new,"lena_Gaussian");

	//calcuate PSNR
	PSNR_GD = psnr(I_GD_new, lena);
	cout << "PSNR of noisy image with gaussian random noise:" << PSNR_GD << endl;

	//Calculate values of the normalized 3×3 blur kernel with the standard normal distribution
	float* G = new float[3 * 3];
	for (i = 0; i < 9; i++)
	{
		G[i] = 0;
	}
	double sum = 0;
	for (int m = -1;m <= 1;m++)
	{
		for (int n = -1;n <= 1;n++)
		{
			G[(m + 1)*3 + (n + 1)] = double(exp(-double(m*m+n*n)/(2*1)));
			sum += G[(m + 1) * 3 + (n + 1)];
		}
	}
	cout << "G:"; //output G
	for (i = 0; i < 9; i++)
	{
		G[i] = G[i]/sum;
		cout << G[i] << endl; //output G
	}

	//Blur a rectangular region
	float* lena_blured = new float[256 * 256];
	lena_blured = blur(I_GD_new, G, 256);
	//store(lena_blured,"lena_Gaussian_partly_blurred");

	//Blur the whole image
	float* lena_blured_wb = new float[256 * 256];
	lena_blured_wb = blur_wb(I_GD_new, G, 256);
	//store(lena_blured_wb,"lena_Gaussian_fully_blurred");

	//Blur noisy image and calculate the PSNR
	PSNR_blured_wb = psnr(lena_blured_wb, lena);
	cout << "PSNR_blured_wb:" << PSNR_blured_wb << endl;

	return 0;
}
