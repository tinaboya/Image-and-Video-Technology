#define _USE_MATH_DEFINES
#include <fstream>
#include <iostream>
#include <cmath>
#include <stdlib.h>
#include <random>
#include <vector>
#include <string>
#include <map>

using namespace std;

// Store function
int store(float* I, int w, string name)
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
float* multiplication(float* a, float* b, int length)
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
float* transpose(float* a, int length)
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
float* transform(float* a, float* b, int length) // (image, basis)
{
	int i, j, k;
	float* result = new float[length * length];
	float* c = new float[length * length];
	c = multiplication(a, b, length);
	result = multiplication(transpose(c, length), b, length);
	return result;
}

// Generate DCT matrix
float* DCTgenerator(int l)
{
	int p, q;
	float* DCT = new float[l * l];
	float* a = new float[l * l];
	for (p = 0; p < l; p++)
	{
		a[p] = sqrt(2. / l);
	}
	a[0] = sqrt(1. / l);

	for (p = 0; p < l; p++)
	{
		for (q = 0; q < l; q++)
		{
			DCT[p * l + q] = float(a[p]) * cosf((q + 0.5f) * p * float(M_PI) / l);
		}
	}
	return DCT;
}

//Create 32x32 pixels image from the quantized DC terms
float* encode(float* image, float* Q)
{
	int i, j, m, n;
	float* DCT = new float[8 * 8];
	float* image_8x8 = new float[8 * 8];
	float* image_8x8_DCT = new float[8 * 8];
	float* image_32 = new float[32 * 32];

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

			//Quantization
			image_32[32*i+j] = round(image_8x8_DCT[0] / Q[0]); //DC term
		}
	}
	return image_32; //return 32 bits image
}

//Create a text file of differences
void difference(float* img,string name) {
	int* d = new int[32*32];
	int i;
	for (i = 0;i<32*32;i++) {
		if (i == 0)
			d[i] = (int)img[i];
		else
			d[i] = (int)img[i] - (int)img[i - 1];
	}
	ofstream out(name);
	for (int i = 0; i < 32*32; i++)
		out << d[i] << " ";
	out.close();
}

//Reconstruct the low-definition image
float* reconstruct(int* d) {
	int i;
	float* reconstructed_image = new float[32 * 32];
	for (i = 0;i < 32 * 32;i++) {
		if (i == 0)
			reconstructed_image[i] = (float)d[i];
		else
			reconstructed_image[i] = (float)d[i] + reconstructed_image[i - 1];
	}
	return reconstructed_image;
}

//Read the text file
int* readtext(string name) {
	int* d = new int[32 * 32];
	int coun = 0;
	ifstream inputFile; 
	inputFile.open(name);

	//read the numbers to the array
	while (coun<32 * 32 && inputFile >> d[coun]) {
		coun ++;
	}
	inputFile.close();//close the file
	return d;
}

//zig zag ordering
float* zigzag(float* matrix, int SIZE) {
	int i=0,j=0,x,y;
	float* new_matrix = new float[SIZE * SIZE];
	if(i < SIZE|| j < SIZE)
	for (x = 0; x < SIZE; x++)
		for (y = 0; y < SIZE; y++){

			new_matrix[SIZE * x + y] = matrix[SIZE * i + j];

			if ((i == SIZE - 1 || i == 0) && j % 2 == 0){
				j++;
				continue;
			}

			if ((j == 0 || j == SIZE - 1) && i % 2 == 1){
				i++;
				continue;
			}

			if ((i + j) % 2 == 0){
				i--;
				j++;
			}
			else if ((i + j) % 2 == 1){
				i++;
				j--;
			}
		}

	return new_matrix;
}

//run length encoding
vector<float> rle_en(float* matrix, int SIZE) {
	int count = 1,i;
	vector<float> coded_matrix; //declare a vector of floats
	for (i = 0;i < SIZE;i++)
	{	
		count = 1;
		while (matrix[i] == matrix[i + 1] && (i < SIZE-1)) {
		count++; //count the number of the same value
		i++; //compare the next element
		}
		coded_matrix.push_back(count); //store the count value to the matrix
		coded_matrix.push_back(matrix[i]); //store the number value
	}

	return coded_matrix;
}

//run length decoding
vector<float> rle_de(vector<float> matrix) {
	int i,count;
	vector<float> decoded_matrix;
	for (i = 0;i < size(matrix);i++){
		if (i % 2 == 0) {
			count = matrix[i];
			while (count > 0){
				decoded_matrix.push_back(matrix[i + 1]);
				count--;
			}
		}
	}
	return decoded_matrix;
}

//inverse zig zag ordering
float* izigzag(float* matrix, int SIZE) {
	int i = 0, j = 0, x, y;
	float* new_matrix = new float[SIZE * SIZE];
	if (i < SIZE || j < SIZE)
		for (x = 0; x < SIZE; x++)
			for (y = 0; y < SIZE; y++) {

				new_matrix[SIZE * i + j] = matrix[SIZE * x + y];

				if ((i == SIZE - 1 || i == 0) && j % 2 == 0) {
					j++;
					continue;
				}

				if ((j == 0 || j == SIZE - 1) && i % 2 == 1) {
					i++;
					continue;
				}

				if ((i + j) % 2 == 0) {
					i--;
					j++;
				}
				else if ((i + j) % 2 == 1) {
					i++;
					j--;
				}
			}

	return new_matrix;
}

//Encode function ac
float* extract_ac(float* image, float* Q)
{
	int i, j, m, n,k = 0;
	float* DCT = new float[8 * 8];
	float* image_8x8 = new float[8 * 8];
	float* image_8x8_DCT = new float[8 * 8];
	float* image_8x8_Q = new float[8 * 8];
	float* image_DCT = new float[256 * 256];
	float* AC_terms = new float[256 * 256-32*32];
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
				}
			}
			int p = 32 * i + j;
			for(int q = 0;q<63;q++) AC_terms[63*p+q] = image_8x8_Q[q + 1];
		}
	}	
	return AC_terms;
}

float* extract_ac_dc(float* image, float* Q)
{
	int i, j, m, n, k = 0;
	float* DCT = new float[8 * 8];
	float* image_8x8 = new float[8 * 8];
	float* image_8x8_DCT = new float[8 * 8];
	float* image_8x8_Q = new float[8 * 8];
	float* image_DCT = new float[256 * 256];
	float* image_Q = new float[256 * 256];
	float* AC_terms = new float[256 * 256];
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
	return image_Q;
}

vector<float> encode_ac(float* ac_terms)
{
	int i, j, m, n;
	float* ac_terms_zigzag = new float[256 * 256-32*32];
	float* ac_terms_zigzag_8x8 = new float[64];
	float* ac_terms_zigzag_8x8_1 = new float[63];
	float* ac_terms_8x8 = new float[64];
	vector<float> ac_terms_rle_en;
	vector<float> ac_terms_rle_en_8x8;
	for (i = 0;i < 32;i++){
		for (j = 0;j < 32;j++){
			for (m = 0;m < 8;m++){
				for (n = 0;n < 8;n++){
					ac_terms_8x8[8 * m + n] = ac_terms[256 * ((8 * i) + m) + (8 * j + n)];
				}
			}

			ac_terms_zigzag_8x8 = zigzag(ac_terms_8x8, 8);
			for (int k = 1;k < 64;k++) ac_terms_zigzag_8x8_1[k-1] = ac_terms_zigzag_8x8[k]; //delete dc terms, only rle ac terms
			ac_terms_rle_en_8x8 = rle_en(ac_terms_zigzag_8x8_1, 63);

			for (m = 0;m < size(ac_terms_rle_en_8x8);m++) {
				ac_terms_rle_en.push_back(ac_terms_rle_en_8x8[m]); 
			}
		}
	}
	return ac_terms_rle_en;
}

float* decode_ac(vector<float> ac_encoded)
{
	int i, j, m, n,p,q,k = 0;
	float* ac_terms_izigzag_8x8 = new float[8 * 8];
	float* ac_terms_izigzag = new float[256 * 256];
	float* real_ac_terms = new float[256 * 256-32*32];
	float* b_8x8 = new float[8 * 8];

	vector<float> ac_terms_rle_de;
	ac_terms_rle_de = rle_de(ac_encoded);
	float* b = new float[size(ac_terms_rle_de)];
	b = &ac_terms_rle_de[0];

	for (i = 0;i < 32;i++) {
		for (j = 0;j < 32;j++) {
			p = 32 * i + j;
			for (q = 0;q < 63;q++) {
				b_8x8[0] = 0; //prepare for izigzag
				b_8x8[q+1] = b[64 * p + q]; //ac terms
				ac_terms_izigzag_8x8 = izigzag(b_8x8, 8);
			}
			for (int q = 0;q < 63;q++) real_ac_terms[63 * p + q] = ac_terms_izigzag_8x8[q + 1];
		}
	}
	return real_ac_terms;
}

float entropy(float* matrix, int length) {
	float sum = 0;
	for (int i = 0;i < length;i++) {
		sum = sum + matrix[i] * log2(matrix[i]);
	}
	sum = -sum;
	return sum;
}

void text(float* matrix, int size, string name) {
	ofstream out(name);
	for (int i = 0; i < size; i++)
		out << matrix[i] << " ";
	out.close();
}

int main() {
	float Q[8 * 8] = { 16,11,10,16,24,40,51,61,12,12,14,19,26,58,60,55,14,13,16,24,40,57,69,56,14,17,22,29,51,87,80,62,18,22,37,56,68,109,103,77,24,35,55,64,81,104,113,92,49,64,78,87,103,121,120,101,72,92,95,98,112,100,103,99 };
	float* lena = load("lena_256x256.raw");

	//---dc terms---

	float* image_32 = new float[32 * 32];
	int* d = new int[32 * 32];
	float* reconstructed_image = new float[32 * 32];

	image_32 = encode(lena, Q);
	store(image_32, 32, "image32");
	difference(image_32,"d.txt");
	d = readtext("d.txt");
	reconstructed_image = reconstruct(d);
	store(reconstructed_image, 32, "image32reconstructed");
	
	//---ac terms---
	float* ac_dc_terms = new float[256 * 256]; //for zigzag
	float* ac_terms = new float[256 * 256-32*32]; //the true ac terms
	vector<float> ac_encoded;
	float* ac_decoded = new float[256 * 256];
	
	ac_terms = extract_ac(lena,Q);
	ac_dc_terms = extract_ac_dc(lena, Q);
	//text(ac_terms, (256*256-32*32),"ACterms.txt");

	ac_encoded = encode_ac(ac_dc_terms);
	float* a = new float[size(ac_encoded)];
	a = &ac_encoded[0];
	//text(a, size(ac_encoded),"encodedACterms.txt");

	ac_decoded = decode_ac(ac_encoded);
	//text(ac_decoded, 256*256-32*32,"decodedACterms.txt");

	//---Histogram P for RLE symbol---
	int n = 0;
	float Psum = 0;
	float* Parray = new float[159];
	float* Parray_normalized = new float[159];

	std::map<float, int> P;
	for (int i = 0;i < size(ac_encoded);i++)
	{
		if (P.find(a[i]) != P.end())
		{
			//key exist and hence update the dulpicate count;
			P[a[i]]++;
		}
		else {
			//it doesn't exist in map
			P.insert(std::pair<float, int>(a[i], 1));//single count of current number
		}
	}
	for (std::map<float, int>::iterator it = P.begin();it != P.end();it++)
	{
		//cout << "\n Number " << it->first << " is repeated " << it->second << " times ";
		Parray[n] = it->second;
		n++;
	}
	//cout << "Longest run length N:" << n<<"\n";
	cout << "runs M:" << size(ac_encoded) <<"\n";
	int value = n;
	for (int i = 0;i < value;i++) Psum = Psum + Parray[i];
	for (int i = 0;i < value;i++) Parray_normalized[i] = Parray[i]/Psum;
	//text(Parray,value,"P array.txt");
	//text(Parray_normalized, value, "P array normalized.txt");

	//---find the entropy---
	float P_entropy = entropy(Parray_normalized,value);
	float bits_min = P_entropy* size(ac_encoded);
	//cout << "Entropy:" << P_entropy<< "\n";
	//cout << "min bits:" << bits_min << " bits\n";

	return 0;
}

//---test---

//float* test_out1 = new float[8 * 8];
//float* test_out4 = new float[8 * 8];
//float test_in1[8 * 8] = { 1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31,32,33,34,35,36,37,38,39,40,41,42,43,44,45,46,47,48,49,50,51,52,53,54,55,56,57,58,59,60,61,62,63,64 };
//float test_in2[3 * 3] = {4,4,2,2,2,3,3,3,5};

//zigzag test
//test_out1 = zigzag(test_in1, 8);
//cout << "test for zig-zag ordering:\n";
//for (int i = 0;i < 8;i++) {
	//for (int j = 0;j < 8;j++) {
		//cout << test_out1[i * 8 + j] << "\n";
	//}
//}

//run length encoding test
//vector<float> test_out2= rle_en(test_in2,3);
//cout << "test for rle encoding:\n";
//for (int i = 0;i < size(test_out2);i++) {
	//cout << test_out2[i] << "\n";
//}


//run length decoding test
//vector<float> test_out3= rle_de(test_out2);
//cout << "test for rle decoding:\n";
//for (int i = 0;i < size(test_out3);i++) {
	//cout << test_out3[i] << "\n";
//}

//inverse zig zag test
//test_out4 = izigzag(test_out1, 8);
//cout << "test for inverse zig-zag ordering:\n";
//for (int i = 0;i < 8;i++) {
	//for (int j = 0;j < 8;j++) {
		//cout << test_out4[i * 8 + j] << "\n";
	//}
//}