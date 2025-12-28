#include "stdafx.h"

/////////////////////////////////////////////////
// Opinion Leader Mining Program
// 意见领袖挖掘：基于多中心性度量的综合评分算法

// 节点综合影响力评分结构
struct TNodeInfluence {
  int NId;
  double PageRank;
  double HubScore;
  double AuthorityScore;
  double Betweenness;
  double Closeness;
  double Eigenvector;
  double Degree;
  double ClusteringCoeff;
  double CompositeScore;  // 综合评分
  
  TNodeInfluence() : NId(-1), PageRank(0), HubScore(0), AuthorityScore(0),
    Betweenness(0), Closeness(0), Eigenvector(0), Degree(0), 
    ClusteringCoeff(0), CompositeScore(0) {}
  
  // 用于排序的比较函数
  static bool CmpCompositeScore(const TNodeInfluence& A, const TNodeInfluence& B) {
    return A.CompositeScore > B.CompositeScore; // 降序
  }
};

// 网络统计信息
struct TNetworkStats {
  int Nodes;
  int Edges;
  double AvgDegree;
  double AvgPathLength;
  double Diameter;
  double EffDiameter;
  double ClusteringCoeff;
  int NumWcc;
  int NumScc;
  int MxWccSize;
  int MxSccSize;
  
  TNetworkStats() : Nodes(0), Edges(0), AvgDegree(0), AvgPathLength(0),
    Diameter(0), EffDiameter(0), ClusteringCoeff(0), NumWcc(0), NumScc(0),
    MxWccSize(0), MxSccSize(0) {}
};

// 计算网络统计信息
template<class PGraph>
TNetworkStats ComputeNetworkStats(const PGraph& Graph, const bool& IsDir=false) {
  TNetworkStats Stats;
  Stats.Nodes = Graph->GetNodes();
  Stats.Edges = Graph->GetEdges();
  
  // 计算平均度
  double SumDeg = 0;
  for (typename PGraph::TObj::TNodeI NI = Graph->BegNI(); NI < Graph->EndNI(); NI++) {
    SumDeg += NI.GetDeg();
  }
  Stats.AvgDegree = Stats.Nodes > 0 ? SumDeg / Stats.Nodes : 0;
  
  // 计算聚类系数
  TIntFltH CcfH;
  TSnap::GetNodeClustCf(Graph, CcfH);
  double SumCcf = 0;
  for (TIntFltH::TIter I = CcfH.BegI(); I < CcfH.EndI(); I++) {
    SumCcf += I->Dat();
  }
  Stats.ClusteringCoeff = CcfH.Len() > 0 ? SumCcf / CcfH.Len() : 0;
  
  // 计算连通分量
  TCnComV WccV, SccV;
  TSnap::GetWccs(Graph, WccV);
  if (Graph->HasFlag(gfDirected)) {
    TSnap::GetSccs(Graph, SccV);
    Stats.NumScc = SccV.Len();
    if (SccV.Len() > 0) {
      Stats.MxSccSize = SccV[0].Len();
    }
  }
  Stats.NumWcc = WccV.Len();
  if (WccV.Len() > 0) {
    Stats.MxWccSize = WccV[0].Len();
  }
  
  // 计算有效直径和平均路径长度（使用ANF算法，适用于大规模网络）
  PGraph WccGraph = TSnap::GetMxWcc(Graph);
  if (WccGraph->GetNodes() > 1) {
    Stats.EffDiameter = TSnap::GetAnfEffDiam(WccGraph, IsDir, 0.9, 32);
    
    // 计算平均路径长度（采样）
    TIntFltKdV DistNbrsV;
    TSnap::GetAnf(WccGraph, DistNbrsV, -1, IsDir, 32);
    double SumDist = 0, SumPairs = 0;
    for (int i = 0; i < DistNbrsV.Len(); i++) {
      SumDist += DistNbrsV[i].Key * DistNbrsV[i].Dat;
      SumPairs += DistNbrsV[i].Dat;
    }
    Stats.AvgPathLength = SumPairs > 0 ? SumDist / SumPairs : 0;
    
    // 计算直径（采样BFS）
    if (WccGraph->GetNodes() < 10000) {
      Stats.Diameter = TSnap::GetBfsFullDiam(WccGraph, TMath::Mn(100, WccGraph->GetNodes()), IsDir);
    } else {
      Stats.Diameter = Stats.EffDiameter * 2; // 近似值
    }
  }
  
  return Stats;
}

// 计算所有中心性指标
template<class PGraph>
void ComputeCentralities(const PGraph& Graph, const PUNGraph& UGraph, 
                         TVec<TNodeInfluence>& NodeInfV) {
  printf("Computing centralities...\n");
  
  TIntFltH PRankH, HubH, AuthH, BtwH, EigH, CcfH;
  
  // PageRank (适用于有向图)
  if (Graph->HasFlag(gfDirected)) {
    printf("  PageRank...");
    TSnap::GetPageRank(Graph, PRankH, 0.85);
    printf(" done\n");
    
    // HITS算法
    printf("  HITS (Hubs & Authorities)...");
    TSnap::GetHits(Graph, HubH, AuthH);
    printf(" done\n");
  }
  
  // 特征向量中心性（无向图）
  printf("  Eigenvector Centrality...");
  TSnap::GetEigenVectorCentr(UGraph, EigH);
  printf(" done\n");
  
  // 聚类系数
  printf("  Clustering Coefficient...");
  TSnap::GetNodeClustCf(UGraph, CcfH);
  printf(" done\n");
  
  // 介数中心性（采样，因为计算量大）
  printf("  Betweenness Centrality (sampling)...");
  double NodeFrac = UGraph->GetNodes() > 1000 ? 0.1 : 1.0; // 大网络采样10%
  TSnap::GetBetweennessCentr(UGraph, BtwH, NodeFrac, false);
  printf(" done\n");
  
  // 接近中心性（仅计算部分节点，因为计算量大）
  printf("  Closeness Centrality (sampling)...");
  TIntFltH CloseH;
  TIntV SampleNodes;
  UGraph->GetNIdV(SampleNodes);
  if (SampleNodes.Len() > 500) {
    SampleNodes.Shuffle(TInt::Rnd);
    for (int i = 0; i < TMath::Mn(500, SampleNodes.Len()); i++) {
      CloseH.AddDat(SampleNodes[i], TSnap::GetClosenessCentr<PUNGraph>(UGraph, SampleNodes[i], false));
    }
  } else {
    for (TUNGraph::TNodeI NI = UGraph->BegNI(); NI < UGraph->EndNI(); NI++) {
      CloseH.AddDat(NI.GetId(), TSnap::GetClosenessCentr<PUNGraph>(UGraph, NI.GetId(), false));
    }
  }
  printf(" done\n");
  
  // 整合所有指标
  NodeInfV.Gen(UGraph->GetNodes());
  int idx = 0;
  for (TUNGraph::TNodeI NI = UGraph->BegNI(); NI < UGraph->EndNI(); NI++) {
    const int NId = NI.GetId();
    TNodeInfluence& Inf = NodeInfV[idx++];
    Inf.NId = NId;
    Inf.Degree = NI.GetDeg();
    Inf.PageRank = PRankH.IsKey(NId) ? PRankH.GetDat(NId) : TFlt(0.0);
    Inf.HubScore = HubH.IsKey(NId) ? HubH.GetDat(NId) : TFlt(0.0);
    Inf.AuthorityScore = AuthH.IsKey(NId) ? AuthH.GetDat(NId) : TFlt(0.0);
    Inf.Betweenness = BtwH.IsKey(NId) ? BtwH.GetDat(NId) : TFlt(0.0);
    Inf.Closeness = CloseH.IsKey(NId) ? CloseH.GetDat(NId) : TFlt(0.0);
    Inf.Eigenvector = EigH.IsKey(NId) ? EigH.GetDat(NId) : TFlt(0.0);
    Inf.ClusteringCoeff = CcfH.IsKey(NId) ? CcfH.GetDat(NId) : TFlt(0.0);
  }
  
  // 归一化并计算综合评分
  // 使用加权平均，权重可根据实际需求调整
  double MaxPR = 0, MaxHub = 0, MaxAuth = 0, MaxBtw = 0, MaxClose = 0, MaxEig = 0, MaxDeg = 0;
  for (int i = 0; i < NodeInfV.Len(); i++) {
    MaxPR = TMath::Mx(MaxPR, NodeInfV[i].PageRank);
    MaxHub = TMath::Mx(MaxHub, NodeInfV[i].HubScore);
    MaxAuth = TMath::Mx(MaxAuth, NodeInfV[i].AuthorityScore);
    MaxBtw = TMath::Mx(MaxBtw, NodeInfV[i].Betweenness);
    MaxClose = TMath::Mx(MaxClose, NodeInfV[i].Closeness);
    MaxEig = TMath::Mx(MaxEig, NodeInfV[i].Eigenvector);
    MaxDeg = TMath::Mx(MaxDeg, NodeInfV[i].Degree);
  }
  
  // 归一化并计算综合评分
  // 权重：PageRank(0.25), Authority(0.20), Betweenness(0.20), Closeness(0.15), 
  //       Eigenvector(0.10), Hub(0.05), Degree(0.05)
  for (int i = 0; i < NodeInfV.Len(); i++) {
    double NormPR = MaxPR > 0 ? NodeInfV[i].PageRank / MaxPR : 0;
    double NormAuth = MaxAuth > 0 ? NodeInfV[i].AuthorityScore / MaxAuth : 0;
    double NormBtw = MaxBtw > 0 ? NodeInfV[i].Betweenness / MaxBtw : 0;
    double NormClose = MaxClose > 0 ? NodeInfV[i].Closeness / MaxClose : 0;
    double NormEig = MaxEig > 0 ? NodeInfV[i].Eigenvector / MaxEig : 0;
    double NormHub = MaxHub > 0 ? NodeInfV[i].HubScore / MaxHub : 0;
    double NormDeg = MaxDeg > 0 ? NodeInfV[i].Degree / MaxDeg : 0;
    
    NodeInfV[i].CompositeScore = 
      0.25 * NormPR + 
      0.20 * NormAuth + 
      0.20 * NormBtw + 
      0.15 * NormClose + 
      0.10 * NormEig + 
      0.05 * NormHub + 
      0.05 * NormDeg;
  }
  
  // 按综合评分排序
  NodeInfV.SortCmp(TNodeInfluence::CmpCompositeScore);
}

// 输出结果到文件
void SaveResults(const TVec<TNodeInfluence>& NodeInfV, const TNetworkStats& Stats,
                 const TStr& OutFNm, const TStr& InFNm, const int& TopK=50) {
  // 保存节点中心性结果
  TStr CentrFNm = OutFNm + "_centrality.tab";
  FILE *F = fopen(CentrFNm.CStr(), "wt");
  fprintf(F, "# Opinion Leader Mining Results\n");
  fprintf(F, "# Network: %s\n", InFNm.CStr());
  fprintf(F, "# Nodes: %d\tEdges: %d\n", Stats.Nodes, Stats.Edges);
  fprintf(F, "# NodeId\tDegree\tPageRank\tHubScore\tAuthorityScore\tBetweenness\t");
  fprintf(F, "Closeness\tEigenvector\tClusteringCoeff\tCompositeScore\tRank\n");
  
  for (int i = 0; i < TMath::Mn(TopK, NodeInfV.Len()); i++) {
    const TNodeInfluence& Inf = NodeInfV[i];
    fprintf(F, "%d\t%.6f\t%.6f\t%.6f\t%.6f\t%.6f\t%.6f\t%.6f\t%.6f\t%.6f\t%d\n",
      Inf.NId, Inf.Degree, Inf.PageRank, Inf.HubScore, Inf.AuthorityScore,
      Inf.Betweenness, Inf.Closeness, Inf.Eigenvector, Inf.ClusteringCoeff,
      Inf.CompositeScore, i+1);
  }
  fclose(F);
  printf("Saved centrality results to: %s\n", CentrFNm.CStr());
  
  // 保存网络统计信息
  TStr StatsFNm = OutFNm + "_network_stats.txt";
  F = fopen(StatsFNm.CStr(), "wt");
  fprintf(F, "# Network Statistics\n");
  fprintf(F, "# Network: %s\n\n", InFNm.CStr());
  fprintf(F, "Basic Statistics:\n");
  fprintf(F, "  Nodes: %d\n", Stats.Nodes);
  fprintf(F, "  Edges: %d\n", Stats.Edges);
  fprintf(F, "  Average Degree: %.4f\n", Stats.AvgDegree);
  fprintf(F, "  Average Clustering Coefficient: %.4f\n", Stats.ClusteringCoeff);
  fprintf(F, "\nConnectivity:\n");
  fprintf(F, "  Number of WCC: %d\n", Stats.NumWcc);
  fprintf(F, "  Largest WCC Size: %d\n", Stats.MxWccSize);
  fprintf(F, "  Number of SCC: %d\n", Stats.NumScc);
  fprintf(F, "  Largest SCC Size: %d\n", Stats.MxSccSize);
  fprintf(F, "\nDistance Metrics:\n");
  fprintf(F, "  Average Path Length: %.4f\n", Stats.AvgPathLength);
  fprintf(F, "  Effective Diameter: %.4f\n", Stats.EffDiameter);
  fprintf(F, "  Diameter: %.4f\n", Stats.Diameter);
  fclose(F);
  printf("Saved network statistics to: %s\n", StatsFNm.CStr());
  
  // 保存Top-K意见领袖
  TStr TopKFNm = OutFNm + "_top" + TStr::Fmt("%d", TopK) + "_leaders.txt";
  F = fopen(TopKFNm.CStr(), "wt");
  fprintf(F, "# Top-%d Opinion Leaders\n", TopK);
  fprintf(F, "# Network: %s\n\n", InFNm.CStr());
  fprintf(F, "Rank\tNodeId\tCompositeScore\tPageRank\tAuthority\tBetweenness\tCloseness\n");
  for (int i = 0; i < TMath::Mn(TopK, NodeInfV.Len()); i++) {
    const TNodeInfluence& Inf = NodeInfV[i];
    fprintf(F, "%d\t%d\t%.6f\t%.6f\t%.6f\t%.6f\t%.6f\n",
      i+1, Inf.NId, Inf.CompositeScore, Inf.PageRank, 
      Inf.AuthorityScore, Inf.Betweenness, Inf.Closeness);
  }
  fclose(F);
  printf("Saved top-%d leaders to: %s\n", TopK, TopKFNm.CStr());
}

// 生成可视化图表
template<class PGraph>
void CreateVisualizations(const PGraph& Graph, const PUNGraph& UGraph,
                          const TStr& OutFNm, const TStr& Desc) {
  printf("Creating visualizations...\n");
  
  // 度分布图
  printf("  Degree distribution...");
  TSnap::PlotOutDegDistr(Graph, OutFNm + "_deg_dist", Desc, false, false);
  TSnap::PlotInDegDistr(Graph, OutFNm + "_deg_dist", Desc, false, false);
  printf(" done\n");
  
  // 累积度分布
  printf("  Cumulative degree distribution...");
  TSnap::PlotOutDegDistr(Graph, OutFNm + "_deg_ccdf", Desc, true, false);
  TSnap::PlotInDegDistr(Graph, OutFNm + "_deg_ccdf", Desc, true, false);
  printf(" done\n");
  
  // Hop plot (路径长度分布)
  printf("  Hop plot (path length distribution)...");
  TSnap::PlotHops(UGraph, OutFNm + "_hops", Desc, false, 32);
  printf(" done\n");
  
  // 连通分量分布
  printf("  Connected components distribution...");
  TSnap::PlotWccDistr(UGraph, OutFNm + "_wcc", Desc);
  if (Graph->HasFlag(gfDirected)) {
    TSnap::PlotSccDistr(Graph, OutFNm + "_scc", Desc);
  }
  printf(" done\n");
  
  // 聚类系数分布
  printf("  Clustering coefficient distribution...");
  TSnap::PlotClustCf(UGraph, OutFNm + "_clust", Desc);
  printf(" done\n");
  
  printf("Visualization files saved with prefix: %s\n", OutFNm.CStr());
}

int main(int argc, char* argv[]) {
  Env = TEnv(argc, argv, TNotify::StdNotify);
  Env.PrepArgs(TStr::Fmt("Opinion Leader Mining. build: %s, %s. Time: %s", 
    __TIME__, __DATE__, TExeTm::GetCurTm()));
  TExeTm ExeTm;
  
  Try
  const TStr InFNm = Env.GetIfArgPrefixStr("-i:", "graph.txt", 
    "Input graph file (edge list format)");
  const TStr OutFNm = Env.GetIfArgPrefixStr("-o:", "opinion_leader", 
    "Output file name prefix");
  const TStr Desc = Env.GetIfArgPrefixStr("-t:", "Opinion Leader Mining", 
    "Description/title");
  const TBool IsDir = Env.GetIfArgPrefixBool("-d:", true, 
    "Directed graph (true) or undirected (false)");
  const int TopK = Env.GetIfArgPrefixInt("-k:", 50, 
    "Number of top opinion leaders to output");
  const TBool CreatePlots = Env.GetIfArgPrefixBool("-p:", true, 
    "Create visualization plots");
  
  if (Env.IsEndOfRun()) { return 0; }
  
  // 加载图
  printf("Loading graph from: %s\n", InFNm.CStr());
  PNGraph Graph;
  PUNGraph UGraph;
  
  if (IsDir) {
    Graph = TSnap::LoadEdgeList<PNGraph>(InFNm);
    printf("Loaded directed graph: %d nodes, %d edges\n", 
      Graph->GetNodes(), Graph->GetEdges());
    UGraph = TSnap::ConvertGraph<PUNGraph>(Graph);
  } else {
    UGraph = TSnap::LoadEdgeList<PUNGraph>(InFNm);
    printf("Loaded undirected graph: %d nodes, %d edges\n", 
      UGraph->GetNodes(), UGraph->GetEdges());
    Graph = TSnap::ConvertGraph<PNGraph>(UGraph, true);
  }
  
  // 计算网络统计信息
  printf("\n=== Computing Network Statistics ===\n");
  TNetworkStats Stats = ComputeNetworkStats(UGraph, !IsDir);
  printf("Network Statistics:\n");
  printf("  Nodes: %d, Edges: %d\n", Stats.Nodes, Stats.Edges);
  printf("  Average Degree: %.4f\n", Stats.AvgDegree);
  printf("  Average Clustering Coefficient: %.4f\n", Stats.ClusteringCoeff);
  printf("  Number of WCC: %d (Largest: %d nodes)\n", Stats.NumWcc, Stats.MxWccSize);
  if (IsDir) {
    printf("  Number of SCC: %d (Largest: %d nodes)\n", Stats.NumScc, Stats.MxSccSize);
  }
  printf("  Average Path Length: %.4f\n", Stats.AvgPathLength);
  printf("  Effective Diameter: %.4f\n", Stats.EffDiameter);
  printf("  Diameter: %.4f\n", Stats.Diameter);
  
  // 计算中心性指标
  printf("\n=== Computing Centrality Measures ===\n");
  TVec<TNodeInfluence> NodeInfV;
  ComputeCentralities(Graph, UGraph, NodeInfV);
  
  // 输出结果
  printf("\n=== Saving Results ===\n");
  SaveResults(NodeInfV, Stats, OutFNm, InFNm, TopK);
  
  // 打印Top-K意见领袖
  printf("\n=== Top-%d Opinion Leaders ===\n", TopK);
  printf("Rank\tNodeId\tCompositeScore\tPageRank\tAuthority\tBetweenness\tCloseness\n");
  for (int i = 0; i < TMath::Mn(TopK, NodeInfV.Len()); i++) {
    const TNodeInfluence& Inf = NodeInfV[i];
    printf("%d\t%d\t%.6f\t%.6f\t%.6f\t%.6f\t%.6f\n",
      i+1, Inf.NId, Inf.CompositeScore, Inf.PageRank, 
      Inf.AuthorityScore, Inf.Betweenness, Inf.Closeness);
  }
  
  // 生成可视化
  if (CreatePlots) {
    printf("\n=== Creating Visualizations ===\n");
    CreateVisualizations(Graph, UGraph, OutFNm, Desc);
  }
  
  Catch
  printf("\nRun time: %s (%s)\n", ExeTm.GetTmStr(), 
    TSecTm::GetCurTm().GetTmStr().CStr());
  return 0;
}

