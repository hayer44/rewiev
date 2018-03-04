#include <vector>
#include <iostream>
#include <algorithm>
#include <istream>
#include <utility>

using std::vector;
using std::istream;

class Graph {
  vector<vector<size_t>> transitions_;

public:
  explicit Graph(size_t size) { transitions_.resize(size); }
  void AddTransition(size_t source, size_t destination) {
    transitions_[source].push_back(destination);
  }
  size_t size() const { return transitions_.size(); }
  size_t GetTransition(size_t source, size_t transition_number) const {
    return transitions_[source][transition_number];
  }
  const std::vector<size_t>& GetTransitions(size_t source) const {
    return transitions_[source];
  }
  size_t TransitionsAmount(size_t source) const {
    return transitions_[source].size();
  }
};

const std::vector<size_t>& OutgoingEdges(const Graph& graph, size_t vertex) {
  return graph.GetTransitions(vertex);
}

size_t GetTarget(const Graph& graph, size_t edge) { return edge; }

struct ComponentsDescription {
  vector<size_t> elements_components;
  vector<size_t> component_sizes;
};

Graph GetTransposedGraph(const Graph& graph);

// SC ~ strongly connected
ComponentsDescription ComputeComponentsSC(const Graph& graph,
                                         const Graph& graph_tr);

ComponentsDescription ComputeComponentsSC(const Graph& graph);

// Compute components that has input edges
vector<char> IsComponentSuccessor(const Graph& graph,
                                  const ComponentsDescription& description);

size_t ComputeCompanySize(const ComponentsDescription& description,
                          const vector<char>& underlying_comp,
                          size_t cand_amount);

std::vector<std::pair<size_t, size_t>> GetWinsList(std::istream& input_stream);

size_t ComputeCompanySize(const Graph& wins_graph);

Graph BuildWinsGraph(size_t cand_amount,
                     const std::vector<std::pair<size_t, size_t>>& wins_list);

namespace traverses {

  template <class Vertex, class Graph, class Visitor>
  void RunDfs(Vertex vertex, const Graph& graph, Visitor& visitor,
              std::vector<char>& used) {
    used[vertex] = true;
    visitor.DiscoverVertex(vertex);
    auto source_edges = OutgoingEdges(graph, vertex);
    for (const auto& edge : source_edges) {
      auto destination_vertex = GetTarget(graph, edge);
      visitor.ExamineEdge(vertex, edge);
      if (!used[destination_vertex]) {
        RunDfs(destination_vertex, graph, visitor, used);
      }
    }
    visitor.FinishVertex(vertex);
  }

  template <class Vertex, class Graph, class Visitor>
  void DFS(Vertex start_vertex, const Graph& graph, Visitor& visitor,
           std::vector<char>& used) {
    RunDfs(start_vertex, graph, visitor, used);
  }

  template <class Vertex, class Graph, class Visitor>
  void DFS(Vertex start_vertex, const Graph& graph, Visitor& visitor) {
    std::vector<char> used(graph.size(), false);
    RunDfs(start_vertex, graph, visitor, used);
  }

  template <class Vertex, class Edge>
  class DfsVisitor {
  public:
    virtual void DiscoverVertex(Vertex /*vertex*/) {}
    virtual void FinishVertex(Vertex /*vertex*/) {}
    virtual void ExamineEdge(Vertex /*vertex*/, const Edge& /*edge*/) {}
    virtual ~DfsVisitor() = default;
  };

}  // namespace traverses

int main() {
  size_t candidats_amount;
  std::cin >> candidats_amount;
  auto wins_list = GetWinsList(std::cin);
  auto wins_graph = BuildWinsGraph(candidats_amount, wins_list);
  auto answer = ComputeCompanySize(wins_graph);

  std::cout << answer << "\n";
}

size_t ComputeCompanySize(const Graph& wins_graph) {
  vector<size_t> components;
  vector<size_t> sizes;
  ComponentsDescription description = ComputeComponentsSC(wins_graph);
  auto successors = IsComponentSuccessor(wins_graph, description);
  return ComputeCompanySize(description, successors, wins_graph.size());
}

Graph BuildWinsGraph(size_t cand_amount,
                     const std::vector<std::pair<size_t, size_t>>& wins_list) {
  Graph wins_graph(cand_amount);

  for (const auto& win : wins_list) {
    wins_graph.AddTransition(win.first, win.second);
  }

  return wins_graph;
}

std::vector<std::pair<size_t, size_t>> GetWinsList(std::istream& input_stream) {
  std::vector<std::pair<size_t, size_t>> wins_list;
  size_t amount_of_games;
  input_stream >> amount_of_games;
  wins_list.reserve(amount_of_games);
  const size_t DRAW = 3;
  const size_t DEFEAT = 2;

  for (size_t game = 0; game < amount_of_games; ++game) {
    size_t first;
    size_t second;
    size_t result;
    input_stream >> first >> second >> result;
    --first, --second;
    if (result == DRAW) {
      continue;
    } else if (result == DEFEAT) {
      std::swap(first, second);
    }
    wins_list.emplace_back(first, second);
  }
  return wins_list;
}

Graph GetTransposedGraph(const Graph& graph) {
  Graph transposed(graph.size());

  for (size_t source = 0; source < graph.size(); ++source) {
    for (size_t i = 0; i < graph.TransitionsAmount(source); ++i) {
      transposed.AddTransition(graph.GetTransition(source, i), source);
    }
  }
  return transposed;
}


class OrderComputerVisitor : public traverses::DfsVisitor<size_t, size_t> {
private:
  vector<size_t>& order;
public:
  explicit OrderComputerVisitor(vector<size_t>& order) : order(order) {}
  void FinishVertex(size_t vertex) override { order.push_back(vertex); }
};


class ComponentsDeterminer : public traverses::DfsVisitor<size_t, size_t> {
private:
  ComponentsDescription& description;
  size_t component_number = 0;
public:
  explicit ComponentsDeterminer(ComponentsDescription& description)
  : description(description) { }
  void DiscoverVertex(size_t vertex) override {
    description.elements_components[vertex] = component_number;
    ++description.component_sizes[component_number];
  }
  void IncrementComponentNumber() {
    ++component_number;
  }
};

ComponentsDescription ComputeComponentsSC(const Graph& graph) {
  auto transposed_graph = GetTransposedGraph(graph);
  return ComputeComponentsSC(graph, transposed_graph);
}

ComponentsDescription ComputeComponentsSC(const Graph& graph,
                                         const Graph& graph_tr) {
  size_t graph_size = graph.size();
  vector<char> order_computer_used(graph_size, false);
  ComponentsDescription description;
  description.elements_components.resize(graph_size);

  vector<size_t> order;
  OrderComputerVisitor order_computer(order);
  for (size_t i = 0; i < graph_size; ++i) {
    if (!order_computer_used[i]) {
      traverses::RunDfs(i, graph, order_computer, order_computer_used);
    }
  }

  vector<char> components_computer_used(graph_size, false);
  ComponentsDeterminer components_computer(description);
  for (size_t i = 0; i < graph_size; ++i) {
    size_t start_vertex = order[graph_size - 1 - i];
    if (!components_computer_used[start_vertex]) {
      description.component_sizes.push_back(0);
      traverses::RunDfs(start_vertex, graph_tr, components_computer, components_computer_used);
      components_computer.IncrementComponentNumber();
    }
  }

  return description;
}


class ComponentSuccessorSearchVisitor
: public traverses::DfsVisitor<size_t, size_t> {
private:
  const ComponentsDescription& description;
  vector<char> underlied_comp;

public:
  explicit ComponentSuccessorSearchVisitor(
                                           const ComponentsDescription& description)
  : description(description), underlied_comp(description.component_sizes.size(), false) { }

  void ExamineEdge(size_t vertex, const size_t& edge) override {
    if (description.elements_components[vertex] !=
        description.elements_components[edge]) {
      underlied_comp[description.elements_components[edge]] = true;
    }
  }

  const vector<char>& UnderliedComponents() { return underlied_comp; }
};

vector<char> IsComponentSuccessor(const Graph& graph,
                                  const ComponentsDescription& description) {
  vector<char> used(graph.size(), false);
  vector<size_t> order;
  ComponentSuccessorSearchVisitor visitor(description);
  for (size_t i = 0; i < description.elements_components.size(); ++i) {
    if (!used[i]) {
      traverses::DFS(i, graph, visitor, used);
    }
  }
  return visitor.UnderliedComponents();
}


size_t ComputeCompanySize(const ComponentsDescription& description,
                          const vector<char>& underlying_comp,
                          size_t cand_amount) {
  size_t min_size = cand_amount + 1;
  const auto& comp_size = description.component_sizes;
  for (size_t i = 0; i < comp_size.size(); ++i) {
    if ((!underlying_comp[i]) && (comp_size[i] <= min_size)) {
      min_size = comp_size[i];
    }
  }
  return cand_amount - min_size + 1;
}
