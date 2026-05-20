#ifndef EDGE
#define EDGE

#include "pointa.h"

// The head of the edge list should be an edge that isn't used in calculations
class Edge {
 public:
  int y_first;    // Final scan line of edge
  int y_last;     // Final scan line of edge
  PointA pointa;  // The values of the edge
  PointA inc;     // The incremental change of the edge's values for each scanline
  Edge* next;     // A pointer to the next edge in a linked list

  Edge() {
    y_first = 0;
    y_last = 0;
    pointa = PointA();
    inc = PointA();
  }

  // Constructs an edge from the higher vertex to the lower vertex
  Edge(PointA v1, PointA v2);
};

void insert_edge(Edge* list, Edge* edge);
void remove_next_edge(Edge* edge);
void sort_table(Edge* list);

#endif  // !EDGE
