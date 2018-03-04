//
//  main.cpp
//  your_mobile
//
//  Created by Igor on 30.05.17.
//  Copyright © 2017 Igor. All rights reserved.
//

#include <iostream>
#include <vector>
#include <set>
#include <queue>
#include <utility>
#include <list>
#include <string>
#include <limits>
#include <math.h>


struct coordinate {
  long double x_coord;
  long double y_coord;
  
  coordinate(long double x_coord, long double y_coord)
  : x_coord(x_coord), y_coord(y_coord) {}
};

struct road {
  road(size_t city_from, size_t city_to)
  : city_from(city_from), city_to(city_to) {}
  size_t city_from;
  size_t city_to;
};

struct city {
  explicit city(coordinate position) : position(position) {}
  coordinate position;
  std::vector<size_t> roads_from;
};

class weighted_graph {
  struct edge {
    size_t edge_to;
    long double weight;
    edge(size_t edge_to, long double weight) : edge_to(edge_to), weight(weight) {}
  };
  
  const long double inf = std::numeric_limits<long double>::infinity();
  
  std::vector<std::vector<edge>> edge_list;
  
public:
  const size_t precision_factor = 10 * 1000 * 1000;
  size_t size() { return edge_list.size(); }
  
  explicit weighted_graph(size_t size) { edge_list.resize(size); }
  
  void add_veretex() { edge_list.resize(size() + 1); }
  
  void add_edge(size_t vertex_from, size_t vertex_to, long double weight) {
    edge_list[vertex_from].emplace_back(vertex_to, weight);
  }
  
  size_t start_vertex;
  std::vector<long double> distances;
  std::vector<size_t> distances_diskr;
  
private:
  void calculate_distances(size_t from) {
    start_vertex = from;
    distances.resize(size(), inf);
    distances[start_vertex] = 0;
    
    std::vector<bool> marks(size(), false);
    
    for (size_t iteration = 0; iteration < size(); ++iteration) {
      size_t vertex = -1;
      for (size_t j = 0; j < size(); ++j) {
        if (not marks[j] and
            (vertex == -1 or distances[j] < distances[vertex])) {
          vertex = j;
        }
      }
      if (distances[vertex] == inf) {
        break;
      }
      marks[vertex] = true;
      
      for (size_t j = 0; j < edge_list[vertex].size(); ++j) {
        size_t to = edge_list[vertex][j].edge_to;
        long double weight = edge_list[vertex][j].weight;
        
        if (distances[vertex] + weight < distances[to]) {
          distances[to] = distances[vertex] + weight;
        }
      }
    }
  }
  
public:
  void calculate_distances_log(size_t from) {
    start_vertex = from;
    distances.resize(size(), inf);
    distances[start_vertex] = 0;
    
    std::set<std::pair<long double, size_t>> queue;
    queue.insert(std::make_pair(distances[start_vertex], start_vertex));
    
    while (not queue.empty()) {
      size_t vertex = queue.begin()->second;
      queue.erase(queue.begin());
      
      for (size_t j = 0; j < edge_list[vertex].size(); ++j) {
        size_t to_vertex = edge_list[vertex][j].edge_to;
        long double length = edge_list[vertex][j].weight;
        if (distances[vertex] + length < distances[to_vertex]) {
          queue.erase(std::make_pair(distances[to_vertex], to_vertex));
          distances[to_vertex] = distances[vertex] + length;
          queue.insert(std::make_pair(distances[to_vertex], to_vertex));
        }
      }
    }
  }
  
  void calculate_distances_log_size_t(size_t from) {
    start_vertex = from;
    distances_diskr.resize(size(), -1);
    distances_diskr[start_vertex] = 0;
    
    std::set<std::pair<size_t, size_t>> queue;
    queue.insert(std::make_pair(distances_diskr[start_vertex], start_vertex));
    
    while (not queue.empty()) {
      size_t vertex = queue.begin()->second;
      queue.erase(queue.begin());
      
      for (size_t j = 0; j < edge_list[vertex].size(); ++j) {
        size_t to_vertex = edge_list[vertex][j].edge_to;
        size_t length = round(edge_list[vertex][j].weight * precision_factor);
        if (distances_diskr[vertex] + length < distances_diskr[to_vertex]) {
          queue.erase(std::make_pair(distances_diskr[to_vertex], to_vertex));
          distances_diskr[to_vertex] = distances_diskr[vertex] + length;
          queue.insert(std::make_pair(distances_diskr[to_vertex], to_vertex));
        }
      }
    }
  }
};

long double distance(coordinate first, coordinate second) {
  long double x_dist = first.x_coord - second.x_coord;
  long double y_dist = first.y_coord - second.y_coord;
  
  return sqrtl(x_dist * x_dist + y_dist * y_dist);
}

long double angle_deg(coordinate first, coordinate centre, coordinate last) {
  long double a_len = distance(first, centre);
  long double b_len = distance(centre, last);
  long double c_len = distance(first, last);
  
  long double cos_phi =
  (a_len * a_len + b_len * b_len - c_len * c_len) / (2. * a_len * b_len);
  return acosl(cos_phi) * 90 / acosl(0);
}

void get_input(std::vector<city>& city_list, std::vector<road>& road_list,
               size_t& start_city, size_t& destination_city,
               long double& angle_weight) {
  size_t city_q;
  size_t road_q;
  
  std::cin >> city_q >> road_q >> start_city >> destination_city >>
  angle_weight;
  
  --start_city;
  --destination_city;
  
  for (size_t i = 0; i < city_q; ++i) {
    coordinate city_position(0, 0);
    std::cin >> city_position.x_coord >> city_position.y_coord;
    city_list.emplace_back(city_position);
  }
  
  for (size_t i = 0; i < road_q; ++i) {
    size_t road_from;
    size_t road_to;
    std::cin >> road_from >> road_to;
    road_list.emplace_back(road_from - 1, road_to - 1);
    
    city_list[road_from - 1].roads_from.push_back(i);
  }
}

weighted_graph build_graph(const std::vector<city>& city_list,
                           const std::vector<road>& road_list,
                           size_t start_city, long double angle_weight) {
  weighted_graph graph(road_list.size());
  for (size_t road_in = 0; road_in < road_list.size(); ++road_in) {
    for (auto road_out : city_list[road_list[road_in].city_to].roads_from) {
      coordinate prev_city = city_list[road_list[road_in].city_from].position;
      coordinate curr_city = city_list[road_list[road_in].city_to].position;
      coordinate next_city = city_list[road_list[road_out].city_to].position;
      
      long double metric_cost = distance(curr_city, next_city);
      long double angle_cost =
      angle_weight * (180 - angle_deg(prev_city, curr_city, next_city));
      long double total_cost = metric_cost + angle_cost;
      
      graph.add_edge(road_in, road_out, total_cost);
    }
  }
  
  size_t initial_vertex = graph.size();
  graph.add_veretex();
  for (auto road : city_list[start_city].roads_from) {
    coordinate one_city = city_list[start_city].position;
    coordinate other_city = city_list[road_list[road].city_to].position;
    
    auto weight = distance(one_city, other_city);
    
    graph.add_edge(initial_vertex, road, weight);
  }
  
  graph.calculate_distances_log(initial_vertex);
  return graph;
}

std::vector<size_t> roads_ends_in_city(std::vector<road>& road_list,
                                       size_t city_ind) {
  std::vector<size_t> reic;
  for (size_t i = 0; i < road_list.size(); ++i) {
    if (road_list[i].city_to == city_ind) {
      reic.push_back(i);
    }
  }
  return reic;
}

void solve() {
  std::vector<city> city_list;
  std::vector<road> road_list;
  size_t start_city;
  size_t destination_city;
  long double angle_weight;
  
  get_input(city_list, road_list, start_city, destination_city, angle_weight);
  if (destination_city == start_city) {
    std::cout << 0 << std::endl;
    return;
  }
  auto graph = build_graph(city_list, road_list, start_city, angle_weight);
  auto reic = roads_ends_in_city(road_list, destination_city);
  
  auto inf = std::numeric_limits<long double>::infinity();
  auto min = inf;
  for (auto road_ind : reic) {
    if (graph.distances[road_ind] < min) {
      min = graph.distances[road_ind];
    }
  }
  
  if (min < inf) {
    std::cout.precision(17);
    std::cout << min << std::endl;
  } else {
    std::cout << -1 << std::endl;
  }
}

// Test Graph
//
// int main(int argc, const char * argv[]) {
//  weighted_graph Graph(5);
//  Graph.add_edge(0, 1, 4 * sqrt(2));
//  Graph.add_edge(0, 3, sqrt(17));
//  Graph.add_edge(1, 2, 4 * sqrt(2));
//  Graph.add_edge(3, 4, 2);
//  Graph.add_edge(4, 2, sqrt(17));
//
//   Graph.calculate_distances_log_size_t(0);
//   auto vec_1 = Graph.distances_diskr;
//   Graph.calculate_distances_log(0);
//   auto vec_2 = Graph.distances;
//
//  return 0;
// }

// Test metrics
//
// int main(int argc, const char * argv[]) {
//  coordinate A(0, 0);
//  coordinate B(1, 0);
//  coordinate C(0.5, 0.5 * sqrt(3));
//  std::cout << distance(A, C) << std::endl;
//  std::cout << angle(C, A, B) << std::endl;
//  return 0;
// }

int main(int argc, const char* argv[]) {
  solve();
  return 0;
}
