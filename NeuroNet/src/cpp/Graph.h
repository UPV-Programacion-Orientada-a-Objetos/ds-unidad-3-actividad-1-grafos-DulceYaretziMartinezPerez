#ifndef GRAPH_H
#define GRAPH_H

#include <algorithm>
#include <fstream>
#include <iostream>
#include <map>
#include <sstream>
#include <string>
#include <vector>

// Abstract Base Class
class IGraph {
public:
  virtual ~IGraph() {}
  virtual void loadFromFile(const std::string &filename) = 0;
  virtual std::vector<int> bfs(int startNode, int maxDepth) = 0;
  virtual int getDegree(int node) = 0;
  virtual int getMaxDegreeNode() = 0;
  virtual int getNumNodes() = 0;
  virtual int getNumEdges() = 0;
};

// Concrete Class using CSR (Compressed Sparse Row) with Manual Pointers
class SparseGraph : public IGraph {
private:
  // CSR format using raw pointers
  int *values; // Not strictly needed for unweighted, but kept for structure
  int *col_indices; // Column indices
  int *row_ptr;     // Row pointers

  int num_nodes;
  int num_edges;

  // Mapping from original IDs to internal 0-indexed IDs
  std::map<int, int> id_map; // external -> internal
  // We can use a raw array for reverse map if we know num_nodes
  int *reverse_map; // internal -> external

public:
  SparseGraph();
  ~SparseGraph(); // Destructor for manual cleanup

  void loadFromFile(const std::string &filename) override;
  std::vector<int> bfs(int startNode, int maxDepth) override;
  int getDegree(int node) override;
  int getMaxDegreeNode() override;
  int getNumNodes() override { return num_nodes; }
  int getNumEdges() override { return num_edges; }

  std::vector<int> getNeighbors(int node);

  int getInternalId(int externalId);
  int getExternalId(int internalId);
};

#endif // GRAPH_H
