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
vector<vector<short>> animals;
forward_list<vector<short >> embryo;
vector<short> maxLevels;
vector<short> minLevels;
//short maxLevels[24];

// construct a trivial random generator engine from a time-based seed:
unsigned seed = chrono::system_clock::now().time_since_epoch().count();
default_random_engine generator(seed);

const short MutationStart = 12;
const short NoOfMutations = 12;
const short NoOfCharac = 24;
// animal characteristics    {position in animals vector, min value, max value, seed value}
const short locationx[3] =					{ 0, 0, 1300 };
const short locationy[3] =					{ 1, 0, 800};
const short energy[3] =						{ 2, 0, 30000 };
const short animalAge[3] =					{ 3, 0, 30000 };
const short pregnant[3] =					{ 4, 0, 30000 };
//updated periodically
const short animalSize[3] =					{ 5, 0, 30000 };
const short theFather[3] =					{ 6, -1, 30000 };
// constants non-mutatable
const short male[3] =						{ 7, 0, 1 };
const short species[3] =					{ 8, -30000, 30000 };
const short hColour[3] =					{ 9, 0, 179 };
const short sColour[3] =					{ 10, 0, 255 };
const short vColour[3] =					{ 11, 0, 255 };
// constants (mutatable)
const short vision[3] =						{ 12, 0, 100};
const short speed[3] =						{ 13, -100, 100 };
const short sporeRadius[3] =				{ 14, -100, 100 };
const short reproductionPersistance[3] =	{ 15, 1, 20 };
const short growthThreshold[3] =			{ 16, 1, 30000 };
const short gestation[3] =					{ 17, 1, 10000 };
const short photosynthesis[3] =				{ 18, -1000, 1000 };
const short maxSize[3] =					{ 19, 1, 30000 };
const short sexualMaturity[3] =				{ 20, 1, 30000 };
const short asexual[3] =					{ 21, -100, 100 }; // combine with reproductive persistance
const short maxAge[3] =						{ 22, 1, 30000 };
const short birthSize[3] =					{ 23, 1, 30000 };


void seedAnimal()
{
	animals.emplace_back();
	animals.back().push_back(200); // locationx);
	animals.back().push_back(200); // locationy);
	animals.back().push_back(1000); // energy);
	animals.back().push_back(0); // animalAge);
	animals.back().push_back(0); // pregnant);
	
	//updated periodically
	animals.back().push_back(1); // animalSize);
	animals.back().push_back(0); // theFather);

	// constants non-mutatable
	animals.back().push_back(0); // male);
	animals.back().push_back(0); // species
	animals.back().push_back(100); // hColour);
	animals.back().push_back(255); // sColour);
	animals.back().push_back(255); // vColour);

	// constants (mutatable)
	animals.back().push_back(0); // vision);
	animals.back().push_back(0); // speed);
	animals.back().push_back(1); // sporeRadius);
	animals.back().push_back(10); // reproductionPersistance);
	animals.back().push_back(2000); // growthThreshold);
	animals.back().push_back(100); // gestation);
	animals.back().push_back(10); // photosynthesis);
	animals.back().push_back(1000); // maxSize);
	animals.back().push_back(100); // sexualMaturity);
	animals.back().push_back(10); // asexual);
	animals.back().push_back(800); // maxAge);
	animals.back().push_back(0); // birthSize);
}



int isFemale(int i, int j)
{
	if (animals[i][male[0]] == 0 && animals[j][male[0]] == 1)
		return i;
	else if (animals[i][male[0]] == 1 && animals[j][male[0]] == 0)
		return j;
	else
		return -1;
}

int isMale(int i, int j)
{
	if (animals[i][male[0]] == 1 && animals[j][male[0]] == 0)
		return i;
	else if (animals[i][male[0]] == 0 && animals[j][male[0]] == 1)
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
		if (animals[j][locationx[0]] == p.x && animals[j][locationy[0]] == p.y)
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
		int x2 = animals[i][locationx[0]];
		int y2 = animals[i][locationy[0]];
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

	int theFemale = isFemale(i, j);
	
	// create the animal
	// first check if there is a free spot in the vector
	int newAnimal = -1;
	
	if (deadAnimals.size() > 0)
	{
		newAnimal = deadAnimals.back();
		deadAnimals.pop_back();
		animals[newAnimal]=animals[i];
	}
	else
	{
		animals.push_back(animals[i]);
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
		Point newPoint = freeLocation(Point(animals[newAnimal][locationx[0]], animals[newAnimal][locationy[0]]), animals[theFemale][reproductionPersistance[0]], animals[theFemale][sporeRadius[0]], animals[theFemale][sporeRadius[0]]);
		
		// if the seed lands where someone is already living -> too bad, its not going to grow.
		if (newPoint == Point(0, 0))
		{
			deadAnimals.push_back(newAnimal);
			animals[newAnimal][energy[0]] = 0;
			return;
		}
		else
		{
			animals[newAnimal][locationx[0]] = newPoint.x;
			animals[newAnimal][locationy[0]] = newPoint.y;
		}
	}
	else if (i == j)//then we have asexual reproduction
	{
		// find a random location within sporeRadius
		Point newPoint = freeLocation(Point(animals[i][locationx[0]], animals[i][locationy[0]]), animals[i][reproductionPersistance[0]], 1, animals[i][sporeRadius[0]]);

		// if we run out of attempts without finding a free spot then we die.
		if (newPoint == Point(0, 0))
		{
			deadAnimals.push_back(newAnimal);
			animals[newAnimal][energy[0]] =0;
			return;
		}
		else
		{
			animals[newAnimal][locationx[0]] = newPoint.x;
			animals[newAnimal][locationy[0]] = newPoint.y;
		}
	}
	else 
	{
		// find a random location within sporeRadius
		Point newPoint = freeLocation(Point(animals[theFemale][locationx[0]], animals[theFemale][locationy[0]]), animals[theFemale][reproductionPersistance[0]], 0, animals[theFemale][sporeRadius[0]]);

		// if we run out of attempts without finding a free spot then we die.
		if (newPoint == Point(0, 0))
		{
			deadAnimals.push_back(newAnimal);
			animals[newAnimal][energy[0]] =0;
			return;
		}
		else
		{
			animals[newAnimal][locationx[0]] = newPoint.x;
			animals[newAnimal][locationy[0]] = newPoint.y;
		}
	}

	if (i != j) // then we have two parents
	{
		uniform_int_distribution<unsigned> distribution(0, 999999999);
		unsigned randNum = distribution(generator);
		bool dna[NoOfMutations];
		//for (auto& k : dna) 
		for (unsigned k = 0; k<NoOfMutations; k++)
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
		
		
		animals[newAnimal][male[0]] = dna[0];

		if (dna[1] == true)
			animals[newAnimal][speed[0]] = animals[i][speed[0]];
		else
			animals[newAnimal][speed[0]] = animals[j][speed[0]];

		if (dna[2] == true)
			animals[newAnimal][sporeRadius[0]] = animals[i][sporeRadius[0]];
		else
			animals[newAnimal][sporeRadius[0]] = animals[j][sporeRadius[0]];

		if (dna[3] == true)
			animals[newAnimal][reproductionPersistance[0]] = animals[i][reproductionPersistance[0]];
		else
			animals[newAnimal][reproductionPersistance[0]] = animals[j][reproductionPersistance[0]];

		if (dna[4] == true)
			animals[newAnimal][gestation[0]] = animals[i][gestation[0]];
		else
			animals[newAnimal][gestation[0]] = animals[j][gestation[0]];

		if (dna[5] == true)
			animals[newAnimal][photosynthesis[0]] = animals[i][photosynthesis[0]];
		else
			animals[newAnimal][photosynthesis[0]] = animals[j][photosynthesis[0]];

		// always born at size of birthsize. birthsize inherited from either parent
		if (dna[6] == true)
			animals[newAnimal][birthSize[0]] = animals[i][birthSize[0]];
		else
			animals[newAnimal][birthSize[0]] = animals[j][birthSize[0]];
		
		if (dna[7] == true)
			animals[newAnimal][maxSize[0]] = animals[i][maxSize[0]];
		else
			animals[newAnimal][maxSize[0]] = animals[j][maxSize[0]];
		if (dna[8] == true)
			animals[newAnimal][growthThreshold[0]] = animals[i][growthThreshold[0]];
		else
			animals[newAnimal][growthThreshold[0]] = animals[j][growthThreshold[0]];

		if (dna[9] == true)
			animals[newAnimal][sexualMaturity[0]] = animals[i][sexualMaturity[0]];
		else
			animals[newAnimal][sexualMaturity[0]] = animals[j][sexualMaturity[0]];

		if (dna[10] == true)
			animals[newAnimal][asexual[0]] = animals[i][asexual[0]];
		else
			animals[newAnimal][asexual[0]] = animals[j][asexual[0]];

		if (dna[11] == true)
			animals[newAnimal][maxAge[0]] = animals[i][maxAge[0]];
		else
			animals[newAnimal][maxAge[0]] = animals[j][maxAge[0]];


		uniform_int_distribution<unsigned> MutDistr(0, 1);
		for (int k = MutationStart; k < animals[i].size() - 1; k++)
		{
			unsigned randNum = MutDistr(generator);
			if (randNum == 1)
				animals[newAnimal][k] = animals[i][k];
			else
				animals[newAnimal][k] = animals[j][k];
		}

//ff

	}
	// some things generic to all newborns
	//never born pregnant
	animals[newAnimal][pregnant[0]] = 0;
	animals[newAnimal][theFather[0]] = -1;
	animals[newAnimal][animalSize[0]] = animals[newAnimal][birthSize[0]];
	animals[newAnimal][animalAge[0]] = 0;
	//new animal gets a starting energy relative to the pregnancy time
	animals[newAnimal][energy[0]] = animals[i][gestation[0]] * 10;

	// gene mutation!!
	uniform_int_distribution<unsigned> distribution2(0, 100);
	for (int n = 0; n < 2; n++)
	{
		unsigned randNum2 = distribution2(generator);
		if (randNum2 == 0) {
			if (n == 0 && animals[newAnimal][speed[0]] < speed[2])
				animals[newAnimal][speed[0]]++;
			else if (animals[newAnimal][speed[0]]>0)
				animals[newAnimal][speed[0]]--;
		}
		else if (randNum2 == 1) {
			if (n == 0 && animals[newAnimal][sporeRadius[0]] < sporeRadius[2])
				animals[newAnimal][sporeRadius[0]]++;
			else  if (animals[newAnimal][sporeRadius[0]]>0)
				animals[newAnimal][sporeRadius[0]]--;
		}
		else if (randNum2 == 2) {
			if (n == 0 && animals[newAnimal][reproductionPersistance[0]] < reproductionPersistance[2])
				animals[newAnimal][reproductionPersistance[0]]++;
			else  if (animals[newAnimal][reproductionPersistance[0]]>0)
				animals[newAnimal][reproductionPersistance[0]]--;
		}
		else if (randNum2 == 3) {
			if (n == 0 && animals[newAnimal][gestation[0]] < gestation[2])
				animals[newAnimal][gestation[0]]++;
			else  if (animals[newAnimal][gestation[0]]>0)
				animals[newAnimal][gestation[0]]--;
		}
		else if (randNum2 == 4) {
			if (n == 0)
				animals[newAnimal][photosynthesis[0]]=1;
			else animals[newAnimal][photosynthesis[0]] =0;
		}
		else if (randNum2 == 5) {
			if (n == 0 && animals[newAnimal][birthSize[0]] < birthSize[2])
				animals[newAnimal][birthSize[0]]++;
			else  if (animals[newAnimal][birthSize[0]]>0)
				animals[newAnimal][birthSize[0]]--;
		}
		else if (randNum2 == 6) {
			if (n == 0 && animals[newAnimal][maxSize[0]] < maxSize[2])
				animals[newAnimal][maxSize[0]]++;
			else  if (animals[newAnimal][maxSize[0]]>0)
				animals[newAnimal][maxSize[0]]--;
		}
		else if (randNum2 == 7) {
			if (n == 0 && animals[newAnimal][growthThreshold[0]] < growthThreshold[2])
				animals[newAnimal][growthThreshold[0]]++;
			else if (animals[newAnimal][growthThreshold[0]]>0)
				animals[newAnimal][growthThreshold[0]]--;
		}
		else if (randNum2 == 8) {
			if (n == 0 && animals[newAnimal][sexualMaturity[0]] < sexualMaturity[2])
				animals[newAnimal][sexualMaturity[0]]++;
			else if (animals[newAnimal][sexualMaturity[0]]>0)
				animals[newAnimal][sexualMaturity[0]]--;
		}
		else if (randNum2 == 9) {
			if (n == 0)
				animals[newAnimal][asexual[0]] =1;
			else animals[newAnimal][asexual[0]] =0;
		}
		else if (randNum2 == 10) {
			if (n == 0 && animals[newAnimal][maxAge[0]] < maxAge[2])
				animals[newAnimal][maxAge[0]]++;
			else if (animals[newAnimal][maxAge[0]]>0)
				animals[newAnimal][maxAge[0]]--;
		}
		
		animals[newAnimal][species[0]] = (
			animals[newAnimal][speed[0]] +
			animals[newAnimal][sporeRadius[0]] +
			animals[newAnimal][gestation[0]] +
			animals[newAnimal][reproductionPersistance[0]] +
			animals[newAnimal][birthSize[0]] +
			animals[newAnimal][maxSize[0]] +
			animals[newAnimal][growthThreshold[0]] +
			animals[newAnimal][sexualMaturity[0]] +
			animals[newAnimal][maxAge[0]]+0.1) / (
				speed[2] +
				sporeRadius[2] +
				gestation[2] +
				reproductionPersistance[2] +
				birthSize[2] +
				maxSize[2] +
				growthThreshold[2] +
				sexualMaturity[2] +
				maxAge[2]+0.1);
		//int(animals[newAnimal].species * 180)
		animals[newAnimal][hColour[0]] = 70;
		animals[newAnimal][sColour[0]] = 255;
		animals[newAnimal][vColour[0]] = 255;
	}
}

void moveAnimal(int i)
{
	int x1 = animals[i][locationx[0]];
	int y1 = animals[i][locationy[0]];
	double dist = 999999999999999999;
	int closest=-1;
	// is i mature?
	if (animals[i][animalSize[0]] > animals[i][sexualMaturity[0]])
	{
		for (int j = 0; j < animals.size(); j++)
		{
			if (j == i)
				continue;
			if (animals[j][animalSize[0]] < animals[j][sexualMaturity[0]])
				continue;
			// if either of the animals are not mature then they are not looking for a mate
			if (animals[i][locationx[0]] == animals[j][locationx[0]] && animals[i][locationy[0]] == animals[j][locationy[0]])
				continue;
			if (animals[i][male[0]] == animals[j][male[0]] || animals[j][pregnant[0]] > 0 || animals[i][pregnant[0]] > 0)
				continue;

			int x2 = animals[j][locationx[0]];
			int y2 = animals[j][locationy[0]];
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
		x2 = animals[i][locationx[0]] + rand;
		y2 = animals[i][locationy[0]] + randPoint(generator);
		dist = sqrt(pow((y2 - y1), 2) + pow((x2 - x1), 2));

	}
	else
	{
		x2 = animals[closest][locationx[0]];
		y2 = animals[closest][locationy[0]];
	}

	double xmultiplier = (x2 - x1) / dist;
	double ymultiplier = (y2 - y1) / dist;
	if (dist < animals[i][speed[0]])
		Point(animals[i][locationx[0]], animals[i][locationy[0]]) = Point(x2, y2);
	else
		Point(animals[i][locationx[0]], animals[i][locationy[0]]) = Point(animals[i][locationx[0]], animals[i][locationy[0]]) + Point(xmultiplier * (animals[i][speed[0]]+1), ymultiplier * (animals[i][speed[0]]+1));
	
	// Make sure the animal hasent gone outside the boundaries
	if (animals[i][locationx[0]] < 0)
		animals[i][locationx[0]] = 0;
	else if (animals[i][locationx[0]] > w)
		animals[i][locationx[0]] = w;
	if (animals[i][locationy[0]] < 0)
		animals[i][locationy[0]] = 0;
	else if (animals[i][locationy[0]] > h)
		animals[i][locationy[0]] = h;
}

void collision(int i, int j, bool spore)
{
	// Mating
	int thefemale = isFemale(i, j);
	if (thefemale > -1 && animals[thefemale][pregnant[0]] == 0)
	{
		animals[thefemale][pregnant[0]] = animals[thefemale][gestation[0]];
		animals[thefemale][theFather[0]] = isMale(i, j);
	}
		

}

void balanceEnergy(int animal, int n, int eatenAnimal=-1)
{
	if (n == 1) // Normal daily energy use
	{
		animals[animal][energy[0]] -= animals[animal][speed[0]];
		if (animals[animal][photosynthesis[0]] > 0)
			animals[animal][energy[0]] += (10+animals[animal][photosynthesis[0]]);
		if (animals[animal][energy[0]] > animals[animal][growthThreshold[0]])
		{
			if (animals[animal][animalSize[0]] < animals[animal][maxSize[0]])
			{
				animals[animal][animalSize[0]] += animals[animal][energy[0]] - animals[animal][growthThreshold[0]];
				animals[animal][energy[0]] = animals[animal][growthThreshold[0]];
			}
			else if (animals[animal][asexual[0]] > 0)
			{
				makeAnimals(animal, animal, false);
				animals[animal][animalSize[0]] = animals[animal][animalSize[0]] / 2;
			}
		}
		animals[animal][animalAge[0]]++;
		if (animals[animal][animalAge[0]] > animals[animal][maxAge[0]])
		{
			deadAnimals.push_back(animal);
			animals[animal][energy[0]] = 0;
		}
	}
	if (n == 2) // eaten something
		animals[animal][energy[0]] += animals[eatenAnimal][animalSize[0]];
	// death from exhaustion
	if (animals[animal][energy[0]] < 1)
	{
		deadAnimals.push_back(animal);
		animals[animal][energy[0]] = 0;
	}
}

int main()
{
	unsigned long clockCycles = 0;
	Rect scenebox(0, 0, w, h);
	
	Mat worldMap = Mat(h, w, CV_8UC3, Vec3b(60, 50, 255));
	circle(worldMap, Point(0, 0), 700, Vec3b(110,50,255), -1);

	seedAnimal();
	//addAnimal(Point(1000, 500), Vec3b(120, 255, 255), 5, 1, 10, 2200);

	namedWindow("World Map", CV_WINDOW_AUTOSIZE);
	while (1)
	{
		Mat viewMap;
		worldMap.copyTo(viewMap);
		for (int i = 0; i < animals.size(); i++)
		{
			if (animals[i][energy[0]] < 1)
			{
				continue;
			}
			if (animals[i][pregnant[0]] > 0)
			{
				animals[i][pregnant[0]]--;
				if (animals[i][pregnant[0]] == 0)
				{
					makeAnimals(i, animals[i][theFather[0]], false);
					animals[i][theFather[0]] = -1;
				}
			}
			if (animals[i][speed[0]] > 0) 
			{
				moveAnimal(i);
				// Check for collisions
				int j = isVacant(Point(animals[i][locationx[0]], animals[i][locationy[0]]), i);
				if (j > -1)
					collision(i, j, false);
			}
			
			circle(viewMap, Point(animals[i][locationx[0]], animals[i][locationy[0]]), 3, Vec3b(animals[i][hColour[0]], animals[i][sColour[0]], animals[i][vColour[0]]), -1);
			balanceEnergy(i, 1);
		}
		cvtColor(viewMap, viewMap, CV_HSV2BGR);
		imshow("World Map", viewMap);
		waitKey(10);
		clockCycles++;
		cout << "clockCycles: "<< clockCycles <<"   Animals:"<< animals.size() << "  Dead: "<< deadAnimals.size()<< endl;
	}
}