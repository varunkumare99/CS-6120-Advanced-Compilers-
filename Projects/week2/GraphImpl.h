#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/compressed_sparse_row_graph.hpp>
#include <boost/graph/graphviz.hpp>
#include <iostream>
#include <map>
#include <vector>

using namespace std;
using namespace boost;

struct Vertex {
  string name;
};

struct Edge {
  string name;
};

class Graph {
private:
  using graph_t = adjacency_list<listS, vecS, directedS, Vertex, Edge>;
  using vertex_t = graph_traits<graph_t>::vertex_descriptor;
  using edge_t = graph_traits<graph_t>::edge_descriptor;

  int m_numOfVertices;
  map<string, int> m_name2int;
  map<string, vertex_t> m_vertices;
  ofstream m_outputFile;
  graph_t m_graph;

public:
  Graph(int vertices, vector<string> vertexNames, string outputFileName)
      : m_numOfVertices(vertices) {
    for (int i = 0; i < vertexNames.size(); ++i) {
      m_name2int[vertexNames[i]] = i;
      m_vertices[vertexNames[i]] =
          (boost::add_vertex(Vertex{vertexNames[i]}, m_graph));
    }
    m_outputFile = ofstream{outputFileName};
  }

  void addEdge(string from, string to, string edgeLabel = "") {
    boost::add_edge(m_vertices[from], m_vertices[to], Edge{edgeLabel}, m_graph);
  }

  void createDotFile() {
    // write to stdout to verify tests
    boost::write_graphviz(
        /* m_outputFile, m_graph, */
        cout, m_graph,
        [&](auto &out, auto vertex) {
          out << "[label=\"" << m_graph[vertex].name << "\"]";
        },
        [&](auto &out, auto edge) {
          out << "[label=\"" << m_graph[edge].name << "\"]";
        });
    cout << flush;
    // write to generate svg files
    boost::write_graphviz(
        m_outputFile, m_graph,
        [&](auto &out, auto vertex) {
          out << "[label=\"" << m_graph[vertex].name << "\"]";
        },
        [&](auto &out, auto edge) {
          out << "[label=\"" << m_graph[edge].name << "\"]";
        });
  }
};
