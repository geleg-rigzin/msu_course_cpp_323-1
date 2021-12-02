#include "graph_generator.hpp"
#include <atomic>
#include <functional>
#include <iostream>
#include <list>
#include <random>
#include <thread>

using VertexId = uni_cpp_practice::VertexId;
using Graph = uni_cpp_practice::Graph;

constexpr int MAX_THREADS_COUNT = 4;

namespace {

constexpr float GREEN_EDGE_PROBABILITY = 0.1;
constexpr float BLUE_EDGE_PROBABILITY = 0.25;
constexpr float RED_EDGE_PROBABILITY = 0.33;
float get_random_probability() {
  std::random_device rd;
  std::mt19937 mt(rd());
  std::uniform_real_distribution<float> probability(0.0, 1);
  return probability(mt);
}

VertexId get_random_vertex_id(const std::vector<VertexId>& vertices) {
  std::random_device rd;
  std::mt19937 mt(rd());
  std::uniform_int_distribution<int> random_vertex_distribution(
      0, vertices.size() - 1);
  return vertices[random_vertex_distribution(mt)];
}

std::vector<VertexId> filter_connected_vertices(
    const VertexId& id,
    const std::vector<VertexId>& vertex_ids,
    const Graph& graph) {
  std::vector<VertexId> result;
  for (const auto& vertex_id : vertex_ids) {
    if (!graph.are_vertices_connected(id, vertex_id)) {
      result.push_back(vertex_id);
    }
  }
  return result;
}
}  // namespace

namespace uni_cpp_practice {

void GraphGenerator::generate_gray_branch(Graph& graph,
                                          std::mutex& mutex,
                                          const VertexId& source_vertex_id,
                                          const VertexDepth depth) const {
  const auto new_vertex_id = [&graph, &mutex, &source_vertex_id]() {
    const std::lock_guard lock(mutex);
    const auto& new_vertex_id = graph.insert_vertex();
    graph.insert_edge(source_vertex_id, new_vertex_id);
    return new_vertex_id;
  }();
  if (depth == params_.max_depth) {
    return;
  }
  const float probability = 1 - (float)depth / (float)params_.max_depth;
  for (int i = 0; i < params_.new_vertices_num; ++i) {
    if (get_random_probability() > probability) {
      generate_gray_branch(graph, mutex, new_vertex_id, depth + 1);
    }
  }
}
void GraphGenerator::generate_vertices_and_gray_edges(
    Graph& graph,
    const VertexId& source_vertex_id) const {
  using JobCallback = std::function<void()>;
  auto jobs = std::list<JobCallback>();
  enum class State { Idle, Working, ShouldTerminate };
  std::atomic<State> state = State::Idle;
  std::atomic<int> jobs_count = 0;
  std::mutex job_mutex;
  VertexDepth depth = 0;

  for (int i = 0; i < params_.new_vertices_num; i++) {
    jobs.emplace_back(
        [this, &graph, &job_mutex, &jobs_count, &source_vertex_id, depth]() {
          generate_gray_branch(graph, job_mutex, source_vertex_id, depth + 1);
          ++jobs_count;
        });
  }

  std::mutex worker_mutex;
  state = State::Working;
  const auto worker = [&state, &worker_mutex, &jobs]() {
    while (true) {
      if (state == State::ShouldTerminate) {
        return;
      }
      const auto job_optional = [&worker_mutex,
                                 &jobs]() -> std::optional<JobCallback> {
        const std::lock_guard lock(worker_mutex);
        if (jobs.empty()) {
          return std::nullopt;
        }
        const auto first_job = jobs.front();
        jobs.pop_front();
        return first_job;
      }();
      if (job_optional.has_value()) {
        const auto& job = job_optional.value();
        job();
      }
    }
  };

  std::array<std::thread, MAX_THREADS_COUNT> threads;
  for (int i = 0; i < MAX_THREADS_COUNT; ++i) {
    threads[i] = std::thread(worker);
  }

  while (jobs_count < params_.new_vertices_num) {
  }

  state = State::ShouldTerminate;
  for (auto& thread : threads) {
    thread.join();
  }

  if (params_.max_depth != graph.depth()) {
    std::cout << "Max depth couldn't be reached. Depth of final vertex: "
              << graph.depth() << "\n";
  }
}

void generate_green_edges(Graph& graph, std::mutex& mutex) {
  for (const auto& vertex : graph.get_vertices()) {
    if (get_random_probability() < GREEN_EDGE_PROBABILITY) {
      const std::lock_guard lock(mutex);
      graph.insert_edge(vertex.id, vertex.id);
    }
  }
}

void generate_blue_edges(Graph& graph, std::mutex& mutex) {
  for (int depth = 0; depth < graph.depth(); depth++) {
    const auto& vertices_in_depth = graph.get_vertices_in_depth(depth);
    for (VertexId j = 0; j < vertices_in_depth.size() - 1; j++) {
      const auto source = vertices_in_depth[j];
      const auto destination = vertices_in_depth[j + 1];
      if (get_random_probability() < BLUE_EDGE_PROBABILITY) {
        const std::lock_guard lock(mutex);
        graph.insert_edge(source, destination);
      }
    }
  }
}

void generate_yellow_edges(Graph& graph, std::mutex& mutex) {
  for (VertexDepth depth = 0; depth < graph.depth(); depth++) {
    float probability = 1 - (float)depth * (1 / (float)(graph.depth() - 1));
    const auto& vertices = graph.get_vertices_in_depth(depth);
    const auto& vertices_next = graph.get_vertices_in_depth(depth + 1);
    for (const auto& vertex_id : vertices) {
      if (get_random_probability() > probability) {
        const auto& filtered_vertex_ids =
            filter_connected_vertices(vertex_id, vertices_next, graph);
        if (filtered_vertex_ids.empty()) {
          continue;
        }
        VertexId random_vertex_id = get_random_vertex_id(filtered_vertex_ids);
        const std::lock_guard lock(mutex);
        graph.insert_edge(vertex_id, random_vertex_id);
      }
    }
  }
}

void generate_red_edges(Graph& graph, std::mutex& mutex) {
  for (VertexDepth depth = 0; depth < graph.depth() - 1; depth++) {
    const auto& vertices = graph.get_vertices_in_depth(depth);
    const auto& vertices_next = graph.get_vertices_in_depth(depth + 2);
    for (const auto& vertex : vertices) {
      if (get_random_probability() < RED_EDGE_PROBABILITY) {
        const std::lock_guard lock(mutex);
        graph.insert_edge(vertex, get_random_vertex_id(vertices_next));
      }
    }
  }
}

Graph GraphGenerator::generate() const {
  Graph graph;
  const auto vertex_zero = graph.insert_vertex();

  std::mutex mutex;

  generate_vertices_and_gray_edges(graph, vertex_zero);
  std::thread green_thread(generate_green_edges, std::ref(graph),
                           std::ref(mutex));

  std::thread blue_thread(generate_blue_edges, std::ref(graph),
                          std::ref(mutex));

  std::thread yellow_thread(generate_yellow_edges, std::ref(graph),
                            std::ref(mutex));

  std::thread red_thread(generate_red_edges, std::ref(graph), std::ref(mutex));

  green_thread.join();
  blue_thread.join();
  yellow_thread.join();
  red_thread.join();

  return graph;
}
}  // namespace uni_cpp_practice
