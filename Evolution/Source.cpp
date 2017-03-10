#include <iostream>
#include <cmath>
#include "opencv2/imgcodecs.hpp"
#include "opencv2/highgui.hpp"
#include "opencv2/imgproc.hpp"
#include <random>
#include <chrono>
#include <array>

using namespace std;
using namespace cv;
unsigned short w = 1300;
unsigned short h = 800;



struct animalStruct {
	Point location;
	Vec3b colour;
	unsigned _int8 speed; //0-255
	unsigned _int16 sporeRadius; //0-65k
	bool male; //
	unsigned _int8 pregnant; //0-255
};

vector<animalStruct> animals;

void addAnimal(Point location, Vec3b colour, unsigned _int8 speed, unsigned _int8 sporeRadius, bool male, unsigned _int8 pregnant)
{
	animals.push_back(animalStruct());
	animals.back().location = location;
	animals.back().colour = colour;
	animals.back().speed = speed;
	animals.back().sporeRadius = sporeRadius;
	animals.back().male = male;
	animals.back().pregnant = pregnant; //if 0, not preganant.

}

unsigned female(unsigned i, unsigned j)
{
	if (animals[i].male == true)
		return j;
	else
		return i;
}

bool isVacant(Point p)
{
	for (unsigned i = 0; i < animals.size(); i++)
	{
		if (animals[i].location == p)
		{
			return false;
		}
	}
	return true;
}

void mateAnimals(unsigned i, unsigned j, bool spore)
{
	const unsigned noOfDNA = 6; // number of variables in the DNA chain
	
	// construct a trivial random generator engine from a time-based seed:
	unsigned seed = chrono::system_clock::now().time_since_epoch().count();
	default_random_engine generator(seed);

	uniform_int_distribution<unsigned> distribution(0, 999999999);
	unsigned randNum = distribution(generator);
	
	bool dna[noOfDNA];
	//for (auto& k : dna) 
	for (unsigned k = 0; k<noOfDNA; k++)
	{
		dna[k] = randNum / (k+1) % 2;
	}
	// create the animal
	animals.push_back(animalStruct());
	
	// fill in its details

	/*	location,
		allways follow the mother, 
		if spore == true then it has been polinated then the location will be random within sporeRadius
		if spore == false then grow next to it if free
	*/

	if (spore == true)
	{
		uniform_int_distribution<int> randPoint(-animals[female(i, j)].sporeRadius, animals[female(i, j)].sporeRadius);
		int randPointx = randPoint(generator);
		animals.back().location.x = animals[female(i, j)].location.x + randPointx;
		int randPointy = randPoint(generator);
		animals.back().location.y = animals[female(i, j)].location.y + randPointy;
	}
	else if (animals[female(i, j)].speed == 0)
	{
		uniform_int_distribution<int> randPoint(-1, 1);
		int randPointx = randPoint(generator);
		animals.back().location.x = animals[female(i, j)].location.x + randPointx;
		int randPointy = randPoint(generator);
		animals.back().location.y = animals[female(i, j)].location.y + randPointy;
		if (isVacant(animals[animals.size() - 1].location) == true)
		{
			animals.pop_back();
		}
	}

	if (dna[0]==true)
		animals.back().location = animals[i].location;
	else
		animals.back().location = animals[j].location;

	if (dna[1] == true)
		animals.back().colour = animals[i].colour;
	else
		animals.back().colour = animals[j].colour;
	if (dna[1] == true)
		animals.back().speed = animals[i].speed;
	else
		animals.back().speed = animals[j].speed;
	
	// sex directly from random bool
	animals.back().speed = dna[1];
	
	//never born pregnant
	animals.back().pregnant = 0;
	
	//cout << dna[0] << ", " << dna[9] << endl;

}

void moveAnimal(unsigned int i)
{
	int x1 = animals[i].location.x;
	int y1 = animals[i].location.y;
	double dist = 999999999999999999;
	unsigned int closest;
	for (unsigned int j = 0; j < animals.size(); j++)
	{
		//mateAnimals(i, j);
		if (j == i)
			continue;
		int x2 = animals[j].location.x;
		int y2 = animals[j].location.y;
		double dist_temp = sqrt(pow((y2 - y1),2) + pow((x2 - x1),2));
		if (abs(dist_temp)<abs(dist))
		{ 
			closest = j;
			dist = dist_temp;
		}
	}

	double xmultiplier = (animals[closest].location.x - x1) / dist;
	double ymultiplier = (animals[closest].location.y - y1) / dist;
	if (dist < animals[i].speed)
		animals[i].location = animals[i].location + Point(xmultiplier * dist, ymultiplier * dist);
	else
		animals[i].location = animals[i].location + Point(xmultiplier * animals[i].speed, ymultiplier * animals[i].speed);
	
}

int main()
{
	Rect scenebox(0, 0, w, h);
	Mat worldMap = cv::Mat(h, w, CV_8UC3, Vec3b(255,255,255));
	circle(worldMap, Point(0, 0), 700, Vec3b(255, 0, 0), -1);
	addAnimal(Point(200, 200), Vec3b(0, 200, 255),10, 1, 0, 0);
	addAnimal(Point(1000, 500), Vec3b(100, 0, 255),10, 0, 0, 0);
	//addAnimal(Point(400, 500), Vec3b(0, 50, 255), 5, 1, 0);
	//addAnimal(Point(700, 400), Vec3b(100, 100, 255), 3, 0, 0);
	namedWindow("World Map", CV_WINDOW_AUTOSIZE);
	while (1)
	{
		Mat viewMap;
		worldMap.copyTo(viewMap);
		for (unsigned int i = 0; i < animals.size(); i++)
		{
			moveAnimal(i);
			circle(viewMap, animals[i].location, 3, animals[i].colour, -1);
		}
		imshow("World Map", viewMap);
		waitKey(1000);
	}
	
}