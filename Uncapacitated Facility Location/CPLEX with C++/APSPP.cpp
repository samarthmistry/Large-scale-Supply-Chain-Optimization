// Solving the all-pairs shortest path problem to get distance-based costs.
//
// -------------------------------------------------------------- -*- C++ -*-
// File: apspp.cpp
// --------------------------------------------------------------------------

#include <iostream>
#include<vector>
#include <fstream>

#define INF 999999 

int main(int argc, char **argv)
{
	const char* filename = "apspp.dat";
	if (argc >= 2) filename = argv[1];
	std::ifstream file(filename);
	if (!file) {
		std::cerr << "No such file: " << filename << std::endl;
		throw(-1);
	}
		
	int nodes; // the dimension of the matrix
	file >> nodes;

	int** adj = new int*[nodes]; // each element is a pointer to an array.

	for (int i = 0; i < nodes; i++)
		adj[i] = new int[nodes]; // build rows

	for (int i = 0; i < nodes; i++) {
		for (int j = 0; j < nodes; j++) {
			file >> adj[i][j];
			printf("%7d", adj[i][j]);
		}
		printf("\n");
	}

	int** dist = new int*[nodes]; // each element is a pointer to an array.

	for (int i = 0; i < nodes; i++)
		dist[i] = new int[nodes]; // build rows

	for (int i = 0; i < nodes; i++) {
		for (int j = 0; j < nodes; j++) {
			if (adj[i][j] == -1) {
				dist[i][j] = INF;
			}
			else {
				dist[i][j] = adj[i][j];
			}
		}
		dist[i][i] = 0;
	}

	for (int k = 0; k < nodes; k++) {
		for (int i = 0; i < nodes; i++)	{
			for (int j = 0; j < nodes; j++) {
				if (dist[i][k] + dist[k][j] < dist[i][j])
					dist[i][j] = dist[i][k] + dist[k][j];
			}
		}
	}

	printf("The following matrix shows the shortest distances"
		" between every pair of vertices \n");
	for (int i = 0; i < nodes; i++)	{
		for (int j = 0; j < nodes; j++)	{
			if (dist[i][j] == INF)
				printf("%7s", "INF");
			else
				printf("%7d", dist[i][j]);
		}
		printf("\n");
	}

	for (int i = 0; i < nodes; i++)
		delete adj[i], dist[i];
	delete adj, dist;
}
