from libcpp.vector cimport vector
from libcpp.string cimport string

cdef extern from "../cpp/Graph.h":
    cdef cppclass IGraph:
        pass

    cdef cppclass SparseGraph(IGraph):
        SparseGraph() except +
        void loadFromFile(string filename)
        vector[int] bfs(int startNode, int maxDepth)
        int getDegree(int node)
        int getMaxDegreeNode()
        int getNumNodes()
        int getNumEdges()
        vector[int] getNeighbors(int node)
