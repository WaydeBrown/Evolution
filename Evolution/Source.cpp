#include <iostream>
#include <cmath>
#include "opencv2/imgcodecs.hpp"
#include "opencv2/highgui.hpp"
#include "opencv2/imgproc.hpp"
#include <random>
#include <chrono>
#include <array>
#include <forward_list>

using namespace std;
using namespace cv;
unsigned short w = 1300;
unsigned short h = 800;
vector<int> deadAnimals;
vector<int> vacantEmbryo;


// construct a trivial random generator engine from a time-based seed:
unsigned seed = chrono::system_clock::now().time_since_epoch().count();
default_random_engine generator(seed);

struct animalStruct {
	Point location;
	Vec3b colour;
	unsigned _int8 speed; //0-255
	unsigned _int16 sporeRadius; //0-65k
	bool male; //
	unsigned _int16 pregnant; //0-65k
	unsigned _int16 gestation; //0-65k the pregnancy period
	unsigned _int8 reproductionPersistance; //0-255
	int theFather; //a spot to saave the name of the father for pregnant animals
	int energy;
	bool photosynthesis;
	unsigned _int16 animalSize; //0-65k
	unsigned _int16 birthSize; //0-65k
	unsigned _int16 maxSize; //0-65k
	int growthThreshold; //0-65k
	unsigned _int16 sexualMaturity; //0-65k
	bool asexual;
	unsigned _int16 maxAge;
	unsigned _int16 animalAge;//0-65k
	bool alive; //is the animal alive or not
	float species;
};

vector<animalStruct> animals;
forward_list<animalStruct> embryo;

void addAnimal(
	Point location, 
	Vec3b colour,
	unsigned _int8 sporeRadius, 
	bool male, 
	unsigned _int8 reproductionPersistance,
	int growthThreshold,
	unsigned _int8 speed = 0,
	int energy=1000,
	unsigned _int8 gestation=100,
	bool photosynthesis=true,
	unsigned _int16 birthSize = 1,
	unsigned _int16 maxSize = 1000,
	unsigned _int16 sexualMaturity=100, //0-65k
	bool asexual = true,
	unsigned _int16 maxAge = 800, //0-65k
	unsigned _int16 animalAge = 0, //0-65k
	bool alive=true
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
	animals.back().energy = energy;
	animals.back().gestation = gestation;
	animals.back().photosynthesis = photosynthesis;
	animals.back().animalSize = birthSize;
	animals.back().birthSize = birthSize;
	animals.back().maxSize = maxSize;
	animals.back().growthThreshold = growthThreshold;
	animals.back().sexualMaturity = sexualMaturity;
	animals.back().asexual = asexual;
	animals.back().maxAge = maxAge;
	animals.back().animalAge = animalAge;
	animals.back().alive = alive;
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
	vector<vector<int>> locals;
	for (int i = 0; i < animals.size(); i++)
	{
		int x2 = animals[i].location.x;
		int y2 = animals[i].location.y;
		//double dist_temp = sqrt(pow((y2 - p.y), 2) + pow((x2 - p.x), 2));
		//if (abs(dist_temp)<maxRadius)
		if (abs(x2-p.x)<= maxRadius && abs(y2 - p.y) <= maxRadius)
		{
			locals.emplace_back();
			locals.back().push_back(i);
			locals.back().push_back(x2);
			locals.back().push_back(y2);
		}
	}
	for (int i = radiusStart; i < maxRadius+1; i++)
	{
		for (int j = 0; j < attempts; j++)
		{
			uniform_int_distribution<int> randPoint(-i, i);
			Point trialPoint;
			trialPoint.x = p.x + randPoint(generator);
			trialPoint.y = p.y + randPoint(generator);
			
			for (int k = 0; k < locals.size(); k++)
			{
				if (locals[k][1] == trialPoint.x && locals[k][2] == trialPoint.y)
				{
					break;
				}
				if (k==locals.size()-1)
					return trialPoint;
			}
		}
	}
	return Point (0,0);
}

void makeAnimals(int i, int j, bool spore)
{
	
	const unsigned _int16 speedMax = 254;
	const unsigned _int16 sporeRadiusMax=2000;
	const unsigned _int16 gestationMax=10000;
	const unsigned _int16 reproductionPersistanceMax=254;
	const unsigned _int16 birthSizeMax=64000;
	const unsigned _int16 maxSizeMax = 64000;
	const unsigned _int16 growthThresholdMax = 64000;
	const unsigned _int16 sexualMaturityMax = 64000;
	const unsigned _int16 maxAgeMax = 64000;

	const unsigned noOfDNA = 12; // number of variables in the DNA chain
	int theFemale = female(i, j);
	
	// create the animal
	// first check if there is a free spot in the vector
	int newAnimal = -1;
	
	if (deadAnimals.size() > 0)
	{
		newAnimal = deadAnimals.back();
		deadAnimals.pop_back();
	}
	else
	{
		animals.push_back(animalStruct());
		newAnimal = animals.size() - 1;
	}
	
	
	// fill in its details

	/*	location,
		allways follow the mother, 
		if spore == true then it has been polinated then the location will be random within sporeRadius
		if spore == false then grow next to it if free
	*/

	if (spore == true) // if it has been fertilized by spores then the female has seeds that can also fly
	{
		// find a random location within sporeRadius
		Point newPoint = freeLocation(animals[newAnimal].location, animals[theFemale].reproductionPersistance, animals[theFemale].sporeRadius, animals[theFemale].sporeRadius);
		
		// if the seed lands where someone is already living -> too bad, its not going to grow.
		if (newPoint == Point(0, 0))
		{
			deadAnimals.push_back(newAnimal);
			animals[newAnimal].alive=false;
			return;
		}
		else
			animals[newAnimal].location = newPoint;
	}
	else if (i == j)//then we have asexual reproduction
	{
		// find a random location within sporeRadius
		Point newPoint = freeLocation(animals[i].location, animals[i].reproductionPersistance, 1, animals[i].sporeRadius);

		// if we run out of attempts without finding a free spot then we die.
		if (newPoint == Point(0, 0))
		{
			deadAnimals.push_back(newAnimal);
			animals[newAnimal].alive=false;
			return;
		}
		else
			animals[newAnimal].location = newPoint;
	}
	else 
	{
		// find a random location within sporeRadius
		Point newPoint = freeLocation(animals[theFemale].location, animals[theFemale].reproductionPersistance, 0, animals[theFemale].sporeRadius);

		// if we run out of attempts without finding a free spot then we die.
		if (newPoint == Point(0, 0))
		{
			deadAnimals.push_back(newAnimal);
			animals[newAnimal].alive=false;
			return;
		}
		else
			animals[newAnimal].location = newPoint;
	}

	if (i == j)
	{
		Point newPoint = animals[newAnimal].location;
		animals[newAnimal] = animals[i];
		animals[newAnimal].location = newPoint;
	}
	else
	{
		uniform_int_distribution<unsigned> distribution(0, 999999999);
		unsigned randNum = distribution(generator);
		bool dna[noOfDNA];
		//for (auto& k : dna) 
		for (unsigned k = 0; k<noOfDNA; k++)
		{
			dna[k] = randNum / (k + 1) % 2;
		}
		/*
		if (dna[0] == true)
		animals[newAnimal].colour = animals[i].colour;
		else
		animals[newAnimal].colour = animals[j].colour;
		*/

		// sex directly from random bool
		animals[newAnimal].male = dna[0];

		if (dna[1] == true)
			animals[newAnimal].speed = animals[i].speed;
		else
			animals[newAnimal].speed = animals[j].speed;

		if (dna[2] == true)
			animals[newAnimal].sporeRadius = animals[i].sporeRadius;
		else
			animals[newAnimal].sporeRadius = animals[j].sporeRadius;

		if (dna[3] == true)
			animals[newAnimal].reproductionPersistance = animals[i].reproductionPersistance;
		else
			animals[newAnimal].reproductionPersistance = animals[j].reproductionPersistance;

		if (dna[4] == true)
			animals[newAnimal].gestation = animals[i].gestation;
		else
			animals[newAnimal].gestation = animals[j].gestation;

		if (dna[5] == true)
			animals[newAnimal].photosynthesis = animals[i].photosynthesis;
		else
			animals[newAnimal].photosynthesis = animals[j].photosynthesis;

		// always born at size of birthsize. birthsize inherited from either parent
		if (dna[6] == true)
			animals[newAnimal].birthSize = animals[i].birthSize;
		else
			animals[newAnimal].birthSize = animals[j].birthSize;
		
		if (dna[7] == true)
			animals[newAnimal].maxSize = animals[i].maxSize;
		else
			animals[newAnimal].maxSize = animals[j].maxSize;
		if (dna[8] == true)
			animals[newAnimal].growthThreshold = animals[i].growthThreshold;
		else
			animals[newAnimal].growthThreshold = animals[j].growthThreshold;

		if (dna[9] == true)
			animals[newAnimal].sexualMaturity = animals[i].sexualMaturity;
		else
			animals[newAnimal].sexualMaturity = animals[j].sexualMaturity;

		if (dna[10] == true)
			animals[newAnimal].asexual = animals[i].asexual;
		else
			animals[newAnimal].asexual = animals[j].asexual;

		if (dna[11] == true)
			animals[newAnimal].maxAge = animals[i].maxAge;
		else
			animals[newAnimal].maxAge = animals[j].maxAge;
	}
	// some things generic to all newborns
	//never born pregnant
	animals[newAnimal].pregnant = 0;
	animals[newAnimal].theFather = -1;
	animals[newAnimal].animalSize = animals[newAnimal].birthSize;
	animals[newAnimal].animalAge = 0;
	animals[newAnimal].alive = true;
	//new animal gets a starting energy relative to the pregnancy time
	animals[newAnimal].energy = animals[i].gestation * 10;

	// gene mutation!!
	uniform_int_distribution<unsigned> distribution2(0, 100);
	for (int n = 0; n < 2; n++)
	{
		unsigned randNum2 = distribution2(generator);
		if (randNum2 == 0) {
			if (n == 0 && animals[newAnimal].speed < speedMax)
				animals[newAnimal].speed++;
			else if (animals[newAnimal].speed>0)
				animals[newAnimal].speed--;
		}
		else if (randNum2 == 1) {
			if (n == 0 && animals[newAnimal].sporeRadius < sporeRadiusMax)
				animals[newAnimal].sporeRadius++;
			else  if (animals[newAnimal].sporeRadius>0)
				animals[newAnimal].sporeRadius--;
		}
		else if (randNum2 == 2) {
			if (n == 0 && animals[newAnimal].reproductionPersistance < reproductionPersistanceMax)
				animals[newAnimal].reproductionPersistance++;
			else  if (animals[newAnimal].reproductionPersistance>0)
				animals[newAnimal].reproductionPersistance--;
		}
		else if (randNum2 == 3) {
			if (n == 0 && animals[newAnimal].gestation < gestationMax)
				animals[newAnimal].gestation++;
			else  if (animals[newAnimal].gestation>0)
				animals[newAnimal].gestation--;
		}
		else if (randNum2 == 4) {
			if (n == 0)
				animals[newAnimal].photosynthesis=true;
			else animals[newAnimal].photosynthesis=false;
		}
		else if (randNum2 == 5) {
			if (n == 0 && animals[newAnimal].birthSize < birthSizeMax)
				animals[newAnimal].birthSize++;
			else  if (animals[newAnimal].birthSize>0)
				animals[newAnimal].birthSize--;
		}
		else if (randNum2 == 6) {
			if (n == 0 && animals[newAnimal].maxSize < maxSizeMax)
				animals[newAnimal].maxSize++;
			else  if (animals[newAnimal].maxSize>0)
				animals[newAnimal].maxSize--;
		}
		else if (randNum2 == 7) {
			if (n == 0 && animals[newAnimal].growthThreshold < growthThresholdMax)
				animals[newAnimal].growthThreshold++;
			else if (animals[newAnimal].growthThreshold>0)
				animals[newAnimal].growthThreshold--;
		}
		else if (randNum2 == 8) {
			if (n == 0 && animals[newAnimal].sexualMaturity < sexualMaturityMax)
				animals[newAnimal].sexualMaturity++;
			else if (animals[newAnimal].sexualMaturity>0)
				animals[newAnimal].sexualMaturity--;
		}
		else if (randNum2 == 9) {
			if (n == 0)
				animals[newAnimal].asexual=true;
			else animals[newAnimal].asexual=false;
		}
		else if (randNum2 == 10) {
			if (n == 0 && animals[newAnimal].maxAge < maxAgeMax)
				animals[newAnimal].maxAge++;
			else if (animals[newAnimal].maxAge>0)
				animals[newAnimal].maxAge--;
		}
		
		animals[newAnimal].species = (
			animals[newAnimal].speed +
			animals[newAnimal].sporeRadius +
			animals[newAnimal].gestation +
			animals[newAnimal].reproductionPersistance +
			animals[newAnimal].birthSize +
			animals[newAnimal].maxSize +
			animals[newAnimal].growthThreshold +
			animals[newAnimal].sexualMaturity +
			animals[newAnimal].maxAge+0.1) / (
				speedMax +
				sporeRadiusMax +
				gestationMax +
				reproductionPersistanceMax +
				birthSizeMax +
				maxSizeMax +
				growthThresholdMax +
				sexualMaturityMax +
				maxAgeMax+0.1);
		//int(animals[newAnimal].species * 180)
		animals[newAnimal].colour = Vec3b(70,255,255);
	}
}

void moveAnimal(int i)
{
	int x1 = animals[i].location.x;
	int y1 = animals[i].location.y;
	double dist = 999999999999999999;
	int closest=-1;
	// is i mature?
	if (animals[i].animalSize > animals[i].sexualMaturity)
	{
		for (int j = 0; j < animals.size(); j++)
		{
			if (j == i)
				continue;
			if (animals[j].animalSize < animals[j].sexualMaturity)
				continue;
			// if either of the animals are not mature then they are not looking for a mate
			if (animals[i].location == animals[j].location)
				continue;
			if (animals[i].male == animals[j].male || animals[j].pregnant > 0 || animals[i].pregnant > 0)
				continue;

			int x2 = animals[j].location.x;
			int y2 = animals[j].location.y;
			double dist_temp = sqrt(pow((y2 - y1), 2) + pow((x2 - x1), 2));
			if (abs(dist_temp)<abs(dist))
			{
				closest = j;
				dist = dist_temp;
			}
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
		animals[i].location = Point(x2, y2);
	else
		animals[i].location = animals[i].location + Point(xmultiplier * (animals[i].speed+1), ymultiplier * (animals[i].speed+1));
	
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
	// Mating
	int thefemale = female(i, j);
	if (thefemale > -1 && animals[thefemale].pregnant == 0)
	{
		animals[thefemale].pregnant = animals[thefemale].gestation;
		animals[thefemale].theFather = male(i, j);
	}
		

}

void balanceEnergy(int animal, int n, int eatenAnimal=-1)
{
	if (n == 1) // Normal daily energy use
	{
		animals[animal].energy -= animals[animal].speed;
		if (animals[animal].photosynthesis == true)
			animals[animal].energy += 10;
		if (animals[animal].energy > animals[animal].growthThreshold)
		{
			if (animals[animal].animalSize < animals[animal].maxSize)
			{
				animals[animal].animalSize += animals[animal].energy - animals[animal].growthThreshold;
				animals[animal].energy = animals[animal].growthThreshold;
			}
			else if (animals[animal].asexual == true)
			{
				makeAnimals(animal, animal, false);
				animals[animal].animalSize = animals[animal].animalSize / 2;
			}
		}
		animals[animal].animalAge++;
		if (animals[animal].animalAge > animals[animal].maxAge)
		{
			deadAnimals.push_back(animal);
			animals[animal].alive = false;
		}
	}
	if (n == 2) // eaten something
		animals[animal].energy += animals[eatenAnimal].animalSize;
	// death from exhaustion
	if (animals[animal].energy < 1)
	{
		deadAnimals.push_back(animal);
		animals[animal].alive = false;
	}
}

int main()
{
	unsigned long clockCycles = 0;
	Rect scenebox(0, 0, w, h);
	
	Mat worldMap = Mat(h, w, CV_8UC3, Vec3b(60, 50, 255));
	circle(worldMap, Point(0, 0), 700, Vec3b(110,50,255), -1);

	addAnimal(Point(200, 200), Vec3b(20, 255, 255), 1, 0, 10, 2000);
	//addAnimal(Point(1000, 500), Vec3b(120, 255, 255), 5, 1, 10, 2200);

	namedWindow("World Map", CV_WINDOW_AUTOSIZE);
	while (1)
	{
		Mat viewMap;
		worldMap.copyTo(viewMap);
		for (int i = 0; i < animals.size(); i++)
		{
			if (animals[i].alive == false)
			{
				continue;
			}
			if (animals[i].pregnant > 0)
			{
				animals[i].pregnant--;
				if (animals[i].pregnant == 0)
				{
					makeAnimals(i, animals[i].theFather, false);
					animals[i].theFather = -1;
				}
			}
			if (animals[i].speed > 0) 
			{
				moveAnimal(i);
				// Check for collisions
				int j = isVacant(animals[i].location, i);
				if (j > -1)
					collision(i, j, false);
			}
			
			circle(viewMap, animals[i].location, 3, animals[i].colour, -1);
			balanceEnergy(i, 1);
		}
		cvtColor(viewMap, viewMap, CV_HSV2BGR);
		imshow("World Map", viewMap);
		waitKey(10);
		clockCycles++;
		cout << "clockCycles: "<< clockCycles <<"   Animals:"<< animals.size() << "  Dead: "<< deadAnimals.size()<< endl;
	}
}