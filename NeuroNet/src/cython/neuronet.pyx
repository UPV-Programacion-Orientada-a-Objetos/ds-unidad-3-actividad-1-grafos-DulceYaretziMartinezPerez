# distutils: language = c++
# distutils: sources = src/cpp/Graph.cpp

from libcpp.vector cimport vector
from libcpp.string cimport string
from neuronet_core cimport SparseGraph

cdef class NeuroNetEngine:
    cdef SparseGraph* c_graph

    def __cinit__(self):
        self.c_graph = new SparseGraph()

    def __dealloc__(self):
        del self.c_graph

    def load_dataset(self, filename: str):
        """Loads a dataset from a file."""
        cdef string c_filename = filename.encode('utf-8')
        self.c_graph.loadFromFile(c_filename)

    def bfs(self, start_node: int, max_depth: int):
        """Runs BFS from start_node up to max_depth."""
        return self.c_graph.bfs(start_node, max_depth)

    def get_degree(self, node: int):
        """Returns the degree of a node."""
        return self.c_graph.getDegree(node)

    def get_max_degree_node(self):
        """Returns the node ID with the maximum degree."""
        return self.c_graph.getMaxDegreeNode()
        
    def get_num_nodes(self):
        return self.c_graph.getNumNodes()
        
    def get_num_edges(self):
        return self.c_graph.getNumEdges()

    def get_neighbors(self, node: int):
        return self.c_graph.getNeighbors(node)
