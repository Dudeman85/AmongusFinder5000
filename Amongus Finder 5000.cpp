#include <iostream>
#include <filesystem>
#include <bitset>
#include "lodepng.h"

using namespace std;

vector<unsigned char> MatrixToRGBA(vector<vector<unsigned int>> mat);

int main()
{
	//Variables for reading in source image and template
	vector<unsigned char> source;
	unsigned width, height;

	//Load and decode PNG pixels into a vector with 4 bytes per pixel
	lodepng::decode(source, width, height, "source.png");

	//2D Vector storing every color of source
	vector<vector<unsigned int>> image;
	image.resize(width);
	for (unsigned i = 0; i < image.size(); i++)
		image[i].resize(height);

	//Convert the source pixel vector to an array with 1 byte per pixel
	for (int i = 0; i < source.size(); i += 4)
	{
		image[i / 4 % width][(int)floor(i / 4 / width)] = (((source[i]) << 24 | source[i + 1] << 16 | source[i + 2] << 8 | source[i + 3]));
	}

	//Vector for storing template
	vector<vector<vector<float>>> templates;
	vector<unsigned> templateWidths, templateHeights;

	//Get all template images from templates folder
	for (const auto& entry : filesystem::directory_iterator("templates")) 
	{
		vector<unsigned char> tempSource;
		unsigned tempWidth, tempHeight;

		//Load amongus
		lodepng::decode(tempSource, tempWidth, tempHeight, entry.path().string());

		//Vectors for template and mirrored template data
		vector<vector<float>> temp, mirroredTemp;
		temp.resize(tempWidth);
		mirroredTemp.resize(tempWidth);
		for (unsigned i = 0; i < tempWidth; i++)
		{
			temp[i].resize(tempHeight);
			mirroredTemp[i].resize(tempHeight);
		}

		//Converts the template pixel vector to a 2D vector pattern for matching
		for (int i = 0; i < tempSource.size(); i += 4)
		{
			float* pixel = &temp[i / 4 % tempWidth][(int)floor(i / 4 / tempWidth)];

			if ((int)tempSource[i] == 255) //White is ignored
				*pixel = 0;
			else if ((int)tempSource[i] == 0) //Black must be different from reds
				*pixel = 2;
			else  //reds are important ranked by shade where darker(lower) is more important
				*pixel = 1; //(float)tempSource[i] / 255;
		}

		//Mirror the template
		for (unsigned x = 0; x < tempWidth; x++)
			for (unsigned y = 0; y < tempHeight; y++)
				mirroredTemp[x][y] = temp[tempWidth - x - 1][y];

		//Add normal and mirrored templates to list
		templateHeights.push_back(tempHeight);
		templateWidths.push_back(tempWidth);
		templates.push_back(temp);
		templates.push_back(mirroredTemp);
	}

	//Print the templates
	for (int i = 0; i < templates.size(); i++)
	{
		for (int y = 0; y < templates[i][0].size(); y++)
		{
			for (int x = 0; x < templates[i].size(); x++)
			{
				cout << (char)(178 - templates[i][x][y]) << (char)(178 - templates[i][x][y]);
			}
			cout << endl;
		}
		cout << endl;
	}

	//Pixel matrix for storing each amongus in a catalogue
	int catalogueWidth = 40;
	vector<vector<unsigned int>> catalogue;
	catalogue.resize(catalogueWidth * (templateWidths[0] + 1));
	for (unsigned i = 0; i < catalogue.size(); i++)
		catalogue[i].resize(height);

	//Create a copy of the image to draw boxes around hits
	vector<vector<unsigned int>> imageCopy(image);

	int color = 0;
	int hits = 0;

	//Loop through every pixel and match it to each template
	for (unsigned x = 0; x < width - templateWidths[0] + 3; x++)
	{
		for (unsigned y = 0; y < height - templateHeights[0] + 3; y++)
		{
			//Get the color to match to 1 from a known pixel
			color = image[x + 2][y + 1];

			//Check every template for a match
			for (int i = 0; i < templates.size(); i++)
			{
				//Loop through every position of the template
				for (int xi = 0; xi < templates[i].size(); xi++)
				{
					for (int yi = 0; yi < templates[i][xi].size(); yi++)
					{
						//Check that the pixel is in bounds
						if (x + xi > width || y + yi > height)
							continue;

						//If the pixel does not match the template pattern continue to next pattern
						if (templates[i][xi][yi] == 1 && image[x + xi][y + yi] != color)
							goto miss;
						if (templates[i][xi][yi] == 2 && image[x + xi][y + yi] == color)
							goto miss;
					}
				}
				//If the position matched a template, copy the pixels to the catalogue
				for (int xi = 0; xi < templates[i].size(); xi++)
				{
					for (int yi = 0; yi < templates[i][xi].size(); yi++)
					{
						if (x + xi < width && y + yi < height)
						{
							//Copy to catalogue
							catalogue[hits * (templateWidths[0] + 1) - floor(hits / catalogueWidth) * catalogueWidth * (templateWidths[0] + 1) + xi][floor(hits / catalogueWidth) * (templateHeights[0] + 1) + yi] = image[x + xi][y + yi];

							//Draw a red outline around the detection if it's in bounds
							if (xi == 0 || yi == 0 || xi == templateWidths[0] - 1 || yi == templateHeights[0] - 1)
								imageCopy[x + xi][y + yi] = 0xFF0000FF;
						}
					}
				}
				//Increment hit counter and continue to the next pixel position
				hits++;
				break;
			miss:
				continue;
			}
		}
		if (x % 160 == 0)
			cout << ". ";
	}

	cout << "\n\nFound " << hits << " amonguses";
	cout << "\n\nSaving Results to \"results.png\" and \"catalogue.png\"\n";

	//Convert RGBA matrixes back to vector
	vector<unsigned char> fCatalogue = MatrixToRGBA(catalogue);
	vector<unsigned char> fResults = MatrixToRGBA(imageCopy);

	//Output results to a PNG
	lodepng::encode("catalogue.png", fCatalogue, catalogueWidth * (templateWidths[0] + 1), floor(hits / catalogueWidth)* (templateHeights[0] + 1) + templateHeights[0]);
	lodepng::encode("results.png", fResults, width, height);

	cout << endl;
	system("pause");
}

//Convert a hex color matrix to a 1D vector of RGBA chars for lodepng
vector<unsigned char> MatrixToRGBA(vector<vector<unsigned int>> mat)
{
	vector<unsigned char> results;
	for (int y = 0; y < mat[0].size(); y++)
	{
		for (int x = 0; x < mat.size(); x++)
		{
			for (int i = 3; i >= 0; i--)
			{
				results.push_back((mat[x][y] & ((1 << (8)) - 1) << (i * 8)) >> (i * 8));
			}
		}
		if (y % 320 == 0)
			cout << ". ";
	}
	return results;
}