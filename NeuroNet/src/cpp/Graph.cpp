#include "Graph.h"
#include <algorithm>
#include <cstring> // for memset
#include <fstream>
#include <iostream>
#include <sstream>

SparseGraph::SparseGraph()
    : num_nodes(0), num_edges(0), values(nullptr), col_indices(nullptr),
      row_ptr(nullptr), reverse_map(nullptr) {}

SparseGraph::~SparseGraph() {
  if (values)
    delete[] values;
  if (col_indices)
    delete[] col_indices;
  if (row_ptr)
    delete[] row_ptr;
  if (reverse_map)
    delete[] reverse_map;
}

// Helper struct for sorting edges
struct Edge {
  int u, v;
  bool operator<(const Edge &other) const {
    if (u != other.u)
      return u < other.u;
    return v < other.v;
  }
};

void SparseGraph::loadFromFile(const std::string &filename) {
  std::cout << "[C++ Core] Loading dataset '" << filename
            << "' (Manual Pointers)..." << std::endl;

  // Pass 1: Count edges and identify unique nodes to build map
  std::ifstream file(filename);
  if (!file.is_open()) {
    std::cerr << "[C++ Core] Error: Could not open file " << filename
              << std::endl;
    return;
  }

  std::string line;
  int edge_count = 0;

  // We need to build the ID map first to know num_nodes
  // For massive graphs, using std::map is the only sane way to handle sparse
  // IDs without a huge array unless we do coordinate compression with sorting,
  // which is also heavy. We will stick to std::map for the mapping phase as
  // it's auxiliary.

  // To avoid storing all edges in a vector, we will count them first.
  while (std::getline(file, line)) {
    if (line.empty() || line[0] == '#')
      continue;
    std::stringstream ss(line);
    int u, v;
    if (ss >> u >> v) {
      edge_count++;
      if (id_map.find(u) == id_map.end()) {
        int new_id = static_cast<int>(id_map.size());
        id_map[u] = new_id;
      }
      if (id_map.find(v) == id_map.end()) {
        int new_id = static_cast<int>(id_map.size());
        id_map[v] = new_id;
      }
    }
  }
  file.clear();
  file.seekg(0, std::ios::beg);

  num_nodes = static_cast<int>(id_map.size());
  num_edges = edge_count;

  std::cout << "[C++ Core] Counted " << num_edges << " edges and " << num_nodes
            << " nodes." << std::endl;

  // Allocate memory for CSR and Reverse Map
  row_ptr = new int[num_nodes + 1];
  col_indices = new int[num_edges];
  values = new int[num_edges];
  reverse_map = new int[num_nodes];

  // Build reverse map
  for (auto const &[key, val] : id_map) {
    reverse_map[val] = key;
  }

  // Read edges into a temporary raw array to sort them
  Edge *edge_list = new Edge[num_edges];
  int idx = 0;

  while (std::getline(file, line)) {
    if (line.empty() || line[0] == '#')
      continue;
    std::stringstream ss(line);
    int u, v;
    if (ss >> u >> v) {
      edge_list[idx].u = id_map[u];
      edge_list[idx].v = id_map[v];
      idx++;
    }
  }
  file.close();

  // Sort edges manually (using std::sort on raw pointers)
  std::sort(edge_list, edge_list + num_edges);

  // Build CSR
  // Initialize row_ptr to 0
  std::memset(row_ptr, 0, (num_nodes + 1) * sizeof(int));

  int current_row = 0;
  for (int i = 0; i < num_edges; ++i) {
    int u = edge_list[i].u;
    int v = edge_list[i].v;

    // When we move to a new row, fill the gaps
    while (current_row < u) {
      current_row++;
      row_ptr[current_row] = i;
    }

    col_indices[i] = v;
    values[i] = 1;
  }
  // Finish row_ptr
  while (current_row < num_nodes) {
    current_row++;
    row_ptr[current_row] = num_edges;
  }
  row_ptr[num_nodes] = num_edges; // Sentinel

  // Clean up temp edge list
  delete[] edge_list;

  std::cout << "[C++ Core] CSR Structure built with raw pointers." << std::endl;
}

int SparseGraph::getInternalId(int externalId) {
  auto it = id_map.find(externalId);
  if (it != id_map.end()) {
    return it->second;
  }
  return -1;
}

int SparseGraph::getExternalId(int internalId) {
  if (internalId >= 0 && internalId < num_nodes) {
    return reverse_map[internalId];
  }
  return -1;
}

std::vector<int> SparseGraph::bfs(int startNodeExt, int maxDepth) {
  std::vector<int>
      visited_ext; // Return type still vector for Cython compatibility
  int startNode = getInternalId(startNodeExt);

  if (startNode == -1) {
    std::cerr << "[C++ Core] Start node not found." << std::endl;
    return visited_ext;
  }

  // Manual Queue implementation
  int *queue_nodes = new int[num_nodes]; // Max size is num_nodes
  int *queue_depths = new int[num_nodes];
  int head = 0;
  int tail = 0;

  // Manual Visited array
  bool *visited = new bool[num_nodes];
  std::memset(visited, 0, num_nodes * sizeof(bool));

  // Push start node
  queue_nodes[tail] = startNode;
  queue_depths[tail] = 0;
  tail++;

  visited[startNode] = true;
  visited_ext.push_back(startNodeExt);

  while (head < tail) {
    int u = queue_nodes[head];
    int d = queue_depths[head];
    head++;

    if (d >= maxDepth)
      continue;

    int start_idx = row_ptr[u];
    int end_idx = row_ptr[u + 1];

    for (int i = start_idx; i < end_idx; ++i) {
      int v = col_indices[i];
      if (!visited[v]) {
        visited[v] = true;
        visited_ext.push_back(getExternalId(v));

        queue_nodes[tail] = v;
        queue_depths[tail] = d + 1;
        tail++;
      }
    }
  }

  delete[] queue_nodes;
  delete[] queue_depths;
  delete[] visited;

  return visited_ext;
}

int SparseGraph::getDegree(int nodeExt) {
  int u = getInternalId(nodeExt);
  if (u == -1)
    return 0;
  return row_ptr[u + 1] - row_ptr[u];
}

int SparseGraph::getMaxDegreeNode() {
  int max_deg = -1;
  int max_node = -1;

  for (int i = 0; i < num_nodes; ++i) {
    int deg = row_ptr[i + 1] - row_ptr[i];
    if (deg > max_deg) {
      max_deg = deg;
      max_node = i;
    }
  }

  return getExternalId(max_node);
}

std::vector<int> SparseGraph::getNeighbors(int nodeExt) {
  std::vector<int> neighbors;
  int u = getInternalId(nodeExt);
  if (u == -1)
    return neighbors;

  int start_idx = row_ptr[u];
  int end_idx = row_ptr[u + 1];

  for (int i = start_idx; i < end_idx; ++i) {
    neighbors.push_back(getExternalId(col_indices[i]));
  }
  return neighbors;
}
