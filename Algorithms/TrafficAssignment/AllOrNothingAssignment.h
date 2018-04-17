#pragma once

#include <algorithm>
#include <array>
#include <cassert>
#include <cmath>
#include <cstdlib>
#include <iostream>
#include <ostream>
#include <vector>

#include "DataStructures/Utilities/OriginDestination.h"
#include "Stats/TrafficAssignment/AllOrNothingAssignmentStats.h"
#include "Tools/CommandLine/ProgressBar.h"
#include "Tools/Simd/AlignVector.h"
#include "Tools/Timer.h"

// Implementation of an iterative all-or-nothing traffic assignment. Each OD-pair is processed in
// turn and the corresponding OD-flow (in our case always a single flow unit) is assigned to each
// edge on the shortest path between O and D. Other O-D paths are not assigned any flow. The
// procedure can be used with different shortest-path algorithms.
template <typename ShortestPathAlgoT>
class AllOrNothingAssignment {
 private:
  using InputGraph = typename ShortestPathAlgoT::InputGraph;

 public:
  // Constructs an all-or-nothing assignment instance.
  AllOrNothingAssignment(const InputGraph& graph,
                         const std::vector<ClusteredOriginDestination>& odPairs,
                         const bool verbose = true)
      : stats(odPairs.size()),
        shortestPathAlgo(graph),
        inputGraph(graph),
        odPairs(odPairs),
        verbose(verbose) {
    Timer timer;
    shortestPathAlgo.preprocess();
    stats.totalPreprocessingTime = timer.elapsed();
    stats.lastRoutingTime = stats.totalPreprocessingTime;
    stats.totalRoutingTime = stats.totalPreprocessingTime;
    if (verbose) std::cout << "  Prepro: " << stats.totalPreprocessingTime << "ms" << std::endl;
  }

  // Assigns all OD-flows to their currently shortest paths.
  void run(const int samplingInterval = 1) {
    Timer timer;
    ++stats.numIterations;
    if (verbose) std::cout << "Iteration " << stats.numIterations << ": " << std::flush;
    shortestPathAlgo.customize();
    stats.lastCustomizationTime = timer.elapsed();

    timer.restart();
    ProgressBar bar(std::ceil(1.0 * odPairs.size() / (K * samplingInterval)), verbose);
    trafficFlows.assign(inputGraph.numEdges() + shortestPathAlgo.getNumShortcuts(), 0);
    stats.startIteration();
    int totalNumPairsSampledBefore = 0;
    #pragma omp parallel
    {
      auto queryAlgo = shortestPathAlgo.getQueryAlgoInstance();
      std::vector<int> flows(inputGraph.numEdges() + shortestPathAlgo.getNumShortcuts());
      int64_t checksum = 0;
      double avgChange = 0;
      double maxChange = 0;
      int numPairsSampledBefore = 0;

      #pragma omp for schedule(dynamic, 64) nowait
      for (int i = 0; i < odPairs.size(); i += K * samplingInterval) {
        // Run multiple shortest-path computations simultaneously.
        std::array<int, K> sources;
        std::array<int, K> targets;
        sources.fill(odPairs[i].origin);
        targets.fill(odPairs[i].destination);
        int k = 1;
        for (; k < K && i + k * samplingInterval < odPairs.size(); ++k) {
          sources[k] = odPairs[i + k * samplingInterval].origin;
          targets[k] = odPairs[i + k * samplingInterval].destination;
        }
        queryAlgo.run(sources, targets);

        for (int j = 0; j < k; ++j) {
          // Maintain the avg and max change in the OD-distances between the last two iterations.
          const int dst = odPairs[i + j * samplingInterval].destination;
          const int dist = queryAlgo.getDistance(dst, j);
          const int prevDist = stats.lastDistances[i + j * samplingInterval];
          const double change = 1.0 * std::abs(dist - prevDist) / prevDist;
          numPairsSampledBefore += prevDist != -1;
          checksum += dist;
          stats.lastDistances[i + j * samplingInterval] = dist;
          avgChange += std::max(0.0, change);
          maxChange = std::max(maxChange, change);

          // Assign the OD-flow to each edge on the computed path.
          for (const auto e : queryAlgo.getPackedEdgePath(dst, j)) {
            assert(e >= 0); assert(e < flows.size());
            ++flows[e];
          }
        }
        ++bar;
      }

      #pragma omp critical (mergeResults)
      {
        stats.lastChecksum += checksum;
        stats.avgChangeInDistances += avgChange;
        stats.maxChangeInDistances = std::max(stats.maxChangeInDistances, maxChange);
        totalNumPairsSampledBefore += numPairsSampledBefore;
        assert(trafficFlows.size() == flows.size());
        for (int e = 0; e < flows.size(); ++e)
          trafficFlows[e] += flows[e] * samplingInterval;
      }
    }
    bar.finish();

    // Propagate traffic flows from shortcut to original edges.
    const int maxShortcutId = inputGraph.numEdges() + shortestPathAlgo.getNumShortcuts() - 1;
    for (int s = maxShortcutId; s >= inputGraph.numEdges(); --s) {
      trafficFlows[shortestPathAlgo.getShortcutsFirstEdge(s)] += trafficFlows[s];
      trafficFlows[shortestPathAlgo.getShortcutsSecondEdge(s)] += trafficFlows[s];
    }
    stats.lastQueryTime = timer.elapsed();
    stats.avgChangeInDistances /= totalNumPairsSampledBefore;
    stats.finishIteration();

    if (verbose) {
      std::cout << " done.\n";
      std::cout << "  Checksum: " << stats.lastChecksum;
      std::cout << "  Custom: " << stats.lastCustomizationTime << "ms";
      std::cout << "  Queries: " << stats.lastQueryTime << "ms";
      std::cout << "  Routing: " << stats.lastRoutingTime << "ms\n";
      std::cout << std::flush;
    }
  }

  // Returns the traffic flow on edge e.
  const int& trafficFlowOn(const int e) const {
    assert(e >= 0); assert(e < inputGraph.numEdges());
    return trafficFlows[e];
  }

  AllOrNothingAssignmentStats stats; // Statistics about the execution.

 private:
  // The maximum number of simultaneous shortest-path computations.
  static constexpr int K = ShortestPathAlgoT::K;

  using ODPairs = std::vector<ClusteredOriginDestination>;

  ShortestPathAlgoT shortestPathAlgo; // Algo computing shortest paths between OD-pairs.
  const InputGraph& inputGraph;       // The input graph.
  const ODPairs& odPairs;             // The OD-pairs to be assigned onto the graph.
  AlignVector<int> trafficFlows;      // The traffic flows on the edges.
  const bool verbose;                 // Should informative messages be displayed?
};
