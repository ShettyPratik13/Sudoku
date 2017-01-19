#include<iostream>
#include<stdio.h>
#include<vector>
#include<algorithm>
#include<ctime>

using namespace std;
int matrix[10][10];
unsigned int squares[10],rows[10],columns[10];

/*Util function, to get the Sq number of a given Cell*/
int getSquareNumber(int x, int y) {
	if((x >= 1 && x <4) && (y >= 1 && y <4))
		return 1;
	else if((x >= 1 && x <4) && (y >=4 && y<7))
		return 2;
	else  if((x >= 1 && x <4) && (y >=7 && y<10))
		return 3;
	else if((x >= 4 && x <7) && (y >= 1 && y <4))
		return 4;
	else if((x >= 4 && x <7) && (y >=4 && y<7))
		return 5;
	else  if((x >= 4 && x <7) && (y >=7 && y<10))
		return 6;
	else if((x >= 7 && x <10) && (y >= 1 && y <4))
		return 7;
	else if((x >= 7 && x <10) && (y >=4 && y<7))
		return 8;
	else
		return 9;
}

class Cell {
	public:
		int x, y;
		vector<int> domain;
		int square;
		Cell(int i, int j) {
			x = i;
			y = j;
			for(int m=1;m<10;m++)
				domain.push_back(m);

			square = getSquareNumber(x,y);
		}
	
	/*Typically called when there is the domain size is 1*/
	int getValue() {
		return domain.at(0);
	}

	int getDomainSize() {
		return domain.size();
	}

	vector<int> getDomain() {
		return domain;
	}

	void updateDomain() {
		for(vector<int>::iterator it = domain.begin();it != domain.end();) {
			int val = (*it);
			int bitVal = 1<<val;
			if(((int)(rows[x] & bitVal)!=0) || ((int)(columns[y] & bitVal) !=0) || ((int)(squares[square] & bitVal) !=0)) {
				std::swap(*it,domain.back());
				domain.pop_back();
			} else {
			    ++it;
			}
		}
	}

	void resetDomain() {
		domain.clear();
		for(int i=1;i<10;i++) {
			int bitVal = 1<<i;
			if(!(((int)(rows[x] & bitVal)!=0) || ((int)(columns[y] & bitVal) !=0) || ((int)(squares[square] & bitVal) !=0)))
				domain.push_back(i);
		}
	}

	void print() {
		cout<<"X: "<<x<<"  Y: "<<y<<"  SQ: "<<square<<"  DMSIZE: "<<getDomainSize()<<endl;
	}

	bool equals(Cell xyz) {
		return (xyz.x == x && xyz.y ==y && xyz.square == square);
	}
};

vector<Cell*> collections, reverseSquares[10], reverseRows[10], reverseColumns[10];

struct domainComparator{
  bool operator() (Cell* lhs, Cell* rhs) {
    	if(lhs->getDomainSize() == rhs->getDomainSize()) {
    		
    		int leftCount = reverseRows[lhs->x].size() + reverseColumns[lhs->y].size() + reverseSquares[lhs->square].size();
			int rightCount = reverseRows[rhs->x].size() + reverseColumns[rhs->y].size() + reverseSquares[rhs->square].size();

    		if(leftCount != rightCount)
    			return leftCount > rightCount;
    		/*TODO: Before going for x/y comparision we can implement degree 
			checking, basically, the cell, which will cause domain 
			reduction in maximum cells should be selected first*/
	 		else if(lhs->x == rhs->x)
	 			return lhs->y<rhs->y;
	 		else
	 			return lhs->x<rhs->x;
		}
		return lhs->getDomainSize()>rhs->getDomainSize();
  }
};

void updateRelatedDomains(Cell cell, bool reset) {

	vector<Cell*>::iterator it = reverseRows[cell.x].begin();
	for(;it != reverseRows[cell.x].end();) {
		if((*it)->equals(cell)) {
			std::swap(*it,reverseRows[cell.x].back());
			reverseRows[cell.x].pop_back();
		} else {
			if(reset)
				(*it)->resetDomain();
			else
				(*it)->updateDomain();
			++it;
		}
	}

	it = reverseColumns[cell.y].begin();
	for(;it != reverseColumns[cell.y].end();) {
		if((*it)->equals(cell)) {
			std::swap(*it,reverseColumns[cell.y].back());
			reverseColumns[cell.y].pop_back();
		} else {
			if(reset)
				(*it)->resetDomain();
			else
				(*it)->updateDomain();
			++it;
		}
	}

	it = reverseSquares[cell.square].begin();
	for(;it != reverseSquares[cell.square].end();) {
		if((*it)->equals(cell)) {
			std::swap(*it,reverseSquares[cell.square].back());
			reverseSquares[cell.square].pop_back();
		} else if(((*it)->x != cell.x) || ((*it)->y != cell.y)) {
			if(reset)
				(*it)->resetDomain();
			else
				(*it)->updateDomain();
			++it;
		}
	}
}

bool removeSingletonDomains() {
    std::make_heap(collections.begin(),collections.end(),domainComparator());

	vector<Cell*>::iterator abc = collections.begin();
	while((*abc)->getDomainSize() == 1) {
		Cell *cell = *abc;

		matrix[cell->x][cell->y] = cell->getValue();
		rows[cell->x] |= 1<<cell->getValue();
		columns[cell->y] |= 1<<cell->getValue();
		squares[cell->square] |= 1<<cell->getValue();

		//cout<<"Setting value "<<cell->getValue()<<" at x = "<<cell->x<<"   y = "<<cell->y<<" in square "<<cell->square<<endl;
		/*Deleting the Cell that we have freezed because 
		 *don't want it to waste my time. laters*/
		std::swap(*abc,collections.back());
		collections.pop_back();

		updateRelatedDomains(*cell,false);

		if(collections.empty())
			return true;

		std::make_heap(collections.begin(),collections.end(),domainComparator());
		abc = collections.begin();
	}
	return false;
}

bool canWeFreeze() {
	if(collections.empty())
		return true;

	//std::make_heap(collections.begin(),collections.end(),domainComparator());
	vector<Cell*>::iterator it = collections.begin();
	Cell *current = *it;
	
	if(current->getDomainSize() == 0)
		return false;

	std::swap(*it,collections.back());
	collections.pop_back();

	/*TODO: Before getting the domain, we should sort the domain on basis of LRV, 
	basically try and select the value that will cause the least number of 
	domain reductions in other cells, so that we can avoid a dead-end and 
	get more close to the solution*/
	vector<int> domain = current->getDomain();
	for(vector<int>::iterator interator = domain.begin(); interator != domain.end(); ++interator) {
		int val = *interator;
		int bitVal = 1<<val;
		/*if(((int)(rows[current->x] & bitVal) ==0) && ((int)(columns[current->y] & bitVal) ==0) && ((int)(squares[current->square] & bitVal) ==0))*/ {
			matrix[current->x][current->y] = val;
			rows[current->x] |= 1<<val;
			columns[current->y] |= 1<<val;
			squares[current->square] |= 1<<val;
			//cout<<"Calling for "<<it->x<<"  "<<it->y<<"   "<<it->square<<endl;
			updateRelatedDomains(*current,false);

			if(canWeFreeze()) {
				//cout<<"Freezing value "<<(*interator)<<"for "<<current->x<<"  "<<current->y<<"   "<<current->square<<endl;
				return true;
			}

			/*No we can't, so revert all the changes*/
			rows[current->x] &= ~(1u<<val);
			columns[current->y] &= ~(1u<<val);
			squares[current->square] &= ~(1u<<val);
			matrix[current->x][current->y] = 0;
			//cout<<"Re-Calling for "<<it->x<<"  "<<it->y<<"   "<<it->square<<endl;
			updateRelatedDomains(*current,true);
		}
	}
	reverseRows[current->x].push_back(current);
	reverseColumns[current->y].push_back(current);
	reverseSquares[current->square].push_back(current);
	collections.push_back(current);
	return false;
}

bool searchNeedleInHayStack() {

	//std::make_heap(collections.begin(),collections.end(),domainComparator());
	vector<Cell*>::iterator it = collections.begin();
	Cell *cell = *it;
	std::swap(*it,collections.back());
	collections.pop_back();

	vector<int> domain = cell->getDomain();
	for(vector<int>::iterator interator = domain.begin(); interator != domain.end(); ++interator) {

		matrix[cell->x][cell->y] = (*interator);
		rows[cell->x] |= 1<<(*interator);
		columns[cell->y] |= 1<<(*interator);
		squares[cell->square] |= 1<<(*interator);
		//cout<<"Calling for "<<it->x<<"  "<<it->y<<"   "<<it->square<<endl;
		updateRelatedDomains(*cell,false);

		if(canWeFreeze()) {
			//cout<<"Freezing value "<<(*interator)<<"for "<<cell->x<<"  "<<cell->y<<"   "<<cell->square<<endl;
			return true;
		}

		/*No we can't, so revert all the changes*/
		rows[cell->x] &= ~(1u<<(*interator));
		columns[cell->y] &= ~(1u<<(*interator));
		squares[cell->square] &= ~(1u<<(*interator));
		matrix[cell->x][cell->y] = 0;
		//cout<<"Re-Calling for "<<it->x<<"  "<<it->y<<"   "<<it->square<<endl;
		updateRelatedDomains(*cell,true);
	}
	
	reverseRows[cell->x].push_back(cell);
	reverseColumns[cell->y].push_back(cell);
	reverseSquares[cell->square].push_back(cell);
	collections.push_back(cell);
	return false;
}

void printAndExit() {
	cout<<"The solution is \n";
	for(int i=1;i<10;i++) {
		for(int j=1;j<10;j++)
			cout<<matrix[i][j]<<" ";
		cout<<endl;
	}	
}

int main() {
	cout<<"Hello World | We are awesome | Welcome to Suduko\n";
	cout<<"Please enter the values for the Sudoku\n";
	bool wrongInput = false, sol = true;

	for(int i=1;i<10;i++)
		for(int j=1;j<10;j++) {
			cin>>matrix[i][j];
			if(wrongInput)
				continue;

			int bitVal = 1<<matrix[i][j];
			if(matrix[i][j]==0) {
				sol = false;
				Cell *cell = new Cell(i,j);
				collections.push_back(cell);
				reverseColumns[j].push_back(cell);
				reverseRows[i].push_back(cell);
				reverseSquares[cell->square].push_back(cell);
			} else if ((((int)(rows[i] & bitVal)) == 0) && (((int)(columns[j] & bitVal)) == 0) && (((int)(squares[getSquareNumber(i,j)] & bitVal)) == 0)) {
				rows[i] |= bitVal;
				columns[j] |= bitVal;
				squares[getSquareNumber(i,j)] |= bitVal;
				//cout<<"Final answer is "<<rows[i]<<"   "<<columns[j]<<"  "<<squares[getSquareNumber(i,j)]<<endl;
				//cout<<"Final answer is "<<i<<"   "<<j<<"   "<<matrix[i][j]<<endl;
			} else {
				wrongInput = true;
			}
		}

	clock_t begin_time = clock();
	double elapsed_secs = 0;

	if(!wrongInput)	{

		for(vector<Cell*>::iterator it = collections.begin(); it != collections.end(); ++it)
			(*it)->updateDomain();

		if(!collections.empty() && (/*removeSingletonDomains() || */searchNeedleInHayStack())) {
			printAndExit();
		} else if(!sol) { 
			cout<<"The given Soduku is NOT solvable \n";
		} else {
			cout<<"The given Soduku is ALREADY solved \n";
		}
		elapsed_secs = double(clock() - begin_time) / CLOCKS_PER_SEC;
	} else {
		cout<<"The Given Soduko has wrong input.";
		elapsed_secs = double(clock() - begin_time) / CLOCKS_PER_SEC;
	}
	cout<<"\nThe time elapsed is "<<elapsed_secs<<" TIME UNIT";
}
