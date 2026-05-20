#include "edge.h"

#include <cmath>

Edge::Edge(PointA v1, PointA v2) {
  next = nullptr;

  PointA* upper;
  PointA* lower;

  if (v1.pointh.get_y() > v2.pointh.get_y()) {
    upper = &v1;
    lower = &v2;
  }
  else {
    lower = &v1;
    upper = &v2;
  }

  float dy = upper->pointh.y - lower->pointh.y;
  inc = (*upper - *lower) / dy;

  float factor = std::ceil(lower->pointh.y) - lower->pointh.y;

  pointa = *lower + factor * inc;
  y_first = std::ceil(pointa.pointh.y);
  y_last = std::ceil(upper->pointh.y) - 1;
}

// Inserts an edge into the linked list
void insert_edge(Edge* list, Edge* edge) {
  Edge* q = list;
  Edge* p = q->next;

  while (p != nullptr && (edge->pointa.pointh.x > p->pointa.pointh.x)) {
    // step to the next edge
    q = p;
    p = p->next;
  }
  // link the edge into the list after q
  edge->next = q->next;
  q->next = edge;
}

void remove_next_edge(Edge* edge) {
  Edge* p = edge->next;
  edge->next = p->next;
}

void sort_table(Edge* list) {
  Edge* q;
  Edge* p = list->next;

  list->next = nullptr;

  while (p) {
    q = p->next;
    insert_edge(list, p);
    p = q;
  }
}
