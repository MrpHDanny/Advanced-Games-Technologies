#include "NavigationGrid.h"
#include "../../Common/Assets.h"

#include <fstream>

using namespace NCL;
using namespace CSC8503;

const int LEFT_NODE		= 0;
const int RIGHT_NODE	= 1;
const int TOP_NODE		= 2;
const int BOTTOM_NODE	= 3;

const char WALL_NODE	= 'x';
const char FLOOR_NODE	= '.';

NavigationGrid::NavigationGrid()	{
	nodeSize	= 0;
	gridWidth	= 12;
	gridHeight	= 12;
	allNodes	= nullptr;

	allNodes = new GridNode[gridWidth * gridHeight];
}

NavigationGrid::NavigationGrid(const std::string&filename) : NavigationGrid() {
	std::ifstream infile(Assets::DATADIR + filename);

	infile >> nodeSize;
	infile >> gridWidth;
	infile >> gridHeight;

	allNodes = new GridNode[gridWidth * gridHeight];

	for (int y = 0; y < gridHeight; ++y) {
		for (int x = 0; x < gridWidth; ++x) {
			GridNode& n = allNodes[(gridWidth * y) + x];
			char type = 0;
			infile >> type;
			n.type = type;
			n.position = Vector3((float)(x * nodeSize + 10), 0, (float)(y * nodeSize + 10));
		}
	}
	
	//now to build the connectivity between the nodes
	for (int y = 0; y < gridHeight; ++y) {
		for (int x = 0; x < gridWidth; ++x) {
			GridNode&n = allNodes[(gridWidth * y) + x];		

			if (y > 0) { //get the above node
				n.connected[0] = &allNodes[(gridWidth * (y - 1)) + x];
			}
			if (y < gridHeight - 1) { //get the below node
				n.connected[1] = &allNodes[(gridWidth * (y + 1)) + x];
			}
			if (x > 0) { //get left node
				n.connected[2] = &allNodes[(gridWidth * (y)) + (x - 1)];
			}
			if (x < gridWidth - 1) { //get right node
				n.connected[3] = &allNodes[(gridWidth * (y)) + (x + 1)];
			}
			for (int i = 0; i < 4; ++i) {
				if (n.connected[i]) {
					if (n.connected[i]->type == '.') {
						n.costs[i]		= 1;
					}
					if (n.connected[i]->type == 8) {
						n.connected[i] = nullptr; //actually a wall, disconnect!
					}
				}
			}
		}	
	}
}

NavigationGrid::~NavigationGrid()	{
	delete[] allNodes;
}

bool NavigationGrid::FindPath(const Vector3& from, const Vector3& to, NavigationPath& outPath) {
	//need to work out which node 'from' sits in, and 'to' sits in
	int fromX = ((int)from.x / nodeSize);
	int fromZ = (abs((int)from.z) / nodeSize);

	int toX = ((int)to.x / nodeSize);
	int toZ = (-(int)to.z / nodeSize);

	if (fromX < 0 || fromX > gridWidth - 1 ||
		fromZ < 0 || fromZ > gridHeight - 1) {
		return false; //outside of map region!
	}

	if (toX < 0 || toX > gridWidth - 1 ||
		toZ < 0 || toZ > gridHeight - 1) {
		return false; //outside of map region!
	}

	GridNode* startNode = &allNodes[(fromZ * gridWidth) + fromX];
	GridNode* endNode	= &allNodes[(toZ * gridWidth) + toX];

	// Consider other container types
	std::vector<GridNode*>  openList;
	std::vector<GridNode*>  closedList;

	openList.emplace_back(startNode);

	startNode->f = 0;
	startNode->g = 0;
	startNode->parent = nullptr;

	GridNode* currentBestNode = nullptr;

	while (!openList.empty()) {
		currentBestNode = RemoveBestNode(openList);

		if (currentBestNode == endNode) {			//we've found the path!
			GridNode* node = endNode;
			while (node != nullptr) {
				outPath.PushWaypoint(Vector3(node->position.x + nodeSize / 2 , node->position.y , -node->position.z - nodeSize / 2));
				node = node->parent;
			}
			return true;
		}
		else {
			for (int i = 0; i < 4; ++i) {
				GridNode* neighbour = currentBestNode->connected[i];
				if (!neighbour) { //might not be connected...
					continue;
				}	
				bool inClosed	= NodeInList(neighbour, closedList);
				if (inClosed) {
					continue; //already discarded this neighbour...
				}

				float h = Heuristic(neighbour, endNode);				
				float g = currentBestNode->g + currentBestNode->costs[i];
				float f = h + g;

				bool inOpen		= NodeInList(neighbour, openList);

				if (!inOpen) { //first time we've seen this neighbour
					openList.emplace_back(neighbour);
				}
				if (!inOpen || f < neighbour->f) {//might be a better route to this neighbour
					neighbour->parent = currentBestNode;
					neighbour->f = f;
					neighbour->g = g;
				}
			}
			closedList.emplace_back(currentBestNode);
		}
	}
	return false; //open list emptied out with no path!
}

bool NavigationGrid::NodeInList(GridNode* n, std::vector<GridNode*>& list) const {
	std::vector<GridNode*>::iterator i = std::find(list.begin(), list.end(), n);
	return i == list.end() ? false : true;
}

GridNode*  NavigationGrid::RemoveBestNode(std::vector<GridNode*>& list) const {
	std::vector<GridNode*>::iterator bestI = list.begin();

	GridNode* bestNode = *list.begin();

	for (auto i = list.begin(); i != list.end(); ++i) {
		if ((*i)->f < bestNode->f) {
			bestNode	= (*i);
			bestI		= i;
		}
	}
	list.erase(bestI);

	return bestNode;
}

float NavigationGrid::Heuristic(GridNode* hNode, GridNode* endNode) const {
	return (hNode->position - endNode->position).Length();
}

void NavigationGrid::PrintGrid()
{
	std::string out;

	for (int i = 11; i >= 0; --i)
	{
		for (int j = 0; j < 12; ++j)
		{
			out = out + std::to_string(grid[i][j]) + " ";
		}
		out += '\n';
	}
	out += '\n';

	std::cout << out;
}

void NavigationGrid::PrintNodeGrid()
{
	/* Don't use this */
	std::string out;

	for (int y = 0; y < gridHeight; ++y) {
		for (int x = 0; x < gridWidth; ++x) {
			GridNode& n = allNodes[(gridWidth * y) + x];
			out = out + std::to_string(n.type) + " ";
		}
		out += '\n';
	}
	out += '\n';


	std::cout << out;
}

void NavigationGrid::UpdateGrid()
{
	// Reset grid to 0's
	for (int i = 11; i >= 0; --i)
	{
		for (int j = 0; j < 12; ++j)
		{
			if(grid[i][j] != 8 && grid[i][j] != 3)
				grid[i][j] = 0;
		}
	}

	for (GameObject* g : gameObjects)
	{
		Vector3 pos = g->GetTransform().GetPosition();
		int index = coordToIndex(pos);
		int row = index / 12;
		int col = index - row * 12;
		if(grid[row][col] != 8 && grid[row][col] != 3)
			grid[row][col] = 1;
	}
}

Vector3 NavigationGrid::indexToCoord(int i)
{
	float tile_size = 20;
	int row;
	int col;

	row = i / 12;
	col = i - row * 12;

	return { col * tile_size + tile_size / 2, 2, -row * tile_size - tile_size / 2 };
}

int NavigationGrid::coordToIndex(Vector3 pos)
{
	float tile_size = 20;
	int row = abs(pos.z / tile_size);
	int col = abs(pos.x / tile_size);

	return row * 12 + col;
}

void NavigationGrid::createConnectivity()
{
	nodeSize = 20;
	gridWidth = 12;
	gridHeight = 12;
	grid[0][0] = 1;

	for (int y = 0; y < gridHeight; ++y) {
		for (int x = 0; x < gridWidth; ++x) {
			GridNode& n = allNodes[(gridWidth * y) + x];
			n.type = grid[y][x];
			n.position = Vector3((float)(x * nodeSize), 0, (float)(y * nodeSize));
		}
	}

	// Bug is here somewhere 
	// Most likely because it reads the map upside down
	
	//now to build the connectivity between the nodes
	for (int y = 0; y < gridHeight; ++y) {
		for (int x = 0; x < gridWidth; ++x) {
			GridNode& n = allNodes[(gridWidth * y) + x];

			if (y > 0) { //get the above node
				n.connected[0] = &allNodes[(gridWidth * (y - 1)) + x];
			}
			if (y < gridHeight - 1) { //get the below node
				n.connected[1] = &allNodes[(gridWidth * (y + 1)) + x];
			}
			if (x > 0) { //get left node
				n.connected[2] = &allNodes[(gridWidth * (y)) + (x - 1)];
			}
			if (x < gridWidth - 1) { //get right node
				n.connected[3] = &allNodes[(gridWidth * (y)) + (x + 1)];
			}
			for (int i = 0; i < 4; ++i) {
				if (n.connected[i]) {
					if (n.connected[i]->type != 8) {
						n.costs[i] = 1;
					}
					if (n.connected[i]->type == 8) {
						n.connected[i] = nullptr; //actually a wall, disconnect!
					}
				}
			}
		}
	}
}

void NCL::CSC8503::NavigationGrid::emplaceObstacle(GameObject* obj)
{
	int index = coordToIndex(obj->getParentedPosition());
	allNodes[index].type = 8;
	
	int x = index / 12;
	int y = index - x * 12;

	grid[x][y] = 8;
}

void NCL::CSC8503::NavigationGrid::emplaceBonus(GameObject* obj)
{
	int index = coordToIndex(obj->getParentedPosition());
	allNodes[index].type = 3;

	int x = index / 12;
	int y = index - x * 12;

	grid[x][y] = 3;
}



void NCL::CSC8503::NavigationGrid::removeBonus(GameObject* obj)
{
	int index = coordToIndex(obj->getParentedPosition());

	allNodes[index].type = 0;
	int x = index / 12;
	int y = index - x * 12;

	grid[x][y] = 0;

	createConnectivity();
}