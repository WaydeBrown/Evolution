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

// construct a trivial random generator engine from a time-based seed:
unsigned seed = chrono::system_clock::now().time_since_epoch().count();
default_random_engine generator(seed);

struct animalStruct {
	Point location;
	Vec3b colour;
	unsigned _int8 speed; //0-255
	unsigned _int16 sporeRadius; //0-65k
	bool male; //
	unsigned _int8 pregnant; //0-255
	unsigned _int8 reproductionPersistance; //0-255
	int theFather; //a spot to saave the name of the father for pregnant animals
};

vector<animalStruct> animals;

void addAnimal(
	Point location, 
	Vec3b colour, 
	unsigned _int8 speed, 
	unsigned _int8 sporeRadius, 
	bool male, 
	unsigned _int8 reproductionPersistance
	)
{
	animals.push_back(animalStruct());
	animals.back().location = location;
	animals.back().colour = colour;
	animals.back().speed = speed;
	animals.back().sporeRadius = sporeRadius;
	animals.back().male = male;
	animals.back().pregnant = 0; //if 0, not preganant.
	animals.back().reproductionPersistance = reproductionPersistance;
	animals.back().theFather=-1;

}

int female(int i, int j)
{
	if (animals[i].male == false && animals[j].male == true)
		return i;
	else if (animals[i].male == true && animals[j].male == false)
		return j;
	else
		return -1;
}

int male(int i, int j)
{
	if (animals[i].male == true && animals[j].male == false)
		return i;
	else if (animals[i].male == false && animals[j].male == true)
		return j;
	else
		return -1;
}

int isVacant(Point p, int i =-1)
{
	for (int j = 0; j < animals.size(); j++)
	{
		if (i == j)
			continue;
		if (animals[j].location == p)
		{
			return j;
		}
	}
	return -1;
}
// find a random location within radiusStart
Point freeLocation(Point p, unsigned attempts, unsigned radiusStart, unsigned maxRadius)
{
	for (int i = radiusStart; i < maxRadius+1; i++)
	{
		for (int j = 0; j < attempts; j++)
		{
			uniform_int_distribution<int> randPoint(-i, i);
			Point trialPoint;
			trialPoint.x = p.x + randPoint(generator);
			trialPoint.y = p.y + randPoint(generator);
			if (isVacant(trialPoint) > -1)
			{
				return trialPoint;
			}
		}
	}
	return Point(0, 0);
}

void mateAnimals(int i, int j, bool spore)
{
	const unsigned noOfDNA = 6; // number of variables in the DNA chain
	
	uniform_int_distribution<unsigned> distribution(0, 999999999);
	unsigned randNum = distribution(generator);

	int theFemale = female(i, j);
	
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

	if (spore == true) // if it has been fertilized by spores then the femal has seeds that can also fly
	{
		// find a random location within sporeRadius
		Point newPoint = freeLocation(animals.back().location, animals[theFemale].reproductionPersistance, animals[theFemale].sporeRadius, animals[theFemale].sporeRadius);
		
		// if the seed lands where someone is already living -> too bad, its not going to grow.
		if (newPoint == Point(0, 0))
		{
			animals.pop_back();
			return;
		}
		else
			animals.back().location = newPoint;
	}
	else 
	{
		// find a random location within sporeRadius
		
		Point newPoint = freeLocation(animals[theFemale].location, animals[theFemale].reproductionPersistance, 0, animals[theFemale].sporeRadius);

		// if we run out of attempts without finding a free spot then we die.
		if (newPoint == Point(0, 0))
		{
			animals.pop_back();
			return;
		}
		else
			animals.back().location = newPoint;
	}

	if (dna[0] == true)
		animals.back().colour = animals[i].colour;
	else
		animals.back().colour = animals[j].colour;
	if (dna[1] == true)
		animals.back().speed = animals[i].speed;
	else
		animals.back().speed = animals[j].speed;
	
	if (dna[2] == true)
		animals.back().sporeRadius = animals[i].sporeRadius;
	else
		animals.back().sporeRadius = animals[j].sporeRadius;
	
	// sex directly from random bool
	animals.back().male = dna[1];

	//never born pregnant
	animals.back().pregnant = 0;
	animals.back().theFather = -1;

	if (dna[3] == true)
		animals.back().reproductionPersistance = animals[i].reproductionPersistance;
	else
		animals.back().reproductionPersistance = animals[j].reproductionPersistance;

	//cout << dna[0] << ", " << dna[9] << endl;

}

void moveAnimal(int i)
{
	int x1 = animals[i].location.x;
	int y1 = animals[i].location.y;
	double dist = 999999999999999999;
	int closest=-1;
	for (int j = 0; j < animals.size(); j++)
	{
		if (j == i)
			continue;
		if (animals[i].location == animals[j].location)
			continue;
		if (animals[i].male == animals[j].male || animals[j].pregnant > 0 || animals[i].pregnant > 0)
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

	int x2, y2;
	if (closest == -1)
	{
		uniform_int_distribution<int> randPoint(-20, 20);
		

		// cant let the point be 0,0
		int rand=0;
		while (rand==0)
		{
			rand = randPoint(generator);
		}
		x2 = animals[i].location.x + rand;
		y2 = animals[i].location.y + randPoint(generator);
		dist = sqrt(pow((y2 - y1), 2) + pow((x2 - x1), 2));

	}
	else
	{
		x2 = animals[closest].location.x;
		y2 = animals[closest].location.y;
	}

	double xmultiplier = (x2 - x1) / dist;
	double ymultiplier = (y2 - y1) / dist;
	if (dist < animals[i].speed)
		animals[i].location = animals[i].location + Point(xmultiplier * dist, ymultiplier * dist);
	else
		animals[i].location = animals[i].location + Point(xmultiplier * animals[i].speed, ymultiplier * animals[i].speed);
	
	// Make sure the animal hasent gone outside the boundaries
	if (animals[i].location.x < 0)
		animals[i].location.x = 0;
	else if (animals[i].location.x > w)
		animals[i].location.x = w;
	if (animals[i].location.y < 0)
		animals[i].location.y = 0;
	else if (animals[i].location.y > h)
		animals[i].location.y = h;
}

void collision(int i, int j, bool spore)
{
	int thefemale = female(i, j);
	if (thefemale > -1 && animals[thefemale].pregnant == 0)
	{
		animals[thefemale].pregnant = 100;
		animals[thefemale].theFather = male(i, j);
	}
		

}

int main()
{
	Rect scenebox(0, 0, w, h);
	Mat worldMap = cv::Mat(h, w, CV_8UC3, Vec3b(255,255,255));
	circle(worldMap, Point(0, 0), 700, Vec3b(255, 0, 0), -1);
	addAnimal(Point(200, 200), Vec3b(0, 200, 255),10, 5, 1, 10);
	addAnimal(Point(1000, 500), Vec3b(100, 0, 255),10, 5, 0, 10);
	//addAnimal(Point(400, 500), Vec3b(0, 50, 255), 5, 1, 0);
	//addAnimal(Point(700, 400), Vec3b(100, 100, 255), 3, 0, 0);
	namedWindow("World Map", CV_WINDOW_AUTOSIZE);
	while (1)
	{
		Mat viewMap;
		worldMap.copyTo(viewMap);
		for (int i = 0; i < animals.size(); i++)
		{
			if (animals[i].pregnant > 0)
			{
				animals[i].pregnant--;
				if (animals[i].pregnant == 0)
				{
					mateAnimals(i, animals[i].theFather, false);
					animals[i].theFather = -1;
				}
					

			}
			moveAnimal(i);
			// Check for collisions
			int j = isVacant(animals[i].location, i);
			if (j > -1)
				collision(i, j, false);
			circle(viewMap, animals[i].location, 3, animals[i].colour, -1);
		}
		imshow("World Map", viewMap);
		waitKey(100);
	}
}