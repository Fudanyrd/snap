========================================================================
    Opinion Leader Mining (意见领袖挖掘)
========================================================================

This program implements a comprehensive opinion leader mining algorithm based on
multiple centrality measures. It identifies the most influential, authoritative,
and hub nodes in a network by combining various centrality metrics.

本程序实现了基于多中心性度量的综合意见领袖挖掘算法，通过整合多种中心性指标
来识别网络中最具影响力、权威性和中枢性的节点。

Author: Xitong Wang (王熙同)

/////////////////////////////////////////////////////////////////////////////
Features (功能特性):

1. Multiple Centrality Measures (多中心性度量):
   - PageRank: Measures node importance based on link structure
   - HITS (Hubs & Authorities): Identifies hub and authority nodes
   - Betweenness Centrality: Measures node's role as a bridge
   - Closeness Centrality: Measures average distance to all other nodes
   - Eigenvector Centrality: Measures influence based on neighbors' importance
   - Degree Centrality: Simple measure of node connectivity
   - Clustering Coefficient: Measures local connectivity

2. Composite Scoring (综合评分):
   - Combines multiple centrality measures with weighted average
   - Default weights: PageRank(25%), Authority(20%), Betweenness(20%), 
     Closeness(15%), Eigenvector(10%), Hub(5%), Degree(5%)
   - Ranks nodes by composite influence score

3. Network Statistics (网络统计):
   - Basic statistics: nodes, edges, average degree
   - Connectivity: number and size of connected components
   - Distance metrics: average path length, diameter, effective diameter
   - Clustering coefficient

4. Visualization (可视化):
   - Degree distribution plots
   - Cumulative degree distribution (CCDF)
   - Hop plot (path length distribution)
   - Connected components distribution
   - Clustering coefficient distribution

/////////////////////////////////////////////////////////////////////////////
Parameters (参数):

   -i:Input graph file (edge list format, tab/space separated)
      Default: 'graph.txt'
      
   -o:Output file name prefix
      Default: 'opinion_leader'
      Output files:
        - {prefix}_centrality.tab: All centrality measures for all nodes
        - {prefix}_network_stats.txt: Network statistics
        - {prefix}_top{K}_leaders.txt: Top-K opinion leaders
        - {prefix}_deg_dist.*: Degree distribution plots
        - {prefix}_deg_ccdf.*: Cumulative degree distribution plots
        - {prefix}_hops.*: Hop plot
        - {prefix}_wcc.*: Weakly connected components distribution
        - {prefix}_scc.*: Strongly connected components distribution (if directed)
        - {prefix}_clust.*: Clustering coefficient distribution
      
   -t:Description/title for plots
      Default: 'Opinion Leader Mining'
      
   -d:Directed graph (true) or undirected (false)
      Default: true
      
   -k:Number of top opinion leaders to output
      Default: 50
      
   -p:Create visualization plots (true/false)
      Default: true

/////////////////////////////////////////////////////////////////////////////
Usage Examples (使用示例):

1. Basic usage with default parameters:
   ./opinion_leader -i:graph.txt

2. Analyze undirected graph and output top 100 leaders:
   ./opinion_leader -i:social_network.txt -d:false -k:100

3. Analyze directed graph without creating plots:
   ./opinion_leader -i:twitter_network.txt -p:false

4. Custom output prefix and description:
   ./opinion_leader -i:network.txt -o:results -t:"Twitter Network Analysis"

/////////////////////////////////////////////////////////////////////////////
Input Format (输入格式):

The input graph file should be an edge list, one edge per line:
  SourceNodeId  TargetNodeId

Example:
  1  2
  1  3
  2  4
  3  4

For directed graphs, the edge direction is from first to second column.
For undirected graphs, each edge should appear only once.

/////////////////////////////////////////////////////////////////////////////
Output Format (输出格式):

1. Centrality Results ({prefix}_centrality.tab):
   Tab-separated file with columns:
     NodeId, Degree, PageRank, HubScore, AuthorityScore, Betweenness,
     Closeness, Eigenvector, ClusteringCoeff, CompositeScore, Rank

2. Network Statistics ({prefix}_network_stats.txt):
   Text file with network properties

3. Top-K Leaders ({prefix}_top{K}_leaders.txt):
   Tab-separated file with top K opinion leaders

4. Visualization Files:
   Various plot files (PNG/PDF) generated using GnuPlot

/////////////////////////////////////////////////////////////////////////////
Algorithm Details (算法细节):

The composite score is calculated as:
  CompositeScore = 0.25*NormPR + 0.20*NormAuth + 0.20*NormBtw + 
                   0.15*NormClose + 0.10*NormEig + 0.05*NormHub + 
                   0.05*NormDeg

where each metric is normalized to [0,1] range before combination.

For large networks (>1000 nodes), the algorithm uses sampling for:
  - Betweenness Centrality: 10% node sampling
  - Closeness Centrality: up to 500 nodes

/////////////////////////////////////////////////////////////////////////////
Performance Notes (性能说明):

- For networks with >10,000 nodes, some computations may take significant time
- Betweenness and Closeness centrality are computationally expensive
- The program automatically uses sampling for large networks
- Visualization requires GnuPlot to be installed and in PATH

/////////////////////////////////////////////////////////////////////////////
Compilation (编译):

To compile from command line:
  cd examples/opinion_leader
  make all

Or use the provided Makefile in the examples directory.

/////////////////////////////////////////////////////////////////////////////

